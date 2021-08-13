#include <stdio.h>
#include "Assm1.h"
#include "Assm2.h"
#include <ctype.h>

extern	int	optab[];
extern	int	step[];

/* translate source line to machine language */

assemble()
{
	int	flg;
	int	i;		/* prlnbuf pointer */

	if ((prlnbuf[SFIELD] == ';') | (prlnbuf[SFIELD] == 0)) {
		if (pass == LAST_PASS)
			println();
		return;
	}
	lablptr = -1;
	i = SFIELD;
	udtype = UNDEF;
	if (colsym(&i) != 0 && (lablptr = stlook()) == -1)
		return;
	while (prlnbuf[++i] == ' ');	/* find first non-space */
	if ((flg = oplook(&i)) < 0) {	/* collect operation code */
		labldef(loccnt);
		if (flg == -1)
			error("Invalid operation code");
		if ((flg == -2) && (pass == LAST_PASS)) {
			if (lablptr != -1)
				loadlc(loccnt, 1, 0);
			println();
		}
		return;
	}
	if (opflg == PSEUDO)
		pseudo(&i);
	else if (labldef(loccnt) == -1)
		return;
	else {
		if (opflg == CLASS1)
			class1();
		else if (opflg == CLASS2)
			class2(&i);
		else class3(&i);
	}
}

/****************************************************************************/

/* printline prints the contents of prlnbuf */

println()
{
	if (lflag > 0)
		{
		if (paglin == pagesize) printhead();
		prlnbuf[linesize] = '\0';
		fprintf(stdout, "%s\n", prlnbuf);
		paglin++ ;
		}
}

/****************************************************************************/

/* printhead prints the page heading   */

printhead()
{
	if (pagesize == 0) return;
	pagect++ ;
	fprintf(stdout, "\f\nAmiga 6502 assembler :  -  %s PAGE %d\n",
	titlbuf,pagect);
	fprintf(stdout, "Line      Location     Label Opcode Operand  Comment   %s\n\n",date);
	paglin = 0;
}

/* colsym() collects a symbol from prlnbuf into symbol[],
 *    leaves prlnbuf pointer at first invalid symbol character,
 *    returns 0 if no symbol collected
 */

colsym(ip)
    int *ip;
{
	int	valid;
	int	i;
	char	ch;

	valid = 1;
	i = 0;
	while (valid == 1) {
		ch = prlnbuf[*ip];
		if (ch == '_' || ch == '.');
		else if (ch >= 'a' && ch <= 'z');
		else if (ch >= 'A' && ch <= 'Z');
		else if (i >= 1 && ch >= '0' && ch <= '9');
		else if (i == 1 && ch == '=');
		else valid = 0;
		if (valid == 1) {
			if (i < SBOLSZ - 1)
				symbol[++i] = ch;
			(*ip)++;
		}
	}
	if (i == 1) {
		switch (symbol[1]) {
		case 'A': case 'a':
		case 'X': case 'x':
		case 'Y': case 'y':
			error("Symbol is reserved (A, X or Y)");
			i = 0;
		}
	}
	symbol[0] = i;
	return(i);
}

/* symbol table lookup
 *	if found, return pointer to symbol
 *	else, install symbol as undefined, and return pointer
 */

stlook()
{
int ptr, ln, eq;
ptr = 0;
while (ptr < nxt_free)
	{
	ln = symbol[0]; if (symtab[ptr] < ln) ln = symtab[ptr];
	if ((eq = strncmp(&symtab[ptr+1], &symbol[1], ln)) == 0 &&
	   symtab[ptr] == symbol[0]) return ptr;
	if (eq > 0) return(stinstal(ptr));
	ptr = ptr+6+ symtab[ptr];
	ptr = ptr +1 + 2*(symtab[ptr] & 0xff);
	}
return (stinstal(ptr));
}


/*  instal symbol into symtab
 */

stinstal(ptr)
int ptr;
{
int ptr2, i;
if (openspc(ptr,symbol[0]+7) == -1) {
	error("Symbol Table Full"); /* print error msg and ...  */
	pass = DONE;		    /* cause termination of assembly */
	return -1; }
ptr2 = ptr;
for (i=0; i< symbol[0]+1; i++)
	symtab[ptr2++] = symbol[i];
symtab[ptr2++] = udtype;
symtab[ptr2+4] = 0;
return(ptr);
}


/*   addref : add a reference line to the  symbol pointed to    */
/*            by ip.                        */

addref(ip)
int ip;
{
int rct, ptr;
rct = ptr =ip + symtab[ip] + 6;
if ((symtab[rct] & 0xff) == 255) {	/* non-fatal error   */
	fprintf(stderr,"%s\n",prlnbuf);
	fprintf(stderr,"Too many references\n");
	return; }
ptr += (symtab[rct] & 0xff) * 2 +1;
if (openspc(ptr,2) == -1) {
	error("Symbol Table Full");
	return -1; }
symtab[ptr] = slnum & 0xff;
symtab[ptr+1] = (slnum >> 8) & 0xff;
symtab[rct]++;
}


/* openspc : open up a space in the symbol table        */
/*           the space will be at (ptr) and will be     */
/*           len characters long. return -1 if no room. */

openspc(ptr,len)
int ptr,len;
{
int ptr2, ptr3;
if (nxt_free + len > size) return -1;
if (ptr != nxt_free)
	{
	ptr2 = nxt_free -1;
	ptr3 = ptr2 + len;
	while (ptr2 >= ptr) symtab[ptr3--] = symtab[ptr2--];
	}
nxt_free += len;
if (lablptr >= ptr) lablptr += len;
return 0;
}


/* operation code table lookup
 *	if found, return pointer to symbol,
 *	else, return -1
 */

oplook(ip)
   int	*ip;
{
register	char	ch;
register	int	i;
register	int	j;
	int	k;
	int	temp[2];

	i = j = 0;
	temp[0] = temp[1] = 0;
	while((ch=prlnbuf[*ip])!= ' ' && ch!= 0 && ch!= '\t' && ch!= ';') {
		if (ch >= 'A' && ch <= 'Z')
			ch &= 0x1f;
		else if (ch >= 'a' && ch <= 'z')
			ch &= 0x1f;
		else if (ch == '.')
			ch = 31;
		else if (ch == '*')
			ch = 30;
		else if (ch == '=')
			ch = 29;
		else return(-1);
		temp[j] = (temp[j] * 0x20) + (ch & 0xff);
		if (ch == 29)
			break;
		++(*ip);
		if (++i >= 3) {
			i = 0;
			if (++j >= 2) {
				return(-1);
			}
		}
	}
	if ((j = temp[0]^temp[1]) == 0)
		return(-2);
	k = 0;
	i = step[k] - 3;
	do {
		if (j == optab[i]) {
			opflg = optab[++i];
			opval = optab[++i];
			return(i);
		}
		else if (j < optab[i])
			i -= step[++k];
		else i += step[++k];
	} while (step[k] != 0);
	return(-1);
}

/* error printing routine */

error(stptr)
   char *stptr;
{
	loadlc(loccnt, 0, 1);
	loccnt += 3;
	loadv(0,0,0);
	loadv(0,1,0);
	loadv(0,2,0);
	fprintf(stderr, "%s\n", prlnbuf);
	fprintf(stderr, "%s\n", stptr);
	errcnt++;
}

/* load 16 bit value in printable form into prlnbuf */

loadlc(val, f, outflg)
    int val;
    int f;
    int outflg;
{
	int	i;

	i = 6 + 7*f;
	hexcon(4, val);
	if (nflag == 0) {
		prlnbuf[i++]  = hex[3];
		prlnbuf[i++]  = hex[4];
		prlnbuf[i++]  = ':';
		prlnbuf[i++]  = hex[1];
		prlnbuf[i] = hex[2];
	}
	else {
		prlnbuf[i++] = hex[1];
		prlnbuf[i++] = hex[2];
		prlnbuf[i++] = hex[3];
		prlnbuf[i] = hex[4];
	}
	if ((pass == LAST_PASS)&&(oflag != 0)&&(objcnt <= 0)&&(outflg != 0))
		{
		if (mflag != 0) start_obj(val);
		else fprintf(optr, "\n;%c%c%c%c", hex[3], hex[4], hex[1], hex[2]);
		objcnt=22;
		}
}



/* load value in hex into prlnbuf[contents[i]] */
/* and output hex characters to obuf if LAST_PASS & oflag == 1 */

loadv(val,f,outflg)
   int	val;
   int	f;		/* contents field subscript */
   int	outflg;		/* flag to output object bytes */
{

	hexcon(2, val);
	prlnbuf[13 + 3*f] = hex[1];
	prlnbuf[14 + 3*f] = hex[2];
	if ((pass == LAST_PASS) && (oflag != 0) && (outflg != 0)) {
		if (mflag != 0) put_obj(val);
		else {	fputc(hex[1], optr);
			fputc(hex[2], optr); }
		--objcnt;
	}
}

/* convert number supplied as argument to hexadecimal in hex[digit] (lsd)
		through hex[1] (msd)		*/

hexcon(digit, num)
    int digit;
   int	num;
{

	for (; digit > 0; digit--) {
		hex[digit] = (num & 0x0f) + '0';
		if (hex[digit] > '9')
			hex[digit] += 'A' -'9' - 1;
		num >>= 4;
	}
}

/* assign <value> to label pointed to by lablptr,
 *	checking for valid definition, etc.
 */

labldef(lval)
    int lval;
{
	int	i;

	if (lablptr != -1) {
		lablptr += symtab[lablptr] + 1;
		if (pass == FIRST_PASS) {
			if (symtab[lablptr] == UNDEF) {
				symtab[lablptr + 1] = lval & 0xff;
				i = symtab[lablptr + 2] = (lval >> 8) & 0xff;
				if (i == 0)
					symtab[lablptr] = DEFZRO;
				else	symtab[lablptr] = DEFABS;
			}
			else if (symtab[lablptr] == UNDEFAB) {
				symtab[lablptr] = DEFABS;
				symtab[lablptr + 1] = lval & 0xff;
				symtab[lablptr + 2] = (lval >> 8) & 0xff;
			}
			else {
				symtab[lablptr] = MDEF;
				symtab[lablptr + 1] = 0;
				symtab[lablptr + 2] = 0;
				error("Label multiply defined");
				return(-1);
			}
		symtab[lablptr+3] = slnum & 0xff;
		symtab[lablptr+4] = (slnum >> 8) & 0xff;
		}
		else {
			i = (symtab[lablptr + 2] << 8) +
				(symtab[lablptr+1] & 0xff);
			i &= 0xffff;
			if (i != lval && pass == LAST_PASS) {
				error("Sync error");
				return(-1);
			}
		}
	}
	return(0);
}

/* determine the value of the symbol,
 * given pointer to first character of symbol in symtab
 */

symval(ip)
    int *ip;
{
	int	ptr;
	int	svalue;

	svalue = 0;
	colsym(ip);
	if ((ptr = stlook()) == -1)
		undef = 1;		/* no room error */
	else if (symtab[ptr + symtab[ptr] + 1] == UNDEF)
		undef = 1;
	else if (symtab[ptr + symtab[ptr] + 1] == UNDEFAB)
		undef = 1;
	else svalue = ((symtab[ptr + symtab[ptr] + 3] << 8) +
		(symtab[ptr + symtab[ptr] + 2] & 0xff)) & 0xffff;
	if (symtab[ptr + symtab[ptr] + 1] == DEFABS)
		zpref = 1;
	if (undef != 0)
		zpref = 1;

	/* add a reference entry to symbol table on first pass only,
	   except for branch instructions (CLASS2) which do not come
	   through here on the first pass                            */
	if (ptr >= 0 && pass == FIRST_PASS) addref(ptr);
	if (ptr >= 0 && opflg == CLASS2) addref(ptr); /* branch addresses */
	return(svalue);
}

/*    object code record generation routines    */
/*    added to generate MOS Technology format   */
/*       object records                         */
/*           By Joel Swank 12/86                */

char obj_rec[60];		/* buffer for object record */
unsigned  obj_ptr = 0;		/* pointer for above  */
unsigned  obj_bytes = 0;		/* count of bytes in current record */
unsigned  rec_cnt = 0;		/* count of records in this file */
unsigned  cksum = 0;			/* record check sum accumulator  */

/*   put one object byte in hex   */

put_obj(val)
unsigned val;
{
	hexcon(2,val);
	obj_rec[obj_ptr++] = hex[1];
	obj_rec[obj_ptr++] = hex[2];
	cksum += (val & 0xff);
	obj_bytes++;
}


/*    start an object record (end previous) */

start_obj(val)
unsigned val; 	/*  current location counter */
{
	prt_obj();	/* print the current record if any */
	hexcon(4,val);
	obj_bytes=0;
	for (obj_ptr=0; obj_ptr<4; obj_ptr++) obj_rec[obj_ptr] = hex[obj_ptr+1];
	cksum = (val>>8) + (val & 0xff);
	rec_cnt++;
}


/*    print the current object record if any */

prt_obj()
{
	if (obj_bytes == 0) return;
	cksum += obj_bytes;
	hexcon(2,obj_bytes);
	obj_rec[obj_ptr] = '\0';
	fprintf(optr,";%c%c%s",hex[1],hex[2],obj_rec);
	hexcon(4,cksum);
	fprintf(optr,"%c%c%c%c\n",hex[1],hex[2],hex[3],hex[4]);
}


/*    finish object file       */

fin_obj()
{
	unsigned i;
	prt_obj();
	hexcon(4,++rec_cnt);
	fprintf(optr,";00");
	for (i=1; i<5; i++) fputc(hex[i],optr);
	rec_cnt = rec_cnt/256 + (rec_cnt & 0xff);
	hexcon(4,rec_cnt);
	for (i=1; i<5; i++) fputc(hex[i],optr);
	fputc('\n',optr);
}
/*     MOS Tech. object format is as follows    */
/*
	( all data is in ASCII encoded hexidecimal)

 Data record : ;nnaaaadddd...xxxx[cr]
 Last record : ;00ccccxxxx[cr]

 Where:
	;	= Start of record (ASCII 3B)
	nn	= Number of data bytes in the record.
		  max = 24 bytes.
	aaaa	= address of first data byte in the record.
	dd	= 1 data byte.
	xxxx	= checksum that is the twos compliment sum of all
		  data bytes, the count byte and the address bytes.
	cccc	= count of records in the file.
	[cr]	= ASCII Carriage Return (ASCII 0D).

*/
