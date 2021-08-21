#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "ex.h"
#include "st.h"
#include "res.h"
#include "op.h"

#define ELEMENTS(Arr) (sizeof (Arr)/sizeof (Arr)[0])

/* Translate controls:
   X: . = x     # = #x      / = /x      n = Rn
      A = A     B = AB      C = C       D = DPTR
      i = @Ri   @ = @DPTR   > = @A+DPTR $ = @A+PC
   Y: (x)  L = Long Code Address
           P = Paged Code Address
           R = Relative Code Address
           D = Direct Register Address
      (x/) B = Bit Address
      (@)  w = Word
           b = Byte
      (i)  i = Register Pointer R0/R1
      (n)  n = Register R0/R1/R2/R3/R4/R5/R6/R7
           x = Reverse Order of Next 2 Operands.
  */

Mode ModeTab[] = {
/* acall */ { 0x11, ".",   "P" },
/* add */   { 0x24, "A#",  "b" },  { 0x26, "Ai",  "i" }, { 0x25, "A.",  "D" },
            { 0x28, "An",  "n" },
/* addc */  { 0x34, "A#",  "b" },  { 0x36, "Ai",  "i" }, { 0x35, "A.",  "D" },
            { 0x38, "An",  "n" },
/* ajmp */  { 0x01, ".",   "P" },
/* anl */   { 0x54, "A#",  "b" },  { 0x56, "Ai",  "i" }, { 0x55, "A.",  "D" },
            { 0x58, "An",  "n" },  { 0xb0, "C/",  "B" }, { 0x82, "C.",  "B" },
            { 0x53, ".#",  "Db" }, { 0x52, ".A",  "D" },
/* cjne */  { 0xb6, "i#.", "ibR" },{ 0xb4, "A#.", "bR" },{ 0xb5, "A..", "DR" },
            { 0xb8, "n#.", "nbR" },
/* clr */   { 0xe4, "A",   "" },   { 0xc2, ".",   "B" }, { 0xc3, "C",   "" },
/* cpl */   { 0xf4, "A",   "" },   { 0xb2, ".",   "B" }, { 0xb3, "C",   "" },
/* da */    { 0xd4, "A",   "" },
/* dec */   { 0x16, "i",   "i" },  { 0x14, "A",   ""},   { 0x15, ".",   "D" },
            { 0x18, "n",   "n" },
/* div */   { 0x84, "B",   "" },
/* djnz */  { 0xd5, "..",  "DR" }, { 0xd8, "n.",  "nR" },
/* inc */   { 0x06, "i",   "i" },  { 0x04, "A",   "" },  { 0x05, ".",   "D" },
            { 0xa3, "D",   "" },   { 0x08, "n",   "n" },
/* jb */    { 0x20, "..",  "BR" },
/* jbc */   { 0x10, "..",  "BR" },
/* jc */    { 0x40, ".",   "R" },
/* jmp */   { 0x73, ">",   "" },
/* jnb */   { 0x30, "..",  "BR" },
/* jnc */   { 0x50, ".",   "R" },
/* jnz */   { 0x70, ".",   "R" },
/* jz */    { 0x60, ".",   "R" },
/* lcall */ { 0x12, ".",   "L" },
/* ljmp */  { 0x02, ".",   "L" },
/* mov */   { 0x76, "i#",  "ib" }, { 0xf6, "iA",  "i" }, { 0xa6, "i.",  "iD" },
            { 0x74, "A#",  "b" },  { 0xe6, "Ai",  "i" }, { 0xe5, "A.",  "D" },
            { 0xe8, "An",  "n" },  { 0x92, ".C",  "B" }, { 0xa2, "C.",  "B" },
            { 0x75, ".#",  "Db" }, { 0x86, ".i",  "iD" },{ 0xf5, ".A",  "D" },
            { 0x85, "..",  "xDD" },{ 0x88, ".n",  "nD" },{ 0x90, "D#",  "w" },
            { 0x78, "n#",  "nb" }, { 0xf8, "nA",  "n" }, { 0xa8, "n.",  "nD" },
/* movc */  { 0x93, "A>",  "" },   { 0x83, "A$",  "" },
/* movx */  { 0xf0, "@A",  "" },   { 0xf2, "iA",  "i" }, { 0xe0, "A@",  "" },
            { 0xe2, "Ai",  "i" },
/* mul */   { 0xa4, "B",   "" },
/* nop */   { 0x00, "",    "" },
/* orl */   { 0x44, "A#",  "b" },  { 0x46, "Ai",  "i" }, { 0x45, "A.",  "D" },
            { 0x48, "An",  "n" },  { 0xa0, "C/",  "B" }, { 0x72, "C.",  "B" },
            { 0x43, ".#",  "Db" }, { 0x42, ".A",  "D" },
/* pop */   { 0xd0, ".",   "D" },
/* push */  { 0xc0, ".",   "D" },
/* ret */   { 0x22, "",    "" },
/* reti */  { 0x32, "",    "" },
/* rl */    { 0x23, "A",   "" },
/* rlc */   { 0x33, "A",   "" },
/* rr */    { 0x03, "A",   "" },
/* rrc */   { 0x13, "A",   "" },
/* setb */  { 0xd2, ".",   "B" },  { 0xd3, "C",   "" },
/* sjmp */  { 0x80, ".",   "R" },
/* subb */  { 0x94, "A#",  "b" },  { 0x96, "Ai",  "i" }, { 0x95, "A.",  "D" },
            { 0x98, "An",  "n" },
/* swap */  { 0xc4, "A",   "" },
/* xch */   { 0xc6, "Ai",  "i" },  { 0xc5, "A.",  "D" }, { 0xc8, "An",  "n" },
/* xchd */  { 0xd6, "Ai",  "i" },
/* xrl */   { 0x64, "A#",  "b" },  { 0x66, "Ai",  "i" }, { 0x65, "A.",  "D" },
            { 0x68, "An",  "n" },  { 0x63, ".#",  "Db" },{ 0x62, ".A",  "D" }
};

Code CodeTab[] = {
   { "acall", 1 }, { "add",   4 }, { "addc",  4 }, { "ajmp",  1 },
   { "anl",   8 }, { "cjne",  4 }, { "clr",   3 }, { "cpl",   3 },
   { "da",    1 }, { "dec",   4 }, { "div",   1 }, { "djnz",  2 },
   { "inc",   5 }, { "jb",    1 }, { "jbc",   1 }, { "jc",    1 },
   { "jmp",   1 }, { "jnb",   1 }, { "jnc",   1 }, { "jnz",   1 },
   { "jz",    1 }, { "lcall", 1 }, { "ljmp",  1 }, { "mov",  18 },
   { "movc",  2 }, { "movx",  4 }, { "mul",   1 }, { "nop",   1 },
   { "orl",   8 }, { "pop",   1 }, { "push",  1 }, { "ret",   1 },
   { "reti",  1 }, { "rl",    1 }, { "rlc",   1 }, { "rr",    1 },
   { "rrc",   1 }, { "setb",  2 }, { "sjmp",  1 }, { "subb",  4 },
   { "swap",  1 }, { "xch",   3 }, { "xchd",  1 }, { "xrl",   6 },
   { 0, 0 }
};

/* Argument buffer. */
#define A_MAX 4
char XBuf[A_MAX + 1]; Exp XExp[A_MAX];

void OpInit(void) {
   Mode *M = ModeTab;
   for (Code *OC = CodeTab; OC < CodeTab + ELEMENTS(CodeTab); OC++) {
      OC->Start = M; M += OC->Modes;
      if (M > ModeTab + ELEMENTS(ModeTab))
         fprintf(stderr, "Bad opcode initialization.\n"), exit(1);
   }
}

void ParseArgs(byte Mnem) {
   char *XP = XBuf; Exp *EP = XExp;
   Lexical L = OldL; int XReg;
   do {
      if ((L = Scan()) == SEMI) {
         if (XP > XBuf) Error("Extra ','."); break;
      }
      switch (L) {
         case POUND: *XP = '#', Scan(), *EP++ = Parse(2); break;
         case DIV: *XP = '/', Scan(), *EP++ = Parse(2); break;
         case AT: switch (L = Scan()) {
            case REGISTER: switch ((Register)Value) {
               case ACC:
                  if (Scan() != PLUS)
                     Error("Cannot use @A."), *XP = ' ';
                  else if (Scan() != REGISTER)
                     Error("Missing DPTR or PC after @A+"), *XP = ' ';
                  else {
                     switch (Value) {
                        case ACC: Error("Cannot use @A+A"), *XP = ' '; break;
                        case AB: Error("Cannot use @A+AB."); *XP = ' '; break;
                        case CY: Error("Cannot use @A+C."); *XP = ' '; break;
                        case DPTR: *XP = '>'; break;
                        case PC: *XP = '$'; break;
                        default: Error("Cannot use @A+Rn"); *XP = ' '; break;
                     }
                     Scan();
                  }
               break;
               case AB: Error("Cannot use @AB."); *XP = ' '; break;
               case CY: Error("Cannot use @C."); *XP = ' '; break;
               case DPTR: *XP = '@'; Scan(); break;
               case PC: Error("Cannot use @PC."); *XP = ' '; break;
               default:
                  Value -= (short)R0;
                  if (Value != 0 && Value != 1) {
                     Error("Register in @Rn out of range."); *XP = ' ';
                  } else *XP = 'i', XReg = Value;
                  Scan();
               break;
            }
            break;
            default:
               Error("Register must appear after @."); *XP = ' ';
            break;
         }
         break;
         case REGISTER: switch ((Register)Value) {
            case ACC: *XP = 'A'; break;
            case AB: *XP = 'B'; break;
            case CY: *XP = 'C'; break;
            case DPTR: *XP = 'D'; break;
            case PC:
               Error("Cannot use PC as a register.");
               *XP = '.', *EP++ = MakeExp(AddrX, SegP, (word)CurLoc);
            break;
            default: *XP = 'n', XReg = Value - (short)R0; break;
         }
            Scan();
         break;
         default: *XP = '.', *EP++ = Parse(2); break;
      }
      L = OldL;
      if (XP++ >= XBuf + A_MAX) {
         Error("Too many arguments specified.");
         while (L != SEMI && L != 0) L = Scan();
         break;
      }
   } while (L == COMMA);
   if (!Active) return;
   *XP = 0, *EP = NULL;
   Code *CP = CodeTab + Mnem;
   Mode *MP = CP->Start;
   for (; MP < CP->Start + CP->Modes; MP++)
      if (strcmp(XBuf, MP->X) == 0) break;
   if (MP >= CP->Start + CP->Modes) {
      Error("Invalid addressing mode: %s.", CP->Name); return;
   }
   EP = XExp;
   byte Op = MP->OpCode;
   char *S = MP->Y;
   switch (*S) {
      case 'n': S++; Op |= XReg&7; break;/* Register R0/R1/R2/R3/R4/R5/R6/R7 */
      case 'i': S++; Op |= XReg&1; break;/* Register Pointer @R0/@R1 */
      case 'x': { Exp E = EP[0]; EP[0] = EP[1], EP[1] = E; S++; } break;
      case 'P': Reloc(Op, 'P', *EP++); return; /* P Paged Address */
   }
   PByte(Op);
   for (; *S != '\0'; S++) Reloc(0, *S, *EP++);
}
