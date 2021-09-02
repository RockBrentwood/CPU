;; RS-485 SERIAL COMMUNICATIONS.
;; Modes:
;; (1) Receive data (REN = 1, T0 = 0, SM2 = 1)
;; (2) Send data    (REM = 0, T0 = 1)

SetPort:
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

getchar:
   jnb T0, InRx
      clr T0    ;; Set mode for "Receive data"
      setb REN
   InRx:
   mov R0, #SP_RI
   acall Pause
   mov A, SBUF
jnb ACC.6, getchar
   anl A, #00111111b
ret

putchar:
   jb T0, InTx
      mov R6, #100 ;; Set mode for "Send data"
      djnz R6, $   ;; Wait 201 cycles for other end to release the line.
      clr REN
      setb T0
   InTx:
   mov SBUF, A
   mov R0, #SP_TI
   acall Pause
ret
