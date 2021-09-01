#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>

const bool Loud = false; // True for verbose output.

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
// Unnamed registers in the spaces xdata[], bdata[], cdata[], ddata[] are decompiled respectively as Axxxx, Bxx, {B-Z}xxxx, Dxx,
// containing an uppercase prefix (A-Z) and a hexadecimal numeral of 2 or 4 ‟hexits” (0-9, a-f).

struct {
   char *Name; byte Key;
} DTab[] = {
// MCS-51 generic special function byte registers.
   {"P0", 0x80}, {"SP", 0x81}, {"DPL", 0x82}, {"DPH", 0x83},
   {"PCON", 0x87}, {"TCON", 0x88}, {"TMOD", 0x89}, {"TL0", 0x8a},
   {"TL1", 0x8b}, {"TH0", 0x8c}, {"TH1", 0x8d}, {"P1", 0x90},
   {"SCON", 0x98}, {"SBUF", 0x99}, {"P2", 0xa0}, {"IE", 0xa8},
   {"P3", 0xb0}, {"IP", 0xb8}, {"PSW", 0xd0}, {"A", 0xe0}, {"B", 0xf0},
// 8052-specific registers.
   {"T2CON", 0xc8}, {"RCAP2L", 0xca}, {"RCAP2H", 0xcb}, {"TL2", 0xcc}, {"TH2", 0xcd}
}, BTab[] = {
// MCS-51 generic special function bit registers.
   {"IT0", 0x88}, {"IE0", 0x89}, {"IT1", 0x8a}, {"IE1", 0x8b}, {"TR0", 0x8c}, {"TF0", 0x8d},
   {"TR1", 0x8e}, {"TF1", 0x8f}, {"RI", 0x98}, {"TI", 0x99}, {"RB8", 0x9a}, {"TB8", 0x9b},
   {"REN", 0x9c}, {"SM2", 0x9d}, {"SM1", 0x9e}, {"SM0", 0x9f}, {"EX0", 0xa8}, {"ET0", 0xa9},
   {"EX1", 0xaa}, {"ET1", 0xab}, {"ES", 0xac}, {"EA", 0xaf}, {"PX0", 0xb8}, {"PT0", 0xb9},
   {"PX1", 0xba}, {"PT1", 0xbb}, {"PS", 0xbc}, {"RXD", 0xb0}, {"TXD", 0xb1}, {"INT0", 0xb2},
   {"INT1", 0xb3}, {"T0", 0xb4}, {"T1", 0xb5}, {"WR", 0xb6}, {"RD", 0xb7}, {"P", 0xd0},
   {"OV", 0xd2}, {"RS0", 0xd3}, {"RS1", 0xd4}, {"F0", 0xd5}, {"AC", 0xd6}, {"C", 0xd7},
// 8052-specific registers.
   {"T2", 0x90}, {"T2EX", 0x91}, {"ET2", 0xad}, {"PT2", 0xbd}, {"CP_RL2", 0xc8}, {"C_T2", 0xc9},
   {"TR2", 0xca}, {"EXEN2", 0xcb}, {"TCLK", 0xcc}, {"RCLK", 0xcd}, {"EXF2", 0xce}, {"TF2", 0xcf}
};

const size_t Ds = sizeof DTab/sizeof DTab[0], Bs = sizeof BTab/sizeof BTab[0];

char *Code[0x100] = {
   "", "P", "L", "A", "A", "D", "i", "i", "n", "n", "n", "n", "n", "n", "n", "n",
   "BR", "P", "L", "A", "A", "D", "i", "i", "n", "n", "n", "n", "n", "n", "n", "n",
   "BR", "P", "", "A", "AI", "AD", "Ai", "Ai", "An", "An", "An", "An", "An", "An", "An", "An",
   "BR", "P", "", "A", "AI", "AD", "Ai", "Ai", "An", "An", "An", "An", "An", "An", "An", "An",
   "CR", "P", "DA", "DI", "AI", "AD", "Ai", "Ai", "An", "An", "An", "An", "An", "An", "An", "An",
   "CR", "P", "DA", "DI", "AI", "AD", "Ai", "Ai", "An", "An", "An", "An", "An", "An", "An", "An",
   "AR", "P", "DA", "DI", "AI", "AD", "Ai", "Ai", "An", "An", "An", "An", "An", "An", "An", "An",
   "AR", "P", "CB", "dA", "AI", "DI", "iI", "iI", "nI", "nI", "nI", "nI", "nI", "nI", "nI", "nI",
   "R", "P", "CB", "AZ", "bA", "DD", "iD", "iD", "nD", "nD", "nD", "nD", "nD", "nD", "nD", "nD",
   "dW", "P", "CB", "Ad", "AI", "AD", "Ai", "Ai", "An", "An", "An", "An", "An", "An", "An", "An",
   "CB", "P", "CB", "d", "bA", "", "iD", "iD", "nD", "nD", "nD", "nD", "nD", "nD", "nD", "nD",
   "CB", "P", "B", "C", "AIR", "ADR", "iIR", "iIR", "nIR", "nIR", "nIR", "nIR", "nIR", "nIR", "nIR", "nIR",
   "D", "P", "B", "C", "A", "AD", "Ai", "Ai", "An", "An", "An", "An", "An", "An", "An", "An",
   "D", "P", "B", "C", "A", "DR", "Ai", "Ai", "nR", "nR", "nR", "nR", "nR", "nR", "nR", "nR",
   "Ad", "P", "Ap", "Ap", "A", "AD", "Ai", "Ai", "An", "An", "An", "An", "An", "An", "An", "An",
   "Ad", "P", "Ap", "Ap", "A", "AD", "Ai", "Ai", "An", "An", "An", "An", "An", "An", "An", "An"
};

char Mode[0x100] =
   "aHHmAAAAAAAAAAAA" "bIIoBBBBBBBBBBBB" "cHhnCCCCCCCCCCCC" "dIipDDDDDDDDDDDD"
   "cHJJJJJJJJJJJJJJ" "dIKKKKKKKKKKKKKK" "dHLLLLLLLLLLLLLL" "cIJeMMMMMMMMMMMM"
   "HHKufNNNNNNNNNNN" "MINuEEEEEEEEEEEE" "qHMAg.MMMMMMMMMM" "rIssFFFFFFFFFFFF"
   "jHxxlOOOOOOOOOOO" "kIyyzGPPGGGGGGGG" "vHvvxMMMMMMMMMMM" "wIwwtNNNNNNNNNNN";

bool Generating;
unsigned Line;
bool Recursing = true, NoData = false;

void Error(bool Fatal, char *Format, ...) {
   fprintf(stderr, "[%u] ", Line);
   va_list AP; va_start(AP, Format), vfprintf(stderr, Format, AP), va_end(AP);
   fputc('\n', stderr);
   if (Fatal) exit(EXIT_FAILURE);
}

void *Allocate(size_t N) {
   void *X = malloc(N);
   if (X == NULL) Error(true, "Out of memory.");
   return X;
}

// ALink threads the symbols by Name and Namespace.
// BLink threads the symbols by Key and Namespace.
// CLink threads the symbols separately for each Namespace.
typedef struct Symbol *Symbol;
struct Symbol {
   char *Name; word Key; bool Defined;
   Symbol ALink, BLink, CLink;
};

Symbol GetSym(byte Type, char *Name) {
   static Symbol HTab[0x100];
   byte H = Type;
   for (char *S = Name; *S != '\0'; H += *S++);
   Symbol *HP = &HTab[H];
   for (Symbol Sym = *HP; Sym != NULL; Sym = Sym->ALink) if (strcmp(Sym->Name, Name) == 0) return Sym;
   Symbol Sym = Allocate(sizeof *Sym);
   Sym->Name = strdup(Name), Sym->Defined = false, Sym->ALink = *HP;
   return *HP = Sym;
}

char *LookUp(byte Type, Symbol Sym, word Key) {
// Type = {0,1,2} for {c,d,b}data address.
   static Symbol HTab[0x100];
   byte H = Key ^ (Key >> 8) ^ Type;
   Symbol *HP = &HTab[H], Cur, Prev;
   for (Prev = NULL, Cur = *HP; Cur != NULL; Prev = Cur, Cur = Cur->BLink) if (Cur->Key == Key) {
      if (Sym != NULL) {
         Sym->Defined = true, Sym->Key = Key, Sym->BLink = Cur->BLink, Cur->BLink = NULL;
         if (Prev == NULL) *HP = Sym; else Prev->BLink = Sym;
      }
      return Cur->Name;
   }
   if (Sym != NULL) Sym->Defined = true, Sym->Key = Key, Sym->BLink = *HP, *HP = Sym;
   return NULL;
}

byte Ref[0x10000], Hex[0x10000];
word LoPC, HiPC, PC;

word Entries[0x400], *EP;
const word *EEnd = Entries + sizeof Entries/sizeof Entries[0];

void PushAddr(word Address) {
   const unsigned MaxLinks = 0x19;
   if (Ref[Address] == 0) {
      printf("// External reference at %04x to %04x\n", PC, Address); return;
   } else if ((Ref[Address]&~0x80) == 0) {
      Error(false, "Entry into ARG at %04x.", PC); return;
   } else if ((Ref[Address]&~0x80) < MaxLinks) Ref[Address]++;
   if (Ref[Address] > 2) return;
   if (EP >= EEnd) Error(true, "Too many addresses, PC = %04x.", PC);
   *EP++ = Address;
}

char *Decompile(unsigned char Op) {
   switch (Mode[Op]) {
   // nop
      case 'a': return "delta(1);";
   // jbc B, Addr
      case 'b': return "if (%1) { %1 = 0; goto %2; }";
   // jb B, Addr/jc Addr/jz Addr
      case 'c': return "if (%1) goto %2;";
   // jnb B, Addr/jnc Addr/jnz Addr
      case 'd': return "if (!%1) goto %2;";
   // cjne D1, D2, Addr
      case 'F': return "C = (%1 < %2); if (%1 != %2) goto %3;";
   // djnz D, Addr
      case 'G': return "if (--%1 != 0) goto %2;";
   // jmp @A+DPTR
      case 'e': return "goto %1[%2];";
   // sjmp Addr/ajmp Addr/ljmp Addr
      case 'H': return "goto %1;";
   // acall Addr/lcall Addr
      case 'I': return "%1();";
   // ret
      case 'h': return "return;";
   // reti
      case 'i': return "clri(); return;";
   // swap D
      case 'l': return "%1 = rot(%1, 4);";
   // rr D
      case 'm': return "%1 = rot(%1, -1);";
   // rl D
      case 'n': return "%1 = rot(%1, 1);";
   // rrc D
      case 'o': return "%1:C = C:%1;";
   // rlc D
      case 'p': return "C:%1 = %1:C;";
   // inc D
      case 'A': return "%1++;";
   // dec D
      case 'B': return "%1--;";
   // add D1, D2
      case 'C': return "C:%1 += %2;";
   // addc D1, D2
      case 'D': return "C:%1 += %2 + C;";
   // subb D1, D2
      case 'E': return "C:%1 -= %2 + C;";
   // div AB
      case 'f': return "%1:%2 = div(%1:%2);";
   // mul AB
      case 'g': return "%1:%2 = %2*%1;";
   // orl D1, D2
      case 'J': return "%1 |= %2;";
   // anl D1, D2
      case 'K': return "%1 &= %2;";
   // xrl D1, D2
      case 'L': return "%1 ^= %2;";
   // orl B1, /B2
      case 'q': return "%1 |= !%2;";
   // anl B1, /B2
      case 'r': return "%1 &= !%2;";
   // cpl B
      case 's': return "%1 = !%1;";
   // cpl D
      case 't': return "%1 = ~%1;";
   // mov D1, D2
      case 'M': return "%1 = %2;";
   // mov D2, D1
      case 'N': return "%2 = %1;";
   // xch D1, D2
      case 'O': return "%1 <-> %2;";
   // movc A, @A+PC/movc A, @A+DPTR
      case 'u': return "%1 = %2[%1];";
   // movx A, @DPTR/movx A, @Ri
      case 'v': return "%1 = *%2;";
   // movx @DPTR, A/movx @Ri, A
      case 'w': return "*%2 = %1;";
   // push D
      case 'j': return "push(%1);";
   // pop D
      case 'k': return "%1 = pop();";
   // clr D/clr B
      case 'x': return "%1 = 0;";
   // setb B
      case 'y': return "%1 = 1;";
   // (undefined)
      case '.': return "ERROR";
   // da D
      case 'z': return "%1 = da(%1);";
   // xchd D1, @D2
      case 'P': return "%1 <-d-> %2;";
      default: return NULL;
   }
}

void PutByte(byte B) {
   printf("0x%02x", (unsigned)B);
}

void PutWord(word W) {
   printf("0x%04x", (unsigned)W);
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
      byte HBuf[0x20];
      const size_t HMax = sizeof HBuf/sizeof HBuf[0];
      CheckSum = 0;
      byte Size = GetByte(); word CurPC = GetWord(); bool Mark = GetByte() != 0;
      if (Size > HMax) Error(true, "Record size too large.");
      for (word H = 0; H < Size; H++) HBuf[H] = (byte)GetByte();
      (void)GetByte();
      if (CheckSum != 0) Error(true, "Bad checksum.");
      if (Mark || Size == 0) break;
      if (CurPC < LoPC) LoPC = CurPC;
      for (word H = 0; H < Size; H++, CurPC++) Hex[CurPC] = HBuf[H], Ref[CurPC] = 1;
      if (CurPC > HiPC) HiPC = CurPC;
      if (getchar() == '\n') Line++;
   }
}

void PutDReg(byte D) {
   char *Name = LookUp(1, NULL, D);
   if (Name != NULL) printf("%s", Name); else printf("D%02x", D);
}

void PutBReg(byte B) {
   char *Name = LookUp(2, NULL, B);
   if (Name != NULL) { printf("%s", Name); return; }
   Name = LookUp(1, NULL, B < 0x80? B >> 3 | 0x20: B&~7);
   if (Name != NULL) { printf("%s.%u", Name, B&7); return; }
   printf("B%02x", B);
}

void PutXData(word X) { printf("A%04x", (unsigned)X); }

void PutLabel(word C) {
   char *Name = LookUp(0, NULL, C);
   if (Name != NULL) { printf("%s", Name); return; }
   byte R = Ref[C];
   if (R&0x80) printf("%c%04x", 'A' + (R&0x7f) - 1, C);
   else PutWord(C);
}

int Fetch(bool IsOp) {
   if (Ref[PC] == 0) {
      if (!Generating) Error(false, "Code fall-through at %04x.", PC);
      return EOF;
   } else if (IsOp) {
      if ((Ref[PC]&~0x80) == 0) {
         Error(false, "OP into ARG at %04x.", PC++); return EOF;
      } else if (!Generating) {
         if (Ref[PC]&0x80) return EOF; else Ref[PC] |= 0x80;
      } else if ((Ref[PC]&~0x80) > 1) PutLabel(PC), printf(":\n");
   } else {
      if (Ref[PC] > 0x80) {
         Error(false, "ARG into OP at %04x.", PC); return EOF;
      } else if ((Ref[PC]&~0x80) > 1) {
         Error(false, "ARG into Entry at %04x.", PC); return EOF;
      } else if (!Generating) Ref[PC] = 0x80;
   }
   return (int)Hex[PC++];
}

void Disassemble(void) {
   struct { word Value; int Space; } Rand[4];
   for (int Ch; (Ch = Fetch(true)) != EOF; ) {
      byte Op = (byte)Ch;
   // This is the only indirectly-addressed jump or call in the MCS-51.
      bool Indirect = Op == 0x73;
   // These are the *call operations.
      bool Calling = (Op&0x1f) == 0x11 || Op == 0x12;
   // These are the operations, *jmp and ret*, which do not go onto the operation addressed immediately after them.
      bool Breaking = (Op&~0x10) == 0x22 || (Op&0x1f) == 0x01 || Op == 0x02 || Op == 0x80 || Op == 0x73;
      if (Generating) {
         if (!Breaking) printf("   ");
      } else if (Indirect) printf("// Indirect jump at %04x\n", (unsigned)PC - 1);
      unsigned Rx = 0, Lab;
      for (char *S = Code[Op]; *S != '\0'; S++) switch (*S) {
      // Data register De0 (ACC) is A.
         case 'A': Rand[Rx].Value = 0xe0, Rand[Rx++].Space = 1; break;
      // 8-bit bit register address Bxx in bdata[].
         case 'B':
            if ((Ch = Fetch(false)) == EOF) return; else Rand[Rx].Value = (byte)Ch, Rand[Rx++].Space = 2;
         break;
      // Bit register Bd7 (CY) is C.
         case 'C': Rand[Rx].Value = 0xd7, Rand[Rx++].Space = 2; break;
      // 8-bit data register address Dxx in ddata[].
         case 'D':
            if ((Ch = Fetch(false)) == EOF) return; else Rand[Rx].Value = (byte)Ch, Rand[Rx++].Space = 1;
         break;
      // 8-bit data value xx.
         case 'I':
            if ((Ch = Fetch(false)) == EOF) return; else Rand[Rx].Value = (byte)Ch, Rand[Rx++].Space = 4;
         break;
      // 16-bit code address xxxx in cdata[].
         case 'L':
            if ((Ch = Fetch(false)) == EOF) return; else Lab = (byte)Ch;
            if ((Ch = Fetch(false)) == EOF) return; else Lab = Lab << 8 | (byte)Ch;
         goto AddLabel;
      // 8-bit paged code address xx in cdata[].
         case 'P':
            if ((Ch = Fetch(false)) == EOF) return; else Lab = PC&0xf800 | (Op&0xe0) << 3 | (byte)Ch;
         goto AddLabel;
      // 8-bit code address offset xx in cdata[].
         case 'R':
            if ((Ch = Fetch(false)) == EOF) return; else Lab = PC + (sbyte)Ch;
         AddLabel:
            Rand[Rx].Value = Lab, Rand[Rx++].Space = 0;
            if (!Generating && (Recursing || Calling)) PushAddr(Lab);
         break;
      // 16-bit data value xxxx.
         case 'W':
            if ((Ch = Fetch(false)) == EOF) return; else Lab = (byte)Ch;
            if ((Ch = Fetch(false)) == EOF) return; else Lab = Lab << 8 | (byte)Ch;
            Rand[Rx].Value = Lab, Rand[Rx++].Space = 5;
         break;
      // 16-bit program counter PC.
         case 'Z': Rand[Rx].Value = PC, Rand[Rx++].Space = 3; break;
      // Data register Df0 (ACC) is B.
         case 'b': Rand[Rx].Value = 0xf0, Rand[Rx++].Space = 1; break;
      // The 16-bit DPTR register (aliased as DPTR = DPH:DPL = D83:D82).
         case 'd': Rand[Rx].Value = 0, Rand[Rx++].Space = 8; break;
      // The 1-bit address (i = 0, 1) for the 8-bit register pointer @Ri into idata[].
         case 'i': Rand[Rx].Value = Op&1, Rand[Rx++].Space = 7; break;
      // The 3-bit address (n = 0,1,2,3,4,5,6,7) for register Rn in rdata[].
         case 'n': Rand[Rx].Value = Op&7, Rand[Rx++].Space = 6; break;
      // The 1-bit address (i = 0, 1) for the 16-bit register pointer P2:@Ri into xdata[].
         case 'p': Rand[Rx].Value = Op&1, Rand[Rx++].Space = 9; break;
      }
      if (Generating) {
         for (char *S = Decompile(Op); *S != '\0'; S++) {
            if (*S != '%') { putchar(*S); continue; }
            int Ch;
            switch (*++S) {
               case '1': case '2': case '3': case '4': Ch = *S - '1'; break;
               default: Error(true, "Bad format string, PC = %04x.", PC); continue;
            }
            switch (Rand[Ch].Space) {
               case 0: PutLabel(Rand[Ch].Value); break;
               case 1: PutDReg(Rand[Ch].Value); break;
               case 2: PutBReg(Rand[Ch].Value); break;
               case 3: PutXData(Rand[Ch].Value); break;
               case 4: PutByte(Rand[Ch].Value); break;
               case 5: PutWord(Rand[Ch].Value); break;
               case 6: printf("R%1x", Rand[Ch].Value); break;
               case 7: printf("*R%1x", Rand[Ch].Value); break;
               case 8: printf("DPTR"); break;
               case 9: printf("P2:R%1x", Rand[Ch].Value); break;
            }
         }
         putchar('\n');
      }
      if (Breaking) break;
   }
}

void PutData(byte *Buf, size_t N, size_t Max) {
   if (NoData) return;
   for (size_t n = 0; n < N; n++) {
      if (n == 0) printf("// db "); else putchar(',');
      unsigned Ch = Buf[n];
      printf("0x%02x", Ch);
   }
   for (size_t n = N; n < Max; n++) printf("     ");
   printf(" ;; %04x:", PC - N);
   for (size_t n = 0; n < N; n++) {
      int Ch = Buf[n];
      putchar(isprint(Ch)? Ch: ' ');
   }
   putchar('\n');
}

Symbol BList = NULL, CList = NULL, DList = NULL;

bool Configure(char *Path) {
   for (int D = 0; D < Ds; D++) LookUp(1, GetSym(1, DTab[D].Name), DTab[D].Key);
   for (int B = 0; B < Bs; B++) LookUp(2, GetSym(2, BTab[B].Name), BTab[B].Key);
   EP = Entries, Line = 1;
   FILE *InF = fopen(Path, "r");
   if (InF == NULL) return false;
   int Ch = fgetc(InF);
   char *S; Symbol Sym; int Type; word Key; bool Empty;
   char NBuf[0x20], *NEnd = NBuf + sizeof NBuf/sizeof NBuf[0] - 1;
StartLine:
   while (Ch == ' ' || Ch == '\t') Ch = fgetc(InF);
   switch (Ch) {
      case EOF: fclose(InF); return true;
      case 'B': case 'C': case 'D': case 'E':
      case 'b': case 'c': case 'd': case 'e': Type = tolower(Ch); goto InList;
      case 'X': case 'x': Recursing = false; goto EndLine;
      case 'Y': case 'y': NoData = true; goto EndLine;
      default:
         if (isxdigit(Ch)) { Type = 'e'; goto EntryList; }
      goto EndLine;
   }
InList:
   switch (Ch = fgetc(InF)) {
      case EOF: case '\n': case '/': case ';':
         if (Type == 'e') Error(false, "No entry points listed on an entry points line.");
         else Error(false, "No definitions listed on a definitions line.");
      goto EndLine;
      case '\t': case ' ':
         Empty = true, Ch = fgetc(InF);
         if (Type == 'e') goto EntryList;
      goto Definition;
      default: goto InList;
   }
EntryList:
   switch (Ch) {
      case EOF: case '\n': case '/': case ';':
         if (Empty) Error(false, "No entry points listed on an entry points line.");
      goto EndLine;
      default:
         if (!isxdigit(Ch)) Ch = fgetc(InF);
         else {
            Empty = false;
            for (Key = 0; isxdigit(Ch); Ch = fgetc(InF)) Key = Key << 4 | Nib(Ch);
            PushAddr(Key);
         }
      goto EntryList;
   }
Definition:
   switch (Ch) {
      case EOF: case '\n': case '/': case ';':
         if (Empty) Error(false, "No definitions listed on a definitions line.");
      goto EndLine;
      default:
         if (isalpha(Ch) || Ch == '_') goto GetName;
         Ch = fgetc(InF);
      goto Definition;
   }
GetName:
   for (Empty = false, S = NBuf; isalnum(Ch) || Ch == '_'; Ch = fgetc(InF)) if (S < NEnd) *S++ = Ch;
   *S = '\0';
   switch (Type) {
      case 'b': Sym = GetSym(2, NBuf); break;
      case 'd': Sym = GetSym(1, NBuf); break;
      default: Sym = GetSym(0, NBuf); break;
   }
Define:
   switch (Ch) {
      case EOF: case '\n': case '/': case ';': Error(false, "No address listed for %s.", Sym->Name); goto EndLine;
      default:
         if (isxdigit(Ch)) goto GetValue;
         Ch = fgetc(InF);
      goto Define;
   }
GetValue:
   for (Key = 0; isxdigit(Ch); Ch = fgetc(InF)) Key = Key << 4 | Nib(Ch);
   if (Sym->Defined) Error(false, "%s is already defined.", Sym->Name);
   else {
      Sym->Defined = true, Sym->Key = Key;
      switch (Type) {
         case 'b': LookUp(2, Sym, Key), Sym->CLink = BList, BList = Sym; break;
         case 'd': LookUp(1, Sym, Key), Sym->CLink = DList, DList = Sym; break;
         default: LookUp(0, Sym, Key), Sym->CLink = CList, CList = Sym; break;
      }
   }
goto Definition;
EndLine:
   while (Ch != '\n' && Ch != EOF) Ch = fgetc(InF);
   if (Ch == '\n') Line++, Ch = fgetc(InF);
goto StartLine;
}

int main(void) {
   HexLoad();
   Generating = false; if (Loud) fprintf(stderr, "First pass\n");
   if (!Configure("entries")) Error(false, "No entry points listed, using 0x%04x as the starting address.", LoPC), PushAddr(LoPC);
   while (EP > Entries) PC = *--EP, Disassemble();
   Generating = true; if (Loud) fprintf(stderr, "Second pass\n");
   for (Symbol Sym = BList; Sym != NULL; Sym = Sym->CLink)
      if (Sym->Defined) printf("// %s bit ", Sym->Name), PutByte(Sym->Key), putchar('\n');
   for (Symbol Sym = DList; Sym != NULL; Sym = Sym->CLink)
      if (Sym->Defined) printf("// %s %s ", Sym->Name, Sym->Key >= 0x80? "sfr": "data"), PutByte(Sym->Key), putchar('\n');
   for (Symbol Sym = CList; Sym != NULL; Sym = Sym->CLink)
      if (Sym->Defined) printf("// %s code ", Sym->Name), PutWord(Sym->Key), printf(" [%d]\n", (Ref[Sym->Key] - 1)&0x7f);
   for (PC = 0x0000; PC < HiPC; ) {
      printf("// org "), PutWord(PC), putchar('\n');
      while (Ref[PC] > 0) {
         if (Ref[PC]&0x80) { Disassemble(); continue; }
         printf("// DATA at "), PutWord(PC);
         if (NoData) printf(" (not shown)");
         putchar('\n');
         byte Buf[0x10]; size_t Max = sizeof Buf/sizeof Buf[0], N = 0;
         while (Ref[PC] > 0 && !(Ref[PC]&0x80)) {
            Buf[N] = Hex[PC++];
            if (++N >= Max) PutData(Buf, Max, Max), N = 0;
         }
         if (N > 0) PutData(Buf, N, Max);
      }
      while (Ref[PC] == 0 && PC < HiPC) PC++;
   }
   return EXIT_SUCCESS;
}
