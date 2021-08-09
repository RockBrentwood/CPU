## Frankenstein Cross-Assemblers, version 2.0.
## Reconfigurable Cross-assembler producing Intel (TM) Hex format object records.
## Keywords: Cross-assemblers, 1805, 2650, 6301, 6502, 6805, 6809, 6811, tms7000, 8048, 8051, 8096, Z8, Z80.
##
## This software is in the public domain.
## Any prior copyright claims are relinquished.
##
## This software is distributed with no warranty whatever.
## The author takes no responsibility for the consequences of its use.
##
## Original author: Mark Zenier.
##
## Usage:
##	make CPU=cpu	compile the assembler for the given cpu target.
##	make clean	clean out for the release distribution.
##	make newmachine	clean out CPU-dependent generated files only.
## Supported values for CPU:
##	1805, 2650, 6502, 6805, 6809, 6811, 7000, 8048, 8051, 8096, Z80, Z8
## Example:
##	make CPU=6502
## Conditional Compilation Flags:
##	DOSTMP		use the current directory for temporary intermediate file.
##	NOGETOPT	use the getopt.h file

## Utilities:
RM=rm -f
#RM=del
#RM=delete QUIET
CP=cp
#CP=copy
MV=mv
#MV=rename
DIFF=diff
#DIFF=fc
PRINTSPOOL=lpr

## Suffixes:
O=.o
#O=.obj
W=*
#W="\#?"

## Compilers and options:
## "yacc" is linked to byacc on many systems, which is preferred here to bison.
YACC=yacc
#YACC=bison

## For Linux, Amiga and DOS cross-compiled on Xenix and DOS hosts:
YACCLEXLIB =
## For SunOS 4.1, MicroPort, Xenix and V7:
#YACCLEXLIB = -ly

LEXNEEDS =
#LEXNEEDS=-DDEBUG=1

## For Linux:
CC=gcc
## The host's native C99 compiler on POSIX-based systems.
#CC=cc
## For DOS hosts:
#CC=tcc

## For Linux:
CFLAGS = -g -std=gnu99
## For Amiga:
#CFLAGS = -O2 -Wno-unused #-m68030 -m68881
## For SunOS 4.1 and V7 and DOS hosts (V7's native compiler, "pcc", now supports C99):
CFLAGS =
## For MicroPort and Xenix:
#CFLAGS = -Ml -g
## For DOS cross-compiled on Xenix:
#CFLAGS = -dos -Ml

## Configuration:
## For Linux, Amiga, SunOS 4.1, MicroPort and Xenix:
MAINNEEDS =
## For V7:
#MAINNEEDS = -DNOGETOPT
## For DOS cross-compiled on Xenix and DOS hosts:
#MAINNEEDS = -DNOGETOPT -DDOSTMP

## For Linux, Amiga, SunOS 4.1, MicroPort and Xenix:
MAINDEPENDS =
## For V7 and DOS cross-compiled on Xenix and DOS hosts:
#MAINDEPENDS = getopt.h

all: as$(CPU)
clean:
	$(RM) $W$O
	$(RM) fraytok.h
	$(RM) y.tab.[ch]
	$(RM) as$W.c
	$(RM) as$W.h
newmachine:
	$(RM) fryylex$O
	$(RM) as$W$O
	$(RM) fraytok.h
	$(RM) as$W.c
	$(RM) as$W.h
#test: # as$(CPU)
#	./as$(CPU) -l as.ex as$(CPU).tst
#	$(DIFF) as.ex as$(CPU).tut

as$(CPU): frasmain$O frapsub$O fryylex$O as$(CPU)$O fraosub$O
	$(CC) $(CFLAGS) -o as$(CPU) frasmain$O frapsub$O fraosub$O fryylex$O as$(CPU)$O $(YACCLEXLIB)
	$(RM) fraytok.h
frasmain$O: frasmain.c frasmdat.h $(MAINDEPENDS)
	$(CC) $(CFLAGS) $(MAINNEEDS) -c frasmain.c
fraytok.h: as$(CPU).h
	$(CP) as$(CPU).h fraytok.h
fryylex$O: fryylex.c frasmdat.h fraytok.h
	$(CC) $(CFLAGS) $(LEXNEEDS) -c fryylex.c
as$(CPU).c as$(CPU).h: as$(CPU).y
	$(YACC) -d as$(CPU).y
	$(MV) y.tab.c as$(CPU).c
	$(MV) y.tab.h as$(CPU).h
	$(RM) fraytok.h
as$(CPU)$O: as$(CPU).c frasmdat.h fragcon.h
	$(CC) $(CFLAGS) -c as$(CPU).c
frapsub$O: frapsub.c fragcon.h frasmdat.h fraeuni.h fraebin.h
	$(CC) $(CFLAGS) -c frapsub.c
fraosub$O: fraosub.c frasmdat.h fragcon.h fraeuni.h fraebin.h
	$(CC) $(CFLAGS) -c fraosub.c
