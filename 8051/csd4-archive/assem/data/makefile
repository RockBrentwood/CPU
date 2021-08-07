all: data.hex
data.hex:	data.o kernel.o math.o stdio.o
	cas -o data.hex math.o data.o kernel.o stdio.o
data.o:	8051fa.h kernel.h math.h stdio.h data.s
	cas -c data.s
kernel.o:	8051fa.h kernel.h data.h kernel.s
	cas -c kernel.s
math.o:	8051fa.h math.s
	cas -c math.s
stdio.o:	8051fa.h kernel.h stdio.s
	cas -c stdio.s
