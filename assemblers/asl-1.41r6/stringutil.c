/* stringutil.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* haeufig benoetigte String-Funktionen                                      */
/*                                                                           */
/* Historie:  5. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "stringutil.h"
#undef strlen   /* VORSICHT, Rekursion!!! */

bool HexLowerCase;	    /* Hex-Konstanten mit Kleinbuchstaben */

/*--------------------------------------------------------------------------*/
/* eine bestimmte Anzahl Leerzeichen liefern */

        char *Blanks(int cnt)
{
   static char *BlkStr="                                                                                                           ";

   if (cnt<0) cnt=0;

   return BlkStr+(strlen(BlkStr)-cnt);
}


/****************************************************************************/
/* eine Integerzahl in eine Hexstring umsetzen. Weitere vordere Stellen als */
/* Nullen */

#define BufferCnt 8

        char *HexString(LargeWord i, Byte Stellen)
{
   static char *UDigitVals="0123456789ABCDEF",*LDigitVals="0123456789abcdef",*ptr;
   static String h[BufferCnt];
   static int z=0;

   if (Stellen>8) Stellen=8;
   h[z][0]='\0';
   do
    {
     memmove(h[z]+1,h[z],strlen(h[z])+1);
     h[z][0]=(HexLowerCase)?(LDigitVals[i&15]):(UDigitVals[i&15]);
     i>>=4;
    }
   while ((i!=0) || (strlen(h[z])<Stellen));
   ptr=h[z];
   z=(z+1)%BufferCnt;
   return ptr;
}


/*--------------------------------------------------------------------------*/
/* dito, nur vorne Leerzeichen */

        char *HexBlankString(LargeWord i, Byte Stellen)
{
   static String temp;

   strmaxcpy(temp,HexString(i,0),255);
   if (strlen(temp)<Stellen) strmaxprep(temp,Blanks(Stellen-strlen(temp)),255);
   return temp;
}

/*---------------------------------------------------------------------------*/
/* Da manche Systeme (SunOS) Probleme mit der Asugabe extra langer Ints
   haben, machen wir das jetzt zu Fuss :-( */

	char *LargeString(LargeInt i)
{
   bool SignFlag=false;
   static String s;
   String tmp;
   char *p,*p2;

   if (i<0)
    {
     i=(-i); SignFlag=true;
    }

   p=tmp;
   do
    {
     *(p++)='0'+(i%10);
     i/=10;
    }
   while (i>0);

   p2=s; if (SignFlag) *(p2++)='-';
   while (p>tmp) *(p2++)=(*(--p));
   *p2='\0'; return s;
}


/*---------------------------------------------------------------------------*/
/* manche haben's nicht... */

#ifdef NEEDS_STRDUP
	char *strdup(char *s)
{
   char *ptr=malloc(strlen(s)+1);
   if (ptr!=0) strcpy(ptr,s);
   return ptr;
}
#endif

#ifdef NEEDS_CASECMP
	int strcasecmp(const char *src1, const char *src2)
{
   while (toupper(*src1)==toupper(*src2))
    {
     if ((! *src1) && (! *src2)) return 0;
     src1++; src2++;
    }
   return ((int) toupper(*src1))-((int) toupper(*src2));
}

	int strncasecmp(const char *src1, const char *src2, int len)
{
   while (toupper(*src1)==toupper(*src2))
    {
     if (--len==0) return 0;
     if ((! *src1) && (! *src2)) return 0;
     src1++; src2++;
    }
   return ((int) toupper(*src1))-((int) toupper(*src2));
}
#endif

#ifdef NEEDS_STRSTR
	char *strstr(char *haystack, char *needle)
{
   int lh=strlen(haystack), ln=strlen(needle);
   int z;
   char *p;

   for (z=0; z<=lh-ln; z++)
    if (strncmp(p=haystack+z,needle,ln)==0) return p;
   return NULL;
}
#endif

/*---------------------------------------------------------------------------*/
/* wenn man mit unsigned arbeitet, gibt das boese Seiteneffekte bei direkter
   Arithmetik mit strlen... */

	signed int strslen(const char *s)
{
   return strlen(s);
}

/*---------------------------------------------------------------------------*/
/* das originale strncpy plaettet alle ueberstehenden Zeichen mit Nullen */

	void strmaxcpy(char *dest, const char *src, int Max)
{
   int cnt=strlen(src);

   if (cnt>Max) cnt=Max;
   memcpy(dest,src,cnt); dest[cnt]='\0';
}

/*---------------------------------------------------------------------------*/
/* einfuegen, mit Begrenzung */

	void strmaxcat(char *Dest, const char *Src, int MaxLen)
{
   int TLen=strlen(Src),DLen=strlen(Dest);

   if (TLen>MaxLen-DLen) TLen=MaxLen-DLen;
   if (TLen>0)
    {
     memcpy(Dest+DLen,Src,TLen);
     Dest[DLen+TLen]='\0';
    }
}

	void strprep(char *Dest, const char *Src)
{
   memmove(Dest+strlen(Src),Dest,strlen(Dest)+1);
   memmove(Dest,Src,strlen(Src));
}

	void strmaxprep(char *Dest, const char *Src, int MaxLen)
{
   int RLen;

   RLen=strlen(Src); if (RLen>MaxLen-strlen(Dest)) RLen=MaxLen-strlen(Dest);
   memmove(Dest+RLen,Dest,strlen(Dest)+1);
   memmove(Dest,Src,RLen);
}

	void strins(char *Dest, const char *Src, int Pos)
{
   memmove(Dest+Pos+strlen(Src),Dest+Pos,strlen(Dest)+1-Pos);
   memmove(Dest+Pos,Src,strlen(Src));
}

	void strmaxins(char *Dest, const char *Src, int Pos, int MaxLen)
{
   int RLen;

   RLen=strlen(Src); if (RLen>MaxLen-strlen(Dest)) RLen=MaxLen-strlen(Dest);
   memmove(Dest+Pos+RLen,Dest+Pos,strlen(Dest)+1-Pos);
   memmove(Dest+Pos,Src,RLen);
}

/*---------------------------------------------------------------------------*/
/* Bis Zeilenende lesen */

        void ReadLn(FILE *Datei, char *Zeile)
{
   int Zeichen='\0';
   char *Run=Zeile;

   while ((Zeichen!='\n') && (Zeichen!=EOF) && (!feof(Datei)))
    {
     Zeichen=fgetc(Datei);
     if (Zeichen!=26) *Run++=Zeichen;
    }

   if ((Run>Zeile) && ((Zeichen==EOF) || (Zeichen=='\n'))) Run--;
   if ((Run>Zeile) && (Run[-1]==13)) Run--;
   *Run++='\0';
}

/*--------------------------------------------------------------------*/
/* Zahlenkonstante umsetzen: $ hex, % binaer, @ oktal */
/* inp: Eingabezeichenkette */
/* erg: Zeiger auf Ergebnis-Longint */
/* liefert true, falls fehlerfrei, sonst false */

        LongInt ConstLongInt(const char *inp, bool *err)
{
   static char Prefixes[4]={'$','@','%','\0'}; /* die moeglichen Zahlensysteme */
   static LongInt Bases[3]={16,8,2};           /* die dazugehoerigen Basen */
   LongInt erg;
   LongInt Base=10,z,val,vorz=1;  /* Vermischtes */

   /* eventuelles Vorzeichen abspalten */

   if (*inp=='-')
    {
     vorz=(-1); inp++;
    }


   /* Jetzt das Zahlensystem feststellen.  Vorgabe ist dezimal, was
      sich aber durch den Initialwert von Base jederzeit aendern
      laesst.  Der break-Befehl verhindert, dass mehrere Basenzeichen
      hintereinander eingegeben werden koennen */

   for (z=0; z<3; z++)
    if (*inp==Prefixes[z])
     {
      Base=Bases[z]; inp++; break;
     }

   /* jetzt die Zahlenzeichen der Reihe nach durchverwursten */

   erg=0; *err=false;
   for(; *inp; inp++)
    {
     /* Ziffern 0..9 ergeben selbiges */

     if ((*inp>='0') && (*inp<='9')) val=(*inp)-'0';

     /* Grossbuchstaben fuer Hexziffern */

     else if ((*inp>='A') && (*inp<='F')) val=(*inp)-'A'+10;

     /* Kleinbuchstaben nicht vergessen...! */

     else if ((*inp>='a') && (*inp<='f')) val=(*inp)-'a'+10;

     /* alles andere ist Schrott */

     else return erg;

     /* entsprechend der Basis zulaessige Ziffer ? */

     if (val>=Base) return erg;

     /* Zahl linksschieben, zusammenfassen, naechster bitte */

     erg=erg*Base+val;
    }

   /* Vorzeichen beruecksichtigen */

   erg*=vorz;

   *err=true; return erg;
}

	void stringutil_init(void)
{
   HexLowerCase=false;
}
