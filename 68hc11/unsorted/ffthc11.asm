* fft.c11 - fast fourier transform for the MC68HC11
*        written by:
*
*        Ron Williams
*        Department of Chemistry
*        Ohio University
*        Athens, OH 45701
*
* This is a modification of the 6800 FFT presented by:
*      Richard Lord
*      Byte Magazine, pp. 108-119
*      February 1979
*
* My version is written in ROMable code for the HC11. 
* It uses a sine look-up table for speed and can only 
* transform 256 8-bit data points.  The program 
* assumes that the address of the real data is pushed 
* on the stack prior to the call and that a 256 byte 
* imaginary buffer is at data+256 therefore you must 
* declare a 512 byte data array in the calling routine 
* and load the lower 256 bytes with data.  The FFT 
* will zero out the imaginary portion.  Also note that 
* the FFT uses memory in the stack RAM for its dynamic 
* variables and the FFT returns a value on the stack 
* which contains the number of times the data was 
* divided by 2 during transform. 
* 
* As mentioned in Lord's article, "power" spectra can 
* be computed using the sum of absolute values routine 
* included at the end of the FFT. Simply change the 
* beq done at the end of the FFT to beq smsq. 
* 
* note - this copy has been modified to use $DD00 for 
* data because this is easiest with BUFFALO
* I have timed this transform on some test data.  The 
* results are an impressive 350 milliseconds per 
* transform including the "power" spectra computation. 
* 
* Please note that the origin of this code may require 
* adjustment for your specific memory map. 
* 
* Please let me know of any bugs you find.

return  equ     0
real    equ     2
celnm   equ     4
celct   equ     5
pairnm  equ     6
celdis  equ     7
delta   equ     8
sclfct  equ     9
cosa    equ     $0A
sina    equ     $0B
sinpt   equ     $0C
real1   equ     $0E
real2   equ     $10
treal   equ     $12
timag   equ     $13
tmp     equ     $14
tmp2    equ     $15
data    equ     $DD00
 
        org     $C000

        tsx             top of stack for frame pointer
        xgdx                to be placed in X
        subd    #$18    subtract offset to make room
        xgdx            X now has frame pointer
        puly            get return address
        sty     return,X save it
*        puly            get data address
*        sty     real,X  save it
        ldy    #data
        sty    real,X
        clr     sclfct,X zero scale factor
        iny             inc y for imag data
        clrb
zero    clr     $FF,Y   note special place of imag
        iny              256 above data
        decb
        bne     zero
*
* must do bit sorting before transforming
*
        ldab    #$FE    setup start for bit reversal
revbit  ldaa    #08     get # of bits to reverse
        pshb            save address offset
rev1    rorb            rotate b right - bit to carry
        rol     tmp,X   rotate left - carry bit in
        deca            decrement counter
        bne     rev1    go back if not done
        pulb            get unshifted address
        pshb            save copy
        cmpb    tmp,X   check to see if already done
        bcs     noswap  if so don't swap bytes
swap    ldy     real,X  get data address
        aby             add to base address
        ldaa    0,Y     get value
        pshy            store away
        ldy     real,X  get base again
        ldab    tmp,X   get shifted address
        aby             add to base
        ldab    0,Y     get second member
        staa    0,Y     put away first member
        puly            get first address
        stab    0,Y     put second member in first slot
noswap  pulb            get current address back
        decb            decrement it
        bne     revbit  do next if not done
*
* special case of first pass of FFT
*
        jsr     scale
        ldy     real,X  set up data pointer
        ldaa    #128    get number of cells
        staa    tmp,X   store in temp
fpss    ldaa    0,Y     get RM
        ldab    1,Y     get RN
        psha            make copy
        aba             RM'=RM+RN
        staa    0,Y     save back in data array
        pula            get RM again
        sba             RN'=RM-RN
        staa    1,Y     put away
        iny             point to next pair
        iny
        dec     tmp,X   decrement # cells
        bne     fpss    go back if not done
*
* now the FFT proper for passes 2 thru N
*
four    ldaa    #64     # of cells is now 64
        staa    celnm,X store
        staa    delta,X so is delta
        ldaa    #02     number of pairs is 2
        staa    pairnm,X
        staa    celdis,X so is distance between
npass   jsr     scale   check for over-range
        ldaa    celnm,X get current cell #
        staa    celct,X store at cell counter
        ldy     real,X
        sty     real1,X get copy of data
ncell   ldy     #sintab get address of sines
        sty     sinpt,X save copy
        ldaa    pairnm,X get current pairnm
np1     psha            save pair counter
        ldaa    0,Y     get cosine
        ldab    64,Y    get sine
        staa    cosa,X  save copy
        stab    sina,X  ditto
        ldy     real1,X point to top of data
        ldab    celdis,X get current offset
        aby             add to Y for current 
        sty     real2,X copy it
        ldaa    0,Y     get data point rn
        psha            copy it
        ldab    cosa,X  get cosine
        jsr     smul    rn*cos(a)
        staa    treal,X
        pula            get copy of rn
        ldab    sina,X  get sin(a)
        jsr     smul    rn*sin(a)
        staa    timag,X store imaginary tmp
        iny
        ldaa    $FF,Y   get imaginary data
        psha            save it
        ldab    sina,X  get sin(a)
        jsr     smul    in*sin(a)
        adda    treal,X
        staa    treal,X  tr=rn*cos + in*sin
        pula            get data back
        ldab    cosa,X  get cosine
        jsr     smul    in*cos(a)
        suba    timag,X  ti=in*cos-rn*sin
        staa    timag,X
        ldy     real1,X
        ldaa    00,Y    get rm 
        tab             save a copy
        adda    treal,X rm'=rm+tr
        staa    00,Y    store new rm
        subb    treal,X rn'=rm-tr
        ldy     real2,X
        stab    00,Y    store new rn
        ldy     real1,X
        iny
        sty     real1,X save real1 for nxt
        ldaa    $FF,Y   get im
        tab             save copy
        adda    timag,X im'=im+ti
        staa    $FF,Y   put back in array
        ldy     real2,X
        iny
        subb    timag,X in'=im-ti
        stab    $FF,Y   put back in array
        ldy     sinpt,X
        ldab    delta,X increment sine pntr
        aby
        sty     sinpt,X save away
        pula
        deca            dec pair counter
        bne     np1
ar1     ldy     real1,X
        ldab    celdis,X
        aby
        sty     real1,X
        dec     celct,X
        beq     ar3
        jmp     ncell
ar3     lsr     celnm,X  half cells
        beq     smsq    done when all cells
        asl     pairnm,X double pairs
        asl     celdis,X twice as far apart
        lsr     delta,X  delta is half
        jmp     npass    one more time!
done    ldaa    sclfct,X get scale factor
        psha             save on stack
        ldy     return,X
        pshy
        rts
*
* sum of absolute values instead of sum of squares
*
smsq    ldy     real,X   compute sum of "sqrs"
        clra             clear byte counter
sum     psha             save on stack
        ldaa    0,Y      get real data point
        bpl     sm1      force positive
        nega
        bvc     sm1      watch for $80
        clra               which is really 0
sm1     iny              get imaginary data
        ldab    $FF,Y
        bpl     sm2      force positive
        negb
        bvc     sm2      watch for $80 again
        clrb
sm2     dey              correct data pointer
        aba              compute sum
        staa    0,Y      save back in real
        iny              inc Y for next round
        pula             get byte counter
        deca             done when zero
        bne     sum
        bra     done     lets get out of here
*
* subroutine for catching overscaled data
*
scale   ldy     real,X   start at top of data
        ldab    #$FF
        aby              top of data
        aby              top or imag
        iny              need two more
        iny
        ldaa    #$C0     -64
        ldab    #$40     +64
top     cmpa    0,Y      check for minimum
        blo     nxt      if more negative fix
        cmpb    0,Y      check for too big
        bcs     scl      go fix it
nxt     dey              bump pointer
        cpy  real,X      done when both
        bne     top      imag and data done
        rts
scl     inc     sclfct,X keep track of scale
        ldy     real,X   set up pointer
        ldab    #$FF
        aby
        aby
        iny
        iny
scl1    ldaa    0,Y      get data
        adda    #$80     make positive
        lsra             divide by two
        suba    #$40     put back
        staa    0,Y      store away
        dey              bump pointer
        cpy     real,X   done when both
        bne     scl1     imag and data done
        rts
*
* the HC11 multiply must be modified to handle
* negative data
*
smul    staa    tmp,X   copy multiplier
        stab    tmp2,X  ditto multiplicand
        tsta            check sign of multiplier
        bpl     sk1     skip negation
        nega
        bvs     sko     check for $80
        beq     sko     check for zero
sk1     tstb            check multiplier sign
        bpl     sk2
        negb
        bvs     sko     check for $80
        beq     sko
sk2     mul             do multiplication
        adca    #0      8 bit conversion
        asla            and correct for sine
        ldab    tmp2,X  get original multiplicand
        eorb    tmp,X   check for result
        bpl     out
        nega            result is negative
out     rts
sko     clra            return zero to main
        rts
*
* now for the sine look up table
*
sintab            
 fcb  127, 127, 127, 127, 126, 126, 126, 125, 125, 124
 fcb  123, 122, 122, 121, 120, 118, 117, 116, 115, 113
 fcb  112, 111, 109, 107, 106, 104, 102, 100,  98,  96
 fcb   94,  92,  90,  88,  85,  83,  81,  78,  76,  73
 fcb   71,  68,  65,  63,  60,  57,  54,  51,  49,  46
 fcb   43,  40,  37,  34,  31,  28,  25,  22,  19,  16
 fcb   12,   9,   6,   3,   0,  -3,  -6,  -9, -12, -16
 fcb  -19, -22, -25, -28, -31, -34, -37, -40, -43, -46
 fcb  -49, -51, -54, -57, -60, -63, -65, -68, -71, -73
 fcb  -76, -78, -81, -83, -85, -88, -90, -92, -94, -96
 fcb  -98,-100,-102,-104,-106,-107,-109,-111,-112,-113
 fcb -115,-116,-117,-118,-120,-121,-122,-122,-123,-124
 fcb -125,-125,-126,-126,-126,-127,-127,-127,-127,-127
 fcb -127,-127,-126,-126,-126,-125,-125,-124,-123,-122
 fcb -122,-121,-120,-118,-117,-116,-115,-113,-112,-111
 fcb -109,-107,-106,-104,-102,-100, -98, -96, -94, -92
 fcb  -90, -88, -85, -83, -81, -78, -76, -73, -71, -68
 fcb  -65, -63, -60, -57, -54, -51, -49, -46, -43, -40
 fcb  -37, -34, -31, -28, -25, -22, -19, -16, -12,  -9
 fcb   -6,  -3,   0,   3,   6,   9,  12,  16,  19,  22
 fcb   25,  28,  31,  34,  37,  40,  43,  46,  49,  51
 fcb   54,  57,  60,  63,  65,  68,  71,  73,  76,  78
 fcb   81,  83,  85,  88,  90,  92,  94,  96,  98, 100
 fcb  102, 104, 106, 107, 109, 111, 112, 113, 115, 116
 fcb  117, 118, 120, 121, 122, 122, 123, 124, 125, 125
 fcb  126, 126, 126, 127, 127, 127
*
* Some routines for fast A2D and for sending data to 
* host
*   these have not been debugged
*
chan4   equ     $04
atdctr  equ     $1030
portc   equ     $1003
ccontr  equ     $1002
lportc  equ     $1005
strt    equ     $02
        ldy     #data
        ldaa    #chan4
        ldab    #strt
wait:   bitb    portc
        beq     wait
cklp:   ldab    ccontr
        bpl     cklp
        staa    atdctr
        ldab    lportc
        iny
        cpy     #data+256
        beq     adone
        nop
        nop
        nop
        nop
        ldab    atdctr+1
        stab    $00,Y
        bra     cklp
adone:  rts

output  equ    #$E3B3
sendat  ldx    #data
        clrb
s1      jsr    output
        decb
        bne    s1
        rts


ut  equ    #$E3B