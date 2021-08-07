*-----------------------------------------------------------------------*
*   Multiply 16 by 16 bit unsigned integer routine for the 6811.
*
*       1/23/87
*       D. G. Weiss
*       MCU Technical Marketing
*       Motorola Microprocessor Group
*       Oak Hill, TX
*
*    Accepts:
*       Multiplier:    Register D
*       Multiplicand:  Y^[0..1]
*    Yields:
*       Product:       X^[0..3]
*-----------------------------------------------------------------------*
        ORG  $00        Beginning of RAM

mpr     rmb  2          Multiplier Scratch Locn.
mpr1    equ  mpr           "          byte 1 (high order)
mpr0    equ  mpr+1         "          byte 0 (low order)

mpd     equ  0          Multiplicand Offsets from Y Reg.
mpd1    equ  mpd           "          byte 1 (high order)
mpd0    equ  mpd+1         "          byte 0 (low order)

prod    equ  0          Product Offsets from X Reg.
prod3   equ  prod          "          byte 3 (high order)
prod2   equ  prod+1
prod1   equ  prod+2
prod0   equ  prod+3        "          byte 0 (low order)

CTR     RMB  1          Iteration counter

        ORG  $10

mpy16   EQU  *
        std     mpr
        ldd     #0
        std     prod3,X
        lda     mpr0
        ldb     mpd0,Y
        mul
        std     prod1,X
        lda     mpr0
        ldb     mpd1,Y
        mul
        addd    prod2,X
        std     prod2,X
        lda     mpr1
        ldb     mpd0,Y
        mul
        addd    prod2,X
        std     prod2,X
        rol     prod3,X
        lda     mpr1
        ldb     mpd1,Y
        mul
        addd    prod3,X
        std     prod3,X
        rts
