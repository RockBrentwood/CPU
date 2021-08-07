1     RFRV20     08/29/89 12:45:36     WACCVM2 MAIL

 _______________(Message From PROFS on 08/29/89 at 12:45:25)___________

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
*
*
*
         ORG    œ0000
*
FPACC1EX RMB    1            FLOATING POINT ACCUMULATOR #1..
FPACC1MN RMB    3
MANTSGN1 RMB    1            MANTISSA SIGN FOR FPACC1 (0=+, FF=-).
FPACC2EX RMB    1            FLOATING POINT ACCUMULATOR #2.
FPACC2MN RMB    3
MANTSGN2 RMB    1            MANTISSA SIGN FOR FPACC2 (0=+, FF=-).
*
*
FLTFMTER EQU    1           /* floating point format error in ASCFLT */
OVFERR   EQU    2           /* floating point overflow error */
UNFERR   EQU    3           /* floating point underflow error */
DIV0ERR  EQU    4           /* division by 0 error */
TOLGSMER EQU    5           /* number too large or small to convert to int. */
NSQRTERR EQU    6           /* tried to take the square root of negative # */
TAN90ERR EQU    7           /* TANgent of 90 degrees attempted */
*
*
         TTL    ASCFLT
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
*       The exponent is biased by 128 to facilitate floating point           *
*       comparisons.  A pointer to the ASCII string is passed to the         *
*       routine in the D-register.                                           *
*                                                                            *
*                                                                            *
******************************************************************************
*
*
*        ORG    œ0000
*
*        FPACC1EX RMB    1            FLOATING POINT ACCUMULATOR #1..
*        FPACC1MN RMB    3
*        MANTSGN1 RMB    1            MANTISSA SIGN FOR FPACC1 (0=+, FF=-).
*        FPACC2EX RMB    1            FLOATING POINT ACCUMULATOR #2.
*        FPACC2MN RMB    3
*        MANTSGN2 RMB    1            MANTISSA SIGN FOR FPACC2 (0=+, FF=-).
*
*
*        FLTFMTER EQU    1
*
*
*        LOCAL VARIABLES (ON STACK POINTED TO BY Y)
*
EXPSIGN  EQU    0            EXPONENT SIGN (0=+, FF=-).
PWR10EXP EQU    1            POWER 10 EXPONENT.
*
*
         ORG    œC000        (TEST FOR EVB)
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
ASCFLT2  CMPA   #'-          NO. IS IT A MINUS SIGN?
         BNE    ASCFLT3      NO. GO CHECK FOR DECIMAL POINT.
         COM    MANTSGN1     YES. SET MANTISSA SIGN. LEADING MINUS BEFORE?
         INX                 POINT TO NEXT CHARACTER.
         LDAA   0,X          GET IT.
         JSR    NUMERIC      IS IT A NUMBER?
         BCS    ASCFLT4      YES. GO PROCESS IT.
*
*        LEADING DECIMAL POINT?
*

ASCFLT3  CMPA   #'.          IS IT A DECIMAL POINT?
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
         CMPA   #'.          NO. IS IT A DECIMAL POINT?
         BNE    ASCFLT7      NO. GO CHECK FOR THE EXPONENT.
*
*        ANY FRACTIONAL DIGITS ARE NOT SIGNIFIGANT
*
ASCFLT8  LDAA   0,X          GET THE NEXT CHARACTER.
         JSR    NUMERIC      IS IT A DIGIT?
         BCC    ASCFLT7      NO. GO CHECK FOR AN EXPONENT.
         INX                 POINT TO THE NEXT CHARACTER.
         BRA    ASCFLT8      FLUSH REMAINING DIGITS.
ASCFLT7  CMPA   #'E          NO. IS IT THE EXPONENT?
         BEQ    ASCFLT13     YES. GO PROCESS IT.
         JMP    FINISH       NO. GO FINISH THE CONVERSION.
*
*        PROCESS THE EXPONENT
*
ASCFLT13 INX                 POINT TO NEXT CHARACTER.
         LDAA   0,X          GET THE NEXT CHARACTER.
         JSR    NUMERIC      SEE IF IT'S A DIGIT.
         BCS    ASCFLT9      YES. GET THE EXPONENT.
         CMPA   #'-          NO. IS IT A MINUS SIGN?
         BEQ    ASCFLT15     YES. GO FLAG A NEGATIVE EXPONENT.
         CMPA   #'+          NO. IS IT A PLUS SIGN?
         BEQ    ASCFLT16     YES. JUST IGNORE IT.
         BRA    ASCFLT5      NO. FORMAT ERROR.
ASCFLT15 COM    EXPSIGN,Y    FLAG A NEGATIVE EXPONENT. IS IT 1ST?
ASCFLT16 INX                 POINT TO NEXT CHARACTER.
         LDAA   0,X          GET NEXT CHARACTER.
         JSR    NUMERIC      IS IT A NUMBER?
         BCC    ASCFLT5      NO. FORMAT ERROR.
ASCFLT9  SUBA   #œ30         MAKE IT BINARY.
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
         SUBA   #œ30         MAKE SECOND DIGIT BINARY.
         ABA                 ADD IT TO FIRST DIGIT.
         STAA   PWR10EXP,Y
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
ASCFLT10 CMPA   #'.          IS IT A DECIMAL POINT?
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
         ADCA   #0           ADDIN POSSABLE CARRY FROM LOWER 16 BITS.
         ADDA   FPACC2MN     ADD IN UPPER 8 BITS.
         STAA   FPACC1MN     SAVE IT.
         PULA                RESTORE A.
         BCS    ADDNXTD1     OVERFLOW? IF SO DON'T ADD IT IN.
         LSLD                BY 10.
         ROL    FPACC1MN
         STD    FPACC1MN+1   SAVE THE LOWER 16 BITS.
         BCS    ADDNXTD1     OVERFLOW? IF SO DON'T ADD IT IN.
         LDAB   0,X          GET CURRENT DIGIT.
         SUBB   #œ30         MAKE IT BINARY.
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
         LDAA   #œ80+24      SET UP INITIAL EXPONENT (# OF BITS + BIAS).
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
         CMPA   #'0          IS IT LESS THAN AN ASCII 0?
         BLO    NUMERIC1     YES. NOT NUMERIC.
         CMPA   #'9          IS IT GREATER THAN AN ASCII 9?
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
CONSTP1  FCB    œ7D,œ4C,œCC,œCD         0.1 DECIMAL
CONST10  FCB    œ84,œ20,œ00,œ00         10.0 DECIMAL
*
*
         TTL    FLTMUL
******************************************************************************
*                                                                            *
*                      FPMULT: FLOATING POINT MULTIPLY                       *
*                                                                            *
*       THIS FLOATING POINT MULTIPLY ROUTINE MULTIPLIES "FPACC1" BY          *
*       "FPACC2" AND PLACES THE RESULT IN TO FPACC1. FPACC2 REMAINS          *
*       UNCHANGED.                                                           *
*                          WORSE CASE = 2319 CYCLES = 1159 uS @ 2MHz         *
*                                                                            *
******************************************************************************
*
*
FLTMUL   EQU    *
         JSR    PSHFPAC2     SAVE FPACC2.
         LDX    #FPACC1EX    POINT TO FPACC1
         JSR    CHCK0        CHECK TO SEE IF FPACC1 IS ZERO.
         BEQ    FPMULT3      IT IS. ANSWER IS 0.
         LDX    #FPACC2EX    POINT TO FPACC2.
         JSR    CHCK0        IS IT 0?
         BNE    FPMULT4      NO. CONTINUE.
         CLRA                CLEAR D.
         CLRB
         STD    FPACC1EX     MAKE FPACC1 0.
         STD    FPACC1MN+1
         BRA    FPMULT3      RETURN.
FPMULT4  LDAA   MANTSGN1     GET FPACC1 EXPONENT.
         EORA   MANTSGN2     SET THE SIGN OF THE RESULT.
         STAA   MANTSGN1     SAVE THE SIGN OF THE RESULT.
         LDAA   FPACC1EX     GET FPACC1 EXPONENT.
         ADDA   FPACC2EX     ADD IT TO FPACC2 EXPONENT.
         BPL    FPMULT1      IF RESULT IS MINUS AND
         BCC    FPMULT2      THE CARRY IS SET THEN:
FPMULT5  LDAA   #OVFERR      OVERFLOW ERROR.
         SEC                 SET ERROR FLAG.
         BRA    FPMULT6      RETURN.
FPMULT1  BCS    FPMULT2      IF RESULT IS PLUS & THE CARRY IS SET THEN ALL OK.
         LDAA   #UNFERR      ELSE UNDERFLOW ERROR OCCURED.
         SEC                 FLAG ERROR.
         BRA    FPMULT6      RETURN.
FPMULT2  ADDA   #œ80         ADD 128 BIAS BACK IN THAT WE LOST.
         STAA   FPACC1EX     SAVE THE NEW EXPONENT.
         JSR    UMULT        GO MULTIPLY THE "INTEGER" MANTISSAS.
FPMULT3  TST    FPACC1EX     WAS THERE AN OVERFLOW ERROR FROM ROUNDING?
         BEQ    FPMULT5      YES. RETURN ERROR.
         CLC                 SHOW NO ERRORS.
FPMULT6  JSR    PULFPAC2     RESTORE FPACC2.
         RTS
*
*
UMULT    EQU    *
         LDX    #0
         PSHX                CREATE PARTIAL PRODUCT REGISTER AND COUNTER.
         PSHX
         TSX                 POINT TO THE VARIABLES.
         LDAA   #24          SET COUNT TO THE NUMBER OF BITS.
         STAA   0,X
UMULT1   LDAA   FPACC2MN+2   GET THE L.S. BYTE OF THE MULTIPLIER.
         LSRA                PUT L.S. BIT IN CARRY.
         BCC    UMULT2       IF CARRY CLEAR, DON'T ADD MULTIPLICAND TO P.P.
         LDD    FPACC1MN+1   GET MULTIPLICAND L.S. 16 BITS.
         ADDD   2,X          ADD TO PARTIAL PRODUCT.
         STD    2,X          SAVE IN P.P.
         LDAA   FPACC1MN     GET UPPER 8 BITS OF MULTIPLICAND.
         ADCA   1,X          ADD IT W/ CARRY TO P.P.
         STAA   1,X          SAVE TO PARTIAL PRODUCT.
UMULT2   ROR    1,X          ROTATE PARTIAL PRODUCT TO THE RIGHT.
         ROR    2,X
         ROR    3,X
         ROR    FPACC2MN     SHIFT THE MULTIPLIER TO THE RIGHT 1 BIT.
         ROR    FPACC2MN+1
         ROR    FPACC2MN+2
         DEC    0,X          DONE YET?
         BNE    UMULT1       NO. KEEP GOING.
         TST    1,X          DOES PARTIAL PRODUCT NEED TO BE NORMALIZED?
         BMI    UMULT3       NO. GET ANSWER & RETURN.
         LSL    FPACC2MN     GET BIT THAT WAS SHIFTED OUT OF P.P REGISTER.
         ROL    3,X          PUT IT BACK INTO THE PARTIAL PRODUCT.
         ROL    2,X
         ROL    1,X
         DEC    FPACC1EX     FIX EXPONENT.
UMULT3   TST    FPACC2MN     DO WE NEED TO ROUND THE PARTIAL PRODUCT?
         BPL    UMULT4       NO. JUST RETURN.
         LDD    2,X          YES. GET THE LEAST SIGNIFIGANT 16 BITS.
         ADDD   #1           ADD 1.
         STD    2,X          SAVE RESULT.
         LDAA   1,X          PROPIGATE THROUGH.
         ADCA   #0
         STAA   1,X
         BCC    UMULT4       IF CARRY CLEAR ALL IS OK.
         ROR    1,X          IF NOT OVERFLOW. ROTATE CARRY INTO P.P.
         ROR    2,X
         ROR    3,X
         INC    FPACC1EX     UP THE EXPONENT.
UMULT4   INS                 TAKE COUNTER OFF STACK.
         PULX                GET M.S. 16 BITS OF PARTIAL PRODUCT.
         STX    FPACC1MN     PUT IT IN FPACC1.
         PULA                GET L.S. 8 BITS OF PARTIAL PRODUCT.
         STAA   FPACC1MN+2   PUT IT IN FPACC1.
         RTS                 RETURN.
*
*
*
         TTL    FLTADD
******************************************************************************
*                                                                            *
*                       FLOATING POINT ADDITION                              *
*                                                                            *
*       This subroutine performs floating point addition of the two numbers  *
*       in FPACC1 and FPACC2.  The result of the addition is placed in       *
*       FPACC1 while FPACC2 remains unchanged.  This subroutine performs     *
*       full signed addition so either number may be of the same or opposite *
*       sign.                                                                *
*                          WORSE CASE = 1030 CYCLES = 515 uS @ 2MHz          *
*                                                                            *
******************************************************************************
*
*
FLTADD   EQU    *
         JSR    PSHFPAC2     SAVE FPACC2.
         LDX    #FPACC2EX    POINT TO FPACC2
         JSR    CHCK0        IS IT ZERO?
         BNE    FLTADD1      NO. GO CHECK FOR 0 IN FPACC1.
FLTADD6  CLC                 NO ERRORS.
FLTADD10 JSR    PULFPAC2     RESTORE FPACC2.
         RTS                 ANSWER IN FPACC1. RETURN.
FLTADD1  LDX    #FPACC1EX    POINT TO FPACC1.
         JSR    CHCK0        IS IT ZERO?
         BNE    FLTADD2      NO. GO ADD THE NUMBER.
FLTADD4  LDD    FPACC2EX     ANSWER IS IN FPACC2. MOVE IT INTO FPACC1.
         STD    FPACC1EX
         LDD    FPACC2MN+1   MOVE LOWER 16 BITS OF MANTISSA.
         STD    FPACC1MN+1
         LDAA   MANTSGN2     MOVE FPACC2 MANTISSA SIGN INTO FPACC1.
         STAA   MANTSGN1
         BRA    FLTADD6      RETURN.
FLTADD2  LDAA   FPACC1EX     GET FPACC1 EXPONENT.
         CMPA   FPACC2EX     ARE THE EXPONENTS THE SAME?
         BEQ    FLTADD7      YES. GO ADD THE MANTISSA'S.
         SUBA   FPACC2EX     NO. FPACC1EX-FPACC2EX. IS FPACC1 > FPACC2?
         BPL    FLTADD3      YES. GO CHECK RANGE.
         NEGA                NO. FPACC1 < FPACC2. MAKE DIFFERENCE POSITIVE.
         CMPA   #23          ARE THE NUMBERS WITHIN RANGE?
         BHI    FLTADD4      NO. FPACC2 IS LARGER. GO MOVE IT INTO FPACC1.
         TAB                 PUT DIFFERENCE IN B.
         ADDB   FPACC1EX     CORRECT FPACC1 EXPONENT.
         STAB   FPACC1EX     SAVE THE RESULT.
         LDX    #FPACC1MN    POINT TO FPACC1 MANTISSA.
         BRA    FLTADD5      GO DENORMALIZE FPACC1 FOR THE ADD.
FLTADD3  CMPA   #23          FPACC1 > FPACC2. ARE THE NUMBERS WITHIN RANGE?
         BHI    FLTADD6      NO. ANSWER ALREADY IN FPACC1. JUST RETURN.
         LDX    #FPACC2MN    POINT TO THE MANTISSA TO DENORMALIZE.
FLTADD5  LSR    0,X          SHIFT THE FIRST BYTE OF THE MANTISSA.
         ROR    1,X          THE SECOND.
         ROR    2,X          AND THE THIRD.
         DECA                DONE YET?
         BNE    FLTADD5      NO. KEEP SHIFTING.
FLTADD7  LDAA   MANTSGN1     GET FPACC1 MANTISSA SIGN.
         CMPA   MANTSGN2     ARE THE SIGNS THE SAME?
         BEQ    FLTADD11     YES. JUST GO ADD THE TWO MANTISSAS.
         TST    MANTSGN1     NO. IS FPACC1 THE NEGATIVE NUMBER?
         BPL    FLTADD8      NO. GO DO FPACC1-FPACC2.
         LDX    FPACC2MN     YES. EXCHANGE FPACC1 & FPACC2 BEFORE THE SUB.
         PSHX                SAVE IT.
         LDX    FPACC1MN     GET PART OF FPACC1.
         STX    FPACC2MN     PUT IT IN FPACC2.
         PULX                GET SAVED PORTION OF FPACC2
         STX    FPACC1MN     PUT IT IN FPACC1.
         LDX    FPACC2MN+2   GET LOWER 8 BITS & SIGN OF FPACC2.
         PSHX                SAVE IT.
         LDX    FPACC1MN+2   GET LOWER 8 BITS & SIGN OF FPACC1.
         STX    FPACC2MN+2   PUT IT IN FPACC2.
         PULX                GET SAVED PART OF FPACC2.
         STX    FPACC1MN+2   PUT IT IN FPACC1.
FLTADD8  LDD    FPACC1MN+1   GET LOWER 16 BITS OF FPACC1.
         SUBD   FPACC2MN+1   SUBTRACT LOWER 16 BITS OF FPACC2.
         STD    FPACC1MN+1   SAVE RESULT.
         LDAA   FPACC1MN     GET HIGH 8 BITS OF FPACC1 MANTISSA.
         SBCA   FPACC2MN     SUBTRACT HIGH 8 BITS OF FPACC2.
         STAA   FPACC1MN     SAVE THE RESULT. IS THE RESULT NEGATIVE?
         BCC    FLTADD9      NO. GO NORMALIZE THE RESULT.
         LDAA   FPACC1MN     YES. NEGATE THE MANTISSA.
         COMA
         PSHA                SAVE THE RESULT.
         LDD    FPACC1MN+1   GET LOWER 16 BITS.
         COMB                FORM THE ONE'S COMPLEMENT.
         COMA
         ADDD   #1           FORM THE TWO'S COMPLEMENT.
         STD    FPACC1MN+1   SAVE THE RESULT.
         PULA                GET UPPER 8 BITS BACK.
         ADCA   #0           ADD IN POSSIBLE CARRY.
         STAA   FPACC1MN     SAVE RESULT.
         LDAA   #œFF         SHOW THAT FPACC1 IS NEGATIVE.
         STAA   MANTSGN1
FLTADD9  JSR    FPNORM       GO NORMALIZE THE RESULT.
         BCC    FLTADD12     EVERYTHING'S OK SO RETURN.
         LDAA   #UNFERR      UNDERFLOW OCCURED DURING NORMALIZATION.
         SEC                 FLAG ERROR.
         JMP    FLTADD10     RETURN.
FLTADD12 JMP    FLTADD6      CAN'T BRANCH THAT FAR FROM HERE.
*
FLTADD11 LDD    FPACC1MN+1   GET LOWER 16 BITS OF FPACC1.
         ADDD   FPACC2MN+1   ADD IT TO THE LOWER 16 BITS OF FPACC2.
         STD    FPACC1MN+1   SAVE RESULT IN FPACC1.
         LDAA   FPACC1MN     GET UPPER 8 BITS OF FPACC1.
         ADCA   FPACC2MN     ADD IT (WITH CARRY) TO UPPER 8 BITS OF FPACC2.
         STAA   FPACC1MN     SAVE THE RESULT.
         BCC    FLTADD12     NO OVERFLOW SO JUST RETURN.
         ROR    FPACC1MN     PUT THE CARRY INTO THE MANTISSA.
         ROR    FPACC1MN+1   PROPIGATE THROUGH MANTISSA.
         ROR    FPACC1MN+2
         INC    FPACC1EX     UP THE MANTISSA BY 1.
         BNE    FLTADD12     EVERYTHING'S OK JUST RETURN.
         LDAA   #OVFERR      RESULT WAS TOO LARGE. OVERFLOW.
         SEC                 FLAG ERROR.
         JMP    FLTADD10     RETURN.
*
*
*
         TTL    FLTSUB
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
*                          WORSE CASE = 1062 CYCLES = 531 uS @ 2MHz          *
*                                                                            *
******************************************************************************
*
*
FLTSUB   EQU    *
         BSR    FLTSUB1      INVERT SIGN.
         JSR    FLTADD       GO DO FLOATING POINT ADD.
FLTSUB1  LDAA   MANTSGN2     GET FPACC2 MANTISSA SIGN.
         EORA   #œFF         INVERT THE SIGN.
         STAA   MANTSGN2     PUT BACK.
         RTS                 RETURN.
*
*
*
         TTL    FLTDIV
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
*                          WORSE CASE = 2911 CYCLES = 1455 uS @ 2MHz         *
*                                                                            *
******************************************************************************
*
*
FLTDIV   EQU    *
         LDX    #FPACC2EX    POINT TO FPACC2.
         JSR    CHCK0        IS THE DIVISOR 0?
         BNE    FLTDIV1      NO. GO SEE IF THE DIVIDEND IS ZERO.
         LDAA   #DIV0ERR     YES. RETURN A DIVIDE BY ZERO ERROR.
         SEC                 FLAG ERROR.
         RTS                 RETURN.
FLTDIV1  LDX    #FPACC1EX    POINT TO FPACC1.
         JSR    CHCK0        IS THE DIVIDEND 0?
         BNE    FLTDIV2      NO. GO PERFORM THE DIVIDE.
         CLC                 YES. ANSWER IS ZERO. NO ERRORS.
         RTS                 RETURN.
FLTDIV2  JSR    PSHFPAC2     SAVE FPACC2.
         LDAA   MANTSGN2     GET FPACC2 MANTISSA SIGN.
         EORA   MANTSGN1     SET THE SIGN OF THE RESULT.
         STAA   MANTSGN1     SAVE THE RESULT.
         LDX    #0           SET UP WORK SPACE ON THE STACK.
         PSHX
         PSHX
         PSHX
         LDAA   #24          PUT LOOP COUNT ON STACK.
         PSHA
         TSX                 SET UP POINTER TO WORK SPACE.
         LDD    FPACC1MN     COMPARE FPACC1 & FPACC2 MANTISSAS.
         CPD    FPACC2MN     ARE THE UPPER 16 BITS THE SAME?
         BNE    FLTDIV3      NO.
         LDAA   FPACC1MN+2   YES. COMPARE THE LOWER 8 BITS.
         CMPA   FPACC2MN+2
FLTDIV3  BHS    FLTDIV4      IS FPACC2 MANTISSA > FPACC1 MANTISSA? NO.
         INC    FPACC2EX     ADD 1 TO THE EXPONENT TO KEEP NUMBER THE SAME.
*                            DID OVERFLOW OCCUR?
         BNE    FLTDIV14     NO. GO SHIFT THE MANTISSA RIGHT 1 BIT.
FLTDIV8  LDAA   #OVFERR      YES. GET ERROR CODE.
         SEC                 FLAG ERROR.
FLTDIV6  PULX                REMOVE WORKSPACE FROM STACK.
         PULX
         PULX
         INS
         JSR    PULFPAC2     RESTORE FPACC2.
         RTS                 RETURN.
FLTDIV4  LDD    FPACC1MN+1   DO AN INITIAL SUBTRACT IF DIVIDEND MANTISSA IS
         SUBD   FPACC2MN+1   GREATER THAN DIVISOR MANTISSA.
         STD    FPACC1MN+1
         LDAA   FPACC1MN
         SBCA   FPACC2MN
         STAA   FPACC1MN
         DEC    0,X           SUBTRACT 1 FROM THE LOOP COUNT.
FLTDIV14 LSR    FPACC2MN      SHIFT THE DIVISOR TO THE RIGHT 1 BIT.
         ROR    FPACC2MN+1
         ROR    FPACC2MN+2
         LDAA   FPACC1EX     GET FPACC1 EXPONENT.
         LDAB   FPACC2EX     GET FPACC2 EXPONENT.
         NEGB                ADD THE TWO'S COMPLEMENT TO SET FLAGS PROPERLY.
         ABA
         BMI    FLTDIV5      IF RESULT MINUS CHECK CARRY FOR POSS. OVERFLOW.
         BCS    FLTDIV7      IF PLUS & CARRY SET ALL IS OK.
         LDAA   #UNFERR      IF NOT, UNDERFLOW ERROR.
         BRA    FLTDIV6      RETURN WITH ERROR.
FLTDIV5  BCS    FLTDIV8      IF MINUS & CARRY SET OVERFLOW ERROR.
FLTDIV7  ADDA   #œ81         ADD BACK BIAS+1 (THE '1' COMPENSATES FOR ALGOR.)
         STAA   FPACC1EX     SAVE RESULT.
FLTDIV9  LDD    FPACC1MN     SAVE DIVIDEND IN CASE SUBTRACTION DOESN'T GO.
         STD    4,X
         LDAA   FPACC1MN+2
         STAA   6,X
         LDD    FPACC1MN+1   GET LOWER 16 BITS FOR SUBTRACTION.
         SUBD   FPACC2MN+1
         STD    FPACC1MN+1   SAVE RESULT.
         LDAA   FPACC1MN     GET HIGH 8 BITS.
         SBCA   FPACC2MN
         STAA   FPACC1MN
         BPL    FLTDIV10     SUBTRACTION WENT OK. GO DO SHIFTS.
         LDD    4,X          RESTORE OLD DIVIDEND.
         STD    FPACC1MN
         LDAA   6,X
         STAA   FPACC1MN+2
FLTDIV10 ROL    3,X          ROTATE CARRY INTO QUOTIENT.
         ROL    2,X
         ROL    1,X
         LSL    FPACC1MN+2   SHIFT DIVIDEND TO LEFT FOR NEXT SUBTRACT.
         ROL    FPACC1MN+1
         ROL    FPACC1MN
         DEC    0,X          DONE YET?
         BNE    FLTDIV9      NO. KEEP GOING.
         COM    1,X          RESULT MUST BE COMPLEMENTED.
         COM    2,X
         COM    3,X
         LDD    FPACC1MN+1   DO 1 MORE SUBTRACT FOR ROUNDING.
         SUBD   FPACC2MN+1   ( DON'T NEED TO SAVE THE RESULT. )
         LDAA   FPACC1MN
         SBCA   FPACC2MN     ( NO NEED TO SAVE THE RESULT. )
         LDD    2,X          GET LOW 16 BITS.
         BCC    FLTDIV11     IF IT DIDNT GO RESULT OK AS IS.
         CLC                 CLEAR THE CARRY.
         BRA    FLTDIV13     GO SAVE THE NUMBER.
FLTDIV11 ADDD   #1           ROUND UP BY 1.
FLTDIV13 STD    FPACC1MN+1   PUT IT IN FPACC1.
         LDAA   1,X          GET HIGH 8 BITS.
         ADCA   #0
         STAA   FPACC1MN     SAVE RESULT.
         BCC    FLTDIV12     IF CARRY CLEAR ANSWER OK.
         ROR    FPACC1MN     IF NOT OVERFLOW. ROTATE CARRY IN.
         ROR    FPACC1MN+1
         ROR    FPACC1MN+2
FLTDIV12 CLC                 NO ERRORS.
         JMP    FLTDIV6      RETURN.
*
*
*
         TTL    FLTASC
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
         LDD    #œ3000       GET ASCII CHARACTER + TERMINATING BYTE.
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
         LDAA   #œ20         PUT A SPACE IN THE BUFFER IF NUMBER NOT NEGATIVE.
         TST    MANTSGN1     IS IT NEGATIVE?
         BEQ    FLTASC2      NO. GO PUT SPACE.
         CLR    MANTSGN1     MAKE NUMBER POSITIVE FOR REST OF CONVERSION.
         LDAA   #'-          YES. PUT MINUS SIGN IN BUFFER.
FLTASC2  STAA   0,X
         INX                 POINT TO NEXT LOCATION.
         STX    0,Y          SAVE POINTER.
FLTASC5  LDX    #N9999999    POINT TO CONSTANT 9999999.
         JSR    GETFPAC2     GET INTO FPACC2.
         JSR    FLTCMP       COMPARE THE NUMBERS. IS FPACC1 > 9999999?
         BHI    FLTASC3      YES. GO DIVIDE FPACC1 BY 10.
         LDX    #P9999999    POINT TO CONSTANT 999999.9
         JSR    GETFPAC2     MOVE IT INTO FPACC2.
         JSR    FLTCMP       COMPARE NUMBERS. IS FPACC1 > 999999.9?
         BHI    FLTASC4      YES. GO CONTINUE THE CONVERSION.
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
         SUBB   #œ81         TAKE OUT BIAS +1.
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
         LDAA   #'.          NO. GET DECIMAL POINT.
         LDX    0,Y          GET POINTER TO BUFFER.
         STAA   0,X          PUT THE DECIMAL POINT IN THE BUFFER.
         INX                 POINT TO NEXT BUFFER LOCATION.
         TST    3,Y          IS THE DIGIT COUNT TILL EXPONENT =0?
         BEQ    FLTASC18     NO. NUMBER IS <.1
         LDAA   #'0          YES. FORMAT NUMBER AS .0XXXXXXX
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
         ADDA   #œ30         MAKE IT ASCII.
         PSHX                SAVE POINTER TO CONSTANTS.
         LDX    0,Y          GET POINTER TO BUFFER.
         STAA   0,X          PUT DIGIT IN BUFFER.
         INX                 POINT TO NEXT BUFFER LOCATION.
         DEC    3,Y          SHOULD WE PUT A DECIMAL POINT IN THE BUFFER YET?
         BNE    FLTASC16     NO. CONTINUE THE CONVERSION.
         LDAA   #'.          YES. GET DECIMAL POINT.
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
         CMPA   #œ30         WAS IT AN ASCII 0?
         BEQ    FLTASC13     YES. REMOVE TRAILING ZEROS.
         INX                 POINT TO NEXT AVAILABLE LOCATION IN BUFFER.
         LDAB   2,Y          DO WE NEED TO PUT OUT AN EXPONENT?
         BEQ    FLTASC15     NO. WE'RE DONE.
         LDAA   #'E          YES. PUT AN 'E' IN THE BUFFER.
         STAA   0,X
         INX                 POINT TO NEXT BUFFER LOCATION.
         LDAA   #'+          ASSUME EXPONENT IS POSITIVE.
         STAA   0,X          PUT PLUS SIGN IN THE BUFFER.
         TSTB                IS IT REALLY MINUS?
         BPL    FLTASC14     NO. IS'S OK AS IS.
         NEGB                YES. MAKE IT POSITIVE.
         LDAA   #'-          PUT THE MINUS SIGN IN THE BUFFER.
         STAA   0,X
FLTASC14 INX                 POINT TO NEXT BUFFER LOCATION.
         STX    0,Y          SAVE POINTER TO STRING BUFFER.
         CLRA                SET UP FOR DIVIDE.
         LDX    #10          DIVIDE DECIMAL EXPONENT BY 10.
         IDIV
         PSHB                SAVE REMAINDER.
         XGDX                PUT QUOTIENT IN D.
         ADDB   #œ30         MAKE IT ASCII.
         LDX    0,Y          GET POINTER.
         STAB   0,X          PUT NUMBER IN BUFFER.
         INX                 POINT TO NEXT LOCATION.
         PULB                GET SECOND DIGIT.
         ADDB   #œ30         MAKE IT ASCII.
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
         FCB    œ0F,œ42,œ40  DECIMAL 1,000,000
         FCB    œ01,œ86,œA0  DECIMAL   100,000
         FCB    œ00,œ27,œ10  DECIMAL    10,000
         FCB    œ00,œ03,œE8  DECIMAL     1,000
         FCB    œ00,œ00,œ64  DECIMAL       100
         FCB    œ00,œ00,œ0A  DECIMAL        10
         FCB    œ00,œ00,œ01  DECIMAL         1
*
*
P9999999 EQU    *            CONSTANT 999999.9
         FCB    œ94,œ74,œ23,œFE
*
N9999999 EQU    *            CONSTANT 9999999.
         FCB    œ98,œ18,œ96,œ7F
*
CONSTP5 EQU    *            CONSTANT .5
         FCB    œ80,œ00,œ00,œ00
*
*
FLTCMP   EQU    *
         TST    MANTSGN1     IS FPACC1 NEGATIVE?
         BPL    FLTCMP2      NO. CONTINUE WITH COMPARE.
         TST    MANTSGN2     IS FPACC2 NEGATIVE?
         BPL    FLTCMP2      NO. CONTINUE WITH COMPARE.
         LDD    FPACC2EX     YES. BOTH ARE NEGATIVE SO COMPARE MUST BE DONE
         CPD    FPACC1EX     BACKWARDS. ARE THEY EQUAL SO FAR?
         BNE    FLTCMP1      NO. RETURN WITH CONDITION CODES SET.
         LDD    FPACC2MN+1   YES. COMPARE LOWER 16 BITS OF MANTISSAS.
         CPD    FPACC1MN+1
FLTCMP1  RTS                 RETURN WITH CONDITION CODES SET.
FLTCMP2  LDAA   MANTSGN1     GET FPACC1 MANTISSA SIGN.
         CMPA   MANTSGN2     BOTH POSITIVE?
         BNE    FLTCMP1      NO. RETURN WITH CONDITION CODES SET.
         LDD    FPACC1EX     GET FPACC1 EXPONENT & UPPER 8 BITS OF MANTISSA.
         CPD    FPACC2EX     SAME AS FPACC2?
         BNE    FLTCMP1      NO. RETURN WITH CONDITION CODES SET.
         LDD    FPACC1MN+1   GET FPACC1 LOWER 16 BITS OF MANTISSA.
         CPD    FPACC2MN+1   COMPARE WITH FPACC2 LOWER 16 BITS OF MANTISSA.
         RTS                 RETURN WITH CONDITION CODES SET.
*
*
*
         TTL    INT2FLT
******************************************************************************
*                                                                            *
*                     UNSIGNED INTEGER TO FLOATING POINT                     *
*                                                                            *
*        This subroutine performs "unsigned" integer to floating point       *
*        conversion of a 16 bit word.  The 16 bit integer must be in the     *
*        lower 16 bits of FPACC1 mantissa.  The resulting floating point     *
*        number is returned in FPACC1.                                       *
*                                                                            *
******************************************************************************
*
*
UINT2FLT EQU    *
         LDX    #FPACC1EX    POINT TO FPACC1.
         JSR    CHCK0        IS IT ALREADY 0?
         BNE    UINTFLT1     NO. GO CONVERT.
         RTS                 YES. JUST RETURN.
UINTFLT1 LDAA   #œ98         GET BIAS + NUMBER OF BITS IN MANTISSA.
         STAA   FPACC1EX     INITIALIZE THE EXPONENT.
         JSR    FPNORM       GO MAKE IT A NORMALIZED FLOATING POINT VALUE.
         CLC                 NO ERRORS.
         RTS                 RETURN.
*
*
*
******************************************************************************
*                                                                            *
*                      SIGNED INTEGER TO FLOATING POINT                      *
*                                                                            *
*        This routine works just like the unsigned integer to floating       *
*        point routine except the the 16 bit integer in the FPACC1           *
*        mantissa is considered to be in two's complement format.  This      *
*        will return a floating point number in the range -32768 to +32767.  *
*                                                                            *
******************************************************************************
*
*
SINT2FLT EQU    *
         LDD    FPACC1MN+1   GET THE LOWER 16 BITS OF FPACC1 MANTISSA.
         PSHA                SAVE SIGN OF NUMBER.
         BPL    SINTFLT1     IF POSITIVE JUST GO CONVERT.
         COMA                MAKE POSITIVE.
         COMB
         ADDD   #1           TWO'S COMPLEMENT.
         STD    FPACC1MN+1   PUT IT BACK IN FPACC1 MANTISSA.
SINTFLT1 BSR    UINT2FLT     GO CONVERT.
         PULA                GET SIGN OF ORIGINAL INTEGER.
         LDAB   #œFF         GET "MINUS SIGN".
         TSTA                WAS THE NUMBER NEGATIVE?
         BPL    SINTFLT2     NO. RETURN.
         STAB   MANTSGN1     YES. SET FPACC1 SIGN BYTE.
SINTFLT2 CLC                 NO ERRORS.
         RTS                 RETURN.
*
*
*
         TTL    FLT2INT
******************************************************************************
*                                                                            *
*                   FLOATING POINT TO INTEGER CONVERSION                     *
*                                                                            *
*        This subroutine will perform "unsigned" floating point to integer   *
*        conversion.  The floating point number if positive, will be         *
*        converted to an unsigned 16 bit integer ( 0 <= X <= 65535 ).  If    *
*        the number is negative it will be converted to a twos complement    *
*        16 bit integer.  This type of conversion will allow 16 bit          *
*        addresses to be represented as positive numbers when in floating    *
*        point format.  Any fractional number part is disguarded             *
*                                                                            *
******************************************************************************
*
*
FLT2INT  EQU    *
         LDX    #FPACC1EX    POINT TO FPACC1.
         JSR    CHCK0        IS IT 0?
         BEQ    FLT2INT3     YES. JUST RETURN.
         LDAB   FPACC1EX     GET FPACC1 EXPONENT.
         CMPB   #œ81         IS THERE AN INTEGER PART?
         BLO    FLT2INT2     NO. GO PUT A 0 IN FPACC1.
         TST    MANTSGN1     IS THE NUMBER NEGATIVE?
         BMI    FLT2INT1     YES. GO CONVERT NEGATIVE NUMBER.
         CMPB   #œ90         IS THE NUMBER TOO LARGE TO BE MADE AN INTEGER?
         BHI    FLT2INT4     YES. RETURN WITH AN ERROR.
         SUBB   #œ98         SUBTRACT THE BIAS PLUS THE NUMBER OF BITS.
FLT2INT5 LSR    FPACC1MN     MAKE THE NUMBER AN INTEGER.
         ROR    FPACC1MN+1
         ROR    FPACC1MN+2
         INCB                DONE SHIFTING?
         BNE    FLT2INT5     NO. KEEP GOING.
         CLR    FPACC1EX     ZERO THE EXPONENT (ALSO CLEARS THE CARRY).
         RTS
FLT2INT1 CMPB   #œ8F         IS THE NUMBER TOO SMALL TO BE MADE AN INTEGER?
         BHI    FLT2INT4     YES. RETURN ERROR.
         SUBB   #œ98         SUBTRACT BIAS PLUS NUMBER OF BITS.
         BSR    FLT2INT5     GO DO SHIFT.
         LDD    FPACC1MN+1   GET RESULTING INTEGER.
         COMA                MAKE IT NEGATIVE.
         COMB
         ADDD   #1           TWO'S COMPLEMENT.
         STD    FPACC1MN+1   SAVE RESULT.
         CLR    MANTSGN1     CLEAR MANTISSA SIGN. (ALSO CLEARS THE CARRY)
         RTS                 RETURN.
FLT2INT4 LDAA   #TOLGSMER    NUMBER TOO LARGE OR TOO SMALL TO CONVERT TO INT.
         SEC                 FLAG ERROR.
         RTS                 RETURN.
FLT2INT2 LDD    #0
         STD    FPACC1EX     ZERO FPACC1.
         STD    FPACC1MN+1   (ALSO CLEARS THE CARRY)
FLT2INT3 RTS                 RETURN.
*
*
*
         TTL    FLTSQR
******************************************************************************
*                                                                            *
*                         SQUARE ROOT SUBROUTINE                             *
*                                                                            *
*        This routine is used to calculate the square root of the floating   *
*        point number in FPACC1.  If the number in FPACC1 is negative an     *
*        error is returned.                                                  *
*                                                                            *
*                          WORSE CASE = 16354 CYCLES = 8177 uS @ 2MHz        *
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
         SUBA   #œ80         REMOVE BIAS FROM EXPONENT.
         INCA                COMPENSATE FOR ODD EXPONENTS (GIVES CLOSER GUESS)
         BPL    FLTSQR3      IF NUMBER >1 DIVIDE EXPONENT BY 2 & ADD BIAS.
         LSRA                IF <1 JUST DIVIDE IT BY 2.
         BRA    FLTSQR4      GO CALCULATE THE SQUARE ROOT.
FLTSQR3  LSRA                DIVIDE EXPONENT BY 2.
         ADDA   #œ80         ADD BIAS BACK IN.
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
         CLC                 NO ERRORS.
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
         TTL    FLTSIN
******************************************************************************
*                                                                            *
*                        FLOATING POINT SINE                                 *
*                                                                            *
******************************************************************************
*
*
FLTSIN   EQU    *
         JSR    PSHFPAC2     SAVE FPACC2 ON THE STACK.
         JSR    ANGRED       GO REDUCE THE ANGLE TO BETWEEN +/-PI.
         PSHB                SAVE THE QUAD COUNT.
         PSHA                SAVE THE SINE/COSINE FLAG.
         JSR    DEG2RAD      CONVERT DEGREES TO RADIANS.
         PULA                RESTORE THE SINE/COSINE FLAG.
FLTSIN1  JSR    SINCOS       GO GET THE SINE OF THE ANGLE.
         PULA                RESTORE THE QUAD COUNT.
         CMPA   #2           WAS THE ANGLE IN QUADS 1 OR 2?
         BLS    FLTSIN2      YES. SIGN OF THE ANSWER IS OK.
         COM    MANTSGN1     NO. SINE IN QUADS 3 & 4 IS NEGATIVE.
FLTSIN2  CLC                 SHOW NO ERRORS.
         JSR    PULFPAC2     RESTORE FPACC2
         RTS                 RETURN.
*
*
*
        TTL    FLTCOS
******************************************************************************
*                                                                            *
*                        FLOATING POINT COSINE                               *
*                                                                            *
******************************************************************************
*
*
FLTCOS   EQU    *
         JSR    PSHFPAC2     SAVE FPACC2 ON THE STACK.
         JSR    ANGRED       GO REDUCE THE ANGLE TO BETWEEN +/-PI.
         PSHB                SAVE THE QUAD COUNT.
         PSHA                SAVE THE SINE/COSINE FLAG.
         JSR    DEG2RAD      CONVERT TO RADIANS.
         PULA                RESTORE THE SINE/COSINE FLAG.
         EORA   #œ01         COMPLIMENT 90'S COPMLIMENT FLAG FOR COSINE.
         JSR    SINCOS       GO GET THE COSINE OF THE ANGLE.
         PULA                RESTORE THE QUAD COUNT.
         CMPA   #1           WAS THE ORIGINAL ANGLE IN QUAD 1?
         BEQ    FLTCOS1      YES. SIGN IS OK.
         CMPA   #4           WAS IT IN QUAD 4?
         BEQ    FLTCOS1      YES. SIGN IS OK.
         COM    MANTSGN1     NO. COSINE IS NEGATIVE IN QUADS 2 & 3.
FLTCOS1  JMP    FLTSIN2      FLAG NO ERRORS, RESTORE FPACC2, & RETURN.
*
*
*
         TTL    SINCOS
******************************************************************************
*                                                                            *
*                  FLOATING POINT SINE AND COSINE SUBROUTINE                 *
*                                                                            *
******************************************************************************
*
*
SINCOS   EQU    *
         PSHA                SAVE SINE/COSINE FLAG ON STACK.
         LDX    FPACC1MN+1   SAVE THE VALUE OF THE ANGLE.
         PSHX
         LDX    FPACC1EX
         PSHX
         LDAA   MANTSGN1
         PSHA
         LDX    #SINFACT     POINT TO THE FACTORIAL TABLE.
         PSHX                SAVE POINTER TO THE SINE FACTORIAL TABLE.
         PSHX                JUST ALLOCATE ANOTHER LOCAL (VALUE NOT IMPORTANT)
         LDAA   #œ4          GET INITIAL LOOP COUNT.
         PSHA                SAVE AS LOCAL ON STACK
         TSY                 POINT TO LOCALS.
         JSR    TFR1TO2      TRANSFER FPACC1 TO FPACC2.
         JSR    FLTMUL       GET Xª2 IN FPACC1.
         TST    10,Y         ARE WE DOING THE SINE?
         BEQ    SINCOS7      YES. GO DO IT.
         LDX    #COSFACT     NO. GET POINTER TO COSINE FACTORIAL TABLE.
         STX    1,Y          SAVE IT.
         JSR    TFR1TO2      COPY Xª2 INTO FPACC2.
         BRA    SINCOS4      GENERATE EVEN POWERS OF "X" FOR COSINE.
SINCOS7  JSR    EXG1AND2     PUT Xª2 IN FPACC2 & X IN FPACC1.
SINCOS1  JSR    FLTMUL       CREATE Xª3,5,7,9 OR Xª2,4,6,8.
SINCOS4  LDX    FPACC1MN+1   SAVE EACH ONE ON THE STACK.
         PSHX
         LDX    FPACC1EX
         PSHX
         LDAA   MANTSGN1
         PSHA                SAVE THE MANTISSA SIGN.
         DEC    0,Y          HAVE WE GENERATED ALL THE POWERS YET?
         BNE    SINCOS1      NO. GO DO SOME MORE.
         LDAA   #œ4          SET UP LOOP COUNT.
         STAA   0,Y
         TSX                 POINT TO POWERS ON THE STACK.
SINCOS2  STX    3,Y          SAVE THE POINTER.
         LDX    1,Y          GET THE POINTER TO THE FACTORIAL CONSTANTS.
         JSR    GETFPAC2     PUT THE NUMBER IN FPACC2.
         INX                 POINT TO THE NEXT CONSTANT.
         INX
         INX
         INX
         STX    1,Y          SAVE THE POINTER.
         LDX    3,Y          GET POINTER TO POWERS.
         LDAA   0,X          GET NUMBER SIGN.
         STAA   MANTSGN1     PUT IN FPACC1 MANTISSA SIGN.
         LDD    1,X          GET LOWER 16-BITS OF THE MANTISSA.
         STD    FPACC1EX     PUT IN FPACC1 MANTISSA.
         LDD    3,X          GET HIGH 8 BITS OF THE MANTISSA & EXPONENT.
         STD    FPACC1MN+1   PUT IT IN FPACC1 EXPONENT & MANTISSA.
         JSR    FLTMUL       MULTIPLY THE TWO.
         LDX    3,Y          GET POINTER TO POWERS BACK.
         LDD    FPACC1MN+1   SAVE RESULT WHERE THE POWER OF X WAS.
         STD    3,X
         LDD    FPACC1EX
         STD    1,X
         LDAA   MANTSGN1     SAVE SIGN.
         STAA   0,X
         INX                 POINT TO THE NEXT POWER.
         INX
         INX
         INX
         INX
         DEC    0,Y          DONE?
         BNE    SINCOS2      NO. GO DO ANOTHER MULTIPLICATION.
         LDAA   #œ3          GET LOOP COUNT.
         STAA   0,Y          SAVE IT.
SINCOS3  LDX    3,Y          POINT TO RESULTS ON THE STACK.
         DEX                 POINT TO PREVIOUS RESULT.
         DEX
         DEX
         DEX
         DEX
         STX    3,Y          SAVE THE NEW POINTER.
         LDAA   0,X          GET NUMBERS SIGN.
         STAA   MANTSGN2     PUT IT IN FPACC2.
         LDD    1,X          GET LOW 16 BITS OF THE MANTISSA.
         STD    FPACC2EX     PUT IN FPACC2.
         LDD    3,X          GET HIGH 8 BIT & EXPONENT.
         STD    FPACC2MN+1   PUT IN FPACC2.
         JSR    FLTADD       GO ADD THE TWO NUMBERS.
         DEC    0,Y          DONE?
         BNE    SINCOS3      NO. GO ADD THE NEXT TERM IN.
         TST    10,Y         ARE WE DOING THE SINE?
         BEQ    SINCOS5      YES. GO PUT THE ORIGINAL ANGLE INTO FPACC2.
         LDX    #ONE         NO. FOR COSINE PUT THE CONSTANT 1 INTO FPACC2.
         JSR    GETFPAC2
         BRA    SINCOS6      GO ADD IT TO THE SUM OF THE TERMS.
SINCOS5  LDAA   5,Y          GET THE VALUE OF THE ORIGINAL ANGLE.
         STAA   MANTSGN2     PUT IT IN FPACC2.
         LDD    6,Y
         STD    FPACC2EX
         LDD    8,Y
         STD    FPACC2MN+1
SINCOS6  JSR    FLTADD       GO ADD IT TO THE SUM OF THE TERMS.
         TSX                 NOW CLEAN UP THE STACK.
         XGDX                PUT STACK IN D.
         ADDD   #31          CLEAR ALL THE TERMS & TEMPS OFF THE STACK.
         XGDX
         TXS                 UPDATE THE STACK POINTER.
         RTS                 RETURN.
*
*
ANGRED   EQU    *
         CLRA                INITIALIZE THE 45'S COMPLIMENT FLAG.
         PSHA                PUT IT ON THE STACK.
         INCA                INITIALIZE THE QUAD COUNT TO 1.
         PSHA                PUT IT ON THE STACK.
         TSY                 POINT TO IT.
         LDX    #THREE60     POINT TO THE CONSTANT 360.
         JSR    GETFPAC2     GET IT INTO FPACC.
         TST    MANTSGN1     IS THE INPUT ANGLE NEGATIVE:
         BPL    ANGRED1      NO. SKIP THE ADD.
         JSR    FLTADD       YES. MAKE THE ANGLE POSITIVE BY ADDING 360 DEG.
ANGRED1  DEC    FPACC2EX     MAKE THE CONSTANT IN FPACC2 90 DEGREES.
         DEC    FPACC2EX
ANGRED2  JSR    FLTCMP       IS THE ANGLE LESS THAN 90 DEGREES ALREADY?
         BLS    ANGRED3      YES. RETURN WITH QUAD COUNT.
         JSR    FLTSUB       NO. REDUCE ANGLE BY 90 DEGREES.
         INC    0,Y          INCREMENT THE QUAD COUNT.
         BRA    ANGRED2      GO SEE IF IT'S LESS THAN 90 NOW.
ANGRED3  LDAA   0,Y          GET THE QUAD COUNT.
         CMPA   #1           WAS THE ORIGINAL ANGLE IN QUAD 1?
         BEQ    ANGRED4      YES. COMPUTE TRIG FUNCTION AS IS.
         CMPA   #3           NO. WAS THE ORIGINAL ANGLE IN QUAD 3?
         BEQ    ANGRED4      YES. COMPUTE THE TRIG FUNCTION AS IF IN QUAD 1.
         LDAA   #œFF         NO. MUST COMPUTE THE TRIG FUNCTION OF THE 90'S
         STAA   MANTSGN1     COMPLIMENT ANGLE.
         JSR    FLTADD       ADD 90 DEGREES TO THE NEGATED ANGLE.
ANGRED4  DEC    FPACC2EX     MAKE THE ANGLE IN FPACC2 45 DEGREES.
         JSR    FLTCMP       IS THE ANGLE < 45 DEGREES?
         BLS    ANGRED5      YES. IT'S OK AS IT IS.
         INC    FPACC2EX     NO. MUST GET THE 90'S COMPLIMENT.
         LDAA   #œFF         MAKE FPACC1 NEGATIVE.
         STAA   MANTSGN1
         JSR    FLTADD       GET THE 90'S COMPLIMENT.
         INC    1,Y          SET THE FLAG.
ANGRED5  PULB                GET THE QUAD COUNT.
         PULA                GET THE COMPLIMENT FLAG.
         RTS                 RETURN WITH THE QUAD COUNT & COMPLIMENT FLAG.
*
*
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
         RTS                 RETURN.
*
*
SINFACT  EQU    *
         FCB    œ6E,œ38,œEF,œ1D         +(1/9!)
         FCB    œ74,œD0,œ0D,œ01         -(1/7!)
         FCB    œ7A,œ08,œ88,œ89         +(1/5!)
         FCB    œ7E,œAA,œAA,œAB         -(1/3!)
*
*
COSFACT  EQU    *
         FCB    œ71,œ50,œ0D,œ01         +(1/8!)
         FCB    œ77,œB6,œ0B,œ61         -(1/6!)
         FCB    œ7C,œ2A,œAA,œAB         +(1/4!)
         FCB    œ80,œ80,œ00,œ00         -(1/2!)
*
*
ONE      FCB    œ81,œ00,œ00,œ00         1.0
PI       FCB    œ82,œ49,œ0F,œDB         3.1415927
THREE60  FCB    œ89,œ34,œ00,œ00         360.0
*
*
*
         TTL    FLTTAN
******************************************************************************
*                                                                            *
*                        FLOATING POINT TANGENT                              *
*                                                                            *
******************************************************************************
*
*
FLTTAN   EQU    *
         JSR    PSHFPAC2     SAVE FPACC2 ON THE STACK.
         JSR    TFR1TO2       PUT A COPY OF THE ANGLE IN FPACC2.
         JSR    FLTCOS        GET COSINE OF THE ANGLE.
         JSR    EXG1AND2      PUT RESULT IN FPACC2 & PUT ANGLE IN FPACC1.
         JSR    FLTSIN        GET SIN OF THE ANGLE.
         JSR    FLTDIV        GET TANGENT OF ANGLE BY DOING SIN/COS.
         BCC    FLTTAN1       IF CARRY CLEAR, ANSWER OK.
         LDX    #MAXNUM       TANGENT OF 90 WAS ATTEMPTED. PUT LARGEST
         JSR    GETFPAC1      NUMBER IN FPACC1.
         LDAA   #TAN90ERR     GET ERROR CODE IN A.
FLTTAN1  JSR    PULFPAC2      RESTORE FPACC2.
         RTS                  RETURN.
*
*
MAXNUM   EQU    *
         FCB    œFE,œ7F,œFF,œFF         LARGEST POSITIVE NUMBER WE CAN HAVE.
*
*
*
         TTL    TRIGUTIL
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
         LDX    #PIOV180     POINT TO CONVERSION CONSTANT PI/180.
DEG2RAD1 JSR    GETFPAC2     PUT IT INTO FPACC2.
         JSR    FLTMUL       CONVERT DEGREES TO RADIANS.
         JSR    PULFPAC2     RESTORE FPACC2.
         RTS                 RETURN. (NOTE! DON'T REPLACE THE "JSR/RTS" WITH
*                            A "JMP" IT WILL NOT WORK.)
*
*
RAD2DEG  EQU    *
         JSR    PSHFPAC2     SAVE FPACC2.
         LDX    #C180OVPI    POINT TO CONVERSION CONSTANT 180/PI.
         BRA    DEG2RAD1     GO DO CONVERSION & RETURN.
*
*
GETPI    EQU    *
         LDX    #PI          POINT TO CONSTANT "PI".
         JMP    GETFPAC1     PUT IT IN FPACC1 AND RETURN.
*
*
PIOV180  EQU    *
         FCB    œ7B,œ0E,œFA,œ35
*
C180OVPI EQU    *
         FCB    œ86,œ65,œ2E,œE1
*
*
*
         TTL    PSHPULFPAC2
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
         TTL    GETFPAC
******************************************************************************
*                                                                            *
*                           GETFPACx SUBROUTINE                              *
*                                                                            *
*       The GETFPAC1 and GETFPAC2 subroutines get a floating point number    *
*       stored in memory and put it into either FPACC1 or FPACC2 in a format *
*       that is expected by all the floating point math routines. These      *
*       routines may easily be replaced to convert any binary floating point *
*       format (i.e. IEEE format) to the format required by the math         *
*       routines.  The "memory" format converted by these routines is shown  *
*       below:                                                               *
*                                                                            *
*       31_______24 23 22_____________________0                              *
*        exponent   s         mantissa                                       *
*                                                                            *
*       The exponent is biased by 128 to facilitate floating point           *
*       comparisons.  The sign bit is 0 for positive numbers and 1           *
*       for negative numbers.  The mantissa is stored in hidden bit          *
*       normalized format so that 24 bits of precision can be obtained.      *
*       Since a normalized floating point number always has its most         *
*       significant bit set, we can use the 24th bit to hold the mantissa    *
*       sign.  This allows us to get 24 bits of precision in the mantissa    *
*       and store the entire number in just 4 bytes.  The format required by *
*       the math routines uses a seperate byte for the sign, therfore each   *
*       floating point accumulator requires five bytes.                      *
*                                                                            *
******************************************************************************
*
*
GETFPAC1 EQU    *
         LDD    0,X          GET THE EXPONENT & HIGH BYTE OF THE MANTISSA,
         BEQ    GETFP12      IF NUMBER IS ZERO, SKIP SETTING THE MS BIT.
         CLR    MANTSGN1     SET UP FOR POSITIVE NUMBER.
         TSTB                IS NUMBER NEGATIVE?
         BPL    GETFP11      NO. LEAVE SIGN ALONE.
         COM    MANTSGN1     YES. SET SIGN TO NEGATIVE.
GETFP11  ORAB   #œ80         RESTORE MOST SIGNIFICANT BIT IN MANTISSA.
GETFP12  STD    FPACC1EX     PUT IN FPACC1.
         LDD    2,X          GET LOW 16-BITS OF THE MANTISSA.
         STD    FPACC1MN+1   PUT IN FPACC1.
         RTS                 RETURN.
*
*
GETFPAC2 EQU    *
         LDD    0,X          GET THE EXPONENT & HIGH BYTE OF THE MANTISSA,
         BEQ    GETFP22      IF NUMBER IS 0, SKIP SETTING THE MS BIT.
         CLR    MANTSGN2     SET UP FOR POSITIVE NUMBER.
         TSTB                IS NUMBER NEGATIVE?
         BPL    GETFP21      NO. LEAVE SIGN ALONE.
         COM    MANTSGN2     YES. SET SIGN TO NEGATIVE.
GETFP21  ORAB   #œ80         RESTORE MOST SIGNIFICANT BIT IN MANTISSA.
GETFP22  STD    FPACC2EX     PUT IN FPACC1.
         LDD    2,X          GET LOW 16-BITS OF THE MANTISSA.
         STD    FPACC2MN+1   PUT IN FPACC1.
         RTS                 RETURN.
*
*
*
         TTL    PUTFPAC
******************************************************************************
*                                                                            *
*                        PUTFPACx SUBROUTINE                                 *
*                                                                            *
*       These two subroutines perform to opposite function of GETFPAC1 and   *
*       GETFPAC2. Again, these routines are used to convert from the         *
*       internal format used by the floating point package to a "memory"     *
*       format. See the GETFPAC1 and GETFPAC2, documentation for a           *
*       description of the "memory" format.                                  *
*                                                                            *
******************************************************************************
*
*
PUTFPAC1 EQU    *
         LDD    FPACC1EX     GET FPACC1 EXPONENT & UPPER 8 BITS OF MANT.
         TST    MANTSGN1     IS THE NUMBER NEGATIVE?
         BMI    PUTFP11      YES.  LEAVE THE M.S. BIT SET.
         ANDB   #œ7F         NO.  CLEAR THE M.S. BIT.
PUTFP11  STD    0,X          SAVE IT IN MEMORY
         LDD    FPACC1MN+1   GET L.S. 16 BITS OF THE MANTISSA.
         STD    2,X
         RTS
*
*
PUTFPAC2 EQU    *
         LDD    FPACC2EX     GET FPACC1 EXPONENT & UPPER 8 BITS OF MANT.
         TST    MANTSGN2     IS THE NUMBER NEGATIVE?
         BMI    PUTFP21      YES.  LEAVE THE M.S. BIT SET.
         ANDB   #œ7F         NO.  CLEAR THE M.S. BIT.
PUTFP21  STD    0,X          SAVE IT IN MEMORY
         LDD    FPACC2MN+1   GET L.S. 16 BITS OF THE MANTISSA.
         STD    2,X
         RTS
*



