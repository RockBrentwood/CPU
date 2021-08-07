#include <stdio.h>
#include <ctype.h>

typedef unsigned char byte;
typedef unsigned short word;

byte Table[0x100];

/* This is the polynomial long multiplication algorithm
   (Mult(Poly, B) = B*Poly), but with coefficients added modulo 2.
   The resulting 16-bit word corresponding to Mult(B) is in the form f(B):g(B)
   where f(B) is the upper 8 bits and g(B) the lower 8 bits.  The table is
   formed as the composition: Table[I] = g(f(I)).
 */
word Mult(word Poly, byte B) {
   word A, C;
   for (C = 0, A = Poly; B != 0; B >>= 1, A <<= 1)
      if (B&1) C ^= A;
   return C;
}

void SetTable(word Poly) {
   int I; byte g[0x100], f[0x100];
   for (I = 0; I < 0x100; I++) {
      word W = Mult(Poly, (byte)I);
      f[I] = W >> 8; g[I] = W&0xff;
   }
   for (I = 0; I < 0x100; I++) Table[I] = g[f[I]];
}

byte *Demo = "This is a test input.";

byte GetNib(void) {
   int Ch = getchar();
   if (Ch == EOF) printf("Unexpected end of file.\n"), exit(0);
   if (isdigit(Ch)) return Ch - '0';
   else if (isxdigit(Ch)) return tolower(Ch) - 'a' + 10;
   else printf("Corrupt input.\n"), exit(0);
}

byte CRC;
byte GetHex(void) {
   byte B = GetNib();
   B = B << 4 | GetNib();
   CRC ^= Table[B];        /* Update check sum */
   return B;
}

word GetWord(void) {
   word W = GetHex();
   return W << 8 | GetHex();
}

main(void) { /* Verify CRC checksums on an intel hex format-like file. */
   int Ch; byte Size, Mark; word Addr;
   int Mismatches;
/* 0x169 (hex) = 101101001 (binary) = x^8 + x^6 + x^5 + x^3 + x^0, for x = 2. */
   SetTable(0x169);
   Mismatches = 0;
   do {
      byte C0, C1;
      do {
         Ch = getchar();
         if (Ch == EOF) printf("Unexpected end of file.\n"), exit(0);
      } while (Ch != ':');
      CRC = 0;
      Size = GetHex(), Addr = GetWord(), Mark = GetHex();
      for (; Size > 0; Size--) GetHex();
      C0 = CRC, C1 = GetHex();
      printf("CRC: Calculated -- %02x, Received -- %02x\n", C0, C1);
      if (C0 != C1) Mismatches++;
      (void)getchar();
   } while (!Mark);
   printf("%d mismatched checksum(s)\n", Mismatches);
}
