00000055 immed            00000022 direct           00007654 extend           
00000077 index1           00003333 index2           
 0x1234                 	org	$1234
 0x55                   immed	equ	$55
 0x22                   direct	equ	$22
 0x7654                 extend	equ	$7654
 0x77                   index1	equ	$77
 0x3333                 index2	equ	$3333

1234 a9 55 			adc	#immed
1236 b9 22 			adc	direct
1238 c9 76 54 			adc	extend
123b f9 			adc	,x
123c e9 77 			adc	index1,x
123e d9 33 33 			adc	index2,x
1241 ab 55 			add	#immed
1243 bb 22 			add	direct
1245 cb 76 54 			add	extend
1248 fb 			add	,x
1249 eb 77 			add	index1,x
124b db 33 33 			add	index2,x
124e a4 55 			and	#immed
1250 b4 22 			and	direct
1252 c4 76 54 			and	extend
1255 f4 			and	,x
1256 e4 77 			and	index1,x
1258 d4 33 33 			and	index2,x
125b 38 22 			asl	direct
125d 78 			asl	,x
125e 68 77 			asl	index1,x
1260 48 			asla
1261 58 			aslx
1262 37 22 			asr	direct
1264 77 			asr	,x
1265 67 77 			asr	index1,x
1267 47 			asra
1268 57 			asrx
1269 24 e7 			bcc	*-23
126b 11 22 			bclr	0,direct
126d 13 22 			bclr	1,direct
126f 15 22 			bclr	2,direct
1271 17 22 			bclr	3,direct
1273 19 22 			bclr	4,direct
1275 1b 22 			bclr	5,direct
1277 1d 22 			bclr	6,direct
1279 1f 22 			bclr	7,direct
127b 25 e7 			bcs	*-23
127d 27 e7 			beq	*-23
127f 28 e7 			bhcc	*-23
1281 29 e7 			bhcs	*-23
1283 22 e7 			bhi	*-23
1285 24 e7 			bhs	*-23
1287 2f e7 			bih	*-23
1289 2e e7 			bil	*-23
128b a5 55 			bit	#immed
128d b5 22 			bit	direct
128f c5 76 54 			bit	extend
1292 f5 			bit	,x
1293 e5 77 			bit	index1,x
1295 d5 33 33 			bit	index2,x
1298 25 e7 			blo	*-23
129a 23 e7 			bls	*-23
129c 2c e7 			bmc	*-23
129e 2b e7 			bmi	*-23
12a0 2d e7 			bms	*-23
12a2 26 e7 			bne	*-23
12a4 2a e7 			bpl	*-23
12a6 20 e7 			bra	*-23
12a8 01 22 e6 			brclr	0,direct,*-23
12ab 03 22 e6 			brclr	1,direct,*-23
12ae 05 22 e6 			brclr	2,direct,*-23
12b1 07 22 e6 			brclr	3,direct,*-23
12b4 09 22 e6 			brclr	4,direct,*-23
12b7 0b 22 e6 			brclr	5,direct,*-23
12ba 0d 22 e6 			brclr	6,direct,*-23
12bd 0f 22 e6 			brclr	7,direct,*-23
12c0 21 e7 			brn	*-23
12c2 00 22 e6 			brset	0,direct,*-23
12c5 02 22 e6 			brset	1,direct,*-23
12c8 04 22 e6 			brset	2,direct,*-23
12cb 06 22 e6 			brset	3,direct,*-23
12ce 08 22 e6 			brset	4,direct,*-23
12d1 0a 22 e6 			brset	5,direct,*-23
12d4 0c 22 e6 			brset	6,direct,*-23
12d7 0e 22 e6 			brset	7,direct,*-23
12da 10 22 			bset	0,direct
12dc 12 22 			bset	1,direct
12de 14 22 			bset	2,direct
12e0 16 22 			bset	3,direct
12e2 18 22 			bset	4,direct
12e4 1a 22 			bset	5,direct
12e6 1c 22 			bset	6,direct
12e8 1e 22 			bset	7,direct
12ea ad e7 			bsr	*-23
12ec 98 			clc
12ed 9a 			cli
12ee 3f 22 			clr	direct
12f0 7f 			clr	,x
12f1 6f 77 			clr	index1,x
12f3 4f 			clra
12f4 5f 			clrx
12f5 a1 55 			cmp	#immed
12f7 b1 22 			cmp	direct
12f9 c1 76 54 			cmp	extend
12fc f1 			cmp	,x
12fd e1 77 			cmp	index1,x
12ff d1 33 33 			cmp	index2,x
1302 33 22 			com	direct
1304 73 			com	,x
1305 63 77 			com	index1,x
1307 43 			coma
1308 53 			comx
1309 a3 55 			cpx	#immed
130b b3 22 			cpx	direct
130d c3 76 54 			cpx	extend
1310 f3 			cpx	,x
1311 e3 77 			cpx	index1,x
1313 d3 33 33 			cpx	index2,x
1316 3a 22 			dec	direct
1318 7a 			dec	,x
1319 6a 77 			dec	index1,x
131b 4a 			deca
131c 5a 			decx
131d a8 55 			eor	#immed
131f b8 22 			eor	direct
1321 c8 76 54 			eor	extend
1324 f8 			eor	,x
1325 e8 77 			eor	index1,x
1327 d8 33 33 			eor	index2,x
132a 3c 22 			inc	direct
132c 7c 			inc	,x
132d 6c 77 			inc	index1,x
132f 4c 			inca
1330 5c 			incx
1331 bc 22 			jmp	direct
1333 cc 76 54 			jmp	extend
1336 fc 			jmp	,x
1337 ec 77 			jmp	index1,x
1339 dc 33 33 			jmp	index2,x
133c bd 22 			jsr	direct
133e cd 76 54 			jsr	extend
1341 fd 			jsr	,x
1342 ed 77 			jsr	index1,x
1344 dd 33 33 			jsr	index2,x
1347 a6 55 			lda	#immed
1349 b6 22 			lda	direct
134b c6 76 54 			lda	extend
134e f6 			lda	,x
134f e6 77 			lda	index1,x
1351 d6 33 33 			lda	index2,x
1354 ae 55 			ldx	#immed
1356 be 22 			ldx	direct
1358 ce 76 54 			ldx	extend
135b fe 			ldx	,x
135c ee 77 			ldx	index1,x
135e de 33 33 			ldx	index2,x
1361 38 22 			lsl	direct
1363 78 			lsl	,x
1364 68 77 			lsl	index1,x
1366 48 			lsla
1367 58 			lslx
1368 34 22 			lsr	direct
136a 74 			lsr	,x
136b 64 77 			lsr	index1,x
136d 44 			lsra
136e 54 			lsrx
136f 30 22 			neg	direct
1371 70 			neg	,x
1372 60 77 			neg	index1,x
1374 40 			nega
1375 50 			negx
1376 9d 			nop
1377 aa 55 			ora	#immed
1379 ba 22 			ora	direct
137b ca 76 54 			ora	extend
137e fa 			ora	,x
137f ea 77 			ora	index1,x
1381 da 33 33 			ora	index2,x
1384 39 22 			rol	direct
1386 79 			rol	,x
1387 69 77 			rol	index1,x
1389 49 			rola
138a 59 			rolx
138b 36 22 			ror	direct
138d 76 			ror	,x
138e 66 77 			ror	index1,x
1390 46 			rora
1391 56 			rorx
1392 9c 			rsp
1393 80 			rti
1394 81 			rts
1395 a2 55 			sbc	#immed
1397 b2 22 			sbc	direct
1399 c2 76 54 			sbc	extend
139c f2 			sbc	,x
139d e2 77 			sbc	index1,x
139f d2 33 33 			sbc	index2,x
13a2 99 			sec
13a3 9b 			sei
13a4 b7 22 			sta	direct
13a6 c7 76 54 			sta	extend
13a9 f7 			sta	,x
13aa e7 77 			sta	index1,x
13ac d7 33 33 			sta	index2,x
13af bf 22 			stx	direct
13b1 cf 76 54 			stx	extend
13b4 ff 			stx	,x
13b5 ef 77 			stx	index1,x
13b7 df 33 33 			stx	index2,x
13ba a0 55 			sub	#immed
13bc b0 22 			sub	direct
13be c0 76 54 			sub	extend
13c1 f0 			sub	,x
13c2 e0 77 			sub	index1,x
13c4 d0 33 33 			sub	index2,x
13c7 83 			swi
13c8 97 			tax
13c9 3d 22 			tst	direct
13cb 7d 			tst	,x
13cc 6d 77 			tst	index1,x
13ce 4d 			tsta
13cf 5d 			tstx
13d0 9f 			txa
