           ;*******************************************************;
           ;                                                       ;
           ;                         87C751                        ;
           ;                SERIAL STREAM GENERATOR                ;
           ;                                                       ;
           ;                 Signetics Corporation                 ;
           ;                                                       ;
           ;                     AUGUST 1988                       ;
           ;*******************************************************;

$DEBUG
$MOD51

         BLOCK_END    DATA   022H
         BLOCK_START  DATA   023H

;******************************************************************************
;                               INTERRUPT VECTORS
;******************************************************************************

         ORG    00H                             ;
                AJMP    RESET                   ;

         ORG    03H                             ;EXTERNAL INTERRUPT 0
                RETI                            ;

         ORG    0BH                             ;
                RETI
    
         ORG    13H                             ;
                RETI                            

         ORG    1BH                             ;
                RETI

         ORG    23H                             ;SERIAL PORT INTERRUPT
                JMP     SEND_NXT                ;SERVICE INTERRUPT
    
    
;*****************************************************************************
;                               RESET ROUTINE
;*****************************************************************************

RESET: 
                JB      P3.2,HI_BAUD            ;HIGHER BAUD RATES?
LO_BAUD:        JB      P3.3,BAUD_2400     
BAUD_1200:      MOV     TH1,#0E8H               ;TIMER RELOAD VALUE, 1.2K  
                JMP     DATA_TYPE
BAUD_2400:      MOV     TH1,#0F4H               ;TIMER RELOAD VALUE, 2.4K 
                JMP     DATA_TYPE

HI_BAUD:        JB      P3.3,BAUD_9600
BAUD_4800:      MOV     TH1,#0FAH               ;TIMER RELOAD VALUE, 4.8K  
                JMP     DATA_TYPE
BAUD_9600:      MOV     TH1,#0FDH               ;TIMER RELOAD VALUE, 9.6K 

DATA_TYPE:      JB      P3.4,UNI_DATA
VARI_DATA:      MOV     BLOCK_START,#BLOCK_START_1
                MOV     BLOCK_END,#BLOCK_END_1
                JMP     SEND_DATA
UNI_DATA:       MOV     BLOCK_START,#BLOCK_START_2
                MOV     BLOCK_END,#BLOCK_END_2

SEND_DATA:      MOV     SCON,#040H              ;SERIAL MODE 1, 10 BIT MODE
                MOV     TMOD,#22H               ;TIMER 1, MODE 1
                MOV     TCON,#40H               ;RUN 
                MOV     IE,#90H                 ;ENABLE SERIAL INTERRUPT
                MOV     DPL,BLOCK_START
                MOV     DPH,#00H

;send first byte          
                JB      P1.0,$                  ;IF RTS IS UN-ASSERTED, WAIT
                CLR     TI                      ;CLEAR TRANSMIT INTERRUPT FLAG
                MOV     A,#00H
                MOVC    A,@A+DPTR
                MOV     SBUF,A                  ;
                INC     DPL
                JMP     WAIT                   

SEND_NXT:       JB      P3.5,NO_HOLD            ;IF PIN IS LOW, WAIT
                JNB     P3.5,$                  ;DON'T SEND ANY MORE BYTES
                ACALL   XRETI                   ;ENABLE SUBSEQUENT INTERRUPTS
                POP     ACC                     ;DISCARD INTERRUPT RETURN ADDR
                POP     ACC                     ;
                JMP     RESET                   ;POLL SET UPS AGAIN

NO_HOLD:        JB      P1.0,$                  ;IF RTS IS NEGATED, WAIT HERE 
                CLR     TI                      ;CLEAR TRANSMIT INTERRUPT FLAG
                MOV     A,#00H
                MOVC    A,@A+DPTR
                MOV     SBUF,A                  ;
                MOV     A,DPL
                CJNE    A,BLOCK_END,CYCLE
                MOV     DPL,BLOCK_START
                RETI
CYCLE:          INC     DPL
                RETI
 
WAIT:           JMP     $                       ;WAIT FOR NEXT INTERRUPT
;

XRETI:          RETI

BLOCK_START_1:  DB 055H
                DB 055H
BLOCK_END_1:    DB 055H
BLOCK_START_2:  DB 021H
                DB 022H
                DB 023H
                DB 024H
                DB 025H
                DB 026H
                DB 027H
                DB 028H
                DB 029H
                DB 02AH
                DB 02BH
                DB 02CH
                DB 02DH
                DB 02EH
                DB 02FH
                DB 030H
                DB 031H
                DB 032H
                DB 033H
                DB 034H
                DB 035H
                DB 036H
                DB 00DH
                DB 00AH
BLOCK_END_2:    DB 037H


END

����������������������������������������
