;;; An interrupt-driven multi-processing library for the Intel 8052.
;;; This package requires an 80C52-compatible microprocessor with at least
;;; 256k of indirectly addressible RAM.  All interrupts are reserved for
;;; process-handling, in this library.  But you still have to write the
;;; software that enables/disables them.  Timer 2's interrupt-handlers are set
;;; up to handle capture mode.  Different handlers should be written in place
;;; of this for the different timer 2 modes.

;;; Initially, the only process run is main().  It or any other process can
;;; Spawn() other processes and main() can even go idle with the other
;;; processes running in the background.  A background process is tied to an
;;; interrupt or to another (accessible) background process through Resume().
;;;  When Resume() is called, the process continues where it left off at the
;;; last call of Pause() and continues until either it returns or calls Pause()
;;; again.

;;; /* Task control block */
;;; typedef struct {
;;;    char *SP; /* The process's stack pointer */
;;; } Task;
;;; /* Process stack. */
PSP    equ 105  ;;; Task *PSP;
PStack equ 106  ;;; Task PStack[8];

;;; /* Interrupt descriptor table. */
SP_IE0  equ  114
SP_TF0  equ  115
SP_IE1  equ  116
SP_TF1  equ  117
SP_RI   equ  118
SP_TI   equ  119
SP_TF2  equ  120
SP_EXF2 equ  121
SP_SW0  equ  122
SP_SW1  equ  123
SP_SW2  equ  124
SP_SW3  equ  125
SP_BASE equ  126

;;; Special function registers for the 8052
T2CON  equ 0xc8
RCAP2L equ 0xca
RCAP2H equ 0xcb
TL2    equ 0xcc
TH2    equ 0xcd

;;; SFR bits for the 80c52.
;;; P1
T2Ex bit P1.1
T2   bit P1.0

;;; T2CON
TF2    bit T2CON.7
EXF2   bit T2CON.6
RCLK   bit T2CON.5
TCLK   bit T2CON.4
EXEN2  bit T2CON.3
TR2    bit T2CON.2
C_T2   bit T2CON.1
CP_RL2 bit T2CON.0

;;; IE and IP
ET2 bit IE.5
PT2 bit IP.5

org 00h
sjmp Start

org 03h
   mov R0, #SP_IE0
   acall Resume
reti

org 0bh
   mov R0, #SP_TF0
   acall Resume
reti

org 13h
   mov R0, #SP_IE1
   acall Resume
reti

org 1bh
   mov R0, #SP_TF1
   acall Resume
reti

org 23h
   jbc TI, DidTx
   jbc RI, DidRx
reti

org 2bh
   jnb EXEN2, DidTF2
   jnb EXF2, DidTF2
   jnb TF2, DidEXF2
   mov A, RCAP2H
   jnb Acc.7, DidTF2 ;;; This determines if TF2 or EXF2 was first.
DidEXF2:
   clr EXF2
   mov R0, #SP_EXF2
   acall Resume
reti
DidTF2:
   clr TF2
   mov R0, #SP_TF2
   acall Resume
reti

DidTx:
   mov R0, #SP_TI
   acall Resume
reti

DidRx:
   mov R0, #SP_RI
   acall Resume
reti

Start: ;;; Install main(), set its return address to Die().
   mov PSP, #PStack        ;;; PSP = &PStack[0];
   mov SP, #(SP_BASE - 1)  ;;; SP = SP_BASE - 1;
   mov DPTR, #Exit
   push DPL
   push DPH                ;;; *SP++ = Exit();
   acall main
Die:
   orl PCON, #1
sjmp Die

Pause:                     ;;; void Pause(Task *R0) {
   mov @R0, SP             ;;;    R0->SP = SP;
   dec PSP
   mov R0, PSP
   mov SP, @R0             ;;;    SP = (--PSP)->SP;
ret                        ;;;    "wait for Resume";
Resume:
   mov R1, PSP
   mov @R1, SP
   inc PSP                 ;;;    (PSP++)->SP = SP;
   mov SP, @R0             ;;;    SP = R0->SP;
   mov @R0, #(SP_BASE + 1) ;;;    R0->SP = SP_BASE + 1;
ret                        ;;; }

Spawn:                   ;;; int Spawn(char *R0, void (*DPTR)()) {
   mov R1, PSP
   mov @R1, SP
   inc PSP               ;;;    (PSP++)->SP = SP;
   dec R0
   mov SP, R0            ;;;    SP = --R0;
   acall Enter           ;;;    (*DPTR)();
Exit:
   dec PSP
   mov R0, PSP
   mov SP, @R0           ;;;    SP = (--PSP)->SP;
ret                      ;;; }
Enter:
   push DPL
   push DPH
ret
