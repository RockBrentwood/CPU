#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char byte;
typedef unsigned int word;

byte Nib(int X) {
   if (X >= '0' && X <= '9') return X - '0';
   else if (X >= 'a' && X <= 'f') return X - 'a' + 0xa;
   else if (X >= 'A' && X <= 'F') return X - 'A' + 0xA;
   else fprintf(stderr, "Bad hexadecimal digit in input\n"), exit(EXIT_FAILURE);
}

byte CheckSum;
byte GetHex(FILE *InF) {
   int A = fgetc(InF), B = fgetc(InF);
   if (A == EOF || B == EOF)
      fprintf(stderr, "Unexpected EOF.\n"), exit(EXIT_FAILURE);
   byte Bt = Nib(A) << 4 | Nib(B);
   CheckSum = (CheckSum + Bt)&0xff; return Bt;
}

word GetWord(FILE *InF) {
   word A = GetHex(InF), B = GetHex(InF);
   return (A << 8) | B;
}

void PutHex(FILE *ExF, byte Ch) {
   fprintf(ExF, "%02x", Ch), CheckSum = (CheckSum + Ch)&0xff;
}

void PutWord(FILE *ExF, word W) {
   PutHex(ExF, (byte)(W >> 8)), PutHex(ExF, (byte)(W&0xff));
}

static bool Relocate(word Offset, FILE *InF, FILE *ExF) {
   byte Mark;
   do {
      int Ch;
      do
         if ((Ch = fgetc(InF)) == EOF) { fprintf(stderr, "Unexpected EOF.\n"); return false; }
      while (Ch != ':');
      CheckSum = 0;
      byte Size = GetHex(InF); word Addr = GetWord(InF) + Offset; Mark = GetHex(InF);
      byte Buffer[0x10];
      for (int I = 0; I < Size; I++) Buffer[I] = GetHex(InF);
      GetHex(InF), fgetc(InF);
      if (CheckSum != 0) {
         fprintf(stderr, "Bad checksum.\n"); return false;
      }
      fputc(':', ExF);
      CheckSum = 0;
      PutHex(ExF, Size), PutWord(ExF, Addr), PutHex(ExF, Mark);
      for (int I = 0; I < Size; I++) PutHex(ExF, Buffer[I]);
      PutHex(ExF, (byte)-CheckSum);
      fputc('\n', ExF);
   } while (Mark == 0);
   return true;
}

word atoh(char *S) {
   word Val = 0;
   for (; *S != '\0'; S++)
      if (isdigit(*S)) Val = (Val << 4) | (*S - '0');
      else if (isxdigit(*S)) Val = (Val << 4) | (tolower(*S) - 'a' + 0xa);
      else return 0;
   return Val;
}

char *Convert(char *Path) {
   char *S = strdup(Path), *T = S + strlen(S);
   for (; T > S; T--)
      if (T[-1] == '.') break;
   if (T == S) { free(S); return NULL; }
   *T++ = 'h', *T++ = 'x', *T = '\0';
   return S;
}

int main(int AC, char *AV[]) {
   char *App = AC > 0? AV[0]: NULL; if (App == NULL || *App == '\0') App = "reloc";
   int Status = EXIT_FAILURE;
   if (AC != 3) { printf("Usage: %s Offset Input.\n", App); goto Exit0; }
   word Offset = atoh(AV[1]);
   if (Offset == 0) printf("Zero offset/bad offset specified.\n");
   char *ExName = Convert(AV[2]);
   if (ExName == NULL) { printf("%s not of form *.hex.\n", AV[2]); goto Exit0; }
   FILE *InF = fopen(AV[2], "r");
   if (InF == NULL) { printf("Cannot open %s.\n", AV[2]); goto Exit1; }
   FILE *ExF = fopen(ExName, "w");
   if (ExF == NULL) { printf("Cannot open %s.\n", ExName); goto Exit2; }
   if (!Relocate(Offset, InF, ExF)) goto Exit3;
   Status = EXIT_SUCCESS;
Exit3:
   fclose(ExF);
Exit2:
   fclose(InF);
Exit1:
   free(ExName);
Exit0:
   return Status;
}
