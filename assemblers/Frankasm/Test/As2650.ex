00001234 absolute         000000a5 immed            
 0x1234                 absolute	equ	1234h
 0xa5                   immed		equ	$a5
				adda,r0   *absolute
 WARNING - Page Boundary
0000 8c 92 34 
0003 8c 12 34 			adda,r0   absolute
0006 8c 52 34 			adda,r0   absolute,r0,-
0009 8d 52 34 			adda,r0   absolute,r1,-
000c 8e 52 34 			adda,r0   absolute,r2,-
000f 8f 52 34 			adda,r0   absolute,r3,-
0012 8c f2 34 			adda,r0  *absolute,r0
0015 8c b2 34 			adda,r0  *absolute,r0,+
0018 8c d2 34 			adda,r0  *absolute,r0,-
001b 8d f2 34 			adda,r0  *absolute,r1
001e 8d b2 34 			adda,r0  *absolute,r1,+
0021 8d d2 34 			adda,r0  *absolute,r1,-
0024 8e f2 34 			adda,r0  *absolute,r2
0027 8e b2 34 			adda,r0  *absolute,r2,+
002a 8e d2 34 			adda,r0  *absolute,r2,-
002d 8f f2 34 			adda,r0  *absolute,r3
0030 8f b2 34 			adda,r0  *absolute,r3,+
0033 8f d2 34 			adda,r0  *absolute,r3,-
0036 8c 72 34 			adda,r0  absolute,r0
0039 8c 32 34 			adda,r0  absolute,r0,+
003c 8d 72 34 			adda,r0  absolute,r1
003f 8d 32 34 			adda,r0  absolute,r1,+
0042 8e 72 34 			adda,r0  absolute,r2
0045 8e 32 34 			adda,r0  absolute,r2,+
0048 8f 72 34 			adda,r0  absolute,r3
004b 8f 32 34 			adda,r0  absolute,r3,+
004e 8d 92 34 			adda,r1   *absolute
0051 8d 12 34 			adda,r1   absolute
0054 8e 92 34 			adda,r2   *absolute
0057 8e 12 34 			adda,r2   absolute
005a 8f 92 34 			adda,r3   *absolute
005d 8f 12 34 			adda,r3   absolute
0060 84 a5 			addi,r0   immed
0062 85 a5 			addi,r1   immed
0064 86 a5 			addi,r2   immed
0066 87 a5 			addi,r3   immed
0068 88 5b 			addr,r0   $-23h
006a 88 db 			addr,r0   *$-23h
006c 89 5b 			addr,r1   $-23h
006e 89 db 			addr,r1   *$-23h
0070 8a 5b 			addr,r2   $-23h
0072 8a db 			addr,r2   *$-23h
0074 8b 5b 			addr,r3   $-23h
0076 8b db 			addr,r3   *$-23h
0078 80 			addz  r0
0079 81 			addz  r1
007a 82 			addz  r2
007b 83 			addz  r3
007c 4c 92 34 			anda,r0   *absolute
007f 4c 12 34 			anda,r0   absolute
0082 4c 52 34 			anda,r0   absolute,r0,-
0085 4d 52 34 			anda,r0   absolute,r1,-
0088 4e 52 34 			anda,r0   absolute,r2,-
008b 4f 52 34 			anda,r0   absolute,r3,-
008e 4c f2 34 			anda,r0  *absolute,r0
0091 4c b2 34 			anda,r0  *absolute,r0,+
0094 4c d2 34 			anda,r0  *absolute,r0,-
0097 4d f2 34 			anda,r0  *absolute,r1
009a 4d b2 34 			anda,r0  *absolute,r1,+
009d 4d d2 34 			anda,r0  *absolute,r1,-
00a0 4e f2 34 			anda,r0  *absolute,r2
00a3 4e b2 34 			anda,r0  *absolute,r2,+
00a6 4e d2 34 			anda,r0  *absolute,r2,-
00a9 4f f2 34 			anda,r0  *absolute,r3
00ac 4f b2 34 			anda,r0  *absolute,r3,+
00af 4f d2 34 			anda,r0  *absolute,r3,-
00b2 4c 72 34 			anda,r0  absolute,r0
00b5 4c 32 34 			anda,r0  absolute,r0,+
00b8 4d 72 34 			anda,r0  absolute,r1
00bb 4d 32 34 			anda,r0  absolute,r1,+
00be 4e 72 34 			anda,r0  absolute,r2
00c1 4e 32 34 			anda,r0  absolute,r2,+
00c4 4f 72 34 			anda,r0  absolute,r3
00c7 4f 32 34 			anda,r0  absolute,r3,+
00ca 4d 92 34 			anda,r1   *absolute
00cd 4d 12 34 			anda,r1   absolute
00d0 4e 92 34 			anda,r2   *absolute
00d3 4e 12 34 			anda,r2   absolute
00d6 4f 92 34 			anda,r3   *absolute
00d9 4f 12 34 			anda,r3   absolute
00dc 44 a5 			andi,r0   immed
00de 45 a5 			andi,r1   immed
00e0 46 a5 			andi,r2   immed
00e2 47 a5 			andi,r3   immed
00e4 48 5b 			andr,r0   $-23h
00e6 48 db 			andr,r0   *$-23h
00e8 49 5b 			andr,r1   $-23h
00ea 49 db 			andr,r1   *$-23h
00ec 4a 5b 			andr,r2   $-23h
00ee 4a db 			andr,r2   *$-23h
00f0 4b 5b 			andr,r3   $-23h
00f2 4b db 			andr,r3   *$-23h
00f4 41 			andz  r1
00f5 42 			andz  r2
00f6 43 			andz  r3
00f7 9c 92 34 			bcfa,eq   *absolute
00fa 9c 12 34 			bcfa,eq   absolute
00fd 9d 92 34 			bcfa,gt   *absolute
0100 9d 12 34 			bcfa,gt   absolute
0103 9e 92 34 			bcfa,lt   *absolute
0106 9e 12 34 			bcfa,lt   absolute
0109 98 5b 			bcfr,eq   $-23h
010b 98 db 			bcfr,eq   *$-23h
010d 99 5b 			bcfr,gt   $-23h
010f 99 db 			bcfr,gt   *$-23h
0111 9a 5b 			bcfr,lt   $-23h
0113 9a db 			bcfr,lt   *$-23h
0115 1f 92 34 			bcta,always   *absolute
0118 1f 12 34 			bcta,always   absolute
011b 1c 92 34 			bcta,eq   *absolute
011e 1c 12 34 			bcta,eq   absolute
0121 1d 92 34 			bcta,gt   *absolute
0124 1d 12 34 			bcta,gt   absolute
0127 1e 92 34 			bcta,lt   *absolute
012a 1e 12 34 			bcta,lt   absolute
012d 1b 5b 			bctr,always   $-23h
012f 1b db 			bctr,always   *$-23h
0131 18 5b 			bctr,eq   $-23h
0133 18 db 			bctr,eq   *$-23h
0135 19 5b 			bctr,gt   $-23h
0137 19 db 			bctr,gt   *$-23h
0139 1a 5b 			bctr,lt   $-23h
013b 1a db 			bctr,lt   *$-23h
013d fc 92 34 			bdra,r0   *absolute
0140 fc 12 34 			bdra,r0   absolute
0143 fd 92 34 			bdra,r1   *absolute
0146 fd 12 34 			bdra,r1   absolute
0149 fe 92 34 			bdra,r2   *absolute
014c fe 12 34 			bdra,r2   absolute
014f ff 92 34 			bdra,r3   *absolute
0152 ff 12 34 			bdra,r3   absolute
0155 f8 5b 			bdrr,r0   $-23h
0157 f8 db 			bdrr,r0   *$-23h
0159 f9 5b 			bdrr,r1   $-23h
015b f9 db 			bdrr,r1   *$-23h
015d fa 5b 			bdrr,r2   $-23h
015f fa db 			bdrr,r2   *$-23h
0161 fb 5b 			bdrr,r3   $-23h
0163 fb db 			bdrr,r3   *$-23h
0165 dc 92 34 			bira,r0   *absolute
0168 dc 12 34 			bira,r0   absolute
016b dd 92 34 			bira,r1   *absolute
016e dd 12 34 			bira,r1   absolute
0171 de 92 34 			bira,r2   *absolute
0174 de 12 34 			bira,r2   absolute
0177 df 92 34 			bira,r3   *absolute
017a df 12 34 			bira,r3   absolute
017d d8 5b 			birr,r0   $-23h
017f d8 db 			birr,r0   *$-23h
0181 d9 5b 			birr,r1   $-23h
0183 d9 db 			birr,r1   *$-23h
0185 da 5b 			birr,r2   $-23h
0187 da db 			birr,r2   *$-23h
0189 db 5b 			birr,r3   $-23h
018b db db 			birr,r3   *$-23h
018d 5c 92 34 			brna,r0   *absolute
0190 5c 12 34 			brna,r0   absolute
0193 5d 92 34 			brna,r1   *absolute
0196 5d 12 34 			brna,r1   absolute
0199 5e 92 34 			brna,r2   *absolute
019c 5e 12 34 			brna,r2   absolute
019f 5f 92 34 			brna,r3   *absolute
01a2 5f 12 34 			brna,r3   absolute
01a5 58 5b 			brnr,r0   $-23h
01a7 58 db 			brnr,r0   *$-23h
01a9 59 5b 			brnr,r1   $-23h
01ab 59 db 			brnr,r1   *$-23h
01ad 5a 5b 			brnr,r2   $-23h
01af 5a db 			brnr,r2   *$-23h
01b1 5b 5b 			brnr,r3   $-23h
01b3 5b db 			brnr,r3   *$-23h
01b5 bc 92 34 			bsfa,eq   *absolute
01b8 bc 12 34 			bsfa,eq   absolute
01bb bd 92 34 			bsfa,gt   *absolute
01be bd 12 34 			bsfa,gt   absolute
01c1 be 92 34 			bsfa,lt   *absolute
01c4 be 12 34 			bsfa,lt   absolute
01c7 b8 5b 			bsfr,eq   $-23h
01c9 b8 db 			bsfr,eq   *$-23h
01cb b9 5b 			bsfr,gt   $-23h
01cd b9 db 			bsfr,gt   *$-23h
01cf ba 5b 			bsfr,lt   $-23h
01d1 ba db 			bsfr,lt   *$-23h
01d3 7c 92 34 			bsna,r0   *absolute
01d6 7c 12 34 			bsna,r0   absolute
01d9 7d 92 34 			bsna,r1   *absolute
01dc 7d 12 34 			bsna,r1   absolute
01df 7e 92 34 			bsna,r2   *absolute
01e2 7e 12 34 			bsna,r2   absolute
01e5 7f 92 34 			bsna,r3   *absolute
01e8 7f 12 34 			bsna,r3   absolute
01eb 78 5b 			bsnr,r0   $-23h
01ed 78 db 			bsnr,r0   *$-23h
01ef 79 5b 			bsnr,r1   $-23h
01f1 79 db 			bsnr,r1   *$-23h
01f3 7a 5b 			bsnr,r2   $-23h
01f5 7a db 			bsnr,r2   *$-23h
01f7 7b 5b 			bsnr,r3   $-23h
01f9 7b db 			bsnr,r3   *$-23h
01fb 3f 92 34 			bsta,always   *absolute
01fe 3f 12 34 			bsta,always   absolute
0201 3c 92 34 			bsta,eq   *absolute
0204 3c 12 34 			bsta,eq   absolute
0207 3d 92 34 			bsta,gt   *absolute
020a 3d 12 34 			bsta,gt   absolute
020d 3e 92 34 			bsta,lt   *absolute
0210 3e 12 34 			bsta,lt   absolute
0213 3b 5b 			bstr,always   $-23h
0215 3b db 			bstr,always   *$-23h
0217 38 5b 			bstr,eq   $-23h
0219 38 db 			bstr,eq   *$-23h
021b 39 5b 			bstr,gt   $-23h
021d 39 db 			bstr,gt   *$-23h
021f 3a 5b 			bstr,lt   $-23h
0221 3a db 			bstr,lt   *$-23h
0223 bf 92 34 			bsxa  *absolute,r3
0226 bf 12 34 			bsxa  absolute,r3
0229 9f 92 34 			bxa  *absolute,r3
022c 9f 12 34 			bxa  absolute,r3
022f ec 92 34 			coma,r0   *absolute
0232 ec 12 34 			coma,r0   absolute
0235 ec 52 34 			coma,r0   absolute,r0,-
0238 ed 52 34 			coma,r0   absolute,r1,-
023b ee 52 34 			coma,r0   absolute,r2,-
023e ef 52 34 			coma,r0   absolute,r3,-
0241 ec f2 34 			coma,r0  *absolute,r0
0244 ec b2 34 			coma,r0  *absolute,r0,+
0247 ec d2 34 			coma,r0  *absolute,r0,-
024a ed f2 34 			coma,r0  *absolute,r1
024d ed b2 34 			coma,r0  *absolute,r1,+
0250 ed d2 34 			coma,r0  *absolute,r1,-
0253 ee f2 34 			coma,r0  *absolute,r2
0256 ee b2 34 			coma,r0  *absolute,r2,+
0259 ee d2 34 			coma,r0  *absolute,r2,-
025c ef f2 34 			coma,r0  *absolute,r3
025f ef b2 34 			coma,r0  *absolute,r3,+
0262 ef d2 34 			coma,r0  *absolute,r3,-
0265 ec 72 34 			coma,r0  absolute,r0
0268 ec 32 34 			coma,r0  absolute,r0,+
026b ed 72 34 			coma,r0  absolute,r1
026e ed 32 34 			coma,r0  absolute,r1,+
0271 ee 72 34 			coma,r0  absolute,r2
0274 ee 32 34 			coma,r0  absolute,r2,+
0277 ef 72 34 			coma,r0  absolute,r3
027a ef 32 34 			coma,r0  absolute,r3,+
027d ed 92 34 			coma,r1   *absolute
0280 ed 12 34 			coma,r1   absolute
0283 ee 92 34 			coma,r2   *absolute
0286 ee 12 34 			coma,r2   absolute
0289 ef 92 34 			coma,r3   *absolute
028c ef 12 34 			coma,r3   absolute
028f e4 a5 			comi,r0   immed
0291 e5 a5 			comi,r1   immed
0293 e6 a5 			comi,r2   immed
0295 e7 a5 			comi,r3   immed
0297 e8 5b 			comr,r0   $-23h
0299 e8 db 			comr,r0   *$-23h
029b e9 5b 			comr,r1   $-23h
029d e9 db 			comr,r1   *$-23h
029f ea 5b 			comr,r2   $-23h
02a1 ea db 			comr,r2   *$-23h
02a3 eb 5b 			comr,r3   $-23h
02a5 eb db 			comr,r3   *$-23h
02a7 e0 			comz  r0
02a8 e1 			comz  r1
02a9 e2 			comz  r2
02aa e3 			comz  r3
02ab 75 a5 			cpsl  immed
02ad 74 a5 			cpsu  immed
02af 94 			dar,r0
02b0 95 			dar,r1
02b1 96 			dar,r2
02b2 97 			dar,r3
02b3 2c 92 34 			eora,r0   *absolute
02b6 2c 12 34 			eora,r0   absolute
02b9 2c 52 34 			eora,r0   absolute,r0,-
02bc 2d 52 34 			eora,r0   absolute,r1,-
02bf 2e 52 34 			eora,r0   absolute,r2,-
02c2 2f 52 34 			eora,r0   absolute,r3,-
02c5 2c f2 34 			eora,r0  *absolute,r0
02c8 2c b2 34 			eora,r0  *absolute,r0,+
02cb 2c d2 34 			eora,r0  *absolute,r0,-
02ce 2d f2 34 			eora,r0  *absolute,r1
02d1 2d b2 34 			eora,r0  *absolute,r1,+
02d4 2d d2 34 			eora,r0  *absolute,r1,-
02d7 2e f2 34 			eora,r0  *absolute,r2
02da 2e b2 34 			eora,r0  *absolute,r2,+
02dd 2e d2 34 			eora,r0  *absolute,r2,-
02e0 2f f2 34 			eora,r0  *absolute,r3
02e3 2f b2 34 			eora,r0  *absolute,r3,+
02e6 2f d2 34 			eora,r0  *absolute,r3,-
02e9 2c 72 34 			eora,r0  absolute,r0
02ec 2c 32 34 			eora,r0  absolute,r0,+
02ef 2d 72 34 			eora,r0  absolute,r1
02f2 2d 32 34 			eora,r0  absolute,r1,+
02f5 2e 72 34 			eora,r0  absolute,r2
02f8 2e 32 34 			eora,r0  absolute,r2,+
02fb 2f 72 34 			eora,r0  absolute,r3
02fe 2f 32 34 			eora,r0  absolute,r3,+
0301 2d 92 34 			eora,r1   *absolute
0304 2d 12 34 			eora,r1   absolute
0307 2e 92 34 			eora,r2   *absolute
030a 2e 12 34 			eora,r2   absolute
030d 2f 92 34 			eora,r3   *absolute
0310 2f 12 34 			eora,r3   absolute
0313 24 a5 			eori,r0   immed
0315 25 a5 			eori,r1   immed
0317 26 a5 			eori,r2   immed
0319 27 a5 			eori,r3   immed
031b 28 5b 			eorr,r0   $-23h
031d 28 db 			eorr,r0   *$-23h
031f 29 5b 			eorr,r1   $-23h
0321 29 db 			eorr,r1   *$-23h
0323 2a 5b 			eorr,r2   $-23h
0325 2a db 			eorr,r2   *$-23h
0327 2b 5b 			eorr,r3   $-23h
0329 2b db 			eorr,r3   *$-23h
032b 20 			eorz  r0
032c 21 			eorz  r1
032d 22 			eorz  r2
032e 23 			eorz  r3
032f 40 			halt
0330 6c 92 34 			iora,r0   *absolute
0333 6c 12 34 			iora,r0   absolute
0336 6c 52 34 			iora,r0   absolute,r0,-
0339 6d 52 34 			iora,r0   absolute,r1,-
033c 6e 52 34 			iora,r0   absolute,r2,-
033f 6f 52 34 			iora,r0   absolute,r3,-
0342 6c f2 34 			iora,r0  *absolute,r0
0345 6c b2 34 			iora,r0  *absolute,r0,+
0348 6c d2 34 			iora,r0  *absolute,r0,-
034b 6d f2 34 			iora,r0  *absolute,r1
034e 6d b2 34 			iora,r0  *absolute,r1,+
0351 6d d2 34 			iora,r0  *absolute,r1,-
0354 6e f2 34 			iora,r0  *absolute,r2
0357 6e b2 34 			iora,r0  *absolute,r2,+
035a 6e d2 34 			iora,r0  *absolute,r2,-
035d 6f f2 34 			iora,r0  *absolute,r3
0360 6f b2 34 			iora,r0  *absolute,r3,+
0363 6f d2 34 			iora,r0  *absolute,r3,-
0366 6c 72 34 			iora,r0  absolute,r0
0369 6c 32 34 			iora,r0  absolute,r0,+
036c 6d 72 34 			iora,r0  absolute,r1
036f 6d 32 34 			iora,r0  absolute,r1,+
0372 6e 72 34 			iora,r0  absolute,r2
0375 6e 32 34 			iora,r0  absolute,r2,+
0378 6f 72 34 			iora,r0  absolute,r3
037b 6f 32 34 			iora,r0  absolute,r3,+
037e 6d 92 34 			iora,r1   *absolute
0381 6d 12 34 			iora,r1   absolute
0384 6e 92 34 			iora,r2   *absolute
0387 6e 12 34 			iora,r2   absolute
038a 6f 92 34 			iora,r3   *absolute
038d 6f 12 34 			iora,r3   absolute
0390 64 a5 			iori,r0   immed
0392 65 a5 			iori,r1   immed
0394 66 a5 			iori,r2   immed
0396 67 a5 			iori,r3   immed
0398 68 5b 			iorr,r0   $-23h
039a 68 db 			iorr,r0   *$-23h
039c 69 5b 			iorr,r1   $-23h
039e 69 db 			iorr,r1   *$-23h
03a0 6a 5b 			iorr,r2   $-23h
03a2 6a db 			iorr,r2   *$-23h
03a4 6b 5b 			iorr,r3   $-23h
03a6 6b db 			iorr,r3   *$-23h
03a8 60 			iorz  r0
03a9 61 			iorz  r1
03aa 62 			iorz  r2
03ab 63 			iorz  r3
03ac 0c 92 34 			loda,r0   *absolute
03af 0c 12 34 			loda,r0   absolute
03b2 0c 52 34 			loda,r0   absolute,r0,-
03b5 0d 52 34 			loda,r0   absolute,r1,-
03b8 0e 52 34 			loda,r0   absolute,r2,-
03bb 0f 52 34 			loda,r0   absolute,r3,-
03be 0c f2 34 			loda,r0  *absolute,r0
03c1 0c b2 34 			loda,r0  *absolute,r0,+
03c4 0c d2 34 			loda,r0  *absolute,r0,-
03c7 0d f2 34 			loda,r0  *absolute,r1
03ca 0d b2 34 			loda,r0  *absolute,r1,+
03cd 0d d2 34 			loda,r0  *absolute,r1,-
03d0 0e f2 34 			loda,r0  *absolute,r2
03d3 0e b2 34 			loda,r0  *absolute,r2,+
03d6 0e d2 34 			loda,r0  *absolute,r2,-
03d9 0f f2 34 			loda,r0  *absolute,r3
03dc 0f b2 34 			loda,r0  *absolute,r3,+
03df 0f d2 34 			loda,r0  *absolute,r3,-
03e2 0c 72 34 			loda,r0  absolute,r0
03e5 0c 32 34 			loda,r0  absolute,r0,+
03e8 0d 72 34 			loda,r0  absolute,r1
03eb 0d 32 34 			loda,r0  absolute,r1,+
03ee 0e 72 34 			loda,r0  absolute,r2
03f1 0e 32 34 			loda,r0  absolute,r2,+
03f4 0f 72 34 			loda,r0  absolute,r3
03f7 0f 32 34 			loda,r0  absolute,r3,+
03fa 0d 92 34 			loda,r1   *absolute
03fd 0d 12 34 			loda,r1   absolute
0400 0e 92 34 			loda,r2   *absolute
0403 0e 12 34 			loda,r2   absolute
0406 0f 92 34 			loda,r3   *absolute
0409 0f 12 34 			loda,r3   absolute
040c 04 a5 			lodi,r0   immed
040e 05 a5 			lodi,r1   immed
0410 06 a5 			lodi,r2   immed
0412 07 a5 			lodi,r3   immed
0414 08 5b 			lodr,r0   $-23h
0416 08 db 			lodr,r0   *$-23h
0418 09 5b 			lodr,r1   $-23h
041a 09 db 			lodr,r1   *$-23h
041c 0a 5b 			lodr,r2   $-23h
041e 0a db 			lodr,r2   *$-23h
0420 0b 5b 			lodr,r3   $-23h
0422 0b db 			lodr,r3   *$-23h
0424 60 			lodz  r0
0425 01 			lodz  r1
0426 02 			lodz  r2
0427 03 			lodz  r3
0428 93 			lpsl
0429 92 			lpsu
042a c0 			nop
042b 77 a5 			ppsl  immed
042d 76 a5 			ppsu  immed
042f 30 			redc,r0
0430 31 			redc,r1
0431 32 			redc,r2
0432 33 			redc,r3
0433 70 			redd,r0
0434 71 			redd,r1
0435 72 			redd,r2
0436 73 			redd,r3
0437 54 a5 			rede,r0   immed
0439 55 a5 			rede,r1   immed
043b 56 a5 			rede,r2   immed
043d 57 a5 			rede,r3   immed
043f 17 			retc,always
0440 14 			retc,eq
0441 15 			retc,gt
0442 16 			retc,lt
0443 37 			rete,always
0444 34 			rete,eq
0445 35 			rete,gt
0446 36 			rete,lt
0447 d0 			rrl,r0
0448 d1 			rrl,r1
0449 d2 			rrl,r2
044a d3 			rrl,r3
044b 50 			rrr,r0
044c 51 			rrr,r1
044d 52 			rrr,r2
044e 53 			rrr,r3
044f 13 			spsl
0450 12 			spsu
0451 cc 92 34 			stra,r0   *absolute
0454 cc 12 34 			stra,r0   absolute
0457 cc 52 34 			stra,r0   absolute,r0,-
045a cd 52 34 			stra,r0   absolute,r1,-
045d ce 52 34 			stra,r0   absolute,r2,-
0460 cf 52 34 			stra,r0   absolute,r3,-
0463 cc f2 34 			stra,r0  *absolute,r0
0466 cc b2 34 			stra,r0  *absolute,r0,+
0469 cc d2 34 			stra,r0  *absolute,r0,-
046c cd f2 34 			stra,r0  *absolute,r1
046f cd b2 34 			stra,r0  *absolute,r1,+
0472 cd d2 34 			stra,r0  *absolute,r1,-
0475 ce f2 34 			stra,r0  *absolute,r2
0478 ce b2 34 			stra,r0  *absolute,r2,+
047b ce d2 34 			stra,r0  *absolute,r2,-
047e cf f2 34 			stra,r0  *absolute,r3
0481 cf b2 34 			stra,r0  *absolute,r3,+
0484 cf d2 34 			stra,r0  *absolute,r3,-
0487 cc 72 34 			stra,r0  absolute,r0
048a cc 32 34 			stra,r0  absolute,r0,+
048d cd 72 34 			stra,r0  absolute,r1
0490 cd 32 34 			stra,r0  absolute,r1,+
0493 ce 72 34 			stra,r0  absolute,r2
0496 ce 32 34 			stra,r0  absolute,r2,+
0499 cf 72 34 			stra,r0  absolute,r3
049c cf 32 34 			stra,r0  absolute,r3,+
049f cd 92 34 			stra,r1   *absolute
04a2 cd 12 34 			stra,r1   absolute
04a5 ce 92 34 			stra,r2   *absolute
04a8 ce 12 34 			stra,r2   absolute
04ab cf 92 34 			stra,r3   *absolute
04ae cf 12 34 			stra,r3   absolute
04b1 c8 5b 			strr,r0   $-23h
04b3 c8 db 			strr,r0   *$-23h
04b5 c9 5b 			strr,r1   $-23h
04b7 c9 db 			strr,r1   *$-23h
04b9 ca 5b 			strr,r2   $-23h
04bb ca db 			strr,r2   *$-23h
04bd cb 5b 			strr,r3   $-23h
04bf cb db 			strr,r3   *$-23h
04c1 c1 			strz  r1
04c2 c2 			strz  r2
04c3 c3 			strz  r3
04c4 ac 92 34 			suba,r0   *absolute
04c7 ac 12 34 			suba,r0   absolute
04ca ac 52 34 			suba,r0   absolute,r0,-
04cd ad 52 34 			suba,r0   absolute,r1,-
04d0 ae 52 34 			suba,r0   absolute,r2,-
04d3 af 52 34 			suba,r0   absolute,r3,-
04d6 ac f2 34 			suba,r0  *absolute,r0
04d9 ac b2 34 			suba,r0  *absolute,r0,+
04dc ac d2 34 			suba,r0  *absolute,r0,-
04df ad f2 34 			suba,r0  *absolute,r1
04e2 ad b2 34 			suba,r0  *absolute,r1,+
04e5 ad d2 34 			suba,r0  *absolute,r1,-
04e8 ae f2 34 			suba,r0  *absolute,r2
04eb ae b2 34 			suba,r0  *absolute,r2,+
04ee ae d2 34 			suba,r0  *absolute,r2,-
04f1 af f2 34 			suba,r0  *absolute,r3
04f4 af b2 34 			suba,r0  *absolute,r3,+
04f7 af d2 34 			suba,r0  *absolute,r3,-
04fa ac 72 34 			suba,r0  absolute,r0
04fd ac 32 34 			suba,r0  absolute,r0,+
0500 ad 72 34 			suba,r0  absolute,r1
0503 ad 32 34 			suba,r0  absolute,r1,+
0506 ae 72 34 			suba,r0  absolute,r2
0509 ae 32 34 			suba,r0  absolute,r2,+
050c af 72 34 			suba,r0  absolute,r3
050f af 32 34 			suba,r0  absolute,r3,+
0512 ad 92 34 			suba,r1   *absolute
0515 ad 12 34 			suba,r1   absolute
0518 ae 92 34 			suba,r2   *absolute
051b ae 12 34 			suba,r2   absolute
051e af 92 34 			suba,r3   *absolute
0521 af 12 34 			suba,r3   absolute
0524 a4 a5 			subi,r0   immed
0526 a5 a5 			subi,r1   immed
0528 a6 a5 			subi,r2   immed
052a a7 a5 			subi,r3   immed
052c a8 5b 			subr,r0   $-23h
052e a8 db 			subr,r0   *$-23h
0530 a9 5b 			subr,r1   $-23h
0532 a9 db 			subr,r1   *$-23h
0534 aa 5b 			subr,r2   $-23h
0536 aa db 			subr,r2   *$-23h
0538 ab 5b 			subr,r3   $-23h
053a ab db 			subr,r3   *$-23h
053c a0 			subz  r0
053d a1 			subz  r1
053e a2 			subz  r2
053f a3 			subz  r3
0540 f4 a5 			tmi,r0   immed
0542 f5 a5 			tmi,r1   immed
0544 f6 a5 			tmi,r2   immed
0546 f7 a5 			tmi,r3   immed
0548 b5 a5 			tpsl  immed
054a b4 a5 			tpsu  immed
054c b0 			wrtc,r0
054d b1 			wrtc,r1
054e b2 			wrtc,r2
054f b3 			wrtc,r3
0550 f0 			wrtd,r0
0551 f1 			wrtd,r1
0552 f2 			wrtd,r2
0553 f3 			wrtd,r3
0554 d4 a5 			wrte,r0   immed
0556 d5 a5 			wrte,r1   immed
0558 d6 a5 			wrte,r2   immed
055a d7 a5 			wrte,r3   immed
055c 9b a3 			zbrr  *23h
055e 9b 23 			zbrr  23h
0560 bb a3 			zbsr  *23h
0562 bb 23 			zbsr  23h
 Error Summary: 1 warning(s).
