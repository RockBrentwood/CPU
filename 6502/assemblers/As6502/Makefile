CC=cc
CFLAGS=-c
LN=cc
O=.o

As6502: Assm0$O Assm1$O Assm2$O Assm3$O
		$(LN) Assm0$O Assm1$O Assm2$O Assm3$O -o As6502 -lc -lm
Assm0$O: Assm1.h Assm2.h Assm0.c
		$(CC) $(CFLAGS) Assm0.c
Assm1$O: Assm1.h Assm2.h Assm1.c
		$(CC) $(CFLAGS) Assm1.c
Assm2$O: Assm1.h Assm2.h Assm2.c
		$(CC) $(CFLAGS) Assm2.c
Assm3$O: Assm1.h Assm2.h Assm3.c
		$(CC) $(CFLAGS) Assm3.c

check: AssmTest.ex
AssmTest.ex: As6502
##		./As6502 -nisv AssmTest.in > Assm.ex	## The test output in AssmTest.ex goes with -nsv, not -nisv.
		./As6502 -nsv AssmTest.in > Assm.ex
		touch AssmTest.ex
		diff AssmTest.ex Assm.ex

manuals: As6502.man User.man

As6502.man: As6502.1
		man -d As6502.1 > As6502.man
User.man: User.1
		nroff User.1 > User.man
clean:
		rm Assm0.o Assm1.o Assm2.o Assm3.o Assm.ex As6502.man User.man
