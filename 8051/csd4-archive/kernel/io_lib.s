;;; The keypad-LCD interface library.

BASE equ 0e000h
LCD_REGISTER equ  80h
LCD_DATA     equ  81h
LCD_CONTROL  equ  90h
KEYPAD       equ 0a0h
Row     equ 32
Col     equ 33
OldRow  equ 34
OldCol  equ 35
Counter equ 36

key_init:
   clr PX1
   setb INT1
   setb IT1
ret

cgetc:
   clr IE1
   setb EX1
   mov R0, #SP_IE1
   acall Pause
   clr EX1
   mov DPTR, #(BASE + KEYPAD)
   movx A, @DPTR
   anl A, #0fh
ret

Strobe:
   mov DPTR, #(BASE + LCD_CONTROL)
   mov A, #1
   movx @DPTR, A
   mov A, #0
   movx @DPTR, A
ret

clear:
   mov OldRow, #0
   mov OldCol, #0
   mov Row, #0
   mov Col, #0
   mov A, #01
   acall WriteRegister
sjmp ClearDelay

home:
   mov Row, #0
   mov Col, #0
   mov A, #02
   acall WriteRegister
sjmp ClearDelay

ClearDelay:   ;;; A 1600 microsecond delay (+ 40 for WriteRegister)
   mov (Counter + 1), #10
   ClearLoop:
      mov Counter, #00
      djnz Counter, $
   djnz (Counter + 1), ClearLoop
ret

WriteRegister:
   mov DPTR, #(BASE + LCD_REGISTER)
   movx @DPTR, A
   acall Strobe
   mov Counter, #50    ;;; A 40 microsecond delay is needed.
   djnz Counter, $
ret

DISPLAY_ON   equ 4
CURSOR_ON    equ 2
CURSOR_BLINK equ 1
set_cursor:
   add A, #08
sjmp WriteRegister

cursor_left:
   dec Col
   mov A, #10h
sjmp WriteRegister

cursor_right:
   inc Col
   mov A, #14h
sjmp WriteRegister

shift_left:
   mov A, #18h
sjmp WriteRegister

shift_right:
   mov A, #1ch
sjmp WriteRegister

locate:
   mov A, #80h
   add A, Col
   jnb Row.0, XX0
      add A, #40h
   XX0:
   jnb Row.1, XX1
      add A, #14h
   XX1:
sjmp WriteRegister

restore_cursor:
   mov Row, OldRow
   mov Col, OldCol
sjmp locate

save_cursor:
   mov OldRow, Row
   mov OldCol, Col
ret

scr_init:
   mov A, #38h
   acall WriteRegister ;;; 1/16 duty, 5x7 font, 8 bit interface.
   mov A, #06h
   acall WriteRegister ;;; Cursor increments, no display shift.
   mov A, #DISPLAY_ON
   acall set_cursor
   acall clear
ret

cputc:
   inc Col
   mov DPTR, #(BASE + LCD_DATA)
   movx @DPTR, A
   acall Strobe
   mov Counter, #50    ;;; A 40 microsecond delay is needed.
   djnz Counter, $
ret

cputs:
   cputsLoop:
      clr A
      movc A, @A + DPTR
      inc DPTR
   jz cputsBreak
      push DPL
      push DPH
      acall cputc
      pop DPH
      pop DPL
   sjmp cputsLoop
cputsBreak:
ret
