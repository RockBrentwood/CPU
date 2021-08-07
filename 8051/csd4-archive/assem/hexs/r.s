org 0
B0000:
sjmp B0034
DATA AT 0002h
 0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0|                |
 0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0|                |
 0                                             |                |
org 0023h
B0023:
   jbc TI, B002a
   jbc RI, B002f
reti
B002a:
   mov R0, #7dh
   acall C0051
reti
B002f:
   mov R0, #7ch
   acall C0051
reti
B0034:
   mov 73h, #74h
   mov SP, #7dh
   mov DPTR, #0067h
   push DPL
   push DPH
   acall B0144
B0043:
   orl PCON, #01h
sjmp B0043
C0048:
   mov @R0, SP
   dec 73h
   mov R0, 73h
   mov SP, @R0
ret
C0051:
   mov R1, 73h
   inc 73h
   mov @R1, SP
   mov SP, @R0
   mov @R0, #7fh
ret
B005c:
   mov R1, 73h
   inc 73h
   mov @R1, SP
   dec R0
   mov SP, R0
   acall B006e
B0067:
   dec 73h
   mov R0, 73h
   mov SP, @R0
ret
B006e:
   push DPL
   push DPH
ret
B0073:
   mov SCON, #0c0h
   clr PS
   clr RCLK
   clr TCLK
   mov A, TMOD
   anl A, #0fh
   add A, #20h
   mov TMOD, A
   mov TH1, #0ffh
   orl PCON, #80h
   setb TR1
   clr TB8
   clr RI
   clr TI
   setb ES
ret
B0095:
   clr T0
   setb SM2
   setb REN
ret
B009c:
   clr REN
   setb T0
ret
C00a1:
   mov R0, #7ch
   acall C0048
   mov A, SBUF
   jnb SM2, B00b1
   cjne A, #3ah, B00af
   clr SM2
B00af:
sjmp C00a1
B00b1:
ret
C00b2:
   mov A, #0dh
   acall G00b8
   mov A, #0ah
G00b8:
   mov SBUF, A
   mov R0, #7dh
   acall C0048
ret
C00bf:
   anl A, #0fh
   cjne A, #0ah, B00c4
B00c4:
   jc B00cb
   add A, #57h
   acall G00b8
ret
B00cb:
   add A, #30h
   acall G00b8
ret
L00d0:
   xch A, R3
   add A, R3
   xch A, R3
   push ACC
   swap A
   acall C00bf
   pop ACC
   acall C00bf
ret
B00dd:
   mov A, #3ah
   acall G00b8
   mov R3, #00h
   mov A, #00h
   acall L00d0
   mov A, DPH
   acall L00d0
   mov A, DPL
   acall L00d0
   mov A, #01h
   acall L00d0
   mov A, R3
   cpl A
   acall L00d0
   acall C00b2
ret
B00fa:
   mov A, #3ah
   acall G00b8
   mov R3, #00h
   mov A, #10h
   acall L00d0
   mov A, DPH
   acall L00d0
   mov A, DPL
   acall L00d0
   mov A, #00h
   acall L00d0
   mov R2, #10h
B0112:
   clr A
   movc A, @A+DPTR
   inc DPTR
   acall L00d0
   djnz R2, B0112
   mov A, R3
   cpl A
   acall L00d0
   acall C00b2
ret
B0120:
   mov DPTR, #0000h
B0123:
   mov A, DPH
   cjne A, #20h, B0128
B0128:
   jnc B012e
   acall B00fa
sjmp B0123
B012e:
   acall B00dd
ret
B0131:
   acall B0073
C0133:
   acall B0095
   acall C00a1
   cjne A, #73h, B013e
   acall B0120
sjmp C0133
B013e:
   acall B009c
   acall G00b8
sjmp C0133
B0144:
   mov R0, #90h
   mov DPTR, #0131h
   acall B005c
   setb EA
ret
