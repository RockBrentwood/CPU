// AS-Portierung
// Verarbeitung Kommandozeilenparameter
typedef enum { CMDErr, CMDFile, CMDOK, CMDArg } CMDResult;

typedef CMDResult(*CMDCallback)(bool NegFlag, char *Arg);

typedef void (*CMDErrCallback)(bool InEnv, char *Arg);

typedef struct {
   char Ident[11];
   CMDCallback Callback;
} CMDRec;

#define MAXPARAM 256
typedef bool CMDProcessed[MAXPARAM + 1];

extern LongInt ParamCount;
extern char **ParamStr;

bool ProcessedEmpty(CMDProcessed Processed);
void ProcessCMD(CMDRec * Def, Integer Cnt, CMDProcessed Unprocessed, char *EnvName, CMDErrCallback ErrProc);
char *GetEXEName(void);
void decodecmd_init(void);
