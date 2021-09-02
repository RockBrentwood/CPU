Quo     equ 60
Divisor equ 64
Op      equ 68
Rem     equ 72
Carry   equ 72
Fract   equ 76
Aux     equ 80

Div1:
   clr C
   mov R0, #Op
   mov R2, #4
   acall ShiftX          ;;; C:Op = Op*2;
   mov R0, #Rem
   mov R2, #4
   acall ShiftX          ;;; Rem = Rem*2 + C;
   mov R0, #Rem
   mov R1, #Divisor
   mov R2, #4
   acall LessX
   jc Quot               ;;; if (Rem >= Divisor) {
      mov R0, #Rem
      mov R1, #Divisor
      mov R2, #4
      acall SubtractX    ;;;    Rem -= Divisor; C = 0;
      clr C
   Quot:                 ;;; } else C = 1;
ret

Div32:
   mov R0, #Rem
   mov R2, #4
   acall ClearX          ;;; Rem = 0;
   mov R0, #Quo
   mov R2, #4
   acall ClearX          ;;; Quo = 0;
   mov R3, #32
   DivLoop:              ;;; for (R3 = 32; R3 > 0; R3--) {
      acall Div1
      cpl C
      mov R0, #Quo
      mov R2, #4
      acall ShiftX          ;;; Quo = 2*Quo + (1 - C);
   djnz R3, DivLoop      ;;; }
   mov R0, #Fract
   mov R2, #4
   acall ClearX          ;;; Fract = 0;
   mov R3, #32
   FractLoop:            ;;; for (R3 = 32; R3 > 0; R3--) {
      acall Div1
      cpl C
      mov R0, #Fract
      mov R2, #4
      acall ShiftX          ;;; Fract = 2*Fract + (1 - C);
   djnz R3, FractLoop    ;;; }
ret

MulBy10: ;;; Aux:Fract = 10*Fract;
   mov Aux, #0
   mov R0, #Fract
   mov R1, #Carry
   mov R2, #4
   MulLoop:
      mov A, @R0
      mov B, #10
      mul AB
      mov @R0, A
      inc R0
      mov @R1, B
      inc R1
   djnz R2, MulLoop
   mov R0, #(Fract + 1)
   mov R1, #Carry
   mov R2, #4
   acall AddX
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

ShiftX:
   ShiftLoop:
      mov A, @R0
      rlc A
      mov @R0, A
      inc R0
   djnz R2, ShiftLoop
ret

CplX:
   CplLoop:
      mov A, @R0
      cpl A
      mov @R0, A
      inc R0
   djnz R2, CplLoop
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
