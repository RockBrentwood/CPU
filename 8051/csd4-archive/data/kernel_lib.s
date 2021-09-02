;;; kernel.lib
;;; /* Process stack. */
HW    equ 19h ;;; 1 bytes  ;;; int **HW;
Stack equ 1ah ;;; 3 bytes  ;;; int *Stack[3];

;;; /* Interrupt descriptor table. */
SP_IE0  equ 1dh  ;;; 1 byte
SP_RI   equ 1eh  ;;; 1 byte
SP_TI   equ 1fh  ;;; 1 byte
SP_BASE equ 0aeh ;;; 2 bytes

org 0
ajmp Start

org 03h
   mov R0, #SP_IE0
   acall Resume
reti

org 0bh
   acall Process
reti

DidTx:
   mov R0, #SP_TI
   acall Resume
reti
DidRx:
   mov R0, #SP_RI
   acall Resume
reti

org 23h
   jbc TI, DidTx
   clr RI
sjmp DidRx

org 2bh
   jnb EXF2, DidTF2
   jnb TF2, DidEXF2
sjmp DidBoth

org 33h
   jb PCF, DequeueHigh
   jnb CF, DequeueLow
sjmp UpdateStatus

DidBoth:
   mov A, RCAP2H
   jb ACC.7, DidEXF2 ;;; This determines if TF2 or EXF2 was first.
DidTF2:
   clr TF2
   acall Tick
reti
DidEXF2:
   clr EXF2
   djnz Sector0, RetX0
      mov Sector0, #SECTORS
      mov (TIME0 + 0), RCAP2L
      mov (TIME0 + 1), RCAP2H
      mov (TIME0 + 2), (TIME + 0)
      mov (TIME0 + 3), (TIME + 1)
      setb TC0
   RetX0:
reti

;;; Software to insert to handle 5 additional falling edge capture counters.
;;; This will resolve multiple EC interrupts in such a way that
;;; captures that occur before timer overflows are handled first.
CSTATUS equ 20h ;;; 1 byte  ;;; High-priority interrupt queue.
PCF  bit CSTATUS.7
PCF0 bit CSTATUS.0
PCF1 bit CSTATUS.1
PCF2 bit CSTATUS.2
PCF3 bit CSTATUS.3
PCF4 bit CSTATUS.4

UpdateStatus: ;;; Update CSTATUS 
   setb PCF
Q0: jnb CCF0, Q1
   mov A, CCAP0H
   mov C, ACC.7
   mov PCF0, C
Q1: jnb CCF1, Q2
   mov A, CCAP1H
   mov C, ACC.7
   mov PCF1, C
Q2: jnb CCF2, Q3
   mov A, CCAP2H
   mov C, ACC.7
   mov PCF2, C
Q3: jnb CCF3, Q4
   mov A, CCAP3H
   mov C, ACC.7
   mov PCF3, C
Q4: jnb CCF4, DequeueHigh
   mov A, CCAP4H
   mov C, ACC.7
   mov PCF4, C
DequeueHigh:        ;;; Dequeue high priority flags.
   jbc PCF0, DoCCF0
   jbc PCF1, DoCCF1
   jbc PCF2, DoCCF2
   jbc PCF3, DoCCF3
   jbc PCF4, DoCCF4
DoCF:
   clr PCF
   clr CF
   acall CTick
reti
DequeueLow:         ;;; Dequeue low priority flags.
   jbc CCF0, DidCCF0
   jbc CCF1, DidCCF1
   jbc CCF2, DidCCF2
   jbc CCF3, DidCCF3
DoCCF4:
   clr CCF4
DidCCF4:
   djnz Sector5, RetX5
      mov Sector5, #SECTORS
      mov (TIME5 + 0), CCAP4L
      mov (TIME5 + 1), CCAP4H
      mov (TIME5 + 2), (CTIME + 0)
      mov (TIME5 + 3), (CTIME + 1)
      setb TC5
   RetX5:
reti
DoCCF3:
   clr CCF3
DidCCF3:
   djnz Sector4, RetX4
      mov Sector4, #SECTORS
      mov (TIME4 + 0), CCAP3L
      mov (TIME4 + 1), CCAP3H
      mov (TIME4 + 2), (CTIME + 0)
      mov (TIME4 + 3), (CTIME + 1)
      setb TC4
   RetX4:
reti
DoCCF2:
   clr CCF2
DidCCF2:
   djnz Sector3, RetX3
      mov Sector3, #SECTORS
      mov (TIME3 + 0), CCAP2L
      mov (TIME3 + 1), CCAP2H
      mov (TIME3 + 2), (CTIME + 0)
      mov (TIME3 + 3), (CTIME + 1)
      setb TC3
   RetX3:
reti
DoCCF1:
   clr CCF1
DidCCF1:
   djnz Sector2, RetX2
      mov Sector2, #SECTORS
      mov (TIME2 + 0), CCAP1L
      mov (TIME2 + 1), CCAP1H
      mov (TIME2 + 2), (CTIME + 0)
      mov (TIME2 + 3), (CTIME + 1)
      setb TC2
   RetX2:
reti
DoCCF0:
   clr CCF0
DidCCF0:
   djnz Sector1, RetX1
      mov Sector1, #SECTORS
      mov (TIME1 + 0), CCAP0L
      mov (TIME1 + 1), CCAP0H
      mov (TIME1 + 2), (CTIME + 0)
      mov (TIME1 + 3), (CTIME + 1)
      setb TC1
   RetX1:
reti

;;; Pulse input scheduler
;;; When SECTORS (20) pulse inputs are counted from an input source, one full
;;; nutation is marked off, the time of that nutation is saved in the
;;; corresponding TIMEn variable, and the sector count is reset.  TCn marks this
;;; event.  At regular intervals a pending input is dequeued by looking for and
;;; resetting the first marked TCn flag.
;;; FXn is used to mark the first nutation in a test.  It is cleared thereafter.
TSTATUS equ 21h ;;; 1 byte  ;;; Pulse-counter queue.
TC0 bit TSTATUS.0
TC1 bit TSTATUS.1
TC2 bit TSTATUS.2
TC3 bit TSTATUS.3
TC4 bit TSTATUS.4
TC5 bit TSTATUS.5

State   equ 22h ;;; 1 byte
FX0 bit State.0
FX1 bit State.1
FX2 bit State.2
FX3 bit State.3
FX4 bit State.4
FX5 bit State.5
FXX bit State.6

Counter equ 23h ;;; 1 byte

PROCESS_RATE equ 2000
Scheduler:
   mov TL0, #low(-PROCESS_RATE)
   mov TH0, #high(-PROCESS_RATE)
   clr TF0
   setb TR0
   setb ET0
ret
Process:             ;;; "Pause on TF0"
   clr ET0
   clr TR0
   jbc TC0, ResumeX0  ;;; Dequeue input from pulse-counter queue.
   jbc TC1, ResumeX1
   jbc TC2, ResumeX2
   jbc TC3, ResumeX3
   jbc TC4, ResumeX4
   jbc TC5, ResumeX5
sjmp Scheduler
FirstPartial:
   acall Partial
sjmp Scheduler
Next:
   acall Nutation
sjmp Scheduler
ResumeX0:
   mov Counter, #0
   jbc FX0, FirstPartial
sjmp Next
ResumeX1:
   mov Counter, #1
   jbc FX1, FirstPartial
sjmp Next
ResumeX2:
   mov Counter, #2
   jbc FX2, FirstPartial
sjmp Next
ResumeX3:
   mov Counter, #3
   jbc FX3, FirstPartial
sjmp Next
ResumeX4:
   mov Counter, #4
   jbc FX4, FirstPartial
sjmp Next
ResumeX5:
   mov Counter, #5
   jbc FX5, FirstPartial
sjmp Next

Start: ;;; Install main(), set its return address to Idle().
   mov HW, #Stack          ;;; HW = &Stack[0];
   mov SP, #(SP_BASE - 1)  ;;; SP = SP_BASE - 1;
   mov DPTR, #Exit
   push DPL
   push DPH                ;;; @SP++ = Exit();
   acall main
Idle:
   orl PCON, #1
sjmp Idle

Spawn:                   ;;; int Spawn(int @R0, void *(DPTR())) {
   mov R1, HW
   mov @R1, SP
   inc HW                ;;;    @HW++ = SP;
   dec R0
   mov SP, R0            ;;;    SP = --R0;
   acall Enter           ;;;    (*DPTR)();
Exit:
   dec HW
   mov R0, HW
   mov SP, @R0           ;;;    SP = @--HW;
ret                      ;;; }
Enter:
   push DPL
   push DPH
ret

Pause:                     ;;; void Pause(int @@R0) {
   mov @R0, SP             ;;;    @R0 = SP;
   dec HW
   mov R0, HW
   mov SP, @R0             ;;;    SP = @--HW;
ret                        ;;;    "idle until resume";
Resume:
   mov R1, HW
   mov @R1, SP
   inc HW                  ;;;    @HW++ = SP;
   mov SP, @R0             ;;;    SP = @R0;
ret                        ;;; }
