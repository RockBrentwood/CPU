#include <stdio.h>
#include <ctype.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned int word;

byte Nib(int X) {
   if (X >= '0' && X <= '9') return X - '0';
   if (X >= 'a' && X <= 'f') return X - 'a' + 10;
   if (X >= 'A' && X <= 'F') return X - 'A' + 10;
   fprintf(stderr, "Bad hexadecimal digit in input\n");
   exit(1);
}

byte CheckSum;
byte GetHex(FILE *InF) {
   int A, B; byte Bt;
   A = fgetc(InF); B = fgetc(InF);
   if (A == EOF || B == EOF) {
      fprintf(stderr, "Unexpected EOF.\n"); exit(1);
   }
   Bt = Nib(A) << 4 | Nib(B);
   CheckSum = (CheckSum + Bt)&0xff; return Bt;
}

word GetWord(FILE *InF) {
   word A, B;
   A = GetHex(InF); B = GetHex(InF);
   return (A << 8) | B;
}

void PutHex(FILE *OutF, byte Ch) {
   fprintf(OutF, "%02x", Ch); CheckSum = (CheckSum + Ch)&0xff;
}

void PutWord(FILE *OutF, word W) {
   PutHex(OutF, (byte)(W >> 8)); PutHex(OutF, (byte)(W&0xff));
}

void Relocate(word Offset, FILE *InF, FILE *OutF) {
   int Ch, I;
   byte Size, Mark; word Addr;
   byte Buffer[0x10];
   while (1) {
      do {
         Ch = fgetc(InF);
         if (Ch == EOF) { fprintf(stderr, "Unexpected EOF.\n"); exit(1); }
      } while (Ch != ':');
      CheckSum = 0;
      Size = GetHex(InF); Addr = GetWord(InF) + Offset; Mark = GetHex(InF);
      for (I = 0; I < Size; I++) Buffer[I] = GetHex(InF);
      (void)GetHex(InF); (void)fgetc(InF);
      if (CheckSum != 0) {
         fprintf(stderr, "Bad checksum.\n"); exit(1);
      }
      fputc(':', OutF);
      CheckSum = 0;
      PutHex(OutF, Size); PutWord(OutF, Addr); PutHex(OutF, Mark);
      for (I = 0; I < Size; I++) PutHex(OutF, Buffer[I]);
      PutHex(OutF, (byte)-CheckSum);
      fputc('\n', OutF);
      if (Mark) break;
   }
}

word atoh(char *S) {
   word Val;
   for (Val = 0; *S != '\0'; S++) {
      if (isdigit(*S)) Val = (Val << 4) | (*S - '0');
      else if (isxdigit(*S)) Val = (Val << 4) | (tolower(*S) - 'a' + 10);
      else return 0;
   }
   return Val;
}

char *Convert(char *Path) {
   static char *S; char *T;
   S = (char *)malloc(strlen(Path) + 1);
   if (S == 0) return 0;
   strcpy(S, Path);
   for (T = S + strlen(S); T > S; T--)
      if (T[-1] == '.') break;
   if (T == S) { free(S); return 0; }
   *T++ = 'h', *T++ = 'x', *T = '\0';
   return S;
}

main(int ArgC, char **ArgV) {
   word Offset; FILE *InF, *OutF; char *OutName;
   if (ArgC != 3) { printf("Usage: relocate Offset Input.\n"); exit(0); }
   Offset = atoh(ArgV[1]);
   if (Offset == 0) printf("Zero offset/bad offset specified.\n");
   OutName = Convert(ArgV[2]);
   if (OutName == 0) { printf("%s not of form *.hex.\n", ArgV[2]); exit(0); }
   InF = fopen(ArgV[2], "r");
   if (InF == 0) { printf("Cannot open %s.\n", ArgV[2]); exit(0); }
   OutF = fopen(OutName, "w");
   if (OutF == 0) { printf("Cannot open %s.\n", OutName); exit(0); }
   Relocate(Offset, InF, OutF);
   fclose(InF), fclose(OutF);
}
