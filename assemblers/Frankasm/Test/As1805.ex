00002300 expr             00000055 immed            
 0x2300                 	org 2300h
 0x2300                 expr	equ *
 0x55                   immed	equ $55
2300 74 			 adc
2301 7c 55 			 adci # immed
2303 f4 			 add
2304 fc 55 			 adi # immed
2306 f2 			 and
2307 fa 55 			 ani # immed
2309 34 00 			 b1 expr
230b 35 00 			 b2 expr
230d 36 00 			 b3 expr
230f 37 00 			 b4 expr
2311 68 3e 00 			 bci expr
2314 33 00 			 bdf expr
2316 33 00 			 bge expr
2318 3b 00 			 bl expr
231a 3b 00 			 bm expr
231c 3c 00 			 bn1 expr
231e 3d 00 			 bn2 expr
2320 3e 00 			 bn3 expr
2322 3f 00 			 bn4 expr
2324 3b 00 			 bnf expr
2326 39 00 			 bnq expr
2328 3a 00 			 bnz expr
232a 33 00 			 bpz expr
232c 31 00 			 bq expr
232e 30 00 			 br expr
2330 68 3f 00 			 bxi expr
2333 32 00 			 bz expr
2335 68 0d 			 cid
2337 68 0c 			 cie
2339 68 7c 55 			 daci # immed
233c 68 74 			 dadc
233e 68 f4 			 dadd
2340 68 fc 55 			 dadi # immed
2343 68 20 23 00 		 dbnz 0 , expr
2347 68 21 23 00 		 dbnz 1 , expr
234b 68 2a 23 00 		 dbnz 10 , expr
234f 68 2b 23 00 		 dbnz 11 , expr
2353 68 2c 23 00 		 dbnz 12 , expr
2357 68 2d 23 00 		 dbnz 13 , expr
235b 68 2e 23 00 		 dbnz 14 , expr
235f 68 2f 23 00 		 dbnz 15 , expr
2363 68 22 23 00 		 dbnz 2 , expr
2367 68 23 23 00 		 dbnz 3 , expr
236b 68 24 23 00 		 dbnz 4 , expr
236f 68 25 23 00 		 dbnz 5 , expr
2373 68 26 23 00 		 dbnz 6 , expr
2377 68 27 23 00 		 dbnz 7 , expr
237b 68 28 23 00 		 dbnz 8 , expr
237f 68 29 23 00 		 dbnz 9 , expr
2383 20 			 dec 0
2384 21 			 dec 1
2385 2a 			 dec 10
2386 2b 			 dec 11
2387 2c 			 dec 12
2388 2d 			 dec 13
2389 2e 			 dec 14
238a 2f 			 dec 15
238b 22 			 dec 2
238c 23 			 dec 3
238d 24 			 dec 4
238e 25 			 dec 5
238f 26 			 dec 6
2390 27 			 dec 7
2391 28 			 dec 8
2392 29 			 dec 9
2393 71 			 dis
2394 68 76 			 dsav
2396 68 7f 55 			 dsbi # immed
2399 68 f7 			 dsm
239b 68 77 			 dsmb
239d 68 ff 55 			 dsmi # immed
23a0 68 01 			 dtc
23a2 68 09 			 etq
23a4 68 08 			 gec
23a6 90 			 ghi 0
23a7 91 			 ghi 1
23a8 9a 			 ghi 10
23a9 9b 			 ghi 11
23aa 9c 			 ghi 12
23ab 9d 			 ghi 13
23ac 9e 			 ghi 14
23ad 9f 			 ghi 15
23ae 92 			 ghi 2
23af 93 			 ghi 3
23b0 94 			 ghi 4
23b1 95 			 ghi 5
23b2 96 			 ghi 6
23b3 97 			 ghi 7
23b4 98 			 ghi 8
23b5 99 			 ghi 9
23b6 80 			 glo 0
23b7 81 			 glo 1
23b8 8a 			 glo 10
23b9 8b 			 glo 11
23ba 8c 			 glo 12
23bb 8d 			 glo 13
23bc 8e 			 glo 14
23bd 8f 			 glo 15
23be 82 			 glo 2
23bf 83 			 glo 3
23c0 84 			 glo 4
23c1 85 			 glo 5
23c2 86 			 glo 6
23c3 87 			 glo 7
23c4 88 			 glo 8
23c5 89 			 glo 9
23c6 00 			 idl
23c7 10 			 inc 0
23c8 11 			 inc 1
23c9 1a 			 inc 10
23ca 1b 			 inc 11
23cb 1c 			 inc 12
23cc 1d 			 inc 13
23cd 1e 			 inc 14
23ce 1f 			 inc 15
23cf 12 			 inc 2
23d0 13 			 inc 3
23d1 14 			 inc 4
23d2 15 			 inc 5
23d3 16 			 inc 6
23d4 17 			 inc 7
23d5 18 			 inc 8
23d6 19 			 inc 9
23d7 69 			 inp 1
23d8 6a 			 inp 2
23d9 6b 			 inp 3
23da 6c 			 inp 4
23db 6d 			 inp 5
23dc 6e 			 inp 6
23dd 6f 			 inp 7
23de 60 			 irx
23df c3 23 00 			 lbdf expr
23e2 cb 23 00 			 lbnf expr
23e5 c9 23 00 			 lbnq expr
23e8 ca 23 00 			 lbnz expr
23eb c1 23 00 			 lbq expr
23ee c0 23 00 			 lbr expr
23f1 c2 23 00 			 lbz expr
23f4 40 			 lda 0
23f5 41 			 lda 1
23f6 4a 			 lda 10
23f7 4b 			 lda 11
23f8 4c 			 lda 12
23f9 4d 			 lda 13
23fa 4e 			 lda 14
23fb 4f 			 lda 15
23fc 42 			 lda 2
23fd 43 			 lda 3
23fe 44 			 lda 4
23ff 45 			 lda 5
2400 46 			 lda 6
2401 47 			 lda 7
2402 48 			 lda 8
2403 49 			 lda 9
2404 68 06 			 ldc
2406 f8 55 			 ldi # immed
2408 01 			 ldn 1
2409 0a 			 ldn 10
240a 0b 			 ldn 11
240b 0c 			 ldn 12
240c 0d 			 ldn 13
240d 0e 			 ldn 14
240e 0f 			 ldn 15
240f 02 			 ldn 2
2410 03 			 ldn 3
2411 04 			 ldn 4
2412 05 			 ldn 5
2413 06 			 ldn 6
2414 07 			 ldn 7
2415 08 			 ldn 8
2416 09 			 ldn 9
2417 f0 			 ldx
2418 72 			 ldxa
2419 cf 			 lsdf
241a cc 			 lsie
241b c8 			 lskp
241c c7 			 lsnf
241d c5 			 lsnq
241e c6 			 lsnz
241f cd 			 lsq
2420 ce 			 lsz
2421 79 			 mark
2422 38 25 			 nbr *+3
2424 c8 23 00 			 nlbr expr
2427 c4 			 nop
2428 f1 			 or
2429 f9 55 			 ori # immed
242b 61 			 out 1
242c 62 			 out 2
242d 63 			 out 3
242e 64 			 out 4
242f 65 			 out 5
2430 66 			 out 6
2431 67 			 out 7
2432 b0 			 phi 0
2433 b1 			 phi 1
2434 ba 			 phi 10
2435 bb 			 phi 11
2436 bc 			 phi 12
2437 bd 			 phi 13
2438 be 			 phi 14
2439 bf 			 phi 15
243a b2 			 phi 2
243b b3 			 phi 3
243c b4 			 phi 4
243d b5 			 phi 5
243e b6 			 phi 6
243f b7 			 phi 7
2440 b8 			 phi 8
2441 b9 			 phi 9
2442 a0 			 plo 0
2443 a1 			 plo 1
2444 aa 			 plo 10
2445 ab 			 plo 11
2446 ac 			 plo 12
2447 ad 			 plo 13
2448 ae 			 plo 14
2449 af 			 plo 15
244a a2 			 plo 2
244b a3 			 plo 3
244c a4 			 plo 4
244d a5 			 plo 5
244e a6 			 plo 6
244f a7 			 plo 7
2450 a8 			 plo 8
2451 a9 			 plo 9
2452 7a 			 req
2453 70 			 ret
2454 68 c0 23 00 		 rldi 0 , # expr
2458 68 c1 23 00 		 rldi 1 , # expr
245c 68 ca 23 00 		 rldi 10 , # expr
2460 68 cb 23 00 		 rldi 11 , # expr
2464 68 cc 23 00 		 rldi 12 , # expr
2468 68 cd 23 00 		 rldi 13 , # expr
246c 68 ce 23 00 		 rldi 14 , # expr
2470 68 cf 23 00 		 rldi 15 , # expr
2474 68 c2 23 00 		 rldi 2 , # expr
2478 68 c3 23 00 		 rldi 3 , # expr
247c 68 c4 23 00 		 rldi 4 , # expr
2480 68 c5 23 00 		 rldi 5 , # expr
2484 68 c6 23 00 		 rldi 6 , # expr
2488 68 c7 23 00 		 rldi 7 , # expr
248c 68 c8 23 00 		 rldi 8 , # expr
2490 68 c9 23 00 		 rldi 9 , # expr
2494 68 60 			 rlxa 0
2496 68 61 			 rlxa 1
2498 68 6a 			 rlxa 10
249a 68 6b 			 rlxa 11
249c 68 6c 			 rlxa 12
249e 68 6d 			 rlxa 13
24a0 68 6e 			 rlxa 14
24a2 68 6f 			 rlxa 15
24a4 68 62 			 rlxa 2
24a6 68 63 			 rlxa 3
24a8 68 64 			 rlxa 4
24aa 68 65 			 rlxa 5
24ac 68 66 			 rlxa 6
24ae 68 67 			 rlxa 7
24b0 68 68 			 rlxa 8
24b2 68 69 			 rlxa 9
24b4 68 b0 			 rnx 0
24b6 68 b1 			 rnx 1
24b8 68 ba 			 rnx 10
24ba 68 bb 			 rnx 11
24bc 68 bc 			 rnx 12
24be 68 bd 			 rnx 13
24c0 68 be 			 rnx 14
24c2 68 bf 			 rnx 15
24c4 68 b2 			 rnx 2
24c6 68 b3 			 rnx 3
24c8 68 b4 			 rnx 4
24ca 68 b5 			 rnx 5
24cc 68 b6 			 rnx 6
24ce 68 b7 			 rnx 7
24d0 68 b8 			 rnx 8
24d2 68 b9 			 rnx 9
24d4 7e 			 rshl
24d5 76 			 rshr
24d6 68 a0 			 rsxd 0
24d8 68 a1 			 rsxd 1
24da 68 aa 			 rsxd 10
24dc 68 ab 			 rsxd 11
24de 68 ac 			 rsxd 12
24e0 68 ad 			 rsxd 13
24e2 68 ae 			 rsxd 14
24e4 68 af 			 rsxd 15
24e6 68 a2 			 rsxd 2
24e8 68 a3 			 rsxd 3
24ea 68 a4 			 rsxd 4
24ec 68 a5 			 rsxd 5
24ee 68 a6 			 rsxd 6
24f0 68 a7 			 rsxd 7
24f2 68 a8 			 rsxd 8
24f4 68 a9 			 rsxd 9
24f6 78 			 sav
24f7 68 80 23 00 		 scal 0 , expr
24fb 68 81 23 00 		 scal 1 , expr
24ff 68 8a 23 00 		 scal 10 , expr
2503 68 8b 23 00 		 scal 11 , expr
2507 68 8c 23 00 		 scal 12 , expr
250b 68 8d 23 00 		 scal 13 , expr
250f 68 8e 23 00 		 scal 14 , expr
2513 68 8f 23 00 		 scal 15 , expr
2517 68 82 23 00 		 scal 2 , expr
251b 68 83 23 00 		 scal 3 , expr
251f 68 84 23 00 		 scal 4 , expr
2523 68 85 23 00 		 scal 5 , expr
2527 68 86 23 00 		 scal 6 , expr
252b 68 87 23 00 		 scal 7 , expr
252f 68 88 23 00 		 scal 8 , expr
2533 68 89 23 00 		 scal 9 , expr
2537 68 05 			 scm1
2539 68 03 			 scm2
253b f5 			 sd
253c 75 			 sdb
253d 7d 55 			 sdbi # immed
253f fd 55 			 sdi # immed
2541 d0 			 sep 0
2542 d1 			 sep 1
2543 da 			 sep 10
2544 db 			 sep 11
2545 dc 			 sep 12
2546 dd 			 sep 13
2547 de 			 sep 14
2548 df 			 sep 15
2549 d2 			 sep 2
254a d3 			 sep 3
254b d4 			 sep 4
254c d5 			 sep 5
254d d6 			 sep 6
254e d7 			 sep 7
254f d8 			 sep 8
2550 d9 			 sep 9
2551 7b 			 seq
2552 e0 			 sex 0
2553 e1 			 sex 1
2554 ea 			 sex 10
2555 eb 			 sex 11
2556 ec 			 sex 12
2557 ed 			 sex 13
2558 ee 			 sex 14
2559 ef 			 sex 15
255a e2 			 sex 2
255b e3 			 sex 3
255c e4 			 sex 4
255d e5 			 sex 5
255e e6 			 sex 6
255f e7 			 sex 7
2560 e8 			 sex 8
2561 e9 			 sex 9
2562 fe 			 shl
2563 7e 			 shlc
2564 f6 			 shr
2565 76 			 shrc
2566 38 			 skp
2567 f7 			 sm
2568 77 			 smb
2569 7f 55 			 smbi # immed
256b ff 55 			 smi # immed
256d 68 04 			 spm1
256f 68 02 			 spm2
2571 68 90 			 sret 0
2573 68 91 			 sret 1
2575 68 9a 			 sret 10
2577 68 9b 			 sret 11
2579 68 9c 			 sret 12
257b 68 9d 			 sret 13
257d 68 9e 			 sret 14
257f 68 9f 			 sret 15
2581 68 92 			 sret 2
2583 68 93 			 sret 3
2585 68 94 			 sret 4
2587 68 95 			 sret 5
2589 68 96 			 sret 6
258b 68 97 			 sret 7
258d 68 98 			 sret 8
258f 68 99 			 sret 9
2591 68 07 			 stm
2593 68 00 			 stpc
2595 50 			 str 0
2596 51 			 str 1
2597 5a 			 str 10
2598 5b 			 str 11
2599 5c 			 str 12
259a 5d 			 str 13
259b 5e 			 str 14
259c 5f 			 str 15
259d 52 			 str 2
259e 53 			 str 3
259f 54 			 str 4
25a0 55 			 str 5
25a1 56 			 str 6
25a2 57 			 str 7
25a3 58 			 str 8
25a4 59 			 str 9
25a5 73 			 stxd
25a6 68 0b 			 xid
25a8 68 0a 			 xie
25aa f3 			 xor
25ab fb 55 			 xri # immed
