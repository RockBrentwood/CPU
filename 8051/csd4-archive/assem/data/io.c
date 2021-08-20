#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "io.h"
#include "op.h"
 
extern int Active;
 
/* SKIP LISTS are used to implement the symbol table */
 
char *CopyS(char *S) {
   char *NewS;
   if (S == 0) return 0;
   NewS = Allocate(strlen(S) + 1);
   strcpy(NewS, S);
   return NewS;
}
 
int CompS(char *A, char *B) {
   if (A == 0) return (B == 0)? 0: +1;
   if (B == 0) return -1;
   for (; *A != '\0'; A++, B++) {
      int Diff = tolower(*A) - tolower(*B);
      if (Diff < 0) return -1;
      if (Diff > 0) return +1;
   }
   return (*B == '\0')? 0: -1;
}
 
static int Random(void) {
   static byte B = 0, X = 0;
   register int L = 0, D;
   while (1) {
      if (X == 0) X = 4, B = rand()&0xff;
      D = B&3, B >>= 2, X--;
      if (D) return L;
      L++;
   }
}
 
#define MAX_BREADTH 16
Symbol NIL;
static Symbol Path[MAX_BREADTH];
static int Breadth;
 
static Symbol Form(int B, char *Name) {
   Symbol N = (Symbol)malloc(sizeof *N + B*sizeof(Symbol));
   if (N == 0) fprintf(stderr, "Out of memory.\n"), exit(1);
   N->Name = CopyS(Name),
   N->Defined = N->Global = N->Variable = N->Address = N->Map = 0;
   return N;
}
 
#include <time.h>
 
void SymInit(void) {
   int K;
   srand((unsigned)time(NULL));
   NIL = Form(MAX_BREADTH - 1, 0);
   for (K = 0; K < MAX_BREADTH; K++) NIL->Next[K] = NIL;
   Breadth = 0;
}
 
int Next, Value;
int Line, StartLine;
 
Symbol LookUp(char *Name) {
   int K, B; Symbol P, Q;
   if (!Active) return 0;
   for (K = Breadth, P = NIL; K >= 0; K--) {
      Q = P->Next[K];
      while (CompS(Q->Name, Name) < 0) P = Q, Q = Q->Next[K];
      Path[K] = P;
   }
   if (CompS(Q->Name, Name) == 0) return Q;
   K = Random(); if (K > Breadth) K = ++Breadth, Path[K] = NIL;
   Q = Form(K, Name);
   for (; K >= 0; K--) Q->Next[K] = Path[K]->Next[K], Path[K]->Next[K] = Q;
   return Q;
}

#define LINE_MAX 200
char Text[LINE_MAX]; static char *U;
 
#define INCLUDE_MAX 5
struct {
   word Path; long Loc; int Next, Line;
} IS[INCLUDE_MAX], *ISP;
static FILE *InF; FILE *OutF;
char **FileTab; long Files;
int StartF, CurF;
static int FileMax;
 
void FileInit(void) {
   ISP = IS - 1, FileTab = 0, Files = 0, FileMax = 0;
}
 
static byte ERRORS = 0;
char InSeg = 0;
void ERROR(const char *Format, ...) {
   va_list AP;
   if (InSeg) printf("%s: [%d] ", FileTab[StartF], StartLine);
   va_start(AP, Format);
   vprintf(Format, AP); putchar('\n');
   va_end(AP);
   if (++ERRORS >= 24) printf("Too many errors.  Aborting.\n"), exit(1);
} 
 
void FATAL(const char *Format, ...) {
   va_list AP;
   if (InSeg) printf("%s: [%d] ", FileTab[StartF], StartLine);
   va_start(AP, Format);
   vprintf(Format, AP); putchar('\n');
   va_end(AP);
   exit(1);
}
 
void CHECK(void) {
   if (ERRORS > 0) printf("Errors present.  Assembly stopped.\n"), exit(1);
}
 
byte GetB(FILE *FP) {
   int A;
   A = fgetc(FP); if (A == EOF) FATAL("Unexpected EOF.");
   return A&0xff;
}
 
word GetW(FILE *FP) {
   int A, B;
   A = fgetc(FP); if (A == EOF) FATAL("Unexpected EOF.");
   B = fgetc(FP); if (B == EOF) FATAL("Unexpected EOF.");
   return (A&0xff) << 8 | B&0xff;
}
 
unsigned long GetL(FILE *FP) {
   int A, B, C, D;
   A = fgetc(FP); if (A == EOF) FATAL("Unexpected EOF.");
   B = fgetc(FP); if (B == EOF) FATAL("Unexpected EOF.");
   C = fgetc(FP); if (C == EOF) FATAL("Unexpected EOF.");
   D = fgetc(FP); if (D == EOF) FATAL("Unexpected EOF.");
   return (A&0xff) << 24 | (B&0xff) << 16 | (C&0xff) << 8 | D&0xff;
}
 
void PutB(byte B, FILE *FP) { fputc(B, FP); }
 
void PutW(word W, FILE *FP) {
   char A = (W >> 8)&0xff, B = W&0xff;
   fputc(A, FP), fputc(B, FP);
}
 
void PutL(unsigned long L, FILE *FP) {
   char A = (L >> 24)&0xff, B = (L >> 16)&0xff, C = (L >> 8)&0xff, D = L&0xff;
   fputc(A, FP), fputc(B, FP), fputc(C, FP), fputc(D, FP);
}
 
void *Allocate(unsigned Size) {
   void *X;
   if (Size == 0) return 0;
   X = malloc(Size); if (X == 0) FATAL("Out of memory.");
   return X;
}
 
void OpenF(char *Name) {
   if (!Active) return;
   if (ISP >= IS + INCLUDE_MAX - 1) FATAL("Too many nested include files.");
   if (ISP >= IS) {
      ISP->Loc = ftell(InF);
      if (ISP->Loc == -1L)
         FATAL("Could not save %s's position.", FileTab[CurF]);
      fclose(InF);
      ISP->Next = Next, ISP->Line = Line, ISP->Path = CurF;
   }
   if (Files >= FileMax) {
      FileMax += 4;
      FileTab = (char **)realloc(FileTab, FileMax * sizeof *FileTab);
      if (FileTab == 0) FATAL("Out of memory.");
   }
   ISP++, CurF = Files, FileTab[Files++] = CopyS(Name);
   StartLine = Line = 1;
   InF = fopen(Name, "r");
   if (InF == 0) FATAL("Cannot open %s", Name);
   Next = fgetc(InF);
}
 
#define SEG_MAX 0x20
struct Segment SegTab[SEG_MAX], *SegP;
unsigned long LOC;
 
#define GAP_MAX 0x100
struct Gap GapTab[GAP_MAX], *GapP;
 
struct AddrCard AddrTab[TYPES] = {
   { 0, 0xffff, 1 },  /* CODE */
   { 0, 0xffff, 0 },  /* XDATA */
   { 0, 0xff, 0 },    /* DATA */
   { 0x80, 0xff, 0 }, /* SFR */
   { 0, 0xff, 0 }     /* BIT */
};
static struct AddrCard *AdP;
 
Symbol Sym;
 
static Symbol BTab[10], FTab[10];
static Symbol MakeLabel(void) {
   static unsigned long LAB = 0L; static char Buf[10];
   sprintf(Buf, "#%x", LAB++);
   return LookUp(Buf);
}
 
void SegInit(void) {
   int I, D;
   for (SegP = SegTab, AdP = AddrTab, I = 0; I < TYPES; AdP++, I++, SegP++) {
      if (SegP >= SegTab + SEG_MAX) FATAL("Too many segments.");
      SegP->Type = I, SegP->Rel;
      SegP->Base = 0, SegP->Size = 0, SegP->Loc = ftell(OutF);
   }
   AdP = AddrTab; InSeg = 1;
   SegP->Type = 0, SegP->Rel = 1,
   SegP->Line = StartLine, SegP->File = StartF,
   SegP->Base = 0, SegP->Size = 0, SegP->Loc = ftell(OutF);
   LOC = 0;
   for (D = 0; D < 10; D++) BTab[0] = FTab[0] = 0;
   GapP = GapTab;
}
 
void StartSeg(byte Type, byte Rel, word Base) {
   int D;
   if (!Active) return;
   if (SegP >= SegTab + SEG_MAX) FATAL("Too many segments.");
   if (Type > sizeof AddrTab/sizeof *AddrTab)
      FATAL("Undefined segment type.");
   AdP = AddrTab + Type;
   if (!Rel && (Base < AdP->Lo || Base > AdP->Hi))
      FATAL("Address %u out of range", Base);
   InSeg = 1;
   SegP->Type = Type, SegP->Rel = Rel,
   SegP->Line = StartLine, SegP->File = StartF,
   SegP->Base = Base, SegP->Size = 0, SegP->Loc = ftell(OutF);
   LOC = 0;
   for (D = 0; D < 10; D++) BTab[0] = FTab[0] = 0;
}
 
void EndSeg(void) {
   int D;
   if (!Active) return;
   for (D = 0; D < 10; D++)
      if (FTab[D] != 0) ERROR("Undefined label %d", D);
   SegP->Size = (word)LOC;
   if (SegP->Size > 0) SegP++;
   InSeg = 0;
}
 
void Space(word Rel) {
   unsigned long Addr;
   if (!Active) return;
   if (AdP->ReadOnly) {
      if (GapP >= GapTab + GAP_MAX) FATAL("Too many gaps.");
      GapP->Seg = SegP, GapP->Offset = LOC, GapP->Size = Rel;
      GapP++;
   }
   LOC += Rel;
   Addr = SegP->Base + LOC;
   if (!SegP->Rel && Addr > AdP->Hi + 1) FATAL("Address %u out of range", Addr);
}
 
void PByte(byte B) {
   unsigned long Addr;
   if (!Active) return;
   if (!AdP->ReadOnly) FATAL("Attempting to write to a non-code segment.");
   LOC++; Addr = SegP->Base + LOC;
   if (!SegP->Rel && Addr > AdP->Hi + 1) FATAL("Address %u out of range", Addr);
   fputc(B, OutF);
}
 
void PString(char *S) {
   if (!Active) return;
   for (; *S != '\0'; S++) PByte(*S);
}
 
static void Get(void) {
   int Ch;
   if (Next == EOF) {
      while (Next == EOF && ISP > IS) {
         fclose(InF); ISP--;
         Next = ISP->Next, Line = ISP->Line, CurF = ISP->Path;
         InF = fopen(FileTab[CurF], "r");
         if (InF == 0) FATAL("Cannot reopen %s", FileTab[CurF]);
         if (fseek(InF, ISP->Loc, SEEK_SET) != 0) {
            fclose(InF);
            FATAL("Could not restore %s's position.", FileTab[CurF]);
         }
      }
      if (Next == EOF) fclose(InF), Next = 0;
      *U++ = ' ';
   } else {
      if (Next == '\n') Line++;
      *U++ = Next; Next = fgetc(InF);
   }
}
 
typedef struct {
   char *Name; Lexical Tag; int Value;
} Card;
static Card Reserved[] = {
   { "ds", RB, 0 }, { "rb", RB, 0 }, { "rw", RW, 0 },
   { "byte", DB, 0 }, { "word", DW, 0 }, { "db", DB, 0 }, { "dw", DW, 0 },
   { "seg", SEG, 0 }, { "end", END, 0 },
   { "org", ORG, 0 }, { "at", ORG, 0 }, { "equ", EQU, 0 }, { "set", SET, 0 },
   { "include", INCLUDE, 0 }, { "global", GLOBAL, 0 }, { "public", GLOBAL, 0 },
   { "extern", EXTERN, 0 }, { "if", IF, 0 }, { "else", ELSE, 0 },
   { "high", HIGH, 0 }, { "low", LOW, 0 }, { "by", BY, 0 },
   { "code", TYPE, CODE }, { "xdata", TYPE, XDATA },
   { "bit", TYPE, BIT }, { "sfr", TYPE, SFR }, { "data", TYPE, DATA },
   { "a", REGISTER, ACC }, { "ab", REGISTER, AB }, { "c", REGISTER, CY },
   { "dptr", REGISTER, DPTR }, { "pc", REGISTER, PC },
   { "r0", REGISTER, R0 }, { "r1", REGISTER, R1 },
   { "r2", REGISTER, R2 }, { "r3", REGISTER, R3 },
   { "r4", REGISTER, R4 }, { "r5", REGISTER, R5 },
   { "r6", REGISTER, R6 }, { "r7", REGISTER, R7 }
};
 
#define ELEMENTS(Arr) (sizeof(Arr)/sizeof(Arr[0]))
 
static Lexical FindKey(char *Name) {
   Code *CP; Card *C;
   static Card *EndR = Reserved + ELEMENTS(Reserved);
   for (CP = CodeTab; CP->Name != 0; CP++)
      if (CompS(Name, CP->Name) == 0) {
         Value = CP - CodeTab; return MNEMONIC;
      }
   for (C = Reserved; C < EndR; C++)
      if (CompS(Name, C->Name) == 0) {
         Value = C->Value; return C->Tag;
      }
   Sym = LookUp(Name);
   return SYMBOL;
}

typedef struct {
   char *Name; byte Type; word Val;
} ValCard;
static ValCard ValTab[] = {
/* Special function registers */
   { "P0", SFR, 0x80 }, { "P1", SFR, 0x90 },
   { "P2", SFR, 0xa0 }, { "P3", SFR, 0xb0 },
   { "PCON", SFR, 0x87 }, { "TCON", SFR, 0x88 }, { "TMOD", SFR, 0x89 },
   { "SCON", SFR, 0x98 }, { "SBUF", SFR, 0x99 },
   { "IE",   SFR, 0xa8 }, { "IP",   SFR, 0xb8 },
   { "TL0", SFR, 0x8a }, { "TL1", SFR, 0x8b },
   { "TH0", SFR, 0x8c }, { "TH1", SFR, 0x8d },
   { "SP",   SFR, 0x81 }, { "DPL",  SFR, 0x82 }, { "DPH",  SFR, 0x83 },
   { "PSW",  SFR, 0xd0 }, { "ACC",  SFR, 0xe0 }, { "B",    SFR, 0xf0 },
/* Special funcgtion register bits */
   { "RI",  BIT, 0x98 }, { "TI",  BIT, 0x99 },    /* SCON */
   { "RB8", BIT, 0x9a }, { "TB8", BIT, 0x9b },
   { "REN", BIT, 0x9c }, { "SM2", BIT, 0x9d },
   { "SM1", BIT, 0x9e }, { "SM0", BIT, 0x9f },
   { "RXD",  BIT, 0xb0 }, { "TXD",  BIT, 0xb1 },  /* Port 3 */
   { "INT0", BIT, 0xb2 }, { "INT1", BIT, 0xb3 },
   { "T0",   BIT, 0xb4 }, { "T1",   BIT, 0xb5 },
   { "WR",   BIT, 0xb6 }, { "RD",   BIT, 0xb7 },
   { "P",   BIT, 0xd0 }, { "OV",  BIT, 0xd2 },    /* PSW */
   { "RS0", BIT, 0xd3 }, { "RS1", BIT, 0xd4 },
   { "F0",  BIT, 0xd5 }, { "AC",  BIT, 0xd6 }, { "CY",  BIT, 0xd7 },
   { "IT0", BIT, 0x88 }, { "IE0", BIT, 0x89 },    /* TCON */
   { "IT1", BIT, 0x8a }, { "IE1", BIT, 0x8b },
   { "TR0", BIT, 0x8c }, { "TF0", BIT, 0x8d },
   { "TR1", BIT, 0x8e }, { "TF1", BIT, 0x8f },
   { "EX0", BIT, 0xa8 }, { "ET0", BIT, 0xa9 },    /* IE */
   { "EX1", BIT, 0xaa }, { "ET1", BIT, 0xab },
   { "ES",  BIT, 0xac }, { "EA",  BIT, 0xaf },
   { "PX0", BIT, 0xb8 }, { "PT0", BIT, 0xb9 },    /* IP */
   { "PX1", BIT, 0xba }, { "PT1", BIT, 0xbb }, { "PS",  BIT, 0xbc }
};

void RegInit(void) {
   ValCard *V; Symbol Sym;
   static ValCard *EndV = ValTab + ELEMENTS(ValTab);
   for (V = ValTab; V < EndV; V++) {
      Sym = LookUp(V->Name);
      Sym->Defined = Sym->Address = 1;
      Sym->Seg = &SegTab[V->Type], Sym->Offset = V->Val;
   }
}
 
#define isoctal(Ch) (isdigit(Ch) && Ch < '8')
#define isx(Ch) (tolower(Ch) == 'x' || tolower(Ch) == 'h')
#define isq(Ch) (tolower(Ch) == 'q' || tolower(Ch) == 'o')
 
char InExp = 0, InSemi = 0;
Lexical OldL;
#define Ret(Token) return (*U = '\0', OldL = (Token))
 
Lexical Scan(void) {
   char *S; int Bad;
Start:
   U = Text;
   if (isalpha(Next) || Next == '_') {
      while (isalnum(Next) || Next == '_') Get();
      Ret(FindKey(Text));
   } else if (isdigit(Next)) {
      char Ch = Next; Get();
      if (Ch == '0' && isx(Next)) {
         do Get(); while (isxdigit(Next));
         goto RetHex;
      }
      if (Next == ':' && !InExp) {
         char D = *Text - '0';
         if (Active) {
            Sym = BTab[D] = (FTab[D] == 0)? MakeLabel(): FTab[D];
            FTab[D] = 0;
            Sym->Global = 0, Sym->Defined = Sym->Address = Sym->Variable = 1,
            Sym->Seg = SegP, Sym->Offset = (word)LOC;
         } else Sym = 0;
         Ret(SYMBOL);
      }
      while (isxdigit(Next)) Get();
      if (isx(Next)) { Get(); goto RetHex; }
      if (isq(Next)) { Get(); goto RetOct; }
      if (U - Text == 2) {
         if (tolower(Text[1]) == 'b') {
            char D = *Text - '0';
            if (Active) {
               Sym = BTab[D];
               if (Sym == 0) {
                  BTab[D] = Sym = MakeLabel();
                  ERROR("Undefined local symbol %cb", D);
                  Sym->Global = Sym->Defined = 0,
                  Sym->Address = Sym->Variable = 1,
                  Sym->Seg = SegP, Sym->Offset = 0;
               }
            } else Sym = 0;
            Ret(SYMBOL);
         } else if (tolower(Text[1]) == 'f') {
            char D = *Text - '0';
            if (Active) {
               if (FTab[D] == 0) FTab[D] = MakeLabel();
               Sym = FTab[D];
            } else Sym = 0;
            Ret(SYMBOL);
         }
      }
      if (tolower(U[-1]) == 'b') goto RetBin;
      if (tolower(U[-1]) == 'd' || *Text != '0') goto RetDec;
      if (U == Text) { Value = 0; Ret(NUMBER); }
      if (isx(Text[1])) goto RetHex;
      if (tolower(Text[1]) == 'b') goto RetBin;
      goto RetDec;
RetBin:
      S = Text;
      if (tolower(U[-1]) == 'b') U--;
      else if (U >= S + 2 && S[0] == '0' && tolower(S[1]) == 'b') S += 2;
      for (Bad = 0, Value = 0; S < U; S++) {
         int D = *S - '0';
         if (!isdigit(*S) || D >= 2) Bad++, D = 0;
         Value = (Value << 1) | D;
      }
      if (Bad) ERROR("Binary number has non-binary digits.");
      Ret(NUMBER);
RetOct:
      if (isq(U[-1])) U--;
      for (Bad = 0, Value = 0, S = Text; S < U; S++) {
         int D = *S - '0';
         if (!isdigit(*S) || D >= 010) Bad++, D = 0;
         Value = (Value << 3) | D;
      }
      if (Bad) ERROR("Octal number has non-octal digits.");
      Ret(NUMBER);
RetDec:
      if (tolower(U[-1]) == 'd') U--;
      for (Bad = 0, Value = 0, S = Text; S < U; S++) {
         int D = *S - '0';
         if (!isdigit(*S)) Bad++, D = 0;
         Value = 10*Value + D;
      }
      if (Bad) ERROR("Decimal number has non-decimal digits.");
      Ret(NUMBER);
RetHex:
      S = Text;
      if (isx(U[-1])) U--;
      else if (U >= S + 2 && S[0] == '0' && isx(S[1])) S += 2;
      for (Value = 0; S < U; S++) {
         Value <<= 4, Value += isdigit(*S)? *S - '0': tolower(*S) - 'a' + 10;
      }
      Ret(NUMBER);
   }
   switch (Next) {
      case ';': switch (Get(), Next) {
         case ';':
            while (Next != '\n') {
               if (Next == EOF) FATAL("Unexpected EOF inside ;; comment.");
               Next = fgetc(InF);
            }
            Get();
            if (!InSemi) goto Start;
         default: Ret(SEMI);
      }
      break;
      case '/': switch (Get(), Next) {
         case '/':
            while (Next != '\n') {
               if (Next == EOF) FATAL("Unexpected EOF inside // comment.");
               Next = fgetc(InF);
            }
            Get();
            if (!InSemi) goto Start;
         Ret(SEMI);
         case '*': {
            int Line0 = Line;
            Next = fgetc(InF);
            while (Next != EOF) {
               if (Next == '*') {
                  while (Next == '*') Next = fgetc(InF);
                  if (Next == '/') {
                     Next = fgetc(InF);
                     if (Line <= Line0 || !InSemi) goto Start;
                     Ret(SEMI);
                  }
               } else {
                  if (Next == '\n') Line++; Next = fgetc(InF);
               }
            }
            FATAL("Unexpected EOF inside comment.");
         }
         default: Ret(DIV);
      }
      case '\'':
	 Get();
         if (Next == '\'') {
            ERROR("Empty character constant."); Get(); Value = 0; Ret(NUMBER);
         }
	 if (Next == '\\') {
            int I;
	    Get();
	    if (isoctal(Next)) {
	       for (Value = 0; isoctal(Next); Get())
                  Value <<= 3, Value += Next - '0';
	    } else if (tolower(Next) == 'x') {
	       Get();
               if (!isxdigit(Next)) {
                  ERROR("Bad hexadecimal character.");
                  Value = 'x';
               } else for (I = 0; I < 2 && isxdigit(Next); I++, Get()) {
                  Value <<= 4;
                  Value += isdigit(Next)? Next - '0': tolower(Next) - 'a' + 10;
               }
	    } else {
               switch (Next) {
                  case 'a': Value = 0x07; break;
                  case 'b': Value = 0x08; break;
                  case 't': Value = 0x09; break;
                  case 'n': Value = 0x0a; break;
                  case 'v': Value = 0x0b; break;
                  case 'f': Value = 0x0c; break;
                  case 'r': Value = 0x0d; break;
                  default: Value = Next; break;
               }
               Get();
            }
	 } else Value = Next, Get();
	 if (Next != '\'') ERROR("Missing a ' in character constant.");
         else Get();
      Ret(NUMBER);
      case '"':
         Next = fgetc(InF);
         while (Next != '\"') {
            if (Next == EOF) FATAL("Unexpected EOF inside string.");
            else if (Next == '\\') {
               Next = fgetc(InF);
               if (Next == EOF) FATAL("Unexpected EOF inside string.");
	       if (isoctal(Next)) {
                  char Value = 0;
                  while (isoctal(Next)) {
                     Value = (Value << 3) + (Next - '0');
                     Next = fgetc(InF);
                  }
                  *U++ = Value;
	       } else if (tolower(Next) == 'x') {
                  char Value;
                  Next = fgetc(InF);
                  if (!isxdigit(Next)) Value = 'x';
		  else while (isxdigit(Next)) {
                     Value <<= 4;
                     Value += isdigit(Next)? Next - '0': tolower(Next) - 'a' + 10;
                     Next = fgetc(InF);
                  }
                  *U++ = Value;
	       } else {
                  if (Next == '\n') Line++;
                  switch (Next) {
                     case 'a': *U++ = '\a'; break;
                     case 'b': *U++ = '\b'; break;
                     case 't': *U++ = '\t'; break;
                     case 'n': *U++ = '\n'; break;
                     case 'v': *U++ = '\v'; break;
                     case 'f': *U++ = '\f'; break;
                     case 'r': *U++ = '\r'; break;
                     default: *U++ = Next; break;
                  }
                  Next = fgetc(InF);
               }
	    } else {
               if (Next == '\n') Line++;
               *U++ = Next; Next = fgetc(InF);
            }
         }
         Next = fgetc(InF);
      Ret(STRING);
      case '<': switch (Get(), Next) {
	 case '=': Get(); Ret(LE);
	 case '<': Get(); Ret(SHL);
         default: Ret(LT);
      }
      case '>': switch (Get(), Next) {
	 case '=': Get(); Ret(GE);
	 case '>': Get(); Ret(SHR);
	 default: Ret(GT);
      }
      case '&': switch (Get(), Next) {
	 case '&': Get(); Ret(AND_AND);
	 default: Ret(AND);
      }
      case '|': switch (Get(), Next) {
	 case '|': Get(); Ret(OR_OR);
	 default: Ret(OR);
      }
      case '=': switch (Get(), Next) {
	 case '=': Get(); Ret(EQ);
	 default: Ret(SET);
      }
      case '!': switch (Get(), Next) {
	 case '=': Get(); Ret(NE);
	 default: Ret(NOT_NOT);
      }
      case '\n': Get(); if (!InSemi) goto Start; Ret(SEMI);
      case '^': Get(); Ret(XOR);
      case '+': Get(); Ret(PLUS);
      case '-': Get(); Ret(MINUS);
      case '*': Get(); Ret(MULT);
      case '%': Get(); Ret(MOD);
      case '@': Get(); Ret(AT);
      case '#': Get(); Ret(POUND);
      case '$': Get(); Ret(DOLLAR);
      case '.': Get(); Ret(DOT);
      case '~': Get(); Ret(NOT);
      case ',': Get(); Ret(COMMA);
      case ':': Get(); Ret(COLON);
      case '?': Get(); Ret(QUEST);
      case '{': Get(); Ret(LCURL);
      case '}': Get(); Ret(RCURL);
      case '(': Get(); Ret(LPAR);
      case ')': Get(); Ret(RPAR);
      case 0: Ret(0);
      default: Get(); goto Start;
   }
}
 
