// Frankenstein Cross-Assemblers, version 2.0.
// Original author: Mark Zenier.
// Parser phase utility routines.
#include <stdio.h>
#include "Extern.h"
#include "Constants.h"

// Save a character string in permanent (interpass) memory
// Parameters:
//	the string and its length
// Globals:
//	the string pool
// Return:
//	a pointer to the saved string
char *savestring(char *stx, int len) {
   const size_t STRALLOCSZ = 0x1000;
   static char *currstr;
   static int savestrleft = 0;

   if (savestrleft < (len + 1)) {
      if ((currstr = malloc(STRALLOCSZ)) == (char *)NULL) {
         frafatal("cannot allocate string storage");
      }
      savestrleft = STRALLOCSZ;
   }

   savestrleft -= (len + 1);

   char *rv = currstr;
   for (; len > 0; len--)
      *currstr++ = *stx++;
   *currstr++ = '\0';

   return rv;
}

/* expression node operations */

/* expression tree element */
struct etelem {
   extag evs;
   int op;
   int left, right;
   long val;
   struct symel *sym;
};

struct etelem enode[INBUFFSZ];
static const size_t NUMENODE = sizeof enode/sizeof enode[0];

static int nextenode = 1;

/* non general, one exprlist or stringlist per line */
int nextexprs = 0;
int exprlist[INBUFFSZ/2];

int nextstrs = 0;
char *stringlist[INBUFFSZ/2];

// Clear out the stuff used for each line
//	the temporary string pool
//	the expression tree storage pool
//	the string and expression lists
void clrexpr(void) {
   nextenode = 1;
   nextexprs = nextstrs = 0;
}

// Add an element to the expression tree pool
// Parameters:
//	swact, the action performed by the switch in
//		the polish conversion routine, the category
//		of the expression node.
//	left, right  the subscripts of the decendent nodes
//		of the expression tree element
//	op, the operation to preform
//	value, a constant value (maybe)
//	symbol, a pointer to a symbol element (maybe)
// Globals:
//	the next available table element
// Return:
//	the subscript of the expression node
int exprnode(extag swact, int left, int op, int right, long value, struct symel *symbol) {
   if (nextenode >= NUMENODE) {
      frafatal("excessive number of subexpressions");
   }

   enode[nextenode].evs = swact;
   enode[nextenode].left = left;
   enode[nextenode].op = op;
   enode[nextenode].right = right;
   enode[nextenode].val = value;
   enode[nextenode].sym = symbol;

   return nextenode++;
}

struct symel **symbindex, *endsymbol;
int nextsymnum = 1;

static struct symel *syallob;
static const size_t SYELPB = 0x200;
static int nxtsyel = SYELPB;

// Allocate a symbol table element, and allocate
// a block if the current one is empty.  A fatal
// error if no more space can be gotten
// Globals:
//	the pointer to the current symbol table block
//	the count of elements used in the block
// Return:
//	a pointer to the symbol table element
static struct symel *allocsym(void) {

   if (nxtsyel >= SYELPB) {
      if ((syallob = (struct symel *)calloc(SYELPB, sizeof(struct symel)))
         == (struct symel *)NULL) {
         frafatal("cannot allocate symbol space");
      }

      nxtsyel = 0;
   }

   return &syallob[nxtsyel++];
}

#define SYHASHOFF 13
#define SYHASHSZ 0x3ff
static struct symel *shashtab[SYHASHSZ];

// Produce a hash index from a character string for
// the symbol table.
// Parameters:
//	a character string
// Return:
//	an integer related in some way to the character string
static int syhash(char *str) {
   unsigned rv = 0;
   int offset = 1;

   for (int c; (c = *(str++)) > 0; ) {
      rv += (c - ' ') * offset;
      offset *= SYHASHOFF;
   }

   return rv % SYHASHSZ;
}

// Find an existing symbol in the symbol table, or
// allocate an new element if the symbol doen't exist.
// Action:
//	hash the string
//	if there are no symbols for the hash value
//		create one for this string
//	otherwise
//		scan the linked list until the symbol is
//		found or the end of the list is found
//		if the symbol was found
//			exit
//		if the symbol was not found, allocate and
//		add at the end of the linked list
//		fill out the symbol
// Parameters:
//	the character string
// Globals:
//	all the symbol table
// Return:
//	a pointer to the symbol table element for this
//	character string
static struct symel *getsymslot(char *str) {
   int hv = syhash(str);
   struct symel *currel = shashtab[hv];
   if (currel == (struct symel *)NULL) {
      shashtab[hv] = currel = allocsym();
   } else {
      struct symel *prevel;

      do {
         if (strcmp(currel->symstr, str) == 0) {
            return currel;
         } else {
            prevel = currel;
            currel = currel->nextsym;
         }
      } while (currel != (struct symel *)NULL);

      prevel->nextsym = currel = allocsym();
   }

   currel->symstr = savestring(str, strlen(str));
   currel->nextsym = (struct symel *)NULL;
   currel->tok = 0;
   currel->value = 0;
   currel->seg = SSG_UNUSED;

   return currel;
}

// Find or add a nonreserved symbol to the symbol table
// Parameters:
//	the character string
//	the syntactic token type for this charcter string
//	(this is a parameter so the routine doesn't
//	have to be recompiled since the yacc grammer
//	provides the value)
// Globals:
//	the symbol table in all its messy glory
// Return:
//	a pointer to the symbol table element
struct symel *symbentry(char *str, int toktyp) {
   struct symel *rv = getsymslot(str);

   if (rv->seg == SSG_UNUSED) {
      rv->tok = toktyp;
      rv->symnum = nextsymnum++;
      rv->seg = SSG_UNDEF;
   }

   return rv;
}

// Add a reserved symbol to the symbol table.
// Parameters:
//	the character string, must be a constant as
//	the symbol table does not copy it, only point to it.
//	The syntactic token value.
//	The associated value of the symbol.
void reservedsym(char *str, int tok, int value) {
   struct symel *tv = getsymslot(str);

   if (tv->seg != SSG_UNUSED) {
      frafatal("cannot redefine reserved symbol");
   }

   tv->symnum = 0;
   tv->tok = tok;
   tv->seg = SSG_RESV;
   tv->value = value;

}

// Allocate and fill an array that points to each
// nonreserved symbol table element, used to reference
// the symbols in the intermediate file, in the output
// pass.
// Globals:
//	the symbol table
void buildsymbolindex(void) {
   if ((symbindex = (struct symel **)calloc((unsigned)nextsymnum, sizeof(struct symel *))) == (struct symel **)NULL) {
      frafatal(" unable to allocate symbol index");
   }

   for (int hi = 0; hi < SYHASHSZ; hi++) {
      struct symel *curr = shashtab[hi];
      if (curr != NULL) {
         do {
            if (curr->symnum)
               symbindex[curr->symnum] = curr;

            curr = curr->nextsym;
         } while (curr != NULL);
      }
   }
}

/* opcode symbol table */
static int ohashtab[0x3ff];
static const size_t OPHASHSZ = sizeof ohashtab/sizeof ohashtab[0];

// Hash a character string
// Return:
//	an integer related somehow to the character string
static int opcodehash(char *str) {
   const int OPHASHOFF = 13;
   unsigned rv = 0;
   int offset = 1;

   for (int c; (c = *(str++)) > 0; ) {
      rv += (c - ' ') * offset;
      offset *= OPHASHOFF;
   }

   return rv % OPHASHSZ;
}

// Set up the linked list hash table for the
// opcode symbols
// Globals:
//	the opcode hash table
//	the opcode table
void setophash(void) {
/* optab[0] is reserved for the "invalid" entry */
/*  opcode subscripts range from 0 to numopcode - 1 */
   for (int opn = 1; opn < gnumopcode; opn++) {
      int hv = opcodehash(optab[opn].opstr);

      int pl = ohashtab[hv];
      if (pl == 0) {
         ohashtab[hv] = opn;
      } else {
         while (ophashlnk[pl] != 0) {
            pl = ophashlnk[pl];
         }

         ophashlnk[pl] = opn;
         ophashlnk[opn] = 0;
      }
   }
}

// Find an opcode table subscript
// Parameters:
//	the character string
// Globals:
//	the opcode hash linked list table
//	the opcode table
// Return:
//	0 if not found
//	the subscript of the matching element if found
int findop(char *str) {
   int ts = ohashtab[opcodehash(str)];
   if (ts == 0) {
      return 0;
   }

   do {
      if (strcmp(str, optab[ts].opstr) == 0) {
         return ts;
      } else {
         ts = ophashlnk[ts];
      }
   } while (ts != 0);

   return 0;
}

// Given the subscript of the opcode table element,
// find the instruction generation string for the
// opcode with the given syntax and fitting the
// given criteria.  This implement a sparse matrix
// for  the dimensions [opcode, syntax] and then
// points to a list of generation elements that
// are matched to the criteria (binary set) that
// are provided by the action in the grammer for that
// specific syntax.
// Parameters:
//	Opcode table subscript
//		note 0 is the value which points to an
//		syntax list that will accept anything
//		and gives the invalid instruction error
//	Syntax, a selector, a set member
//	Criteria, a integer used a a group of bit sets
// Globals:
//	the opcode table, the opcode syntax table, the
//	instruction generation table
// Return:
//	a pointer to a character string, either a
//	error message, or the generation string for the
//	instruction
char *findgen(int op, int syntax, int crit) {
   int sys = optab[op].subsyn, gsub = 0;

   for (int stc = optab[op].numsyn; stc > 0; stc--) {
      if ((ostab[sys].syntaxgrp & syntax) != 0) {
         gsub = ostab[sys].gentabsub;
         break;
      } else
         sys++;
   }

   if (gsub == 0)
      return ignosyn;

   for (int dctr = ostab[sys].elcnt; dctr > 0; dctr--) {
      if ((igtab[gsub].selmask & crit) == igtab[gsub].criteria) {
         return igtab[gsub].genstr;
      } else {
         gsub++;
      }
   }

   return ignosel;
}

// Output to the intermediate file, a 'P' record
// giving the current location counter.  Segment
// is not used at this time.
void genlocrec(int seg, long loc) {
   fprintf(intermedf, "P:%x:%lx\n", seg, loc);
}

static char *goutptr, goutbuff[INBUFFSZ] = "D:";
static char *goutend = goutbuff + sizeof goutbuff/sizeof goutbuff[0] - 1;

// Put a character in the intermediate file buffer
// for 'D' data records
// Globals:
//	the buffer, its current position pointer
static void goutch(char ch) {
   if (goutptr < goutend) {
      *goutptr++ = ch;
   } else {
      *goutend = '\0';
      goutptr = goutend + 1;
      fraerror("overflow in instruction generation");
   }
}

// Output to the 'D' buffer, a byte in ASCII hexidecimal
static void gout2hex(int inv) {
   goutch(hexch(inv >> 4));
   goutch(hexch(inv));
}

// Output to the 'D' record buffer a long integer in
// hexidecimal
static void goutxnum(unsigned long num) {
   if (num > 15)
      goutxnum(num >> 4);
   goutch(hexch((int)num));
}

struct evalrel evalr[6];

// Process an instruction generation string, from
// the parser, into a polish form expression line
// in a 'D' record in the intermediate file, after
// merging in the expression results.
// Parameters:
//	the instruction generation string
// Globals:
//	the evaluation results
//		evalr[].value	a numeric value known at
//			the time of the first pass
//		evalr[].exprstr  a polish form expression
//			derived from the expression
//			parse tree, to be evaluated in
//			the output phase.
// Return:
//	the length of the instruction (machine code bytes)
int geninstr(char *str) {
   const bool GSTR_PASS = false, GSTR_PROCESS = true;
   int len = 0;
   bool state = GSTR_PASS;
   int innum = 0;

   goutptr = &goutbuff[2];

   while (*str != '\0') {
      if (state == GSTR_PASS) {
         switch (*str) {
            case IG_START:
               state = GSTR_PROCESS;
               innum = 0;
               str++;
               break;

            case IFC_EMU8:
            case IFC_EMS7:
               len++;
               goutch(*str++);
               break;

            case IFC_EM16:
            case IFC_EMBR16:
               len += 2;
               goutch(*str++);
               break;

            default:
               goutch(*str++);
               break;
         }
      } else {
         switch (*str) {
            case IG_END:
               state = GSTR_PASS;
               str++;
               break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
               innum = (innum << 4) + (*str++) - '0';
               break;

            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
               innum = (innum << 4) + (*str++) - 'a' + 10;
               break;

            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
               innum = (innum << 4) + (*str++) - 'A' + 10;
               break;

            case IG_CPCON:
               goutxnum((unsigned long)evalr[innum].value);
               innum = 0;
               str++;
               break;

            case IG_CPEXPR: {
               char *exp = &evalr[innum].exprstr[0];
               innum = 0;
               while (*exp != '\0')
                  goutch(*exp++);
               str++;
            }
               break;

            case IG_ERROR:
               fraerror(++str);
               return 0;

            default:
               fraerror("invalid char in instruction generation");
               break;
         }
      }
   }

   if (goutptr > &goutbuff[2]) {
      goutch('\n');
      fwrite(goutbuff, sizeof(char), goutptr - &goutbuff[0], intermedf);
   }

   return len;
}

int chtnxalph = 1, *chtcpoint = NULL, *chtnpoint = NULL;
int *chtatab[6];

// Allocate and initialize a character translate
// table
// Return:
//	0 for error, subscript into chtatab to pointer
//	to the allocated block
int chtcreate(void) {
   const size_t NUM_CHTA = sizeof chtatab/sizeof chtatab[0];

   if (chtnxalph >= NUM_CHTA)
      return 0; /* too many */

   int *trantab = (int *)calloc(512, sizeof(int));
   if (trantab == (int *)NULL)
      return 0;

   for (int cnt = 0; cnt < 512; cnt++)
      trantab[cnt] = -1;

   chtatab[chtnxalph] = chtnpoint = trantab;

   return chtnxalph++;
}

// Find a character in a translate table
// Parameters:
//	pointer to translate table
//	pointer to pointer to input string
//	pointer to return value integer pointer
//	pointer to numeric return
// Return:
//	status of search
char_tx chtcfind(int *chtab, char **sourcepnt, int **tabpnt, int *numret) {
   char *sptr = *sourcepnt;

   char cv = *sptr;
   switch (cv) {
      case '\0':
         return CF_END;

      default:
         if (chtab == (int *)NULL) {
            *numret = *sptr;
            *sourcepnt = ++sptr;
            return CF_NUMBER;
         } else {
            int *valaddr = &(chtab[cv & 0xff]);
            *sourcepnt = ++sptr;
            *tabpnt = valaddr;
            return (*valaddr == -1) ? CF_UNDEF : CF_CHAR;
         }

      case '\\':
         switch (cv = *(++sptr)) {
            case '\0':
               *sourcepnt = sptr;
               return CF_INVALID;

            case '\'':
            case '\"':
            case '\\':
               if (chtab == (int *)NULL) {
                  *numret = *sptr;
                  *sourcepnt = ++sptr;
                  return CF_NUMBER;
               } else {
                  int *valaddr = &(chtab[(cv & 0xff) + 256]);
                  *sourcepnt = ++sptr;
                  *tabpnt = valaddr;
                  return (*valaddr == -1) ? CF_UNDEF : CF_CHAR;
               }

            default:
               if (chtab == (int *)NULL) {
                  *sourcepnt = ++sptr;
                  return CF_INVALID;
               } else {
                  int *valaddr = &(chtab[(cv & 0xff) + 256]);
                  *sourcepnt = ++sptr;
                  *tabpnt = valaddr;
                  return (*valaddr == -1) ? CF_UNDEF : CF_CHAR;
               }

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            {
               int numval = cv - '0';
               cv = *(++sptr);
               if (cv >= '0' && cv <= '7') {
                  numval = numval * 8 + cv - '0';

                  cv = *(++sptr);
                  if (cv >= '0' && cv <= '7') {
                     numval = numval * 8 + cv - '0';
                     ++sptr;
                  }
               }
               *sourcepnt = sptr;
               *numret = numval & 0xff;
               return CF_NUMBER;
            }

            case 'x': {
               int numval = cv = *(++sptr);
               switch (cv) {
                  case '0':
                  case '1':
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9':
                     numval -= '0';
                     break;

                  case 'a':
                  case 'b':
                  case 'c':
                  case 'd':
                  case 'e':
                  case 'f':
                     numval -= 'a' - 10;
                     break;

                  case 'A':
                  case 'B':
                  case 'C':
                  case 'D':
                  case 'E':
                  case 'F':
                     numval -= 'A' - 10;
                     break;

                  default:
                     *sourcepnt = sptr;
                     return CF_INVALID;
               }

               switch (cv = *(++sptr)) {
                  case '0':
                  case '1':
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9':
                     numval = numval * 16 + cv - '0';
                     ++sptr;
                     break;

                  case 'a':
                  case 'b':
                  case 'c':
                  case 'd':
                  case 'e':
                  case 'f':
                     numval = numval * 16 + cv - 'a' + 10;
                     ++sptr;
                     break;

                  case 'A':
                  case 'B':
                  case 'C':
                  case 'D':
                  case 'E':
                  case 'F':
                     numval = numval * 16 + cv - 'A' + 10;
                     ++sptr;
                     break;

                  default:
                     break;
               }

               *sourcepnt = sptr;
               *numret = numval;
               return CF_NUMBER;
            }
         }
   }
}

int chtran(char **sourceptr) {
   char *beforeptr = *sourceptr;

   int *retptr, numval;
   switch (chtcfind(chtcpoint, sourceptr, &retptr, &numval)) {
      case CF_END:
      default:
         return 0;

      case CF_INVALID:
         fracherror("invalid character constant", beforeptr, *sourceptr);
         return 0;

      case CF_UNDEF:
         fracherror("undefined character value", beforeptr, *sourceptr);
         return 0;

      case CF_NUMBER:
         return numval;

      case CF_CHAR:
         return *retptr;
   }
}

// Produce 'D' records for a ASCII string constant
// by chopping it up into lengths that will fit
// in the intermediate file
// Parameters:
//	a character string
// Return:
//	the length of the string total (machine code bytes)
int genstring(char *str) {
   const int STCHPERLINE = 20;
   int rvlen = 0;

   while (*str != '\0') {
      goutptr = &goutbuff[2];

      for (int linecount = 0; linecount < STCHPERLINE && *str != '\0'; linecount++) {
         gout2hex(chtran(&str));
         goutch(IFC_EMU8);
         rvlen++;
      }

      if (goutptr > &goutbuff[2]) {
         goutch('\n');
         fwrite(goutbuff, sizeof(char), goutptr - &goutbuff[0], intermedf);
      }
   }

   return rvlen;
}

static char *pepolptr;
static int pepolcnt;
static long etop;
static seg_t etopseg;
static const size_t STACKALLOWANCE = 4; /* number of level used outside polish expr */

// Output a character to a evar[?].exprstr array
// Globals:
//	parser expression to polish pointer pepolptr
static void polout(char ch) {
   if (pepolcnt > 1) {
      *pepolptr++ = ch;
      pepolcnt--;
   } else {
      *pepolptr = '\0';
      fraerror("overflow in polish expression conversion");
   }
}

// Output a long constant to a polish expression
static void polnumout(unsigned long inv) {
   if (inv > 15)
      polnumout(inv >> 4);
   polout(hexch((int)inv));
}

// Convert an expression tree to polish notation
// and do a preliminary evaluation of the numeric value
// of the expression
// Parameters:
//	the subscript of an expression node
// Globals:
//	the expression stack
//	the polish expression string in an evalr element
// Return:
//	False if the expression stack overflowed
//	The expression stack top contains the
//	value and segment for the result of the expression
//	which are propgated along as numeric operators are
//	evaluated.  Undefined references result in an
//	undefined result.
static bool pepolcon(int esub) {
   switch (enode[esub].evs) {
      case PCCASE_UN:
      {
         if (!pepolcon(enode[esub].left))
            return false;

         polout(enode[esub].op);

         etop = EvalUnOp(enode[esub].op, etop);
      }
         break;

      case PCCASE_BIN:
      {
         if (!pepolcon(enode[esub].left))
            return false;

         polout(IFC_LOAD);

         if (estkm1p >= &estk[PESTKDEPTH - 1 - STACKALLOWANCE]) {
            fraerror("expression stack overflow");
            return false;
         }

         (++estkm1p)->v = etop;
         estkm1p->s = etopseg;
         etopseg = SSG_UNUSED;
         etop = 0;

         if (!pepolcon(enode[esub].right))
            return false;

         polout(enode[esub].op);

         if (estkm1p->s != SSG_ABS)
            etopseg = estkm1p->s;

         etop = EvalBinOp(enode[esub].op, (estkm1p--)->v, etop);
      }
         break;

      case PCCASE_DEF:
         if (seg_valued(enode[esub].sym->seg)) {
            polnumout(1L);
            etop = 1;
            etopseg = SSG_ABS;
         } else {
            polnumout(0L);
            etop = 0;
            etopseg = SSG_ABS;
         }
         break;

      case PCCASE_SYMB:
         etop = (enode[esub].sym)->value;
         etopseg = (enode[esub].sym)->seg;
         if (etopseg == SSG_EQU || etopseg == SSG_SET) {
            etopseg = SSG_ABS;
            polnumout((unsigned long)(enode[esub].sym)->value);
         } else {
            polnumout((unsigned long)(enode[esub].sym)->symnum);
            polout(IFC_SYMB);
         }
         break;

      case PCCASE_PROGC:
         polout(IFC_PROGCTR);
         etop = locctr;
         etopseg = SSG_ABS;
         break;

      case PCCASE_CONS:
         polnumout((unsigned long)enode[esub].val);
         etop = enode[esub].val;
         etopseg = SSG_ABS;
         break;

   }
   return true;
}

// Evaluate and save the results of an expression tree
// Parameters:
//	the subscript to the evalr element to place the results
//	the subscript of the root node of a parser expression
//	tree
// Globals:
//	the evaluation results array
//	the expression stack
//	the expression tree node array
// Return:
//	in evalr[sub].seg == SSG_UNDEF if the polish expression
//	conversion overflowed, or any undefined symbols were
//	referenced.
void pevalexpr(int sub, int exn) {
   etop = 0;
   etopseg = SSG_UNUSED;
   estkm1p = &estk[0];

   pepolptr = &evalr[sub].exprstr[0];
   pepolcnt = PPEXPRLEN;

   if (pepolcon(exn)) {
      evalr[sub].seg = etopseg;
      evalr[sub].value = etop;
      polout('\0');
   } else {
      evalr[sub].exprstr[0] = '\0';
      evalr[sub].seg = SSG_UNDEF;
   }
}
