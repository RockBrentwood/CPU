// AS-Portierung
// Verwaltung von Symbolen und das ganze Drumherum...
typedef enum { UInt1, UInt2, UInt3, SInt4, UInt4, Int4,
   SInt5, UInt5, Int5, UInt6, UInt7, SInt8, UInt8,
   Int8, UInt9, UInt10, Int10, UInt11, UInt12, Int12,
   UInt13, SInt16, UInt16, Int16, UInt18, SInt20,
   UInt20, Int20, UInt22, SInt24, UInt24, Int24, SInt32, UInt32,
   Int32,
#ifdef HAS64
   Int64,
#endif
   IntTypeCnt
} IntType;

typedef enum { Float32, Float64, Float80, FloatDec, FloatCo, FloatTypeCnt } FloatType;

extern LargeWord IntMasks[IntTypeCnt];
extern LargeInt IntMins[IntTypeCnt];
extern LargeInt IntMaxs[IntTypeCnt];

extern bool FirstPassUnknown;
extern bool SymbolQuestionable;
extern bool UsesForwards;
extern LongInt MomLocHandle;
extern LongInt LocHandleCnt;
extern bool BalanceTree;
extern LongInt MomLocHandle;

void AsmParsInit(void);
LargeInt ConstIntVal(char *Asc_O, IntType Typ, bool *Ok);
Double ConstFloatVal(char *Asc_O, FloatType Typ, bool *Ok);
void ConstStringVal(char *Asc, char *Erg, bool *OK);
bool RangeCheck(LargeInt Wert, IntType Typ);
bool FloatRangeCheck(Double Wert, FloatType Typ);
bool IdentifySection(char *Name, LongInt * Erg);
bool ExpandSymbol(char *Name);
void EnterIntSymbol(char *Name_O, LargeInt Wert, Byte Typ, bool MayChange);
void EnterFloatSymbol(char *Name_O, Double Wert, bool MayChange);
void EnterStringSymbol(char *Name_O, char *Wert, bool MayChange);
bool GetIntSymbol(char *Name, LargeInt * Wert);
bool GetFloatSymbol(char *Name, Double * Wert);
bool GetStringSymbol(char *Name, char *Wert);
void PrintSymbolList(void);
void PrintDebSymbols(FILE * f);
void PrintSymbolTree(void);
void ClearSymbolList(void);
void ResetSymbolDefines(void);
void PrintSymbolDepth(void);
void SetSymbolSize(char *Name, ShortInt Size);
ShortInt GetSymbolSize(char *Name);
bool IsSymbolFloat(char *Name);
bool IsSymbolString(char *Name);
bool IsSymbolDefined(char *Name);
bool IsSymbolUsed(char *Name);
bool IsSymbolChangeable(char *Name);
void EvalExpression(char *Asc_O, TempResult * Erg);
LargeInt EvalIntExpression(char *Asc, IntType Typ, bool *OK);
Double EvalFloatExpression(char *Asc, FloatType Typ, bool *OK);
void EvalStringExpression(char *Asc, bool *OK, char *Result);
bool PushSymbol(char *SymName_O, char *StackName_O);
bool PopSymbol(char *SymName_O, char *StackName_O);
void ClearStacks(void);
void EnterFunction(char *FName, char *FDefinition, Byte NewCnt);
PFunction FindFunction(char *Name);
void PrintFunctionList(void);
void ClearFunctionList(void);
void AddDefSymbol(char *Name, TempResult * Value);
void RemoveDefSymbol(char *Name);
void CopyDefSymbols(void);
void PrintCrossList(void);
void ClearCrossList(void);
LongInt GetSectionHandle(char *SName_O, bool AddEmpt, LongInt Parent);
char *GetSectionName(LongInt Handle);
void SetMomSection(LongInt Handle);
void AddSectionUsage(LongInt Start, LongInt Length);
void PrintSectionList(void);
void PrintDebSections(FILE * f);
void ClearSectionList(void);
void SetFlag(bool *Flag, char *Name, bool Wert);
LongInt GetLocHandle(void);
void PushLocHandle(LongInt NewLoc);
void PopLocHandle(void);
void ClearLocStack(void);
void asmpars_init(void);
