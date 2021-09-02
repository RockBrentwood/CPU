;**************************************************************************
;*                                                                        *     
;*                                                                        *        
;*       80C552 Processor Declarations, by Audioaccess Hayward, CA        *
;*       For use with the Franklin "A51" Assembler                        *
;*                                                                        *     
;*                                                                        *     
;**************************************************************************




;***************************  BYTE Registers ******************************

P0      DATA    080H        ;Port 0                  
P1      DATA    090H        ;Port 1                  
P2      DATA    0A0H        ;Port 2                  
P3      DATA    0B0H        ;Port 3                  
P4      DATA    0C0H        ;Port 4                  
P5      DATA    0C4H        ;Port 5                  
PSW     DATA    0D0H        ;Program Status Word     
ACC     DATA    0E0H        ;Accumulator             
B       DATA    0F0H        ;Multiplication Register 
SP      DATA    081H        ;Stack Pointer           
DPL     DATA    082H        ;Data Pointer Low Byte   
DPH     DATA    083H        ;Data Pointer Hi Byte    
PCON    DATA    087H        ;Power Control           
TCON    DATA    088H        ;Timer Control           
TMOD    DATA    089H        ;Timer Mode              
TL0     DATA    08AH        ;Timer 0 Low Byte        
TL1     DATA    08BH        ;Timer 1 Low Byte        
TH0     DATA    08CH        ;Timer 0 Hi Byte         
TH1     DATA    08DH        ;Timer 1 Hi Byte         
S0CON   DATA    098H        ;Serial Port 0 Control   
S0BUF   DATA    099H        ;Serial Port 0 Buffer    
IEN0    DATA    0A8H        ;Int Enable Reg 0        
CML0    DATA    0A9H        ;Compare 0 Low Byte      
CML1    DATA    0AAH        ;Compare 1 Low Byte      
CML2    DATA    0ABH        ;Compare 2 Low Byte      
CTL0    DATA    0ACH        ;Capture 0 Low Byte      
CTL1    DATA    0ADH        ;Capture 1 Low Byte      
CTL2    DATA    0AEH        ;Capture 2 Low Byte      
CTL3    DATA    0AFH        ;Capture 3 Low Byte      
IP0     DATA    0B8H        ;Int Priority Reg 0      
ADCON   DATA    0C5H        ;A/D Converter Control   
ADCH    DATA    0C6H        ;A/D Converter Hi Byte   
TM2IR   DATA    0C8H        ;Timer 2 Interupt Flags  
CMH0    DATA    0C9H        ;Compare 0 Hi Byte       
CMH1    DATA    0CAH        ;Compare 1 Hi Byte       
CMH2    DATA    0CBH        ;Compare 2 Hi Byte       
CTH0    DATA    0CCH        ;Capture 0 Hi Byte       
CTH1    DATA    0CDH        ;Capture 1 Hi Byte       
CTH2    DATA    0CEH        ;Capture 2 Hi Byte       
CTH3    DATA    0CFH        ;Capture 3 Hi Byte       
S1CON   DATA    0D8H        ;Serial Port 1 Control   
S1STA   DATA    0D9H        ;Serial Port 1 Status    
S1DAT   DATA    0DAH        ;Serial Port 1 Data      
S1ADR   DATA    0DBH        ;Serial Port 1 Slave Add 
IEN1    DATA    0E8H        ;Int Enable Reg 1        
IP1     DATA    0F8H        ;Int Priority Reg 1      
TM2CON  DATA    0EAH        ;Timer 2 Counter Control 
CTCON   DATA    0EBH        ;Capture Control         
TML2    DATA    0ECH        ;Timer 2 Low Byte        
TMH2    DATA    0EDH        ;Timer 2 Hi Byte         
STE     DATA    0EEH        ;Set Enable              
RTE     DATA    0EFH        ;Reset/Toggle Enable     
PWM0    DATA    0FCH        ;Pulse Width Register 0  
PWM1    DATA    0FDH        ;Pulse Width Register 1  
PWMP    DATA    0FEH        ;Prescaler Freq Control  
T3      DATA    0FFH        ;Watchdog Timer          
                            
;***************************  BIT Registers *******************************
               
TF1     BIT     08FH        ;TCON.7      Timer 1 Overflow Flag     
TR1     BIT     08EH        ;TCON.6      Timer 1 On/Off Control    
TF0     BIT     08DH        ;TCON.5      Timer 0 Overflow Flag     
TR0     BIT     08CH        ;TCON.4      Timer 0 On/Off Control    
IE1     BIT     08BH        ;TCON.3      Ext Interupt 1 Edge Flag  
IT1     BIT     08AH        ;TCON.2      Ext Interupt 1 Type       
IE0     BIT     089H        ;TCON.1      Ext Interupt 0 Edge Flag  
IT0     BIT     088H        ;TCON.0      Ext Interupt 0 Type       
                 
CT0I    BIT     090H        ;P1.0        Capture/Timer Input 0     
CT1I    BIT     091H        ;P1.1        Capture/Timer Input 1     
CT2I    BIT     092H        ;P1.2        Capture/Timer Input 2     
CT3I    BIT     093H        ;P1.3        Capture/Timer Input 3     
T2      BIT     094H        ;P1.4        T2 Event Input            
RT2     BIT     095H        ;P1.5        T2 Timer Reset Signal     
SCL     BIT     096H        ;P1.6        Serial Port Clock Line I2C
SDA     BIT     097H        ;P1.7        Serial Port Data Line I2C 

RI      BIT     098H        ;S0CON.0     Reveive Interupt Flag     
TI      BIT     099H        ;S0CON.1     Transmit Interupt Flag    
RB8     BIT     09AH        ;S0CON.2     Receive Bit 8             
TB8     BIT     09BH        ;S0CON.3     Transmit Bit 8            
REN     BIT     09CH        ;S0CON.4     Receive Enable            
SM2     BIT     09DH        ;S0CON.5     Serial Mode Control Bit 2 
SM1     BIT     09EH        ;S0CON.6     Serial Mode Control Bit 1  
SM0     BIT     09FH        ;S0CON.7     Serial Mode Control Bit 0 

EA      BIT     0AFH        ;IEN0.7      Global Interupt Enable    
EAD     BIT     0AEH        ;IEN0.6      Enable A/D Interupt       
ES1     BIT     0ADH        ;IEN0.5      Serial Port 1 Int Enable  
ES0     BIT     0ACH        ;IEN0.4      Serial Port 0 Int Enable  
ET1     BIT     0ABH        ;IEN0.3      Timer 1 Interupt Enable   
EX1     BIT     0AAH        ;IEN0.2      Ext Interupt 1 Enable     
ET0     BIT     0A9H        ;IEN0.1      Timer 0 Interupt Enable   
EX0     BIT     0A8H        ;IEN0.0      Ext Interupt 0 Enable     

PAD     BIT     0BEH        ;IP0.6       A/D Priority              
PS1     BIT     0BDH        ;IP0.5       SIO 1 Priority            
PS0     BIT     0BCH        ;IP0.4       SIO 0 Priority            
PT1     BIT     0BBH        ;IP0.3       Timer 1 Priority          
PX1     BIT     0BAH        ;IP0.2       Ext Interupt 1 Priority   
PT0     BIT     0B9H        ;IP0.1       Timer 0 Priority          
PX0     BIT     0B8H        ;IP0.0       Ext Interupt 0 Priority   
                            
RD      BIT     0B7H        ;P3.7        Read Enable               
WR      BIT     0B6H        ;P3.6        Write Enable              
T1      BIT     0B5H        ;P3.5        Timer 1 Count Input       
T0      BIT     0B4H        ;P3.4        Timer 0 Count Input       
INT1    BIT     0B3H        ;P3.3        Ext Interupt 1 Input      
INT0    BIT     0B2H        ;P3.2        Ext Interupt 0 Input      
TXD     BIT     0B1H        ;P3.1        Serial Port Transmit      
RXD     BIT     0B0H        ;P3.0        Serial Port Receive       

T2OV    BIT     0CFH        ;TM2IR.7     T2 Overflow               
CMI2    BIT     0CEH        ;TM2IR.6     T2 Comparator 2           
CMI1    BIT     0CDH        ;TM2IR.5     T2 Comparator 1           
CMI0    BIT     0CCH        ;TM2IR.4     T2 Comparator 0           
CTI3    BIT     0CBH        ;TM2IR.3     T2 Capture 3              
CTI2    BIT     0CAH        ;TM2IR.2     T2 Capture 2              
CTI1    BIT     0C9H        ;TM2IR.1     T2 Capture 1              
CTI0    BIT     0C8H        ;TM2IR.0     T2 Capture 0              

CMT1    BIT     0C7H        ;P4.7        T2 Compare & Toggle Out  
CMT0    BIT     0C6H        ;P4.6        T2 Compare & Toggle Out  
CMSR5   BIT     0C5H        ;P4.5        T2 Compare & Set/Reset Out
CMSR4   BIT     0C4H        ;P4.4        T2 Compare & Set/Reset Out
CMSR3   BIT     0C3H        ;P4.3        T2 Compare & Set/Reset Out
CMSR2   BIT     0C2H        ;P4.2        T2 Compare & Set/Reset Out
CMSR1   BIT     0C1H        ;P4.1        T2 Compare & Set/Reset Out
CMSR0   BIT     0C0H        ;P4.0        T2 Compare & Set/Reset Out

ENS1    BIT     0DEH        ;S1CON.6     Enable Serial I/O         
STA     BIT     0DDH        ;S1CON.5     Start Flag                
STO     BIT     0DCH        ;S1CON.4     Stop Flag                 
SI      BIT     0DBH        ;S1CON.3     Serial I/O Interupt       
AA      BIT     0DAH        ;S1CON.2     Assert Acknowledge        
CR1     BIT     0D9H        ;S1CON.1     Clock Rate 1              
CR0     BIT     0D8H        ;S1CON.0     Clock Rate 0              
                
CY      BIT     0D7H        ;PSW.7       Carry Flag                
AC      BIT     0D6H        ;PSW.6       Aux Carry Flag            
F0      BIT     0D5H        ;PSW.5       Flag 0                    
RS1     BIT     0D4H        ;PSW.4       Register Bank Select 1    
RS0     BIT     0D3H        ;PSW.3       Register Bank Select 0    
OV      BIT     0D2H        ;PSW.2       Overflow Flag             
F1      BIT     0D1H        ;PSW.1       Flag 1                    
P       BIT     0D0H        ;PSW.0       Accumulator Parity Flag   

ECT0    BIT     0E8H        ;IEN1.0      Enable T2 Capture 0       
ECT1    BIT     0E9H        ;IEN1.1      Enable T2 Capture 1       
ECT2    BIT     0EAH        ;IEN1.2      Enable T2 Capture 2       
ECT3    BIT     0EBH        ;IEN1.3      Enable T2 Capture 3       
ECM0    BIT     0ECH        ;IEN1.4      Enable T2 Comparator 0    
ECM1    BIT     0EDH        ;IEN1.5      Enable T2 Comparator 1    
ECM2    BIT     0EEH        ;IEN1.6      Enable T2 Comparator 2    
ET2     BIT     0EFH        ;IEN1.7      Enable T2 Overflow        

PCT0    BIT     0F8H        ;IP1.0       T2 Capture 0              
PCT1    BIT     0F9H        ;IP1.1       T2 Capture 1              
PCT2    BIT     0FAH        ;IP1.2       T2 Capture 2              
PCT3    BIT     0FBH        ;IP1.3       T2 Capture 3              
PCM0    BIT     0FCH        ;IP1.4       T2 Comparator 0           
PCM1    BIT     0FDH        ;IP1.5       T2 Comparator 1           
PCM2    BIT     0FEH        ;IP1.6       T2 Comparator 2           
PT2     BIT     0FFH        ;IP1.7       T2 Overflow               

                
                
����������������������������������������������������������������������������������������
