// Frankenstein Cross-Assemblers, version 2.0.
// Original author: Mark Zenier.
#include "Constants.h"

#define high(A) (((A) >> 8)&0xff)
#define low(A) ((A)&0xff)

long EvalUnOp(char Op, long A) {
// Switch case actions for unary operators for both the parse and output phase expression evaluators.
   switch (Op) {
      case IFC_NEG: return -A;
      case IFC_NOT: return ~A;
      case IFC_HIGH: return high(A);
      case IFC_LOW: return low(A);
   }
// Anything else is treated as a no-op on A.
   return A;
}

long EvalBinOp(char Op, long A, long B) {
// Switch case actions for binary operators for both the parse and output phase expression evaluators.
   switch (Op) {
      case IFC_ADD: return A + B;
      case IFC_SUB: return A - B;
      case IFC_MUL: return A*B;
      case IFC_DIV: return A/B;
      case IFC_MOD: return A%B;
      case IFC_SHL: return A << B;
      case IFC_SHR: return A >> B;
      case IFC_OR: return A | B;
      case IFC_XOR: return A ^ B;
      case IFC_AND: return A&B;
      case IFC_GT: return A > B;
      case IFC_GE: return A >= B;
      case IFC_LT: return A < B;
      case IFC_LE: return A <= B;
      case IFC_NE: return A != B;
      case IFC_EQ: return A == B;
   }
// Anything else is treated as a no-op on B and a pop on A.
   return B; 
}
