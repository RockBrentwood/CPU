#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "io.h"
#include "op.h"

extern bool Active;

// Skip lists are used to implement the symbol table.

static int CompS(char *A, char *B) {
   if (A == NULL) return B == NULL? 0: +1;
   else if (B == NULL) return -1;
   for (; *A != '\0'; A++, B++) {
      int Diff = tolower(*A) - tolower(*B);
      if (Diff < 0) return -1;
      else if (Diff > 0) return +1;
   }
   return *B == '\0'? 0: -1;
}

static int Random(void) {
   static byte B = 0, X = 0;
   int L = 0;
   while (1) {
      if (X == 0) X = 4, B = rand()&0xff;
      int D = B&3;
      B >>= 2, X--;
      if (D) return L;
      L++;
   }
}

#define MAX_BREADTH 16
Symbol NIL;
static Symbol Path[MAX_BREADTH];
static int Breadth;

static Symbol Form(int B, char *Name) {
   Symbol N = malloc(sizeof *N + B*sizeof(Symbol));
   if (N == NULL) fprintf(stderr, "Out of memory.\n"), exit(EXIT_FAILURE);
   N->Name = Name == NULL? NULL: strdup(Name), N->Defined = N->Global = N->Variable = N->Address = N->Map = false;
   return N;
}

#include <time.h>

void SymInit(void) {
   srand((unsigned)time(NULL));
   NIL = Form(MAX_BREADTH - 1, NULL);
   for (int K = 0; K < MAX_BREADTH; K++) NIL->Next[K] = NIL;
   Breadth = 0;
}

static int Next;
int Value;
int Line, StartLine;

Symbol LookUp(char *Name) {
   if (!Active) return NULL;
   Symbol P = NIL;
   for (int K = Breadth; K >= 0; K--) {
      Symbol Q = P->Next[K];
      while (CompS(Q->Name, Name) < 0) P = Q, Q = Q->Next[K];
      Path[K] = P;
      if (CompS(Q->Name, Name) == 0) return Q;
   }
   int K = Random(); if (K > Breadth) K = ++Breadth, Path[K] = NIL;
   Symbol Q = Form(K, Name);
   for (; K >= 0; K--) Q->Next[K] = Path[K]->Next[K], Path[K]->Next[K] = Q;
   return Q;
}

#define LINE_MAX 0x100
char Text[LINE_MAX]; static char *TextP;

#define INCLUDE_MAX 5
struct {
   word Path; long Loc; int Next, Line;
} IS[INCLUDE_MAX], *ISP;
static FILE *InF;
FILE *ExF;
char **FileTab; long Files;
int StartF, CurF;
static int FileMax;

void FileInit(void) {
   ISP = IS - 1, FileTab = NULL, Files = 0, FileMax = 0;
}

static byte Errors = 0;
bool InSeg = false;

void Error(const char *Format, ...) {
   if (InSeg) printf("%s: [%d] ", FileTab[StartF], StartLine);
   va_list AP; va_start(AP, Format);
   vprintf(Format, AP); putchar('\n');
   va_end(AP);
   if (++Errors >= 24) printf("Too many errors. Aborting.\n"), exit(1);
}

void Fatal(const char *Format, ...) {
   if (InSeg) printf("%s: [%d] ", FileTab[StartF], StartLine);
   va_list AP; va_start(AP, Format);
   vprintf(Format, AP); putchar('\n');
   va_end(AP);
   exit(1);
}

void Check(void) {
   if (Errors > 0) printf("Errors present. Assembly stopped.\n"), exit(1);
}

byte GetB(FILE *InF) {
   int A = fgetc(InF); if (A == EOF) Fatal("Unexpected EOF.");
   return A&0xff;
}

word GetW(FILE *InF) {
   int A = fgetc(InF); if (A == EOF) Fatal("Unexpected EOF.");
   int B = fgetc(InF); if (B == EOF) Fatal("Unexpected EOF.");
   return (A&0xff) << 8 | B&0xff;
}

unsigned long GetL(FILE *InF) {
   int A = fgetc(InF); if (A == EOF) Fatal("Unexpected EOF.");
   int B = fgetc(InF); if (B == EOF) Fatal("Unexpected EOF.");
   int C = fgetc(InF); if (C == EOF) Fatal("Unexpected EOF.");
   int D = fgetc(InF); if (D == EOF) Fatal("Unexpected EOF.");
   return (A&0xff) << 24 | (B&0xff) << 16 | (C&0xff) << 8 | D&0xff;
}

void PutB(byte B, FILE *ExF) { fputc(B, ExF); }

void PutW(word W, FILE *ExF) {
   char A = (W >> 8)&0xff, B = W&0xff;
   fputc(A, ExF), fputc(B, ExF);
}

void PutL(unsigned long L, FILE *ExF) {
   char A = (L >> 24)&0xff, B = (L >> 16)&0xff, C = (L >> 8)&0xff, D = L&0xff;
   fputc(A, ExF), fputc(B, ExF), fputc(C, ExF), fputc(D, ExF);
}

void *Allocate(unsigned Size) {
   if (Size == 0) return NULL;
   void *X = malloc(Size); if (X == NULL) Fatal("Out of memory.");
   return X;
}

void OpenF(char *Name) {
   if (!Active) return;
   if (ISP >= IS + INCLUDE_MAX - 1) Fatal("Too many nested include files.");
   if (ISP >= IS) {
      ISP->Loc = ftell(InF);
      if (ISP->Loc == -1L) Fatal("Could not save %s's position.", FileTab[CurF]);
      fclose(InF);
      ISP->Next = Next, ISP->Line = Line, ISP->Path = CurF;
   }
   if (Files >= FileMax) {
      FileMax += 4, FileTab = realloc(FileTab, FileMax*sizeof *FileTab);
      if (FileTab == NULL) Fatal("Out of memory.");
   }
   ISP++, CurF = Files, FileTab[Files++] = strdup(Name);
   StartLine = Line = 1;
   InF = fopen(Name, "r");
   if (InF == NULL) Fatal("Cannot open %s", Name);
   Next = fgetc(InF);
}

#define SEG_MAX 0x20
struct Segment SegTab[SEG_MAX], *SegP;
unsigned long CurLoc;

#define GAP_MAX 0x100
struct Gap GapTab[GAP_MAX], *GapP;

struct AddrItem AddrTab[TYPES] = {
   { 0, 0xffff, true },		// code (read only)
   { 0, 0xffff, false },	// xdata
   { 0, 0xff, false },		// data
   { 0x80, 0xff, false },	// sfr
   { 0, 0xff, false }		// bit
};
static struct AddrItem *AdP;

Symbol Sym;

static Symbol BTab[10], FTab[10];
static Symbol MakeLabel(void) {
   static unsigned CurLabel = 0; static char Buf[10];
   sprintf(Buf, "#%x", CurLabel++);
   return LookUp(Buf);
}

void SegInit(void) {
   SegP = SegTab, AdP = AddrTab;
   for (int I = 0; I < TYPES; SegP++, AdP++, I++) {
      if (SegP >= SegTab + SEG_MAX) Fatal("Too many segments.");
      SegP->Type = I, SegP->Rel = false;
      SegP->Base = 0, SegP->Size = 0, SegP->Loc = ftell(ExF);
   }
   AdP = AddrTab; InSeg = true;
   SegP->Type = 0, SegP->Rel = true,
   SegP->Line = StartLine, SegP->File = StartF,
   SegP->Base = 0, SegP->Size = 0, SegP->Loc = ftell(ExF);
   CurLoc = 0;
   for (int D = 0; D < 10; D++) BTab[D] = FTab[D] = NULL;
   GapP = GapTab;
}

void StartSeg(byte Type, bool Rel, word Base) {
   if (!Active) return;
   if (SegP >= SegTab + SEG_MAX) Fatal("Too many segments.");
   if (Type > sizeof AddrTab/sizeof *AddrTab) Fatal("Undefined segment type.");
   AdP = AddrTab + Type;
   if (!Rel && (Base < AdP->Lo || Base > AdP->Hi)) Fatal("Address %u out of range", Base);
   InSeg = true;
   SegP->Type = Type, SegP->Rel = Rel,
   SegP->Line = StartLine, SegP->File = StartF,
   SegP->Base = Base, SegP->Size = 0, SegP->Loc = ftell(ExF);
   CurLoc = 0;
   for (int D = 0; D < 10; D++) BTab[D] = FTab[D] = NULL;
}

void EndSeg(void) {
   if (!Active) return;
   for (int D = 0; D < 10; D++)
      if (FTab[D] != NULL) Error("Undefined label %d", D);
   SegP->Size = (word)CurLoc;
   if (SegP->Size > 0) SegP++;
   InSeg = false;
}

void Space(word Size) {
   if (!Active) return;
   if (AdP->ReadOnly) {
      if (GapP >= GapTab + GAP_MAX) Fatal("Too many gaps.");
      GapP->Seg = SegP, GapP->Offset = CurLoc, GapP->Size = Size, GapP++;
   }
   CurLoc += Size;
   unsigned long Addr = SegP->Base + CurLoc;
   if (!SegP->Rel && Addr > AdP->Hi + 1) Fatal("Address %u out of range", Addr);
}

void PByte(byte B) {
   if (!Active) return;
   if (!AdP->ReadOnly) Fatal("Attempting to write to a non-code segment.");
   CurLoc++;
   unsigned long Addr = SegP->Base + CurLoc;
   if (!SegP->Rel && Addr > AdP->Hi + 1) Fatal("Address %u out of range", Addr);
   fputc(B, ExF);
}

void PString(char *S) {
   if (!Active) return;
   for (; *S != '\0'; S++) PByte(*S);
}

static void Get(void) {
   if (Next == EOF) {
      while (Next == EOF && ISP > IS) {
         fclose(InF); ISP--;
         Next = ISP->Next, Line = ISP->Line, CurF = ISP->Path;
         InF = fopen(FileTab[CurF], "r");
         if (InF == NULL) Fatal("Cannot reopen %s", FileTab[CurF]);
         if (fseek(InF, ISP->Loc, SEEK_SET) != 0) {
            fclose(InF);
            Fatal("Could not restore %s's position.", FileTab[CurF]);
         }
      }
      if (Next == EOF) fclose(InF), Next = '\0';
      *TextP++ = ' ';
   } else {
      if (Next == '\n') Line++;
      *TextP++ = Next; Next = fgetc(InF);
   }
}

typedef struct DirItem {
   char *Name; Lexical Tag; int Value;
} *DirItem;
static struct DirItem Reserved[] = {
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
   static DirItem EndR = Reserved + ELEMENTS(Reserved);
   for (Code *CP = CodeTab; CP->Name != NULL; CP++)
      if (CompS(Name, CP->Name) == 0) {
         Value = CP - CodeTab; return MNEMONIC;
      }
   for (DirItem D = Reserved; D < EndR; D++)
      if (CompS(Name, D->Name) == 0) {
         Value = D->Value; return D->Tag;
      }
   Sym = LookUp(Name);
   return SYMBOL;
}

typedef struct ValItem {
   char *Name; byte Type; word Val;
} *ValItem;
static struct ValItem ValTab[] = {
// Special function registers.
   { "P0", SFR, 0x80 }, { "P1", SFR, 0x90 }, { "P2", SFR, 0xa0 }, { "P3", SFR, 0xb0 },
   { "PCON", SFR, 0x87 }, { "TCON", SFR, 0x88 }, { "TMOD", SFR, 0x89 },
   { "SCON", SFR, 0x98 }, { "SBUF", SFR, 0x99 },
   { "IE", SFR, 0xa8 }, { "IP", SFR, 0xb8 },
   { "TL0", SFR, 0x8a }, { "TL1", SFR, 0x8b }, { "TH0", SFR, 0x8c }, { "TH1", SFR, 0x8d },
   { "SP", SFR, 0x81 }, { "DPL", SFR, 0x82 }, { "DPH", SFR, 0x83 },
   { "PSW", SFR, 0xd0 }, { "ACC", SFR, 0xe0 }, { "B", SFR, 0xf0 },
// Special funcgtion register bits.
// SCON:
   { "RI", BIT, 0x98 }, { "TI", BIT, 0x99 }, { "RB8", BIT, 0x9a }, { "TB8", BIT, 0x9b },
   { "REN", BIT, 0x9c }, { "SM2", BIT, 0x9d }, { "SM1", BIT, 0x9e }, { "SM0", BIT, 0x9f },
// Port 3:
   { "RXD", BIT, 0xb0 }, { "TXD", BIT, 0xb1 }, { "INT0", BIT, 0xb2 }, { "INT1", BIT, 0xb3 },
   { "T0", BIT, 0xb4 }, { "T1", BIT, 0xb5 }, { "WR", BIT, 0xb6 }, { "RD", BIT, 0xb7 },
// PSW:
   { "P", BIT, 0xd0 }, { "OV", BIT, 0xd2 }, { "RS0", BIT, 0xd3 }, { "RS1", BIT, 0xd4 },
   { "F0", BIT, 0xd5 }, { "AC", BIT, 0xd6 }, { "CY", BIT, 0xd7 },
// TCON:
   { "IT0", BIT, 0x88 }, { "IE0", BIT, 0x89 }, { "IT1", BIT, 0x8a }, { "IE1", BIT, 0x8b },
   { "TR0", BIT, 0x8c }, { "TF0", BIT, 0x8d }, { "TR1", BIT, 0x8e }, { "TF1", BIT, 0x8f },
// IE:
   { "EX0", BIT, 0xa8 }, { "ET0", BIT, 0xa9 }, { "EX1", BIT, 0xaa }, { "ET1", BIT, 0xab }, { "ES", BIT, 0xac },
   { "EA", BIT, 0xaf },
// IP:
   { "PX0", BIT, 0xb8 }, { "PT0", BIT, 0xb9 }, { "PX1", BIT, 0xba }, { "PT1", BIT, 0xbb }, { "PS", BIT, 0xbc }
};

void RegInit(void) {
   static ValItem EndV = ValTab + ELEMENTS(ValTab);
   for (ValItem V = ValTab; V < EndV; V++) {
      Symbol Sym = LookUp(V->Name);
      Sym->Defined = Sym->Address = true, Sym->Seg = &SegTab[V->Type], Sym->Offset = V->Val;
   }
}

#define isoctal(Ch) (isdigit(Ch) && Ch < '8')
#define isx(Ch) (tolower(Ch) == 'x' || tolower(Ch) == 'h')
#define isq(Ch) (tolower(Ch) == 'q' || tolower(Ch) == 'o')

bool InExp = false, InSemi = false;
Lexical OldL;
#define Ret(Token) return (*TextP = '\0', OldL = (Token))

Lexical Scan(void) {
   char *S; int Bad;
Start:
   TextP = Text;
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
         int D = *Text - '0';
         if (Active) {
            Sym = BTab[D] = FTab[D] == NULL? MakeLabel(): FTab[D];
            FTab[D] = NULL;
            Sym->Global = false, Sym->Defined = Sym->Address = Sym->Variable = true,
            Sym->Seg = SegP, Sym->Offset = (word)CurLoc;
         } else Sym = NULL;
         Ret(SYMBOL);
      }
      while (isxdigit(Next)) Get();
      if (isx(Next)) { Get(); goto RetHex; }
      if (isq(Next)) { Get(); goto RetOct; }
      if (TextP - Text == 2) {
         if (tolower(Text[1]) == 'b') {
            int D = *Text - '0';
            if (Active) {
               Sym = BTab[D];
               if (Sym == NULL) {
                  BTab[D] = Sym = MakeLabel();
                  Error("Undefined local symbol %db", D);
                  Sym->Global = Sym->Defined = false,
                  Sym->Address = Sym->Variable = true,
                  Sym->Seg = SegP, Sym->Offset = 0;
               }
            } else Sym = NULL;
            Ret(SYMBOL);
         } else if (tolower(Text[1]) == 'f') {
            int D = *Text - '0';
            if (Active) {
               if (FTab[D] == NULL) FTab[D] = MakeLabel();
               Sym = FTab[D];
            } else Sym = NULL;
            Ret(SYMBOL);
         }
      }
      if (tolower(TextP[-1]) == 'b') goto RetBin;
      if (tolower(TextP[-1]) == 'd' || *Text != '0') goto RetDec;
      if (TextP == Text) { Value = 0; Ret(NUMBER); }
      if (isx(Text[1])) goto RetHex;
      if (tolower(Text[1]) == 'b') goto RetBin;
      goto RetDec;
   RetBin:
      S = Text;
      if (tolower(TextP[-1]) == 'b') TextP--;
      else if (TextP >= S + 2 && S[0] == '0' && tolower(S[1]) == 'b') S += 2;
      for (Bad = 0, Value = 0; S < TextP; S++) {
         int D = *S - '0';
         if (!isdigit(*S) || D >= 2) Bad++, D = 0;
         Value = (Value << 1) | D;
      }
      if (Bad) Error("Binary number has non-binary digits.");
      Ret(NUMBER);
   RetOct:
      if (isq(TextP[-1])) TextP--;
      for (Bad = 0, Value = 0, S = Text; S < TextP; S++) {
         int D = *S - '0';
         if (!isdigit(*S) || D >= 010) Bad++, D = 0;
         Value = (Value << 3) | D;
      }
      if (Bad) Error("Octal number has non-octal digits.");
      Ret(NUMBER);
   RetDec:
      if (tolower(TextP[-1]) == 'd') TextP--;
      for (Bad = 0, Value = 0, S = Text; S < TextP; S++) {
         int D = *S - '0';
         if (!isdigit(*S)) Bad++, D = 0;
         Value = 10*Value + D;
      }
      if (Bad) Error("Decimal number has non-decimal digits.");
      Ret(NUMBER);
   RetHex:
      S = Text;
      if (isx(TextP[-1])) TextP--;
      else if (TextP >= S + 2 && S[0] == '0' && isx(S[1])) S += 2;
      for (Value = 0; S < TextP; S++)
         Value <<= 4, Value += isdigit(*S)? *S - '0': tolower(*S) - 'a' + 0xa;
      Ret(NUMBER);
   }
   switch (Next) {
      case ';': switch (Get(), Next) {
         case ';':
            while (Next != '\n') {
               if (Next == EOF) Fatal("Unexpected EOF inside ;; comment.");
               Next = fgetc(InF);
            }
            Get();
            if (!InSemi) goto Start;
         default: break;
      }
      Ret(SEMI);
      case '/': switch (Get(), Next) {
         case '/':
            while (Next != '\n') {
               if (Next == EOF) Fatal("Unexpected EOF inside // comment.");
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
            Fatal("Unexpected EOF inside comment.");
         }
         default: Ret(DIV);
      }
      case '\'':
         Get();
         if (Next == '\'') {
            Error("Empty character constant."); Get(); Value = 0; Ret(NUMBER);
         }
         if (Next == '\\') {
            Get();
            if (isoctal(Next))
               for (Value = 0; isoctal(Next); Get()) Value <<= 3, Value += Next - '0';
            else if (tolower(Next) == 'x') {
               Get();
               if (!isxdigit(Next))
                  Error("Bad hexadecimal character."), Value = 'x';
               else for (int I = 0; I < 2 && isxdigit(Next); I++, Get())
                  Value <<= 4, Value += isdigit(Next)? Next - '0': tolower(Next) - 'a' + 0xa;
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
         if (Next != '\'') Error("Missing a ' in character constant.");
         else Get();
      Ret(NUMBER);
      case '"':
         Next = fgetc(InF);
         while (Next != '\"') {
            if (Next == EOF) Fatal("Unexpected EOF inside string.");
            else if (Next == '\\') {
               Next = fgetc(InF);
               if (Next == EOF) Fatal("Unexpected EOF inside string.");
               if (isoctal(Next)) {
                  char Value = 0;
                  for (; isoctal(Next); Next = fgetc(InF))
                     Value = (Value << 3) + (Next - '0');
                  *TextP++ = Value;
               } else if (tolower(Next) == 'x') {
                  Next = fgetc(InF);
                  if (!isxdigit(Next)) *TextP++ = 'x';
                  else {
                     char Value = 0;
                     for (; isxdigit(Next); Next = fgetc(InF))
                        Value <<= 4, Value += isdigit(Next)? Next - '0': tolower(Next) - 'a' + 0xa;
                     *TextP++ = Value;
                  }
               } else {
                  if (Next == '\n') Line++;
                  switch (Next) {
                     case 'a': *TextP++ = '\a'; break;
                     case 'b': *TextP++ = '\b'; break;
                     case 't': *TextP++ = '\t'; break;
                     case 'n': *TextP++ = '\n'; break;
                     case 'v': *TextP++ = '\v'; break;
                     case 'f': *TextP++ = '\f'; break;
                     case 'r': *TextP++ = '\r'; break;
                     default: *TextP++ = Next; break;
                  }
                  Next = fgetc(InF);
               }
            } else {
               if (Next == '\n') Line++;
               *TextP++ = Next; Next = fgetc(InF);
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
