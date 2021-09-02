#include <stdio.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include "MicroNet.h"
#include "Com.h"
#include "Port.h"
#include "Console.h"

FILE *FP = NULL;

byte TestX = 2, AbortX = 2, StatusX = 0, DumpX = 0, DataX = 0, Cursor = 0;

void ShowTest(void) {
   char Ch;
   switch (TestX) {
      case 0: Ch = '1'; break;
      case 1: Ch = '2'; break;
      case 2: Ch = '*'; break;
   }
   PutString(2, 15, RedC, "%c", Ch);
}

void ShowAbort(void) {
   char Ch;
   switch (AbortX) {
      case 0: Ch = '1'; break;
      case 1: Ch = '2'; break;
      case 2: Ch = '*'; break;
   }
   PutString(2, 26, RedC, "%c", Ch);
}

void ShowStatus(void) {
   switch (StatusX) {
      case 0: PutString(2, 37, RedC, "1"); break;
      case 1: PutString(2, 37, RedC, "2"); break;
   }
}

void ShowDump(void) { PutString(4, 4, RedC, "%2d", DumpX + 1); }

void ShowData(void) { PutString(4, 15, RedC, "%2d", DataX + 1); }

void Screen(void) {
   ScrInit();
   PutString(0, 23, WhiteC, "C O M M A N D    I N T E R F A C E");
   PutString(2, 4, YellowC, "COM1");     PutString(2, 18, YellowC, "Test");
   PutString(2, 28, YellowC, "Abort");   PutString(2, 39, YellowC, "Status");
   PutString(4,  7, YellowC, "Dump");    PutString(4, 18, YellowC, "Data");
   Border(ROW(2, -4), ROW(3, 3), COL(15, -4), COL(16, 3), GreenC);
   Border(ROW(2, -4), ROW(3, 3), COL(26, -4), COL(27, 3), GreenC);
   Border(ROW(2, -4), ROW(3, 3), COL(37, -4), COL(38, 3), GreenC);
   Border(ROW(4, -4), ROW(5, 3), COL(4, -4), COL(6, 3), GreenC);
   Border(ROW(4, -4), ROW(5, 3), COL(15, -4), COL(17, 3), GreenC);
   ShowTest(), ShowAbort(), ShowStatus(), ShowDump(), ShowData(),
   PutString(4, 59, CyanC, "Line Status");
   PutString(5, 52, CyanC, "Overflow");
   PutString(6, 52, CyanC, "Overrun");
   PutString(7, 52, CyanC, "Parity");
   PutString(8, 52, CyanC, "Framing");
   PutString(5, 64, CyanC, "Timed Out");
   PutString(6, 64, CyanC, "Bad Header");
   PutString(7, 64, CyanC, "Bad Checksum");
   PutString(8, 64, CyanC, "Packet Full");
}

void Prompt(int Cursor, int On) {
   int Color = On? RedC: GreenC;
   switch (Cursor) {
      case 0: Border(ROW(2, -4), ROW(3, 3), COL(15, -4), COL(16, 3), Color); break;
      case 1: Border(ROW(2, -4), ROW(3, 3), COL(26, -4), COL(27, 3), Color); break;
      case 2: Border(ROW(2, -4), ROW(3, 3), COL(37, -4), COL(38, 3), Color); break;
      case 3: Border(ROW(4, -4), ROW(5, 3), COL(4, -4), COL(6, 3), Color); break;
      case 4: Border(ROW(4, -4), ROW(5, 3), COL(15, -4), COL(17, 3), Color); break;
   }
}

void ShowLine(void) {
   static int LastState = 0;
   if (LastState == Status) return; else LastState = Status;
   if (FP) fprintf(FP, "Status %2x\n", Status);
   Box(ROW(5, 0), ROW(6, -1), COL(50, 0), COL(51, -1), Status&BUFFER_FULL? RedC: BlackC);
   Box(ROW(6, 0), ROW(7, -1), COL(50, 0), COL(51, -1), Status&OVERRUN_ERR? RedC: BlackC);
   Box(ROW(7, 0), ROW(8, -1), COL(50, 0), COL(51, -1), Status&PARITY_ERR? RedC: BlackC);
   Box(ROW(8, 0), ROW(9, -1), COL(50, 0), COL(51, -1), Status&FRAMING_ERR? RedC: BlackC);
   Box(ROW(5, 0), ROW(6, -1), COL(62, 0), COL(63, -1), Status&TIMED_OUT? RedC: BlackC);
   Box(ROW(6, 0), ROW(7, -1), COL(62, 0), COL(63, -1), Status&BAD_HEADER? RedC: BlackC);
   Box(ROW(7, 0), ROW(8, -1), COL(62, 0), COL(63, -1), Status&BAD_SUM? RedC: BlackC);
   Box(ROW(8, 0), ROW(9, -1), COL(62, 0), COL(63, -1), Status&PACKET_OVER? RedC: BlackC);
   FlushPort();
}

unsigned long GetInt(byte *B, int Size) {
   unsigned long Val;
   for (Val = 0, B = B + Size - 1; Size > 0; Size--, B--) Val = (Val << 8) | *B;
   return Val;
}

double GetD(byte *B, int Size) {
   double Val;
   for (Val = 0.0, B = B + Size - 1; Size > 0; Size--, B--) Val = 256.0*Val + (double)*B;
   return Val;
}

#define OSC 921600.0
#define STAT_SIZE 0x17
#define X_SIZE 0x1540
static byte StatBuf[STAT_SIZE], XBuf[X_SIZE];

int main(void) {
   int Ch, Rx, Bytes, I; char Buf[80];
   Screen(); OpenPort(COM1, 9600, DATA8 | PAR_LOW | STOP1, 0x4000);
   Cursor = 0; Prompt(Cursor, 1);
   while (1) switch (Keyboard()) {
      case 3:
         if (FP != NULL) fclose(FP);
         ClosePort(); ScrReset();
      return 0;
      case 6: FlushPort(), ShowLine(); break;
      case 4: Box(ROW(10, 0), ROW(30, -1), COL(0, 0), COL(80, -1), BlackC); break;
      case Esc:
         if (FP != NULL) fclose(FP), Box(ROW(2, 0), ROW(3, -1), COL(46, 0), COL(80, -1), BlackC);
      break;
      case 12:
         PutString(2, 46, RedC, "Logging.  Filename: ");
         for (I = 0; ; ) switch (Ch = Keyboard()) {
            case '\b':
               if (I > 0) PutString(2, 66 + --I, BlackC, " ");
            break;
            case Esc:
               Box(ROW(2, 0), ROW(3, -1), COL(46, 0), COL(80, -1), BlackC);
            goto BreakEnter;
            case '\r':
               Buf[I] = '\0';
               FP = fopen(Buf, "w");
               if (FP == 0)
                  Box(ROW(2, 0), ROW(3, -1), COL(46, 0), COL(80, -1), BlackC);
            goto BreakEnter;
            case '\0': break;
            default:
               if (I < 14)
                  PutString(2, 66 + I, RedC, "%c", Buf[I] = Ch), I++;
            break;
         }
      BreakEnter:
      break;
      case Right: Prompt(Cursor, 0); Cursor = (Cursor + 1)%5; Prompt(Cursor, 1); break;
      case Left: Prompt(Cursor, 0); Cursor = (Cursor + 4)%5; Prompt(Cursor, 1); break;
      case Up: switch (Cursor) {
         case 0: TestX = (TestX + 1)%3; ShowTest(); break;
         case 1: AbortX = (AbortX + 1)%3; ShowAbort(); break;
         case 2: StatusX = (StatusX + 1)%2; ShowStatus(); break;
         case 3: DumpX = (DumpX + 1)%12; ShowDump(); break;
         case 4: DataX = (DataX + 1)%12; ShowData(); break;
      }
      break;
      case Down: switch (Cursor) {
         case 0: TestX = (TestX + 2)%3; ShowTest(); break;
         case 1: AbortX = (AbortX + 2)%3; ShowAbort(); break;
         case 2: StatusX = (StatusX + 1)%2; ShowStatus(); break;
         case 3: DumpX = (DumpX + 11)%12; ShowDump(); break;
         case 4: DataX = (DataX + 11)%12; ShowData(); break;
      }
      break;
      case '\r': switch (Cursor) {
         case 0:
            SendTest(TestX); if (FP != NULL) fprintf(FP, "TEST %c\n", Ch);
            ShowLine();
         break;
         case 1:
            FlushPort();
            SendAbort(AbortX); if (FP != NULL) fprintf(FP, "ABORT %c\n", Ch);
            ShowLine();
         break;
         case 2:
            Rx = SendStatus(StatusX);
            switch (Rx) {
               case '0': PutString(6, 35, WhiteC, " Active"); break;
               case '1': PutString(6, 35, WhiteC, "Waiting"); break;
               case '2': PutString(6, 35, WhiteC, "Testing"); break;
               default:  PutString(6, 35, WhiteC, "(%2x)  ", Rx); break;
            }
            if (FP != NULL) fprintf(FP, "STATUS %d => %c\n", StatusX + 1, Rx);
            ShowLine();
         break;
         case 3:
            Box(ROW(10, 0), ROW(30, -1), COL(0, 0), COL(80, -1), BlackC);
            if (FP != NULL) fprintf(FP, "DUMP %2d\n", DumpX + 1);
            Bytes = GetDump(DumpX, XBuf, X_SIZE);
            if (Status == 0) { /* DUMP */
              int Row, Col, I;
              for (Row = 10, Col = 0, I = 0; I < Bytes; I += 4) {
                 PutString(Row, Col, WhiteC, "%9.6f", GetD(XBuf + I, 4)/OSC);
                 if (FP != NULL) fprintf(FP, "%9.6f\n", GetD(XBuf + I, 4)/OSC);
                 if (++Row == 30) {
                    Row = 10, Col += 10;
                    if (Col == 80) {
                    Check:
                       switch (Keyboard()) {
                          case '\r': Col = 0, Box(ROW(10, 0), ROW(30, -1), COL(0, 0), COL(80, -1), BlackC); break;
                          case Esc: goto QuitDump;
                          default: goto Check;
                       }
                    }
                 }
              }
            }
         QuitDump:
            ShowLine();
         break;
         case 4:
            Box(ROW(10, 0), ROW(30, -1), COL(0, 0), COL(80, -1), BlackC);
            if (FP != NULL) fprintf(FP, "DATA %2d\n", DataX + 1);
            Bytes = GetStats(DataX, StatBuf, STAT_SIZE);
            if (Status == 0 && Bytes != STAT_SIZE) {
               PutString(10, 5, RedC, "Statistics ... packet error");
               if (FP != NULL) fprintf(FP, "Packet error.\n");
            } else if (Status == 0) { /* DATA */
               byte StatusS; word CyclesS;
               unsigned long TimeS, FirstS, LastS;
               double SquareS;
               StatusS = GetInt(StatBuf +  0, 1);
               CyclesS = GetInt(StatBuf +  1, 2);
               TimeS   = GetInt(StatBuf +  3, 4);
               FirstS  = GetInt(StatBuf +  7, 4);
               LastS   = GetInt(StatBuf + 11, 4);
               SquareS = GetD(StatBuf + 15, 8);
               PutString(10, 5, WhiteC, "Statistics");
               PutString(12, 5, WhiteC, "Status %2x", StatusS);
               PutString(14, 5, WhiteC, "Cycles %4d", CyclesS);
               PutString(16, 5, WhiteC, "First  %11.6f", (double)FirstS/OSC);
               PutString(18, 5, WhiteC, "Last   %11.6f", (double)LastS/OSC);
               PutString(20, 5, WhiteC, "Time   %11.6f", (double)TimeS/OSC);
               PutString(22, 5, WhiteC, "Square %11.6f", SquareS/OSC/OSC);
               if (FP != NULL) {
                  fprintf(FP, "Status %2x\n", StatusS);
                  fprintf(FP, "Cycles %4d\n", CyclesS);
                  fprintf(FP, "First  %11.6f\n", (double)FirstS/OSC);
                  fprintf(FP, "Last   %11.6f\n", (double)LastS/OSC);
                  fprintf(FP, "Time   %11.6f\n", (double)TimeS/OSC);
                  fprintf(FP, "Square %11.6f\n", SquareS/OSC/OSC);
               }
               PutString(10, 49, WhiteC, "Results");
               PutString(12, 49, WhiteC, "Test Time %11.6f", (double)TimeS/OSC);
               if (StatusS&0x40) {
                  PutString(14, 49, RedC, "Aborted.");
                  if (FP != NULL) fprintf(FP, "Aborted.\n");
               } else if (FirstS == LastS) {
                  PutString(14, 49, RedC, "No pulses.");
                  if (FP != NULL) fprintf(FP, "No pulses.\n");
               } else {
                  double Ave = (double)(LastS - FirstS)/(double)CyclesS, Ave2 = SquareS/(double)CyclesS;
                  PutString(14, 49, WhiteC, "Nutations %11.6f", (double)TimeS/Ave);
                  PutString(15, 52, WhiteC, "First Partial %11.6f", (double)FirstS/Ave);
                  PutString(16, 52, WhiteC, "Whole Cycles  %11.6f", (double)CyclesS);
                  PutString(17, 52, WhiteC, "Last Partial  %11.6f", (double)(TimeS - LastS)/Ave);
                  if (FP != NULL)
                     fprintf(FP, "Nutations: %11.6f (%11.6f + %11.6f + %11.6f)\n",
                        (double)TimeS/Ave, (double)FirstS/Ave, (double)CyclesS, (double)(TimeS - LastS)/Ave
                     );
                  if (Ave2 < Ave*Ave) {
                     PutString(19, 49, RedC, "Cannot calculate variation");
                     if (FP != NULL) fprintf(FP, "Cannot calculate variation.\n");
                  } else {
                     PutString(19, 49, WhiteC, "Variation %3.4f%%", sqrt(Ave2/(Ave*Ave) - 1.0)*100.0);
                     if (FP != NULL) fprintf(FP, "Variation %3.4f%%\n", sqrt(Ave2/(Ave*Ave) - 1.0)*100.0);
                  }
               }
            }
            ShowLine();
         break;
      }
      break;
   }
}
