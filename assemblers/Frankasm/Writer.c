// Frankenstein Cross-Assemblers, version 2.0.
// Original author: Mark Zenier.
// Output pass utility routines.
#include <stdio.h>
#include "Extern.h"
#include "Constants.h"

const int SOURCEOFFSET = 24, NUMHEXSOURCE = 6;

struct evstkel estk[0x20];
const size_t PESTKDEPTH = sizeof estk/sizeof estk[0];
int currfstk, currseg;
long locctr;

static char currentfnm[100];
static int linenumber = 0;
static char lineLbuff[INBUFFSZ];
static bool lineLflag = false;

static unsigned char outresult[0x100];
static int nextresult;
static long genlocctr, resultloc;
static char *oeptr;

// widthmask[n] = max(1, 2ⁿ - 1), for n ∈ [0, 24].
static long widthmask[] = {
   1L,
   1L, (1L << 2) - 1, (1L << 3) - 1, (1L << 4) - 1,
   (1L << 5) - 1, (1L << 6) - 1, (1L << 7) - 1, (1L << 8) - 1,
   (1L << 9) - 1, (1L << 10) - 1, (1L << 11) - 1, (1L << 12) - 1,
   (1L << 13) - 1, (1L << 14) - 1, (1L << 15) - 1, (1L << 16) - 1,
   (1L << 17) - 1, (1L << 18) - 1, (1L << 19) - 1, (1L << 20) - 1,
   (1L << 21) - 1, (1L << 22) - 1, (1L << 23) - 1, (1L << 24) - 1
};
static const size_t maskmax = sizeof widthmask/sizeof widthmask[0];

// Convert the character string pointed to by the output expression pointer to a long integer.
// Global:
// ∙	oeptr:	the output expression pointer.
// Return:
// ∙	the value.
static long dgethex(void) {
   long rv = 0;
   for (; *oeptr != '\0'; oeptr++) switch (*oeptr) {
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
         rv = (rv << 4) + (*oeptr - '0');
      break;
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': rv = (rv << 4) + (*oeptr - 'a' + 10); break;
      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': rv = (rv << 4) + (*oeptr - 'A' + 10); break;
      default: return rv;
   }
   return rv;
}

// Flush the listing line buffer before an error for that line is printed.
static void flushsourceline(void) {
   if (listflag && lineLflag)
      fputs("\t\t\t", loutf), fputs(&lineLbuff[2], loutf), lineLflag = false;
}

// Second pass ― print undefined symbol error message on the output listing device.
// If the the listing flag is false, the output device is the standard output, and the message format is different.
// Parameter:
// ∙	symp:	a pointer to a symbol table element.
// Global:
// ∙	the count of errors.
static void frp2undef(struct symel *symp) {
   if (listflag)
      flushsourceline(), fprintf(loutf, " ERROR -  undefined symbol %s\n", symp->symstr);
   else
      fprintf(loutf, "%s - line %d - ERROR - undefined symbol  %s\n", currentfnm, linenumber, symp->symstr);
   errorcnt++;
}

// Second pass ― print a warning message on the listing file, varying the format for console messages.
// Parameter:
// ∙	str:	the message.
// Global:
// ∙	the count of warnings.
static void frp2warn(char *str) {
   if (listflag)
      flushsourceline(), fprintf(loutf, " WARNING - %s\n", str);
   else
      fprintf(loutf, "%s - line %d - WARNING - %s\n", currentfnm, linenumber, str);
   warncnt++;
}

// Second pass ― print a message on the listing file.
// Parameter:
// ∙	str:	the message.
// Global:
// ∙	the count of errors.
static void frp2error(char *str) {
   if (listflag)
      flushsourceline(), fprintf(loutf, " ERROR - %s\n", str);
   else
      fprintf(loutf, "%s - line %d - ERROR - %s\n", currentfnm, linenumber, str);
   errorcnt++;
}

static long lhaddr, lhnextaddr;
static bool lhnew;
static int lhnext = 0;
static unsigned char listbuffhex[0x10];
static const size_t NUMHEXPERL = sizeof listbuffhex/sizeof listbuffhex[0];

// Print a line of hexadecimal on the listing.
// Global:
// ∙	the hex listing buffer.
static void listouthex(void) {
   if (lhnext > 0) {
      fputc(hexch((int)lhaddr >> 12), loutf), fputc(hexch((int)lhaddr >> 8), loutf);
      fputc(hexch((int)lhaddr >> 4), loutf), fputc(hexch((int)lhaddr), loutf);
      fputc(' ', loutf);
      for (int cn = 0; cn < lhnext; cn++) {
         int tc = listbuffhex[cn];
         fputc(hexch((int)tc >> 4), loutf), fputc(hexch(tc), loutf), fputc(' ', loutf);
      }
      if (!lineLflag) fputc('\n', loutf);
   }
   if (lineLflag) {
      if (lineLbuff[2] != '\n') {
         switch (lhnext) {
            case 0: fputs("\t\t\t", loutf); break;
            case 1: case 2: case 3: fputs("\t\t", loutf); break;
            case 4: case 5: case 6: fputs("\t", loutf); break;
            default: break;
         }
         fputs(&lineLbuff[2], loutf), lineLflag = false;
      } else
         fputc('\n', loutf);
   }
   lhnext = 0;
}

// Output the residue of the hexidecimal values for the previous assembler statement.
// Global:
// ∙	the new hex list flag.
static void flushlisthex(void) { listouthex(), lhnew = true; }

// Convert the polish form character string in the intermediate file 'D' line to binary values in the output result array.
// Globals:
// ∙	the output expression pointer,
// ∙	the output result array.
static void outeval(void) {
   long etop = 0;
   struct evstkel *estkm1p = estk;
   while (*oeptr != '\0') {
      switch (*oeptr) {
         case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            etop = (etop << 4) + ((*oeptr) - '0');
         break;
         case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': etop = (etop << 4) + ((*oeptr) - 'a' + 10); break;
         case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': etop = (etop << 4) + ((*oeptr) - 'A' + 10); break;
      // Unary expression {-,~,high,low} x:
         case IFC_NEG: case IFC_NOT: case IFC_HIGH: case IFC_LOW:
            etop = EvalUnOp(*oeptr, etop);
         break;
      // Binary expression x {+,-,*,/,%,<<,>>,|,^,&,>,>=,<,<=,!=,==} y:
         case IFC_ADD: case IFC_SUB: case IFC_MUL: case IFC_DIV: case IFC_MOD:
         case IFC_SHL: case IFC_SHR: case IFC_OR: case IFC_XOR: case IFC_AND:
         case IFC_GT: case IFC_GE: case IFC_LT: case IFC_LE: case IFC_NE: case IFC_EQ:
            etop = EvalBinOp(*oeptr, (estkm1p--)->v, etop);
         break;
      // Symbol:
         case IFC_SYMB: {
            struct symel *tsy = symbindex[etop];
            if (!seg_valued(tsy->seg))
               frp2undef(tsy), etop = 0;
            else {
               if (tsy->seg == SSG_EQU || tsy->seg == SSG_SET) frp2warn("forward reference to SET/EQU symbol");
               etop = tsy->value;
            }
         }
         break;
      // Current location:
         case IFC_CURRLOC: etop = genlocctr; break;
      // Program counter:
         case IFC_PROGCTR: etop = locctr; break;
      // Duplicate:
         case IFC_DUP:
            if (estkm1p >= &estk[PESTKDEPTH - 1])
               frp2error("expression stack overflow");
            else
               (++estkm1p)->v = etop;
         break;
      // Load:
         case IFC_LOAD:
            if (estkm1p >= &estk[PESTKDEPTH - 1])
               frp2error("expression stack overflow");
            else
               (++estkm1p)->v = etop;
            etop = 0;
         break;
      // Clear:
         case IFC_CLR: etop = 0; break;
      // Clear all:
         case IFC_CLRALL: etop = 0, estkm1p = estk; break;
      // Pop:
         case IFC_POP: etop = (estkm1p--)->v; break;
      // Test error:
         case IFC_TESTERR:
            if (etop) frp2error("expression fails validity test");
         break;
      // SWidth:
         case IFC_SWIDTH:
            if (etop > 0 && etop < maskmax) {
               if (estkm1p->v < -(widthmask[etop - 1] + 1) || estkm1p->v > widthmask[etop - 1])
                  frp2error("expression exceeds available field width");
               etop = ((estkm1p--)->v)&widthmask[etop];
            } else
               frp2error("unimplemented width");
         break;
      // Width:
         case IFC_WIDTH:
            if (etop > 0 && etop < maskmax) {
               if (estkm1p->v < -(widthmask[etop - 1] + 1) || estkm1p->v > widthmask[etop])
                  frp2error("expression exceeds available field width");
               etop = ((estkm1p--)->v)&widthmask[etop];
            } else
               frp2error("unimplemented width");
         break;
      // IWidth:
         case IFC_IWIDTH:
            if (etop > 0 && etop < maskmax) {
               if (estkm1p->v < 0 || estkm1p->v > widthmask[etop])
                  frp2error("expression exceeds available field width");
               etop = ((estkm1p--)->v)&widthmask[etop];
            } else
               frp2error("unimplemented width");
         break;
      // EMU8:
         case IFC_EMU8:
            if (etop >= -0x80 && etop <= 0xff)
               outresult[nextresult++] = etop&0xff;
            else
               outresult[nextresult++] = 0, frp2error("expression exceeds available field width");
            genlocctr++, etop = 0;
         break;
      // EMS7:
         case IFC_EMS7:
            if (etop >= -0x80 && etop <= 0x7f)
               outresult[nextresult++] = etop&0xff;
            else
               outresult[nextresult++] = 0, frp2error("expression exceeds available field width");
            genlocctr++, etop = 0;
         break;
      // EM16:
         case IFC_EM16:
            if (etop >= -0x8000L && etop <= 0xffffL)
               outresult[nextresult++] = (etop >> 8)&0xff, outresult[nextresult++] = etop&0xff;
            else
               outresult[nextresult++] = 0, outresult[nextresult++] = 0, frp2error("expression exceeds available field width");
            genlocctr += 2, etop = 0;
         break;
      // EMBR16:
         case IFC_EMBR16:
            if (etop >= -0x8000L && etop <= 0xffffL)
               outresult[nextresult++] = etop&0xff, outresult[nextresult++] = (etop >> 8)&0xff;
            else
               outresult[nextresult++] = 0, outresult[nextresult++] = 0, frp2error("expression exceeds available field width");
            genlocctr += 2, etop = 0;
         break;
         default: break;
      }
      oeptr++;
   }
}

static long nextoutaddr, blockaddr;
static int hnextsub;
static char hlinebuff[0x20];
static const size_t INTELLEN = sizeof hlinebuff/sizeof hlinebuff[0];

// Print a line of intel format hex data to the output file.
// Parameters:
// ∙	see manual for record description.
static void intelout(int type, long addr, int count, char data[]) {
// Start and Size:
   fputc(':', hexoutf), fputc(hexch(count >> 4), hexoutf), fputc(hexch(count), hexoutf);
// Address:
   fputc(hexch((int)addr >> 12), hexoutf), fputc(hexch((int)addr >> 8), hexoutf);
   fputc(hexch((int)addr >> 4), hexoutf), fputc(hexch((int)addr), hexoutf);
// Type:
   fputc(hexch(type >> 4), hexoutf), fputc(hexch(type), hexoutf);
// Line:
   int checksum = ((addr >> 8)&0xff) + (addr&0xff) + (count&0xff) + (type&0xff);
   for (int temp = 0; temp < count; temp++)
      checksum += data[temp]&0xff, fputc(hexch(data[temp] >> 4), hexoutf), fputc(hexch(data[temp]), hexoutf);
   checksum = (-checksum)&0xff;
// Check Sum and End:
   fputc(hexch(checksum >> 4), hexoutf), fputc(hexch(checksum), hexoutf), fputc('\n', hexoutf);
}

// Buffer the output result to group adjacent output data into longer lines.
// Globals:
// ∙	the output result array,
// ∙	the intel hex line buffer.
static void outhexblock(void) {
   long inbuffaddr = resultloc;
   static bool first = true;
   if (first) nextoutaddr = blockaddr = resultloc, hnextsub = 0, first = false;
   for (int loopc = 0; loopc < nextresult; nextoutaddr++, inbuffaddr++, loopc++) {
      if (nextoutaddr != inbuffaddr || hnextsub >= INTELLEN)
         intelout(0, blockaddr, hnextsub, hlinebuff), blockaddr = nextoutaddr = inbuffaddr, hnextsub = 0;
      hlinebuff[hnextsub++] = outresult[loopc];
   }
}

// Buffer the output result to block the hexidecimal listing on the output file to NUMHEXPERL bytes per listing line.
// Globals:
// ∙	the output result array and count,
// ∙	the hex line buffer and counts.
static void listhex(void) {
   long inhaddr = resultloc;
   if (lhnew) lhaddr = lhnextaddr = resultloc, lhnew = false;
   for (int cht = 0; cht < nextresult; lhnextaddr++, inhaddr++, cht++) {
      if (lhnextaddr != inhaddr || lhnext >= (lineLflag? NUMHEXSOURCE: NUMHEXPERL))
         listouthex(), lhaddr = lhnextaddr = inhaddr;
      listbuffhex[lhnext++] = outresult[cht];
   }
}

// Flush the intel hex line buffer at the end of the second pass.
// Global:
// ∙	the intel hex line buffer.
static void flushhex(void) {
   if (hnextsub > 0) intelout(0, blockaddr, hnextsub, hlinebuff);
   intelout(1, endsymbol != NULL && seg_valued(endsymbol->seg)? endsymbol->value: 0L, 0, "");
}

// Process all the lines in the intermediate file.
// Globals:
// ∙	the input line,
// ∙	the output expression pointer,
// ∙	line number,
// ∙	file name,
// ∙	the binary output array and counts.
void outphase(void) {
   for (int firstchar; (firstchar = fgetc(intermedf)) != EOF; ) {
      if (firstchar == 'L') {
         if (listflag) flushlisthex();
         if (fgets(&lineLbuff[1], INBUFFSZ - 1, intermedf) == NULL) {
            frp2error("error or premature end of intermediate file");
            break;
         }
         lineLflag = true;
      } else {
         finbuff[0] = firstchar;
         if (fgets(&finbuff[1], INBUFFSZ - 1, intermedf) == NULL) {
            frp2error("error or premature end of intermediate file");
            break;
         }
      }
      switch (firstchar) {
      // Error.
         case 'E':
            if (listflag)
               flushsourceline(), fputs(&finbuff[2], loutf);
            else
               fprintf(loutf, "%s - line %d - %s", currentfnm, linenumber, &finbuff[2]);
         break;
      // Listing.
         case 'L': linenumber++; break;
      // Comment/uncounted listing.
         case 'C':
            if (listflag) {
               char *stuff = strchr(finbuff, '\n'); if (stuff != NULL) *stuff = '\0';
               fprintf(loutf, "%-*.*s", SOURCEOFFSET, SOURCEOFFSET, &finbuff[2]);
               if (lineLflag)
                  fputs(&lineLbuff[2], loutf), lineLflag = false;
               else
                  fputc('\n', loutf);
            }
         break;
      // Location set.
         case 'P': oeptr = &finbuff[2], currseg = dgethex(), oeptr++, genlocctr = locctr = dgethex(); break;
      // Data.
         case 'D':
            oeptr = &finbuff[2], nextresult = 0, resultloc = genlocctr, outeval();
            if (hexflag) outhexblock();
            if (listflag) listhex();
         break;
      // File start.
         case 'F':
            for (char *tp; (tp = strchr(finbuff, '\n')) != NULL; ) *tp = '\0';
            strncpy(currentfnm, &finbuff[2], 100), currentfnm[99] = '\0';
            infilestk[currfstk++].line = linenumber, linenumber = 0;
         break;
      // File resume.
         case 'X':
            for (char *tp; (tp = strchr(finbuff, '\n')) != NULL; ) *tp = '\0';
            strncpy(currentfnm, &finbuff[2], 100), currentfnm[99] = '\0';
            linenumber = infilestk[--currfstk].line;
         break;
         default: frp2error("unknown intermediate file command"); break;
      }
   }
   if (hexflag) flushhex();
   if (listflag) flushlisthex();
}
