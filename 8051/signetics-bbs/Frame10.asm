;
;
;	FRAME BUFFER INIT PROGRAM FOR SAA9051, TDA 4680 AND TDA 8444
;	87C751 CPU SENDS I2C INIT DATA AFTER 1 SEC. POWER UP DELAY.
;
;
;
;	FRAME10.ASM	03/30/90 CHANGE DATA FIRST SHIP OF BOARD
;			FOR TDA4680 V4X. NTSC MATRIX 1.0 VPP OUTPUT
;      
;
;	FRAME03.ASM	03/09/90 CHANGE COMMENT ; FOR DMSD DATA
;       FRAME02.ASM     03/09/89 HERB KNIESS
;	FRAME01.ASM	12/04/89 HERB KNIESS
;	I2CINIT.ASM	11/16/89 GREG GOODHUE
;
;
;
;
;*****************************************************************************
;
;
;
;            I2C Peripheral Initialization Program for the 83C751
;
; This program first provides both a positive and negative going reset pulse 
; on two port pins to allow any system devices requiring a reset pulse to be 
; initialized. Then, the program will send out one or more I2C messages (from 
; a table) to allow I2C devices to be initialized with specific data. 
;
;
;
;*****************************************************************************
;
$TITLE(FRAME10.ASM  FRAME BUFFER BOARD DMSD2B INIT PROGRAM)
$DATE(03/30/90)
$MOD751
$DEBUG
;
;
; Value definitions.
;
CTVAL      EQU     02h                 ;CT1, CT0 bit values for I2C.
;
;
; Masks for I2CFG bits.
;
BTIR       EQU     10h                 ;Mask for TIRUN bit.
BMRQ       EQU     40h                 ;Mask for MASTRQ bit.
;
;
; Masks for I2CON bits.
;
BCXA       EQU     80h                 ;Mask for CXA bit.
BIDLE      EQU     40h                 ;Mask for IDLE bit.
BCDR       EQU     20h                 ;Mask for CDR bit.
BCARL      EQU     10h                 ;Mask for CARL bit.
BCSTR      EQU     08h                 ;Mask for CSTR bit.
BCSTP      EQU     04h                 ;Mask for CSTP bit.
BXSTR      EQU     02h                 ;Mask for XSTR bit.
BXSTP      EQU     01h                 ;Mask for XSTP bit.
;
;
; RAM locations used by I2C routines.
;
BitCnt     DATA    21h                 ;I2C bit counter.
ByteCnt    DATA    22h
SlvAdr     DATA    23h                 ;Address of active slave.
SubAdr     DATA    24h
;
CmdSav     DATA    25h                 ;Saves last command byte.
PtrSav     DATA    26h                 ;Saves starting index for last command.
StackSave  DATA    27h                 ;Saves stack address for bus recovery.
;
Flags      DATA    20h                 ;I2C software status flags.
NoAck      BIT     Flags.0             ;Indicates missing acknowledge.
Fault      BIT     Flags.1             ;Indicates a bus fault of some kind.
Retry      BIT     Flags.2             ;Indicates that last I2C transmission
;                                      ;  failed and should be repeated.
;
SCL        BIT     P0.0                ;Port bit for I2C serial clock line.
SDA        BIT     P0.1                ;Port bit for I2C serial data line.
;
PosRst     BIT     P3.4                ;Port pin for positive reset pulse.
NegRst     BIT     P3.3                ;Port pin for negative reset pulse.
;
;*****************************************************************************
;                                Begin Code
;*****************************************************************************
;
; Reset and interrupt vectors.
;
           AJMP    Reset               ;Reset vector at address 0.
;
;
; A timer I timeout usually indicates a 'hung' bus.
;
           ORG     1Bh                 ;Timer I (I2C timeout) interrupt.
TimerI:    SETB    CLRTI               ;Clear timer I interrupt.
           CLR     TIRUN
           ACALL   ClrInt              ;Clear interrupt pending.
           MOV     SP,StackSave        ;Restore stack for return to main.
           AJMP    Recover             ;Attempt bus recovery.
ClrInt:    RETI
;
;
;*****************************************************************************
;                    Main Transmit and Receive Routines
;*****************************************************************************
;
; Send data byte(s) to slave.
;   Enter with slave address in SlvAdr, data in XmtDat buffer, # of data 
;   bytes to send in ByteCnt.
;
SendData:  CLR     NoAck               ;Clear error flags.
           CLR     Fault
           CLR     Retry
           MOV     StackSave,SP        ;Save stack address for bus fault.
           MOV     A,SlvAdr            ;Get slave address.
           ACALL   SendAddr            ;Get bus and send slave address.
           JB      NoAck,SDEX          ;Check for missing acknowledge.
           JB      Fault,SDatErr       ;Check for bus fault.
;
SDLoop:    ACALL   GetNext             ;Get next data byte for slave.
           ACALL   XmitByte            ;Send data to slave.
           JB      NoAck,SDEX          ;Check for missing acknowledge.
           JB      Fault,SDatErr       ;Check for bus fault.
           DJNZ    ByteCnt,SDLoop
;
SDEX:      ACALL   SendStop            ;Send an I2C stop.
           RET
;
;
; Handle a transmit bus fault.
;
SDatErr:   AJMP    Recover             ;Attempt bus recovery.
;
;
;*****************************************************************************
;                               Subroutines
;*****************************************************************************
;
; Send address byte.
;   Enter with address in ACC.
;
SendAddr:  MOV     I2CFG,#BMRQ+BTIR+CTVAL ;Request I2C bus.
           JNB     ATN,$               ;Wait for bus granted.
           JNB     Master,SAErr        ;Should have become the bus master.
           MOV     I2DAT,A             ;Send first bit, clears DRDY.
           MOV     I2CON,#BCARL+BCSTR+BCSTP ;Clear start, releases SCL.
           ACALL   XmitAddr            ;Finish sending address.
           RET
;
SAErr:     SETB    Fault               ;Return bus fault status.
           RET
;
;
; Byte transmit routine.
;   Enter with data in ACC.
;   XmitByte : transmits 8 bits.
;   XmitAddr : transmits 7 bits (for address only).
;
XmitAddr:  MOV     BitCnt,#8           ;Set 7 bits of address count.
           SJMP    XmBit2
;
XmitByte:  MOV     BitCnt,#8           ;Set 8 bits of data count.
XmBit:     MOV     I2DAT,A             ;Send this bit.
XmBit2:    RL      A                   ;Get next bit.
           JNB     ATN,$               ;Wait for bit sent.
           JNB     DRDY,XMErr          ;Should be data ready.
           DJNZ    BitCnt,XmBit        ;Repeat until all bits sent.
           MOV     I2CON,#BCDR+BCXA    ;Switch to receive mode.
           JNB     ATN,$               ;Wait for acknowledge bit.
           JNB     RDAT,XMBX           ;Was there an ack?
           SETB    NoAck               ;Return no acknowledge status.
XMBX:      RET
;
XMErr:     SETB    Fault               ;Return bus fault status.
           RET
;
;
; I2C stop routine.
;
SendStop:  CLR     MASTRQ              ;Release bus mastership.
           MOV     I2CON,#BCDR+BXSTP   ;Generate a bus stop.
           JNB     ATN,$               ;Wait for atn.
           MOV     I2CON,#BCDR         ;Clear data ready.
           JNB     ATN,$               ;Wait for stop sent.
           MOV     I2CON,#BCARL+BCSTP+BCXA ;Clear I2C bus.
           CLR     TIRUN               ;Stop timer I.
           RET
;
;
; Bus fault recovery routine.
;
Recover:   ACALL   FixBus              ;See if bus is dead or can be 'fixed'.
           JC      BusReset            ;If not 'fixed', try extreme measures.
           SETB    Retry               ;If bus OK, return to main routine.
           CLR     Fault
           CLR     NoAck
           SETB    CLRTI
           SETB    TIRUN               ;Enable timer I.
           SETB    ETI                 ;Turn on timer I interrupts.
           RET
;
; This routine tries a more extreme method of bus recovery.
;   This is used if SCL or SDA are stuck and cannot otherwise be freed.
;   (will return to the Recover routine when Timer I times out)
;
BusReset:  CLR     MASTRQ              ;Release bus.
           MOV     I2CON,#0BCh         ;Clear all I2C flags.
           SETB    TIRUN
           SJMP    $                   ;Wait for timer I timeout (this will
;                                      ;  reset the I2C hardware).
;
;
; This routine attempts to regain control of the I2C bus after a bus fault.
;   Returns carry clear if successful, carry set if failed.
;
FixBus:    CLR     MastRQ              ;Turn off I2C functions.
           SETB    C
           SETB    SCL                 ;Insure I/O port is not locking I2C.
           SETB    SDA
           JNB     SCL,FixBusEx        ;If SCL is low, bus cannot be 'fixed'.
           JB      SDA,RStop           ;If SCL & SDA are high, force a stop.
           MOV     BitCnt,#9           ;Set max # of tries to clear bus.
ChekLoop:  CLR     SCL                 ;Force an I2C clock.
           ACALL   SDelay
           JB      SDA,RStop           ;Did it work?
           SETB    SCL
           ACALL   SDelay
           DJNZ    BitCnt,ChekLoop     ;Repeat clocks until either SDA clears
                                       ;  or we run out of tries.
           SJMP    FixBusEx            ;Failed to fix bus by this method.
;
RStop:     CLR     SDA                 ;Try forcing a stop since SCL & SDA
           ACALL   SDelay              ;  are both high.
           SETB    SCL
           ACALL   SDelay
           SETB    SDA
           ACALL   SDelay
           JNB     SCL,FixBusEx        ;Are SCL & SDA still high? If so,
           JNB     SDA,FixBusEx        ;  assume bus is now OK, and return
           CLR     C                   ;  with carry cleared.
FixBusEx:  RET
;
;
; Short delay routine (10 machine cycles).
;
SDelay:    NOP
           NOP
           NOP
           NOP
           NOP
           NOP
           NOP
           NOP
           RET
;
;
; Long delay routine (approx 250,000 machine cycles = 250 ms @12MHz).
;
LDelay:    MOV     R6,A
           MOV     R7,#250
LD1:       NOP
           NOP
           DJNZ    R7,LD1              ;250 x 4 cycles = 1 msec (@12MHz).
           DJNZ    R6,LD1              ;approx. 250 msec (@12MHz)
           RET
;
;
; This routine gets the next command/data byte from the command table.
;   Returns data in ACC.
;   Note: The table length can be 256 bytes maximum, due to the way this 
;   routine is written. 
;
GetNext:   MOV     A,R0                ;Set offset into table.
           MOV     DPTR,#CmdTab        ;Set table start address.
           MOVC    A,@A+DPTR           ;Get table entry
           INC     R0
           RET
;
;
;*****************************************************************************
;                               Main Program
;*****************************************************************************
;
Reset:     MOV     SP,#07h             ;Set stack location.
           SETB    ETI                 ;Enable timer I interrupts.
           SETB    EA                  ;Enable global interrupts.
           MOV     R0,#0               ;Set up initial command pointer.
;
           SETB    PosRst              ;Assert positive reset pulse.
           CLR     NegRst              ;Assert negative reset pulse.
           MOV     A,#250              ;Set for 250 msec delay.
           ACALL   LDelay              ;Wait.
           CLR     PosRst              ;De-assert positive reset pulse.
           SETB    NegRst              ;De-assert negative reset pulse.
           MOV     A,#100              ;Set for 100 msec delay.
           ACALL   LDelay              ;Wait.
;
MainLoop:  MOV     PtrSav,R0
           ACALL   GetNext             ;Get next command.
           JZ      Done                ;Found STOP command.
           MOV     ByteCnt,A           ;Set up byte count.
           ACALL   GetNext             ;Get slave address.
           MOV     SlvAdr,A            ;Set up slave address.
           ACALL   SendData            ;Send data to slave.
           SJMP    MainLoop
;
MainRetry: MOV     R0,PtrSav           ;Restore command pointer to retry.
           AJMP    MainLoop            ;Repeat last command.
;
Done:      MOV     PCON,#02h           ;Done, enter power down mode. 
           NOP
	   NOP	
	   NOP
;
;
;
;		 I2C INITIALIZATION DATA TABLE.
;
;		 Definition of data table contents:
;
;
;	1st byte = Command : 00 - Stop.
;                    : nonzero = length count for sub address and
;		       data bytes. slave address is not included in this
;		       number.
;
;	2nd byte = Slave address. 
;	3rd byte = First data byte or slave subaddress.
;	4th to last byte = Additional data bytes.
;
;
;
           ORG     200h		;COMMAND TABLE AT 200H
;
;
CmdTab:		
;
;
;
;	SAA 9051 ADDRESS 8AH
;	
 	DB      0Dh,8Ah,00H             ;13 DATA BYTES TOTAL        
        DB      64H,35H,0AH,0F8H,0CAH,0FEH,029H,00H,77H,0E0H,40H,00H
;
;
;
;	TDA 8444-1			;ADDRESS 40H
;
	DB	09H,40H,00H		; 9 BYTES TOTAL AFTER SLAVE ADD	
	DB	26H,26H,1EH,00H,00H,23H,3FH,3FH
;
;
;
;
;	TDA4680 ADDRESS 88H		;4680 MOVED TO LAST INIT DEVICE
;
	DB	0CH,88H,00H		;12 DATA BYTES TOTAL
	DB	2AH,13H,33H,22H,34H,34H,34H,20H,20H,20H,3FH             
;
;
;
;	TDA 4680 OTHER ADDRESSES BEGINNING AT SUB ADD OC. (0B IS OMITTED)
;
;
	DB 	03H,88H,0CH 		;SUB ADDRESS 0C AND 2 COMMAND DATA 
	DB	89H,10H	
;
;
	DB	00H		;SETS END OF TABLE 0 LENGTH MESSAGE
	DB	00H
 	DB	00H		;JUST FOR SAFTY
	DB	00H
	DB	00H
	DB	00H
	DB	00H
	DB	00H
	DB	00H
	DB	00H
;
;
           END

