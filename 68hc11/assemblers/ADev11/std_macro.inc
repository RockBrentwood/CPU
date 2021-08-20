	mac	lbeq
	bne	.+5
	jmp	{1}
	endm

	mac	lbne
	beq	.+5
	jmp	{1}
	endm

	mac	lblo
	bhs	.+5
	jmp	{1}
	endm
