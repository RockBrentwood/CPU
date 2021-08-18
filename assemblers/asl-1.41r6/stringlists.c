// AS-Portierung
// Implementation von String-Listen
#include "stdinc.h"
#include <string.h>
#include "stringutil.h"
#include "stringlists.h"

void InitStringList(StringList * List) {
   *List = NULL;
}

void ClearStringEntry(StringRecPtr * Elem) {
   free((*Elem)->Content);
   free(*Elem);
   *Elem = NULL;
}

void ClearStringList(StringList * List) {
   StringRecPtr Hilf;

   while (*List != NULL) {
      Hilf = (*List);
      *List = (*List)->Next;
      ClearStringEntry(&Hilf);
   }
}

void AddStringListFirst(StringList * List, char *NewStr) {
   StringRecPtr Neu;

   Neu = (StringRecPtr) malloc(sizeof(StringRec));
   Neu->Content = strdup(NewStr);
   Neu->Next = (*List);
   *List = Neu;
}

void AddStringListLast(StringList * List, char *NewStr) {
   StringRecPtr Neu, Lauf;

   Neu = (StringRecPtr) malloc(sizeof(StringRec));
   Neu->Content = strdup(NewStr);
   Neu->Next = NULL;
   if (*List == NULL) *List = Neu;
   else {
      Lauf = (*List);
      while (Lauf->Next != NULL) Lauf = Lauf->Next;
      Lauf->Next = Neu;
   }
}

void RemoveStringList(StringList * List, char *OldStr) {
   StringRecPtr Lauf, Hilf;

   if (*List == NULL) return;
   if (strcmp((*List)->Content, OldStr) == 0) {
      Hilf = (*List);
      *List = (*List)->Next;
      ClearStringEntry(&Hilf);
   } else {
      Lauf = (*List);
      while ((Lauf->Next != NULL) && (strcmp(Lauf->Next->Content, OldStr) != 0)) Lauf = Lauf->Next;
      if (Lauf->Next != NULL) {
         Hilf = Lauf->Next;
         Lauf->Next = Hilf->Next;
         ClearStringEntry(&Hilf);
      }
   }
}

char *GetStringListFirst(StringList List, StringRecPtr * Lauf) {
   static char *Dummy = "", *tmp;

   *Lauf = List;
   if (*Lauf == NULL) return Dummy;
   else {
      tmp = (*Lauf)->Content;
      *Lauf = (*Lauf)->Next;
      return tmp;
   }
}

char *GetStringListNext(StringRecPtr * Lauf) {
   static char *Dummy = "", *tmp;

   if (*Lauf == NULL) return Dummy;
   else {
      tmp = (*Lauf)->Content;
      *Lauf = (*Lauf)->Next;
      return tmp;
   }
}

char *GetAndCutStringList(StringList * List) {
   StringRecPtr Hilf;
   static String Result;

   if (*List == NULL) Result[0] = '\0';
   else {
      Hilf = (*List);
      *List = (*List)->Next;
      strmaxcpy(Result, Hilf->Content, 255);
      free(Hilf->Content);
      free(Hilf);
   }
   return Result;
}

bool StringListEmpty(StringList List) {
   return (List == NULL);
}

StringList DuplicateStringList(StringList Src) {
   StringRecPtr Lauf;
   StringList Dest;

   InitStringList(&Dest);
   if (Src != NULL) {
      AddStringListLast(&Dest, GetStringListFirst(Src, &Lauf));
      while (Lauf != NULL)
         AddStringListLast(&Dest, GetStringListNext(&Lauf));
   }
   return Dest;
}

bool StringListPresent(StringList List, char *Search) {
   while ((List != NULL) && (strcmp(List->Content, Search) != 0)) List = List->Next;
   return (List != NULL);
}

void stringlists_init(void) {
}
