// Frankenstein Cross-Assemblers, version 2.0.
// Original author: Mark Zenier.
// Main file.
#include <stdio.h>
#include "frasmdat.h"

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

// First pass - generate warning message by writing line
// to intermediate file
// Parameters:
//	message
// Globals:
//	the count of warnings
void frawarn(char *str) {
   fprintf(intermedf, "E: WARNING - %s\n", str);
   warncnt++;
}

// First pass - generate error message by writing line to
// intermediate file
// Parameters:
//	message
// Globals:
//	count of errors
void fraerror(char *str) {
   fprintf(intermedf, "E: ERROR - %s\n", str);
   errorcnt++;
}

// Fatal error subroutine, shutdown and quit right now!
// Parameters:
//	message
// Globals:
//	if debug mode is true, save intermediate file
// Return:
//	exit(2)
void frafatal(char *str) {
   fprintf(stderr, "Fatal error - %s\n", str);

   if (intermedf != (FILE *) NULL) {
      fclose(intermedf);
      if (!debugmode)
         remove(interfn);
   }

   exit(2);
}

// First pass - generate error message by writing line to
// intermediate file
// Parameters:
//	message
//	pointer to bad character definition
//	pointer after bad definition
// Globals:
//	count of errors
void fracherror(char *str, char *start, char *beyond) {
   char bcbuff[8];
   int cnt;

   for (cnt = 0; start < beyond && *start != '\0' && cnt < 7; cnt++) {
      bcbuff[cnt] = *start++;
   }
   bcbuff[cnt] = '\0';

   fprintf(intermedf, "E: ERROR - %s \'%s\'\n", str, bcbuff);
   errorcnt++;
}

// First pass - generate comment lines in intermediate file
// for the value in a set, equate, or org statement, etc...
// Parameters:
//	format string and a long integer value
void prtequvalue(char *fstr, long lv) {
   fprintf(intermedf, fstr, lv);
}

#define SYMPERLINE 3

// Print the symbols on the listing file, 3 symbols
// across.  Only the first 15 characters are printed
// though all are significant.  Reserved symbols are
// not assigned symbol numbers and thus are not printed.
// Globals:
//	the symbol index array and the symbol table elements.
static void printsymbols(void) {
   int syn, npl = 0;
   struct symel *syp;

   for (syn = 1; syn < nextsymnum; syn++) {
      if (npl >= SYMPERLINE) {
         fputc('\n', loutf);
         npl = 0;
      }

      syp = symbindex[syn];

      if (syp->seg != SSG_UNDEF)
         fprintf(loutf, "%8.8lx %-15.15s  ", syp->value, syp->symstr);
      else
         fprintf(loutf, "???????? %-15.15s  ", syp->symstr);
      npl++;
   }

   if (npl > 0)
      fputc('\n', loutf);

   fputc('\f', loutf);
}

// Print the symbols to the symbol table file
// Globals:
//	the symbol index array and the symbol table elements.
static void filesymbols(void) {
   int syn;
   struct symel *syp;

   for (syn = 1; syn < nextsymnum; syn++) {
      syp = symbindex[syn];

      if (syp->seg != SSG_UNDEF)
         fprintf(symbf, "%8.8lx %s\n", syp->value, syp->symstr);
      else
         fprintf(symbf, "???????? %s\n", syp->symstr);
   }
}

// Top driver routine for framework cross assembler
// set the cpu type if implemented in parser
// process the command line parameters
// setup the tables
// call the first pass parser
// print the symbol table
// call the second pass
// close down and delete the outputs if any errors
// Return:
//	exit(2) for error, exit(0) for OK
int main(int argc, char *argv[]) {
   int grv;

   grv = cpumatch(argv[0]);

   while ((grv = getopt(argc, argv, "dh:o:l:s:p:")) != EOF) {
      switch (grv) {
         case 'o':
         case 'h':
            hexfn = optarg;
            hexflag = true;
            break;

         case 'l':
            loutfn = optarg;
            listflag = true;
            break;

         case 'd':
            debugmode = true;
            break;

         case 's':
            symbflag = true;
            symbfn = optarg;
            break;

         case 'p':
            if (!cpumatch(optarg)) {
               fprintf(stderr, "%s: no match on CPU type %s, default used\n", argv[0], optarg);
            }
            break;

         case '?':
            break;
      }
   }

   if (optind < argc) {
      if (strcmp(argv[optind], "-") == 0) {
         yyin = stdin;
      } else {
         if ((yyin = fopen(argv[optind], "r")) == (FILE *) NULL) {
            fprintf(stderr, "%s: cannot open input file %s\n", argv[0], argv[optind]);
            return 1;
         }
      }
   } else {
      fprintf(stderr, "%s: no input file\n", argv[0]);
      return 1;
   }

   if (listflag) {
      if (strcmp(argv[optind], loutfn) == 0) {
         fprintf(stderr, "%s: list file overwrites input %s\n", argv[0], loutfn);
         listflag = false;
      } else if ((loutf = fopen(loutfn, "w")) == (FILE *) NULL) {
         fprintf(stderr, "%s: cannot open list file %s\n", argv[0], loutfn);
         listflag = false;
      }
   }

   if (!listflag) {
      loutf = stdout;
   }

   mkstemp(interfn);
   if ((intermedf = fopen(interfn, "w")) == (FILE *) NULL) {
      fprintf(stderr, "%s: cannot open temp file %s\n", argv[0], interfn);
      return 1;
   }

   setophash();
   setreserved();
   ifstk[0].Else = ifstk[0].EndIf = If_Err;
   fprintf(intermedf, "F:%s\n", argv[optind]);
   infilestk[0].fpt = yyin;
   infilestk[0].fnm = argv[optind];
   currfstk = 0;
   currseg = 0;

   yyparse();

   if (ifstkpt != 0)
      fraerror("active IF at end of file");

   buildsymbolindex();
   if (listflag)
      printsymbols();

   if (symbflag) {
      if (strcmp(argv[optind], symbfn) == 0) {
         fprintf(stderr, "%s: symbol file overwrites input %s\n", argv[0], symbfn);
      } else if ((symbf = fopen(symbfn, "w")) == (FILE *) NULL) {
         fprintf(stderr, "%s: cannot open symbol file %s\n", argv[0], symbfn);
      } else {
         filesymbols();
         fclose(symbf);
      }
   }

   fclose(intermedf);
   if ((intermedf = fopen(interfn, "r")) == (FILE *) NULL) {
      fprintf(stderr, "%s: cannot open temp file %s\n", argv[0], interfn);
      return 1;
   }

   if (errorcnt > 0)
      hexflag = false;

   if (hexflag) {
      if (strcmp(argv[optind], hexfn) == 0) {
         fprintf(stderr, "%s: hex output overwrites input %s\n", argv[0], hexfn);
         hexflag = false;
      } else if ((hexoutf = fopen(hexfn, "w")) == (FILE *) NULL) {
         fprintf(stderr, "%s: cannot open hex output %s\n", argv[0], hexfn);
         hexflag = false;
      }
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

   if (listflag)
      fclose(loutf);

   if (hexflag) {
      fclose(hexoutf);
      if (errorcnt > 0)
         remove(hexfn);
   }

   fclose(intermedf);
   if (!debugmode)
      remove(interfn);
   else
      abort();

   return errorcnt > 0 ? 2 : 0;
}