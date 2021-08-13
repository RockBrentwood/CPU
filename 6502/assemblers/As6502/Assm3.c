#include <stdio.h>
#include "Assm1.h"
#include "Assm2.h"
#include <ctype.h>

/* class 1 machine operations processor - 1 byte, no operand field */

class1()
{
	if (pass == LAST_PASS) {
		loadlc(loccnt, 0, 1);
		loadv(opval, 0, 1);
		println();
	}
	loccnt++;
}


/* class 2 machine operations processor - 2 byte, relative addressing */

class2(ip)
    int *ip;
{

	if (pass == LAST_PASS) {
		loadlc(loccnt, 0, 1);
		loadv(opval, 0, 1);
		while (prlnbuf[++(*ip)] == ' ');
		if (evaluate(ip) != 0) {
			loccnt += 2;
			return;
		}
		loccnt += 2;
		if ((value -= loccnt) >= -128 && value < 128) {
			loadv(value, 1, 1);
			println();
		}
		else error("Invalid branch address");
	}
	else loccnt += 2;
}


/* class 3 machine operations processor - various addressing modes */

class3(ip)
    int *ip;
{
	char	ch;
	int	code;
	int	flag;
	int	i;
	int	ztmask;

	while ((ch = prlnbuf[++(*ip)]) == ' ');
	switch(ch) {
	case 0:
	case ';':
		error("Operand field missing");
		return;
	case 'A':
	case 'a':
		if ((ch = prlnbuf[*ip + 1]) == ' ' || ch == 0) {
			flag = ACC;
			break;
		}
	default:
		switch(ch = prlnbuf[*ip]) {
		case '#': case '=':
			flag = IMM1 | IMM2;
			++(*ip);
			break;
		case '(':
			flag = IND | INDX | INDY;
			++(*ip);
			break;
		default:
			flag = ABS | ZER | ZERX | ABSX | ABSY | ABSY2 | ZERY;
		}
		if ((flag & (INDX | INDY | ZER | ZERX | ZERY) & opflg) != 0)
			udtype = UNDEFAB;
		if (evaluate(ip) != 0)
			return;
		if (zpref != 0) {
			flag &= (ABS | ABSX | ABSY | ABSY2 | IND | IMM1 | IMM2);
			ztmask = 0;
		}
		else ztmask = ZER | ZERX | ZERY;
		code = 0;
		i = 0;
		while (( ch = prlnbuf[(*ip)++]) != ' ' && ch != 0 && i++ < 4) {
			code *= 8;
			switch(ch) {
			case ')':		/* ) = 4 */
				++code;
			case ',':		/* , = 3 */
				++code;
			case 'X':		/* X = 2 */
			case 'x':
				++code;
			case 'Y':		/* Y = 1 */
			case 'y':
				++code;
				break;
			default:
				flag = 0;
			}
		}
		switch(code) {
		case 0:		/* no termination characters */
			flag &= (ABS | ZER | IMM1 | IMM2);
			break;
		case 4:		/* termination = ) */
			flag &= IND;
			break;
		case 25:		/* termination = ,Y */
			flag &= (ABSY | ABSY2 | ZERY);
			break;
		case 26:		/* termination = ,X */
			flag &= (ABSX | ZERX);
			break;
		case 212:		/* termination = ,X) */
			flag &= INDX;
			break;
		case 281:		/* termination = ),Y */
			flag &= INDY;
			break;
		default:
			flag = 0;
		}
	}
	if ((opflg &= flag) == 0) {
		error("Invalid addressing mode");
		return;
	}
	if ((opflg & ztmask) != 0)
		opflg &= ztmask;
	switch(opflg) {
	case ACC:		/* single byte - class 3 */
		if (pass == LAST_PASS) {
			loadlc(loccnt, 0, 1);
			loadv(opval + 8, 0, 1);
			println();
		}
		loccnt++;
		return;
	case ZERX: case ZERY:	/* double byte - class 3 */
		opval += 4;
	case INDY:
		opval += 8;
	case IMM2:
		opval += 4;
	case ZER:
		opval += 4;
	case INDX: case IMM1:
		if (pass == LAST_PASS) {
			loadlc(loccnt, 0, 1);
			loadv(opval, 0, 1);
			loadv(value, 1, 1);
			println();
		}
		loccnt += 2;
		return;
	case IND:		/* triple byte - class 3 */
		opval += 16;
	case ABSX:
	case ABSY2:
		opval += 4;
	case ABSY:
		opval += 12;
	case ABS:
		if (pass == LAST_PASS) {
			opval += 12;
			loadlc(loccnt, 0, 1);
			loadv(opval, 0, 1);
			loadv(value, 1, 1);
			loadv(value >> 8, 2, 1);
			println();
		}
		loccnt += 3;
		return;
	default:
		error("Invalid addressing mode");
		return;
	}
}

/* pseudo operations processor */

pseudo(ip)
    int *ip;
{
	int	count;
	int	i,j;
	int	tvalue;

	switch(opval) {
	case 0:				/* .byte pseudo */
		labldef(loccnt);
		loadlc(loccnt, 0, 1);
		while (prlnbuf[++(*ip)] == ' ');	/*    field	*/
		count = 0;
		do {
			if (prlnbuf[*ip] == '"') {
				while ((tvalue = prlnbuf[++(*ip)]) != '"') {
					if (tvalue == 0) {
						error("Unterminated ASCII string");
						return;
					}
					if (tvalue == '\\')
						switch(tvalue = prlnbuf[++(*ip)]) {
						case 'n':
							tvalue = '\n';
							break;
						case 't':
							tvalue = '\t';
							break;
						}
					loccnt++;
					if (pass == LAST_PASS) {
						loadv(tvalue, count, 1);
						if (++count >= 3) {
							println();
							for (i = 0; i < SFIELD; i++)
								prlnbuf[i] = ' ';
							prlnbuf[i] = 0;
							count = 0;
							loadlc(loccnt, 0, 1);
						}
					}
				}
				++(*ip);
			}
			else {
				if (evaluate(ip) != 0) {
					loccnt++;
					return;
				}
				loccnt++;
				if (value > 0xff) {
					error("Operand field size error");
					return;
				}
				else if (pass == LAST_PASS) {
					loadv(value, count, 1);
					if (++count >= 3) {
						println();
						for (i = 0; i < SFIELD; i++)
							prlnbuf[i] = ' ';
						prlnbuf[i] = 0;
						count = 0;
						loadlc(loccnt, 0, 1);
					}
				}
			}
		} while (prlnbuf[(*ip)++] == ',');
		if ((pass == LAST_PASS) && (count != 0))
			println();
		return;
	case 1:				/* = pseudo */
		while (prlnbuf[++(*ip)] == ' ');
		if (evaluate(ip) != 0)
			return;
		labldef(value);
		if (pass == LAST_PASS) {
			loadlc(value, 1, 0);
			println();
		}
		return;
	case 2:				/* .word pseudo */
		labldef(loccnt);
		loadlc(loccnt, 0, 1);
		while (prlnbuf[++(*ip)] == ' ');
		do {
			if (evaluate(ip) != 0) {
				loccnt += 2;
				return;
			}
			loccnt += 2;
			if (pass == LAST_PASS) {
				loadv(value, 0, 1);
				loadv(value>>8, 1, 1);
				println();
				for (i = 0; i < SFIELD; i++)
					prlnbuf[i] = ' ';
				prlnbuf[i] = 0;
				loadlc(loccnt, 0, 1);
			}
		} while (prlnbuf[(*ip)++] == ',');
		return;
	case 3:				/* *= pseudo */
		while (prlnbuf[++(*ip)] == ' ');
		if (prlnbuf[*ip] == '*') {
			if (evaluate(ip) != 0)
				return;
			if (undef != 0) {
				error("Undefined symbol in operand field.");
				return;
			}
			tvalue = loccnt;
		}
		else {
			if (evaluate(ip) != 0)
				return;
			if (undef != 0) {
				error("Undefined symbol in operand field.");
				return;
			}
			tvalue = value;
		}
		loccnt = value;
		labldef(tvalue);
		if (pass == LAST_PASS) {
			objcnt = 0;
			loadlc(tvalue, 1, 0);
			println();
		}
		return;
	case 4:				/* .list pseudo */
		if (lflag >= 0)
			lflag = 1;
		return;
	case 5:				/* .nlst pseudo */
		if (lflag >= 0)
			lflag = iflag;
		return;
	case 6:				/* .dbyt pseudo */
		labldef(loccnt);
		loadlc(loccnt, 0, 1);
		while (prlnbuf[++(*ip)] == ' ');
		do {
			if (evaluate(ip) != 0) {
				loccnt += 2;
				return;
			}
			loccnt += 2;
			if (pass == LAST_PASS) {
				loadv(value>>8, 0, 1);
				loadv(value, 1, 1);
				println();
				for (i = 0; i < SFIELD; i++)
					prlnbuf[i] = ' ';
				prlnbuf[i] = 0;
				loadlc(loccnt, 0, 1);
			}
		} while (prlnbuf[(*ip)++] == ',');
		return;
	case 7:                    /* .page pseudo  */
		if (pagesize == 0) return;
		while (prlnbuf[++(*ip)] == ' ');
		if (prlnbuf[(*ip)] == '"')
			{
			i=0;
			while ((tvalue = prlnbuf[++(*ip)]) != '"')
				{
				if (tvalue =='\0') {
					error("Unterminated ASCII string");
					return; }
				titlbuf[i++] = tvalue;
				if (i == titlesize) {
					error("Title too long");
					return; }
				}
			if (i<titlesize) for (j=i; j<titlesize; j++) titlbuf[j]=' ';
			}
		if (lflag > 0) printhead();
		return;
	}
}

/* evaluate expression */

evaluate(ip)
   int	*ip;
{
	int	tvalue;
	int	invalid;
	int	parflg, value2;
	char	ch;
	char	op;
	char	op2;

	op = '+';
	parflg = zpref = undef = value = invalid = 0;
/* hcj: zpref should reflect the value of the expression, not the value of
   the intermediate symbols
*/
	while ((ch=prlnbuf[*ip]) != ' ' && ch != ')' && ch != ',' && ch != ';') {
		tvalue = 0;
		if (ch == '$' || ch == '@' || ch == '%')
			tvalue = colnum(ip);
		else if (ch >= '0' && ch <= '9')
			tvalue = colnum(ip);
		else if (ch >= 'a' && ch <= 'z')
			tvalue = symval(ip);
		else if (ch >= 'A' && ch <= 'Z')
			tvalue = symval(ip);
		else if ((ch == '_') || (ch == '.'))
			tvalue = symval(ip);
		else if (ch == '*') {
			tvalue = loccnt;
			++(*ip);
		}
		else if (ch == '\'') {
			++(*ip);
			tvalue = prlnbuf[*ip] & 0xff;
			++(*ip);
		}
		else if (ch == '[') {
			if (parflg == 1) {
				error("Too many ['s in expression");
				invalid++;
			}
			else {
				value2 = value;
				op2 = op;
				value = tvalue = 0;
				op = '+';
				parflg = 1;
			}
			goto next;
		}
		else if (ch == ']') {
			if (parflg == 0) {
				error("No matching [ for ] in expression");
				invalid++;
			}
			else {
				parflg = 0;
				tvalue = value;
				value = value2;
				op = op2;
			}
			++(*ip);
		}
		switch(op) {
		case '+':
			value += tvalue;
			break;
		case '-':
			value -= tvalue;
			break;
		case '/':
			value = (unsigned) value/tvalue;
			break;
		case '*':
			value *= tvalue;
			break;
		case '%':
			value = (unsigned) value%tvalue;
			break;
		case '^':
			value ^= tvalue;
			break;
		case '~':
			value = ~tvalue;
			break;
		case '&':
			value &= tvalue;
			break;
		case '|':
			value |= tvalue;
			break;
		case '>':
			tvalue >>= 8;		/* fall through to '<' */
		case '<':
			if (value != 0) {
				error("High or low byte operator not first in operand field");
			}
			value = tvalue & 0xff;
			zpref = 0;
			break;
		default:
			invalid++;
		}
		if ((op=prlnbuf[*ip]) == ' '
				|| op == ')'
				|| op == ','
				|| op == ';')
			break;
		else if (op != ']')
next:			++(*ip);
	}
	if (parflg == 1) {
		error("Missing ] in expression");
		return(1);
	}
	if (value < 0 || value >= 256) {
		zpref = 1;
	}
	if (undef != 0) {
		if (pass != FIRST_PASS) {
			error("Undefined symbol in operand field");
			invalid++;
		}
		value = 0;
	}
	else if (invalid != 0)
	{
		error("Invalid operand field");
	}
	else {
/*
 This is the only way out that may not signal error
*/
		if (value < 0 || value >= 256) {
			zpref = 1;
		}
		else {
			zpref = 0;
		}
	}
	return(invalid);
}

/* collect number operand		*/

colnum(ip)
	int	*ip;
{
	int	mul;
	int	nval;
	char	ch;

	nval = 0;
	if ((ch = prlnbuf[*ip]) == '$')
		mul = 16;
	else if (ch >= '1' && ch <= '9') {
		mul = 10;
		nval = ch - '0';
	}
	else if (ch == '@' || ch == '0')
		mul = 8;
	else if (ch == '%')
		mul = 2;
	while ((ch = prlnbuf[++(*ip)] - '0') >= 0) {
		if (ch > 9) {
			ch -= ('A' - '9' - 1);
			if (ch > 15)
				ch -= ('a' - 'A');
			if (ch > 15)
				break;
			if (ch < 10)
				break;
		}
		if (ch >= mul)
			break;
		nval = (nval * mul) + ch;
	}
	return(nval);
}
