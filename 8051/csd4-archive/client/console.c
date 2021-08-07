#include <stdarg.h>
#include <graph.h>
#include <bios.h>
#include <dos.h>
#include <stdio.h>

void Box(int N, int S, int W, int E, int Hue) {
   _setcolor(Hue); _rectangle(_GFILLINTERIOR, W, N, E, S);
}

void Border(int N, int S, int W, int E, int Hue) {
   _setcolor(Hue); _rectangle(_GBORDER, W, N, E, S);
}

void PutString(int N, int W, int Hue, const char *Format, ...) {
   static char Buf[80];
   va_list AP;
   va_start(AP, Format);
   _settextcolor(Hue); _settextposition(N + 1, W + 1);
   vsprintf(Buf, Format, AP); _outtext(Buf);
   va_end(AP);
}

void ScrInit(void) {
   _clearscreen(_GCLEARSCREEN);
   if (_setvideomode(_VRES16COLOR) != 30)
      printf("30 line graphic mode not available on this machine.\n"), exit(1);
}

void ScrReset(void) {
   _setvideomode(_DEFAULTMODE); _clearscreen(_GCLEARSCREEN);
}

int KeyHit(void) {
   return _bios_keybrd(_KEYBRD_READY);
}

int Keyboard(void) {
   int Ch = _bios_keybrd(_KEYBRD_READ);
   return (Ch&0xff) == '\0'? 0x100 | (Ch >> 8)&0xff: Ch&0xff;
}
