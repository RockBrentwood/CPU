*-----------------------------------------------------------------------*
*   Multiply 16 by 16 bit unsigned integer routine for the 6801/6811.
*
*       1/23/87
*       D. G. Weiss
*       MCU Technical Marketing
*       Motorola Microprocessor Group
*       Oak Hill, TX
*
*    Accepts:
*       Multiplier:    Mpr[1..0]
*       Multiplicand:  Mpd[1..0]
*    Yields:
*       Product:       Prod[3..0]
*    Time used: 114 cycles to execute (absolute addressing)
*-----------------------------------------------------------------------*
	
        ORG  $00       Beginning of RAM
	
mpr     rmb  2          Multiplier
mpr1    equ  mpr           "          byte 1 (high order)
mpr0    equ  mpr+1         "          byte 0 (low order)

mpd     rmb  2          Multiplicand
mpd1    equ  mpd           "          byte 1 (high order)
mpd0    equ  mpd+1         "          byte 0 (low order)

prod    rmb  4          Product
prod3   equ  prod          "          byte 3 (high order)
prod2   equ  prod+1
prod1   equ  prod+2
prod0   equ  prod+3        "          byte 0 (low order)

CTR     RMB  1          Iteration counter

        ORG  $10

mpy16   EQU  *
        ldd     #0
        std     prod3
        lda     mpr0
        ldb     mpd0
        mul
        std     prod1
        lda     mpr0
        ldb     mpd1
        mul
        addd    prod2
        std     prod2
        lda     mpr1
        ldb     mpd0
        mul
        addd    prod2
        std     prod2
        rol     prod3
        lda     mpr1
        ldb     mpd1
        mul
        addd    prod3
        std     prod3
        rts
