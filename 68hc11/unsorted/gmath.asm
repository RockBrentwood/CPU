
*			 GMATH.ASM
*
*		       Copyright 1988
*			     by
*		       James C. Shultz
*
* The source code for this General Math package for the 
* MC68HC11 may be freely distributed under the rules of
* public domain.
*
* If there are any questions or comments about this General
* Math package please feel free to contact me.
*
*		       James C. Shultz
*		       J & J Electronic Industries, Inc.
*		       526 North Main Street
*		       Glen Ellyn, IL  60137
*		       (312) 858-2999
*
*
*		REVISION HISTORY:
*	1.0	12-30-88	Release to public domain of a 
*				general math package including
*				the following:
*				HEXBIN - ASCII hex to binary
*					 convertion
*				UPCASE - Convert Lower case
*					 to Upper case
*				BTOA - Binary to ASCII
*				BTOD - Binary to Decimal
*				DTOB - Decimal to Binary
*				WMUL - 24 bit by 16 bit multiply
*				       with a 40 bit result
*				XMUL - 16 bit by 16 bit multiply
*				       with a 32 bit result
*				ZMUL - 24 bit by 8 bit multiply
*				       with a 32 bit result
*	1.1	MM-DD-YY
*


*	EQUATES
RAMBS	EQU	$0000		;START OF RAM
PAREA	EQU	$E000		;START OF PROGRAM AREA

*	VARIABLES
	ORG	RAMBS
SHFTRGH	RMB	1	;INPUT SHIFT REGISTER HIGH
SHFTRG	RMB	2		;INPUT SHIFT REGISTER
TMP1	RMB	1		;USED BY HEXBIN
TEMP	RMB	1		;USED BY BTOD
TEMP0	RMB	1		;USED BY DTOB
TEMP1	RMB	1		;USED BY DTOB
TEMP2	RMB	1		;USED BY DTOB
TEMP3	RMB	1		;USED BY DTOB
VALUE	RMB	8		;SPI DISPLAY VALUE
PROD0	RMB	1		;WMUL PRODUCT
PROD1	RMB	1		;XMUL PRODUCT
PROD2	RMB	1		;XMUL PRODUCT
PROD3	RMB	1		;XMUL PRODUCT
PROD4	RMB	1		;XMUL PRODUCT
FACTA	RMB	2		;XMUL FACTOR
FACTB	RMB	2		;XMUL FACTOR

* Note: Set program area
	ORG	PAREA		;set up program area

***************************************************************
*                     INIT & MAIN PROGRAM
***************************************************************
START:
* .	.	.		;......
*	.	.		;......
*	.	.		;......
*etc. etc

MAIN:
* .	.	.		;......
*	.	.		;......
*	.	.		;......
*etc. etc
	JMP	MAIN		;AND ANOTHER LOOP

***************************************************************
*                     SUBROUTINES
***************************************************************

*********
* Start  MATH related subroutines
*********
*********
* HEXBIN(A) - CONVERT THE ASCII CHAR IN (ACCA) TO BINARY
*	      Place result in ACCA & SHFTRG+1 (lo-byte)
*	      Increments TMP1 if ACCA does not have a valid
*	      hex character
*********
HEXBIN	PSHA
	PSHB
	PSHX
	JSR	UPCASE		;CONVERT TO UPPER CASE
	CMPA	#'0'
	BLT	HEXNOT		;JUMP IF A < $30
	CMPA	#'9'
	BLE	HEXNMB		;JUMP IF 0-9
	CMPA	#'A'
	BLT	HEXNOT		;JUMP ID $39 > A < $41
	CMPA	#'F'
	BGT	HEXNOT		;JUMP IF A > $46
	ADDA	#$9		;CONVERT $A-$F
HEXNMB	ANDA	#$0F		;CONVERT TO BINARY
	LDX	#SHFTRGH
	LDAB	#4
HEXSHFT	ASL	2,X		;3 BYTE SHIFT THROUGH
	ROL	1,X		; CARRY BIT
	ROL	0,X		; CARRY BIT
	DECB
	BGT	HEXSHFT		;SHIFT 4 TIMES
	ORAA	2,X		;GET HIGH 4 BITS
	STAA	2,X		;WRITE NEW LOW BYTE
	BRA	HEXRTS
HEXNOT	INC	TMP1		;INDICATE NOT HEX
HEXRTS	PULX
	PULB
	PULA
	RTS
*********
* UPCASE() - RETURNS ACCA CONVERTED TO UPPERCASE
*********
UPCASE	CMPA	#'a'
	BLT	UPCASE1		;JUMP IF < a
	CMPA	#'z'
	BGT	UPCASE1		;JUMP IF > z
	SUBA	#$20		;CONVERT
UPCASE1	RTS
*********
* BTOA(A) - CONVERT THE BINARY CHAR IN ACCA TO ASCII 
*	    Place result at Index X (upper 4 bits) and
*	    X+1 (lower 4 bits)
*********
BTOA:	PSHA			;SAVE ACCA
	PSHA
	JSR	BTOAL		;CONVERT UPPER 4 BITS
	STAA	0,X		;STORE IT AT X
	PULA			;RESTORE ACCA
	JSR	BTOAR		;CONVERT LOWER 4 BITS
	STAA	1,X		;STORE IT AT X+1
	PULA			;RESTORE ACCA
	RTS
*********
* BTOAL(A) - CONVERT THE LEFT 4 BITS IN A TO ASCII HEX
*********
BTOAL:	LSRA			;SHIFT RIGHT 4 TIMES
	LSRA
	LSRA
	LSRA
*********
* BTOAR(A) - CONVERT THE RIGHT 4 BITS IN A TO ASCII HEX
*********
BTOAR:	ANDA	#$0F		;MASK FOR LOWER 4 BITS
	ADDA	#$30		;CONVERT TO ASCII
	CMPA	#$39		;CHECK IF A-F
	BLE	BTR1		;SKIP IF NOT
	ADDA	#$07		;CONVERT TO A-F
BTR1:	RTS
*********
* BTOD() - BINARY TO DECIMAL CONVERTER 
*	   Input value is in SHFTRGH (hi-byte), 
*	   SHFTRG (middle-byte), and SHFTRG+1 (lo-byte)
*	   The result is in VALUE thru VALUE+8
*********
BTOD:	PSHA			;SAVE ACCA
	PSHB			;SAVE ACCB
	PSHX			;SAVE X
	PSHY			;SAVE Y
	LDX	#PWRTBL		;INIT POWER TABLE PTR
	LDY	#VALUE		;RESULT AREA
	LDD	#$0000		;GET ZERO
	STD	0,Y		;CLEAR DIGIT
	STD	2,Y		;CLEAR DIGIT
	STD	4,Y		;CLEAR DIGIT
	STD	6,Y		;CLEAR DIGIT
BTA:	LDAA	SHFTRG+1	;GET LO BYTE
	SUBA	2,X		;SUB LO BYTE OF 10
	STAA	TEMP		;SAVE NEW LO BYTE
	LDD	SHFTRGH		;LOAD HI BYTE
	BCC	BTB		;CK IF BORROW
	SUBD	#$0001		;GET FROM HI BYTE
	BCS	BTNEXT		;NO BORROW GOTO NEXT
BTB:	SUBD	0,X		;SUB HI BYTE OF 10
	BCS	BTNEXT		;IF FAIL GOTO NEXT
	INC	0,Y		;ELSE ADD ONE TO DISP
	STD	SHFTRGH		;SAVE UPDATED HIBYTE
	LDAA	TEMP		;GET UPDATED LO BYTE
	STAA	SHFTRG+1	;SAVE UPDATED LO BYTE
	BRA	BTA		;DO IT AGAIN
BTNEXT:	INX			;NEXT LOWER PWR OF 10
	INX			;NEXT LOWER PWR OF 10
	INX			;NEXT LOWER PWR OF 10
	INY			;NEXT LOWER DIGIT
	CPX	#PWREND		;CHECK FOR END OF TABLE
	BLO	BTA		;BR IF NOT DONE
	LDAA	SHFTRG+1	;GET REMAINDER
	STA	0,Y		;PUT IN LAST DISPLAY DIGIT
	PULY			;RESTORE Y
	PULX			;RESTORE X
	PULB			;RESTORE ACCB
	PULA			;RESTORE ACCA
	RTS
*********
* DTOB() - DECIMAL TO BINARY CONVERTER 
*	   Index X points to Decimal string
*	   Results will be in SHFTRGH (hi-byte), 
*	   SHFTRG (middle-byte), and SHFTRG+1 (lo-byte)
*********
DTOB:	PSHA			;SAVE ACCA
	PSHB			;SAVE ACCB
	LDD	#$0000		;CLEAR ACCUMULATOR
	STAA	TEMP		;CLEAR TEMP
	STAA	TEMP0		;CLEAR TEMP0
	STAA	TEMP2		;CLEAR TEMP2
	STAA	SHFTRGH		;CLEAR BINARY HIGH BYTE
	STD	SHFTRG		;CLEAR BINARY LOW WORD
DTNEXT:	LDAA	0,X		;LOAD COMPARE VALUE
	SUBA	#$30		;SUB ASCII OFFSET
	CMPA	#$09		;CHECK IF 0 - 9
	BHI	DTEND		;END IF NOT 0 - 9
	STAA	TEMP1		;SAVE IT
	LDD	SHFTRG		;GET LOW WORD
	LSLD			;MULTIPLY BYTE BY 2
	ROL	TEMP2		;MULTIPLY HI BYTE BY 2
	LSLD			;MULTIPLY BYTE BY 2
	ROL	TEMP2		;MULTIPLY HI BYTE BY 2
	ADDD	SHFTRG		;ADD
	BCC	DT20		;CHECK FOR CARRY
	INC	TEMP2		;DO CARRY
DT20:	STAA	TEMP3		;SAVE ACCA
	LDAA	TEMP2		;GET HIGH
	ADDA	SHFTRGH		;ADD
	STAA	TEMP2		;SAVE HIGH
	LDAA	TEMP3		;RESTORE ACCA
	LSLD			;MULTIPLY BYTE BY 2
	ROL	TEMP2		;MULTIPLY HI BYTE BY 2
	ADDD	TEMP0		;ADD TEMP
	BCC	DT30		;CHECK FOR CARRY
	INC	TEMP2		;DO CARRY
DT30:	STD	SHFTRG		;SAVE
	LDAA	TEMP2		;GET HIGH
	STAA	SHFTRGH		;SAVE
	INX			;NEXT
	BRA	DTNEXT		;DO NEXT
DTEND:	PULB			;RESTORE ACCB
	PULA			;RESTORE ACCA
	RTS			;RETURN
*********
* WMUL() - 24 BIT BY 16 BIT MULTIPLY, PRODUCT = 40 BITS
*	   Input - the 16 bits are in FACTA (hi-byte) & 
*	   FACTA+1 (lo-byte),with the 24 bits being pointed 
*	   to by Index X with the low byte being X+2
*	   The result is in PROD0 thru PROD4 with the low
*	   byte being in PROD4
*********
WMUL:	PSHA			;SAVE ACCA
	PSHB			;SAVE ACCB
	LDD	#$0000		;GET ZERO
	STAA	PROD0		;CLEAR
	STD	PROD1		;CLEAR PRODUCT
	LDAA	FACTA+1		;GET LOW BYTE
	LDAB	2,X		;GET LOW BYTE
	MUL			;MULTIPLY
	STD	PROD3		;SAVE
	LDAA	FACTA+1		;GET LOW BYTE
	LDAB	1,X		;GET HIGH BYTE
	MUL			;MULTIPLY
	ADDD	PROD2		;SUM INTO PRODUCT
	STD	PROD2		;SAVE
	BCC	WMU10		;CHECK FOR CARRY
	INC	PROD1		;DO CARRY
WMU10:	LDAA	FACTA		;GET HIGH BYTE
	LDAB	2,X		;GET LOW BYTE
	MUL			;MULTIPLY
	ADDD	PROD2		;SUM INTO PRODUCT
	STD	PROD2		;SAVE
	BCC	WMU20		;CHECK FOR CARRY
	INC	PROD1		;DO CARRY
WMU20:	LDAA	FACTA		;GET HIGH BYTE
	LDAB	1,X		;GET HIGH BYTE
	MUL			;MULTIPLY
	ADDD	PROD1		;SUM INTO PRODUCT
	STD	PROD1		;SAVE
	LDAA	FACTA+1		;GET HIGH BYTE
	LDAB	0,X		;GET LOW BYTE
	MUL			;MULTIPLY
	ADDD	PROD1		;SUM INTO PRODUCT
	STD	PROD1		;SAVE
	BCC	WMU30		;CHECK FOR CARRY
	INC	PROD0		;DO CARRY
WMU30:	LDAA	FACTA		;GET HIGH BYTE
	LDAB	0,X		;GET HIGH BYTE
	MUL			;MULTIPLY
	ADDD	PROD0		;SUM INTO PRODUCT
	STD	PROD0		;SAVE
WMU50:	PULB			;RESTORE ACCB
	PULA			;RESTORE ACCA
	RTS
*********
* XMUL() - 16 BIT BY 16 BIT MULTIPLY, PRODUCT = 32 BITS
*	   Input -  16 bits are in FACTA (hi-byte) & 
*	   FACTA+1 (lo-byte),with the other 16 bits in 
*	   FACTB (hi-byte) & FACTB+1 (lo-byte)
*	   The result is in PROD1 thru PROD4 with the low
*	   byte being in PROD4
*********
XMUL:	PSHA			;SAVE ACCA
	PSHB			;SAVE ACCB
	LDD	#$0000		;GET ZERO
	STD	PROD1		;CLEAR PRODUCT
	LDAA	FACTA+1		;GET LOW BYTE
	LDAB	FACTB+1		;GET LOW BYTE
	MUL			;MULTIPLY
	STD	PROD3		;SAVE
	LDAA	FACTA+1		;GET LOW BYTE
	LDAB	FACTB		;GET HIGH BYTE
	MUL			;MULTIPLY
	ADDD	PROD2		;SUM INTO PRODUCT
	STD	PROD2		;SAVE
	BCC	XMU10		;CHECK FOR CARRY
	INC	PROD1		;DO CARRY
XMU10:	LDAA	FACTA		;GET HIGH BYTE
	LDAB	FACTB+1		;GET LOW BYTE
	MUL			;MULTIPLY
	ADDD	PROD2		;SUM INTO PRODUCT
	STD	PROD2		;SAVE
	BCC	XMU20		;CHECK FOR CARRY
	INC	PROD1		;DO CARRY
XMU20:	LDAA	FACTA		;GET HIGH BYTE
	LDAB	FACTB		;GET HIGH BYTE
	MUL			;MULTIPLY
	ADDD	PROD1		;SUM INTO PRODUCT
	STD	PROD1		;SAVE
XMU50:	PULB			;RESTORE ACCB
	PULA			;RESTORE ACCA
	RTS
 
*********
* ZMUL() - 24 BIT BY 8 BIT MULTIPLY, PRODUCT = 32 BITS
*	   Input - the 8 bits are in FACTA and the 24 
*	   bits are being pointed to by Index X with the 
*	   low byte being X+2
*	   The result is in PROD1 thru PROD4 with the low
*	   byte being in PROD4
*********
ZMUL:	PSHA			;SAVE ACCA
	PSHB			;SAVE ACCB
	LDD	#$0000		;GET ZERO
	STD	PROD1		;CLEAR PRODUCT
	LDAA	FACTA		;GET LOW BYTE
	LDAB	2,X		;GET LOW BYTE
	MUL			;MULTIPLY
	STD	PROD3		;SAVE
	LDAA	FACTA		;GET LOW BYTE
	LDAB	1,X		;GET MIDDLE BYTE
	MUL			;MULTIPLY
	ADDD	PROD2		;SUM INTO PRODUCT
	STD	PROD2		;SAVE
	BCC	ZMU10		;CHECK FOR CARRY
	INC	PROD1		;DO CARRY
ZMU10:	LDAA	FACTA		;GET HIGH BYTE
	LDAB	0,X		;GET HI BYTE
	MUL			;MULTIPLY
	ADDD	PROD1		;SUM INTO PRODUCT
	STD	PROD1		;SAVE
	PULB			;RESTORE ACCB
	PULA			;RESTORE ACCA
	RTS
*********
* End MATH related subroutines
*********
 
PWRTBL:	FDB	$9896		;10,000,000
	FCB	$80
	FDB	$0F42		;1,000,000
	FCB	$40
	FDB	$0186		;100,000
	FCB	$A0
	FDB	$0027		;10,000
	FCB	$10
	FDB	$0003		;1,000
	FCB	$E8
	FDB	$0000		;100
	FCB	$64
	FDB	$0000		;10
	FCB	$0A
	FDB	$0000		;1
	FCB	$01
PWREND:
	ENDWR OF 10
