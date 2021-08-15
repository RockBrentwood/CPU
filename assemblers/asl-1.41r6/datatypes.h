#include "sysdefs.h"

typedef Card8 Byte;       /* Integertypen */
typedef Integ8 ShortInt;

#ifdef HAS16
typedef Card16 Word;
typedef Integ16 Integer;
#endif

typedef Card32 LongWord;
typedef Integ32 LongInt;

#ifdef HAS64
typedef Card64 QuadWord;
typedef Integ64 QuadInt;
#endif

#ifdef HAS64
typedef QuadInt LargeInt;
typedef QuadWord LargeWord;
#else
typedef LongInt LargeInt;
typedef LongWord LargeWord;
#endif

typedef char Char;

typedef double Double;
typedef float Single;

typedef Byte Boolean; 

typedef char String[256];

#ifndef TRUE
#define TRUE 1
#endif
#ifndef True
#define True 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef False
#define False 0
#endif

