CC=cc
CFLAGS=-c
LN=cc

as6502: assm0.o assm1.o assm2.o assm3.o
		$(LN) assm0.o assm1.o assm2.o assm3.o -o as6502 -lc -lm
assm0.o: assm.d1 assm.d2 assm0.c
		$(CC) $(CFLAGS) assm0.c
assm1.o: assm.d1 assm.d2 assm1.c
		$(CC) $(CFLAGS) assm1.c
assm2.o: assm.d1 assm.d2 assm2.c
		$(CC) $(CFLAGS) assm2.c
assm3.o: assm.d1 assm.d2 assm3.c
		$(CC) $(CFLAGS) assm3.c

check: assmtest.l
assmtest.l: as6502
##		as6502 -nisv assmtest > temp	## The test output in assmtest.l goes with -nsv, not -nisv.
		./as6502 -nsv assmtest > temp
		touch assmtest.l
		diff assmtest.l temp

manuals: as6502.l user.l

as6502.l: as6502.1
		man -d as6502.1 > as6502.l
user.l: user.nr
		nroff user.nr > user.l
clean:
		rm assm0.o assm1.o assm2.o assm3.o temp as6502.l user.l
