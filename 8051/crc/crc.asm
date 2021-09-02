include "8052.h"

;;; Process stack
HW    equ 73h  ;;; int **HW;
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
   dec HW
   mov R0, HW
   mov SP, @R0             ;;;    SP = @--HW;
ret                        ;;;    "idle until resume";
Resume:
   mov R1, HW
   inc HW
   mov @R1, SP             ;;;    @HW++ = SP;
   mov SP, @R0             ;;;    SP = @R0;
   mov @R0, #(SP_BASE + 1) ;;;    @R0 = SP_BASE + 1;
ret                        ;;; }

Spawn:                   ;;; int Spawn(int @R0, void *(DPTR())) {
   mov R1, HW
   inc HW
   mov @R1, SP           ;;;    @HW++ = SP;
   dec R0
   mov SP, R0            ;;;    SP = --R0;
   acall _Enter          ;;;    (*DPTR)();
Exit:
   dec HW
   mov R0, HW
   mov SP, @R0           ;;;   SP = @--HW;
ret                      ;;; }
_Enter:
   push DPL
   push DPH
ret

;;; RS-485 SERIAL COMMUNICATIONS.
;;; Modes:
;;; (1) Receive data     (REN = 1, T0 = 0)
;;;    (a) Wait for address (SM2 = 1)
;;;    (b) Wait for data    (SM2 = 0)
;;; (2) Send data        (REN = 0, T0 = 1)

SER_ADDR equ ':'

SetPort:
   mov SCON, #11000000b ;;; Serial comm: 8 bits, no parity, 1 stop bit.
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

SetRx: ;;; Set mode for "Receive data"
   clr T0
   setb REN
ret

SetTx: ;;; Set mode for "Send data"
   clr REN
   setb T0
ret

getchar:
   setb SM2          ;;; Set mode for "Wait for address";
   GetAddress:       ;;; do {
      mov R0, #SP_RI
      acall Pause    ;;;    wait for reception;
      mov A, SBUF    ;;;    A = received character;
      xrl A, #SER_ADDR
   jnz GetAddress    ;;; } while (A != SER_ADDR);
   clr SM2           ;;; Set mode for "Wait for data";
   mov R0, #SP_RI
   acall Pause       ;;; wait for reception;
   mov A, SBUF       ;;; return received character;
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
push ACC
push DPL
push DPH
   mov DPTR, #Values
   movc A, @A + DPTR
   xrl A, R3
   mov R3, A
pop DPH
pop DPL
pop ACC
push ACC
   swap A
   acall putnib
pop ACC
   acall putnib
ret

hexs:
   mov DPTR, #0000h
   hexsLoop:
   mov A, DPH
   cjne A, #20h, $ + 3
   jnc hexsBreak         ;;; for (Addr = 0x0000; Addr <= 0x2000; ) {
      mov A, #':'
      acall putchar      ;;;    putchar(':');
      mov R3, #0         ;;;    CheckSum = 0;
      mov A, #10h
      acall puthex
      mov A, DPH
      acall puthex
      mov A, DPL
      acall puthex
      mov A, #0
      acall puthex       ;;;    printf("%2x%4x%2x", 0x10, Addr, 0);
      mov R2, #10h
      DataLoop:          ;;;    for (R2 = 0x10; R2 > 0; R2--) {
         clr A
         movc A, @A + DPTR
         inc DPTR
         acall puthex    ;;;       printf("%2x", *Addr++);
      djnz R2, DataLoop  ;;;    }
      mov A, R3
      acall puthex
      acall nl           ;;;    printf("%2x\n", CheckSum);
   sjmp hexsloop         ;;; }
hexsBreak:
   mov A, #':'
   acall putchar         ;;; putchar(':');
   mov R3, #0
   mov A, #00h
   acall puthex
   mov A, DPH
   acall puthex
   mov A, DPL
   acall puthex
   mov A, #1
   acall puthex          ;;; printf("%2x%4x%2x", 0, Addr, 1);
   mov A, R3
   acall puthex
   acall nl              ;;; printf("%2x\n", CheckSum);
ret

Echo:
   acall SetPort         ;;; SetPort();
   acall SetRx           ;;; set for receive mode;
EchoLoop:
   acall getchar         ;;; while (1) switch (A = getchar()) {
X0: cjne A, #'s', X1     ;;;    case 's':
   acall SetTx           ;;;       set for transmit mode;
   acall hexs            ;;;       hexs();
   acall SetRx           ;;;       set for receive mode;
sjmp EchoLoop            ;;;    break;
X1:                      ;;;    default:
   acall SetTx           ;;;       set for transmit mode;
   acall putchar         ;;;       putchar(A);
   acall SetRx           ;;;       set for receive mode;
sjmp EchoLoop            ;;; }

main:
   mov R0, #90h
   mov DPTR, #Echo
   acall Spawn
   setb EA
ret

;;; Polynomial used: x^8 + x^6 + x^5 + x^3 + x^0
Values:
db  00h, 69h,0d2h,0bbh,0cdh,0a4h, 1fh, 76h
db 0f3h, 9ah, 21h, 48h, 3eh, 57h,0ech, 85h
db 0e6h, 8fh, 34h, 5dh, 2bh, 42h,0f9h, 90h
db  15h, 7ch,0c7h,0aeh,0d8h,0b1h, 0ah, 63h
db 0a5h,0cch, 77h, 1eh, 68h, 01h,0bah,0d3h
db  56h, 3fh, 84h,0edh, 9bh,0f2h, 49h, 20h
db  43h, 2ah, 91h,0f8h, 8eh,0e7h, 5ch, 35h
db 0b0h,0d9h, 62h, 0bh, 7dh, 14h,0afh,0c6h
db  4ah, 23h, 98h,0f1h, 87h,0eeh, 55h, 3ch
db 0b9h,0d0h, 6bh, 02h, 74h, 1dh,0a6h,0cfh
db 0ach,0c5h, 7eh, 17h, 61h, 08h,0b3h,0dah
db  5fh, 36h, 8dh,0e4h, 92h,0fbh, 40h, 29h
db 0efh, 86h, 3dh, 54h, 22h, 4bh,0f0h, 99h
db  1ch, 75h,0ceh,0a7h,0d1h,0b8h, 03h, 6ah
db  09h, 60h,0dbh,0b2h,0c4h,0adh, 16h, 7fh
db 0fah, 93h, 28h, 41h, 37h, 5eh,0e5h, 8ch
db  94h,0fdh, 46h, 2fh, 59h, 30h, 8bh,0e2h
db  67h, 0eh,0b5h,0dch,0aah,0c3h, 78h, 11h
db  72h, 1bh,0a0h,0c9h,0bfh,0d6h, 6dh, 04h
db  81h,0e8h, 53h, 3ah, 4ch, 25h, 9eh,0f7h
db  31h, 58h,0e3h, 8ah,0fch, 95h, 2eh, 47h
db 0c2h,0abh, 10h, 79h, 0fh, 66h,0ddh,0b4h
db 0d7h,0beh, 05h, 6ch, 1ah, 73h,0c8h,0a1h
db  24h, 4dh,0f6h, 9fh,0e9h, 80h, 3bh, 52h
db 0deh,0b7h, 0ch, 65h, 13h, 7ah,0c1h,0a8h
db  2dh, 44h,0ffh, 96h,0e0h, 89h, 32h, 5bh
db  38h, 51h,0eah, 83h,0f5h, 9ch, 27h, 4eh
db 0cbh,0a2h, 19h, 70h, 06h, 6fh,0d4h,0bdh
db  7bh, 12h,0a9h,0c0h,0b6h,0dfh, 64h, 0dh
db  88h,0e1h, 5ah, 33h, 45h, 2ch, 97h,0feh
db  9dh,0f4h, 4fh, 26h, 50h, 39h, 82h,0ebh
db  6eh, 07h,0bch,0d5h,0a3h,0cah, 71h, 18h
end

