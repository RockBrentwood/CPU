	TITLE   "ASM17 Sample Code"
	STITLE  "Sample 2 - Conditional Test and Branch"
	LIST    COLUMNS=132, NOWRAP

	NOLIST
 include        "regs.def"
	LIST

;
; Compare contents of F18 to a constant, and take conditional
; branch.
;

#define CONSTANT        2

Start;
	movpf           0x18,wreg       ; Move the contents of F18 to w
					;
	xorlw           CONSTANT        ; Exclusive OR the contents of W
					; and the literal CONSTANT.  If
					; they are equal, all zero bits
					; will result in W and Bit 2
					; in the status register (Z bit)
					; will be set to a one.  Although
					; the SUBWF instruction could be
					; used, it would also alter the
					; CARRY bit.
	btfss           alusta,2        ; Skip if Z bit set
					;
	goto            not_equal       ; Executed if not equal
					;
	goto            equal           ; Executed if equal
					;
not_equal;
					; The values were not equal, so
	clrf            0x18            ;    execute exception handling. . .
					;
equal;
					; else
					;    just exit
	END
