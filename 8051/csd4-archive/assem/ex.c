#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "io.h"
#include "ex.h"
#include "st.h"
#include "res.h"

/* Derived from the syntax:
   Ex = primary | unary Ex | Ex binary Ex | Ex "?" Ex ":" Ex | "(" Ex ")".
   with the usual C-like precedence rules.
 */

Exp ExpHead;
static Exp ExpTail;
static Exp ExpHash[0x100];

void ExpInit(void) {
   int H;
   ExpTail = ExpHead = 0;
   for (H = 0; H < 0x100; H++) ExpHash[H] = 0;
}

static byte Direct = 2; /* 0 = Absolute, 1 = Relative, 2 = Undefined */

#define STACK_MAX 100

typedef enum { BOT, PAR, COND, ELS, BIN, UN } StackTag;

static StackTag TStack[STACK_MAX], *TP;
static Lexical OStack[STACK_MAX], *OP;
static struct Exp EStack[STACK_MAX], *EP;
static Exp AStack[STACK_MAX], *AP;

static void PUSH(StackTag Tag) {
   if (TP >= TStack + STACK_MAX)
      FATAL("Expression too complex ... aborting.");
   *TP++ = Tag;
}

static void PutE(Exp E) { if (Direct < 2) EP++; else *AP++ = E; }
static Exp GetE(void) { return (Direct < 2)? --EP: *--AP; }

void MarkExp(Exp E) {
   if (E->Map) return;
   E->Map = 1;
   switch (E->Tag) {
      case NumX: case AddrX: break;
      case SymX: SYM(E)->Map = 1; break;
      case CondX:
         MarkExp(ARG3(E));
      case BinX:
         MarkExp(ARG2(E));
      case UnX:
         MarkExp(ARG1(E));
      break;
   }
}

Exp EvalExp(Exp E) {
   ExpTag Tag = E->Tag;
   Symbol Sym; Lexical Op; Exp A, B, C, E1; byte H, Bs;
   Segment Seg; word Offset, Value, vB;
   if (Phase == 1) {
      if (E->Mark) return E;
      StartLine = E->Line, StartF = E->File;
   }
   switch (Tag) {
      case NumX: Value = VALUE(E); break;
      case AddrX: Seg = SEG(E), Offset = OFFSET(E); break;
      case SymX:
         Sym = SYM(E);
         if (!Sym->Defined) break;
         else if (Sym->Address)
            Tag = AddrX, Seg = Sym->Seg, Offset = Sym->Offset;
         else
            Tag = NumX, Value = Sym->Offset;
      break;
      case UnX:
         Op = OP(E), A = ARG1(E);
         if (Phase == 1) A = EvalExp(A);
         if (Op == PLUS) return A;
         switch (A->Tag) {
            case AddrX:
               ERROR("Address cannot be used with prefix operator.");
               Tag = NumX; Value = SEG(A)->Base + OFFSET(A);
            break;
            case NumX: Tag = NumX; Value = VALUE(A); break;
         }
         if (Tag == NumX) switch (Op) {
            case HIGH: Value = (Value >> 8)&0xff; break;
            case LOW: Value = Value&0xff; break;
            case NOT_NOT: Value = !Value; break;
            case NOT: Value = ~Value; break;
            case MINUS: Value = -Value; break;
         }
      break;
      case BinX:
         Op = OP(E), A = ARG1(E), B = ARG2(E);
         if (Phase == 1) A = EvalExp(A), B = EvalExp(B);
         if (Op == PLUS) {
            if (A->Tag == NumX && B->Tag == NumX) {
               Tag = NumX, Value = VALUE(A) + VALUE(B);
            } else if (A->Tag == NumX && B->Tag == AddrX) {
               Tag = AddrX, Seg = SEG(B), Offset = VALUE(A) + OFFSET(B);
            } else if (A->Tag == AddrX && B->Tag == NumX) {
               Tag = AddrX, Seg = SEG(A), Offset = OFFSET(A) + VALUE(B);
            } else if (A->Tag == AddrX && B->Tag == AddrX) {
               ERROR("Illegal combination: address + address");
               Tag = AddrX, Seg = SEG(A), Offset = OFFSET(A) + OFFSET(B);
            }
         } else if (Op == MINUS) {
            if (A->Tag == NumX && B->Tag == NumX) {
               Tag = NumX, Value = VALUE(A) - VALUE(B);
            } else if (A->Tag == NumX && B->Tag == AddrX) {
               ERROR("Illegal combination: number - address");
               Tag = NumX, Value = VALUE(A) - (SEG(B)->Base + OFFSET(B));
            } else if (A->Tag == AddrX && B->Tag == NumX) {
               Tag = AddrX, Seg = SEG(A), Offset = OFFSET(A) - VALUE(B);
            } else if (A->Tag == AddrX && B->Tag == AddrX) {
               if (SEG(A)->Type != SEG(B)->Type) {
                  ERROR("Addresses of different types cannot be subtracted.");
                  Tag = AddrX, Seg = SEG(A), Offset = OFFSET(A) - OFFSET(B);
               } else if (SEG(A) == SEG(B))
                  Tag = NumX, Value = OFFSET(A) - OFFSET(B);
            }
         } else if (Op == DOT) {
            Tag = AddrX, Seg = SegTab + BIT;
            switch (A->Tag) {
               case NumX:
                  Offset = VALUE(A);
               XX:
                  if ((Offset&0xfff0) == 0x20) Offset = (Offset - 0x20) << 3;
                  else if ((Offset&0xff87) != 0x80)
                     ERROR("Register in reg.bit not bit addressible."),
                     Offset = 0;
               break;
               case AddrX:
                  if (SEG(A)->Type != DATA && SEG(A)->Type != SFR)
                     ERROR("Address of wrong type in reg.pos"), Offset = 0;
                  else if (SEG(A)->Rel) {
                     Tag = BinX; break;
                  } else {
                     Offset = SEG(A)->Base + OFFSET(A);
                     if (SEG(A)->Type == DATA && Offset >= 0x80)
                        ERROR("Indirect registers are not bit addressible."),
                        Offset = 0;
                  }
               goto XX;
               default: Tag = BinX; break;
            }
            if (Tag == AddrX) switch (B->Tag) {
               case NumX:
                  vB = VALUE(B);
                  if (vB >= 8)
                     ERROR("Bit position out of range."), vB &= 7;
               break;
               case AddrX:
                  ERROR("Illegal combination: reg.bit"), vB = 0;
               break;
               default: Tag = BinX; break;
            }
            if (Tag == AddrX) Offset += vB;
         } else {
            Tag = NumX;
            if (A->Tag == AddrX || B->Tag == AddrX)
               ERROR("Address cannot be used with infix operator.");
            else if (A->Tag != NumX && B->Tag != NumX) Tag = BinX;
            if (Tag == NumX) {
               Value = A->Tag == NumX? VALUE(A): OFFSET(A),
               vB = B->Tag == NumX? VALUE(B): OFFSET(B);
               switch (Op) {
                  case BY: Value = (Value << 8) | (vB&0xff); break;
                  case OR_OR: if (vB) Value = 1; break;
                  case AND_AND: if (!vB) Value = 0; break;
                  case OR: Value |= vB; break;
                  case XOR: Value ^= vB; break;
                  case AND: Value &= vB; break;
                  case SHL: Value <<= vB; break;
                  case SHR: Value >>= vB; break;
                  case MULT: Value *= vB; break;
                  case EQ: Value = Value == vB; break;
                  case NE: Value = Value != vB; break;
                  case LE: Value = Value <= vB; break;
                  case LT: Value = Value < vB; break;
                  case GE: Value = Value >= vB; break;
                  case GT: Value = Value > vB; break;
                  case DIV:
                     if (vB == 0) ERROR("Division by 0."), Value = -1;
                     else Value /= vB;
                  break;
                  case MOD:
                     if (vB == 0) ERROR("Modulo by 0."), Value = -1;
                     else Value %= vB;
                  break;
               }
            }
         }
      break;
      case CondX:
         A = ARG1(E), B = ARG2(E), C = ARG3(E);
         if (Phase == 1) A = EvalExp(A), B = EvalExp(B), C = EvalExp(C);
         if (A->Tag == AddrX) {
            ERROR("Address cannot appear in: x? x: x"); return C;
         } else if (A->Tag != NumX) Tag = CondX;
         else return EvalExp((VALUE(A))? B: C);
      break;
   }
   if (Direct < 2) {
      if (Tag == AddrX) {
         if (Direct == 0) {
            if (Seg->Rel) ERROR("Relative address cannot be used here");
            Tag = NumX, Value = Seg->Base + Offset;
         }
      } else {
         if (Tag == SymX)
            ERROR("Undefined symbol: %s", Sym->Name), Tag = NumX, Value = 0;
         else if (Tag != NumX)
            ERROR("Undefined expression"), Tag = NumX, Value = 0;
      }
      if (Tag == NumX)
         EP->Tag = NumX, VALUE(EP) = Value;
      else
         EP->Tag = AddrX, SEG(EP) = Seg, OFFSET(EP) = Offset;
      return EP;
   }
   Bs = Seg - SegTab;
   if (Phase == 1) E1 = E;
   switch (Tag) {
      case NumX:
         H = (Value^(Value>>6)^(Value>>12))&0x3f;
         for (E = ExpHash[H]; E != 0; E = E->Tail)
            if (Value == VALUE(E)) return E;
      break;
      case AddrX:
         H = (Bs^(Bs>>6)^Offset^(Offset>>6)^(Offset>>12))&0x3f|0x40;
         for (E = ExpHash[H]; E != 0; E = E->Tail)
            if (Seg == SEG(E) && Offset == OFFSET(E)) return E;
      break;
      case SymX: {
         char *S;
         for (H = 0, S = Sym->Name; *S != '\0'; S++) H ^= *S;
         H = H&0x3f|0x80;
         for (E = ExpHash[H]; E != 0; E = E->Tail)
            if (Sym == SYM(E)) return E;
      }
      break;
      case UnX:
         H = (Op^(Op>>6)^A->Hash)&0xf|0xe0;
         for (E = ExpHash[H]; E != 0; E = E->Tail)
            if (Op == OP(E) && A == ARG1(E)) return E;
      break;
      case BinX:
         H = (Op^(Op>>6)^A->Hash^B->Hash)&0x1f|0xc0;
         for (E = ExpHash[H]; E != 0; E = E->Tail)
            if (Op == OP(E) && A == ARG1(E) && B == ARG2(E)) return E;
      break;
      case CondX:
         H = (A->Hash^B->Hash^C->Hash)&0xf|0xf0;
         for (E = ExpHash[H]; E != 0; E = E->Tail)
            if (A == ARG1(E) && B == ARG2(E) && C == ARG3(E)) return E;
      break;
   }
   if (Phase == 1) E = E1, E->Mark = 1;
   else {
      E = (Exp)Allocate(sizeof *E);
      E->Hash = H, E->Tail = ExpHash[H], ExpHash[H] = E;
      E->Next = 0, E->Map = 0,
      E->Line = StartLine, E->File = StartF,
      E->Mark = (Phase == 0)? 0: 1;
      if (ExpHead == 0) ExpHead = E; else ExpTail->Next = E;
      ExpTail = E;
   }
   switch (E->Tag = Tag) {
      case NumX: VALUE(E) = Value; break;
      case AddrX: SEG(E) = Seg, OFFSET(E) = Offset; break;
      case SymX: SYM(E) = Sym; break;
      case UnX: OP(E) = Op, ARG1(E) = A; break;
      case BinX: OP(E) = Op, ARG1(E) = A, ARG2(E) = B; break;
      case CondX: ARG1(E) = A, ARG2(E) = B, ARG3(E) = C; break;
   }
   return E;
}

static struct Exp EBuf;
Exp MakeExp(ExpTag Tag, ...) {
   va_list AP; Exp E = &EBuf;
   if (!Active) return 0;
   va_start(AP, Tag);
   switch (E->Tag = Tag) {
      case NumX: VALUE(E) = (word)va_arg(AP, unsigned); break;
      case AddrX:
         SEG(E) = va_arg(AP, Segment), OFFSET(E) = (word)va_arg(AP, unsigned);
      break;
      case SymX: SYM(E) = va_arg(AP, Symbol); break;
      case UnX:
         OP(E) = va_arg(AP, Lexical), ARG1(E) = va_arg(AP, Exp);
      break;
      case BinX:
         OP(E) = va_arg(AP, Lexical), ARG1(E) = va_arg(AP, Exp),
         ARG2(E) = va_arg(AP, Exp);
      break;
      case CondX:
         ARG1(E) = va_arg(AP, Exp), ARG2(E) = va_arg(AP, Exp),
         ARG3(E) = va_arg(AP, Exp);
      break;
   }
   va_end(AP);
   return EvalExp(E);
}

#define uO 0x10
/* Operator types: 20 = ), 40 = :, 60 = ?, 80 = binary */
#define bO 0x80
int OpTab[] = { /* Bits 0-3: precedence, Bit 4:unary, Bits 5-7: operator type */
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0x20, 0x40, 0x60,
   uO, uO, uO, uO, uO|bO|2, uO|bO|2, bO|1, bO|1, bO|1,
   bO|4, bO|4, bO|4, bO|4, bO|5, bO|5, bO|9, bO|10,
   bO|6, bO|7, bO|8, bO|3, bO|3, bO, bO,
   0, 0, 0, 0, 0
};

char *Action[6] = {
/*  .):?+ */
   "...?+", /* BOT:     -> Exp $                 */
   "B)B?+", /* PAR: Exp -> '(' Exp $ ')'         */
   "AA:?+", /* COND Exp -> Exp '?' Exp $ ':' Exp */
   "ccc?+", /* ELS: Exp -> Exp '?' Exp ':' Exp $ */
   "bbbbC", /* BIN: Exp -> Exp bin Exp $         */
   "uuuuu"  /* UN:  Exp -> un Exp $              */
};

Exp Parse(int Dir) {
   Lexical L = OldL; Symbol ID; Exp A, B; int Act;
   InExp = 1; Direct = Dir;
   EP = EStack, OP = OStack, AP = AStack, TP = TStack;
   PUSH(BOT);
EXP:
   switch (L) {
      case LPAR: PUSH(PAR); L = Scan(); goto EXP;
      case NUMBER: A = MakeExp(NumX, Value); L = Scan(); goto END_EX;
      case DOLLAR:
         A = MakeExp(AddrX, SegP, (word)LOC); L = Scan();
      goto END_EX;
      case SYMBOL: A = MakeExp(SymX, Sym); L = Scan(); goto END_EX;
      default:
         if ((OpTab[L]&uO)) { PUSH(UN); *OP++ = L; L = Scan(); goto EXP; }
         ERROR("Expected an argument/prefix operator.");
         A = MakeExp(NumX, 0);
      goto END_EX;
   }
END_EX:
   Act = Action[*--TP][OpTab[L] >> 5];
   if (Act == 'C')
      Act = ((OpTab[L]&0xe0) < (OpTab[OP[-1]]&0xe0))? '+': 'b';
   switch (Act) {
      case '+': TP++; PUSH(BIN); *OP++ = L; PutE(A); L = Scan(); goto EXP;
      case '?': TP++; PUSH(COND); PutE(A); L = Scan(); goto EXP;
      case ':': PUSH(ELS); PutE(A); L = Scan(); goto EXP;
      case 'A': ERROR("Missing ':'"); PUSH(ELS); PutE(A); goto EXP;
      case ')': L = Scan(); goto END_EX;
      case 'B': ERROR("Missing ')'"); goto END_EX;
      case 'u': A = MakeExp(UnX, *--OP, A); goto END_EX;
      case 'b': A = MakeExp(BinX, *--OP, GetE(), A); goto END_EX;
      case 'c': B = GetE(); A = MakeExp(CondX, GetE(), B, A); goto END_EX;
      case '.': InExp = 0, Direct = 2; return A;
   }
}
