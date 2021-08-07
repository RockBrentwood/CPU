extern void OpInit(void);
extern void ParseArgs(byte Mnem);

typedef struct { byte OpCode; char *X, *Y; } Mode;
typedef struct { char *Name; int Modes; Mode *Start; } Code;
extern Code CodeTab[];
