// AS-Portierung
// Haeufiger benutzte Pseudo-Befehle
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

int FindInst(void *Field, int Size, int Count);
bool IsIndirect(char *Asc);
void ConvertDec(Double F, Word * w);
bool DecodeIntelPseudo(bool Turn);
bool DecodeMotoPseudo(bool Turn);
bool DecodeMoto16Pseudo(ShortInt OpSize, bool Turn);
void CodeEquate(ShortInt DestSeg, LargeInt Min, LargeInt Max);
void CodeASSUME(ASSUMERec * Def, Integer Cnt);
bool CodeONOFF(ONOFFRec * Def, Integer Cnt);
void codepseudo_init(void);
