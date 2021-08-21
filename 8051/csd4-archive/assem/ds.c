#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef unsigned short word;

void Fatal(char *Msg) { printf(Msg), putchar('\n'); exit(EXIT_FAILURE); }

byte GetB(FILE *FP) {
   int A= fgetc(FP); if (A == EOF) Fatal("Unexpected EOF.");
   return A&0xff;
}

word GetW(FILE *FP) {
   int A = fgetc(FP); if (A == EOF) Fatal("Unexpected EOF.");
   int B = fgetc(FP); if (B == EOF) Fatal("Unexpected EOF.");
   return (A&0xff) << 8 | B&0xff;
}

unsigned long GetL(FILE *FP) {
   int A = fgetc(FP); if (A == EOF) Fatal("Unexpected EOF.");
   int B = fgetc(FP); if (B == EOF) Fatal("Unexpected EOF.");
   int C = fgetc(FP); if (C == EOF) Fatal("Unexpected EOF.");
   int D = fgetc(FP); if (D == EOF) Fatal("Unexpected EOF.");
   return (A&0xff) << 24 | (B&0xff) << 16 | (C&0xff) << 8 | D&0xff;
}

int main(int AC, char *AV[]) {
   char *App = AC > 0? AV[0]: NULL; if (App == NULL || *App == '\0') App = "ds";
   int Status = EXIT_FAILURE;
   if (AC != 2) { fprintf(stderr, "Usage: %s Input.\n", App); goto Exit0; }
   FILE *FP = fopen(AV[1], "rb+");
   if (FP == NULL) { fprintf(stderr, "Cannot open %s.\n", AV[1]); goto Exit0; }
   word Signature = GetW(FP);
   if (Signature != 0x55aa) {
      fprintf(stderr, "Invalid object file (%4x).\n", Signature); goto Exit1;
   }
   long SegLoc = GetL(FP), Files = GetL(FP), Segs = GetL(FP), Gaps = GetL(FP);
   long Syms = GetL(FP), Exps = GetL(FP), Relocs = GetL(FP), Sum = GetL(FP);
   if (Sum != (SegLoc + Files + Segs + Gaps + Syms + Exps + Relocs)&0xffffffff) {
      fprintf(stderr, "Corrupt object file."); goto Exit1;
   }
   printf("Image:\n");
   unsigned Field = 0;
   for (; ftell(FP) < SegLoc; Field++) {
      if (Field%0x10 == 0) printf("%4x:", Field);
      printf(" %2x", (unsigned)fgetc(FP));
      if (Field%0x10 == 0x0f) putchar('\n');
   }
   if (Field%0x10 > 0) putchar('\n');
   if (ftell(FP) != SegLoc) { fprintf(stderr, "Internal Error (0).\n"); goto Exit1; }
   printf("File(s): %ld\n", Files);
   if (Files > 0) {
      printf("## Name,\n");
      for (long S = 0; S < Files; S++) {
         char Buf[0x100]; word L = GetW(FP);
         fread(Buf, 1, L, FP), Buf[L] = '\0';
         printf("%2ld %15s\n", S, Buf);
      }
   }
   printf("Segment(s): %ld\n", Segs);
   if (Segs > 5) { /* The number of types */
      printf("## Line File Rel Type Base Size  Loc\n");
      for (long S = 5; S < Segs; S++) {
         word Line = GetW(FP), File = GetW(FP);
         word U = GetW(FP), Size = GetW(FP), Base = GetW(FP); long Loc = GetL(FP);
         word Rel = (U >> 8)&1, Type = U&0xff;
         printf("%2ld %4d %4d  %c %4x %4x %4x %8lx\n",
            S, Line, File, Rel? 'r': ' ', Type, Base, Size, Loc
         );
      }
   }
   printf("Gap(s): %ld\n", Gaps);
   if (Gaps > 0) {
      printf("##  Seg  Off Size\n");
      for (long S = 0; S < Gaps; S++) {
         word Seg = GetW(FP), Off = GetW(FP), Size = GetW(FP);
         printf("%2ld %4x %4x %4x\n", S, Seg, Off, Size);
      }
   }
   printf("Symbol(s): %ld\n", Syms);
   if (Syms > 0) {
      printf("##     Scope Var Type   Value  Name\n");
      for (unsigned long S = 0; S < Syms; S++) {
         byte B = GetB(FP); word U = GetW(FP), Offset = GetW(FP), L = GetW(FP);
         char Buf[0x80];
         if (L > 0) fread(&Buf, 1, L, FP);
         Buf[L] = '\0';
         printf("%2lx ", S);
         switch (B&0xc) {
            case 0x0: printf("undefined"); break;
            case 0x4: printf("    local"); break;
            case 0x8: printf(" external"); break;
            case 0xc: printf("   global"); break;
         }
         printf("  %c  ", B&1? 'v': ' ');
         if (B&2) { /* Address */
            printf("ADDR  %2d:%4x", U, Offset);
         } else {
            printf(" NUM  %4x   ", Offset);
         }
         printf(" %s\n", Buf);
      }
   }
   printf("Expression(s): %ld\n", Exps);
   if (Exps > 0) {
      printf("## Line File  Tag Args...\n");
      for (unsigned long S = 0; S < Exps; S++) {
         word Line = GetW(FP), File = GetW(FP); char Tag = GetB(FP);
         printf("%2lx %4d %4d ", S, Line, File);
         switch (Tag) {
            case 0: printf(" NUM %4x", GetW(FP)); break;
            case 1: {
               word Seg = GetW(FP), Offset = GetW(FP);
               printf("ADDR %2d %4x", Seg, Offset);
            }
            break;
            case 2: printf(" SYM %2x", GetW(FP)); break;
            case 3: {
               char Op = GetB(FP); word A = GetW(FP);
               printf("  UN %2d %4x", Op, A);
            }
            break;
            case 4: {
               char Op = GetB(FP); word A = GetW(FP), B = GetW(FP);
               printf(" BIN %2d %4x %4x", Op, A, B);
            }
            break;
            case 5: {
               word A = GetW(FP), B = GetW(FP), C = GetW(FP);
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
      for (long S = 0; S < Relocs; S++) {
         word Line = GetW(FP), File = GetW(FP); char Tag = GetB(FP);
         word Index = GetW(FP), U = GetW(FP), Offset = GetW(FP);
         printf("%4d %4d  %c %4x  @ %2d:%4x\n", Line, File, Tag, Index, U, Offset);
      }
   }
   Status = EXIT_SUCCESS;
Exit1:
   fclose(FP);
Exit0:
   return Status;
}
