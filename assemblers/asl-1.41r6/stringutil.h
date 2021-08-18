// AS-Portierung
// haeufig benoetigte String-Funktionen
extern bool HexLowerCase;

char *Blanks(int cnt);
char *HexString(LargeWord i, Byte Stellen);
char *HexBlankString(LargeWord i, Byte Stellen);
char *LargeString(LargeInt i);

#ifdef NEEDS_STRDUP
char *strdup(char *S);
#endif

#ifdef NEEDS_CASECMP
int strcasecmp(const char *src1, const char *src2);
int strncasecmp(const char *src1, const char *src2, int maxlen);
#endif

#ifdef NEEDS_STRSTR
char *strstr(char *haystack, char *needle);
#endif

// Working with unsigned results in bad side effects when doing arithmetic directly with strlen().
#undef strlen
#define strlen(S) ((signed int)strlen(S))
void strmaxcpy(char *Dest, const char *Src, int Max);
void strmaxcat(char *Dest, const char *Src, int MaxLen);
void strmove(char *S, int dS);
void strcopy(char *Dest, char *Src);
void stradd(char *Dest, char *Src);
void strprep(char *Dest, const char *Src);
void strmaxprep(char *Dest, const char *Src, int MaxLen);
void strins(char *Dest, const char *Src, int Pos);
void strmaxins(char *Dest, const char *Src, int Pos, int MaxLen);
void ReadLn(FILE * Datei, char *Zeile);
LongInt ConstLongInt(const char *inp, bool *err);
void stringutil_init(void);
