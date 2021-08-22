#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "io.h"
#include "ex.h"
#include "op.h"
#include "res.h"

#define MAX_ST 20
typedef enum { BotS, IfS, ElseS, GroupS } StTag;
bool Active;
int Phase;

StTag SStack[MAX_ST], *SP;
byte IStack[MAX_ST], *IP;

static void Push(StTag Tag) {
   if (SP >= SStack + MAX_ST) Fatal("Statement too complex");
   *SP++ = Tag;
}

static void SetColon(bool Global, Symbol Sym) {
   if (!Active) return;
   if (Sym->Defined) {
      if (*Sym->Name != '#') {
         Error("Attempting to redefine %s.", Sym->Name); return;
      } else if (Global) {
         Error("Attempting to make local label global."); return;
      } else
         Sym->Address = true, Sym->Global = false, Sym->Variable = true;
   } else if (Sym->Global) {
      if (!Global) {
         Error("Symbol %s already declared as external.", Sym->Name); return;
      } else if (!Sym->Address) {
         Error("Symbol %s already declared as number.", Sym->Name); return;
      } else if (Sym->Seg->Type != SegP->Type) {
         Error("Type mismatch: %s.", Sym->Name); return;
      }
   } else
      Sym->Address = true, Sym->Global = Global, Sym->Variable = false;
   Sym->Defined = true, Sym->Seg = SegP, Sym->Offset = CurLoc;
}

static void SetVar(bool Global, Symbol Sym, Exp E) {
   if (!Active) return;
   if (Sym->Defined && !Sym->Variable)
      Error("Symbol %s already defined as constant.", Sym->Name);
   else if (Global)
      Error("Variables cannot be made global.");
   else if (Sym->Global)
      Error("Symbol %s already declared as external.", Sym->Name);
   else if (!Sym->Defined) {
      if (E->Tag == AddrX)
         Sym->Address = true, Sym->Seg = SegOf(E), Sym->Offset = OffOf(E);
      else
         Sym->Address = false, Sym->Offset = ValOf(E);
      Sym->Variable = true, Sym->Defined = true;
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

static void SetConst(byte Global, bool Addr, byte Type, Symbol Sym, Exp E) {
   if (!Active) return;
   if (Addr && E->Tag == AddrX && SegOf(E)->Type != Type) {
      Error("Expression type does not match."); return;
   } else if (Sym->Defined) {
      Error("Symbol %s already defined.", Sym->Name); return;
   } else if (!Sym->Global)
      Sym->Global = Global, Sym->Variable = false, Sym->Address = E->Tag == AddrX;
   else if (!Global) {
      Error("Symbol %s already declared as external.", Sym->Name); return;
   } else if (Sym->Address) {
      if (Addr && Sym->Seg->Type != Type) {
         Error("Symbol %s's type does not match.", Sym->Name); return;
      } else if (E->Tag != AddrX) {
         Sym->Defined = true, Sym->Offset = ValOf(E); return;
      } else if (SegOf(E)->Type != Sym->Seg->Type) {
         Error("Type mismatch: %s.", Sym->Name); return;
      }
   } else if (Addr) {
      Error("Symbol %s already declared as external number.", Sym->Name); return;
   } else if (E->Tag == AddrX) {
      if (SegOf(E)->Rel) {
         Error("Symbol %s cannot be equated to relative address.", Sym->Name); return;
      } else {
         Sym->Defined = true, Sym->Offset = SegOf(E)->Base + OffOf(E); return;
      }
   }
   Sym->Defined = true;
   if (E->Tag == AddrX)
      Sym->Seg = SegOf(E), Sym->Offset = OffOf(E);
   else
      Sym->Offset = ValOf(E);
}

FILE *OpenObj(char *Obj) {
   unsigned long L = 0L;
   FILE *ExF = fopen(Obj, "wb");
   if (ExF == NULL) return NULL;
   PutW(0x55aa, ExF);
   for (int I = 0; I < 8; I++) PutL(L, ExF); // Empty the header.
   fclose(ExF), ExF = fopen(Obj, "rb+"); // Reopen.
   if (ExF == NULL) return NULL;
   fseek(ExF, 34L, SEEK_SET); // Skip the header.
   return ExF;
}

void Assemble(char *Path) {
   Phase = 0;
   SymInit(), FileInit(), SegInit(), ExpInit(), ResInit();
   IP = IStack, SP = SStack;
   Active = true; RegInit(); OpenF(Path);
   Lexical L = Scan();
   bool Global, Addr;
   byte Type; Symbol Label; Exp E;
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
         if (Active) Active = ValOf(E) != 0;
         if ((L = OldL) != RPAR) Error("Missing ')' in 'if (...)'."); else L = Scan();
      goto BegSt;
      case LCURL:
         if ((L = Scan()) == RCURL) { L = Scan(); goto EndSt; }
         Push(GroupS);
      goto BegSt;
      case SEMI: L = Scan(); goto EndSt;
      case RB:
         Scan(), InSemi = true, Space(ValOf(Parse(0)));
      goto AtSemi;
      case RW:
         Scan(), InSemi = true, Space(2*ValOf(Parse(0)));
      goto AtSemi;
      case SEG:
         if ((L = Scan()) != TYPE) { Error("Expected segment type."); goto FlushSt; }
         if (Value != CODE && Value != XDATA && Value != DATA) {
            Error("Only code, xdata, or data segments allowed."); goto FlushSt;
         }
         Type = Value, InSemi = true;
         if ((L = Scan()) == ORG) { InSemi = false; goto DoOrg; }
         if (Active) EndSeg(), StartSeg(Type, 1, 0);
      goto AtSemi;
      case ORG:
         Type = SegP->Type;
      DoOrg:
         Scan(), InSemi = true, E = Parse(0);
         if (Active) EndSeg(), StartSeg(Type, 0, ValOf(E));
      goto AtSemi;
      case INCLUDE: {
         if ((L = Scan()) != STRING) Fatal("Missing filename in 'include'.");
         char *S = CopyS(Text);
         InSemi = true;
         if ((L = Scan()) != SEMI) Fatal("Missing ';' after include statement.");
         OpenF(S), free(S);
         InSemi = false, L = Scan();
      }
      goto EndSt;
      case GLOBAL:
         Global = true;
         if ((L = Scan()) != SYMBOL) {
            Error("Expected symbol after 'global'"); goto FlushSt;
         }
      goto Define;
      case SYMBOL:
         Global = false;
      Define:
         Label = Sym;
         switch (L = Scan()) {
            case COLON:
               SetColon(Global, Label), InSemi = true, L = Scan(), InSemi = false;
            goto BegSt;
            case SET:
               Scan(), InSemi = true, SetVar(Global, Label, Parse(1));
            goto AtSemi;
            case TYPE: Type = Value, Addr = true; goto DoEqu;
            case EQU: Type = 0, Addr = false;
            DoEqu:
               Scan(), InSemi = true, SetConst(Global, Addr, Type, Label, Parse(1));
            goto AtSemi;
            default: Error("Undefined symbol: %s.", Label->Name); break;
         }
      goto FlushSt;
      case EXTERN:
         if ((L = Scan()) == EQU) Addr = false;
         else if (L == TYPE) Addr = true, Type = Value;
         else {
            Error("Expected type or 'equ' after 'extern'."); goto FlushSt;
         }
         InSemi = true;
         do {
            if ((L = Scan()) != SYMBOL) {
               Error("Expected symbol in 'extern'"); goto FlushSt;
            }
            if (Active) {
               if (Sym->Global) {
                  bool Addr1 = Sym->Address;
                  if (!Addr && Addr1)
                     Error("Redeclaring number %s as address.", Sym->Name);
                  else if (Addr && !Addr1)
                     Error("Redeclaring address %s as number.", Sym->Name);
                  else if (Addr && Addr1 && Type != Sym->Seg->Type)
                     Error("Type mismatch: %s.", Sym->Name);
               } else if (Sym->Defined)
                  Error("Redeclaring local symbol %s as external.", Sym->Name);
               else {
                  Sym->Global = true;
                  if (Addr) Sym->Address = true, Sym->Seg = SegTab + Type;
               }
            }
            Scan();
         } while (OldL == COMMA);
      goto AtSemi;
      case DB:
         InSemi = true;
         do
            if ((L = Scan()) == STRING) PString(Text), Scan();
            else Reloc(0, 'b', Parse(2));
         while (OldL == COMMA);
      goto AtSemi;
      case DW:
         InSemi = true;
         do
            Scan(), Reloc(0, 'w', Parse(2));
         while (OldL == COMMA);
      goto AtSemi;
      case END:
         if (Active) { EndSeg(); return; }
         InSemi = true, Scan();
      goto AtSemi;
      case MNEMONIC: InSemi = true, ParseArgs((byte)Value); goto AtSemi;
      default: Error("Syntax error"); goto FlushSt;
   }
FlushSt:
   InSemi = true;
   while (L != SEMI) {
      if (L == 0) { InSemi = false; goto EndSt; }
      L = Scan();
   }
AtSemi:
   InSemi = false; L = OldL;
   if (L != SEMI) Error("Missing ';'"); else L = Scan();
EndSt:
   switch (*--SP) {
      case BotS: goto Start;
      case IfS:
         if (L == ELSE) {
            if (IP == IStack || IP[-1]) Active = !Active;
            *SP++ = ElseS, L = Scan(); goto BegSt;
         }
      case ElseS:
         if (*--IP && !Active) {
            Active = true;
            if (L == SYMBOL) Sym = LookUp(Text);
         }
      goto EndSt;
      case GroupS:
         if (L == RCURL) { L = Scan(); goto EndSt; }
         else if (L == 0) { Error("Missing '}'."); goto EndSt; }
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
   free(FileTab), free(NIL);
   for (Exp E = ExpHead; E != NULL; ) {
      Exp NextE = E->Next; free(E), E = NextE;
   }
   free(RTab);
}

void Generate(void) {
   Phase = 1;
   unsigned long SegLoc = ftell(ExF), Segs = SegP - SegTab, Gaps = GapP - GapTab;
   for (Item IP = RTab; IP < RTab + RCur; IP++) Resolve(IP);
   fseek(ExF, SegLoc, SEEK_SET); // Skip the code image.
   for (word File = 0; File < Files; File++) {
      word L = strlen(FileTab[File]);
      PutW(L, ExF), fwrite(FileTab[File], 1, L, ExF);
   }
   for (Segment Seg = SegTab + TYPES; Seg < SegP; Seg++)
      PutW(Seg->Line, ExF), PutW(Seg->File, ExF), PutW((Seg->Rel&1) << 8 | Seg->Type&0xff, ExF),
      PutW(Seg->Size, ExF), PutW(Seg->Base, ExF), PutL(Seg->Loc, ExF);
   for (Gap G = GapTab; G < GapP; G++)
      PutW(G->Seg - SegTab, ExF), PutW(G->Offset, ExF), PutW(G->Size, ExF);
   unsigned long Syms = 0UL;
   for (Symbol S = NIL->Next[0]; S != NIL; S = S->Next[0]) if (S->Map || S->Global && S->Defined) {
      byte B = S->Global << 3  | S->Defined << 2 | S->Address << 1 | S->Variable;
      word L = strlen(S->Name);
      if (!S->Global && !S->Defined)
         Error("Undefined symbol: %s.", S->Name);
      S->Index = Syms++;
      PutB(B, ExF), PutW(S->Seg - SegTab, ExF), PutW(S->Offset, ExF), PutW(L, ExF);
      if (L > 0) fwrite(S->Name, 1, L, ExF);
   }
   unsigned long Exps = 0UL;
   for (Exp E = ExpHead; E != NULL; E = E->Next) if (E->Map) {
      byte Tag = E->Tag;
      E->Hash = Exps++;
      PutW(E->Line, ExF), PutW(E->File, ExF), PutB(Tag, ExF);
      switch (Tag) {
         case NumX: PutW(ValOf(E), ExF); break;
         case AddrX:
            PutW(SegOf(E) - SegTab, ExF), PutW(OffOf(E), ExF);
         break;
         case SymX: PutW(SymOf(E)->Index, ExF); break;
         case UnX:
            PutB(OpOf(E), ExF), PutW(ArgA(E)->Hash, ExF);
         break;
         case BinX:
            PutB(OpOf(E), ExF), PutW(ArgA(E)->Hash, ExF), PutW(ArgB(E)->Hash, ExF);
         break;
         case CondX:
            PutW(ArgA(E)->Hash, ExF), PutW(ArgB(E)->Hash, ExF), PutW(ArgC(E)->Hash, ExF);
         break;
      }
   }
   unsigned long Relocs = 0UL;
   for (Item IP = RTab; IP < RTab + RCur; IP++) if (IP->Map)
      Relocs++,
      PutW(IP->Line, ExF), PutW(IP->File, ExF), PutB(IP->Tag, ExF),
      PutW(IP->E->Hash, ExF), PutW((word)(IP->Seg - SegTab), ExF), PutW(IP->Offset, ExF);
   fseek(ExF, 2L, SEEK_SET); // Do the header.
   PutL(SegLoc, ExF), PutL(Files, ExF), PutL(Segs, ExF),
   PutL(Gaps, ExF), PutL(Syms, ExF), PutL(Exps, ExF), PutL(Relocs, ExF);
   PutL(SegLoc + Files + Segs + Gaps + Syms + Exps + Relocs, ExF);
   fclose(ExF);
   Purge();
}
