#include <stdio.h>
#include <stdlib.h>

int main(int AC, char *AV[]) {
   char *App = AC > 0? AV[0]: NULL; if (App == NULL || *App == '\0') App = "nl";
   int Status = EXIT_FAILURE;
   FILE *FP; int Ch;
   if (AC < 2) { fprintf(stderr, "Usage: %s Input\n", AV[0]); goto Exit; }
   FP = fopen(AV[1], "rb");
   if (FP == 0) { fprintf(stderr, "Cannot open %s.\n", AV[1]); goto Exit; }
   while ((Ch = fgetc(FP)) != EOF) putchar(Ch);
   Status = EXIT_SUCCESS;
Exit:
   return Status;
}
