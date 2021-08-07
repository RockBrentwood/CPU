	TITLE	"ASM17 Sample Code"
	STITLE	"Sample 3 - Serially output the 8 bits in a File Register"
	LIST	COLUMNS=132, NOWRAP

	NOLIST
#include	"regs.def"
	LIST
;
;	In this example, file register 18 are output via I/O B0
;	(Bank 0, 0x12, bit 0).  I/O line B1 (Bank 0, 0x12, bit 1) is
;	used to synchronize the output using the rising edge.
;

Start;
	movlw		08		; Load the decimal 8 into w reg
	movpf		wreg,0x19	; Put 8 into temp register.
	movlb		0		; Make sure bank zero is selected
loop;
	bcf             portb,1		; Clear the sync output
	rrcf		0x18,1		; Rotate right 1 bit, bit 0 to carry
					;
	btfss		alusta,0	; Test Carry, skip if set
					;
	goto		carry_clear	; branch if carry clear
					;
	bsf		portb,0		; otherwise, output positive signal
	goto		break		;   and break.
					;
carry_clear;
	bcf		portb,0		; Carry was clear, so output negative
					;    signal.
break;
	bsf		portb,1		; Raise sync line
					;
	decfsz		0x19		; If all eight bits are not output,
	goto		loop		;   then continue loop
					; otherwise
	bcf		portb,1		;   clear sync output and
	END				;   exit.
	END