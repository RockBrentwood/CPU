/* asmfnums.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Datei-Nummern                                              */
/*                                                                           */
/* Historie: 15. 5.96 Grundsteinlegung                                       */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"

#include "asmfnums.h"

typedef struct _TToken {
   struct _TToken *Next;
   char *Name;
} TToken, *PToken;

static PToken FirstFile;

void InitFileList(void) {
   FirstFile = NULL;
}

void ClearFileList(void) {
   PToken F;

   while (FirstFile != NULL) {
      F = FirstFile->Next;
      free(FirstFile->Name);
      free(FirstFile);
      FirstFile = F;
   }
}

void AddFile(char *FName) {
   PToken Lauf, Neu;

   if (GetFileNum(FName) != -1) return;

   Neu = (PToken) malloc(sizeof(TToken));
   Neu->Next = NULL;
   Neu->Name = strdup(FName);
   if (FirstFile == NULL) FirstFile = Neu;
   else {
      Lauf = FirstFile;
      while (Lauf->Next != NULL) Lauf = Lauf->Next;
      Lauf->Next = Neu;
   }
}

Integer GetFileNum(char *Name) {
   PToken FLauf = FirstFile;
   Integer Cnt = 0;

   while ((FLauf != NULL) && (strcmp(FLauf->Name, Name) != 0)) {
      Cnt++;
      FLauf = FLauf->Next;
   }
   return (FLauf == NULL) ? (-1) : (Cnt);
}

char *GetFileName(Byte Num) {
   PToken Lauf;
   Integer z;
   static char *Dummy = "";

   Lauf = FirstFile;
   for (z = 0; z < Num; z++)
      if (Lauf != NULL) Lauf = Lauf->Next;
   return (Lauf == NULL) ? (Dummy) : (Lauf->Name);
}

Integer GetFileCount(void) {
   PToken Lauf = FirstFile;
   Integer z = 0;

   while (Lauf != NULL) {
      z++;
      Lauf = Lauf->Next;
   };
   return z;
}

void asmfnums_init(void) {
   FirstFile = NULL;
}
