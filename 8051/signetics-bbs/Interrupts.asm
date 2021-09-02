; This program activates the most commonly used interrupts
; of the MCS-51 controller.  These are:
;       serial interrupt
;       timer 0 interrupt
;       timer 1 interrupt
;       external interrupt 0
;       external interrupt 1

; This program services the above interrupts.

; the serial routine sends a byte out and receives it back.
;**********************************************************
;*      the user must jumper P3.0 and P3.1 together       *
;**********************************************************
; the stack is used in the serial interrupt to a depth of 5.

$debug
$mod51

ea_byte		equ	09fh	; all -51 interrupts enabled
tmod_val        equ     11h     ; timer0 in 16-bit mode
                                ; timer1 in 16-bit mode
t0_val          equ     00h     ; timer0 reload, LSB
t1_val          equ     80h     ; timer1 reload, LSB
t_val           equ     0ffh    ; timer MSB

end_it		equ	0ffh	; inc a up till this value
delay		equ	0ffh	; base for delay time
                                ; w/12 MHz clock, routine works
                                ; at delay=14h, not 0ah
scon_val	equ	098h	; 9 bit uart, ninth bit = 1
				; rcv enabled
pcon_val	equ	000h	; $80, 1/32*(clock), 19.2 kbaud
				; $00, 1/64*(clock), 9600 baud
stk_ptr         equ     31h     ; stack begins immediately
                                ; after 2 data bytes at 30h,31h
low_val         equ     01h     ; low byte of timer 0 reload
hi_val          equ     58h     ; hi byte of timer 0 reload

	dseg
        org     30h             ; scratchpad RAM
wait:		ds	1
rcv_data:	ds	1
;
	cseg
	org	0h
initz:	jmp	start

	org	3h
int_0:	jnb	p3.2,$		; wait till int0 goes high
        inc     r0              ; use r0 to count # of int0's
	reti

	org	0bh
t0_int:	clr     tr0              ; stop timer0
        jmp     t0_work

	org	13h
int_1:	jnb	p3.3,$		; wait till int1 goes high
        inc     r1              ; use r1 to count # of int0's
	reti

        org     1bh
t1_int: clr     tr1             ; stop timer1
        jmp     t1_work

        org     23h
s_int:  ljmp    ser_int

        org     60h
start:	mov     sp,#stk_ptr     ; set the stack pointer
        mov     scon,#scon_val
        mov     pcon,#pcon_val
        mov	ie,#ea_byte

	mov	tmod,#tmod_val
        mov     tl0,#t0_val
        mov     th0,#t_val
        mov     tl1,#t1_val
        mov     th1,#t_val

        mov     r0,#00h
        mov     r1,#00h

        setb    p3.0            ; alternate func. is RXD
        setb    p3.1            ; alternate func. is TXD

        mov     dpl,#00h
        clr     ti
        clr     ri

        setb    it0             ; int0 is edge sensitive
        setb    it1             ; int1 is edge sensitive

        setb    tr0             ; start timer0
        setb    tr1             ; start timer1

xmit:	mov	sbuf,dpl	; dpl inits to 0
set_wait:
        mov	wait,#delay     ; dead time: adjust this 
                                ; to tighten noose
;
loop2:	nop
	djnz	wait,loop2
        jmp     xmit
;
ser_int:
        push    acc
        push    dpl
        push    dph
        push    wait
        push    rcv_data
;
        mov     a,dpl                   ; manipulate data by acc
	mov	rcv_data,sbuf		; read the rcv'd byte
	cjne	a,rcv_data,bombout	; if different, bomb
	clr	ri			; clear the rcv interrupt
        clr     ti                      ; clear xmt interrupt too
	inc	a			; then inc a

; exit condition is commented out for this version
;	cjne	a,end_it,popm		; and compare to $ff
					; go to top if not done
;	jmp	the_end			; go to the end if done

popm:   pop     rcv_data
        pop     wait
        pop     dph
        pop     dpl
        mov     dpl,a                   ; xmit byte will inc'd
        pop     acc

        reti

bombout:	jmp	$		; screwed up

the_end:	jmp	$		; ok

t0_work:        mov     tl0,#t0_val
                mov     th0,#t_val
                setb    tr0             ; start timer0 again
                reti

t1_work:        mov     tl1,#t1_val
                mov     th1,#t_val
                setb    tr1             ; start timer1 again
                reti
	end

��������������������������������������������������������������������������
