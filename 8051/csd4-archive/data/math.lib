;;; ARITHMETIC: Integer arithmetic routines.
ClearX:
   ClearLoop:
      mov @R0, #0
      inc R0
   djnz R2, ClearLoop
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

CopyX:
   CopyLoop:
      mov A, @R1
      inc R1
      mov @R0, A
      inc R0
   djnz R2, CopyLoop
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

SizeX equ 14h ;;; 1 byte
SizeY equ 15h ;;; 1 byte
ArgX  equ 16h ;;; 1 byte
ArgY  equ 17h ;;; 1 byte
ArgZ  equ 18h ;;; 1 byte
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
