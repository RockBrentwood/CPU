/* codepseudo.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Pseudo-Befehle                                         */
/*                                                                           */
/* Historie: 23. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

typedef struct {
   char *Name;
   LongInt *Dest;
   LongInt Min, Max;
   LongInt NothingVal;
} ASSUMERec;

typedef struct {
   char *Name;
   bool *Dest;
   char *FlagName;
} ONOFFRec;

extern int FindInst(void *Field, int Size, int Count);

extern bool IsIndirect(char *Asc);

extern void ConvertDec(Double F, Word * w);

extern bool DecodeIntelPseudo(bool Turn);

extern bool DecodeMotoPseudo(bool Turn);

extern bool DecodeMoto16Pseudo(ShortInt OpSize, bool Turn);

extern void CodeEquate(ShortInt DestSeg, LargeInt Min, LargeInt Max);

extern void CodeASSUME(ASSUMERec * Def, Integer Cnt);

extern bool CodeONOFF(ONOFFRec * Def, Integer Cnt);

extern void codepseudo_init(void);
