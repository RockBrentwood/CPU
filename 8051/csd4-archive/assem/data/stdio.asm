include "8051fa.h"
include "kernel.h"

;; RS-485 SERIAL COMMUNICATIONS.
;; Modes:
;; (1) Receive data (REN = 1, T0 = 0, SM2 = 1)
;; (2) Send data    (REM = 0, T0 = 1)

global SetPort:
   mov SCON, #11000000b ;; Serial comm: 9 bits, no parity, 1 stop bit.   
   clr PS
   clr RCLK
   clr TCLK
   mov A, TMOD
   anl A, #00001111b
   add A, #00100000b ;; Timer 1: baud rate timer.
   mov TMOD, A
   mov TH1, #-3         ;; Baud rate: 19200 baud.
   setb TR1
   clr TB8
   clr RI
   clr TI
   setb SM2
   setb ES
ret

global getchar:
   2:
      jnb T0, 1f
         clr T0    ;; Set mode for "Receive data"
         setb REN
      1:
      mov R0, #SP_RI
      acall Pause
      mov A, SBUF
   jnb ACC.6, 2b
   anl A, #00111111b
ret

global putchar:
   jb T0, 1f
      mov R6, #100 ;; Set mode for "Send data"
      djnz R6, $   ;; Wait 201 cycles for other end to release the line.
      clr REN
      setb T0
   1:
   mov SBUF, A
   mov R0, #SP_TI
   acall Pause
ret
