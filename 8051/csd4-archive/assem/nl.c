#include <stdio.h>
#include <stdlib.h>

int main(int AC, char *AV[]) {
   char *App = AC > 0? AV[0]: NULL; if (App == NULL || *App == '\0') App = "nl";
   int Status = EXIT_FAILURE;
   if (AC < 2) { fprintf(stderr, "Usage: %s Input\n", AV[0]); goto Exit; }
   FILE *InF = fopen(AV[1], "rb");
   if (InF == NULL) { fprintf(stderr, "Cannot open %s.\n", AV[1]); goto Exit; }
   int Ch;
   while ((Ch = fgetc(InF)) != EOF) putchar(Ch);
   Status = EXIT_SUCCESS;
Exit:
   return Status;
}
