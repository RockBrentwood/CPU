// AS-Portierung
// Abhandlung landesspezifischer Unterschiede
typedef enum { TimeFormatUSA, TimeFormatEurope, TimeFormatJapan } TimeFormat;
typedef enum { DateFormatMTY, DateFormatTMY, DateFormatYMT } DateFormat;
typedef enum { CurrFormatPreNoBlank, CurrFormatPostNoBlank, CurrFormatPreBlank, CurrFormatPostBlank } CurrFormat;

typedef struct {
   Word Country; /* = internationale Vorwahl */
   Word CodePage; /* mom. gewaehlter Zeichensatz */
   DateFormat DateFmt; /* Datumsreihenfolge */
   char *DateSep; /* Trennzeichen zwischen Datumskomponenten */
   TimeFormat TimeFmt; /* 12/24-Stundenanzeige */
   char *TimeSep; /* Trennzeichen zwischen Zeitkomponenten */
   char *Currency; /* Waehrungsname */
   CurrFormat CurrFmt; /* Anzeigeformat Waehrung */
   Byte CurrDecimals; /* Nachkommastellen Waehrungsbetraege */
   char *ThouSep; /* Trennzeichen fuer Tausenderbloecke */
   char *DecSep; /* Trennzeichen fuer Nachkommastellen */
   char *DataSep; /* ??? */
} NLS_CountryInfo;

typedef char CharTable[256];

extern CharTable UpCaseTable, LowCaseTable;

void NLS_Initialize(void);
void NLS_GetCountryInfo(NLS_CountryInfo * Info);
void NLS_DateString(Word Year, Word Month, Word Day, char *Dest);
void NLS_CurrDateString(char *Dest);
void NLS_TimeString(Word Hour, Word Minute, Word Second, Word Sec100, char *Dest);
void NLS_CurrTimeString(bool Use100, char *Dest);
void NLS_CurrencyString(double inp, char *erg);
char Upcase(char inp);
void NLS_UpString(char *s);
void NLS_LowString(char *s);
int NLS_StrCmp(const char *s1, const char *s2);
void nls_init(void);
