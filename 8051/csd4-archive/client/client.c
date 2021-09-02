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

int KeyHit(void) { return _bios_keybrd(_KEYBRD_READY); }

int Keyboard(void) {
   int Ch = _bios_keybrd(_KEYBRD_READ);
   return (Ch&0xff) == '\0'? 0x100 | (Ch >> 8)&0xff: Ch&0xff;
}

int main(void) {
   int Ch; FILE *FP = NULL; char Buf[80];
   OpenPort(COM2, 9600, PAR_NONE | DATA8 | STOP1, 0x4000);
   while (1) {
      if (KeyHit()) switch (Ch = Keyboard()) {
         case 0x1b: if (FP != NULL) fclose(FP), FP = NULL; break;
         case Left: ClosePort(); return 0;
         case Down:
            if (FP != NULL) break;
            printf("Binary Load.  Filename: "); gets(Buf); putchar('\n');
            FP = fopen(Buf, "rb");
            if (FP == NULL) { printf("Cannot find %s.\n", Buf); break; }
            while ((Ch = fgetc(FP)) != EOF) {
               Send(Ch&0xff);
               while ((Ch = Recv()) != -1) putchar(Ch);
            }
            fclose(FP); FP = NULL;
         break;
         case Up:
            if (FP != NULL) break;
            printf("Acsii Dump.  Filename: "); gets(Buf); putchar('\n');
            FP = fopen(Buf, "w");
            if (FP == NULL) { printf("Cannot find %s.\n", Buf); break; }
         break;
         default: Send(Ch);
         case '\0': break;
      }
      while ((Ch = Recv()) != -1) {
         putchar(Ch);
         if (FP != NULL) fputc(Ch, FP);
      }
   }
}
