;;; If running as a stand-alone, uncomment the following lines:
;;; org 00h
;;; ajmp Start
;;; org 03h
;;;    push PSW
;;; ljmp 4003h
;;; Start:
;;;    acall main
;;; Idle:
;;;    orl PCON, #1
;;; sjmp Idle

include "8052.h"
org 4000h
ajmp main

org 4003h
   push 0
   push 1
   push 2
   push ACC
   push DPL
   push DPH
   acall getLoop
   pop DPH
   pop DPL
   pop ACC
   pop 2
   pop 1
   pop 0
   pop PSW
reti

main:
   clr INT0
   setb PX0
   setb IT0
   setb EX0
   setb IE0
   setb EA
ret

;;; Standard I/O routines.  Not interrupt driven.
getchar:
   jnb RI, $
   mov A, SBUF
   clr RI
   acall putchar   ;;; ECHO
ret

nib:          ;;; scanf("%1x", &A);
   cjne A, #'a', $ + 3
jc tryUpper
   cjne A, #('f' + 1), $ + 3
jnc tryUpper
   add A, #(10 - 'a')
ret
tryUpper:
   cjne A, #'A', $ + 3
jc tryDigit
   cjne A, #('F' + 1), $ + 3
jnc tryDigit
   add A, #(10 - 'A')
ret
tryDigit:
   cjne A, #'0', $ + 3
jc badInput
   cjne A, #('9' + 1), $ + 3
jnc BadInput
   add A, #(0 - '0')
ret
badInput:
   mov A, #0
ret

gethex:       ;;; scanf("%2x", &A);
   acall getchar
   acall nib
   swap A
   mov B, A
   acall getchar
   acall nib
   add A, B
ret

getword:       ;;; scanf("%4x", &DPTR);
   acall gethex
   mov DPH, A
   acall gethex
   mov DPL, A
ret

nl:            ;;; putchar('\n');
   mov A, #13
   acall putchar
   mov A, #10
putchar:       ;;; putchar(A);
   clr TI
   mov SBUF, A
   jnb TI, $
ret

putnib:        ;;; printf("%1x", A&0xf);
   anl A, #0fh
   cjne A, #10, $ + 3
jc isdigit
   add A, #('a' - 10)
   acall putchar
ret
isdigit:
   add A, #'0'
   acall putchar
ret

puthex:         ;;; printf("%2x", A);
   push ACC
   swap A
   acall putnib
   pop ACC
   acall putnib
ret

putword:         ;;; printf("%4x", DPTR);
   mov A, DPH
   acall puthex
   mov A, DPL
   acall puthex
ret

puts:             ;;; puts(DPTR);
   putsLoop:
      clr A
      movc A, @A + DPTR
   jz putsBreak
      inc DPTR
      acall putchar
   sjmp putsLoop
putsBreak:
ret

;;; The Stack Frame is partially determined by the interrupt handler at 4003h.
;;; Its entire contents are (lowest address first):
;;; Pushed on stack by the hardware interrupt:
;;;    SP[-12]: PC (Low byte, High byte)
;;; Pushed on stack by the interrupt handler at 0003h:
;;;    SP[-10]: PSW
;;; Pushed on stack by the vectored interrupt handler at 4003h:
;;;    SP[-9]:  R0
;;;    SP[-8]:  R1
;;;    SP[-7]:  R2
;;;    SP[-6]:  A
;;;    SP{-5]:  DPTR
;;;    SP[-3]:  Return address from "acall getLoop"
;;; Pushed on stack when display routine is called from getLoop:
;;;    SP[-1]:  Return address of current display routine (ShowRegs, ShowSFRs)

PQ0: db "REGS:", 13, 10, "R0 ", 0
PQ1: db "    R1 ", 0
PQ2: db "    R2 ", 0
PQ3: db "    R3 ", 0
PQ4: db "    R4 ", 0
PQ5: db "    R5 ", 0
PQ6: db "    R6 ", 0
PQ7: db "    R7 ", 0
ShowRegs:           ;;; void ShowRegs(void) {
   mov DPTR, #PQ0
   acall puts
   mov A, SP
   add A, #-9
   acall ShowR      ;;;    printf("REGS:\nR0 %2x", SP[-9]);
   mov DPTR, #PQ1
   acall puts
   mov A, SP
   add A, #-8
   acall ShowR      ;;;    printf("    R1 %2x", SP[-8]);
   mov DPTR, #PQ2
   acall puts
   mov A, SP
   add A, #-7
   acall ShowR      ;;;    printf("   R2 %2x", SP[-7]);
   mov DPTR, #PQ3
   acall puts
   mov A, R3
   acall puthex     ;;;    printf("   R3 %2x", R3);
   mov DPTR, #PQ4
   acall puts
   mov A, R4
   acall puthex     ;;;    printf("   R4 %2x", R4);
   mov DPTR, #PQ5
   acall puts
   mov A, R5
   acall puthex     ;;;    printf("   R5 %2x", R5);
   mov DPTR, #PQ6
   acall puts
   mov A, R6
   acall puthex     ;;;    printf("   R6 %2x", R6);
   mov DPTR, #PQ7
   acall puts
   mov A, R7
   acall puthex
   acall nl         ;;;    printf("   R7 %2x\n", R7);
ret                 ;;; }

ShowR:            ;;; ShowR(byte *A) {
   mov R0, A
   mov A, @R0
   acall puthex   ;;;    printf("%2x", *A);
ret               ;;; }

PP0: db "SFRs:", 13, 10, "A ", 0
PP1: db "         B ", 0
PP2: db "         PSW ", 0
PP3: db "       SP ", 0
PP4: db "        DPTR ", 0
PP5: db 13, 10, "PCON ", 0
PP6: db "      TCON ", 0
PP7: db "      TMOD ", 0
PP8: db "      T2CON ", 0
PP9: db "     SCON ", 0
PPa: db "     IE ", 0
PPb: db "     IP ", 0
PPc: db 13, 10, "P0 ", 0
PPd: db "        P1 ", 0
PPe: db "        P2 ", 0
PPf: db "        P3 ", 0
PPg: db 13, 10, "TIMER0 ", 0
PPh: db "  TIMER1 ", 0
PPi: db "  TIMER2 ", 0
PPj: db "  RCAP2 ", 0

;;; There is just no easier way of doing this.
ShowSFRs:          ;;; void ShowSFRs(void) {
   mov DPTR, #PP0
   acall puts
   mov A, SP
   add A, #-6
   acall ShowR     ;;;    printf("SFRs:\nA %2x", SP[-6]);
   mov DPTR, #PP1
   acall puts
   mov A, B
   acall puthex    ;;;    printf("         B %2x", B);
   mov DPTR, #PP2
   acall puts
   mov A, SP
   add A, #-10
   acall ShowR     ;;;    printf("         PSW %2x", SP[-10]);
   mov DPTR, #PP3
   acall puts
   mov A, SP
   add A, #-13
   acall puthex    ;;;    printf("       SP %2x", SP - 13);
   mov DPTR, #PP4
   acall puts
   mov A, SP
   add A, #-3
   mov R0, A
   mov DPL, @R0
   mov A, SP
   add A, #-2
   mov R0, A
   mov DPH, @R0
   acall putword   ;;;    printf("        DPTR %4x", *(word *)(SP - 5));
   mov DPTR, #PP5
   acall puts
   mov A, PCON
   acall puthex    ;;;    printf("\nPCON %2x", PCON);
   mov DPTR, #PP6
   acall puts
   mov A, TCON
   acall puthex    ;;;    printf("      TCON %2x", TCON);
   mov DPTR, #PP7
   acall puts
   mov A, TMOD
   acall puthex    ;;;    printf("      TMOD %2x", TMOD);
   mov DPTR, #PP8
   acall puts
   mov A, T2CON
   acall puthex    ;;;    printf("      T2CON %2x", T2CON);
   mov DPTR, #PP9
   acall puts
   mov A, SCON
   acall puthex    ;;;    printf("     SCON %2x", SCON);
   mov DPTR, #PPa
   acall puts
   mov A, IE
   acall puthex    ;;;    printf("     IE %2x", IE);
   mov DPTR, #PPb
   acall puts
   mov A, IP
   acall puthex    ;;;    printf("     IP %2x", IP);
   mov DPTR, #PPc
   acall puts
   mov A, P0
   acall puthex    ;;;    printf("\nP0 %2x", P0);
   mov DPTR, #PPd
   acall puts
   mov A, P1
   acall puthex    ;;;    printf("        P1 %2x", P1);
   mov DPTR, #PPe
   acall puts
   mov A, P2
   acall puthex    ;;;    printf("        P2 %2x", P2);
   mov DPTR, #PPf
   acall puts
   mov A, P3
   acall puthex    ;;;    printf("        P3 %2x", P3);
   mov DPTR, #PPg
   acall puts
   mov DPL, TL0
   mov DPH, TH0
   acall putword   ;;;    printf("\nTIMER0 %4x", TIMER0);
   mov DPTR, #PPh
   acall puts
   mov DPL, TL1
   mov DPH, TH1
   acall putword   ;;;    printf("  TIMER1 %4x", TIMER1);
   mov DPTR, #PPi
   acall puts
   mov DPL, TL2
   mov DPH, TH2
   acall putword   ;;;    printf("  TIMER2 %4x", TIMER2);
   mov DPTR, #PPj
   acall puts
   mov DPL, RCAP2L
   mov DPH, RCAP2H
   acall putword
   acall nl        ;;;    printf("  RCAP2 %4x", RCAP2);
ret                ;;; }

ShowDBY:                ;;; void ShowDBY(byte *Base, byte Limit) {
   mov R1, #0           ;;;    register byte Field = 0;
   L1:                  ;;;    for (; Limit > 0; Limit--) {
      mov A, R1
      jnz L2            ;;;       if (Field == 0) {
         mov A, R0
         acall puthex
         mov A, #':'
         acall putchar  ;;;          printf("%2x:", (int)Base);
      L2:               ;;;       }
      mov A, #' '
      acall putchar
      mov A, @R0
      inc R0
      acall puthex      ;;;       printf(" %2x", *Base++);
      mov A, R1
      inc A
      anl A, #0fh
      mov R1, A         ;;;       Field = (Field + 1)%16;
      jnz L3            ;;;       if (Field == 0) {
         acall nl       ;;;          putchar('\n');
      L3:               ;;;       }
   djnz R2, L1          ;;;    }
   acall nl             ;;;    putchar('\n');
ret                     ;;; }

ShowCBY:                  ;;; void ShowCBY(byte *Base, word Limit) {
   mov R1, #0             ;;;    register byte Field = 0;
   sjmp ContinueC         ;;;    for (; Limit > 0; Limit--) {
   LoopC:
      mov A, R1
      jnz L4              ;;;       if (Field == 0) {
         acall putword
         mov A, #':'
         acall putchar    ;;;          printf("%4x:", (word)Base);
      L4:                 ;;;       }
      mov A, #' '
      acall putchar
      clr A
      movc A, @A + DPTR
      inc DPTR
      acall puthex        ;;;       printf(" %2x", *Base++);
      mov A, R1
      inc A
      anl A, #0fh
      mov R1, A           ;;;       Field = (Field + 1)%16;
      jnz L5              ;;;       if (Field == 0) {
         acall nl         ;;;          putchar('\n');
      L5:                 ;;;       }
   ContinueC:
      mov A, R2
      dec R2
   jnz LoopC
      mov A, R3 
      dec R3 
   jnz LoopC              ;;;    }
   acall nl               ;;;    putchar('\n');
ret                       ;;; }

ShowXBY:                  ;;; void ShowXBY(byte *Base, word Limit) {
   mov R1, #0             ;;;    register byte Field = 0;
   sjmp ContinueX         ;;;    for (; Limit > 0; Limit--) {
   LoopX:
      mov A, R1
      jnz L6              ;;;       if (Field == 0) {
         acall putword
         mov A, #':'
         acall putchar    ;;;          printf("%4x:", (word)Base);
      L6:                 ;;;       }
      mov A, #' '
      acall putchar
      movx A, @DPTR
      inc DPTR
      acall puthex        ;;;       printf(" %2x", *Base++);
      mov A, R1
      inc A
      anl A, #0fh
      mov R1, A           ;;;       Field = (Field + 1)%16;
      jnz L7              ;;;       if (Field == 0) {
         acall nl         ;;;          putchar('\n');
      L7:                 ;;;       }
   ContinueX:
      mov A, R2
      dec R2
   jnz LoopX
      mov A, R3 
      dec R3 
   jnz LoopX              ;;;    }
   acall nl               ;;;    putchar('\n');
ret                       ;;; }

putOne:
   xch A, R4
   add A, R4
   xch A, R4
   acall puthex     ;;; printf("%2x", A); CheckSum += A;
ret

HexSave:                  ;;; void HexSave(byte *Base, word Limit) {
   mov R1, #0             ;;;    register byte Field = 0;
   mov A, R2
   add A, #0fh
   anl A, #0f0h
   mov R2, A
   mov A, R3
   addc A, #0
   mov R3, A              ;;;    Round Limit up to the nearest multiple of 0x10;
   sjmp ContinueH         ;;;    for (; Limit > 0; Limit--) {
   LoopH:
      mov A, R1
      jnz L8              ;;;       if (Field == 0) {
         mov R4, #0       ;;;          Clear CheckSum;
         mov A, #':'
         acall putchar
         mov A, #10h
         acall putOne
         mov A, DPH
         acall putOne
         mov A, DPL
         acall putOne
         mov A, #0
         acall putOne     ;;;          printf(":%2x%4x%2x", 0x10, Base, 0);
      L8:                 ;;;       }
      clr A
      movc A, @A + DPTR
      inc DPTR
      acall putOne        ;;;       printf(" %2x", *Base++);
      mov A, R1
      inc A
      anl A, #0fh
      mov R1, A           ;;;       Field = (Field + 1)%16;
      jnz L9              ;;;       if (Field == 0) {
         mov A, R4
         cpl A
         inc A
         acall puthex
         acall nl         ;;;          printf("%2x\n", -CheckSum);
      L9:                 ;;;       }
   ContinueH:
      mov A, R2
      dec R2
   jnz LoopH
      mov A, R3 
      dec R3 
   jnz LoopH              ;;;    }
   mov R4, #0             ;;;    Clear CheckSum;
   mov A, #':'
   acall putchar
   mov A, #00h
   acall putOne
   mov A, DPH
   acall putOne
   mov A, DPL
   acall putOne
   mov A, #1
   acall putOne           ;;;    printf(":%2x%4x%2x", 0, Base, 1);
   mov A, R4
   cpl A
   inc A
   acall puthex
   acall nl               ;;;    printf("%2x\n", -CheckSum);
ret                       ;;; }

getOne:
   acall gethex     ;;; scanf("%2x", &A); CheckSum += A;
   xch A, R4
   add A, R4
   xch A, R4
ret

HexLoad:                  ;;; void HexLoad(void) {
   waitHex:
      acall getchar
   cjne A, #':', waitHex  ;;;    while ((A = getchar()) != ':');
   mov R4, #0             ;;;    Clear CheckSum;
   acall getOne
   mov R1, A
   acall getOne
   mov DPH, A
   acall getOne
   mov DPL, A
   acall getOne
   mov R2, A              ;;;    scanf("%2x%4x%2x", &Field, &Base, &Mark);
   sjmp ContinueL         ;;;    for (; Field > 0; Field--) {
   LoopL:
      acall getOne
      movx @DPTR, A
      inc DPTR            ;;;       printf(" %2x", *Base++);
   ContinueL:
      mov A, R1
      dec R1
   jnz LoopL              ;;;    }
   acall getOne           ;;;    Clear out CheckSum
   mov A, R4
   jnz LoadError          ;;;    if (CheckSum != 0) goto LoadError;
   mov A, R2
   jz WaitHex             ;;;    if (!Mark) goto waitHex;
ret                       ;;; }
LOAD_ERR: db "Check sum error", 13, 10, 0
LoadError:
   mov DPTR, #LOAD_ERR
   acall puts
ret

;;; Insert the return address under the stack frame.
CallCBY:            ;;; CallCBY(byte *Address) {
   mov A, SP
   mov R1, A        ;;;    register byte *Src = SP;
   add A, #2
   mov R0, A        ;;;    register byte *Dst = SP + 2;
   mov SP, A        ;;;    SP += 2;
   mov R2, #11      ;;;    for (R2 = 11; R2 > 0; R2--) { /* Shift stack frame */
   shiftSP:
      mov A, @R1
      dec R1
      mov @R0, A
      dec R0        ;;;       *Dst-- = *Src--;
   djnz R2, shiftSP ;;;    }
   mov @R0, DPH
   dec R0
   mov @R0, DPL     ;;;    *(word *)Dst-- = Address;
ret                 ;;; }

;;; Replace PC in the stack frame with Address.
JumpCBY:         ;;; void JumpCBY(byte *Address) {
   mov A, SP
   add A, #-12
   mov R0, A     ;;;    register byte *R0 = SP - 12;
   mov @R0, DPL
   inc R0
   mov @R0, DPH  ;;;    *(word *)R0 = Address;
ret              ;;; }

Sizes: ;;; Instruction encoding sizes for each opcode.
db 1, 2, 3, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
db 3, 2, 3, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
db 3, 2, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
db 3, 2, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
db 2, 2, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
db 2, 2, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
db 2, 2, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
db 2, 2, 2, 1, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
db 2, 2, 2, 1, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
db 3, 2, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
db 2, 2, 2, 1, 1, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
db 2, 2, 2, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
db 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
db 2, 2, 2, 1, 1, 3, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2
db 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
db 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1

Disassemble:            ;;; void Disassemble(void) {
   mov A, SP
   add A, #-12
   mov R0, A
   mov DPL, @R0
   inc R0
   mov DPH, @R0         ;;;    register byte *DPTR = *(byte **)(SP - 12);
   acall putword
   mov A, #':'
   acall putchar
   mov A, #' '
   acall putchar        ;;;    printf("%4x: ", DPTR);
   clr A
   movc A, @A + DPTR
   inc DPTR
   mov R2, A
   acall puthex         ;;;    printf("%2x", R2 = *DPTR);
push DPL
push DPH
   mov A, R2
   mov DPTR, #Sizes
   movc A, @A + DPTR    ;;;    A = Sizes[R2];
pop DPH
pop DPL
   dec A
   jz showBreak         ;;;    while (--A > 0) {
   mov R2, A
   showLoop:
      mov A, #' '
      acall putchar
      clr A
      movc A, @A + DPTR
      inc DPTR
      acall puthex      ;;;       printf(" %2x", *DPTR++);
   djnz R2, showLoop 
   showBreak:           ;;;    }
   acall nl
ret                     ;;; }

getLoop:                ;;; getLoop() {
   acall getchar        ;;;    while (1) switch (getchar()) {
X0: cjne A, #'a', X1    ;;;    case 'a':
   acall nl             ;;;       putchar('\n');
   acall ShowRegs       ;;;       ShowRegs();
ajmp getLoop            ;;;    break;
X1: cjne A, #'b', X2    ;;;    case 'b':
   acall nl             ;;;       putchar('\n');
   acall ShowSFRs       ;;;       ShowSFRs();
ajmp getLoop            ;;;    break;
X2: cjne A, #'c', X3    ;;;    case 'c': {
   acall gethex
   mov R2, A            ;;;       register byte Limit = gethex();
   acall gethex
   mov R0, A            ;;;       register byte *Base = (byte *)gethex();
   acall nl             ;;;       putchar('\n');
   acall ShowDBY        ;;;       ShowDBY(Base, Limit);
ajmp getLoop            ;;;    } break;
X3: cjne A, #'d', X4    ;;;    case 'd': {
   acall getword
   mov R3, DPH
   mov R2, DPL          ;;;       register word Limit = getword();
   acall getword        ;;;       register byte *Base = (byte *)getword();
   acall nl             ;;;       putchar('\n');
   acall ShowCBY        ;;;       ShowCBY(Base, Limit);
ajmp getLoop            ;;;    } break;
X4: cjne A, #'e', X5    ;;;    case 'e': {
   acall getword
   mov R3, DPH
   mov R2, DPL          ;;;       register word Limit = getword();
   acall getword        ;;;       register byte *Base = (byte *)getword();
   acall nl             ;;;       putchar('\n');
   acall ShowXBY        ;;;       ShowXBY(Base, Limit);
ajmp getLoop            ;;;    } break;
X5: cjne A, #'f', X6    ;;;    case 'f': {
   acall getword
   mov R3, DPH
   mov R2, DPL          ;;;       register word Limit = getword();
   acall getword        ;;;       register byte *Base = (byte *)getword();
   acall nl             ;;;       putchar('\n');
   acall HexSave        ;;;       HexSave(Base, Limit);
ajmp getLoop            ;;;    } break;
X6: cjne A, #'g', X7    ;;;    case 'g':
   acall nl             ;;;       putchar('\n');
   acall HexLoad        ;;;       HexLoad();
ajmp getLoop            ;;;    break;
X7: cjne A, #'h', X8    ;;;    case 'h': {
   acall getword        ;;;       register byte *Address = (byte *)getword();
   acall nl             ;;;       putchar('\n');
   acall CallCBY        ;;;       CallCBY(Address);
ajmp getLoop            ;;;    } break;
X8: cjne A, #'i', X9    ;;;    case 'i': {
   acall getword        ;;;       register byte *Address = (byte *)getword();
   acall nl             ;;;       putchar('\n');
   acall JumpCBY        ;;;       JumpCBY(Address);
ajmp getLoop            ;;;    } break;
X9: cjne A, #'j', X10   ;;;    case 'j':
   acall nl             ;;;       putchar('\n');
   acall Disassemble    ;;;       Disassemble();
ajmp getLoop            ;;;    break;
X10: cjne A, #'k', X11  ;;;    case 'k':  /* Single Step */
   acall nl             ;;;       putchar('\n');
   setb IE0             ;;;       Enable single step.
ret                     ;;;    return;
X11: cjne A, #'l', X12  ;;;    case 'l':  /* Exit */
   acall nl             ;;;       putchar('\n');
   clr EX0
   clr IE0
   clr EA               ;;;       Disable single step.
ret                     ;;;    return;
X12: ajmp getLoop       ;;; }
end                   ;;; }
