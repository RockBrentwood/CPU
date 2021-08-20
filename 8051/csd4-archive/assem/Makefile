CC=gcc
.c.o:	ds cas das reloc semi nl
	$(CC) -c -g $*.c
DASOBJS=das.o
DSOBJS=ds.o
CASOBJS=cas.o link.o st.o res.o op.o ex.o io.o
RELOCOBJS=reloc.o
SEMIOBJS=semi.o
NLOBJS=nl.o
all:	ds cas das reloc semi nl
das:	$(DASOBJS)
	$(CC) -o das $(DASOBJS)
ds:	$(DSOBJS)
	$(CC) -o ds $(DSOBJS)
cas:	$(CASOBJS)
	$(CC) -o cas $(CASOBJS)
reloc:	$(RELOCOBJS)
	$(CC) -o reloc $(RELOCOBJS)
semi:	$(SEMIOBJS)
	$(CC) -o semi $(SEMIOBJS)
nl:	$(NLOBJS)
	$(CC) -o nl $(NLOBJS)
cas.o:	io.h op.h st.h link.h
link.o:	io.h ex.h res.h st.h
st.o:	io.h ex.h op.h res.h
res.o:	io.h ex.h st.h res.h
op.o:	io.h ex.h st.h res.h op.h
ex.o:	io.h ex.h st.h res.h
io.o:	io.h op.h
