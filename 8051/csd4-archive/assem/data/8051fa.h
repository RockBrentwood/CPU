/* Special function registers for the 8051FA and 8052 */
SADDR  sfr 0xa9; SADEN  sfr 0xb9

T2CON  sfr 0xc8; CCON   sfr 0xd8
T2MOD  sfr 0xc9; CMOD   sfr 0xd9; CL     sfr 0xe9; CH     sfr 0xf9
RCAP2L sfr 0xca; CCAPM0 sfr 0xda; CCAP0L sfr 0xea; CCAP0H sfr 0xfa
RCAP2H sfr 0xcb; CCAPM1 sfr 0xdb; CCAP1L sfr 0xeb; CCAP1H sfr 0xfb
TL2    sfr 0xcc; CCAPM2 sfr 0xdc; CCAP2L sfr 0xec; CCAP2H sfr 0xfc
TH2    sfr 0xcd; CCAPM3 sfr 0xdd; CCAP3L sfr 0xed; CCAP3H sfr 0xfd
                 CCAPM4 sfr 0xde; CCAP4L sfr 0xee; CCAP4H sfr 0xfe

/* Special funcgtion register bits */
/* Port 1 ... does NOT match with Intel's description. */
T2   bit P1.0; T2EX bit P1.1; ECI  bit P1.2; CEX0 bit P1.3
CEX1 bit P1.4; CEX2 bit P1.5; CEX3 bit P1.6; CEX4 bit P1.7

/* IE and IP */
ET2 bit IE.5; EC  bit IE.6
PT2 bit IP.5; PPC bit IP.6

/* T2CON */
CP_RL2 bit T2CON.0; C_T2   bit T2CON.1; TR2    bit T2CON.2; EXEN2  bit T2CON.3
TCLK   bit T2CON.4; RCLK   bit T2CON.5; EXF2   bit T2CON.6; TF2    bit T2CON.7

/* CCON */
CCF0 bit CCON.0; CCF1 bit CCON.1; CCF2 bit CCON.2; CCF3 bit CCON.3
CCF4 bit CCON.4;                  CR   bit CCON.6; CF   bit CCON.7

/*  Non-addressible bits */
/* T2MOD */
DCEN equ 0x00

/* CMOD */
CIDL equ 0x80
WDTE equ 0x40
CPS1 equ 0x04
CPS0 equ 0x02
ECF  equ 0x01

/* CCAPM* */
// 16-bit capture mode    = x 0 * * 0 0 0 *
// 16-bit comparator mode = x * 0 0 * * 0 *
//  8-bit PWM mode        = x * 0 0 0 0 * 0
// Watchdog timer mode    = x * 0 0 1 x 0 x  Counter 4 only, WDTE set.
ECOM equ 0x40
CAPP equ 0x20
CAPN equ 0x10
MAT  equ 0x08
TOG  equ 0x04
PWM  equ 0x02
ECCF equ 0x01
