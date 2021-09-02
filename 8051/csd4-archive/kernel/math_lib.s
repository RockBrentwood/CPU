;;; ARITHMETIC: Integer arithmetic routines.
ClearX:
   ClearLoop:
      mov @R0, #0
      inc R0
   djnz R2, ClearLoop
ret

CopyX:
   CopyLoop:
      mov A, @R1
      inc R1
      mov @R0, A
      inc R0
   djnz R2, CopyLoop
ret

CplX:
   CplLoop:
      mov A, @R0
      cpl A
      mov @R0, A
      inc R0
   djnz R2, CplLoop
ret

RolX:
   clr C
ShiftX:
   ShiftLoop:
      mov A, @R0
      rlc A
      mov @R0, A
      inc R0 
   djnz R2, ShiftLoop
ret

LessX:
   clr C
   LessLoop:
      mov A, @R0
      inc R0
      subb A, @R1
      inc R1
   djnz R2, LessLoop
ret

ZeroX:
   clr C
   ZeroLoop:
      mov A, @R0
      jnz NotZero
      inc R0
   djnz R2, ZeroLoop
   setb C
NotZero:
ret

IncX:
   setb C
CarryX:
   CarryLoop:
      mov A, @R0 
      addc A, #0
      mov @R0, A
      inc R0
   djnz R2, CarryLoop
ret

DecX:
   setb C
BorrowX:
   BorrowLoop:
      mov A, @R0 
      subb A, #0
      mov @R0, A
      inc R0
   djnz R2, BorrowLoop
ret

AddX:
   clr C
   AddLoop:
      mov A, @R0
      addc A, @R1
      inc R1
      mov @R0, A
      inc R0
   djnz R2, AddLoop
ret

SubbX:
   setb C
sjmp SubtractLoop
SubtractX:
   clr C
   SubtractLoop:
      mov A, @R0
      subb A, @R1
      inc R1
      mov @R0, A
      inc R0
   djnz R2, SubtractLoop
ret

NegX:
   setb C
NegLoop:
   mov A, @R0
   cpl A
   addc A, #0
   mov @R0, A
   inc R0
djnz R2, NegLoop
ret

Inc10X:
   Inc10Loop:
      inc @R0
      mov A, @R0
   cjne A, #10, Inc10Break
      mov @R0, #0
      inc R0
   djnz R2, Inc10Loop
   Inc10Break:
ret

;;; Multiplication Routines
SizeX equ 30h ;;; 1 byte
SizeY equ 31h ;;; 1 byte
ArgX  equ 32h ;;; 1 byte
ArgY  equ 33h ;;; 1 byte
ArgZ  equ 34h ;;; 1 byte
MultiplyX: ;;; The new multiplication algorithm R5 is the carry.
   mov R5, #0
MultiplyLoop:
   mov R0, ArgX
   mov A, @R0
   mov R2, A
   inc ArgX      ;;; R2 = *ArgX++;
   mov R0, ArgZ
   inc ArgZ      ;;; R0 = ArgZ++;
   mov R1, ArgY
   mov R3, SizeY
   mov R4, #0
   jz BreakCarry
   MulNibble:
      mov A, R2
      mov B, @R1
      inc R1
      mul AB     ;;; B:A = *R1++ * R2;
      add A, R4
      xch A, B
      addc A, #0
      xch A, B   ;;; B:A += R4;
      add A, @R0
      mov @R0, A
      inc R0
      mov A, B
      addc A, #0
      mov R4, A  ;;; R4:*R0++ = B:A + *R0;
   djnz R3, MulNibble
   mov R3, SizeX
   mov A, R4
   CarryBy1:
      add A, @R0
      mov @R0, A
      inc R0     ;;; C:*R0++ += *R0 + A;
   jnc BreakCarry
      mov A, #1
   djnz R3, CarryBy1
   inc R5
BreakCarry:
djnz SizeX, MultiplyLoop
ret

MulByR1: ;;; R3:(@R0) = R1*(@R0);
   mov R3, #0
   MulLoop:
      mov A, @R0
      mov B, R1
      mul AB
      add A, R3
      mov @R0, A
      inc R0
      mov A, B
      addc A, #0
      mov R3, A
   djnz R2, MulLoop
ret

Op      equ 35h ;;; 8 bytes
Divisor equ 3dh ;;; 8 bytes
Quo     equ 45h ;;; 8 bytes
Fract   equ 4dh ;;; 8 bytes
Rem     equ 55h ;;; 8 bytes
Div1:
   clr C
   mov R0, #Op
   mov R2, SizeX
   acall ShiftX          ;;; C:Op = Op*2;
   mov R0, #Rem
   mov R2, SizeX
   acall ShiftX          ;;; Rem = Rem*2 + C;
   mov R0, #Rem
   mov R1, #Divisor
   mov R2, SizeX
   acall LessX
   jc Quot               ;;; if (Rem >= Divisor) {
      mov R0, #Rem
      mov R1, #Divisor
      mov R2, SizeX
      acall SubtractX    ;;;    Rem -= Divisor;
   Quot:                 ;;; }
   cpl C                 ;;; C = !C;
ret

DivideX:
   mov R0, #Rem
   mov R2, SizeX
   acall ClearX          ;;; Rem = 0;
   mov R0, #Quo
   mov R2, SizeX
   acall ClearX          ;;; Quo = 0;
   mov A, SizeX
   rl A
   rl A
   rl A
   mov R3, A
   DivLoop:              ;;; for (R3 = 8*SizeX; R3 > 0; R3--) {
      acall Div1            ;;; Rem:Op <<= 1; C = Rem/Divisor; Rem %= Divisor;
      mov R0, #Quo
      mov R2, SizeX
      acall ShiftX          ;;; Quo = Quo << 1 | C;
   djnz R3, DivLoop      ;;; }
   mov R0, #Fract
   mov R2, SizeX
   acall ClearX          ;;; Fract = 0;
   mov A, SizeX
   rl A
   rl A
   rl A
   mov R3, A
   FractLoop:            ;;; for (R3 = 8*SizeX; R3 > 0; R3--) {
      acall Div1            ;;; Rem:Op <<= 1; C = Rem/Divisor; Rem %= Divisor;
      mov R0, #Fract
      mov R2, SizeX
      acall ShiftX          ;;; Fract = Fract << 1 | C;
   djnz R3, FractLoop    ;;; }
ret

;;; Square root of 64-bit word W returned in 32-bit word Q.
Q equ 35h ;;; 8 bytes
D equ 3dh ;;; 8 bytes
X equ 45h ;;; 8 bytes
W equ 4dh ;;; 8 bytes
Y bit F0 ;;; 1 bit
Sqrt64:
   mov R0, #Q
   mov R2, #4
   acall ClearX      ;;; Q = 0;
   mov R0, #D
   mov R2, #8
   acall ClearX      ;;; D = 0;
   mov R0, #X
   mov R2, #8
   acall ClearX      ;;; X = 0;
   mov R3, #32       ;;; for (I = 0; I < 32; I++) {
   SqrtLoop:
      mov R0, #W
      mov R2, #8
      acall RolX
      mov R0, #D
      mov R2, #8
      acall ShiftX
      mov R0, #W
      mov R2, #8
      acall RolX
      mov R0, #D
      mov R2, #8
      acall ShiftX   ;;;    D <<= 2, D += (W&0xc0000000) >> 30, W <<= 2;
      mov R0, #X
      mov R1, #D
      mov R2, #8
      acall LessX
      mov Y, C       ;;;    Y = (X < D);
      mov R0, #Q
      mov R2, #4
      acall ShiftX   ;;;    Q <<= 1; Q |= Y;
      jnb Y, YY0     ;;;    if (Y) {
         mov R0, #D
         mov R1, #X
         mov R2, #8
         acall SubbX ;;;       D -= (X + 1);
      YY0:           ;;;    }
      mov R0, #X
      mov R2, #8
      acall RolX     ;;;    X <<= 1;
      jnb Y, YY1     ;;;    if (Y) {
         orl X, #4   ;;;       X |= 0x04;
      YY1:           ;;;    }
   djnz R3, SqrtLoop ;;; }
ret
