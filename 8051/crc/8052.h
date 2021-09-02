;;; Special function registers for the 8052
T2CON  sfr 0xc8
RCAP2L sfr 0xca
RCAP2H sfr 0xcb
TL2    sfr 0xcc
TH2    sfr 0xcd

;;; Special funcgtion register bits
;;; Port 1
T2   bit P1.0
T2EX bit P1.1

;;; IE and IP
ET2 bit IE.5
PT2 bit IP.5

;;; T2CON
CP_RL2 bit T2CON.0
C_T2   bit T2CON.1
TR2    bit T2CON.2
EXEN2  bit T2CON.3
TCLK   bit T2CON.4
RCLK   bit T2CON.5
EXF2   bit T2CON.6
TF2    bit T2CON.7
