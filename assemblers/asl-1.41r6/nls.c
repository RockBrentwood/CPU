// AS-Portierung
// Abhandlung landesspezifischer Unterschiede
#include "stdinc.h"
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef LOCALE_NLS
#   include <locale.h>
#   include <langinfo.h>
#endif

#ifdef OS2_NLS
#   define INCL_DOSNLS
#   include <os2.h>
#endif

#include "stringutil.h"

#include "nls.h"

CharTable UpCaseTable; /* Umsetzungstabellen */
CharTable LowCaseTable;

static NLS_CountryInfo NLSInfo;
static CharTable CollateTable;

#if 0
/*-------------------------------------------------------------------------------*/

/* einen String anhand einer Tabelle uebersetzen: */

static void TranslateString(char *s, CharTable Table) {
   for (; *s != '\0'; s++) *s = Table[((unsigned int)*s) & 0xff];
}
#endif

/*-------------------------------------------------------------------------------*/
/* Da es moeglich ist, die aktuelle Codeseite im Programmlauf zu wechseln,
   ist die Initialisierung in einer getrennten Routine untergebracht.  Nach
   einem Wechsel stellt ein erneuter Aufruf wieder korrekte Verhaeltnisse
   her.  Wen das stoert, der schreibe einfach einen Aufruf in den Initiali-
   sierungsteil der Unit hinein. */

void NLS_Initialize(void) {
   char *tmpstr, *run, *cpy;
   Word FmtBuffer;
   Integer z;
   bool DidDate;

#ifdef LOCALE_NLS
   struct lconv *lc;
#endif

#ifdef OS2_NLS
   COUNTRYCODE ccode;
   COUNTRYINFO cinfo;
   ULONG erglen;
#endif

/* get currency format, separators */

#ifdef NO_NLS
   NLSInfo.DecSep = ".";
   NLSInfo.ThouSep = ",";
   NLSInfo.Currency = "$";
   NLSInfo.CurrDecimals = 2;
   NLSInfo.CurrFmt = CurrFormatPreNoBlank;
#endif

#ifdef LOCALE_NLS
   lc = localeconv();

   NLSInfo.DecSep = (lc->mon_decimal_point != NULL) ? lc->decimal_point : ".";

   NLSInfo.ThouSep = (lc->mon_thousands_sep != NULL) ? lc->mon_thousands_sep : ",";

   NLSInfo.Currency = (lc->currency_symbol != NULL) ? lc->currency_symbol : "$";

   NLSInfo.CurrDecimals = lc->int_frac_digits;
   if (NLSInfo.CurrDecimals > 4) NLSInfo.CurrDecimals = 2;

   if (lc->p_cs_precedes)
      if (lc->p_sep_by_space) NLSInfo.CurrFmt = CurrFormatPreBlank;
      else NLSInfo.CurrFmt = CurrFormatPreNoBlank;
   else if (lc->p_sep_by_space) NLSInfo.CurrFmt = CurrFormatPostBlank;
   else NLSInfo.CurrFmt = CurrFormatPostNoBlank;
#endif

#ifdef OS2_NLS
   ccode.country = 0;
   ccode.codepage = 0;
   DosQueryCtryInfo(sizeof(cinfo), &ccode, &cinfo, &erglen);

   NLSInfo.DecSep = strdup(cinfo.szDecimal);

   NLSInfo.ThouSep = strdup(cinfo.szThousandsSeparator);

   NLSInfo.Currency = strdup(cinfo.szCurrency);

   NLSInfo.CurrDecimals = cinfo.cDecimalPlace;

   NLSInfo.CurrFmt = (CurrFormat) cinfo.fsCurrencyFmt;
#endif

/* get date format */

#ifdef NO_NLS
   tmpstr = "%m/%d/%y";
   DidDate = false;
#endif

#ifdef LOCALE_NLS
   tmpstr = nl_langinfo(D_FMT);
   DidDate = false;
#endif

#ifdef OS2_NLS
   NLSInfo.DateFmt = (DateFormat) cinfo.fsDateFmt;
   NLSInfo.DateSep = strdup(cinfo.szDateSeparator);
   DidDate = true;
#endif

   if (!DidDate) {
      NLSInfo.DateSep = NULL;
      FmtBuffer = 0;
      run = tmpstr;
      while (*run != '\0')
         if (*run == '%') {
            FmtBuffer <<= 4;
            switch (toupper(*(++run))) {
               case 'D':
                  FmtBuffer += 1;
                  break;
               case 'M':
                  FmtBuffer += 2;
                  break;
               case 'Y':
                  FmtBuffer += 3;
                  break;
            }
            if (NLSInfo.DateSep == NULL) {
               run++;
               cpy = NLSInfo.DateSep = strdup("                  ");
               while ((*run != ' ') && (*run != '%')) *(cpy++) = (*(run++));
               *cpy = '\0';
            } else run++;
         } else run++;
      if (FmtBuffer == 0x213) NLSInfo.DateFmt = DateFormatMTY;
      else if (FmtBuffer == 0x123) NLSInfo.DateFmt = DateFormatTMY;
      else NLSInfo.DateFmt = DateFormatYMT;
   }

/* get time format */

#ifdef NO_NLS
   tmpstr = "%H:%M:%S";
   DidDate = false;
#endif

#ifdef LOCALE_NLS
   tmpstr = nl_langinfo(T_FMT);
   DidDate = false;
#endif

#ifdef OS2_NLS
   NLSInfo.TimeFmt = (TimeFormat) cinfo.fsTimeFmt;
   NLSInfo.TimeSep = strdup(cinfo.szTimeSeparator);
   DidDate = true;
#endif

   if (!DidDate) {
      NLSInfo.TimeSep = NULL;
      FmtBuffer = 0;
      run = tmpstr;
      while (*run != '\0')
         if (*run == '%') {
            FmtBuffer <<= 4;
            switch (toupper(*(++run))) {
               case 'S':
                  FmtBuffer += 1;
                  break;
               case 'M':
                  FmtBuffer += 2;
                  break;
               case 'H':
                  FmtBuffer += 3;
                  break;
            }
            if (NLSInfo.TimeSep == NULL) {
               run++;
               cpy = NLSInfo.TimeSep = strdup("                  ");
               while ((*run != ' ') && (*run != '%')) *(cpy++) = (*(run++));
               *cpy = '\0';
            } else run++;
         } else run++;
      NLSInfo.TimeFmt = TimeFormatEurope;
   }

/* get lower->upper case table */

#if defined(NO_NLS) || defined(LOCALE_NLS)
   for (z = 0; z < 256; z++) UpCaseTable[z] = toupper(z);
#endif

#ifdef OS2_NLS
   for (z = 0; z < 256; z++) UpCaseTable[z] = (char)z;
   for (z = 'a'; z <= 'z'; z++) UpCaseTable[z] -= 'a' - 'A';
   DosMapCase(sizeof(UpCaseTable), &ccode, UpCaseTable);
#endif

/* get upper->lower case table */

#if defined(NO_NLS) || defined(LOCALE_NLS)
   for (z = 0; z < 256; z++) LowCaseTable[z] = tolower(z);
#endif

#ifdef OS2_NLS
   for (z = 0; z < 256; z++) LowCaseTable[z] = (char)z;
   for (z = 0; z < 256; z++)
      if (UpCaseTable[z] != (char)z)
         LowCaseTable[((unsigned int)UpCaseTable[z]) & 0xff] = (char)z;
#endif

/* get collation table */
#if defined(NO_NLS) || defined(LOCALE_NLS)
   for (z = 0; z < 256; z++) CollateTable[z] = z;
   for (z = 'a'; z <= 'z'; z++) CollateTable[z] = toupper(z);
#endif

#ifdef OS2_NLS
   for (z = 0; z < 256; z++) CollateTable[z] = (char)z;
   DosQueryCollate(sizeof(CollateTable), &ccode, CollateTable, &erglen);
#endif
}

void NLS_GetCountryInfo(NLS_CountryInfo * Info) {
   *Info = NLSInfo;
}

void NLS_DateString(Word Year, Word Month, Word Day, char *Dest) {
   switch (NLSInfo.DateFmt) {
      case DateFormatMTY:
         sprintf(Dest, "%d%s%d%s%d", Month, NLSInfo.DateSep, Day, NLSInfo.DateSep, Year);
         break;
      case DateFormatTMY:
         sprintf(Dest, "%d%s%d%s%d", Day, NLSInfo.DateSep, Month, NLSInfo.DateSep, Year);
         break;
      case DateFormatYMT:
         sprintf(Dest, "%d%s%d%s%d", Year, NLSInfo.DateSep, Month, NLSInfo.DateSep, Day);
         break;
   }
}

void NLS_CurrDateString(char *Dest) {
   time_t timep;
   struct tm *trec;

   time(&timep);
   trec = localtime(&timep);
   NLS_DateString(trec->tm_year + 1900, trec->tm_mon + 1, trec->tm_mday, Dest);
}

void NLS_TimeString(Word Hour, Word Minute, Word Second, Word Sec100, char *Dest) {
   Word OriHour;
   String ext;

   OriHour = Hour;
   if (NLSInfo.TimeFmt == TimeFormatUSA) {
      Hour %= 12;
      if (Hour == 0) Hour = 12;
   }
   sprintf(Dest, "%d%s%02d%s%02d", Hour, NLSInfo.TimeSep, Minute, NLSInfo.TimeSep, Second);
   if (Sec100 < 100) {
      sprintf(ext, "%s%02d", NLSInfo.DecSep, Sec100);
      strcat(Dest, ext);
   }
   if (NLSInfo.TimeFmt == TimeFormatUSA)
      strcat(Dest, (OriHour > 12) ? "p" : "a");
}

void NLS_CurrTimeString(bool Use100, char *Dest) {
   time_t timep;
   struct tm *trec;

   time(&timep);
   trec = localtime(&timep);
   NLS_TimeString(trec->tm_hour, trec->tm_min, trec->tm_sec, Use100 ? 0 : 100, Dest);
}

void NLS_CurrencyString(double inp, char *erg) {
   char s[1024], form[1024];
   char *p, *z;

/* Schritt 1: mit passender Nachkommastellenzahl wandeln */

   sprintf(form, "%%0.%df", NLSInfo.CurrDecimals);
   sprintf(s, form, inp);

/* Schritt 2: vorne den Punkt suchen */

   p = (NLSInfo.CurrDecimals == 0) ? s + strlen(s) : strchr(s, '.');

/* Schritt 3: Tausenderstellen einfuegen */

   z = p;
   while (z - s > 3) {
      strins(s, NLSInfo.ThouSep, z - s - 3);
      z -= 3;
      p += strlen(NLSInfo.ThouSep);
   }

/* Schritt 4: Komma anpassen */

   strmove(p, 1);
   strins(s, NLSInfo.DecSep, p - s);

/* Schritt 5: Einheit anbauen */

   switch (NLSInfo.CurrFmt) {
      case CurrFormatPreNoBlank:
         sprintf(erg, "%s%s", NLSInfo.Currency, s);
         break;
      case CurrFormatPreBlank:
         sprintf(erg, "%s %s", NLSInfo.Currency, s);
         break;
      case CurrFormatPostNoBlank:
         sprintf(erg, "%s%s", s, NLSInfo.Currency);
         break;
      case CurrFormatPostBlank:
         sprintf(erg, "%s%s", s, NLSInfo.Currency);
         break;
      default:
         strmove(p, strlen(NLSInfo.DecSep));
         strins(NLSInfo.Currency, s, p - s);
   }
}

char Upcase(char inp) {
   return UpCaseTable[((unsigned int)inp) & 0xff];
}

void NLS_UpString(char *s) {
   char *z;

   for (z = s; *z != '\0'; z++) *z = UpCaseTable[((unsigned int)*z) & 0xff];
}

void NLS_LowString(char *s) {
   char *z;

   for (z = s; *z != '\0'; z++) *z = LowCaseTable[((unsigned int)*z) & 0xff];
}

int NLS_StrCmp(const char *s1, const char *s2) {
   while (CollateTable[((unsigned int)*s1) & 0xff] == CollateTable[((unsigned int)*s2) & 0xff]) {
      if ((!*s1) && (!*s2)) return 0;
      s1++;
      s2++;
   }
   return ((int)CollateTable[((unsigned int)*s1) & 0xff] - CollateTable[((unsigned int)*s2) & 0xff]);
}

void nls_init(void) {
#ifdef LOCALE_NLS
   (void)setlocale(LC_ALL, "");
   (void)setlocale(LC_MONETARY, "");
#endif
}
