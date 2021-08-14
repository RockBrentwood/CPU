// As6502 main routine.
// First Author: J. H. Van Ornum, AT&T Bell Laboratories.
// Second author: George V. Wilder
// Third author: Joel Swank
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include "Constants.h"
#include "Extern.h"

#define CPMEOF EOF

static bool badflag;
static int act; static char **avt;

static FILE *iptr;
FILE *optr;
static bool dflag; // debug flag
static bool sflag; // symbol table output flag

int errcnt; // error counter
bool iflag; // ignore .nlst flag
int lflag; // disable listing flag
bool mflag; // generate MOS Technology object format
bool nflag; // normal/split address mode
bool oflag; // object output flag

int nxt_free; // next free location in symtab
int pass; // pass counter

// Added by Joel Swank (1986/12).
int pagect; // count of pages
int paglin; // lines printed on current page
int pagesize; // maximum lines per page
int linesize; // maximum characters per line
int titlesize; // maximum characters per line
char titlbuf[100]; // buffer for title from .page
static char syspc[80]; // variable filler for heading
char *date; // pointer to formatted date string

static char *App;

// Parse the command args and the save data.
static void getargs(int argc, char *argv[]) {
   App = argc < 1? NULL: argv[0]; if (App == NULL || *App == '\0') App = "As6502";
   while (--argc > 0 && (*++argv)[0] == '-') {
      for (int i = 1, c; (c = (*argv)[i]) != '\0'; i++) switch (c) {
      // Debug flag.
         case 'd': dflag = true; break;
      // Ignore .nlst flag.
         case 'i': iflag = true; break;
      // Disable listing flag.
         case 'l': lflag--; break;
      // Normal/split address mode.
         case 'n': nflag = true; break;
      // MOS Tech. object format; -m implies -o.
         case 'm':
            mflag = true;
      // Object output flag.
         case 'o':
            oflag = true;
         break;
      // List symbol table flag.
         case 's': sflag = true; break;
      // Print the assembler version.
         case 'v': fprintf(stderr, "%s - Amiga version 5.0 - 3/1/87 - JHV [gvw,jhs]\n", App); break;
      // Input symbol table size.
         case 't': {
            int sz;
            if ((*argv)[++i] == '\0')
               ++argv, argc--, sz = atoi(*argv);
            else
               sz = atoi(&(*argv)[i]);
            if (sz > 1000)
               size = sz;
            else
               fprintf(stderr, "Invalid Symbol table size - minimum is 1000\n"), badflag = true;
         }
         goto Break;
      // Input lines per page.
         case 'p': {
            int sz;
            if ((*argv)[++i] == '\0')
               ++argv, argc--, sz = atoi(*argv);
            else
               sz = atoi(&(*argv)[i]);
            if (sz > 10 || sz == 0)
               pagesize = sz;
            else
               fprintf(stderr, "Invalid Pagesize - minimum is 10\n"), badflag = true;
         }
         goto Break;
      // Input characters per line.
         case 'w': {
            int sz;
            if ((*argv)[++i] == '\0')
               ++argv, argc--, sz = atoi(*argv);
            else
               sz = atoi(&(*argv)[i]);
            if (sz >= 80 && sz < 133)
               linesize = sz;
            else
               fprintf(stderr, "Invalid Linesize - min is 80, max is 133\n"), badflag = true;
         }
         goto Break;
         default: fprintf(stderr, "Unknown flag '%c'\n", c), badflag = true; break;
      }
   Break:;
   }
// Return values to main.
   act = argc, avt = argv;
}

// Initialize opens files.
static void initialize(int ac, char *av[], int argc) {
   iptr = fopen(*av, "r");
   if (iptr == NULL) fprintf(stderr, "Open error for file '%s'.\n", *av), exit(1);
   if (pass == LAST_PASS && oflag && ac == argc) {
      optr = fopen("6502.out", "w");
      if (optr == NULL) fprintf(stderr, "Create error for object file 6502.out.\n"), exit(1);
   }
}

static int field[] = { SFIELD, SFIELD + 8, SFIELD + 14, SFIELD + 23, SFIELD + 43, SFIELD + 75 };

// Clear the print buffer.
static void clrlin(void) {
   for (int i = 0; i < LAST_CH_POS; i++) prlnbuf[i] = ' ';
}

// Read and format an input line.
static int readline(void) {
// temp used for line number conversion.
   int temp1 = ++slnum;
   clrlin();
// Pointer into prlnbuf.
   int i = 3;
   for (; temp1 != 0; temp1 /= 10) // Put the source line number into prlnbuf.
      prlnbuf[i--] = temp1%10 + '0';
   i = SFIELD;
// Comment line flag.
   int cmnt = 0;
   int spcnt = 0;
// Consecutive space counter.
// ASCII string flag.
   int string = 0;
// Pointer to the current field start.
   int j = 1;
   for (int ch; (ch = getc(iptr)) != '\n'; ) if (ch != '\r') {
      prlnbuf[i++] = ch;
      if (ch == ' ' && string == 0) {
         if (spcnt != 0) --i;
         else if (cmnt == 0) {
            ++spcnt;
            if (i < field[j]) i = field[j];
            if (++j > 3) spcnt = 0, ++cmnt;
         }
      } else if (ch == '\t') {
         prlnbuf[i - 1] = ' ', spcnt = 0;
         if (cmnt == 0) {
            if (i < field[j]) i = field[j];
            if (++j > 3) ++cmnt;
         } else i = (i + 8)&0x78;
      } else if (ch == ';' && string == 0) {
         spcnt = 0;
         if (i == SFIELD + 1) ++cmnt;
         else if (prlnbuf[i - 2] != '\'') {
            ++cmnt, prlnbuf[i - 1] = ' ';
            if (i < field[3]) i = field[3];
            prlnbuf[i++] = ';';
         }
      } else if (ch == EOF || ch == CPMEOF) return -1;
      else {
         if (ch == '"' && cmnt == 0) string ^= 1;
         spcnt = 0;
         if (i >= LAST_CH_POS - 1) --i;
      }
   }
   prlnbuf[i] = 0;
   return 0;
}

// Close the source, object and stdout files.
static void wrapup(void) {
   fclose(iptr); // Close the source file.
   if (pass == DONE && oflag && optr != 0) {
      if (mflag) fin_obj(); else fputc('\n', optr);
      fclose(optr);
   }
}

// Print the page heading.
static void prsymhead(void) {
   if (pagesize == 0) return;
   pagect++;
   fprintf(stdout,
      "\f\nAmiga 6502 assembler :  Symbol Cross Reference - %s PAGE %d\n"
      "Symbol                Value Defined References\n"
      "\n",
      syspc, pagect
   );
   paglin = 0;
}

// Print the contents of prlnbuf.
static void prsyline(void) {
   if (paglin == pagesize) prsymhead();
   prlnbuf[linesize] = '\0';
   fprintf(stdout, "%s\n", prlnbuf);
   paglin++;
   clrlin();
}

// Symbol table print.
static void stprnt(void) {
   paglin = pagesize;
// Symbol table position.
   int ptr = 0;
   clrlin();
   while (ptr < nxt_free) {
   // Print line position.
      int i = 1;
      for (; i <= symtab[ptr]; i++) prlnbuf[i] = symtab[ptr + i];
      ptr += i + 1;
      i = 23; /* value at pos 23  */
   // Integer conversion variable.
      int j = symtab[ptr++]&0xff; j += (symtab[ptr++] << 8);
      hexcon(4, j);
      if (nflag)
         for (int k = 1; k < 5; k++) prlnbuf[i++] = hex[k];
      else
         i--, prlnbuf[i++] = hex[3], prlnbuf[i++] = hex[4], prlnbuf[i++] = ':', prlnbuf[i++] = hex[1], prlnbuf[i++] = hex[2];
      j = symtab[ptr++]&0xff;
      j += (symtab[ptr++] << 8);
      char buf[6]; sprintf(buf, "%d", j);
   // Printf buffer pointer.
      int k = 0;
      i = 30; /* line defined at pos 30 */
      while (buf[k] != '\0') prlnbuf[i++] = buf[k++];
      k = 0;
      i = 37; /* count of references    */
   // Counter for references.
      int refct = symtab[ptr++]&0xff;
      sprintf(buf, "(%d)", refct);
      while (buf[k] != '\0') prlnbuf[i++] = buf[k++];
      i++; /* and all the references   */
      while (refct > 0) {
         j = symtab[ptr++]&0xff, j += (symtab[ptr++] << 8);
         sprintf(buf, "%d", j);
         k = 0;
         while (buf[k] != '\0') prlnbuf[i++] = buf[k++];
         i++;
         refct--;
         if (i > linesize - 5 && refct > 0) prlnbuf[i] = '\0', prsyline(), i = 37;
      }
      prlnbuf[i] = '\0', prsyline();
   }

}

int main(int argc, char *argv[]) {
   size = STABSZ, pagesize = PAGESIZE, linesize = LINESIZE;
   getargs(argc, argv); // Parse the command line arguments.
   if (badflag) return 1;
   if (act == 0) {
      fprintf(stderr, "USAGE: %s -[milnosv] [-p -t -w] file ...\n", App);
      return 1;
   }
   symtab = malloc(size);
   if (symtab == 0) {
      fprintf(stderr, "Symbol table allocation failed - specify a smaller size\n");
      return 2;
   }
   long l; time(&l); date = ctime(&l), date[24] = '\0';
   pagect = 0, paglin = pagesize, titlesize = linesize - 36;
   for (int i = 0; i < 100; i++) titlbuf[i] = ' ';
   titlbuf[titlesize] = '\0';
   int i = 0;
   for (; i < linesize - 58; i++) syspc[i] = ' ';
   syspc[i] = '\0';
   int ac = act; char **av = avt;
   errcnt = loccnt = slnum = 0;
   fprintf(stderr, "Initialization complete\n");
   for (pass = FIRST_PASS; pass != DONE; ) {
      initialize(ac, av, act);
      fprintf(stderr, "PASS %d %s\n", pass + 1, *av);
      if (pass == LAST_PASS && ac == act) errcnt = loccnt = slnum = 0;
   // Lower-level routines can terminate assembly by setting pass = DONE ('symbol table full' does this).
      while (readline() != -1 && pass != DONE) assemble(); // The rest of the assembler executes from here.
      if (errcnt != 0) pass = DONE, fprintf(stderr, "Terminated with error counter = %d\n", errcnt);
      switch (pass) {
         case FIRST_PASS:
            --ac, ++av;
            if (ac == 0) {
               pass = LAST_PASS;
               if (lflag == 0) lflag++;
               ac = act, av = avt;
            }
         break;
         case LAST_PASS:
            --ac, ++av;
            if (ac == 0) {
               pass = DONE;
               if (sflag) stprnt();
            }
         break;
      }
      wrapup();
      if (dflag && pass == LAST_PASS) fprintf(stdout, "nxt_free = %d\n", nxt_free);
   }
   fclose(stdout);
   free(symtab);
   return 0;
}
