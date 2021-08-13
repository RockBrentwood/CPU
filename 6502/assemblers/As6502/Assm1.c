/*    As6502 main routine: assm1.c     */
#include <stdio.h>
#include <ctype.h>
#include "Assm1.h"
#include "Assm2.h"
#include <time.h>
#include <stdlib.h>

#define CPMEOF EOF

/*  Version 5.0 ported to the Amiga 3/1/87 by Joel Swank                */

/*  Version 5.0 is a major revision by Joel Swank. It adds the following
 *  features: '.PAGE' pseudo with optional title; automatic paging and
 *  -p flag to specify page length; A sorted symbol cross reference;
 *  -t flag to specify the symbol table size; -w flag to specify the width
 *  of a listing line. The -m option causes the object file to be formatted
 *  as a standard MOS Technology object file. Also added error checking
 *  and error messages to the argument parsing routine.
 */

/*
 *  Two changes to version 1.4 have been made to "port" as6502 to CP/M(tm).
 *  A "tolower()" function call was add to the command line processing
 *  code to (re)map the command line arguments to lower case (CP/M
 *  converts all command line arguments to upper case).  The readline()
 *  function has code added to "ignore" the '\r' character (CP/M includes
 *  the \r character along with \n).
 *
 *  Also, the ability to process multiple files on the command line has been
 *  added.  Now one can do, for example:
 *
 *	as6502 -nisvo header.file source.file data.file ...
 *
 *    George V. Wilder
 *	IX 1A-360 x1937
 *	ihuxp!gvw1
 */
 /*   Had to take out tolower call to work on 4.2bsd.  Joel Swank 5/9/85 */
 /*   Added USAGE message in place of Invalid count message JS 12/2/86   */

int badflag;
int act;
char **avt;

/*****************************************************************************/

/*   parse the command args and save data   */

getargs(argc, argv)
int argc;
char *argv[];
{
   int i;
   char c;
   int sz;
   while (--argc > 0 && (*++argv)[0] == '-') {
      for (i = 1; (c = (*argv)[i]) != '\0'; i++) {
         switch (c) {
            case 'd': /* debug flag */
               dflag++;
               break;
            case 'i': /* ignore .nlst flag */
               iflag++;
               break;
            case 'l': /* disable listing flag */
               lflag--;
               break;
            case 'n': /* normal/split address mode */
               nflag++;
               break;
            case 'o': /* object output flag */
               oflag++;
               break;
            case 'm': /* MOS Tech. object format  */
               mflag++;
               oflag++; /* -m implies -o     */
               break;
            case 's': /* list symbol table flag */
               sflag++;
               break;
            case 'v': /* print assembler version */
               fprintf(stderr, "as6502 - Amiga version 5.0 - 3/1/87 - JHV [gvw,jhs]\n");
               break;
            case 't': /* input symbol table size */
            {
               if ((*argv)[++i] == '\0') {
                  ++argv;
                  argc--;
                  sz = atoi(*argv);
               } else sz = atoi(&(*argv)[i]);
               if (sz > 1000) size = sz;
               else {
                  fprintf(stderr, "Invalid Symbol table size - minimum is 1000\n");
                  badflag++;
               }
               goto outofloop;
            }
            case 'p': /* input lines per page */
            {
               if ((*argv)[++i] == '\0') {
                  ++argv;
                  argc--;
                  sz = atoi(*argv);
               } else sz = atoi(&(*argv)[i]);
               if (sz > 10 || sz == 0) pagesize = sz;
               else {
                  fprintf(stderr, "Invalid Pagesize - minimum is 10\n");
                  badflag++;
               }
               goto outofloop;
            }
            case 'w': /* input characters per line */
            {
               if ((*argv)[++i] == '\0') {
                  ++argv;
                  argc--;
                  sz = atoi(*argv);
               } else sz = atoi(&(*argv)[i]);
               if (sz >= 80 && sz < 133) linesize = sz;
               else {
                  fprintf(stderr, "Invalid Linesize - min is 80, max is 133\n");
                  badflag++;
               }
               goto outofloop;
            }
            default:
               fprintf(stderr, "Unknown flag '%c'\n", c);
               badflag++;
         } /* end switch */
      } /* end for  */
    outofloop:;
   }
   act = argc; /* return values to main */
   avt = argv;
}

/*****************************************************************************/

/* initialize opens files */

initialize(ac, av, argc)
int ac;
char *av[];
int argc;
{

   if ((iptr = fopen(*av, "r")) == NULL) {
      fprintf(stderr, "Open error for file '%s'.\n", *av);
      exit(1);
   }
   if ((pass == LAST_PASS) && (oflag != 0) && ac == argc) {
      if ((optr = fopen("6502.out", "w")) == NULL) {
         fprintf(stderr, "Create error for object file 6502.out.\n");
         exit(1);
      }
   }
}

int field[] = {
   SFIELD,
   SFIELD + 8,
   SFIELD + 14,
   SFIELD + 23,
   SFIELD + 43,
   SFIELD + 75
};

/*    clear the print buffer      */

clrlin() {
   int i;
   for (i = 0; i < LAST_CH_POS; i++)
      prlnbuf[i] = ' ';
}

/* readline reads and formats an input line	*/

readline() {
   int i; /* pointer into prlnbuf */
   int j; /* pointer to current field start       */
   int ch; /* current character            */
   int cmnt; /* comment line flag    */
   int spcnt; /* consecutive space counter    */
   int string; /* ASCII string flag    */
   int temp1; /* temp used for line number conversion */

   temp1 = ++slnum;
   clrlin();
   i = 3;
   while (temp1 != 0) { /* put source line number into prlnbuf */
      prlnbuf[i--] = temp1 % 10 + '0';
      temp1 /= 10;
   }
   i = SFIELD;
   cmnt = spcnt = string = 0;
   j = 1;
   while ((ch = getc(iptr)) != '\n') {
      if (ch == '\r')
         continue;
      prlnbuf[i++] = ch;
      if ((ch == ' ') && (string == 0)) {
         if (spcnt != 0)
            --i;
         else if (cmnt == 0) {
            ++spcnt;
            if (i < field[j])
               i = field[j];
            if (++j > 3) {
               spcnt = 0;
               ++cmnt;
            }
         }
      } else if (ch == '\t') {
         prlnbuf[i - 1] = ' ';
         spcnt = 0;
         if (cmnt == 0) {
            if (i < field[j])
               i = field[j];
            if (++j > 3)
               ++cmnt;
         } else i = (i + 8) & 0x78;
      } else if ((ch == ';') && (string == 0)) {
         spcnt = 0;
         if (i == SFIELD + 1)
            ++cmnt;
         else if (prlnbuf[i - 2] != '\'') {
            ++cmnt;
            prlnbuf[i - 1] = ' ';
            if (i < field[3])
               i = field[3];
            prlnbuf[i++] = ';';
         }
      } else if (ch == EOF || ch == CPMEOF)
         return (-1);
      else {
         if ((ch == '"') && (cmnt == 0))
            string = string ^ 1;
         spcnt = 0;
         if (i >= LAST_CH_POS - 1)
            --i;
      }
   }
   prlnbuf[i] = 0;
   return (0);
}

/*
 * wrapup() closes the source, object and stdout files
 */

wrapup() {

   fclose(iptr); /* close source file */
   if (pass == DONE) {
      if ((oflag != 0) && (optr != 0)) {
         if (mflag != 0) fin_obj();
         else fputc('\n', optr);
         fclose(optr);
      }
   }
   return;
}

/****************************************************************************/

/* prsymhead prints the page heading   */

prsymhead() {
   if (pagesize == 0) return;
   pagect++;
   fprintf(stdout, "\f\nAmiga 6502 assembler :  Symbol Cross Reference - %s PAGE %d\n", syspc, pagect);
   fprintf(stdout, "Symbol                Value Defined References\n\n");
   paglin = 0;
}

/* prsyline prints the contents of prlnbuf */

prsyline() {
   if (paglin == pagesize) prsymhead();
   prlnbuf[linesize] = '\0';
   fprintf(stdout, "%s\n", prlnbuf);
   paglin++;
   clrlin();
}

/* symbol table print
 */

stprnt() {
   int i; /* print line position */
   int ptr; /* symbol table position */
   int j; /* integer conversion variable */
   int k; /* printf buffer pointer */
   int refct; /* counter for references  */
   char buf[6];
   paglin = pagesize;
   ptr = 0;
   clrlin();
   while (ptr < nxt_free) {
      for (i = 1; i <= symtab[ptr]; i++) prlnbuf[i] = symtab[ptr + i];
      ptr += i + 1;
      i = 23; /* value at pos 23  */
      j = symtab[ptr++] & 0xff;
      j += (symtab[ptr++] << 8);
      hexcon(4, j);
      if (nflag == 0) {
         i--;
         prlnbuf[i++] = hex[3];
         prlnbuf[i++] = hex[4];
         prlnbuf[i++] = ':';
         prlnbuf[i++] = hex[1];
         prlnbuf[i++] = hex[2];
         } else for (k = 1; k < 5; k++) prlnbuf[i++] = hex[k];
      j = symtab[ptr++] & 0xff;
      j += (symtab[ptr++] << 8);
      sprintf(buf, "%d", j);
      k = 0;
      i = 30; /* line defined at pos 30 */
      while (buf[k] != '\0') prlnbuf[i++] = buf[k++];
      k = 0;
      i = 37; /* count of references    */
      refct = symtab[ptr++] & 0xff;
      sprintf(buf, "(%d)", refct);
      while (buf[k] != '\0') prlnbuf[i++] = buf[k++];
      i++; /* and all the references   */
      while (refct > 0) {
         j = symtab[ptr++] & 0xff;
         j += (symtab[ptr++] << 8);
         sprintf(buf, "%d", j);
         k = 0;
         while (buf[k] != '\0') prlnbuf[i++] = buf[k++];
         i++;
         refct--;
         if (i > linesize - 5 && refct > 0) {
            prlnbuf[i] = '\0';
            prsyline();
            i = 37;
         }
      }
      prlnbuf[i] = '\0';
      prsyline();
   }

}

main(argc, argv)
int argc;
char *argv[];
{
   int cnt;
   int i;
   int ac;
   char **av;
   long l;
   size = STABSZ;
   pagesize = PAGESIZE;
   linesize = LINESIZE;
   getargs(argc, argv); /*   parse the command line arguments   */
   if (badflag > 0) exit(1);
   if (act == 0) {
      fprintf(stderr, "USAGE: as6502 -[milnosv] [-p -t -w] file ...\n");
      exit(1);
   }
   symtab = malloc(size);
   if (symtab == 0) {
      fprintf(stderr, "Symbol table allocation failed - specify a smaller size\n");
      exit(2);
   }
   time(&l);
   date = ctime(&l);
   date[24] = '\0';
   pagect = 0;
   paglin = pagesize;
   titlesize = linesize - 36;
   for (i = 0; i < 100; i++) titlbuf[i] = ' ';
   titlbuf[titlesize] = '\0';
   for (i = 0; i < linesize - 58; i++) syspc[i] = ' ';
   syspc[i] = '\0';
   ac = act;
   av = avt;
   pass = FIRST_PASS;
   errcnt = loccnt = slnum = 0;
   fprintf(stderr, "Initialization complete\n");
   while (pass != DONE) {
      initialize(ac, av, act);
      fprintf(stderr, "PASS %d %s\n", pass + 1, *av);
      if (pass == LAST_PASS && ac == act)
         errcnt = loccnt = slnum = 0;
   /* lower level routines can terminate assembly by setting
      pass = DONE  ('symbol table full' does this)   */
      while (readline() != -1 && pass != DONE)
         assemble(); /* rest of assembler executes from here */
      if (errcnt != 0) {
         pass = DONE;
         fprintf(stderr, "Terminated with error counter = %d\n", errcnt);
      }
      switch (pass) {
         case FIRST_PASS:
            --ac;
            ++av;
            if (ac == 0) {
               pass = LAST_PASS;
               if (lflag == 0)
                  lflag++;
               ac = act;
               av = avt;
            }
            break;
         case LAST_PASS:
            --ac;
            ++av;
            if (ac == 0) {
               pass = DONE;
               if (sflag != 0)
                  stprnt();
            }
      }
      wrapup();
      if ((dflag != 0) && (pass == LAST_PASS)) {
         fprintf(stdout, "nxt_free = %d\n", nxt_free);
         cnt = 0;
      }
   }
   fclose(stdout);
   free(symtab);
   return (0);
}
