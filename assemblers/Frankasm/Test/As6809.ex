00007890 lbtarget         00000099 stuff            00000055 direct           
00006543 extended         00006543 extend           00000567 long             
0000002d middle           fffffffb short            0000005b btarget          
 0x7890                 lbtarget equ 7890h
 0x99                   stuff equ $99
 0x55                   direct equ 55h
 0x6543                 extended equ 6543h
 0x6543                 extend equ extended
 0x567                  long equ 567h
 0x2d                   middle equ 45
 0xfffffffb             short equ -5
0000 3a 			 abx
0001 89 99 			 adca # stuff
0003 a9 a4 			 adca ,y
0005 99 55 			 adca direct
0007 b9 65 43 			 adca extended
000a c9 99 			 adcb # stuff
000c e9 a4 			 adcb ,y
000e d9 55 			 adcb direct
0010 f9 65 43 			 adcb extended
0013 8b 99 			 adda # stuff
0015 ab a4 			 adda ,y
0017 9b 55 			 adda direct
0019 bb 65 43 			 adda extended
001c cb 99 			 addb # stuff
001e eb a4 			 addb ,y
0020 db 55 			 addb direct
0022 fb 65 43 			 addb extended
0025 c3 00 99 			 addd # stuff
0028 e3 a4 			 addd ,y
002a d3 55 			 addd direct
002c f3 65 43 			 addd extended
002f 84 99 			 anda # stuff
0031 a4 a4 			 anda ,y
0033 94 55 			 anda direct
0035 b4 65 43 			 anda extended
0038 c4 99 			 andb # stuff
003a e4 a4 			 andb ,y
003c d4 55 			 andb direct
003e f4 65 43 			 andb extended
0041 1c 99 			 andcc # stuff
0043 68 a4 			 asl ,y
0045 08 55 			 asl direct
0047 78 65 43 			 asl extend
004a 48 			 asla
004b 58 			 aslb
004c 67 a4 			 asr ,y
004e 07 55 			 asr direct
0050 77 65 43 			 asr extend
0053 47 			 asra
0054 57 			 asrb
0055 24 04 			 bcc btarget
0057 25 02 			 bcs btarget
0059 27 00 			 beq btarget
005b 2c fe 		btarget bge btarget
005d 2e fc 			 bgt btarget
005f 22 fa 			 bhi btarget
0061 24 f8 			 bhs btarget
0063 85 99 			 bita # stuff
0065 a5 a4 			 bita ,y
0067 95 55 			 bita direct
0069 b5 65 43 			 bita extended
006c c5 99 			 bitb # stuff
006e e5 a4 			 bitb ,y
0070 d5 55 			 bitb direct
0072 f5 65 43 			 bitb extended
0075 2f e4 			 ble btarget
0077 25 e2 			 blo btarget
0079 23 e0 			 bls btarget
007b 2d de 			 blt btarget
007d 2b dc 			 bmi btarget
007f 26 da 			 bne btarget
0081 2a d8 			 bpl btarget
0083 20 d6 			 bra btarget
0085 21 d4 			 brn btarget
0087 8d d2 			 bsr btarget
0089 28 d0 			 bvc btarget
008b 29 ce 			 bvs btarget
008d 6f a4 			 clr ,y
008f 0f 55 			 clr direct
0091 7f 65 43 			 clr extend
0094 4f 			 clra
0095 5f 			 clrb
0096 81 99 			 cmpa # stuff
0098 a1 a4 			 cmpa ,y
009a 91 55 			 cmpa direct
009c b1 65 43 			 cmpa extended
009f c1 99 			 cmpb # stuff
00a1 e1 a4 			 cmpb ,y
00a3 d1 55 			 cmpb direct
00a5 f1 65 43 			 cmpb extended
00a8 10 83 00 99 		 cmpd # stuff
00ac 10 a3 a4 			 cmpd ,y
00af 10 93 55 			 cmpd direct
00b2 10 b3 65 43 		 cmpd extended
00b6 11 8c 00 99 		 cmps # stuff
00ba 11 ac a4 			 cmps ,y
00bd 11 9c 55 			 cmps direct
00c0 11 bc 65 43 		 cmps extended
00c4 11 83 00 99 		 cmpu # stuff
00c8 11 a3 a4 			 cmpu ,y
00cb 11 93 55 			 cmpu direct
00ce 11 b3 65 43 		 cmpu extended
00d2 8c 00 99 			 cmpx # stuff
00d5 ac a4 			 cmpx ,y
00d7 9c 55 			 cmpx direct
00d9 bc 65 43 			 cmpx extended
00dc 10 8c 00 99 		 cmpy # stuff
00e0 10 ac a4 			 cmpy ,y
00e3 10 9c 55 			 cmpy direct
00e6 10 bc 65 43 		 cmpy extended
00ea 63 a4 			 com ,y
00ec 03 55 			 com direct
00ee 73 65 43 			 com extend
00f1 43 			 coma
00f2 53 			 comb
00f3 3c 99 			 cwai # stuff
00f5 19 			 daa
00f6 6a a4 			 dec ,y
00f8 0a 55 			 dec direct
00fa 7a 65 43 			 dec extend
00fd 4a 			 deca
00fe 5a 			 decb
00ff 88 99 			 eora # stuff
0101 a8 a4 			 eora ,y
0103 98 55 			 eora direct
0105 b8 65 43 			 eora extended
0108 c8 99 			 eorb # stuff
010a e8 a4 			 eorb ,y
010c d8 55 			 eorb direct
010e f8 65 43 			 eorb extended
0111 1e 89 			 exg a,b
0113 1e 45 			 exg s,pc
0115 1e 12 			 exg x,y
0117 6c a4 			 inc ,y
0119 0c 55 			 inc direct
011b 7c 65 43 			 inc extend
011e 4c 			 inca
011f 5c 			 incb
0120 6e a4 			 jmp ,y
0122 0e 55 			 jmp direct
0124 7e 65 43 			 jmp extend
0127 ad a4 			 jsr ,y
0129 9d 55 			 jsr direct
012b bd 65 43 			 jsr extend
012e 10 24 77 5e 		 lbcc lbtarget
0132 10 25 77 5a 		 lbcs lbtarget
0136 10 27 77 56 		 lbeq lbtarget
013a 10 2c 77 52 		 lbge lbtarget
013e 10 2e 77 4e 		 lbgt lbtarget
0142 10 22 77 4a 		 lbhi lbtarget
0146 10 24 77 46 		 lbhs lbtarget
014a 10 2f 77 42 		 lble lbtarget
014e 10 25 77 3e 		 lblo lbtarget
0152 10 23 77 3a 		 lbls lbtarget
0156 10 2d 77 36 		 lblt lbtarget
015a 10 2b 77 32 		 lbmi lbtarget
015e 10 26 77 2e 		 lbne lbtarget
0162 10 2a 77 2a 		 lbpl lbtarget
0166 16 77 27 			 lbra lbtarget
0169 10 21 77 23 		 lbrn lbtarget
016d 17 77 20 			 lbsr lbtarget
0170 10 28 77 1c 		 lbvc lbtarget
0174 10 29 77 18 		 lbvs lbtarget
0178 86 99 			 lda # stuff
017a a6 a4 			 lda ,y
017c 96 55 			 lda direct
017e b6 65 43 			 lda extended
0181 c6 99 			 ldb # stuff
0183 e6 a4 			 ldb ,y
0185 d6 55 			 ldb direct
0187 f6 65 43 			 ldb extended
018a cc 00 99 			 ldd # stuff
018d ec a4 			 ldd ,y
018f dc 55 			 ldd direct
0191 fc 65 43 			 ldd extended
0194 10 ce 00 99 		 lds # stuff
0198 10 ee a4 			 lds ,y
019b 10 de 55 			 lds direct
019e 10 fe 65 43 		 lds extended
01a2 ce 00 99 			 ldu # stuff
01a5 ee a4 			 ldu ,y
01a7 de 55 			 ldu direct
01a9 fe 65 43 			 ldu extended
01ac 8e 00 99 			 ldx # stuff
01af ae a4 			 ldx ,y
01b1 9e 55 			 ldx direct
01b3 be 65 43 			 ldx extended
01b6 10 8e 00 99 		 ldy # stuff
01ba 10 ae a4 			 ldy ,y
01bd 10 9e 55 			 ldy direct
01c0 10 be 65 43 		 ldy extended
01c4 32 e3 			 leas ,--s
01c6 33 e3 			 leau ,--s
01c8 30 e3 			 leax ,--s
01ca 31 e3 			 leay ,--s
01cc 31 c3 			 leay ,--u
01ce 31 83 			 leay ,--x
01d0 31 a3 			 leay ,--y
01d2 31 e2 			 leay ,-s
01d4 31 c2 			 leay ,-u
01d6 31 82 			 leay ,-x
01d8 31 a2 			 leay ,-y
01da 31 e4 			 leay ,s
01dc 31 e0 			 leay ,s+
01de 31 e1 			 leay ,s++
01e0 31 c4 			 leay ,u
01e2 31 c0 			 leay ,u+
01e4 31 c1 			 leay ,u++
01e6 31 84 			 leay ,x
01e8 31 80 			 leay ,x+
01ea 31 81 			 leay ,x++
01ec 31 a4 			 leay ,y
01ee 31 a0 			 leay ,y+
01f0 31 a1 			 leay ,y++
01f2 31 f3 			 leay [,--s]
01f4 31 d3 			 leay [,--u]
01f6 31 93 			 leay [,--x]
01f8 31 b3 			 leay [,--y]
01fa 31 f1 			 leay [,s++]
01fc 31 f4 			 leay [,s]
01fe 31 d1 			 leay [,u++]
0200 31 d4 			 leay [,u]
0202 31 91 			 leay [,x++]
0204 31 94 			 leay [,x]
0206 31 b1 			 leay [,y++]
0208 31 b4 			 leay [,y]
020a 31 f6 			 leay [a,s]
020c 31 d6 			 leay [a,u]
020e 31 96 			 leay [a,x]
0210 31 b6 			 leay [a,y]
0212 31 f5 			 leay [b,s]
0214 31 d5 			 leay [b,u]
0216 31 95 			 leay [b,x]
0218 31 b5 			 leay [b,y]
021a 31 fb 			 leay [d,s]
021c 31 db 			 leay [d,u]
021e 31 9b 			 leay [d,x]
0220 31 bb 			 leay [d,y]
0222 31 f9 05 67 		 leay [long,s]
0226 31 d9 05 67 		 leay [long,u]
022a 31 99 05 67 		 leay [long,x]
022e 31 b9 05 67 		 leay [long,y]
0232 31 9f 05 67 		 leay [long]
0236 31 f8 2d 			 leay [middle,s]
0239 31 d8 2d 			 leay [middle,u]
023c 31 98 2d 			 leay [middle,x]
023f 31 b8 2d 			 leay [middle,y]
0242 31 9d 34 52 		 leay [*+3456h,pcr]
0246 31 9c 64 			 leay [*+67h,pcr]
0249 31 f8 fb 			 leay [short,s]
024c 31 d8 fb 			 leay [short,u]
024f 31 98 fb 			 leay [short,x]
0252 31 b8 fb 			 leay [short,y]
0255 31 e6 			 leay a,s
0257 31 c6 			 leay a,u
0259 31 86 			 leay a,x
025b 31 a6 			 leay a,y
025d 31 e5 			 leay b,s
025f 31 c5 			 leay b,u
0261 31 85 			 leay b,x
0263 31 a5 			 leay b,y
0265 31 eb 			 leay d,s
0267 31 cb 			 leay d,u
0269 31 8b 			 leay d,x
026b 31 ab 			 leay d,y
026d 31 e9 05 67 		 leay long,s
0271 31 c9 05 67 		 leay long,u
0275 31 89 05 67 		 leay long,x
0279 31 a9 05 67 		 leay long,y
027d 31 e8 2d 			 leay middle,s
0280 31 c8 2d 			 leay middle,u
0283 31 88 2d 			 leay middle,x
0286 31 a8 2d 			 leay middle,y
0289 31 8d 34 52 		 leay *+3456h,pcr
028d 31 8c 64 			 leay *+67h,pcr
0290 31 7b 			 leay short,s
0292 31 5b 			 leay short,u
0294 31 1b 			 leay short,x
0296 31 3b 			 leay short,y
0298 68 a4 			 lsl ,y
029a 08 55 			 lsl direct
029c 78 65 43 			 lsl extend
029f 48 			 lsla
02a0 58 			 lslb
02a1 64 a4 			 lsr ,y
02a3 04 55 			 lsr direct
02a5 74 65 43 			 lsr extend
02a8 44 			 lsra
02a9 54 			 lsrb
02aa 3d 			 mul
02ab 60 a4 			 neg ,y
02ad 00 55 			 neg direct
02af 70 65 43 			 neg extend
02b2 40 			 nega
02b3 50 			 negb
02b4 12 			 nop
02b5 8a 99 			 ora # stuff
02b7 aa a4 			 ora ,y
02b9 9a 55 			 ora direct
02bb ba 65 43 			 ora extended
02be ca 99 			 orb # stuff
02c0 ea a4 			 orb ,y
02c2 da 55 			 orb direct
02c4 fa 65 43 			 orb extended
02c7 1a 99 			 orcc # stuff
02c9 34 17 			 pshs a,b,cc,x
02cb 36 17 			 pshu a,b,cc,x
02cd 35 17 			 puls a,b,cc,x
02cf 37 17 			 pulu a,b,cc,x
02d1 69 a4 			 rol ,y
02d3 09 55 			 rol direct
02d5 79 65 43 			 rol extend
02d8 49 			 rola
02d9 59 			 rolb
02da 66 a4 			 ror ,y
02dc 06 55 			 ror direct
02de 76 65 43 			 ror extend
02e1 46 			 rora
02e2 56 			 rorb
02e3 3b 			 rti
02e4 39 			 rts
02e5 82 99 			 sbca # stuff
02e7 a2 a4 			 sbca ,y
02e9 92 55 			 sbca direct
02eb b2 65 43 			 sbca extended
02ee c2 99 			 sbcb # stuff
02f0 e2 a4 			 sbcb ,y
02f2 d2 55 			 sbcb direct
02f4 f2 65 43 			 sbcb extended
02f7 1d 			 sex
02f8 a7 a4 			 sta ,y
02fa 97 55 			 sta direct
02fc b7 65 43 			 sta extended
02ff e7 a4 			 stb ,y
0301 d7 55 			 stb direct
0303 f7 65 43 			 stb extended
0306 ed a4 			 std ,y
0308 dd 55 			 std direct
030a fd 65 43 			 std extended
030d 10 ef a4 			 sts ,y
0310 10 df 55 			 sts direct
0313 10 ff 65 43 		 sts extended
0317 ef a4 			 stu ,y
0319 df 55 			 stu direct
031b ff 65 43 			 stu extended
031e af a4 			 stx ,y
0320 9f 55 			 stx direct
0322 bf 65 43 			 stx extended
0325 10 af a4 			 sty ,y
0328 10 9f 55 			 sty direct
032b 10 bf 65 43 		 sty extended
032f 80 99 			 suba # stuff
0331 a0 a4 			 suba ,y
0333 90 55 			 suba direct
0335 b0 65 43 			 suba extended
0338 c0 99 			 subb # stuff
033a e0 a4 			 subb ,y
033c d0 55 			 subb direct
033e f0 65 43 			 subb extended
0341 83 00 99 			 subd # stuff
0344 a3 a4 			 subd ,y
0346 93 55 			 subd direct
0348 b3 65 43 			 subd extended
034b 3f 			 swi
034c 10 3f 			 swi2
034e 11 3f 			 swi3
0350 13 			 sync
0351 1f 89 			 tfr a,b
0353 1f 45 			 tfr s,pc
0355 1f 12 			 tfr x,y
0357 6d a4 			 tst ,y
0359 0d 55 			 tst direct
035b 7d 65 43 			 tst extend
035e 4d 			 tsta
035f 5d 			 tstb
