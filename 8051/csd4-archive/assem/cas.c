#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "io.h"
#include "op.h"
#include "st.h"
#include "link.h"

int main(int AC, char *AV[]) {
   char *App = AC > 0? AV[0]: NULL; if (App == NULL || *App == '\0') App = "cas";
   int Status = EXIT_FAILURE;
   byte DoHex = 0;
   if (AC < 2) {
      fprintf(stderr, "Usage: %s -c File... to assemble\n", App);
      fprintf(stderr, "Usage: %s [-o Output] File... to link\n", App);
      goto Exit;
   }
   char **AP = AV, *Hex = NULL; byte DoLink = 1;
   Fs = AC;
   if (AV[1][0] != '-') Fs--, AP++;
   else if (strcmp(AV[1], "-c") == 0) {
      if (AC < 3) {
         fprintf(stderr, "Usage: %s -c File ... to assemble\n", App);
         goto Exit;
      }
      DoLink = 0, Fs -= 2, AP += 2;
   } else if (strcmp(AV[1], "-o") == 0) {
      if (AC < 4) {
         fprintf(stderr, "Usage: %s -o Output File... to link\n", App);
         goto Exit;
      }
      DoHex = 1, Hex = CopyS(AV[2]), Fs -= 3, AP += 3;
   } else {
      fprintf(stderr, "Invalid option: %s\n", AV[1]); goto Exit;
   }
   if (DoLink) FTab = Allocate(Fs*sizeof *FTab); else FTab = NULL;
   OpInit();
   for (int A = 0; A < Fs; A++) {
      char *Src = AP[A], *S = Src + strlen(Src) - 1;
      for (; S > Src; S--)
         if (*S == '.') break;
      char *Obj;
      if (strcmp(S, ".o") == 0) Obj = CopyS(Src), Src = 0;
      else {
         char Ch;
         if (S > Src) Ch = *S, *S = '\0';
         Obj = Allocate(strlen(Src) + 3);
         sprintf(Obj, "%s.o", Src);
         if (S > Src) *S = Ch;
      }
      if (DoLink) FTab[A].Name = Obj;
      if (Src != NULL) {
         fprintf(stderr, "assembling %s -> %s\n", Src, Obj);
         OutF = OpenObj(Obj);
         if (OutF == NULL) {
            fprintf(stderr, "Cannot open object file for %s.\n", Src); goto Exit;
         }
         Assemble(Src), Generate();
      }
   }
   Check();
   Status = EXIT_SUCCESS;
   if (!DoLink) goto Exit;
   if (!DoHex) {
      char *Obj = FTab[0].Name;
      word L = strlen(Obj) + 3;
      char *S = Obj + strlen(Obj) - 1;
      for (; S > Obj; S--)
         if (*S == '.') break;
      char Ch;
      if (S > Obj) Ch = *S, *S = '\0';
      Hex = Allocate(strlen(Obj) + 5);
      sprintf(Hex, "%s.hex", Obj);
      if (S > Obj) *S = Ch;
   }
   fprintf(stderr, "linking");
   for (int A = 0; A < Fs; A++)
      fputc(' ', stderr), fprintf(stderr, FTab[A].Name);
   fprintf(stderr, " -> %s\n", Hex);
   Link(Hex);
Exit:
   return Status;
}
