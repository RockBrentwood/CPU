org 0
B0000:
ajmp B04d0
;; DATA at 0002h
 0 78  c b1  4 32  0  0  0 91 80 32            | x   2     2    |
org 000eh
B000e:
   mov R0, #0eh
   acall C0504
reti
B0013:
   mov R0, #0dh
   acall C0504
reti
;; DATA at 0018h
 0  0  0  0  0  0  0  0  0  0  0               |                |
org 0023h
B0023:
   jbc TI, B000e
   clr RI
sjmp B0013
;; DATA at 002ah
 0 30 ce 12 30 cf 14 80  8 20  7 55 30 df 68 80| 0  0      U0 h |
21 e5 cb 20 e7  5 c2 cf 31 df 32 c2 ce d5  f 11|!       1 2     |
75  f 14 85 ca 38 85 cb 39 85 2b 3a 85 2c 3b d2|u    8  9 +: ,; |
 8 32 d2  7 30 d8  6 e5 fa a2 e7 92  0         | 2  0           |
org 0067h
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
10 d8 67 10 d9 4d 10 da 33 10 db 19            |  g  M  3       |
org 00adh
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
76  0  8 da fb 22 d3 e6 34  0 f6  8 da f9 22   |v    "  4     " |
org 012fh
B012f:
   mov A, @R1
   inc R1
B0131:
   mov @R0, A
   inc R0
   djnz R2, B012f
ret
;; DATA at 0136h
c3 e6 97  9 f6  8 da f9 22 7d  0 a8 26 e6 fa  5|        "}  &   |
26 a8 28  5 28 a9 27 ab 25 7c  0 60 23 ea 87 f0|& ( ( ' %| `#   |
 9 a4 2c c5 f0 34  0 c5 f0 26 f6  8 e5 f0 34  0|  ,  4   &    4 |
fc db ea ab 24 ec 26 f6  8 50  5 74  1 db f7  d|    $ &  P t    |
d5 24 c8 22                                    | $ "            |
org 017ah
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
75 e9  0 75 f9  0 75 29  0 75 2a  0 c2 df d2 ae|u  u  u) u*     |
d2 de                                          |                |
org 01c1h
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
75 cc  0 75 cd  0 75 2b  0 75 2c  0 c2 cf d2 ad|u  u  u+ u,     |
d2 ca 22  5 2b e5 2b 70  2  5 2c 80 f5 85 cc 38|  " + +p  ,    8|
85 cd 39 85 2b 3a 85 2c 3b 85 e9 3c 85 f9 3d 85|  9 +: ,;  <  = |
29 3e 85 2a 3f 85 e9 40 85 f9 41 85 29 42 85 2a|)> *?  @  A )B *|
43 85 e9 44 85 f9 45 85 29 46 85 2a 47 85 e9 48|C  D  E )F *G  H|
85 f9 49 85 29 4a 85 2a 4b 85 e9 4c 85 f9 4d 85|  I )J *K  L  M |
29 4e 85 2a 4f 22  0 80 15 c0 2b  0 3f 40 55 80|)N *O"    + ?@U |
6a c0 e5 23 23 f5 f0 90  2 32 93 c5 f0  4 93 f5|j  ##    2      |
34 85 f0 35 22 e5 23 23 24 50 f8 e6 8a f0 a4 f5|4  5" ##$P      |
36 85 f0 37  8 e6 8a f0 a4 25 37 c5 37 22 e5 34|6  7     %7 7" 4|
25 36 f5 82 e5 35 35 37 f5 83 e6  8 f0 a3 da fa|%6   557        |
22 78  f 7a  6 76  1  8 da fb 75 22 3f 78 38 7a|"x z v    u"?x8z|
84 31 20 22 e5 23 23 23 24 5c f8 24 dc f9 7a  4| 1 " ###$\ $  z |
31 2f e5 23 23 23 24 74 f8 24 c4 f9 7a  4 31 2f|1/ ###$t $  z 1/|
22 78 30 e5 23 23 23 24 38 f9 7a  4 31 2f 78 30|"x0 ###$8 z 1/x0|
e5 23 23 23 24 74 f9 7a  4 31 36 51 3e 7a  4 51| ###$t z 16Q>z Q|
51 78 30 7a  4 51 6a e5 23 23 24 50 f8 7a  2 31|Qx0z Qj ##$P z 1|
26 75 26 30 75 24  4 75 27 30 75 25  4 e5 23 23|&u&0u$ u'0u%  ##|
23 23 24 8c f5 28 31 3f 80 a8                  |##$  (1?        |
org 02f6h
B02f6:
   setb INT0
   setb IT0
   clr PX0
ret
;; DATA at 02fdh
c2 ce c2 d8 c2 d9 c2 da c2 db c2 dc 75 21  0 75|            u! u|
20  0 22                                       |  "             |
org 0310h
B0310:
   acall B02f6
   acall B0199
   acall B017a
   acall B01a4
   mov 22h, #40h
ret
;; DATA at 031ch
51 7d c2 89 d2 a8 78  c 91 fb 31 cc 31 af 51 fd|Q}    x   1 1 Q |
91 73 c2 89 78  c 91 fb c2 a8 c2 8c c2 a9 c2 ca| s  x           |
c2 ad c2 de c2 ae 31 e9 22 74 23 b1 41 74 17 b1|      1 "t# At  |
41 e5 22 b1 41 ab 22 e5 23 23 ff 24 50 f5 34 7a|A " A " ## $P 4z|
 2 71 8c 78 30 ef 23 ff 24 38 f5 34 7a  4 71 8c| q x0 # $8 4z q |
ef 24 5c f5 34 7a  4 71 8c ef 24 74 f5 34 7a  4| $\ 4z q  $t 4z |
71 8c ef 23 24 8c f5 34 7a  8 71 8c eb b1 41 22|q  #$  4z q   A"|
a8 34  5 34 e6 cb 2b cb b1 41 da f4 22 7c  3 e5| 4 4  +  A  "|  |
36 45 37 70  5 74 2e b1 41 22 74 3a b1 41 e5 36|6E7p t. A"t: A 6|
b1 41 7b  0 85 34 82 85 35 83 aa 36 e0 a3 cb 2b| A{  4  5  6   +|
cb b1 41 da f7 eb b1 41 b1 2e b4 1b  5 dc  1 22|  A    A .     "|
61 a6 b4 1c 13 e5 36 60  5 75 36  0 80  2 15 37|a     6` u6    7|
85 82 34 85 83 35 61 99 22 b1  d b1 2e f5 23 54|  4  5a "   . #T|
38 c5 23 54  7 c5 23 b4  0  e 20 a8 ee e5 23 b4|8 #T  #       # |
 6  0 50 e7 71 45 80 e3 b4  8 14 20 a8 dd e5 23|  P qE         #|
b4  6  0 50 d6 51 3e 7a  4 51 51 71 99 80 cc b4|   P Q>z QQq    |
10 19 e5 23 b4  0 c4 30 ad  4 74 32 80  9 30 a8|   #   0  t2  0 |
 4 74 31 80  2 74 30 b1 41 80 b0 b4 18 2b e5 23| t1  t0 A    + #|
b4  0  c 20 a8 a5 78 cc 90  3 1c 91 e4 80 9c b4|      x         |
 7 15 30 a8 96 c2 a8 c2 8c c2 a9 c2 ca c2 ad c2|  0             |
de c2 ae 31 e9 d2 16 61 e7 61 e7               |   1   a a      |
org 0467h
B0467:
   acall B0310
   mov R0, #0bch
   mov DPTR, #03e5h
   acall B04e4
   setb EA
ret
;; DATA at 0473h
75 8a 30 75 8c f8 c2 8d d2 8c d2 a9 22 c2 a9 c2|u 0u        "   |
8c 10  8 19 10  9 1e 10  a 23 10  b 28 10  c 2d|         #  (  -|
10  d 32 80 db 51 90 80 d7 51 ad 80 d3 75 23  0|  2  Q   Q   u# |
10 10 f2 80 f4 75 23  1 10 11 ea 80 ec 75 23  2|     u#      u# |
10 12 e2 80 e4 75 23  3 10 13 da 80 dc 75 23  4|     u#      u# |
10 14 d2 80 d4 75 23  5 10 15 ca 80 cc         |     u#         |
org 04d0h
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
a6 81 15  8 a8  8 86 81 22                     |        "       |
org 0504h
C0504:
   mov R1, 08h
   mov @R1, SP
   inc 08h
   mov SP, @R0
ret
;; DATA at 050dh
75 98 c0 c2 bc c2 cd c2 cc e5 89 54  f 24 20 f5|u          T $  |
89 75 8d fd d2 8e c2 9b c2 98 c2 99 d2 9d d2 ac| u              |
22 30 b4  4 c2 b4 d2 9c 78  d 91 fb e5 99 30 e6|"0      x     0 |
f0 54 3f 22 20 b4  8 7e 64 de fe c2 9c d2 b4 f5| T?"   ~d       |
99 78  e 91 fb 22                              | x   "          |
