
;**************************************************************************

;                       Software Simplex UART Routines
;                for 83C751 and 83C752 series Microcontrollers

;                          Signetics Corporation

;**************************************************************************

$Title(Software Driven Simplex UART Routines)
$Date(06/27/89)
$MOD751
$Debug

;**************************************************************************

BaudVal  EQU       -139                ;Timer value for 9600 baud @ 16 MHz.
                                       ;(one bit cell time)
StrtVal  EQU       -39                 ;Timer value to start receive.
                                       ;(half of one bit cell time, minus the
                                       ; time it takes the code to sample RxD)

XmtDat   DATA      10h                 ;Data for RS-232 transmit routine.
RcvDat   DATA      11h                 ;Data from RS-232 receive routine.
BitCnt   DATA      12h                 ;RS-232 transmit & receive bit count.
LoopCnt  DATA      13h                 ;Loop counter for test routine.

Flags    DATA      20h
TxFlag   BIT       Flags.0             ;Receive-in-progress flag.
RxFlag   BIT       Flags.1             ;Transmit-in-progress flag.
RxErr    BIT       Flags.2             ;Receiver framing error.
RcvRdy   BIT       Flags.3             ;Receiver ready flag.

TxD      BIT       P1.0                ;Port bit for RS-232 transmit.
RxD      BIT       P1.5                ;Port bit for RS-232 receive (INT0).

;**************************************************************************

; Interrupt Vectors

         ORG       0                   ;Reset vector.
         AJMP      Reset

         ORG       03H                 ;External interrupt 0.
         AJMP      Int0                ;Indicates RS-232 start bit received.

         ORG       0BH                 ;Timer 0 interrupt.
         AJMP      Timr0               ;Baud rate generator.

         ORG       13H                 ;External interrupt 1 (not used).
         RETI

         ORG       1BH                 ;Timer I interrupt (not used).
         RETI

         ORG       23H                 ;I2C interrupt (not used).
         RETI

;*****************************************************************************

;Simple test of RS-232 transmit and receive.

Reset:   MOV       SP,#30h
         MOV       Flags,#0            ;Clear RS-232 flags.
         CLR       RxFlag
         MOV       TCON,#00h           ;Set up timer controls.
         MOV       IE,#82h             ;Enable timer 0 interrupts.

         MOV       LoopCnt,#16         ;Test transmit first.
         MOV       R1,#0               ;Zero line count.
         MOV       DPTR,#Msg1          ;Point to message string.
Loop1:   ACALL     Mess                ;Send an RS-232 message repeatedly.
         MOV       A,#':'
         ACALL     XmtByte
         MOV       A,R1
         ACALL     PrByte              ;Print R1 contents.
         INC       R1                  ;Advance R1 value.
         DJNZ      LoopCnt,Loop1

Loop2:   SETB      EX0                 ;Enable interrupt 0 (RS-232 receive).
         JNB       RcvRdy,$            ;Wait for data available.
         CLR       RcvRdy
         MOV       A,RcvDat            ;Echo same byte.
         ACALL     XmtByte
         SJMP      Loop2


; Send a byte out RS-232 and wait for completion before returning.
; (use if there is nothing else to do while RS-232 is busy)

XmtByte: JB        RxFlag,$            ;Wait for receive complete.
         ACALL     RSXmt               ;Send ACC to RS-232 output.
         JB        TxFlag,$            ;Wait for transmit complete.
         RET


; Begin RS-232 transmit.

RSXmt:   MOV       XmtDat,A            ;Save data to be transmitted.
         MOV       BitCnt,#10          ;Set bit count.
         MOV       TH,#High BaudVal    ;Set timer for baud rate.
         MOV       TL,#Low BaudVal
         MOV       RTH,#High BaudVal   ;Also set timer reload value.
         MOV       RTL,#Low BaudVal
         SETB      TR                  ;Start timer.
         CLR       TxD                 ;Begin start bit.
         SETB      TxFlag              ;Set transmit-in-progress flag.
         RET


; Timer 0 timeout: RS-232 receive bit or transmit bit.

Timr0:   PUSH      ACC
         PUSH      PSW
         JB        RxFlag,RxBit        ;Is this a receive timer interrupt?
         JB        TxFlag,TxBit        ;Is this a transmit timer interrupt?
T0Ex1:   CLR       TR                  ;Stop timer.
T0Ex2:   POP       PSW
         POP       ACC
         RETI


; RS-232 transmit bit routine.

TxBit:   DJNZ      BitCnt,TxBusy       ;Decrement bit count, test for done.
         CLR       TxFlag              ;End of stop bit, release timer.
         SJMP      T0Ex1               ;Stop timer and exit.

TxBusy:  MOV       A,BitCnt            ;Get bit count.
         CJNE      A,#1,TxNext         ;Is this a stop bit?
         SETB      TxD                 ;Set stop bit.
         SJMP      T0Ex2               ;Exit.

TxNext:  MOV       A,XmtDat            ;Get data.
         RRC       A                   ;Advance to next bit.
         MOV       XmtDat,A
         MOV       TxD,C               ;Send data bit.
         SJMP      T0Ex2               ;Exit.


;Begin RS-232 receive (after external interrupt 0).

Int0:    MOV       BitCnt,#10          ;Set receive bit count.
         MOV       TH,#High StrtVal    ;First timeout in HALF a bit time.
         MOV       TL,#Low StrtVal
         MOV       RTH,#High BaudVal   ;Set timer reload for baud rate.
         MOV       RTL,#Low BaudVal
         MOV       RcvDat,#0           ;Initialize received data to 0.
         CLR       EX0                 ;Disable external interrupt 0.
         CLR       RxErr               ;Clear error flag.
         SETB      TR                  ;Start timer.
         SETB      RxFlag              ;Set receive-in-progress flag.
         RETI


; RS-232 receive bit routine.

RxBit:   DJNZ      BitCnt,RxBusy       ;Decrement bit count, test for stop.
         JB        RxD,RxBitEx         ;Valid stop bit?
RxBtErr: SETB      RxErr               ;Bad stop bit, tell mainline.
RxBitEx: CLR       RxFlag              ;Release timer for other purposes.
         SETB      EX0                 ;Re-enable external interrupt 0.
         SETB      RcvRdy              ;Tell mainline that a byte is ready.
         SJMP      T0Ex1               ;Stop timer and exit.

RxBusy:  MOV       A,BitCnt            ;Get bit count.
         CJNE      A,#9,RxNext         ;Is this a start bit?
         JB        RxD,RxBtErr         ;Valid start bit?
         SJMP      T0Ex2               ;Exit.

RxNext:  MOV       A,RcvDat            ;Get partial receive byte.
         MOV       C,RxD               ;Get receive pin value.
         RRC       A                   ;Shift in new bit.
         MOV       RcvDat,A            ;Save updated receive byte.
         SJMP      T0Ex2               ;Exit.


; Print byte routine: print ACC contents as ASCII hexadecimal.

PrByte:  PUSH   ACC
         SWAP   A
         ACALL  HexAsc
         ACALL  XmtByte
         POP    ACC
         ACALL  HexAsc            ;Print nibble in ACC as ASCII hex.
         ACALL  XmtByte
         RET


; Hexadecimal to ASCII conversion routine.

HexAsc:  ANL    A,#0FH            ;Convert a nibble to ASCII hex.
         JNB    ACC.3,NoAdj
         JB     ACC.2,Adj
         JNB    ACC.1,NoAdj
Adj:     ADD    A,#07H
NoAdj:   ADD    A,#30H
         RET


; Message string transmit routine.

Mess:    PUSH      ACC
         MOV       R0,#0               ;R0 is character pointer (string
Mesl:    MOV       A,R0                ; length is limited to 256 bytes).
         MOVC      A,@A+DPTR           ;Get byte to send.
         CJNE      A,#0,Send           ;End of string is indicated by a 0.
         POP       ACC
         RET

Send:    ACALL     XmtByte             ;Send a character.
         INC       R0                  ;Next character.
         SJMP      Mesl

Msg1:    DB        0Dh, 0Ah
         DB        'This is a test of the software RS-232 transmit routine'
         DB        0

         END
                                                                                                         