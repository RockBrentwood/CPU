#include <stdio.h>
main(int AC, char **AV) {
   FILE *FP; int Ch;
   if (AC < 2) fprintf(stderr, "Usage: %s Input\n", AV[0]), exit(1);
   FP = fopen(AV[1], "rb");
   if (FP == 0) fprintf(stderr, "Cannot open %s.\n", AV[1]), exit(1);
   while ((Ch = fgetc(FP)) != EOF) putchar(Ch);
}
