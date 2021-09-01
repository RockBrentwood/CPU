include "8051fa.h"
include "kernel.h"    ;; This MUST be the first module included.
include "math.h"
include "stdio.h"

;; Timing
seg data
global CTIME: ds 2
global TIME:  ds 2

seg code
SetCEX:
   mov CMOD, #ECF
   mov CCAPM0, #(CAPN + ECCF)
   mov CCAPM1, #(CAPN + ECCF)
   mov CCAPM2, #(CAPN + ECCF)
   mov CCAPM3, #(CAPN + ECCF)
   mov CCAPM4, #(CAPN + ECCF)   ;; Set CEX inputs for falling edge mode.
   setb CEX0
   setb CEX1
   setb CEX2
   setb CEX3
   setb CEX4                    ;; Latch CEX inputs for input.
   clr PPC                      ;; Set interrupt for low priority.
ret

SetT2EX:
   setb T2EX
   setb CP_RL2
   setb EXEN2  ;; Enable captures in T2EX.
   clr C_T2    ;; Set TIMER2 as a timer.
   clr PT2     ;; And set its interrupt priority low.
ret

SetTF0:
   mov A, TMOD
   anl A, #11110000b
   add A, #01h      ;; Timer 0: 16-bit internal timer.
   mov TMOD, A
   clr PT0          ;; Set for low priority.
ret

SetCClock:
   mov CL, #0
   mov CH, #0
   mov CTIME, #0
   mov (CTIME + 1), #0 ;; CTIME = 0;
   clr CF
   setb EC
   setb CR             ;; CLOCK ON;
1:                     ;; while (1) {
ret                    ;;    "pause on CF";
global CTick:
   inc CTIME
   mov A, CTIME
   jnz 2f
      inc (CTIME + 1)
   2:                  ;;    CTIME++;
sjmp 1b                ;; }

SetClock:
   mov TL2, #0
   mov TH2, #0
   mov TIME, #0
   mov (TIME + 1), #0 ;; TIME = 0;
   clr TF2
   setb ET2
   setb TR2           ;; CLOCK ON;
1:                    ;; while (1) {
ret
global Tick:          ;;    "pause on TF2";
   inc TIME
   mov A, TIME
   jnz 2f
      inc (TIME + 1)
   2:                 ;;    TIME++;
sjmp 1b               ;; }

Capture: ;; "capture the time of the last sensor";
   mov (TIME0 + 0), TL2
   mov (TIME0 + 1), TH2
   mov (TIME0 + 2), (TIME + 0)
   mov (TIME0 + 3), (TIME + 1)
   mov (TIME1 + 0), CL
   mov (TIME1 + 1), CH
   mov (TIME1 + 2), (CTIME + 0)
   mov (TIME1 + 3), (CTIME + 1)
   mov (TIME2 + 0), CL
   mov (TIME2 + 1), CH
   mov (TIME2 + 2), (CTIME + 0)
   mov (TIME2 + 3), (CTIME + 1)
   mov (TIME3 + 0), CL
   mov (TIME3 + 1), CH
   mov (TIME3 + 2), (CTIME + 0)
   mov (TIME3 + 3), (CTIME + 1)
   mov (TIME4 + 0), CL
   mov (TIME4 + 1), CH
   mov (TIME4 + 2), (CTIME + 0)
   mov (TIME4 + 3), (CTIME + 1)
   mov (TIME5 + 0), CL
   mov (TIME5 + 1), CH
   mov (TIME5 + 2), (CTIME + 0)
   mov (TIME5 + 3), (CTIME + 1)
ret

ARR_BASE: dw 0080h, 15c0h, 2b00h, 3f40h, 5580h, 6ac0h
GetBase:
   mov A, Counter
   rl A
   mov B, A
   mov DPTR, #ARR_BASE
   movc A, @A + DPTR
   xch A, B              ;; B = ARR_BASE[2*Counter]
   inc A
   movc A, @A + DPTR     ;; A = ARR_BASE[2*Counter + 1];
   mov Bot, A
   mov (Bot + 1), B      ;; Bot = ((word *)ARR_BASE)[Counter];
ret
GetSize:
   mov A, Counter
   rl A
   add A, #Cycles
   mov R0, A
   mov A, @R0
   mov B, R2
   mul AB
   mov Size, A
   mov (Size + 1), B
   inc R0
   mov A, @R0
   mov B, R2
   mul AB
   add A, (Size + 1)
   xch A, (Size + 1)  ;; Size = R2 * ((word *)Cycles)[Counter];
ret

Store:
   mov A, Bot
   add A, Size
   mov DPL, A
   mov A, (Bot + 1)
   addc A, (Size + 1)
   mov DPH, A             ;; for (DPTR = Bot + Size; R2 > 0; R2--) {
   1:
      mov A, @R0
      inc R0
      movx @DPTR, A
      inc DPTR            ;;    *DPTR++ = R0++;
   djnz R2, 1b            ;; }
ret

;; All the statistical variables for the 6 inputs are replicated 6 times.
SECTORS equ 20
seg data
global Sector0: ds 1
global Sector1: ds 1
global Sector2: ds 1
global Sector3: ds 1
global Sector4: ds 1
global Sector5: ds 1

seg data
StatSeg:
global TIME0: ds 4
global TIME1: ds 4
global TIME2: ds 4
global TIME3: ds 4
global TIME4: ds 4
global TIME5: ds 4
Cycles:  ds 2*6
First:   ds 4*6
Last:    ds 4*6
Square:  ds 8*6
STAT_SIZE equ $ - StatSeg

seg data at 0x30
Width:  ds 4
Bot:    ds 2
Size:   ds 2

seg code
SetStats:
   mov R0, #Sector0
   mov R2, #6
   1:
      mov @R0, #1
      inc R0
   djnz R2, 1b           ;; Set all sector counts to 0;
   mov State, #00111111b ;; Set all FXn to 1, and clear the abort flag FXX.
   mov R0, #StatSeg
   mov R2, #STAT_SIZE
   acall ClearX          ;; Clear all statistical registers;
ret
global Partial:          ;; "pause on PULSE";
   mov A, Counter; rl A; rl A; add A, #First
   mov R0, A
   add A, #(TIME0 - First)
   mov R1, A
   mov R2, #4
   acall CopyX           ;;    First = TIME;
StatsLoop:               ;;    while (1) {
   mov A, Counter; rl A; rl A; add A, #Last
   mov R0, A
   add A, #(TIME0 - Last)
   mov R1, A
   mov R2, #4
   acall CopyX           ;;       Last = TIME;
ret
global Nutation:         ;;       "pause on PULSE";
   mov R0, #Width
   mov A, Counter; rl A; rl A; add A, #TIME0
   mov R1, A
   mov R2, #4
   acall CopyX           ;;      Width = TIME;
   mov R0, #Width
   mov A, Counter; rl A; rl A; add A, #Last
   mov R1, A
   mov R2, #4
   acall SubtractX       ;;      Width -= Last;
   acall GetBase         ;;      Bot = ARR_BASE;
   mov R2, #4
   acall GetSize         ;;      Size = 4*Cycles;
   mov R0, #Width
   mov R2, #4
   acall Store           ;;      Bot[Size] = Width;
   mov A, Counter; rl A; add A, #Cycles
   mov R0, A
   mov R2, #2
   acall IncX            ;;      Cycles++;
   mov ArgX, #Width
   mov SizeX, #4
   mov ArgY, #Width
   mov SizeY, #4
   mov A, Counter; rl A; rl A; rl A; add A, #Square
   mov ArgZ, A
   acall MultiplyX       ;;      Square += Width*Width;
sjmp StatsLoop           ;; }

SetIE0:
   setb INT0 ;; Latch INT0 for input.
   setb IT0
   clr PX0   ;; And set its interrupt priority low (should be high).
ret

ClearFlags:
   clr EXF2
   clr CCF0
   clr CCF1
   clr CCF2
   clr CCF3
   clr CCF4
   mov TSTATUS, #0
   mov CSTATUS, #0
ret

SetTest:
   acall SetIE0          ;; Enable IE0;
   acall SetT2EX         ;; Enable TF2, EXF2;
   acall SetCEX          ;; Enable CF, CCFn;
   acall SetTF0          ;; Enable process scheduler.
   mov State, #01000000b ;; Set abort flag, clear all others.
ret
TestCycle:
   acall SetStats
   clr IE0
   setb EX0         ;; "turn on IE0";
   mov R0, #SP_IE0
   acall Pause      ;; "pause on IE0, first sensor";
   acall SetClock
   acall SetCClock
   acall ClearFlags
   acall Scheduler
   clr IE0
   mov R0, #SP_IE0
   acall Pause      ;; "pause on IE0, second sensor";
   clr EX0          ;; "turn off IE0";
   clr TR0
   clr ET0
   clr TR2
   clr ET2          ;; "turn off TF2, EXF2";
   clr CR
   clr EC           ;; "turn off CF, CCF0, CCF1, CCF2, CCF3, CCF4";
   acall Capture    ;; "save the time of the last sensor";
ret

;; Data transfer
;; # SIZE <Data> Sum <-- Packet
SendDBY: ;; R0: Base, R2: Size
   mov A, #'#'
   acall putchar     ;; putchar('#');
   mov A, #23
   acall putchar     ;; putchar(23);
   mov A, State
   acall putchar     ;; putchar(State);
   mov R3, State
   mov A, Counter
   rl A
   mov R7, A
   add A, #Cycles
   mov Bot, A
   mov R2, #2
   acall DBYLoop     ;; Send(Cycles[Counter]);
   mov R0, #Width
   mov A, R7
   rl A
   mov R7, A
   add A, #TIME0
   mov Bot, A
   mov R2, #4
   acall DBYLoop     ;; Send(TIME[Counter]);
   mov A, R7
   add A, #First
   mov Bot, A
   mov R2, #4
   acall DBYLoop     ;; Send(First[Counter]);
   mov A, R7
   add A, #Last
   mov Bot, A
   mov R2, #4
   acall DBYLoop     ;; Send(Last[Counter]);
   mov A, R7
   rl A
   add A, #Square
   mov Bot, A
   mov R2, #8
   acall DBYLoop     ;; Send(Square[Counter]);
   mov A, R3
   acall putchar     ;; Send(CheckSum);
ret

DBYLoop:
   1:
      mov R0, Bot
      inc Bot
      mov A, @R0
      xch A, R3
      add A, R3
      xch A, R3
      acall putchar
   djnz R2, 1b
ret

;; : SIZE <Data> Sum  <-- Packet SIZE == 0 means a full packet: 100h bytes.
;; .                  <-- End of transmission
RETRIES equ 3
SendXBY:
   mov R4, #RETRIES    ;; Retry = RETRIES;
   mov A, Size
   orl A, (Size + 1)
jnz SendPacket         ;; if (Size == 0) {
   mov A, #'.'
   acall putchar       ;;    putchar('.'); return;
ret                    ;; }
SendPacket:
   mov A, #':'
   acall putchar       ;; putchar(':');
   mov A, Size
   acall putchar       ;; putchar(Size);
   mov R3, #0          ;; CheckSum = 0;
   mov DPL, Bot
   mov DPH, (Bot + 1)
   mov R2, Size
   1:                  ;; for (DPTR = Bot, R2 = Size; R2 > 0; R2--) {
      movx A, @DPTR
      inc DPTR
      xch A, R3
      add A, R3
      xch A, R3
      acall putchar    ;;    putchar(A = *DPTR++), CheckSum += A;
   djnz R2, 1b         ;; }
   mov A, R3
   acall putchar       ;; putchar(CheckSum);
   acall getchar       ;; switch (getchar()) {
1: cjne A, #1bh, 1f    ;;    case NAK:
   djnz R4, 2f         ;;       if (--Retry == 0) {
      ret              ;;          return;
   2:                  ;;       }
ajmp SendPacket        ;;    resend the packet;
1: cjne A, #1ch, 1f    ;;    case ACK:
   mov A, Size
   jz 2f               ;;       if (Size%0x100 != 0) {
      mov Size, #0     ;;          Size -= Size%0x100;
   sjmp 3f             ;;       }
   2:                  ;;       else {
      dec (Size + 1)   ;;          Size -= 0x100;
   3:                  ;;       }
   mov Bot, DPL
   mov (Bot + 1), DPH  ;;       Bot = DPTR;
ajmp SendXBY           ;;    send the next packet;
1: ret                 ;;    default: abort;

;; Command Interpreter:
;; Data format (in octal):
;; 000 + m -- Stats m
;; 010 + m -- Dump m
;; 020     -- Status
;; 030     -- Test
;; 037     -- Abort
Interpreter:              ;; Interpreter() {
   acall SetPort          ;;    open the serial port at 9600,N,9,1;
CommandLoop:              ;;    while (1) {
   acall getchar          ;;       A = getchar();
   mov Counter, A
   anl A, #00111000b
   xch A, Counter
   anl A, #00000111b
   xch A, Counter         ;;       Counter = A&7, switch (A&0370) {
1: cjne A, #00h, 1f       ;;       case 000:
jb EX0, CommandLoop       ;;          if (in test) break;
   mov A, Counter
   cjne A, #6, $ + 3
jnc CommandLoop           ;;          if (Counter >= 6) break;
   acall SendDBY          ;;          SendDBY(Counter);
sjmp CommandLoop          ;;       break;
1: cjne A, #08h, 1f       ;;       case 010:
jb EX0, CommandLoop       ;;          if (in test) break;
   mov A, Counter
   cjne A, #6, $ + 3
jnc CommandLoop           ;;          if (Counter >= 6) break;
   acall GetBase          ;;          Bot = ARR_BASE[Counter];
   mov R2, #4
   acall GetSize          ;;          Size = 4*Cycles[Counter];
   acall SendXBY          ;;          SendXBY(Bot, Size);
sjmp CommandLoop          ;;       break;
1: cjne A, #10h, 1f       ;;       case 020:
   mov A, Counter
cjne A, #0, CommandLoop   ;;          if (Counter != 0) break;
   jnb ET2, 2f            ;;          if (in test && counting) {
      mov A, #'2'         ;;             putchar('2');
   sjmp 3f                ;;          }
   2:                     ;;          else
   jnb EX0, 2f            ;;          if (in test) {
      mov A, #'1'         ;;             putchar('1`);
   sjmp 3f                ;;          }
   2:                     ;;          else {
      mov A, #'0'         ;;             putchar('0');
   3:                     ;;          }
   acall putchar
sjmp CommandLoop          ;;       break;
1: cjne A, #18h, 1f       ;;       case 030:
   mov A, Counter         ;;          switch (Counter) {
   2: cjne A, #00h, 2f       ;;       case 0:
      jb EX0, CommandLoop    ;;          if (in test) break;
      mov R0, #TEST_SEG
      mov DPTR, #TestCycle
      acall Spawn            ;;          Spawn(0xe0, TestCycle);
   sjmp CommandLoop          ;;       break;
   2: cjne A, #07h, 2f       ;;       case 7:
      jnb EX0, CommandLoop   ;;          if (!in test) break;
      clr EX0                ;;          "turn off sensor";
      clr TR0                ;;          "turn off the scheduler";
      clr ET0
      clr TR2
      clr ET2                ;;          "turn off timer 2";
      clr CR
      clr EC                 ;;          "turn off the capture timer";
      acall Capture          ;;          "save the time of the abort";
      setb FXX               ;;          flag the abort;
   2: ajmp CommandLoop       ;;       break;
1: ajmp CommandLoop          ;;    }

seg data
INTERP_SEG: ds 0x10
TEST_SEG:   ds 0x10

seg code
global main:              ;; main() {
   acall SetTest          ;;    SetTest();
   mov R0, #INTERP_SEG
   mov DPTR, #Interpreter
   acall Spawn            ;;    Spawn(0xc8, Interpreter);
   setb EA                ;;    "turn on multi-tasker";
ret                       ;; }
