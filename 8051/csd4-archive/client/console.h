#define BlackC   0
#define BrownC   6
#define BlueC    9
#define GreenC  10
#define CyanC   11
#define RedC    12
#define PinkC   13
#define YellowC 14
#define WhiteC  15

typedef unsigned char byte;
typedef unsigned int word;

/* Defined for high resolution VGA */
#define ROW(Base, Offset) (16*(Base) + Offset)
#define COL(Base, Offset) (8*(Base) + Offset)

extern void Box(int N, int S, int W, int E, int Hue);
extern void Border(int N, int S, int W, int E, int Hue);
extern void PutString(int N, int W, int Hue, const char *Format, ...);
extern void ScrInit(void);
extern void ScrReset(void);

#define Esc   0x01b
#define Up    0x148
#define Left  0x14b
#define Right 0x14d
#define Down  0x150

extern int KeyHit(void);
extern int Keyboard(void);
