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

void PUSH(StTag Tag) {
   if (SP >= SStack + MAX_ST) FATAL("Statement too complex");
   *SP++ = Tag;
}

void SetColon(byte Glo, Symbol Sym) {
   if (!Active) return;
   if (Sym->Defined) {
      if (*Sym->Name != '#') {
         ERROR("Attempting to redefine %s.", Sym->Name); return;
      } else if (Glo) {
         ERROR("Attempting to make local label global."); return;
      } else {
         Sym->Address = 1, Sym->Global = 0, Sym->Variable = 1;
      }
   } else if (Sym->Global) {
      if (!Glo) {
         ERROR("Symbol %s already declared as external.", Sym->Name); return;
      } else if (!Sym->Address) {
         ERROR("Symbol %s already declared as number.", Sym->Name); return;
      } else if (Sym->Seg->Type != SegP->Type) {
         ERROR("Type mismatch: %s.", Sym->Name); return;
      }
   } else
      Sym->Address = 1, Sym->Global = Glo, Sym->Variable = 0;
   Sym->Defined = 1, Sym->Seg = SegP, Sym->Offset = LOC;
}

void SetVar(byte Glo, Symbol Sym, Exp E) {
   if (!Active) return;
   if (Sym->Defined && !Sym->Variable)
      ERROR("Symbol %s already defined as constant.", Sym->Name);
   else if (Glo)
      ERROR("Variables cannot be made global.");
   else if (Sym->Global)
      ERROR("Symbol %s already declared as external.", Sym->Name);
   else if (!Sym->Defined) {
      if (E->Tag == AddrX)
         Sym->Address = 1, Sym->Seg = SEG(E), Sym->Offset = OFFSET(E);
      else
         Sym->Address = 0, Sym->Offset = VALUE(E);
      Sym->Variable = 1, Sym->Defined = 1;
   } else if (Sym->Address) {
      if (E->Tag != AddrX)
         Sym->Seg = SegTab + Sym->Seg->Type, Sym->Offset = VALUE(E);         
      else if (SEG(E)->Type != Sym->Seg->Type)
         ERROR("Type mismatch: %s.", Sym->Name);
      else
         Sym->Seg = SEG(E), Sym->Offset = OFFSET(E);
   } else if (E->Tag == AddrX) {
      if (SEG(E)->Rel)
         ERROR("Symbol %s cannot be set to relative address.", Sym->Name);
      else
         Sym->Offset = SEG(E)->Base + OFFSET(E);
   } else
      Sym->Offset = VALUE(E);
}

void SetConst(byte Glo, byte Addr, byte Type, Symbol Sym, Exp E) {
   if (!Active) return;
   if (Addr && E->Tag == AddrX && SEG(E)->Type != Type) {
      ERROR("Expression type does not match."); return;
   } else if (Sym->Defined) {
      ERROR("Symbol %s already defined.", Sym->Name); return;
   } else if (!Sym->Global)
      Sym->Global = Glo, Sym->Variable = 0, Sym->Address = (E->Tag == AddrX);
   else if (!Glo) {
      ERROR("Symbol %s already declared as external.", Sym->Name); return;
   } else if (Sym->Address) {
      if (Addr && Sym->Seg->Type != Type) {
         ERROR("Symbol %s's type does not match.", Sym->Name); return;
      } else if (E->Tag != AddrX) {
         Sym->Defined = 1, Sym->Offset = VALUE(E); return;
      } else if (SEG(E)->Type != Sym->Seg->Type) {
         ERROR("Type mismatch: %s.", Sym->Name); return;
      }
   } else if (Addr) {
      ERROR("Symbol %s already declared as external number.", Sym->Name);
      return;
   } else if (E->Tag == AddrX) {
      if (SEG(E)->Rel) {
         ERROR("Symbol %s cannot be equated to relative address.", Sym->Name);
         return;
      } else {
         Sym->Defined = 1, Sym->Offset = SEG(E)->Base + OFFSET(E); return;
      }
   }
   Sym->Defined = 1;
   if (E->Tag == AddrX)
      Sym->Seg = SEG(E), Sym->Offset = OFFSET(E);
   else
      Sym->Offset = VALUE(E);
}

FILE *OpenObj(char *Obj) {
   FILE *FP; word MAGIC = 0x55aa;
   unsigned long L = 0; int I;
   FP = fopen(Obj, "wb");
   if (FP == 0) return 0;
   PutW(MAGIC, FP);
   for (I = 0; I < 8; I++) PutL(L, FP); /* Empty the header */
   fclose(FP);
   FP = fopen(Obj, "rb+");
   if (FP == 0) return 0;
   fseek(FP, 34L, SEEK_SET); /* Skip the header */
   return FP;
}

void Assemble(char *Path) {
   Lexical L; Exp E; byte Glo, Addr, Type; Symbol Label;
   Phase = 0;
   SymInit(), FileInit(), SegInit(), ExpInit(), ResInit();
   IP = IStack, SP = SStack;
   Active = 1; RegInit(); OpenF(Path); L = Scan();
START:
   if (L == 0) { EndSeg(); return; }
   PUSH(BotS);
STATEMENT:
   StartLine = Line, StartF = CurF;
   switch (L) {
      case IF:
         L = Scan();
         if (L != LPAR) ERROR("Missing '(' in 'if (...)'"); else Scan();
         E = Parse(0);
         PUSH(IfS), *IP++ = Active;
         if (Active) Active = VALUE(E);
         L = OldL;
         if (L != RPAR) ERROR("Missing ')' in 'if (...)'."); else L = Scan();
      goto STATEMENT;
      case LCURL:
         L = Scan();
         if (L == RCURL) { L = Scan(); goto END_ST; }
         PUSH(GroupS);
      goto STATEMENT;
      case SEMI: L = Scan(); goto END_ST;
      case RB:
         Scan(), InSemi = 1, E = Parse(0);
         Space(VALUE(E));
      goto SEM;
      case RW:
         Scan(), InSemi = 1, E = Parse(0);
         Space(2*VALUE(E));
      goto SEM;
      case SEG:
         L = Scan();
         if (L != TYPE) { ERROR("Expected segment type."); goto PURGE; }
         if (Value != CODE && Value != XDATA && Value != DATA) {
            ERROR("Only code, xdata, or data segments allowed."); goto PURGE;
         }
         Type = Value;
         InSemi = 1, L = Scan();
         if (L == ORG) { InSemi = 0; goto DoOrg; }
         if (Active) EndSeg(), StartSeg(Type, 1, 0);
      goto SEM;
      case ORG:
         Type = SegP->Type;
      DoOrg:
         Scan(), InSemi = 1, E = Parse(0);
         if (Active) EndSeg(), StartSeg(Type, 0, VALUE(E));
      goto SEM;
      case INCLUDE: {
         char *S;
         L = Scan();
         if (L != STRING) FATAL("Missing filename in 'include'.");
         S = CopyS(Text);
         InSemi = 1, L = Scan();
         if (L != SEMI) FATAL("Missing ';' after include statement.");
         OpenF(S), free(S);
         InSemi = 0, L = Scan();
      }
      goto END_ST;
      case GLOBAL:
         Glo = 1;
         L = Scan();
         if (L != SYMBOL) {
            ERROR("Expected symbol after 'global'"); goto PURGE;
         }
      goto DEFINE;
      case SYMBOL:
         Glo = 0;
      DEFINE:
         Label = Sym; L = Scan();
         switch (L) {
            case COLON:
               SetColon(Glo, Label);
               InSemi = 1, L = Scan(), InSemi = 0;
            goto STATEMENT;
            case SET:
               Scan(), InSemi = 1, E = Parse(1), SetVar(Glo, Label, E);
            goto SEM;
            case TYPE: Type = Value, Addr = 1; goto DoEqu;
            case EQU: Type = 0, Addr = 0;
            DoEqu:
               Scan(), InSemi = 1, E = Parse(1);
               SetConst(Glo, Addr, Type, Label, E);
            goto SEM;
            default: ERROR("Undefined symbol: %s.", Label->Name); goto PURGE;
         }
      break;
      case EXTERN:
         L = Scan();
         if (L == EQU) Addr = 0;
         else if (L == TYPE) Addr = 1, Type = Value;
         else {
            ERROR("Expected type or 'equ' after 'extern'."); goto PURGE;
         }
         InSemi = 1;
         do {
            L = Scan();
            if (L != SYMBOL) {
               ERROR("Expected symbol in 'extern'"); goto PURGE;
            }
            if (Active) {
               if (Sym->Global) {
                  byte Addr1 = Sym->Address;
                  if (!Addr && Addr1)
                     ERROR("Redeclaring number %s as address.", Sym->Name);
                  else if (Addr && !Addr1)
                     ERROR("Redeclaring address %s as number.", Sym->Name);
                  else if (Addr && Addr1 && Type != Sym->Seg->Type)
                     ERROR("Type mismatch: %s.", Sym->Name);
               } else if (Sym->Defined)
                  ERROR("Redeclaring local symbol %s as external.", Sym->Name);
               else {
                  Sym->Global = 1;
                  if (Addr) Sym->Address = 1, Sym->Seg = SegTab + Type;
               }
            }
            Scan();
         } while (OldL == COMMA);
      goto SEM;
      case DB:
         InSemi = 1;
         do {
            L = Scan();
            if (L == STRING) PString(Text), Scan();
            else Reloc(0, 'b', Parse(2));
         } while (OldL == COMMA);
      goto SEM;
      case DW:
         InSemi = 1;
         do
            Scan(), Reloc(0, 'w', Parse(2));
         while (OldL == COMMA);
      goto SEM;
      case END:
         if (Active) { EndSeg(); return; }
         InSemi = 1, Scan();
      goto SEM;
      case MNEMONIC: InSemi = 1, ParseArgs((byte)Value); goto SEM;
      default: ERROR("Syntax error"); goto PURGE;
   }
PURGE:
   InSemi = 1;
   while (L != SEMI) {
      if (L == 0) { InSemi = 0; goto END_ST; }
      L = Scan();
   }
SEM:
   InSemi = 0; L = OldL;
   if (L != SEMI) ERROR("Missing ';'"); else L = Scan();
END_ST:
   switch (*--SP) {
      case BotS: goto START;
      case IfS:
         if (L == ELSE) {
            if (IP == IStack || IP[-1]) Active = !Active;
            *SP++ = ElseS, L = Scan(); goto STATEMENT;
         }
      case ElseS: EndIf:
         if (*--IP && !Active) {
            Active = 1;
            if (L == SYMBOL) Sym = LookUp(Text);
         }
      goto END_ST;
      case GroupS:
         if (L == RCURL) { L = Scan(); goto END_ST; }
         if (L == 0) { ERROR("Missing '}'."); goto END_ST; }
         *SP++ = GroupS;
      goto STATEMENT;
   }
}

static void Purge(void) {
   Symbol Sym, NextS; Exp E, NextE; char **F;
   for (Sym = NIL->Next[0]; Sym != NIL; Sym = NextS)
      NextS = Sym->Next[0], free(Sym->Name), free(Sym);
   for (F = FileTab; F < FileTab + Files; F++) free(*F);
   free(FileTab);
   free(NIL);
   for (E = ExpHead; E != 0; E = NextE) NextE = E->Next, free(E);
   free(RTab);
}

void Generate(void) {
   Symbol S; Segment Seg; Exp E; Item IP; word Value, File; Gap G;
   unsigned long SegLoc, Segs, Gaps, Syms, Exps, Relocs, Sum;
   Phase = 1;
   SegLoc = ftell(OutF), Segs = SegP - SegTab, Gaps = GapP - GapTab;
   for (IP = RTab; IP < RTab + RCur; IP++) Resolve(IP);
   fseek(OutF, SegLoc, SEEK_SET); /* Skip the code image */
   for (File = 0; File < Files; File++) {
      word L = strlen(FileTab[File]);
      PutW(L, OutF), fwrite(FileTab[File], 1, L, OutF);
   }
   for (Seg = SegTab + TYPES; Seg < SegP; Seg++) {
      word U = (Seg->Rel&1) << 8 | Seg->Type&0xff;
      PutW(Seg->Line, OutF), PutW(Seg->File, OutF),
      PutW(U, OutF), PutW(Seg->Size, OutF),
      PutW(Seg->Base, OutF), PutL(Seg->Loc, OutF);
   }
   for (G = GapTab; G < GapP; G++)
      PutW(G->Seg - SegTab, OutF), PutW(G->Offset, OutF), PutW(G->Size, OutF);
   for (Syms = 0, S = NIL->Next[0]; S != NIL; S = S->Next[0])
   if (S->Map || S->Global && S->Defined) {
      byte B = (S->Global&1) << 3  | (S->Defined&1) << 2 |
               (S->Address&1) << 1 | (S->Variable&1);
      word U = S->Seg - SegTab, L = strlen(S->Name);
      if (!S->Global && !S->Defined)
         ERROR("Undefined symbol: %s.", S->Name);
      S->Index = Syms++;
      PutB(B, OutF), PutW(U, OutF), PutW(S->Offset, OutF), PutW(L, OutF);
      if (L > 0) fwrite(S->Name, 1, L, OutF);
   }
   for (Exps = 0, E = ExpHead; E != 0; E = E->Next)
   if (E->Map) {
      byte Tag = E->Tag, Op; word U;
      E->Hash = Exps++;
      PutW(E->Line, OutF), PutW(E->File, OutF), PutB(Tag, OutF);
      switch (Tag) {
         case NumX: PutW(VALUE(E), OutF); break;
         case AddrX:
            PutW(SEG(E) - SegTab, OutF), PutW(OFFSET(E), OutF);
         break;
         case SymX: PutW(SYM(E)->Index, OutF); break;
         case UnX:
            Op = OP(E);
            PutB(Op, OutF), PutW(ARG1(E)->Hash, OutF);
         break;
         case BinX:
            Op = OP(E);
            PutB(Op, OutF), PutW(ARG1(E)->Hash, OutF),
            PutW(ARG2(E)->Hash, OutF);
         break;
         case CondX:
            PutW(ARG1(E)->Hash, OutF), PutW(ARG2(E)->Hash, OutF),
            PutW(ARG3(E)->Hash, OutF);
         break;
      }
   }
   for (Relocs = 0, IP = RTab; IP < RTab + RCur; IP++)
   if (IP->Map) {
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
   Sum = SegLoc + Files + Segs + Gaps + Syms + Exps + Relocs;
   PutL(Sum, OutF);
   fclose(OutF);
   Purge();
}
