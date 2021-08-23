#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef unsigned char byte;
typedef unsigned int word;

byte Arg[3];
word PC;
bool Generating;
byte Ref[0x4000], Hex[0x4000];
#define ENTRY 0x80
#define OP    0x40
#define ARG   0x20
#define LINKS 0x1f

word Paged(byte P) {
   return PC&0xf800 | (Arg[0]&0xe0) << 3 | P;
}

word Relative(byte R) {
   return PC + (signed char)R;
}

word Entries[0x100], *EP = Entries;

void PushAddr(word Address) {
   byte B = Ref[Address]&LINKS;
   if (Ref[Address]&ARG) {
      fprintf(stderr, "Entry into ARG at %04x.\n", PC); return;
   }
   if (++B <= LINKS) Ref[Address] = Ref[Address]&~LINKS | B;
   if (Ref[Address]&ENTRY) return;
   Ref[Address] |= ENTRY;
   if (Ref[Address]&OP) return;
   if (EP - Entries == 0x100) {
      fprintf(stderr, "Too many entries, PC = %04x.\n", PC); exit(EXIT_FAILURE);
   }
   *EP++ = Address;
}

// (Indirect? 0x10: 0) | (Breaking? 8: 0) | (Addressing? 4: 0) | OpBytes
// Indirect is true only for opcode 0x73: it is the only indirectly-addressed jump or call in the MCS-51.
// Breaking is true only for *jmp and ret* operations.
// Addressing is already determined from Code[] (true if and only if any of %L,%P,%R appear).
// OpBytes is already determined from Code[] (1 for a valid mnemonic + 1 for %D,%1D,%2D,%R,%P,%B,%I + 2 for %L,%W).
byte Mode[0x100] = {
 1, 14, 15,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 7,  6,  7,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 7, 14,  9,  1,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 7,  6,  9,  1,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 6, 14,  2,  3,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 6,  6,  2,  3,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 6, 14,  2,  3,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 6,  6,  2, 25,  2,  3,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
14, 14,  2,  1,  1,  3,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
 3,  6,  2,  1,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 2, 14,  2,  1,  1,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
 2,  6,  2,  1,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
 2, 14,  2,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 2,  6,  2,  1,  1,  7,  1,  1,  6,  6,  6,  6,  6,  6,  6,  6,
 1, 14,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 1,  6,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1
};

char *Code[0x100] = {
"nop",           "ajmp %P",  "ljmp %L",     "rr A",
   "inc A",          "inc %D",         "inc %i",          "inc %i",
   "inc %n",          "inc %n",          "inc %n",          "inc %n",
   "inc %n",          "inc %n",          "inc %n",          "inc %n",
"jbc %B, %R",    "acall %P", "lcall %L",    "rrc A",
   "dec A",          "dec %D",         "dec %i",          "dec %i",
   "dec %n",          "dec %n",          "dec %n",          "dec %n",
   "dec %n",          "dec %n",          "dec %n",          "dec %n",
"jb %B, %R",     "ajmp %P",  "ret",         "rl A",
   "add A, %I",      "add A, %D",      "add A, %i",       "add A, %i",
   "add A, %n",       "add A, %n",       "add A, %n",       "add A, %n",
   "add A, %n",       "add A, %n",       "add A, %n",       "add A, %n",
"jnb %B, %R",    "acall %P", "reti",        "rlc A",
   "addc A, %I",     "addc A, %D",     "addc A, %i",      "addc A, %i",
   "addc A, %n",      "addc A, %n",      "addc A, %n",      "addc A, %n",
   "addc A, %n",      "addc A, %n",      "addc A, %n",      "addc A, %n",
"jc %R",         "ajmp %P",  "orl %D, A",   "orl %D, %I",
   "orl A, %I",      "orl A, %D",      "orl A, %i",       "orl A, %i",
   "orl A, %n",       "orl A, %n",       "orl A, %n",       "orl A, %n",
   "orl A, %n",       "orl A, %n",       "orl A, %n",       "orl A, %n",
"jnc %R",        "acall %P", "anl %D, A",   "anl %D, %I",
   "anl A, %I",      "anl A, %D",      "anl A, %i",       "anl A, %i",
   "anl A, %n",       "anl A, %n",       "anl A, %n",       "anl A, %n",
   "anl A, %n",       "anl A, %n",       "anl A, %n",       "anl A, %n",
"jz %R",         "ajmp %P",  "xrl %D, A",   "xrl %D, %I",
   "xrl A, %I",      "xrl A, %D",      "xrl A, %i",       "xrl A, %i",
   "xrl A, %n",       "xrl A, %n",       "xrl A, %n",       "xrl A, %n",
   "xrl A, %n",       "xrl A, %n",       "xrl A, %n",       "xrl A, %n",
"jnz %R",        "acall %P", "orl C, %B",   "jmp @A+DPTR",
   "mov A, %I",      "mov %D, %I",     "mov %i, %I",      "mov %i, %I",
   "mov %n, %I",      "mov %n, %I",      "mov %n, %I",      "mov %n, %I",
   "mov %n, %I",      "mov %n, %I",      "mov %n, %I",      "mov %n, %I",
"sjmp %R",       "ajmp %P",  "anl C, %B",   "movc A, @A+PC",
   "div AB",         "mov %2D, %1D",   "mov %D, %i",      "mov %D, %i",
   "mov %D, %n",      "mov %D, %n",      "mov %D, %n",      "mov %D, %n",
   "mov %D, %n",      "mov %D, %n",      "mov %D, %n",      "mov %D, %n",
"mov DPTR, %W",  "acall %P", "mov %B, C",   "movc A, @A+DPTR",
   "subb A, %I",     "subb A, %D",     "subb A, %i",      "subb A, %i",
   "subb A, %n",      "subb A, %n",      "subb A, %n",      "subb A, %n",
   "subb A, %n",      "subb A, %n",      "subb A, %n",      "subb A, %n",
"orl C, /%B",    "ajmp %P",  "mov C, %B",   "inc DPTR",
   "mul AB",         "ERROR",          "mov %i, %D",      "mov %i, %D",
   "mov %n, %D",      "mov %n, %D",      "mov %n, %D",      "mov %n, %D",
   "mov %n, %D",      "mov %n, %D",      "mov %n, %D",      "mov %n, %D",
"anl C, /%B",    "acall %P", "cpl %B",      "cpl C",
   "cjne A, %I, %R", "cjne A, %D, %R", "cjne %i, %I, %R", "cjne %i, %I, %R",
   "cjne %n, %I, %R", "cjne %n, %I, %R", "cjne %n, %I, %R", "cjne %n, %I, %R",
   "cjne %n, %I, %R", "cjne %n, %I, %R", "cjne %n, %I, %R", "cjne %n, %I, %R",
"push %D",       "ajmp %P",  "clr %B",      "clr C",
   "swap A",         "xch A, %D",      "xch A, %i",       "xch A, %i",
   "xch A, %n",       "xch A, %n",       "xch A, %n",       "xch A, %n",
   "xch A, %n",       "xch A, %n",       "xch A, %n",       "xch A, %n",
"pop %D",        "acall %P", "setb %B",     "setb C",
   "da A",           "djnz %D, %R",    "xchd A, %i",      "xchd A, %i",
   "djnz %n, %R",     "djnz %n, %R",     "djnz %n, %R",     "djnz %n, %R",
   "djnz %n, %R",     "djnz %n, %R",     "djnz %n, %R",     "djnz %n, %R",
"movx A, @DPTR", "ajmp %P",  "movx A, %i", "movx A, %i",
   "clr A",          "mov A, %D",      "mov A, %i",       "mov A, %i",
   "mov A, %n",       "mov A, %n",       "mov A, %n",       "mov A, %n",
   "mov A, %n",       "mov A, %n",       "mov A, %n",       "mov A, %n",
"movx @DPTR, A", "acall %P", "movx %i, A", "movx %i, A",
   "cpl A",          "mov %D, A",      "mov %i, A",       "mov %i, A",
   "mov %n, A",       "mov %n, A",       "mov %n, A",       "mov %n, A",
   "mov %n, A",       "mov %n, A",       "mov %n, A",       "mov %n, A"
};

void PrintByte(byte B) {
   if (B >= 0xa0) putchar('0');
   printf("%02xh", B);
}

void PrintWord(word W) {
   if (W >= 0xa000) putchar('0');
   printf("%04xh", W);
}

char *SFRs[0x80] = {
  "P0", "SP", "DPL", "DPH", 0, 0, 0, "PCON",
  "TCON", "TMOD", "TL0", "TL1", "TH0", "TH1", 0, 0,
  "P1", 0, 0, 0, 0, 0, 0, 0, "SCON", "SBUF", 0, 0, 0, 0, 0, 0,
  "P2", 0, 0, 0, 0, 0, 0, 0, "IE", 0, 0, 0, 0, 0, 0, 0,
  "P3", 0, 0, 0, 0, 0, 0, 0, "IP", 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, "T2CON", 0, "RCAP2L", "RCAP2H", "TL2", "TH2", 0, 0,
  "PSW", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  "ACC", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  "B", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void PrintData(byte D) { /* THIS ASSUMES THE 8052 */
   if (!Generating) return;
   if (D < 0x80 || SFRs[D - 0x80] == 0) { PrintByte(D); return; }
   printf(SFRs[D - 0x80]);
}

char *Bits[0x80] = {
   0, 0, 0, 0, 0, 0, 0, 0,
   "IT0", "IE0", "IT1", "IE1", "TR0", "TF0", "TR1", "TF1",
   "T2", "T2EX", 0, 0, 0, 0, 0, 0,
   "RI", "TI", "RB8", "TB8", "REN", "SM2", "SM1", "SM0",
   0, 0, 0, 0, 0, 0, 0, 0,
   "EX0", "ET0", "EX1", "ET1", "ES", "ET2", 0, "EA",
   "RXD", "TXD", "INT0", "INT1", "T0", "T1", "WR", "RD",
   "PX0", "PT0", "PX1", "PT1", "PS", "PT2", 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   "CP_RL2", "C_T2", "TR2", "EXEN2", "TCLK", "RCLK", "EXF2", "TF2",
   "P", 0, "OV", "RS0", "RS1", "F0", "AC", "CY", 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void PrintBit(byte B) {
   if (!Generating) return;
   if (B < 0x80) { PrintByte(B); return; }
   B -= 0x80;
   if (Bits[B] != 0) { printf(Bits[B]); return; }
   byte Byte = B&0x78, Bit = B&0x07;
   if (SFRs[Byte] != 0) { printf("%s.%1x", SFRs[Byte], Bit); return; }
   PrintByte((byte)(B + 0x80));
}

void PrintImmByte(byte B) {
   if (Generating) putchar('#'), PrintByte(B);
}

void PrintImmWord(word W) {
   if (Generating) putchar('#'), PrintWord(W);
}

byte CheckSum; word HighAddr;

byte Nib(int X) {
   if (X >= '0' && X <= '9') return X - '0';
   if (X >= 'a' && X <= 'f') return X - 'a' + 10;
   if (X >= 'A' && X <= 'F') return X - 'A' + 10;
   fprintf(stderr, "Bad hexadecimal digit in input.\n");
   exit(EXIT_FAILURE);
}

byte GetHex(void) {
   int A = getchar(), B = getchar();
   if (A == EOF || B == EOF) {
      fprintf(stderr, "Unexpected EOF.\n"); exit(EXIT_FAILURE);
   }
   byte Bt = Nib(A) << 4 | Nib(B);
   CheckSum = (CheckSum + Bt)&0xff; return Bt;
}

word GetWord(void) {
   word A = GetHex(), B = GetHex();
   return (A << 8) | B;
}

void HexLoad(void) {
   HighAddr = 0;
   while (true) {
      int Ch;
      do {
         Ch = getchar();
         if (Ch == EOF) { fprintf(stderr, "Unexpected EOF.\n"); exit(EXIT_FAILURE); }
      } while (Ch != ':');
      byte CheckSum = 0;
      byte Size = GetHex(); word Addr = GetWord(); byte Mark = GetHex();
      byte Buffer[0x10]; for (int I = 0; I < Size; I++) Buffer[I] = GetHex();
      (void)GetHex();
      if (CheckSum != 0) {
         fprintf(stderr, "Bad checksum.\n"); exit(EXIT_FAILURE);
      }
      if (Mark) break;
      if (Addr >= 0x4000 - Size) {
         printf("Address out of range 0 - 4000h in input.\n"); exit(EXIT_FAILURE);
      }
      for (int I = 0; I < Size; I++, Addr++) Hex[Addr] = Buffer[I], Ref[Addr] = 0;
      if (Addr > HighAddr) HighAddr = Addr;
      (void)getchar();
   }
}

#define IN_RANGE(A) ((A) >= 0x0000 && (A) < HighAddr)

word Address;
void PutLabel(word PC) {
   if (!Generating) Address = PC;
   else if (PC >= 0x4000) PrintWord(PC);
   else printf("%c%04x", (Ref[PC]&LINKS) + 'A', PC);
}

void MakeOp(char *S) {
   for (int A = 1; *S != '\0'; S++) {
      if (*S != '%') {
         if (Generating) putchar(*S);
         continue;
      }
      S++;
      if (*S >= '0' && *S <= '2') A = *S++ - '0';
      switch (*S) {
         case 'L': { byte B = Arg[A++]; PutLabel(B << 8 | Arg[A++]); } break;
         case 'P': PutLabel(Paged(Arg[A++])); break;
         case 'R': PutLabel(Relative(Arg[A++])); break;
         case 'D': PrintData(Arg[A++]); break;
         case 'B': PrintBit(Arg[A++]); break;
         case 'I': PrintImmByte(Arg[A++]); break;
         case 'W': { byte B = Arg[A++]; PrintImmWord(B << 8 | Arg[A++]); } break;
         case 'i': if (Generating) printf("@R%1x", Arg[0]&1); break;
         case 'n': if (Generating) printf("R%1x", Arg[0]&7); break;
         default:
            fprintf(stderr, "Bad format string, PC = %04x.\n", PC);
         exit(EXIT_FAILURE);
      }
   }
   if (Generating) putchar('\n');
   else if (!IN_RANGE(Address)) printf("REF: %04x\n", Address);
   else PushAddr(Address);
}

bool Disassemble(void) {
   if (Ref[PC]&ARG) {
      fprintf(stderr, "OP into ARG at %04x.\n", PC); return true;
   }
   if (Generating) {
      if (Ref[PC]&ENTRY) { PutLabel(PC); printf(":\n"); }
   } else {
      if (Ref[PC]&OP) return true;
      Ref[PC] |= OP;
   }
   byte Op = Arg[0] = Hex[PC++];
   byte Xs = Mode[Op];
   bool Indirect = (Xs&0x10) != 0, Breaking = (Xs&8) != 0, Addressing = (Xs&4) != 0;
   byte OpBytes = Xs&3;
   char *Name = Code[Op];
   for (int I = 1; I < OpBytes; I++) {
      if (Ref[PC]&OP) {
         fprintf(stderr, "ARG into OP at %04x.\n", PC); return true;
      }
      if (!Generating) {
         if (Ref[PC]&ARG) {
            fprintf(stderr, "ARG into ARG at %04x.\n", PC); return true;
         }
         Ref[PC] |= ARG;
      }
      Arg[I] = Hex[PC++];
   }
   if (Generating) {
      if (!Breaking) printf("   ");
      MakeOp(Name);
   } else {
      if (Addressing) MakeOp(Name);
      if (Indirect) {
         printf("Indirect jump at %04x\n", PC - OpBytes);
      }
   }
   return Generating? !Ref[PC]: Breaking;
}

void PCHAR(byte B) { putchar((B < 0x20 || B >= 0x7f)? ' ': B); }

int Ended; FILE *EntryF;

byte fGetHex(void) {
   if (Ended) return 0;
   int A = fgetc(EntryF), B = fgetc(EntryF);
   if (A == EOF || B == EOF) { Ended = 1; return 0; }
   return Nib(A) << 4 | Nib(B);
}

word fGetWord(void) {
   word A = fGetHex(), B = fGetHex();
   (void)fgetc(EntryF);
   if (Ended) return 0;
   return (A << 8) | B;
}

int main(void) {
   HexLoad();
   Generating = false, fprintf(stderr, "First pass\n");
   EntryF = fopen("entries", "r");
   if (EntryF == NULL) {
      fprintf(stderr, "No entry points listed.\n"); return EXIT_FAILURE;
   }
   for (Ended = 0, EP = Entries; !Ended; ) {
      word W = fGetWord(); if (!Ended) PushAddr(W);
   }
   while (EP > Entries) {
      PC = *--EP;
      while (!Disassemble());
   }
   Generating = true, fprintf(stderr, "Second pass\n");
   if (Ref[0]) printf("org 0\n");
   for (PC = 0x00; IN_RANGE(PC); ) {
      byte F, Buf[16];
      if (!Ref[PC]) {
         printf("DATA AT "); PrintWord(PC); putchar('\n');
         for (F = 0; !Ref[PC] && IN_RANGE(PC); PC++) {
            Buf[F++] = Hex[PC];
            if (F == 16) {
               for (F = 0; F < 16; F++) {
                  printf("%2x", Buf[F]);
                  if (F < 15) putchar(' '); else putchar('|');
               }
               for (F = 0; F < 16; F++) PCHAR(Buf[F]);
               putchar('|'); putchar('\n');
               F = 0;
            }
         }
         if (F > 0) {
            byte F1;
            for (F1 = 0; F1 < F; F1++) printf("%2x ", Buf[F1]);
            for (; F1 < 16; F1++) {
               Buf[F1] = 0;
               printf("  "); if (F1 < 15) putchar(' '); else putchar('|');
            }
            for (F = 0; F < 16; F++) PCHAR(Buf[F]);
            putchar('|'); putchar('\n');
         }
         if (!IN_RANGE(PC)) break;
         printf("org "); PrintWord(PC); putchar('\n');
      }
      while (!Disassemble());
   }
   return EXIT_SUCCESS;
}
