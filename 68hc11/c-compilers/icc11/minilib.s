***************************************************************************
*									  *
* Run-time library for the Mini Board 2.0				  *
*									  *
* for version 0.4x of Richard Man's icc11 compiler			  *
* for info on icc11, send e-mail to imagecft@netcom.com			  *
*									  *
* (C) 1992-93 Fred G. Martin (fredm@media.mit.edu)			  *
* last updated Sat Dec 18  9:55:34 1993					  *
*									  *
* TO INSTALL, COPY THIS FILE TO REPLACE THE "crt.s" THAT IS DISTRIBUTED	  *
* WITH VERSION 0.4x OF ICC11.  MAKE A BACK-UP COPY OF THE ORIGINAL "crt.s"*
* FILE FOR YOUR OWN USAGE.						  *
*									  *
* version history:							  *
* Sat Dec 18  9:55:34 1993						  *
*   installed "SECT" pseudo-ops for program code and global variables	  *
*									  *
* Mon Nov 29 13:45:45 1993						  *
*   fixed bug in putchar routine; thanks to dyoest@zaphod.gsfc.nasa.gov	  *
*									  *
* Fri Nov 19 10:00:38 1993						  *
*   added "__global_start" label to allow global variables to be defined  *
*   in separate .s file							  *
*									  *
* Thanks to two people who helped adapt my Mini Board code, originally	  *
* written for Dunfield Development System's Micro-C compiler, to	  *
* Richard Man's ICC11 cross-compiler:					  *
*									  *
*   Dennis Venable		Serge Boentges				  *
*   Xerox Corporation		Departement of Physical Chemistry,	  *
*   800 Phillips Rd.		ETH Zurich, Switzerland,		  *
*   MS 128-27E			(ported MB library for gcc11 compiler)	  *
*   Webster, NY 14580							  *
*   venable@wrc.xerox.com	sebo@nmr.lpc.ethz.ch			  *
*									  *
*									  *
* The following functions are defined:					  *
* ----------------------------------------------------------------------- *
*									  *
* int analog(int port)		Returns analog reading from port.	  *
*				Ports are numbered 0 to 7.		  *
*									  *
* int digital(int port)		Returns 1 or 0 (true or false) value	  *
*				from Port C.				  *
*									  *
* void motor(int m, int s)	Sets motor m at speed s.  Motors are	  *
*				numbered from 1 to 4, speeds are numbered *
*				from -16 (full backward) to +16 (full	  *
*				forward).				  *
*									  *
* void off(int m)		Turns off motor m.			  *
*									  *
* int button(void)		Returns 1 when IRQ button is depressed.	  *
*				System will hang while button is held	  *
*				down, so effectively, returns true	  *
*				when button is released.		  *
*									  *
*				Button presses are automatically	  *
*				debounced, but a press will get queued if *
*				it happens before you ask for it.	  *
*									  *
* int dipsw(int sw)		Returns state of DIP switch plugged into  *
*				Port D.	 Port D2 is switch #1;		  *
*				D5 is sw #4.				  *
*				Open position is true, closed is false.	  *
*									  *
* void msleep(int msec)		Wastes time for msec milliseconds.	  *
*									  *
* void tone(int freq,		Generates a beep of frequency "freq"	  *
*	    int duration)	for "duration" milliseconds.		  *
*									  *
* void printdec(int n)		prints value of n as unsigned decimal	  *
*				decimal. Appends CR and LF characters.	  *
*									  *
* int getchar()			waits for serial character and returns	  *
*				that value.				  *
*									  *
* putchar(char c)		writes c out to serial line.		  *
*									  *
* int peek(addr)		returns byte value at addr		  *
* int peekword(addr)		returns word value at addr		  *
* poke(addr, byte)		pokes byte at addr			  *
* pokeword(addr, word)		pokes high of word at addr,		  *
*				low of word at addr+1			  *
*									  *
* The following variables are defined:					  *
* ----------------------------------------------------------------------- *
*									  *
* int time			Free-running counter, increments every 	  *
*				millisecond.  Use declaration		  *
*				"extern int time;" in C file to allow	  *
*				use.  Write 0 to time (i.e., "time= 0;")  *
*				to reset to zero.			  *
*									  *
***************************************************************************

*
* file of standard 6811 register declarations
*
*********************************************************************
* Control Registers
*
BASE	EQU	$1000
*
PORTA	EQU	$1000	* Port A data register
RESV1	EQU	$1001	* Reserved
PIOC	EQU	$1002	* Parallel I/O Control register
PORTC	EQU	$1003	* Port C latched data register
PORTB	EQU	$1004	* Port B data register
PORTCL	EQU	$1005	*
DDRC	EQU	$1007	* Data Direction register for port C
PORTD	EQU	$1008	* Port D data register
DDRD	EQU	$1009	* Data Direction register for port D
PORTE	EQU	$100A	* Port E data register
CFORC	EQU	$100B	* Timer Compare Force Register
OC1M	EQU	$100C	* Output Compare 1 Mask register
OC1D	EQU	$100D	* Output Compare 1 Data register
*
* Two-Byte Registers (High,Low -- Use Load & Store Double to access)
TCNT	EQU	$0E   * Timer Count Register
TIC1	EQU	$10   * Timer Input Capture register 1
TIC2	EQU	$12   * Timer Input Capture register 2
TIC3	EQU	$14   * Timer Input Capture register 3
TOC1	EQU	$16   * Timer Output Compare register 1
TOC2	EQU	$18   * Timer Output Compare register 2
TOC3	EQU	$1A   * Timer Output Compare register 3
TOC4	EQU	$1C   * Timer Output Compare register 4
TI4O5	EQU	$1E   * Timer Input compare 4 or Output compare 5 register
*
TCTL1	EQU	$20   * Timer Control register 1
TCTL2	EQU	$21   * Timer Control register 2
TMSK1	EQU	$22   * main Timer interrupt Mask register 1
TFLG1	EQU	$23   * main Timer interrupt Flag register 1
TMSK2	EQU	$24   * misc Timer interrupt Mask register 2
TFLG2	EQU	$25   * misc Timer interrupt Flag register 2
PACTL	EQU	$26   * Pulse Accumulator Control register
PACNT	EQU	$27   * Pulse Accumulator Count register
SPCR	EQU	$28   * SPI Control Register
SPSR	EQU	$29   * SPI Status Register
SPDR	EQU	$2A   * SPI Data Register
BAUD	EQU	$2B   * SCI Baud Rate Control Register
SCCR1	EQU	$2C   * SCI Control Register 1
SCCR2	EQU	$2D   * SCI Control Register 2
SCSR	EQU	$2E   * SCI Status Register
SCDR	EQU	$2F   * SCI Data Register
ADCTL	EQU	$30   * A/D Control/status Register
ADR1	EQU	$31   * A/D Result Register 1
ADR2	EQU	$32   * A/D Result Register 2
ADR3	EQU	$33   * A/D Result Register 3
ADR4	EQU	$34   * A/D Result Register 4
BPROT	EQU	$35   * Block Protect register
RESV2	EQU	$36   * Reserved
RESV3	EQU	$37   * Reserved
RESV4	EQU	$38   * Reserved
OPTION	EQU	$39   * system configuration Options
COPRST	EQU	$3A   * Arm/Reset COP timer circuitry
PPROG	EQU	$3B   * EEPROM Programming register
HPRIO	EQU	$3C   * Highest Priority Interrupt and misc.
INIT	EQU	$3D   * RAM and I/O Mapping Register
TEST1	EQU	$3E   * factory Test register
CONFIG	EQU	$3F   * Configuration Control Register
*
*
* Interrupt Vector locations
*
SCIINT	EQU	$D6	* SCI serial system
SPIINT	EQU	$D8	* SPI serial system
PAIINT EQU	$DA	* Pulse Accumulator Input Edge
PAOVINT EQU	$DC	* Pulse Accumulator Overflow
TOINT	EQU	$DE	* Timer Overflow
TOC5INT EQU	$E0	* Timer Output Compare 5
TOC4INT EQU	$E2	* Timer Output Compare 4
TOC3INT EQU	$E4	* Timer Output Compare 3
TOC2INT EQU	$E6	* Timer Output Compare 2
TOC1INT EQU	$E8	* Timer Output Compare 1
TIC3INT EQU	$EA	* Timer Input Capture 3
TIC2INT EQU	$EC	* Timer Input Capture 2
TIC1INT EQU	$EE	* Timer Input Capture 1
RTIINT	EQU	$F0	* Real Time Interrupt
IRQINT	EQU	$F2	* IRQ External Interrupt
XIRQINT EQU	$F4	* XIRQ External Interrupt
SWIINT	EQU	$F6	* Software Interrupt
BADOPINT EQU	$F8	* Illegal Opcode Trap Interrupt
NOCOPINT EQU	$FA	* COP Failure (Reset)
CMEINT	EQU	$FC	* COP Clock Monitor Fail (Reset)
RESETINT EQU	$FE	* RESET Interrupt
*
* Masks for serial port
PORTDWOM	EQU	$20
BAUD1200	EQU	$B3
BAUD9600	EQU	$B0
TRENA		EQU	$0C	* Transmit, Receive ENAble
*
*
	SECT	1		* variables (RAM) section

	ORG	$00
*
temp	RMB	1
*
* button variables
irqval	RMB	1
buttime RMB	2
*
* system time:	4 bytes
st_hi	RMB	2	* high word
_time	EQU	*	* C accessible time variable
st_lo	RMB	2	* low word
*
* motor control
motctrl RMB	1	* high nybble=on/off* low nybble=dir
speed1	RMB	2
speed2	RMB	2
speed3	RMB	2
speed4	RMB	2
*
* beeper period
beeptone RMB	2
*
* this label tells you where to ORG your globals
* use "sect" commands in v0.4 of icc11;
* this code left in for benefit of v0.3 users
__global_start	EQU	*

	SECT	0		* code section

*
* interrupt vector definitions
*
*
	ORG	$FF00+IRQINT
	FDB	irqi		* defined in 6811st.asm
*
	ORG	$FF00+TOC4INT
	FDB	systemi		* defined in 6811st.asm
*
	ORG	$FF00+TOC5INT
	FDB	beepi
*
	ORG	$FF00+RESETINT
	FDB	$F800
*
***********************************************************************
* 6811st.src
*
*
*
***********************************************************************
*
* start of code
*
* (1) perform initialization of stack, interrupts, system
* (2) code for system interrupt
*
*
	ORG	$F800
	LDS	#$FF
	LDX	#$1000
*
* initialize serial port
	LDAA	#$30		* 9600 baud
	STAA	BAUD,X
	BSET	SCCR2,X $0C	* transmit, receive enable
*
* turn on analog subsystem
	BSET	OPTION,X $80
*
* set up interrupts
* OC4:	1 kHz system interrupt
	LDAA	#%00010000
	STAA	TFLG1,X
	STAA	TMSK1,X
*
* clear D register
	CLRA
	CLRB
*
* initialize system time
	STD	st_hi
	STD	st_lo
*
* initialize button variables
	STAA	irqval
	STD	buttime
*
* initialize motor control
	STAA	motctrl		* default == OFF
	LDD	#$FFFF
	STD	speed1		* full speed
	STD	speed2
	STD	speed3
	STD	speed4
*
* initialize beeper
	BSET	PACTL,X %00001000	* set PA3 for output
*
* enable interrupts
	CLI
*
* call main procedure
	JSR	   _main
*
* if main exits, perform a noop forever
exit	bra	    exit

*
*
* 1 KHz driver routine
*
*
*
systemi EQU	*
	LDX	#BASE		* point to register base
*
* setup for next interrupt
	LDD	#2000		* 2000 cycles = 1 millisec.
	ADDD	TOC4,X		* add TOC5 to D
	STD	TOC4,X		* store back
	BCLR	TFLG1,X %11101111	* clear OC4 for next compare
*
* enable beeper to interrupt us
	BSET	TMSK1,X %00001000
	CLI
*
* increment system time
	LDX	st_lo
	INX
	STX	st_lo
	BNE	si_noinc
	LDX	st_hi
	INX
	STX	st_hi
si_noinc EQU	*
*
*
* do pulse width modulation, 4 motors
* motor 1 (ls bit of low nybble is dir, ls bit of high nybble is on/off
*
pwm	LDX	motctrl		* upper nybble is on/off* lower is dir
*
	LDD	speed1
	LSLD
	BCC	pwmoff1
	ADDD	#1
	XGDX			* get output byte into D* save new speed in X
	EORA	#%00010000	* toggle motor 1 ctrl bit
	BRA	pwmset1
pwmoff1 XGDX
	ORAA	#%00010000	* set motor 1 ctrl bit
pwmset1 XGDX
	STD	speed1
*
	LDD	speed2
	LSLD
	BCC	pwmoff2
	ADDD	#1
	XGDX			* get output byte into D* save new speed in X
	EORA	#%00100000	* toggle motor 2 ctrl bit
	BRA	pwmset2
pwmoff2 XGDX
	ORAA	#%00100000	* set motor 2 ctrl bit
pwmset2 XGDX
	STD	speed2
*
	LDD	speed3
	LSLD
	BCC	pwmoff3
	ADDD	#1
	XGDX			* get output byte into D* save new speed in X
	EORA	#%01000000	* toggle motor 3 ctrl bit
	BRA	pwmset3
pwmoff3 XGDX
	ORAA	#%01000000	* set motor 3 ctrl bit
pwmset3 XGDX
	STD	speed3
*
	LDD	speed4
	LSLD
	BCC	pwmoff4
	ADDD	#1
	XGDX			* get output byte into D* save new speed in X
	EORA	#%10000000	* toggle motor 4 ctrl bit
	BRA	pwmset4
pwmoff4 XGDX
	ORAA	#%10000000	* set motor 4 ctrl bit
pwmset4 XGDX
	STD	speed4
*
*
	XGDX			* get control bits into A
	EORA	#$F0		* toggle speed bits
*
	STAA	PORTB		* do it
*
	RTI
*
*
* PA3: beeper interrupt
beepi	EQU	*
	LDX	#BASE
	LDD	beeptone
	ADDD	TI4O5,X
	STD	TI4O5,X
	BCLR	TFLG1,X %11110111
	RTI
*
*
* IRQ interrupt:  sets irqval to one when interrupt occurs
irqi	EQU	*
	LDD	st_lo
	SUBD	buttime
	CPD	#60
	BLO	irqdone
	LDAA	#1
	STAA	irqval
	LDD	st_lo
	STD	buttime
irqdone RTI
*
*-----------------------------------------
* end startup code
*-----------------------------------------
*
*
* library of functions for Mini Board rev 1.5
* (C) 1992-93 Fred Martin
*
*
_digital	EQU	*
	TSX
	LDAB	PORTC		* get PORTC into B
	LDAA	3,X		* get port # into A
*
digloop BEQ	digdone
	LSRB			* shift B bits down
	DECA
	BRA	digloop
*
digdone ANDB	#1		* mask off all but low bit
	EORB	#1		* invert logical sense due to hardware
*				  A is already zero
	RTS
*
_analog EQU	*
	TSX
	LDAA	3,X		* get port # into A
	SEI			* disable interrupts
	ldx	#BASE
	STAA	ADCTL,X
analoop LDAA	ADCTL,X
	ANDA	#$80
	BEQ	analoop
	CLI			* re-enable them
	LDAB	ADR1,X
	CLRA
	RTS
*
_dipsw	EQU	*
	TSX
	LDAB	PORTD
	LSRB			* desired bits in pos
	LDAA	3,X		* switch number (1 to 4) in A
*
diploop LSRB			* shift B bits down
	DECA
	BNE	diploop
*
	ANDB	#1
	RTS			* A is already zero* B has 1 or 0
*
*
*
* motor(int m, int s)
* m= 1, 2, 3, or 4
* s= -16 (full on backward) to +16 (full on forward)
*
_motor	EQU	*
	TSX
	LDAB	3,X	* motor number	(was 7,X in Dunfield)
	PSHB		* save for later
	LDAA	#$08	* bit mask
	SEC		* set carry, will be rotated into A
msloop	ROLA
	DECB
	BNE	msloop
* A now has motor enable mask
	PSHA
	ORAA	motctrl
	STAA	temp
	PULA
	ANDA	#$0F	* keep dir bit only
	TST	4,X	* high byte of speed
	BPL	mset2
* negative speed => clear direction bit
	COMA
	ANDA	temp
	BRA	mset1
mset2	LDAA	temp
mset1	STAA	motctrl
*
	LDD	4,X	* speed word
	BPL	speedok
	LDD	#0
	SUBD	4,X
*
* number from 0 to 16 now in B
speedok ASLB		* double it
	LDX	#mtable
	ABX		* index into speed table
	LDY	0,X	* get speed bits
	PULB		* get motor number from 1 to 4
	ASLB
	LDX	#speed1-2
	ABX		* ptr to speed entry
	STY	0,X	* store speed bits
* done!
	RTS
*
* table of motor speeds
mtable
	FDB	%0000000000000000	* 0/16
	FDB	%0000000000000001	* 1/16
	FDB	%0000000100000001	* 2/16
	FDB	%0000100001000010	* 3/16
	FDB	%0001000100010001	* 4/16
	FDB	%0010001001001001	* 5/16
	FDB	%0100100100101001	* 6/16
	FDB	%0101001010101010	* 7/16
	FDB	%0101010101010101	* 8/16
	FDB	%1010101010101011	* 9/16
	FDB	%0101011010101011	* 10/16
	FDB	%1101110110110110	* 11/16
	FDB	%1110111011101110	* 12/16
	FDB	%1110111011101111	* 13/16
	FDB	%1110111111101111	* 14/16
	FDB	%1111111011111111	* 15/16
	FDB	%1111111111111111	* 16/16
*
*
* void off(int m)
*
_off	EQU	*
	TSX
	LDAB	3,X	* motor number
	LDAA	#$08	* bit mask
offloop ROLA
	DECB
	BNE	offloop
* A now has motor enable mask
	COMA
	ANDA	motctrl
	STAA	motctrl
	RTS
*
*
* delay for nn,nnn milliseconds
*
_msleep EQU	*
	TSX
	LDD	2,X
sleepsub SEI	* halt interrupts until we perform calculation
	ADDD	st_lo
	CLI	* okay again
sleeplp CPD	st_lo
	BNE	sleeplp
	RTS
*
*
* button(void)
* returns 1 if irqval is one (sets irqval to zero in this case)
* else returns 0
*
_button EQU	*
	CLRA
	LDAB	irqval
	BEQ	butndone
	CLR	irqval
butndone RTS
*
*
* tone( int freq, int duration)
*
_tone	EQU	*
	TSX			*
	LDD	2,X		* frequency
	LSRD
	LSRD
	LSRD
	LSRD			* divide by 16
	PSHX			* save stack ptr
	LDX	#62500		* 1E6 divided by 16
	XGDX
	IDIV			* answer in X
	STX	beeptone
	LDX	#BASE
	BSET	TCTL1,X $01	* set beeper pin to toggle
	BSET	TMSK1,X $08	* turn on beeper interrupt
***	BSET	PORTB $80	* enable motor 4
	PULX			* restore argument SP
	LDD	4,X		* duration arg
	BSR	sleepsub	* sleep for desired duration
	LDX	#BASE
	BCLR	TMSK1,X $08	* turn off beeper interrupt
	BCLR	TCTL1,X $01	* set beeper pin to do nothing
	BCLR	PORTA $08     * turn off beeper
***	BCLR	PORTB $80     * disable motor 4
	LDX	#0
	STX	beeptone
	RTS
**********************************************************************
*	 End of Library and Startup Code			     *
**********************************************************************

**********************************************************************
*	 Serial line Code					     *
*								     *
* functions for interaction over serial line			     *
*								     *
* by Fred Martin, 1992						     *
*								     *
* contains:							     *
*								     *
* void printdec(int n)	prints value of n as unsigned integer.	     *
*			Appends CR and LF characters.		     *
*								     *
* int getchar()		waits for serial character and returns that  *
*			value.					     *
*								     *
* int putchar(int c)	writes c over serial line (w/o translation)  *
*								     *
**********************************************************************
*
RDRF	EQU	$20	* Receive Data Register Full
TDRE	EQU	$80	* Transmit Data Register Empty
*
CR	EQU	$0D
LF	EQU	$0A
*
_printdec	EQU    *
	TSX
	LDD	2,X		* get number
	LDY	#5		* num of digits
	LDX	#10
decdvlp IDIV			* least significant digit in B
	PSHB			* save it
	LDD	#10
	XGDX
	DEY
	BNE	decdvlp
*
* have five digs saved on stack; start printing when not zero
	LDAB	#5		* num digs to output
decoutlp	PULA
	ADDA	#'0'
	BSR	putch
	DECB
	BNE	decoutlp
*
	LDAA	#CR
	BSR	putch
	LDAA	#LF
	BSR	putch
*
	RTS
*
*
* write character in A over serial line
* destroys X register; preserves others
*
_putchar EQU	*
	TSX
	LDAA	3,X
putch	LDX	#BASE
	BRCLR	SCSR,X TDRE   * wait for serial line to finish last char
	STAA	SCDR,X
	RTS
*
_getchar	EQU	*
	LDX	#BASE
	BRCLR	SCSR,X RDRF   * wait for serial char
	LDAB	SCDR,X
	CLRA
	RTS
*
*
*********************************************************************
*								    *
*		memory addressing functions			    *
*								    *
* contains:							    *
*								    *
* peek(addr)		returns byte value in D			    *
* peekword(addr)	returns word value in D			    *
* poke(addr, byte)	pokes byte at addr			    *
* pokeword(addr, word)	pokes high of word at addr,		    *
*			low of word at addr+1			    *
*								    *
*********************************************************************
_peek	EQU *
	TSX		* get SP into X
	LDX	2,X	* get addr into X
	LDAB	0,X	* peek byte
	CLRA
	RTS
*
_peekword	EQU *
	TSX		* get SP into X
	LDX	2,X	* get addr into X
	LDD	0,X	* peek word
	RTS
*
_poke	EQU *
	TSX
	LDAB	5,X	* low of byte
	LDX	2,X	* addr
	STAB	0,X
	CLRA		* return byte
	RTS
*
_pokeword	EQU *
	TSX
	LDD	4,X	* word to poke
	LDX	2,X	* addr
	STD	0,X	* do it
	RTS
*** end of Mini Board stuff

*** start of Richard Man's crt.s
*** _putchar routine replaced by routine above
*** code from here on down is by Richard Man
__lsrd:
	cpy		#0
	beq		__lsrd_done
	lsrd
	dey
	bra		__lsrd
__lsrd_done:
	rts
__lsld:
	cpy		#0
	beq		__lsld_done
	lsld
	dey
	bra		__lsld
__lsld_done:
	rts
__asrd:
	cmpa	#0
	bpl		__lsrd
__asrd_loop:
	cpy		#0
	beq		__asrd_done
	asra
	rorb
	dey
	bra		__asrd_loop
__asrd_done:
	rts
__asgnblk:
	cpd		#0
	beq		__asgnblk_done
__asgnblk_loop:
	pshb
	ldab	0,y
	stab	0,x
	pulb
	subd	#1
	beq		__asgnblk_done
	inx
	iny
	bra		__asgnblk_loop
__asgnblk_done:
	rts
__muli:
__mulu:
	pshx
	pshy
	pshb
	psha
	tsx
	ldaa 3,x
	mul
	pshb
	psha
	ldab 0,x
	ldaa 3,x
	mul
	pula
	aba
	psha
	ldab 1,x
	ldaa 2,x
	mul
	pula
	aba
	pulb
	pulx
	pulx
	pulx
	rts
__divi:
	pshx		
	psha	
	tsta
	bpl	__divi_1
	coma
	comb
	addd	#1
__divi_1:	
	pshy		
	pulx	
	tsy
	cpx	#0
	bpl	__divi_2
	xgdx
	coma
	comb
	xgdx
	inx
	com	0,y
__divi_2:	
	idiv
	xgdx
	tst	0,y	
	bpl	__divi_3
	coma	
	comb
	addd	#1
__divi_3:	
	ins
	pulx
	rts
__modi:
	pshx
	psha
	tsta
	bpl	__modi_1
	coma	
	comb
	addd	#1
__modi_1:	
	pshy
	pulx
	cpx	#0
	bpl	__modi_2
	xgdx	
	coma
	comb
	xgdx
	inx
__modi_2:	
	idiv
	tsx	
	tst	0,x
	bpl	__modi_3
	coma	
	comb
	addd	#1
__modi_3:	
	ins		
	pulx
	rts
