
Amiga 6502 assembler :  -  A test program for as6502                                                                         PAGE 1
Line      Location     Label Opcode Operand  Comment   Fri Aug 13 22:06:01 2021

   3                   ;******************************************
   4                   ; Test file for the 6502 assembler - as6502
   5                   ; assemble as
   6                   ;     as6502 -nsv AssmTest.in
   7                   ; and compare output with AssmTest.ex
   8                   ;******************************************
   9                   ;                      ; comment treatment
  10                   ;******************************************
  11         0010      aa      =     $10      ; ';' immediately after the '0'
  12         0020      B       =     $20      space to comment subfield
  13         0030      C       =     $30      tab to comment subfield
  14         FFEE      DEFGHIjkl =   $FFEE
  15         FFEE      D       =DEFGHIjkl
  16                   ;******************************************
  17                   ; Number formats
  18                   ;******************************************
  19  0000   05                .byte %0101     ; binary number
  20  0001   12 12             .byte 022,@22   ; octal numbers - two forms
  21  0003   16                .byte 22        ; decimal number
  22  0004   22 FF FF          .byte $22,$ff,$FF  ; hex - upper/lower case
  23  0007   61 62             .byte 'a,'b     ; single ASCII characters
  24                   ;******************************************
  25                   ;                      ; ASCII character string
  26                   ;******************************************
  27  0009   61 62 63          .byte "abcd\t\n",0  ;   tab and new line escaped
      000C   64 09 0A  
      000F   00        
  28                   ;******************************************
  29                   ; Operation checks
  30                   ;******************************************
  31  0010   30 00             .word aa+B      ; addition
  32  0012   F0 FF             .word aa-B      ; subtraction
  33  0014   00 02             .word aa*B      ; multiplication
  34  0016   02 00             .word B/aa      ; division
  35  0018   10 00             .word C%B       ; modulo
  36  001A   10 00             .word B^C       ; exclusive OR
  37  001C   CF FF             .word ~C        ; one's complement
  38  001E   20 00             .word B&C       ; logical AND
  39  0020   30 00             .word aa|B      ; logical OR
  40  0022   EE 00             .word <D        ; low byte
  41  0024   FF 00             .word >D        ; high byte
  42  0026   26 00             .word *         ; current location
  43  0028   10 00             .word aa,B,C
      002A   20 00     
      002C   30 00     
  44  002E   00 08             .word B*[aa+C]  ; one level of parenthesis
  45  0030   FF EE             .dbyt D         ; high byte-low byte word
  46  0032   FF 00             .word D/256,D%256
      0034   EE 00     
  47                   ;******************************************
  48                   ; Addressing Mode Check
  49                   ;******************************************
  50         0100              *=$0100
  51  0100   A9 10             lda   =aa       ; immediate addressing
  52  0102   A9 10             lda   #aa       ; immediate addressing, alternate
  53  0104   AD EE FF          lda   D         ; direct addessing
  54  0107   A5 10             LDA   aa        ; page zero addressing, aa < 256
  55         0200      a1      =     512
  56         01F4      a2      =     500
  57  0109   A5 0C             lda   a1-a2     ; also page zero

Amiga 6502 assembler :  -  A test program for as6502                                                                         PAGE 2
Line      Location     Label Opcode Operand  Comment   Fri Aug 13 22:06:01 2021

  58  010B   0A                asl   A         ; accumulator addressing
  59  010C   0A                AsL   a         ; accumulator addressing also
  60  010D   00                brk            ; implied addressing
  61  010E   A1 10             lda   (aa,X)    ; indirect,X addressing
  62  0110   B1 10             lda   (aa),Y    ; indirect,Y addressing
  63  0112   B5 10             lda   aa,X      ; zero page,X addressing
  64  0114   BD EE FF          lda   D,X       ; absolute,X addressing
  65  0117   B9 EE FF          lda   D,Y       ; absolute,Y addressing
  66  011A   90 EE             bcc   *-$10     ; relative addressing
  67  011C   6C EE FF          jmp   (D)       ; indirect addressing
  68  011F   B6 10             ldx   aa,Y      ; zero page,Y addressing
  69  0121   B6 10             ldx   aa,y      ; alternate index name
  73                   ;******************************************
  74                   ; opcode check
  75                   ;******************************************
  76  0123   69 01             adc   =01
  77  0125   29 01             and   =01
  78  0127   0A                asl   A
  79  0128   90 00             bcc   *+2
  80  012A   B0 00             bcs   *+2
  81  012C   F0 00             beq   *+2
  82  012E   24 01             bit   $01
  83  0130   30 00             bmi   *+2
  84  0132   D0 00             bne   *+2
  85  0134   10 00             bpl   *+2
  86  0136   00                brk
  87  0137   50 00             bvc   *+2
  88  0139   70 00             bvs   *+2
  89  013B   18                clc
  90  013C   D8                cld
  91  013D   58                cli
  92  013E   B8                clv
  93  013F   C9 01             cmp   =01
  94  0141   E0 01             cpx   =01
  95  0143   C0 01             cpy   =01
  96  0145   C6 01             dec   $01
  97  0147   CA                dex
  98  0148   88                dey
  99  0149   49 01             eor   =01
 100  014B   E6 01             inc   $01
 101  014D   E8                inx
 102  014E   C8                iny
 103  014F   4C 52 01          jmp   *+3
 104  0152   20 55 01          jsr   *+3
 105  0155   A9 01             lda   =01
 106  0157   A2 01             ldx   =01
 107  0159   A0 01             ldy   =01
 108  015B   4A                lsr   A
 109  015C   EA                nop
 110  015D   09 01             ora   =01
 111  015F   48                pha
 112  0160   08                php
 113  0161   68                pla
 114  0162   28                plp
 115  0163   2A                rol   A
 116  0164   6A                ror   A
 117  0165   40                rti
 118  0166   60                rts
 119  0167   E9 01             sbc   =01
 120  0169   38                sec

Amiga 6502 assembler :  -  A test program for as6502                                                                         PAGE 3
Line      Location     Label Opcode Operand  Comment   Fri Aug 13 22:06:01 2021

 121  016A   F8                sed
 122  016B   78                sei
 123  016C   85 01             sta   $01
 124  016E   86 01             stx   $01
 125  0170   84 01             sty   $01
 126  0172   AA                tax
 127  0173   A8                tay
 128  0174   BA                tsx
 129  0175   8A                txa
 130  0176   9A                txs
 131  0177   98                tya

Amiga 6502 assembler :  Symbol Cross Reference -                                                                             PAGE 4
Symbol                Value Defined References

 B                     0020   12     (10) 31 32 33 34 35 36 38 39 43 44 
 C                     0030   13     (6) 35 36 37 38 43 44 
 DEFGHIjkl             FFEE   14     (1) 15 
 D                     FFEE   15     (9) 40 41 45 46 46 53 64 65 67 
 a1                    0200   55     (1) 57 
 a2                    01F4   56     (1) 57 
 aa                    0010   11     (15) 31 32 33 34 39 43 44 51 52 54 61 62 63 68 69 
