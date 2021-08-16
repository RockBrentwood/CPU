/* stringutil.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* haeufig benoetigte String-Funktionen                                      */
/*                                                                           */
/* Historie:  5. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

extern bool HexLowerCase;

extern char *Blanks(int cnt);

extern char *HexString(LargeWord i, Byte Stellen);

extern char *HexBlankString(LargeWord i, Byte Stellen);

extern char *LargeString(LargeInt i);

#ifdef NEEDS_STRDUP
extern char *strdup(char *S);
#endif

#ifdef NEEDS_CASECMP
extern int strcasecmp(const char *src1, const char *src2);
extern int strncasecmp(const char *src1, const char *src2, int maxlen);
#endif

#ifdef NEEDS_STRSTR
extern char *strstr(char *haystack, char *needle);
#endif

// Working with unsigned results in bad side effects when doing arithmetic directly with strlen().
#undef strlen
#define strlen(S) ((signed int)strlen(S))
extern void strmaxcpy(char *Dest, const char *Src, int Max);
extern void strmaxcat(char *Dest, const char *Src, int MaxLen);
extern void strmove(char *S, int dS);
extern void strcopy(char *Dest, char *Src);
extern void stradd(char *Dest, char *Src);
extern void strprep(char *Dest, const char *Src);
extern void strmaxprep(char *Dest, const char *Src, int MaxLen);
extern void strins(char *Dest, const char *Src, int Pos);
extern void strmaxins(char *Dest, const char *Src, int Pos, int MaxLen);

extern void ReadLn(FILE * Datei, char *Zeile);

extern LongInt ConstLongInt(const char *inp, bool *err);

extern void stringutil_init(void);
