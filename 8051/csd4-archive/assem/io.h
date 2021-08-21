typedef unsigned char byte;
typedef unsigned short word;

extern FILE *ExF;

extern char InSeg;
extern char **FileTab;
extern int StartF, CurF;
extern long Files;

#define TYPES 5
#define SEG_TYPES 3
typedef struct Segment *Segment;
extern struct Segment {
   int Type; bool Rel;
   word Line, File, Base, Size;
   long Loc;
} SegTab[], *SegP;
extern unsigned long CurLoc;

typedef struct Gap *Gap;
extern struct Gap {
   Segment Seg; word Offset, Size;
} GapTab[], *GapP;

typedef struct Symbol *Symbol;
struct Symbol {
   char *Name; word Index;
   bool Global, Defined, Address, Variable, Map;
   Segment Seg; word Offset;
   Symbol Next[1];
};

extern Symbol NIL;
extern void SegInit(void);
extern void StartSeg(byte Type, bool Rel, word Base);
extern void EndSeg(void);
extern void Space(word Size);

extern void PByte(byte B);
extern void PString(char *S);

extern void SymInit(void);
extern char *CopyS(char *S);
extern Symbol LookUp(char *Name);
extern void RegInit(void);

typedef enum {
   RB = 1, RW, DB, DW, ORG, SEG, END, GLOBAL, EXTERN, INCLUDE, IF, ELSE,
   EQU, SET, LCURL, RCURL, LPAR, COMMA, SEMI,
   SYMBOL, STRING, NUMBER, DOLLAR,
   RPAR, COLON, QUEST,
   NOT, NOT_NOT, HIGH, LOW, PLUS, MINUS, MULT, DIV, MOD,
   LT, LE, GT, GE, EQ, NE, AND_AND, OR_OR,
   AND, XOR, OR, SHL, SHR, DOT, BY,
   AT, POUND, REGISTER, TYPE, MNEMONIC
} Lexical;
extern Lexical OldL;

typedef enum {
   ACC, AB, CY, DPTR, PC, R0, R1, R2, R3, R4, R5, R6, R7
} Register;
typedef enum { CODE, XDATA, DATA, SFR, BIT } Type;

extern char Text[]; extern int Value;
extern Symbol Sym;
extern bool InExp, InSemi;

extern int Line, StartLine;
extern Lexical Scan(void);
extern void FileInit(void);
extern void Error(const char *Msg, ...);
extern void Fatal(const char *Msg, ...);
extern void Check(void);
extern void *Allocate(unsigned Size);

extern byte GetB(FILE *InF);
extern word GetW(FILE *InF);
extern unsigned long GetL(FILE *InF);
extern void PutB(byte B, FILE *ExF);
extern void PutW(word W, FILE *ExF);
extern void PutL(unsigned long L, FILE *ExF);

extern void OpenF(char *Name);

extern struct AddrItem {
   long Lo, Hi; bool ReadOnly;
} AddrTab[];
