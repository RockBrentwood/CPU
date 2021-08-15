#include "sysdefs.h"
#include <stdbool.h>

#define MaxLongInt 0x7fffffff

typedef Card8 Byte; /* Integertypen */
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

typedef char String[256];
