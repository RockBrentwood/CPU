// Frankenstein Cross-Assemblers, version 2.0.
// Original author: Mark Zenier.
// The structures and data used in parser and output phases.
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// GetOpt.c:
#if !defined NOGETOPT
#   include <getopt.h>
#else
extern int optind;
extern char *optarg;
#endif
int GetOpt(int AC, char **AV, char *OptStr);

// Assem.c:
extern struct ifstack_t { enum { If_Active, If_Skip, If_Err } Else, EndIf; } ifstk[];
extern int ifstkpt;
extern const size_t IFSTKDEPTH;

extern struct fstkel { char *fnm; FILE *fpt; int line; } infilestk[];
extern const size_t FILESTKDPTH;

extern FILE *intermedf;
extern FILE *hexoutf, *loutf;
extern int errorcnt, warncnt;
extern bool hexflag, listflag;

extern char hexcva[];
#define hexch(cv) (hexcva[(cv)&0xf])

void frawarn(char *str);
void fraerror(char *str);
void frafatal(char *str);
void fracherror(char *str, char *start, char *beyond);
void prtequvalue(char *fstr, long lv);

// Scan.c:
#define INBUFFSZ 0x102
extern char finbuff[INBUFFSZ];

extern FILE *yyin;

typedef enum readacts { Nra_normal, Nra_new, Nra_end } readacts;
extern readacts nextreadact;

int Scan(void);
void yyerror(char *str);

// Syn*.y:
extern char *ignosyn, *ignosel;

/* opcode symbol table element */
struct opsym {
   char *opstr;
   int token;
   int numsyn;
   int subsyn;
};
extern struct opsym optab[];
extern int gnumopcode;
extern int ophashlnk[];

struct opsynt {
   int syntaxgrp;
   int elcnt;
   int gentabsub;
};
extern struct opsynt ostab[];

struct igel {
   int selmask;
   int criteria;
   char *genstr;
};
extern struct igel igtab[];

int yyparse(void);
void setreserved(void);
bool cpumatch(char *str);

// ExOp.c:
long EvalUnOp(char Op, long A);
long EvalBinOp(char Op, long A, long B);

// Reader.c:
extern int nextexprs;
extern int exprlist[];

extern int nextstrs;
extern char *stringlist[];

/* symbol table element */
typedef enum seg_t { SSG_RESV = -2, SSG_UNDEF = -1, SSG_UNUSED = 0, SSG_EQU = 2, SSG_SET = 3, SSG_ABS = 8 } seg_t;
#define seg_valued(seg) ((seg) > 0)
struct symel {
   char *symstr;
   int tok;
   seg_t seg;
   long value;
   struct symel *nextsym;
   int symnum;
};
extern struct symel **symbindex, *endsymbol;
extern int nextsymnum;

#define PPEXPRLEN 0x100
struct evalrel {
   seg_t seg;
   long value;
   char exprstr[PPEXPRLEN];
};
extern struct evalrel evalr[];

extern int chtnxalph, *chtcpoint, *chtnpoint;
extern int *chtatab[];
typedef enum char_tx { CF_END = -2, CF_INVALID, CF_UNDEF, CF_CHAR, CF_NUMBER } char_tx;

typedef enum { PCCASE_BIN = 1, PCCASE_UN, PCCASE_DEF, PCCASE_SYMB, PCCASE_CONS, PCCASE_PROGC } extag;

char *savestring(char *stx, int len);
void clrexpr(void);
int exprnode(extag swact, int left, int op, int right, long value, struct symel *symbol);
struct symel *symbentry(char *str, int toktyp);
void reservedsym(char *str, int tok, int value);
void buildsymbolindex(void);
void setophash(void);
int findop(char *str);
char *findgen(int op, int syntax, int crit);
void genlocrec(int seg, long loc);
int geninstr(char *str);
int chtcreate(void);
char_tx chtcfind(int *chtab, char **sourcepnt, int **tabpnt, int *numret);
int chtran(char **sourceptr);
int genstring(char *str);
void pevalexpr(int sub, int exn);

// Writer.c:
struct evstkel {
   long v;
   seg_t s;
};
extern struct evstkel estk[], *estkm1p;
extern const size_t PESTKDEPTH;

extern int currfstk;
#define nextfstk (currfstk+1)

extern int currseg;
extern long locctr;

void outphase(void);
