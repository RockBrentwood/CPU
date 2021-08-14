#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "Constants.h"
#include "Extern.h"

char hex[5]; // hexadecimal character buffer
static int lablptr; // label pointer into symbol table
int objcnt; // object byte counter
int opflg; // operation code flags
char prlnbuf[LAST_CH_POS + 1]; // print line buffer
int slnum; // source line number counter

char *symtab; // symbol table
#if 0
struct sym_tab {
   char size;
   char chars[size];
   char flag;
   int value;
   int line defined
   char number of references
   int line referenced
// The above occurs 0 or more times.
};
#endif
unsigned size; // symbol table size
char symbol[SBOLSZ]; // temporary symbol storage
int udtype; // undefined symbol type
int undef; // undefined symbol in expression flg

// Print the page heading.
void printhead(void) {
   if (pagesize == 0) return;
   pagect++;
   fprintf(stdout,
      "\f\n"
      "Amiga 6502 assembler :  -  %s PAGE %d\n"
      "Line      Location     Label Opcode Operand  Comment   %s\n"
      "\n",
      titlbuf, pagect, date
   );
   paglin = 0;
}

// Print the contents of prlnbuf.
void println(void) {
   if (lflag > 0) {
      if (paglin == pagesize) printhead();
      prlnbuf[linesize] = '\0';
      fprintf(stdout, "%s\n", prlnbuf);
      paglin++;
   }
}

// Convert the number supplied as an argument to hexadecimal in hex[digit] (lsd) through hex[1] (msd).
void hexcon(int digit, int num) {
   for (; digit > 0; digit--) {
      hex[digit] = (num&0x0f) + '0';
      if (hex[digit] > '9') hex[digit] += 'A' - '9' - 1;
      num >>= 4;
   }
}

// Object code record generation routines.
static char obj_rec[60]; // The buffer for the object record.
static unsigned obj_ptr = 0; // The pointer for the buffer.
static unsigned obj_bytes = 0; // The number of bytes in the current record.
static unsigned rec_cnt = 0; // The number of records in this file.
static unsigned cksum = 0; // The record check sum accumulator.

// MOS Tech. object format is as follows:
// ( all data is in ASCII encoded hexidecimal)
//	Data record: ;nnaaaadddd...xxxx[cr]
//	Last record: ;00ccccxxxx[cr]
// Where:
//	;	=	Start of record (ASCII 3B)
//	nn	=	The number of data bytes in the record.
//			max = 24 bytes.
//	aaaa	=	The address of the first data byte in the record.
//	dd	=	1 data byte.
//	xxxx	=	The checksum that is the twos compliment sum of all data bytes, the count byte and the address bytes.
//	cccc	=	The number of records in the file.
//	[cr]	=	Carriage Return (0x0d in ASCII).

// Put one object byte in hex.
static void put_obj(unsigned val) {
   hexcon(2, val);
   obj_rec[obj_ptr++] = hex[1];
   obj_rec[obj_ptr++] = hex[2];
   cksum += val&0xff;
   obj_bytes++;
}

// Print the current object record if any.
static void prt_obj(void) {
   if (obj_bytes == 0) return;
   cksum += obj_bytes;
   hexcon(2, obj_bytes);
   obj_rec[obj_ptr] = '\0';
   fprintf(optr, ";%c%c%s", hex[1], hex[2], obj_rec);
   hexcon(4, cksum);
   fprintf(optr, "%c%c%c%c\n", hex[1], hex[2], hex[3], hex[4]);
}

// Start an object record (end previous).
// ∙	val:	the current location counter.
static void start_obj(unsigned val) {
   prt_obj(); // Print the current record if any.
   hexcon(4, val);
   obj_bytes = 0;
   for (obj_ptr = 0; obj_ptr < 4; obj_ptr++) obj_rec[obj_ptr] = hex[obj_ptr + 1];
   cksum = (val >> 8) + (val&0xff);
   rec_cnt++;
}

// Finish the object file.
void fin_obj(void) {
   prt_obj();
   hexcon(4, ++rec_cnt);
   fprintf(optr, ";00");
   for (unsigned i = 1; i < 5; i++) fputc(hex[i], optr);
   rec_cnt = rec_cnt / 256 + (rec_cnt&0xff);
   hexcon(4, rec_cnt);
   for (unsigned i = 1; i < 5; i++) fputc(hex[i], optr);
   fputc('\n', optr);
}

// Load a 16-bit value in printable form into prlnbuf.
void loadlc(int val, int f, int outflg) {
   int i = 6 + 7*f;
   hexcon(4, val);
   if (nflag == 0)
      prlnbuf[i++] = hex[3], prlnbuf[i++] = hex[4], prlnbuf[i++] = ':', prlnbuf[i++] = hex[1], prlnbuf[i] = hex[2];
   else
      prlnbuf[i++] = hex[1], prlnbuf[i++] = hex[2], prlnbuf[i++] = hex[3], prlnbuf[i] = hex[4];
   if (pass == LAST_PASS && oflag != 0 && objcnt <= 0 && outflg != 0) {
      if (mflag != 0) start_obj(val); else fprintf(optr, "\n;%c%c%c%c", hex[3], hex[4], hex[1], hex[2]);
      objcnt = 22;
   }
}

// Load a value in hex into prlnbuf[contents[i]] and output hex characters to obuf if LAST_PASS&oflag == 1.
// ∙	f:	the contents field subscript.
// ∙	outflg:	a flag to output object bytes.
void loadv(int val, int f, int outflg) {
   hexcon(2, val);
   prlnbuf[13 + 3*f] = hex[1];
   prlnbuf[14 + 3*f] = hex[2];
   if (pass == LAST_PASS && oflag != 0 && outflg != 0) {
      if (mflag != 0)
         put_obj(val);
      else
         fputc(hex[1], optr), fputc(hex[2], optr);
      --objcnt;
   }
}

// Error printing routine.
void error(char *stptr) {
   loadlc(loccnt, 0, 1);
   loccnt += 3;
   loadv(0, 0, 0);
   loadv(0, 1, 0);
   loadv(0, 2, 0);
   fprintf(stderr, "%s\n", prlnbuf);
   fprintf(stderr, "%s\n", stptr);
   errcnt++;
}

// Collect a symbol from prlnbuf into symbol[],
// ∙	leave prlnbuf pointer at the first invalid symbol character,
// ∙	return 0 if no symbol is collected.
static int colsym(int *ip) {
   int valid = 1;
   int i = 0;
   while (valid == 1) {
      char ch = prlnbuf[*ip];
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
   if (i == 1) switch (symbol[1]) {
      case 'A': case 'X': case 'Y':
      case 'a': case 'x': case 'y':
         error("Symbol is reserved (A, X or Y)"), i = 0;
      break;
   }
   return symbol[0] = i;
}

// Open up a space in the symbol table
// the space will be at (ptr) and will be len characters long.
// return -1 if no room.
static int openspc(int ptr, int len) {
   if (nxt_free + len > size) return -1;
   if (ptr != nxt_free)
      for (int ptr2 = nxt_free - 1, ptr3 = ptr2 + len; ptr2 >= ptr; ptr2--, ptr3--) symtab[ptr3] = symtab[ptr2];
   nxt_free += len;
   if (lablptr >= ptr) lablptr += len;
   return 0;
}

// Install a symbol into symtab.
static int stinstal(int ptr) {
   if (openspc(ptr, symbol[0] + 7) == -1) {
      error("Symbol Table Full"); /* print error msg and ...  */
      pass = DONE; /* cause termination of assembly */
      return -1;
   }
   int ptr2 = ptr;
   for (int i = 0; i < symbol[0] + 1; i++) symtab[ptr2++] = symbol[i];
   symtab[ptr2++] = udtype, symtab[ptr2 + 4] = 0;
   return ptr;
}

// Symbol table lookup:
// ∙	if found, return a pointer to the symbol,
// ∙	else, install the symbol as undefined, and return the pointer.
static int stlook(void) {
   int ptr = 0;
   while (ptr < nxt_free) {
      int ln = symbol[0];
      if (symtab[ptr] < ln) ln = symtab[ptr];
      int eq = strncmp(&symtab[ptr + 1], &symbol[1], ln);
      if (eq == 0 && symtab[ptr] == symbol[0]) return ptr;
      if (eq > 0) return (stinstal(ptr));
      ptr = ptr + 6 + symtab[ptr];
      ptr = ptr + 1 + 2*(symtab[ptr]&0xff);
   }
   return (stinstal(ptr));
}

// Operation code table lookup:
// ∙	if found, return a pointer to the symbol,
// ∙	else, return -1.
static int oplook(int *ip) {
   char ch;
   int k;
   int i = 0, j = 0;
   int temp[2] = { 0, 0 };
   while ((ch = prlnbuf[*ip]) != ' ' && ch != '\0' && ch != '\t' && ch != ';') {
      if (ch >= 'A' && ch <= 'Z') ch &= 0x1f;
      else if (ch >= 'a' && ch <= 'z') ch &= 0x1f;
      else if (ch == '.') ch = 31;
      else if (ch == '*') ch = 30;
      else if (ch == '=') ch = 29;
      else return -1;
      temp[j] = temp[j] << 5 | ch&0xff;
      if (ch == 29) break;
      ++*ip;
      if (++i >= 3) {
         i = 0;
         if (++j >= 2) return -1;
      }
   }
   j = temp[0] ^ temp[1];
   if (j == 0) return -2;
   k = 0;
   i = step[k] - 1;
   do {
      if (j == optab[i].Nmemonic) {
         opflg = optab[i].Operation, opval = optab[i].Code;
         return 3*i + 2;
      } else if (j < optab[i].Nmemonic) i -= step[++k];
      else i += step[++k];
   } while (step[k] > 0);
   return -1;
}

// Assign <value> to label pointed to by lablptr, checking for valid definition, etc.
int labldef(int lval) {
   int i;
   if (lablptr != -1) {
      lablptr += symtab[lablptr] + 1;
      if (pass == FIRST_PASS) {
         if (symtab[lablptr] == UNDEF) {
            symtab[lablptr + 1] = lval&0xff;
            int i = symtab[lablptr + 2] = (lval >> 8)&0xff;
            symtab[lablptr] = i == 0? DEFZRO: DEFABS;
         } else if (symtab[lablptr] == UNDEFAB)
            symtab[lablptr] = DEFABS, symtab[lablptr + 1] = lval&0xff, symtab[lablptr + 2] = (lval >> 8)&0xff;
         else {
            symtab[lablptr] = MDEF, symtab[lablptr + 1] = 0, symtab[lablptr + 2] = 0;
            error("Label multiply defined");
            return -1;
         }
         symtab[lablptr + 3] = slnum&0xff, symtab[lablptr + 4] = (slnum >> 8)&0xff;
      } else {
         i = ((symtab[lablptr + 2] << 8) + (symtab[lablptr + 1]&0xff))&0xffff;
         if (i != lval && pass == LAST_PASS) {
            error("Sync error");
            return -1;
         }
      }
   }
   return 0;
}

// Translate a source line to machine language.
void assemble(void) {
   if (prlnbuf[SFIELD] == ';' | prlnbuf[SFIELD] == 0) {
      if (pass == LAST_PASS) println();
      return;
   }
   lablptr = -1;
   int i = SFIELD; // prlnbuf pointer.
   udtype = UNDEF;
   if (colsym(&i) != 0 && (lablptr = stlook()) == -1)
      return;
   while (prlnbuf[++i] == ' '); /* find first non-space */
   int flg = oplook(&i);
   if (flg < 0) { // Collect operation code.
      labldef(loccnt);
      if (flg == -1) error("Invalid operation code");
      else if (flg == -2 && pass == LAST_PASS) {
         if (lablptr != -1) loadlc(loccnt, 1, 0);
         println();
      }
      return;
   }
   if (opflg == PSEUDO) pseudo(&i);
   else if (labldef(loccnt) == -1) return;
   else if (opflg == CLASS1) class1();
   else if (opflg == CLASS2) class2(&i);
   else class3(&i);
}

// Add a reference line to the symbol pointed to by ip.
static void addref(int ip) {
   int rct = ip + symtab[ip] + 6;
   int ptr = rct;
   if ((symtab[rct]&0xff) == 255) { /* non-fatal error   */
      fprintf(stderr, "%s\n", prlnbuf);
      fprintf(stderr, "Too many references\n");
      return;
   }
   ptr += (symtab[rct]&0xff)*2 + 1;
   if (openspc(ptr, 2) == -1) {
      error("Symbol Table Full");
      return;
   }
   symtab[ptr] = slnum&0xff, symtab[ptr + 1] = (slnum >> 8)&0xff;
   symtab[rct]++;
}

// Determine the value of the symbol, given a pointer to first character of symbol in symtab.
int symval(int *ip) {
   int svalue = 0;
   colsym(ip);
   int ptr = stlook();
   if (ptr == -1) undef = 1; // No-room error.
   else if (symtab[ptr + symtab[ptr] + 1] == UNDEF) undef = 1;
   else if (symtab[ptr + symtab[ptr] + 1] == UNDEFAB) undef = 1;
   else svalue = ((symtab[ptr + symtab[ptr] + 3] << 8) + (symtab[ptr + symtab[ptr] + 2]&0xff))&0xffff;
   if (symtab[ptr + symtab[ptr] + 1] == DEFABS) zpref = 1;
   if (undef != 0) zpref = 1;
// Add a reference entry to symbol table on first pass only,
// except for branch instructions (CLASS2) which do not come through here on the first pass.
   if (ptr >= 0 && pass == FIRST_PASS) addref(ptr);
   if (ptr >= 0 && opflg == CLASS2) addref(ptr); // Branch addresses.
   return svalue;
}
