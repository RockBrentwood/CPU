// AS-Portierung
// haeufig benoetigte String-Funktionen
#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "stringutil.h"
#undef strlen /* VORSICHT, Rekursion!!! */

bool HexLowerCase; /* Hex-Konstanten mit Kleinbuchstaben */

/*--------------------------------------------------------------------------*/
/* eine bestimmte Anzahl Leerzeichen liefern */

char *Blanks(int cnt) {
   static char *BlkStr = "                                                                                                           ";

   if (cnt < 0) cnt = 0;

   return BlkStr + (strlen(BlkStr) - cnt);
}

/****************************************************************************/
/* eine Integerzahl in eine Hexstring umsetzen. Weitere vordere Stellen als */
/* Nullen */

#define BufferCnt 8

char *HexString(LargeWord i, Byte Stellen) {
   static char *UDigitVals = "0123456789ABCDEF", *LDigitVals = "0123456789abcdef", *ptr;
   static String h[BufferCnt];
   static int z = 0;

   if (Stellen > 8) Stellen = 8;
   h[z][0] = '\0';
   do {
      memmove(h[z] + 1, h[z], strlen(h[z]) + 1);
      h[z][0] = (HexLowerCase) ? (LDigitVals[i & 15]) : (UDigitVals[i & 15]);
      i >>= 4;
   }
   while ((i != 0) || (strlen(h[z]) < Stellen));
   ptr = h[z];
   z = (z + 1) % BufferCnt;
   return ptr;
}

/*--------------------------------------------------------------------------*/
/* dito, nur vorne Leerzeichen */

char *HexBlankString(LargeWord i, Byte Stellen) {
   static String temp;

   strmaxcpy(temp, HexString(i, 0), 255);
   if (strlen(temp) < Stellen) strmaxprep(temp, Blanks(Stellen - strlen(temp)), 255);
   return temp;
}

/*---------------------------------------------------------------------------*/
/* Da manche Systeme (SunOS) Probleme mit der Asugabe extra langer Ints
   haben, machen wir das jetzt zu Fuss :-( */

char *LargeString(LargeInt i) {
   bool SignFlag = false;
   static String s;
   String tmp;
   char *p, *p2;

   if (i < 0) {
      i = (-i);
      SignFlag = true;
   }

   p = tmp;
   do {
      *(p++) = '0' + (i % 10);
      i /= 10;
   }
   while (i > 0);

   p2 = s;
   if (SignFlag) *(p2++) = '-';
   while (p > tmp) *(p2++) = (*(--p));
   *p2 = '\0';
   return s;
}

/*---------------------------------------------------------------------------*/
/* manche haben's nicht... */

#ifdef NEEDS_STRDUP
char *strdup(char *S) {
   char *ptr = malloc(strlen(S) + 1);
   if (ptr != NULL) strcpy(ptr, S);
   return ptr;
}
#endif

#ifdef NEEDS_CASECMP
int strcasecmp(const char *src1, const char *src2) {
   while (toupper(*src1) == toupper(*src2)) {
      if ((!*src1) && (!*src2)) return 0;
      src1++;
      src2++;
   }
   return ((int)toupper(*src1)) - ((int)toupper(*src2));
}

int strncasecmp(const char *src1, const char *src2, int len) {
   while (toupper(*src1) == toupper(*src2)) {
      if (--len == 0) return 0;
      if ((!*src1) && (!*src2)) return 0;
      src1++;
      src2++;
   }
   return ((int)toupper(*src1)) - ((int)toupper(*src2));
}
#endif

#ifdef NEEDS_STRSTR
char *strstr(char *haystack, char *needle) {
   int lh = strlen(haystack), ln = strlen(needle);
   int z;
   char *p;

   for (z = 0; z <= lh - ln; z++)
      if (strncmp(p = haystack + z, needle, ln) == 0) return p;
   return NULL;
}
#endif

/*---------------------------------------------------------------------------*/
/* das originale strncpy plaettet alle ueberstehenden Zeichen mit Nullen */

void strmaxcpy(char *Dest, const char *Src, int Max) {
   int cnt = strlen(Src);

   if (cnt > Max) cnt = Max;
   memcpy(Dest, Src, cnt);
   Dest[cnt] = '\0';
}

/*---------------------------------------------------------------------------*/
/* einfuegen, mit Begrenzung */

void strmaxcat(char *Dest, const char *Src, int MaxLen) {
   int TLen = strlen(Src), DLen = strlen(Dest);

   if (TLen > MaxLen - DLen) TLen = MaxLen - DLen;
   if (TLen > 0) {
      memcpy(Dest + DLen, Src, TLen);
      Dest[DLen + TLen] = '\0';
   }
}

void strmove(char *S, int dS) {
   if (dS == 0) return;
   int Len = strlen(S);
#if 1
// Fast, insecure.
   if (dS >= Len || dS <= -Len) *S = '\0';
   else if (dS > 0) memmove(S, S + dS, Len - dS), S[Len - dS] = '\0';
#else
// Slow, secure.
   if (dS >= Len || dS <= -Len) memset(S, '\0', Len);
   else if (dS > 0) memmove(S, S + dS, Len - dS), memset(S + Len - dS, '\0', dS);
#endif
   else memmove(S - dS, S, Len + dS), memset(S, '\0', -dS);
}

void strcopy(char *Dest, char *Src) { memmove(Dest, Src, strlen(Src) + 1); }
void stradd(char *Dest, char *Src) { memmove(Dest + strlen(Dest), Src, strlen(Src) + 1); }

void strprep(char *Dest, const char *Src) {
   memmove(Dest + strlen(Src), Dest, strlen(Dest) + 1);
   memmove(Dest, Src, strlen(Src));
}

void strmaxprep(char *Dest, const char *Src, int MaxLen) {
   int RLen;

   RLen = strlen(Src);
   if (RLen > MaxLen - strlen(Dest)) RLen = MaxLen - strlen(Dest);
   memmove(Dest + RLen, Dest, strlen(Dest) + 1);
   memmove(Dest, Src, RLen);
}

void strins(char *Dest, const char *Src, int Pos) {
   memmove(Dest + Pos + strlen(Src), Dest + Pos, strlen(Dest) + 1 - Pos);
   memmove(Dest + Pos, Src, strlen(Src));
}

void strmaxins(char *Dest, const char *Src, int Pos, int MaxLen) {
   int RLen;

   RLen = strlen(Src);
   if (RLen > MaxLen - strlen(Dest)) RLen = MaxLen - strlen(Dest);
   memmove(Dest + Pos + RLen, Dest + Pos, strlen(Dest) + 1 - Pos);
   memmove(Dest + Pos, Src, RLen);
}

/*---------------------------------------------------------------------------*/
/* Bis Zeilenende lesen */

void ReadLn(FILE * Datei, char *Zeile) {
   int Zeichen = '\0';
   char *Run = Zeile;

   while ((Zeichen != '\n') && (Zeichen != EOF) && (!feof(Datei))) {
      Zeichen = fgetc(Datei);
      if (Zeichen != 26) *Run++ = Zeichen;
   }

   if ((Run > Zeile) && ((Zeichen == EOF) || (Zeichen == '\n'))) Run--;
   if ((Run > Zeile) && (Run[-1] == 13)) Run--;
   *Run++ = '\0';
}

// Convert numeric constants: $ base 0x10 (16), % base 2, @ base 010 (8) or else base 10 by default.
// Input:
// ∙	inp:	Input string.
// ∙	erg:	Pointer to the LongInt result.
// Return: true ⇔ free of errors.
LongInt ConstLongInt(const char *inp, bool *err) {
// First, remove and process any signs, and then any base prefixes.
// The numeral is positive (+) and decimal (base 10) by default.
   LongInt vorz = *inp == '-'? -1: +1;
   if (*inp == '-' || *inp == '+') inp++;
   LongInt Base = *inp == '$'? 0x10: *inp == '@'? 010: *inp == '%'? 2: 10;
   if (Base != 10) inp++;
// Now, accumulate the total of the {hex/oct/b/dig}its into erg.
   LongInt erg = 0;
   *err = false;
   for (; *inp != '\0'; inp++) {
      LongInt Xit = *inp;
   // Digits '0'-'9' have values 0-9.
      if (Xit >= '0' && Xit <= '9') Xit -= '0';
   // Uppercase hexits 'A'-'F' have values 0xA-0xF.
      else if (Xit >= 'A' && Xit <= 'F') Xit -= 'A' - 0xA;
   // Lowercase hexits 'a'-'f' have values 0xa-0xf.
      else if (Xit >= 'a' && Xit <= 'f') Xit -= 'a' - 0xa;
   // Everything else, including an Xit out of the range [0,Base) permitted by the base, cuts the process short.
      else return vorz*erg;
   // else return erg; //(@) Formerly.
      if (Xit >= Base) return vorz*erg;
   // if (Xit >= Base) return erg; //(@) Formerly.
   // Shift, add and next.
      erg = Base*erg + Xit;
   }
// Add back on the sign.
   erg *= vorz;
   *err = true;
   return erg;
}

void stringutil_init(void) {
   HexLowerCase = false;
}
