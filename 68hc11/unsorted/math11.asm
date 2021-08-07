*******************************************************************************
*                                  HC11FP                                     *
*                                                                             *
*                              Copyright 1986                                 *
*                                    by                                       *
*                              Gordon Doughman                                *
*                                                                             *
*       The source code for this floating point package for the MC68HC11      *
*       may be freely distributed under the rules of public domain. However   *
*       it is a copyrighted work and as such may not be sold as a product     *
*       or be included as part of a product for sale without the express      *
*       permission of the author. Any object code produced by the source      *
*       code may be included as part of a product for sale.                   *
*                                                                             *
*       If there are any questions or comments about the floating point       *
*       package please feel free to contact me.                               *
*                                                                             *
*                              Gordon Doughman                                *
*                              Motorola Semiconductor                         *
*                              3490 South Dixie Drive                         *
*                              Dayton, OH  45439                              *
*                              (513) 294-2231                                 *
*                                                                             *
*******************************************************************************
*                                                                             *
*                                 MATH11                                      *
*                                                                             *
*                  Revisions to FP11 floating point code                      *
*                  Modifications Copyright 1988, Scott Wagner                 *
*                                                                             *
*        The following improvements have been made to the HC11FP code:        *
*               1) Execution time and stack space requirements of the basic   *
*                  arithmetic operations (+-*/) have been reduced.            *
*               2) The efficiency of the trigonometric functions has been     *
*                  improved.                                                  *
*               3) New functions FLTATAN, FLTLN, FLTLGT, FLTETOX, FLT10TX     *
*                  and FLTXTOY have been added.                               *
*               4) The polynomial expansion routine POLYNOM is available to   *
*                  evaluate series of arbitrary order.  The coefficient table *
*                  supplied by the user determines the polynomial order.      *
*               5) All floating point numbers are now stored in memory in the *
*                  IEEE floating point format for compatibility with other    *
*                  compilers.                                                 *
*               6) All functions now signal errors by setting the Carry bit.  *
*                  If Carry is clear on return, contents of ACCA are          *
*                  indeterminate; if Carry is set, error code is in ACCA.     *
*               7) For compatibility with error returns, FLTCMP returns Z=1   *
*                  if FPACC1 == FPACC2, Z=0 if FPACC1 != FPACC2, and N=1 if   *
*                  FPACC1 < FPACC2, N=0 if FPACC1 >= FPACC2.  Note that this  *
*                  changes the conditional branch instructions following      *
*                  calls to FLTCMP in the routines in this package which use  *
*                  the FLTCMP routine.                                        *
*                                                                             *
*        These modifications to the HC11FP package are provided under the     *
*        rules of public domain stated above.  Please direct comments about   *
*        these modifications to:                                              *
*                                       Scott Wagner                          *
*                                       Rochester Instrument Systems          *
*                                       255 North Union Street                *
*                                       Rochester, New York 14605             *
*                                                                             *
*******************************************************************************
*
*               REVISION HISTORY:
*       1.0     02-11-88        Release to public domain of enhancements to
*                               Gordon Doughman's 68HC11 FP package.  S. Wagner
*       1.1     07-07-88        Corrected errors/anomalies in FLTCMP.  Made
*                               error returns more consistent.  S. Wagner
*       1.2     07-22-88        Corrected two digit exponent pointer error and
*                               mantissa conversion overflow error in ASCFLT.
*                               Addition involving zero or arguments with large
*                               exponent difference now returns no error and
*                               ACCA clear.  ASCFLT accepts 'E' or 'e' as
*                               exponent delimiter.  S. Wagner
*       1.3     09-21-88        Corrected error in storage format - storage is
*                               now true IEEE floating point format. Added new
*                               functions FLTABS (absolute value), FLTSGN
*                               (signum function), FLTMIN (negative). S. Wagner
*               02-19-92        EAS calculated 4MHz bus(16MHZ xtal) times *
*
         ORG    $0000
*
*
FPACC1EX RMB    1            FLOATING POINT ACCUMULATOR #1.
FPACC1MN RMB    3
MANTSGN1 RMB    1            MANTISSA SIGN FOR FPACC1 (0=+, FF=-).
FPACC2EX RMB    1            FLOATING POINT ACCUMULATOR #2.
FPACC2MN RMB    3
MANTSGN2 RMB    1            MANTISSA SIGN FOR FPACC2 (0=+, FF=-).
*
*
OVFERR   EQU    1           /* floating point overflow error */
UNFERR   EQU    1           /* floating point underflow error */
DIV0ERR  EQU    5           /* division by 0 error */
TOLGSMER EQU    6           /* number too large or small to convert to int. */
NSQRTERR EQU    7           /* tried to take the square root of negative # */
TAN90ERR EQU    8           /* TANgent of 90 degrees attempted */
LNNEGERR EQU    9           /* LOG or LN of negative number or 0 */
ACOSERR  EQU    10          /* Arc cosine not implemented */
ASINERR  EQU    11          /* Arc sine not implemented */
FLTFMTER EQU    12          /* floating point format error in ASCFLT */
*
*
******************************************************************************
*                                                                            *
*                        ASCII TO FLOATING POINT ROUTINE                     *
*                                                                            *
*       This routine will accept most any ASCII floating point format        *
*       and return a 32-bit floating point number.  The following are        *
*       some examples of legal ASCII floating point numbers.                 *
*                                                                            *
*       20.095                                                               *
*       0.125                                                                *
*       7.2984E10                                                            *
*       167.824E5                                                            *
*       5.9357E-7                                                            *
*       500                                                                  *
*                                                                            *
*       The floating point number returned is in "FPACC1".                   *
*                                                                            *
*                                                                            *
*       The exponent is biased by 127 to facilitate floating point           *
*       comparisons.  A pointer to the ASCII string is passed to the         *
*       routine in the X-register.                                           *
*                                                                            *
*                                                                            *
******************************************************************************
*
*
*
*        LOCAL VARIABLES (ON STACK POINTED TO BY Y)
*
EXPSIGN  EQU    0            EXPONENT SIGN (0=+, FF=-).
PWR10EXP EQU    1            POWER 10 EXPONENT.
*
*
         ORG    $C000        (TEST FOR EVB)
*
ASCFLT   EQU    *
         PSHX                SAVE POINTER TO ASCII STRING.
         JSR    PSHFPAC2     SAVE FPACC2.
         LDX    #0           PUSH ZEROS ON STACK TO INITIALIZE LOCALS.
         PSHX                ALLOCATE 2 BYTES FOR LOCALS.
         STX    FPACC1EX     CLEAR FPACC1.
         STX    FPACC1EX+2
         CLR    MANTSGN1     MAKE THE MANTISSA SIGN POSITIVE INITIALLY.
         TSY                 POINT TO LOCALS.
         LDX    6,Y          GET POINTER TO ASCII STRING.
ASCFLT1  LDAA   0,X          GET 1ST CHARACTER IN STRING.
         JSR    NUMERIC      IS IT A NUMBER.
         BCS    ASCFLT4      YES. GO PROCESS IT.
*
*        LEADING MINUS SIGN ENCOUNTERED?
*
ASCFLT2  CMPA   #'-'         NO. IS IT A MINUS SIGN?
         BNE    ASCFLT3      NO. GO CHECK FOR DECIMAL POINT.
         COM    MANTSGN1     YES. SET MANTISSA SIGN. LEADING MINUS BEFORE?
         INX                 POINT TO NEXT CHARACTER.
         LDAA   0,X          GET IT.
         JSR    NUMERIC      IS IT A NUMBER?
         BCS    ASCFLT4      YES. GO PROCESS IT.
*
*        LEADING DECIMAL POINT?
*

ASCFLT3  CMPA   #'.'          IS IT A DECIMAL POINT?
         BNE    ASCFLT5      NO. FORMAT ERROR.
         INX                 YES. POINT TO NEXT CHARACTER.
         LDAA   0,X          GET IT.
         JSR    NUMERIC      MUST HAVE AT LEAST ONE DIGIT AFTER D.P.
         BCC    ASCFLT5      GO REPORT ERROR.
         JMP    ASCFLT11     GO BUILD FRACTION.
*
*        FLOATING POINT FORMAT ERROR
*
ASCFLT5  INS                 DE-ALLOCATE LOCALS.
         INS
         JSR    PULFPAC2     RESTORE FPACC2.
         PULX                GET POINTER TO TERMINATING CHARACTER IN STRING.
         LDAA   #FLTFMTER    FORMAT ERROR.
         SEC                 SET ERROR FLAG.
         RTS                 RETURN.
*
*        PRE DECIMAL POINT MANTISSA BUILD
*
ASCFLT4  LDAA   0,X
         JSR    NUMERIC
         BCC    ASCFLT10
         JSR    ADDNXTD
         INX
         BCC    ASCFLT4
*
*        PRE DECIMAL POINT MANTISSA OVERFLOW
*
ASCFLT6  INC    FPACC1EX     INC FOR EACH DIGIT ENCOUNTERED PRIOR TO D.P.
         LDAA   0,X          GET NEXT CHARACTER.
         INX                 POINT TO NEXT.
         JSR    NUMERIC      IS IT S DIGIT?
         BCS    ASCFLT6      YES. KEEP BUILDING POWER 10 MANTISSA.
         CMPA   #'.'         NO. IS IT A DECIMAL POINT?
         BNE    ASCFLT7      NO. GO CHECK FOR THE EXPONENT.
*
*        ANY FRACTIONAL DIGITS ARE NOT SIGNIFIGANT
*
ASCFLT8  LDAA   0,X          GET THE NEXT CHARACTER.
         JSR    NUMERIC      IS IT A DIGIT?
         BCC    ASCFLT7      NO. GO CHECK FOR AN EXPONENT.
         INX                 POINT TO THE NEXT CHARACTER.
         BRA    ASCFLT8      FLUSH REMAINING DIGITS.
ASCFLT7  CMPA   #'E'         NO. IS IT THE EXPONENT?
         BEQ    ASCFLT13     YES. GO PROCESS IT.
         CMPA   #'e'         IS IT THE exponent?
         BEQ    ASCFLT13     YES. GO PROCESS IT.
         JMP    FINISH       NO. GO FINISH THE CONVERSION.
*
*        PROCESS THE EXPONENT
*
ASCFLT13 INX                 POINT TO NEXT CHARACTER.
         LDAA   0,X          GET THE NEXT CHARACTER.
         JSR    NUMERIC      SEE IF IT'S A DIGIT.
         BCS    ASCFLT9      YES. GET THE EXPONENT.
         CMPA   #'-'         NO. IS IT A MINUS SIGN?
         BEQ    ASCFLT15     YES. GO FLAG A NEGATIVE EXPONENT.
         CMPA   #'+'         NO. IS IT A PLUS SIGN?
         BEQ    ASCFLT16     YES. JUST IGNORE IT.
         BRA    ASCFLT5      NO. FORMAT ERROR.
ASCFLT15 COM    EXPSIGN,Y    FLAG A NEGATIVE EXPONENT. IS IT 1ST?
ASCFLT16 INX                 POINT TO NEXT CHARACTER.
         LDAA   0,X          GET NEXT CHARACTER.
         JSR    NUMERIC      IS IT A NUMBER?
         BCC    ASCFLT5      NO. FORMAT ERROR.
ASCFLT9  SUBA   #$30         MAKE IT BINARY.
         STAA   PWR10EXP,Y   BUILD THE POWER 10 EXPONENT.
         INX                 POINT TO NEXT CHARACTER.
         LDAA   0,X          GET IT.
         JSR    NUMERIC      IS IT NUMERIC?
         BCC    ASCFLT14     NO. GO FINISH UP THE CONVERSION.
         LDAB   PWR10EXP,Y   YES. GET PREVIOUS DIGIT.
         LSLB                MULT. BY 2.
         LSLB                NOW BY 4.
         ADDB   PWR10EXP,Y   BY 5.
         LSLB                BY 10.
         SUBA   #$30         MAKE SECOND DIGIT BINARY.
         ABA                 ADD IT TO FIRST DIGIT.
         STAA   PWR10EXP,Y
         INX                 POINT TO CHARACTER FOLLOWING EXPONENT
         CMPA   #38          IS THE EXPONENT OUT OF RANGE?
         BHI    ASCFLT5      YES. REPORT ERROR.
ASCFLT14 LDAA   PWR10EXP,Y   GET POWER 10 EXPONENT.
         TST    EXPSIGN,Y    WAS IT NEGATIVE?
         BPL    ASCFLT12     NO. GO ADD IT TO BUILT 10 PWR EXPONENT.
         NEGA
ASCFLT12 ADDA   FPACC1EX     FINAL TOTAL PWR 10 EXPONENT.
         STAA   FPACC1EX     SAVE RESULT.
         BRA    FINISH       GO FINISH UP CONVERSION.
*
*        PRE-DECIMAL POINT NON-DIGIT FOUND, IS IT A DECIMAL POINT?
*
ASCFLT10 CMPA   #'.'         IS IT A DECIMAL POINT?
         BNE    ASCFLT7      NO. GO CHECK FOR THE EXPONENT.
         INX                 YES. POINT TO NEXT CHARACTER.
*
*        POST DECIMAL POINT PROCESSING
*
ASCFLT11 LDAA   0,X          GET NEXT CHARACTER.
         JSR    NUMERIC      IS IT NUMERIC?
         BCC    ASCFLT7      NO. GO CHECK FOR EXPONENT.
         BSR    ADDNXTD      YES. ADD IN THE DIGIT.
         INX                 POINT TO THE NEXT CHARACTER.
         BCS    ASCFLT8      IF OVER FLOW, FLUSH REMAINING DIGITS.
         DEC    FPACC1EX     ADJUST THE 10 POWER EXPONENT.
         BRA    ASCFLT11     PROCESS ALL FRACTIONAL DIGITS.
*
*
*
ADDNXTD  LDAA   FPACC1MN     GET UPPER 8 BITS.
         STAA   FPACC2MN     COPY INTO FPAC2.
         LDD    FPACC1MN+1   GET LOWER 16 BITS OF MANTISSA.
         STD    FPACC2MN+1   COPY INTO FPACC2.
         LSLD                MULT. BY 2.
         ROL    FPACC1MN     OVERFLOW?
         BCS    ADDNXTD1     YES. DON'T ADD THE DIGIT IN.
         LSLD                MULT BY 4.
         ROL    FPACC1MN     OVERFLOW?
         BCS    ADDNXTD1     YES. DON'T ADD THE DIGIT IN.
         ADDD   FPACC2MN+1   BY 5.
         PSHA                SAVE A.
         LDAA   FPACC1MN     GET UPPER 8 BITS.
         ADCA   FPACC2MN     ADD IN UPPER 8 BITS, CARRY FROM LOWER 16 BITS.
         STAA   FPACC1MN     SAVE IT.
         PULA                RESTORE A.
         BCS    ADDNXTD1     OVERFLOW? IF SO DON'T ADD IT IN.
         LSLD                BY 10.
         ROL    FPACC1MN
         STD    FPACC1MN+1   SAVE THE LOWER 16 BITS.
         BCS    ADDNXTD1     OVERFLOW? IF SO DON'T ADD IT IN.
         LDAB   0,X          GET CURRENT DIGIT.
         SUBB   #$30         MAKE IT BINARY.
         CLRA                16-BIT.
         ADDD   FPACC1MN+1   ADD IT IN TO TOTAL.
         STD    FPACC1MN+1   SAVE THE RESULT.
         LDAA   FPACC1MN     GET UPPER 8 BITS.
         ADCA   #0           ADD IN POSSIBLE CARRY. OVERFLOW?
         BCS    ADDNXTD1     YES. COPY OLD MANTISSA FROM FPACC2.
         STAA   FPACC1MN     NO. EVERYHING OK.
         RTS                 RETURN.
ADDNXTD1 LDD    FPACC2MN+1   RESTORE THE ORIGINAL MANTISSA BECAUSE
         STD    FPACC1MN+1   OF OVERFLOW.
         LDAA   FPACC2MN
         STAA   FPACC1MN
         RTS                 RETURN.
*
*
*
*        NOW FINISH UP CONVERSION BY MULTIPLYING THE RESULTANT MANTISSA
*        BY 10 FOR EACH POSITIVE POWER OF 10 EXPONENT RECIEVED OR BY .1
*        (DIVIDE BY 10) FOR EACH NEGATIVE POWER OF 10 EXPONENT RECIEVED.
*
*
FINISH   EQU    *
         STX    6,Y          SAVE POINTER TO TERMINATING CHARACTER IN STRING.
         LDX    #FPACC1EX    POINT TO FPACC1.
         JSR    CHCK0        SEE IF THE NUMBER IS ZERO.
         BEQ    FINISH3      QUIT IF IT IS.
         LDAA   FPACC1EX     GET THE POWER 10 EXPONENT.
         STAA   PWR10EXP,Y   SAVE IT.
         LDAA   #$7E+24      SET UP INITIAL EXPONENT (# OF BITS + BIAS).
         STAA   FPACC1EX
         JSR    FPNORM       GO NORMALIZE THE MANTISSA.
         TST    PWR10EXP,Y   IS THE POWER 10 EXPONENT POSITIVE OR ZERO?
         BEQ    FINISH3      IT'S ZERO, WE'RE DONE.
         BPL    FINISH1      IT'S POSITIVE MULTIPLY BY 10.
         LDX    #CONSTP1     NO. GET CONSTANT .1 (DIVIDE BY 10).
         JSR    GETFPAC2     GET CONSTANT INTO FPACC2.
         NEG    PWR10EXP,Y   MAKE THE POWER 10 EXPONENT POSITIVE.
         BRA    FINISH2      GO DO THE MULTIPLIES.
FINISH1  LDX    #CONST10     GET CONSTANT '10' TO MULTIPLY BY.
         JSR    GETFPAC2     GET CONSTANT INTO FPACC2.
FINISH2  JSR    FLTMUL       GO MULTIPLY FPACC1 BY FPACC2, RESULT IN FPACC1.
         DEC    PWR10EXP,Y   DECREMENT THE POWER 10 EXPONENT.
         BNE    FINISH2      GO CHECK TO SEE IF WE'RE DONE.
FINISH3  INS                 DE-ALLOCATE LOCALS.
         INS
         JSR    PULFPAC2     RESTORE FPACC2.
         PULX                GET POINTER TO TERMINATING CHARACTER IN STRING.
         RTS                 RETURN WITH NUMBER IN FPACC1.
*
*
NUMERIC  EQU    *
         CMPA   #'0'         IS IT LESS THAN AN ASCII 0?
         BLO    NUMERIC1     YES. NOT NUMERIC.
         CMPA   #'9'         IS IT GREATER THAN AN ASCII 9?
         BHI    NUMERIC1     YES. NOT NUMERIC.
         SEC                 IT WAS NUMERIC. SET THE CARRY.
         RTS                 RETURN.
NUMERIC1 CLC                 NON-NUMERIC CHARACTER. CLEAR THE CARRY.
         RTS                 RETURN.
*
FPNORM   EQU    *
         LDX    #FPACC1EX    POINT TO FPACC1.
         BSR    CHCK0        CHECK TO SEE IF IT'S 0.
         BEQ    FPNORM3      YES. JUST RETURN.
         TST    FPACC1MN     IS THE NUMBER ALREADY NORMALIZED?
         BMI    FPNORM3      YES. JUST RETURN..
FPNORM1  LDD    FPACC1MN+1   GET THE LOWER 16 BITS OF THE MANTISSA.
FPNORM2  DEC    FPACC1EX     DECREMENT THE EXPONENT FOR EACH SHIFT.
         BEQ    FPNORM4      EXPONENT WENT TO 0. UNDERFLOW.
         LSLD                SHIFT THE LOWER 16 BITS.
         ROL    FPACC1MN     ROTATE THE UPPER 8 BITS. NUMBER NORMALIZED?
         BPL    FPNORM2      NO. KEEP SHIFTING TO THE LEFT.
         STD    FPACC1MN+1   PUT THE LOWER 16 BITS BACK INTO FPACC1.
FPNORM3  CLC                 SHOW NO ERRORS.
         RTS                 YES. RETURN.
FPNORM4  SEC                 FLAG ERROR.
         RTS                 RETURN.
*
CHCK0    EQU    *            CHECKS FOR ZERO IN FPACC POINTED TO BY X.
         PSHB                SAVE D.
         PSHA
         LDD    0,X          GET FPACC EXPONENT & HIGH 8 BITS.
         BNE    CHCK01       NOT ZERO. RETURN.
         LDD    2,X          CHECK LOWER 16 BITS.
CHCK01   PULA                RESTORE D.
         PULB
         RTS                 RETURN WITH CC SET.
*
CONSTP1  FCB    $3D,$CC,$CC,$CD         0.1 DECIMAL
CONST10  FCB    $41,$20,$00,$00         10.0 DECIMAL
*
*
FLTMOD   EQU    *            FLOATING POINT MODULUS
         JSR    FLTDIV       DO DIVISION
         JSR    PSHFPAC2     SAVE ARGUMENT
         JSR    INTFRAC      FIND FRACTIONAL PART
         JSR    PULFPAC2     RECOVER ARGUMENT
*                            FALL THROUGH TO MULTIPLY
*
******************************************************************************
*                                                                            *
*                      FPMULT: FLOATING POINT MULTIPLY                       *
*                                                                            *
*       THIS FLOATING POINT MULTIPLY ROUTINE MULTIPLIES "FPACC1" BY          *
*       "FPACC2" AND PLACES THE RESULT IN TO FPACC1. FPACC2 REMAINS          *
*       UNCHANGED.                                                           *
*                          WORST CASE = 480 CYCLES = 240 uS @ 2MHz           *
*                          WORST CASE = 480 CYCLES = 120 uS @ 4MHz           *
*                                                                            *
******************************************************************************
*
*
FLTMUL   EQU    *
         TST    FPACC1EX     CHECK TO SEE IF FPACC1 IS ZERO.
         BEQ    FPMULT3      IT IS. ANSWER IS 0.
         TST    FPACC2EX     CHECK TO SEE IF FPACC2 IS ZERO.
         BNE    FPMULT8      IT IS NOT. GO DO MULTIPLY
FPMULT3  LDD    #0           ZERO RESULT
         STAA   MANTSGN1
         STD    FPACC1EX
         STD    FPACC1MN+1
         RTS                 RETURN.
FPMULT8  LDAA   MANTSGN1     GET FPACC1 EXPONENT.
         EORA   MANTSGN2     SET THE SIGN OF THE RESULT.
         STAA   MANTSGN1     SAVE THE SIGN OF THE RESULT.
         LDAA   FPACC1EX     GET FPACC1 EXPONENT.
         ADDA   FPACC2EX     ADD IT TO FPACC2 EXPONENT.
         BPL    FPMULT1      IF RESULT IS MINUS AND
         BCC    FPMULT2      THE CARRY IS SET THEN:
RTNMAX   LDAA   #OVFERR      OVERFLOW ERROR.
RTNDIV0  LDX    #$FFFF       MAXIMUM MAGNITUDE RESULT
         BRA    FPMULT7      DO IT AND RETURN.
FPMULT1  BCS    FPMULT2      IF RESULT IS PLUS & THE CARRY IS SET THEN ALL OK.
RTNZERO  LDAA   #UNFERR      ELSE UNDERFLOW ERROR OCCURED.
         LDX    #0           ZERO RESULT
         STX    FPACC1MN+2   THIS CLEARS MANTSGN1 BYTE
FPMULT7  STX    FPACC1EX
         STX    FPACC1MN+1
         SEC                 FLAG ERROR.
         RTS                 RETURN.
FPMULT2  ADDA   #$82         ADD BIAS BACK IN THAT WE LOST.
         STAA   FPACC1EX     SAVE THE NEW EXPONENT.
         LDX    #0
         PSHX                CREATE PARTIAL PRODUCT REGISTER AND COUNTER.
         PSHX
         TSX                 POINT TO THE VARIABLES.
         JSR    UMULT        GO MULTIPLY THE "INTEGER" MANTISSAS.
         TST    0,X          DOES RESULT NEED TO BE NORMALIZED?
         BMI    FPMROUND     NO - GO ROUND RESULT
         ROL    3,X          FIRST NORMALIZE RESULT
         ROL    2,X
         ROL    1,X
         ROL    0,X
         DEC    FPACC1EX     NOW DECREMENT EXPONENT
FPMROUND TST    3,X          CHECK MSB OF BYTE 4 (TO BE DISCARDED LATER)
         BPL    FPMULT4      NO ROUNDING NECESSARY
         LDAA   2,X          ROUND LSB UP
         INCA                INCREMENT RESULT LSB
         STAA   2,X          PUT LSB BACK
         BNE    FPMULT4      IF NO CARRY TO RESULT BYTES 1 AND 2
         LDAB   #1           SET D REGISTER TO 1 (ACCA IS ALREADY 0)
         ADDD   0,X          INCREMENT BYTES 1 AND 2
         BCC    FPMULT5      NO OVERFLOW FROM BYTES 1 AND 2
         RORA                RESULT CHANGES FROM $7FFFFF TO $800000
         INC    FPACC1EX     EXPONENT INCREMENTED (BACK TO WHERE IT WAS)
FPMULT5  STD    0,X          PUT BYTES 1 AND 2 BACK
FPMULT4  PULX                RETRIEVE BYTES 1 AND 2
         STX    FPACC1MN     STORE IN MANTISSA HIGH BYTES
         PULA                RETRIEVE BYTE 3 (LSB)
         STAA   FPACC1MN+2   STORE IN MANTISSA LOW BYTE
         INS                 DISCARD BYTE 4
         TST    FPACC1EX     WAS THERE AN UNDERFLOW ERROR?
         BEQ    RTNZERO      YES. RETURN ERROR.
         CLRB                SHOW NO ERRORS.
         RTS
*
*
UMULT    EQU    *
         LDAA   FPACC2MN+2   GET MULTIPLIER LSB
         LDAB   FPACC1MN+2   GET MULTIPLICAND LSB
         MUL
         STAA   1,X          TEMPORARILY SAVE RESULT MSB; DISCARD LSB (BYTE 6)
         LDAA   FPACC2MN+1   GET MULTIPLIER NSB
         LDAB   FPACC1MN+2   GET MULTIPLICAND LSB
         MUL
         ADDD   0,X          ADD IN LAST PARTIAL RESULT
         STD    0,X          TEMPORARILY SAVE RESULT
         LDAA   FPACC2MN+2   GET MULTIPLIER LSB
         LDAB   FPACC1MN+1   GET MULTIPLICAND NSB
         MUL
         ADDD   0,X          ADD IN LAST PARTIAL RESULT
         STAA   3,X          SAVE PARTIAL PRODUCT BYTE 4; DISCARD LSB (BYTE 5)
         BCC    UMULT1       IF NO CARRY OUT TO PRODUCT BYTE 3
         INC    2,X          CARRY TO PRODUCT BYTE 3
UMULT1   CLR    0,X          ZERO PRODUCT BYTES 1 & 2 (USED FOR TEMP. STORAGE)
         CLR    1,X
         LDAA   FPACC2MN     GET MULTIPLIER MSB
         LDAB   FPACC1MN+2   GET MULTIPLICAND LSB
         MUL
         ADDD   2,X          ADD IN LAST PARTIAL RESULT
         STD    2,X          SAVE IN PARTIAL PRODUCT BYTES 3 AND 4
REMCOMP  LDAA   FPACC2MN+1   GET MULTIPLIER NSB
         LDAB   FPACC1MN+1   GET MULTIPLICAND NSB
         MUL
         ADDD   2,X          ADD IN LAST PARTIAL RESULT
         STD    2,X          SAVE IN PARTIAL PRODUCT BYTES 3 AND 4
         BCC    UMULT2       IF NO CARRY OUT TO PRODUCT BYTE 2
         INC    1,X          CARRY TO PRODUCT BYTE 2
UMULT2   LDAA   FPACC2MN+2   GET MULTIPLIER LSB
         LDAB   FPACC1MN     GET MULTIPLICAND MSB
         MUL
         ADDD   2,X          ADD IN LAST PARTIAL RESULT
         STD    2,X          SAVE IN PARTIAL PRODUCT BYTES 3 AND 4
         BCC    UMULT3       IF NO CARRY OUT TO PRODUCT BYTE 2
         INC    1,X          CARRY TO PRODUCT BYTE 2
UMULT3   LDAA   FPACC2MN     GET MULTIPLIER MSB
         LDAB   FPACC1MN+1   GET MULTIPLICAND NSB
         MUL
         ADDD   1,X          ADD IN LAST PARTIAL RESULT
         STD    1,X          SAVE IN PARTIAL PRODUCT BYTES 2 AND 3
         BCC    UMULT4       IF NO CARRY OUT TO PRODUCT BYTE 1
         INC    0,X          CARRY TO PRODUCT BYTE 1
UMULT4   LDAA   FPACC2MN+1   GET MULTIPLIER NSB
         LDAB   FPACC1MN     GET MULTIPLICAND MSB
         MUL
         ADDD   1,X          ADD IN LAST PARTIAL RESULT
         STD    1,X          SAVE IN PARTIAL PRODUCT BYTES 2 AND 3
         BCC    UMULT5       IF NO CARRY OUT TO PRODUCT BYTE 1
         INC    0,X          CARRY TO PRODUCT BYTE 1
UMULT5   LDAA   FPACC2MN     GET MULTIPLIER MSB
         LDAB   FPACC1MN     GET MULTIPLICAND MSB
         MUL
         ADDD   0,X          ADD IN LAST PARTIAL RESULT
         STD    0,X          SAVE IN PARTIAL PRODUCT BYTES 2 AND 3
         RTS                 RETURN TO DO NORMALIZATION AND ROUNDING
*
*
*
******************************************************************************
*                                                                            *
*                   FLOATING POINT TO INTEGER CONVERSION                     *
*                                                                            *
*        The subroutine FLT2INT will perform  floating point to integer      *
*        conversion.  The floating point number if positive, will be         *
*        converted to an unsigned 16 bit integer ( 0 <= X <= 65535 ), and    *
*        the N flag will be cleared.  If negative, the floating point number *
*        will be converted to a signed 16-bit (twos complement) integer      *
*        (-32768 <= X <= -1), and the N flag will be set.  The converted     *
*        integer is returned in the double accumulator D.  Truncation is     *
*        always toward zero, and the fractional part of the argument is      *
*        discarded.  If the argument is too large or too small, the carry    *
*        flag is set and the error code is returned in ACCA.                 *
*                                                                            *
*        The subroutine FLTROUND is similar to FLT2INT except that it rounds *
*        to the nearest integer instead of truncating the fractional part.   *
*                                                                            *
******************************************************************************
*
*
*
FLTROUND EQU    *
         JSR    PSHFPAC2             SAVE FPACC2
         LDX    #CONSTP5             POINT TO CONSTANT 0.5
         JSR    GETFPAC2             PUT IT INTO FPACC2
         TST    MANTSGN1             CHECK FOR NEGATIVE ARGUMENT
         BEQ    FLTROU1              ARGUMENT POSITIVE - ADD +0.5
         COM    MANTSGN2             ARGUMENT NEGATIVE - ADD -0.5
FLTROU1  JSR    FLTADD               ADD 0.5 TO ROUND
         JSR    PULFPAC2             RESTORE FPACC2 - FALL THROUGH TO FLT2INT
*
FLT2INT  EQU    *
         CLRA                ZERO UPPER BYTE OF D
         LDAB   FPACC1EX     GET EXPONENT
         CMPB   #$7F         CHECK FOR INTEGRAL PART
         BHS    FLT2INT1     IF INTEGRAL PART
         CLRB                NO INTEGRAL PART
         BRA    FLT2INT4     RETURN ZERO
FLT2INT1 COMB                COMPUTE NUMBER OF SHIFTS REQUIRED ...
         ADDD   #$FF90       ... TO GENERATE RESULT
         BLE    FLT2INTE     ERROR IF > 65535
         XGDX                SAVE SHIFT COUNTER IN X
         LDD    FPACC1MN     GET SIGNIFICANT PART OF MANTISSA
         BRA    FLT2INT3     GO DO SHIFTING
FLT2INT2 LSRD                SHIFT RESULT
FLT2INT3 DEX                 DECREMENT SHIFT COUNTER
         BNE    FLT2INT2     IF NOT DONE SHIFTING YET
         TST    MANTSGN1     CHECK FOR POSITIVE ARGUMENT
         BPL    FLT2INT4     IF POSITIVE, WE ARE DONE
         COMA                COMPLEMENT RESULT
         COMB
         ADDD   #1           ADD 1 FOR TWOS COMPLEMENT
         BPL    FLT2INTE     IF NEGATIVE RESULT < -32768
FLT2INT4 CLC                 NO ERRORS
         RTS                 RETURN
FLT2INTE LDAA   #TOLGSMER    NUMBER TOO LARGE OR TOO SMALL
         SEC                 FLAG ERROR
         RTS                 RETURN
*
*
******************************************************************************
*                                                                            *
*     SEPARATE A FLOATING POINT NUMBER INTO INTEGER AND FRACTIONAL PARTS     *
*                                                                            *
*        This subroutine separates the floating point number in FPACC1 into  *
*        integer and fractional parts.  The fractional part is returned in   *
*        FPACC1, and the integer part is returned in FPACC2.                 *
*                                                                            *
******************************************************************************
*
INTFRAC  EQU    *
         JSR    TFR1TO2              SAVE ARGUMENT IN FPACC2
         LDX    #FPACC2MN+2          POINT TO MANTISSA LS BYTE
         LDAA   FPACC2EX             GET EXPONENT
         SUBA   #$97                 REMOVE BIAS + 2^24
         BPL    FLTSUB               ARGUMENT IS LARGE INTEGER
         LDAB   #$03                 DO THIS 3 TIMES, MAXIMUM
INTFRAC1 ADDA   #$08                 INCREASE EXPONENT
         BPL    INTFRAC2             IF IN RANGE
         CLR    0,X                  CLEAR BYTE
         DEX                         POINT TO NEXT MOST SIGNIFICANT BYTE
         DECB                        DECREMENT COUNTER
         BNE    INTFRAC1             IF COUNT NOT ZERO
         CLR    0,X                  CLEAR EXPONENT FOR NUMBER LESS THAN 1
         RTS                         NO SUBTRACTION NECESSARY FOR ZERO
INTFRAC2 LDAB   #$80                 SET UP MASK
INTFRAC3 DECA                        DECREMENT COUNTER
         BMI    INTFRAC4             COUNTER < 0
         ASRB                        SHIFT NEXT BIT IN MASK
         BRA    INTFRAC3             KEEP GOING TILL COUNTER RUNS OUT
INTFRAC4 ANDB   0,X                  MASK MANTISSA BYTE
         STAB   0,X                  SAVE MANTISSA BYTE
*                                    FALL THROUGH TO SUBTRACT FOR FRACTION PART
*
*
*
******************************************************************************
*                                                                            *
*                     FLOATING POINT SUBTRACT SUBROUTINE                     *
*                                                                            *
*       This subroutine performs floating point subtraction ( FPACC1-FPACC2) *
*       by inverting the sign of FPACC2 and then calling FLTADD since        *
*       FLTADD performs complete signed addition.  Upon returning from       *
*       FLTADD the sign of FPACC2 is again inverted to leave it unchanged    *
*       from its original value.                                             *
*                                                                            *
*                          WORSE CASE = 601 CYCLES = 301   uS @ 2MHz         *
*                          WORSE CASE = 601 CYCLES = 150.5 uS @ 4MHz         *
*                                                                            *
******************************************************************************
*
*
FLTSUB   EQU    *
         BSR    FLTSUB1      INVERT SIGN.
         BSR    FLTADD       GO DO FLOATING POINT ADD.
FLTSUB1  LDAA   MANTSGN2     GET FPACC2 MANTISSA SIGN.
         EORA   #$FF         INVERT THE SIGN.
         STAA   MANTSGN2     PUT BACK.
         RTS                 RETURN.
*
*
*
******************************************************************************
*                                                                            *
*                       FLOATING POINT ADDITION                              *
*                                                                            *
*       This subroutine performs floating point addition of the two numbers  *
*       in FPACC1 and FPACC2.  The result of the addition is placed in       *
*       FPACC1 while FPACC2 remains unchanged.  This subroutine performs     *
*       full signed addition so either number may be of the same or opposite *
*       sign.                                                                *
*                          WORSE CASE = 563 CYCLES = 282 uS @ 2MHz           *
*                          WORSE CASE = 563 CYCLES = 141 uS @ 4MHz           *
*                                                                            *
******************************************************************************
*
*
FLTADD   EQU    *
         LDAA   FPACC1EX     LOAD FP1 EXPONENT
         BNE    FLTADD1      IF NOT ZERO
TFR2TO1  LDD    FPACC2EX     GET FPACC2 EXPONENT & HIGH 8 BIT OF MANTISSA.
         STD    FPACC1EX     PUT IT IN FPACC1.
         LDD    FPACC2MN+1   GET FPACC2 LOW 16 BITS OF MANTISSA.
         STD    FPACC1MN+1   PUT IT IN FPACC1.
         LDAA   MANTSGN2     TRANSFER THE SIGN.
         STAA   MANTSGN1
FLTADDR  CLRB                NO ERRORS.
         RTS                 RETURN.
FLTADD1  TST    FPACC2EX     CHECK FP2 EXPONENT
         BEQ    FLTADDR      RETURN IF ZERO
         LDAB   MANTSGN1     LOAD SIGN FROM 1
         EORB   MANTSGN2     DECIDE TO ADD OR SUBTRACT
         SUBA   FPACC2EX     COMPARE EXPONENTS
         BCS    FLTADD2      IF FPACC2 > FPACC1
         CMPA   #24          IF FPACC1 >> FPACC2
         BHI    FLTADDR      RETURN UNDERFLOW
         PSHY                SAVE Y REGISTER
         PSHA                SAVE SHIFT COUNTER
         LDX    #FPACC2MN    ADDEND TO BE DENORMALIZED POINTED TO BY X
         LDY    #FPACC1MN    ADDEND TO BE LEFT NORMALIZED POINTED TO BY Y
         BRA    FLTADD3      GO DO NORMALIZATION
FLTADD2  NEGA                CHANGE SIGN OF A FOR SHIFT COUNTER
         CMPA   #24          IF FPACC2 >> FPACC1
         BHI    TFR2TO1      PUT FPACC2 INTO FPACC1 AND RETURN
         PSHY                SAVE Y REGISTER
         PSHA                SAVE SHIFT COUNTER
         LDAA   FPACC2EX     LOAD EXPONENT FROM 2
         STAA   FPACC1EX     ... AND TRANSFER IT TO 1
         LDAA   MANTSGN2     LOAD SIGN FROM 2
         STAA   MANTSGN1     ... AND TRANSFER IT TO 1
         LDX    #FPACC1MN    ADDEND TO BE DENORMALIZED POINTED TO BY X
         LDY    #FPACC2MN    ADDEND TO BE LEFT NORMALIZED POINTED TO BY Y
FLTADD3  TBA                 PUT ADD/SUBTRACT FLAG BYTE IN BOTH A AND B
         PSHA                CREATE 3 BYTE STACK WORKSPACE SET TO $00 FOR ...
         PSHA                ... ADDITION AND $FF FOR SUBTRACTION.
         PSHA
         EORA   0,X          LOAD MANTISSA MSB; COMPLEMENT IF SUBTRACTION
         PSHA                PUT IN STACK WORKSPACE
         TBA                 PUT ADD/SUBTRACT FLAG INTO A AGAIN
         EORA   1,X          LOAD MANTISSA NSB; COMPLEMENT IF SUBTRACTION
         PSHA                PUT IN STACK WORKSPACE
         TBA                 PUT ADD/SUBTRACT FLAG INTO A AGAIN
         EORA   2,X          LOAD MANTISSA LSB; COMPLEMENT IF SUBTRACTION
         PSHA                PUT IN STACK WORKSPACE
         TSX                 PUT POINTER TO WORKSPACE IN X
         LDAA   6,X          GET SHIFT COUNTER
         STAB   6,X          SAVE ADD/SUBTRACT FLAG
         TAB                 B REGISTER WILL BE BYTE DISPLACEMENT COUNTER
         ANDA   #$07         REMOVE BYTE DISPLACEMENT FROM A
         INCA                PRE-INCREMENT SHIFT COUNTER
         ANDB   #$18         REMOVE SHIFT DISPLACEMENT FROM B
         LSRB                RIGHT JUSTIFY BYTE DISPLACEMENT COUNTER
         LSRB
         LSRB
         ABX                 DO BYTE DISPLACEMENT BY ADDING TO POINTER
         LDAB   3,X          LOAD ADD/SUBTRACT FLAG (OK SINCE FLAG IN 4 BYTES)
FLTADD4  DECA                DECREMENT BYTE SHIFT COUNTER
         BEQ    FLTADD5      IF DONE SHIFTING
         RORB                SHIFT ADD/SUBTRACT FLAG INTO CARRY
         ROR    2,X          SHIFT MANTISSA MSB
         ROR    1,X          SHIFT MANTISSA NSB
         ROR    0,X          SHIFT MANTISSA LSB
         BRA    FLTADD4      GO AROUND AGAIN
FLTADD5  RORB                SHIFT ADD/SUBTRACT FLAG INTO CARRY
         LDAA   2,Y          LOAD NORMALIZED ADDEND LSB
         ADCA   0,X          ADD IN DENORMALIZED ADDEND LSB
         STAA   FPACC1MN+2   STORE NORMALIZED SUM LSB
         LDAA   1,Y          LOAD NORMALIZED ADDEND NSB
         ADCA   1,X          ADD IN DENORMALIZED ADDEND NSB
         STAA   FPACC1MN+1   STORE NORMALIZED SUM NSB
         LDAA   0,Y          LOAD NORMALIZED ADDEND NSB
         ADCA   2,X          ADD IN DENORMALIZED ADDEND NSB
         STAA   FPACC1MN     STORE NORMALIZED SUM MSB
         TSX                 RESTORE WORKSPACE POINTER
         LDAA   6,X          GET BACK ADD/SUBTRACT FLAG
         BMI    FLTADD6      OPERATION WAS SUBTRACTION
         BCC    FLTADD7      IF ADD AND NO CARRY, DO NOTHING
         ROR    FPACC1MN     CARRY WAS SET; ROTATE TO NORMALIZE OVERFLOW
         ROR    FPACC1MN+1
         ROR    FPACC1MN+2
         INC    FPACC1EX     NOW INCREMENT EXPONENT TO ACCOMODATE SHIFT
         BNE    FLTADD7      SUCCESSFUL COMPLETION
         LDD    #RTNMAX      OVERFLOW - RETURN MAXIMUM NUMBER
         BRA    FLTADD11
FLTADD6  BCS    FLTADD8      NO SIGN CHANGE
         COM    MANTSGN1     CHANGE SIGN OF RESULT
         NEG    FPACC1MN+2   CHANGE SIGN OF LSB
         BCC    FLTADD12     IF LSB WAS ZERO
         INC    FPACC1MN+1   CARRY FROM LSB
         BNE    FLTADD12     IF NSB NOT ZERO
         INC    FPACC1MN     CARRY FROM NSB
FLTADD12 NEG    FPACC1MN+1   CHANGE SIGN OF NSB
         BCC    FLTADD13     IF NSB WAS ZERO
         INC    FPACC1MN     CARRY FROM NSB
FLTADD13 NEG    FPACC1MN     CHANGE SIGN OF MSB
FLTADD8  TST    FPACC1MN     IS MANTISSA NORMALIZED?
         BMI    FLTADD7      YES - DONE
         BNE    FLTADD9      DO BIT SHIFT
         LDAA   FPACC1EX     DECREMENT EXPONENT BY 8 FOR BYTE SHIFT
         SUBA   #8
         BLS    FLTADD10     IF UNDERFLOW
         STAA   FPACC1EX     REPLACE DECREMENTED EXPONENT
         LDD    FPACC1MN+1   DO BYTE SHIFT
         BEQ    FLTADD10     IF RESULT MANTISSA IS ZERO
         CLR    FPACC1MN+2   CLEAR BYTE 3
         STD    FPACC1MN     STORE BYTES 1 AND 2
         BRA    FLTADD8
FLTADD9  DEC    FPACC1EX     DECREMENT EXPONENT TO ACCOMODATE SHIFT
         BEQ    FLTADD10     IF UNDERFLOW
         LSL    FPACC1MN+2   DO BIT SHIFT
         ROL    FPACC1MN+1
         ROL    FPACC1MN
         BPL    FLTADD9
FLTADD7  LDY    7,X          PULL Y FROM STACK
         LDAB   #9           RESTORE STACK - ADD 7 TO X (STACK BASE)
         ABX                 DO ADDITION
         TXS                 NEW STACK BASE (EFFECTIVELY PULLED STACK)
         CLRB                NO ERRORS
         RTS                 RETURN
*
FLTADD10 LDD    #RTNZERO     UNDERFLOW - RETURN ZERO
FLTADD11 LDY    7,X          PULL Y FROM STACK
         STD    7,X          PUT RETURN ADDRESS ON STACK
         LDAB   #7           RESTORE STACK - ADD 5 TO X (STACK BASE)
         ABX                 DO ADDITION
         TXS                 NEW STACK BASE (EFFECTIVELY PULLED STACK)
         RTS                 RETURN THROUGH OVERFLOW OR UNDERFLOW
*
*
*
******************************************************************************
*                                                                            *
*                         FLOATING POINT DIVIDE                              *
*                                                                            *
*        This subroutine performs signed floating point divide. The          *
*        operation performed is FPACC1/FPACC2.  The divisor (FPACC2) is left *
*        unaltered and the answer is placed in FPACC1.  There are several    *
*        error conditions that can be returned by this routine.  They are:   *
*        a) division by zero.  b) overflow.  c) underflow.  As with all      *
*        other routines, an error is indicated by the carry being set and    *
*        the error code being in the A-reg.                                  *
*                                                                            *
*                          WORSE CASE = 495 CYCLES = 248 uS @ 2MHz           *
*                          WORSE CASE = 495 CYCLES = 124 uS @ 4MHz           *
*                                                                            *
******************************************************************************
*
*
FLTDIV   EQU    *
         LDAA   MANTSGN2     GET FPACC2 MANTISSA SIGN.
         EORA   MANTSGN1     SET THE SIGN OF THE RESULT.
         STAA   MANTSGN1     SAVE THE RESULT.
         TST    FPACC2EX     IS THE DIVISOR 0?
         BNE    FLTDIV1      NO. GO SEE IF THE DIVIDEND IS ZERO.
         LDAA   #DIV0ERR     YES. RETURN A DIVIDE BY ZERO ERROR.
         JMP    RTNDIV0      FLAG ERROR AND RETURN.
FLTDIV1  TST    FPACC1EX     IS THE DIVIDEND 0?
         BNE    FLTDIV2      NO. GO PERFORM THE DIVIDE.
         CLRB                YES. ANSWER IS ZERO. NO ERRORS.
         CLR    MANTSGN1     SIGN OF ZERO IS POSITIVE.
         RTS                 RETURN.
FLTDIV2  LDAA   FPACC1EX     NOW COMPUTE EXPONENT - GET NUMERATOR EXPONENT
         SUBA   FPACC2EX     SUBTRACT DENOMINATOR EXPONENT
         BHS    FLTDIV8      IF CARRY CLEAR, RESULT MUST BE POSITIVE
         BMI    FLTDIV6      RESULT IN RANGE
         JMP    RTNZERO      UNDERFLOW - RETURN ZERO
FLTDIV8  BMI    FLTDIV7      OVERFLOW - RETURN MAX. NUMBER
FLTDIV6  ADDA   #$7E         PUT BACK EXPONENT BIAS
         STAA   FPACC1EX     STORE CORRECTED QUOTIENT EXPONENT
         LDD    FPACC1MN     TO DO DIVIDE, NUMERATOR MANTISSA MUST BE LESS ...
         SUBD   FPACC2MN     ... THAN DENOMINATOR MANTISSA.
         BMI    FLTDIV3      BRANCH IF NUMERATOR LESS THAN DENOMINATOR MANTISSA
         BNE    FLTDIV4      TEST LSBs ONLY IF NUMERATOR = DENOMINATOR
         LDAA   FPACC1MN+2   CHECK LSBs SINCE BYTES 1 AND 2 OF ...
         SUBA   FPACC2MN+2   NUMERATOR AND DENOMINATOR ARE SAME.
         BMI    FLTDIV3      BRANCH IF NUMERATOR LESS THAN DENOMINATOR MANTISSA
FLTDIV4  LDD    FPACC1MN     DIVIDE NUMERATOR BY 2 SO IT IS SMALLER THAN DENOM
         LSRD                SHIFT TO DIVIDE BYTES 1 AND  2
         ROR    FPACC1MN+2   SHIFT BYTE 3
         STD    FPACC1MN     PUT BYTES 1 AND 2 BACK
         INC    FPACC1EX     NOW INCREMENT EXPONENT
         BNE    FLTDIV3      CHECK FOR OVERFLOW
FLTDIV7  JMP    RTNMAX       RETURN MAXIMUM MAGNITUDE NUMBER
FLTDIV3  LDD    FPACC1MN     GET PARTIAL NUMERATOR
         LDX    FPACC2MN     GET PARTIAL DENOMINATOR
         INX                 PARTIAL QUOTIENT MUST BE LESS THAN FULL QUOTIENT
         BNE    FLTDIV5      IF X REGISTER ROLLED OVER, WE BLEW IT ...
         DEX                 ... SO RESTORE X REGISTER TO WHERE IT WAS
FLTDIV5  FDIV                COMPUTE PARTIAL QUOTIENT
         PSHY                SAVE Y REGISTER
         LDY    FPACC1MN     TEMPORARY STORAGE FOR NUMERATOR
         STX    FPACC1MN     PARTIAL QUOTIENT TO MULTIPLY BY DENOMINATOR
         PSHX                SAVE PARTIAL QUOTIENT
         LDX    #0           SET UP WORK SPACE ON THE STACK.
         PSHX
         PSHX
         TSX                 SET UP POINTER TO WORK SPACE.
         LDAA   FPACC2MN+2   MULTIPLY DENOMINATOR LS BYTE ...
         LDAB   FPACC1MN+1   ... BY NUMERATOR LS BYTE
         MUL
         STAA   3,X          SAVE PARTIAL RESULT ON STACK
         JSR    REMCOMP      JUMP INTO FLOATING MULTIPLY TO FINISH OPERATION
         STY    FPACC1MN     RESTORE NUMERATOR
         LDD    FPACC1MN+1   MOVE NUMERATOR BYTES 2 AND 3 TO D
         SUBD   1,X          SUBTRACT TO COMPUTE REMAINDER (ASSUME BYTE 1 SAME)
         LDX    FPACC2MN     LOAD DENOMINATOR
         FDIV                DIVIDE REMAINDER BY DENOMINATOR
         STX    FPACC1MN+1   STORE SECOND PARTIAL PRODUCT
         CLR    FPACC1MN     MS BYTE OF SECOND PARTIAL PRODUCT IS ZERO
         PULX                SCRAP STACK WORKSPACE
         PULX
         PULA                RETRIEVE FIRST PARTIAL PRODUCT
         PULB
         PULY                RETRIEVE Y REGISTER
         ADDD   FPACC1MN     ADD TO SECOND PARTIAL PRODUCT TO GET QUOTIENT
         STD    FPACC1MN     SAVE QUOTIENT IN FLOATING ACCUMULATOR
         CLRB                NO ERRORS
         RTS                 DONE - RETURN
*
*
*
******************************************************************************
*                                                                            *
*                FLOATING POINT TO ASCII CONVERSION SUBROUTINE               *
*                                                                            *
*        This subroutine performs floating point to ASCII conversion of      *
*        the number in FPACC1.  The ascii string is placed in a buffer       *
*        pointed to by the X index register.  The buffer must be at least    *
*        14 bytes long to contain the ASCII conversion.  The resulting       *
*        ASCII string is terminated by a zero (0) byte.  Upon exit the       *
*        X Index register will be pointing to the first character of the     *
*        string.  FPACC1 and FPACC2 will remain unchanged.                   *
*                                                                            *
******************************************************************************
*
*
FLTASC   EQU    *
         PSHX                SAVE THE POINTER TO THE STRING BUFFER.
         LDX    #FPACC1EX    POINT TO FPACC1.
         JSR    CHCK0        IS FPACC1 0?
         BNE    FLTASC1      NO. GO CONVERT THE NUMBER.
         PULX                RESTORE POINTER.
         LDD    #$3000       GET ASCII CHARACTER + TERMINATING BYTE.
         STD    0,X          PUT IT IN THE BUFFER.
         RTS                 RETURN.
FLTASC1  LDX    FPACC1EX     SAVE FPACC1.
         PSHX
         LDX    FPACC1MN+1
         PSHX
         LDAA   MANTSGN1
         PSHA
         JSR    PSHFPAC2     SAVE FPACC2.
         LDX    #0
         PSHX                ALLOCATE LOCALS.
         PSHX
         PSHX                SAVE SPACE FOR STRING BUFFER POINTER.
         TSY                 POINT TO LOCALS.
         LDX    15,Y         GET POINTER FROM STACK.
         LDAA   #$20         PUT A SPACE IN THE BUFFER IF NUMBER NOT NEGATIVE.
         TST    MANTSGN1     IS IT NEGATIVE?
         BEQ    FLTASC2      NO. GO PUT SPACE.
         CLR    MANTSGN1     MAKE NUMBER POSITIVE FOR REST OF CONVERSION.
         LDAA   #'-'         YES. PUT MINUS SIGN IN BUFFER.
FLTASC2  STAA   0,X
         INX                 POINT TO NEXT LOCATION.
         STX    0,Y          SAVE POINTER.
FLTASC5  LDX    #N9999999    POINT TO CONSTANT 9999999.
         JSR    GETFPAC2     GET INTO FPACC2.
         JSR    FLTCMP       COMPARE THE NUMBERS. IS FPACC1 > 9999999?
         BGT    FLTASC3      YES. GO DIVIDE FPACC1 BY 10.
         LDX    #P9999999    POINT TO CONSTANT 999999.9
         JSR    GETFPAC2     MOVE IT INTO FPACC2.
         JSR    FLTCMP       COMPARE NUMBERS. IS FPACC1 > 999999.9?
         BGT    FLTASC4      YES. GO CONTINUE THE CONVERSION.
         DEC    2,Y          DECREMENT THE MULT./DIV. COUNT.
         LDX    #CONST10     NO. MULTIPLY BY 10. POINT TO CONSTANT.
FLTASC6  JSR    GETFPAC2     MOVE IT INTO FPACC2.
         JSR    FLTMUL
         BRA    FLTASC5      GO DO COMPARE AGAIN.
FLTASC3  INC    2,Y          INCREMENT THE MULT./DIV. COUNT.
         LDX    #CONSTP1     POINT TO CONSTANT ".1".
         BRA    FLTASC6      GO DIVIDE FPACC1 BY 10.
FLTASC4  LDX    #CONSTP5     POINT TO CONSTANT OF ".5".
         JSR    GETFPAC2     MOVE IT INTO FPACC2.
         JSR    FLTADD       ADD .5 TO NUMBER IN FPACC1 TO ROUND IT.
         LDAB   FPACC1EX     GET FPACC1 EXPONENT.
         SUBB   #$7F         TAKE OUT BIAS.
         NEGB                MAKE IT NEGATIVE.
         ADDB   #23          ADD IN THE NUMBER OF MANTISSA BITS -1.
         BRA    FLTASC17     GO CHECK TO SEE IF WE NEED TO SHIFT AT ALL.
FLTASC7  LSR    FPACC1MN     SHIFT MANTISSA TO THE RIGHT BY THE RESULT (MAKE
         ROR    FPACC1MN+1   THE NUMBER AN INTEGER).
         ROR    FPACC1MN+2
         DECB                DONE SHIFTING?
FLTASC17 BNE    FLTASC7      NO. KEEP GOING.
         LDAA   #1           GET INITIAL VALUE OF "DIGITS AFTER D.P." COUNT.
         STAA   3,Y          INITIALIZE IT.
         LDAA   2,Y          GET DECIMAL EXPONENT.
         ADDA   #8           ADD THE NUMBER OF DECIMAL +1 TO THE EXPONENT.
*                            WAS THE ORIGINAL NUMBER > 9999999?
         BMI    FLTASC8      YES. MUST BE REPRESENTED IN SCIENTIFIC NOTATION.
         CMPA   #8           WAS THE ORIGINAL NUMBER < 1?
         BHS    FLTASC8      YES. MUST BE REPRESENTED IN SCIENTIFIC NOTATION.
         DECA                NO. NUMBER CAN BE REPRESENTED IN 7 DIGITS.
         STAA   3,Y          MAKE THE DECIMAL EXPONENT THE DIGIT COUNT BEFORE
*                            THE DECIMAL POINT.
         LDAA   #2           SETUP TO ZERO THE DECIMAL EXPONENT.
FLTASC8  SUBA   #2           SUBTRACT 2 FROM THE DECIMAL EXPONENT.
         STAA   2,Y          SAVE THE DECIMAL EXPONENT.
         TST    3,Y          DOES THE NUMBER HAVE AN INTEGER PART? (EXP. >0)
         BGT    FLTASC9      YES. GO PUT IT OUT.9
         LDAA   #'.'         NO. GET DECIMAL POINT.
         LDX    0,Y          GET POINTER TO BUFFER.
         STAA   0,X          PUT THE DECIMAL POINT IN THE BUFFER.
         INX                 POINT TO NEXT BUFFER LOCATION.
         TST    3,Y          IS THE DIGIT COUNT TILL EXPONENT =0?
         BEQ    FLTASC18     NO. NUMBER IS <.1
         LDAA   #'0'         YES. FORMAT NUMBER AS .0XXXXXXX
         STAA   0,X          PUT THE 0 IN THE BUFFER.
         INX                 POINT TO THE NEXT LOCATION.
FLTASC18 STX    0,Y          SAVE NEW POINTER VALUE.
FLTASC9  LDX    #DECDIG      POINT TO THE TABLE OF DECIMAL DIGITS.
         LDAA   #7           INITIALIZE THE THE NUMBER OF DIGITS COUNT.
         STAA   5,Y
FLTASC10 CLR    4,Y          CLEAR THE DECIMAL DIGIT ACCUMULATOR.
FLTASC11 LDD    FPACC1MN+1   GET LOWER 16 BITS OF MANTISSA.
         SUBD   1,X          SUBTRACT LOWER 16 BITS OF CONSTANT.
         STD    FPACC1MN+1   SAVE RESULT.
         LDAA   FPACC1MN     GET UPPER 8 BITS.
         SBCA   0,X          SUBTRACT UPPER 8 BITS.
         STAA   FPACC1MN     SAVE RESULT. UNDERFLOW?
         BCS    FLTASC12     YES. GO ADD DECIMAL NUMBER BACK IN.
         INC    4,Y          ADD 1 TO DECIMAL NUMBER.
         BRA    FLTASC11     TRY ANOTHER SUBTRACTION.
FLTASC12 LDD    FPACC1MN+1   GET FPACC1 MANTISSA LOW 16 BITS.
         ADDD   1,X          ADD LOW 16 BITS BACK IN.
         STD    FPACC1MN+1   SAVE THE RESULT.
         LDAA   FPACC1MN     GET HIGH 8 BITS.
         ADCA   0,X          ADD IN HIGH 8 BITS OF CONSTANT.
         STAA   FPACC1MN     SAVE RESULT.
         LDAA   4,Y          GET DIGIT.
         ADDA   #$30         MAKE IT ASCII.
         PSHX                SAVE POINTER TO CONSTANTS.
         LDX    0,Y          GET POINTER TO BUFFER.
         STAA   0,X          PUT DIGIT IN BUFFER.
         INX                 POINT TO NEXT BUFFER LOCATION.
         DEC    3,Y          SHOULD WE PUT A DECIMAL POINT IN THE BUFFER YET?
         BNE    FLTASC16     NO. CONTINUE THE CONVERSION.
         LDAA   #'.'         YES. GET DECIMAL POINT.
         STAA   0,X          PUT IT IN THE BUFFER.
         INX                 POINT TO THE NEXT BUFFER LOCATION.
FLTASC16 STX    0,Y          SAVE UPDATED POINTER.
         PULX                RESTORE POINTER TO CONSTANTS.
         INX                 POINT TO NEXT CONSTANT.
         INX
         INX
         DEC    5,Y          DONE YET?
         BNE    FLTASC10     NO. CONTINUE CONVERSION OF "MANTISSA".
         LDX    0,Y          YES. POINT TO BUFFER STRING BUFFER.
FLTASC13 DEX                 POINT TO LAST CHARACTER PUT IN THE BUFFER.
         LDAA   0,X          GET IT.
         CMPA   #$30         WAS IT AN ASCII 0?
         BEQ    FLTASC13     YES. REMOVE TRAILING ZEROS.
         INX                 POINT TO NEXT AVAILABLE LOCATION IN BUFFER.
         LDAB   2,Y          DO WE NEED TO PUT OUT AN EXPONENT?
         BEQ    FLTASC15     NO. WE'RE DONE.
         LDAA   #'E'         YES. PUT AN 'E' IN THE BUFFER.
         STAA   0,X
         INX                 POINT TO NEXT BUFFER LOCATION.
         LDAA   #'+'         ASSUME EXPONENT IS POSITIVE.
         STAA   0,X          PUT PLUS SIGN IN THE BUFFER.
         TSTB                IS IT REALLY MINUS?
         BPL    FLTASC14     NO. IS'S OK AS IS.
         NEGB                YES. MAKE IT POSITIVE.
         LDAA   #'-'         PUT THE MINUS SIGN IN THE BUFFER.
         STAA   0,X
FLTASC14 INX                 POINT TO NEXT BUFFER LOCATION.
         STX    0,Y          SAVE POINTER TO STRING BUFFER.
         CLRA                SET UP FOR DIVIDE.
         LDX    #10          DIVIDE DECIMAL EXPONENT BY 10.
         IDIV
         PSHB                SAVE REMAINDER.
         XGDX                PUT QUOTIENT IN D.
         ADDB   #$30         MAKE IT ASCII.
         LDX    0,Y          GET POINTER.
         STAB   0,X          PUT NUMBER IN BUFFER.
         INX                 POINT TO NEXT LOCATION.
         PULB                GET SECOND DIGIT.
         ADDB   #$30         MAKE IT ASCII.
         STAB   0,X          PUT IT IN THE BUFFER.
         INX                 POINT TO NEXT LOCATION.
FLTASC15 CLR    0,X          TERMINATE STRING WITH A ZERO BYTE.
         PULX                CLEAR LOCALS FROM STACK.
         PULX
         PULX
         JSR    PULFPAC2     RESTORE FPACC2.
         PULA
         STAA   MANTSGN1
         PULX                RESTORE FPACC1.
         STX    FPACC1MN+1
         PULX
         STX    FPACC1EX
         PULX                POINT TO THE START OF THE ASCII STRING.
         RTS                 RETURN.
*
*
DECDIG   EQU    *
         FCB    $0F,$42,$40  DECIMAL 1,000,000
         FCB    $01,$86,$A0  DECIMAL   100,000
         FCB    $00,$27,$10  DECIMAL    10,000
         FCB    $00,$03,$E8  DECIMAL     1,000
         FCB    $00,$00,$64  DECIMAL       100
         FCB    $00,$00,$0A  DECIMAL        10
         FCB    $00,$00,$01  DECIMAL         1
*
*
P9999999 EQU    *            CONSTANT 999999.9
         FCB    $49,$74,$23,$FE
*
N9999999 EQU    *            CONSTANT 9999999.
         FCB    $4B,$18,$96,$7F
*
*
*
******************************************************************************
*                                                                            *
*                FLOATING POINT COMPARE SUBROUTINE                           *
*                                                                            *
*        This subroutine performs floating point comparison of the arguments *
*        in FPACC1 and FPACC2.  The routine returns Z = 1 if FPACC1 = FPACC2 *
*        and Z = 0 otherwise; N = 1 if FPACC1 < FPACC2 and N = 0 otherwise;  *
*        C = 0, and V = 0 in the condition code register.  The results of    *
*        this comparison may be tested with the twos complement signed       *
*        number conditional branch instructions BLE, BLT, BEQ, BNE, BGT, and *
*        BGE.  FPACC1 and FPACC2 will remain unchanged.                      *
*                                                                            *
******************************************************************************
*
*
FLTCMP   EQU    *
         LDAA   MANTSGN2     IS FPACC2 NEGATIVE?
         BPL    FLTCMP2      NO. CONTINUE WITH COMPARE.
         LDAB   MANTSGN1     IS FPACC1 NEGATIVE?
         BPL    FLTCMP2      NO. CONTINUE WITH COMPARE.
         LDD    FPACC2EX     YES. BOTH ARE NEGATIVE SO COMPARE MUST BE DONE
         CPD    FPACC1EX     BACKWARDS. ARE THEY EQUAL SO FAR?
         BNE    FLTCMP1      NO. RETURN WITH CONDITION CODES SET.
         LDD    FPACC2MN+1   YES. COMPARE LOWER 16 BITS OF MANTISSAS.
         CPD    FPACC1MN+1
         BRA    FLTCMP1
FLTCMP2  CMPA   MANTSGN1     BOTH POSITIVE?
         BNE    FLTCMP1      NO. RETURN WITH CONDITION CODES SET.
         LDD    FPACC1EX     GET FPACC1 EXPONENT & UPPER 8 BITS OF MANTISSA.
         CPD    FPACC2EX     SAME AS FPACC2?
         BNE    FLTCMP1      NO. RETURN WITH CONDITION CODES SET.
         LDD    FPACC1MN+1   GET FPACC1 LOWER 16 BITS OF MANTISSA.
         CPD    FPACC2MN+1   COMPARE WITH FPACC2 LOWER 16 BITS OF MANTISSA.
FLTCMP1  TPA                 GET CONDITION CODES SO WE CAN MANIPULATE BITS
         ANDA   #$F4         CLEAR OVERFLOW, NEGATIVE, AND CARRY BITS
         BCC    FLTCMP3      IF NO CARRY
         ORAA   #$08         SET NEGATIVE BIT
FLTCMP3  TAP                 PUT CONDITION CODES BACK
         RTS                 RETURN WITH CONDITION CODES SET.
*
*
*
******************************************************************************
*                                                                            *
*                     UNSIGNED INTEGER TO FLOATING POINT                     *
*                                                                            *
*        This subroutine performs "unsigned" integer to floating point       *
*        conversion of a 16 bit word.  The 16 bit integer must be in the     *
*        double accumulator D.  The resulting floating point number is       *
*        returned in FPACC1.                                                 *
*                                                                            *
******************************************************************************
*
*
UINT2FLT EQU    *
         LDX    #$008E       LOAD SIGN AND EXPONENT
SINTFLT1 ADDD   #0           CHECK FOR ZERO AND CHECK NORMALIZATION
         BNE    UINTFLT2     NOT ZERO
         JMP    RTNZERO      ZERO - RETURN FLOATING ZERO
UINTFLT1 DEX                 DECREMENT EXPONENT
         LSLD                MULTIPLY MANTISSA BY 2
UINTFLT2 BPL    UINTFLT1     KEEP GOING IF NOT NORMALIZED
         STD    FPACC1MN     SAVE MANTISSA
         CLR    FPACC1MN+2   MANTISSA LSB IS ALWAYS ZERO
         XGDX                GET SIGN AND EXPONENT BYTES
         STAA   MANTSGN1     SAVE SIGN BYTE
         STAB   FPACC1EX     SAVE EXPONENT BYTE
         CLRB                NO ERRORS.
         RTS                 RETURN.
*
*
*
******************************************************************************
*                                                                            *
*                      SIGNED INTEGER TO FLOATING POINT                      *
*                                                                            *
*        This routine works just like the unsigned integer to floating       *
*        point routine except the the 16 bit integer in the double accum-    *
*        ulator D is considered to be in two's complement format.  This      *
*        will return a floating point number in the range -32768 to +32767.  *
*                                                                            *
******************************************************************************
*
*
SINT2FLT EQU    *
         TSTA                CHECK FOR NEGATIVE INTEGER
         BPL    UINT2FLT     NUMBER IS POSITIVE - TREAT IT LIKE UNSIGNED
         COMA                TAKE TWOS COMPLEMENT TO MAKE POSITIVE
         COMB
         ADDD   #1
         LDX    #$FF8E       LOAD SIGN AND EXPONENT
         BRA    SINTFLT1     CONTINUE WITH CONVERSION

*
*
*
******************************************************************************
*                                                                            *
*                         SQUARE ROOT SUBROUTINE                             *
*                                                                            *
*        This routine is used to calculate the square root of the floating   *
*        point number in FPACC1.  If the number in FPACC1 is negative an     *
*        error is returned.                                                  *
*                                                                            *
*                          WORSE CASE = 16354 CYCLES = 8177   uS @ 2MHz      *
*                          WORSE CASE = 16354 CYCLES = 4088.5 uS @ 4MHz      *
*                                                                            *
******************************************************************************
*
*
FLTSQR   EQU    *
         LDX    #FPACC1EX    POINT TO FPACC1.
         JSR    CHCK0        IS IT ZERO?
         BNE    FLTSQR1      NO. CHECK FOR NEGATIVE.
         RTS                 YES. RETURN.
FLTSQR1  TST    MANTSGN1     IS THE NUMBER NEGATIVE?
         BPL    FLTSQR2      NO. GO TAKE ITS SQUARE ROOT.
         LDAA   #NSQRTERR    YES. ERROR.
         SEC                 FLAG ERROR.
         RTS                 RETURN.
FLTSQR2  JSR    PSHFPAC2     SAVE FPACC2.
         LDAA   #4           GET ITERATION LOOP COUNT.
         PSHA                SAVE IT ON THE STACK.
         LDX    FPACC1MN+1   SAVE INITIAL NUMBER.
         PSHX
         LDX    FPACC1EX
         PSHX
         TSY                 POINT TO IT.
         BSR    TFR1TO2      TRANSFER FPACC1 TO FPACC2.
         LDAA   FPACC2EX     GET FPACC1 EXPONENT.
         SUBA   #$7E         REMOVE BIAS FROM EXPONENT.
         INCA                COMPENSATE FOR ODD EXPONENTS (GIVES CLOSER GUESS)
         BPL    FLTSQR3      IF NUMBER >1 DIVIDE EXPONENT BY 2 & ADD BIAS.
         LSRA                IF <1 JUST DIVIDE IT BY 2.
         BRA    FLTSQR4      GO CALCULATE THE SQUARE ROOT.
FLTSQR3  LSRA                DIVIDE EXPONENT BY 2.
         ADDA   #$7E         ADD BIAS BACK IN.
FLTSQR4  STAA   FPACC2EX     SAVE EXPONENT/2.
FLTSQR5  JSR    FLTDIV       DIVIDE THE ORIGINAL NUMBER BY THE GUESS.
         JSR    FLTADD       ADD THE "GUESS" TO THE QUOTIENT.
         DEC    FPACC1EX     DIVIDE THE RESULT BY 2 TO PRODUCE A NEW GUESS.
         BSR    TFR1TO2      PUT THE NEW GUESS INTO FPACC2.
         LDD    0,Y          GET THE ORIGINAL NUMBER.
         STD    FPACC1EX     PUT IT BACK IN FPACC1.
         LDD    2,Y          GET MANTISSA LOWER 16 BITS.
         STD    FPACC1MN+1
         DEC    4,Y          BEEN THROUGH THE LOOP 4 TIMES?
         BNE    FLTSQR5      NO. KEEP GOING.
         LDD    FPACC2EX     THE FINAL GUESS IS THE ANSWER.
         STD    FPACC1EX     PUT IT IN FPACC1.
         LDD    FPACC2MN+1
         STD    FPACC1MN+1
         PULX                GET RID OF ORIGINAL NUMBER.
         PULX
         INS                 GET RID OF LOOP COUNT VARIABLE.
         JSR    PULFPAC2     RESTORE FPACC2.
         CLRB                NO ERRORS.
         RTS
*
*
TFR1TO2  EQU    *
         LDD    FPACC1EX     GET FPACC1 EXPONENT & HIGH 8 BIT OF MANTISSA.
         STD    FPACC2EX     PUT IT IN FPACC2.
         LDD    FPACC1MN+1   GET FPACC1 LOW 16 BITS OF MANTISSA.
         STD    FPACC2MN+1   PUT IT IN FPACC2.
         LDAA   MANTSGN1     TRANSFER THE SIGN.
         STAA   MANTSGN2
         RTS                 RETURN.
*
*
*
******************************************************************************
*                                                                            *
*                        FLOATING POINT EXP(X) AND 10^X                      *
*                                                                            *
******************************************************************************
*
*
FLT10TX  EQU    *
         JSR    PSHFPAC2     SAVE FPACC2
         LDX    #N1DLN10     POINT TO 1/LN(10)
         JSR    GETFPAC2     PUT IT IN FPACC2
         JSR    FLTDIV       COMPUTE LOG BASE 10
         JSR    PULFPAC2     RESTORE FPACC2
FLTETOX  EQU    *
         LDD    FPACC1EX     GET ARGUMENT EXPONENT AND MANTISSA MSB
         CPD    #$85B3       CHECK FOR ARGUMENT IN RANGE (< 89.0)
         BMI    FLTETOX1     IN RANGE
         TST    MANTSGN1     CHECK FOR NEGATIVE ARGUMENT
         BNE    FLTETOX0     NEGATIVE ARGUMENT - UNDERFLOW
         JMP    RTNMAX       OVERFLOW
FLTETOX0 JMP    RTNZERO      UNDERFLOW
FLTETOX1 JSR    PSHFPAC2     SAVE FPACC2
         LDAA   MANTSGN1     SAVE SIGN OF ARGUMENT FOR LATER
         PSHA
         CLR    MANTSGN1     WORK WITH POSITIVE ARGUMENT
         JSR    INTFRAC      SEPARATE IT INTO INTEGRAL AND FRACTIONAL PARTS
         LDD    FPACC2EX     GET EXPONENT OF INTEGER IN A; MANTISSA IN B
         BEQ    FLTETOX6     IF NO INTEGRAL PART OF ARGUMENT
         SUBA   #$86         SET UP SHIFT COUNTER
FLTETOX5 LSRB                SHIFT TO DENORMALIZE INTEGER
         INCA                INCREMENT COUNTER
         BNE    FLTETOX5     IF NOT FINISHED SHIFTING
FLTETOX6 INCB                INTEGRAL PART OF ARG BECOMES ITERATION COUNTER
         PSHB                SAVE INTEGRAL PART FOR NOW
         LDX    #ETOXTBL     POINT TO COEFFICIENT TABLE
         JSR    POLYNOM      EVALUATE FRACTIONAL PART OF E^X
         LDX    #NCONSTE     POINT TO CONSTANT E (2.71828)
         JSR    GETFPAC2     PUT IT INTO FPACC2
         BRA    FLTETOX2     GO DO MULTIPLICATION ITERATION
FLTETOX3 JSR    FLTMUL       MULTIPLY RESULT BY E
FLTETOX2 TSX                 GET BACK POINTER TO ITERATION COUNTER
         DEC    0,X          DECREMENT ITERATION COUNTER
         BNE    FLTETOX3     IF ITERATION COUNTER NOT ZERO, KEEP GOING
         PULA                DISCARD ITERATION COUNTER
         PULA                RETRIEVE SIGN OF ORIGINAL ARGUMENT
         TSTA                WAS IT POSITIVE?
         BPL    FLTETOX4     YES - DONE
FLTRCP1  JSR    TFR1TO2      TAKE RECIPROCAL OF RESULT - MOVE IT TO FPACC2
         LDX    #ONE         POINT TO CONSTANT 1.0
         JSR    GETFPAC1     PUT IT IN FPACC1
         JSR    FLTDIV       TAKE RECIPROCAL
FLTETOX4 JSR    PULFPAC2     RESTORE FPACC2
         CLRB                NO ERRORS
         RTS                 RETURN
*
FLTRECIP EQU    *
         JSR    PSHFPAC2     SAVE FPACC2
         BRA    FLTRCP1      DO RECIPROCAL
*
NCONSTE  FCB    $40,$2D,$F8,$54
*
ETOXTBL  EQU    *
         FCB    $36,$38,$EF,$1D         +(1/9!)
         FCB    $37,$D0,$0D,$01         +(1/8!)
         FCB    $39,$50,$0D,$01         +(1/7!)
         FCB    $3A,$B6,$0B,$61         +(1/6!)
         FCB    $3C,$08,$88,$89         +(1/5!)
         FCB    $3D,$2A,$AA,$AB         +(1/4!)
         FCB    $3E,$2A,$AA,$AB         +(1/3!)
CONSTP5  FCB    $3F,$00,$00,$00         +(1/2!)
         FCB    $3F,$80,$00,$00         +(1/1!)
         FCB    $3F,$80,$00,$00         +(1/0!)
         FCB    $FF
*
*
******************************************************************************
*                                                                            *
*                        FLOATING POINT X ^ Y                                *
*                                                                            *
*        X is in FPACC1, Y is in FPACC2                                      *
*                                                                            *
******************************************************************************
*
*
FLTXTOY  EQU    *
         JSR    FLTLN
         JSR    FLTMUL
         JMP    FLTETOX
*
*
*
******************************************************************************
*                                                                            *
*                        FLOATING POINT NATURAL LOG AND LOG BASE 10          *
*                                                                            *
******************************************************************************
*
*
FLTLGT   EQU    *
         BSR    FLTLN        FIRST FIND NATURAL LOG
         JSR    PSHFPAC2     SAVE FPACC2
         LDX    #N1DLN10     POINT TO 1/LN(10)
         JSR    GETFPAC2     PUT IT IN FPACC2
         JSR    FLTMUL       COMPUTE LOG BASE 10
         JSR    PULFPAC2     RESTORE FPACC2
         CLRB                NO ERRORS
         RTS                 DONE
*
FLTLN    EQU    *
         LDAA   MANTSGN1     CHECK FOR NEGATIVE
         BEQ    LN1          NOT NEGATIVE
LN0      LDAA   #LNNEGERR    NEGATIVE OR ZERO ARGUMENT RETURN
         SEC                 SIGNAL ERROR
         RTS                 RETURN
LN1      LDAA   FPACC1EX     CHECK FOR ZERO
         BEQ    LN0          ZERO - RETURN ERROR
         JSR    PSHFPAC2     SAVE ACCUMULATOR 2
         PSHX                CREATE STACK STORAGE SPACE FOR INTERMEDIATE RESULT
         PSHX
         TSX                 CREATE POINTER TO INTERMEDIATE RESULT
         JSR    PUTFPAC1     SAVE ARGUMENT
         CLRA                GET EXPONENT OF ARGUMENT INTO D TO CONVERT ...
         LDAB   FPACC1EX     ... TO FLOATING POINT.
         SUBB   #$7F         CONVERT EXPONENT FROM EXCESS 127 TO 2'S COMPLEMENT
         BPL    LN2          IF EXPONENT IS POSITIVE, NO NEED TO EXTEND SIGN
         COMA                EXTEND SIGN THROUGH MSB OF D REGISTER
LN2      JSR    SINT2FLT     DO INTEGER TO FLOATING POINT CONVERSION
         LDX    #NLN2        POINT TO CONSTANT LN(2)
         JSR    GETFPAC2     PUT IT INTO FPACC2
         JSR    FLTMUL       NOW HAVE PART OF ANSWER DEPENDENT ON EXPONENT
         TSX                 GET BACK TEMPORARY STORAGE POINTER
         JSR    GETFPAC2     LOAD ORIGINAL ARGUMENT
         JSR    PUTFPAC1     SAVE PARTIAL RESULT
         LDAA   #$7F         SINCE WE HAVE LOG OF EXPONENT, TAKE LOG OF ...
         STAA   FPACC2EX     ... MANTISSA WITH ZERO EXPONENT.
         PSHX                CREATE STORAGE SPACE FOR MANTISSA
         PSHX
         TSX                 GET POINTER TO NEW STORAGE
         JSR    PUTFPAC2     SAVE MANTISSA (M)
         LDX    #ONE         POINT TO CONSTANT ONE
         JSR    GETFPAC1     PUT IT INTO FPACC1
         JSR    FLTADD       (M+1)
         TSX                 GET MANTISSA POINTER BACK
         JSR    GETFPAC2     (M)
         JSR    PUTFPAC1     (M+1)
         LDX    #MONE        POINT TO CONSTANT MINUS ONE
         JSR    GETFPAC1     PUT IT INTO FPACC1
         JSR    FLTADD       (M-1)
         TSX                 GET POINTER TO (M+1)
         JSR    GETFPAC2     PUT IT INTO FPACC2
         JSR    FLTDIV       (M-1)/(M+1)
         TSX                 POINT TO TEMPORARY STORAGE
         JSR    PUTFPAC1     SAVE POWER SERIES VARIABLE
         JSR    TFR1TO2      PUT POWER SERIES VARIABLE INTO FPACC2
         JSR    FLTMUL       SQUARE IT
         LDX    #LNTBL       POINTER TO POLYNOMIAL COEFFICIENTS
         JSR    POLYNOM      COMPUTE POWER SERIES RESULT
         TSX                 GET POINTER TO (M-1)/(M+1)
         JSR    GETFPAC2     PUT IN FPACC2
         JSR    FLTMUL       MULTIPLY TO GET LOG OF MANTISSA
         PULX                DISCARD (M-1)/(M+1)
         PULX
         TSX                 GET POINTER TO LOG OF EXPONENT
         JSR    GETFPAC2     PUT IN FPACC2
         JSR    FLTADD       ADD TO GET COMPLETE LOG RESULT
         PULX                DISCARD LOG OF EXPONENT
         PULX
         JSR    PULFPAC2     RESTORE FPACC2
         CLRB                NO ERRORS
         RTS                 RETURN.
*
*
LNTBL    EQU    *
         FCB    $3E,$1D,$89,$D9         2/13
         FCB    $3E,$3A,$2E,$8C         2/11
         FCB    $3E,$63,$8E,$39         2/9
         FCB    $3E,$92,$49,$25         2/7
         FCB    $3E,$CC,$CC,$CD         2/5
         FCB    $3F,$2A,$AA,$AB         2/3
         FCB    $40,$00,$00,$00         2/1
         FCB    $FF                     END OF TABLE
*
MONE     FCB    $BF,$80,$00,$00         -1.0
*
NLN2     FCB    $3F,$31,$72,$18         LN(2)
*
N1DLN10  FCB    $3E,$DE,$5B,$D9         1/LN(10)
*
*
******************************************************************************
*                                                                            *
*                        FLOATING POINT ARC SINE                             *
*                        FLOATING POINT ARC COSINE                           *
*                        FLOATING POINT ARC TANGENT                          *
*                                                                            *
******************************************************************************
*
*
FLTASIN  EQU    *
         LDAA   #ASINERR     ARC SINE NOT IMPLEMENTED
         SEC
         RTS
FLTACOS  EQU    *
         LDAA   #ACOSERR     ARC COSINE NOT IMPLEMENTED
         SEC
         RTS
FLTATAN  EQU    *
         JSR    PSHFPAC2     SAVE FPACC2
         LDAA   MANTSGN1     GET SIGN OF ARGUMENT
         PSHA                SAVE IT FOR RESULT
         CLR    MANTSGN1     WORK WITH POSITIVE NUMBER (FOR COMPARE)
         LDX    #ONE         POINT TO FLOATING POINT CONSTANT 1.0
         JSR    GETFPAC2     PUT IT ON STACK
         JSR    FLTCMP       CHECK FOR ARGUMENT GREATER THAN 1.0
         BLE    FLTATAN1     IF <= 1.0
         JSR    FLTRECIP     TAKE RECIPROCAL OF ARGUMENT
         LDAA   #$FF         INDICATE > 1.0
         BRA    FLTATAN2     CONTINUE
FLTATAN1 CLRA                INDICATE <= 1.0
FLTATAN2 PSHA                SAVE ON STACK
         PSHX                CREATE STACK SPACE FOR ARGUMENT
         PSHX
         TSX                 POINT TO STORAGE SPACE
         JSR    PUTFPAC1     PUT ARGUMENT ON STACK
         JSR    TFR1TO2      PUT POWER SERIES VARIABLE INTO FPACC2
         JSR    FLTMUL       SQUARE IT
         LDX    #ATANTBL     POINT TO ARC TANGENT TABLE
         JSR    POLYNOM      COMPUTE ARC TANGENT
         TSX                 GET POINTER TO ARGUMENT
         JSR    GETFPAC2     PUT IN FPACC2
         JSR    FLTMUL       MULTIPLY TO GET RESULT
         PULX                DISCARD ARGUMENT
         PULX
         PULA                FIND OUT IF ARGUMENT WAS > 1.0
         TSTA
         BEQ    FLTATAN3     NO - LEAVE RESULT ALONE
         LDX    #PIOV2       YES - SUBTRACT IT FROM PI/2
         JSR    GETFPAC2     PUT PI/2 ON STACK
         COM    MANTSGN1     MAKE RESULT NEGATIVE
         JSR    FLTADD       DO SUBTRACTION
FLTATAN3 JSR    RAD2DEG      CONVERT TO DEGREES
         PULA                RETRIEVE ORIGINAL SIGN
         STAA   MANTSGN1     PUT IT IN RESULT
         JSR    PULFPAC2     RETRIEVE FPACC2
         CLRB                NO ERRORS
         RTS                 RETURN
*
PIOV2    FCB    $3F,$C9,$0F,$DB      1.5707963
*
ATANTBL  EQU    *
         FCB    $BC,$E2,$DD,$1B         -1/19 + FUDGE FACTOR
         FCB    $3D,$70,$F0,$F1         1/17
         FCB    $BD,$88,$88,$89         -1/15
         FCB    $3D,$9D,$89,$D9         1/13
         FCB    $BD,$BA,$2E,$8C         -1/11
         FCB    $3D,$E3,$8E,$39         1/9
         FCB    $BE,$12,$49,$25         -1/7
         FCB    $3E,$4C,$CC,$CD         1/5
         FCB    $BE,$AA,$AA,$AB         -1/3
         FCB    $3F,$80,$00,$00         1/1
         FCB    $FF                     END OF TABLE
*
*
*
******************************************************************************
*                                                                            *
*                        FLOATING POINT SINE & COSINE                        *
*                                                                            *
******************************************************************************
*
*
FLTSIN   EQU    *
         JSR    PSHFPAC2     SAVE FLOATING ACCUMULATOR
         CLRA                OPERATION IS SINE, RESULT IS POSITIVE
         BRA    SIN0         CONTINUE
*
FLTCOS   EQU    *
         JSR    PSHFPAC2     SAVE FLOATING ACCUMULATOR
         LDAA   #$F0         OPERATION IS COSINE, RESULT IS POSITIVE
SIN0     PSHA                SAVE OPERATION/SIGN FLAG
         LDX    #N360        POINT TO FLOATING POINT CONSTANT 360.0
         JSR    GETFPAC2     PUT IT INTO FP ACC 2
         BRA    SIN1         CHECK FOR NEGATIVE ARGUMENT
SIN2     JSR    FLTADD       ADD 360 AND TRY AGAIN
SIN1     LDAA   MANTSGN1     IS ARGUMENT NEGATIVE?
         BNE    SIN2         YES - MUST BE MADE POSITIVE
         BRA    SIN3         CHECK FOR ARGUMENT > 360.0
SIN4     JSR    FLTSUB       SUBTRACT 360 AND TRY AGAIN
SIN3     JSR    FLTCMP       IS ARGUMENT > 360?
         BGT    SIN4         YES - MUST BE LESS THAN 360
         BSR    ANGRED       IF 180 < ARG < 360, ARG = 360 - ARG ...
         BCC    SIN5         ... AND CHANGE SIGN IF SIN FUNCTION.
         TSX                 GET BACK OPERATION/SIGN FLAG
         LDAA   0,X          ARE WE DOING SINE?
         BMI    SIN5         NO - GO DO NEXT REDUCTION
         EORA   #$0F         YES - SINE IS NEGATIVE IN QUADS 3 AND 4
         STAA   0,X          PUT OPERATION/SIGN FLAG BACK
SIN5     BSR    ANGRED       IF 90 < ARG < 180, ARG = 180 - ARG ...
         BCC    SIN6         ... AND CHANGE SIGN IF COS FUNCTION.
         TSX                 GET BACK OPERATION/SIGN FLAG
         LDAA   0,X          ARE WE DOING COSINE?
         BPL    SIN6         NO - GO DO NEXT REDUCTION
         EORA   #$0F         YES - COSINE IS NEGATIVE IN QUADS 2 AND 3
         STAA   0,X          PUT OPERATION/SIGN FLAG BACK
SIN6     BSR    ANGRED       IF 45 < ARG < 90, ARG = 90 - ARG ...
         BCC    SIN7         ... AND CHANGE OPERATION SIN <=> COS.
         TSX                 GET BACK OPERATION/SIGN FLAG
         LDAA   0,X
         EORA   #$F0         CHANGE SINE TO COSINE; COSINE TO SINE
         STAA   0,X          PUT OPERATION/SIGN FLAG BACK
SIN7     LDX    #NPID180     POINT TO FLOATING POINT CONSTANT PI/180
         JSR    GETFPAC2     LOAD INTO FLOATING ACCUMULATOR 2
         JSR    FLTMUL       DO DEGREES TO RADIANS CONVERSION
         JSR    TFR1TO2      COPY ARGUMENT INTO FPACC2
         JSR    FLTMUL       COMPUTE ARGUMENT^2
         PSHX                CREATE STORAGE SPACE FOR ARGUMENT
         PSHX
         TSX                 GET POINTER TO STORAGE
         JSR    PUTFPAC2     SAVE ARGUMENT
         LDAA   4,X          ARE WE DOING SINE?
         BPL    SIN8         YES - LOAD SINE TABLE POINTER
         LDX    #COSTBL      NO - LOAD COSINE TABLE POINTER
         BRA    SIN85
SIN8     LDX    #SINTBL      LOAD SINE TABLE POINTER
SIN85    JSR    POLYNOM      GO DO TAYLOR EXPANSION
         TSX
         LDAA   4,X          GET BACK OPERATION/SIGN FLAG
         ASRA                CHECK FOR NEGATIVE
         BCC    SIN9         IF POSITIVE, LEAVE RESULT ALONE
         COM    MANTSGN1     IF NEGATIVE, COMPLEMENT SIGN
SIN9     TSTA                CHECK SINE/COSINE FLAG
         BMI    SIN10        IF COSINE, WE ARE FINISHED
         JSR    GETFPAC2     GET ARGUMENT BACK
         JSR    FLTMUL       FINAL COMPUTATION FOR SINE
SIN10    PULX                DISCARD STACK TEMPORARIES
         PULX
         INS
         JSR    PULFPAC2     RECOVER FLOATING ACCUMULATOR
         CLRB                NO ERRORS
         RTS                 DONE
*
*
ANGRED   EQU    *
         DEC    FPACC2EX     MAKE N/2 FOR COMPARE
         JSR    FLTCMP       IS ACC1 > N/2?
         BGT    ANGRED1      YES - REDUCE IT
         CLC                 NO REDUCTION
         RTS                 RETURN
ANGRED1  INC    FPACC2EX     RECOVER N
         COM    MANTSGN1     MAKE ACC1 NEGATIVE
         JSR    FLTADD       ACC1 = -ACC1 + N
         DEC    FPACC2EX     BACK TO N/2
         SEC                 SIGNAL REDUCTION
         RTS                 RETURN
*
*
*
*
FLTINT   EQU    *
         JSR    INTFRAC      DO SEPARATION INTO INTEGER AND FRACTIONAL PARTS
*                            FALL THROUGH TO EXCHANGE INTEGER PART INTO FPACC1
EXG1AND2 EQU    *
         LDD    FPACC1EX
         LDX    FPACC2EX
         STD    FPACC2EX
         STX    FPACC1EX
         LDD    FPACC1MN+1
         LDX    FPACC2MN+1
         STD    FPACC2MN+1
         STX    FPACC1MN+1
         LDAA   MANTSGN1
         LDAB   MANTSGN2
         STAA   MANTSGN2
         STAB   MANTSGN1
         CLRB
         RTS                 RETURN.
*
*
SINTBL   EQU    *
         FCB    $36,$38,$EF,$1D         +(1/9!)
         FCB    $B9,$50,$0D,$01         -(1/7!)
         FCB    $3C,$08,$88,$89         +(1/5!)
         FCB    $BE,$2A,$AA,$AB         -(1/3!)
ONE      FCB    $3F,$80,$00,$00         +(1/1!)
         FCB    $FF
*
*
COSTBL   EQU    *
         FCB    $37,$D0,$0D,$01         +(1/8!)
         FCB    $BA,$B6,$0B,$61         -(1/6!)
         FCB    $3D,$2A,$AA,$AB         +(1/4!)
         FCB    $BF,$00,$00,$00         -(1/2!)
         FCB    $3F,$80,$00,$00         +(1/1!)
         FCB    $FF
*
*
PI       FCB    $40,$49,$0F,$DB         3.1415927
N360     FCB    $43,$B4,$00,$00         360.0
*
*
*
******************************************************************************
*                                                                            *
*                        FLOATING POINT TANGENT                              *
*                                                                            *
******************************************************************************
*
*
FLTTAN   EQU    *
         JSR    PSHFPAC2      SAVE FPACC2 ON THE STACK.
         JSR    TFR1TO2       PUT A COPY OF THE ANGLE IN FPACC2.
         JSR    FLTCOS        GET COSINE OF THE ANGLE.
         JSR    EXG1AND2      PUT RESULT IN FPACC2 & PUT ANGLE IN FPACC1.
         JSR    FLTSIN        GET SIN OF THE ANGLE.
         JSR    FLTDIV        GET TANGENT OF ANGLE BY DOING SIN/COS.
         BCC    FLTTAN1       IF CARRY CLEAR, ANSWER OK.
         JSR    PULFPAC2      RESTORE FPACC2
         LDAA   #TAN90ERR     GET ERROR CODE IN B.
         SEC                  FLAG ERROR
         RTS                  RETURN
FLTTAN1  JSR    PULFPAC2      RESTORE FPACC2.
         CLRB                 NO ERRORS
         RTS                  RETURN.
*
*
MAXNUM   EQU    *
         FCB    $7F,$FF,$FF,$FF         LARGEST POSITIVE NUMBER WE CAN HAVE.
*
*
*
******************************************************************************
*                                                                            *
*                              TRIG UTILITIES                                *
*                                                                            *
*        The routines "DEG2RAD" and "RAD2DEG" are used to convert angles     *
*        from degrees-to-radians and radians-to-degrees respectively. The    *
*        routine "GETPI" will place the value of PI into FPACC1. This        *
*        routine should be used if the value of PI is needed in calculations *
*        since it is accurate to the full 24-bits of the mantissa.           *
*                                                                            *
******************************************************************************
*
*
DEG2RAD  EQU    *
         JSR    PSHFPAC2     SAVE FPACC2.
         LDX    #NPID180     POINT TO CONVERSION CONSTANT PI/180.
DEG2RAD1 JSR    GETFPAC2     PUT IT INTO FPACC2.
         JSR    FLTMUL       CONVERT DEGREES TO RADIANS.
         JSR    PULFPAC2     RESTORE FPACC2.
         RTS                 RETURN. (NOTE! DON'T REPLACE THE "JSR/RTS" WITH
*                            A "JMP" IT WILL NOT WORK.)
*
*
RAD2DEG  EQU    *
         JSR    PSHFPAC2     SAVE FPACC2.
         LDX    #N180DPI     POINT TO CONVERSION CONSTANT 180/PI.
         BRA    DEG2RAD1     GO DO CONVERSION & RETURN.
*
*
GETPI    EQU    *
         LDX    #PI          POINT TO CONSTANT "PI".
         JMP    GETFPAC1     PUT IT IN FPACC1 AND RETURN.
*
*
NPID180  EQU    *
         FCB    $3C,$8E,$FA,$31
*
N180DPI  EQU    *
         FCB    $42,$65,$2E,$E1
*
*
*
******************************************************************************
*                                                                            *
*        POLYNOM evaluates a polynomial with constant coefficients.  On      *
*        entry, FPACC1 contains the independent variable.  The X register    *
*        contains a pointer to a table of floating point coefficients,       *
*        stored with the highest order coefficient first.  The polynomial    *
*        is of arbitrary order; evaluation ends when $FF is encountered      *
*        after the last (lowest order) coefficient in the coefficient table. *
*                                                                            *
******************************************************************************
*
*
POLYNOM  EQU    *
         XGDX                HOLD COEFFICIENT TABLE POINTER IN D
         LDX    FPACC1EX     SAVE F.P. ARGUMENT ON STACK (NOT MEMORY FORMAT!)
         PSHX
         LDX    FPACC1MN+1
         PSHX
         XGDX                GET COEFFICIENT TABLE POINTER BACK
         LDAA   MANTSGN1     SAVE F.P. ARGUMENT SIGN ON STACK
         PSHA
         PSHX                SAVE COEFFICIENT TABLE POINTER
         LDD    #0           CLEAR RESULT ACCUMULATOR
         STD    FPACC1EX
         STD    FPACC1MN+1
         STD    FPACC1MN+2
         BRA    POLY1
POLY2    TSX                 GET POINTER TO COEFFICIENT TABLE POINTER
         LDAA   2,X          PUT INDEPENDENT VARIABLE INTO ACCUMULATOR 2
         STAA   MANTSGN2
         LDD    3,X
         STD    FPACC2MN+1
         LDD    5,X
         STD    FPACC2EX
         JSR    FLTMUL       DO MULTIPLICATION
POLY1    TSX                 GET POINTER TO COEFFICIENT TABLE POINTER
         LDX    0,X          GET POINTER TO COEFFICIENT TABLE
         JSR    GETFPAC2     PUT COEFFICIENT INTO ACCUMULATOR
         JSR    FLTADD       ADD TO RESULT
         PULX                GET COEFFICIENT TABLE POINTER OFF STACK
         LDAB   #4           INCREMENT COEFFICIENT POINTER
         ABX
         PSHX                SAVE NEW COEFFICIENT POINTER
         LDAA   0,X          CHECK FOR END OF TABLE
         COMA                IF IT WAS $FF, WE ARE AT END
         BNE    POLY2        NOT AT END - KEEP GOING
         TSX                 DISCARD COEFFICIENT POINTER AND ARGUMENT
         LDAB   #7           NUMBER OF BYTES TO PULL OFF STACK
         ABX                 ADD 7 TO STACK POINTER
         TXS                 PUT STACK POINTER BACK
         RTS                 DONE
*
*
*
******************************************************************************
*                                                                            *
*        The following two subroutines, PSHFPAC2 & PULPFAC2, push FPACC2     *
*        onto and pull FPACC2 off of the hardware stack respectively.        *
*        The number is stored in the "memory format".                        *
*                                                                            *
******************************************************************************
*
*
PSHFPAC2 EQU    *
         PULX                GET THE RETURN ADDRESS OFF OF THE STACK.
         PSHX                ALLOCATE FOUR BYTES OF STACK SPACE.
         PSHX
         XGDX                PUT THE RETURN ADDRESS IN D.
         TSX                 POINT TO THE STORAGE AREA.
         PSHB                PUT THE RETURN ADDRESS BACK ON THE STACK.
         PSHA
         JMP    PUTFPAC2     GO PUT FPACC2 ON THE STACK & RETURN.
*
*
PULFPAC2 EQU    *
         TSX                 POINT TO THE RETURN ADDRESS.
         INX                 POINT TO THE SAVED NUMBER.
         INX
         JSR    GETFPAC2     RESTORE FPACC2.
         PULX                GET THE RETURN ADDRESS OFF THE STACK.
         INS                 REMOVE THE NUMBER FROM THE STACK.
         INS
         INS
         INS
         JMP    0,X          RETURN.
*
*
*
******************************************************************************
*                                                                            *
*                           GETFPACx SUBROUTINE                              *
*                                                                            *
*       The GETFPAC1 and GETFPAC2 subroutines get a floating point number    *
*       stored in memory and put it into either FPACC1 or FPACC2 in a format *
*       that is expected by all the floating point math routines. These      *
*       routines convert the IEEE binary floating point format to the format *
*       required by the math routines.  The IEEE format converted by these   *
*       routines is shown below:                                             *
*                                                                            *
*       31 30_______23 22_____________________0                              *
*       s   exponent          mantissa                                       *
*                                                                            *
*       The exponent is biased by 127 to facilitate floating point           *
*       comparisons.  The sign bit is 0 for positive numbers and 1           *
*       for negative numbers.  The mantissa is stored in hidden bit          *
*       normalized format so that 24 bits of precision can be obtained.      *
*       Since a normalized floating point number always has its most         *
*       significant bit set, we can use the 24th bit to hold the exponent    *
*       LSB.  This allows us to get 24 bits of precision in the mantissa     *
*       and store the entire number in just 4 bytes.  The format required by *
*       the math routines uses a seperate byte for the sign, therfore each   *
*       floating point accumulator requires five bytes.                      *
*                                                                            *
******************************************************************************
*
*
RETONE   LDX    #ONE         POINT TO CONSTANT 1.0
GETFPAC1 EQU    *
         CLR    MANTSGN1     SET UP FOR POSITIVE NUMBER.
         LDD    2,X          GET LOW 16-BITS OF THE MANTISSA.
         STD    FPACC1MN+1   PUT IN FPACC1.
         LDD    0,X          GET THE EXPONENT & HIGH BYTE OF THE MANTISSA
         LSLD                SHIFT SIGN INTO CARRY; EXPONENT INTO ACCA
         BCC    GETFP11      IF NUMBER IS POSITIVE, SKIP SETTING THE SIGN BYTE
         COM    MANTSGN1     SET SIGN TO NEGATIVE.
GETFP11  STAA   FPACC1EX     STORE EXPONENT; CHECK FOR ZERO
         BEQ    GETFP12      IF NUMBER IS ZERO, DON'T SET MANTISSA MSB
         SEC                 SET CARRY TO SHIFT INTO MANTISSA MSB
         RORB                NORMALIZED MANTISSA NOW IN B
GETFP12  STAB   FPACC1MN     PUT IN FPACC1.
         CLRB                NO ERRORS.
         RTS                 RETURN.
*
*
GETFPAC2 EQU    *
         CLR    MANTSGN2     SET UP FOR POSITIVE NUMBER.
         LDD    2,X          GET LOW 16-BITS OF THE MANTISSA.
         STD    FPACC2MN+1   PUT IN FPACC2.
         LDD    0,X          GET THE EXPONENT & HIGH BYTE OF THE MANTISSA
         LSLD                SHIFT SIGN INTO CARRY; EXPONENT INTO ACCA
         BCC    GETFP21      IF NUMBER IS POSITIVE, SKIP SETTING THE SIGN BYTE
         COM    MANTSGN2     SET SIGN TO NEGATIVE.
GETFP21  STAA   FPACC2EX     STORE EXPONENT; CHECK FOR ZERO
         BEQ    GETFP22      IF NUMBER IS ZERO, DON'T SET MANTISSA MSB
         SEC                 SET CARRY TO SHIFT INTO MANTISSA MSB
         RORB                NORMALIZED MANTISSA NOW IN B
GETFP22  STAB   FPACC2MN     PUT IN FPACC2.
         RTS                 RETURN.
*
*
*
******************************************************************************
*                                                                            *
*                        PUTFPACx SUBROUTINE                                 *
*                                                                            *
*       These two subroutines perform to opposite function of GETFPAC1 and   *
*       GETFPAC2. Again, these routines are used to convert from the         *
*       internal format used by the floating point package to the IEEE       *
*       floating point format. See the GETFPAC1 and GETFPAC2, documentation  *
*       for a description of the IEEE format.                                *
*                                                                            *
******************************************************************************
*
*
PUTFPAC1 EQU    *
         LDD    FPACC1MN+1   GET L.S. 16 BITS OF THE MANTISSA.
         STD    2,X          SAVE IT
         LDD    FPACC1EX     GET FPACC1 EXPONENT & UPPER 8 BITS OF MANT.
         LSLB                DROP MANTISSA MSB (IMPLIED), ALSO MAKE ACCB < $FF
         CMPB   MANTSGN1     SIGN BIT INTO CARRY. (B-$FF => C SET; B-0 =>C CLR)
         RORA                NOW ACCA HAS SIGN:EXPONENT[7-1]; EXPONENT[0] => C
         RORB                NOW ACCB HAS EXPONENT[0]:MANTISSA[22-16]
         STD    0,X          SAVE IT IN MEMORY
         RTS
*
*
PUTFPAC2 EQU    *
         LDD    FPACC2MN+1   GET L.S. 16 BITS OF THE MANTISSA.
         STD    2,X          SAVE IT
         LDD    FPACC2EX     GET FPACC2 EXPONENT & UPPER 8 BITS OF MANT.
         LSLB                DROP MANTISSA MSB (IMPLIED), ALSO MAKE ACCB < $FF
         CMPB   MANTSGN2     SIGN BIT INTO CARRY. (B-$FF => C SET; B-0 =>C CLR)
         RORA                NOW ACCA HAS SIGN:EXPONENT[7-1]; EXPONENT[0] => C
         RORB                NOW ACCB HAS EXPONENT[0]:MANTISSA[22-16]
         STD    0,X          SAVE IT IN MEMORY
         RTS
*
FLTABS   EQU    *
         CLR    MANTSGN1     TAKE ABSOLUTE VALUE
         CLRB                RETURN PROPER CONDITION CODE
         RTS
*
FLTSGN   EQU    *
         TST    FPACC1MN     CHECK FOR ZERO
         BEQ    FLTSGNZ      DO NOTHING IF ZERO
         LDD    #$7F80       MANTISSA/EXPONENT IS 1.000
         STD    FPACC1EX     SAVE EXPONENT AND MANTISSA HIGH BYTE
FLTSGNZ  CLRA                MID BYTE IS ZERO
         CLRB                LOW BYTE IS ZERO
         STD    FPACC1MN+1   SAVE MANTISSA LOW BYTES
         RTS
*
FLTMIN   EQU    *
         TST    FPACC1MN     CHECK FOR ZERO
         BEQ    FLTMINZ      DO NOTHING IF ZERO
         COM    MANTSGN1     CHANGE SIGN
FLTMINZ  CLRB                CONDITION CODE 0
         RTS
*
