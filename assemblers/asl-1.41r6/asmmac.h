/* asmmac.h  */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterroutinen des Makroprozessors                                         */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

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

extern void Preprocess(void);

extern void AddMacro(PMacroRec Neu, LongInt DefSect, bool Protest);

extern bool FoundMacro(PMacroRec * Erg);

extern void ClearMacroList(void);

extern void ResetMacroDefines(void);

extern void ClearMacroRec(PMacroRec * Alt);

extern void PrintMacroList(void);

extern void PrintDefineList(void);

extern void ClearDefineList(void);

extern void ExpandDefines(char *Line);

extern void asmmac_init(void);
