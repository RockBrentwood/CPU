
;**************************************************************************

;                 87C752 A/D and PWM Demonstration Program

;  This program first reads all five A/D channels and outputs the values in
;  hexadecimal as RS-232 data.  It then programs the PWM output to reflect 
;  the value on A/D channel 0, and again outputs the A/D value to RS-232.
;  Note that the A/D value is inverted before being moved to the PWM 
;  compare register in order to compensate for the inversion on the PWM 
;  output pin.  This process is repeated continuously. 

;  Thus, a voltage may be applied to ADC0 (P1.0, pin 13) to vary the PWM 
;  pulse width.  

;  A simple test of this function is to measure the voltage on ADC0 and PWM 
;  with a voltmeter.  The meter should integrate the waveform on PWM and 
;  read a voltage within about 20mV of that on ADC0. 

;  The RS-232 output appears on Port 1 pin 5, which must be buffered with
;  an MC1488 or perhaps a MAX232 chip prior to being connected to a 
;  terminal.  The transmission rate with a 16MHz crystal will be 9600 baud.

;**************************************************************************

$Title(87C752 A/D and PWM Test)
$Date(07/26/89)
$MOD752

;**************************************************************************

BaudVal  EQU       -139                ;Timer value for 9600 baud @ 16 MHz.
                                       ;(one bit cell time)

XmtDat   DATA      10h                 ;Data for RS-232 transmit routine.
BitCnt   DATA      12h                 ;RS-232 transmit bit count.

Flags    DATA      20h
TxFlag   BIT       Flags.0             ;Transmit-in-progress flag.
ADFlag   BIT       Flags.1             ;A/D conversion complete flag.

TxD      BIT       P1.5                ;Port bit for RS-232 transmit.

;**************************************************************************

; Interrupt Vectors

         ORG       0                   ;Reset vector.
         AJMP      Reset

         ORG       0BH                 ;Timer 0 interrupt.
         AJMP      Timr0               ;Baud rate generator.

         ORG       2Bh                 ;A/D conversion complete interrupt.
         SETB      ADFlag              ;Tell mainline.
         RETI

;*****************************************************************************

Reset:   MOV       SP,#30h
         MOV       Flags,#0            ;Clear RS-232 flags.
         MOV       TCON,#00h           ;Set up timer controls.
         MOV       IE,#82h             ;Enable timer 0 interrupt.

         MOV       DPTR,#Msg1          ;Point to message string.
         ACALL     Mess                ;Send message.

         MOV       R1,#0               ;Start with A/D channel 0.
Loop1:   MOV       A,R1
         ACALL     ADConv              ;Start A/D conversion.
         MOV       R2,A

         MOV       DPTR,#Msg2          ;Point to message string.
         ACALL     Mess                ;Send message.
         MOV       A,R1
         ACALL     PrByte              ;Print channel #.
         MOV       DPTR,#Msg3          ;Point to message string.
         ACALL     Mess                ;Send message.

         MOV       A,R2
         ACALL     PrByte              ;Print A/D value.
         INC       R1                  ;Advance R1 value.
         CJNE      R1,#5,Loop1
         MOV       DPTR,#CRLF          ;Point to message string.
         ACALL     Mess

; Now use A/D channel 0 value to control the PWM.

         MOV       IE,#0C2h            ;Now enable the A/D interrupt.
         MOV       PWMP,#80h           ;Set up PWM prescaler.
         MOV       PWCM,#0             ;Set initial PWM value.
         MOV       PWENA,#01h          ;Start PWM.
Loop2:   MOV       A,#0                ;Read A/D channel 0.
         ACALL     ADStart             ;Start A/D conversion.
         JNB       ADFlag,$            ;Wait for A/D conversion complete.
         MOV       A,ADAT
         CPL       A                   ;Complement the value for the PWM.
         MOV       PWCM,A              ;Set PWM value.
         ACALL     PrByte              ;Print PWM value.
         MOV       DPTR,#Msg4          ;Point to message string.
         ACALL     Mess
         SJMP      Loop2


; A/D Conversion Routines.
;   The following shows two ways to use the A/D. Both routines are used by 
;   different portions of the sample program.

;   Method 1: This version of the routine starts the conversion and then 
;   returns. The mainline program can detect when the conversion is 
;   complete by checking the A/D conversion complete flag (ADFlag) which is 
;   set by the A/D interrupt service routine. A/D data must be read by the 
;   calling routine.

ADStart: CLR       ADFlag              ;Clear A/D conversion complete flag.
         ORL       A,#28h              ;Add control bits to channel #.
         MOV       ADCON,A             ;Start conversion.
         RET


;   Method 2: This is an alternative version of the A/D routine which 
;   starts the conversion and then waits for it to complete before 
;   returning. A/D data is returned in the ACC.

ADConv:  ORL       A,#28h              ;Add control bits to channel #.
         MOV       ADCON,A             ;Start conversion.
ADC1:    MOV       A,ADCON
         JNB       ACC.4,ADC1          ;Wait for conversion complete.
         MOV       A,ADAT              ;Read A/D.
         RET


; Send a byte out RS-232 and wait for completion before returning.

XmtByte: ACALL     RSXmt               ;Send ACC to RS-232 output.
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

Msg1:    DB        0Dh, 0Ah, 'This is a test of the 87C752 A/D and PWM.'
CRLF:    DB        0Dh, 0Ah, 0

Msg2:    DB        0Dh, 0Ah, 'A/D Channel ', 0

Msg3:    DB        ' = ', 0

Msg4:    DB        '  ', 0

         END
������������������������������������
