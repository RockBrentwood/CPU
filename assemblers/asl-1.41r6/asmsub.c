/* asmsub.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterfunktionen, vermischtes                                              */
/*                                                                           */
/* Historie:  4. 5. 1996  Grundsteinlegung                                   */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "endian.h"
#include "stdhandl.h"
#include "nls.h"
#include "stringutil.h"
#include "stringlists.h"
#include "chunks.h"
#include "ioerrors.h"
#include "asmdef.h"

#include "asmsub.h"

Word ErrorCount, WarnCount;
static StringList CopyrightList, OutList;

static LongWord StartStack, MinStack, LowStack;

#define ERRMSG
#include "as.rsc"
#include "ioerrors.rsc"

/****************************************************************************/
/* Modulinitialisierung */

void AsmSubInit(void) {
   PageLength = 60;
   PageWidth = 0;
   ErrorCount = 0;
   WarnCount = 0;
}

/****************************************************************************/
/* neuen Prozessor definieren */

CPUVar AddCPU(char *NewName, TSwitchProc Switcher) {
   PCPUDef Lauf, Neu;
   char *p;

   Neu = (PCPUDef) malloc(sizeof(TCPUDef));
   Neu->Name = strdup(NewName);
/* kein UpString, weil noch nicht initialisiert ! */
   for (p = Neu->Name; *p != '\0'; p++) *p = toupper(*p);
   Neu->SwitchProc = Switcher;
   Neu->Next = NULL;
   Neu->Number = Neu->Orig = CPUCnt;

   Lauf = FirstCPUDef;
   if (Lauf == NULL) FirstCPUDef = Neu;
   else {
      while (Lauf->Next != NULL) Lauf = Lauf->Next;
      Lauf->Next = Neu;
   }

   return CPUCnt++;
}

bool AddCPUAlias(char *OrigName, char *AliasName) {
   PCPUDef Lauf = FirstCPUDef, Neu;

   while ((Lauf != NULL) && (strcmp(Lauf->Name, OrigName) != 0)) Lauf = Lauf->Next;

   if (Lauf == NULL) return false;
   else {
      Neu = (PCPUDef) malloc(sizeof(TCPUDef));
      Neu->Next = NULL;
      Neu->Name = strdup(AliasName);
      Neu->Number = CPUCnt++;
      Neu->Orig = Lauf->Orig;
      Neu->SwitchProc = Lauf->SwitchProc;
      while (Lauf->Next != NULL) Lauf = Lauf->Next;
      Lauf->Next = Neu;
      return true;
   }
}

void PrintCPUList(TSwitchProc NxtProc) {
   PCPUDef Lauf;
   TSwitchProc Proc;

   Lauf = FirstCPUDef;
   Proc = NullProc;
   while (Lauf != NULL) {
      if (Lauf->Number == Lauf->Orig) {
         if (Lauf->SwitchProc != Proc) {
            Proc = Lauf->SwitchProc;
            printf("\n");
            NxtProc();
         }
         printf("%-10s", Lauf->Name);
      }
      Lauf = Lauf->Next;
   }
   printf("\n");
   NxtProc();
}

void ClearCPUList(void) {
   PCPUDef Save;

   while (FirstCPUDef != NULL) {
      Save = FirstCPUDef;
      FirstCPUDef = Save->Next;
      free(Save->Name);
      free(Save);
   }
}

/****************************************************************************/
/* Copyrightlistenverwaltung */

void AddCopyright(char *NewLine) {
   AddStringListLast(&CopyrightList, NewLine);
}

void WriteCopyrights(TSwitchProc NxtProc) {
   StringRecPtr Lauf;

   if (!StringListEmpty(CopyrightList)) {
      printf("%s\n", GetStringListFirst(CopyrightList, &Lauf));
      NxtProc();
      while (Lauf != NULL) {
         printf("%s\n", GetStringListNext(&Lauf));
         NxtProc();
      }
   }
}

/*--------------------------------------------------------------------------*/
/* ermittelt das erste/letzte Auftauchen eines Zeichens ausserhalb */
/* "geschuetzten" Bereichen */

char *QuotPos(char *s, char Zeichen) {
   register ShortInt Brack = 0, AngBrack = 0;
   char *i;
   register LongWord Flag = 0;

   for (i = s; *i != '\0'; i++)
      if (*i == Zeichen) {
         if ((AngBrack | Brack | Flag) == 0) return i;
      } else switch (*i) {
            case '"':
               if ((Brack == 0) && (AngBrack == 0) && ((Flag & 2) == 0)) Flag ^= 1;
               break;
            case '\'':
               if ((Brack == 0) && (AngBrack == 0) && ((Flag & 1) == 0)) Flag ^= 2;
               break;
            case '(':
               if ((AngBrack | Flag) == 0) Brack++;
               break;
            case ')':
               if ((AngBrack | Flag) == 0) Brack--;
               break;
            case '[':
               if ((Brack | Flag) == 0) AngBrack++;
               break;
            case ']':
               if ((Brack | Flag) == 0) AngBrack--;
               break;
      }

   return NULL;
}

char *RQuotPos(char *s, char Zeichen) {
   ShortInt Brack = 0, AngBrack = 0;
   char *i;
   bool Quot = false, Paren = false;

   for (i = s + strlen(s) - 1; i >= s; i--)
      if (*i == Zeichen) {
         if ((AngBrack == 0) && (Brack == 0) && (!Paren) && (!Quot)) return i;
      } else switch (*i) {
            case '"':
               if ((Brack == 0) && (AngBrack == 0) && (!Quot)) Paren = !Paren;
               break;
            case '\'':
               if ((Brack == 0) && (AngBrack == 0) && (!Paren)) Quot = !Quot;
               break;
            case ')':
               if ((AngBrack == 0) && (!Paren) && (!Quot)) Brack++;
               break;
            case '(':
               if ((AngBrack == 0) && (!Paren) && (!Quot)) Brack--;
               break;
            case ']':
               if ((Brack == 0) && (!Paren) && (!Quot)) AngBrack++;
               break;
            case '[':
               if ((Brack == 0) && (!Paren) && (!Quot)) AngBrack--;
               break;
      }

   return NULL;
}

/*--------------------------------------------------------------------------*/
/* ermittelt das erste Leerzeichen in einem String */

char *FirstBlank(char *s) {
   char *h, *Min = NULL;

   h = strchr(s, ' ');
      if (h != NULL) if ((Min == NULL) || (h < Min)) Min = h;
   h = strchr(s, Char_HT);
      if (h != NULL) if ((Min == NULL) || (h < Min)) Min = h;
   return Min;
}

/*--------------------------------------------------------------------------*/
/* einen String in zwei Teile zerlegen */

void SplitString(char *Source, char *Left, char *Right, char *Trenner) {
   char Save;
   LongInt slen = strlen(Source);

   if ((Trenner == NULL) || (Trenner >= Source + slen))
      Trenner = Source + slen;
   Save = (*Trenner);
   *Trenner = '\0';
   strcpy(Left, Source);
   *Trenner = Save;
   if (Trenner >= Source + slen) *Right = '\0';
   else strcpy(Right, Trenner + 1);
}

/*--------------------------------------------------------------------------*/
/* verbesserte Grossbuchstabenfunktion */

/* einen String in Grossbuchstaben umwandeln.  Dabei Stringkonstanten in Ruhe */
/* lassen */

void UpString(char *s) {
   char *z;
   Integer hyp = 0, quot = 0;

   for (z = s; *z != '\0'; z++) {
      if ((*z == '\'') && ((quot & 1) == 0)) hyp ^= 1;
      else if ((*z == '"') && ((hyp & 1) == 0)) quot ^= 1;
      else if ((quot | hyp) == 0) *z = UpCaseTable[(int)*z];
   }
}

/*--------------------------------------------------------------------------*/
/* alle Leerzeichen aus einem String loeschen */

void KillBlanks(char *s) {
   char *z;
   Integer dest = 0;
   bool InHyp = false, InQuot = false;

   for (z = s; *z != '\0'; z++) {
      switch (*z) {
         case '\'':
            if (!InQuot) InHyp = !InHyp;
            break;
         case '"':
            if (!InHyp) InQuot = !InQuot;
            break;
      }
      if ((!isspace(*z)) || (InHyp) || (InQuot)) s[dest++] = (*z);
   }
   s[dest] = '\0';
}

/*--------------------------------------------------------------------------*/
/* fuehrende Leerzeichen loeschen */

void KillPrefBlanks(char *s) {
   char *z = s;

   while ((*z != '\0') && (isspace(*z))) z++;
   if (z != s) strcpy(s, z);
}

/*--------------------------------------------------------------------------*/
/* anhaengende Leerzeichen loeschen */

void KillPostBlanks(char *s) {
   char *z = s + strlen(s) - 1;

   while ((z >= s) && (isspace(*z))) *(z--) = '\0';
}

/****************************************************************************/

void TranslateString(char *s) {
   char *z;

   for (z = s; *z != '\0'; z++) *z = CharTransTable[((unsigned int)(*z)) & 0xff];
}

ShortInt StrCmp(char *s1, char *s2, LongInt Hand1, LongInt Hand2) {
   ShortInt tmp;

   tmp = strcmp(s1, s2);
   if (tmp < 0) return -1;
   else if (tmp > 0) return 1;
   else if (Hand1 < Hand2) return -1;
   else if (Hand1 > Hand2) return 1;
   else return 0;
}

/****************************************************************************/
/* an einen Dateinamen eine Endung anhaengen */

void AddSuffix(char *s, char *Suff) {
   char *p, *z, *Part;

   p = NULL;
      for (z = s; *z != '\0'; z++) if (*z == '\\') p = z;
   Part = (p != NULL) ? (p) : (s);
   if (strchr(Part, '.') == NULL) strmaxcat(s, Suff, 255);
}

/*--------------------------------------------------------------------------*/
/* von einem Dateinamen die Endung loeschen */

void KillSuffix(char *s) {
   char *p, *z, *Part;

   p = NULL;
      for (z = s; *z != '\0'; z++) if (*z == '\\') p = z;
   Part = (p != NULL) ? (p) : (s);
   Part = strchr(Part, '.');
   if (Part != NULL) *Part = '\0';
}

/*--------------------------------------------------------------------------*/
/* Pfadanteil (Laufwerk+Verzeichnis) von einem Dateinamen abspalten */

char *PathPart(char *Name) {
   static String s;
   char *p;

   strmaxcpy(s, Name, 255);

   p = strrchr(Name, PATHSEP);
#ifdef DRSEP
   if (p == NULL) p = strrchr(Name, DRSEP);
#endif

   if (p == NULL) *s = '\0';
   else s[1] = '\0';

   return s;
}

/*--------------------------------------------------------------------------*/
/* Namensanteil von einem Dateinamen abspalten */

char *NamePart(char *Name) {
   char *p = strrchr(Name, PATHSEP);

#ifdef DRSEP
   if (p == NULL) p = strrchr(Name, DRSEP);
#endif

   return (p == NULL) ? (Name) : (p + 1);
}

/****************************************************************************/
/* eine Gleitkommazahl in einen String umwandeln */

char *FloatString(Double f) {
#define MaxLen 18
   static String s;
   char *p, *d;
   Integer n, ExpVal, nzeroes;
   bool WithE, OK;

/* 1. mit Maximallaenge wandeln, fuehrendes Vorzeichen weg */

   sprintf(s, "%27.15e", f);
   while ((s[0] == ' ') || (s[0] == '+')) strcpy(s, s + 1);

/* 2. Exponenten soweit als moeglich kuerzen, evtl. ganz streichen */

   p = strchr(s, 'e');
   switch (*(++p)) {
      case '+':
         strcpy(p, p + 1);
         break;
      case '-':
         p++;
         break;
   }

   while (*p == '0') strcpy(p, p + 1);
   WithE = (*p != '\0');
   if (!WithE) s[strlen(s) - 1] = '\0';

/* 3. Nullen am Ende der Mantisse entfernen, Komma bleibt noch */

   if (WithE) p = strchr(s, 'e');
   else p = s + strlen(s);
   p--;
   while (*p == '0') {
      strcpy(p, p + 1);
      p--;
   }

/* 4. auf die gewuenschte Maximalstellenzahl begrenzen */

   if (WithE) p = strchr(s, 'e');
   else p = s + strlen(s);
   d = strchr(s, '.');
   n = p - d - 1;

/* 5. Maximallaenge ueberschritten ? */

   if (strlen(s) > MaxLen) strcpy(d + (n - (strlen(s) - MaxLen)), d + n);

/* 6. Exponentenwert berechnen */

   if (WithE) {
      p = strchr(s, 'e');
      ExpVal = ConstLongInt(p + 1, &OK);
   } else {
      p = s + strlen(s);
      ExpVal = 0;
   }

/* 7. soviel Platz, dass wir den Exponenten weglassen und evtl. Nullen
   anhaengen koennen ? */

   if (ExpVal > 0) {
      nzeroes = ExpVal - (p - strchr(s, '.') - 1); /* = Zahl von Nullen, die anzuhaengen waere */

   /* 7a. nur Kommaverschiebung erforderlich. Exponenten loeschen und
      evtl. auch Komma */

      if (nzeroes <= 0) {
         *p = '\0';
         d = strchr(s, '.');
         strcpy(d, d + 1);
         if (nzeroes != 0) {
            memmove(s + strlen(s) + nzeroes + 1, s + strlen(s) + nzeroes, -nzeroes);
            s[strlen(s) - 1 + nzeroes] = '.';
         }
      }

   /* 7b. Es muessen Nullen angehaengt werden. Schauen, ob nach Loeschen von
      Punkt und E-Teil genuegend Platz ist */

      else {
         n = strlen(p) + 1 + (MaxLen - strlen(s)); /* = Anzahl freizubekommender Zeichen+Gutschrift */
         if (n >= nzeroes) {
            *p = '\0';
            d = strchr(s, '.');
            strcpy(d, d + 1);
            d = s + strlen(s);
            for (n = 0; n < nzeroes; n++) *(d++) = '0';
            *d = '\0';
         }
      }
   }

/* 8. soviel Platz, dass Exponent wegkann und die Zahl mit vielen Nullen
   vorne geschrieben werden kann ? */

   else if (ExpVal < 0) {
      n = (-ExpVal) - (strlen(p)); /* = Verlaengerung nach Operation */
      if (strlen(s) + n <= MaxLen) {
         *p = '\0';
         d = strchr(s, '.');
         strcpy(d, d + 1);
         if (s[0] == '-') d = s + 1;
         else d = s;
         memmove(d - ExpVal + 1, d, strlen(s) + 1);
         *(d++) = '0';
         *(d++) = '.';
         for (n = 0; n < -ExpVal - 1; n++) *(d++) = '0';
      }
   }

/* 9. Ueberfluessiges Komma entfernen */

   if (WithE) {
      p = strchr(s, 'e');
      if (p != NULL) *p = 'E';
   } else p = s + strlen(s);
   if ((p != NULL) && (*(p - 1) == '.')) strcpy(p - 1, p);

   return s;
}

/****************************************************************************/
/* Symbol in String wandeln */

void StrSym(TempResult * t, bool WithSystem, char *Dest) {
   switch (t->Typ) {
      case TempInt:
         strcpy(Dest, HexString(t->Contents.Int, 1));
         if (WithSystem)
            switch (ConstMode) {
               case ConstModeIntel:
                  strcat(Dest, "H");
                  break;
               case ConstModeMoto:
                  strprep(Dest, "$");
                  break;
               case ConstModeC:
                  strprep(Dest, "0x");
                  break;
            }
         break;
      case TempFloat:
         strcpy(Dest, FloatString(t->Contents.Float));
         break;
      case TempString:
         strcpy(Dest, t->Contents.Ascii);
         break;
      default:
         strcpy(Dest, "???");
   }
}

/****************************************************************************/
/* Listingzaehler zuruecksetzen */

void ResetPageCounter(void) {
   Integer z;

   for (z = 0; z <= ChapMax; z++) PageCounter[z] = 0;
   LstCounter = 0;
   ChapDepth = 0;
}

/*--------------------------------------------------------------------------*/
/* eine neue Seite im Listing beginnen */

void NewPage(ShortInt Level, bool WithFF) {
   ShortInt z;
   String Header, s;
   char Save;

   if (ListOn == 0) return;

   LstCounter = 0;

   if (ChapDepth < (Byte) Level) {
      memmove(PageCounter + (Level - ChapDepth), PageCounter, (ChapDepth + 1) * sizeof(Word));
      for (z = 0; z <= Level - ChapDepth; PageCounter[z++] = 1);
      ChapDepth = Level;
   }
   for (z = 0; z <= Level - 1; PageCounter[z++] = 1);
   PageCounter[Level]++;

   if (WithFF) {
      errno = 0;
      fprintf(LstFile, "%c", Char_FF);
      ChkIO(10002);
   }

   sprintf(Header, " AS V%s%s%s", Version, HeadingFileNameLab, NamePart(SourceFile));
   if ((strcmp(CurrFileName, "INTERNAL") != 0) && (strcmp(NamePart(CurrFileName), NamePart(SourceFile)) != 0)) {
      strmaxcat(Header, "(", 255);
      strmaxcat(Header, NamePart(CurrFileName), 255);
      strmaxcat(Header, ")", 255);
   }
   strmaxcat(Header, HeadingPageLab, 255);

   for (z = ChapDepth; z >= 0; z--) {
      sprintf(s, "%d", PageCounter[z]);
      strmaxcat(Header, s, 255);
      if (z != 0) strmaxcat(Header, ".", 255);
   }

   strmaxcat(Header, " - ", 255);
   NLS_CurrDateString(s);
   strmaxcat(Header, s, 255);
   strmaxcat(Header, " ", 255);
   NLS_CurrTimeString(false, s);
   strmaxcat(Header, s, 255);

   if (PageWidth != 0)
      while (strlen(Header) > PageWidth) {
         Save = Header[PageWidth];
         Header[PageWidth] = '\0';
         errno = 0;
         fprintf(LstFile, "%s\n", Header);
         ChkIO(10002);
         Header[PageWidth] = Save;
         strmove(Header, PageWidth);
      // strcpy(Header, Header + PageWidth); //(@) Formerly
      }
   errno = 0;
   fprintf(LstFile, "%s\n", Header);
   ChkIO(10002);

   if (PrtTitleString[0] != '\0') {
      errno = 0;
      fprintf(LstFile, "%s\n", PrtTitleString);
      ChkIO(10002);
   }

   errno = 0;
   fprintf(LstFile, "\n\n");
   ChkIO(10002);
}

/*--------------------------------------------------------------------------*/
/* eine Zeile ins Listing schieben */

void WrLstLine(char *Line) {
   Integer LLength;
   char bbuf[2500];
   String LLine;
   Integer blen = 0, hlen, z, Start;

   if (ListOn == 0) return;

   if (PageLength == 0) {
      errno = 0;
      fprintf(LstFile, "%s\n", Line);
      ChkIO(10002);
   } else {
      if ((PageWidth == 0) || ((strlen(Line) << 3) < PageWidth)) LLength = 1;
      else {
         blen = 0;
         for (z = 0; z < strlen(Line); z++)
            if (Line[z] == Char_HT) {
               memset(bbuf + blen, 8 - (blen & 7), ' ');
               blen += 8 - (blen & 7);
            } else bbuf[blen++] = Line[z];
         LLength = blen / PageWidth;
         if (blen % PageWidth != 0) LLength++;
      }
      if (LLength == 1) {
         errno = 0;
         fprintf(LstFile, "%s\n", Line);
         ChkIO(10002);
         if ((++LstCounter) == PageLength) NewPage(0, true);
      } else {
         Start = 0;
         for (z = 1; z <= LLength; z++) {
            hlen = PageWidth;
            if (blen - Start < hlen) hlen = blen - Start;
            memcpy(LLine, bbuf + Start, hlen);
            LLine[hlen] = '\0';
            errno = 0;
            fprintf(LstFile, "%s\n", LLine);
            if ((++LstCounter) == PageLength) NewPage(0, true);
            Start += hlen;
         }
      }
   }
}

/*****************************************************************************/
/* Ausdruck in Spalte vor Listing */

void SetListLineVal(TempResult * t) {
   StrSym(t, true, ListLine);
   strmaxprep(ListLine, "=", 255);
   if (strlen(ListLine) > 14) {
      ListLine[12] = '\0';
      strmaxcat(ListLine, "..", 255);
   }
}

/****************************************************************************/
/* einen Symbolnamen auf Gueltigkeit ueberpruefen */

bool ChkSymbName(char *sym) {
   char *z;

   if (*sym == '\0') return false;
   if (!(isalpha(*sym) || (*sym == '_') || (*sym == '.'))) return false;
   for (z = sym; *z != '\0'; z++)
      if (!(isalnum(*z) || (*z == '_') || (*z == '.'))) return false;
   return true;
}

bool ChkMacSymbName(char *sym) {
   Integer z;

   if (*sym == '\0') return false;
   if (!isalpha(*sym)) return false;
   for (z = 1; z < strlen(sym); z++)
      if (!isalnum(sym[z])) return false;
   return true;
}

/****************************************************************************/
/* Fehlerkanal offen ? */

static void ForceErrorOpen(void) {
   if (!IsErrorOpen) {
      RewriteStandard(&ErrorFile, ErrorName);
      IsErrorOpen = true;
      if (ErrorFile == NULL) ChkIO(10001);
   }
}

/*--------------------------------------------------------------------------*/
/* eine Fehlermeldung  mit Klartext ausgeben */

static void EmergencyStop(void) {
   if ((IsErrorOpen) && (ErrorFile != NULL)) fclose(ErrorFile);
   fclose(LstFile);
   if (ShareMode != 0) {
      fclose(ShareFile);
      unlink(ShareName);
   }
   if (MacProOutput) {
      fclose(MacProFile);
      unlink(MacProName);
   }
   if (MacroOutput) {
      fclose(MacroFile);
      unlink(MacroName);
   }
   if (MakeDebug) fclose(Debug);
   if (CodeOutput) {
      fclose(PrgFile);
      unlink(OutName);
   }
}

void WrErrorString(char *Message, char *Add, bool Warning, bool Fatal) {
   String h, h2;

   strmaxcpy(h, ErrorPos, 255);
   if (!Warning) {
      strmaxcat(h, ErrName, 255);
      strmaxcat(h, Add, 255);
      strmaxcat(h, " : ", 255);
      ErrorCount++;
   } else {
      strmaxcat(h, WarnName, 255);
      strmaxcat(h, Add, 255);
      strmaxcat(h, " : ", 255);
      WarnCount++;
   }

   if ((strcmp(LstName, "/dev/null") != 0) && (!Fatal)) {
      strmaxcpy(h2, h, 255);
      strmaxcat(h2, Message, 255);
      WrLstLine(h2);
      if ((ExtendErrors) && (*ExtendError != '\0')) {
         sprintf(h2, "> > > %s", ExtendError);
         WrLstLine(h2);
      }
   }
   ForceErrorOpen();
   if ((strcmp(LstName, "!1") != 0) || (Fatal)) {
      fprintf((ErrorFile == NULL) ? stdout : ErrorFile, "%s%s%s\n", h, Message, ClrEol);
      if ((ExtendErrors) && (*ExtendError != '\0'))
         fprintf((ErrorFile == NULL) ? stdout : ErrorFile, "> > > %s%s\n", ExtendError, ClrEol);
   }
   *ExtendError = '\0';

   if (Fatal) {
      fprintf((ErrorFile == NULL) ? stdout : ErrorFile, "%s\n", ErrMsgIsFatal);
      EmergencyStop();
      exit(3);
   }
}

/*--------------------------------------------------------------------------*/
/* eine Fehlermeldung ueber Code ausgeben */

static void WrErrorNum(Word Num) {
   String h;
   char Add[11];

   if ((!CodeOutput) && (Num == 1200)) return;

   if ((SuppWarns) && (Num < 1000)) return;

   switch (Num) {
      case 0:
         strmaxcpy(h, ErrMsgUselessDisp, 255);
         break;
      case 10:
         strmaxcpy(h, ErrMsgShortAddrPossible, 255);
         break;
      case 20:
         strmaxcpy(h, ErrMsgShortJumpPossible, 255);
         break;
      case 30:
         strmaxcpy(h, ErrMsgNoShareFile, 255);
         break;
      case 40:
         strmaxcpy(h, ErrMsgBigDecFloat, 255);
         break;
      case 50:
         strmaxcpy(h, ErrMsgPrivOrder, 255);
         break;
      case 60:
         strmaxcpy(h, ErrMsgDistNull, 255);
         break;
      case 70:
         strmaxcpy(h, ErrMsgWrongSegment, 255);
         break;
      case 75:
         strmaxcpy(h, ErrMsgInAccSegment, 255);
         break;
      case 80:
         strmaxcpy(h, ErrMsgPhaseErr, 255);
         break;
      case 90:
         strmaxcpy(h, ErrMsgOverlap, 255);
         break;
      case 100:
         strmaxcpy(h, ErrMsgNoCaseHit, 255);
         break;
      case 110:
         strmaxcpy(h, ErrMsgInAccPage, 255);
         break;
      case 120:
         strmaxcpy(h, ErrMsgRMustBeEven, 255);
         break;
      case 130:
         strmaxcpy(h, ErrMsgObsolete, 255);
         break;
      case 140:
         strmaxcpy(h, ErrMsgUnpredictable, 255);
         break;
      case 150:
         strmaxcpy(h, ErrMsgAlphaNoSense, 255);
         break;
      case 160:
         strmaxcpy(h, ErrMsgSenseless, 255);
         break;
      case 170:
         strmaxcpy(h, ErrMsgRepassUnknown, 255);
         break;
      case 180:
         strmaxcpy(h, ErrMsgAddrNotAligned, 255);
         break;
      case 190:
         strmaxcpy(h, ErrMsgIOAddrNotAllowed, 255);
         break;
      case 200:
         strmaxcpy(h, ErrMsgPipeline, 255);
         break;
      case 210:
         strmaxcpy(h, ErrMsgDoubleAdrRegUse, 255);
         break;
      case 220:
         strmaxcpy(h, ErrMsgNotBitAddressable, 255);
         break;
      case 230:
         strmaxcpy(h, ErrMsgStackNotEmpty, 255);
         break;
      case 240:
         strmaxcpy(h, ErrMsgNULCharacter, 255);
         break;
      case 1000:
         strmaxcpy(h, ErrMsgDoubleDef, 255);
         break;
      case 1010:
         strmaxcpy(h, ErrMsgSymbolUndef, 255);
         break;
      case 1020:
         strmaxcpy(h, ErrMsgInvSymName, 255);
         break;
      case 1090:
         strmaxcpy(h, ErrMsgInvFormat, 255);
         break;
      case 1100:
         strmaxcpy(h, ErrMsgUseLessAttr, 255);
         break;
      case 1105:
         strmaxcpy(h, ErrMsgTooLongAttr, 255);
         break;
      case 1107:
         strmaxcpy(h, ErrMsgUndefAttr, 255);
         break;
      case 1110:
         strmaxcpy(h, ErrMsgWrongArgCnt, 255);
         break;
      case 1115:
         strmaxcpy(h, ErrMsgWrongOptCnt, 255);
         break;
      case 1120:
         strmaxcpy(h, ErrMsgOnlyImmAddr, 255);
         break;
      case 1130:
         strmaxcpy(h, ErrMsgInvOpsize, 255);
         break;
      case 1131:
         strmaxcpy(h, ErrMsgConfOpSizes, 255);
         break;
      case 1132:
         strmaxcpy(h, ErrMsgUndefOpSizes, 255);
         break;
      case 1135:
         strmaxcpy(h, ErrMsgInvOpType, 255);
         break;
      case 1140:
         strmaxcpy(h, ErrMsgTooMuchArgs, 255);
         break;
      case 1200:
         strmaxcpy(h, ErrMsgUnknownOpcode, 255);
         break;
      case 1300:
         strmaxcpy(h, ErrMsgBrackErr, 255);
         break;
      case 1310:
         strmaxcpy(h, ErrMsgDivByZero, 255);
         break;
      case 1315:
         strmaxcpy(h, ErrMsgUnderRange, 255);
         break;
      case 1320:
         strmaxcpy(h, ErrMsgOverRange, 255);
         break;
      case 1325:
         strmaxcpy(h, ErrMsgNotAligned, 255);
         break;
      case 1330:
         strmaxcpy(h, ErrMsgDistTooBig, 255);
         break;
      case 1335:
         strmaxcpy(h, ErrMsgInAccReg, 255);
         break;
      case 1340:
         strmaxcpy(h, ErrMsgNoShortAddr, 255);
         break;
      case 1350:
         strmaxcpy(h, ErrMsgInvAddrMode, 255);
         break;
      case 1351:
         strmaxcpy(h, ErrMsgMustBeEven, 255);
         break;
      case 1355:
         strmaxcpy(h, ErrMsgInvParAddrMode, 255);
         break;
      case 1360:
         strmaxcpy(h, ErrMsgUndefCond, 255);
         break;
      case 1370:
         strmaxcpy(h, ErrMsgJmpDistTooBig, 255);
         break;
      case 1375:
         strmaxcpy(h, ErrMsgDistIsOdd, 255);
         break;
      case 1380:
         strmaxcpy(h, ErrMsgInvShiftArg, 255);
         break;
      case 1390:
         strmaxcpy(h, ErrMsgRange18, 255);
         break;
      case 1400:
         strmaxcpy(h, ErrMsgShiftCntTooBig, 255);
         break;
      case 1410:
         strmaxcpy(h, ErrMsgInvRegList, 255);
         break;
      case 1420:
         strmaxcpy(h, ErrMsgInvCmpMode, 255);
         break;
      case 1430:
         strmaxcpy(h, ErrMsgInvCPUType, 255);
         break;
      case 1440:
         strmaxcpy(h, ErrMsgInvCtrlReg, 255);
         break;
      case 1445:
         strmaxcpy(h, ErrMsgInvReg, 255);
         break;
      case 1450:
         strmaxcpy(h, ErrMsgNoSaveFrame, 255);
         break;
      case 1460:
         strmaxcpy(h, ErrMsgNoRestoreFrame, 255);
         break;
      case 1465:
         strmaxcpy(h, ErrMsgUnknownMacArg, 255);
         break;
      case 1470:
         strmaxcpy(h, ErrMsgMissEndif, 255);
         break;
      case 1480:
         strmaxcpy(h, ErrMsgInvIfConst, 255);
         break;
      case 1483:
         strmaxcpy(h, ErrMsgDoubleSection, 255);
         break;
      case 1484:
         strmaxcpy(h, ErrMsgInvSection, 255);
         break;
      case 1485:
         strmaxcpy(h, ErrMsgMissingEndSect, 255);
         break;
      case 1486:
         strmaxcpy(h, ErrMsgWrongEndSect, 255);
         break;
      case 1487:
         strmaxcpy(h, ErrMsgNotInSection, 255);
         break;
      case 1488:
         strmaxcpy(h, ErrMsgUndefdForward, 255);
         break;
      case 1489:
         strmaxcpy(h, ErrMsgContForward, 255);
         break;
      case 1490:
         strmaxcpy(h, ErrMsgInvFuncArgCnt, 255);
         break;
      case 1495:
         strmaxcpy(h, ErrMsgMissingLTORG, 255);
         break;
      case 1500:
         strmaxcpy(h, ErrMsgNotOnThisCPU1, 255);
         strmaxcat(h, MomCPUIdent, 255);
         strmaxcat(h, ErrMsgNotOnThisCPU2, 255);
         break;
      case 1505:
         strmaxcpy(h, ErrMsgNotOnThisCPU3, 255);
         strmaxcat(h, MomCPUIdent, 255);
         strmaxcat(h, ErrMsgNotOnThisCPU2, 255);
         break;
      case 1510:
         strmaxcpy(h, ErrMsgInvBitPos, 255);
         break;
      case 1520:
         strmaxcpy(h, ErrMsgOnlyOnOff, 255);
         break;
      case 1530:
         strmaxcpy(h, ErrMsgStackEmpty, 255);
         break;
      case 1600:
         strmaxcpy(h, ErrMsgShortRead, 255);
         break;
      case 1700:
         strmaxcpy(h, ErrMsgRomOffs063, 255);
         break;
      case 1710:
         strmaxcpy(h, ErrMsgInvFCode, 255);
         break;
      case 1720:
         strmaxcpy(h, ErrMsgInvFMask, 255);
         break;
      case 1730:
         strmaxcpy(h, ErrMsgInvMMUReg, 255);
         break;
      case 1740:
         strmaxcpy(h, ErrMsgLevel07, 255);
         break;
      case 1750:
         strmaxcpy(h, ErrMsgInvBitMask, 255);
         break;
      case 1760:
         strmaxcpy(h, ErrMsgInvRegPair, 255);
         break;
      case 1800:
         strmaxcpy(h, ErrMsgOpenMacro, 255);
         break;
      case 1805:
         strmaxcpy(h, ErrMsgEXITMOutsideMacro, 255);
         break;
      case 1810:
         strmaxcpy(h, ErrMsgTooManyMacParams, 255);
         break;
      case 1815:
         strmaxcpy(h, ErrMsgDoubleMacro, 255);
         break;
      case 1820:
         strmaxcpy(h, ErrMsgFirstPassCalc, 255);
         break;
      case 1830:
         strmaxcpy(h, ErrMsgTooManyNestedIfs, 255);
         break;
      case 1840:
         strmaxcpy(h, ErrMsgMissingIf, 255);
         break;
      case 1850:
         strmaxcpy(h, ErrMsgRekMacro, 255);
         break;
      case 1860:
         strmaxcpy(h, ErrMsgUnknownFunc, 255);
         break;
      case 1870:
         strmaxcpy(h, ErrMsgInvFuncArg, 255);
         break;
      case 1880:
         strmaxcpy(h, ErrMsgFloatOverflow, 255);
         break;
      case 1890:
         strmaxcpy(h, ErrMsgInvArgPair, 255);
         break;
      case 1900:
         strmaxcpy(h, ErrMsgNotOnThisAddress, 255);
         break;
      case 1905:
         strmaxcpy(h, ErrMsgNotFromThisAddress, 255);
         break;
      case 1910:
         strmaxcpy(h, ErrMsgTargOnDiffPage, 255);
         break;
      case 1920:
         strmaxcpy(h, ErrMsgCodeOverflow, 255);
         break;
      case 1925:
         strmaxcpy(h, ErrMsgAdrOverflow, 255);
         break;
      case 1930:
         strmaxcpy(h, ErrMsgMixDBDS, 255);
         break;
      case 1940:
         strmaxcpy(h, ErrMsgOnlyInCode, 255);
         break;
      case 1950:
         strmaxcpy(h, ErrMsgParNotPossible, 255);
         break;
      case 1960:
         strmaxcpy(h, ErrMsgInvSegment, 255);
         break;
      case 1961:
         strmaxcpy(h, ErrMsgUnknownSegment, 255);
         break;
      case 1962:
         strmaxcpy(h, ErrMsgUnknownSegReg, 255);
         break;
      case 1970:
         strmaxcpy(h, ErrMsgInvString, 255);
         break;
      case 1980:
         strmaxcpy(h, ErrMsgInvRegName, 255);
         break;
      case 1985:
         strmaxcpy(h, ErrMsgInvArg, 255);
         break;
      case 1990:
         strmaxcpy(h, ErrMsgNoIndir, 255);
         break;
      case 1995:
         strmaxcpy(h, ErrMsgNotInThisSegment, 255);
         break;
      case 1996:
         strmaxcpy(h, ErrMsgNotInMaxmode, 255);
         break;
      case 1997:
         strmaxcpy(h, ErrMsgOnlyInMaxmode, 255);
         break;
      case 10001:
         strmaxcpy(h, ErrMsgOpeningFile, 255);
         break;
      case 10002:
         strmaxcpy(h, ErrMsgListWrError, 255);
         break;
      case 10003:
         strmaxcpy(h, ErrMsgFileReadError, 255);
         break;
      case 10004:
         strmaxcpy(h, ErrMsgFileWriteError, 255);
         break;
      case 10006:
         strmaxcpy(h, ErrMsgHeapOvfl, 255);
         break;
      case 10007:
         strmaxcpy(h, ErrMsgStackOvfl, 255);
         break;
      default:
         strmaxcpy(h, ErrMsgIntError, 255);
   }

   if (((Num == 1910) || (Num == 1370)) && (!Repass)) JmpErrors++;

   if (NumericErrors) sprintf(Add, "#%d", Num);
   else *Add = '\0';
   WrErrorString(h, Add, Num < 1000, Num >= 10000);
}

void WrError(Word Num) {
   *ExtendError = '\0';
   WrErrorNum(Num);
}

void WrXError(Word Num, char *Message) {
   strmaxcpy(ExtendError, Message, 255);
   WrErrorNum(Num);
}

/*--------------------------------------------------------------------------*/
/* I/O-Fehler */

void ChkIO(Word ErrNo) {
   int io;

   io = errno;
   if ((io == 0) || (io == 19)) return;

   WrXError(ErrNo, GetErrorMsg(io));
}

/*--------------------------------------------------------------------------*/
/* Bereichsfehler */

bool ChkRange(LargeInt Value, LargeInt Min, LargeInt Max) {
   char s1[100], s2[100];

   if (Value < Min) {
      strmaxcpy(s1, LargeString(Value), 99);
      strmaxcpy(s2, LargeString(Min), 99);
      strmaxcat(s1, "<", 99);
      strmaxcat(s1, s2, 99);
      WrXError(1315, s1);
      return false;
   } else if (Value > Max) {
      strmaxcpy(s1, LargeString(Value), 99);
      strmaxcpy(s2, LargeString(Max), 99);
      strmaxcat(s1, ">", 99);
      strmaxcat(s1, s2, 99);
      WrXError(1320, s1);
      return false;
   } else return true;
}

/****************************************************************************/

LargeWord ProgCounter(void) {
   return PCs[ActPC];
}

/*--------------------------------------------------------------------------*/
/* aktuellen Programmzaehler mit Phasenverschiebung holen */

LargeWord EProgCounter(void) {
   return PCs[ActPC] + Phases[ActPC];
}

/*--------------------------------------------------------------------------*/
/* Granularitaet des aktuellen Segments holen */

Word Granularity(void) {
   return Grans[ActPC];
}

/*--------------------------------------------------------------------------*/
/* Linstingbreite des aktuellen Segments holen */

Word ListGran(void) {
   return ListGrans[ActPC];
}

/*--------------------------------------------------------------------------*/
/* pruefen, ob alle Symbole einer Formel im korrekten Adressraum lagen */

void ChkSpace(Byte Space) {
   Byte Mask = 0xff - (1 << Space);

   if ((TypeFlag & Mask) != 0) WrError(70);
}

/****************************************************************************/
/* eine Chunkliste im Listing ausgeben & Speicher loeschen */

void PrintChunk(ChunkList * NChunk) {
   LargeWord NewMin, FMin;
   bool Found;
   Word p = 0, z;
   Integer BufferZ;
   String BufferS;

   NewMin = 0;
   BufferZ = 0;
   *BufferS = '\0';

   do {
   /* niedrigsten Start finden, der ueberhalb des letzten Endes liegt */
      Found = false;
#ifdef __STDC__
      FMin = 0xffffffffu;
#else
      FMin = 0xffffffff;
#endif
      for (z = 0; z < NChunk->RealLen; z++)
         if (NChunk->Chunks[z].Start >= NewMin)
            if (FMin > NChunk->Chunks[z].Start) {
               Found = true;
               FMin = NChunk->Chunks[z].Start;
               p = z;
            }

      if (Found) {
         strmaxcat(BufferS, HexString(NChunk->Chunks[p].Start, 0), 255);
         if (NChunk->Chunks[p].Length != 1) {
            strmaxcat(BufferS, "-", 255);
            strmaxcat(BufferS, HexString(NChunk->Chunks[p].Start + NChunk->Chunks[p].Length - 1, 0), 255);
         }
         strmaxcat(BufferS, Blanks(19 - strlen(BufferS) % 19), 255);
         if (++BufferZ == 4) {
            WrLstLine(BufferS);
            *BufferS = '\0';
            BufferZ = 0;
         }
         NewMin = NChunk->Chunks[p].Start + NChunk->Chunks[p].Length;
      }
   }
   while (Found);

   if (BufferZ != 0) WrLstLine(BufferS);
}

/*--------------------------------------------------------------------------*/
/* Listen ausgeben */

void PrintUseList(void) {
   Integer z, z2, l;
   String s;

   for (z = 1; z <= PCMax; z++)
      if (SegChunks[z].Chunks != NULL) {
         sprintf(s, "  %s%s%s", ListSegListHead1, SegNames[z], ListSegListHead2);
         WrLstLine(s);
         strcpy(s, "  ");
         l = strlen(SegNames[z]) + strlen(ListSegListHead1) + strlen(ListSegListHead2);
         for (z2 = 0; z2 < l; z2++) strmaxcat(s, "-", 255);
         WrLstLine(s);
         WrLstLine("");
         PrintChunk(SegChunks + z);
         WrLstLine("");
      }
}

void ClearUseList(void) {
   Integer z;

   for (z = 1; z <= PCMax; z++)
      ClearChunk(SegChunks + z);
}

/****************************************************************************/
/* Include-Pfadlistenverarbeitung */

static char *GetPath(char *Acc) {
   char *p;
   static String tmp;

   p = strchr(Acc, ':');
   if (p == NULL) {
      strmaxcpy(tmp, Acc, 255);
      Acc[0] = '\0';
   } else {
      *p = '\0';
      strmaxcpy(tmp, Acc, 255);
      strcpy(Acc, p + 1);
   }
   return tmp;
}

void AddIncludeList(char *NewPath) {
   String Test;

   strmaxcpy(Test, IncludeList, 255);
   while (*Test != '\0')
      if (strcmp(GetPath(Test), NewPath) == 0) return;
   if (*IncludeList != '\0') strmaxprep(IncludeList, ":", 255);
   strmaxprep(IncludeList, NewPath, 255);
}

void RemoveIncludeList(char *RemPath) {
   String Save;
   char *Part;

   strmaxcpy(IncludeList, Save, 255);
   IncludeList[0] = '\0';
   while (Save[0] != '\0') {
      Part = GetPath(Save);
      if (strcmp(Part, RemPath) != 0) {
         if (IncludeList[0] != '\0') strmaxcat(IncludeList, ":", 255);
         strmaxcat(IncludeList, Part, 255);
      }
   }
}

/****************************************************************************/
/* Liste mit Ausgabedateien */

void ClearOutList(void) {
   ClearStringList(&OutList);
}

void AddToOutList(char *NewName) {
   AddStringListLast(&OutList, NewName);
}

void RemoveFromOutList(char *OldName) {
   RemoveStringList(&OutList, OldName);
}

char *GetFromOutList(void) {
   return GetAndCutStringList(&OutList);
}

/****************************************************************************/
/* Tokenverarbeitung */

static bool CompressLine_NErl(char ch) {
   return (((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z')) || ((ch >= '0') && (ch <= '9')));
}

void CompressLine(char *TokNam, Byte Num, char *Line) {
   Integer z, e, tlen, llen;
   bool SFound;

   z = 0;
   tlen = strlen(TokNam);
   llen = strlen(Line);
   while (z <= llen - tlen) {
      e = z + strlen(TokNam);
      SFound = (CaseSensitive) ? (strncmp(Line + z, TokNam, tlen) == 0)
         : (strncasecmp(Line + z, TokNam, tlen) == 0);
      if ((SFound)
         && ((z == 0) || (!CompressLine_NErl(Line[z - 1])))
         && ((e >= strlen(Line)) || (!CompressLine_NErl(Line[e])))) {
         strcpy(Line + z + 1, Line + e);
         Line[z] = Num;
         llen = strlen(Line);
      };
      z++;
   }
}

void ExpandLine(char *TokNam, Byte Num, char *Line) {
   char *z;

   do {
      z = strchr(Line, Num);
      if (z != NULL) {
         strcpy(z, z + 1);
         strmaxins(Line, TokNam, z - Line, 255);
      }
   }
   while (z != 0);
}

void KillCtrl(char *Line) {
   char *z;

   if (*(z = Line) == '\0') return;
   do {
      if (*z == '\0');
      else if (*z == Char_HT) {
         strcpy(z, z + 1);
         strprep(z, Blanks(8 - ((z - Line) % 8)));
      } else if (*z < ' ') *z = ' ';
      z++;
   }
   while (*z != '\0');
}

/****************************************************************************/
/* Differenz zwischen zwei Zeiten mit Jahresueberlauf berechnen */

long DTime(long t1, long t2) {
   LongInt d;

   d = t2 - t1;
   if (d < 0) d += (24 * 360000);
   return (d > 0) ? d : -d;
}

/*--------------------------------------------------------------------------*/
/* Zeit holen */

#ifdef __MSDOS__

#   include <dos.h>

long GTime(void) {
   struct time ti;
   struct date da;

   gettime(&ti);
   getdate(&da);
   return (dostounix(&da, &ti) * 100) + ti.ti_hund;
}

#else

#   include <sys/time.h>

long GTime(void) {
   struct timeval tv;

   gettimeofday(&tv, NULL);
   return (tv.tv_sec * 100) + (tv.tv_usec / 10000);
}

#endif
/**
{****************************************************************************}
{ Heapfehler abfedern }

        FUNCTION MyHeapError(Size:Word):Integer;
        Far;
 {
   IF Size<>0 THEN WrError(10006);
   MyHeapError:=1;
};
**/
/*-------------------------------------------------------------------------*/
/* Stackfehler abfangen - bis auf DOS nur Dummies */

#ifdef __TURBOC__
unsigned _stklen = 65520;
#   include <malloc.h>
#endif

void ChkStack(void) {
#ifdef __TURBOC__
   LongWord avail = stackavail();
   if (avail < MinStack) WrError(10007);
   if (avail < LowStack) LowStack = avail;
#endif
}

void ResetStack(void) {
#ifdef __TURBOC__
   LowStack = stackavail();
#endif
}

LongWord StackRes(void) {
#ifdef __TURBOC__
   return LowStack - MinStack;
#else
   return 0;
#endif
}

/****************************************************************************/
/**
{$IFDEF DPMI}
        FUNCTION MemInitSwapFile(FileName: pChar; FileSize: LongInt): INTEGER;
        EXTERNAL 'RTM' INDEX 35;

        FUNCTION MemCloseSwapFile(Delete: INTEGER): INTEGER;
        EXTERNAL 'RTM' INDEX 36;
{$ENDIF}

VAR
   Cnt:Char;
   FileLen:LongInt;
   p,err:Integer;
   MemFlag,TempName:String;**/

void asmsub_init(void) {
   char *CMess = InfoMessCopyright;
   Word z;
   LongWord XORVal;

/**
   { Fuer DPMI evtl. Swapfile anlegen }

{$IFDEF DPMI}
   MemFlag:=GetEnv('ASXSWAP');
   IF MemFlag<>'' THEN
    {
     p:=Pos(',',MemFlag);
     IF p=0 THEN TempName:='ASX.TMP'
     ELSE
      {
       TempName:=Copy(MemFlag,p+1,Length(MemFlag)-p);
       MemFlag:=Copy(MemFlag,1,p-1);
      };
     KillBlanks(TempName); KillBlanks(MemFlag);
     TempName:=TempName+#0;
     Val(MemFlag,FileLen,Err);
     IF Err<>0 THEN
      {
       WriteLn(StdErr,ErrMsgInvSwapSize); Halt(4);
      };
     IF MemInitSwapFile(@TempName[1],FileLen SHL 20)<>0 THEN
      {
       WriteLn(StdErr,ErrMsgSwapTooBig); Halt(4);
      };
    };
{$ENDIF}

   HeapError:=@MyHeapError;
**/

   for (z = 0; z < strlen(CMess); z++) {
      XORVal = CMess[z];
      XORVal = XORVal << (((z + 1) % 4) * 8);
      Magic = Magic ^ XORVal;
   }

   InitStringList(&CopyrightList);
   InitStringList(&OutList);

#ifdef __TURBOC__
   StartStack = stackavail();
   LowStack = stackavail();
   MinStack = StartStack - 65520 + 0x800;
#else
   StartStack = LowStack = MinStack = 0;
#endif
}
