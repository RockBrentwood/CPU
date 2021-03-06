00000055 imm              00003456 label            00000102 port             
00000078 rd               00000034 rs               000000a5 jmpdst           
 0x55                   imm    equ    $55
 0x3456                 label    equ    $3456
 0x102                  port    equ    $102
 0x78                   rd    equ    $78
 0x34                   rs    equ    $34
0000 69 		    adc    b,a
0001 19 34 		    adc    rs,a
0003 39 34 		    adc    rs,b
0005 49 34 78 		    adc    rs,rd
0008 29 55 		    adc   #imm,a
000a 59 55 		    adc   #imm,b
000c 79 55 78 		    adc   #imm,rd
000f 68 		    add    b,a
0010 18 34 		    add    rs,a
0012 38 34 		    add    rs,b
0014 48 34 78 		    add    rs,rd
0017 28 55 		    add   #imm,a
0019 58 55 		    add   #imm,b
001b 78 55 78 		    add   #imm,rd
001e 63 		    and    b,a
001f 13 34 		    and    rs,a
0021 33 34 		    and    rs,b
0023 43 34 78 		    and    rs,rd
0026 23 55 		    and   #imm,a
0028 53 55 		    and   #imm,b
002a 73 55 78 		    and   #imm,rd
002d 83 02 		    andp    a,port
002f 93 02 		    andp    b,port
0031 a3 55 02 		    andp   #imm,port
0034 8c 34 56 		    br    label
0037 ac 34 56 		    br    label(b)
003a 9c 34 		    br    [rs]
003c 66 67 		    btjo    b,a,jmpdst
003e 16 34 64 		    btjo    rs,a,jmpdst
0041 36 34 61 		    btjo    rs,b,jmpdst
0044 46 34 78 5d 	    btjo    rs,rd,jmpdst
0048 26 55 5a 		    btjo   #imm,a,jmpdst
004b 56 55 57 		    btjo   #imm,b,jmpdst
004e 76 55 78 53 	    btjo   #imm,rd,jmpdst
0052 86 02 50 		    btjop    a,port,jmpdst
0055 96 02 4d 		    btjop    b,port,jmpdst
0058 a6 55 02 49 	    btjop   #imm,port,jmpdst
005c 67 47 		    btjz    b,a,jmpdst
005e 17 34 44 		    btjz    rs,a,jmpdst
0061 37 34 41 		    btjz    rs,b,jmpdst
0064 47 34 78 3d 	    btjz    rs,rd,jmpdst
0068 27 55 3a 		    btjz   #imm,a,jmpdst
006b 57 55 37 		    btjz   #imm,b,jmpdst
006e 77 55 78 33 	    btjz   #imm,rd,jmpdst
0072 87 02 30 		    btjzp    a,port,jmpdst
0075 97 02 2d 		    btjzp    b,port,jmpdst
0078 a7 55 02 29 	    btjzp   #imm,port,jmpdst
007c 8e 34 56 		    call    label
007f ae 34 56 		    call    label(b)
0082 9e 34 		    call    [rs]
0084 b5 		    clr    a
0085 c5 		    clr    b
0086 d5 78 		    clr    rd
0088 b0 		    clrc
0089 6d 		    cmp    b,a
008a 1d 34 		    cmp    rs,a
008c 3d 34 		    cmp    rs,b
008e 4d 34 78 		    cmp    rs,rd
0091 2d 55 		    cmp   #imm,a
0093 5d 55 		    cmp   #imm,b
0095 7d 55 78 		    cmp   #imm,rd
0098 8d 34 56 		    cmpa    label
009b ad 34 56 		    cmpa    label(b)
009e 9d 34 		    cmpa    [rs]
00a0 6e 		    dac    b,a
00a1 1e 34 		    dac    rs,a
00a3 3e 34 		    dac    rs,b
00a5 4e 34 78 		jmpdst    dac    rs,rd
00a8 2e 55 		    dac   #imm,a
00aa 5e 55 		    dac   #imm,b
00ac 7e 55 78 		    dac   #imm,rd
00af b2 		    dec    a
00b0 c2 		    dec    b
00b1 d2 78 		    dec    rd
00b3 bb 		    decd    a
00b4 cb 		    decd    b
00b5 db 78 		    decd    rd
00b7 06 		    dint
00b8 ba eb 		    djnz    a,jmpdst
00ba ca e9 		    djnz    b,jmpdst
00bc da 78 e6 		    djnz    rd,jmpdst
00bf 6f 		    dsb    b,a
00c0 1f 34 		    dsb    rs,a
00c2 3f 34 		    dsb    rs,b
00c4 4f 34 78 		    dsb    rs,rd
00c7 2f 55 		    dsb   #imm,a
00c9 5f 55 		    dsb   #imm,b
00cb 7f 55 78 		    dsb   #imm,rd
00ce 05 		    eint
00cf 01 		    idle
00d0 b3 		    inc    a
00d1 c3 		    inc    b
00d2 d3 78 		    inc    rd
00d4 b4 		    inv    a
00d5 c4 		    inv    b
00d6 d4 78 		    inv    rd
00d8 e3 cb 		    jc    jmpdst
00da e2 c9 		    jeq    jmpdst
00dc e5 c7 		    jge    jmpdst
00de e4 c5 		    jgt    jmpdst
00e0 e3 c3 		    jhs    jmpdst
00e2 e7 c1 		    jl    jmpdst
00e4 e1 bf 		    jlt    jmpdst
00e6 e0 bd 		    jmp    jmpdst
00e8 e1 bb 		    jn    jmpdst
00ea e7 b9 		    jnc    jmpdst
00ec e6 b7 		    jne    jmpdst
00ee e6 b5 		    jnz    jmpdst
00f0 e4 b3 		    jp    jmpdst
00f2 e5 b1 		    jpz    jmpdst
00f4 e2 af 		    jz    jmpdst
00f6 8a 34 56 		    lda    label
00f9 aa 34 56 		    lda    label(b)
00fc 9a 34 		    lda    [rs]
00fe 0d 		    ldsp
00ff c0 		    mov    a,b
0100 d0 78 		    mov    a,rd
0102 62 		    mov    b,a
0103 d1 78 		    mov    b,rd
0105 12 34 		    mov    rs,a
0107 32 34 		    mov    rs,b
0109 42 34 78 		    mov    rs,rd
010c 22 55 		    mov   #imm,a
010e 52 55 		    mov   #imm,b
0110 72 55 78 		    mov   #imm,rd
0113 98 34 78 		    movd    rs,rd
0116 a8 00 55 78 	    movd   #imm(b),rd
011a 88 00 55 78 	    movd   #imm,rd
011e 82 02 		    movp    a,port
0120 92 02 		    movp    b,port
0122 80 02 		    movp    port,a
0124 91 02 		    movp    port,b
0126 a2 55 02 		    movp   #imm,port
0129 6c 		    mpy    b,a
012a 1c 34 		    mpy    rs,a
012c 3c 34 		    mpy    rs,b
012e 4c 34 78 		    mpy    rs,rd
0131 2c 55 		    mpy   #imm,a
0133 5c 55 		    mpy   #imm,b
0135 7c 55 78 		    mpy   #imm,rd
0138 00 		    nop
0139 64 		    or    b,a
013a 14 34 		    or    rs,a
013c 34 34 		    or    rs,b
013e 44 34 78 		    or    rs,rd
0141 24 55 		    or   #imm,a
0143 54 55 		    or   #imm,b
0145 74 55 78 		    or   #imm,rd
0148 84 02 		    orp    a,port
014a 94 02 		    orp    b,port
014c a4 55 02 		    orp   #imm,port
014f b9 		    pop    a
0150 c9 		    pop    b
0151 d9 78 		    pop    rd
0153 08 		    pop    st
0154 b8 		    push    a
0155 c8 		    push    b
0156 d8 78 		    push    rd
0158 0e 		    push    st
0159 0b 		    reti
015a 0a 		    rets
015b be 		    rl    a
015c ce 		    rl    b
015d de 78 		    rl    rd
015f bf 		    rlc    a
0160 cf 		    rlc    b
0161 df 78 		    rlc    rd
0163 bc 		    rr    a
0164 cc 		    rr    b
0165 dc 78 		    rr    rd
0167 bd 		    rrc    a
0168 cd 		    rrc    b
0169 dd 78 		    rrc    rd
016b 6b 		    sbb    b,a
016c 1b 34 		    sbb    rs,a
016e 3b 34 		    sbb    rs,b
0170 4b 34 78 		    sbb    rs,rd
0173 2b 55 		    sbb   #imm,a
0175 5b 55 		    sbb   #imm,b
0177 7b 55 78 		    sbb   #imm,rd
017a 07 		    setc
017b 8b 34 56 		    sta    label
017e ab 34 56 		    sta    label(b)
0181 9b 34 		    sta    [rs]
0183 09 		    stsp
0184 6a 		    sub    b,a
0185 1a 34 		    sub    rs,a
0187 3a 34 		    sub    rs,b
0189 4a 34 78 		    sub    rs,rd
018c 2a 55 		    sub   #imm,a
018e 5a 55 		    sub   #imm,b
0190 7a 55 78 		    sub   #imm,rd
0193 b7 		    swap    a
0194 c7 		    swap    b
0195 d7 78 		    swap    rd
0197 ff 		    trap    00
0198 fe 		    trap    01
0199 fd 		    trap    02
019a fc 		    trap    03
019b fb 		    trap    04
019c fa 		    trap    05
019d f9 		    trap    06
019e f8 		    trap    07
019f f7 		    trap    08
01a0 f6 		    trap    09
01a1 f5 		    trap    10
01a2 f4 		    trap    11
01a3 f3 		    trap    12
01a4 f2 		    trap    13
01a5 f1 		    trap    14
01a6 f0 		    trap    15
01a7 ef 		    trap    16
01a8 ee 		    trap    17
01a9 ed 		    trap    18
01aa ec 		    trap    19
01ab eb 		    trap    20
01ac ea 		    trap    21
01ad e9 		    trap    22
01ae e8 		    trap    23
01af b0 		    tsta
01b0 c1 		    tstb
01b1 b6 		    xchb    a
01b2 c6 		    xchb    b
01b3 d6 78 		    xchb    rd
01b5 65 		    xor    b,a
01b6 15 34 		    xor    rs,a
01b8 35 34 		    xor    rs,b
01ba 45 34 78 		    xor    rs,rd
01bd 25 55 		    xor   #imm,a
01bf 55 55 		    xor   #imm,b
01c1 75 55 78 		    xor   #imm,rd
01c4 85 02 		    xorp    a,port
01c6 95 02 		    xorp    b,port
01c8 a5 55 02 		    xorp   #imm,port
