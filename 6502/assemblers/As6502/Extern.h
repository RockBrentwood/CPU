extern FILE *optr;
extern FILE *iptr;
extern int dflag; /* debug flag */
extern int errcnt; /* error counter */
extern char hex[]; /* hexadecimal character buffer */
extern int iflag; /* ignore .nlst flag */
extern int lablptr; /* label pointer into symbol table */
extern int lflag; /* disable listing flag */
extern int loccnt; /* location counter     */
extern int nflag; /* normal/split address mode */
extern int mflag; /* generate MOS Technology object format */
extern int nxt_free; /* next free location in symtab */
extern int objcnt; /* object byte counter */
extern int oflag; /* object output flag */
extern int opflg; /* operation code flags */
extern int opval; /* operation code value */
extern int pass; /* pass counter         */
extern char prlnbuf[]; /* print line buffer    */
extern int sflag; /* symbol table output flag */
extern int slnum; /* source line number counter */
extern char *symtab; /* symbol table         */
extern unsigned size; /* symbol table size            */
extern char symbol[]; /* temporary symbol storage     */
extern int udtype; /* undefined symbol type        */
extern int undef; /* undefined symbol in expression flg  */
extern int value; /* operand field value */
extern char zpref; /* zero page reference flag     */
/* added by Joel Swank 12/86   */
extern int pagect; /* count of pages               */
extern int paglin; /* lines printed on current page */
extern int pagesize; /* maximum lines per page       */
extern int linesize; /* maximum characters oer line  */
extern int titlesize; /* maximum characters oer line  */
extern char titlbuf[]; /* buffer for title from .page  */
extern char syspc[80]; /* variable filler for heading  */
extern char *date; /* pointer to formatted date string */

// Assm1.c:
extern int optab[];
extern int step[];

// Assm2.c:
void printhead(void);
void println(void);
void hexcon(int digit, int num);
void fin_obj(void);
void loadlc(int val, int f, int outflg);
void loadv(int val, int f, int outflg);
void error(char *stptr);
int labldef(int lval);
void assemble(void);
int symval(int *ip);

// Assm3.c:
void class1(void);
void class2(int *ip);
void class3(int *ip);
void pseudo(int *ip);
