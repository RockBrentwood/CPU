## For DOS:
#X=.exe
#O=.obj
#RM=del
#CC=qcl
#CFLAGS=-g

## For Linux:
X=
O=.o
RM=rm -f
CC=gcc
CFLAGS=-g

BINDIR=/usr/local/bin

EXES=ds$X cas$X dac$X das$X reloc$X semi$X nl$X
.c$O:
	$(CC) $(CFLAGS) -c $*.c
DACOBJS=dac$O
DASOBJS=das$O
DSOBJS=ds$O
CASOBJS=cas$O link$O st$O res$O op$O ex$O io$O
RELOCOBJS=reloc$O
SEMIOBJS=semi$O
NLOBJS=nl$O
all:	ds$X cas$X dac$X das$X reloc$X semi$X nl$X
dac$X:	$(DACOBJS)
	$(CC) -o dac$X $(DACOBJS)
das$X:	$(DASOBJS)
	$(CC) -o das$X $(DASOBJS)
ds$X:	$(DSOBJS)
	$(CC) -o ds$X $(DSOBJS)
cas$X:	$(CASOBJS)
	$(CC) -o cas$X $(CASOBJS)
reloc$X:	$(RELOCOBJS)
	$(CC) -o reloc$X $(RELOCOBJS)
semi$X:	$(SEMIOBJS)
	$(CC) -o semi$X $(SEMIOBJS)
nl$X:	$(NLOBJS)
	$(CC) -o nl$X $(NLOBJS)
cas$O:	io.h op.h st.h link.h
link$O:	io.h ex.h res.h st.h
st$O:	io.h ex.h op.h res.h
res$O:	io.h ex.h st.h res.h
op$O:	io.h ex.h st.h res.h op.h
ex$O:	io.h ex.h st.h res.h
io$O:	io.h op.h
test:
	@sh test.sh >& Log
	@echo "=== Current Test versus Previous Tests (Differences): ==="
	@diff Log LogEx
	@rm Log
install:
	cp $(EXES) $(BINDIR)
clean:
	$(RM) *$O
clobber: clean
	$(RM) $(EXES)
