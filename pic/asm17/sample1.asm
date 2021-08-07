	TITLE   "ASM17 Sample Code"
	STITLE  "Sample 1 - 3ms Pulse on I/O Line C5"
	LIST    COLUMNS=132, NOWRAP

;
; Generate a 3ms pulse on I/O line C5 (Bank 1, F0x11, bit 5)
; at 16mhz.
;

Start
	movlw           0200            ; Load decimal 200 into w
	movwf           0x18            ; Transfer 200 to F18
	movlb           1               ; Select Bank 1
	bsf             0x11,5          ; Set output file 7, bit 5 high
loop    decfsz          0x18,1          ; Decrement F11, skip if zero
	goto            loop            ; This goto will cause F11 to be
					; decremented 200 times.  The
					; decrement executes in 4us while
					; the goto takes 8us, therefore
					; the loop executes in
					; 12us * 250 = 3ms
	BCF             0x11,5          ; Reset output file 7, bit 5 high
	END
