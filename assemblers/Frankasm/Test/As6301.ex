00000040 m                00000099 d                00001234 e                
00000066 o                00000032 i                
 0x99                   	org $99
 0x40                   m	equ	$40
0099 01 23 		d	fdb	123H
 0x1234                 	org	$1234
1234 02 a6 		e	fdb	678
 0x2000                 	org	$2000
 0x66                   o	set	$66
 0x32                   i	equ	32H
2000 1b 			aba
2001 3a 			abx
2002 89 32 			adc a #i
2004 99 99 			adc a d
2006 b9 12 34 			adc a e
2009 a9 66 			adc a o,x
200b c9 32 			adc b #i
200d d9 99 			adc b d
200f f9 12 34 			adc b e
2012 e9 66 			adc b o,x
2014 89 32 			adca #i
2016 99 99 			adca d
2018 b9 12 34 			adca e
201b a9 66 			adca o,x
201d c9 32 			adcb #i
201f d9 99 			adcb d
2021 f9 12 34 			adcb e
2024 e9 66 			adcb o,x
2026 8b 32 			add a #i
2028 9b 99 			add a d
202a bb 12 34 			add a e
202d ab 66 			add a o,x
202f cb 32 			add b #i
2031 db 99 			add b d
2033 fb 12 34 			add b e
2036 eb 66 			add b o,x
2038 8b 32 			adda #i
203a 9b 99 			adda d
203c bb 12 34 			adda e
203f ab 66 			adda o,x
2041 cb 32 			addb #i
2043 db 99 			addb d
2045 fb 12 34 			addb e
2048 eb 66 			addb o,x
204a c3 00 32 			addd #i
204d d3 99 			addd d
204f f3 12 34 			addd e
2052 e3 66 			addd o,x
2054 71 32 99 			aim #i,d
2057 61 32 66 			aim #i,o,x
205a 84 32 			and a #i
205c 94 99 			and a d
205e b4 12 34 			and a e
2061 a4 66 			and a o,x
2063 c4 32 			and b #i
2065 d4 99 			and b d
2067 f4 12 34 			and b e
206a e4 66 			and b o,x
206c 84 32 			anda #i
206e 94 99 			anda d
2070 b4 12 34 			anda e
2073 a4 66 			anda o,x
2075 c4 32 			andb #i
2077 d4 99 			andb d
2079 f4 12 34 			andb e
207c e4 66 			andb o,x
207e 48 			asl a
207f 58 			asl b
2080 78 12 34 			asl e
2083 68 66 			asl o,x
2085 48 			asla
2086 58 			aslb
2087 05 			asld
2088 47 			asr a
2089 57 			asr b
208a 77 12 34 			asr e
208d 67 66 			asr o,x
208f 47 			asra
2090 57 			asrb
2091 24 fa 			bcc *-4
2093 71 fe 99 			bclr 0,d
2096 71 fd 99 			bclr 1,d
2099 71 fb 99 			bclr 2,d
209c 71 f7 99 			bclr 3,d
209f 71 ef 99 			bclr 4,d
20a2 71 df 99 			bclr 5,d
20a5 71 bf 99 			bclr 6,d
20a8 71 7f 99 			bclr 7,d
20ab 61 fe 66 			bclr 0,o,x
20ae 61 fd 66 			bclr 1,o,x
20b1 61 fb 66 			bclr 2,o,x
20b4 61 f7 66 			bclr 3,o,x
20b7 61 ef 66 			bclr 4,o,x
20ba 61 df 66 			bclr 5,o,x
20bd 61 bf 66 			bclr 6,o,x
20c0 61 7f 66 			bclr 7,o,x
20c3 25 fa 			bcs *-4
20c5 27 fa 			beq *-4
20c7 2c fa 			bge *-4
20c9 2e fa 			bgt *-4
20cb 22 fa 			bhi *-4
20cd 24 fa 			bhs *-4
20cf 85 32 			bit a #i
20d1 95 99 			bit a d
20d3 b5 12 34 			bit a e
20d6 a5 66 			bit a o,x
20d8 c5 32 			bit b #i
20da d5 99 			bit b d
20dc f5 12 34 			bit b e
20df e5 66 			bit b o,x
20e1 85 32 			bita #i
20e3 95 99 			bita d
20e5 b5 12 34 			bita e
20e8 a5 66 			bita o,x
20ea c5 32 			bitb #i
20ec d5 99 			bitb d
20ee f5 12 34 			bitb e
20f1 e5 66 			bitb o,x
20f3 2f fa 			ble *-4
20f5 25 fa 			blo *-4
20f7 23 fa 			bls *-4
20f9 2d fa 			blt *-4
20fb 2b fa 			bmi *-4
20fd 26 fa 			bne *-4
20ff 2a fa 			bpl *-4
2101 20 fa 			bra *-4
2103 21 fa 			brn *-4
2105 72 01 99 			bset 0,d
2108 72 02 99 			bset 1,d
210b 72 04 99 			bset 2,d
210e 72 08 99 			bset 3,d
2111 72 10 99 			bset 4,d
2114 72 20 99 			bset 5,d
2117 72 40 99 			bset 6,d
211a 72 80 99 			bset 7,d
211d 62 01 66 			bset 0,o,x
2120 62 02 66 			bset 1,o,x
2123 62 04 66 			bset 2,o,x
2126 62 08 66 			bset 3,o,x
2129 62 10 66 			bset 4,o,x
212c 62 20 66 			bset 5,o,x
212f 62 40 66 			bset 6,o,x
2132 62 80 66 			bset 7,o,x
2135 8d fa 			bsr *-4
2137 75 01 99 			btgl 0,d
213a 75 02 99 			btgl 1,d
213d 75 04 99 			btgl 2,d
2140 75 08 99 			btgl 3,d
2143 75 10 99 			btgl 4,d
2146 75 20 99 			btgl 5,d
2149 75 40 99 			btgl 6,d
214c 75 80 99 			btgl 7,d
214f 65 01 66 			btgl 0,o,x
2152 65 02 66 			btgl 1,o,x
2155 65 04 66 			btgl 2,o,x
2158 65 08 66 			btgl 3,o,x
215b 65 10 66 			btgl 4,o,x
215e 65 20 66 			btgl 5,o,x
2161 65 40 66 			btgl 6,o,x
2164 65 80 66 			btgl 7,o,x
2167 7b 01 99 			btst 0,d
216a 7b 02 99 			btst 1,d
216d 7b 04 99 			btst 2,d
2170 7b 08 99 			btst 3,d
2173 7b 10 99 			btst 4,d
2176 7b 20 99 			btst 5,d
2179 7b 40 99 			btst 6,d
217c 7b 80 99 			btst 7,d
217f 6b 01 66 			btst 0,o,x
2182 6b 02 66 			btst 1,o,x
2185 6b 04 66 			btst 2,o,x
2188 6b 08 66 			btst 3,o,x
218b 6b 10 66 			btst 4,o,x
218e 6b 20 66 			btst 5,o,x
2191 6b 40 66 			btst 6,o,x
2194 6b 80 66 			btst 7,o,x
2197 28 fa 			bvc *-4
2199 29 fa 			bvs *-4
219b 11 			cba
219c 0c 			clc
219d 0e 			cli
219e 4f 			clr a
219f 5f 			clr b
21a0 7f 12 34 			clr e
21a3 6f 66 			clr o,x
21a5 4f 			clra
21a6 5f 			clrb
21a7 0a 			clv
21a8 81 32 			cmp a #i
21aa 91 99 			cmp a d
21ac b1 12 34 			cmp a e
21af a1 66 			cmp a o,x
21b1 c1 32 			cmp b #i
21b3 d1 99 			cmp b d
21b5 f1 12 34 			cmp b e
21b8 e1 66 			cmp b o,x
21ba 81 32 			cmpa #i
21bc 91 99 			cmpa d
21be b1 12 34 			cmpa e
21c1 a1 66 			cmpa o,x
21c3 c1 32 			cmpb #i
21c5 d1 99 			cmpb d
21c7 f1 12 34 			cmpb e
21ca e1 66 			cmpb o,x
21cc 43 			com a
21cd 53 			com b
21ce 73 12 34 			com e
21d1 63 66 			com o,x
21d3 43 			coma
21d4 53 			comb
21d5 8c 00 32 			cpx #i
21d8 9c 99 			cpx d
21da bc 12 34 			cpx e
21dd ac 66 			cpx o,x
21df 19 			daa
21e0 4a 			dec a
21e1 5a 			dec b
21e2 7a 12 34 			dec e
21e5 6a 66 			dec o,x
21e7 4a 			deca
21e8 5a 			decb
21e9 34 			des
21ea 09 			dex
21eb 75 32 99 			eim #i,d
21ee 65 32 66 			eim #i,o,x
21f1 88 32 			eor a #i
21f3 98 99 			eor a d
21f5 b8 12 34 			eor a e
21f8 a8 66 			eor a o,x
21fa c8 32 			eor b #i
21fc d8 99 			eor b d
21fe f8 12 34 			eor b e
2201 e8 66 			eor b o,x
2203 88 32 			eora #i
2205 98 99 			eora d
2207 b8 12 34 			eora e
220a a8 66 			eora o,x
220c c8 32 			eorb #i
220e d8 99 			eorb d
2210 f8 12 34 			eorb e
2213 e8 66 			eorb o,x
2215 4c 			inc a
2216 5c 			inc b
2217 7c 12 34 			inc e
221a 6c 66 			inc o,x
221c 4c 			inca
221d 5c 			incb
221e 31 			ins
221f 08 			inx
2220 7e 12 34 			jmp e
2223 6e 66 			jmp o,x
2225 9d 99 			jsr d
2227 bd 12 34 			jsr e
222a ad 66 			jsr o,x
222c 86 32 			lda a #i
222e 96 99 			lda a d
2230 b6 12 34 			lda a e
2233 a6 66 			lda a o,x
2235 c6 32 			lda b #i
2237 d6 99 			lda b d
2239 f6 12 34 			lda b e
223c e6 66 			lda b o,x
223e 86 32 			ldaa #i
2240 96 99 			ldaa d
2242 b6 12 34 			ldaa e
2245 a6 66 			ldaa o,x
2247 c6 32 			ldab #i
2249 d6 99 			ldab d
224b f6 12 34 			ldab e
224e e6 66 			ldab o,x
2250 cc 00 32 			ldd #i
2253 dc 99 			ldd d
2255 fc 12 34 			ldd e
2258 ec 66 			ldd o,x
225a 8e 00 32 			lds #i
225d 9e 99 			lds d
225f be 12 34 			lds e
2262 ae 66 			lds o,x
2264 ce 00 32 			ldx #i
2267 de 99 			ldx d
2269 fe 12 34 			ldx e
226c ee 66 			ldx o,x
226e 48 			lsl a
226f 58 			lsl b
2270 78 12 34 			lsl e
2273 68 66 			lsl o,x
2275 48 			lsla
2276 58 			lslb
2277 05 			lsld
2278 44 			lsr a
2279 54 			lsr b
227a 74 12 34 			lsr e
227d 64 66 			lsr o,x
227f 44 			lsra
2280 54 			lsrb
2281 04 			lsrd
2282 3d 			mul
2283 40 			neg a
2284 50 			neg b
2285 70 12 34 			neg e
2288 60 66 			neg o,x
228a 40 			nega
228b 50 			negb
228c 01 			nop
228d 72 32 99 			oim #i,d
2290 62 32 66 			oim #i,o,x
2293 8a 32 			ora a #i
2295 9a 99 			ora a d
2297 ba 12 34 			ora a e
229a aa 66 			ora a o,x
229c ca 32 			ora b #i
229e da 99 			ora b d
22a0 fa 12 34 			ora b e
22a3 ea 66 			ora b o,x
22a5 8a 32 			oraa #i
22a7 9a 99 			oraa d
22a9 ba 12 34 			oraa e
22ac aa 66 			oraa o,x
22ae ca 32 			orab #i
22b0 da 99 			orab d
22b2 fa 12 34 			orab e
22b5 ea 66 			orab o,x
22b7 36 			psh a
22b8 37 			psh b
22b9 3c 			psh x
22ba 36 			psha
22bb 37 			pshb
22bc 3c 			pshx
22bd 32 			pul a
22be 33 			pul b
22bf 38 			pul x
22c0 32 			pula
22c1 33 			pulb
22c2 38 			pulx
22c3 49 			rol a
22c4 59 			rol b
22c5 79 12 34 			rol e
22c8 69 66 			rol o,x
22ca 49 			rola
22cb 59 			rolb
22cc 46 			ror a
22cd 56 			ror b
22ce 76 12 34 			ror e
22d1 66 66 			ror o,x
22d3 46 			rora
22d4 56 			rorb
22d5 3b 			rti
22d6 39 			rts
22d7 10 			sba
22d8 82 32 			sbc a #i
22da 92 99 			sbc a d
22dc b2 12 34 			sbc a e
22df a2 66 			sbc a o,x
22e1 c2 32 			sbc b #i
22e3 d2 99 			sbc b d
22e5 f2 12 34 			sbc b e
22e8 e2 66 			sbc b o,x
22ea 82 32 			sbca #i
22ec 92 99 			sbca d
22ee b2 12 34 			sbca e
22f1 a2 66 			sbca o,x
22f3 c2 32 			sbcb #i
22f5 d2 99 			sbcb d
22f7 f2 12 34 			sbcb e
22fa e2 66 			sbcb o,x
22fc 0d 			sec
22fd 0f 			sei
22fe 0b 			sev
22ff 1a 			slp
2300 97 99 			sta a d
2302 b7 12 34 			sta a e
2305 a7 66 			sta a o,x
2307 d7 99 			sta b d
2309 f7 12 34 			sta b e
230c e7 66 			sta b o,x
230e 97 99 			staa d
2310 b7 12 34 			staa e
2313 a7 66 			staa o,x
2315 d7 99 			stab d
2317 f7 12 34 			stab e
231a e7 66 			stab o,x
231c dd 99 			std d
231e fd 12 34 			std e
2321 ed 66 			std o,x
2323 9f 99 			sts d
2325 bf 12 34 			sts e
2328 af 66 			sts o,x
232a df 99 			stx d
232c ff 12 34 			stx e
232f ef 66 			stx o,x
2331 80 32 			sub a #i
2333 90 99 			sub a d
2335 b0 12 34 			sub a e
2338 a0 66 			sub a o,x
233a c0 32 			sub b #i
233c d0 99 			sub b d
233e f0 12 34 			sub b e
2341 e0 66 			sub b o,x
2343 80 32 			suba #i
2345 90 99 			suba d
2347 b0 12 34 			suba e
234a a0 66 			suba o,x
234c c0 32 			subb #i
234e d0 99 			subb d
2350 f0 12 34 			subb e
2353 e0 66 			subb o,x
2355 83 00 32 			subd #i
2358 93 99 			subd d
235a b3 12 34 			subd e
235d a3 66 			subd o,x
235f 3f 			swi
2360 16 			tab
2361 06 			tap
2362 17 			tba
2363 7b 32 99 			tim #i,d
2366 6b 32 66 			tim #i,o,x
2369 07 			tpa
236a 4d 			tst a
236b 5d 			tst b
236c 7d 12 34 			tst e
236f 6d 66 			tst o,x
2371 4d 			tsta
2372 5d 			tstb
2373 30 			tsx
2374 35 			txs
2375 3e 			wai
2376 18 			xgdx
				end
