#include "stdio.h"
#include "assm.d1"
#include "ctype.h"

/*  Assembler for the MOS Technology 650X series of microprocessors
 *  Written by J. H. Van Ornum (201) 949-1781
 *		AT&T Bell Laboratories
 *		 Holmdel, NJ
 */

FILE	*optr;
FILE	*iptr;
int	dflag;			/* debug flag */
int	errcnt;			/* error counter */
char	hex[5];			/* hexadecimal character buffer */
int	iflag;			/* ignore .nlst flag */
int	lablptr;		/* label pointer into symbol table */
int	lflag;			/* disable listing flag */
int	loccnt;			/* location counter	*/
int	nflag;			/* normal/split address mode */
int	mflag;			/* generate MOS Technology object format */
int	nxt_free;		/* next free location in symtab */
int	objcnt;			/* object byte counter */
int	oflag;			/* object output flag */
int	opflg;			/* operation code flags */
int	opval;			/* operation code value */
int	pass;			/* pass counter		*/
char	prlnbuf[LAST_CH_POS+1]; /* print line buffer	*/
int	sflag;			/* symbol table output flag */
int	slnum;			/* source line number counter */
char	*symtab;		/* symbol table		*/
				/* struct sym_tab		*/
				/* {	char	size;		*/
				/*	char	chars[size];	*/
				/*	char	flag;		*/
				/*	int	value;		*/
				/*	int	line defined	*/
				/*	char	# references	*/
				/*	int	line referenced	*/
				/* (above occurs 0 or more times*/
				/* }				*/
unsigned size;			/* symbol table size 		*/
char	symbol[SBOLSZ];		/* temporary symbol storage	*/
int	udtype;			/* undefined symbol type	*/
int	undef;			/* undefined symbol in expression flg  */
int	value;			/* operand field value */
char	zpref;			/* zero page reference flag	*/
/* added by Joel Swank 12/86   */
int	pagect;			/* count of pages               */
int	paglin;			/* lines printed on current page */
int	pagesize;		/* maximum lines per page       */
int	linesize;		/* maximum characters oer line  */
int	titlesize;		/* maximum characters oer line  */
char	titlbuf[100];		/* buffer for title from .page  */
char	syspc[80];		/* variable filler for heading  */
char	*date;			/* pointer to formatted date string */


#define A	0x20)+('A'&0x1f))
#define B	0x20)+('B'&0x1f))
#define C	0x20)+('C'&0x1f))
#define D	0x20)+('D'&0x1f))
#define E	0x20)+('E'&0x1f))
#define F	0x20)+('F'&0x1f))
#define G	0x20)+('G'&0x1f))
#define H	0x20)+('H'&0x1f))
#define I	0x20)+('I'&0x1f))
#define J	0x20)+('J'&0x1f))
#define K	0x20)+('K'&0x1f))
#define L	0x20)+('L'&0x1f))
#define M	0x20)+('M'&0x1f))
#define N	0x20)+('N'&0x1f))
#define O	0x20)+('O'&0x1f))
#define P	0x20)+('P'&0x1f))
#define Q	0x20)+('Q'&0x1f))
#define R	0x20)+('R'&0x1f))
#define S	0x20)+('S'&0x1f))
#define T	0x20)+('T'&0x1f))
#define U	0x20)+('U'&0x1f))
#define V	0x20)+('V'&0x1f))
#define W	0x20)+('W'&0x1f))
#define X	0x20)+('X'&0x1f))
#define Y	0x20)+('Y'&0x1f))
#define Z	0x20)+('Z'&0x1f))

#define OPSIZE	127

int	optab[]	=		/* nmemonic  operation code table	*/
{				/* '.' = 31, '*' = 30, '=' = 29		*/
	((0*0x20)+(29)),PSEUDO,1,
	((((0*0x20)+(30))*0x20)+(29)),PSEUDO,3,
	((((((0*A*D*C,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0x61,
	((((((0*A*N*D,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0x21,
	((((((0*A*S*L,ABS|ZER|ZERX|ABSX|ACC,0x02,
	((((((0*B*C*C,CLASS2,0x90,
	((((((0*B*C*S,CLASS2,0xb0,
	((((((0*B*E*Q,CLASS2,0xf0,
	((((((0*B*I*T,ABS|ZER,0x20,
	((((((0*B*M*I,CLASS2,0x30,
	((((((0*B*N*E,CLASS2,0xd0,
	((((((0*B*P*L,CLASS2,0x10,
	((((((0*B*R*K,CLASS1,0x00,
	((((((0*B*V*C,CLASS2,0x50,
	((((((0*B*V*S,CLASS2,0x70,
	((((((0*C*L*C,CLASS1,0x18,
	((((((0*C*L*D,CLASS1,0xd8,
	((((((0*C*L*I,CLASS1,0x58,
	((((((0*C*L*V,CLASS1,0xb8,
	((((((0*C*M*P,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0xc1,
	((((((0*C*P*X,IMM1|ABS|ZER,0xe0,
	((((((0*C*P*Y,IMM1|ABS|ZER,0xc0,
	((((((0*D*E*C,ABS|ZER|ZERX|ABSX,0xc2,
	((((((0*D*E*X,CLASS1,0xca,
	((((((0*D*E*Y,CLASS1,0x88,
	((((((0*E*O*R,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0x41,
	((((((0*I*N*C,ABS|ZER|ZERX|ABSX,0xe2,
	((((((0*I*N*X,CLASS1,0xe8,
	((((((0*I*N*Y,CLASS1,0xc8,
	((((((0*J*M*P,ABS|IND,0x40,
	((((((0*J*S*R,ABS,0x14,
	((((((0*L*D*A,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0xa1,
	((((((0*L*D*X,IMM1|ABS|ZER|ABSY2|ZERY,0xa2,
	((((((0*L*D*Y,IMM1|ABS|ZER|ABSX|ZERX,0xa0,
	((((((0*L*S*R,ABS|ZER|ZERX|ABSX|ACC,0x42,
	((((((0*N*O*P,CLASS1,0xea,
	((((((0*O*R*A,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0x01,
	((((((0*P*H*A,CLASS1,0x48,
	((((((0*P*H*P,CLASS1,0x08,
	((((((0*P*L*A,CLASS1,0x68,
	((((((0*P*L*P,CLASS1,0x28,
	((((((0*R*O*L,ABS|ZER|ZERX|ABSX|ACC,0x22,
	((((((0*R*O*R,ABS|ZER|ZERX|ABSX|ACC,0x62,
	((((((0*R*T*I,CLASS1,0x40,
	((((((0*R*T*S,CLASS1,0x60,
	((((((0*S*B*C,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0xe1,
	((((((0*S*E*C,CLASS1,0x38,
	((((((0*S*E*D,CLASS1,0xf8,
	((((((0*S*E*I,CLASS1,0x78,
	((((((0*S*T*A,ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0x81,
	((((((0*S*T*X,ABS|ZER|ZERY,0x82,
	((((((0*S*T*Y,ABS|ZER|ZERX,0x80,
	((((((0*T*A*X,CLASS1,0xaa,
	((((((0*T*A*Y,CLASS1,0xa8,
	((((((0*T*S*X,CLASS1,0xba,
	((((((0*T*X*A,CLASS1,0x8a,
	((((((0*T*X*S,CLASS1,0x9a,
	((((((0*T*Y*A,CLASS1,0x98,
	((((((0*0x20)+(31))*W*O^((((0*R*D,PSEUDO,2,	/* 0x7cab */
	((((((0*0x20)+(31))*B*Y^((((0*T*E,PSEUDO,0,	/* 0x7edc */
	((((((0*0x20)+(31))*P*A^((((0*G*E,PSEUDO,7,	/* 0x7ee4 */
	((((((0*0x20)+(31))*D*B^((((0*Y*T,PSEUDO,6,	/* 0x7fb6 */
	((((((0*0x20)+(31))*N*L^((((0*S*T,PSEUDO,5,	/* 0x7fb8 */
	((((((0*0x20)+(31))*L*I^((((0*S*T,PSEUDO,4,	/* 0x7ffd */
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0,
	0x7fff,0,0
};

int	step[] =
{
	3*((OPSIZE+1)/2),
	3*((((OPSIZE+1)/2)+1)/2),
	3*((((((OPSIZE+1)/2)+1)/2)+1)/2),
	3*((((((((OPSIZE+1)/2)+1)/2)+1)/2)+1)/2),
	3*((((((((((OPSIZE+1)/2)+1)/2)+1)/2)+1)/2)+1)/2),
	3*(2),
	3*(1),
	0
};
