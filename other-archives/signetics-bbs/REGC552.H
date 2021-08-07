/**************************************************************************/
/*                                                                        */     
/*                                                                        */        
/*       80C552 Processor Declarations, by Audioaccess Hayward, CA        */
/*       For use with the Franklin "C51" C Compiler                       */
/*                                                                        */     
/*                                                                        */     
/**************************************************************************/




/***************************  BYTE Registers ******************************/

sfr P0     = 0x80;  /* Port 0                  */
sfr P1     = 0x90;  /* Port 1                  */
sfr P2     = 0xA0;  /* Port 2                  */
sfr P3     = 0xB0;  /* Port 3                  */
sfr P4     = 0xC0;  /* Port 4                  */
sfr P5     = 0xC4;  /* Port 5                  */
sfr PSW    = 0xD0;  /* Program Status Word     */
sfr ACC    = 0xE0;  /* Accumulator             */
sfr B      = 0xF0;  /* Multiplication Register */
sfr SP     = 0x81;  /* Stack Pointer           */
sfr DPL    = 0x82;  /* Data Pointer Low Byte   */
sfr DPH    = 0x83;  /* Data Pointer Hi Byte    */
sfr PCON   = 0x87;  /* Power Control           */
sfr TCON   = 0x88;  /* Timer Control           */
sfr TMOD   = 0x89;  /* Timer Mode              */
sfr TL0    = 0x8A;  /* Timer 0 Low Byte        */
sfr TL1    = 0x8B;  /* Timer 1 Low Byte        */
sfr TH0    = 0x8C;  /* Timer 0 Hi Byte         */
sfr TH1    = 0x8D;  /* Timer 1 Hi Byte         */
sfr S0CON  = 0x98;  /* Serial Port 0 Control   */
sfr S0BUF  = 0x99;  /* Serial Port 0 Buffer    */
sfr IEN0   = 0xA8;  /* Int Enable Reg 0        */
sfr CML0   = 0xA9;  /* Compare 0 Low Byte      */
sfr CML1   = 0xAA;  /* Compare 1 Low Byte      */
sfr CML2   = 0xAB;  /* Compare 2 Low Byte      */
sfr CTL0   = 0xAC;  /* Capture 0 Low Byte      */
sfr CTL1   = 0xAD;  /* Capture 1 Low Byte      */
sfr CTL2   = 0xAE;  /* Capture 2 Low Byte      */
sfr CTL3   = 0xAF;  /* Capture 3 Low Byte      */
sfr IP0    = 0xB8;  /* Int Priority Reg 0      */
sfr ADCON  = 0xC5;  /* A/D Converter Control   */
sfr ADCH   = 0xC6;  /* A/D Converter Hi Byte   */
sfr TM2IR  = 0xC8;  /* Timer 2 Interupt Flags  */
sfr CMH0   = 0xC9;  /* Compare 0 Hi Byte       */
sfr CMH1   = 0xCA;  /* Compare 1 Hi Byte       */
sfr CMH2   = 0xCB;  /* Compare 2 Hi Byte       */
sfr CTH0   = 0xCC;  /* Capture 0 Hi Byte       */
sfr CTH1   = 0xCD;  /* Capture 1 Hi Byte       */
sfr CTH2   = 0xCE;  /* Capture 2 Hi Byte       */
sfr CTH3   = 0xCF;  /* Capture 3 Hi Byte       */
sfr S1CON  = 0xD8;  /* Serial Port 1 Control   */
sfr S1STA  = 0xD9;  /* Serial Port 1 Status    */
sfr S1DAT  = 0xDA;  /* Serial Port 1 Data      */
sfr S1ADR  = 0xDB;  /* Serial Port 1 Slave Add */
sfr IEN1   = 0xE8;  /* Int Enable Reg 1        */
sfr IP1    = 0xF8;  /* Int Priority Reg 1      */
sfr TM2CON = 0xEA;  /* Timer 2 Counter Control */
sfr CTCON  = 0xEB;  /* Capture Control         */
sfr TML2   = 0xEC;  /* Timer 2 Low Byte        */
sfr TMH2   = 0xED;  /* Timer 2 Hi Byte         */
sfr STE    = 0xEE;  /* Set Enable              */
sfr RTE    = 0xEF;  /* Reset/Toggle Enable     */
sfr PWM0   = 0xFC;  /* Pulse Width Register 0  */
sfr PWM1   = 0xFD;  /* Pulse Width Register 1  */
sfr PWMP   = 0xFE;  /* Prescaler Freq Control  */
sfr T3     = 0xFF;  /* Watchdog Timer          */

/***************************  BIT Registers *******************************/

sbit TF1  = 0x8F;  /* TCON.7      Timer 1 Overflow Flag     */
sbit TR1  = 0x8E;  /* TCON.6      Timer 1 On/Off Control    */
sbit TF0  = 0x8D;  /* TCON.5      Timer 0 Overflow Flag     */
sbit TR0  = 0x8C;  /* TCON.4      Timer 0 On/Off Control    */
sbit IE1  = 0x8B;  /* TCON.3      Ext Interupt 1 Edge Flag  */
sbit IT1  = 0x8A;  /* TCON.2      Ext Interupt 1 Type       */
sbit IE0  = 0x89;  /* TCON.1      Ext Interupt 0 Edge Flag  */
sbit IT0  = 0x88;  /* TCON.0      Ext Interupt 0 Type       */

sbit CT0I = 0x90;  /* P1.0        Capture/Timer Input 0     */
sbit CT1I = 0x91;  /* P1.1        Capture/Timer Input 1     */
sbit CT2I = 0x92;  /* P1.2        Capture/Timer Input 2     */
sbit CT3I = 0x93;  /* P1.3        Capture/Timer Input 3     */
sbit T2   = 0x94;  /* P1.4        T2 Event Input            */
sbit RT2  = 0x95;  /* P1.5        T2 Timer Reset Signal     */
sbit SCL  = 0x96;  /* P1.6        Serial Port Clock Line I2C*/
sbit SDA  = 0x97;  /* P1.7        Serial Port Data Line I2C */

sbit RI   = 0x98;  /* S0CON.0     Reveive Interupt Flag     */
sbit TI   = 0x99;  /* S0CON.1     Transmit Interupt Flag    */
sbit RB8  = 0x9A;  /* S0CON.2     Receive Bit 8             */
sbit TB8  = 0x9B;  /* S0CON.3     Transmit Bit 8            */
sbit REN  = 0x9C;  /* S0CON.4     Receive Enable            */
sbit SM2  = 0x9D;  /* S0CON.5     Serial Mode Control Bit 2 */
sbit SM1  = 0x9E;  /* S0CON.6     Serial Mode Control Bit 1 */ 
sbit SM0  = 0x9F;  /* S0CON.7     Serial Mode Control Bit 0 */

sbit EA   = 0xAF;  /* IEN0.7      Global Interupt Enable    */
sbit EAD  = 0xAE;  /* IEN0.6      Enable A/D Interupt       */
sbit ES1  = 0xAD;  /* IEN0.5      Serial Port 1 Int Enable  */
sbit ES0  = 0xAC;  /* IEN0.4      Serial Port 0 Int Enable  */
sbit ET1  = 0xAB;  /* IEN0.3      Timer 1 Interupt Enable   */
sbit EX1  = 0xAA;  /* IEN0.2      Ext Interupt 1 Enable     */
sbit ET0  = 0xA9;  /* IEN0.1      Timer 0 Interupt Enable   */
sbit EX0  = 0xA8;  /* IEN0.0      Ext Interupt 0 Enable     */

sbit PAD  = 0xBE;  /* IP0.6       A/D Priority              */
sbit PS1  = 0xBD;  /* IP0.5       SIO 1 Priority            */
sbit PS0  = 0xBC;  /* IP0.4       SIO 0 Priority            */
sbit PT1  = 0xBB;  /* IP0.3       Timer 1 Priority          */
sbit PX1  = 0xBA;  /* IP0.2       Ext Interupt 1 Priority   */
sbit PT0  = 0xB9;  /* IP0.1       Timer 0 Priority          */
sbit PX0  = 0xB8;  /* IP0.0       Ext Interupt 0 Priority   */

sbit RD   = 0xB7;  /* P3.7        Read Enable               */
sbit WR   = 0xB6;  /* P3.6        Write Enable              */
sbit T1   = 0xB5;  /* P3.5        Timer 1 Count Input       */
sbit T0   = 0xB4;  /* P3.4        Timer 0 Count Input       */
sbit INT1 = 0xB3;  /* P3.3        Ext Interupt 1 Input      */
sbit INT0 = 0xB2;  /* P3.2        Ext Interupt 0 Input      */
sbit TXD  = 0xB1;  /* P3.1        Serial Port Transmit      */
sbit RXD  = 0xB0;  /* P3.0        Serial Port Receive       */

sbit T2OV = 0xCF;  /* TM2IR.7     T2 Overflow               */
sbit CMI2 = 0xCE;  /* TM2IR.6     T2 Comparator 2           */
sbit CMI1 = 0xCD;  /* TM2IR.5     T2 Comparator 1           */
sbit CMI0 = 0xCC;  /* TM2IR.4     T2 Comparator 0           */
sbit CTI3 = 0xCB;  /* TM2IR.3     T2 Capture 3              */
sbit CTI2 = 0xCA;  /* TM2IR.2     T2 Capture 2              */
sbit CTI1 = 0xC9;  /* TM2IR.1     T2 Capture 1              */
sbit CTI0 = 0xC8;  /* TM2IR.0     T2 Capture 0              */

sbit CMT1  = 0xC7; /* P4.7        T2 Compare & Toggle Out   */
sbit CMT0  = 0xC6; /* P4.6        T2 Compare & Toggle Out   */
sbit CMSR5 = 0xC5; /* P4.5        T2 Compare & Set/Reset Out*/
sbit CMSR4 = 0xC4; /* P4.4        T2 Compare & Set/Reset Out*/
sbit CMSR3 = 0xC3; /* P4.3        T2 Compare & Set/Reset Out*/
sbit CMSR2 = 0xC2; /* P4.2        T2 Compare & Set/Reset Out*/
sbit CMSR1 = 0xC1; /* P4.1        T2 Compare & Set/Reset Out*/
sbit CMSR0 = 0xC0; /* P4.0        T2 Compare & Set/Reset Out*/

sbit ENS1 = 0xDE;  /* S1CON.6     Enable Serial I/O         */
sbit STA  = 0xDD;  /* S1CON.5     Start Flag                */
sbit STO  = 0xDC;  /* S1CON.4     Stop Flag                 */
sbit SI   = 0xDB;  /* S1CON.3     Serial I/O Interupt       */
sbit AA   = 0xDA;  /* S1CON.2     Assert Acknowledge        */
sbit CR1  = 0xD9;  /* S1CON.1     Clock Rate 1              */
sbit CR0  = 0xD8;  /* S1CON.0     Clock Rate 0              */

sbit CY   = 0xD7;  /* PSW.7       Carry Flag                */
sbit AC   = 0xD6;  /* PSW.6       Aux Carry Flag            */
sbit F0   = 0xD5;  /* PSW.5       Flag 0                    */
sbit RS1  = 0xD4;  /* PSW.4       Register Bank Select 1    */
sbit RS0  = 0xD3;  /* PSW.3       Register Bank Select 0    */
sbit OV   = 0xD2;  /* PSW.2       Overflow Flag             */
sbit F1   = 0xD1;  /* PSW.1       Flag 1                    */
sbit P    = 0xD0;  /* PSW.0       Accumulator Parity Flag   */

sbit ECT0 = 0xE8;  /* IEN1.0      Enable T2 Capture 0       */
sbit ECT1 = 0xE9;  /* IEN1.1      Enable T2 Capture 1       */
sbit ECT2 = 0xEA;  /* IEN1.2      Enable T2 Capture 2       */
sbit ECT3 = 0xEB;  /* IEN1.3      Enable T2 Capture 3       */
sbit ECM0 = 0xEC;  /* IEN1.4      Enable T2 Comparator 0    */
sbit ECM1 = 0xED;  /* IEN1.5      Enable T2 Comparator 1    */
sbit ECM2 = 0xEE;  /* IEN1.6      Enable T2 Comparator 2    */
sbit ET2  = 0xEF;  /* IEN1.7      Enable T2 Overflow        */

sbit PCT0 = 0xF8;  /* IP1.0       T2 Capture 0              */
sbit PCT1 = 0xF9;  /* IP1.1       T2 Capture 1              */
sbit PCT2 = 0xFA;  /* IP1.2       T2 Capture 2              */
sbit PCT3 = 0xFB;  /* IP1.3       T2 Capture 3              */
sbit PCM0 = 0xFC;  /* IP1.4       T2 Comparator 0           */
sbit PCM1 = 0xFD;  /* IP1.5       T2 Comparator 1           */
sbit PCM2 = 0xFE;  /* IP1.6       T2 Comparator 2           */
sbit PT2  = 0xFF;  /* IP1.7       T2 Overflow               */

              