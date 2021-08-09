// Frankenstein Cross-Assemblers, version 2.0.
// Original author: Mark Zenier.
// The structures and data used in parser and output phases.
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define PRINTCTRL(char) ((char)+'@')

#ifndef Global
#   define	Global	extern
#endif

#define TRUE 1
#define FALSE 0

#define hexch(cv) (hexcva[(cv)&0xf])
extern char hexcva[];

/* symbol table element */
struct symel {
   char *symstr;
   int tok;
   int seg;
   long value;
   struct symel *nextsym;
   int symnum;
};

#define SSG_UNUSED 0
#define SSG_UNDEF -1
#define SSG_ABS 8
#define SSG_RESV -2
#define SSG_EQU 2
#define SSG_SET 3

#define SYMNULL (struct symel *) NULL

/* opcode symbol table element */

struct opsym {
   char *opstr;
   int token;
   int numsyn;
   int subsyn;
};

struct opsynt {
   int syntaxgrp;
   int elcnt;
   int gentabsub;
};

struct igel {
   int selmask;
   int criteria;
   char *genstr;
};

#define PPEXPRLEN 256

struct evalrel {
   int seg;
   long value;
   char exprstr[PPEXPRLEN];
};

#define INBUFFSZ 258
extern char finbuff[INBUFFSZ];

extern int nextsymnum;
Global struct symel **symbindex;

#define EXPRLSIZE (INBUFFSZ/2)
extern int nextexprs;
Global int exprlist[EXPRLSIZE];

#define STRLSIZE (INBUFFSZ/2)
extern int nextstrs;
Global char *stringlist[STRLSIZE];

extern struct opsym optab[];
extern int gnumopcode;
extern struct opsynt ostab[];
extern struct igel igtab[];
extern int ophashlnk[];

#define NUMPEXP 6
Global struct evalrel evalr[NUMPEXP];

#define PESTKDEPTH 32
struct evstkel {
   long v;
   int s;
};

Global struct evstkel estk[PESTKDEPTH], *estkm1p;

Global int currseg;
Global long locctr;

extern FILE *yyin;
extern FILE *intermedf;
extern int listflag;
extern int hexvalid, hexflag;
Global FILE *hexoutf, *loutf;
extern int errorcnt, warncnt;

extern int linenumber;

#define IFSTKDEPTH 32
extern int ifstkpt;
Global enum { If_Active, If_Skip, If_Err } elseifstk[IFSTKDEPTH], endifstk[IFSTKDEPTH];

#define FILESTKDPTH 20
Global int currfstk;
#define nextfstk (currfstk+1)
Global struct fstkel {
   char *fnm;
   FILE *fpt;
} infilestk[FILESTKDPTH];

Global int lnumstk[FILESTKDPTH];
Global char currentfnm[100];

extern struct symel *endsymbol;

enum readacts {
   Nra_normal,
   Nra_new,
   Nra_end
};

extern enum readacts nextreadact;

extern struct symel *endsymbol;
extern char ignosyn[];
extern char ignosel[];

#define NUM_CHTA 6
extern int chtnxalph, *chtcpoint, *chtnpoint;
Global int *chtatab[NUM_CHTA];

#define CF_END		-2
#define CF_INVALID 	-1
#define CF_UNDEF 	0
#define CF_CHAR 	1
#define CF_NUMBER 	2

// frasmain.c:
void frawarn(char *str);
void fraerror(char *str);
void frafatal(char *str);
void fracherror(char *str, char *start, char *beyond);
void prtequvalue(char *fstr, long lv);
int main(int argc, char *argv[]);

// fryylex.c:
int yylex(void);
void yyerror(char *str);

// as*.y:
int yyparse(void);
int lexintercept(void);
void setreserved(void);
int cpumatch(char *str);

// frapsub.c:
char *savestring(char *stx, int len);
void clrexpr(void);
int exprnode(int swact, int left, int op, int right, long value, struct symel *symbol);
struct symel *symbentry(char *str, int toktyp);
void reservedsym(char *str, int tok, int value);
void buildsymbolindex(void);
void setophash(void);
int findop(char *str);
char *findgen(int op, int syntax, int crit);
void genlocrec(int seg, long loc);
int geninstr(char *str);
int chtcreate(void);
int chtcfind(int *chtab, char **sourcepnt, int **tabpnt, int *numret);
int chtran(char **sourceptr);
int genstring(char *str);
void pevalexpr(int sub, int exn);

// fraosub.c:
void outphase(void);
