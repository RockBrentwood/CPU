	TITLE	"ASM17 Sample Code"
	STITLE	"Sample 5 - Output the File Pointed to by F0x18 via Port C"
	LIST	COLUMNS=132, LINES=58, NOWRAP

	NOLIST
#include	"regs.def"
	LIST

;
;	Assume:	0x0A in F0x18, and 0x5A in F0x0A.  The following program
;		will output 0x5A via Port C.
;

	ORG		0x00
Start;
	movpf		0x18,wreg	; Move the contents of F0x18 to W.
					;   After execution, W contains 0A.
	movwf		fsr0		; Move the contents of W to FSR0.
					;   After execution, FSR0 contains
					;   0A.
	movpf		indf0,wreg	; Move the contents of the file
					;   pointed to by the FSR (the
					;   contents of F0x18) to W.  Thus,
					;   W contains 5a after execution.
	movlb		1		; Select register bank 1.
	movwf		portc		; Output on Port C.

	END				;