
;****************************************************************************

;                     Software Implemented I2C Drivers

; These routines allow an 80C51 based microcontroller to drive the I2C bus 
; as a single master. The main program at the end demonstrates writing and 
; reading several types of devices: 

;    PCF8570 256 byte static RAM.
;    PCF8574 8-bit I/O expander.
;    SAA1064 4 digit LED display driver.


;             Written by G.Goodhue, Philips Components-Signetics

;****************************************************************************


$Title(I2C Routines for 80C51)
$Date(08/14/90)
$MOD51
$DEBUG


;****************************************************************************
;                               Definitions
;****************************************************************************


; Addresses of several I2C devices as connected on the Signetics I2C 
; Evaluation Board.

I2CRAM     EQU     0AEh           ;Slave address for PCF8570 RAM chip.
I2CIO      EQU     4Eh            ;Slave address for PCF8574 I/O expandor.
I2CLED     EQU     76h            ;Slave address for SAA1064 LED driver.


; Data storage locations

BitCnt     DATA    8h             ;Bit counter for I2C routines.
ByteCnt    DATA    9h             ;Byte counter for I2C routines.
SlvAdr     DATA    0Ah            ;Slave address for I2C routines.

XmtDat     DATA    10h            ;I2C transmit buffer, 8 bytes max.
RcvDat     DATA    18h            ;I2C receive buffer, 8 bytes max.
AltRcv     DATA    20h            ;Alternate I2C receive buffer, 8 bytes max.

Flags      DATA    28h            ;Location for bit flags
NoAck      BIT     Flags.0        ;I2C no acknowledge flag.
BusFault   BIT     Flags.1        ;I2C bus fault flag.
I2CBusy    BIT     Flags.2        ;I2C busy flag.


; I2C connections.

SCLPin     BIT     P0.0           ;I2C serial clock line.
SDAPin     BIT     P0.1           ;I2C serial data line.


;****************************************************************************
;                       Reset and Interrupt Vectors
;****************************************************************************


           ORG     0
           AJMP    Reset


;****************************************************************************
;                               Subroutines
;****************************************************************************


           ORG     30h

; BitDly - insures minimum high and low clock times on I2C bus.
; This routine must be tuned for the actual oscilator frequency used, shown 
; here tuned for a 12MHz clock. Note that the CALL instruction that invokes 
; BitDly already uses 2 machine cycles.

BitDly:    NOP                    ;NOPs to delay 5 microseconds (minus 4
                                  ;  machine cycles for CALL and RET).
           RET


; SCLHigh - sends SCL pin high and waits for any clock stretching peripherals.

SCLHigh:   SETB    SCLPin         ;Set SCL from our end.
           JNB     SCLPin,$       ;Wait for pin to actually go high.
           RET


; SendStop - sends an I2C stop, releasing the bus.

SendStop:  CLR     SDAPin         ;Get SDA ready for stop.
           ACALL   SCLHigh        ;Set clock for stop.
           ACALL   BitDly
           SETB    SDAPin         ;Send I2C stop.
           ACALL   BitDly
           CLR     I2CBusy        ;Clear I2C busy status.
           RET                    ;Bus should now be released.


; SendByte - sends one byte of data to an I2C slave device.
; Enter with:
;   ACC = data byte to be sent.

SendByte:  MOV     BitCnt,#8      ;Set bit count.

SBLoop:    RLC     A              ;Send one data bit.
           MOV     SDAPin,C       ;Put data bit on pin.
           ACALL   SCLHigh        ;Send clock.
           ACALL   BitDly
           CLR     SCLPin
           ACALL   BitDly
           DJNZ    BitCnt,SBloop  ;Repeat until all bits sent.

           SETB    SDAPin         ;Release data line for acknowledge.
           ACALL   SCLHigh        ;Send clock for acknowledge.
           ACALL   BitDly
           JNB     SDAPin,SBEX    ;Check for valid acknowledge bit.
           SETB    NoAck          ;Set status for no acknowledge.
SBEX:      CLR     SCLPin         ;Finish acknowledge bit.
           ACALL   BitDly
           RET


; GoMaster - sends an I2C start and slave address.
; Enter with:
;   SlvAdr = slave address.

GoMaster:  SETB    I2CBusy        ;Indicate that I2C frame is in progress.
           CLR     NoAck          ;Clear error status flags.
           CLR     BusFault
           JNB     SCLPin,Fault   ;Check for bus clear.
           JNB     SDAPin,Fault
           CLR     SDAPin         ;Begin I2C start.
           ACALL   BitDly
           CLR     SCLPin
           ACALL   BitDly         ;Complete I2C start.
           MOV     A,SlvAdr       ;Get slave address.
           ACALL   SendByte       ;Send slave address.
           RET

Fault:     SETB    BusFault       ;Set fault status
           RET                    ;  and exit.


; SendData - sends one or more bytes of data to an I2C slave device.
; Enter with:
;   ByteCnt = count of bytes to be sent.
;   SlvAdr  = slave address.
;   @R0     = data to be sent (the first data byte will be the 
;             subaddress, if the I2C device expects one).

SendData:  ACALL   GoMaster       ;Acquire bus and send slave address.
           JB      NoAck,SDEX     ;Check for slave not responding.

SDLoop:    MOV     A,@R0          ;Get data byte from buffer.
           ACALL   SendByte       ;Send next data byte.
           INC     R0             ;Advance buffer pointer.
           JB      NoAck,SDEX     ;Check for slave not responding.
           DJNZ    ByteCnt,SDLoop ;All bytes sent?

SDEX:      ACALL   SendStop       ;Done, send an I2C stop.
           RET


;RcvByte - receives one byte of data from an I2C slave device.
; Returns:
;   ACC = data byte received.

RcvByte:   MOV     BitCnt,#8      ;Set bit count.

RBLoop:    ACALL   SCLHigh        ;Read one data bit.
           ACALL   BitDly
           MOV     C,SDAPin       ;Get data bit from pin.
           RLC     A              ;Rotate bit into result byte.
           CLR     SCLPin
           ACALL   BitDly
           DJNZ    BitCnt,RBLoop  ;Repeat until all bits received.

           PUSH    ACC            ;Save accumulator
           MOV     A,ByteCnt
           CJNE    A,#1,RBAck     ;Check for last byte of frame.
           SETB    SDAPin         ;Send no acknowledge on last byte.
           SJMP    RBAClk

RBAck:     CLR     SDAPin         ;Send acknowledge bit.
RBAClk:    ACALL   SCLHigh        ;Send acknowledge clock.
           POP     ACC            ;Restore accumulator
           ACALL   BitDly
           CLR     SCLPin
           SETB    SDAPin         ;Clear acknowledge bit.
           ACALL   BitDly
           RET


;RcvData - receives sends one or more bytes of data from an I2C slave device.
; Enter with:
;   ByteCnt = count of bytes to be sent.
;   SlvAdr  = slave address.
; Returns:
;   @R0     = data received.

; Note: to receive with a subaddress, use SendData to set the subaddress
;   first (no provision for repeated start).

RcvData:   INC     SlvAdr         ;Set for READ of slave.
           ACALL   GoMaster       ;Acquire bus and send slave address.
           JB      NoAck,RDEX     ;Check for slave not responding.

RDLoop:    ACALL   RcvByte        ;Recieve next data byte.
           MOV     @R0,A          ;Save data byte in buffer.
           INC     R0             ;Advance buffer pointer.
           DJNZ    ByteCnt,RDLoop ;Repeat untill all bytes received.

RDEX:      ACALL   SendStop       ;Done, send an I2C stop.
           RET


;****************************************************************************
;                               Main Program
;****************************************************************************


Reset:     MOV     SP,#2Fh        ;Set stack to start at 30h.

           MOV     XmtDat,#0      ;Initialize transmit data area.
           MOV     XmtDat+1,#37h
           MOV     XmtDat+2,#0AAh
           MOV     XmtDat+3,#055h
           MOV     XmtDat+4,#33h
           MOV     XmtDat+5,#0CCh
           MOV     XmtDat+6,#0FFh
           MOV     XmtDat+7,#0BBh


TestLoop:  MOV     SlvAdr,#I2CIO  ;Write data to PCF8574 I/O expandor.
           MOV     R0,#XmtDat+2   ;Start of data.
           MOV     ByteCnt,#1     ;Send one data byte.
           ACALL   SendData

           MOV     SlvAdr,#I2CIO  ;Read back data from PCF8574 I/O expandor.
           MOV     R0,#AltRcv     ;Start of data.
           MOV     ByteCnt,#1     ;Read one data byte.
           ACALL   RcvData
           INC     XmtDat+2       ;Advance data to next value.

           MOV     SlvAdr,#I2CLED ;Write data to SAA1064 LED driver.
           MOV     R0,#XmtDat     ;Start of data.
           MOV     ByteCnt,#6     ;Send 6 bytes (subaddress, control, data).
           ACALL   SendData

           MOV     SlvAdr,#I2CRAM ;Write data to PCF8570 RAM.
           MOV     R0,#XmtDat     ;Start of data.
           MOV     ByteCnt,#8     ;Send 8 bytes (subaddress + 7 data bytes).
           ACALL   SendData

           MOV     SlvAdr,#I2CRAM ;Write subaddress to PCF8570 RAM.
           MOV     R0,#XmtDat     ;Start of data.
           MOV     ByteCnt,#1     ;Send one byte (subaddress).
           ACALL   SendData
           MOV     SlvAdr,#I2CRAM ;Read back data from PCF8570 RAM.
           MOV     R0,#RcvDat     ;Start of data.
           MOV     ByteCnt,#7     ;Read 7 data bytes.
           ACALL   RcvData

           AJMP    TestLoop       ;Repeat operation for scope watchers.

           END
