;;; /* Process stack. */
HW    equ 105  ;;; int **HW;
Stack equ 106  ;;; int *Stack[8];

;;; /* Interrupt descriptor table. */
SP_IE0  equ  114
SP_TF0  equ  115
SP_IE1  equ  116
SP_TF1  equ  117
SP_RI   equ  118
SP_TI   equ  119
SP_TF2  equ  120
SP_BASE equ  126

;;; Special function registers for the 8052
T2CON  equ 0xc8
RCAP2L equ 0xca
RCAP2H equ 0xcb
TL2    equ 0xcc
TH2    equ 0xcd

;;; SFR bits for the 80c52.
;;; IE and IP
ET2 bit IE.5
PT2 bit IP.5

;;; P1
T2Ex bit P1.1
T2   bit P1.0

;;; T2Con
TF2    bit T2Con.7
EXF2   bit T2Con.6
RCLK   bit T2Con.5
TCLK   bit T2Con.4
EXEN2  bit T2Con.3
TR2    bit T2Con.2
C_T2   bit T2Con.1
CP_RL2 bit T2Con.0

org 0
sjmp Start

org 03h
   mov R0, #SP_IE0
   acall Resume
reti

org 13h
   mov R0, #SP_IE1
   acall Resume
reti

org 23h
   jbc TI, DidTx
   jbc RI, DidRx
reti

org 2bh
push PSW
push Acc
push 0
push 1
   clr TF2
   mov R0, #SP_TF2
   acall Resume
pop 1
pop 0
pop Acc
pop PSW
reti

DidTx:
   mov R0, #SP_TI
   acall Resume
reti
DidRx:
   mov R0, #SP_RI
   acall Resume
reti

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

Pause:                     ;;; void Pause(int @@R0) {
   mov @R0, SP             ;;;    @R0 = SP;
clr EA
   dec HW
   mov R0, HW
setb EA
   mov SP, @R0             ;;;    SP = @--HW;
ret                        ;;;    "idle until resume";
Resume:
clr EA
   mov R1, HW
   inc HW
setb EA
   mov @R1, SP             ;;;    @HW++ = SP;
   mov SP, @R0             ;;;    SP = @R0;
   mov @R0, #(SP_BASE + 1) ;;;    @R0 = SP_BASE + 1;
ret                        ;;; }

Spawn:                   ;;; int Spawn(int @R0, void (*DPTR)())) {
clr EA
   mov R1, HW
   inc HW
setb EA
   mov @R1, SP           ;;;    @HW++ = SP;
   dec R0
   mov SP, R0            ;;;    SP = --R0;
   acall _Enter          ;;;    (*DPTR)();
Exit:
clr EA
   dec HW
   mov R0, HW
setb EA
   mov SP, @R0           ;;;   SP = @--HW;
ret                      ;;; }
_Enter:
   push DPL
   push DPH
ret
