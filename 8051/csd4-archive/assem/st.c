#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "io.h"
#include "ex.h"
#include "op.h"
#include "res.h"

#define MAX_ST 20
typedef enum { BotS, IfS, ElseS, GroupS } StTag;
byte Active;
int Phase;

StTag SStack[MAX_ST], *SP;
byte IStack[MAX_ST], *IP;

static void Push(StTag Tag) {
   if (SP >= SStack + MAX_ST) Fatal("Statement too complex");
   *SP++ = Tag;
}

static void SetColon(byte Glo, Symbol Sym) {
   if (!Active) return;
   if (Sym->Defined) {
      if (*Sym->Name != '#') {
         Error("Attempting to redefine %s.", Sym->Name); return;
      } else if (Glo) {
         Error("Attempting to make local label global."); return;
      } else {
         Sym->Address = 1, Sym->Global = 0, Sym->Variable = 1;
      }
   } else if (Sym->Global) {
      if (!Glo) {
         Error("Symbol %s already declared as external.", Sym->Name); return;
      } else if (!Sym->Address) {
         Error("Symbol %s already declared as number.", Sym->Name); return;
      } else if (Sym->Seg->Type != SegP->Type) {
         Error("Type mismatch: %s.", Sym->Name); return;
      }
   } else
      Sym->Address = 1, Sym->Global = Glo, Sym->Variable = 0;
   Sym->Defined = 1, Sym->Seg = SegP, Sym->Offset = CurLoc;
}

static void SetVar(byte Glo, Symbol Sym, Exp E) {
   if (!Active) return;
   if (Sym->Defined && !Sym->Variable)
      Error("Symbol %s already defined as constant.", Sym->Name);
   else if (Glo)
      Error("Variables cannot be made global.");
   else if (Sym->Global)
      Error("Symbol %s already declared as external.", Sym->Name);
   else if (!Sym->Defined) {
      if (E->Tag == AddrX)
         Sym->Address = 1, Sym->Seg = SegOf(E), Sym->Offset = OffOf(E);
      else
         Sym->Address = 0, Sym->Offset = ValOf(E);
      Sym->Variable = 1, Sym->Defined = 1;
   } else if (Sym->Address) {
      if (E->Tag != AddrX)
         Sym->Seg = SegTab + Sym->Seg->Type, Sym->Offset = ValOf(E);
      else if (SegOf(E)->Type != Sym->Seg->Type)
         Error("Type mismatch: %s.", Sym->Name);
      else
         Sym->Seg = SegOf(E), Sym->Offset = OffOf(E);
   } else if (E->Tag == AddrX) {
      if (SegOf(E)->Rel)
         Error("Symbol %s cannot be set to relative address.", Sym->Name);
      else
         Sym->Offset = SegOf(E)->Base + OffOf(E);
   } else
      Sym->Offset = ValOf(E);
}

void SetConst(byte Glo, byte Addr, byte Type, Symbol Sym, Exp E) {
   if (!Active) return;
   if (Addr && E->Tag == AddrX && SegOf(E)->Type != Type) {
      Error("Expression type does not match."); return;
   } else if (Sym->Defined) {
      Error("Symbol %s already defined.", Sym->Name); return;
   } else if (!Sym->Global)
      Sym->Global = Glo, Sym->Variable = 0, Sym->Address = (E->Tag == AddrX);
   else if (!Glo) {
      Error("Symbol %s already declared as external.", Sym->Name); return;
   } else if (Sym->Address) {
      if (Addr && Sym->Seg->Type != Type) {
         Error("Symbol %s's type does not match.", Sym->Name); return;
      } else if (E->Tag != AddrX) {
         Sym->Defined = 1, Sym->Offset = ValOf(E); return;
      } else if (SegOf(E)->Type != Sym->Seg->Type) {
         Error("Type mismatch: %s.", Sym->Name); return;
      }
   } else if (Addr) {
      Error("Symbol %s already declared as external number.", Sym->Name);
      return;
   } else if (E->Tag == AddrX) {
      if (SegOf(E)->Rel) {
         Error("Symbol %s cannot be equated to relative address.", Sym->Name);
         return;
      } else {
         Sym->Defined = 1, Sym->Offset = SegOf(E)->Base + OffOf(E); return;
      }
   }
   Sym->Defined = 1;
   if (E->Tag == AddrX)
      Sym->Seg = SegOf(E), Sym->Offset = OffOf(E);
   else
      Sym->Offset = ValOf(E);
}

FILE *OpenObj(char *Obj) {
   unsigned long L = 0L;
   FILE *FP = fopen(Obj, "wb");
   if (FP == NULL) return NULL;
   PutW(0x55aa, FP);
   for (int I = 0; I < 8; I++) PutL(L, FP); /* Empty the header */
   fclose(FP);
   FP = fopen(Obj, "rb+");
   if (FP == NULL) return NULL;
   fseek(FP, 34L, SEEK_SET); /* Skip the header */
   return FP;
}

void Assemble(char *Path) {
   Phase = 0;
   SymInit(), FileInit(), SegInit(), ExpInit(), ResInit();
   IP = IStack, SP = SStack;
   Active = 1; RegInit(); OpenF(Path);
   Lexical L = Scan();
   byte Glo, Addr, Type; Symbol Label; Exp E;
Start:
   if (L == 0) { EndSeg(); return; }
   Push(BotS);
BegSt:
   StartLine = Line, StartF = CurF;
   switch (L) {
      case IF:
         if ((L = Scan()) != LPAR) Error("Missing '(' in 'if (...)'"); else Scan();
         E = Parse(0);
         Push(IfS), *IP++ = Active;
         if (Active) Active = ValOf(E);
         L = OldL;
         if (L != RPAR) Error("Missing ')' in 'if (...)'."); else L = Scan();
      goto BegSt;
      case LCURL:
         if ((L = Scan()) == RCURL) { L = Scan(); goto EndSt; }
         Push(GroupS);
      goto BegSt;
      case SEMI: L = Scan(); goto EndSt;
      case RB:
         Scan(), InSemi = 1, E = Parse(0);
         Space(ValOf(E));
      goto AtSemi;
      case RW:
         Scan(), InSemi = 1, E = Parse(0);
         Space(2*ValOf(E));
      goto AtSemi;
      case SEG:
         if ((L = Scan()) != TYPE) { Error("Expected segment type."); goto FlushSt; }
         if (Value != CODE && Value != XDATA && Value != DATA) {
            Error("Only code, xdata, or data segments allowed."); goto FlushSt;
         }
         Type = Value;
         InSemi = 1, L = Scan();
         if (L == ORG) { InSemi = 0; goto DoOrg; }
         if (Active) EndSeg(), StartSeg(Type, 1, 0);
      goto AtSemi;
      case ORG:
         Type = SegP->Type;
      DoOrg:
         Scan(), InSemi = 1, E = Parse(0);
         if (Active) EndSeg(), StartSeg(Type, 0, ValOf(E));
      goto AtSemi;
      case INCLUDE: {
         if ((L = Scan()) != STRING) Fatal("Missing filename in 'include'.");
         char *S = CopyS(Text);
         InSemi = 1;
         if ((L = Scan()) != SEMI) Fatal("Missing ';' after include statement.");
         OpenF(S), free(S);
         InSemi = 0, L = Scan();
      }
      goto EndSt;
      case GLOBAL:
         Glo = 1;
         if ((L = Scan()) != SYMBOL) {
            Error("Expected symbol after 'global'"); goto FlushSt;
         }
      goto Define;
      case SYMBOL:
         Glo = 0;
      Define:
         Label = Sym;
         switch (L = Scan()) {
            case COLON:
               SetColon(Glo, Label);
               InSemi = 1, L = Scan(), InSemi = 0;
            goto BegSt;
            case SET:
               Scan(), InSemi = 1, E = Parse(1), SetVar(Glo, Label, E);
            goto AtSemi;
            case TYPE: Type = Value, Addr = 1; goto DoEqu;
            case EQU: Type = 0, Addr = 0;
            DoEqu:
               Scan(), InSemi = 1, E = Parse(1);
               SetConst(Glo, Addr, Type, Label, E);
            goto AtSemi;
            default: Error("Undefined symbol: %s.", Label->Name); break;
         }
      goto FlushSt;
      case EXTERN:
         if ((L = Scan()) == EQU) Addr = 0;
         else if (L == TYPE) Addr = 1, Type = Value;
         else {
            Error("Expected type or 'equ' after 'extern'."); goto FlushSt;
         }
         InSemi = 1;
         do {
            if ((L = Scan()) != SYMBOL) {
               Error("Expected symbol in 'extern'"); goto FlushSt;
            }
            if (Active) {
               if (Sym->Global) {
                  byte Addr1 = Sym->Address;
                  if (!Addr && Addr1)
                     Error("Redeclaring number %s as address.", Sym->Name);
                  else if (Addr && !Addr1)
                     Error("Redeclaring address %s as number.", Sym->Name);
                  else if (Addr && Addr1 && Type != Sym->Seg->Type)
                     Error("Type mismatch: %s.", Sym->Name);
               } else if (Sym->Defined)
                  Error("Redeclaring local symbol %s as external.", Sym->Name);
               else {
                  Sym->Global = 1;
                  if (Addr) Sym->Address = 1, Sym->Seg = SegTab + Type;
               }
            }
            Scan();
         } while (OldL == COMMA);
      goto AtSemi;
      case DB:
         InSemi = 1;
         do
            if ((L = Scan()) == STRING) PString(Text), Scan();
            else Reloc(0, 'b', Parse(2));
         while (OldL == COMMA);
      goto AtSemi;
      case DW:
         InSemi = 1;
         do
            Scan(), Reloc(0, 'w', Parse(2));
         while (OldL == COMMA);
      goto AtSemi;
      case END:
         if (Active) { EndSeg(); return; }
         InSemi = 1, Scan();
      goto AtSemi;
      case MNEMONIC: InSemi = 1, ParseArgs((byte)Value); goto AtSemi;
      default: Error("Syntax error"); goto FlushSt;
   }
FlushSt:
   InSemi = 1;
   while (L != SEMI) {
      if (L == 0) { InSemi = 0; goto EndSt; }
      L = Scan();
   }
AtSemi:
   InSemi = 0; L = OldL;
   if (L != SEMI) Error("Missing ';'"); else L = Scan();
EndSt:
   switch (*--SP) {
      case BotS: goto Start;
      case IfS:
         if (L == ELSE) {
            if (IP == IStack || IP[-1]) Active = !Active;
            *SP++ = ElseS, L = Scan(); goto BegSt;
         }
      case ElseS: EndIf:
         if (*--IP && !Active) {
            Active = 1;
            if (L == SYMBOL) Sym = LookUp(Text);
         }
      goto EndSt;
      case GroupS:
         if (L == RCURL) { L = Scan(); goto EndSt; }
         if (L == 0) { Error("Missing '}'."); goto EndSt; }
         *SP++ = GroupS;
      goto BegSt;
   }
}

static void Purge(void) {
   for (Symbol Sym = NIL->Next[0]; Sym != NIL; ) {
      Symbol NextS = Sym->Next[0];
      free(Sym->Name), free(Sym), Sym = NextS;
   }
   for (char **F = FileTab; F < FileTab + Files; F++) free(*F);
   free(FileTab);
   free(NIL);
   for (Exp E = ExpHead; E != NULL; ) {
      Exp NextE = E->Next; free(E), E = NextE;
   }
   free(RTab);
}

void Generate(void) {
   Phase = 1;
   unsigned long SegLoc = ftell(OutF), Segs = SegP - SegTab, Gaps = GapP - GapTab;
   for (Item IP = RTab; IP < RTab + RCur; IP++) Resolve(IP);
   fseek(OutF, SegLoc, SEEK_SET); /* Skip the code image */
   for (word File = 0; File < Files; File++) {
      word L = strlen(FileTab[File]);
      PutW(L, OutF), fwrite(FileTab[File], 1, L, OutF);
   }
   for (Segment Seg = SegTab + TYPES; Seg < SegP; Seg++) {
      word U = (Seg->Rel&1) << 8 | Seg->Type&0xff;
      PutW(Seg->Line, OutF), PutW(Seg->File, OutF),
      PutW(U, OutF), PutW(Seg->Size, OutF),
      PutW(Seg->Base, OutF), PutL(Seg->Loc, OutF);
   }
   for (Gap G = GapTab; G < GapP; G++)
      PutW(G->Seg - SegTab, OutF), PutW(G->Offset, OutF), PutW(G->Size, OutF);
   unsigned long Syms = 0UL;
   for (Symbol S = NIL->Next[0]; S != NIL; S = S->Next[0]) if (S->Map || S->Global && S->Defined) {
      byte B = (S->Global&1) << 3  | (S->Defined&1) << 2 |
               (S->Address&1) << 1 | (S->Variable&1);
      word U = S->Seg - SegTab, L = strlen(S->Name);
      if (!S->Global && !S->Defined)
         Error("Undefined symbol: %s.", S->Name);
      S->Index = Syms++;
      PutB(B, OutF), PutW(U, OutF), PutW(S->Offset, OutF), PutW(L, OutF);
      if (L > 0) fwrite(S->Name, 1, L, OutF);
   }
   unsigned long Exps = 0UL;
   for (Exp E = ExpHead; E != NULL; E = E->Next) if (E->Map) {
      byte Tag = E->Tag;
      E->Hash = Exps++;
      PutW(E->Line, OutF), PutW(E->File, OutF), PutB(Tag, OutF);
      switch (Tag) {
         case NumX: PutW(ValOf(E), OutF); break;
         case AddrX:
            PutW(SegOf(E) - SegTab, OutF), PutW(OffOf(E), OutF);
         break;
         case SymX: PutW(SymOf(E)->Index, OutF); break;
         case UnX:
            PutB(OpOf(E), OutF), PutW(ArgA(E)->Hash, OutF);
         break;
         case BinX:
            PutB(OpOf(E), OutF), PutW(ArgA(E)->Hash, OutF),
            PutW(ArgB(E)->Hash, OutF);
         break;
         case CondX:
            PutW(ArgA(E)->Hash, OutF), PutW(ArgB(E)->Hash, OutF),
            PutW(ArgC(E)->Hash, OutF);
         break;
      }
   }
   unsigned long Relocs = 0UL;
   for (Item IP = RTab; IP < RTab + RCur; IP++) if (IP->Map) {
      word U = IP->Seg - SegTab;
      Relocs++;
      PutW(IP->Line, OutF), PutW(IP->File, OutF),
      PutB(IP->Tag, OutF), PutW(IP->E->Hash, OutF),
      PutW(U, OutF),
      PutW(IP->Offset, OutF);
   }
   fseek(OutF, 2L, SEEK_SET); /* Do the header */
   PutL(SegLoc, OutF), PutL(Files, OutF), PutL(Segs, OutF),
   PutL(Gaps, OutF), PutL(Syms, OutF), PutL(Exps, OutF), PutL(Relocs, OutF);
   PutL(SegLoc + Files + Segs + Gaps + Syms + Exps + Relocs, OutF);
   fclose(OutF);
   Purge();
}
