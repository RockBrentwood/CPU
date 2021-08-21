typedef struct Exp *Exp;
typedef enum { NumX, AddrX, SymX, UnX, BinX, CondX } ExpTag;
struct Exp {
   word Hash, Line, File; int Mark:1, Map:1;
   ExpTag Tag; Exp Tail, Next;
   union {
      word Value;
      struct { Segment Seg; word Offset; } Addr;
      Symbol Sym;
      struct { Lexical Op; Exp A, B, C; } Funct;
   } Body;
};
extern Exp ExpHead;

#define ValOf(E)	((E)->Body.Value)
#define SegOf(E)	((E)->Body.Addr.Seg)
#define OffOf(E)	((E)->Body.Addr.Offset)
#define SymOf(E)	((E)->Body.Sym)
#define OpOf(E)		((E)->Body.Funct.Op)
#define ArgA(E)		((E)->Body.Funct.A)
#define ArgB(E)		((E)->Body.Funct.B)
#define ArgC(E)		((E)->Body.Funct.C)

extern Exp Parse(int Dir);
extern Exp MakeExp(ExpTag Tag, ...);
extern Exp EvalExp(Exp E);
extern void MarkExp(Exp E);
extern void ExpInit(void);
