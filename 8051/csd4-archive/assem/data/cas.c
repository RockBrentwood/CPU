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
   int A; char **AP, *Hex; byte DoHex = 0, DoLink = 1;
   if (AC < 2) {
      fprintf(stderr, "Usage: %s -c File... to assemble\n", App);
      fprintf(stderr, "Usage: %s [-o Output] File... to link\n", App);
      goto Exit;
   }
   if (AV[1][0] == '-') {
      if (strcmp(AV[1], "-c") == 0) {
         if (AC < 3) {
            fprintf(stderr, "Usage: %s -c File ... to assemble\n", App);
            goto Exit;
         }
         DoLink = 0, Fs = AC - 2, AP = AV + 2;
      } else if (strcmp(AV[1], "-o") == 0) {
         if (AC < 4) {
            fprintf(stderr, "Usage: %s -o Output File... to link\n", App);
            goto Exit;
         }
         DoHex = 1, Hex = CopyS(AV[2]), Fs = AC - 3, AP = AV + 3;
      } else {
         fprintf(stderr, "Invalid option: %s\n", AV[1]); goto Exit;
      }
   } else Fs = AC - 1, AP = AV + 1;
   if (DoLink) FTab = (FileBuf)Allocate(Fs * sizeof *FTab); else FTab = 0;
   OpInit();
   for (A = 0; A < Fs; A++) {
      char *S, *Src, *Obj, Ch;
      Src = AP[A];
      for (S = Src + strlen(Src) - 1; S > Src; S--)
         if (*S == '.') break;
      if (strcmp(S, ".o") == 0) Obj = CopyS(Src), Src = 0;
      else {
         if (S > Src) Ch = *S, *S = '\0';
         Obj = (char *)Allocate(strlen(Src) + 3);
         sprintf(Obj, "%s.o", Src);
         if (S > Src) *S = Ch;
      }
      if (DoLink) FTab[A].Name = Obj;
      if (Src != 0) {
         fprintf(stderr, "assembling %s -> %s\n", Src, Obj);
         OutF = OpenObj(Obj);
         if (OutF == 0) {
            fprintf(stderr, "Cannot open object file for %s.\n", Src); goto Exit;
         }
         Assemble(Src), Generate();
      }
   }
   CHECK();
   if (DoLink) {
      if (!DoHex) {
         char *S, *Obj = FTab[0].Name, Ch;
         word L = strlen(Obj) + 3;
         for (S = Obj + strlen(Obj) - 1; S > Obj; S--)
            if (*S == '.') break;
         if (S > Obj) Ch = *S, *S = '\0';
         Hex = (char *)Allocate(strlen(Obj) + 5);
         sprintf(Hex, "%s.hex", Obj);
         if (S > Obj) *S = Ch;
      }
      fprintf(stderr, "linking");
      for (A = 0; A < Fs; A++)
         fputc(' ', stderr), fprintf(stderr, FTab[A].Name);
      fprintf(stderr, " -> %s\n", Hex);
      Link(Hex);
   }
   Status = EXIT_SUCCESS;
Exit:
   return Status;
}
