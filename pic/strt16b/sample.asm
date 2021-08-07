;*******************************************************************
;			    SAMPLE.ASM
;		    8x8 Software Multiplier
;*******************************************************************
;
;   The 16 bit result is stored in 2 bytes
;
; Before calling the subroutine " mpy ", the multiplier should
; be loaded in location " mulplr ", and the multiplicand in
; " mulcnd " . The 16 bit result is stored in locations
; H_byte & L_byte.
;
;	Performance :
;			Program Memory	:  15 locations
;			# of cycles	:  71
;			Scratch RAM	:   0 locations
;
;  This routine is optimized for code efficiency ( looped code )
;  For time efficiency code refer to "mult8x8F.asm" ( straight line code )
;*******************************************************************
;
mulcnd	equ	09	; 8 bit multiplicand
mulplr	equ	10	; 8 bit multiplier
H_byte	equ	12	; High byte of the 16 bit result
L_byte	equ	13	; Low byte of the 16 bit result
count	equ	14	; loop counter
portb	equ	06	; I/O register F6
;
;
     include	     "picreg.equ"
;
; ***************************** 	Begin Multiplier Routine
mpy_S	clrf	H_byte
	clrf	L_byte
	movlw	8
	movwf	count
	movf	mulcnd,w
	bcf	STATUS,CARRY	; Clear the carry bit in the status Reg.
loop	rrf	mulplr
	btfsc	STATUS,CARRY
	addwf	H_byte,Same
	rrf	H_byte,Same
	rrf	L_byte,Same
	decfsz	count
	goto	loop
;
	retlw	0
;
;********************************************************************
;		Test Program
;*********************************************************************
start	clrw
	option
main	movf	portb,w
	movwf	mulplr		; multiplier (in mulplr) = 05
	movf	portb,w
	movwf	mulcnd
;
call_m	call	mpy_S		; The result is in locations F12 & F13
			       ; H_byte & L_byte
;
	goto	main
;
	org	01FFh
	goto	start
;
     END


