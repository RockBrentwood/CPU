;; RS-485 SERIAL COMMUNICATIONS.
;; Modes:
;; (1) Wait for address (REN = 1, T0 = 0, SM2 = 1)
;; (2) Wait for data    (REN = 1, T0 = 0, SM2 = 0)
;; (3) Send data        (REN = 0, T0 = 1)

SER_ADDR equ 11110111b
;; All addresses should have the same number (7) of bits set.

SetPort:
   mov SCON, #11000000b ;; Serial comm: 8 bits, no parity, 1 stop bit.   
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
   setb ES
ret

SetRx: ;; Set mode for "Wait for address"
   clr T0
   setb SM2
   setb REN
ret

SetTx: ;; Set mode for "Send data"
   clr REN
   setb T0
ret

getchar:
   RecvLoop:
      mov R0, #SP_RI
      acall Pause
      mov A, SBUF
   jnb SM2, RecvData
      cpl A
      anl A, #SER_ADDR   ;; A = SER_ADDR & ~SBUF
      jnz NoMatch
         clr SM2         ;; if (A == 0) Set mode for "Wait for data";
      NoMatch:
   sjmp RecvLoop
   RecvData:
ret

putchar:
   mov SBUF, A
   mov R0, #SP_TI
   acall Pause
ret
