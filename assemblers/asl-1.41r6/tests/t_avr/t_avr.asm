
	cpu     at90s8515
	page    0
	include regavr.inc

	adc     r3,r1

	add     r28,r28

	and     r2,r16

	andi    r19,$aa

	adiw    r26,14

	asr     r17

	bclr    7

	bld     r0,4

	brbc    1,*

	brbs    6,*

	brcc    next    ; 1   = 01

	brcs    next    ; 0   = 00
next:
	breq    next    ; -1  = 7F

	brge    next    ; -2  = 7E

	brsh    next    ; -3  = 7D

	brid    next    ; -4  = 7C

	brie    next    ; -5  = 7B

	brlo    next    ; -6  = 7A

	brlt    next    ; -7  = 79

	brmi    next    ; -8  = 78

	brne    next    ; -9  = 77

	brhc    next    ; -10 = 76

	brhs    next    ; -11 = 75

	brpl    next    ; -12 = 74

	brtc    next    ; -13 = 73

	brts    next    ; -14 = 72

	brvc    next    ; -15 = 71

	brvs    next    ; -16 = 70

	bset    6

	bst     r1,2

	call    $123456

	cbr     r16,$f0

	cbi     $12,7

	clc

	cli

	cln

	clh

	clr     r18

	cls

	clt

	clv

	clz

	com     r4

	cp      r4,r19

	cpc     r3,r1

	cpi     r19,3

	cpse    r4,r0

	dec     r17

	eor     r0,r22

	icall

	ijmp

	in      r23,$34

	inc     r22

	jmp     $123456

	ld      r2,x
	ld      r0,x+
	ld      r3,-x

	ld      r1,y
	ld      r0,y+
	ld      r3,-y
	ldd     r4,y+$33

	ld      r1,z
	ld      r0,z+
	ld      r3,-z
	ldd     r4,z+$33

	ldi     r30,$f0

	lds     r2,$ff00

	lpm

	lsl     r0

	lsr     r0

	mov     r16,r0

;        mul     r6,r5

	neg     r11

	nop

	or      r15,r16

	ori     r16,$f0

	out     $18,r16

	pop     r13

	push    r14

	rcall   *

	ret

	reti

	rjmp    *

	rol     r15

	ror     r15

	sbc     r3,r1

	sbci    r17,$4f

	sbi     $1c,3

	sbic    $1c,1

	sbis    $10,3

	sbr     r16,3

	sbrc    r0,7

	sbrs    r0,7

	sec

	sei

	sen

	seh

	ser     r17

	ses

	set

	sev

	sez

	sleep

	st      x,r1
	st      x+,r0
	st      -x,r3

	st      y,r1
	st      y+,r0
	st      -y,r3
	std     y+2,r4

	st      z,r1
	st      z+,r0
	st      -z,r3
	std     z+2,r4

	sts     $ff00,r2

	sub     r13,r12

	subi    r22,$11

	swap    r1

	tst     r3

	wdr

