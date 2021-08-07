/* The 8255 interrupt controller registers */
#define IntControl 0x20
#define IntMask    0x21

/* IntControl bits */
#define EOI 0x20

/* IntMask bits */
#define IRQ0 0x01
#define IRQ1 0x02
#define IRQ2 0x04
#define IRQ3 0x08
#define IRQ4 0x10
#define IRQ5 0x20
#define IRQ6 0x40
#define IRQ7 0x80

/* The 8250 UART registers */
#define FREQ 115200

/* COMx: ports */
#define COM1 0
#define COM2 1
#define COM3 2
#define COM4 3

/* 8250 UART registers. */
#define DLL 0
#define DLH 1
#define RxD 0
#define TxD 0
#define IER 1
#define IIR 2
#define LCR 3
#define MCR 4
#define LSR 5
#define MSR 6

/* IER */
#define RxD_INT 0x1
#define TxD_INT 0x2
#define LSR_INT 0x4
#define MSR_INT 0x8

/* IIR */
#define INT_OFF 0x01
#define INT_ID  0x06
#define LSR_ID  0x06 /* Overrun, Parity, Frame, Break: Read LSR */
#define RxD_ID  0x04 /* Data received: Read RxD */
#define TxD_ID  0x02 /* Data transmitted: Write to TxD */
#define MSR_ID  0x00 /* CTS, DSR, RI, received line signal detect: Read MSR */

/* MCR */
#define DTR  0x01
#define RTS  0x02
#define OUT1 0x04
#define OUT2 0x08 /* Used to enable interrupts */
#define LOOP_BACK 0x10 /* Set for testing purposes to tie TxD to RxD. */

/* LCR */
#define DLAB 0x80
#define PAR  0x38
#define DATA 0x03
#define STOP 0x04

#define PAR_NONE 0
#define PAR_ODD  0x08
#define PAR_EVEN 0x18
#define PAR_HIGH 0x28
#define PAR_LOW  0x38

#define STOP1 0
#define STOP2 4

#define DATA5 0
#define DATA6 1
#define DATA7 2
#define DATA8 3

/* LSR */
#define RX_READY     0x01
#define OVERRUN_ERR  0x02
#define PARITY_ERR   0x04
#define FRAMING_ERR  0x08
#define BREAK_RECV   0x10
#define TX_READY     0x20
#define TX_SHIFT     0x40
#define TX_DONE (TX_READY | TX_SHIFT)
#define LINE_ERR (OVERRUN_ERR | PARITY_ERR | FRAMING_ERR)

/* MSR */
#define DELTA_CTS 0x01
#define DELTA_DSR 0x02
#define DELTA_RI  0x04
#define DELTA_CD  0x08
#define _CTS 0x10
#define _DSR 0x20
#define _RI  0x40
#define _CD  0x80
