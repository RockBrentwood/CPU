include "kernel.lib"
include "io.lib"

PP0: DB "This is a test.", 0
LCDTest:
   acall cgetc
X0: cjne A, #0, X1
   acall cgetc
   mov R5, A
   acall cgetc
   xch A, R5
   swap A
   add A, R5
   acall cputc
ajmp LCDTest
X1: cjne A, #1, X2
   acall clear
ajmp LCDTest
X2: cjne A, #2, X3
   acall home
ajmp LCDTest
X3: cjne A, #3, X4
   acall cgetc
   anl A, #7
   acall set_cursor
ajmp LCDTest
X4: cjne A, #4, X5
   acall cursor_left
ajmp LCDTest
X5: cjne A, #5, X6
   acall cursor_right
ajmp LCDTest
X6: cjne A, #6, X7
   acall shift_left
ajmp LCDTest
X7: cjne A, #7, X8
   acall shift_right
ajmp LCDTest
X8: cjne A, #8, X9
   acall cgetc
   anl A, #3
   mov Row, A
   acall cgetc
   anl A, #31
   mov Col, A
   acall locate
ajmp LCDTest
X9: cjne A, #9, X10
   acall scr_init
ajmp LCDTest
X10: cjne A, #10, X11
   clr A
   mov DPtr, #PP0
   acall cputs
ajmp LCDTest
X11: cjne A, #11, X12
   acall save_cursor
ajmp LCDTest
X12: cjne A, #12, X13
   acall restore_cursor
ajmp LCDTest
X13:
ajmp LCDTest

main:
   acall key_init
   mov R0, #90h
   mov DPtr, #LCDTest
   acall Spawn
   setb EA
ret
end
