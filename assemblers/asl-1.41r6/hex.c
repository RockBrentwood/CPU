// AS-Portierung
// Dezimal-->Hexadezimal-Wandlung, Grossbuchstaben
#include "stdinc.h"
#include <ctype.h>

#include "hex.h"

#define BUFFERCNT 8

char *HexNibble(Byte inp) {
   static char Buffers[BUFFERCNT][3], *ret;
   static int z = 0;

   sprintf(Buffers[z], "%01x", inp & 0xf);
   for (ret = Buffers[z]; *ret != '\0'; ret++) *ret = toupper(*ret);
   ret = Buffers[z];
   z = (z + 1) % BUFFERCNT;
   return ret;
}

char *HexByte(Byte inp) {
   static char Buffers[BUFFERCNT][4], *ret;
   static int z = 0;

   sprintf(Buffers[z], "%02x", inp & 0xff);
   for (ret = Buffers[z]; *ret != '\0'; ret++) *ret = toupper(*ret);
   ret = Buffers[z];
   z = (z + 1) % BUFFERCNT;
   return ret;
}

char *HexWord(Word inp) {
   static char Buffers[BUFFERCNT][6], *ret;
   static int z = 0;

   sprintf(Buffers[z], "%04x", inp & 0xffff);
   for (ret = Buffers[z]; *ret != '\0'; ret++) *ret = toupper(*ret);
   ret = Buffers[z];
   z = (z + 1) % BUFFERCNT;
   return ret;
}

char *HexLong(LongWord inp) {
   static char Buffers[BUFFERCNT][10], *ret;
   static int z = 0;

   sprintf(Buffers[z], "%08x", inp & 0xffffffffu);
   for (ret = Buffers[z]; *ret != '\0'; ret++) *ret = toupper(*ret);
   ret = Buffers[z];
   z = (z + 1) % BUFFERCNT;
   return ret;
}

void hex_init(void) {
}
