
/*
	Copyright 1990 Compouter Design Solutions Inc.
	This code may be used by anyone in anyway.
	CDS Inc. grants full rights to code to all users.
	
	Computer Design Solutions Inc.
	P.O. Box 127
	Statesville NC	28677
	704-876-2346	Fax 704-876-2348

*/


#define TRUE	1
#define FALSE	0
#define CR 	0x0d

/*	Flag definations	*/
#define OK	0
#define	NODATA	1		/* Returns from get_v when no data found */


static unsigned int start,end,value,last;
static unsigned char temp[16];
static unsigned char cin,flag;
static unsigned char *p;

main()
	{
/*
	Sign on and say hello
*/

	crlf();
	print("CDS Small Monitor Version 1.0");
	start=end=value=last=0;	/* init vars	*/

/*	
	Loop forever or until control C

*/

	while(1)

	{

/*	
	Get Command
*/

	crlf();
	putchar('>');
	cin=toupper(getchar());

	switch(cin)
	{	
	case 'F':			/* Fill Memory */
		f_mem();
		break;
	case 'D':			/* Dump Memory */
		d_mem();
		break;	
	case 'E':			/* Examine Memory */
		e_mem();
		break;
	case 3:				/* Ctl C  Exit	*/
		exit_mon();
	default:
		break;

	}

	}
	

	}

/*	Not normally used. This is for testing on a CPM machine */

exit_mon()
	{
#asm
	jp	0
#endasm
	}

/* 	Examine memory	*/

e_mem()
	{
	print("xamine  ");
/*
	Get address
*/
	start=get_v();
	cin=' ';
	p=start;
	flag=OK;

/*
 	Loop till CR
*/

	while(flag==OK)
		{
		crlf();
/*
	Print address
*/

		phex(p,4,2);
		print("=  ");
/*
	Print contents
*/

		phex(*p,2,2);
/*
	Get new value
*/

		value=get_v();
		*p=value;
/*
	Next address
*/

		p++;
		}
	}

/*	Fill memory 	*/
f_mem()
	{
	print("ill ");
/*
	Get start, end, and data pattern
*/

	start=get_v();
	if(flag==NODATA)
		fer();
	end=get_v();
	if(flag==NODATA)
		fer();
	value=get_v();
	rcheck();
	fill(start,end,value);
	}

fer()
	{
	error("Fill needs more data");
	}


/*	Dump memory	*/
d_mem()
	{
	clear();
	print("ump ");
/*
	Get Start and end
*/

	start=get_v();
	if(cin!=CR)
	end=get_v();
	crlf();
	rcheck();
	dump(start,end);
	}



/*	Error processing	*/

error(s)
	char *s;
	{
	crlf();
	bell();
	print(s);
	crlf();
	main();			/* sort of silly but works	*/
	}

/*	Ring the bell		*/
bell()
	{
	putchar(7);
	}


/*	Range check start to end	*/

rcheck()
	{
	if(end<=start)
		end=start+16;
	}
/*	Clear start,end,value,cin	*/
clear()
	{
	start=end=value=0;
	cin=0;
	}

/* get a hexadecimal value from the keyboard	*/

get_v()
	{
	unsigned int val;
	unsigned char cnt;
	cnt=cin=0;
	while((cin!=CR) && (cin!=','))
	{
	cin=getchar();
	if(cnt==0) flag=NODATA; else flag=OK;
	temp[cnt++]=cin;
	if(cnt>5)
		{
		bell();
		cnt--;
		}

	/*	Back space occured	*/

	if(cin=='\b')
		{
		if(cnt>0) cnt--; else bell();
		print(" \b");
		}

	}
	temp[--cnt]=0;
	upcase(temp);	
	val=htob(temp,16);
	if(flag==NODATA) return(last);
	last=val;
	return(val);
	}
 

/* 	Fill from ad1 to ad2 with data				*/

fill(start,end,data)
	unsigned int start,end;
	unsigned char data;
	{
	char *p;
	for(p=start;p<end;)
		*p++=data;
	}



/*	Hex dump with address on left and ASCII on right	*/
/*	Start is the begin address and end is the end		*/

dump(start,end)
	int start,end;

	{
	int c_cnt;
	crlf();
	for (p=start;p<end;)
	{
					/* Print the address */
	phex(p,4,4);
	for(c_cnt=0;c_cnt<16;c_cnt++)
	{
	phex(*p++,2,1);
	}
	p-=16;				/* reset */
					/* Print the ASCII	*/
	for(c_cnt=0;c_cnt<16;c_cnt++)
	{
	pascii(*p++,'.',0);
	}	
	crlf();
	}
	}



	/*	Print a new line seq.	*/

crlf()
	{
	print("\n\r");
	}


/*	rept will print char. s n times 	*/

rept(s,n)
	char s,n;
	{
	if(n==0) return;
	while(n--)
		putchar(s);
	}


/* pascii(char,fill,spc)
	print ascii or fill and spc traiing spaces
*/

pascii(chr,fill,spc)
	unsigned char chr,fill,spc;
	{
	if(chr<' ' || chr>'}') 
	putchar(fill);
	else
	putchar(chr);
	space(spc);
	}



/* phex(n,len,spc)
	print in hex the number (n) 0 filled of len with spc trailing
	spaces. phex(100,4,2) will print 0064  .
*/

phex(n,lgth,spc)
	unsigned int n;
	unsigned char lgth,spc;

	{

	itob(n,temp,16);
	rept('0',(lgth-len(temp)));	/* 0 fill	*/
	print(temp);
	space(spc);			/* print sep.	 */
					/* Print the 16 hex digits */
	}
	


/* String Print	*/

print(s)
	char *s;
	{
	while(*s)
	putchar(*s++);
	}


	/*	 space(x)   will print x spaces	*/

space(x)
	char x;
	{
	if(x==0) return;
	while (x--)
	putchar(' ');
	}


	/* This is where you put your char. out
	   code. The CDS Z-80/Z-280 compiler passes
	   the char in L. */

putchar(s)
	char s;
	{

#asm
	push	bc
	push	ix
	ld	e,l
	ld	c,2
	call	5
	pop	ix
	pop	bc
#endasm

	}

	/* get a charector from the keyboard	*/

getchar()
	{
#asm
	push	bc
	push	ix
	ld	c,1
	call	5
;	and	a,5fh		; force upper case
	ld	l,a
	pop	ix
	ld	h,0
	pop	bc
#endasm
	}
	
/* len returns the length of a string */

len(s)
	char *s;
	{
	char i;
	i=0;
	while(*s++)
	i++;
	return i;
	}



upcase(str)                     /* convert a string to uppercase */
   char *str;
{
   char *start;
   start = str;
   while (*str) {
      if (islower(*str))
         *str = toupper(*str);
      str++; }
   return(start);
}

/* itob -- convert an integer to a string in any base (2-36) 
   where
	n is the number to convert
	s is a temp. string
	base is the number base
*/

itob(n, s, base)
char *s;
	{
	unsigned int u;
	char *p, *q;
	int negative, c;

	if (n < 0 && base == -10) {
		negative = TRUE;
		u = -n;
		}
	else {
		negative = FALSE;
		u = n;
		}
	if (base == -10)		/* signals signed conversion */
		base = 10;
	p = q = s;
	do {				/* generate digits in reverse order */
		if ((*p = u % base + '0') > '9')
			*p += ('A' - ('9' + 1));
		++p;
		} while ((u /= base) > 0);
	if (negative)
		*p++ = '-';
	*p = '\0';			/* terminate the string */
	while (q < --p) {		/* reverse the digits */
		c = *q;
		*q++ = *p;
		*p = c;
		}
	return s;

	}


int htob(str,radix)                /* convert string to integer */
   char *str;
   unsigned int radix;
{
   unsigned int digit, number;
   number=0;
   while (*str) {
      if (isdigit(*str)) digit = *str - '0';
      else if (islower(digit)) digit = *str - 'a' + 10;
      else digit = *str - 'A' + 10;
      number = number * radix + digit;
      str++; }
   return(number);
}
		crlf();
/*
	Print address
*/

		phex(p,4,2);
		print("=  ");
/*
	Print contents
*/

		phex(*p,2,2);
/*
	Get