00000088 bit              0000008d dirbit           00000044 direct           
00000055 immed            00000033 srcdirect        00000098 addr             
00000124 saddr            
 0x88                   bit	equ 	$88
 0x8d                   dirbit	equ	$8d
 0x44                   direct	equ	$44
 0x55                   immed	equ	$55
 0x33                   srcdirect	equ	$33
0000 11 98 			acall	addr
0002 24 55 			add	a, #immed
0004 26 			add	a, @ r0
0005 27 			add	a, @ r1
0006 25 44 			add	a, direct
0008 28 			add	a, r0
0009 29 			add	a, r1
000a 2a 			add	a, r2
000b 2b 			add	a, r3
000c 2c 			add	a, r4
000d 2d 			add	a, r5
000e 2e 			add	a, r6
000f 2f 			add	a, r7
0010 34 55 			addc	a, #immed
0012 36 			addc	a, @ r0
0013 37 			addc	a, @ r1
0014 35 44 			addc	a, direct
0016 38 			addc	a, r0
0017 39 			addc	a, r1
0018 3a 			addc	a, r2
0019 3b 			addc	a, r3
001a 3c 			addc	a, r4
001b 3d 			addc	a, r5
001c 3e 			addc	a, r6
001d 3f 			addc	a, r7
001e 01 98 			ajmp	addr
0020 54 55 			anl	a, #immed
0022 56 			anl	a, @ r0
0023 57 			anl	a, @ r1
0024 55 44 			anl	a, direct
0026 58 			anl	a, r0
0027 59 			anl	a, r1
0028 5a 			anl	a, r2
0029 5b 			anl	a, r3
002a 5c 			anl	a, r4
002b 5d 			anl	a, r5
002c 5e 			anl	a, r6
002d 5f 			anl	a, r7
002e b0 8d 			anl	c, /bit.5
0030 b0 8d 			anl	c, /dirbit
0032 82 8d 			anl	c, bit.5
0034 82 8d 			anl	c, dirbit
0036 53 44 55 			anl	direct, # immed
0039 52 44 			anl	direct, a
003b b6 55 5a 			cjne	@ r0, # immed, addr
003e b7 55 57 			cjne	@ r1, # immed, addr
0041 b4 55 54 			cjne	a, #immed, addr
0044 b5 44 51 			cjne	a, direct, addr
0047 b8 55 4e 			cjne	r0, # immed, addr
004a b9 55 4b 			cjne	r1, # immed, addr
004d ba 55 48 			cjne	r2, # immed, addr
0050 bb 55 45 			cjne	r3, # immed, addr
0053 bc 55 42 			cjne	r4, # immed, addr
0056 bd 55 3f 			cjne	r5, # immed, addr
0059 be 55 3c 			cjne	r6, # immed, addr
005c bf 55 39 			cjne	r7, # immed, addr
005f e4 			clr	a
0060 c2 8d 			clr	bit.5
0062 c3 			clr	c
0063 c2 8d 			clr	dirbit
0065 f4 			cpl	a
0066 b2 8d 			cpl	bit.5
0068 b3 			cpl	c
0069 b2 8d 			cpl	dirbit
006b d4 			da	a
006c 16 			dec	@ r0
006d 17 			dec	@ r1
006e 14 			dec	a
006f 15 44 			dec	direct
0071 18 			dec	r0
0072 19 			dec	r1
0073 1a 			dec	r2
0074 1b 			dec	r3
0075 1c 			dec	r4
0076 1d 			dec	r5
0077 1e 			dec	r6
0078 1f 			dec	r7
0079 84 			div	ab
007a d5 44 1b 			djnz	direct, addr
007d d8 19 			djnz	r0, addr
007f d9 17 			djnz	r1, addr
0081 da 15 			djnz	r2, addr
0083 db 13 			djnz	r3, addr
0085 dc 11 			djnz	r4, addr
0087 dd 0f 			djnz	r5, addr
0089 de 0d 			djnz	r6, addr
008b df 0b 			djnz	r7, addr
008d 06 			inc	@ r0
008e 07 			inc	@ r1
008f 04 			inc	a
0090 05 44 			inc	direct
0092 a3 			inc	dptr
0093 08 			inc	r0
0094 09 			inc	r1
0095 0a 			inc	r2
0096 0b 			inc	r3
0097 0c 			inc	r4
0098 0d 		addr	inc	r5
0099 0e 			inc	r6
009a 0f 			inc	r7
009b 20 8d fa 			jb	bit.5, addr
009e 20 8d f7 			jb	dirbit, addr
00a1 10 8d f4 			jbc	bit.5, addr
00a4 10 8d f1 			jbc	dirbit, addr
00a7 40 ef 			jc	addr
00a9 73 			jmp	@a+dptr
00aa 30 8d eb 			jnb	bit.5, addr
00ad 30 8d e8 			jnb	dirbit, addr
00b0 50 e6 			jnc	addr
00b2 70 e4 			jnz	addr
00b4 60 e2 			jz	addr
00b6 12 00 98 			lcall	addr
00b9 02 00 98 			ljmp	addr
00bc 76 55 			mov	@ r0, # immed
00be f6 			mov	@ r0, a
00bf a6 44 			mov	@ r0, direct
00c1 77 55 			mov	@ r1, # immed
00c3 f7 			mov	@ r1, a
00c4 a7 44 			mov	@ r1, direct
00c6 74 55 			mov	a, #immed
00c8 e6 			mov	a, @ r0
00c9 e7 			mov	a, @ r1
00ca e5 44 			mov	a, direct
00cc e8 			mov	a, r0
00cd e9 			mov	a, r1
00ce ea 			mov	a, r2
00cf eb 			mov	a, r3
00d0 ec 			mov	a, r4
00d1 ed 			mov	a, r5
00d2 ee 			mov	a, r6
00d3 ef 			mov	a, r7
00d4 92 8d 			mov	bit.5, c
00d6 a2 8d 			mov	c, bit.5
00d8 a2 8d 			mov	c, dirbit
00da 92 8d 			mov	dirbit, c
00dc 75 44 55 			mov	direct, # immed
00df 86 44 			mov	direct, @ r0
00e1 87 44 			mov	direct, @ r1
00e3 f5 44 			mov	direct, a
00e5 88 44 			mov	direct, r0
00e7 89 44 			mov	direct, r1
00e9 8a 44 			mov	direct, r2
00eb 8b 44 			mov	direct, r3
00ed 8c 44 			mov	direct, r4
00ef 8d 44 			mov	direct, r5
00f1 8e 44 			mov	direct, r6
00f3 8f 44 			mov	direct, r7
00f5 85 33 44 			mov	direct, srcdirect
00f8 90 00 55 			mov	dptr, #immed
00fb 78 55 			mov	r0, # immed
00fd f8 			mov	r0, a
00fe a8 44 			mov	r0, direct
0100 79 55 			mov	r1, # immed
0102 f9 			mov	r1, a
0103 a9 44 			mov	r1, direct
0105 7a 55 			mov	r2, # immed
0107 fa 			mov	r2, a
0108 aa 44 			mov	r2, direct
010a 7b 55 			mov	r3, # immed
010c fb 			mov	r3, a
010d ab 44 			mov	r3, direct
010f 7c 55 			mov	r4, # immed
0111 fc 			mov	r4, a
0112 ac 44 			mov	r4, direct
0114 7d 55 			mov	r5, # immed
0116 fd 			mov	r5, a
0117 ad 44 			mov	r5, direct
0119 7e 55 			mov	r6, # immed
011b fe 			mov	r6, a
011c ae 44 			mov	r6, direct
011e 7f 55 			mov	r7, # immed
0120 ff 			mov	r7, a
0121 af 44 			mov	r7, direct
0123 93 			movc	a, @a+dptr
0124 83 		saddr	movc	a, @a+pc
0125 f0 			movx	@dptr, a
0126 f2 			movx	@r0, a
0127 f3 			movx	@r1, a
0128 e2 			movx	a, @ r0
0129 e3 			movx	a, @ r1
012a e0 			movx	a, @dptr
012b a4 			mul	ab
012c 00 			nop
012d 44 55 			orl	a, #immed
012f 46 			orl	a, @ r0
0130 47 			orl	a, @ r1
0131 45 44 			orl	a, direct
0133 48 			orl	a, r0
0134 49 			orl	a, r1
0135 4a 			orl	a, r2
0136 4b 			orl	a, r3
0137 4c 			orl	a, r4
0138 4d 			orl	a, r5
0139 4e 			orl	a, r6
013a 4f 			orl	a, r7
013b a0 8d 			orl	c, /bit.5
013d a0 8d 			orl	c, /dirbit
013f 72 8d 			orl	c, bit.5
0141 72 8d 			orl	c, dirbit
0143 43 44 55 			orl	direct, # immed
0146 42 44 			orl	direct, a
0148 d0 44 			pop	direct
014a c0 44 			push	direct
014c 22 			ret
014d 32 			reti
014e 23 			rl	a
014f 33 			rlc	a
0150 03 			rr	a
0151 13 			rrc	a
0152 d2 8d 			setb	bit.5
0154 d3 			setb	c
0155 d2 8d 			setb	dirbit
0157 80 cb 			sjmp	saddr
0159 94 55 			subb	a, #immed
015b 96 			subb	a, @ r0
015c 97 			subb	a, @ r1
015d 95 44 			subb	a, direct
015f 98 			subb	a, r0
0160 99 			subb	a, r1
0161 9a 			subb	a, r2
0162 9b 			subb	a, r3
0163 9c 			subb	a, r4
0164 9d 			subb	a, r5
0165 9e 			subb	a, r6
0166 9f 			subb	a, r7
0167 c4 			swap	a
0168 c6 			xch	a, @ r0
0169 c7 			xch	a, @ r1
016a c5 44 			xch	a, direct
016c c8 			xch	a, r0
016d c9 			xch	a, r1
016e ca 			xch	a, r2
016f cb 			xch	a, r3
0170 cc 			xch	a, r4
0171 cd 			xch	a, r5
0172 ce 			xch	a, r6
0173 cf 			xch	a, r7
0174 d6 			xchd	a, @ r0
0175 d7 			xchd	a, @ r1
0176 64 55 			xrl	a, #immed
0178 66 			xrl	a, @ r0
0179 67 			xrl	a, @ r1
017a 65 44 			xrl	a, direct
017c 68 			xrl	a, r0
017d 69 			xrl	a, r1
017e 6a 			xrl	a, r2
017f 6b 			xrl	a, r3
0180 6c 			xrl	a, r4
0181 6d 			xrl	a, r5
0182 6e 			xrl	a, r6
0183 6f 			xrl	a, r7
0184 63 44 55 			xrl	direct, # immed
0187 62 44 			xrl	direct, a
