00000098 immed            00000038 srcreg           00000030 dstreg           
00007654 longoff          00000033 shortoff         00001234 extern           
00000044 srcreg2          0000000a shiftcount       0000021d jmpdst           
 0x98                   immed	equ	98h
 0x38                   srcreg	equ	38h
 0x30                   dstreg	equ	30h
 0x7654                 longoff	equ	7654h
 0x33                   shortoff	equ	33h
 0x1234                 extern	equ	1234h
 0x44                   srcreg2	equ	44h
 0xa                    shiftcount	equ	10
0000 65 98 00 30 		add	dstreg, #immed
0004 66 38 30 			add	dstreg, [ srcreg ]
0007 66 39 30 			add	dstreg, [ srcreg ] +
000a 67 01 34 12 30 		add	dstreg, extern
000f 67 39 54 76 30 		add	dstreg, longoff [ srcreg ]
0014 67 38 33 30 		add	dstreg, shortoff [ srcreg ]
0018 64 38 30 			add	dstreg, srcreg
001b 45 98 00 44 30 		add	dstreg, srcreg2, #immed
0020 46 38 44 30 		add	dstreg, srcreg2, [ srcreg ]
0024 46 39 44 30 		add	dstreg, srcreg2, [ srcreg ] +
0028 47 01 34 12 44 30 		add	dstreg, srcreg2, extern
002e 47 39 54 76 44 30 		add	dstreg, srcreg2, longoff [ srcreg ]
0034 47 38 33 44 30 		add	dstreg, srcreg2, shortoff [ srcreg ]
0039 44 38 44 30 		add	dstreg, srcreg2, srcreg
003d 75 98 30 			addb	dstreg, #immed
0040 76 38 30 			addb	dstreg, [ srcreg ]
0043 76 39 30 			addb	dstreg, [ srcreg ] +
0046 77 01 34 12 30 		addb	dstreg, extern
004b 77 39 54 76 30 		addb	dstreg, longoff [ srcreg ]
0050 77 38 33 30 		addb	dstreg, shortoff [ srcreg ]
0054 74 38 30 			addb	dstreg, srcreg
0057 55 98 44 30 		addb	dstreg, srcreg2, #immed
005b 56 38 44 30 		addb	dstreg, srcreg2, [ srcreg ]
005f 56 39 44 30 		addb	dstreg, srcreg2, [ srcreg ] +
0063 57 01 34 12 44 30 		addb	dstreg, srcreg2, extern
0069 57 39 54 76 44 30 		addb	dstreg, srcreg2, longoff [ srcreg ]
006f 57 38 33 44 30 		addb	dstreg, srcreg2, shortoff [ srcreg ]
0074 54 38 44 30 		addb	dstreg, srcreg2, srcreg
0078 a5 98 00 30 		addc	dstreg, #immed
007c a6 38 30 			addc	dstreg, [ srcreg ]
007f a6 39 30 			addc	dstreg, [ srcreg ] +
0082 a7 01 34 12 30 		addc	dstreg, extern
0087 a7 39 54 76 30 		addc	dstreg, longoff [ srcreg ]
008c a7 38 33 30 		addc	dstreg, shortoff [ srcreg ]
0090 a4 38 30 			addc	dstreg, srcreg
0093 b5 98 30 			addcb	dstreg, #immed
0096 b6 38 30 			addcb	dstreg, [ srcreg ]
0099 b6 39 30 			addcb	dstreg, [ srcreg ] +
009c b7 01 34 12 30 		addcb	dstreg, extern
00a1 b7 39 54 76 30 		addcb	dstreg, longoff [ srcreg ]
00a6 b7 38 33 30 		addcb	dstreg, shortoff [ srcreg ]
00aa b4 38 30 			addcb	dstreg, srcreg
00ad 61 98 00 30 		and	dstreg, #immed
00b1 62 38 30 			and	dstreg, [ srcreg ]
00b4 62 39 30 			and	dstreg, [ srcreg ] +
00b7 63 01 34 12 30 		and	dstreg, extern
00bc 63 39 54 76 30 		and	dstreg, longoff [ srcreg ]
00c1 63 38 33 30 		and	dstreg, shortoff [ srcreg ]
00c5 60 38 30 			and	dstreg, srcreg
00c8 41 98 00 44 30 		and	dstreg, srcreg2, #immed
00cd 42 38 44 30 		and	dstreg, srcreg2, [ srcreg ]
00d1 42 39 44 30 		and	dstreg, srcreg2, [ srcreg ] +
00d5 43 01 34 12 44 30 		and	dstreg, srcreg2, extern
00db 43 39 54 76 44 30 		and	dstreg, srcreg2, longoff [ srcreg ]
00e1 43 38 33 44 30 		and	dstreg, srcreg2, shortoff [ srcreg ]
00e6 40 38 44 30 		and	dstreg, srcreg2, srcreg
00ea 71 98 30 			andb	dstreg, #immed
00ed 72 38 30 			andb	dstreg, [ srcreg ]
00f0 72 39 30 			andb	dstreg, [ srcreg ] +
00f3 73 01 34 12 30 		andb	dstreg, extern
00f8 73 39 54 76 30 		andb	dstreg, longoff [ srcreg ]
00fd 73 38 33 30 		andb	dstreg, shortoff [ srcreg ]
0101 70 38 30 			andb	dstreg, srcreg
0104 51 98 44 30 		andb	dstreg, srcreg2, #immed
0108 52 38 44 30 		andb	dstreg, srcreg2, [ srcreg ]
010c 52 39 44 30 		andb	dstreg, srcreg2, [ srcreg ] +
0110 53 01 34 12 44 30 		andb	dstreg, srcreg2, extern
0116 53 39 54 76 44 30 		andb	dstreg, srcreg2, longoff [ srcreg ]
011c 53 38 33 44 30 		andb	dstreg, srcreg2, shortoff [ srcreg ]
0121 50 38 44 30 		andb	dstreg, srcreg2, srcreg
0125 e3 38 			br	[ srcreg ]
0127 f8 			clrc
0128 01 30 			clr	dstreg
012a 11 30 			clrb	dstreg
012c fc 			clrvt
012d 89 98 00 30 		cmp	dstreg, #immed
0131 8a 38 30 			cmp	dstreg, [ srcreg ]
0134 8a 39 30 			cmp	dstreg, [ srcreg ] +
0137 8b 01 34 12 30 		cmp	dstreg, extern
013c 8b 39 54 76 30 		cmp	dstreg, longoff [ srcreg ]
0141 8b 38 33 30 		cmp	dstreg, shortoff [ srcreg ]
0145 88 38 30 			cmp	dstreg, srcreg
0148 99 98 30 			cmpb	dstreg, #immed
014b 9a 38 30 			cmpb	dstreg, [ srcreg ]
014e 9a 39 30 			cmpb	dstreg, [ srcreg ] +
0151 9b 01 34 12 30 		cmpb	dstreg, extern
0156 9b 39 54 76 30 		cmpb	dstreg, longoff [ srcreg ]
015b 9b 38 33 30 		cmpb	dstreg, shortoff [ srcreg ]
015f 98 38 30 			cmpb	dstreg, srcreg
0162 05 30 			dec	dstreg
0164 15 30 			decb	dstreg
0166 fa 			di
0167 fe 8d 98 00 30 		div	dstreg, #immed
016c fe 8e 38 30 		div	dstreg, [ srcreg ]
0170 fe 8e 39 30 		div	dstreg, [ srcreg ] +
0174 fe 8f 01 34 12 30 		div	dstreg, extern
017a fe 8f 39 54 76 30 		div	dstreg, longoff [ srcreg ]
0180 fe 8f 38 33 30 		div	dstreg, shortoff [ srcreg ]
0185 fe 8c 38 30 		div	dstreg, srcreg
0189 fe 9d 98 30 		divb	dstreg, #immed
018d fe 9e 38 30 		divb	dstreg, [ srcreg ]
0191 fe 9e 39 30 		divb	dstreg, [ srcreg ] +
0195 fe 9f 01 34 12 30 		divb	dstreg, extern
019b fe 9f 39 54 76 30 		divb	dstreg, longoff [ srcreg ]
01a1 fe 9f 38 33 30 		divb	dstreg, shortoff [ srcreg ]
01a6 fe 9c 38 30 		divb	dstreg, srcreg
01aa 8d 98 00 30 		divu	dstreg, #immed
01ae 8e 38 30 			divu	dstreg, [ srcreg ]
01b1 8e 39 30 			divu	dstreg, [ srcreg ] +
01b4 8f 01 34 12 30 		divu	dstreg, extern
01b9 8f 39 54 76 30 		divu	dstreg, longoff [ srcreg ]
01be 8f 38 33 30 		divu	dstreg, shortoff [ srcreg ]
01c2 8c 38 30 			divu	dstreg, srcreg
01c5 9d 98 30 			divub	dstreg, #immed
01c8 9e 38 30 			divub	dstreg, [ srcreg ]
01cb 9e 39 30 			divub	dstreg, [ srcreg ] +
01ce 9f 01 34 12 30 		divub	dstreg, extern
01d3 9f 39 54 76 30 		divub	dstreg, longoff [ srcreg ]
01d8 9f 38 33 30 		divub	dstreg, shortoff [ srcreg ]
01dc 9c 38 30 			divub	dstreg, srcreg
01df e0 38 3b 			djnz	srcreg, jmpdst
01e2 fb 			ei
01e3 06 30 			ext	dstreg
01e5 16 30 			extb	dstreg
01e7 07 30 			inc	dstreg
01e9 17 30 			incb	dstreg
01eb 30 38 2f 			jbc	srcreg, 0, jmpdst
01ee 31 38 2c 			jbc	srcreg, 1, jmpdst
01f1 32 38 29 			jbc	srcreg, 2, jmpdst
01f4 33 38 26 			jbc	srcreg, 3, jmpdst
01f7 34 38 23 			jbc	srcreg, 4, jmpdst
01fa 35 38 20 			jbc	srcreg, 5, jmpdst
01fd 36 38 1d 			jbc	srcreg, 6, jmpdst
0200 37 38 1a 			jbc	srcreg, 7, jmpdst
0203 38 38 17 			jbs	srcreg, 0, jmpdst
0206 39 38 14 			jbs	srcreg, 1, jmpdst
0209 3a 38 11 			jbs	srcreg, 2, jmpdst
020c 3b 38 0e 			jbs	srcreg, 3, jmpdst
020f 3c 38 0b 			jbs	srcreg, 4, jmpdst
0212 3d 38 08 			jbs	srcreg, 5, jmpdst
0215 3e 38 05 			jbs	srcreg, 6, jmpdst
0218 3f 38 02 			jbs	srcreg, 7, jmpdst
021b db 00 			jc	jmpdst
021d df fe 		jmpdst	je	jmpdst
021f d6 fc 			jge	jmpdst
0221 d2 fa 			jgt	jmpdst
0223 d9 f8 			jh	jmpdst
0225 da f6 			jle	jmpdst
0227 de f4 			jlt	jmpdst
0229 d3 f2 			jnc	jmpdst
022b d7 f0 			jne	jmpdst
022d d1 ee 			jnh	jmpdst
022f d0 ec 			jnst	jmpdst
0231 d5 ea 			jnv	jmpdst
0233 d4 e8 			jnvt	jmpdst
0235 d8 e6 			jst	jmpdst
0237 dd e4 			jv	jmpdst
0239 dc e2 			jvt	jmpdst
023b ef df ff 			lcall	jmpdst
023e a1 98 00 30 		ld	dstreg, #immed
0242 a2 38 30 			ld	dstreg, [ srcreg ]
0245 a2 39 30 			ld	dstreg, [ srcreg ] +
0248 a3 01 34 12 30 		ld	dstreg, extern
024d a3 39 54 76 30 		ld	dstreg, longoff [ srcreg ]
0252 a3 38 33 30 		ld	dstreg, shortoff [ srcreg ]
0256 a0 38 30 			ld	dstreg, srcreg
0259 b1 98 30 			ldb	dstreg, #immed
025c b2 38 30 			ldb	dstreg, [ srcreg ]
025f b2 39 30 			ldb	dstreg, [ srcreg ] +
0262 b3 01 34 12 30 		ldb	dstreg, extern
0267 b3 39 54 76 30 		ldb	dstreg, longoff [ srcreg ]
026c b3 38 33 30 		ldb	dstreg, shortoff [ srcreg ]
0270 b0 38 30 			ldb	dstreg, srcreg
0273 bd 98 30 			ldbse	dstreg, #immed
0276 be 38 30 			ldbse	dstreg, [ srcreg ]
0279 be 39 30 			ldbse	dstreg, [ srcreg ] +
027c bf 01 34 12 30 		ldbse	dstreg, extern
0281 bf 39 54 76 30 		ldbse	dstreg, longoff [ srcreg ]
0286 bf 38 33 30 		ldbse	dstreg, shortoff [ srcreg ]
028a bc 38 30 			ldbse	dstreg, srcreg
028d ad 98 30 			ldbze	dstreg, #immed
0290 ae 38 30 			ldbze	dstreg, [ srcreg ]
0293 ae 39 30 			ldbze	dstreg, [ srcreg ] +
0296 af 01 34 12 30 		ldbze	dstreg, extern
029b af 39 54 76 30 		ldbze	dstreg, longoff [ srcreg ]
02a0 af 38 33 30 		ldbze	dstreg, shortoff [ srcreg ]
02a4 ac 38 30 			ldbze	dstreg, srcreg
02a7 e7 73 ff 			ljmp	jmpdst
02aa fe 6d 98 00 30 		mul	dstreg, #immed
02af fe 6e 38 30 		mul	dstreg, [ srcreg ]
02b3 fe 6e 39 30 		mul	dstreg, [ srcreg ] +
02b7 fe 6f 01 34 12 30 		mul	dstreg, extern
02bd fe 6f 39 54 76 30 		mul	dstreg, longoff [ srcreg ]
02c3 fe 6f 38 33 30 		mul	dstreg, shortoff [ srcreg ]
02c8 fe 6c 38 30 		mul	dstreg, srcreg
02cc fe 4d 98 00 44 30 		mul	dstreg, srcreg2, #immed
02d2 fe 4e 38 44 30 		mul	dstreg, srcreg2, [ srcreg ]
02d7 fe 4e 39 44 30 		mul	dstreg, srcreg2, [ srcreg ] +
02dc fe 4f 01 34 12 44 		mul	dstreg, srcreg2, extern
02e2 30 
02e3 fe 4f 39 54 76 44 		mul	dstreg, srcreg2, longoff [ srcreg ]
02e9 30 
02ea fe 4f 38 33 44 30 		mul	dstreg, srcreg2, shortoff [ srcreg ]
02f0 fe 4c 38 44 30 		mul	dstreg, srcreg2, srcreg
02f5 fe 7d 98 30 		mulb	dstreg, #immed
02f9 fe 7e 38 30 		mulb	dstreg, [ srcreg ]
02fd fe 7e 39 30 		mulb	dstreg, [ srcreg ] +
0301 fe 7f 01 34 12 30 		mulb	dstreg, extern
0307 fe 7f 39 54 76 30 		mulb	dstreg, longoff [ srcreg ]
030d fe 7f 38 33 30 		mulb	dstreg, shortoff [ srcreg ]
0312 fe 7c 38 30 		mulb	dstreg, srcreg
0316 fe 5d 98 44 30 		mulb	dstreg, srcreg2, #immed
031b fe 5e 38 44 30 		mulb	dstreg, srcreg2, [ srcreg ]
0320 fe 5e 39 44 30 		mulb	dstreg, srcreg2, [ srcreg ] +
0325 fe 5f 01 34 12 44 		mulb	dstreg, srcreg2, extern
032b 30 
032c fe 5f 39 54 76 44 		mulb	dstreg, srcreg2, longoff [ srcreg ]
0332 30 
0333 fe 5f 38 33 44 30 		mulb	dstreg, srcreg2, shortoff [ srcreg ]
0339 fe 5c 38 44 30 		mulb	dstreg, srcreg2, srcreg
033e 6d 98 00 30 		mulu	dstreg, #immed
0342 6e 38 30 			mulu	dstreg, [ srcreg ]
0345 6e 39 30 			mulu	dstreg, [ srcreg ] +
0348 6f 01 34 12 30 		mulu	dstreg, extern
034d 6f 39 54 76 30 		mulu	dstreg, longoff [ srcreg ]
0352 6f 38 33 30 		mulu	dstreg, shortoff [ srcreg ]
0356 6c 38 30 			mulu	dstreg, srcreg
0359 4d 98 00 44 30 		mulu	dstreg, srcreg2, #immed
035e 4e 38 44 30 		mulu	dstreg, srcreg2, [ srcreg ]
0362 4e 39 44 30 		mulu	dstreg, srcreg2, [ srcreg ] +
0366 4f 01 34 12 44 30 		mulu	dstreg, srcreg2, extern
036c 4f 39 54 76 44 30 		mulu	dstreg, srcreg2, longoff [ srcreg ]
0372 4f 38 33 44 30 		mulu	dstreg, srcreg2, shortoff [ srcreg ]
0377 4c 38 44 30 		mulu	dstreg, srcreg2, srcreg
037b 7d 98 30 			mulub	dstreg, #immed
037e 7e 38 30 			mulub	dstreg, [ srcreg ]
0381 7e 39 30 			mulub	dstreg, [ srcreg ] +
0384 7f 01 34 12 30 		mulub	dstreg, extern
0389 7f 39 54 76 30 		mulub	dstreg, longoff [ srcreg ]
038e 7f 38 33 30 		mulub	dstreg, shortoff [ srcreg ]
0392 7c 38 30 			mulub	dstreg, srcreg
0395 5d 98 44 30 		mulub	dstreg, srcreg2, #immed
0399 5e 38 44 30 		mulub	dstreg, srcreg2, [ srcreg ]
039d 5e 39 44 30 		mulub	dstreg, srcreg2, [ srcreg ] +
03a1 5f 01 34 12 44 30 		mulub	dstreg, srcreg2, extern
03a7 5f 39 54 76 44 30 		mulub	dstreg, srcreg2, longoff [ srcreg ]
03ad 5f 38 33 44 30 		mulub	dstreg, srcreg2, shortoff [ srcreg ]
03b2 5c 38 44 30 		mulub	dstreg, srcreg2, srcreg
03b6 03 30 			neg	dstreg
03b8 13 30 			negb	dstreg
03ba fd 			nop
03bb 0f 38 30 			norml	dstreg, srcreg
03be 02 30 			not	dstreg
03c0 12 30 			notb	dstreg
03c2 81 98 00 30 		or	dstreg, #immed
03c6 82 38 30 			or	dstreg, [ srcreg ]
03c9 82 39 30 			or	dstreg, [ srcreg ] +
03cc 83 01 34 12 30 		or	dstreg, extern
03d1 83 39 54 76 30 		or	dstreg, longoff [ srcreg ]
03d6 83 38 33 30 		or	dstreg, shortoff [ srcreg ]
03da 80 38 30 			or	dstreg, srcreg
03dd 91 98 30 			orb	dstreg, #immed
03e0 92 38 30 			orb	dstreg, [ srcreg ]
03e3 92 39 30 			orb	dstreg, [ srcreg ] +
03e6 93 01 34 12 30 		orb	dstreg, extern
03eb 93 39 54 76 30 		orb	dstreg, longoff [ srcreg ]
03f0 93 38 33 30 		orb	dstreg, shortoff [ srcreg ]
03f4 90 38 30 			orb	dstreg, srcreg
03f7 ce 38 			pop	[ srcreg ]
03f9 ce 39 			pop	[ srcreg ] +
03fb cf 01 34 12 		pop	extern
03ff cf 39 54 76 		pop	longoff [ srcreg ]
0403 cf 38 33 			pop	shortoff [ srcreg ]
0406 cc 38 			pop	srcreg
0408 f3 			popf
0409 c9 98 00 			push	#immed
040c ca 38 			push	[ srcreg ]
040e ca 39 			push	[ srcreg ] +
0410 cb 01 34 12 		push	extern
0414 cb 39 54 76 		push	longoff [ srcreg ]
0418 cb 38 33 			push	shortoff [ srcreg ]
041b c8 38 			push	srcreg
041d f2 			pushf
041e f0 			ret
041f ff 			rst
0420 2d fb 			scall	jmpdst
0422 f9 			setc
0423 09 0a 30 			shl	dstreg, # shiftcount
0426 09 38 30 			shl	dstreg, srcreg
0429 19 0a 30 			shlb	dstreg, # shiftcount
042c 19 38 30 			shlb	dstreg, srcreg
042f 0d 0a 30 			shll	dstreg, # shiftcount
0432 0d 38 30 			shll	dstreg, srcreg
0435 08 0a 30 			shr	dstreg, # shiftcount
0438 08 38 30 			shr	dstreg, srcreg
043b 0a 0a 30 			shra	dstreg, # shiftcount
043e 0a 38 30 			shra	dstreg, srcreg
0441 1a 0a 30 			shrab	dstreg, # shiftcount
0444 1a 38 30 			shrab	dstreg, srcreg
0447 0e 0a 30 			shral	dstreg, # shiftcount
044a 0e 38 30 			shral	dstreg, srcreg
044d 18 0a 30 			shrb	dstreg, # shiftcount
0450 18 38 30 			shrb	dstreg, srcreg
0453 0c 0a 30 			shrl	dstreg, # shiftcount
0456 0c 38 30 			shrl	dstreg, srcreg
0459 25 c2 			sjmp	jmpdst
045b 00 30 			skip	dstreg
045d c2 38 30 			st	dstreg, [ srcreg ]
0460 c2 39 30 			st	dstreg, [ srcreg ] +
0463 c3 01 34 12 30 		st	dstreg, extern
0468 c3 39 54 76 30 		st	dstreg, longoff [ srcreg ]
046d c3 38 33 30 		st	dstreg, shortoff [ srcreg ]
0471 c0 38 30 			st	dstreg, srcreg
0474 c6 38 30 			stb	dstreg, [ srcreg ]
0477 c6 39 30 			stb	dstreg, [ srcreg ] +
047a c7 01 34 12 30 		stb	dstreg, extern
047f c7 39 54 76 30 		stb	dstreg, longoff [ srcreg ]
0484 c7 38 33 30 		stb	dstreg, shortoff [ srcreg ]
0488 c4 38 30 			stb	dstreg, srcreg
048b 69 98 00 30 		sub	dstreg, #immed
048f 6a 38 30 			sub	dstreg, [ srcreg ]
0492 6a 39 30 			sub	dstreg, [ srcreg ] +
0495 6b 01 34 12 30 		sub	dstreg, extern
049a 6b 39 54 76 30 		sub	dstreg, longoff [ srcreg ]
049f 6b 38 33 30 		sub	dstreg, shortoff [ srcreg ]
04a3 68 38 30 			sub	dstreg, srcreg
04a6 49 98 00 44 30 		sub	dstreg, srcreg2, #immed
04ab 4a 38 44 30 		sub	dstreg, srcreg2, [ srcreg ]
04af 4a 39 44 30 		sub	dstreg, srcreg2, [ srcreg ] +
04b3 4b 01 34 12 44 30 		sub	dstreg, srcreg2, extern
04b9 4b 39 54 76 44 30 		sub	dstreg, srcreg2, longoff [ srcreg ]
04bf 4b 38 33 44 30 		sub	dstreg, srcreg2, shortoff [ srcreg ]
04c4 48 38 44 30 		sub	dstreg, srcreg2, srcreg
04c8 79 98 30 			subb	dstreg, #immed
04cb 7a 38 30 			subb	dstreg, [ srcreg ]
04ce 7a 39 30 			subb	dstreg, [ srcreg ] +
04d1 7b 01 34 12 30 		subb	dstreg, extern
04d6 7b 39 54 76 30 		subb	dstreg, longoff [ srcreg ]
04db 7b 38 33 30 		subb	dstreg, shortoff [ srcreg ]
04df 78 38 30 			subb	dstreg, srcreg
04e2 59 98 44 30 		subb	dstreg, srcreg2, #immed
04e6 5a 38 44 30 		subb	dstreg, srcreg2, [ srcreg ]
04ea 5a 39 44 30 		subb	dstreg, srcreg2, [ srcreg ] +
04ee 5b 01 34 12 44 30 		subb	dstreg, srcreg2, extern
04f4 5b 39 54 76 44 30 		subb	dstreg, srcreg2, longoff [ srcreg ]
04fa 5b 38 33 44 30 		subb	dstreg, srcreg2, shortoff [ srcreg ]
04ff 58 38 44 30 		subb	dstreg, srcreg2, srcreg
0503 a9 98 00 30 		subc	dstreg, #immed
0507 aa 38 30 			subc	dstreg, [ srcreg ]
050a aa 39 30 			subc	dstreg, [ srcreg ] +
050d ab 01 34 12 30 		subc	dstreg, extern
0512 ab 39 54 76 30 		subc	dstreg, longoff [ srcreg ]
0517 ab 38 33 30 		subc	dstreg, shortoff [ srcreg ]
051b a8 38 30 			subc	dstreg, srcreg
051e b9 98 30 			subcb	dstreg, #immed
0521 ba 38 30 			subcb	dstreg, [ srcreg ]
0524 ba 39 30 			subcb	dstreg, [ srcreg ] +
0527 bb 01 34 12 30 		subcb	dstreg, extern
052c bb 39 54 76 30 		subcb	dstreg, longoff [ srcreg ]
0531 bb 38 33 30 		subcb	dstreg, shortoff [ srcreg ]
0535 b8 38 30 			subcb	dstreg, srcreg
0538 85 98 00 30 		xor	dstreg, #immed
053c 86 38 30 			xor	dstreg, [ srcreg ]
053f 86 39 30 			xor	dstreg, [ srcreg ] +
0542 87 01 34 12 30 		xor	dstreg, extern
0547 87 39 54 76 30 		xor	dstreg, longoff [ srcreg ]
054c 87 38 33 30 		xor	dstreg, shortoff [ srcreg ]
0550 84 38 30 			xor	dstreg, srcreg
0553 95 98 30 			xorb	dstreg, #immed
0556 96 38 30 			xorb	dstreg, [ srcreg ]
0559 96 39 30 			xorb	dstreg, [ srcreg ] +
055c 97 01 34 12 30 		xorb	dstreg, extern
0561 97 39 54 76 30 		xorb	dstreg, longoff [ srcreg ]
0566 97 38 33 30 		xorb	dstreg, shortoff [ srcreg ]
056a 94 38 30 			xorb	dstreg, srcreg
