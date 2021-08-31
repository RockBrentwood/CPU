org 0000h
B0000:
ajmp B04d0
org 0003h
;; DATA at 0003h
db  78h, 0ch,0b1h, 04h, 32h                                                        ;; 0003:x   2
org 000bh
;; DATA at 000bh
db  91h, 80h, 32h                                                                  ;; 000b:  2
B000e:
   mov R0, #0eh
   acall C0504
reti
B0013:
   mov R0, #0dh
   acall C0504
reti
org 0023h
B0023:
   jbc TI, B000e
   clr RI
sjmp B0013
org 002bh
;; DATA at 002bh
db  30h,0ceh, 12h, 30h,0cfh, 14h, 80h, 08h, 20h, 07h, 55h, 30h,0dfh, 68h, 80h, 21h ;; 002b:0  0      U0 h !
db 0e5h,0cbh, 20h,0e7h, 05h,0c2h,0cfh, 31h,0dfh, 32h,0c2h,0ceh,0d5h, 0fh, 11h, 75h ;; 003b:       1 2     u
db  0fh, 14h, 85h,0cah, 38h, 85h,0cbh, 39h, 85h, 2bh, 3ah, 85h, 2ch, 3bh,0d2h, 08h ;; 004b:    8  9 +: ,;  
db  32h,0d2h, 07h, 30h,0d8h, 06h,0e5h,0fah,0a2h,0e7h, 92h, 00h                     ;; 005b:2  0        
B0067:
   jnb 0d9h, B0070
   mov A, 0fbh
   mov C, ACC.7
   mov 01h, C
B0070:
   jnb 0dah, B0079
   mov A, 0fch
   mov C, ACC.7
   mov 02h, C
B0079:
   jnb 0dbh, B0082
   mov A, 0fdh
   mov C, ACC.7
   mov 03h, C
B0082:
   jnb 0dch, B008b
   mov A, 0feh
   mov C, ACC.7
   mov 04h, C
B008b:
   jbc 00h, B0109
   jbc 01h, B00f2
   jbc 02h, B00db
   jbc 03h, B00c4
   jbc 04h, B00ad
   clr 07h
   clr 0dfh
   acall B01c2
reti
;; DATA at 00a1h
db  10h,0d8h, 67h, 10h,0d9h, 4dh, 10h,0dah, 33h, 10h,0dbh, 19h                     ;; 00a1:  g  M  3   
B00ad:
   clr 0dch
   djnz 14h, B00c3
   mov 14h, #14h
   mov 4ch, 0eeh
   mov 4dh, 0feh
   mov 4eh, 29h
   mov 4fh, 2ah
   setb 0dh
B00c3:
reti
B00c4:
   clr 0dbh
   djnz 13h, B00da
   mov 13h, #14h
   mov 48h, 0edh
   mov 49h, 0fdh
   mov 4ah, 29h
   mov 4bh, 2ah
   setb 0ch
B00da:
reti
B00db:
   clr 0dah
   djnz 12h, B00f1
   mov 12h, #14h
   mov 44h, 0ech
   mov 45h, 0fch
   mov 46h, 29h
   mov 47h, 2ah
   setb 0bh
B00f1:
reti
B00f2:
   clr 0d9h
   djnz 11h, B0108
   mov 11h, #14h
   mov 40h, 0ebh
   mov 41h, 0fbh
   mov 42h, 29h
   mov 43h, 2ah
   setb 0ah
B0108:
reti
B0109:
   clr 0d8h
   djnz 10h, B011f
   mov 10h, #14h
   mov 3ch, 0eah
   mov 3dh, 0fah
   mov 3eh, 29h
   mov 3fh, 2ah
   setb 09h
B011f:
reti
;; DATA at 0120h
db  76h, 00h, 08h,0dah,0fbh, 22h,0d3h,0e6h, 34h, 00h,0f6h, 08h,0dah,0f9h, 22h      ;; 0120:v    "  4     "
B012f:
   mov A, @R1
   inc R1
B0131:
   mov @R0, A
   inc R0
   djnz R2, B012f
ret
;; DATA at 0136h
db 0c3h,0e6h, 97h, 09h,0f6h, 08h,0dah,0f9h, 22h, 7dh, 00h,0a8h, 26h,0e6h,0fah, 05h ;; 0136:        "}  &   
db  26h,0a8h, 28h, 05h, 28h,0a9h, 27h,0abh, 25h, 7ch, 00h, 60h, 23h,0eah, 87h,0f0h ;; 0146:& ( ( ' %| `#   
db  09h,0a4h, 2ch,0c5h,0f0h, 34h, 00h,0c5h,0f0h, 26h,0f6h, 08h,0e5h,0f0h, 34h, 00h ;; 0156:  ,  4   &    4 
db 0fch,0dbh,0eah,0abh, 24h,0ech, 26h,0f6h, 08h, 50h, 05h, 74h, 01h,0dbh,0f7h, 0dh ;; 0166:    $ &  P t    
db 0d5h, 24h,0c8h, 22h                                                             ;; 0176: $ "
B017a:
   mov 0d9h, #01h
   mov 0dah, #11h
   mov 0dbh, #11h
   mov 0dch, #11h
   mov 0ddh, #11h
   mov 0deh, #11h
   setb P1.3
   setb P1.4
   setb P1.5
   setb P1.6
   setb P1.7
   clr IP.6
ret
B0199:
   setb T2EX
   setb CP_RL2
   setb EXEN2
   clr C_T2
   clr PT2
ret
B01a4:
   mov A, TMOD
   anl A, #0f0h
   add A, #01h
   mov TMOD, A
   clr PT0
ret
;; DATA at 01afh
db  75h,0e9h, 00h, 75h,0f9h, 00h, 75h, 29h, 00h, 75h, 2ah, 00h,0c2h,0dfh,0d2h,0aeh ;; 01af:u  u  u) u*     
db 0d2h,0deh                                                                       ;; 01bf:  
B01c1:
ret
B01c2:
   inc 29h
   mov A, 29h
   jnz B01ca
   inc 2ah
B01ca:
sjmp B01c1
;; DATA at 01cch
db  75h,0cch, 00h, 75h,0cdh, 00h, 75h, 2bh, 00h, 75h, 2ch, 00h,0c2h,0cfh,0d2h,0adh ;; 01cc:u  u  u+ u,     
db 0d2h,0cah, 22h, 05h, 2bh,0e5h, 2bh, 70h, 02h, 05h, 2ch, 80h,0f5h, 85h,0cch, 38h ;; 01dc:  " + +p  ,    8
db  85h,0cdh, 39h, 85h, 2bh, 3ah, 85h, 2ch, 3bh, 85h,0e9h, 3ch, 85h,0f9h, 3dh, 85h ;; 01ec:  9 +: ,;  <  = 
db  29h, 3eh, 85h, 2ah, 3fh, 85h,0e9h, 40h, 85h,0f9h, 41h, 85h, 29h, 42h, 85h, 2ah ;; 01fc:)> *?  @  A )B *
db  43h, 85h,0e9h, 44h, 85h,0f9h, 45h, 85h, 29h, 46h, 85h, 2ah, 47h, 85h,0e9h, 48h ;; 020c:C  D  E )F *G  H
db  85h,0f9h, 49h, 85h, 29h, 4ah, 85h, 2ah, 4bh, 85h,0e9h, 4ch, 85h,0f9h, 4dh, 85h ;; 021c:  I )J *K  L  M 
db  29h, 4eh, 85h, 2ah, 4fh, 22h, 00h, 80h, 15h,0c0h, 2bh, 00h, 3fh, 40h, 55h, 80h ;; 022c:)N *O"    + ?@U 
db  6ah,0c0h,0e5h, 23h, 23h,0f5h,0f0h, 90h, 02h, 32h, 93h,0c5h,0f0h, 04h, 93h,0f5h ;; 023c:j  ##    2      
db  34h, 85h,0f0h, 35h, 22h,0e5h, 23h, 23h, 24h, 50h,0f8h,0e6h, 8ah,0f0h,0a4h,0f5h ;; 024c:4  5" ##$P      
db  36h, 85h,0f0h, 37h, 08h,0e6h, 8ah,0f0h,0a4h, 25h, 37h,0c5h, 37h, 22h,0e5h, 34h ;; 025c:6  7     %7 7" 4
db  25h, 36h,0f5h, 82h,0e5h, 35h, 35h, 37h,0f5h, 83h,0e6h, 08h,0f0h,0a3h,0dah,0fah ;; 026c:%6   557        
db  22h, 78h, 0fh, 7ah, 06h, 76h, 01h, 08h,0dah,0fbh, 75h, 22h, 3fh, 78h, 38h, 7ah ;; 027c:"x z v    u"?x8z
db  84h, 31h, 20h, 22h,0e5h, 23h, 23h, 23h, 24h, 5ch,0f8h, 24h,0dch,0f9h, 7ah, 04h ;; 028c: 1 " ###$\ $  z 
db  31h, 2fh,0e5h, 23h, 23h, 23h, 24h, 74h,0f8h, 24h,0c4h,0f9h, 7ah, 04h, 31h, 2fh ;; 029c:1/ ###$t $  z 1/
db  22h, 78h, 30h,0e5h, 23h, 23h, 23h, 24h, 38h,0f9h, 7ah, 04h, 31h, 2fh, 78h, 30h ;; 02ac:"x0 ###$8 z 1/x0
db 0e5h, 23h, 23h, 23h, 24h, 74h,0f9h, 7ah, 04h, 31h, 36h, 51h, 3eh, 7ah, 04h, 51h ;; 02bc: ###$t z 16Q>z Q
db  51h, 78h, 30h, 7ah, 04h, 51h, 6ah,0e5h, 23h, 23h, 24h, 50h,0f8h, 7ah, 02h, 31h ;; 02cc:Qx0z Qj ##$P z 1
db  26h, 75h, 26h, 30h, 75h, 24h, 04h, 75h, 27h, 30h, 75h, 25h, 04h,0e5h, 23h, 23h ;; 02dc:&u&0u$ u'0u%  ##
db  23h, 23h, 24h, 8ch,0f5h, 28h, 31h, 3fh, 80h,0a8h                               ;; 02ec:##$  (1?  
B02f6:
   setb INT0
   setb IT0
   clr PX0
ret
;; DATA at 02fdh
db 0c2h,0ceh,0c2h,0d8h,0c2h,0d9h,0c2h,0dah,0c2h,0dbh,0c2h,0dch, 75h, 21h, 00h, 75h ;; 02fd:            u! u
db  20h, 00h, 22h                                                                  ;; 030d:  "
B0310:
   acall B02f6
   acall B0199
   acall B017a
   acall B01a4
   mov 22h, #40h
ret
;; DATA at 031ch
db  51h, 7dh,0c2h, 89h,0d2h,0a8h, 78h, 0ch, 91h,0fbh, 31h,0cch, 31h,0afh, 51h,0fdh ;; 031c:Q}    x   1 1 Q 
db  91h, 73h,0c2h, 89h, 78h, 0ch, 91h,0fbh,0c2h,0a8h,0c2h, 8ch,0c2h,0a9h,0c2h,0cah ;; 032c: s  x           
db 0c2h,0adh,0c2h,0deh,0c2h,0aeh, 31h,0e9h, 22h, 74h, 23h,0b1h, 41h, 74h, 17h,0b1h ;; 033c:      1 "t# At  
db  41h,0e5h, 22h,0b1h, 41h,0abh, 22h,0e5h, 23h, 23h,0ffh, 24h, 50h,0f5h, 34h, 7ah ;; 034c:A " A " ## $P 4z
db  02h, 71h, 8ch, 78h, 30h,0efh, 23h,0ffh, 24h, 38h,0f5h, 34h, 7ah, 04h, 71h, 8ch ;; 035c: q x0 # $8 4z q 
db 0efh, 24h, 5ch,0f5h, 34h, 7ah, 04h, 71h, 8ch,0efh, 24h, 74h,0f5h, 34h, 7ah, 04h ;; 036c: $\ 4z q  $t 4z 
db  71h, 8ch,0efh, 23h, 24h, 8ch,0f5h, 34h, 7ah, 08h, 71h, 8ch,0ebh,0b1h, 41h, 22h ;; 037c:q  #$  4z q   A"
db 0a8h, 34h, 05h, 34h,0e6h,0cbh, 2bh,0cbh,0b1h, 41h,0dah,0f4h, 22h, 7ch, 03h,0e5h ;; 038c: 4 4  +  A  "|  
db  36h, 45h, 37h, 70h, 05h, 74h, 2eh,0b1h, 41h, 22h, 74h, 3ah,0b1h, 41h,0e5h, 36h ;; 039c:6E7p t. A"t: A 6
db 0b1h, 41h, 7bh, 00h, 85h, 34h, 82h, 85h, 35h, 83h,0aah, 36h,0e0h,0a3h,0cbh, 2bh ;; 03ac: A{  4  5  6   +
db 0cbh,0b1h, 41h,0dah,0f7h,0ebh,0b1h, 41h,0b1h, 2eh,0b4h, 1bh, 05h,0dch, 01h, 22h ;; 03bc:  A    A .     "
db  61h,0a6h,0b4h, 1ch, 13h,0e5h, 36h, 60h, 05h, 75h, 36h, 00h, 80h, 02h, 15h, 37h ;; 03cc:a     6` u6    7
db  85h, 82h, 34h, 85h, 83h, 35h, 61h, 99h, 22h,0b1h, 0dh,0b1h, 2eh,0f5h, 23h, 54h ;; 03dc:  4  5a "   . #T
db  38h,0c5h, 23h, 54h, 07h,0c5h, 23h,0b4h, 00h, 0eh, 20h,0a8h,0eeh,0e5h, 23h,0b4h ;; 03ec:8 #T  #       # 
db  06h, 00h, 50h,0e7h, 71h, 45h, 80h,0e3h,0b4h, 08h, 14h, 20h,0a8h,0ddh,0e5h, 23h ;; 03fc:  P qE         #
db 0b4h, 06h, 00h, 50h,0d6h, 51h, 3eh, 7ah, 04h, 51h, 51h, 71h, 99h, 80h,0cch,0b4h ;; 040c:   P Q>z QQq    
db  10h, 19h,0e5h, 23h,0b4h, 00h,0c4h, 30h,0adh, 04h, 74h, 32h, 80h, 09h, 30h,0a8h ;; 041c:   #   0  t2  0 
db  04h, 74h, 31h, 80h, 02h, 74h, 30h,0b1h, 41h, 80h,0b0h,0b4h, 18h, 2bh,0e5h, 23h ;; 042c: t1  t0 A    + #
db 0b4h, 00h, 0ch, 20h,0a8h,0a5h, 78h,0cch, 90h, 03h, 1ch, 91h,0e4h, 80h, 9ch,0b4h ;; 043c:      x         
db  07h, 15h, 30h,0a8h, 96h,0c2h,0a8h,0c2h, 8ch,0c2h,0a9h,0c2h,0cah,0c2h,0adh,0c2h ;; 044c:  0             
db 0deh,0c2h,0aeh, 31h,0e9h,0d2h, 16h, 61h,0e7h, 61h,0e7h                          ;; 045c:   1   a a 
B0467:
   acall B0310
   mov R0, #0bch
   mov DPTR, #03e5h
   acall B04e4
   setb EA
ret
;; DATA at 0473h
db  75h, 8ah, 30h, 75h, 8ch,0f8h,0c2h, 8dh,0d2h, 8ch,0d2h,0a9h, 22h,0c2h,0a9h,0c2h ;; 0473:u 0u        "   
db  8ch, 10h, 08h, 19h, 10h, 09h, 1eh, 10h, 0ah, 23h, 10h, 0bh, 28h, 10h, 0ch, 2dh ;; 0483:         #  (  -
db  10h, 0dh, 32h, 80h,0dbh, 51h, 90h, 80h,0d7h, 51h,0adh, 80h,0d3h, 75h, 23h, 00h ;; 0493:  2  Q   Q   u# 
db  10h, 10h,0f2h, 80h,0f4h, 75h, 23h, 01h, 10h, 11h,0eah, 80h,0ech, 75h, 23h, 02h ;; 04a3:     u#      u# 
db  10h, 12h,0e2h, 80h,0e4h, 75h, 23h, 03h, 10h, 13h,0dah, 80h,0dch, 75h, 23h, 04h ;; 04b3:     u#      u# 
db  10h, 14h,0d2h, 80h,0d4h, 75h, 23h, 05h, 10h, 15h,0cah, 80h,0cch                ;; 04c3:     u#      
B04d0:
   mov 08h, #09h
   mov SP, #0dbh
   mov DPTR, #04efh
   push DPL
   push DPH
   acall B0467
B04df:
   orl PCON, #01h
sjmp B04df
B04e4:
   mov R1, 08h
   mov @R1, SP
   inc 08h
   dec R0
   mov SP, R0
   acall B04f6
   dec 08h
   mov R0, 08h
   mov SP, @R0
ret
B04f6:
   push DPL
   push DPH
ret
;; DATA at 04fbh
db 0a6h, 81h, 15h, 08h,0a8h, 08h, 86h, 81h, 22h                                    ;; 04fb:        "
C0504:
   mov R1, 08h
   mov @R1, SP
   inc 08h
   mov SP, @R0
ret
;; DATA at 050dh
db  75h, 98h,0c0h,0c2h,0bch,0c2h,0cdh,0c2h,0cch,0e5h, 89h, 54h, 0fh, 24h, 20h,0f5h ;; 050d:u          T $  
db  89h, 75h, 8dh,0fdh,0d2h, 8eh,0c2h, 9bh,0c2h, 98h,0c2h, 99h,0d2h, 9dh,0d2h,0ach ;; 051d: u              
db  22h, 30h,0b4h, 04h,0c2h,0b4h,0d2h, 9ch, 78h, 0dh, 91h,0fbh,0e5h, 99h, 30h,0e6h ;; 052d:"0      x     0 
db 0f0h, 54h, 3fh, 22h, 20h,0b4h, 08h, 7eh, 64h,0deh,0feh,0c2h, 9ch,0d2h,0b4h,0f5h ;; 053d: T?"   ~d       
db  99h, 78h, 0eh, 91h,0fbh, 22h                                                   ;; 054d: x   "
