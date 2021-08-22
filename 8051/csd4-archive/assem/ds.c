#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef unsigned short word;

void Fatal(char *Msg) { printf(Msg), putchar('\n'); exit(EXIT_FAILURE); }

byte GetB(FILE *InF) {
   int A = fgetc(InF); if (A == EOF) Fatal("Unexpected EOF.");
   return A&0xff;
}

word GetW(FILE *InF) {
   int A = fgetc(InF); if (A == EOF) Fatal("Unexpected EOF.");
   int B = fgetc(InF); if (B == EOF) Fatal("Unexpected EOF.");
   return (A&0xff) << 8 | B&0xff;
}

unsigned long GetL(FILE *InF) {
   int A = fgetc(InF); if (A == EOF) Fatal("Unexpected EOF.");
   int B = fgetc(InF); if (B == EOF) Fatal("Unexpected EOF.");
   int C = fgetc(InF); if (C == EOF) Fatal("Unexpected EOF.");
   int D = fgetc(InF); if (D == EOF) Fatal("Unexpected EOF.");
   return (A&0xff) << 24 | (B&0xff) << 16 | (C&0xff) << 8 | D&0xff;
}

int main(int AC, char *AV[]) {
   char *App = AC > 0? AV[0]: NULL; if (App == NULL || *App == '\0') App = "ds";
   int Status = EXIT_FAILURE;
   if (AC != 2) { fprintf(stderr, "Usage: %s Input.\n", App); goto Exit0; }
   FILE *InF = fopen(AV[1], "rb+");
   if (InF == NULL) { fprintf(stderr, "Cannot open %s.\n", AV[1]); goto Exit0; }
   word Signature = GetW(InF);
   if (Signature != 0x55aa) {
      fprintf(stderr, "Invalid object file (%4x).\n", Signature); goto Exit1;
   }
   long SegLoc = GetL(InF), Files = GetL(InF), Segs = GetL(InF), Gaps = GetL(InF);
   long Syms = GetL(InF), Exps = GetL(InF), Relocs = GetL(InF), Sum = GetL(InF);
   if (Sum != (SegLoc + Files + Segs + Gaps + Syms + Exps + Relocs)&0xffffffff) {
      fprintf(stderr, "Corrupt object file."); goto Exit1;
   }
   printf("Image:\n");
   unsigned Field = 0;
   for (; ftell(InF) < SegLoc; Field++) {
      if (Field%0x10 == 0) printf("%4x:", Field);
      printf(" %2x", (unsigned)fgetc(InF));
      if (Field%0x10 == 0x0f) putchar('\n');
   }
   if (Field%0x10 > 0) putchar('\n');
   if (ftell(InF) != SegLoc) { fprintf(stderr, "Internal Error (0).\n"); goto Exit1; }
   printf("File(s): %ld\n", Files);
   if (Files > 0) {
      printf("## Name,\n");
      for (long F = 0; F < Files; F++) {
         char Buf[0x100]; word L = GetW(InF);
         fread(Buf, 1, L, InF), Buf[L] = '\0';
         printf("%2ld %15s\n", F, Buf);
      }
   }
   printf("Segment(s): %ld\n", Segs);
   if (Segs > 5) { // The number of types.
      printf("## Line File Rel Type Base Size  Loc\n");
      for (long S = 5; S < Segs; S++) {
         word Line = GetW(InF), File = GetW(InF);
         word U = GetW(InF), Size = GetW(InF), Base = GetW(InF); unsigned long Loc = GetL(InF);
         word Rel = (U >> 8)&1, Type = U&0xff;
         printf("%2ld %4d %4d  %c %4x %4x %4x %8lx\n", S, Line, File, Rel? 'r': ' ', Type, Base, Size, Loc);
      }
   }
   printf("Gap(s): %ld\n", Gaps);
   if (Gaps > 0) {
      printf("##  Seg  Off Size\n");
      for (long G = 0; G < Gaps; G++) {
         word Seg = GetW(InF), Off = GetW(InF), Size = GetW(InF);
         printf("%2ld %4x %4x %4x\n", G, Seg, Off, Size);
      }
   }
   printf("Symbol(s): %ld\n", Syms);
   if (Syms > 0) {
      printf("##     Scope Var Type   Value  Name\n");
      for (unsigned long S = 0; S < Syms; S++) {
         byte B = GetB(InF); word U = GetW(InF), Offset = GetW(InF), L = GetW(InF);
         char Buf[0x80];
         if (L > 0) fread(&Buf, 1, L, InF);
         Buf[L] = '\0';
         printf("%2lx ", S);
         switch (B&0xc) {
            case 0x0: printf("undefined"); break;
            case 0x4: printf("    local"); break;
            case 0x8: printf(" external"); break;
            case 0xc: printf("   global"); break;
         }
         printf("  %c  ", B&1? 'v': ' ');
         if (B&2) // Address.
            printf("ADDR  %2d:%4x", U, Offset);
         else
            printf(" NUM  %4x   ", Offset);
         printf(" %s\n", Buf);
      }
   }
   printf("Expression(s): %ld\n", Exps);
   if (Exps > 0) {
      printf("## Line File  Tag Args...\n");
      for (unsigned long E = 0; E < Exps; E++) {
         word Line = GetW(InF), File = GetW(InF); char Tag = GetB(InF);
         printf("%2lx %4d %4d ", E, Line, File);
         switch (Tag) {
            case 0: printf(" NUM %4x", GetW(InF)); break;
            case 1: {
               word Seg = GetW(InF), Offset = GetW(InF);
               printf("ADDR %2d %4x", Seg, Offset);
            }
            break;
            case 2: printf(" SYM %2x", GetW(InF)); break;
            case 3: {
               char Op = GetB(InF); word A = GetW(InF);
               printf("  UN %2d %4x", Op, A);
            }
            break;
            case 4: {
               char Op = GetB(InF); word A = GetW(InF), B = GetW(InF);
               printf(" BIN %2d %4x %4x", Op, A, B);
            }
            break;
            case 5: {
               word A = GetW(InF), B = GetW(InF), C = GetW(InF);
               printf("COND %4x %4x %4x", A, B, C);
            }
            break;
         }
         putchar('\n');
      }
   }
   printf("Relocation(s): %ld\n", Relocs);
   if (Relocs > 0) {
      printf("Line File Tag  Exp  Seg: Off\n");
      for (long R = 0; R < Relocs; R++) {
         word Line = GetW(InF), File = GetW(InF); char Tag = GetB(InF);
         word Index = GetW(InF), U = GetW(InF), Offset = GetW(InF);
         printf("%4d %4d  %c %4x  @ %2d:%4x\n", Line, File, Tag, Index, U, Offset);
      }
   }
   Status = EXIT_SUCCESS;
Exit1:
   fclose(InF);
Exit0:
   return Status;
}
