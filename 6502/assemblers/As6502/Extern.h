#include <stdbool.h>

// Assm0.c:
extern int opval; // operation code value
extern struct OpT { int Nmemonic, Operation, Code; } optab[];
extern int step[];

// Assm1.c:
extern FILE *optr;
extern int errcnt; // error counter
extern bool iflag; // ignore .nlst flag
extern int lflag; // disable listing flag
extern bool mflag; // generate MOS Technology object format
extern bool nflag; // normal/split address mode
extern bool oflag; // object output flag
extern int nxt_free; // next free location in symtab
extern int pass; // pass counter

// Added by Joel Swank (1986/12).
extern int pagect; // count of pages
extern int paglin; // lines printed on current page
extern int pagesize; // maximum lines per page
extern int linesize; // maximum characters oer line
extern int titlesize; // maximum characters oer line
extern char titlbuf[]; // buffer for title from .page
extern char *date; // pointer to formatted date string

// Assm2.c:
extern char hex[]; // hexadecimal character buffer
extern int objcnt; // object byte counter
extern int opflg; // operation code flags
extern char prlnbuf[]; // print line buffer
extern int slnum; // source line number counter
extern char *symtab; // symbol table
extern unsigned size; // symbol table size
extern char symbol[]; // temporary symbol storage
extern int udtype; // undefined symbol type
extern bool undef; // undefined symbol in expression flg

void printhead(void);
void println(void);
void hexcon(int digit, int num);
void fin_obj(void);
void loadlc(int val, int f, bool outflg);
void loadv(int val, int f, bool outflg);
void error(char *stptr);
int labldef(int lval);
void assemble(void);
int symval(int *ip);

// Assm3.c:
extern int loccnt; // location counter
extern bool zpref; // zero page reference flag

void class1(void);
void class2(int *ip);
void class3(int *ip);
void pseudo(int *ip);
