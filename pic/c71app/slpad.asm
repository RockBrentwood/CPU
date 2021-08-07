TITLE   "A/D in Sleep Mode"
;This program is a simple implementation of the PIC16C71's
;A/D feature. This program demonstrates
;how to do a a/d in sleep mode on the PIC16C71.
;The A/D is configured as follows:
;       Vref = +5V internal.
;       A/D Osc. = internal RC
;       A/D Interrupt = OFF
;       A/D Channels = "Round Robin" format.
;       A/D reuslts are stored in ram locations as follows:
;       ch0 --> ADTABLE + 0
;       ch1 --> ADTABLE + 1
;       ch2 --> ADTABLE + 2
;       ch3 --> ADTABLE + 3
;
;The ch3 A/D result is displayed on a 3 digit LED display LTM8522. 
;
;
	LIST P=16C71,F=INHX8M
;
	include "d:\pic16c\ad\picreg.equ"
;
;       REGISTER EQUATES FOR DISPLAY ROUTINES
;
ACCA    EQU     18h
ACCA1   equ     19h
ACCB    EQU     1Ah
ACCB1   EQU     1Bh
ACCC    EQU     1Ch
ACCC1   equ     1Dh
ACCD    EQU     1Eh
ACCD1   equ     1Fh
ACCE    EQU     20h
ACCE1   equ     21h
TCAL    EQU     22h
TEMP    EQU     24h
;
ch2     equ     6
ch3     equ     7
flag    equ     0C
ADTABLE equ     20
;
	ORG     0x00
;
;
	goto    start
;
	org     0x04
	goto    service_int      ;interrupt vector
;
;
	org     0x10
start
	call    ClearRam
	movlw   B'11110000'     ;init i/o ports
	movwf   PORT_B
	tris    PORT_B
;
	call    InitializeAD   
update      
	bcf     ADCON0,ADIF     ;clr a/d flag
	movf    ADRES,W
	movwf   0               ;save in table
	movlw   ADTABLE         ;chk if ch3
	subwf   FSR,W           ;     /
	xorlw   3               ;    /
	btfss   STATUS,Z        ;yes then skip
	goto    NextAd          ;else do next channel
	movf    ADRES,W         ;get a/d value
	movwf   ACCB1           ;in ACCB and ACCB1
	clrf    ACCB            ;       /
;code below is used to to display the a/d result on the
;LTM8522 LED display unit.
	movlw   .196            ;multilply a/d result
	movwf   ACCA1           ;by 196 to take into
	clrf    ACCA            ;account 5V ref.        
	call    MPY             ;ACCA*ACCB (16 bit)
	movf    ACCC1,W         ;transfer result to
	movwf   ACCB1           ;ACCB and ACCB1        
	movf    ACCC,W          ;       /
	movwf   ACCB            ;       /
	CALL    BINTOB          ;convert to BCD
	CALL    DRIVR           ;LOAD LED Display
;******** end of display setup/update ********************
NextAd
	call    NextChannel     ;select next channel
	bsf     ADCON0,2        ;start new a/d conversion
;
	sleep
	goto    update          ;wake up and update
;
service_int
	return                  ;do not enable int
;
PAGE
;
;ClearRam clears ram from 0x0C to 0x2F.
ClearRam
	movlw   0x0c            ;get first ram location
	movwf   FSR             ;load in indirect address
CR1
	clrf    0               ;do indirect ram clr
	incf    FSR             ;inc FSR
	btfss   FSR,5           ;if bit 5 set then skip
	goto    CR1             ;else clr next
	btfss   FSR,4           ;if bit 4 set then done
	goto    CR1             ;else clr next
	return                  
;
;InitializeAD, initializes and sets up the A/D hardware.
InitializeAD
	bsf     STATUS,5        ;select pg1
	movlw   B'00000000'     ;select ch0-ch3...
	movwf   ADCON1          ;as analog inputs
	bcf     STATUS,5        ;select pg0
	movlw   B'11000001'     ;select:internal RC, ch0.
	movwf   ADCON0          ;turn on a/d
	movlw   ADTABLE         ;get top of table address
	movwf   FSR             ;load into indirect reg
	clrf    ADRES           ;clr result reg.
	clrf    INTCON          ;clear all interrupts
	bsf     INTCON,ADIE     ;eanble a/d
	return
;
;NextChannel, selects the next channel to be sampled in a 
;"round-robin" format.
NextChannel
	movlw   0x08            ;load channel offset
	addwf   ADCON0          ;add to conf. reg.
	bcf     ADCON0,5        ;clear any carry over
;increment pointer to correct a/d result register
	clrf    TEMP
	btfsc   ADCON0,3        ;test lsb of chnl select
	bsf     TEMP,0          ;set if ch1 or ch3
	btfsc   ADCON0,4        ;test msb of chnl select
	bsf     TEMP,1          ;set if ch0 or ch2
	movlw   ADTABLE         ;get top of table
	addwf   TEMP,0          ;add with temp
	movwf   FSR
	return
;
PAGE
;
;The rest of the subroutines below, are required to display the a/d
;result on the LED display LTM8522.
;
;***********************************************************************
;
;       MULTIPLY SUBROUTINE 16 BIT * 16 BIT FOR 32 BIT RESULT
;
;       INPUTS  ACCA (16BITS)
;               ACCB (16BITS)
;
;       OUTPUTS ACCB  (16BITS) ACCC (16BITS) 
;
;***********************************************************************
MPY     
	CALL    SETUP   ;RESULTS IN B(16 MSB'S) AND C(16 LSB'S)
MLOOP
	RRF     ACCD    ;ROTATE D RIGHT
	RRF     ACCD+1
	SKPNC           ;NEED TO ADD?
	CALL    MADD
	RRF     ACCB
	RRF     ACCB+1
	RRF     ACCC
	RRF     ACCC+1
	DECFSZ  TEMP    ;LOOP UNTIL ALL BITS CHECKED
	GOTO    MLOOP
	RETLW   0

MADD
	MOVF    ACCA+1,W
	ADDWF   ACCB+1  ;ADD LSB
	BTFSC   STATUS,0        ;ADD IN CARRY
	INCF    ACCB
	MOVF    ACCA,W
	ADDWF   ACCB    ;ADD MSB
	RETLW   0

SETUP
	MOVLW   10h
	MOVWF   TEMP
	MOVF    ACCB,W  ;MOVE B TO D
	MOVWF   ACCD
	MOVF    ACCB+1,W
	MOVWF   ACCD+1
	MOVF    ACCC,W
	MOVWF   ACCE
	MOVF    ACCC+1,W
	MOVWF   ACCE+1
	CLRF    ACCB
	CLRF    ACCB+1
	RETLW   0

;*****************************************************************************
;
;       DIVIDE SUBROUTINE  32 BIT / 16 BIT FOR 16 BIT RESULT
;
;       INPUTS  ACCB (16 BIT) ACCC (16BIT ) / ACCA (16 BIT)
;
;       OUTPUTS ACCB (16 BIT)
;
;*****************************************************************************
 
DIV
	CALL    SETUP
	MOVLW   20h
	MOVWF   TEMP
	CLRF    ACCC
	CLRF    ACCC+1
DLOOP
	CLRC
	RLF     ACCE+1
	RLF     ACCE
	RLF     ACCD+1
	RLF     ACCD
	RLF     ACCC+1
	RLF     ACCC
	MOVF    ACCA,W
	SUBWF   ACCC,W  ;CHECK IF A>C
	SKPZ
	GOTO    NOCHK
	MOVF    ACCA+1,W
	SUBWF   ACCC+1,W        ;IF MSB EQUAL THEN CHECK LSB
NOCHK
	SKPC            ;CARRY SET IF C>A
	GOTO    NOGO
	MOVF    ACCA+1,W        ;C-A INTO C
	SUBWF   ACCC+1
	BTFSS   STATUS,0
	DECF    ACCC
	MOVF    ACCA,W
	SUBWF   ACCC
	SETC            ;SHIFT A 1 INTO B (RESULT)
NOGO
	RLF     ACCB+1
	RLF     ACCB
	DECFSZ  TEMP    ;LOOP UNTILL ALL BITS CHECKED
	GOTO    DLOOP
	RETLW   0

;*************************************************************************
;       THIS SUBROUTINE CONVERTS A 16 BIT BINARY WORD
;       INTO A 4 DIGIT BCD
;       THE HIGH ORDER INPUT IS IN ACCB, LOW ORDER IN ACCB+1
;       OUTPUT IS IN ACCD, ACCD+1 AND ACCE, ACCE+1
;       WITH THE 4 LSB'S OF ACCD HIGH ORDER
;
;*************************************************************************
;
BINTOB
	MOVLW   10h
	MOVWF   ACCE+1
	CLRF    ACCE    ;CLEAR OUTPUT REGISTERS
	CLRF    ACCD+1
	CLRF    ACCD
LOOPC
	RLF     ACCB+1  ;SHIFT BINARY INTO BCD REGS ONE BIT
	RLF     ACCB
	RLF     ACCE
	RLF     ACCD+1
	RLF     ACCD
	DECFSZ  ACCE+1
	GOTO    ADJDEC
	MOVF    ACCE,W
	MOVWF   ACCE+1
	MOVLW   0Fh
	RRF     ACCE+1
	RRF     ACCE+1
	RRF     ACCE+1
	RRF     ACCE+1
	ANDWF   ACCE+1,F
	MOVF    ACCD+1,W
	MOVWF   ACCE
	MOVLW   0Fh
	ANDWF   ACCE
	RRF     ACCD+1
	RRF     ACCD+1
	RRF     ACCD+1
	RRF     ACCD+1
	ANDWF   ACCD+1
	RETLW   0       ;EXIT AFTER 16 SHIFTS

ADJDEC
	MOVLW   ACCE
	MOVWF   FSR
	CALL    ADJBCD
	MOVLW   ACCD+1
	MOVWF   FSR
	CALL    ADJBCD
	MOVLW   ACCD
	MOVWF   FSR
	CALL    ADJBCD
	GOTO    LOOPC

ADJBCD
	MOVLW   03h
	ADDWF   0,W     ;ADD 3 TO LSD
	MOVWF   TEMP
	BTFSC   TEMP,3  ;SAVE IF RESULT >7
	MOVWF   0
	MOVLW   30h
	ADDWF   0,W     ;ADD 3 TO MSD
	MOVWF   TEMP
	BTFSC   TEMP,7  ;SAVE IF RESULT >7
	MOVWF   0
	RETLW   0

;***************************************************************************
;
;       BEGIN OF LED DRIVER ROUTINES
;
;***************************************************************************
;
BCDT
	ADDWF   PC      ;ADD NUMBER TO PC - MAKE JUMP TABLE
	RETLW   0FCh    ;CODE FOR ZERO
	RETLW   060h    ;CODE FOR ONE
	RETLW   0DAh    ;CODE FOR TWO
	RETLW   0F2h    ;CODE FOR THREE
	RETLW   066h    ;CODE FOR FOUR
	RETLW   0B6h    ;CODE FOR FIVE
	RETLW   0BEh    ;CODE FOR SIX
	RETLW   0E0h    ;CODE FOR SEVEN
	RETLW   0FEh    ;CODE FOR EIGHT
	RETLW   0E6h    ;CODE FOR NINE

LDATA
	MOVLW   8
	MOVWF   TEMP
DRLOOP  
	BCF     PORT_B,1        ;SET CLOCK LO
	NOP
	BSF     PORT_B,2        ;SET DATA HIGH
	RLF     ACCA
	BTFSS   STATUS,0        ;CHECK DATA
	BCF     PORT_B,2        ;SET DATA LOW IF DATA LOW
	NOP
	BSF     PORT_B,1        ;SET CLOCK HIGH
	DECFSZ  TEMP
	GOTO    DRLOOP
	RETLW   0
;*********************************************************************
;
;       DISPLAY DRIVER ROUTINE
;       DISPLAYS 3 DIGITS 
;       INPUT IN ACCD ADDC+1 ACCE
;       EXPECTS DECIMAL DIGIT (0-9) IN REGISTER
;
;**********************************************************************
;
DRIVR
	MOVF    ACCD,W
;        BTFSS   STATUS,Z
	GOTO    ALL_DIGIT
	MOVF    ACCD+1,W
	BTFSS   STATUS,Z
	GOTO    TWO_DIGIT
	GOTO    ONE_DIGIT
ALL_DIGIT
	CALL    BCDT    ;FIND LED CODE FOR MSB
	MOVWF   ACCD
	movlw   1       ;set decimal on
	addwf   ACCD
	MOVF    ACCD+1,W
TWO_DIGIT
	CALL    BCDT    ;FIND LED CODE FOR MIDDLE BITS
	MOVWF   ACCD+1
ONE_DIGIT
	MOVF    ACCE,W
	CALL    BCDT    ;FIND LED CODE FOR LSB
	MOVWF   ACCE
DISPLAY
	CLRF    ACCA
	BCF     PORT_B,1        ;SET CLOCK LOW
	NOP
	BCF     PORT_B,3        ;SET ENABLE LOW  (ON)
	NOP
	BSF     PORT_B,2        ;SET DATA HIGH
	NOP
	BSF     PORT_B,1        ;CLOCK HIGH TO LOAD START BIT
	NOP
	BCF     PORT_B,2        ;DATA LOW
	NOP
	BCF     PORT_B,1        ;CLOCK LOW
	MOVF    ACCD,W
	MOVWF   ACCA    ;LOAD MSB
	CALL    LDATA   ;SHIFT IN MSB
	MOVF    ACCD+1,W
	MOVWF   ACCA    ;MDDL TO LOAD REGISTER
	CALL    LDATA   ;SHIFT IN MDDL
	MOVF    ACCE,W
	MOVWF   ACCA    ;LSB TO LOAD REGISTER
	CALL    LDATA   ;SHIFT IN LSB
	CLRF    ACCA
	CALL    LDATA   ;LOAD IN EIGHT ZEROES
	CALL    LDATA   ;LOAD IN EIGHT ZEROES
	BCF     PORT_B,1        ;SET CLOCK LOW
	NOP
	BSF     PORT_B,3        ;SET ENABLE HIGH (OFF)

	MOVLW   b'00001111'     ;SET RB1 HIGH (ON WHEN ACTIVATED)
	MOVWF   PORT_B
	RETLW   0


	END

