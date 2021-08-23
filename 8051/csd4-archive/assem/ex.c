#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include "io.h"
#include "ex.h"
#include "st.h"
#include "res.h"

// Derived from the syntax:
// Ex = primary | unary Ex | Ex binary Ex | Ex "?" Ex ":" Ex | "(" Ex ")",
// with the usual C-like precedence rules.

Exp ExpHead;
static Exp ExpTail, ExpHash[0x100];

void ExpInit(void) {
   ExpTail = ExpHead = NULL;
   for (int H = 0; H < 0x100; H++) ExpHash[H] = NULL;
}

static byte Direct = 2; // 0 = Absolute, 1 = Relative, 2 = Undefined.

#define STACK_MAX 100

typedef enum { BOT, PAR, COND, ELS, BIN, UN } StackTag;

static StackTag TStack[STACK_MAX], *TP;
static Lexical OStack[STACK_MAX], *OP;
static struct Exp EStack[STACK_MAX], *EP;
static Exp AStack[STACK_MAX], *AP;

static void Push(StackTag Tag) {
   if (TP >= TStack + STACK_MAX)
      Fatal("Expression too complex ... aborting.");
   *TP++ = Tag;
}

static void PutE(Exp E) { if (Direct < 2) EP++; else *AP++ = E; }
static Exp GetE(void) { return (Direct < 2)? --EP: *--AP; }

void MarkExp(Exp E) {
   if (E->Map) return; else E->Map = true;
   switch (E->Tag) {
      case NumX: case AddrX: break;
      case SymX: SymOf(E)->Map = true; break;
      case CondX:
         MarkExp(ArgC(E));
      case BinX:
         MarkExp(ArgB(E));
      case UnX:
         MarkExp(ArgA(E));
      break;
   }
}

Exp EvalExp(Exp E) {
   if (Phase == 1) {
      if (E->Mark) return E;
      StartLine = E->Line, StartF = E->File;
   }
   Segment Seg; word Offset, Value;
   Symbol Sym; Lexical Op; Exp A, B, C;
   ExpTag Tag = E->Tag;
   switch (Tag) {
      case NumX: Value = ValOf(E); break;
      case AddrX: Seg = SegOf(E), Offset = OffOf(E); break;
      case SymX:
         Sym = SymOf(E);
         if (!Sym->Defined) break;
         else if (Sym->Address)
            Tag = AddrX, Seg = Sym->Seg, Offset = Sym->Offset;
         else
            Tag = NumX, Value = Sym->Offset;
      break;
      case UnX:
         Op = OpOf(E), A = ArgA(E);
         if (Phase == 1) A = EvalExp(A);
         if (Op == PLUS) return A;
         switch (A->Tag) {
            case AddrX:
               Error("Address cannot be used with prefix operator.");
               Tag = NumX; Value = SegOf(A)->Base + OffOf(A);
            break;
            case NumX: Tag = NumX; Value = ValOf(A); break;
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
         Op = OpOf(E), A = ArgA(E), B = ArgB(E);
         if (Phase == 1) A = EvalExp(A), B = EvalExp(B);
         if (Op == PLUS) {
            if (A->Tag == NumX && B->Tag == NumX)
               Tag = NumX, Value = ValOf(A) + ValOf(B);
            else if (A->Tag == NumX && B->Tag == AddrX)
               Tag = AddrX, Seg = SegOf(B), Offset = ValOf(A) + OffOf(B);
            else if (A->Tag == AddrX && B->Tag == NumX)
               Tag = AddrX, Seg = SegOf(A), Offset = OffOf(A) + ValOf(B);
            else if (A->Tag == AddrX && B->Tag == AddrX)
               Error("Illegal combination: address + address"),
               Tag = AddrX, Seg = SegOf(A), Offset = OffOf(A) + OffOf(B);
         } else if (Op == MINUS) {
            if (A->Tag == NumX && B->Tag == NumX)
               Tag = NumX, Value = ValOf(A) - ValOf(B);
            else if (A->Tag == NumX && B->Tag == AddrX)
               Error("Illegal combination: number - address"),
               Tag = NumX, Value = ValOf(A) - (SegOf(B)->Base + OffOf(B));
            else if (A->Tag == AddrX && B->Tag == NumX)
               Tag = AddrX, Seg = SegOf(A), Offset = OffOf(A) - ValOf(B);
            else if (A->Tag == AddrX && B->Tag == AddrX) {
               if (SegOf(A)->Type != SegOf(B)->Type)
                  Error("Addresses of different types cannot be subtracted."),
                  Tag = AddrX, Seg = SegOf(A), Offset = OffOf(A) - OffOf(B);
               else if (SegOf(A) == SegOf(B))
                  Tag = NumX, Value = OffOf(A) - OffOf(B);
            }
         } else if (Op == DOT) {
            Tag = AddrX, Seg = SegTab + BIT;
            switch (A->Tag) {
               case NumX:
                  Offset = ValOf(A);
               XX:
                  if ((Offset&0xfff0) == 0x20) Offset = (Offset - 0x20) << 3;
                  else if ((Offset&0xff87) != 0x80)
                     Error("Register in reg.bit not bit addressible."),
                     Offset = 0;
               break;
               case AddrX:
                  if (SegOf(A)->Type != DATA && SegOf(A)->Type != SFR)
                     Error("Address of wrong type in reg.pos"), Offset = 0;
                  else if (SegOf(A)->Rel) {
                     Tag = BinX; break;
                  } else {
                     Offset = SegOf(A)->Base + OffOf(A);
                     if (SegOf(A)->Type == DATA && Offset >= 0x80)
                        Error("Indirect registers are not bit addressible."),
                        Offset = 0;
                  }
               goto XX;
               default: Tag = BinX; break;
            }
            if (Tag == AddrX) switch (B->Tag) {
               case NumX: {
                  word ValB = ValOf(B);
                  if (ValB >= 8)
                     Error("Bit position out of range."), ValB &= 7;
                  Offset += ValB;
               }
               break;
               case AddrX:
                  Error("Illegal combination: reg.bit");
               break;
               default: Tag = BinX; break;
            }
         } else {
            Tag = NumX;
            if (A->Tag == AddrX || B->Tag == AddrX)
               Error("Address cannot be used with infix operator.");
            else if (A->Tag != NumX && B->Tag != NumX) Tag = BinX;
            if (Tag == NumX) {
               Value = A->Tag == NumX? ValOf(A): OffOf(A);
               word ValB = B->Tag == NumX? ValOf(B): OffOf(B);
               switch (Op) {
                  case BY: Value = (Value << 8) | (ValB&0xff); break;
                  case OR_OR: if (ValB) Value = 1; break;
                  case AND_AND: if (!ValB) Value = 0; break;
                  case OR: Value |= ValB; break;
                  case XOR: Value ^= ValB; break;
                  case AND: Value &= ValB; break;
                  case SHL: Value <<= ValB; break;
                  case SHR: Value >>= ValB; break;
                  case MULT: Value *= ValB; break;
                  case EQ: Value = Value == ValB; break;
                  case NE: Value = Value != ValB; break;
                  case LE: Value = Value <= ValB; break;
                  case LT: Value = Value < ValB; break;
                  case GE: Value = Value >= ValB; break;
                  case GT: Value = Value > ValB; break;
                  case DIV:
                     if (ValB == 0) Error("Division by 0."), Value = -1;
                     else Value /= ValB;
                  break;
                  case MOD:
                     if (ValB == 0) Error("Modulo by 0."), Value = -1;
                     else Value %= ValB;
                  break;
               }
            }
         }
      break;
      case CondX:
         A = ArgA(E), B = ArgB(E), C = ArgC(E);
         if (Phase == 1) A = EvalExp(A), B = EvalExp(B), C = EvalExp(C);
         if (A->Tag == AddrX) {
            Error("Address cannot appear in: x? x: x"); return C;
         } else if (A->Tag != NumX) Tag = CondX;
         else return EvalExp((ValOf(A))? B: C);
      break;
   }
   if (Direct < 2) {
      if (Tag == AddrX) {
         if (Direct == 0) {
            if (Seg->Rel) Error("Relative address cannot be used here");
            Tag = NumX, Value = Seg->Base + Offset;
         }
      } else {
         if (Tag == SymX)
            Error("Undefined symbol: %s", Sym->Name), Tag = NumX, Value = 0;
         else if (Tag != NumX)
            Error("Undefined expression"), Tag = NumX, Value = 0;
      }
      if (Tag == NumX)
         EP->Tag = NumX, ValOf(EP) = Value;
      else
         EP->Tag = AddrX, SegOf(EP) = Seg, OffOf(EP) = Offset;
      return EP;
   }
   byte Bs = Seg - SegTab;
   Exp E1;
   if (Phase == 1) E1 = E;
   byte H;
   switch (Tag) {
      case NumX:
         H = (Value^(Value>>6)^(Value>>12))&0x3f;
         for (E = ExpHash[H]; E != NULL; E = E->Tail)
            if (Value == ValOf(E)) return E;
      break;
      case AddrX:
         H = (Bs^(Bs>>6)^Offset^(Offset>>6)^(Offset>>12))&0x3f|0x40;
         for (E = ExpHash[H]; E != NULL; E = E->Tail)
            if (Seg == SegOf(E) && Offset == OffOf(E)) return E;
      break;
      case SymX: {
         char *S;
         for (H = 0, S = Sym->Name; *S != '\0'; S++) H ^= *S;
         H = H&0x3f|0x80;
         for (E = ExpHash[H]; E != NULL; E = E->Tail)
            if (Sym == SymOf(E)) return E;
      }
      break;
      case UnX:
         H = (Op^(Op>>6)^A->Hash)&0xf|0xe0;
         for (E = ExpHash[H]; E != NULL; E = E->Tail)
            if (Op == OpOf(E) && A == ArgA(E)) return E;
      break;
      case BinX:
         H = (Op^(Op>>6)^A->Hash^B->Hash)&0x1f|0xc0;
         for (E = ExpHash[H]; E != NULL; E = E->Tail)
            if (Op == OpOf(E) && A == ArgA(E) && B == ArgB(E)) return E;
      break;
      case CondX:
         H = (A->Hash^B->Hash^C->Hash)&0xf|0xf0;
         for (E = ExpHash[H]; E != NULL; E = E->Tail)
            if (A == ArgA(E) && B == ArgB(E) && C == ArgC(E)) return E;
      break;
   }
   if (Phase == 1) E = E1, E->Mark = true;
   else {
      E = Allocate(sizeof *E);
      E->Hash = H, E->Tail = ExpHash[H], ExpHash[H] = E;
      E->Next = NULL, E->Map = false,
      E->Line = StartLine, E->File = StartF,
      E->Mark = Phase != 0;
      if (ExpHead == 0) ExpHead = E; else ExpTail->Next = E;
      ExpTail = E;
   }
   switch (E->Tag = Tag) {
      case NumX: ValOf(E) = Value; break;
      case AddrX: SegOf(E) = Seg, OffOf(E) = Offset; break;
      case SymX: SymOf(E) = Sym; break;
      case UnX: OpOf(E) = Op, ArgA(E) = A; break;
      case BinX: OpOf(E) = Op, ArgA(E) = A, ArgB(E) = B; break;
      case CondX: ArgA(E) = A, ArgB(E) = B, ArgC(E) = C; break;
   }
   return E;
}

static struct Exp EBuf;
Exp MakeExp(ExpTag Tag, ...) {
   Exp E = &EBuf;
   if (!Active) return NULL;
   va_list AP; va_start(AP, Tag);
   switch (E->Tag = Tag) {
      case NumX: ValOf(E) = (word)va_arg(AP, unsigned); break;
      case AddrX:
         SegOf(E) = va_arg(AP, Segment), OffOf(E) = (word)va_arg(AP, unsigned);
      break;
      case SymX: SymOf(E) = va_arg(AP, Symbol); break;
      case UnX:
         OpOf(E) = va_arg(AP, Lexical), ArgA(E) = va_arg(AP, Exp);
      break;
      case BinX:
         OpOf(E) = va_arg(AP, Lexical), ArgA(E) = va_arg(AP, Exp),
         ArgB(E) = va_arg(AP, Exp);
      break;
      case CondX:
         ArgA(E) = va_arg(AP, Exp), ArgB(E) = va_arg(AP, Exp),
         ArgC(E) = va_arg(AP, Exp);
      break;
   }
   va_end(AP);
   return EvalExp(E);
}

#define uO 0x10
// Operator types: 20 = ), 40 = :, 60 = ?, 80 = binary.
#define bO 0x80
int OpTab[] = { // Bits 0-3: precedence, Bit 4: unary, Bits 5-7: operator type.
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0x20, 0x40, 0x60,
   uO, uO, uO, uO, uO|bO|2, uO|bO|2, bO|1, bO|1, bO|1,
   bO|4, bO|4, bO|4, bO|4, bO|5, bO|5, bO|9, bO|10,
   bO|6, bO|7, bO|8, bO|3, bO|3, bO, bO,
   0, 0, 0, 0, 0
};

char *Action[6] = {
//  .):?+
   "...?+", // BOT:     → Exp ∙
   "B)B?+", // PAR: Exp → '(' Exp ∙ ')'
   "AA:?+", // COND Exp → Exp '?' Exp ∙ ':' Exp
   "ccc?+", // ELS: Exp → Exp '?' Exp ':' Exp ∙
   "bbbbC", // BIN: Exp → Exp bin Exp ∙
   "uuuuu"  // UN:  Exp → un Exp ∙
};

Exp Parse(int Dir) {
   Lexical L = OldL;
   InExp = true; Direct = Dir;
   EP = EStack, OP = OStack, AP = AStack, TP = TStack;
   Push(BOT);
   Exp A, B; int Act;
BegEx:
   switch (L) {
      case LPAR: Push(PAR); L = Scan(); goto BegEx;
      case NUMBER: A = MakeExp(NumX, Value); L = Scan(); goto EndEx;
      case DOLLAR:
         A = MakeExp(AddrX, SegP, (word)CurLoc); L = Scan();
      goto EndEx;
      case SYMBOL: A = MakeExp(SymX, Sym); L = Scan(); goto EndEx;
      default:
         if ((OpTab[L]&uO)) { Push(UN); *OP++ = L; L = Scan(); goto BegEx; }
         Error("Expected an argument/prefix operator.");
         A = MakeExp(NumX, 0);
      goto EndEx;
   }
EndEx:
   Act = Action[*--TP][OpTab[L] >> 5];
   if (Act == 'C')
      Act = ((OpTab[L]&0xe0) < (OpTab[OP[-1]]&0xe0))? '+': 'b';
   switch (Act) {
      case '+': TP++; Push(BIN); *OP++ = L; PutE(A); L = Scan(); goto BegEx;
      case '?': TP++; Push(COND); PutE(A); L = Scan(); goto BegEx;
      case ':': Push(ELS); PutE(A); L = Scan(); goto BegEx;
      case 'A': Error("Missing ':'"); Push(ELS); PutE(A); goto BegEx;
      case ')': L = Scan(); goto EndEx;
      case 'B': Error("Missing ')'"); goto EndEx;
      case 'u': A = MakeExp(UnX, *--OP, A); goto EndEx;
      case 'b': A = MakeExp(BinX, *--OP, GetE(), A); goto EndEx;
      case 'c': B = GetE(); A = MakeExp(CondX, GetE(), B, A); goto EndEx;
      default:
      case '.': InExp = false, Direct = 2; break;
   }
   return A;
}
