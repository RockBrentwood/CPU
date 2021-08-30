;;; ARITHMETIC: Integer arithmetic routines.
global ClearX:
   1:
      mov @R0, #0; inc R0
   djnz R2, 1b
ret

global IncX:
   setb C
global CarryX:
   1:
      mov A, @R0
      addc A, #0
      mov @R0, A; inc R0
   djnz R2, 1b
ret

global CopyX:
   1:
      mov A, @R1; inc R1
      mov @R0, A; inc R0
   djnz R2, 1b
ret

global SubtractX:
   clr C
   1:
      mov A, @R0
      subb A, @R1; inc R1
      mov @R0, A; inc R0
   djnz R2, 1b
ret

seg data
global SizeX: ds 1
global SizeY: ds 1
global ArgX:  ds 1
global ArgY:  ds 1
global ArgZ:  ds 1

seg code
global MultiplyX: ;;; The new multiplication algorithm R5 is the carry.
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
   jz 1f
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
   jnc 1f
      mov A, #1
   djnz R3, CarryBy1
   inc R5
   1:
djnz SizeX, MultiplyLoop
ret
