00000055 immed            0000006b dest             
				cpu	"80c50"

 0x55                   immed	equ	$55

0000 03 55 			add	a, # immed
0002 60 			add	a, @ r0
0003 61 			add	a, @ r1
0004 68 			add	a, r0
0005 69 			add	a, r1
0006 6a 			add	a, r2
0007 6b 			add	a, r3
0008 6c 			add	a, r4
0009 6d 			add	a, r5
000a 6e 			add	a, r6
000b 6f 			add	a, r7
000c 13 55 			addc	a, # immed
000e 70 			addc	a, @ r0
000f 71 			addc	a, @ r1
0010 78 			addc	a, r0
0011 79 			addc	a, r1
0012 7a 			addc	a, r2
0013 7b 			addc	a, r3
0014 7c 			addc	a, r4
0015 7d 			addc	a, r5
0016 7e 			addc	a, r6
0017 7f 			addc	a, r7
0018 53 55 			anl	a, # immed
001a 50 			anl	a, @ r0
001b 51 			anl	a, @ r1
001c 58 			anl	a, r0
001d 59 			anl	a, r1
001e 5a 			anl	a, r2
001f 5b 			anl	a, r3
0020 5c 			anl	a, r4
0021 5d 			anl	a, r5
0022 5e 			anl	a, r6
0023 5f 			anl	a, r7
0024 98 55 			anl	bus, # immed
0026 99 55 			anl	p1, # immed
0028 9a 55 			anl	p2, # immed
002a 9c 			anld	p4, a
002b 9d 			anld	p5, a
002c 9e 			anld	p6, a
002d 9f 			anld	p7, a
002e 14 23 			call	$023
0030 34 23 			call	$123
0032 54 23 			call	$223
0034 74 23 			call	$323
0036 94 23 			call	$423
0038 b4 23 			call	$523
003a d4 23 			call	$623
003c f4 23 			call	$723
003e 27 			clr	a
003f 97 			clr	c
0040 85 			clr	f0
0041 a5 			clr	f1
0042 37 			cpl	a
0043 a7 			cpl	c
0044 95 			cpl	f0
0045 b5 			cpl	f1
0046 57 			da	a
0047 07 			dec	a
0048 c8 			dec	r0
0049 c9 			dec	r1
004a ca 			dec	r2
004b cb 			dec	r3
004c cc 			dec	r4
004d cd 			dec	r5
004e ce 			dec	r6
004f cf 			dec	r7
0050 15 			dis	i
0051 35 			dis	tcnti
0052 e8 6b 			djnz	r0 , dest
0054 e9 6b 			djnz	r1 , dest
0056 ea 6b 			djnz	r2 , dest
0058 eb 6b 			djnz	r3 , dest
005a ec 6b 			djnz	r4 , dest
005c ed 6b 			djnz	r5 , dest
005e ee 6b 			djnz	r6 , dest
0060 ef 6b 			djnz	r7 , dest
0062 05 			en	i
0063 25 			en	tcnti
0064 75 			ent0	clk
0065 01 			halt
0066 01 			idl
0067 09 			in	a, p1
0068 0a 			in	a, p2
0069 10 			inc	@ r0
006a 11 			inc	@ r1
006b 17 		dest	inc	a
006c 18 			inc	r0
006d 19 			inc	r1
006e 1a 			inc	r2
006f 1b 			inc	r3
0070 1c 			inc	r4
0071 1d 			inc	r5
0072 1e 			inc	r6
0073 1f 			inc	r7
0074 08 			ins	a, bus
0075 12 6b 			jb0	dest
0077 32 6b 			jb1	dest
0079 52 6b 			jb2	dest
007b 72 6b 			jb3	dest
007d 92 6b 			jb4	dest
007f b2 6b 			jb5	dest
0081 d2 6b 			jb6	dest
0083 f2 6b 			jb7	dest
0085 f6 6b 			jc	dest
0087 b6 6b 			jf0	dest
0089 76 6b 			jf1	dest
008b 04 23 			jmp	$023
008d 24 23 			jmp	$123
008f 44 23 			jmp	$223
0091 64 23 			jmp	$323
0093 84 23 			jmp	$423
0095 a4 23 			jmp	$523
0097 c4 23 			jmp	$623
0099 e4 23 			jmp	$723
009b b3 			jmpp	@a
009c e6 6b 			jnc	dest
009e 86 6b 			jni	dest
00a0 26 6b 			jnt0	dest
00a2 46 6b 			jnt1	dest
00a4 96 6b 			jnz	dest
00a6 36 6b 			jt0	dest
00a8 56 6b 			jt1	dest
00aa 16 6b 			jtf	dest
00ac c6 6b 			jz	dest
00ae b0 55 			mov	@ r0, # immed
00b0 a0 			mov	@ r0, a
00b1 b1 55 			mov	@ r1, # immed
00b3 a1 			mov	@ r1, a
00b4 23 55 			mov	a, # immed
00b6 f0 			mov	a, @ r0
00b7 f1 			mov	a, @ r1
00b8 c7 			mov	a, psw
00b9 f8 			mov	a, r0
00ba f9 			mov	a, r1
00bb fa 			mov	a, r2
00bc fb 			mov	a, r3
00bd fc 			mov	a, r4
00be fd 			mov	a, r5
00bf fe 			mov	a, r6
00c0 ff 			mov	a, r7
00c1 42 			mov	a, t
00c2 d7 			mov	psw, a
00c3 b8 55 			mov	r0, # immed
00c5 a8 			mov	r0, a
00c6 b9 55 			mov	r1, # immed
00c8 a9 			mov	r1, a
00c9 ba 55 			mov	r2, # immed
00cb aa 			mov	r2, a
00cc bb 55 			mov	r3, # immed
00ce ab 			mov	r3, a
00cf bc 55 			mov	r4, # immed
00d1 ac 			mov	r4, a
00d2 bd 55 			mov	r5, # immed
00d4 ad 			mov	r5, a
00d5 be 55 			mov	r6, # immed
00d7 ae 			mov	r6, a
00d8 bf 55 			mov	r7, # immed
00da af 			mov	r7, a
00db 62 			mov	t, a
00dc 0c 			movd	a, p4
00dd 0d 			movd	a, p5
00de 0e 			movd	a, p6
00df 0f 			movd	a, p7
00e0 3c 			movd	p4, a
00e1 3d 			movd	p5, a
00e2 3e 			movd	p6, a
00e3 3f 			movd	p7, a
00e4 a3 			movp	a, @a
00e5 e3 			movp3	a, @a
00e6 90 			movx	@ r0, a
00e7 91 			movx	@ r1, a
00e8 80 			movx	a, @ r0
00e9 81 			movx	a, @ r1
00ea 00 			nop
00eb 43 55 			orl	a, # immed
00ed 40 			orl	a, @ r0
00ee 41 			orl	a, @ r1
00ef 48 			orl	a, r0
00f0 49 			orl	a, r1
00f1 4a 			orl	a, r2
00f2 4b 			orl	a, r3
00f3 4c 			orl	a, r4
00f4 4d 			orl	a, r5
00f5 4e 			orl	a, r6
00f6 4f 			orl	a, r7
00f7 88 55 			orl	bus, # immed
00f9 89 55 			orl	p1, # immed
00fb 8a 55 			orl	p2, # immed
00fd 8c 			orld	p4, a
00fe 8d 			orld	p5, a
00ff 8e 			orld	p6, a
0100 8f 			orld	p7, a
0101 02 			outl	bus, a
0102 39 			outl	p1, a
0103 3a 			outl	p2, a
0104 83 			ret
0105 93 			retr
0106 e7 			rl	a
0107 f7 			rlc	a
0108 77 			rr	a
0109 67 			rrc	a
010a e5 			sel	mb0
010b f5 			sel	mb1
010c c5 			sel	rb0
010d d5 			sel	rb1
010e 65 			stop	tcnt
010f 45 			strt	cnt
0110 55 			strt	t
0111 47 			swap	a
0112 20 			xch	a, @ r0
0113 21 			xch	a, @ r1
0114 28 			xch	a, r0
0115 29 			xch	a, r1
0116 2a 			xch	a, r2
0117 2b 			xch	a, r3
0118 2c 			xch	a, r4
0119 2d 			xch	a, r5
011a 2e 			xch	a, r6
011b 2f 			xch	a, r7
011c 30 			xchd	a, @ r0
011d 31 			xchd	a, @ r1
011e d3 55 			xrl	a, # immed
0120 d0 			xrl	a, @ r0
0121 d1 			xrl	a, @ r1
0122 d8 			xrl	a, r0
0123 d9 			xrl	a, r1
0124 da 			xrl	a, r2
0125 db 			xrl	a, r3
0126 dc 			xrl	a, r4
0127 dd 			xrl	a, r5
0128 de 			xrl	a, r6
0129 df 			xrl	a, r7
