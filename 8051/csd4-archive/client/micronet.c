#include <stdio.h>
#include <bios.h>
#include <dos.h>
#include <math.h>
#include <float.h>
#include "Port.h"
#include "Com.h"
#include "Micronet.h"

void SendCmd(int Addr, int Tx) {
   SetTx();
   Send((Addr << 6)&0300 | Tx&077);
   SetRx();
}

int GetStats(int Counter, byte *Buf, int Bytes) {
   int Header, Size, I, Rx, Sum;
   int Addr = (Counter >= 6)? 2: 1;
   SendCmd(Addr, Counter%6); if (Status) return -1;
   if ((Header = Recv()) == -1) return -1;
   if (Header != '#') { Status |= BAD_HEADER; return -1; }
   if ((Size = Recv()) == -1) return -1;
   if (Size == 0) Size = 0x100;
   for (I = 0, Sum = 0; I < Size; I++) {
      if ((Rx = Recv()) == -1) return -1;
      if (I >= Bytes) { Status |= PACKET_OVER; return -1; }
      Buf[I] = (byte)Rx;
      Sum = (Sum + Rx)&0xff;
   }
   if ((Rx = Recv()) == -1) return -1;
   if (Rx != Sum) Status |= BAD_SUM;
   return I;
}

#define NAK   033
#define ACK   034
#define ABORT 037
int GetDump(int Counter, byte *Buf, int Bytes) {
   int Header, Size, Bot, Top, Rx, Sum, Retries;
   int Addr = (Counter >= 6)? 2: 1;
   SendCmd(Addr, 010 | (Counter%6));
   Bot = 0;
NextPacket:
   Retries = 0;
RecvPacket:
   if (Status) return -1;
   if ((Header = Recv()) == -1) return -1;
   switch (Header) {
      case '.': return Bot;
      case ':': break;
      default: Status |= BAD_HEADER; return -1;
   }
   if ((Size = Recv()) == -1) return -1;
   if (Size == 0) Size = 0x100;
   for (Top = Bot, Sum = 0; Top < Bot + Size; Top++) {
      if ((Rx = Recv()) == -1) return -1;
      if (Top >= Bytes) { Status |= PACKET_OVER; return -1; }
      Buf[Top] = (byte)Rx; Sum = (Sum + Rx)&0xff;
   }
   if ((Rx = Recv()) == -1) return -1;
   if (Rx != Sum) {
      if (++Retries == 10) {
         FlushPort(); SendCmd(Addr, ABORT); Status |= BAD_SUM; return -1;
      }
      SendCmd(Addr, NAK); Top = Bot; goto RecvPacket;
   } else {
      SendCmd(Addr, ACK); Bot = Top; goto NextPacket;
   }
}

int SendStatus(int Unit) {
   int Rx;
   switch (Unit) {
      case 0: SendCmd(1, 020); break;
      case 1: SendCmd(2, 020); break;
   }
   if (Status) return -1;
   Rx = Recv(); if (Status) return -1;
   return Rx;
}

void SendAbort(int Unit) {
   FlushPort();
   switch (Unit) {
      case 0:  SendCmd(1, ABORT); break;
      case 1:  SendCmd(2, ABORT); break;
      default: SendCmd(3, ABORT); break;
   }
} 
void SendTest(int Unit) {
   switch (Unit) {
      case 0:  SendCmd(1, 030); break;
      case 1:  SendCmd(2, 030); break;
      default: SendCmd(3, 030); break;
   }
}
