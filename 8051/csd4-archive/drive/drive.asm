include "kc.lib" ;;; Multiprocessing support: this MUST be first.
include "rx9.lib"
include "timer.lib"
include "math.lib"

;;; Set the outputs prior to assembly.
PULSE     bit P1.0 ;;; The pulse output (ASSUMED ACTIVE HIGH).
CW_CCW    bit P1.1 ;;; Clockwise/counterclockwise output.
HP_FP     bit P1.2 ;;; Half pulse/full pulse output.
WIND_OFF  bit P1.3 ;;; All windings off (active low).
RED_LED   bit P1.4
GREEN_LED bit P1.5

SetInt0:
   setb Int0 ;;; Latch Int0 for input for the Sensor interrupt.
   clr PX0
   setb IT0
ret
SetInt1:
   setb Int1 ;;; Latch Int1 for input for the Micro-switch interrupt.
   clr PX1
   setb IT1
ret

Sensors equ 08
Count   equ 09
Percent equ 12
FindAccuracy:
   mov R0, #Op
   mov R1, #Count
   mov R2, #3
   acall CopyX
   mov (Op + 3), #0
   mov R0, #Divisor
   mov R1, #Pulses
   mov R2, #3
   acall CopyX
   mov (Divisor + 3), #0
   acall Div32       ;;; Quo:Fract = Count/Pulses;
   mov A, (Quo + 1)
   orl A, (Quo + 2)
   orl A, (Quo + 3)
   jnz TT0
      mov A, Quo
      cjne A, #3, $ + 3
   jc TT1
   TT0:
      mov Quo, #2
   TT1:              ;;; if (Quo >= 3) Quo = 2;
   jz TT2            ;;; if (Quo > 0) {
      dec Quo
      setb Sign      ;;;    Quo--; Sign = 1;
   sjmp TT3
   TT2:              ;;; } else {
      mov R0, #Fract
      mov R2, #4
      acall CplX
      clr Sign       ;;;    Fract = -Fract; Sign = 0;
   TT3:              ;;; }
   mov (Percent + 3), Quo
   acall MulBy10
   mov (Percent + 2), Aux
   acall MulBy10
   mov (Percent + 1), Aux
   acall MulBy10
   mov Percent, Aux  ;;; sprintf(Percent, "%3.1f", Quo:Fract);
   mov A, (Fract + 3)
   jnb Acc.7, WW
      mov R0, #Percent
      mov R2, #4
      acall Inc10X
   WW:               ;;; if (Fract >= 1/2) Percent++; ... round up
   mov A, (Percent + 3)
   orl A, (Percent + 2)
   jz VV
      mov (Percent + 1), #9
      mov Percent, #9
   VV:               ;;; if (Percent >= 10.0) Percent = 9.9;
   mov A, (Percent + 1)
   jnz NotOK
   mov A, Percent
   cjne A, #5, $ + 3
   jnc NotOK
      setb Good
      clr GREEN_LED
   sjmp UU
   NotOK:
      clr Good
      clr RED_LED
   UU:               ;;; Good = (Percent <= 0.4);
ret

CountSensors:
   clr IE0
   setb EX0
   clr Good
   clr Sign
   clr Change
   mov Sensors, #0
   clr Counting
   mov Percent, #0
   mov (Percent + 1), #0
   mov Count, #0
   mov (Count + 1), #0
   mov (Count + 2), #0   ;;; Count = 0;
   mov Pulses, Load
   mov (Pulses + 1), (Load + 1)
   mov (Pulses + 2), (Load + 2) ;;; Pulses = Load;
   mov R0, #SP_IE0
   acall Pause
   inc Sensors
   setb Counting
   mov R0, #SP_IE0
   acall Pause
   inc Sensors
   clr Counting
   setb Change
   clr EX0
ret

LookUp: ;;; LookUp[N] = -(delay for the speed 25*N RPM).
;;; LookUp[N] = -11059.2/N (rounded), for N in 1..80.
dw -11059, -5530, -3686, -2765, -2212, -1843, -1580, -1382 
dw  -1229, -1106, -1005,  -922,  -851,  -790,  -737,  -691 
dw   -651,  -614,  -582,  -553,  -527,  -503,  -481,  -461 
dw   -442,  -425,  -410,  -395,  -381,  -369,  -357,  -346 
dw   -335,  -325,  -316,  -307,  -299,  -291,  -284,  -276 
dw   -270,  -263,  -257,  -251,  -246,  -240,  -235,  -230 
dw   -226,  -221,  -217,  -213,  -209,  -205,  -201,  -197 
dw   -194,  -191,  -187,  -184,  -181,  -178,  -176,  -173 
dw   -170,  -168,  -165,  -163,  -160,  -158,  -156,  -154 
dw   -151,  -149,  -147,  -146,  -144,  -142,  -140,  -138 

Speed  equ 16
Time   equ 17

SetDriver:
   setb PULSE
   setb CW_CCW
   setb HP_FP
   clr WIND_OFF
   setb RED_LED
   setb GREEN_LED
   mov Speed, #0
   mov R2, #16
   DelayLoop:
      mov DPTR, #0000h
      acall Delay2
   djnz R2, DelayLoop
ret

ResetDriver:
   mov Speed, #0
   setb WIND_OFF
   clr ET2
ret

SetDelay:
   mov DPTR, #LookUp
   mov A, Speed
   dec A
   rl A
   push Acc
   movc A, @A + DPTR
   mov RCAP2H, A
   pop Acc
   inc A
   movc A, @A + DPTR
   mov RCAP2L, A        ;;; RCAP2 = LookUp[Speed - 1];
ret

DoPulse:
   setb ET2
   mov R0, #SP_TF2
   acall Pause
   clr ET2                   ;;; pause on timer 2;
   clr PULSE
   mov R2, #8
   djnz R2, $
   setb PULSE                ;;; strobe a pulse;
   jnb Counting, XX1
      clr A
      inc Count
   cjne A, Count, XX1
      inc (Count + 1)
   cjne A, (Count + 1), XX1
      inc (Count + 2)        ;;; if (Counting) Count++;
   XX1:
ret

Drive:
   acall SetDelay
   mov R3, Time
   TimeLoop:
      mov R4, Speed
      SpeedLoop:
         acall DoPulse
      djnz R4, SpeedLoop
   djnz R3, TimeLoop
ret

Accelerate:
   mov Time, #5
   clr C
   subb A, Speed
   mov R5, A
   RampUp:
      inc Speed
      acall Drive
   djnz R5, RampUp
ret

Decelerate:
   mov Time, #2
   clr C
   subb A, Speed
   cpl A
   inc A
   mov R5, A
   RampDown:
      acall Drive
      dec Speed
   djnz R5, RampDown
ret

PulseTrain:
   acall SetDelay
   PulseLoop:
      jb Aborted, StopTrain
      jbc Change, StopTrain
      acall DoPulse
   sjmp PulseLoop
StopTrain:
ret

Pulses equ 18
Load   equ 21
Stepper:
   acall SetTimer2
   acall SetInt0
   acall SetInt1
   setb RED_LED
   setb GREEN_LED
   clr Testing
   clr Aborted
StepperCycle:
   clr Active
   WaitForActive:
      acall ResetDriver
      clr IE1
      setb EX1
      mov R0, #SP_IE1
      acall Pause
      clr EX1
      acall SetDriver
   jb INT1, WaitForActive
   setb Active
jb Testing, RampTest
JogTest:
   clr Change
   setb TR2
   mov A, #(500/25)
   acall Accelerate
   acall PulseTrain
   jbc Aborted, JogBreak
JogLoop:
   mov A, #(1750/25)
   acall Accelerate
   acall PulseTrain
   jbc Aborted, JogBreak
   mov A, #(500/25)
   acall Decelerate
   acall PulseTrain
   jbc Aborted, JogBreak
sjmp JogLoop
JogBreak:
   clr TR2
ajmp StepperCycle
RampTest:
   mov R0, #0d0h
   mov DPTR, #CountSensors
   acall Spawn
   setb TR2
   mov A, #(1750/25)
   acall Accelerate
   acall PulseTrain
   clr TR2
   jbc Aborted, AbortTest
   acall FindAccuracy
ajmp StepperCycle
AbortTest:
   clr EX0
ajmp StepperCycle

Change    bit 0 ;;; Interprocess communication.  Set to change jog modes.
Aborted   bit 1 ;;; Interprocess communication.  Used to kill the test.
Counting  bit 2 ;;; Used to queue the pulse counter on timer2 interrupt.
Testing   bit 3 ;;; Indicates that the current state is in the testing mode.
Active    bit 4 ;;; Indicates that the motor is active.
Good      bit 5 ;;; Indicates a test has passed.
Sign      bit 6 ;;; Indicates whether counted pulses exceeded specification.

;;; Commands
DoTest   equ 0
DoJog    equ 1
DoChange equ 2
DoStatus equ 3
DoAbort  equ 4
DoData   equ 5
Interpreter:
   acall SetPort
CommandLoop:
   acall SetRx
   acall getchar
X0: cjne A, #DoTest, X1
   acall getchar
   mov Load, A
   acall getchar
   mov (Load + 1), A
   acall getchar
   mov (Load + 2), A
   jb Active, WW1
      setb Testing
   sjmp WW2
   WW1:
      mov Load, Pulses
      mov (Load + 1), (Pulses + 1)
      mov (Load + 2), (Pulses + 2)
   WW2:
sjmp CommandLoop
X1: cjne A, #DoJog, X2
   jb Active, WW3
      clr Testing
   WW3:
ajmp CommandLoop
X2: cjne A, #DoChange, X3
   jb Testing, YYY
   jnb Active, YYY
      setb Change
   YYY:
ajmp CommandLoop
X3: cjne A, #DoStatus, X4
   acall SetTx
   mov A, Speed         ;;; convert Speed to decimal form.
   mov B, #10
   div AB               ;;; A, B = Speed/10, Speed%10;
   cjne A, #10, $ + 3   ;;; if (Speed >= 100) {
   jc XX3
      mov A, #9
      mov B, #9         ;;;    A, B = 9, 9;
   XX3:                 ;;; }
   swap A
   add A, B
   acall putchar        ;;; putchar(A << 4 | B);
   clr A
   mov C, Active
   mov Acc.0, C
   mov C, Testing
   mov Acc.1, C
   acall putchar        ;;; putchar(Testing << 1 | Active);
ajmp CommandLoop
X4: cjne A, #DoAbort, X5
   jnb Active, ZZZ       ;;; Ignore if the motor is already stopped.
      setb Aborted
   ZZZ:
ajmp CommandLoop
X5: cjne A, #DoData, X6
   acall SetTx
   mov A, Sensors
   acall putchar            ;;; putchar(Sensors);
   clr A
   mov C, Good
   mov Acc.0, C
   mov C, Sign
   mov Acc.1, C
   acall putchar            ;;; putchar(Sign << 1 | Good);
   mov A, (Percent + 1)
   swap A
   add A, Percent
   acall putchar            ;;; putchar(Percent);
ajmp CommandLoop
X6:
ajmp CommandLoop

main:
   mov R0, #90h
   mov DPTR, #Interpreter
   acall Spawn
   mov R0, #0b0h
   mov DPTR, #Stepper
   acall Spawn
   setb EA
ret
end
