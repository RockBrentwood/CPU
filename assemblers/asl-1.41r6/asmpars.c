/* asmpars.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Symbolen und das ganze Drumherum...                        */
/*                                                                           */
/* Historie:  5. 5.1996 Grundsteinlegung                                     */
/*            4. 1.1997 Umstellung wg. case-sensitiv                         */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "endian.h"
#include "bpemu.h"
#include "nls.h"
#include "stringutil.h"
#include "asmfnums.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"

#include "asmpars.h"

LargeWord IntMasks[IntTypeCnt] = { 0x00000001l, 0x00000002l, 0x00000007l, 0x00000007l, 0x0000000fl, 0x0000000fl,
   0x0000000fl, 0x0000001fl, 0x0000001fl, 0x0000003fl, 0x0000007fl, 0x0000007fl, 0x000000ffl,
   0x000000ffl, 0x000001ffl, 0x000003ffl, 0x000003ffl, 0x000007ffl, 0x00000fffl, 0x00000fffl,
   0x00001fffl, 0x00007fffl, 0x0000ffffl, 0x0000ffffl, 0x0003ffffl, 0x0007ffffl,
   0x000fffffl, 0x000fffffl, 0x003fffffl, 0x007fffffl, 0x00ffffffl, 0x00ffffffl, 0xffffffffl, 0xffffffffl,
   0xffffffffl
#ifdef HAS64
      , 0xffffffffffffffffllu
#endif
};

LargeInt IntMins[IntTypeCnt] = { 0l, 0l, 0l, -8l, 0l, -8l,
   -16l, 0l, -16l, 0l, 0l, -128l, 0l,
   -128l, 0l, 0l, -512l, 0l, 0l, -2047l,
   0l, -32768l, 0l, -32768l, 0l, -524288l,
   0l, -524288l, 0l, -8388608l, 0l, -8388608l, -2147483647l, 0l,
   -2147483647l
#ifdef HAS64
      , -9223372036854775807ll
#endif
};

LargeInt IntMaxs[IntTypeCnt] = { 1l, 3l, 7l, 7l, 15l, 15l,
   31l, 31l, 31l, 63l, 127l, 127l, 255l,
   255l, 511l, 1023l, 1023l, 2047l, 4095l, 4095l,
   8191l, 32767l, 65535l, 65535l, 262143l, 524287l,
#ifdef __STDC__
   1048575l, 1048575l, 4194303l, 8388607l, 16777215l, 16777215l, 2147483647l, 4294967295ul,
   4294967295ul
#else
   1048575l, 1048575l, 4194303l, 8388607l, 16777215l, 16777215l, 2147483647l, 4294967295l,
   4294967295l
#endif
#ifdef HAS64
      , 9223372036854775807ll
#endif
};

bool FirstPassUnknown; /* Hinweisflag: evtl. im ersten Pass unbe-
                          kanntes Symbol, Ausdruck nicht ausgewertet */
bool SymbolQuestionable; /* Hinweisflag:  Dadurch, dass Phasenfehler
                            aufgetreten sind, ist dieser Symbolwert evtl.
                            nicht mehr aktuell */
bool UsesForwards; /* Hinweisflag: benutzt Vorwaertsdefinitionen */
LongInt MomLocHandle; /* Merker, den lokale Symbole erhalten */

LongInt LocHandleCnt; /* mom. verwendeter lokaler Handle */

bool BalanceTree; /* Symbolbaum ausbalancieren */

#define ERRMSG
#include "as.rsc"

static char *DigitVals = "0123456789ABCDEF";
static char BaseIds[3] = { '%', '@', '$' };
static char BaseLetters[3] = { 'B', 'O', 'H' };
static Byte BaseVals[3] = { 2, 8, 16 };

typedef struct _TCrossRef {
   struct _TCrossRef *Next;
   Byte FileNum;
   LongInt LineNum;
   Integer OccNum;
} TCrossRef, *PCrossRef;

typedef struct {
   TempType Typ;
   union {
      LargeInt IWert;
      Double FWert;
      char *SWert;
   } Contents;
} SymbolVal;

typedef struct _SymbolEntry {
   struct _SymbolEntry *Left, *Right;
   ShortInt Balance;
   LongInt Attribute;
   char *SymName;
   Byte SymType;
   ShortInt SymSize;
   bool Defined, Used, Changeable;
   SymbolVal SymWert;
   PCrossRef RefList;
   Byte FileNum;
   LongInt LineNum;
} SymbolEntry, *SymbolPtr;

typedef struct _TSymbolStackEntry {
   struct _TSymbolStackEntry *Next;
   SymbolVal Contents;
} TSymbolStackEntry, *PSymbolStackEntry;

typedef struct _TSymbolStack {
   struct _TSymbolStack *Next;
   char *Name;
   PSymbolStackEntry Contents;
} TSymbolStack, *PSymbolStack;

typedef struct _TDefSymbol {
   struct _TDefSymbol *Next;
   char *SymName;
   TempResult Wert;
} TDefSymbol, *PDefSymbol;

typedef struct _TCToken {
   struct _TCToken *Next;
   char *Name;
   LongInt Parent;
   ChunkList Usage;
} TCToken, *PCToken;

typedef struct Operator {
   char *Id;
   int IdLen;
   bool Dyadic;
   Byte Priority;
   bool MayInt;
   bool MayFloat;
   bool MayString;
   bool Present;
} Operator;

typedef struct _TLocHeap {
   struct _TLocHeap *Next;
   LongInt Cont;
} TLocHeap, *PLocHandle;

static SymbolPtr FirstSymbol, FirstLocSymbol;
static PDefSymbol FirstDefSymbol;
static PCToken FirstSection;
static bool DoRefs; /* Querverweise protokollieren */
static PLocHandle FirstLocHandle;
static PSymbolStack FirstStack;
static PCToken MomSection;

void AsmParsInit(void) {
   FirstSymbol = NULL;
   FirstLocSymbol = NULL;
   MomLocHandle = (-1);
   SetMomSection(-1);
   FirstSection = NULL;
   FirstLocHandle = NULL;
   DoRefs = true;
   FirstStack = NULL;
}

bool RangeCheck(LargeInt Wert, IntType Typ) {
#ifndef HAS64
   if (Typ >= SInt32) return true;
#else
   if (Typ >= Int64) return true;
#endif
   else return ((Wert >= IntMins[Typ]) && (Wert <= IntMaxs[Typ]));
}

bool FloatRangeCheck(Double Wert, FloatType Typ) {
   switch (Typ) {
      case Float32:
         return (fabs(Wert) <= 3.4e38);
      case Float64:
         return (fabs(Wert) <= 1.7e308);
/**     case FloatCo  : FloatRangeCheck:=Abs(Wert)<=9.22e18;
     case Float80  : FloatRangeCheck:=true;
     case FloatDec : FloatRangeCheck:=true;**/
      default:
         return false;
   }
/**   IF (Typ=FloatDec) && (Abs(Wert)>1e1000) THEN WrError(40);**/
}

static void ReplaceBkSlashes(char *s) {
   char *p, Save;
   Integer cnt;
   bool OK;
   char ErgChar;

   p = strchr(s, '\\');
   while (p != NULL) {
      cnt = 1;
      ErgChar = '\\';
      switch (toupper(*(p + 1))) {
         case '\'':
         case '\\':
         case '"':
            ErgChar = (*(p + 1));
            cnt = 2;
            break;
         case 'H':
            ErgChar = '\'';
            cnt = 2;
            break;
         case 'I':
            ErgChar = '"';
            cnt = 2;
            break;
         case 'B':
            ErgChar = Char_BS;
            cnt = 2;
            break;
         case 'A':
            ErgChar = Char_BEL;
            cnt = 2;
            break;
         case 'E':
            ErgChar = Char_ESC;
            cnt = 2;
            break;
         case 'T':
            ErgChar = Char_HT;
            cnt = 2;
            break;
         case 'N':
            ErgChar = Char_LF;
            cnt = 2;
            break;
         case 'R':
            ErgChar = Char_CR;
            cnt = 2;
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
            cnt = 2;
            while ((cnt < 4) && (*(p + cnt) != '\0') && (*(p + cnt) >= '0') && (*(p + cnt) <= '9')) cnt++;
            Save = (*(p + cnt));
            *(p + cnt) = '\0';
            ErgChar = ConstLongInt(p + 1, &OK);
            *(p + cnt) = Save;
            if (!OK) WrError(1320);
            break;
         default:
            WrError(1135);
      };
      *p = ErgChar;
      strcpy(p + 1, p + cnt);
      p = strchr(p + 1, '\\');
   }
}

bool ExpandSymbol(char *Name) {
   char *p1, *p2;
   String h;
   bool OK;

   do {
      if ((p1 = strchr(Name, '{')) == NULL) return true;
      strmaxcpy(h, p1 + 1, 255);
      if ((p2 = QuotPos(h, '}')) == NULL) {
         WrXError(1020, Name);
         return false;
      }
      strcpy(p1, p2 + 1);
      *p2 = '\0';
      FirstPassUnknown = false;
      EvalStringExpression(h, &OK, h);
      if (FirstPassUnknown) {
         WrError(1820);
         return false;
      }
      if (!CaseSensitive) UpString(h);
      strmaxins(Name, h, p1 - Name, 255);
   }
   while (p1 != NULL);
   return true;
}

bool IdentifySection(char *Name, LongInt * Erg) {
   PSaveSection SLauf;
   Integer Depth;

   if (!ExpandSymbol(Name)) return false;
   if (!CaseSensitive) NLS_UpString(Name);

   if (*Name == '\0') {
      *Erg = (-1);
      return true;
   } else if (((strlen(Name) == 6) || (strlen(Name) == 7))
      && (strncasecmp(Name, "PARENT", 6) == 0)
      && ((strlen(Name) == 6) || ((Name[6] >= '0') && (Name[6] <= '9')))) {
      if (strlen(Name) == 6) Depth = 1;
      else Depth = Name[6] - AscOfs;
      SLauf = SectionStack;
      *Erg = MomSectionHandle;
      while ((Depth > 0) && (*Erg != (-2))) {
         if (SLauf == NULL) *Erg = (-2);
         else {
            *Erg = SLauf->Handle;
            SLauf = SLauf->Next;
         }
         Depth--;
      }
      if (*Erg == (-2)) {
         WrError(1484);
         return false;
      } else return true;
   } else if (strcmp(Name, GetSectionName(MomSectionHandle)) == 0) {
      *Erg = MomSectionHandle;
      return true;
   } else {
      SLauf = SectionStack;
      while ((SLauf != NULL) && (strcmp(GetSectionName(SLauf->Handle), Name) != 0))
         SLauf = SLauf->Next;
      if (SLauf == NULL) {
         WrError(1484);
         return false;
      } else {
         *Erg = SLauf->Handle;
         return true;
      }
   }
}

static bool GetSymSection(char *Name, LongInt * Erg) {
   String Part;
   char *q;
   int l = strlen(Name);

   if (Name[l - 1] != ']') {
      *Erg = (-2);
      return true;
   }

   Name[l - 1] = '\0';
   q = RQuotPos(Name, '[');
   Name[l - 1] = ']';
   if (Name + strlen(Name) - q <= 2) {
      WrXError(1020, Name);
      return false;
   }

   Name[strlen(Name) - 1] = '\0';
   strmaxcpy(Part, q + 1, 255);
   *q = '\0';

   return IdentifySection(Part, Erg);
}

LargeInt ConstIntVal(char *Asc_O, IntType Typ, bool *Ok) {
   String Asc;
   Byte Search, Base, Digit;
   LargeInt Wert;
   bool NegFlag;
   TConstMode ActMode = ConstModeC;
   bool Found;
   char *z, *h;

   *Ok = false;
   Wert = 0;
   strmaxcpy(Asc, Asc_O, 255);
   if (Asc[0] == '\0') {
      *Ok = true;
      return 0;
   }

/* ASCII herausfiltern */

   else if (Asc[0] == '\'') {
      if (Asc[strlen(Asc) - 1] != '\'') return -1;
      strcpy(Asc, Asc + 1);
      Asc[strlen(Asc) - 1] = '\0';
      ReplaceBkSlashes(Asc);
      for (Search = 0; Search < strlen(Asc); Search++) {
         Digit = (unsigned int)Asc[Search];
         Wert = (Wert << 8) + CharTransTable[Digit];
      }
      NegFlag = false;
   }

/* Zahlenkonstante */

   else {
   /* Vorzeichen */

      if (*Asc == '+') strcpy(Asc, Asc + 1);
      NegFlag = (*Asc == '-');
      if (NegFlag) strcpy(Asc, Asc + 1);

      if (RelaxedMode) {
         Found = false;
         if ((strlen(Asc) >= 2) && (*Asc == '0') && (toupper(Asc[1]) == 'X')) {
            ActMode = ConstModeC;
            Found = true;
         }
         if ((!Found) && (strlen(Asc) >= 2)) {
            for (Search = 0; Search < 3; Search++)
               if (Asc[0] == BaseIds[Search]) {
                  ActMode = ConstModeMoto;
                  Found = true;
                  break;
               }
         }
         if ((!Found) && (strlen(Asc) >= 2) && (Asc[0] >= '0') && (Asc[0] <= '9')) {
            for (Search = 0; Search < 3; Search++)
               if (toupper(Asc[strlen(Asc) - 1]) == BaseLetters[Search]) {
                  ActMode = ConstModeIntel;
                  Found = true;
                  break;
               }
         }
         if (!Found) ActMode = ConstModeC;
      } else ActMode = ConstMode;

   /* Zahlensystem ermitteln/pruefen */

      Base = 10;
      switch (ActMode) {
         case ConstModeIntel:
            for (Search = 0; Search < 3; Search++)
               if (toupper(Asc[strlen(Asc) - 1]) == BaseLetters[Search]) {
                  Base = BaseVals[Search];
                  Asc[strlen(Asc) - 1] = '\0';
                  break;
               }
            break;
         case ConstModeMoto:
            for (Search = 0; Search < 3; Search++)
               if (Asc[0] == BaseIds[Search]) {
                  Base = BaseVals[Search];
                  strcpy(Asc, Asc + 1);
                  break;
               }
            break;
         case ConstModeC:
            if (strcmp(Asc, "0") == 0) {
               *Ok = true;
               return 0;
            } else if (Asc[0] != '0') Base = 10;
            else if (strlen(Asc) < 2) return -1;
            else {
               strcpy(Asc, Asc + 1);
               switch (toupper(Asc[0])) {
                  case 'X':
                     strcpy(Asc, Asc + 1);
                     Base = 16;
                     break;
                  case 'B':
                     strcpy(Asc, Asc + 1);
                     Base = 2;
                     break;
                  default:
                     Base = 8;
                     break;
               }
               if (Asc[0] == '\0') return -1;
            }
      }

      if (Asc[0] == '\0') return -1;

      if (ActMode == ConstModeIntel)
         if ((Asc[0] < '0') || (Asc[0] > '9')) return -1;

      for (z = Asc; *z != '\0'; z++) {
         if ((h = strchr(DigitVals, toupper(*z))) == NULL) return -1;
         else Search = h - DigitVals;
         if (Search >= Base) return -1;
         Wert = Wert * Base + Search;
      }
   }

   if (NegFlag) Wert = (-Wert);

   *Ok = RangeCheck(Wert, Typ);
   if (Ok) return Wert;
   else {
      WrError(1320);
      return -1;
   }
}

Double ConstFloatVal(char *Asc_O, FloatType Typ, bool *Ok) {
   Double Erg;
   char *end;

   if (Typ); /* satisfy some compilers */

   if (*Asc_O) {
      Erg = strtod(Asc_O, &end);
      *Ok = (*end == '\0');
   } else {
      Erg = 0.0;
      *Ok = true;
   }
   return Erg;
}

void ConstStringVal(char *Asc_O, char *Erg, bool *OK) {
   String Asc, tmp, Part;
   char *z, Save;
   Integer l;
   bool OK2;
   TempResult t;

   *OK = false;

   if ((strlen(Asc_O) < 2) || (*Asc_O != '"') || (Asc_O[strlen(Asc_O) - 1] != '"')) return;

   strmaxcpy(Asc, Asc_O + 1, 255);
   Asc[strlen(Asc) - 1] = '\0';
   *tmp = '\0';

   while (*Asc != '\0') {
      z = strchr(Asc, '\\');
      if (z == NULL) z = Asc + strlen(Asc);
      Save = (*z);
      *z = '\0';
      if (strchr(Asc, '"') != NULL) return;
      strmaxcat(tmp, Asc, 255);
      *z = Save;
      strcpy(Asc, z);
      if (*Asc == '\\') {
         if (strlen(Asc) < 2) return;
         l = strlen(tmp);
         switch (toupper(Asc[1])) {
            case '\'':
            case '\\':
            case '"':
               tmp[l++] = Asc[1];
               tmp[l++] = '\0';
               z = Asc + 2;
               break;
            case 'H':
               tmp[l++] = '\'';
               tmp[l++] = '\0';
               z = Asc + 2;
               break;
            case 'I':
               tmp[l++] = '"';
               tmp[l++] = '\0';
               z = Asc + 2;
               break;
            case 'B':
               tmp[l++] = Char_BS;
               tmp[l++] = '\0';
               z = Asc + 2;
               break;
            case 'A':
               tmp[l++] = Char_BEL;
               tmp[l++] = '\0';
               z = Asc + 2;
               break;
            case 'E':
               tmp[l++] = Char_ESC;
               tmp[l++] = '\0';
               z = Asc + 2;
               break;
            case 'T':
               tmp[l++] = Char_HT;
               tmp[l++] = '\0';
               z = Asc + 2;
               break;
            case 'N':
               tmp[l++] = Char_LF;
               tmp[l++] = '\0';
               z = Asc + 2;
               break;
            case 'R':
               tmp[l++] = Char_CR;
               tmp[l++] = '\0';
               z = Asc + 2;
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
               z = Asc + 1;
               while ((*z != '\0') && (z - Asc < 4) && (*z >= '0') && (*z <= '9')) z++;
               Save = (*z);
               *z = '\0';
               tmp[l++] = ConstLongInt(Asc + 1, &OK2) & 0xff;
               *z = Save;
               if (tmp[l - 1] == '\0') WrError(240);
               if (!OK2) return;
               tmp[l++] = '\0';
               break;
            case '{':
               z = QuotPos(Asc, '}');
               if (z == NULL) return;
               FirstPassUnknown = false;
               *(z++) = '\0';
               strmaxcpy(Part, Asc + 2, 255);
               KillBlanks(Part);
               EvalExpression(Part, &t);
               if (FirstPassUnknown) {
                  WrXError(1820, Part);
                  return;
               } else switch (t.Typ) {
                     case TempInt:
                        strmaxcat(tmp, HexString(t.Contents.Int, 0), 255);
                        break;
                     case TempFloat:
                        strmaxcat(tmp, FloatString(t.Contents.Float), 255);
                        break;
                     case TempString:
                        strmaxcat(tmp, t.Contents.Ascii, 255);
                        break;
                     default:
                        return;
               }
               break;
            default:
               WrError(1135);
               z = Asc + 2;
               break;
         }
         strcpy(Asc, z);
      }
   }

   *OK = true;
   strmaxcpy(Erg, tmp, 255);
}

static SymbolPtr FindLocNode(char *Name, TempType SearchType);

static SymbolPtr FindNode(char *Name, TempType SearchType);

static void EvalExpression_ChgFloat(TempResult * T) {
   if (T->Typ != TempInt) return;
   T->Typ = TempFloat;
   T->Contents.Float = T->Contents.Int;
}

void EvalExpression(char *Asc_O, TempResult * Erg) {
#define OpCnt 23
   static Operator Operators[OpCnt + 1] = {
   /* Dummynulloperator */
      { " ", 1, false, 0, false, false, false, false },
   /* Einerkomplement */
      { "~", 1, false, 1, true, false, false, false },
   /* Linksschieben */
      { "<<", 2, true, 3, true, false, false, false },
   /* Rechtsschieben */
      { ">>", 2, true, 3, true, false, false, false },
   /* Bitspiegelung */
      { "><", 2, true, 4, true, false, false, false },
   /* binaeres AND */
      { "&", 1, true, 5, true, false, false, false },
   /* binaeres OR */
      { "|", 1, true, 6, true, false, false, false },
   /* binaeres EXOR */
      { "!", 1, true, 7, true, false, false, false },
   /* allg. Potenz */
      { "^", 1, true, 8, true, true, false, false },
   /* Produkt */
      { "*", 1, true, 11, true, true, false, false },
   /* Quotient */
      { "/", 1, true, 11, true, true, false, false },
   /* Modulodivision */
      { "#", 1, true, 11, true, false, false, false },
   /* Summe */
      { "+", 1, true, 13, true, true, true, false },
   /* Differenz */
      { "-", 1, true, 13, true, true, false, false },
   /* logisches ! */
      { "~~", 2, false, 2, true, false, false, false },
   /* logisches AND */
      { "&&", 2, true, 15, true, false, false, false },
   /* logisches OR */
      { "||", 2, true, 16, true, false, false, false },
   /* logisches EXOR */
      { "!!", 2, true, 17, true, false, false, false },
   /* Gleichheit */
      { "=", 1, true, 23, true, true, true, false },
   /* Groesser als */
      { ">", 1, true, 23, true, true, true, false },
   /* Kleiner als */
      { "<", 1, true, 23, true, true, true, false },
   /* Kleiner oder gleich */
      { "<=", 2, true, 23, true, true, true, false },
   /* Groesser oder gleich */
      { ">=", 2, true, 23, true, true, true, false },
   /* Ungleichheit */
      { "<>", 2, true, 23, true, true, true, false }
   };
   static Operator *OpEnd = Operators + OpCnt;
   Operator *FOps[OpCnt + 1];
   LongInt FOpCnt = 0;

   bool OK, FFound;
   TempResult LVal, RVal;
   Integer z1;
   Operator *Op;
   char Save = '\0';
   Integer LKlamm, RKlamm, WKlamm, zop;
   Integer OpMax, LocOpMax, OpPos = (-1), OpLen;
   bool OpFnd, InHyp, InQuot;
   LargeInt HVal;
   Double FVal;
   SymbolPtr Ptr;
   PFunction ValFunc;
   String Asc, stemp, ftemp;
   char *KlPos, *zp;

   memset(&LVal, 0, sizeof(LVal));
   memset(&RVal, 0, sizeof(RVal));

   ChkStack();

   strmaxcpy(Asc, Asc_O, 255);
   strmaxcpy(stemp, Asc, 255);
   KillBlanks(Asc);
   if (MakeDebug) fprintf(Debug, "Parse %s", Asc);

/* Annahme Fehler */

   Erg->Typ = TempNone;

/* Programmzaehler ? */

   if (strcasecmp(Asc, PCSymbol) == 0) {
      Erg->Typ = TempInt;
      Erg->Contents.Int = EProgCounter();
      return;
   }

/* Konstanten ? */

   Erg->Contents.Int = ConstIntVal(Asc, IntTypeCnt - 1, &OK);
   if (OK) {
      Erg->Typ = TempInt;
      return;
   }

   Erg->Contents.Float = ConstFloatVal(Asc, Float80, &OK);
   if (OK) {
      Erg->Typ = TempFloat;
      return;
   }

   ConstStringVal(Asc, Erg->Contents.Ascii, &OK);
   if (OK) {
      Erg->Typ = TempString;
      return;
   }

   InternSymbol(Asc, Erg);
   if (Erg->Typ != TempNone) return;

/* Zaehler initialisieren */

   LocOpMax = 0;
   OpMax = 0;
   LKlamm = 0;
   RKlamm = 0;
   WKlamm = 0;
   InHyp = false;
   InQuot = false;
   for (Op = Operators + 1; Op <= OpEnd; Op++)
      if (strstr(Asc, Op->Id) != NULL) FOps[FOpCnt++] = Op;

/* nach Operator hoechster Rangstufe ausserhalb Klammern suchen */

   for (zp = Asc; *zp != '\0'; zp++) {
      switch (*zp) {
         case '(':
            if (!(InHyp || InQuot)) LKlamm++;
            break;
         case ')':
            if (!(InHyp || InQuot)) RKlamm++;
            break;
         case '{':
            if (!(InHyp || InQuot)) WKlamm++;
            break;
         case '}':
            if (!(InHyp || InQuot)) WKlamm--;
            break;
         case '"':
            if (!InHyp) InQuot = !InQuot;
            break;
         case '\'':
            if (!InQuot) InHyp = !InHyp;
            break;
         default:
            if ((LKlamm == RKlamm) && (WKlamm == 0) && (!InHyp) && (!InQuot)) {
               OpFnd = false;
               OpLen = 0;
               LocOpMax = 0;
               for (zop = 0; zop < FOpCnt; zop++)
                  if (strncmp(zp, FOps[zop]->Id, FOps[zop]->IdLen) == 0)
                     if (FOps[zop]->IdLen >= OpLen) {
                        OpFnd = true;
                        OpLen = FOps[zop]->IdLen;
                        LocOpMax = FOps[zop] - Operators;
                        if (Operators[LocOpMax].Priority >= Operators[OpMax].Priority) {
                           OpMax = LocOpMax;
                           OpPos = zp - Asc;
                        }
                     }
               if (OpFnd) zp += strlen(Operators[LocOpMax].Id) - 1;
            }
      }
   }

/* Klammerfehler ? */

   if (LKlamm != RKlamm) {
      WrXError(1300, Asc);
      return;
   }

/* Operator gefunden ? */

   if (OpMax != 0) {
      Op = Operators + OpMax;

   /* Minuszeichen sowohl mit einem als auch 2 Operanden */

      if (strcmp(Op->Id, "-") == 0) Op->Dyadic = (OpPos > 0);

   /* Operandenzahl pruefen */

      if (((Op->Dyadic) && (OpPos == 0)) || ((!Op->Dyadic) && (OpPos != 0)) || (OpPos == strlen(Asc) - 1)) {
         WrError(1110);
         return;
      }

   /* Teilausdruecke rekursiv auswerten */

      Save = Asc[OpPos];
      Asc[OpPos] = '\0';
      if (Op->Dyadic) EvalExpression(Asc, &LVal);
      else {
         LVal.Typ = TempInt;
         LVal.Contents.Int = 0;
      }
      EvalExpression(Asc + OpPos + strlen(Op->Id), &RVal);
      Asc[OpPos] = Save;

   /* Abbruch, falls dabei Fehler */

      if ((LVal.Typ == TempNone) || (RVal.Typ == TempNone)) return;

   /* Typueberpruefung */

      if ((Op->Dyadic) && (LVal.Typ != RVal.Typ)) {
         if ((LVal.Typ == TempString) || (RVal.Typ == TempString)) {
            WrError(1135);
            return;
         }
         if (LVal.Typ == TempInt) EvalExpression_ChgFloat(&LVal);
         if (RVal.Typ == TempInt) EvalExpression_ChgFloat(&RVal);
      }

      switch (RVal.Typ) {
         case TempInt:
            if (!Op->MayInt)
               if (!Op->MayFloat) {
                  WrError(1135);
                  return;
               } else {
                  EvalExpression_ChgFloat(&RVal);
                  if (Op->Dyadic) EvalExpression_ChgFloat(&LVal);
               }
            break;
         case TempFloat:
            if (!Op->MayFloat) {
               WrError(1135);
               return;
            }
            break;
         case TempString:
            if (!Op->MayString) {
               WrError(1135);
               return;
            };
            break;
         default:
            break;
      }

   /* Operanden abarbeiten */

      switch (OpMax) {
         case 1: /* ~ */
            Erg->Typ = TempInt;
            Erg->Contents.Int = ~RVal.Contents.Int;
            break;
         case 2: /* << */
            Erg->Typ = TempInt;
            Erg->Contents.Int = LVal.Contents.Int << RVal.Contents.Int;
            break;
         case 3: /* >> */
            Erg->Typ = TempInt;
            Erg->Contents.Int = LVal.Contents.Int >> RVal.Contents.Int;
            break;
         case 4: /* >< */
            Erg->Typ = TempInt;
            if ((RVal.Contents.Int < 1) || (RVal.Contents.Int > 32)) WrError(1320);
            else {
               Erg->Contents.Int = (LVal.Contents.Int >> RVal.Contents.Int) << RVal.Contents.Int;
               RVal.Contents.Int--;
               for (z1 = 0; z1 <= RVal.Contents.Int; z1++) {
                  if ((LVal.Contents.Int & (1 << (RVal.Contents.Int - z1))) != 0)
                     Erg->Contents.Int += (1 << z1);
               }
            }
            break;
         case 5: /* & */
            Erg->Typ = TempInt;
            Erg->Contents.Int = LVal.Contents.Int & RVal.Contents.Int;
            break;
         case 6: /* | */
            Erg->Typ = TempInt;
            Erg->Contents.Int = LVal.Contents.Int | RVal.Contents.Int;
            break;
         case 7:
            Erg->Typ = TempInt; /* ! */
            Erg->Contents.Int = LVal.Contents.Int ^ RVal.Contents.Int;
            break;
         case 8: /* ^ */
            switch (Erg->Typ = LVal.Typ) {
               case TempInt:
                  if (RVal.Contents.Int < 0) Erg->Contents.Int = 0;
                  else {
                     Erg->Contents.Int = 1;
                     while (RVal.Contents.Int > 0) {
                        if ((RVal.Contents.Int & 1) == 1) Erg->Contents.Int *= LVal.Contents.Int;
                        RVal.Contents.Int >>= 1;
                        if (RVal.Contents.Int != 0) LVal.Contents.Int *= LVal.Contents.Int;
                     }
                  }
                  break;
               case TempFloat:
                  if (RVal.Contents.Float == 0.0) Erg->Contents.Float = 1.0;
                  else if (LVal.Contents.Float == 0.0) Erg->Contents.Float = 0.0;
                  else if (LVal.Contents.Float > 0) Erg->Contents.Float = pow(LVal.Contents.Float, RVal.Contents.Float);
                  else if ((abs(RVal.Contents.Float) <= ((double)MaxLongInt)) && (floor(RVal.Contents.Float) == RVal.Contents.Float)) {
                     HVal = (LongInt) floor(RVal.Contents.Float + 0.5);
                     if (HVal < 0) {
                        LVal.Contents.Float = 1 / LVal.Contents.Float;
                        HVal = (-HVal);
                     }
                     Erg->Contents.Float = 1.0;
                     while (HVal > 0) {
                        if ((HVal & 1) == 1) Erg->Contents.Float *= LVal.Contents.Float;
                        LVal.Contents.Float *= LVal.Contents.Float;
                        HVal >>= 1;
                     }
                  } else {
                     WrError(1890);
                     Erg->Typ = TempNone;
                  }
                  break;
               default:
                  break;
            }
            break;
         case 9: /* * */
            switch (Erg->Typ = LVal.Typ) {
               case TempInt:
                  Erg->Contents.Int = LVal.Contents.Int * RVal.Contents.Int;
                  break;
               case TempFloat:
                  Erg->Contents.Float = LVal.Contents.Float * RVal.Contents.Float;
                  break;
               default:
                  break;
            }
            break;
         case 10: /* / */
            switch (LVal.Typ) {
               case TempInt:
                  if (RVal.Contents.Int == 0) WrError(1310);
                  else {
                     Erg->Typ = TempInt;
                     Erg->Contents.Int = LVal.Contents.Int / RVal.Contents.Int;
                  }
                  break;
               case TempFloat:
                  if (RVal.Contents.Float == 0.0) WrError(1310);
                  else {
                     Erg->Typ = TempFloat;
                     Erg->Contents.Float = LVal.Contents.Float / RVal.Contents.Float;
                  }
               default:
                  break;
            }
            break;
         case 11: /* # */
            if (RVal.Contents.Int == 0) WrError(1310);
            else {
               Erg->Typ = TempInt;
               Erg->Contents.Int = LVal.Contents.Int % RVal.Contents.Int;
            }
            break;
         case 12: /* + */
            switch (Erg->Typ = LVal.Typ) {
               case TempInt:
                  Erg->Contents.Int = LVal.Contents.Int + RVal.Contents.Int;
                  break;
               case TempFloat:
                  Erg->Contents.Float = LVal.Contents.Float + RVal.Contents.Float;
                  break;
               case TempString:
                  strmaxcpy(Erg->Contents.Ascii, LVal.Contents.Ascii, 255);
                  strmaxcat(Erg->Contents.Ascii, RVal.Contents.Ascii, 255);
                  break;
               default:
                  break;
            }
            break;
         case 13: /* - */
            if (Op->Dyadic)
               switch (Erg->Typ = LVal.Typ) {
                  case TempInt:
                     Erg->Contents.Int = LVal.Contents.Int - RVal.Contents.Int;
                     break;
                  case TempFloat:
                     Erg->Contents.Float = LVal.Contents.Float - RVal.Contents.Float;
                     break;
                  default:
                     break;
            } else
               switch (Erg->Typ = RVal.Typ) {
                  case TempInt:
                     Erg->Contents.Int = (-RVal.Contents.Int);
                     break;
                  case TempFloat:
                     Erg->Contents.Float = (-RVal.Contents.Float);
                     break;
                  default:
                     break;
               }
            break;
         case 14: /* ~~ */
            Erg->Typ = TempInt;
            Erg->Contents.Int = (RVal.Contents.Int == 0) ? 1 : 0;
            break;
         case 15: /* && */
            Erg->Typ = TempInt;
            Erg->Contents.Int = ((LVal.Contents.Int != 0) && (RVal.Contents.Int != 0)) ? 1 : 0;
            break;
         case 16: /* || */
            Erg->Typ = TempInt;
            Erg->Contents.Int = ((LVal.Contents.Int != 0) || (RVal.Contents.Int != 0)) ? 1 : 0;
            break;
         case 17: /* !! */
            Erg->Typ = TempInt;
            if ((LVal.Contents.Int != 0) && (RVal.Contents.Int == 0))
               Erg->Contents.Int = 1;
            else if ((LVal.Contents.Int == 0) && (RVal.Contents.Int != 0))
               Erg->Contents.Int = 1;
            else Erg->Contents.Int = 0;
            break;
         case 18: /* = */
            Erg->Typ = TempInt;
            switch (LVal.Typ) {
               case TempInt:
                  Erg->Contents.Int = (LVal.Contents.Int == RVal.Contents.Int) ? 1 : 0;
                  break;
               case TempFloat:
                  Erg->Contents.Int = (LVal.Contents.Float == RVal.Contents.Float) ? 1 : 0;
                  break;
               case TempString:
                  Erg->Contents.Int = (strcmp(LVal.Contents.Ascii, RVal.Contents.Ascii) == 0) ? 1 : 0;
                  break;
               default:
                  break;
            }
            break;
         case 19: /* > */
            Erg->Typ = TempInt;
            switch (LVal.Typ) {
               case TempInt:
                  Erg->Contents.Int = (LVal.Contents.Int > RVal.Contents.Int) ? 1 : 0;
                  break;
               case TempFloat:
                  Erg->Contents.Int = (LVal.Contents.Float > RVal.Contents.Float) ? 1 : 0;
                  break;
               case TempString:
                  Erg->Contents.Int = (strcmp(LVal.Contents.Ascii, RVal.Contents.Ascii) > 0) ? 1 : 0;
                  break;
               default:
                  break;
            }
            break;
         case 20: /* < */
            Erg->Typ = TempInt;
            switch (LVal.Typ) {
               case TempInt:
                  Erg->Contents.Int = (LVal.Contents.Int < RVal.Contents.Int) ? 1 : 0;
                  break;
               case TempFloat:
                  Erg->Contents.Int = (LVal.Contents.Float < RVal.Contents.Float) ? 1 : 0;
                  break;
               case TempString:
                  Erg->Contents.Int = (strcmp(LVal.Contents.Ascii, RVal.Contents.Ascii) < 0) ? 1 : 0;
                  break;
               default:
                  break;
            }
            break;
         case 21: /* <= */
            Erg->Typ = TempInt;
            switch (LVal.Typ) {
               case TempInt:
                  Erg->Contents.Int = (LVal.Contents.Int <= RVal.Contents.Int) ? 1 : 0;
                  break;
               case TempFloat:
                  Erg->Contents.Int = (LVal.Contents.Float <= RVal.Contents.Float) ? 1 : 0;
                  break;
               case TempString:
                  Erg->Contents.Int = (strcmp(LVal.Contents.Ascii, RVal.Contents.Ascii) <= 0) ? 1 : 0;
                  break;
               default:
                  break;
            }
            break;
         case 22: /* >= */
            Erg->Typ = TempInt;
            switch (LVal.Typ) {
               case TempInt:
                  Erg->Contents.Int = (LVal.Contents.Int >= RVal.Contents.Int) ? 1 : 0;
                  break;
               case TempFloat:
                  Erg->Contents.Int = (LVal.Contents.Float >= RVal.Contents.Float) ? 1 : 0;
                  break;
               case TempString:
                  Erg->Contents.Int = (strcmp(LVal.Contents.Ascii, RVal.Contents.Ascii) >= 0) ? 1 : 0;
                  break;
               default:
                  break;
            }
            break;
         case 23: /* <> */
            Erg->Typ = TempInt;
            switch (LVal.Typ) {
               case TempInt:
                  Erg->Contents.Int = (LVal.Contents.Int != RVal.Contents.Int) ? 1 : 0;
                  break;
               case TempFloat:
                  Erg->Contents.Int = (LVal.Contents.Float != RVal.Contents.Float) ? 1 : 0;
                  break;
               case TempString:
                  Erg->Contents.Int = (strcmp(LVal.Contents.Ascii, RVal.Contents.Ascii) != 0) ? 1 : 0;
                  break;
               default:
                  break;
            }
            break;
      }
      return;
   }

/* kein Operator gefunden: Klammerausdruck ? */

   if (LKlamm != 0) {

   /* erste Klammer suchen, Funktionsnamen abtrennen */

      KlPos = strchr(Asc, '(');

   /* Funktionsnamen abschneiden */

      *KlPos = '\0';
      strmaxcpy(ftemp, Asc, 255);
      strcpy(Asc, KlPos + 1);
      Asc[strlen(Asc) - 1] = '\0';

   /* Nullfunktion: nur Argument */

      if (ftemp[0] == '\0') {
         EvalExpression(Asc, &LVal);
         *Erg = LVal;
         return;
      }

   /* selbstdefinierte Funktion ? */

      if ((ValFunc = FindFunction(ftemp)) != NULL) {
         strmaxcpy(ftemp, ValFunc->Definition, 255);
         for (z1 = 1; z1 <= ValFunc->ArguCnt; z1++) {
            if (Asc[0] == '\0') {
               WrError(1490);
               return;
            };
            KlPos = QuotPos(Asc, ',');
            if (KlPos != NULL) *KlPos = '\0';
            EvalExpression(Asc, &LVal);
            if (KlPos == NULL) Asc[0] = '\0';
            else strcpy(Asc, KlPos + 1);
            switch (LVal.Typ) {
               case TempInt:
                  sprintf(stemp, "%s", LargeString(LVal.Contents.Int));
                  break;
               case TempFloat:
                  sprintf(stemp, "%0.16e", LVal.Contents.Float);
                  KillBlanks(stemp);
                  break;
               case TempString:
                  strcpy(stemp, "\"");
                  strmaxcat(stemp, LVal.Contents.Ascii, 255);
                  strmaxcat(stemp, "\"", 255);
                  break;
               default:
                  return;
            }
            memmove(stemp + 1, stemp, strlen(stemp) + 1);
            stemp[0] = '(';
            strmaxcat(stemp, ")", 255);
            ExpandLine(stemp, z1, ftemp);
         }
         if (Asc[0] != '\0') {
            WrError(1490);
            return;
         }
         EvalExpression(ftemp, Erg);
         return;
      }

   /* Unterausdruck auswerten (interne Funktionen nur mit einem Argument */

      EvalExpression(Asc, &LVal);

   /* Abbruch bei Fehler */

      if (LVal.Typ == TempNone) return;

   /* hier einmal umwandeln ist effizienter */

      NLS_UpString(ftemp);

   /* Funktionen fuer Stringargumente */

      if (LVal.Typ == TempString) {
      /* in Grossbuchstaben wandeln ? */

         if (strcmp(ftemp, "UPSTRING") == 0) {
            Erg->Typ = TempString;
            strmaxcpy(Erg->Contents.Ascii, LVal.Contents.Ascii, 255);
            for (KlPos = Erg->Contents.Ascii; *KlPos != '\0'; KlPos++)
               *KlPos = toupper(*KlPos);
         }

      /* in Kleinbuchstaben wandeln ? */

         else if (strcmp(ftemp, "LOWSTRING") == 0) {
            Erg->Typ = TempString;
            strmaxcpy(Erg->Contents.Ascii, LVal.Contents.Ascii, 255);
            for (KlPos = Erg->Contents.Ascii; *KlPos != '\0'; KlPos++)
               *KlPos = tolower(*KlPos);
         }

      /* Parser aufrufen ? */

         else if (strcmp(ftemp, "VAL") == 0) {
            EvalExpression(LVal.Contents.Ascii, Erg);
         }

      /* nix gefunden ? */

         else {
            WrXError(1860, ftemp);
            Erg->Typ = TempNone;
         }
      }

   /* Funktionen fuer Zahlenargumente */

      else {
         FFound = false;
         Erg->Typ = TempNone;

      /* reine Integerfunktionen */

         if (strcmp(ftemp, "TOUPPER") == 0) {
            if (LVal.Typ != TempInt) WrError(1135);
            else if ((LVal.Contents.Int < 0) || (LVal.Contents.Int > 255)) WrError(1320);
            else {
               Erg->Typ = TempInt;
               Erg->Contents.Int = toupper(LVal.Contents.Int);
            }
            FFound = true;
         }

         else if (strcmp(ftemp, "TOLOWER") == 0) {
            if (LVal.Typ != TempInt) WrError(1135);
            else if ((LVal.Contents.Int < 0) || (LVal.Contents.Int > 255)) WrError(1320);
            else {
               Erg->Typ = TempInt;
               Erg->Contents.Int = tolower(LVal.Contents.Int);
            }
            FFound = true;
         }

         else if (strcmp(ftemp, "BITCNT") == 0) {
            if (LVal.Typ != TempInt) WrError(1135);
            else {
               Erg->Typ = TempInt;
               Erg->Contents.Int = 0;
#ifdef HAS64
               for (z1 = 0; z1 < 64; z1++)
#else
               for (z1 = 0; z1 < 32; z1++)
#endif
               {
                  Erg->Contents.Int += (LVal.Contents.Int & 1);
                  LVal.Contents.Int = LVal.Contents.Int >> 1;
               }
            }
            FFound = true;
         }

         else if (strcmp(ftemp, "FIRSTBIT") == 0) {
            if (LVal.Typ != TempInt) WrError(1135);
            else {
               Erg->Typ = TempInt;
               Erg->Contents.Int = 0;
               do {
                  if (!Odd(LVal.Contents.Int)) Erg->Contents.Int++;
                  LVal.Contents.Int = LVal.Contents.Int >> 1;
               }
#ifdef HAS64
               while ((Erg->Contents.Int < 64) && (!Odd(LVal.Contents.Int)));
               if (Erg->Contents.Int >= 64) Erg->Contents.Int = (-1);
#else
               while ((Erg->Contents.Int < 32) && (!Odd(LVal.Contents.Int)));
               if (Erg->Contents.Int >= 32) Erg->Contents.Int = (-1);
#endif
            }
            FFound = true;
         }

         else if (strcmp(ftemp, "LASTBIT") == 0) {
            if (LVal.Typ != TempInt) WrError(1135);
            else {
               Erg->Typ = TempInt;
               Erg->Contents.Int = (-1);
#ifdef HAS64
               for (z1 = 0; z1 < 64; z1++)
#else
               for (z1 = 0; z1 < 32; z1++)
#endif
               {
                  if (Odd(LVal.Contents.Int)) Erg->Contents.Int = z1;
                  LVal.Contents.Int = LVal.Contents.Int >> 1;
               }
            }
            FFound = true;
         }

         else if (strcmp(ftemp, "BITPOS") == 0) {
            if (LVal.Typ != TempInt) WrError(1135);
            else {
               Erg->Typ = TempInt;
               Erg->Contents.Int = 0;
               do {
                  if (!Odd(LVal.Contents.Int)) Erg->Contents.Int++;
                  if (!Odd(LVal.Contents.Int)) LVal.Contents.Int = LVal.Contents.Int >> 1;
               }
#ifdef HAS64
               while ((Erg->Contents.Int < 64) && (!Odd(LVal.Contents.Int)));
               if ((Erg->Contents.Int >= 64) || (LVal.Contents.Int != 1))
#else
               while ((Erg->Contents.Int < 32) && (!Odd(LVal.Contents.Int)));
               if ((Erg->Contents.Int >= 32) || (LVal.Contents.Int != 1))
#endif
               {
                  Erg->Contents.Int = (-1);
                  WrError(1540);
               }
            }
            FFound = true;
         }

      /* variable Integer/Float-Funktionen */

         else if (strcmp(ftemp, "ABS") == 0) {
            switch (Erg->Typ = LVal.Typ) {
               case TempInt:
                  Erg->Contents.Int = abs(LVal.Contents.Int);
                  break;
               case TempFloat:
                  Erg->Contents.Float = fabs(LVal.Contents.Float);
                  break;
               default:
                  break;
            }
            FFound = true;
         }

         else if (strcmp(ftemp, "SGN") == 0) {
            Erg->Typ = TempInt;
            switch (LVal.Typ) {
               case TempInt:
                  if (LVal.Contents.Int < 0) Erg->Contents.Int = (-1);
                  else if (LVal.Contents.Int > 0) Erg->Contents.Int = 1;
                  else Erg->Contents.Int = 0;
                  break;
               case TempFloat:
                  if (LVal.Contents.Float < 0) Erg->Contents.Int = (-1);
                  else if (LVal.Contents.Float > 0) Erg->Contents.Int = 1;
                  else Erg->Contents.Int = 0;
                  break;
               default:
                  break;
            }
            FFound = true;
         }

      /* Funktionen Float und damit auch Int */

         if (!FFound) {
         /* Typkonvertierung */

            EvalExpression_ChgFloat(&LVal);
            Erg->Typ = TempFloat;

         /* Integerwandlung */

            if (strcmp(ftemp, "INT") == 0) {
               if (fabs(LVal.Contents.Float) > MaxLargeInt) {
                  Erg->Typ = TempNone;
                  WrError(1320);
               } else {
                  Erg->Typ = TempInt;
                  Erg->Contents.Int = (LargeInt) floor(LVal.Contents.Float);
               }
            }

         /* Quadratwurzel */

            else if (strcmp(ftemp, "SQRT") == 0) {
               if (LVal.Contents.Float < 0) {
                  Erg->Typ = TempNone;
                  WrError(1870);
               } else Erg->Contents.Float = sqrt(LVal.Contents.Float);
            }

         /* trigonometrische Funktionen */

            else if (strcmp(ftemp, "SIN") == 0) Erg->Contents.Float = sin(LVal.Contents.Float);
            else if (strcmp(ftemp, "COS") == 0) Erg->Contents.Float = cos(LVal.Contents.Float);
            else if (strcmp(ftemp, "TAN") == 0) {
               if (cos(LVal.Contents.Float) == 0.0) {
                  Erg->Typ = TempNone;
                  WrError(1870);
               } else Erg->Contents.Float = tan(LVal.Contents.Float);
            } else if (strcmp(ftemp, "COT") == 0) {
               if ((FVal = sin(LVal.Contents.Float)) == 0.0) {
                  Erg->Typ = TempNone;
                  WrError(1870);
               } else Erg->Contents.Float = cos(LVal.Contents.Float) / FVal;
            }

         /* inverse trigonometrische Funktionen */

            else if (strcmp(ftemp, "ASIN") == 0) {
               if (fabs(LVal.Contents.Float) > 1) {
                  Erg->Typ = TempNone;
                  WrError(1870);
               } else Erg->Contents.Float = asin(LVal.Contents.Float);
            } else if (strcmp(ftemp, "ACOS") == 0) {
               if (fabs(LVal.Contents.Float) > 1) {
                  Erg->Typ = TempNone;
                  WrError(1870);
               } else Erg->Contents.Float = acos(LVal.Contents.Float);
            } else if (strcmp(ftemp, "ATAN") == 0) Erg->Contents.Float = atan(LVal.Contents.Float);
            else if (strcmp(ftemp, "ACOT") == 0) Erg->Contents.Float = M_PI / 2 - (LVal.Contents.Float);

         /* exponentielle & hyperbolische Funktionen */

            else if (strcmp(ftemp, "EXP") == 0) {
               if (LVal.Contents.Float > 709) {
                  Erg->Typ = TempNone;
                  WrError(1880);
               } else Erg->Contents.Float = exp(LVal.Contents.Float);
            } else if (strcmp(ftemp, "ALOG") == 0) {
               if (LVal.Contents.Float > 308) {
                  Erg->Typ = TempNone;
                  WrError(1880);
               } else Erg->Contents.Float = exp(LVal.Contents.Float * log(10.0));
            } else if (strcmp(ftemp, "ALD") == 0) {
               if (LVal.Contents.Float > 1022) {
                  Erg->Typ = TempNone;
                  WrError(1880);
               } else Erg->Contents.Float = exp(LVal.Contents.Float * log(2.0));
            } else if (strcmp(ftemp, "SINH") == 0) {
               if (LVal.Contents.Float > 709) {
                  Erg->Typ = TempNone;
                  WrError(1880);
               } else Erg->Contents.Float = sinh(LVal.Contents.Float);
            } else if (strcmp(ftemp, "COSH") == 0) {
               if (LVal.Contents.Float > 709) {
                  Erg->Typ = TempNone;
                  WrError(1880);
               } else Erg->Contents.Float = cosh(LVal.Contents.Float);
            } else if (strcmp(ftemp, "TANH") == 0) {
               if (LVal.Contents.Float > 709) {
                  Erg->Typ = TempNone;
                  WrError(1880);
               } else Erg->Contents.Float = tanh(LVal.Contents.Float);
            } else if (strcmp(ftemp, "COTH") == 0) {
               if (LVal.Contents.Float > 709) {
                  Erg->Typ = TempNone;
                  WrError(1880);
               } else if ((FVal = tanh(LVal.Contents.Float)) == 0.0) {
                  Erg->Typ = TempNone;
                  WrError(1870);
               } else Erg->Contents.Float = 1.0 / FVal;
            }

         /* logarithmische & inverse hyperbolische Funktionen */

            else if (strcmp(ftemp, "LN") == 0) {
               if (LVal.Contents.Float <= 0) {
                  Erg->Typ = TempNone;
                  WrError(1870);
               } else Erg->Contents.Float = log(LVal.Contents.Float);
            } else if (strcmp(ftemp, "LOG") == 0) {
               if (LVal.Contents.Float <= 0) {
                  Erg->Typ = TempNone;
                  WrError(1870);
               } else Erg->Contents.Float = log10(LVal.Contents.Float);
            } else if (strcmp(ftemp, "LD") == 0) {
               if (LVal.Contents.Float <= 0) {
                  Erg->Typ = TempNone;
                  WrError(1870);
               } else Erg->Contents.Float = log(LVal.Contents.Float) / log(2.0);
            } else if (strcmp(ftemp, "ASINH") == 0)
               Erg->Contents.Float = log(LVal.Contents.Float + sqrt(LVal.Contents.Float * LVal.Contents.Float + 1));
            else if (strcmp(ftemp, "ACOSH") == 0) {
               if (LVal.Contents.Float < 1) {
                  Erg->Typ = TempNone;
                  WrError(1880);
               } else Erg->Contents.Float = log(LVal.Contents.Float + sqrt(LVal.Contents.Float * LVal.Contents.Float - 1));
            } else if (strcmp(ftemp, "ATANH") == 0) {
               if (fabs(LVal.Contents.Float) >= 1) {
                  Erg->Typ = TempNone;
                  WrError(1880);
               } else Erg->Contents.Float = 0.5 * log((1 + LVal.Contents.Float) / (1 - LVal.Contents.Float));
            } else if (strcmp(ftemp, "ACOTH") == 0) {
               if (fabs(LVal.Contents.Float) <= 1) {
                  Erg->Typ = TempNone;
                  WrError(1880);
               } else Erg->Contents.Float = 0.5 * log((LVal.Contents.Float + 1) / (LVal.Contents.Float - 1));
            }

         /* nix gefunden ? */

            else {
               WrXError(1860, ftemp);
               Erg->Typ = TempNone;
            }
         }
      }
      return;
   }

/* nichts dergleichen, dann einfaches Symbol: */

/* interne Symbole ? */

   strmaxcpy(Asc, stemp, 255);
   KillPrefBlanks(Asc);
   KillPostBlanks(Asc);

   if (strcasecmp(Asc, "MOMFILE") == 0) {
      Erg->Typ = TempString;
      strmaxcpy(Erg->Contents.Ascii, CurrFileName, 255);
      return;
   };

   if (strcasecmp(Asc, "MOMLINE") == 0) {
      Erg->Typ = TempInt;
      Erg->Contents.Int = CurrLine;
      return;
   }

   if (strcasecmp(Asc, "MOMPASS") == 0) {
      Erg->Typ = TempInt;
      Erg->Contents.Int = PassNo;
      return;
   }

   if (strcasecmp(Asc, "MOMSECTION") == 0) {
      Erg->Typ = TempString;
      strmaxcpy(Erg->Contents.Ascii, GetSectionName(MomSectionHandle), 255);
      return;
   }

   if (strcasecmp(Asc, "MOMSEGMENT") == 0) {
      Erg->Typ = TempString;
      strmaxcpy(Erg->Contents.Ascii, SegNames[ActPC], 255);
      return;
   }

   if (!ExpandSymbol(Asc)) return;

   KlPos = strchr(Asc, '[');
   if (KlPos != NULL) {
      Save = (*KlPos);
      *KlPos = '\0';
   }
   OK = ChkSymbName(Asc);
   if (KlPos != NULL) *KlPos = Save;
   if (!OK) {
      WrXError(1020, Asc);
      return;
   };

   Ptr = FindLocNode(Asc, TempInt);
   if (Ptr == NULL) Ptr = FindNode(Asc, TempInt);
   if (Ptr != NULL) {
      Erg->Typ = TempInt;
      Erg->Contents.Int = Ptr->SymWert.Contents.IWert;
      if (Ptr->SymType != 0) TypeFlag |= (1 << Ptr->SymType);
      if ((Ptr->SymSize != (-1)) && (SizeFlag == (-1))) SizeFlag = Ptr->SymSize;
      if (!Ptr->Defined) {
         if (Repass) SymbolQuestionable = true;
         UsesForwards = true;
      }
      Ptr->Used = true;
      return;
   }
   Ptr = FindLocNode(Asc, TempFloat);
   if (Ptr == NULL) Ptr = FindNode(Asc, TempFloat);
   if (Ptr != NULL) {
      Erg->Typ = TempFloat;
      Erg->Contents.Float = Ptr->SymWert.Contents.FWert;
      if (Ptr->SymType != 0) TypeFlag |= (1 << Ptr->SymType);
      if ((Ptr->SymSize != (-1)) && (SizeFlag == (-1))) SizeFlag = Ptr->SymSize;
      if (!Ptr->Defined) {
         if (Repass) SymbolQuestionable = true;
         UsesForwards = true;
      }
      Ptr->Used = true;
      return;
   }
   Ptr = FindLocNode(Asc, TempString);
   if (Ptr == NULL) Ptr = FindNode(Asc, TempString);
   if (Ptr != NULL) {
      Erg->Typ = TempString;
      strmaxcpy(Erg->Contents.Ascii, Ptr->SymWert.Contents.SWert, 255);
      if (Ptr->SymType != 0) TypeFlag |= (1 << Ptr->SymType);
      if ((Ptr->SymSize != (-1)) && (SizeFlag == (-1))) SizeFlag = Ptr->SymSize;
      if (!Ptr->Defined) {
         if (Repass) SymbolQuestionable = true;
         UsesForwards = true;
      }
      Ptr->Used = true;
      return;
   }

/* Symbol evtl. im ersten Pass unbekannt */

   if (PassNo <= MaxSymPass) {
      Erg->Typ = TempInt;
      Erg->Contents.Int = EProgCounter();
      Repass = true;
      if ((MsgIfRepass) && (PassNo >= PassNoForMessage)) WrXError(170, Asc);
      FirstPassUnknown = true;
   }

/* alles war nix, Fehler */

   else WrXError(1010, Asc);
}

LargeInt EvalIntExpression(char *Asc, IntType Typ, bool *OK) {
   TempResult t;

   *OK = false;
   TypeFlag = 0;
   SizeFlag = (-1);
   UsesForwards = false;
   SymbolQuestionable = false;
   FirstPassUnknown = false;

   EvalExpression(Asc, &t);
   if (t.Typ != TempInt) {
      if (t.Typ != TempNone) WrError(1135);
      return -1;
   }

   if (FirstPassUnknown) t.Contents.Int &= IntMasks[Typ];

   if (!RangeCheck(t.Contents.Int, Typ)) {
      WrError(1320);
      return -1;
   }

   *OK = true;
   return t.Contents.Int;
}

Double EvalFloatExpression(char *Asc, FloatType Typ, bool *OK) {
   TempResult t;

   *OK = false;
   TypeFlag = 0;
   SizeFlag = (-1);
   UsesForwards = false;
   SymbolQuestionable = false;
   FirstPassUnknown = false;

   EvalExpression(Asc, &t);
   switch (t.Typ) {
      case TempNone:
         return -1;
      case TempInt:
         t.Contents.Float = t.Contents.Int;
         break;
      case TempString:
         WrError(1135);
         return -1;
      default:
         break;
   }

   if (!FloatRangeCheck(t.Contents.Float, Typ)) {
      WrError(1320);
      return -1;
   }

   *OK = true;
   return t.Contents.Float;
}

void EvalStringExpression(char *Asc, bool *OK, char *Result) {
   TempResult t;

   *OK = false;
   TypeFlag = 0;
   SizeFlag = (-1);
   UsesForwards = false;
   SymbolQuestionable = false;
   FirstPassUnknown = false;

   EvalExpression(Asc, &t);
   if (t.Typ != TempString) {
      *Result = '\0';
      if (t.Typ != TempNone) WrError(1135);
      return;
   }

   strmaxcpy(Result, t.Contents.Ascii, 255);
   *OK = true;
}

static void FreeSymbol(SymbolPtr * Node) {
   PCrossRef Lauf;

   free((*Node)->SymName);

   if ((*Node)->SymWert.Typ == TempString)
      free((*Node)->SymWert.Contents.SWert);

   while ((*Node)->RefList != NULL) {
      Lauf = (*Node)->RefList->Next;
      free((*Node)->RefList);
      (*Node)->RefList = Lauf;
   }

   free(*Node);
   *Node = NULL;
}

static String serr;
static char snum[11];

bool EnterTreeNode(SymbolPtr * Node, SymbolPtr Neu, bool MayChange, bool DoCross) {
   SymbolPtr Hilf, p1, p2;
   bool Grown, Result;
   ShortInt CompErg;

/* Stapelueberlauf pruefen, noch nichts eingefuegt */

   ChkStack();
   Result = false;

/* an einem Blatt angelangt--> einfach anfuegen */

   if (*Node == NULL) {
      (*Node) = Neu;
      (*Node)->Balance = 0;
      (*Node)->Left = NULL;
      (*Node)->Right = NULL;
      (*Node)->Defined = true;
      (*Node)->Used = false;
      (*Node)->Changeable = MayChange;
      (*Node)->RefList = NULL;
      if (DoCross) {
         (*Node)->FileNum = GetFileNum(CurrFileName);
         (*Node)->LineNum = CurrLine;
      }
      return true;
   }

   CompErg = StrCmp(Neu->SymName, (*Node)->SymName, Neu->Attribute, (*Node)->Attribute);

   switch (CompErg) {
      case 1:
         Grown = EnterTreeNode(&((*Node)->Right), Neu, MayChange, DoCross);
         if ((BalanceTree) && (Grown))
            switch ((*Node)->Balance) {
               case -1:
                  (*Node)->Balance = 0;
                  break;
               case 0:
                  (*Node)->Balance = 1;
                  Result = true;
                  break;
               case 1:
                  p1 = (*Node)->Right;
                  if (p1->Balance == 1) {
                     (*Node)->Right = p1->Left;
                     p1->Left = (*Node);
                     (*Node)->Balance = 0;
                     *Node = p1;
                  } else {
                     p2 = p1->Left;
                     p1->Left = p2->Right;
                     p2->Right = p1;
                     (*Node)->Right = p2->Left;
                     p2->Left = (*Node);
                     if (p2->Balance == 1) (*Node)->Balance = (-1);
                     else (*Node)->Balance = 0;
                     if (p2->Balance == -1) p1->Balance = 1;
                     else p1->Balance = 0;
                     *Node = p2;
                  }
                  (*Node)->Balance = 0;
                  break;
            }
         break;
      case -1:
         Grown = EnterTreeNode(&((*Node)->Left), Neu, MayChange, DoCross);
         if ((BalanceTree) && (Grown))
            switch ((*Node)->Balance) {
               case 1:
                  (*Node)->Balance = 0;
                  break;
               case 0:
                  (*Node)->Balance = (-1);
                  Result = true;
                  break;
               case -1:
                  p1 = (*Node)->Left;
                  if (p1->Balance == (-1)) {
                     (*Node)->Left = p1->Right;
                     p1->Right = (*Node);
                     (*Node)->Balance = 0;
                     (*Node) = p1;
                  } else {
                     p2 = p1->Right;
                     p1->Right = p2->Left;
                     p2->Left = p1;
                     (*Node)->Left = p2->Right;
                     p2->Right = (*Node);
                     if (p2->Balance == (-1)) (*Node)->Balance = 1;
                     else (*Node)->Balance = 0;
                     if (p2->Balance == 1) p1->Balance = (-1);
                     else p1->Balance = 0;
                     *Node = p2;
                  }
                  (*Node)->Balance = 0;
                  break;
            }
         break;
      case 0:
         if (((*Node)->Defined) && (!MayChange)) {
            strmaxcpy(serr, (*Node)->SymName, 255);
            if (DoCross) {
               sprintf(snum, "%d", (*Node)->LineNum);
               strmaxcat(serr, ", ", 255);
               strmaxcat(serr, PrevDefMsg, 255);
               strmaxcat(serr, " ", 255);
               strmaxcat(serr, GetFileName((*Node)->FileNum), 255);
               strmaxcat(serr, ":", 255);
               strmaxcat(serr, snum, 255);
            }
            WrXError(1000, serr);
         } else {
            if (!MayChange) {
               if ((Neu->SymWert.Typ != (*Node)->SymWert.Typ)
                  || ((Neu->SymWert.Typ == TempString) && (strcmp(Neu->SymWert.Contents.SWert, (*Node)->SymWert.Contents.SWert) != 0))
                  || ((Neu->SymWert.Typ == TempFloat) && (Neu->SymWert.Contents.FWert != (*Node)->SymWert.Contents.FWert))
                  || ((Neu->SymWert.Typ == TempInt) && (Neu->SymWert.Contents.IWert != (*Node)->SymWert.Contents.IWert))) {
                  if ((!Repass) && (JmpErrors > 0)) {
                     if (ThrowErrors) ErrorCount -= JmpErrors;
                     JmpErrors = 0;
                  }
                  Repass = true;
                  if ((MsgIfRepass) && (PassNo >= PassNoForMessage)) {
                     strmaxcpy(serr, Neu->SymName, 255);
                     if (Neu->Attribute != (-1)) {
                        strmaxcat(serr, "[", 255);
                        strmaxcat(serr, GetSectionName(Neu->Attribute), 255);
                        strmaxcat(serr, "]", 255);
                     }
                     WrXError(80, serr);
                  }
               }
            };
            Neu->Left = (*Node)->Left;
            Neu->Right = (*Node)->Right;
            Neu->Balance = (*Node)->Balance;
            if (DoCross) {
               Neu->LineNum = (*Node)->LineNum;
               Neu->FileNum = (*Node)->FileNum;
            }
            Neu->RefList = (*Node)->RefList;
            (*Node)->RefList = NULL;
            Neu->Defined = true;
            Neu->Used = (*Node)->Used;
            Neu->Changeable = MayChange;
            Hilf = (*Node);
            *Node = Neu;
            FreeSymbol(&Hilf);
         }
         break;
   }
   return Result;
}

static void EnterLocSymbol(SymbolPtr Neu) {
   Neu->Attribute = MomLocHandle;
   if (!CaseSensitive) NLS_UpString(Neu->SymName);
   EnterTreeNode(&FirstLocSymbol, Neu, false, false);
}

static void EnterSymbol_Search(PForwardSymbol * Lauf, PForwardSymbol * Prev, PForwardSymbol ** RRoot, SymbolPtr Neu, PForwardSymbol * Root, Byte ResCode, Byte * SearchErg) {
   *Lauf = (*Root);
   *Prev = NULL;
   *RRoot = Root;
   while ((*Lauf != NULL) && (strcmp((*Lauf)->Name, Neu->SymName) != 0)) {
      *Prev = (*Lauf);
      *Lauf = (*Lauf)->Next;
   }
   if (*Lauf != NULL) *SearchErg = ResCode;
}

static void EnterSymbol(SymbolPtr Neu, bool MayChange, LongInt ResHandle) {
   PForwardSymbol Lauf, Prev;
   PForwardSymbol *RRoot;
   Byte SearchErg;
   String CombName;
   PSaveSection RunSect;
   LongInt MSect;
   SymbolPtr Copy;

/*   Neu^.Attribute:=MomSectionHandle;
   IF SectionStack<>NULL THEN
    {
     Search(SectionStack^.GlobSyms);
     IF Lauf<>NULL THEN Neu^.Attribute:=Lauf^.DestSection
     ELSE Search(SectionStack^.LocSyms);
     IF Lauf<>NULL THEN
      {
       FreeMem(Lauf^.Name,Length(Lauf^.Name^)+1);
       IF Prev=NULL THEN RRoot^:=Lauf^.Next
       ELSE Prev^.Next:=Lauf^.Next;
       Dispose(Lauf);
      };
    };
   IF EnterTreeNode(FirstSymbol,Neu,MayChange,MakeCrossList) THEN;*/

   if (!CaseSensitive) NLS_UpString(Neu->SymName);

   SearchErg = 0;
   Neu->Attribute = (ResHandle == (-2)) ? (MomSectionHandle) : (ResHandle);
   if ((SectionStack != NULL) && (Neu->Attribute == MomSectionHandle)) {
      EnterSymbol_Search(&Lauf, &Prev, &RRoot, Neu, &(SectionStack->LocSyms), 1, &SearchErg);
      if (Lauf == NULL) EnterSymbol_Search(&Lauf, &Prev, &RRoot, Neu, &(SectionStack->GlobSyms), 2, &SearchErg);
      if (Lauf == NULL) EnterSymbol_Search(&Lauf, &Prev, &RRoot, Neu, &(SectionStack->ExportSyms), 3, &SearchErg);
      if (SearchErg == 2) Neu->Attribute = Lauf->DestSection;
      if (SearchErg == 3) {
         strmaxcpy(CombName, Neu->SymName, 255);
         RunSect = SectionStack;
         MSect = MomSectionHandle;
         while ((MSect != Lauf->DestSection) && (RunSect != NULL)) {
            strmaxprep(CombName, "_", 255);
            strmaxprep(CombName, GetSectionName(MSect), 255);
            MSect = RunSect->Handle;
            RunSect = RunSect->Next;
         }
         Copy = (SymbolPtr) malloc(sizeof(SymbolEntry));
         *Copy = (*Neu);
         Copy->SymName = strdup(CombName);
         Copy->Attribute = Lauf->DestSection;
         if (Copy->SymWert.Typ == TempString)
            Copy->SymWert.Contents.SWert = strdup(Neu->SymWert.Contents.SWert);
         EnterTreeNode(&FirstSymbol, Copy, MayChange, MakeCrossList);
      }
      if (Lauf != NULL) {
         free(Lauf->Name);
         if (Prev == NULL) *RRoot = Lauf->Next;
         else Prev->Next = Lauf->Next;
         free(Lauf);
      }
   }
   EnterTreeNode(&FirstSymbol, Neu, MayChange, MakeCrossList);
}

void PrintSymTree(char *Name) {
   fprintf(Debug, "---------------------\n");
   fprintf(Debug, "Enter Symbol %s\n\n", Name);
   PrintSymbolTree();
   PrintSymbolDepth();
}

void EnterIntSymbol(char *Name_O, LargeInt Wert, Byte Typ, bool MayChange) {
   SymbolPtr Neu;
   LongInt DestHandle;
   String Name;

   strmaxcpy(Name, Name_O, 255);
   if (!ExpandSymbol(Name)) return;
   if (!GetSymSection(Name, &DestHandle)) return;
   if (!ChkSymbName(Name)) {
      WrXError(1020, Name);
      return;
   }

   Neu = (SymbolPtr) malloc(sizeof(SymbolEntry));
   Neu->SymName = strdup(Name);
   Neu->SymWert.Typ = TempInt;
   Neu->SymWert.Contents.IWert = Wert;
   Neu->SymType = Typ;
   Neu->SymSize = (-1);

   if ((MomLocHandle == (-1)) || (DestHandle != (-2))) {
      EnterSymbol(Neu, MayChange, DestHandle);
      if (MakeDebug) PrintSymTree(Name);
   } else EnterLocSymbol(Neu);
}

void EnterFloatSymbol(char *Name_O, Double Wert, bool MayChange) {
   SymbolPtr Neu;
   LongInt DestHandle;
   String Name;

   strmaxcpy(Name, Name_O, 255);
   if (!ExpandSymbol(Name)) return;
   if (!GetSymSection(Name, &DestHandle)) return;
   if (!ChkSymbName(Name)) {
      WrXError(1020, Name);
      return;
   }
   Neu = (SymbolPtr) malloc(sizeof(SymbolEntry));
   Neu->SymName = strdup(Name);
   Neu->SymWert.Typ = TempFloat;
   Neu->SymWert.Contents.FWert = Wert;
   Neu->SymType = 0;
   Neu->SymSize = (-1);

   if ((MomLocHandle == (-1)) || (DestHandle != (-2))) {
      EnterSymbol(Neu, MayChange, DestHandle);
      if (MakeDebug) PrintSymTree(Name);
   } else EnterLocSymbol(Neu);
}

void EnterStringSymbol(char *Name_O, char *Wert, bool MayChange) {
   SymbolPtr Neu;
   LongInt DestHandle;
   String Name;

   strmaxcpy(Name, Name_O, 255);
   if (!ExpandSymbol(Name)) return;
   if (!GetSymSection(Name, &DestHandle)) return;
   if (!ChkSymbName(Name)) {
      WrXError(1020, Name);
      return;
   }
   Neu = (SymbolPtr) malloc(sizeof(SymbolEntry));
   Neu->SymName = strdup(Name);
   Neu->SymWert.Contents.SWert = strdup(Wert);
   Neu->SymWert.Typ = TempString;
   Neu->SymType = 0;
   Neu->SymSize = (-1);

   if ((MomLocHandle == (-1)) || (DestHandle != (-2))) {
      EnterSymbol(Neu, MayChange, DestHandle);
      if (MakeDebug) PrintSymTree(Name);
   } else EnterLocSymbol(Neu);
}

static void AddReference(SymbolPtr Node) {
   PCrossRef Lauf, Neu;

/* Speicher belegen */

   Neu = (PCrossRef) malloc(sizeof(TCrossRef));
   Neu->LineNum = CurrLine;
   Neu->OccNum = 1;
   Neu->Next = NULL;

/* passende Datei heraussuchen */

   Neu->FileNum = GetFileNum(CurrFileName);

/* suchen, ob Eintrag schon existiert */

   Lauf = Node->RefList;
   while ((Lauf != NULL)
      && ((Lauf->FileNum != Neu->FileNum) || (Lauf->LineNum != Neu->LineNum)))
      Lauf = Lauf->Next;

/* schon einmal in dieser Datei in dieser Zeile aufgetaucht: nur Zaehler
   rauf: */

   if (Lauf != NULL) {
      Lauf->OccNum++;
      free(Neu);
   }

/* ansonsten an Kettenende anhaengen */

   else if (Node->RefList == NULL) Node->RefList = Neu;

   else {
      Lauf = Node->RefList;
      while (Lauf->Next != NULL) Lauf = Lauf->Next;
      Lauf->Next = Neu;
   }
}

static bool FindNode_FNode(char *Name, TempType SearchType, SymbolPtr * FindNode_Result, LongInt Handle) {
   SymbolPtr Lauf = FirstSymbol;
   ShortInt SErg = (-1);
   bool Result = false;

   while ((Lauf != NULL) && (SErg != 0)) {
      SErg = StrCmp(Name, Lauf->SymName, Handle, Lauf->Attribute);
      if (SErg == (-1)) Lauf = Lauf->Left;
      else if (SErg == 1) Lauf = Lauf->Right;
   }
   if (Lauf != NULL)
      if (Lauf->SymWert.Typ == SearchType) {
         *FindNode_Result = Lauf;
         Result = true;
         if (MakeCrossList && DoRefs) AddReference(Lauf);
      }

   return Result;
}

static bool FindNode_FSpec(char *Name, PForwardSymbol Root) {
   while ((Root != NULL) && (strcmp(Root->Name, Name) != 0)) Root = Root->Next;
   return (Root != NULL);
}

static SymbolPtr FindNode(char *Name_O, TempType SearchType) {
   PSaveSection Lauf;
   LongInt DestSection;
   SymbolPtr FindNode_Result;
   String Name;

   strmaxcpy(Name, Name_O, 255);
   FindNode_Result = NULL;
   if (!GetSymSection(Name, &DestSection)) return FindNode_Result;
   if (!CaseSensitive) NLS_UpString(Name);
   if (SectionStack != NULL)
      if (PassNo <= MaxSymPass)
         if (FindNode_FSpec(Name, SectionStack->LocSyms)) DestSection = MomSectionHandle;
/* if (FSpec(SectionStack->GlobSyms)) return; */
   if (DestSection == (-2)) {
      if (FindNode_FNode(Name, SearchType, &FindNode_Result, MomSectionHandle)) return FindNode_Result;
      Lauf = SectionStack;
      while (Lauf != NULL) {
         if (FindNode_FNode(Name, SearchType, &FindNode_Result, Lauf->Handle)) return FindNode_Result;
         Lauf = Lauf->Next;
      }
   } else FindNode_FNode(Name, SearchType, &FindNode_Result, DestSection);

   return FindNode_Result;
}

static bool FindLocNode_FNode(char *Name, TempType SearchType, SymbolPtr * FindLocNode_Result, LongInt Handle) {
   SymbolPtr Lauf = FirstLocSymbol;
   ShortInt SErg = (-1);
   bool Result = false;

   while ((Lauf != NULL) && (SErg != 0)) {
      SErg = StrCmp(Name, Lauf->SymName, Handle, Lauf->Attribute);
      if (SErg == (-1)) Lauf = Lauf->Left;
      else if (SErg == 1) Lauf = Lauf->Right;
   }

   if (Lauf != NULL)
      if (Lauf->SymWert.Typ == SearchType) {
         *FindLocNode_Result = Lauf;
         Result = true;
      }

   return Result;
}

static SymbolPtr FindLocNode(char *Name_O, TempType SearchType) {
   PLocHandle RunLocHandle;
   SymbolPtr FindLocNode_Result;
   String Name;

   FindLocNode_Result = NULL;

   strmaxcpy(Name, Name_O, 255);
   if (!CaseSensitive) NLS_UpString(Name);

   if (MomLocHandle == (-1)) return FindLocNode_Result;

   if (FindLocNode_FNode(Name, SearchType, &FindLocNode_Result, MomLocHandle))
      return FindLocNode_Result;

   RunLocHandle = FirstLocHandle;
   while ((RunLocHandle != NULL) && (RunLocHandle->Cont != (-1))) {
      if (FindLocNode_FNode(Name, SearchType, &FindLocNode_Result, RunLocHandle->Cont))
         return FindLocNode_Result;
      RunLocHandle = RunLocHandle->Next;
   }

   return FindLocNode_Result;
}

/**
        void SetSymbolType(char *Name, Byte NTyp)
{
   Lauf:SymbolPtr;
   HRef:bool;

   IF ! ExpandSymbol(Name) THEN Exit;
   HRef:=DoRefs; DoRefs:=false;
   Lauf:=FindLocNode(Name,TempInt);
   IF Lauf=NULL THEN Lauf:=FindNode(Name,TempInt);
   IF Lauf<>NULL THEN Lauf^.SymType:=NTyp;
   DoRefs:=HRef;
}
**/

bool GetIntSymbol(char *Name, LargeInt * Wert) {
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName, Name, 255);
   if (!ExpandSymbol(NName)) return false;
   Lauf = FindLocNode(NName, TempInt);
   if (Lauf == NULL) Lauf = FindNode(NName, TempInt);
   if (Lauf != NULL) {
      *Wert = Lauf->SymWert.Contents.IWert;
      if (Lauf->SymType != 0) TypeFlag |= (1 << Lauf->SymType);
      if ((Lauf->SymSize != (-1)) && (SizeFlag != (-1))) SizeFlag = Lauf->SymSize;
      Lauf->Used = true;
   } else {
      if (PassNo > MaxSymPass) WrXError(1010, Name);
      *Wert = EProgCounter();
   }
   return (Lauf != NULL);
}

bool GetFloatSymbol(char *Name, Double * Wert) {
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName, Name, 255);
   if (!ExpandSymbol(NName)) return false;
   Lauf = FindLocNode(Name, TempFloat);
   if (Lauf == NULL) Lauf = FindNode(NName, TempFloat);
   if (Lauf != NULL) {
      *Wert = Lauf->SymWert.Contents.FWert;
      Lauf->Used = true;
   } else {
      if (PassNo > MaxSymPass) WrXError(1010, Name);
      *Wert = 0;
   }
   return (Lauf != NULL);
}

bool GetStringSymbol(char *Name, char *Wert) {
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName, Name, 255);
   if (!ExpandSymbol(NName)) return false;
   Lauf = FindLocNode(NName, TempString);
   if (Lauf == NULL) Lauf = FindNode(NName, TempString);
   if (Lauf != NULL) {
      strcpy(Wert, Lauf->SymWert.Contents.SWert);
      Lauf->Used = true;
   } else {
      if (PassNo > MaxSymPass) WrXError(1010, Name);
      *Wert = '\0';
   }
   return (Lauf != NULL);
}

void SetSymbolSize(char *Name, ShortInt Size) {
   SymbolPtr Lauf;
   bool HRef;
   String NName;

   strmaxcpy(NName, Name, 255);
   if (!ExpandSymbol(NName)) return;
   HRef = DoRefs;
   DoRefs = false;
   Lauf = FindLocNode(NName, TempInt);
   if (Lauf == NULL) Lauf = FindNode(Name, TempInt);
   if (Lauf != NULL) Lauf->SymSize = Size;
   DoRefs = HRef;
}

ShortInt GetSymbolSize(char *Name) {
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName, Name, 255);
   if (!ExpandSymbol(NName)) return -1;
   Lauf = FindLocNode(NName, TempInt);
   if (Lauf == NULL) Lauf = FindNode(NName, TempInt);
   return ((Lauf != NULL) ? Lauf->SymSize : -1);
}

bool IsSymbolFloat(char *Name) {
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName, Name, 255);
   if (!ExpandSymbol(NName)) return false;

   Lauf = FindLocNode(NName, TempFloat);
   if (Lauf == NULL) Lauf = FindNode(NName, TempFloat);
   return ((Lauf != NULL) && (Lauf->SymWert.Typ == TempFloat));
}

bool IsSymbolString(char *Name) {
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName, Name, 255);
   if (!ExpandSymbol(NName)) return false;

   Lauf = FindLocNode(NName, TempString);
   if (Lauf == NULL) Lauf = FindNode(NName, TempString);
   return ((Lauf != NULL) && (Lauf->SymWert.Typ == TempString));
}

bool IsSymbolDefined(char *Name) {
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName, Name, 255);
   if (!ExpandSymbol(NName)) return false;

   Lauf = FindLocNode(NName, TempInt);
   if (Lauf == NULL) Lauf = FindLocNode(NName, TempFloat);
   if (Lauf == NULL) Lauf = FindLocNode(NName, TempString);
   if (Lauf == NULL) Lauf = FindNode(NName, TempInt);
   if (Lauf == NULL) Lauf = FindNode(NName, TempFloat);
   if (Lauf == NULL) Lauf = FindNode(NName, TempString);
   return ((Lauf != NULL) && (Lauf->Defined));
}

bool IsSymbolUsed(char *Name) {
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName, Name, 255);
   if (!ExpandSymbol(NName)) return false;

   Lauf = FindLocNode(NName, TempInt);
   if (Lauf == NULL) Lauf = FindLocNode(NName, TempFloat);
   if (Lauf == NULL) Lauf = FindLocNode(NName, TempString);
   if (Lauf == NULL) Lauf = FindNode(NName, TempInt);
   if (Lauf == NULL) Lauf = FindNode(NName, TempFloat);
   if (Lauf == NULL) Lauf = FindNode(NName, TempString);
   return ((Lauf != NULL) && (Lauf->Used));
}

bool IsSymbolChangeable(char *Name) {
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName, Name, 255);
   if (!ExpandSymbol(NName)) return false;

   Lauf = FindLocNode(NName, TempInt);
   if (Lauf == NULL) Lauf = FindLocNode(NName, TempFloat);
   if (Lauf == NULL) Lauf = FindLocNode(NName, TempString);
   if (Lauf == NULL) Lauf = FindNode(NName, TempInt);
   if (Lauf == NULL) Lauf = FindNode(NName, TempFloat);
   if (Lauf == NULL) Lauf = FindNode(NName, TempString);
   return ((Lauf != NULL) && (Lauf->Changeable));
}

static void ConvertSymbolVal(SymbolVal * Inp, TempResult * Outp) {
   switch (Outp->Typ = Inp->Typ) {
      case TempInt:
         Outp->Contents.Int = Inp->Contents.IWert;
         break;
      case TempFloat:
         Outp->Contents.Float = Inp->Contents.FWert;
         break;
      case TempString:
         strmaxcpy(Outp->Contents.Ascii, Inp->Contents.SWert, 255);
         break;
      default:
         break;
   }
}

static void PrintSymbolList_AddOut(char *s, char *Zeilenrest, Integer Width) {
   if (strlen(s) + strlen(Zeilenrest) > Width) {
      Zeilenrest[strlen(Zeilenrest) - 1] = '\0';
      WrLstLine(Zeilenrest);
      strmaxcpy(Zeilenrest, s, 255);
   } else strmaxcat(Zeilenrest, s, 255);
}

static void PrintSymbolList_PNode(SymbolPtr Node, Integer Width, LongInt * Sum, LongInt * USum, char *Zeilenrest) {
   String s1, sh;
   int l1;
   TempResult t;

   ConvertSymbolVal(&(Node->SymWert), &t);
   StrSym(&t, false, s1);

   strmaxcpy(sh, Node->SymName, 255);
   if (Node->Attribute != (-1)) {
      strmaxcat(sh, " [", 255);
      strmaxcat(sh, GetSectionName(Node->Attribute), 255);
      strmaxcat(sh, "]", 255);
   }
   strmaxprep(sh, (Node->Used) ? " " : "*", 255);
   l1 = (strlen(s1) + strlen(sh) + 6) % 40;
   if (l1 < 38) strmaxprep(s1, Blanks(38 - l1), 255);
   strmaxprep(s1, " : ", 255);
   strmaxprep(s1, sh, 255);
   strmaxcat(s1, " ", 255);
   s1[l1 = strlen(s1)] = SegShorts[Node->SymType];
   s1[l1 + 1] = '\0';
   strmaxcat(s1, " | ", 255);
   PrintSymbolList_AddOut(s1, Zeilenrest, Width);
   (*Sum)++;
   if (!Node->Used) (*USum)++;
}

static void PrintSymbolList_PrintNode(SymbolPtr Node, Integer Width, LongInt * Sum, LongInt * USum, char *Zeilenrest) {
   ChkStack();

   if (Node == NULL) return;

   PrintSymbolList_PrintNode(Node->Left, Width, Sum, USum, Zeilenrest);
   PrintSymbolList_PNode(Node, Width, Sum, USum, Zeilenrest);
   PrintSymbolList_PrintNode(Node->Right, Width, Sum, USum, Zeilenrest);
}

void PrintSymbolList(void) {
   Integer Width;
   String Zeilenrest;
   LongInt Sum, USum;

   Width = (PageWidth == 0) ? 80 : PageWidth;
   NewPage(ChapDepth, true);
   WrLstLine(ListSymListHead1);
   WrLstLine(ListSymListHead2);
   WrLstLine("");

   Zeilenrest[0] = '\0';
   Sum = 0;
   USum = 0;
   PrintSymbolList_PrintNode(FirstSymbol, Width, &Sum, &USum, Zeilenrest);
   if (Zeilenrest[0] != '\0') {
      Zeilenrest[strlen(Zeilenrest) - 1] = '\0';
      WrLstLine(Zeilenrest);
   }
   WrLstLine("");
   sprintf(Zeilenrest, "%7d", Sum);
   strmaxcat(Zeilenrest, (Sum == 1) ? ListSymSumMsg : ListSymSumsMsg, 255);
   WrLstLine(Zeilenrest);
   sprintf(Zeilenrest, "%7d", USum);
   strmaxcat(Zeilenrest, (USum == 1) ? ListUSymSumMsg : ListUSymSumsMsg, 255);
   WrLstLine(Zeilenrest);
   WrLstLine("");
}

static bool HWritten;
static int Space;

static void PrintDebSymbols_PNode(FILE * f, SymbolPtr Node) {
   char *p;
   int l1;
   TempResult t;
   String s;

   if (!HWritten) {
      fprintf(f, "\n");
      ChkIO(10004);
      fprintf(f, "Symbols in Segment %s\n", SegNames[Space]);
      ChkIO(10004);
      HWritten = true;
   }

   fprintf(f, "%s", Node->SymName);
   ChkIO(10004);
   l1 = strlen(Node->SymName);
   if (Node->Attribute != (-1)) {
      sprintf(s, "[%d]", Node->Attribute);
      fprintf(f, "%s", s);
      ChkIO(10004);
      l1 += strlen(s);
   }
   fprintf(f, "%s ", Blanks(37 - l1));
   ChkIO(10004);
   switch (Node->SymWert.Typ) {
      case TempInt:
         fprintf(f, "Int    ");
         break;
      case TempFloat:
         fprintf(f, "Float  ");
         break;
      case TempString:
         fprintf(f, "String ");
         break;
      default:
         break;
   }
   ChkIO(10004);
   if (Node->SymWert.Typ == TempString) {
      l1 = 0;
      for (p = Node->SymWert.Contents.SWert; *p != '\0'; p++) {
         if ((*p == '\\') || (*p <= ' ')) {
            fprintf(f, "\\%03d", *p);
            l1 += 4;
         } else {
            fputc(*p, f);
            ChkIO(10004);
            l1++;
         }
      }
   } else {
      ConvertSymbolVal(&(Node->SymWert), &t);
      StrSym(&t, false, s);
      l1 = strlen(s);
      fprintf(f, "%s", s);
      ChkIO(10004);
   }
   fprintf(f, "%s %-3d %d\n", Blanks(25 - l1), Node->SymSize, Node->Used);
   ChkIO(10004);
}

static void PrintDebSymbols_PrintNode(FILE * f, SymbolPtr Node) {
   ChkStack();

   if (Node == NULL) return;

   PrintDebSymbols_PrintNode(f, Node->Left);

   if (Node->SymType == Space) PrintDebSymbols_PNode(f, Node);

   PrintDebSymbols_PrintNode(f, Node->Right);
}

void PrintDebSymbols(FILE * f) {
   for (Space = 0; Space < PCMax; Space++) {
      HWritten = false;
      PrintDebSymbols_PrintNode(f, FirstSymbol);
   }
}

static void PrintSymbolTree_PrintNode(SymbolPtr Node, Integer Shift) {
   Byte z;

   if (Node == NULL) return;

   PrintSymbolTree_PrintNode(Node->Left, Shift + 1);

   for (z = 1; z <= Shift; z++) fprintf(Debug, "%6s", "");
   fprintf(Debug, "%s\n", Node->SymName);

   PrintSymbolTree_PrintNode(Node->Right, Shift + 1);
}

void PrintSymbolTree(void) {
   PrintSymbolTree_PrintNode(FirstSymbol, 0);
}

static void ClearSymbolList_ClearNode(SymbolPtr * Node) {
   if ((*Node)->Left != NULL) ClearSymbolList_ClearNode(&((*Node)->Left));
   if ((*Node)->Right != NULL) ClearSymbolList_ClearNode(&((*Node)->Right));
   FreeSymbol(Node);
}

void ClearSymbolList(void) {

   if (FirstSymbol != NULL) ClearSymbolList_ClearNode(&FirstSymbol);

   if (FirstLocSymbol != NULL) ClearSymbolList_ClearNode(&FirstLocSymbol);
}

/*-------------------------------------------------------------------------*/
/* Stack-Verwaltung */

bool PushSymbol(char *SymName_O, char *StackName_O) {
   SymbolPtr Src;
   PSymbolStack LStack, NStack, PStack;
   PSymbolStackEntry Elem;
   String SymName, StackName;

   strmaxcpy(SymName, SymName_O, 255);
   if (!ExpandSymbol(SymName)) return false;

   Src = FindNode(SymName, TempInt);
   if (Src == NULL) Src = FindNode(SymName, TempFloat);
   if (Src == NULL) Src = FindNode(SymName, TempString);
   if (Src == NULL) {
      WrXError(1010, SymName);
      return false;
   }

   strmaxcpy(StackName, (*StackName_O == '\0') ? DefStackName : StackName_O, 255);
   if (!ExpandSymbol(StackName)) return false;
   if (!ChkSymbName(StackName)) {
      WrXError(1020, StackName);
      return false;
   }

   LStack = FirstStack;
   PStack = NULL;
   while ((LStack != NULL) && (strcmp(LStack->Name, StackName) < 0)) {
      PStack = LStack;
      LStack = LStack->Next;
   }

   if ((LStack == NULL) || (strcmp(LStack->Name, StackName) > 0)) {
      NStack = (PSymbolStack) malloc(sizeof(TSymbolStack));
      NStack->Name = strdup(StackName);
      NStack->Contents = NULL;
      NStack->Next = LStack;
      if (PStack == NULL) FirstStack = NStack;
      else PStack->Next = NStack;
      LStack = NStack;
   }

   Elem = (PSymbolStackEntry) malloc(sizeof(TSymbolStackEntry));
   Elem->Next = LStack->Contents;
   Elem->Contents = Src->SymWert;
   LStack->Contents = Elem;

   return true;
}

bool PopSymbol(char *SymName_O, char *StackName_O) {
   SymbolPtr Dest;
   PSymbolStack LStack, PStack;
   PSymbolStackEntry Elem;
   String SymName, StackName;

   strmaxcpy(SymName, SymName_O, 255);
   if (!ExpandSymbol(SymName)) return false;

   Dest = FindNode(SymName, TempInt);
   if (Dest == NULL) Dest = FindNode(SymName, TempFloat);
   if (Dest == NULL) Dest = FindNode(SymName, TempString);
   if (Dest == NULL) {
      WrXError(1010, SymName);
      return false;
   }

   strmaxcpy(StackName, (*StackName_O == '\0') ? DefStackName : StackName_O, 255);
   if (!ExpandSymbol(StackName)) return false;
   if (!ChkSymbName(StackName)) {
      WrXError(1020, StackName);
      return false;
   }

   LStack = FirstStack;
   PStack = NULL;
   while ((LStack != NULL) && (strcmp(LStack->Name, StackName) < 0)) {
      PStack = LStack;
      LStack = LStack->Next;
   }

   if ((LStack == NULL) || (strcmp(LStack->Name, StackName) > 0)) {
      WrXError(1530, StackName);
      return false;
   }

   Elem = LStack->Contents;
   Dest->SymWert = Elem->Contents;
   if ((LStack->Contents = Elem->Next) == NULL) {
      if (PStack == NULL) FirstStack = LStack->Next;
      else PStack->Next = LStack->Next;
      free(LStack->Name);
      free(LStack);
   }
   free(Elem);

   return true;
}

void ClearStacks(void) {
   PSymbolStack Act;
   PSymbolStackEntry Elem;
   Integer z;
   String s;

   while (FirstStack != NULL) {
      z = 0;
      Act = FirstStack;
      while (Act->Contents != NULL) {
         Elem = Act->Contents;
         Act->Contents = Elem->Next;
         free(Elem);
         z++;
      }
      sprintf(s, "%s(%d)", Act->Name, z);
      WrXError(230, s);
      free(Act->Name);
      FirstStack = Act->Next;
      free(Act);
   }
}

/*-------------------------------------------------------------------------*/
/* Funktionsverwaltung */

void EnterFunction(char *FName, char *FDefinition, Byte NewCnt) {
   PFunction Neu;
   String FName_N;

   if (!CaseSensitive) {
      strmaxcpy(FName_N, FName, 255);
      NLS_UpString(FName_N);
      FName = FName_N;
   }

   if (!ChkSymbName(FName)) {
      WrXError(1020, FName);
      return;
   }

   if (FindFunction(FName) != NULL) {
      if (PassNo == 1) WrXError(1000, FName);
      return;
   }

   Neu = (PFunction) malloc(sizeof(TFunction));
   Neu->Next = FirstFunction;
   Neu->ArguCnt = NewCnt;
   Neu->Name = strdup(FName);
   Neu->Definition = strdup(FDefinition);
   FirstFunction = Neu;
}

PFunction FindFunction(char *Name) {
   PFunction Lauf = FirstFunction;
   String Name_N;

   if (!CaseSensitive) {
      strmaxcpy(Name_N, Name, 255);
      NLS_UpString(Name_N);
      Name = Name_N;
   }

   while ((Lauf != NULL) && (strcmp(Lauf->Name, Name) != 0)) Lauf = Lauf->Next;
   return Lauf;
}

void PrintFunctionList(void) {
   PFunction Lauf;
   String OneS;
   bool cnt;

   if (FirstFunction == NULL) return;

   NewPage(ChapDepth, true);
   WrLstLine(ListFuncListHead1);
   WrLstLine(ListFuncListHead2);
   WrLstLine("");

   OneS[0] = '\0';
   Lauf = FirstFunction;
   cnt = false;
   while (Lauf != NULL) {
      strmaxcat(OneS, Lauf->Name, 255);
      if (strlen(Lauf->Name) < 37) strmaxcat(OneS, Blanks(37 - strlen(Lauf->Name)), 255);
      if (!cnt) strmaxcat(OneS, " | ", 255);
      else {
         WrLstLine(OneS);
         OneS[0] = '\0';
      }
      cnt = !cnt;
      Lauf = Lauf->Next;
   }
   if (cnt) {
      OneS[strlen(OneS) - 1] = '\0';
      WrLstLine(OneS);
   }
   WrLstLine("");
}

void ClearFunctionList(void) {
   PFunction Lauf;

   while (FirstFunction != NULL) {
      Lauf = FirstFunction->Next;
      free(FirstFunction->Name);
      free(FirstFunction->Definition);
      free(FirstFunction);
      FirstFunction = Lauf;
   }
}

/*-------------------------------------------------------------------------*/

static void ResetSymbolDefines_ResetNode(SymbolPtr Node) {
   if (Node->Left != NULL) ResetSymbolDefines_ResetNode(Node->Left);
   if (Node->Right != NULL) ResetSymbolDefines_ResetNode(Node->Right);
   Node->Defined = false;
   Node->Used = false;
}

void ResetSymbolDefines(void) {

   if (FirstSymbol != NULL) ResetSymbolDefines_ResetNode(FirstSymbol);

   if (FirstLocSymbol != NULL) ResetSymbolDefines_ResetNode(FirstLocSymbol);
}

void SetFlag(bool *Flag, char *Name, bool Wert) {
   *Flag = Wert;
   EnterIntSymbol(Name, (*Flag) ? 1 : 0, 0, true);
}

void AddDefSymbol(char *Name, TempResult * Value) {
   PDefSymbol Neu;

   Neu = FirstDefSymbol;
   while (Neu != NULL) {
      if (strcmp(Neu->SymName, Name) == 0) return;
      Neu = Neu->Next;
   }

   Neu = (PDefSymbol) malloc(sizeof(TDefSymbol));
   Neu->Next = FirstDefSymbol;
   Neu->SymName = strdup(Name);
   Neu->Wert = (*Value);
   FirstDefSymbol = Neu;
}

void RemoveDefSymbol(char *Name) {
   PDefSymbol Save, Lauf;

   if (FirstDefSymbol == NULL) return;

   if (strcmp(FirstDefSymbol->SymName, Name) == 0) {
      Save = FirstDefSymbol;
      FirstDefSymbol = FirstDefSymbol->Next;
   } else {
      Lauf = FirstDefSymbol;
      while ((Lauf->Next != NULL) && (strcmp(Lauf->Next->SymName, Name) != 0)) Lauf = Lauf->Next;
      if (Lauf->Next == NULL) return;
      Save = Lauf->Next;
      Lauf->Next = Lauf->Next->Next;
   }
   free(Save->SymName);
   free(Save);
}

void CopyDefSymbols(void) {
   PDefSymbol Lauf;

   Lauf = FirstDefSymbol;
   while (Lauf != NULL) {
      switch (Lauf->Wert.Typ) {
         case TempInt:
            EnterIntSymbol(Lauf->SymName, Lauf->Wert.Contents.Int, 0, true);
            break;
         case TempFloat:
            EnterFloatSymbol(Lauf->SymName, Lauf->Wert.Contents.Float, true);
            break;
         case TempString:
            EnterStringSymbol(Lauf->SymName, Lauf->Wert.Contents.Ascii, true);
            break;
         default:
            break;
      }
      Lauf = Lauf->Next;
   }
}

static void PrintSymbolDepth_SearchTree(SymbolPtr Lauf, LongInt SoFar, LongInt * TreeMin, LongInt * TreeMax) {
   if (Lauf == NULL) {
      if (SoFar > *TreeMax) *TreeMax = SoFar;
      if (SoFar < *TreeMin) *TreeMin = SoFar;
   } else {
      PrintSymbolDepth_SearchTree(Lauf->Right, SoFar + 1, TreeMin, TreeMax);
      PrintSymbolDepth_SearchTree(Lauf->Left, SoFar + 1, TreeMin, TreeMax);
   }
}

void PrintSymbolDepth(void) {
   LongInt TreeMin, TreeMax;

   TreeMin = MaxLongInt;
   TreeMax = 0;
   PrintSymbolDepth_SearchTree(FirstSymbol, 0, &TreeMin, &TreeMax);
   fprintf(Debug, " MinTree %d\n", TreeMin);
   fprintf(Debug, " MaxTree %d\n", TreeMax);
}

LongInt GetSectionHandle(char *SName_O, bool AddEmpt, LongInt Parent) {
   PCToken Lauf, Prev;
   LongInt z;
   String SName;

   strmaxcpy(SName, SName_O, 255);
   if (!CaseSensitive) NLS_UpString(SName);

   Lauf = FirstSection;
   Prev = NULL;
   z = 0;
   while ((Lauf != NULL) && ((strcmp(Lauf->Name, SName) != 0) || (Lauf->Parent != Parent))) {
      z++;
      Prev = Lauf;
      Lauf = Lauf->Next;
   }

   if (Lauf == NULL)
      if (AddEmpt) {
         Lauf = (PCToken) malloc(sizeof(TCToken));
         Lauf->Parent = MomSectionHandle;
         Lauf->Name = strdup(SName);
         Lauf->Next = NULL;
         InitChunk(&(Lauf->Usage));
         if (Prev == NULL) FirstSection = Lauf;
         else Prev->Next = Lauf;
      } else z = (-2);
   return z;
}

char *GetSectionName(LongInt Handle) {
   PCToken Lauf = FirstSection;
   static char *Dummy = "";

   if (Handle == (-1)) return Dummy;
   while ((Handle > 0) && (Lauf != NULL)) {
      Lauf = Lauf->Next;
      Handle--;
   }
   return (Lauf == NULL) ? Dummy : Lauf->Name;
}

void SetMomSection(LongInt Handle) {
   LongInt z;

   MomSectionHandle = Handle;
   if (Handle < 0) MomSection = NULL;
   else {
      MomSection = FirstSection;
      for (z = 1; z <= Handle; z++)
         if (MomSection != NULL) MomSection = MomSection->Next;
   }
}

void AddSectionUsage(LongInt Start, LongInt Length) {
   if ((ActPC != SegCode) || (MomSection == NULL)) return;
   AddChunk(&(MomSection->Usage), Start, Length, false);
}

static void PrintSectionList_PSection(LongInt Handle, Integer Indent) {
   PCToken Lauf;
   LongInt Cnt;
   String h;

   ChkStack();
   if (Handle != (-1)) {
      strmaxcpy(h, Blanks(Indent << 1), 255);
      strmaxcat(h, GetSectionName(Handle), 255);
      WrLstLine(h);
   }
   Lauf = FirstSection;
   Cnt = 0;
   while (Lauf != NULL) {
      if (Lauf->Parent == Handle) PrintSectionList_PSection(Cnt, Indent + 1);
      Lauf = Lauf->Next;
      Cnt++;
   }
}

void PrintSectionList(void) {
   if (FirstSection == NULL) return;

   NewPage(ChapDepth, true);
   WrLstLine(ListSectionListHead1);
   WrLstLine(ListSectionListHead2);
   WrLstLine("");
   PrintSectionList_PSection(-1, 0);
}

void PrintDebSections(FILE * f) {
   PCToken Lauf;
   LongInt Cnt, z, l, s;

   Lauf = FirstSection;
   Cnt = 0;
   while (Lauf != NULL) {
      fprintf(f, "\n");
      ChkIO(10004);
      fprintf(f, "Info for Section %d %s %d\n", Cnt, GetSectionName(Cnt), Lauf->Parent);
      ChkIO(10004);
      for (z = 0; z < Lauf->Usage.RealLen; z++) {
         l = Lauf->Usage.Chunks[z].Length;
         s = Lauf->Usage.Chunks[z].Start;
         fprintf(f, "%s", HexString(s, 0));
         ChkIO(10004);
         if (l == 1) fprintf(f, "\n");
         else fprintf(f, "-%s\n", HexString(s + l - 1, 0));
         ChkIO(10004);
      }
      Lauf = Lauf->Next;
      Cnt++;
   }
}

void ClearSectionList(void) {
   PCToken Tmp;

   while (FirstSection != NULL) {
      Tmp = FirstSection;
      free(Tmp->Name);
      ClearChunk(&(Tmp->Usage));
      FirstSection = Tmp->Next;
      free(Tmp);
   }
}

/*---------------------------------------------------------------------------------*/

static void PrintCrossList_PNode(SymbolPtr Node) {
   Integer FileZ;
   PCrossRef Lauf;
   String LinePart, LineAcc;
   String h, h2;
   TempResult t;

   if (Node->RefList == NULL) return;

   ConvertSymbolVal(&(Node->SymWert), &t);
   strcpy(h, " (=");
   StrSym(&t, false, h2);
   strmaxcat(h, h2, 255);
   strmaxcat(h, ",", 255);
   strmaxcat(h, GetFileName(Node->FileNum), 255);
   strmaxcat(h, ":", 255);
   sprintf(h2, "%d", Node->LineNum);
   strmaxcat(h, h2, 255);
   strmaxcat(h, "):", 255);
   if (Node->Attribute != (-1)) {
      strmaxprep(h, "] ", 255);
      strmaxprep(h, GetSectionName(Node->Attribute), 255);
      strmaxprep(h, " [", 255);
   }

   strmaxprep(h, Node->SymName, 255);
   strmaxprep(h, ListCrossSymName, 255);
   WrLstLine(h);

   for (FileZ = 0; FileZ < GetFileCount(); FileZ++) {
      Lauf = Node->RefList;

      while ((Lauf != NULL) && (Lauf->FileNum != FileZ)) Lauf = Lauf->Next;

      if (Lauf != NULL) {
         strcpy(h, " ");
         strmaxcat(h, ListCrossFileName, 255);
         strmaxcat(h, GetFileName(FileZ), 255);
         strmaxcat(h, " :", 255);
         WrLstLine(h);
         strcpy(LineAcc, "   ");
         while (Lauf != NULL) {
            sprintf(LinePart, "%5d", Lauf->LineNum);
            strmaxcat(LineAcc, LinePart, 255);
            if (Lauf->OccNum != 1) {
               sprintf(LinePart, "(%2d)", Lauf->OccNum);
               strmaxcat(LineAcc, LinePart, 255);
            } else strmaxcat(LineAcc, "    ", 255);
            if (strlen(LineAcc) >= 72) {
               WrLstLine(LineAcc);
               strcpy(LineAcc, "  ");
            }
            Lauf = Lauf->Next;
         }
         if (strcmp(LineAcc, "  ") != 0) WrLstLine(LineAcc);
      }
   }
   WrLstLine("");
}

static void PrintCrossList_PrintNode(SymbolPtr Node) {
   if (Node == NULL) return;

   PrintCrossList_PrintNode(Node->Left);

   PrintCrossList_PNode(Node);

   PrintCrossList_PrintNode(Node->Right);
}

void PrintCrossList(void) {

   WrLstLine("");
   WrLstLine(ListCrossListHead1);
   WrLstLine(ListCrossListHead2);
   WrLstLine("");
   PrintCrossList_PrintNode(FirstSymbol);
   WrLstLine("");
}

static void ClearCrossList_CNode(SymbolPtr Node) {
   PCrossRef Lauf;

   if (Node->Left != NULL) ClearCrossList_CNode(Node->Left);

   if (Node != NULL)
      while (Node->RefList != NULL) {
         Lauf = Node->RefList->Next;
         free(Node->RefList);
         Node->RefList = Lauf;
      }

   if (Node->Right != NULL) ClearCrossList_CNode(Node->Right);
}

void ClearCrossList(void) {
   ClearCrossList_CNode(FirstSymbol);
}

LongInt GetLocHandle(void) {
   return LocHandleCnt++;
}

void PushLocHandle(LongInt NewLoc) {
   PLocHandle NewLocHandle;

   NewLocHandle = (PLocHandle) malloc(sizeof(TLocHeap));
   NewLocHandle->Cont = MomLocHandle;
   NewLocHandle->Next = FirstLocHandle;
   FirstLocHandle = NewLocHandle;
   MomLocHandle = NewLoc;
}

void PopLocHandle(void) {
   PLocHandle OldLocHandle;

   OldLocHandle = FirstLocHandle;
   if (OldLocHandle == NULL) return;
   MomLocHandle = OldLocHandle->Cont;
   FirstLocHandle = OldLocHandle->Next;
   free(OldLocHandle);
}

void ClearLocStack() {
   while (MomLocHandle != (-1)) PopLocHandle();
}

void asmpars_init(void) {
   FirstDefSymbol = NULL;
   FirstFunction = NULL;
   BalanceTree = false;
   IntMins[Int32]--;
   IntMins[SInt32]--;
#ifdef HAS64
   IntMins[Int64]--;
#endif
}
