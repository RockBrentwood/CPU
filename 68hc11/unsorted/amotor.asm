** AMOTOR - HC11 PROGRAM FOR AUTOMATIC MOTOR CONTROL
*       This program generates a pwm signal of constant frequency
*       and variable duty cycle. The duty cycle changes from 0%
*       to 99% then back to 1% then the cycle is repeated until
*       the HC11 is reset. Port B bits 0 and 1 are complememts of
*       eachother and represent the direction of a turning motor.
*       The direction changes after each cycle.
*       The motor with this program will start in the forward direction
*       and accelerate to full speed then will deccelarate to stop.
*       The motor will now change direction and accelerate in the
*       reverse direction until full speed then will deccelerate to
*       stop then the whole cycle is repeated. Short delays occurr
*       between each change in duty cycle to decrease the acceleration
*       and deceleration of the motor. Longer delays are present at
*       full speed and at stop to have a trapoziodal profile.
*
*       The program also outputs to a terminal the current speed and
*       direction through the SCI port to a dummy terminal.
*
*       This program is to be used with any HC11 with BUFFALO 
*       (monitor and debug program) availabe in ROM.  The program length 
*       is approximately 512 bytes and can therefore also be used with 
*       EEPROM versions of the HC11.
*
*       The HC11 is used in conjunction with kits KITDEVB103 and KITDEVB118 
*       and application notes AN1300 and AN1301 for dc brushed motor drive.
*       Kits and ap notes are availale through Motorola Literature Distribution.
* 
* TIMER REGISTER ADDRESSES
REGBAS  EQU     $1000           * Register base
OC1M    EQU     $0C             * OC1 action mask register
OC1D    EQU     $0D             * OC1 action data register
TOC1    EQU     $16             * Output compare 1 register
TOC2    EQU     $18             * Output compare 2 register
TOC3    EQU     $1A             * Output compare 3 register
TCTL1   EQU     $20             * Timer control register 1
TMSK1   EQU     $22             * Timer interrupt mask register 1
TFLG1   EQU     $23             * Timer interrupt flag register 1
TCNT    EQU     $0E             * Timer counter register
 
* BUFFALO I/O ROUTINE ADDRESSES
INCHAR  EQU     $FFCD           * Get character from terminal
OUTCRLF EQU     $FFC4           * Output a CR and LF to screen
OUTSTRG EQU     $FFC7           * Output a string
OTSTRGO EQU     $FFCA           * Out string w/o preceedin crlf
OUTA    EQU     $FFB8           * Output contents of acca
OUTLHLF EQU     $FFB2           * Cnvrt lft nibble to ascii & out
OUTRHLF EQU     $FFB5           * Cnvrt rt nibble to ascii & out
INIT    EQU     $FFA9           * Initialize i/o device routine
VECINIT EQU     $FFD0           * Initialize interrupt vectors routine

* ADDITIONAL ADDRESS AND CONSTANT EQUATES
ACIA    EQU     $9800           * ACIA address
AUTOLF  EQU     $00A6           * Auto lf flag location
BPROT   EQU     $1035           * Block protect
COPRST  EQU     $103A           * Coprest register
DELAY   EQU     $FF00           * Delay time for oc3
PIOC    EQU     $1002           * Parallel i/o control reg
OPTION  EQU     $1039           * Option register
IODEV   EQU     $A7             * I/O device 0=sci 1=acia 2=duarta
EXTDEV  EQU     $A8             * External device 0=none 1=acia
HOSTDEV EQU     $A9             * Host device 0=sci 1=acia
SP      EQU     $006F           * User sp location
UCCR    EQU     $006E           * User ccr location
PERIOD  EQU     $0000           * Program variable location
PERCENT EQU     $0002           *       "
PWIDTH  EQU     $0004           *       "
TRIBYTS EQU     $0006           *       "
FLAGS   EQU     $0009           *       "
PORTB   EQU     $1004           * Port B
EOT     EQU     $04             * End of transmission

* PSEUDO VECTOR EQUATES FOR OUTPUT COMPARE INTERRUPTS
PVOC1   EQU     $00DF
PVOC2   EQU     $00DC
PVOC3   EQU     $00D9
 
 
* START OF MAIN PROGRAM
        ORG     $B600
        LDS     #$0033          * User's stack area on EVB
        JSR     SETUP
***INTRO MESSAGE AND WAIT TILL KEY PRESS TO CONTINUE
        LDX     #INTRO          * X points to intro message
        JSR     OUTSTRG                 * Output message
        JSR     INCHAR          * Wait for a key to continue program
        JSR     INIT_OC         * Initialize timer stuff

*
*RUN CODE FOR AUTOMATIC MODE
RUN     BCLR    FLAGS $01               * Clear cntdir flag (count up 1st)
        LDAA    #$01            * Fwd pin hi rev pin low
        STAA    PORTB           * Initialize the direction
        CLI                     * Enable interrputs
DIRECT  COM     PORTB           * Change directions
        LDAA    PORTB           * Check bit 0
        LSLA                    * Shift Bit 0 into carry
        BCC     BACKWD          * Carry=0, then backward else forward
        LDX     #FWD            * X points to forward sign
        JSR     OUTSTRG         * Output message
        BRA     HOP             * Hop over reverse message
BACKWD  LDX     #REV            * X points to reverse sign
        JSR     OUTSTRG         * Output message
HOP     LDX     #SPDMSG         * X points to % duty cycle message
        JSR     OUTSTRG         * Output message
                LDAA #$01               * Start from 1
CHANGE  PSHA                    * save count on stack
        JSR     UPDATE          * display and change pwidth approp
        PULA                    * Get count from stack
        BSET    FLAGS $80               * Set chgspd flag
        LDY     #$0006          * Count to wait until next change
        JSR     WAIT            * Wait between increments routine
        LDAB    FLAGS           * Get flags byte
        BITB    #$01            * Count direction = 1 ?
        BNE     DOWN            * Count down if =1 else count up
        ADDA    #$01            * Add one to count in acca
        DAA                     * Adding in BCD
        BNE     CHANGE          * Sum not = 0 branch to change
        BSET    FLAGS $01               * Set count direction flag
        LDY     #$0013          * Long delay count before changing
*                               * count direction.
        JSR     WAIT            * Wait between increments
        LDAA    #$99            * Start count at 99 and go down
DOWN    BITA    #$0F            * Check for hex number $_F
        BNE     LEAP            * Not $_F then leap else
        SUBA    #$06            * Subtract 6 from count for bcd #s
LEAP    SUBA    #$01            * Subraact 1 from count
        BNE     CHANGE          * Count not= 0 then change else
        BCLR    FLAGS $01               * Clear cntdir flag (count up)
        LDY     #$0013          * Lng dlay cnt bfor changin dir
        JSR     WAIT            * Wait
        BRA     DIRECT          * Branch always to direct
*
* SETUP-        This subroutine configures the communication port
*       initializes interrupt vectors, sets the default user stack
*       and ccr, clears program flags, stores the period of the pwm
*       signal and sets up the pseudo vectors for the output compares.
*
SETUP   LDAA    #$93
        STAA    OPTION          * adpu, dly, irqe, cop
        CLRA
        STAA    BPROT           * Eeprom block protect
        STAA    IODEV
        JSR     VECINIT                 * Initialize interrupt vectors
        LDX     #$0047
        STX     SP              * Default user stack pointer
        LDAA    #$D0
        STAA    UCCR            * Default user CCR
        LDAA    #$00            * Acca = 0
        STAA    EXTDEV          * External device = ACIA
        STAA    IODEV           * I/O thru ACIA
        STAA    HOSTDEV
        INCA
        STAA    AUTOLF          * Auto lf with cr on
        JSR     INIT            * Initialize ACIA
        CLRA    FLAGS           * Clear all flags
        LDD     #$008D          * Time between oc1 interrupts
        STD     PERIOD          * Save in period
**SETUP OF PSEUDO VECTORS***
        LDAA    #$7E            * Op code for jump
        STAA    PVOC1           * OC1 pseudo vector
        STAA    PVOC2           *       "
        STAA    PVOC3           *       "
        LDD     #OC1ISR         * Address of OC1 interrupt service routine
        STD     PVOC1+1         * Finish JMP inst to OC1 routine
        LDD     #OC2ISR         * Address of OC2 interrupt servic
        STD     PVOC2+1         * Finish JMP inst to OC1 routine
        RTS

*INIT_OC        This subroutine initializes the timer and oc's
*       & toc1 and toc2 registers.
INIT_OC LDX     #REGBAS         * Set for indexed addressing
        LDAA    #%11000000              * Set OM2 & OML
        STAA    TCTL1,X         * OC2 sets its pin hi
        LDAA    #%01000000              * OC1 affects OC2 pin
        STAA    OC1M,X          * Sets OC1 action mask
        CLRA                    * OC1 sets all OCs low
        STAA    OC1D,X          * OC1 action data reg.
        LDD     PERIOD          * Period into accd
        LSLD                    * Period times 2
        ADDD    TCNT,X          * Add 2 periods of TCNT
        ADDD    PERIOD          * Add period to OC1 time
        STD     TOC2,X          * Store OC2 time
        BCLR    TFLG1,X %00111111       * Clear OC1, OC2, &OC3 flags
        BSET    TMSK1,X %11000000       * Enable OCI & OC2 interrupts
        RTS
*
*UPDATE-        This subroutines outputs the current % duty cycle and
*       converts the BCD number to a hex number, then does the
*       multiplication by the period.
UPDATE  PSHA                    * Save count on stack
        PSHA                    * Save count again on stack
        JSR     OUTLHLF                 * Output left nibble as ascii
        STAA    PERCENT         * Store ascii form of left nibble
        PULA                    * Get copy of count from stack
        JSR     OUTRHLF         * Output right nibble as ascii
        STAA    PERCENT+1               * Store ascii form of rt nibble
        LDAA    #$08            * Ascii bs into acca
        JSR     OUTA            * Output a back space
        JSR     OUTA            * Another bs, ready to output %
        PULA                    * Get count from stack
CONVERT LDX     #PERCENT                * X points ascii bcd % characters
        JSR     BCDHEX          * Convert from ascii bcd to hex
        LDX     #PERIOD         * Multiplier
        JSR     MUL2BY1         * period*percent=tribyts
        JSR     DIV3BYT         * tribyts/100=pwidth(accd)
        STD     PWIDTH          * Store result in PWIDTH
        RTS

*WAIT-  This subroutine provides the delays between increments or
*       decrements in the count in automatic mode.      It also serves
*       to give longer delays at full speed in both forward and
*       reverse directions and at stop. A number in Y indicates
*       how many continious delays there will be.
WAIT    LDX     #REGBAS         * Set up for indexed addressing
        PSHA                    * Save count on stack
AGAIN   LDD     #DELAY          * Delay time for oc3
        ADDD    TCNT,X          * Add timer count to delay
        STD     TOC3,X          * Store in oc3 timer
        BCLR    TFLG1,X %11011111       * Clear the oc3 flag
CIRCLE  BRSET   TFLG1,X %00100000 TSTY  * Oc3 flag set? yes>tsty
        LDAA    #$55            * Reset cop so no time out
        STAA    COPRST          *
        LDAA    #$AA            *
        STAA    COPRST          *
        BRA     CIRCLE          * Poll oc3 flag
TSTY    DEY                     * Decrement Y
        BNE     AGAIN           * Y=0 then return, else loop again
                PULA            * Pull count from stack
        RTS
*
*DSPLYSPD-      This subroutine displays current % duty cycle message then
*       the current %. X should point to the ascii characters
*
DSPLASP PSHX                    * Save X on stack
        LDX     #SPDMSG         * X points to message
        JSR     OTSTRGO         * Out message w/o preceeding crlf
        PULX                    * Set for indexed addressing
        LDAA    0,X             * Get first characer
        JSR     OUTA            * Output character
        LDAA    1,X             * Get 2nd character
        JSR     OUTA            * Output character
        RTS
*
*BCD_HEX-       This subroutine takes the two bytes pointed to by X
*       as ascii two digit bcd number and converts to a hex equiv-
*       alent and returns the result in ACCA.
BCDHEX  LDAA    0,X             * Get hi byte of ascii bcd number
        CMPA    #$20            * If a space then only one digit
        BNE     TUDGITS         * not space then 2 digits
        LDAA    1,X             * Only one digit
        ANDA    #$0F            * Mask out the 4 msbs
        BRA     DONE            * result in acca so done
TUDGITS ANDA    #$0F            * Mask out the 4 msbs of hi byte
        LDAB    #$0A            * Ten into accb
        MUL                     * Acca*accb=accb(result of mul)
        LDAA    1,X             * Lo byte of ascii num into acca
        ANDA    #$0F            * Mask out the 4 msbs of lo byte
        ABA                     * acca+accb=hex number in acca
DONE    RTS

*MUL2BY1-       This subroutine multiplies a 2 byte number pointed to by
*       X by a 1byte number located in acca with the result stored
*       number located in ACCB with the result stored in location
*       TRIBYTS
MUL2BY1 PSHA                    * Save hex number on the stack
        LDAB    1,X             * Get low byte of period
        MUL                     * multiply
        STAB    TRIBYTS+2               * Lowest byte of total result
        PULB                    * Get copy of hex rep of %
        PSHA                    * To be added to next mul result
        LDAA    0,X             * Get high byte of period
        MUL                     * Multiply
        STD     TRIBYTS         * Store this result in TRIBYTS
        PULB                    * Part to be added to last mul
        CLRA                    * Hi byte of accd = 0
        ADDD    TRIBYTS         * Add accd to last mul result
        STD     TRIBYTS         * 24 bit result @ TRIBYTS
        RTS
*
*DIV3BYT-       This subroutine divides a 24 bit number pointed to by Y
*       by a number in X with the result to be in ACCD
*
DIV3BYT LDD     TRIBYTS         * Get upper 16 bits of dividend
        LDX     #$0064          * Load X with divisor
        IDIV                    * Divide
        PSHB                    * Save remainder
        XGDX                    * Put quotient in accb
        PULA                    * Remainder as hi byte in accd
        PSHB                    * Save hi byte of result
        LDAB    TRIBYTS+2               * Get last byte of dividend
        LDX     #$0064          * Load X with divisor
        IDIV                    * Divide
        XGDX                    * Quotient of 2nd div in accb
        PULA                    * Get hi byte of total result
*                               * Total result now in accd
        RTS

* OC1 INTERRUPT SERVICE ROUTINE
OC1ISR  LDX     #REGBAS         * Set for indexed addressing
        LDD     TOC1,X          * Get old OC1 time
        ADDD    PERIOD          * Add one period
        STD     TOC1,X          * Store next OC1 time
        BCLR    TFLG1,X %01111111       * Clear OC1 flag bit
        RTI

* OC2 INTERRUPT SERVICE ROUTINE
OC2ISR  LDX     #REGBAS         * Set for indexed addressing
        BRSET   FLAGS $80 NEWPW * bra if newspeed flag set
        LDD     TOC2,X          * Get old OC2 tim
        ADDD    PERIOD          * Add a period
        STD     TOC2,X          * Store next OC2 time
        BCLR    TFLG1,X %10111111       * Clear OC2 flag
        RTI
NEWPW   LDD     TOC1,X          * Use time for oc1 as reference
        ADDD    PERIOD          * Add a period
        ADDD    PERIOD          * Add a second period
        SUBD    PWIDTH          * Subtract pw % to get oc2
        STD     TOC2,X          * Store next oc2 time
        BCLR    FLAGS %10000000 * Clear newspeed flag
        BCLR    TFLG1,X %10111111       * Clear OC2 flag
        RTI
*
**MENUS**
INTRO   FCC     '       MC68HC11/ICePAK MOTOR CONTROL'
        FCB     $0D
        FCB     EOT
FWD     FCC     'FORWARD'
        FCB     EOT
REV     FCC     'REVERSE'
        FCB     EOT
SPDMSG  FCC     'PWM DUTY CYCLE %: '
        FCB     EOT
                                                                                                                   