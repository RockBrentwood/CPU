#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>

typedef uint8_t byte;
typedef int8_t sbyte;
typedef uint16_t word;

// The MCS-51 has the following register/address space alignment:
// ∙	byte xdata[0x10000];	// external RAM for variable data.
// ∙	byte cdata[0x10000];	// external ROM for code and constant data.
// ∙	byte ddata[0x100];	// internal RAM for data and special function registers, accessible only by direct addressing.
// ∙	byte idata[0x100];	// internal RAM for data and stack space, accessible only through 8-bit pointers in R0, R1 and SP.
// ∙	bool bdata[0x100];	// internal RAM for 1-bit variable data.
// ∙	byte rdata[8];		// register file for R0,R1,R2,R3,R4,R5,R6,R7.
// ∙	word DPTR, PC, AB;	// 16-bit registers.
// ∙	byte A;			// 8-bit register.
// ∙	bool C;			// 1-bit register.
// The subspace of ddata[] in the address range 0x80-0xff makes up the ‟special function registers”.
// Several of the registers in ddata[] and bdata[] are specially named, as indicated below, respectively, in DTab[] and BTab[].
// All of the registers named in DTab[] and BTab[] reside in the special function registers area.
// The register spaces, in some places, overlap, with the following aliasing relations:
// ∙	ddata[D] = idata[D], for D ∈ [0x00,0x7f],
// ∙	bdata[B] = ddata[0x20 + B/8].(B%8), for B ∈ [0x00,0x7f],
// ∙	bdata[B] = ddata[B&0xf8].(B%8), for B ∈ [0x80,0xff],
// ∙	rdata[R] = ddata[ddata[0xd0]&0x18|R], for R ∈ [0,7] (i.e. rdata[R] = ddata[PSW&0x18|R] = ddata[RS1:RS0:R]),
// ∙	A = ddata[0xe0] (i.e. A = ACC),
// ∙	DPTR = ddata[0x83]:ddata[0x82] (i.e. DPTR = DPH:DPL),
// ∙	AB = ddata[0xf0]:ddata[0xe0] (i.e. AB = B:ACC),
// ∙	C = bdata[0xd7] (i.e. C = CY = PSW.7).
// As such, the only «independent» spaces are xdata[], cdata[], {d,i}data[0x00-0x7f], idata[0x80-0xff], ddata[0x80-0xff] and PC.
// Everything else is aliased, somewhere, into one of these spaces.
// Note also that the subspace idata[0x80-0xff] is not officially available on the 8051.

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

// (Addressing? 4: 0) | OpBytes
// Addressing is already determined from Code[] (true if and only if any of %L,%P,%R appear).
// OpBytes is already determined from Code[] (1 for a valid mnemonic + 1 for %D,%R,%P,%B,%I + 2 for %L,%W,%X).
byte Mode[0x100] = {
 1,  6,  7,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 7,  6,  7,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 7,  6,  1,  1,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 7,  6,  1,  1,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 6,  6,  2,  3,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 6,  6,  2,  3,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 6,  6,  2,  3,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 6,  6,  2,  9,  2,  3,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
 6,  6,  2,  1,  1,  3,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
 3,  6,  2,  1,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 2,  6,  2,  1,  1,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
 2,  6,  2,  1,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
 2,  6,  2,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 2,  6,  2,  1,  1,  7,  1,  1,  6,  6,  6,  6,  6,  6,  6,  6,
 1,  6,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
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
   "div AB",         "mov %X",         "mov %D, %i",      "mov %D, %i",
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

bool Generating;
unsigned Line;

void Error(bool Fatal, char *Format, ...) {
   fprintf(stderr, "[%u] ", Line);
   va_list AP; va_start(AP, Format), vfprintf(stderr, Format, AP), va_end(AP);
   fputc('\n', stderr);
   if (Fatal) exit(EXIT_FAILURE);
}

char *LookUp(byte Type, word Key) {
   switch (Type) {
      default:
   // cdata address.
      case 0: return NULL; // For now.
   // ddata address.
      case 1: return Key >= 0x80 && Key < 0x100 && SFRs[Key - 0x80] != NULL? SFRs[Key - 0x80]: NULL;
   // bdata address.
      case 2: return Key >= 0x80 && Key < 0x100 && Bits[Key - 0x80] != NULL? Bits[Key - 0x80]: NULL;
   }
}

byte Ref[0x4000], Hex[0x4000];
#define ENTRY 0x80
#define OP    0x40
#define ARG   0x20
#define LINKS 0x1f

word LoPC, HiPC, PC;
#define InRange(A) ((A) >= LoPC && (A) < HiPC)

word Entries[0x400], *EP;
const word *EEnd = Entries + sizeof Entries/sizeof Entries[0];

void PushAddr(word Address) {
   if (!InRange(Address)) {
      printf("External reference at %04x to %04x\n", PC, Address); return;
   } else if (Ref[Address]&ARG) {
      Error(false, "Entry into ARG at %04x.", PC); return;
   }
   if ((Ref[Address]&LINKS) <= LINKS) Ref[Address]++;
   if (Ref[Address]&ENTRY) return; else Ref[Address] |= ENTRY;
   if (Ref[Address]&OP) return;
   if (EP >= EEnd) Error(true, "Too many entries, PC = %04x.", PC);
   *EP++ = Address;
}

void PutByte(byte B) {
   if (B >= 0xa0) putchar('0');
   printf("%02xh", B);
}

void PutWord(word W) {
   if (W >= 0xa000) putchar('0');
   printf("%04xh", W);
}

byte Nib(int Ch) {
   if (Ch == EOF) Error(true, "Unexpected EOF.");
   else if (!isxdigit(Ch)) Error(true, "Bad hexadecimal digit in input.");
   return isdigit(Ch)? Ch - '0': isupper(Ch)? Ch - 'A' + 0xA: Ch - 'a' + 0xa;
}

byte CheckSum;

byte GetByte(void) {
   byte A = Nib(getchar()), B = A << 4 | Nib(getchar());
   CheckSum += B;
   return B;
}

word GetWord(void) {
   word A = GetByte(), B = GetByte();
   return A << 8 | B;
}

void HexLoad(void) {
   Line = 1, HiPC = 0x0000, LoPC = 0xffff;
   while (true) {
      int Ch;
      do
         if ((Ch = getchar()) == '\n') Line++;
         else if (Ch == EOF) Error(true, "Unexpected EOF.");
      while (Ch != ':');
      byte CheckSum = 0;
      byte Size = GetByte(); word CurPC = GetWord(); bool Mark = GetByte() != 0;
      byte Buffer[0x10]; for (word H = 0; H < Size; H++) Buffer[H] = GetByte();
      (void)GetByte();
      if (CheckSum != 0) Error(true, "Bad checksum.");
      if (Mark || Size == 0) break;
      if (CurPC < LoPC) LoPC = CurPC;
      for (word H = 0; H < Size; H++, CurPC++) Hex[CurPC] = Buffer[H], Ref[CurPC] = 0;
      if (CurPC > HiPC) HiPC = CurPC;
      if (getchar() == '\n') Line++;
   }
}

void PutDReg(byte D) { // This assumes the 8052.
   char *Name = LookUp(1, D);
   if (Name != NULL) printf("%s", Name); else PutByte(D);
}

void PutBReg(byte B) {
   char *Name = LookUp(2, B);
   if (Name != NULL) { printf("%s", Name); return; }
   Name = LookUp(1, (B < 0x80? B >> 3 | 0x20: B&~7));
   if (Name != NULL) { printf("%s.%u", Name, B&7); return; }
   PutByte(B);
}

void PutLabel(word C) {
   char *Name = LookUp(0, C);
   if (Name != NULL) { printf("%s", Name); return; }
   byte R = Ref[C];
   if (InRange(C)) printf("%c%04x", 'A' + (R&LINKS), C);
   else PutWord(C);
}

int Fetch(bool IsOp) {
   if (IsOp) {
      if (Ref[PC]&ARG) {
         Error(false, "OP into ARG at %04x.", PC); return EOF;
      }
      if (Generating) {
         if (Ref[PC]&ENTRY) { PutLabel(PC); printf(":\n"); }
      } else {
         if (Ref[PC]&OP) return EOF;
         Ref[PC] |= OP;
      }
   } else {
      if (Ref[PC]&OP) {
         Error(false, "ARG into OP at %04x.", PC); return EOF;
      }
      if (!Generating) {
         if (Ref[PC]&ARG) {
            Error(false, "ARG into ARG at %04x.", PC); return EOF;
         }
         Ref[PC] |= ARG;
      }
   }
   return Hex[PC++];
}

void Disassemble(void) {
   for (int Ch; (Ch = Fetch(true)) != EOF; ) {
      byte Op = (byte)Ch;
      byte Xs = Mode[Op];
   // Indirectly-addressed jump or call.
      bool Indirect = Op == 0x73;
   // For *jmp and ret* operations.
      bool Breaking = (Op&0x1f) == 0x01 || Op == 0x80 || Op == 0x02 || (Op&~0x10) == 0x22;
      if (Generating) {
         if (!Breaking) printf("   ");
      } else if (Indirect) printf("Indirect jump at %04x\n", PC - 1);
   // True if and only if any of %L,%P,%R appear; i.e. the operation makes direct reference to an external code address.
      bool Addressing = (Xs&4) != 0;
   // 1 for a valid mnemonic + 1 for %D,%R,%P,%B,%I + 2 for %L,%W,%X.
      char *Name = Code[Op];
      for (char *S = Name; *S != '\0'; S++) {
         if (*S != '%') {
            if (Generating) putchar(*S);
            continue;
         }
         byte B; word Lab;
         switch (*++S) {
         // 8-bit data or address:
            case 'B': case 'D': case 'I':
               if ((Ch = Fetch(false)) == EOF) return; else B = (byte)Ch;
            break;
         // Relative address (8-bit signed):
            case 'R':
               if ((Ch = Fetch(false)) == EOF) return; else Lab = PC + (sbyte)Ch;
            break;
         // Paged address (3+8-bit unsigned):
            case 'P':
               if ((Ch = Fetch(false)) == EOF) return; else Lab = PC&0xf800 | (Op&0xe0) << 3 | (byte)Ch;
            break;
         // 16-bit data or address
            case 'L': case 'W': case 'X':
               if ((Ch = Fetch(false)) == EOF) return; else Lab = (byte)Ch;
               if ((Ch = Fetch(false)) == EOF) return; else Lab = Lab << 8 | (byte)Ch;
            break;
         }
         if (Generating) switch (*S) {
            case 'I': putchar('#'), PutByte(B); break;
            case 'B': PutBReg(B); break;
            case 'D': PutDReg(B); break;
            case 'X': PutDReg(Lab&0xff), printf(", "), PutDReg(Lab >> 8); break;
            case 'R': case 'P': case 'L': PutLabel(Lab); break;
            case 'W': putchar('#'), PutWord(Lab); break;
            case 'i': printf("@R%1x", (unsigned)Op&1); break;
            case 'n': printf("R%1x", (unsigned)Op&7); break;
            default: Error(true, "Bad format string, PC = %04x.", PC);
         } else switch (*S) {
            case 'R': case 'P': case 'L': PushAddr(Lab); break;
         }
      }
      if (Generating) putchar('\n');
      if (Generating? Ref[PC] == 0: Breaking) break;
   }
}

void PutData(byte *Buf, size_t N, size_t Max) {
   for (size_t n = 0; n < N; n++) {
      printf("%2x", Buf[n]);
      if (n < Max - 1) putchar(' '); else putchar('|');
   }
   for (size_t n = N; n < Max; n++) {
      printf("  ", Buf[n]), Buf[n] = 0;
      if (n < Max - 1) putchar(' '); else putchar('|');
   }
   for (size_t n = 0; n < Max; n++) {
      int Ch = Buf[n]; putchar(isprint(Ch)? Ch: ' ');
   }
   putchar('|'); putchar('\n');
}


bool Configure(char *Path) {
   EP = Entries, Line = 1;
   FILE *EntryF = fopen("entries", "r");
   if (EntryF == NULL) return false;
   for (int Ch; (Ch = fgetc(EntryF)) != EOF && isxdigit(Ch); ) {
      word W = Nib(Ch);
      while ((Ch = fgetc(EntryF)) != EOF && isxdigit(Ch)) W = W << 4 | Nib(Ch);
      PushAddr(W);
      if ((Ch = fgetc(EntryF)) == '\n') Line++; else ungetc(Ch, EntryF);
   }
   return true;
}

int main(void) {
   HexLoad();
   Generating = false, fprintf(stderr, "First pass\n");
   if (!Configure("entries"))
      Error(false, "No entry points listed, using 0x%04x as the starting address.", LoPC), PushAddr(LoPC);
   while (EP > Entries) PC = *--EP, Disassemble();
   Generating = true, fprintf(stderr, "Second pass\n");
   if (Ref[0]) printf("org 0\n");
   for (PC = 0x00; PC < HiPC; ) {
      byte Buf[0x10]; size_t Max = sizeof Buf/sizeof Buf[0], N;
      if (Ref[PC] == 0) {
         printf(";; DATA at "), PutWord(PC), putchar('\n');
         for (N = 0; !Ref[PC] && InRange(PC); PC++) {
            Buf[N] = Hex[PC];
            if (++N >= Max) PutData(Buf, Max, Max), N = 0;
         }
         if (N > 0) PutData(Buf, N, Max);
         if (!InRange(PC)) break;
         printf("org "), PutWord(PC), putchar('\n');
      }
      Disassemble();
   }
   return EXIT_SUCCESS;
}
