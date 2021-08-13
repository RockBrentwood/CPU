
Amiga 6502 assembler :  -  VIC Object code boot from USER port                                                               PAGE 1
Line      Location     Label Opcode Operand  Comment   Fri Aug 13 17:11:13 2021

   2                   ;   BOOT : VIC PROGRAM TO RECEIVE AIM FORMAT OBJECT
   3                   ;          CODE FLIES THROUGH THE VIC USER PORT.
   4                           
   5                   ;   VIC 6522 VIA PORT ADDRESSES
   6                           
   7         9110      ORB1    =$9110         ;I/O REGISTER B
   8         9112      DDRB1   =$9112         ;DATA DIRECTION REGISTER B
   9         911C      PCR1    =$911C         ;PERIPHERAL CONTROL REGISTER 1
  10         911D      IFR1    =$911D         ;INTERRUPT FLAG REGISTER
  11         911E      IER1    =$911E         ;INTERRUPT ENABLE REGISTER
  12                   
  13                   ;   VIC subroutines
  14                   
  15         FFE1      STOP    =$FFE1         ;check for stop key down
  16                   
  17                   ;   ZERO PAGE STORAGE
  18                           
  19         00C5      VICKEY  =$C5           ;current key down
  20         00C6      KISTAK  =$C6           ;keyboard input stack
  21         00FB      staksav =$fb           ;stack save for RCHEK sub
  22         00FD      BYTMP   =$FD           ;INPUT BYTE TEMPORARY
  23         00FE      PTRLO   =$FE           ;LOAD POINTER
  24         00FF      PTRHI   =$FF
  25                   
  26         001A      CTLZ    =$1a           ;end of file charcter
  27                           
  28         033C              *=$33C         ;locate in the tape buffer
  29                           
  30  033C   D8        BOOT    CLD
  31  033D   BA                tsx            ;save stack for STOP exit
  32  033E   86 FB             stx   staksav
  33  0340   20 98 03          JSR   INITIO    ;INIT USER PORT
  34  0343   20 B0 03  RECLUP  JSR   GETCHR    ;SCAN FOR A ';'
  35  0346   C9 1A             cmp   #CTLZ     ;end of file char?
  36  0348   F0 28             beq   bootex    ;yes end
  37  034A   C9 3B             CMP   #$3b
  38  034C   D0 F5             BNE   RECLUP
  39  034E   20 74 03          JSR   GETBYT    ;GET RECORD LENGTH
  40  0351   AA                TAX            ;ZERO?
  41  0352   F0 17             BEQ   EOT       ;YES, WE'RE DUN
  42  0354   20 74 03          JSR   GETBYT    ;GET HI ORDER LOAD ADDRESS
  43  0357   85 FF             STA   PTRHI     ;SAVE
  44  0359   20 74 03          JSR   GETBYT    ;GET LO ORDER
  45  035C   85 FE             STA   PTRLO     ;SAVE
  46  035E   A0 00             LDY   #$0       ;CLEAR INDEX
  47  0360   20 74 03  BYTLUP  JSR   GETBYT    ;GET AN OBJECT BYTE
  48  0363   91 FE             STA   (PTRLO),Y  ;SAVE IT
  49  0365   C8                INY            ;BUMP INDEX
  50  0366   CA                DEX            ;COUNT BYTE
  51  0367   D0 F7             BNE   BYTLUP    ;LOOP 'TILL ZERO
  52  0369   F0 D8             BEQ   RECLUP    ;THEN GET NEXT RECORD
  53                           
  54         036B      EOT                    ;look for control z to end
  55  036B   20 B0 03  ENDLUP  JSR   GETCHR    ;TO END THE RECORD
  56  036E   C9 1A             cmp   #CTLZ
  57  0370   D0 F9             BNE   ENDLUP
  58  0372   18        bootex  clc
  59  0373   60                RTS
  60                           
  61                   ;   GETBYT : INPUT TWO ASCII HEX DIGITS AND

Amiga 6502 assembler :  -  VIC Object code boot from USER port                                                               PAGE 2
Line      Location     Label Opcode Operand  Comment   Fri Aug 13 17:11:13 2021

  62                   ;             PACK INTO ONE BYTE.
  63                           
  64  0374   20 B0 03  GETBYT  JSR   GETCHR    ;INPUT BYTE
  65  0377   20 83 03          JSR   PACK      ;PACK INTO 1 NYBBLE
  66  037A   20 B0 03          JSR   GETCHR    ;GET 2ND HALF
  67  037D   20 83 03          JSR   PACK      ;PACK IT
  68  0380   A5 FD             LDA   BYTMP     ;RETRIEVE RESULT
  69  0382   60                RTS
  70                           
  71                   ;   PACK : PACK ASCII DIGIT INTO NYBBLE
  72                           
  73  0383   C9 3A     PACK    CMP   #$3A      ;NUMERIC?
  74  0385   29 0F             AND   #$F       ;CLEAR HI NYBBLE
  75  0387   90 02             BCC   NOADD     ;YES, SKIP ADD
  76  0389   69 08             ADC   #8        ;ADD 8 + CARRY
  77  038B   06 FD     NOADD   ASL   BYTMP     ;SHIFT BYTMP
  78  038D   06 FD             ASL   BYTMP
  79  038F   06 FD             ASL   BYTMP
  80  0391   06 FD             ASL   BYTMP
  81  0393   05 FD             ORA   BYTMP     ;INSERT DIGIT
  82  0395   85 FD             STA   BYTMP     ;SAVE
  83  0397   60                RTS
  84                           
  85                   ;   INITIO : PREPARE THE VIA FOR INPUT
  86                           
  87  0398   AD 1C 91  INITIO  LDA   PCR1      ;GET PCR
  88  039B   29 0F             AND   #$0F     ;CLEAR B PORT BITS
  89  039D   09 A0             ORA   #$A0      ;SET PULSE MODE
  90  039F   8D 1C 91          STA   PCR1      ;PUT IN PCR
  91  03A2   A9 00             LDA   #0        ;SET ALL BITS TO INPUT
  92  03A4   8D 12 91          STA   DDRB1
  93  03A7   A9 10             LDA   #$10      ;DISABLE INTERRUPT
  94  03A9   8D 1E 91          STA   IER1
  95  03AC   AD 10 91          LDA   ORB1      ;GET FIRST CHARACTER
  96  03AF   60                RTS
  97                           
  98                   ;   GETCHR : INPUT 1 BYTE FROM USER PORT
  99                           
 100  03B0   A9 10     GETCHR  LDA   #$10      ;MASK FOR CB1 INTERRUPT
 101  03B2   20 C1 03  WAITC   JSR   RCHEK     ;ALLOW INTERRUPTS
 102  03B5   2C 1D 91          BIT   IFR1      ;IS IT ON?
 103  03B8   F0 F8             BEQ   WAITC     ;NO, WAIT FOR IT
 104  03BA   AD 10 91          LDA   ORB1      ;RETRIEVE BYTE
 105  03BD   8D 10 91          STA   ORB1      ;NOTIFY SENDER
 106  03C0   60                RTS
 107                           
 108                   
 109                           
 110                   ;   RCHEK : CHECK FOR INTERRUPT
 111                           
 112  03C1   48        RCHEK   PHA            ;SAVE ACCUM
 113  03C2   20 E1 FF          JSR   STOP      ;RUN/STOP KEY DOWN?
 114  03C5   D0 09             BNE   CKSPAC    ;NO, SKIPPIT
 115  03C7   A9 00             LDA   #0        ;CLEAR KEYBOARD STACK
 116  03C9   85 C6             STA   KISTAK
 117  03CB   A6 FB             ldx   staksav   ;restore stack
 118  03CD   9A                txs
 119  03CE   18                clc
 120  03CF   60                rts            ;return to caller
 121                   

Amiga 6502 assembler :  -  VIC Object code boot from USER port                                                               PAGE 3
Line      Location     Label Opcode Operand  Comment   Fri Aug 13 17:11:13 2021

 122  03D0   A5 C5     CKSPAC  LDA   VICKEY    ;GET CURRENT KEY
 123  03D2   C9 20             CMP   #$20      ;SPACE BAR DOWN?
 124  03D4   F0 FA             BEQ   CKSPAC    ;YES, PAUSE TILL IT LIFTS
 125  03D6   A9 00             LDA   #0
 126  03D8   85 C6             STA   KISTAK    ;CLEAR KEYBOARD STACK
 127  03DA   68        RCOUT   PLA
 128  03DB   60                RTS
 129                   

Amiga 6502 assembler :  Symbol Cross Reference -                                                                             PAGE 4
Symbol                Value Defined References

 BOOT                  033C   30     (0) 
 BYTLUP                0360   47     (1) 51 
 BYTMP                 00FD   22     (7) 68 77 78 79 80 81 82 
 CKSPAC                03D0   122    (2) 114 124 
 CTLZ                  001A   26     (2) 35 56 
 DDRB1                 9112   8      (1) 92 
 ENDLUP                036B   55     (1) 57 
 EOT                   036B   54     (1) 41 
 GETBYT                0374   64     (4) 39 42 44 47 
 GETCHR                03B0   100    (4) 34 55 64 66 
 IER1                  911E   11     (1) 94 
 IFR1                  911D   10     (1) 102 
 INITIO                0398   87     (1) 33 
 KISTAK                00C6   20     (2) 116 126 
 NOADD                 038B   77     (1) 75 
 ORB1                  9110   7      (3) 95 104 105 
 PACK                  0383   73     (2) 65 67 
 PCR1                  911C   9      (2) 87 90 
 PTRHI                 00FF   24     (1) 43 
 PTRLO                 00FE   23     (2) 45 48 
 RCHEK                 03C1   112    (1) 101 
 RCOUT                 03DA   127    (0) 
 RECLUP                0343   34     (2) 38 52 
 STOP                  FFE1   15     (1) 113 
 VICKEY                00C5   19     (1) 122 
 WAITC                 03B2   101    (1) 103 
 bootex                0372   58     (1) 36 
 staksav               00FB   21     (2) 32 117 
