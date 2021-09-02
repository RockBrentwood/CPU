;;; The registers defined on the 8051FA and 8052
T2CON  equ 0c8h
RCAP2L equ 0cah
RCAP2H equ 0cbh
TL2    equ 0cch
TH2    equ 0cdh

SADDR equ 0a9h
SADEN equ 0b9h

T2MOD equ 0c9h

CCON equ 0d8h
CMOD equ 0d9h
CCAPM0 equ 0dah
CCAPM1 equ 0dbh
CCAPM2 equ 0dch
CCAPM3 equ 0ddh
CCAPM4 equ 0deh
CL equ 0e9h
CH equ 0f9h
CCAP0L equ 0eah
CCAP0H equ 0fah
CCAP1L equ 0ebh
CCAP1H equ 0fbh
CCAP2L equ 0ech
CCAP2H equ 0fch
CCAP3L equ 0edh
CCAP3H equ 0fdh
CCAP4L equ 0eeh
CCAP4H equ 0feh

;;; Addressible bits
;;; Port 1 ... does NOT match with Intel's description.
T2   bit P1.0
T2EX bit P1.1
ECI  bit P1.2
CEX0 bit P1.3
CEX1 bit P1.4
CEX2 bit P1.5
CEX3 bit P1.6
CEX4 bit P1.7

;;; IE and IP
ET2 bit IE.5
EC  bit IE.6
PT2 bit IP.5
PPC bit IP.6

;;; T2CON
TF2    bit T2CON.7
EXF2   bit T2CON.6
RCLK   bit T2CON.5
TCLK   bit T2CON.4
EXEN2  bit T2CON.3
TR2    bit T2CON.2
C_T2   bit T2CON.1
CP_RL2 bit T2CON.0

;;; CCON
CF bit CCON.7
CR bit CCON.6
CCF4 bit CCON.4
CCF3 bit CCON.3
CCF2 bit CCON.2
CCF1 bit CCON.1
CCF0 bit CCON.0

;;; Non-addressible bits
;;; T2MOD
DCEN equ 00h

;;; CMOD
CIDL equ 80h
WDTE equ 40h
CPS1 equ  4
CPS0 equ  2
ECF  equ  1

;;; CCAPM*
;;; 16-bit capture mode    = x 0 * * 0 0 0 *
;;; 16-bit comparator mode = x * 0 0 * * 0 *
;;;  8-bit PWM mode        = x * 0 0 0 0 * 0
;;; Watchdog timer mode    = x * 0 0 1 x 0 x  Counter 4 only, WDTE set.
ECOM equ 40h
CAPP equ 20h
CAPN equ 10h
MAT  equ  8
TOG  equ  4
PWM  equ  2
ECCF equ  1
