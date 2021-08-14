#include <stdio.h>
#include <ctype.h>
#include "Constants.h"

/*  Assembler for the MOS Technology 650X series of microprocessors
 *  Written by J. H. Van Ornum (201) 949-1781
 *		AT&T Bell Laboratories
 *		 Holmdel, NJ
 */

FILE *optr;
FILE *iptr;
int dflag; /* debug flag */
int errcnt; /* error counter */
char hex[5]; /* hexadecimal character buffer */
int iflag; /* ignore .nlst flag */
int lablptr; /* label pointer into symbol table */
int lflag; /* disable listing flag */
int loccnt; /* location counter     */
int nflag; /* normal/split address mode */
int mflag; /* generate MOS Technology object format */
int nxt_free; /* next free location in symtab */
int objcnt; /* object byte counter */
int oflag; /* object output flag */
int opflg; /* operation code flags */
int opval; /* operation code value */
int pass; /* pass counter         */
char prlnbuf[LAST_CH_POS + 1]; /* print line buffer    */
int sflag; /* symbol table output flag */
int slnum; /* source line number counter */
char *symtab; /* symbol table         */
                                /* struct sym_tab               */
                                /* {    char    size;           */
                                /*      char    chars[size];    */
                                /*      char    flag;           */
                                /*      int     value;          */
                                /*      int     line defined    */
                                /*      char    # references    */
                                /*      int     line referenced */
                                /* (above occurs 0 or more times */
                                /* }                            */
unsigned size; /* symbol table size            */
char symbol[SBOLSZ]; /* temporary symbol storage     */
int udtype; /* undefined symbol type        */
int undef; /* undefined symbol in expression flg  */
int value; /* operand field value */
char zpref; /* zero page reference flag     */
/* added by Joel Swank 12/86   */
int pagect; /* count of pages               */
int paglin; /* lines printed on current page */
int pagesize; /* maximum lines per page       */
int linesize; /* maximum characters oer line  */
int titlesize; /* maximum characters oer line  */
char titlbuf[100]; /* buffer for title from .page  */
char syspc[80]; /* variable filler for heading  */
char *date; /* pointer to formatted date string */

#define Word1(A) ((A)&0x1f)
#define Word2(A, B) (((A)&0x1f) << 5 | (B)&0x1f)
#define Word3(A, B, C) (((A)&0x1f) << 10 | ((B)&0x1f) << 5 | (C)&0x1f)
#define OPSIZE	127

int optab[] = {
/* nmemonic  operation code table       */
/* '.' = 31, '*' = 30, '=' = 29         */
   Word1(29), PSEUDO, 1,
   Word2(30, 29), PSEUDO, 3,
   Word3('A', 'D', 'C'), IMM2 | ABS | ZER | INDX | INDY | ZERX | ABSX | ABSY, 0x61,
   Word3('A', 'N', 'D'), IMM2 | ABS | ZER | INDX | INDY | ZERX | ABSX | ABSY, 0x21,
   Word3('A', 'S', 'L'), ABS | ZER | ZERX | ABSX | ACC, 0x02,
   Word3('B', 'C', 'C'), CLASS2, 0x90,
   Word3('B', 'C', 'S'), CLASS2, 0xb0,
   Word3('B', 'E', 'Q'), CLASS2, 0xf0,
   Word3('B', 'I', 'T'), ABS | ZER, 0x20,
   Word3('B', 'M', 'I'), CLASS2, 0x30,
   Word3('B', 'N', 'E'), CLASS2, 0xd0,
   Word3('B', 'P', 'L'), CLASS2, 0x10,
   Word3('B', 'R', 'K'), CLASS1, 0x00,
   Word3('B', 'V', 'C'), CLASS2, 0x50,
   Word3('B', 'V', 'S'), CLASS2, 0x70,
   Word3('C', 'L', 'C'), CLASS1, 0x18,
   Word3('C', 'L', 'D'), CLASS1, 0xd8,
   Word3('C', 'L', 'I'), CLASS1, 0x58,
   Word3('C', 'L', 'V'), CLASS1, 0xb8,
   Word3('C', 'M', 'P'), IMM2 | ABS | ZER | INDX | INDY | ZERX | ABSX | ABSY, 0xc1,
   Word3('C', 'P', 'X'), IMM1 | ABS | ZER, 0xe0,
   Word3('C', 'P', 'Y'), IMM1 | ABS | ZER, 0xc0,
   Word3('D', 'E', 'C'), ABS | ZER | ZERX | ABSX, 0xc2,
   Word3('D', 'E', 'X'), CLASS1, 0xca,
   Word3('D', 'E', 'Y'), CLASS1, 0x88,
   Word3('E', 'O', 'R'), IMM2 | ABS | ZER | INDX | INDY | ZERX | ABSX | ABSY, 0x41,
   Word3('I', 'N', 'C'), ABS | ZER | ZERX | ABSX, 0xe2,
   Word3('I', 'N', 'X'), CLASS1, 0xe8,
   Word3('I', 'N', 'Y'), CLASS1, 0xc8,
   Word3('J', 'M', 'P'), ABS | IND, 0x40,
   Word3('J', 'S', 'R'), ABS, 0x14,
   Word3('L', 'D', 'A'), IMM2 | ABS | ZER | INDX | INDY | ZERX | ABSX | ABSY, 0xa1,
   Word3('L', 'D', 'X'), IMM1 | ABS | ZER | ABSY2 | ZERY, 0xa2,
   Word3('L', 'D', 'Y'), IMM1 | ABS | ZER | ABSX | ZERX, 0xa0,
   Word3('L', 'S', 'R'), ABS | ZER | ZERX | ABSX | ACC, 0x42,
   Word3('N', 'O', 'P'), CLASS1, 0xea,
   Word3('O', 'R', 'A'), IMM2 | ABS | ZER | INDX | INDY | ZERX | ABSX | ABSY, 0x01,
   Word3('P', 'H', 'A'), CLASS1, 0x48,
   Word3('P', 'H', 'P'), CLASS1, 0x08,
   Word3('P', 'L', 'A'), CLASS1, 0x68,
   Word3('P', 'L', 'P'), CLASS1, 0x28,
   Word3('R', 'O', 'L'), ABS | ZER | ZERX | ABSX | ACC, 0x22,
   Word3('R', 'O', 'R'), ABS | ZER | ZERX | ABSX | ACC, 0x62,
   Word3('R', 'T', 'I'), CLASS1, 0x40,
   Word3('R', 'T', 'S'), CLASS1, 0x60,
   Word3('S', 'B', 'C'), IMM2 | ABS | ZER | INDX | INDY | ZERX | ABSX | ABSY, 0xe1,
   Word3('S', 'E', 'C'), CLASS1, 0x38,
   Word3('S', 'E', 'D'), CLASS1, 0xf8,
   Word3('S', 'E', 'I'), CLASS1, 0x78,
   Word3('S', 'T', 'A'), ABS | ZER | INDX | INDY | ZERX | ABSX | ABSY, 0x81,
   Word3('S', 'T', 'X'), ABS | ZER | ZERY, 0x82,
   Word3('S', 'T', 'Y'), ABS | ZER | ZERX, 0x80,
   Word3('T', 'A', 'X'), CLASS1, 0xaa,
   Word3('T', 'A', 'Y'), CLASS1, 0xa8,
   Word3('T', 'S', 'X'), CLASS1, 0xba,
   Word3('T', 'X', 'A'), CLASS1, 0x8a,
   Word3('T', 'X', 'S'), CLASS1, 0x9a,
   Word3('T', 'Y', 'A'), CLASS1, 0x98,
   Word3(31, 'W', 'O') ^ Word2('R', 'D'), PSEUDO, 2, /* 0x7cab */
   Word3(31, 'B', 'Y') ^ Word2('T', 'E'), PSEUDO, 0, /* 0x7edc */
   Word3(31, 'P', 'A') ^ Word2('G', 'E'), PSEUDO, 7, /* 0x7ee4 */
   Word3(31, 'D', 'B') ^ Word2('Y', 'T'), PSEUDO, 6, /* 0x7fb6 */
   Word3(31, 'N', 'L') ^ Word2('S', 'T'), PSEUDO, 5, /* 0x7fb8 */
   Word3(31, 'L', 'I') ^ Word2('S', 'T'), PSEUDO, 4, /* 0x7ffd */
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0,
   Word3(31, 31, 31), 0, 0
};

#define Div2(X) (((X) + 1)/2)

int step[] = {
   3 * Div2(OPSIZE),
   3 * Div2(Div2(OPSIZE)),
   3 * Div2(Div2(Div2(OPSIZE))),
   3 * Div2(Div2(Div2(Div2(OPSIZE)))),
   3 * Div2(Div2(Div2(Div2(Div2(OPSIZE))))),
   3 * (2),
   3 * (1),
   0
};
