CC=cc
CFLAGS=-c
LN=cc
RM=rm -f
O=.o
W=*

As6502: Assm0$O Assm1$O Assm2$O Assm3$O
	$(LN) Assm0$O Assm1$O Assm2$O Assm3$O -o As6502 -lc -lm
Assm0$O: Constants.h Extern.h Assm0.c
	$(CC) $(CFLAGS) Assm0.c
Assm1$O: Constants.h Extern.h Assm1.c
	$(CC) $(CFLAGS) Assm1.c
Assm2$O: Constants.h Extern.h Assm2.c
	$(CC) $(CFLAGS) Assm2.c
Assm3$O: Constants.h Extern.h Assm3.c
	$(CC) $(CFLAGS) Assm3.c

check: # Test/AssmTest.ex
##	./As6502 -nisv Test/AssmTest.in > Assm.ex	## The test output in AssmTest.ex goes with -nsv, not -nisv.
	./As6502 -nsv Test/AssmTest.in > Assm.ex
	diff Test/AssmTest.ex Assm.ex
# Test/AssmTest.ex: ./As6502
#	./As6502 -nsv Test/AssmTest.in > Test/AssmTest.ex
#	touch Test/AssmTest.ex

#manuals: As6502.man User.man
#As6502.man: Doc/As6502.1
#	man -d Doc/As6502.1 > As6502.man
#User.man: User.1
#	nroff Doc/User.1 > User.man

clean:
	$(RM) $W$O
	$(RM) Assm.ex
	$(RM) $W.man
clobber: clean
	$(RM) As6502
