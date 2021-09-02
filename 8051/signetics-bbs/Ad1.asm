
;COLLINS ASSOC.
;83C552 CODE
;JUNE 1, 1988

.RAMCHK  OFF

        ABIT:   EQU     01H             ;USER DEFINED FLAG
        ADCON:  EQU     0C5H
        ADCB:   EQU     0C6H            ;ADCH REGISTER
        EAD:    EQU     0AEH

        ORG     0
        JMP     PWRON

        ORG     53H
        JMP     ATOD

PWRON:
        CLR     EA
        CLR     ABIT
        CLR     IT0
        CLR     IT1
        CLR     A
        SETB    EX1
        SETB    EX0
        SETB    EAD
        CLR     P3.0
        MOV     R0,#30H
        MOV     R1,#00H
        SETB    EA
        NOP
        NOP
        MOV     ADCON,#00H              ;CHANNEL 0 @ P5.0
TESTAD:
        ORL     ADCON,#08H
        SETB    ABIT
        JB      ABIT,$
        MOV     ADCON,#00H              ;NOT NECESSARY
        JMP     TESTAD
ATOD:
        CLR     EA
        MOV     A,ADCB
        MOV     P1,A                    ;CONV RSLT @ P1
        CLR     ABIT
        SETB    EA
        RETI
.END
�������
