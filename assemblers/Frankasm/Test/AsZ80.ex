0000002b disp             0000004d immed            000003db immed16          
00000118 addr             00000154 addrd            000001c5 addrj            
				cpu "64180"
 0x2b                   disp	equ 43
 0x4d                   immed	equ 77
 0x3db                  immed16 equ 987
0000 8e 			adc a, (hl)
0001 dd 8e 2b 			adc a, (ix+disp)
0004 fd 8e 2b 			adc a, (iy+disp)
0007 8f 			adc a, a
0008 88 			adc a, b
0009 89 			adc a, c
000a 8a 			adc a, d
000b 8b 			adc a, e
000c 8c 			adc a, h
000d ce 4d 			adc a, immed
000f 8d 			adc a, l
0010 ed 4a 			adc hl, bc
0012 ed 5a 			adc hl, de
0014 ed 6a 			adc hl, hl
0016 ed 7a 			adc hl, sp
0018 86 			add a, (hl)
0019 dd 86 2b 			add a, (ix+disp)
001c fd 86 2b 			add a, (iy+disp)
001f 87 			add a, a
0020 80 			add a, b
0021 81 			add a, c
0022 82 			add a, d
0023 83 			add a, e
0024 84 			add a, h
0025 c6 4d 			add a, immed
0027 85 			add a, l
0028 09 			add hl, bc
0029 19 			add hl, de
002a 29 			add hl, hl
002b 39 			add hl, sp
002c dd 09 			add ix, bc
002e dd 19 			add ix, de
0030 dd 29 			add ix, ix
0032 dd 39 			add ix, sp
0034 fd 09 			add iy, bc
0036 fd 19 			add iy, de
0038 fd 29 			add iy, iy
003a fd 39 			add iy, sp
003c a6 			and (hl)
003d dd a6 2b 			and (ix+disp)
0040 fd a6 2b 			and (iy+disp)
0043 a7 			and a
0044 a0 			and b
0045 a1 			and c
0046 a2 			and d
0047 a3 			and e
0048 a4 			and h
0049 e6 4d 			and immed
004b a5 			and l
004c cb 46 			bit 0, (hl)
004e dd cb 2b 46 		bit 0, (ix+disp)
0052 fd cb 2b 46 		bit 0, (iy+disp)
0056 cb 47 			bit 0, a
0058 cb 40 			bit 0, b
005a cb 41 			bit 0, c
005c cb 42 			bit 0, d
005e cb 43 			bit 0, e
0060 cb 44 			bit 0, h
0062 cb 45 			bit 0, l
0064 cb 4e 			bit 1, (hl)
0066 dd cb 2b 4e 		bit 1, (ix+disp)
006a fd cb 2b 4e 		bit 1, (iy+disp)
006e cb 4f 			bit 1, a
0070 cb 48 			bit 1, b
0072 cb 49 			bit 1, c
0074 cb 4a 			bit 1, d
0076 cb 4b 			bit 1, e
0078 cb 4c 			bit 1, h
007a cb 4d 			bit 1, l
007c cb 56 			bit 2, (hl)
007e dd cb 2b 56 		bit 2, (ix+disp)
0082 fd cb 2b 56 		bit 2, (iy+disp)
0086 cb 57 			bit 2, a
0088 cb 50 			bit 2, b
008a cb 51 			bit 2, c
008c cb 52 			bit 2, d
008e cb 53 			bit 2, e
0090 cb 54 			bit 2, h
0092 cb 55 			bit 2, l
0094 cb 5e 			bit 3, (hl)
0096 dd cb 2b 5e 		bit 3, (ix+disp)
009a fd cb 2b 5e 		bit 3, (iy+disp)
009e cb 5f 			bit 3, a
00a0 cb 58 			bit 3, b
00a2 cb 59 			bit 3, c
00a4 cb 5a 			bit 3, d
00a6 cb 5b 			bit 3, e
00a8 cb 5c 			bit 3, h
00aa cb 5d 			bit 3, l
00ac cb 66 			bit 4, (hl)
00ae dd cb 2b 66 		bit 4, (ix+disp)
00b2 fd cb 2b 66 		bit 4, (iy+disp)
00b6 cb 67 			bit 4, a
00b8 cb 60 			bit 4, b
00ba cb 61 			bit 4, c
00bc cb 62 			bit 4, d
00be cb 63 			bit 4, e
00c0 cb 64 			bit 4, h
00c2 cb 65 			bit 4, l
00c4 cb 6e 			bit 5, (hl)
00c6 dd cb 2b 6e 		bit 5, (ix+disp)
00ca fd cb 2b 6e 		bit 5, (iy+disp)
00ce cb 6f 			bit 5, a
00d0 cb 68 			bit 5, b
00d2 cb 69 			bit 5, c
00d4 cb 6a 			bit 5, d
00d6 cb 6b 			bit 5, e
00d8 cb 6c 			bit 5, h
00da cb 6d 			bit 5, l
00dc cb 76 			bit 6, (hl)
00de dd cb 2b 76 		bit 6, (ix+disp)
00e2 fd cb 2b 76 		bit 6, (iy+disp)
00e6 cb 77 			bit 6, a
00e8 cb 70 			bit 6, b
00ea cb 71 			bit 6, c
00ec cb 72 			bit 6, d
00ee cb 73 			bit 6, e
00f0 cb 74 			bit 6, h
00f2 cb 75 			bit 6, l
00f4 cb 7e 			bit 7, (hl)
00f6 dd cb 2b 7e 		bit 7, (ix+disp)
00fa fd cb 2b 7e 		bit 7, (iy+disp)
00fe cb 7f 			bit 7, a
0100 cb 78 			bit 7, b
0102 cb 79 			bit 7, c
0104 cb 7a 			bit 7, d
0106 cb 7b 			bit 7, e
0108 cb 7c 			bit 7, h
010a cb 7d 			bit 7, l
010c cd 18 01 			call addr
010f dc 18 01 			call c, addr
0112 fc 18 01 			call m, addr
0115 d4 18 01 			call nc, addr
0118 c4 18 01 		addr	 call nz, addr
011b f4 18 01 			call p, addr
011e ec 18 01 			call pe, addr
0121 e4 18 01 			call po, addr
0124 cc 18 01 			call z, addr
0127 3f 			ccf
0128 be 			cp (hl)
0129 dd be 2b 			cp (ix+disp)
012c fd be 2b 			cp (iy+disp)
012f bf 			cp a
0130 b8 			cp b
0131 b9 			cp c
0132 ba 			cp d
0133 bb 			cp e
0134 bc 			cp h
0135 fe 4d 			cp immed
0137 bd 			cp l
0138 ed a9 			cpd
013a ed b9 			cpdr
013c ed a1 			cpi
013e ed b1 			cpir
0140 2f 			cpl
0141 27 			daa
0142 35 			dec (hl)
0143 dd 35 2b 			dec (ix+disp)
0146 fd 35 2b 			dec (iy+disp)
0149 3d 			dec a
014a 05 			dec b
014b 0b 			dec bc
014c 0d 			dec c
014d 15 			dec d
014e 1b 			dec de
014f 1d 			dec e
0150 25 			dec h
0151 2b 			dec hl
0152 dd 2b 			dec ix
0154 fd 2b 		addrd	 dec iy
0156 2d 			dec l
0157 3b 			dec sp
0158 f3 			di
0159 10 f9 			djnz addrd
015b fb 			ei
015c e3 			ex ( sp ) , hl
015d dd e3 			ex ( sp ) , ix
015f fd e3 			ex ( sp ) , iy
0161 08 			ex af, af
0162 eb 			ex de, hl
0163 d9 			exx
0164 76 			halt
0165 ed 46 			im 0
0167 ed 56 			im 1
0169 ed 5e 			im 2
016b ed 78 			in a, ( c )
016d db 4d 			in a, ( immed )
016f ed 40 			in b, ( c )
0171 ed 48 			in c, ( c )
0173 ed 50 			in d, ( c )
0175 ed 58 			in e, ( c )
0177 ed 60 			in h, ( c )
0179 ed 68 			in l, ( c )
017b ed 38 4d 			in0 a, ( immed )
017e ed 00 4d 			in0 b, ( immed )
0181 ed 08 4d 			in0 c, ( immed )
0184 ed 10 4d 			in0 d, ( immed )
0187 ed 18 4d 			in0 e, ( immed )
018a ed 20 4d 			in0 h, ( immed )
018d ed 28 4d 			in0 l, ( immed )
0190 34 			inc (hl)
0191 dd 34 2b 			inc (ix+disp)
0194 fd 34 2b 			inc (iy+disp)
0197 3c 			inc a
0198 04 			inc b
0199 03 			inc bc
019a 0c 			inc c
019b 14 			inc d
019c 13 			inc de
019d 1c 			inc e
019e 24 			inc h
019f 23 			inc hl
01a0 dd 23 			inc ix
01a2 fd 23 			inc iy
01a4 2c 			inc l
01a5 33 			inc sp
01a6 ed aa 			ind
01a8 ed ba 			indr
01aa ed a2 			ini
01ac ed b2 			inir
01ae e9 			jp ( hl )
01af dd e9 			jp ( ix )
01b1 fd e9 			jp ( iy )
01b3 c3 c5 01 			jp addrj
01b6 da c5 01 			jp c, addrj
01b9 fa c5 01 			jp m, addrj
01bc d2 c5 01 			jp nc, addrj
01bf c2 c5 01 			jp nz, addrj
01c2 f2 c5 01 			jp p, addrj
01c5 ea c5 01 		addrj	 jp pe, addrj
01c8 e2 c5 01 			jp po, addrj
01cb ca c5 01 			jp z, addrj
01ce 18 f5 			jr addrj
01d0 38 f3 			jr c, addrj
01d2 30 f1 			jr nc, addrj
01d4 20 ef 			jr nz, addrj
01d6 28 ed 			jr z, addrj
01d8 32 18 01 			ld ( addr ) , a
01db ed 43 18 01 		ld ( addr ) , bc
01df ed 53 18 01 		ld ( addr ) , de
01e3 22 18 01 			ld ( addr ) , hl
01e6 22 18 01 			ld ( addr ) , hl
01e9 dd 22 18 01 		ld ( addr ) , ix
01ed fd 22 18 01 		ld ( addr ) , iy
01f1 ed 73 18 01 		ld ( addr ) , sp
01f5 02 			ld ( bc ) , a
01f6 12 			ld ( de ) , a
01f7 77 			ld (hl), a
01f8 70 			ld (hl), b
01f9 71 			ld (hl), c
01fa 72 			ld (hl), d
01fb 73 			ld (hl), e
01fc 74 			ld (hl), h
01fd 36 4d 			ld (hl), immed
01ff 75 			ld (hl), l
0200 dd 77 2b 			ld (ix+disp), a
0203 dd 70 2b 			ld (ix+disp), b
0206 dd 71 2b 			ld (ix+disp), c
0209 dd 72 2b 			ld (ix+disp), d
020c dd 73 2b 			ld (ix+disp), e
020f dd 74 2b 			ld (ix+disp), h
0212 dd 36 2b 4d 		ld (ix+disp), immed
0216 dd 75 2b 			ld (ix+disp), l
0219 fd 77 2b 			ld (iy+disp), a
021c fd 70 2b 			ld (iy+disp), b
021f fd 71 2b 			ld (iy+disp), c
0222 fd 72 2b 			ld (iy+disp), d
0225 fd 73 2b 			ld (iy+disp), e
0228 fd 74 2b 			ld (iy+disp), h
022b fd 36 2b 4d 		ld (iy+disp), immed
022f fd 75 2b 			ld (iy+disp), l
0232 3a 18 01 			ld a, ( addr )
0235 0a 			ld a, ( bc )
0236 1a 			ld a, ( de )
0237 7e 			ld a, (hl)
0238 dd 7e 2b 			ld a, (ix+disp)
023b fd 7e 2b 			ld a, (iy+disp)
023e 7f 			ld a, a
023f 78 			ld a, b
0240 79 			ld a, c
0241 7a 			ld a, d
0242 7b 			ld a, e
0243 7c 			ld a, h
0244 ed 57 			ld a, i
0246 3e 4d 			ld a, immed
0248 7d 			ld a, l
0249 ed 5f 			ld a, r
024b 46 			ld b, (hl)
024c dd 46 2b 			ld b, (ix+disp)
024f fd 46 2b 			ld b, (iy+disp)
0252 47 			ld b, a
0253 40 			ld b, b
0254 41 			ld b, c
0255 42 			ld b, d
0256 43 			ld b, e
0257 44 			ld b, h
0258 06 4d 			ld b, immed
025a 45 			ld b, l
025b ed 4b 18 01 		ld bc, ( addr )
025f 01 db 03 			ld bc, immed16
0262 4e 			ld c, (hl)
0263 dd 4e 2b 			ld c, (ix+disp)
0266 fd 4e 2b 			ld c, (iy+disp)
0269 4f 			ld c, a
026a 48 			ld c, b
026b 49 			ld c, c
026c 4a 			ld c, d
026d 4b 			ld c, e
026e 4c 			ld c, h
026f 0e 4d 			ld c, immed
0271 4d 			ld c, l
0272 56 			ld d, (hl)
0273 dd 56 2b 			ld d, (ix+disp)
0276 fd 56 2b 			ld d, (iy+disp)
0279 57 			ld d, a
027a 50 			ld d, b
027b 51 			ld d, c
027c 52 			ld d, d
027d 53 			ld d, e
027e 54 			ld d, h
027f 16 4d 			ld d, immed
0281 55 			ld d, l
0282 ed 5b 18 01 		ld de, ( addr )
0286 11 db 03 			ld de, immed16
0289 5e 			ld e, (hl)
028a dd 5e 2b 			ld e, (ix+disp)
028d fd 5e 2b 			ld e, (iy+disp)
0290 5f 			ld e, a
0291 58 			ld e, b
0292 59 			ld e, c
0293 5a 			ld e, d
0294 5b 			ld e, e
0295 5c 			ld e, h
0296 1e 4d 			ld e, immed
0298 5d 			ld e, l
0299 66 			ld h, (hl)
029a dd 66 2b 			ld h, (ix+disp)
029d fd 66 2b 			ld h, (iy+disp)
02a0 67 			ld h, a
02a1 60 			ld h, b
02a2 61 			ld h, c
02a3 62 			ld h, d
02a4 63 			ld h, e
02a5 64 			ld h, h
02a6 26 4d 			ld h, immed
02a8 65 			ld h, l
02a9 2a 18 01 			ld hl, ( addr )
02ac 2a 18 01 			ld hl, ( addr )
02af 21 db 03 			ld hl, immed16
02b2 21 db 03 			ld hl, immed16
02b5 ed 47 			ld i, a
02b7 dd 2a 18 01 		ld ix, ( addr )
02bb dd 21 db 03 		ld ix, immed16
02bf fd 2a 18 01 		ld iy, ( addr )
02c3 fd 21 db 03 		ld iy, immed16
02c7 6e 			ld l, (hl)
02c8 dd 6e 2b 			ld l, (ix+disp)
02cb fd 6e 2b 			ld l, (iy+disp)
02ce 6f 			ld l, a
02cf 68 			ld l, b
02d0 69 			ld l, c
02d1 6a 			ld l, d
02d2 6b 			ld l, e
02d3 6c 			ld l, h
02d4 2e 4d 			ld l, immed
02d6 6d 			ld l, l
02d7 ed 4f 			ld r, a
02d9 ed 7b 18 01 		ld sp, ( addr )
02dd f9 			ld sp, hl
02de 31 db 03 			ld sp, immed16
02e1 dd f9 			ld sp, ix
02e3 fd f9 			ld sp, iy
02e5 ed a8 			ldd
02e7 ed b8 			lddr
02e9 ed a0 			ldi
02eb ed b0 			ldir
02ed ed 4c 			mult bc
02ef ed 5c 			mult de
02f1 ed 6c 			mult hl
02f3 ed 7c 			mult sp
02f5 ed 44 			neg
02f7 00 			nop
02f8 b6 			or (hl)
02f9 dd b6 2b 			or (ix+disp)
02fc fd b6 2b 			or (iy+disp)
02ff b7 			or a
0300 b0 			or b
0301 b1 			or c
0302 b2 			or d
0303 b3 			or e
0304 b4 			or h
0305 f6 4d 			or immed
0307 b5 			or l
0308 ed 8b 			otdm
030a ed 9b 			otdmr
030c ed bb 			otdr
030e ed 83 			otim
0310 ed 93 			otimr
0312 ed b3 			otir
0314 ed 79 			out ( c ) , a
0316 ed 41 			out ( c ) , b
0318 ed 49 			out ( c ) , c
031a ed 51 			out ( c ) , d
031c ed 59 			out ( c ) , e
031e ed 61 			out ( c ) , h
0320 ed 69 			out ( c ) , l
0322 d3 4d 			out ( immed ) , a
0324 ed 39 4d 			out0 ( immed ) , a
0327 ed 01 4d 			out0 ( immed ) , b
032a ed 09 4d 			out0 ( immed ) , c
032d ed 11 4d 			out0 ( immed ) , d
0330 ed 19 4d 			out0 ( immed ) , e
0333 ed 21 4d 			out0 ( immed ) , h
0336 ed 29 4d 			out0 ( immed ) , l
0339 ed ab 			outd
033b ed a3 			outi
033d f1 			pop af
033e c1 			pop bc
033f d1 			pop de
0340 e1 			pop hl
0341 dd e1 			pop ix
0343 fd e1 			pop iy
0345 f5 			push af
0346 c5 			push bc
0347 d5 			push de
0348 e5 			push hl
0349 dd e5 			push ix
034b fd e5 			push iy
034d cb 86 			res 0, (hl)
034f dd cb 2b 86 		res 0, (ix+disp)
0353 fd cb 2b 86 		res 0, (iy+disp)
0357 cb 87 			res 0, a
0359 cb 80 			res 0, b
035b cb 81 			res 0, c
035d cb 82 			res 0, d
035f cb 83 			res 0, e
0361 cb 84 			res 0, h
0363 cb 85 			res 0, l
0365 cb 8e 			res 1, (hl)
0367 dd cb 2b 8e 		res 1, (ix+disp)
036b fd cb 2b 8e 		res 1, (iy+disp)
036f cb 8f 			res 1, a
0371 cb 88 			res 1, b
0373 cb 89 			res 1, c
0375 cb 8a 			res 1, d
0377 cb 8b 			res 1, e
0379 cb 8c 			res 1, h
037b cb 8d 			res 1, l
037d cb 96 			res 2, (hl)
037f dd cb 2b 96 		res 2, (ix+disp)
0383 fd cb 2b 96 		res 2, (iy+disp)
0387 cb 97 			res 2, a
0389 cb 90 			res 2, b
038b cb 91 			res 2, c
038d cb 92 			res 2, d
038f cb 93 			res 2, e
0391 cb 94 			res 2, h
0393 cb 95 			res 2, l
0395 cb 9e 			res 3, (hl)
0397 dd cb 2b 9e 		res 3, (ix+disp)
039b fd cb 2b 9e 		res 3, (iy+disp)
039f cb 9f 			res 3, a
03a1 cb 98 			res 3, b
03a3 cb 99 			res 3, c
03a5 cb 9a 			res 3, d
03a7 cb 9b 			res 3, e
03a9 cb 9c 			res 3, h
03ab cb 9d 			res 3, l
03ad cb a6 			res 4, (hl)
03af dd cb 2b a6 		res 4, (ix+disp)
03b3 fd cb 2b a6 		res 4, (iy+disp)
03b7 cb a7 			res 4, a
03b9 cb a0 			res 4, b
03bb cb a1 			res 4, c
03bd cb a2 			res 4, d
03bf cb a3 			res 4, e
03c1 cb a4 			res 4, h
03c3 cb a5 			res 4, l
03c5 cb ae 			res 5, (hl)
03c7 dd cb 2b ae 		res 5, (ix+disp)
03cb fd cb 2b ae 		res 5, (iy+disp)
03cf cb af 			res 5, a
03d1 cb a8 			res 5, b
03d3 cb a9 			res 5, c
03d5 cb aa 			res 5, d
03d7 cb ab 			res 5, e
03d9 cb ac 			res 5, h
03db cb ad 			res 5, l
03dd cb b6 			res 6, (hl)
03df dd cb 2b b6 		res 6, (ix+disp)
03e3 fd cb 2b b6 		res 6, (iy+disp)
03e7 cb b7 			res 6, a
03e9 cb b0 			res 6, b
03eb cb b1 			res 6, c
03ed cb b2 			res 6, d
03ef cb b3 			res 6, e
03f1 cb b4 			res 6, h
03f3 cb b5 			res 6, l
03f5 cb be 			res 7, (hl)
03f7 dd cb 2b be 		res 7, (ix+disp)
03fb fd cb 2b be 		res 7, (iy+disp)
03ff cb bf 			res 7, a
0401 cb b8 			res 7, b
0403 cb b9 			res 7, c
0405 cb ba 			res 7, d
0407 cb bb 			res 7, e
0409 cb bc 			res 7, h
040b cb bd 			res 7, l
040d c9 			ret
040e d8 			ret c
040f f8 			ret m
0410 d0 			ret nc
0411 c0 			ret nz
0412 f0 			ret p
0413 e8 			ret pe
0414 e0 			ret po
0415 c8 			ret z
0416 ed 4d 			reti
0418 ed 45 			retn
041a cb 16 			rl (hl)
041c dd cb 2b 16 		rl (ix+disp)
0420 fd cb 2b 16 		rl (iy+disp)
0424 cb 17 			rl a
0426 cb 10 			rl b
0428 cb 11 			rl c
042a cb 12 			rl d
042c cb 13 			rl e
042e cb 14 			rl h
0430 cb 15 			rl l
0432 17 			rla
0433 cb 06 			rlc (hl)
0435 dd cb 2b 06 		rlc (ix+disp)
0439 fd cb 2b 06 		rlc (iy+disp)
043d cb 07 			rlc a
043f cb 00 			rlc b
0441 cb 01 			rlc c
0443 cb 02 			rlc d
0445 cb 03 			rlc e
0447 cb 04 			rlc h
0449 cb 05 			rlc l
044b 07 			rlca
044c ed 6f 			rld
044e cb 1e 			rr (hl)
0450 dd cb 2b 1e 		rr (ix+disp)
0454 fd cb 2b 1e 		rr (iy+disp)
0458 cb 1f 			rr a
045a cb 18 			rr b
045c cb 19 			rr c
045e cb 1a 			rr d
0460 cb 1b 			rr e
0462 cb 1c 			rr h
0464 cb 1d 			rr l
0466 1f 			rra
0467 cb 0e 			rrc (hl)
0469 dd cb 2b 0e 		rrc (ix+disp)
046d fd cb 2b 0e 		rrc (iy+disp)
0471 cb 0f 			rrc a
0473 cb 08 			rrc b
0475 cb 09 			rrc c
0477 cb 0a 			rrc d
0479 cb 0b 			rrc e
047b cb 0c 			rrc h
047d cb 0d 			rrc l
047f 0f 			rrca
0480 ed 67 			rrd
0482 c7 			rst 0
0483 d7 			rst 16
0484 df 			rst 24
0485 e7 			rst 32
0486 ef 			rst 40
0487 f7 			rst 48
0488 ff 			rst 56
0489 cf 			rst 8
048a 9e 			sbc a, (hl)
048b dd 9e 2b 			sbc a, (ix+disp)
048e fd 9e 2b 			sbc a, (iy+disp)
0491 9f 			sbc a, a
0492 98 			sbc a, b
0493 99 			sbc a, c
0494 9a 			sbc a, d
0495 9b 			sbc a, e
0496 9c 			sbc a, h
0497 de 4d 			sbc a, immed
0499 9d 			sbc a, l
049a ed 42 			sbc hl, bc
049c ed 52 			sbc hl, de
049e ed 62 			sbc hl, hl
04a0 ed 72 			sbc hl, sp
04a2 37 			scf
04a3 cb c6 			set 0, (hl)
04a5 dd cb 2b c6 		set 0, (ix+disp)
04a9 fd cb 2b c6 		set 0, (iy+disp)
04ad cb c7 			set 0, a
04af cb c0 			set 0, b
04b1 cb c1 			set 0, c
04b3 cb c2 			set 0, d
04b5 cb c3 			set 0, e
04b7 cb c4 			set 0, h
04b9 cb c5 			set 0, l
04bb cb ce 			set 1, (hl)
04bd dd cb 2b ce 		set 1, (ix+disp)
04c1 fd cb 2b ce 		set 1, (iy+disp)
04c5 cb cf 			set 1, a
04c7 cb c8 			set 1, b
04c9 cb c9 			set 1, c
04cb cb ca 			set 1, d
04cd cb cb 			set 1, e
04cf cb cc 			set 1, h
04d1 cb cd 			set 1, l
04d3 cb d6 			set 2, (hl)
04d5 dd cb 2b d6 		set 2, (ix+disp)
04d9 fd cb 2b d6 		set 2, (iy+disp)
04dd cb d7 			set 2, a
04df cb d0 			set 2, b
04e1 cb d1 			set 2, c
04e3 cb d2 			set 2, d
04e5 cb d3 			set 2, e
04e7 cb d4 			set 2, h
04e9 cb d5 			set 2, l
04eb cb de 			set 3, (hl)
04ed dd cb 2b de 		set 3, (ix+disp)
04f1 fd cb 2b de 		set 3, (iy+disp)
04f5 cb df 			set 3, a
04f7 cb d8 			set 3, b
04f9 cb d9 			set 3, c
04fb cb da 			set 3, d
04fd cb db 			set 3, e
04ff cb dc 			set 3, h
0501 cb dd 			set 3, l
0503 cb e6 			set 4, (hl)
0505 dd cb 2b e6 		set 4, (ix+disp)
0509 fd cb 2b e6 		set 4, (iy+disp)
050d cb e7 			set 4, a
050f cb e0 			set 4, b
0511 cb e1 			set 4, c
0513 cb e2 			set 4, d
0515 cb e3 			set 4, e
0517 cb e4 			set 4, h
0519 cb e5 			set 4, l
051b cb ee 			set 5, (hl)
051d dd cb 2b ee 		set 5, (ix+disp)
0521 fd cb 2b ee 		set 5, (iy+disp)
0525 cb ef 			set 5, a
0527 cb e8 			set 5, b
0529 cb e9 			set 5, c
052b cb ea 			set 5, d
052d cb eb 			set 5, e
052f cb ec 			set 5, h
0531 cb ed 			set 5, l
0533 cb f6 			set 6, (hl)
0535 dd cb 2b f6 		set 6, (ix+disp)
0539 fd cb 2b f6 		set 6, (iy+disp)
053d cb f7 			set 6, a
053f cb f0 			set 6, b
0541 cb f1 			set 6, c
0543 cb f2 			set 6, d
0545 cb f3 			set 6, e
0547 cb f4 			set 6, h
0549 cb f5 			set 6, l
054b cb fe 			set 7, (hl)
054d dd cb 2b fe 		set 7, (ix+disp)
0551 fd cb 2b fe 		set 7, (iy+disp)
0555 cb ff 			set 7, a
0557 cb f8 			set 7, b
0559 cb f9 			set 7, c
055b cb fa 			set 7, d
055d cb fb 			set 7, e
055f cb fc 			set 7, h
0561 cb fd 			set 7, l
0563 cb 26 			sla (hl)
0565 dd cb 2b 26 		sla (ix+disp)
0569 fd cb 2b 26 		sla (iy+disp)
056d cb 27 			sla a
056f cb 20 			sla b
0571 cb 21 			sla c
0573 cb 22 			sla d
0575 cb 23 			sla e
0577 cb 24 			sla h
0579 cb 25 			sla l
057b ed 76 			slp
057d cb 2e 			sra (hl)
057f dd cb 2b 2e 		sra (ix+disp)
0583 fd cb 2b 2e 		sra (iy+disp)
0587 cb 2f 			sra a
0589 cb 28 			sra b
058b cb 29 			sra c
058d cb 2a 			sra d
058f cb 2b 			sra e
0591 cb 2c 			sra h
0593 cb 2d 			sra l
0595 cb 3e 			srl (hl)
0597 dd cb 2b 3e 		srl (ix+disp)
059b fd cb 2b 3e 		srl (iy+disp)
059f cb 3f 			srl a
05a1 cb 38 			srl b
05a3 cb 39 			srl c
05a5 cb 3a 			srl d
05a7 cb 3b 			srl e
05a9 cb 3c 			srl h
05ab cb 3d 			srl l
05ad 96 			sub (hl)
05ae dd 96 2b 			sub (ix+disp)
05b1 fd 96 2b 			sub (iy+disp)
05b4 97 			sub a
05b5 90 			sub b
05b6 91 			sub c
05b7 92 			sub d
05b8 93 			sub e
05b9 94 			sub h
05ba d6 4d 			sub immed
05bc 95 			sub l
05bd ed 34 			tst ( hl )
05bf ed 3c 			tst a
05c1 ed 04 			tst b
05c3 ed 0c 			tst c
05c5 ed 14 			tst d
05c7 ed 1c 			tst e
05c9 ed 24 			tst h
05cb ed 64 4d 			tst immed
05ce ed 2c 			tst l
05d0 ed 74 4d 			tstio immed
05d3 ae 			xor (hl)
05d4 dd ae 2b 			xor (ix+disp)
05d7 fd ae 2b 			xor (iy+disp)
05da af 			xor a
05db a8 			xor b
05dc a9 			xor c
05dd aa 			xor d
05de ab 			xor e
05df ac 			xor h
05e0 ee 4d 			xor immed
05e2 ad 			xor l
