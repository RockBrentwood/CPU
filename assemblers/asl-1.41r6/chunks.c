// AS-Portierung
// Verwaltung von Adressbereichslisten
#include "stdinc.h"

#include "stringutil.h"

#include "chunks.h"

/*--------------------------------------------------------------------------*/
/* eine Chunkliste initialisieren */

void InitChunk(ChunkList * NChunk) {
   NChunk->RealLen = 0;
   NChunk->AllocLen = 0;
   NChunk->Chunks = NULL;
}

void ClearChunk(ChunkList * NChunk) {
   if (NChunk->AllocLen > 0) free(NChunk->Chunks);
   InitChunk(NChunk);
}

/*--------------------------------------------------------------------------*/
/* eine Chunkliste um einen Eintrag erweitern */

static bool Overlap(LargeWord Start1, LargeWord Len1, LargeWord Start2, LargeWord Len2) {
   return ((Start1 == Start2)
      || ((Start2 > Start1) && (Start1 + Len1 >= Start2))
      || ((Start1 > Start2) && (Start2 + Len2 >= Start1)));
}

static void SetChunk(OneChunk * NChunk, LargeWord Start1, LargeWord Len1, LargeWord Start2, LargeWord Len2) {
   NChunk->Start = min(Start1, Start2);
   NChunk->Length = max(Start1 + Len1 - 1, Start2 + Len2 - 1) - NChunk->Start + 1;
}

static void IncChunk(ChunkList * NChunk) {
   if (NChunk->RealLen + 1 > NChunk->AllocLen) {
      if (NChunk->RealLen == 0)
         NChunk->Chunks = (OneChunk *) malloc(sizeof(OneChunk));
      else
         NChunk->Chunks = (OneChunk *) realloc(NChunk->Chunks, sizeof(OneChunk) * (NChunk->RealLen + 1));
      NChunk->AllocLen = NChunk->RealLen + 1;
   }
}

bool AddChunk(ChunkList * NChunk, LargeWord NewStart, LargeWord NewLen, bool Warn) {
   Word z, f1 = 0, f2 = 0;
   bool Found;
   LongInt PartSum;
   bool Result;

   Result = false;

   if (NewLen == 0) return Result;

/* herausfinden, ob sich das neue Teil irgendwo mitanhaengen laesst */

   Found = false;
   for (z = 0; z < NChunk->RealLen; z++)
      if (Overlap(NewStart, NewLen, NChunk->Chunks[z].Start, NChunk->Chunks[z].Length)) {
         Found = true;
         f1 = z;
         break;
      }

/* Fall 1: etwas gefunden : */

   if (Found) {
   /* gefundene Chunk erweitern */

      PartSum = NChunk->Chunks[f1].Length + NewLen;
      SetChunk(NChunk->Chunks + f1, NewStart, NewLen, NChunk->Chunks[f1].Start, NChunk->Chunks[f1].Length);
      if (Warn)
         if (PartSum != NChunk->Chunks[f1].Length) Result = true;

   /* schauen, ob sukzessiv neue Chunks angebunden werden koennen */

      do {
         Found = false;
         for (z = 1; z < NChunk->RealLen; z++)
            if (z != f1)
               if (Overlap(NChunk->Chunks[z].Start, NChunk->Chunks[z].Length, NChunk->Chunks[f1].Start, NChunk->Chunks[f1].Length)) {
                  Found = true;
                  f2 = z;
                  break;
               }
         if (Found) {
            SetChunk(NChunk->Chunks + f1, NChunk->Chunks[f1].Start, NChunk->Chunks[f1].Length, NChunk->Chunks[f2].Start, NChunk->Chunks[f2].Length);
            NChunk->Chunks[f2] = NChunk->Chunks[--NChunk->RealLen];
         }
      }
      while (Found);
   }

/* ansonsten Feld erweitern und einschreiben */

   else {
      IncChunk(NChunk);

      NChunk->Chunks[NChunk->RealLen].Length = NewLen;
      NChunk->Chunks[NChunk->RealLen].Start = NewStart;
      NChunk->RealLen++;
   }

   return Result;
}

/*--------------------------------------------------------------------------*/
/* Ein Stueck wieder austragen */

void DeleteChunk(ChunkList * NChunk, LargeWord DelStart, LargeWord DelLen) {
   Word z;
   LargeWord OStart;

   if (DelLen == 0) return;

   z = 0;
   while (z <= NChunk->RealLen) {
      if (Overlap(DelStart, DelLen, NChunk->Chunks[z].Start, NChunk->Chunks[z].Length)) {
         if (NChunk->Chunks[z].Start >= DelStart)
            if (DelStart + DelLen >= NChunk->Chunks[z].Start + NChunk->Chunks[z].Length) {
            /* ganz loeschen */
               NChunk->Chunks[z] = NChunk->Chunks[--NChunk->RealLen];
            } else {
            /* unten abschneiden */
               OStart = NChunk->Chunks[z].Start;
               NChunk->Chunks[z].Start = DelStart + DelLen;
               NChunk->Chunks[z].Start -= NChunk->Chunks[z].Start - OStart;
         } else if (DelStart + DelLen >= NChunk->Chunks[z].Start + NChunk->Chunks[z].Length) {
         /* oben abschneiden */
            NChunk->Chunks[z].Length = DelStart - NChunk->Chunks[z].Start;
         /* wenn Laenge 0, ganz loeschen */
            if (NChunk->Chunks[z].Length == 0) {
               NChunk->Chunks[z] = NChunk->Chunks[--NChunk->RealLen];
            }
         } else {
         /* teilen */
            IncChunk(NChunk);
            NChunk->Chunks[NChunk->RealLen].Start = DelStart + DelLen;
            NChunk->Chunks[NChunk->RealLen].Length = NChunk->Chunks[z].Start + NChunk->Chunks[z].Length - NChunk->Chunks[NChunk->RealLen].Start;
            NChunk->Chunks[z].Length = DelStart - NChunk->Chunks[z].Start;
         }
      }
      z++;
   }
}

void chunks_init(void) {
}
