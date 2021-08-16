#include <stdio.h>
#include <ctype.h>
#include "Constants.h"
#include "Extern.h"

int loccnt; // location counter
static int value; // operand field value
bool zpref; // zero page reference flag

// Machine operations processor - 1 byte, no operand field.
void class1(void) {
   if (pass == LAST_PASS)
      loadlc(loccnt, 0, true), loadv(opval, 0, true), println();
   loccnt++;
}

// Collect a number operand.
static int colnum(int *ip) {
   int nval = 0;
   char ch = prlnbuf[*ip];
   int Base;
   if (ch == '$') Base = 0x10;
   else if (ch >= '1' && ch <= '9') Base = 10, nval = ch - '0';
   else if (ch == '@' || ch == '0') Base = 010;
   else if (ch == '%') Base = 2;
   while ((ch = prlnbuf[++*ip] - '0') >= 0) {
      if (ch > 9) {
         ch -= 'A' - '9' - 1;
         if (ch > 0xf) ch -= 'a' - 'A';
         if (ch > 0xf || ch < 10) break;
      }
      if (ch >= Base) break;
      nval = Base*nval + ch;
   }
   return nval;
}

// Evaluate an expression.
static bool evaluate(int *ip) {
   char op = '+';
   bool parflg = false;
   value = 0;
   zpref = undef = false;
   bool invalid = false;
   int value2;
   char op2;
// hcj: zpref should reflect the value of the expression, not the value of the intermediate symbols.
   char ch;
   while ((ch = prlnbuf[*ip]) != ' ' && ch != ')' && ch != ',' && ch != ';') {
      int tvalue = 0;
      if (ch == '$' || ch == '@' || ch == '%') tvalue = colnum(ip);
      else if (ch >= '0' && ch <= '9') tvalue = colnum(ip);
      else if (ch >= 'a' && ch <= 'z') tvalue = symval(ip);
      else if (ch >= 'A' && ch <= 'Z') tvalue = symval(ip);
      else if (ch == '_' || ch == '.') tvalue = symval(ip);
      else if (ch == '*') tvalue = loccnt, ++*ip;
      else if (ch == '\'') ++*ip, tvalue = prlnbuf[*ip]&0xff, ++*ip;
      else if (ch == '[') {
         if (parflg)
            error("Too many ['s in expression"), invalid = true;
         else
            value2 = value, op2 = op, value = tvalue = 0, op = '+', parflg = true;
         ++*ip;
         continue;
      } else if (ch == ']') {
         if (!parflg)
            error("No matching [ for ] in expression"), invalid = true;
         else
            parflg = false, tvalue = value, value = value2, op = op2;
         ++*ip;
      }
      switch (op) {
         case '+': value += tvalue; break;
         case '-': value -= tvalue; break;
         case '/': value = (unsigned)value/tvalue; break;
         case '*': value *= tvalue; break;
         case '%': value = (unsigned)value%tvalue; break;
         case '^': value ^= tvalue; break;
         case '~': value = ~tvalue; break;
         case '&': value &= tvalue; break;
         case '|': value |= tvalue; break;
         case '>':
            tvalue >>= 8;
         case '<':
            if (value != 0) error("High or low byte operator not first in operand field");
            value = tvalue&0xff, zpref = false;
         break;
         default: invalid = true;
      }
      op = prlnbuf[*ip];
      if (op == ' ' || op == ')' || op == ',' || op == ';') break;
      else if (op != ']') ++*ip;
   }
   if (parflg) {
      error("Missing ] in expression");
      return true;
   }
   if (value < 0 || value >= 256) zpref = true;
   if (undef) {
      if (pass != FIRST_PASS) error("Undefined symbol in operand field"), invalid = true;
      value = 0;
   } else if (invalid) error("Invalid operand field");
// This is the only way out that may not signal error
   else zpref = value < 0 || value >= 256;
   return invalid;
}

// Machine operations processor - 2 byte, relative addressing.
void class2(int *ip) {
   if (pass == LAST_PASS) {
      loadlc(loccnt, 0, true);
      loadv(opval, 0, true);
      while (prlnbuf[++*ip] == ' ');
      if (evaluate(ip)) {
         loccnt += 2;
         return;
      }
      loccnt += 2;
      if ((value -= loccnt) >= -128 && value < 128) loadv(value, 1, true), println();
      else error("Invalid branch address");
   } else loccnt += 2;
}

// Machine operations processor - various addressing modes.
void class3(int *ip) {
   char ch;
   while ((ch = prlnbuf[++*ip]) == ' ');
   int flag, ztmask;
   switch (ch) {
      case '\0': case ';': error("Operand field missing"); return;
      case 'A': case 'a':
         if ((ch = prlnbuf[*ip + 1]) == ' ' || ch == 0) {
            flag = ACC;
            break;
         }
      default:
         switch (ch = prlnbuf[*ip]) {
            case '#': case '=': flag = IMM1 | IMM2, ++*ip; break;
            case '(': flag = IND | INDX | INDY, ++*ip; break;
            default: flag = ABS | ZER | ZERX | ABSX | ABSY | ABSY2 | ZERY; break;
         }
         if (flag&(INDX | INDY | ZER | ZERX | ZERY)&opflg) udtype = UNDEFAB;
         if (evaluate(ip)) return;
         if (zpref) flag &= (ABS | ABSX | ABSY | ABSY2 | IND | IMM1 | IMM2), ztmask = 0;
         else ztmask = ZER | ZERX | ZERY;
         int code = 0;
         int i = 0;
         while ((ch = prlnbuf[(*ip)++]) != ' ' && ch != 0 && i++ < 4) {
            code *= 8;
            switch (ch) {
               case ')': // 4.
                  ++code;
               case ',': // 3.
                  ++code;
               case 'X': case 'x': // 2.
                  ++code;
               case 'Y': case 'y': // 1.
                  ++code;
               break;
               default: flag = 0; break;
            }
         }
         switch (code) {
         // No termination characters.
            case 0: flag &= (ABS | ZER | IMM1 | IMM2); break;
         // Termination = ).
            case 4: flag &= IND; break;
         // Termination = ,Y.
            case 25: flag &= (ABSY | ABSY2 | ZERY); break;
         // Termination = ,X.
            case 26: flag &= (ABSX | ZERX); break;
         // Termination = ,X).
            case 212: flag &= INDX; break;
         // Termination = ),Y.
            case 281: flag &= INDY; break;
            default: flag = 0; break;
         }
      break;
   }
   if ((opflg &= flag) == 0) {
      error("Invalid addressing mode");
      return;
   }
   if (opflg&ztmask) opflg &= ztmask;
   switch (opflg) {
   // Single byte - class 3.
      case ACC:
         if (pass == LAST_PASS)
            loadlc(loccnt, 0, true), loadv(opval + 8, 0, true), println();
         loccnt++;
      break;
   // double byte - class 3.
      case ZERX: case ZERY:
         opval += 4;
      case INDY:
         opval += 8;
      case IMM2:
         opval += 4;
      case ZER:
         opval += 4;
      case INDX: case IMM1:
         if (pass == LAST_PASS)
            loadlc(loccnt, 0, true), loadv(opval, 0, true), loadv(value, 1, true), println();
         loccnt += 2;
      break;
   // Triple byte - class 3.
      case IND:
         opval += 16;
      case ABSX: case ABSY2:
         opval += 4;
      case ABSY:
         opval += 12;
      case ABS:
         if (pass == LAST_PASS)
            opval += 12, loadlc(loccnt, 0, true), loadv(opval, 0, true), loadv(value, 1, true), loadv(value >> 8, 2, true), println();
         loccnt += 3;
      break;
      default: error("Invalid addressing mode"); break;
   }
}

// Pseudo-operations processor.
void pseudo(int *ip) {
   switch (opval) {
   // .byte pseudo.
      case 0: {
         labldef(loccnt), loadlc(loccnt, 0, true);
         while (prlnbuf[++*ip] == ' '); // field.
         int count = 0;
         do {
            if (prlnbuf[*ip] == '"') {
               for (int tvalue; (tvalue = prlnbuf[++*ip]) != '"'; ) {
                  if (tvalue == '\0') {
                     error("Unterminated ASCII string");
                     return;
                  } else if (tvalue == '\\') switch (tvalue = prlnbuf[++*ip]) {
                     case 'n': tvalue = '\n'; break;
                     case 't': tvalue = '\t'; break;
                  }
                  loccnt++;
                  if (pass == LAST_PASS) {
                     loadv(tvalue, count, true);
                     if (++count >= 3) {
                        println();
                        int i = 0;
                        for (; i < SFIELD; i++) prlnbuf[i] = ' ';
                        prlnbuf[i] = 0, count = 0, loadlc(loccnt, 0, true);
                     }
                  }
               }
               ++*ip;
            } else {
               if (evaluate(ip)) {
                  loccnt++;
                  return;
               }
               loccnt++;
               if (value > 0xff) {
                  error("Operand field size error");
                  return;
               } else if (pass == LAST_PASS) {
                  loadv(value, count, true);
                  if (++count >= 3) {
                     println();
                     int i = 0;
                     for (; i < SFIELD; i++) prlnbuf[i] = ' ';
                     prlnbuf[i] = 0, count = 0, loadlc(loccnt, 0, true);
                  }
               }
            }
         } while (prlnbuf[(*ip)++] == ',');
         if (pass == LAST_PASS && count != 0) println();
      }
      break;
   // = pseudo.
      case 1:
         while (prlnbuf[++*ip] == ' ');
         if (evaluate(ip)) break;
         labldef(value);
         if (pass == LAST_PASS) loadlc(value, 1, false), println();
      break;
   // .word pseudo.
      case 2:
         labldef(loccnt), loadlc(loccnt, 0, true);
         while (prlnbuf[++*ip] == ' ');
         do {
            if (evaluate(ip)) {
               loccnt += 2;
               break;
            }
            loccnt += 2;
            if (pass == LAST_PASS) {
               loadv(value, 0, true), loadv(value >> 8, 1, true), println();
               int i = 0;
               for (; i < SFIELD; i++) prlnbuf[i] = ' ';
               prlnbuf[i] = 0, loadlc(loccnt, 0, true);
            }
         } while (prlnbuf[(*ip)++] == ',');
      break;
   // *= pseudo.
      case 3: {
         while (prlnbuf[++*ip] == ' ');
         int tvalue;
         if (prlnbuf[*ip] == '*') {
            if (evaluate(ip)) break;
            else if (undef) {
               error("Undefined symbol in operand field.");
               return;
            }
            tvalue = loccnt;
         } else {
            if (evaluate(ip)) break;
            else if (undef) {
               error("Undefined symbol in operand field.");
               return;
            }
            tvalue = value;
         }
         loccnt = value;
         labldef(tvalue);
         if (pass == LAST_PASS) objcnt = 0, loadlc(tvalue, 1, false), println();
      }
      break;
   // .list pseudo.
      case 4:
         if (lflag >= 0) lflag = 1;
      break;
   // .nlst pseudo.
      case 5:
         if (lflag >= 0) lflag = iflag;
      break;
   // .dbyt pseudo.
      case 6:
         labldef(loccnt), loadlc(loccnt, 0, true);
         while (prlnbuf[++*ip] == ' ');
         do {
            if (evaluate(ip)) {
               loccnt += 2;
               break;
            }
            loccnt += 2;
            if (pass == LAST_PASS) {
               loadv(value >> 8, 0, true), loadv(value, 1, true), println();
               int i = 0;
               for (; i < SFIELD; i++) prlnbuf[i] = ' ';
               prlnbuf[i] = 0, loadlc(loccnt, 0, true);
            }
         } while (prlnbuf[(*ip)++] == ',');
      break;
   // .page pseudo.
      case 7:
         if (pagesize == 0) break;
         while (prlnbuf[++*ip] == ' ');
         if (prlnbuf[(*ip)] == '"') {
            int i = 0;
            for (int tvalue; (tvalue = prlnbuf[++*ip]) != '"'; ) {
               if (tvalue == '\0') {
                  error("Unterminated ASCII string");
                  return;
               }
               titlbuf[i++] = tvalue;
               if (i == titlesize) {
                  error("Title too long");
                  return;
               }
            }
            if (i < titlesize) for (int j = i; j < titlesize; j++) titlbuf[j] = ' ';
         }
         if (lflag > 0) printhead();
      break;
   }
}
