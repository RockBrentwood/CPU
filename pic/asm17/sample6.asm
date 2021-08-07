	TITLE	"ASM17 Sample Code"
	STITLE	"Sample 6 - Clear File Registers F18 through F1f"
		LIST	COLUMNS=132, LINES=58, NOWRAP

	NOLIST
#include	"regs.def"
	LIST
;
;	This program illustrates the use of file select registers and
;	the indirect addressing mode using F0.
;
	ORG		0x00
Start;
	movlw		0x18		; Move literal 0x18 to W
	movpf		wreg,fsr0	; Move 0x18 to file select reg 0
loop;
	clrf		indf0		; Clear file pointed to by fsr0
					;
	incf		fsr0		; Inc to next file register
	btfss		fsr0,0x20	; If next file be cleared
					; then
	goto		loop            ;   go do it
					; otherwise
	END				;   get out...