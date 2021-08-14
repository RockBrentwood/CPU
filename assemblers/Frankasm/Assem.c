// Frankenstein Cross-Assemblers, version 2.0.
// Original author: Mark Zenier.
// The main driver file.
#include <stdio.h>
#include "Extern.h"

struct ifstack_t ifstk[0x20];
int ifstkpt = 0;
const size_t IFSTKDEPTH = sizeof ifstk/sizeof ifstk[0];

struct fstkel infilestk[20];
const size_t FILESTKDPTH = sizeof infilestk/sizeof infilestk[0];

FILE *intermedf = NULL;
#if defined DOSTMP
static char interfn[] = "frtXXXXXX";
#elif defined AMIGA
static char interfn[] = "T:frtXXXXXX";
#else
static char interfn[] = "/usr/tmp/frtXXXXXX";
#endif
FILE *hexoutf, *loutf;
static char *hexfn, *loutfn;
int errorcnt = 0, warncnt = 0;
bool listflag = false, hexflag = false;
static bool debugmode = false;
static FILE *symbf;
static char *symbfn;
static bool symbflag = false;
char hexcva[17] = "0123456789abcdef";

// First pass ― generate warning message by writing line to intermediate file.
// Parameter:
// ∙	str: the message.
// Global:
// ∙	the count of warnings.
void frawarn(char *str) { fprintf(intermedf, "E: WARNING - %s\n", str), warncnt++; }

// First pass ― generate error message by writing line to intermediate file.
// Parameter:
// ∙	str: the message.
// Global:
// ∙	the count of errors.
void fraerror(char *str) { fprintf(intermedf, "E: ERROR - %s\n", str), errorcnt++; }

// Fatal error subroutine, shutdown and quit right now!
// Parameter:
// ∙	str: the message.
// Global:
// ∙	if debug mode is true, save the intermediate file.
// No return:
// ∙	exit(2).
void frafatal(char *str) {
   fprintf(stderr, "Fatal error - %s\n", str);
   if (intermedf != NULL) {
      fclose(intermedf);
      if (!debugmode) remove(interfn);
   }
   exit(2);
}

// First pass ― generate error message by writing line to intermediate file.
// Parameters:
// ∙	str:	the message,
// ∙	start:	the pointer to bad character definition,
// ∙	beyond:	the pointer after bad definition.
// Global:
// ∙	the count of errors.
void fracherror(char *str, char *start, char *beyond) {
   char bcbuff[8];
   int cnt = 0;
   for (; start < beyond && *start != '\0' && cnt < 7; cnt++) bcbuff[cnt] = *start++;
   bcbuff[cnt] = '\0';
   fprintf(intermedf, "E: ERROR - %s \'%s\'\n", str, bcbuff);
   errorcnt++;
}

// First pass ― generate comment lines in the intermediate file for the value in a set, equate, or org statement, etc..
// Parameters:
// ∙	fstr:	the format string, and
// ∙	lv:	a long integer value.
void prtequvalue(char *fstr, long lv) { fprintf(intermedf, fstr, lv); }

#define SYMPERLINE 3

// Print the symbols on the listing file, 3 symbols across.
// Only the first 15 characters are printed though all are significant.
// Reserved symbols are not assigned symbol numbers and thus are not printed.
// Globals:
// ∙	the symbol index array and
// ∙	the symbol table elements.
static void printsymbols(void) {
   int npl = 0;
   for (int syn = 1; syn < nextsymnum; npl++, syn++) {
      if (npl >= SYMPERLINE) fputc('\n', loutf), npl = 0;
      struct symel *syp = symbindex[syn];
      if (syp->seg != SSG_UNDEF)
         fprintf(loutf, "%8.8lx %-15.15s  ", syp->value, syp->symstr);
      else
         fprintf(loutf, "???????? %-15.15s  ", syp->symstr);
   }
   if (npl > 0) fputc('\n', loutf);
   fputc('\f', loutf);
}

// Print the symbols to the symbol table file.
// Globals:
// ∙	the symbol index array and
// ∙	the symbol table elements.
static void filesymbols(void) {
   for (int syn = 1; syn < nextsymnum; syn++) {
      struct symel *syp = symbindex[syn];
      if (syp->seg != SSG_UNDEF)
         fprintf(symbf, "%8.8lx %s\n", syp->value, syp->symstr);
      else
         fprintf(symbf, "???????? %s\n", syp->symstr);
   }
}

// The top driver routine for the framework cross-assembler:
// ∙	set the cpu type if implemented in the parser,
// ∙	process the command line parameters,
// ∙	set up the tables,
// ∙	call the first pass parser,
// ∙	print the symbol table,
// ∙	call the second pass,
// ∙	close down and delete the outputs if there were any errors.
// Return:
// ∙	2 for error,
// ∙	0 for OK.
int main(int argc, char *argv[]) {
   char *App = argc < 1? NULL: argv[0]; if (App == NULL || *App == '\0') App = "FrAsm";
   cpumatch(App);
   for (int grv; (grv = getopt(argc, argv, "dh:o:l:s:p:")) != EOF; ) switch (grv) {
      case 'o': case 'h': hexfn = optarg, hexflag = true; break;
      case 'l': loutfn = optarg, listflag = true; break;
      case 'd': debugmode = true; break;
      case 's': symbflag = true, symbfn = optarg; break;
      case 'p':
         if (!cpumatch(optarg)) fprintf(stderr, "%s: no match on CPU type %s, default used\n", App, optarg);
      break;
      case '?': break;
   }
   if (optind >= argc) {
      fprintf(stderr, "%s: no input file\n", App);
      return 1;
   }
   yyin = strcmp(argv[optind], "-") == 0? stdin: fopen(argv[optind], "r");
   if (yyin == NULL) {
      fprintf(stderr, "%s: cannot open input file %s\n", App, argv[optind]);
      return 1;
   }
   if (listflag) {
      if (strcmp(argv[optind], loutfn) == 0)
         fprintf(stderr, "%s: list file overwrites input %s\n", App, loutfn), listflag = false;
      else if ((loutf = fopen(loutfn, "w")) == NULL)
         fprintf(stderr, "%s: cannot open list file %s\n", App, loutfn), listflag = false;
   }
   if (!listflag) loutf = stdout;
   mkstemp(interfn);
   intermedf = fopen(interfn, "w");
   if (intermedf == NULL) {
      fprintf(stderr, "%s: cannot open temp file %s\n", App, interfn);
      return 1;
   }
   setophash();
   setreserved();
   ifstk[0].Else = ifstk[0].EndIf = If_Err;
   fprintf(intermedf, "F:%s\n", argv[optind]);
   infilestk[0].fpt = yyin, infilestk[0].fnm = argv[optind];
   currfstk = 0;
   currseg = 0;
   yyparse();
   if (ifstkpt != 0) fraerror("active IF at end of file");
   buildsymbolindex();
   if (listflag) printsymbols();
   if (symbflag) {
      if (strcmp(argv[optind], symbfn) == 0)
         fprintf(stderr, "%s: symbol file overwrites input %s\n", App, symbfn);
      else if ((symbf = fopen(symbfn, "w")) == NULL)
         fprintf(stderr, "%s: cannot open symbol file %s\n", App, symbfn);
      else
         filesymbols(), fclose(symbf);
   }
   fclose(intermedf), intermedf = fopen(interfn, "r"); // Reopen.
   if (intermedf == NULL) {
      fprintf(stderr, "%s: cannot open temp file %s\n", App, interfn);
      return 1;
   }
   if (errorcnt > 0) hexflag = false;
   if (hexflag) {
      if (strcmp(argv[optind], hexfn) == 0)
         fprintf(stderr, "%s: hex output overwrites input %s\n", App, hexfn), hexflag = false;
      else if ((hexoutf = fopen(hexfn, "w")) == NULL)
         fprintf(stderr, "%s: cannot open hex output %s\n", App, hexfn), hexflag = false;
   }
   currfstk = 0;
   outphase();
   if (errorcnt > 0 || warncnt > 0) {
      fprintf(loutf, " Error Summary:");
      if (errorcnt > 0) fprintf(loutf, " %d error(s)", errorcnt);
      if (errorcnt > 0 && warncnt > 0) fputc(',', loutf);
      if (warncnt > 0) fprintf(loutf, " %d warning(s)", warncnt);
      fprintf(loutf, ".\n");
      if (listflag) {
         fprintf(stderr, " Error Summary:");
         if (errorcnt > 0) fprintf(stderr, " %d error(s)", errorcnt);
         if (errorcnt > 0 && warncnt > 0) fputc(',', stderr);
         if (warncnt > 0) fprintf(stderr, " %d warning(s)", warncnt);
         fprintf(stderr, ".\n");
      }
   }
   if (listflag) fclose(loutf);
   if (hexflag) {
      fclose(hexoutf);
      if (errorcnt > 0) remove(hexfn);
   }
   fclose(intermedf);
   if (!debugmode) remove(interfn); else abort();
   return errorcnt > 0? 2: 0;
}
