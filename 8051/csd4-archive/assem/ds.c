#include <stdio.h>
typedef unsigned char byte;
typedef unsigned short word;

void FATAL(char *Msg) { printf(Msg), putchar('\n'); exit(1); }

byte GetB(FILE *FP) {
   int A;
   A = fgetc(FP); if (A == EOF) FATAL("Unexpected EOF.");
   return A&0xff;
}

word GetW(FILE *FP) {
   int A, B;
   A = fgetc(FP); if (A == EOF) FATAL("Unexpected EOF.");
   B = fgetc(FP); if (B == EOF) FATAL("Unexpected EOF.");
   return (A&0xff) << 8 | B&0xff;
}

unsigned long GetL(FILE *FP) {
   int A, B, C, D;
   A = fgetc(FP); if (A == EOF) FATAL("Unexpected EOF.");
   B = fgetc(FP); if (B == EOF) FATAL("Unexpected EOF.");
   C = fgetc(FP); if (C == EOF) FATAL("Unexpected EOF.");
   D = fgetc(FP); if (D == EOF) FATAL("Unexpected EOF.");
   return (A&0xff) << 24 | (B&0xff) << 16 | (C&0xff) << 8 | D&0xff;
}

main(int AC, char **AV) {
   FILE *FP; int Field; word MAGIC;
   long SegLoc, Files, Segs, Gaps, Syms, Exps, Relocs, Sum;
   if (AC != 2) fprintf(stderr, "Use; display Input.\n"), exit(1);
   FP = fopen(AV[1], "rb+");
   if (FP == 0) fprintf(stderr, "Cannot open %s.\n", AV[1]), exit(1);
   MAGIC = GetW(FP);
   if (MAGIC != 0x55aa)
      fprintf(stderr, "Invalid object file (%4x).\n", MAGIC), exit(1);
   SegLoc = GetL(FP), Files = GetL(FP), Segs = GetL(FP), Gaps = GetL(FP),
   Syms = GetL(FP), Exps = GetL(FP), Relocs = GetL(FP), Sum = GetL(FP);
   if (Sum != (SegLoc + Files + Segs + Gaps + Syms + Exps + Relocs)&0xffffffff)
      fprintf(stderr, "Corrupt object file."), exit(0);
printf("IMAGE:\n");
   for (Field = 0; ftell(FP) < SegLoc; Field++) {
      if (Field%0x10 == 0) printf("%4x:", Field);
      printf(" %2x", fgetc(FP));
      if (Field%0x10 == 0x0f) putchar('\n');
   }
   if (Field%0x10 > 0) putchar('\n');
if (ftell(FP) != SegLoc) fprintf(stderr, "INTERNAL ERROR (0).\n"), exit(0);
   printf("FILE(S): %ld\n", Files);
   if (Files > 0) {
      long S; char Buf[0x100];
      printf("## Name,\n");
      for (S = 0; S < Files; S++) {
         word L;
         L = GetW(FP), fread(Buf, 1, L, FP), Buf[L] = '\0';
         printf("%2ld %15s\n", S, Buf);
      }
   }
   printf("SEGMENT(S): %ld\n", Segs);
   if (Segs > 5) { /* The number of types */
      long S;
      printf("## Line File Rel Type Base Size  Loc\n");
      for (S = 5; S < Segs; S++) {
         word U, Rel, Type, Size, Base, Line, File; long Loc;
         Line = GetW(FP), File = GetW(FP);
         U = GetW(FP), Size = GetW(FP), Base = GetW(FP), Loc = GetL(FP);
         Rel = (U >> 8)&1, Type = U&0xff;
         printf("%2ld %4d %4d  %c %4x %4x %4x %8lx\n",
            S, Line, File, Rel? 'r': ' ', Type, Base, Size, Loc
         );
      }
   }
   printf("GAP(S): %ld\n", Gaps);
   if (Gaps > 0) {
      long S;
      printf("##  Seg  Off Size\n");
      for (S = 0; S < Gaps; S++) {
         word Seg, Off, Size;
         Seg = GetW(FP), Off = GetW(FP), Size = GetW(FP);
         printf("%2ld %4x %4x %4x\n", S, Seg, Off, Size);
      }
   }
   printf("SYMBOL(S): %ld\n", Syms);
   if (Syms > 0) {
      long S; char Buf[0x80];
      printf("##     Scope Var Type   Value  Name\n");
      for (S = 0; S < Syms; S++) {
         byte B; word U, L, Offset;
         B = GetB(FP), U = GetW(FP), Offset = GetW(FP), L = GetW(FP);
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
   printf("EXPRESSION(S): %ld\n", Exps);
   if (Exps > 0) {
      long S;
      printf("## Line File  Tag Args...\n");
      for (S = 0; S < Exps; S++) {
         char Tag, Op; word Seg, Line, File, A, B, C, Value, Offset, Sym;
         Line = GetW(FP), File = GetW(FP), Tag = GetB(FP);
         printf("%2lx %4d %4d ", S, Line, File);
         switch (Tag) {
            case 0: Value = GetW(FP); printf(" NUM %4x", Value); break;
            case 1:
               Seg = GetW(FP), Offset = GetW(FP);
               printf("ADDR %2d %4x", Seg, Offset);
            break;
            case 2: Sym = GetW(FP); printf(" SYM %2x", Sym); break;
            case 3:
               Op = GetB(FP), A = GetW(FP);
               printf("  UN %2d %4x", Op, A);
            break;
            case 4:
               Op = GetB(FP), A = GetW(FP), B = GetW(FP);
               printf(" BIN %2d %4x %4x", Op, A, B);
            break;
            case 5:
               A = GetW(FP), B = GetW(FP), C = GetW(FP);
               printf("COND %4x %4x %4x", A, B, C);
            break;
         }
         putchar('\n');
      }
   }
   printf("RELOCATION(S): %ld\n", Relocs);
   if (Relocs > 0) {
      long S;
      printf("Line File Tag  Exp  Seg: Off\n");
      for (S = 0; S < Relocs; S++) {
         word U, Line, File, Index, Offset; char Tag;
         Line = GetW(FP), File = GetW(FP),
         Tag = GetB(FP), Index = GetW(FP), U = GetW(FP), Offset = GetW(FP);
         printf("%4d %4d  %c %4x  @ %2d:%4x\n", Line, File, Tag, Index, U, Offset);
      }
   }
   fclose(FP);
}
