;;; This particular piece of software is a demo of
;;; (A) An 8052 multi-tasking architecture
;;; (B) 9-bit serial communications (in mode 3) at 57600 baud
;;; (C) A memory dump routine for Intel Hex Format.

;;; It should operate reliably with a host connected over a RS-232, RS-422,
;;; RS-423 or RS-485.  If using half-duplex, T0 is configured here as the
;;; Transmit-Enable control output.

;;; Binary Technology 8051/8052 Cross Assembler (sxa51) used.

;;; The registers defined in the 8052.
T2CON  equ 0xc8
RCAP2L equ 0xca
RCAP2H equ 0xcb
TL2    equ 0xcc
TH2    equ 0xcd

;;; Addressible bits
;;; Port 1
T2   bit P1.0
T2EX bit P1.1

;;; IE and IP
ET2 bit IE.5
PT2 bit IP.5

;;; T2CON
TF2    bit T2CON.7
EXF2   bit T2CON.6
RCLK   bit T2CON.5
TCLK   bit T2CON.4
EXEN2  bit T2CON.3
TR2    bit T2CON.2
C_T2   bit T2CON.1
CP_RL2 bit T2CON.0

;;; An 8052 multi-tasking kernel.
;;; Process stack
PSP   equ 73h  ;;; int **PSP;
Stack equ 74h  ;;; int *Stack[8];

;;; /* Interrupt descriptor table. */
SP_RI   equ 7ch
SP_TI   equ 7dh
SP_BASE equ 7eh

org 00h
sjmp Start

org 23h
   jbc TI, DidTx
   jbc RI, DidRx
reti
DidTx:
   mov R0, #SP_TI
   acall Resume
reti
DidRx:
   mov R0, #SP_RI
   acall Resume
reti

Start: ;;; Install main(), set its return address to _Die().
   mov PSP, #Stack         ;;; PSP = &Stack[0];
   mov SP, #(SP_BASE - 1)  ;;; SP = SP_BASE - 1;
   mov DPTR, #Exit
   push DPL
   push DPH                ;;; *SP++ = _Exit();
   acall main
_Die:
   orl PCon, #1
sjmp _Die

Pause:                     ;;; void Pause(int **R0) {
   mov @R0, SP             ;;;    *R0 = SP;
   dec PSP
   mov R0, PSP
   mov SP, @R0             ;;;    SP = *--PSP;
ret                        ;;;    "idle until resume";
Resume:
   mov R1, PSP
   inc PSP
   mov @R1, SP             ;;;    *PSP++ = SP;
   mov SP, @R0             ;;;    SP = *R0;
   mov @R0, #(SP_BASE + 1) ;;;    *R0 = SP_BASE + 1;
ret                        ;;; }

Spawn:                   ;;; void Spawn(int *R0, void *(DPTR())) {
   mov R1, PSP
   inc PSP
   mov @R1, SP           ;;;    *PSP++ = SP;
   dec R0
   mov SP, R0            ;;;    SP = --R0;
   acall _Enter          ;;;    (*DPTR)();
Exit:
   dec PSP
   mov R0, PSP
   mov SP, @R0           ;;;    SP = *--PSP;
ret                      ;;; }
_Enter:
   push DPL
   push DPH
ret

;;; RS-485 SERIAL COMMUNICATIONS.
;;; Modes:
;;; (1) Wait for address (REN = 1, T0 = 0, SM2 = 1)
;;; (2) Wait for data    (REN = 1, T0 = 0, SM2 = 0)
;;; (3) Send data        (REM = 0, T0 = 1)

SER_ADDR equ ':'

SetPort:
   mov SCON, #11000000b ;;; Serial comm: 9 bits, no parity, 1 stop bit.
   clr PS
   clr RCLK
   clr TCLK
   mov A, TMOD
   anl A, #0fh
   add A, #00100000b ;;; Timer 1: baud rate timer.
   mov TMOD, A
   mov TH1, #-1         ;;; Baud rate: 28800 baud.
   orl PCON, #80h       ;;; Baud rate now 57600 baud.
   setb TR1
   clr TB8
   clr RI
   clr TI
   setb ES
ret

SetRx: ;;; Set mode for "Wait for address"
   clr T0
   setb SM2
   setb REN
ret

SetTx: ;;; Set mode for "Send data"
   clr REN
   setb T0
ret

getchar:
   getcharLoop:
      mov R0, #SP_RI
      acall Pause
      mov A, SBUF
   jnb SM2, getcharBreak
      cjne A, #SER_ADDR, NoMatch
         clr SM2         ;;; if (A == SER_ADDR) Set mode for "Wait for data";
      NoMatch:
   sjmp getcharLoop
   getcharBreak:
ret

nl:
   mov A, #13
   acall putchar
   mov A, #10
putchar:
   mov SBUF, A
   mov R0, #SP_TI
   acall Pause
ret

;;; APPLICATION
putnib:
   anl A, #0fh
   cjne A, #10, $ + 3
jc isdigit
   add A, #('a' - 10)
   acall putchar
ret
isdigit:
   add A, #'0'
   acall putchar
ret

puthex:
   xch A, R3
   add A, R3
   xch A, R3
   push ACC
   swap A
   acall putnib
   pop ACC
   acall putnib
ret

putend:
   mov A, #':'
   acall putchar
   mov R3, #0
   mov A, #00h
   acall puthex
   mov A, DPH
   acall puthex
   mov A, DPL
   acall puthex
   mov A, #1
   acall puthex
   mov A, R3
   cpl A
   acall puthex
   acall nl
ret

putdata:
   mov A, #':'
   acall putchar
   mov R3, #0
   mov A, #10h
   acall puthex
   mov A, DPH
   acall puthex
   mov A, DPL
   acall puthex
   mov A, #0
   acall puthex
   mov R2, #10h
   DataLoop:
      clr A
      movc A, @A + DPTR
      inc DPTR
      acall puthex
   djnz R2, DataLoop
   mov A, R3
   cpl A
   acall puthex
   acall nl
ret

hexs:
   mov DPTR, #0000h
   hexsLoop:
   mov A, DPH
   cjne A, #20h, $ + 3
   jnc hexsBreak
      acall putdata
   sjmp hexsloop
hexsBreak:
   acall putend
ret

Echo:
   acall SetPort
EchoLoop:
   acall SetRx
   acall getchar
X0: cjne A, #'s', X1
   acall hexs
sjmp EchoLoop
X1:
   acall SetTx
   acall putchar
sjmp EchoLoop

main:
   mov R0, #90h
   mov DPTR, #Echo
   acall Spawn
   setb EA
ret
end
