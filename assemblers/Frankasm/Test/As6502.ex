00000055 immed            00000044 direct           00001234 extend           
00000065 jmpdst           
				cpu	"r65c"
 0x55                   immed	equ	$55
 0x44                   direct	equ	$44
 0x1234                 extend	equ	$1234
0000 69 55 			adc	# immed
0002 61 44 			adc	( direct, x )
0004 72 44 			adc	(direct)
0006 71 44 			adc	(direct),y
0008 65 44 			adc	direct
000a 75 44 			adc	direct,x
000c 6d 34 12 			adc	extend
000f 7d 34 12 			adc	extend,x
0012 79 34 12 			adc	extend,y
0015 29 55 			and	# immed
0017 21 44 			and	( direct, x )
0019 32 44 			and	(direct)
001b 31 44 			and	(direct),y
001d 25 44 			and	direct
001f 35 44 			and	direct,x
0021 2d 34 12 			and	extend
0024 3d 34 12 			and	extend,x
0027 39 34 12 			and	extend,y
002a 0a 			asl	a
002b 06 44 			asl	direct
002d 16 44 			asl	direct,x
002f 0e 34 12 			asl	extend
0032 1e 34 12 			asl	extend,x
0035 0f 44 2d 			bbr0	direct, jmpdst
0038 1f 44 2a 			bbr1	direct, jmpdst
003b 2f 44 27 			bbr2	direct, jmpdst
003e 3f 44 24 			bbr3	direct, jmpdst
0041 4f 44 21 			bbr4	direct, jmpdst
0044 5f 44 1e 			bbr5	direct, jmpdst
0047 6f 44 1b 			bbr6	direct, jmpdst
004a 7f 44 18 			bbr7	direct, jmpdst
004d 8f 44 15 			bbs0	direct, jmpdst
0050 9f 44 12 			bbs1	direct, jmpdst
0053 af 44 0f 			bbs2	direct, jmpdst
0056 bf 44 0c 			bbs3	direct, jmpdst
0059 cf 44 09 			bbs4	direct, jmpdst
005c df 44 06 			bbs5	direct, jmpdst
005f ef 44 03 			bbs6	direct, jmpdst
0062 ff 44 00 			bbs7	direct, jmpdst
0065 90 fe 		jmpdst	bcc	jmpdst
0067 b0 fc 			bcs	jmpdst
0069 f0 fa 			beq	jmpdst
006b b0 f8 			bge	jmpdst
006d 89 55 			bit	# immed
006f 24 44 			bit	direct
0071 34 44 			bit	direct ,x
0073 2c 34 12 			bit	extend
0076 3c 34 12 			bit	extend, x
0079 90 ea 			blt	jmpdst
007b 30 e8 			bmi	jmpdst
007d d0 e6 			bne	jmpdst
007f 10 e4 			bpl	jmpdst
0081 80 e2 			bra	jmpdst
0083 00 			brk
0084 00 63 			brk	#99
0086 50 dd 			bvc	jmpdst
0088 70 db 			bvs	jmpdst
008a 18 			clc
008b d8 			cld
008c 58 			cli
008d b8 			clv
008e c9 55 			cmp	# immed
0090 c1 44 			cmp	( direct, x )
0092 d2 44 			cmp	(direct)
0094 d1 44 			cmp	(direct),y
0096 c5 44 			cmp	direct
0098 d5 44 			cmp	direct,x
009a cd 34 12 			cmp	extend
009d dd 34 12 			cmp	extend,x
00a0 d9 34 12 			cmp	extend,y
00a3 e0 55 			cpx	# immed
00a5 e4 44 			cpx	direct
00a7 ec 34 12 			cpx	extend
00aa c0 55 			cpy	# immed
00ac c4 44 			cpy	direct
00ae cc 34 12 			cpy	extend
00b1 3a 			dec	a
00b2 c6 44 			dec	direct
00b4 d6 44 			dec	direct,x
00b6 ce 34 12 			dec	extend
00b9 de 34 12 			dec	extend,x
00bc ca 			dex
00bd 88 			dey
00be 49 55 			eor	# immed
00c0 41 44 			eor	( direct, x )
00c2 52 44 			eor	(direct)
00c4 51 44 			eor	(direct),y
00c6 45 44 			eor	direct
00c8 55 44 			eor	direct,x
00ca 4d 34 12 			eor	extend
00cd 5d 34 12 			eor	extend,x
00d0 59 34 12 			eor	extend,y
00d3 1a 			inc	a
00d4 e6 44 			inc	direct
00d6 f6 44 			inc	direct,x
00d8 ee 34 12 			inc	extend
00db fe 34 12 			inc	extend,x
00de e8 			inx
00df c8 			iny
00e0 6c 65 00 			jmp	(jmpdst)
00e3 7c 65 00 			jmp	(jmpdst,x)
00e6 4c 65 00 			jmp	jmpdst
00e9 20 65 00 			jsr	jmpdst
00ec a9 55 			lda	# immed
00ee a1 44 			lda	( direct, x )
00f0 b2 44 			lda	(direct)
00f2 b1 44 			lda	(direct),y
00f4 a5 44 			lda	direct
00f6 b5 44 			lda	direct,x
00f8 ad 34 12 			lda	extend
00fb bd 34 12 			lda	extend,x
00fe b9 34 12 			lda	extend,y
0101 a2 55 			ldx	# immed
0103 a6 44 			ldx	direct
0105 b6 44 			ldx	direct ,y
0107 ae 34 12 			ldx	extend
010a be 34 12 			ldx	extend, y
010d a0 55 			ldy	# immed
010f a4 44 			ldy	direct
0111 b4 44 			ldy	direct ,x
0113 ac 34 12 			ldy	extend
0116 bc 34 12 			ldy	extend, x
0119 4a 			lsr	a
011a 46 44 			lsr	direct
011c 56 44 			lsr	direct,x
011e 4e 34 12 			lsr	extend
0121 5e 34 12 			lsr	extend,x
			;;	mul
0124 ea 			nop
0125 09 55 			ora	# immed
0127 01 44 			ora	( direct, x )
0129 12 44 			ora	(direct)
012b 11 44 			ora	(direct),y
012d 05 44 			ora	direct
012f 15 44 			ora	direct,x
0131 0d 34 12 			ora	extend
0134 1d 34 12 			ora	extend,x
0137 19 34 12 			ora	extend,y
013a 48 			pha
013b 08 			php
013c da 			phx
013d 5a 			phy
013e 68 			pla
013f 28 			plp
0140 fa 			plx
0141 7a 			ply
0142 07 44 			rmb0	direct
0144 17 44 			rmb1	direct
0146 27 44 			rmb2	direct
0148 37 44 			rmb3	direct
014a 47 44 			rmb4	direct
014c 57 44 			rmb5	direct
014e 67 44 			rmb6	direct
0150 77 44 			rmb7	direct
0152 2a 			rol	a
0153 26 44 			rol	direct
0155 36 44 			rol	direct,x
0157 2e 34 12 			rol	extend
015a 3e 34 12 			rol	extend,x
015d 6a 			ror	a
015e 66 44 			ror	direct
0160 76 44 			ror	direct,x
0162 6e 34 12 			ror	extend
0165 7e 34 12 			ror	extend,x
0168 40 			rti
0169 60 			rts
016a e9 55 			sbc	# immed
016c e1 44 			sbc	( direct, x )
016e f2 44 			sbc	(direct)
0170 f1 44 			sbc	(direct),y
0172 e5 44 			sbc	direct
0174 f5 44 			sbc	direct,x
0176 ed 34 12 			sbc	extend
0179 fd 34 12 			sbc	extend,x
017c f9 34 12 			sbc	extend,y
017f 38 			sec
0180 f8 			sed
0181 78 			sei
0182 87 44 			smb0	direct
0184 97 44 			smb1	direct
0186 a7 44 			smb2	direct
0188 b7 44 			smb3	direct
018a c7 44 			smb4	direct
018c d7 44 			smb5	direct
018e e7 44 			smb6	direct
0190 f7 44 			smb7	direct
0192 81 44 			sta	( direct, x )
0194 92 44 			sta	(direct)
0196 91 44 			sta	(direct),y
0198 85 44 			sta	direct
019a 95 44 			sta	direct,x
019c 8d 34 12 			sta	extend
019f 9d 34 12 			sta	extend,x
01a2 99 34 12 			sta	extend,y
01a5 86 44 			stx	direct
01a7 96 44 			stx	direct,y
01a9 8e 34 12 			stx	extend
01ac 84 44 			sty	direct
01ae 94 44 			sty	direct,x
01b0 8c 34 12 			sty	extend
01b3 64 44 			stz	direct
01b5 74 44 			stz	direct ,x
01b7 9c 34 12 			stz	extend
01ba 9e 34 12 			stz	extend, x
01bd aa 			tax
01be a8 			tay
01bf 14 44 			trb	direct
01c1 1c 34 12 			trb	extend
01c4 04 44 			tsb	direct
01c6 0c 34 12 			tsb	extend
01c9 ba 			tsx
01ca 8a 			txa
01cb 9a 			txs
01cc 98 			tya
