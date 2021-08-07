
;**********************************************************************
;   
;   A/D Test Program for the S87C752
;   COFFEY.ASM                                  CS=
;   2-12-90  4:41:52 pm                         K. COFFEY 
;**********************************************************************

;
; Signetics 87C752 Special Function Registers
;
P0      equ  80h       ;port 0
SP      equ  81h       ;stack pointer
DPL     equ  82h       ;data pointer low
DPH     equ  83h       ;data pointer high
PCON    equ  87h       ;power control
TCON    equ  88h       ;timer control
TL      equ  8ah       ;timer/counter low
RTL     equ  8bh       ;reload timer low
TH      equ  8ch       ;timer/counter high
RTH     equ  8dh       ;reload timer high
P1      equ  90h       ;port 1
I2CON   equ  98h       ;I2C control
I2DAT   equ  99h       ;I2C data
IE      equ  0a8h      ;interrupt enable
P3      equ  0b0h      ;port 3
PSW     equ  0d0h      ;program status word
I2CFG   equ  0d8h      ;I2C configuration
ACC     equ  0e0h      ;accumulator
B       equ  0f0h      ;B register
I2STA   equ  0f8h      ;I2C status
ADAT    equ  084h      ;A/D result
ADCON   equ  0a0h      ;A/D control

            LIMIT   07FFh           ;Sets limit of assembly.

; RAM locations used by A/D routines.


;**********************************************************************
;            Begin ; Interrupt Location 
;**********************************************************************
                
; Reset and interrupt vectors.

            AJMP    Reset           ;Reset vector at address 0.
            ORG     02Bh            ;Set ROM address to 02B.
            AJMP    ADInt           ;Jump to A/D interrupt routine.

;*********************************************************************
;                    A/D Interrupt Routine
;*********************************************************************
            ORG     040h            ;Set origin to 40
ADInt:      SETB    P0.2            ;Toggle interrupt bit.
            CLR     P0.2            ;                              
            ORL     ADCON,#08h      ;Start conversion of AD1.
ADQuit:     RETI                    ;Return from A/D routine.
                                                           
;**********************************************************************
;                    Main Program Routines
;**********************************************************************

Init:       SETB    IE.7            ;Enable global interrupts.
            SETB    IE.6            ;Enable A/D interrupt.
            RET                     ;Return from intitializing Routine.
          
;**********************************************************************
;                        Main Program
;**********************************************************************

Reset:      MOV     P0,#0FFh        ;Insure P0 are all high.
            ACALL   Init            ;Initialize system.
            CLR     P0.3            ;Pull sensor ground low.
            MOV     P1,#0FFh        ;Set Port1 for special function.       
            MOV     ADCON,#20h      ;Load the A/D setup to ACC.
            MOV     ADCON,#28h      ;Send A/D setup to ADCON.
Loop:       SETB    P0.0            ;A/D control display trigger. 
            MOV     P3,ADCON        ;Display A/D control on Port 3.
            CLR     P0.0            ;
            SETB    P0.1            ;A/D data display trigger.
            MOV     P3,ADAT         ;Display A/D data on Port 3.
            CLR     P0.1            ;
            AJMP    Loop            ;Loop main program.
