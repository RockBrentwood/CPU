/* asmpars.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Symbolen und das ganze Drumherum...                        */
/*                                                                           */
/* Historie:  5. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

typedef enum {UInt1    ,UInt2    ,UInt3    ,SInt4    ,UInt4    , Int4    ,
              SInt5    ,UInt5    , Int5    ,UInt6    ,UInt7    ,SInt8    ,UInt8    ,
               Int8    ,UInt9    ,UInt10   , Int10   ,UInt11   ,UInt12   , Int12   ,
              UInt13   ,SInt16   ,UInt16   , Int16   ,UInt18   ,SInt20   ,
              UInt20   , Int20   ,UInt22   ,SInt24   ,UInt24   , Int24   ,SInt32   ,UInt32   ,
               Int32   ,
#ifdef HAS64
               Int64   ,
#endif
               IntTypeCnt} IntType;

typedef enum {Float32,Float64,Float80,FloatDec,FloatCo,FloatTypeCnt} FloatType;

extern LargeWord IntMasks[IntTypeCnt];
extern LargeInt IntMins[IntTypeCnt];
extern LargeInt IntMaxs[IntTypeCnt];

extern Boolean FirstPassUnknown;
extern Boolean SymbolQuestionable;
extern Boolean UsesForwards;
extern LongInt MomLocHandle;
extern LongInt LocHandleCnt;
extern Boolean BalanceTree;
extern LongInt MomLocHandle;


extern void AsmParsInit(void);


extern LargeInt ConstIntVal(char *Asc_O, IntType Typ, Boolean *Ok);

extern Double ConstFloatVal(char *Asc_O, FloatType Typ, Boolean *Ok);

extern void ConstStringVal(char *Asc, char *Erg, Boolean *OK);


extern Boolean RangeCheck(LargeInt Wert, IntType Typ);

extern Boolean FloatRangeCheck(Double Wert, FloatType Typ);


extern Boolean IdentifySection(char *Name, LongInt *Erg);


extern Boolean ExpandSymbol(char *Name);

extern void EnterIntSymbol(char *Name_O, LargeInt Wert, Byte Typ, Boolean MayChange);

extern void EnterFloatSymbol(char *Name_O, Double Wert, Boolean MayChange);

extern void EnterStringSymbol(char *Name_O, char *Wert, Boolean MayChange);

extern Boolean GetIntSymbol(char *Name, LargeInt *Wert);

extern Boolean GetFloatSymbol(char *Name, Double *Wert);

extern Boolean GetStringSymbol(char *Name, char *Wert);

extern void PrintSymbolList(void);

extern void PrintDebSymbols(FILE *f);

extern void PrintSymbolTree(void);

extern void ClearSymbolList(void);

extern void ResetSymbolDefines(void);

extern void PrintSymbolDepth(void);


extern void SetSymbolSize(char *Name, ShortInt Size);

extern ShortInt GetSymbolSize(char *Name);

extern Boolean IsSymbolFloat(char *Name);

extern Boolean IsSymbolString(char *Name);

extern Boolean IsSymbolDefined(char *Name);

extern Boolean IsSymbolUsed(char *Name);

extern Boolean IsSymbolChangeable(char *Name);


extern void EvalExpression(char *Asc_O, TempResult *Erg);

extern LargeInt EvalIntExpression(char *Asc, IntType Typ, Boolean *OK);

extern Double EvalFloatExpression(char *Asc, FloatType Typ, Boolean *OK);

extern void EvalStringExpression(char *Asc, Boolean *OK, char *Result);


extern Boolean PushSymbol(char *SymName_O, char *StackName_O);

extern Boolean PopSymbol(char *SymName_O, char *StackName_O);

extern void ClearStacks(void);
 

extern void EnterFunction(char *FName, char *FDefinition, Byte NewCnt);

extern PFunction FindFunction(char *Name);

extern void PrintFunctionList(void);

extern void ClearFunctionList(void);


extern void AddDefSymbol(char *Name, TempResult *Value);

extern void RemoveDefSymbol(char *Name);

extern void CopyDefSymbols(void);


extern void PrintCrossList(void);

extern void ClearCrossList(void);


extern LongInt GetSectionHandle(char *SName_O, Boolean AddEmpt, LongInt Parent);

extern char *GetSectionName(LongInt Handle);

extern void SetMomSection(LongInt Handle);

extern void AddSectionUsage(LongInt Start, LongInt Length);

extern void PrintSectionList(void);

extern void PrintDebSections(FILE *f);

extern void ClearSectionList(void);


extern void SetFlag(Boolean *Flag, char *Name, Boolean Wert);


extern LongInt GetLocHandle(void);

extern void PushLocHandle(LongInt NewLoc);

extern void PopLocHandle(void);

extern void ClearLocStack(void);


extern void asmpars_init(void);
