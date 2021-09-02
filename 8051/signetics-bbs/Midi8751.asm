;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                          ;
;               CODE FOR MIDI TO RS-232 CONVERTER.  ALLOWS THE READING     ;
;               OF INCOMING MIDI MESSAGES AND DISPLAYS ON THE TERMINAL     ;   
;               THE BINARY BYTE, AS WELL AS WHICH NOTE IS PLAYED.          ;
;               THE 8751 TRANSMIT MUST GO THROUGH A MAXIM -MAX232CPE
;               CONVERTER CHIP.  IT'S A SIMPLE CHIP TO USE. THE MIDI       ;
;               IS HOOKED STRAIGHT IN.  CLOCK PIN IS PIN 5, GROUND IS      ;
;               PIN 2, AND +5V IS PIN 4.  WHILE IT IS POSSIBLE TO GET      ;
;               ENOUGH POWER FROM THE MIDI LINE, I SUGGEST A 7805 INSTEAD. ;
;               CODE BY ROBERT ADRIAN.  SECTIONS c.1993 ADRIONICS.         ;
;               USE FOR NON-PROFIT APPLICATION ONLY.                       ;    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;       REGISTER # ;;  USED FOR:                                           ;
;          0           BYTE INC AND STORAGE                                ;
;          1           NOT USED                                            ;
;          2           NOT USED                                            ;
;          3           NOT USED                                            ;
;          4           NOT USED                                            ;
;          5           NOT USED                                            ;
;          6           NOT USED                                            ;
;          7           BIT COUNT REGISTER FOR BIT CONVERSION               ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       TO USE THE BYTE-BIT FUNCTION PASS THE VALUE IN             *DATAOUT*
;       TO USE THE SERIAL_INFORMATION FUNCTION PASS THE VALUE IN   *SEND*
;       --------------------------------------------------------   *********
;       --------------------------------------------------------   *********
;       --------------------------------------------------------   *********
;       --------------------------------------------------------   *********
;       --------------------------------------------------------   *********
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                   ;
;        Interrupt Vectors          ;
;                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                    ;
         ORG      0000H             ; Power on / RE_SET
         JMP      BODY_O_CODE       ;
                                    ;
         ORG      0003H             ; External Interrupt 0 (pin 12)
         JMP      XINT0             ;
                                    ;
         ORG      000BH             ; Timer 0 Overflow
         JMP      TMR0              ;
                                    ;
         ORG      0013H             ; External Interrupt 1 (pin 13)
         JMP      XINT1             ;
                                    ;
         ORG      001BH             ; Timer 1 Overflow
         JMP      TMR1              ;
                                    ;
         ORG      0023H             ; Serial Communications (TX and RX)
         JMP      SERIAL            ;
                                    ;



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                   ;
;        Timer 0                    ;
;                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                   ;
; This is the routine for handling  ;
; overflow events from Timer 0.     ;
;                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                    ;
TMR0:                               ;
         RETI                       ; Does nothing.
                                    ;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                   ;
;        Timer 1                    ;
;                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                   ;
; This is the routine for handling  ;
; overflow events from Timer 1.     ;
;                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                    ;
TMR1:                               ;
         RETI                       ; Does nothing.
                                    ;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                   ;
;        Interrupt  0               ;
;                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                   ;
; This is the routine for handling  ;
; interrupts on channel 0.          ;
;                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                    ;
XINT0:                              ;
         RETI                       ; Does nothing.
                                    ;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                   ;
;        Interrupt 1                ;
;                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                   ;
; This is the routine for handling  ;
; interrupts an channel 1.          ;
;                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                    ;
XINT1:                              ;
         RETI                       ; Does nothing.
                                    ;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                   ;
;        Serial I/O                 ;
;                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                   ;
; This is the routine for handling  ;
; serial communications.  It has to ;
; distinguish between transmit and  ;
; receive events by polling SCON.   ;
; We use SCON.0 and SCON.1 to refer ;
; to receive and transmit events    ;
; because the assembler doesn't     ;
; handle TI properly.               ;
;                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                    ;
SERIAL:                             ;
         JB       SCON.1,TXEVENT    ; Revector on transmit events
                                    ;
         JB       SCON.0,RXEVENT    ; Revector on receive events
                                    ;
SERR:                               ;
         JMP      SERR              ; This is an error condition
                                    ;
TXEVENT:                            ;
         CLR    TI
         CLR    TFLAG
         RETI                       ; Does nothing.
                                    ;
RXEVENT:                            
         CLR    RI
         SETB   RFLAG
         RETI                       ; Does nothing.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

INITIALIZE:
BSEG    AT      00H
                BSEG
                TFLAG:          DBIT    1
                RFLAG:          DBIT    1
                STATUS_FLAG:    DBIT    1
CSEG
                DATAOUT EQU     7CH
                SEND    EQU     7BH                
                
                PCON    EQU     87H
                TICKS   EQU     7AH
                TWDLS   EQU     79H     
                BAUD    EQU     78H     
                                        ; 230=2400 BAUD

                MOV     TICKS,#198          
                MOV     TWDLS,#50       ; NUMBER = MILLI SEC
                MOV     BAUD,#254       ; BAUD RATE =31.25K
                MOV     PCON,#128       ; SET UP SMOD, CLEAR GF?,PD,ID
                MOV     SCON,#080       ; SET UP SERIAL PORT
                MOV     TMOD,#020H      ; SET UP TIMER 1 FOR BAUD GENERATOR
                
                MOV     P0,#0FFH
                MOV     P1,#0FFH
                MOV     P2,#0FFH
                MOV     P3,#0FFH

                SETB    ES
                SETB    EA
                SETB    TR1     ; START THE TIMER
                RET
                    

TEST:
        MOV     TH1,#230        ; BAUD =2400 BPS
        MOV     DPTR,#TEST_MESSAGE
INNER_TEST:
        MOV     A,#0
        MOVC    A,@A+DPTR
        CJNE    A,#0,PASS
                MOV     DATAOUT,#10100101B
                CALL    BIT_OUT
        MOV     TH1,BAUD
        RET
PASS:
        MOV     SEND,A
        CALL    SERIAL_OUT
        INC     DPTR
        JMP     INNER_TEST

BODY:
        SETB    STATUS_FLAG
        MOV     TH1,BAUD
        MOV     R0,#7DH         ; SEE BELOW FOR BYTE STORAGE ADDRESSES
INNER_BODY:        
        JNB     RFLAG,$         ; LOOP UNTIL BYTE RECIEVED
        MOV     A,SBUF          ; BYTE IN
        CLR     RFLAG           ; RESET RECIEVE FLAG
        JNB     STATUS_FLAG,DATA_VALID
        CJNE    A,#10010000B,INNER_BODY ; WAIT UNTIL STATUS BYTE IS VALID    
DATA_VALID:        
        CLR     STATUS_FLAG
        MOV     @R0,A           ; STORE THE BYTE IN VALUE OF R0
        INC     R0
        CJNE    R0,#80H,INNER_BODY
;;;;;;;;;;;;;;;;;;;;;;;;            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                       7DH = BYTE 1                                      ;
;                       7EH = BYTE 2                                      ;
;                       7FH = BYTE 3                                      ;
        RET                                                               ; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;; PASS A BYTE TO BE DISPLAYED IN DATAOUT;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;      THEN CALL THE FUNCTION BIT_OUT   ;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DISPLAY:
        MOV     DATAOUT,7DH     ; BYTE 1       ; to show binary data stream
        CALL    BIT_OUT                        ; remove all the ';' before
        CALL    SPACE                          ; the code in display.
        MOV     DATAOUT,7EH     ; BYTE 2       ;
        CALL    BIT_OUT                        ;
        CALL    SPACE                          ;
        MOV     DATAOUT,7FH     ; BYTE 3       ;
        CALL    BIT_OUT                        ;
        CALL    SPACE                          ;
        CALL    SPACE                          ;
        
                MOV     DATAOUT,7EH     ; BYTE 2
                CALL    NOTE_OUT
                MOV     DATAOUT,7EH
                CALL    NOTE_SIGN
        
        RET


NOTE_SIGN:
        MOV     A,DATAOUT
        MOV     B,#2
        MUL     AB
        SUBB    A,#1
FLAT:        
        MOV     DPTR,#NOTE_CHART
        MOVC    A,@A+DPTR
        MOV     SEND,A
        CALL    SERIAL_OUT
        RET
NOTE_OUT:
        MOV     A,DATAOUT
        MOV     B,#2
        MUL     AB
        SUBB    A,#2
        MOV     DPTR,#NOTE_CHART
        MOVC    A,@A+DPTR
        MOV     SEND,A
        CALL    SERIAL_OUT
        RET

SPACE:
        MOV     TH1,#230
        MOV     SEND,#13
        CALL    SERIAL_OUT
        MOV     SEND,#10
        CALL    SERIAL_OUT
        MOV     TH1,BAUD
        RET


BIT_OUT:
        MOV     TH1,#230        ; SET FOR 2400 BPS
        MOV     R7,#8
DECODE:
        MOV     A,DATAOUT
        RLC     A
        MOV     DATAOUT,A
        JC      BIT_ONE     
        MOV     SEND,#'0'
RETT:
        CALL    SERIAL_OUT
        DJNZ    R7,DECODE
        CALL    SPACE
        RET

BIT_ONE:
        MOV     SEND,#'1'
        JMP     RETT

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SERIAL_OUT:
        MOV     TH1,#230
        SETB    TFLAG
        MOV     A,SEND
        MOV     SBUF,A ; ANYTHING MOVED INTO SBUF, STARTS TRANSMIT
        JB      TFLAG,$
        RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MILLI:                                  ;
        PUSH    ACC                     ; (024) Save Accumulator
                                        ;
        MOV     A,TICKS                 ; (024) Initialize loop
MLOOP:                                  ;
        NOP                             ; (012)
        NOP                             ; (012)
        NOP                             ; (012)
        DJNZ    ACC,MLOOP               ; (024)
                                        ;
        POP     ACC                     ; (024) Restore Accumulator
        RET                             ; (048) (Includes CALL)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                       ;
;       Long Delay                      ;
;                                       ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                       ;
; This routine waits a long time.  How  ;
; long it waits is determined by the    ;
; value of TWDLS.  (It's that many      ;
; milliseconds plus a little overhead.) ;
;                                       ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                        ;
TWIDDLE:                                ;
        PUSH    ACC                     ; Save the accumulator
        MOV     A,TWDLS                 ; Load the Accumulator
LOOP:                                   ;
        CALL    MILLI                   ; Wait 1 millisecond
        DJNZ    ACC,LOOP                ; Done yet?
                                        ;
        POP     ACC                     ; Yes, restore the accumulator
        RET                             ; 



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                       ;
;       Wait a second!                  ;
;                                       ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                        ;
SECOND:                                 ;
        PUSH    ACC                     ;
        MOV     A,#250                  ;
SL1:    CALL    MILLI                   ;
        CALL    MILLI                   ;
        CALL    MILLI                   ;
        CALL    MILLI                   ;
        DJNZ    ACC,SL1                 ;
                                        ;
        POP     ACC                     ;        
        RET                             ;



BODY_O_CODE:

        CALL    INITIALIZE

        CALL    TEST

MAIN:        
        CALL    BODY
        CALL    DISPLAY
        JMP     MAIN

NOTE_CHART:

        DB      'C#D D#E F F#G G#A A#B C ' 
        DB      'C#D D#E F F#G G#A A#B C ' 
        DB      'C#D D#E F F#G G#A A#B C ' 
        DB      'C#D D#E F F#G G#A A#B C ' 
        DB      'C#D D#E F F#G G#A A#B C ' 
        DB      'C#D D#E F F#G G#A A#B C ' 
        DB      'C#D D#E F F#G G#A A#B C ' 




TEST_MESSAGE:
        DB      ' HELLO WORLD '
        DB      0
END
��������������������������������
