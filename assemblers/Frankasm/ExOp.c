// Frankenstein Cross-Assemblers, version 2.0.
// Original author: Mark Zenier.
#define high(A) (((A) >> 8)&0xff)
#define low(A) ((A)&0xff)

long EvalUnOp(char Op, long A) {
// The switch case actions for unary operators for both the parse and output phase expression evaluators.
   switch (Op) {
      case '_': return -A;
      case 'N': return ~A;
      case 'H': return high(A);
      case 'Z': return low(A);
   }
// Anything else is treated as a no-op on A.
   return A;
}

long EvalBinOp(char Op, long A, long B) {
// The switch case actions for binary operators for both the parse and output phase expression evaluators.
   switch (Op) {
      case '+': return A + B;
      case '-': return A - B;
      case '*': return A*B;
      case '/': return A/B;
      case '%': return A%B;
      case '{': return A << B;
      case '}': return A >> B;
      case '|': return A | B;
      case '^': return A ^ B;
      case '&': return A&B;
      case '>': return A > B;
      case 'G': return A >= B;
      case '<': return A < B;
      case 'L': return A <= B;
      case '?': return A != B;
      case '=': return A == B;
   }
// Anything else is treated as a no-op on B and a pop on A.
   return B; 
}
