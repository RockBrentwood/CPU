;; EVENT TIMER: For timer 2.
SetTimer2:
   setb PT2
   clr EXEN2
   clr C_T2
   clr CP_RL2       ;; Timer 2: 16-bit auto-reload timer.
ret

Delay2:
   mov TL2, DPL
   mov TH2, DPH
   clr TF2
   setb TR2
   setb ET2
   mov R0, #SP_TF2
   acall Pause
   clr ET2
   clr TR2
ret
