// AS-Portierung
// Unterroutinen des Makroprozessors
typedef struct _MacroRec {
   char *Name; /* Name des Makros */
   Byte ParamCount; /* Anzahl Parameter */
   StringList FirstLine; /* Zeiger auf erste Zeile */
   bool Used; /* wird gerade benutzt-verhindert Rekusion */
   bool LocMacExp; /* Makroexpansion wird aufgelistet */
} MacroRec, *PMacroRec;

#define BufferArraySize 1024

typedef struct _TInputTag {
   struct _TInputTag *Next;
   bool IsMacro;
   Integer IfLevel;
   bool First;
   String OrigPos;
   bool OrigDoLst;
   LongInt StartLine;
   bool (*Processor)(struct _TInputTag * P, char *erg);
   LongInt ParCnt, ParZ;
   StringList Params;
   LongInt LineCnt, LineZ;
   StringRecPtr Lines;
   String SpecName, SaveAttr;
   bool IsEmpty;
   FILE *Datei;
   void *Buffer;
   void (*Cleanup)(struct _TInputTag * P);
   void (*Restorer)(struct _TInputTag * P);
} TInputTag, *PInputTag;

typedef struct _TOutputTag {
   struct _TOutputTag *Next;
   void (*Processor)(void);
   Integer NestLevel;
   PInputTag Tag;
   PMacroRec Mac;
   StringList Params;
   LongInt PubSect, GlobSect;
   bool DoExport, DoGlobCopy;
   String GName;
} TOutputTag, *POutputTag;

extern PInputTag FirstInputTag;
extern POutputTag FirstOutputTag;

void Preprocess(void);
void AddMacro(PMacroRec Neu, LongInt DefSect, bool Protest);
bool FoundMacro(PMacroRec * Erg);
void ClearMacroList(void);
void ResetMacroDefines(void);
void ClearMacroRec(PMacroRec * Alt);
void PrintMacroList(void);
void PrintDefineList(void);
void ClearDefineList(void);
void ExpandDefines(char *Line);
void asmmac_init(void);
