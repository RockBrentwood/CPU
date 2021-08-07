	TITLE	"ASM17 Sample Code"
	STITLE	"Sample 4 - Convert a BCD held in 4 LSBs of F0x18"
	LIST	COLUMNS=132, LINES=58, NOWRAP

	NOLIST
#include	"regs.def"
	LIST
;
;	The 4 MSBs are assumed zero to a 7-segment code.  The 7
;	segment code is output via I/O Port C.  This program
;	illustrates the use of a computed GOTO instruction.  This code
;	assumes a typicaly 7-segment display.
;
	ORG		0x00
Start;
	movpf		0x18,wreg	; Starting address of table from 18
					;   Move BCD number as offset into
					;   w register
	call		CONVERT		; Call the convesion subroutine.
					;   The program counter executes the
					;   next instruction at CONVERT
					;
	movlb		1		; Select register bank 1
					;
	movwf		portc		; Output the 7 segment code via I/O
					;   Port C.  the 7-segment display
					;   will now show the BCD number and
					;   this output will remain stable
					;   until Port C is set to a new
					;   value.
	goto		exit		;
					;
CONVERT;
	addwf		pclo		; Compute goto
					; Add the BCD offset to the PC.  This
					; is a compted goto.  Because the
					; ninth bit of PC is set to zero
					; by an "addwf 2", the CONVERT
					; routine must be located within
					; 0x00 to 0xff.
	retlw		00000001b	; complement of 0 in 7-segment code
	retlw		01001111b	; complement of 1 in 7-segment code
	retlw		00010010b	; complement of 2 in 7-segment code
	retlw		00000110b	; complement of 3 in 7-segment code
	retlw		01001100b	; complement of 4 in 7-segment code
	retlw		00100100b	; complement of 5 in 7-segment code
	retlw		01100000b	; complement of 6 in 7-segment code
	retlw		00001111b	; complement of 7 in 7-segment code
	retlw		00000000b	; complement of 8 in 7-segment code
	retlw		00001100b	; complement of 9 in 7-segment code
					;
exit;
	END				;