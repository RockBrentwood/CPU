;; // Process stack.
HW    equ 6fh          ;; data 1  ;; int **HW;
Stack equ 70h          ;; data 8  ;; int *Stack[8];

;; // Interrupt descriptor table.
SP_IE0  equ 79h ;; data 1
SP_IE1  equ 7bh ;; data 1
SP_EXF2 equ 7ch ;; data 1
SP_CCF  equ 7dh ;; data 1
SP_BASE equ 7eh ;; data $(data)

include "8051fa.h"

org 00h
ajmp Start

org 03h
   mov R0, #SP_IE0
   acall Resume
reti

org 13h
   mov R0, #SP_IE1
   acall Resume
reti

org 2bh
   jnb EXF2, DidTF2
   jnb TF2, DidEXF2
sjmp DidBoth

org 33h
   jbc CCF0, DidCCF0
   jbc CCF1, DidCCF1
   jbc CCF2, DidCCF2
   jbc CCF3, DidCCF3
   jbc CCF4, DidCCF4
reti

DidBoth:
   mov A, RCAP2H
   jnb Acc.7, DidTF2 ;; This determines if TF2 or EXF2 was first.
DidEXF2:
   clr EXF2
   mov R0, #SP_EXF2
   acall Resume
reti
DidTF2:
   clr TF2
reti

DidCCF0:
                       mov A, #'0'
                       acall Sto
   mov A, #0
   mov C, CEX0
sjmp CCX
DidCCF1:
                       mov A, #'1'
                       acall Sto
   mov A, #1
   mov C, CEX1
sjmp CCX
DidCCF2:
                       mov A, #'2'
                       acall Sto
   mov A, #2
   mov C, CEX2
sjmp CCX
DidCCF3:
                       mov A, #'3'
                       acall Sto
   mov A, #3
   mov C, CEX3
sjmp CCX
DidCCF4:
                       mov A, #'4'
                       acall Sto
   mov A, #4
   mov C, CEX4
CCX:
   rlc A
   mov R0, #SP_CCF
   acall Resume
reti

Start: ;; Install main(), set its return address to _Die().
   mov HW, #Stack          ;; HW = &Stack[0];
   mov SP, #(SP_BASE - 1)  ;; SP = SP_BASE - 1;
   mov DPTR, #_Exit
   push DPL
   push DPH                ;; @SP++ = _Exit();
   acall main
_Die:
   orl PCon, #1
sjmp _Die

Pause:                     ;; void Pause(int @@R0) {
   mov @R0, SP             ;;    @R0 = SP;
   dec HW
   mov R0, HW
   mov SP, @R0             ;;    SP = @--HW;
ret                        ;;    "idle until resume";
Resume:
   mov R1, HW
   mov @R1, SP
   inc HW                  ;;    @HW++ = SP;
   mov SP, @R0             ;;    SP = @R0;
   mov @R0, #(SP_BASE + 1) ;;    @R0 = SP_BASE + 1;
ret                        ;; }

Spawn:                   ;; int Spawn(int @R0, void *(DPTR())) {
   mov R1, HW
   mov @R1, SP
   inc HW                ;;    @HW++ = SP;
   dec R0
   mov SP, R0            ;;    SP = --R0;
   acall _Enter          ;;    (*DPTR)();
_Exit:
   dec HW
   mov R0, HW
   mov SP, @R0           ;;    SP = @--HW;
ret                      ;; }
_Enter:
   push DPL
   push DPH
ret

include "io_lib.s"

COUNTERS   equ 13
COUNT_BASE equ 8
IE0s   equ  8
IE1s   equ  9
T2EXs  equ 10
PCCF0s equ 11
NCCF0s equ 12
PCCF1s equ 13
NCCF1s equ 14
PCCF2s equ 15
NCCF2s equ 16
PCCF3s equ 17
NCCF3s equ 18
PCCF4s equ 19
NCCF4s equ 20
SetCounts:
   acall scr_init
   mov R0, #COUNT_BASE
   mov R2, #COUNTERS
   SetLoop:
      mov A, #'0'
      acall cputc
      mov @R0, #0
      inc R0
   djnz R2, SetLoop
ret
IncCount:
   mov Row, #0
   mov Col, A
   add A, #COUNT_BASE
   mov R0, A
   acall locate
   inc @R0
   cjne @R0, #10, XX
      mov @R0, #0
   XX:
   mov A, @R0
   add A, #'0'
   acall cputc
ret

RowP equ 21
ColP equ 22
SetArr:
   mov RowP, #1
   mov ColP, #0
ret
Sto:
   mov Row, RowP
   mov Col, ColP
   push ACC
   acall locate
   pop ACC
   acall cputc
   inc ColP
   mov A, ColP
   cjne A, #20, YY
      mov ColP, #0
      inc RowP
      mov A, RowP
      cjne A, #4, ZZ
         mov RowP, #1
      ZZ:
   YY:
ret

;; APPLICATION PROGRAM
TestIE0:
   setb Int0 ;; Latch Int0 for input.
   clr PX0
   setb IT0
   setb EX0
LoopIE0:
                        mov A, #'f'
                        acall Sto
   mov R0, #SP_IE0
   acall Pause
   mov A, #0
   acall IncCount
                        mov A, #'g'
                        acall Sto
sjmp LoopIE0

TestIE1:
   setb Int1 ;; Latch Int1 for input.
   clr PX1
   setb IT1
   setb EX1
LoopIE1:
                        mov A, #'h'
                        acall Sto
   mov R0, #SP_IE1
   acall Pause
   mov A, #1
   acall IncCount
                        mov A, #'i'
                        acall Sto
sjmp LoopIE1

TestT2EX:
   setb T2EX ;; Latch T2EX for input.
   clr PT2
   setb CP_RL2
   clr C_T2
   setb EXEN2
   setb ET2
LoopT2EX:
                        mov A, #'j'
                        acall Sto
   mov R0, #SP_EXF2
   acall Pause
   mov A, #2
   acall IncCount
                        mov A, #'k'
                        acall Sto
sjmp LoopT2EX

TestCCF:
   mov CCAPM0, #(CAPP + CAPN + ECCF)
   mov CCAPM1, #(CAPP + CAPN + ECCF)
   mov CCAPM2, #(CAPP + CAPN + ECCF)
   mov CCAPM3, #(CAPP + CAPN + ECCF)
   mov CCAPM4, #(CAPP + CAPN + ECCF) ;; CCF raises on either edge.
   setb CEX0
   setb CEX1
   setb CEX2
   setb CEX3
   setb CEX4
   setb EC
   clr PPC
LoopCCF:
                        mov A, #'l'
                        acall Sto
   mov R0, #SP_CCF
   acall Pause
   add A, #3
   acall IncCount
                        mov A, #'m'
                        acall Sto
sjmp LoopCCF

main:
   acall SetCounts
                           acall SetArr
                           mov A, #'a'
                           acall Sto
   mov R0, #90h
   mov DPTR, #TestIE0
   acall Spawn
                           mov A, #'b'
                           acall Sto
   mov R0, #0a0h
   mov DPTR, #TestIE1
   acall Spawn
                           mov A, #'c'
                           acall Sto
   mov R0, #0b0h
   mov DPTR, #TestT2EX
   acall Spawn
                           mov A, #'d'
                           acall Sto
   mov R0, #0c0h
   mov DPTR, #TestCCF
   acall Spawn
                           mov A, #'e'
                           acall Sto
   setb EA
ret
end
