// Frankenstein Cross-Assemblers, version 2.0.
// Original author: Mark Zenier.
// The lexical analyzer for the framework cross-assembler.
#include <stdio.h>
#include "Extern.h"
#include "Token.h"

#define PRINTCTRL(Ch) ((Ch) + '@')

#ifndef DEBUG
#   define DEBUG 0
#endif

// extern YYSTYPE yylval; // Already declared in Token.h.

FILE *yyin;

char finbuff[INBUFFSZ] = "L:"; // Initialization non-reusable, wiped out by pass 2.
readacts nextreadact = Nra_normal;

// Read a line, and on end of file, pop the include file stack.
// Return:
// ∙	false:	got a line,
// ∙	true:	end of input.
static bool frareadrec(void) {
   while (fgets(&finbuff[2], INBUFFSZ - 2, yyin) == NULL) {
      if (currfstk == 0) return true;
      fclose(yyin), yyin = infilestk[--currfstk].fpt; // Reopen.
      fprintf(intermedf, "X:%s\n", infilestk[currfstk].fnm);
   }
   return false;
}

static int currtok = 0; // The subscript of the next token to return.
static int intokcnt = 0; // The number of tokens in the queue.

static struct {
   char *textstrt, *textend;
   YYSTYPE lvalv;
   int tokv;
   enum { Yetprint, Yetsymbol, Yetreserved, Yetopcode, Yetconstant, Yetstring, Yetunprint, Yetinvalid } errtype;
} scanqueue[INBUFFSZ], *lasttokfetch, *nexttokload;

static char tempstrpool[2*INBUFFSZ];

typedef enum char_t {
// Control Space   '\n'    Alpha    '"'     Etc     '$'     '%'     '\''    0-1     2-7     8-9     ';'
// '<'     '='     '>'     '@'      ACEF    'B'     'D'     Hh      OoQq    acef    'b'     'd'     '\\'
   SkipX = 0, SpaceX, NlX, AlphaX, Quote2X, EtcX, DollarX, PercentX, Quote1X, BinX, OctX, DecX, SemiX,
   LtX, EqX, GtX, AtX, HexX, BX, DX, HX, OoQqX, hexX, bX, dX, BackX,
   CharXs
} char_t;

static char_t chartrantab[0x80] = {
// nul soh stx etx eot enq ack bel bs  ht  nl  vt  np  cr  so  si
   SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SpaceX, NlX, SkipX, SkipX, SkipX, SkipX, SkipX,
// dle dc1 dc2 dc3 dc4 nak syn etb can em  sub esc fs  gs  rs  us
   SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX, SkipX,
// sp  !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
   SpaceX, AlphaX, Quote2X, EtcX, DollarX, PercentX, AlphaX, Quote1X, EtcX, EtcX, EtcX, EtcX, EtcX, EtcX, EtcX, EtcX,
// 0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
   BinX, BinX, OctX, OctX, OctX, OctX, OctX, OctX, DecX, DecX, EtcX, SemiX, LtX, EqX, GtX, EtcX,
// @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
   AtX, HexX, BX, HexX, DX, HexX, HexX, AlphaX, HX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, OoQqX,
// P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
   AlphaX, OoQqX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, EtcX, BackX, EtcX, AlphaX, AlphaX,
// `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
   EtcX, hexX, bX, hexX, dX, hexX, hexX, AlphaX, HX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, OoQqX,
// p   q   r   s   t   u   v   w   x   y   z   {   vb  }   ~   del
   AlphaX, OoQqX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, AlphaX, EtcX, EtcX, EtcX, AlphaX, SkipX
};

#if DEBUG
// Compensating for lack of transparency in the lexical scanner.
// Slated for removal, after root canal is done to nativize the C code and clean everything up.
static char *statelab[] = {
   " 0 start of label",
   " 1 comment",
   " 2 label",
   " 3 rest of line",
   " 4 symbol",
   " 5 dollar",
   " 6 hex dollar",
   " 7 at sign",
   " 8 octal at",
   " 9 percent",
   "10 bin percent",
   "11 quote string",
   "12 appos. string",
   "13 greater than",
   "14 less than",
   "15 base 2 maybe",
   "16 base 8 maybe",
   "17 base 10 maybe",
   "18 hex",
   "19 found b ",
   "20 found d",
   "21 bslash quote",
   "22 bslash appos",
};

static char *actlab[] = {
   " 0 skip/no op",
   " 1 load EOL token",
   " 2 start string",
   " 3 process label",
   " 4 save string char",
   " 5 load single char token",
   " 6 load EQ token",
   " 7 process symbol",
   " 8 load $ token",
   " 9 setup for $hex",
   "10 accumulate 0-9 constant",
   "11 accumulate A-F constant",
   "12 accumulate a-f constant",
   "13 load Constant token",
   "14 load @ token",
   "15 setup for @octal",
   "16 setup for %binary",
   "17 load % token",
   "18 load String token",
   "19 load GE token",
   "20 load GT token",
   "21 load LE token",
   "22 load NE token",
   "23 load LT token",
   "24 save numeric char 0-9",
   "25 save numeric char A-F",
   "26 save numeric char a-f",
   "27 convert numeric string base 2",
   "28 convert numeric string base 8",
   "29 convert numeric string base 10",
   "30 convert numeric string base 16",
   "31 save numeric 0xb",
   "32 save numeric 0xd",
   "33 set text start",
   "34 token choke"
};
#endif // DEBUG

const bool XX = false, oo = true;
static struct ActItem {
   char action, nextstate; bool contin;
} characttab[][CharXs] = {
     // Control    Space      '\n'       Letter     '"'        Other      '$'        '%'
     // '\''       0-1        2-7        8-9        ';'        '<'        '='        '>'
     // '@'        ACEF       'B'        'D'        Hh         OoQq       acef       'b'
     // 'd'        '\\'
// 0: Start of label
   {
      { 0, 0,XX}, { 0, 3,XX}, { 1, 0,XX}, { 2, 2,oo}, { 2,11,XX}, { 5, 3,XX}, {33, 5,XX}, {33, 9,XX},
      { 2,12,XX}, { 2,15,oo}, { 2,16,oo}, { 2,17,oo}, { 0, 1,XX}, { 0,14,XX}, { 6, 3,XX}, { 0,13,XX},
      {33, 7,XX}, { 2, 2,oo}, { 2, 2,oo}, { 2, 2,oo}, { 2, 2,oo}, { 2, 2,oo}, { 2, 2,oo}, { 2, 2,oo},
      { 2, 2,oo}, { 5, 3,XX}
   },
// 1: Comment
   {
      { 0, 1,XX}, { 0, 1,XX}, { 1, 0,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX},
      { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX},
      { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX}, { 0, 1,XX},
      { 0, 1,XX}, { 0, 1,XX}
   },
// 2: Label
   {
      { 0, 2,XX}, { 3, 3,XX}, { 3, 3,oo}, { 4, 2,XX}, { 3, 3,oo}, { 3, 3,oo}, { 3, 3,oo}, { 3, 3,oo},
      { 3, 3,oo}, { 4, 2,XX}, { 4, 2,XX}, { 4, 2,XX}, { 3, 1,XX}, { 3,14,XX}, { 3, 3,oo}, { 3,13,XX},
      { 3, 3,oo}, { 4, 2,XX}, { 4, 2,XX}, { 4, 2,XX}, { 4, 2,XX}, { 4, 2,XX}, { 4, 2,XX}, { 4, 2,XX},
      { 4, 2,XX}, { 3, 3,oo}
   },
// 3: ...\n
   {
      { 0, 3,XX}, { 0, 3,XX}, { 1, 0,XX}, { 2, 4,oo}, { 2,11,XX}, { 5, 3,XX}, {33, 5,XX}, {33, 9,XX},
      { 2,12,XX}, { 2,15,oo}, { 2,16,oo}, { 2,17,oo}, { 0, 1,XX}, { 0,14,XX}, { 6, 3,XX}, { 0,13,XX},
      {33, 7,XX}, { 2, 4,oo}, { 2, 4,oo}, { 2, 4,oo}, { 2, 4,oo}, { 2, 4,oo}, { 2, 4,oo}, { 2, 4,oo},
      { 2, 4,oo}, { 5, 3,XX}
   },
// 4: Symbol
   {
      { 0, 4,XX}, { 7, 3,XX}, { 7, 3,oo}, { 4, 4,XX}, { 7, 3,oo}, { 7, 3,oo}, { 7, 3,oo}, { 7, 3,oo},
      { 7, 3,oo}, { 4, 4,XX}, { 4, 4,XX}, { 4, 4,XX}, { 7, 1,XX}, { 7,14,XX}, { 7, 3,oo}, { 7,13,XX},
      { 7, 3,oo}, { 4, 4,XX}, { 4, 4,XX}, { 4, 4,XX}, { 4, 4,XX}, { 4, 4,XX}, { 4, 4,XX}, { 4, 4,XX},
      { 4, 4,XX}, { 7, 3,oo}
   },
// 5: $
   {
      { 0, 5,XX}, { 8, 3,XX}, { 8, 3,oo}, { 8, 3,oo}, { 8, 3,oo}, { 8, 3,oo}, { 8, 3,oo}, { 8, 3,oo},
      { 8, 3,oo}, { 9, 6,oo}, { 9, 6,oo}, { 9, 6,oo}, { 8, 1,XX}, { 8,14,XX}, { 8, 3,oo}, { 8,13,XX},
      { 8, 3,oo}, { 9, 6,oo}, { 9, 6,oo}, { 9, 6,oo}, { 8, 3,oo}, { 8, 3,oo}, { 9, 6,oo}, { 9, 6,oo},
      { 9, 6,oo}, { 8, 3,oo}
   },
// 6: $ [0-9a-fA-F]*
   {
      { 0, 6,XX}, {13, 3,XX}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo},
      {13, 3,oo}, {10, 6,XX}, {10, 6,XX}, {10, 6,XX}, {13, 1,XX}, {13,14,XX}, {13, 3,oo}, {13,13,XX},
      {13, 3,oo}, {11, 6,XX}, {11, 6,XX}, {11, 6,XX}, {13, 3,oo}, {13, 3,oo}, {12, 6,XX}, {12, 6,XX},
      {12, 6,XX}, {13, 3,oo}
   },
// 7: @
   {
      { 0, 7,XX}, {14, 3,XX}, {14, 3,oo}, {14, 3,oo}, {14, 3,oo}, {14, 3,oo}, {14, 3,oo}, {14, 3,oo},
      {14, 3,oo}, {15, 8,oo}, {15, 8,oo}, {14, 3,oo}, {14, 1,XX}, {14,14,XX}, {14, 3,oo}, {14,13,XX},
      {14, 3,oo}, {14, 3,oo}, {14, 3,oo}, {14, 3,oo}, {14, 3,oo}, {14, 3,oo}, {14, 3,oo}, {14, 3,oo},
      {14, 3,oo}, {14, 3,oo}
   },
// 8: @ [0-7]*
   {
      { 0, 8,XX}, {13, 3,XX}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo},
      {13, 3,oo}, {10, 8,XX}, {10, 8,XX}, {13, 3,oo}, {13, 1,XX}, {13,14,XX}, {13, 3,oo}, {13,13,XX},
      {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo},
      {13, 3,oo}, {13, 3,oo}
   },
// 9: %
   {
      { 0, 9,XX}, {17, 3,XX}, {17, 3,oo}, {17, 3,oo}, {17, 3,oo}, {17, 3,oo}, {17, 3,oo}, {17, 3,oo},
      {17, 3,oo}, {16,10,oo}, {17, 3,oo}, {17, 3,oo}, {17, 1,XX}, {17,14,XX}, {17, 3,oo}, {17,13,XX},
      {17, 3,oo}, {17, 3,oo}, {17, 3,oo}, {17, 3,oo}, {17, 3,oo}, {17, 3,oo}, {17, 3,oo}, {17, 3,oo},
      {17, 3,oo}, {17, 3,oo}
   },
// 10: % [0-1]*
   {
      { 0,10,XX}, {13, 3,XX}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo},
      {13, 3,oo}, {10,10,XX}, {13, 3,oo}, {13, 3,oo}, {13, 1,XX}, {13,14,XX}, {13, 3,oo}, {13,13,XX},
      {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo}, {13, 3,oo},
      {13, 3,oo}, {13, 3,oo}
   },
// 11: "..."
   {
      { 0,11,XX}, { 4,11,XX}, {34, 3,oo}, { 4,11,XX}, {18, 3,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX},
      { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX},
      { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX},
      { 4,11,XX}, { 4,21,XX}
   },
// 12: '...'
   {
      { 0,12,XX}, { 4,12,XX}, {34, 3,oo}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX},
      {18, 3,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX},
      { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX},
      { 4,12,XX}, { 4,22,XX}
   },
// 13: >
   {
      { 0,13,XX}, {20, 3,XX}, {20, 3,oo}, {20, 3,oo}, {20, 3,oo}, {20, 3,oo}, {20, 3,oo}, {20, 3,oo},
      {20, 3,oo}, {20, 3,oo}, {20, 3,oo}, {20, 3,oo}, {20, 1,XX}, {20,14,XX}, {19, 3,XX}, {20,13,XX},
      {20, 3,oo}, {20, 3,oo}, {20, 3,oo}, {20, 3,oo}, {20, 3,oo}, {20, 3,oo}, {20, 3,oo}, {20, 3,oo},
      {20, 3,oo}, {20, 3,oo}
   },
// 14: <
   {
      { 0,14,XX}, {23, 3,XX}, {23, 3,oo}, {23, 3,oo}, {23, 3,oo}, {23, 3,oo}, {23, 3,oo}, {23, 3,oo},
      {23, 3,oo}, {23, 3,oo}, {23, 3,oo}, {23, 3,oo}, {23, 1,XX}, {23,14,XX}, {21, 3,XX}, {22,13,XX},
      {23, 3,oo}, {23, 3,oo}, {23, 3,oo}, {23, 3,oo}, {23, 3,oo}, {23, 3,oo}, {23, 3,oo}, {23, 3,oo},
      {23, 3,oo}, {23, 3,oo}
    },
// 15: Base 2; or maybe 8, 10 or 16
   {
      { 0,15,XX}, {29, 3,XX}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo},
      {29, 3,oo}, {24,15,XX}, {24,16,XX}, {24,17,XX}, {29, 1,XX}, {29,14,XX}, {29, 3,oo}, {29,13,XX},
      {29, 3,oo}, {25,18,XX}, { 0,19,XX}, { 0,20,XX}, {30, 3,XX}, {28, 3,XX}, {26,18,XX}, { 0,19,XX},
      { 0,20,XX}, {29, 3,oo}
   },
// 16: Base 8; or maybe 10 or 16
   {
      { 0,16,XX}, {29, 3,XX}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo},
      {29, 3,oo}, {24,16,XX}, {24,16,XX}, {24,17,XX}, {29, 1,XX}, {29,14,XX}, {29, 3,oo}, {29,13,XX},
      {29, 3,oo}, {25,18,XX}, {25,18,XX}, { 0,20,XX}, {30, 3,XX}, {28, 3,XX}, {26,18,XX}, {26,18,XX},
      { 0,20,XX}, {29, 3,oo}
   },
// 17: Base 10; or maybe 16
   {
      { 0,17,XX}, {29, 3,XX}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo},
      {29, 3,oo}, {24,17,XX}, {24,17,XX}, {24,17,XX}, {29, 1,XX}, {29,14,XX}, {29, 3,oo}, {29,13,XX},
      {29, 3,oo}, {25,18,XX}, {25,18,XX}, { 0,20,XX}, {30, 3,XX}, {34, 3,XX}, {26,18,XX}, {26,18,XX},
      { 0,20,XX}, {29, 3,oo}
   },
// 18: Base 16
   {
      { 0,18,XX}, {34, 3,XX}, {34, 3,oo}, {34, 3,oo}, {34, 3,oo}, {34, 3,oo}, {34, 3,oo}, {34, 3,oo},
      {34, 3,oo}, {24,18,XX}, {24,18,XX}, {24,18,XX}, {34, 1,XX}, {34,14,XX}, {34, 3,oo}, {34,13,XX},
      {34, 3,oo}, {25,18,XX}, {25,18,XX}, {25,18,XX}, {30, 3,XX}, {34, 3,oo}, {26,18,XX}, {26,18,XX},
      {26,18,XX}, {34, 3,oo}
   },
// 19: Base 2 or 16
   {
      { 0,19,XX}, {27, 3,XX}, {27, 3,oo}, {27, 3,oo}, {27, 3,oo}, {27, 3,oo}, {27, 3,oo}, {27, 3,oo},
      {27, 3,oo}, {31,18,oo}, {31,18,oo}, {31,18,oo}, {27, 1,XX}, {27,14,XX}, {27, 3,oo}, {27,13,XX},
      {27, 3,oo}, {31,18,oo}, {31,18,oo}, {31,18,oo}, {31,18,oo}, {27, 3,oo}, {31,18,oo}, {31,18,oo},
      {31,18,oo}, {27, 3,oo}
   },
// 20: Base 10 or 16
   {
      { 0,20,XX}, {29, 3,XX}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo}, {29, 3,oo},
      {29, 3,oo}, {32,18,oo}, {32,18,oo}, {32,18,oo}, {29, 1,XX}, {29,14,XX}, {29, 3,oo}, {29,13,XX},
      {29, 3,oo}, {32,18,oo}, {32,18,oo}, {32,18,oo}, {32,18,oo}, {29, 3,oo}, {32,18,oo}, {32,18,oo},
      {32,18,oo}, {29, 3,oo}
   },
// 21: \"
   {
      { 0,21,XX}, { 4,11,XX}, {34, 3,oo}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX},
      { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX},
      { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX}, { 4,11,XX},
      { 4,11,XX}, { 4,11,XX}
   },
// 22: \'
   {
      { 0,22,XX}, { 4,12,XX}, {34, 3,oo}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX},
      { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX},
      { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX}, { 4,12,XX},
      { 4,12,XX}, { 4,12,XX}
   }
};

static char yytext[0x20];
static const size_t YEXL = sizeof yytext/sizeof yytext[0];

static char *erryytextex(int type) {
   char *strptr = lasttokfetch->textstrt - 1;
   char *endptr = lasttokfetch->textend - 1;
   if (type != STRING || *endptr == '\n') endptr--;
   int charcnt = 0;
   for (; strptr <= endptr && charcnt < YEXL - 1; strptr++, charcnt++) yytext[charcnt] = *strptr;
   yytext[charcnt] = '\0';
}

int Scan(void) {
   if (currtok >= intokcnt) {
      switch (nextreadact) {
         case Nra_new: // Get the next file.
            fprintf(intermedf, "F:%s\n", infilestk[++currfstk].fnm);
            yyin = infilestk[currfstk].fpt;
            nextreadact = Nra_normal;
         case Nra_normal:
            if (frareadrec()) return 0; // EOF
         break;
         case Nra_end: // Pop the file and get the previous one.
            if (currfstk > 0) {
               fclose(yyin);
               yyin = infilestk[--currfstk].fpt;
               fprintf(intermedf, "X:%s\n", infilestk[currfstk].fnm);
               if (frareadrec()) return 0; // EOF
               nextreadact = Nra_normal;
            } else return 0; // EOF
         break;
      }
      fputs(listflag? finbuff: "L:\n", intermedf);
   // Scan a line.
      char *frainptr = &finbuff[2]; // Point to the NUL byte after L: on start up.
      currtok = intokcnt = 0, nexttokload = scanqueue;
      char *tptrstr = tempstrpool;
      bool havesym = false; // true: symbol, false: opcode
      char *thistokstart;
      long consaccum, consbase;
      int scanstate = 0;
      for (char nextchar; (nextchar = *frainptr++) != '\0'; ) {
         char_t charset = chartrantab[nextchar&0x7f];
         struct ActItem *thisact;
         do {
            thisact = &characttab[scanstate][charset];
#if DEBUG
            if (isprint(nextchar)) printf("%c    ", nextchar); else printf("0x%2.2x ", nextchar);
            printf("%-18s %-33s %-11s %2.2d\n", statelab[scanstate], actlab[thisact->action], thisact->contin? "Continue": "Swallow", thisact->nextstate);
#endif
            switch (thisact->action) {
            // Skip/no op.
               case 0: break;
            // Load '\n'.
               case 1:
                  nexttokload->lvalv.longv = 0, nexttokload->tokv = EOL, nexttokload->errtype = Yetunprint;
                  nexttokload++, intokcnt++;
               break;
            // Start a string.
               case 2:
                  thistokstart = tptrstr, nexttokload->textstrt = frainptr;
               break;
            // Process a label.
               case 3: {
                  *tptrstr++ = '\0';
                  struct symel *tempsym = symbentry(thistokstart, SYMBOL);
                  if (tempsym->seg != SSG_RESV)
                     nexttokload->tokv = LABEL, nexttokload->errtype = Yetsymbol, nexttokload->lvalv.symb = tempsym;
                  else
                     nexttokload->tokv = tempsym->tok, nexttokload->errtype = Yetreserved, nexttokload->lvalv.intv = tempsym->value;
                  nexttokload->textend = frainptr;
                  nexttokload++, intokcnt++;
               }
               break;
            // Save a string character.
               case 4:
                  *tptrstr++ = nextchar;
               break;
            // Load a single character token.
               case 5:
                  nexttokload->lvalv.longv = 0, nexttokload->tokv = nextchar, nexttokload->errtype = Yetprint;
                  nexttokload++, intokcnt++;
               break;
            // Load '='.
               case 6:
                  nexttokload->lvalv.longv = 0, nexttokload->tokv = KEOP_EQ, nexttokload->errtype = Yetunprint;
                  nexttokload++, intokcnt++;
               break;
            // Process a symbol.
               case 7:
                  *tptrstr++ = '\0';
                  if (!havesym) {
                     for (char *ytp = thistokstart; *ytp != '\0'; ytp++) if (islower(*ytp)) *ytp = toupper(*ytp);
                     int tempov = findop(thistokstart);
                     nexttokload->lvalv.intv = tempov, nexttokload->tokv = optab[tempov].token, nexttokload->errtype = Yetopcode;
                     havesym = true;
                  } else {
                     struct symel *symp = symbentry(thistokstart, SYMBOL);
                     if (symp->seg != SSG_RESV)
                        nexttokload->lvalv.symb = symp, nexttokload->errtype = Yetsymbol;
                     else
                        nexttokload->lvalv.intv = symp->value, nexttokload->errtype = Yetreserved;
                     nexttokload->tokv = symp->tok;
                  }
                  nexttokload->textend = frainptr;
                  nexttokload++, intokcnt++;
               break;
            // Load '$'.
               case 8:
                  nexttokload->lvalv.longv = 0, nexttokload->tokv = '$', nexttokload->errtype = Yetprint;
                  nexttokload++, intokcnt++;
               break;
            // Set up for $[0-9a-fA-F]*.
               case 9:
                  consbase = 0x10, consaccum = 0;
               break;
            // Accumulate [0-9].
               case 10:
                  consaccum = consbase*consaccum + nextchar - '0';
               break;
            // Accumulate [A-F].
               case 11:
                  consaccum = consbase*consaccum + (nextchar - 'A' + 10);
               break;
            // Accumulate [a-f].
               case 12:
                  consaccum = consbase*consaccum + (nextchar - 'a' + 10);
               break;
            // Load a Constant token.
               case 13:
                  nexttokload->lvalv.longv = consaccum, nexttokload->tokv = CONSTANT;
                  nexttokload->errtype = Yetconstant, nexttokload->textend = frainptr;
                  nexttokload++, intokcnt++;
               break;
            // Load '@'.
               case 14:
                  nexttokload->lvalv.longv = 0, nexttokload->tokv = '@', nexttokload->errtype = Yetprint;
                  nexttokload++, intokcnt++;
               break;
            // Set up for @[0-7]*.
               case 15:
                  consbase = 010, consaccum = 0;
               break;
            // Set up for %[0-1]*.
               case 16:
                  consbase = 2, consaccum = 0;
               break;
            // Load '%'.
               case 17:
                  nexttokload->lvalv.longv = 0, nexttokload->tokv = '%', nexttokload->errtype = Yetprint;
                  nexttokload++, intokcnt++;
               break;
            // Load a String token.
               case 18:
                  *tptrstr++ = '\0';
                  nexttokload->lvalv.strng = thistokstart, nexttokload->tokv = STRING;
                  nexttokload->errtype = Yetstring, nexttokload->textend = frainptr;
                  nexttokload++, intokcnt++;
               break;
            // Load '≥'.
               case 19:
                  nexttokload->lvalv.longv = 0, nexttokload->tokv = KEOP_GE, nexttokload->errtype = Yetunprint;
                  nexttokload++, intokcnt++;
               break;
            // Load '>'.
               case 20:
                  nexttokload->lvalv.longv = 0, nexttokload->tokv = KEOP_GT, nexttokload->errtype = Yetunprint;
                  nexttokload++, intokcnt++;
               break;
            // Load '≤'.
               case 21:
                  nexttokload->lvalv.longv = 0, nexttokload->tokv = KEOP_LE, nexttokload->errtype = Yetunprint;
                  nexttokload++, intokcnt++;
               break;
            // Load '≠'.
               case 22:
                  nexttokload->lvalv.longv = 0, nexttokload->tokv = KEOP_NE, nexttokload->errtype = Yetunprint;
                  nexttokload++, intokcnt++;
               break;
            // Load '<'.
               case 23:
                  nexttokload->lvalv.longv = 0, nexttokload->tokv = KEOP_LT, nexttokload->errtype = Yetunprint;
                  nexttokload++, intokcnt++;
               break;
            // Save a numeric character [0-9].
               case 24:
                  *tptrstr++ = nextchar - '0';
               break;
            // Save a numeric character [A-F].
               case 25:
                  *tptrstr++ = nextchar - 'A' + 10;
               break;
            // Save a numeric character [a-f].
               case 26:
                  *tptrstr++ = nextchar - 'a' + 10;
               break;
            // Convert a numeric string base 2.
               case 27:
                  for (consaccum = 0; thistokstart < tptrstr; thistokstart++) consaccum = 2*consaccum + *thistokstart;
                  nexttokload->lvalv.longv = consaccum, nexttokload->tokv = CONSTANT;
                  nexttokload->errtype = Yetconstant, nexttokload->textend = frainptr;
                  nexttokload++, intokcnt++;
               break;
            // Convert a numeric string base 8.
               case 28:
                  for (consaccum = 0; thistokstart < tptrstr; thistokstart++) consaccum = 010*consaccum + *thistokstart;
                  nexttokload->lvalv.longv = consaccum, nexttokload->tokv = CONSTANT;
                  nexttokload->errtype = Yetconstant, nexttokload->textend = frainptr;
                  nexttokload++, intokcnt++;
               break;
            // Convert a numeric string base 10.
               case 29:
                  for (consaccum = 0; thistokstart < tptrstr; thistokstart++) consaccum = 10*consaccum + *thistokstart;
                  nexttokload->lvalv.longv = consaccum, nexttokload->tokv = CONSTANT;
                  nexttokload->errtype = Yetconstant, nexttokload->textend = frainptr;
                  nexttokload++, intokcnt++;
               break;
            // Convert a numeric string base 16.
               case 30:
                  for (consaccum = 0; thistokstart < tptrstr; thistokstart++) consaccum = 0x10*consaccum + *thistokstart;
                  nexttokload->lvalv.longv = consaccum, nexttokload->tokv = CONSTANT;
                  nexttokload->errtype = Yetconstant, nexttokload->textend = frainptr;
                  nexttokload++, intokcnt++;
               break;
            // Save numeric 0xb.
               case 31:
                  *tptrstr++ = 0xb;
               break;
            // Save numeric 0xd.
               case 32:
                  *tptrstr++ = 0xd;
               break;
            // Set the text start.
               case 33:
                  nexttokload->textstrt = frainptr;
               break;
            // Token choke.
               case 34:
                  nexttokload->lvalv.longv = 0L, nexttokload->tokv = KTK_invalid;
                  nexttokload->errtype = Yetinvalid, nexttokload->textend = frainptr;
                  nexttokload++, intokcnt++;
               break;
            }
            scanstate = thisact->nextstate;
         } while (thisact->contin);
      }
   // If no tokens were in the line (comment or whitespace overlength).
      if (intokcnt <= 0) scanqueue[0].tokv = EOL, scanqueue[0].errtype = Yetunprint, scanqueue[0].lvalv.longv = 0, intokcnt = 1;
   // If there was no '\n'.
      if (scanstate != 0) fraerror("Overlength/Unterminated Line");
   }
   lasttokfetch = &scanqueue[currtok++];
   yylval = lasttokfetch->lvalv;
   return lasttokfetch->tokv;
}

// First pass ― output a parser error to intermediate file.
void yyerror(char *str) {
   switch (lasttokfetch->errtype) {
      case Yetprint:
         if (!isprint(lasttokfetch->tokv))
            fprintf(intermedf, "E: ERROR - %s at/before character \"^%c\"\n", str, PRINTCTRL(lasttokfetch->tokv));
         else
            fprintf(intermedf, "E: ERROR - %s at/before character \"%c\"\n", str, lasttokfetch->tokv);
      break;
      case Yetsymbol: case Yetreserved: case Yetopcode: case Yetconstant:
         erryytextex(SYMBOL), fprintf(intermedf, "E: ERROR - %s at/before token \"%s\" \n", str, yytext);
      break;
      case Yetinvalid:
         erryytextex(SYMBOL), fprintf(intermedf, "E: ERROR - %s at invalid token \"%s\" \n", str, yytext);
      break;
      case Yetstring:
         erryytextex(STRING), fprintf(intermedf, "E: ERROR - %s at/before string %s \n", str, yytext);
      break;
      case Yetunprint: {
         char *taglab;
         switch (lasttokfetch->tokv) {
            case EOL: taglab = "End of Line"; break;
            case KEOP_EQ: taglab = "\"=\""; break;
            case KEOP_GE: taglab = "\">=\""; break;
            case KEOP_GT: taglab = "\">\""; break;
            case KEOP_LE: taglab = "\"<=\""; break;
            case KEOP_NE: taglab = "\"<>\""; break;
            case KEOP_LT: taglab = "\"<\""; break;
            default: taglab = "Undeterminable Symbol"; break;
         }
         fprintf(intermedf, "E: ERROR - %s at/before %s\n", str, taglab);
      }
      break;
      default: fprintf(intermedf, "E: ERROR - %s - undetermined yyerror type\n", str); break;
   }
   errorcnt++;
}
