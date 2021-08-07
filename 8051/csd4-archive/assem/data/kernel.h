/* Interrupt descriptor table. */
extern data SP_IE0, SP_RI, SP_TI

extern data CSTATUS ;;; High-priority interrupt queue.
extern bit PCF, PCF0, PCF1, PCF2, PCF3, PCF4

extern data TSTATUS ;;; Pulse-counter queue.
extern bit TC0, TC1, TC2, TC3, TC4, TC5

extern data State
extern bit FX0, FX1, FX2, FX3, FX4, FX5, FXX

extern data Counter

extern code Scheduler, Spawn, Pause, Resume
