#include <stdio.h>
#include <ctype.h>
#include <bios.h>
#include "Com.h"
#include "Port.h"

typedef unsigned char byte;
typedef unsigned int word;

#define Up    0x148
#define Left  0x14b
#define Right 0x14d
#define Down  0x150

int KeyHit(void) {
   return _bios_keybrd(_KEYBRD_READY);
}

int Keyboard(void) {
   int Ch = _bios_keybrd(_KEYBRD_READ);
   return (Ch&0xff) == '\0'? 0x100 | (Ch >> 8)&0xff: Ch&0xff;
}

main() {
   int Ch; FILE *FP = 0; char Buf[80];
   OpenPort(COM2, 9600, PAR_NONE | DATA8 | STOP1, 0x4000);
   while (1) {
      if (KeyHit()) switch (Ch = Keyboard()) {
         case 0x1b: if (FP != 0) fclose(FP), FP = 0; break;
         case Left: ClosePort(); exit(0);
         case Down:
            if (FP != 0) break;
            printf("Binary Load.  Filename: "); gets(Buf); putchar('\n');
            FP = fopen(Buf, "rb");
            if (FP == 0) {
               printf("Cannot find %s.\n", Buf); break;
            }
            while ((Ch = fgetc(FP)) != EOF) {
               Send(Ch&0xff);
               while ((Ch = Recv()) != -1) putchar(Ch);
            }
            fclose(FP); FP = 0;
         break;
         case Up:
            if (FP != 0) break;
            printf("Acsii Dump.  Filename: "); gets(Buf); putchar('\n');
            FP = fopen(Buf, "w");
            if (FP == 0) {
               printf("Cannot find %s.\n", Buf); break;
            }
         break;
         default: Send(Ch);
         case '\0': break;
      }
      while ((Ch = Recv()) != -1) {
         putchar(Ch);
         if (FP != 0) fputc(Ch, FP);
      }
   }
}
