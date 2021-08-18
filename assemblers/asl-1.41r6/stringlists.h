// AS-Portierung
// Verwaltung von String-Listen
typedef struct _StringRec {
   struct _StringRec *Next;
   char *Content;
} StringRec, *StringRecPtr;
typedef StringRecPtr StringList;

void InitStringList(StringList * List);
void ClearStringEntry(StringRecPtr * Elem);
void ClearStringList(StringList * List);
void AddStringListFirst(StringList * List, char *NewStr);
void AddStringListLast(StringList * List, char *NewStr);
void RemoveStringList(StringList * List, char *OldStr);
char *GetStringListFirst(StringList List, StringRecPtr * Lauf);
char *GetStringListNext(StringRecPtr * Lauf);
char *GetAndCutStringList(StringList * List);
bool StringListEmpty(StringList List);
StringList DuplicateStringList(StringList Src);
bool StringListPresent(StringList List, char *Search);
void stringlists_init(void);
