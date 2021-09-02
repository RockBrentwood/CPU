#include <stdlib.h>
#include <dos.h>
#include <bios.h>
#include "Port.h"
#include "Com.h"

typedef unsigned char byte;
int Status;
static byte IntNum, IntBit;
static unsigned Base;
static byte *Buffer; static int Back, Front, Bytes, BufSize;

#pragma check_stack(off)
#pragma check_pointer(off)

static void (interrupt far *OldSer)(void);

static void interrupt far NewSer(void) {
   int Ch; byte IntStatus;
   while ((IntStatus = inp(Base + IIR)) != INT_OFF) {
      switch (IntStatus & INT_ID) {
         case LSR_ID: Status |= inp(Base + LSR) & LINE_ERR; break;
         case RxD_ID:
            Ch = inp(Base + RxD);
            if (Bytes == 0) { Status |= BUFFER_FULL; break; }
            Buffer[Back++] = Ch; if (Back == BufSize) Back = 0;
            Bytes--;
         break;
      }
   }
   outp(IntControl, EOI);
}

int Recv(void) {
   int Ch; int I;
/* Wait for reception */
   for (I = 0; Bytes == BufSize; I++)
      if (I == 0x400) { Status |= TIMED_OUT; return -1; }
/* Unbuffer it */
   _disable();
   Ch = Buffer[Front++]; if (Front == BufSize) Front = 0;
   Bytes++;
   _enable();
   return Ch;
}

void Send(int Tx) {
   int I;
   outp(Base + TxD, Tx);
   for (I = 0; (inp(Base + LSR)&TX_DONE) != TX_DONE; I++)
      if (I == 0x400) { Status |= TIMED_OUT; break; }
}

void SetTx(void) {
   outp(Base + MCR, inp(Base + MCR) | OUT1);
   outp(Base + LCR, inp(Base + LCR)&~PAR | PAR_HIGH);
}

void SetRx(void) {
   outp(Base + LCR, inp(Base + LCR)&~PAR | PAR_LOW);
   outp(Base + MCR, inp(Base + MCR) & ~OUT1);
}

int OpenPort(int Net, unsigned Baud, char Line, unsigned Size) {
   unsigned Divisor = FREQ/Baud;
   Buffer = (byte *)malloc(BufSize = Size);
   if (Buffer == 0) return 0;
   _disable();
   switch (Net) {
      case COM1: Base = 0x3f8, IntNum = 0x0c, IntBit = IRQ4; break;
      case COM2: Base = 0x2f8, IntNum = 0x0b, IntBit = IRQ3; break;
      case COM3: Base = 0x3e8, IntNum = 0x0c, IntBit = IRQ4; break;
      case COM4: Base = 0x2e8, IntNum = 0x0b, IntBit = IRQ3; break;
   }
   OldSer = _dos_getvect(IntNum), _dos_setvect(IntNum, NewSer);
   outp(IntMask, inp(IntMask) & ~IntBit); /* Com ports on */
/* Turn on the ports */
   Status = 0;
   outp(Base + LCR, DLAB);
   outp(Base + DLL, Divisor&0xff); outp(Base + DLH, Divisor >> 8);
   outp(Base + LCR, Line);
   Back = Front = 0; Bytes = BufSize;
   outp(Base + MCR, OUT2 | RTS | DTR);
   outp(Base + IER, RxD_INT | LSR_INT);
   (void)inp(Base + RxD); (void)inp(Base + LSR);
   _enable();
   return 1;
}

void ClosePort(void) {
   _disable();
   outp(Base + IER, 0);
   outp(IntMask, inp(IntMask) | IntBit); /* Com port off */
   _dos_setvect(IntNum, OldSer);
   _enable();
   free(Buffer);
}

void FlushPort(void) {
   while (Recv() >= 0);
   Status = 0; Front = Back = 0; Bytes = BufSize;
}
