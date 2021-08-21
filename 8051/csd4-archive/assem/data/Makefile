all: data.hex
data.hex:	data.o kernel.o math.o stdio.o
	cas -o data.hex math.o data.o kernel.o stdio.o
data.o:	8051fa.h kernel.h math.h stdio.h data.asm
	cas -c data.asm
kernel.o:	8051fa.h kernel.h data.h kernel.asm
	cas -c kernel.asm
math.o:	8051fa.h math.asm
	cas -c math.asm
stdio.o:	8051fa.h kernel.h stdio.asm
	cas -c stdio.asm
