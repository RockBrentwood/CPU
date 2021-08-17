/* codetms7.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TMS7000-Familie                                             */
/*                                                                           */
/* Historie: 26.2.1997 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "stringutil.h"
#include "bpemu.h"
#include "nls.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct {
   char *Name;
   Word Code;
} FixedOrder;

typedef struct {
   char *Name;
   int Length;
   Word Code;
} LengthOrder;

#define ModNone (-1)
#define ModAccA 0
#define MModAccA (1 << ModAccA) /* A */
#define ModAccB 1
#define MModAccB (1 << ModAccB) /* B */
#define ModReg 2
#define MModReg (1 << ModReg) /* Rn */
#define ModPort 3
#define MModPort (1 << ModPort) /* Pn */
#define ModAbs 4
#define MModAbs (1 << ModAbs) /* nnnn */
#define ModBRel 5
#define MModBRel (1 << ModBRel) /* nnnn(B) */
#define ModIReg 6
#define MModIReg (1 << ModIReg) /* *Rn */
#define ModImm 7
#define MModImm (1 << ModImm) /* #nn */
#define ModImmBRel 8
#define MModImmBRel (1 << ModImmBRel) /* #nnnn(b) */

#define FixedOrderCount 14
#define Rel8OrderCount 12
#define ALU1OrderCount 7
#define ALU2OrderCount 5
#define JmpOrderCount 2
#define ABRegOrderCount 14

static CPUVar CPU70C40, CPU70C20, CPU70C00, CPU70CT40, CPU70CT20, CPU70C82, CPU70C42, CPU70C02, CPU70C48, CPU70C08;

static Byte OpSize;
static ShortInt AdrType;
static Byte AdrVals[2];

static FixedOrder *FixedOrders;
static FixedOrder *Rel8Orders;
static FixedOrder *ALU1Orders;
static LengthOrder *ALU2Orders;
static FixedOrder *JmpOrders;
static FixedOrder *ABRegOrders;

/*---------------------------------------------------------------------------*/

static void InitFixed(char *NName, Word NCode) {
   if (InstrZ >= FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void InitRel8(char *NName, Word NCode) {
   if (InstrZ >= Rel8OrderCount) exit(255);
   Rel8Orders[InstrZ].Name = NName;
   Rel8Orders[InstrZ++].Code = NCode;
}

static void InitALU1(char *NName, Word NCode) {
   if (InstrZ >= ALU1OrderCount) exit(255);
   ALU1Orders[InstrZ].Name = NName;
   ALU1Orders[InstrZ++].Code = NCode;
}

static void InitALU2(char *NName, Word NCode) {
   if (InstrZ >= ALU2OrderCount) exit(255);
   ALU2Orders[InstrZ].Name = NName;
   ALU2Orders[InstrZ].Length = strlen(NName);
   ALU2Orders[InstrZ++].Code = NCode;
}

static void InitJmp(char *NName, Word NCode) {
   if (InstrZ >= JmpOrderCount) exit(255);
   JmpOrders[InstrZ].Name = NName;
   JmpOrders[InstrZ++].Code = NCode;
}

static void InitABReg(char *NName, Word NCode) {
   if (InstrZ >= ABRegOrderCount) exit(255);
   ABRegOrders[InstrZ].Name = NName;
   ABRegOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCount);
   InstrZ = 0;
   InitFixed("CLRC", 0x00b0);
   InitFixed("DINT", 0x0060);
   InitFixed("EINT", 0x0005);
   InitFixed("IDLE", 0x0001);
   InitFixed("LDSP", 0x000d);
   InitFixed("NOP", 0x0000);
   InitFixed("RETI", 0x000b);
   InitFixed("RTI", 0x000b);
   InitFixed("RETS", 0x000a);
   InitFixed("RTS", 0x000a);
   InitFixed("SETC", 0x0007);
   InitFixed("STSP", 0x0009);
   InitFixed("TSTA", 0x00b0);
   InitFixed("TSTB", 0x00c1);

   Rel8Orders = (FixedOrder *) malloc(sizeof(FixedOrder) * Rel8OrderCount);
   InstrZ = 0;
   InitRel8("JMP", 0xe0);
   InitRel8("JC", 0xe3);
   InitRel8("JEQ", 0xe2);
   InitRel8("JHS", 0xe3);
   InitRel8("JL", 0xe7);
   InitRel8("JN", 0xe1);
   InitRel8("JNC", 0xe7);
   InitRel8("JNE", 0xe6);
   InitRel8("JNZ", 0xe6);
   InitRel8("JP", 0xe4);
   InitRel8("JPZ", 0xe5);
   InitRel8("JZ", 0xe2);

   ALU1Orders = (FixedOrder *) malloc(sizeof(FixedOrder) * ALU1OrderCount);
   InstrZ = 0;
   InitALU1("ADC", 9);
   InitALU1("ADD", 8);
   InitALU1("DAC", 14);
   InitALU1("DSB", 15);
   InitALU1("SBB", 11);
   InitALU1("SUB", 10);
   InitALU1("MPY", 12);

   ALU2Orders = (LengthOrder *) malloc(sizeof(LengthOrder) * ALU2OrderCount);
   InstrZ = 0;
   InitALU2("AND", 3);
   InitALU2("BTJO", 6);
   InitALU2("BTJZ", 7);
   InitALU2("OR", 4);
   InitALU2("XOR", 5);

   JmpOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * JmpOrderCount);
   InstrZ = 0;
   InitJmp("BR", 12);
   InitJmp("CALL", 14);

   ABRegOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * ABRegOrderCount);
   InstrZ = 0;
   InitABReg("CLR", 5);
   InitABReg("DEC", 2);
   InitABReg("DECD", 11);
   InitABReg("INC", 3);
   InitABReg("INV", 4);
   InitABReg("POP", 9);
   InitABReg("PUSH", 8);
   InitABReg("RL", 14);
   InitABReg("RLC", 15);
   InitABReg("RR", 12);
   InitABReg("RRC", 13);
   InitABReg("SWAP", 7);
   InitABReg("XCHB", 6);
   InitABReg("DJNZ", 10);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(Rel8Orders);
   free(ALU1Orders);
   free(ALU2Orders);
   free(JmpOrders);
   free(ABRegOrders);
}

/*---------------------------------------------------------------------------*/

static void ChkAdr(Word Mask) {
   if ((AdrType != -1) && ((Mask & (1L << AdrType)) == 0)) {
      WrError(1350);
      AdrType = ModNone;
      AdrCnt = 0;
   }
}

static void DecodeAdr(char *Asc, Word Mask) {
   Integer HVal, Lev;
   char *p;
   bool OK;

   AdrType = ModNone;
   AdrCnt = 0;

   if (strcasecmp(Asc, "A") == 0) {
      if (((Mask & MModAccA) != 0)) AdrType = ModAccA;
      else if (((Mask & MModReg) != 0)) {
         AdrCnt = 1;
         AdrVals[0] = 0;
         AdrType = ModReg;
      } else {
         AdrCnt = 2;
         AdrVals[0] = 0;
         AdrVals[1] = 0;
         AdrType = ModAbs;
      }
      ChkAdr(Mask);
      return;
   }

   if (strcasecmp(Asc, "B") == 0) {
      if (((Mask & MModAccB) != 0)) AdrType = ModAccB;
      else if (((Mask & MModReg) != 0)) {
         AdrCnt = 1;
         AdrVals[0] = 1;
         AdrType = ModReg;
      } else {
         AdrCnt = 2;
         AdrVals[0] = 0;
         AdrVals[1] = 1;
         AdrType = ModAbs;
      }
      ChkAdr(Mask);
      return;
   }

   if ((*Asc == '#') || (*Asc == '%')) {
      strmove(Asc, 1);
      if (strcasecmp(Asc + strlen(Asc) - 3, "(B)") == 0) {
         Asc[strlen(Asc) - 3] = '\0';
         HVal = EvalIntExpression(Asc, Int16, &OK);
         if (OK) {
            AdrVals[0] = Hi(HVal);
            AdrVals[1] = Lo(HVal);
            AdrType = ModImmBRel;
            AdrCnt = 2;
         }
      } else {
         switch (OpSize) {
            case 0:
               AdrVals[0] = EvalIntExpression(Asc, Int8, &OK);
               break;
            case 1:
               HVal = EvalIntExpression(Asc, Int16, &OK);
               AdrVals[0] = Hi(HVal);
               AdrVals[1] = Lo(HVal);
               break;
         }
         if (OK) {
            AdrCnt = 1 + OpSize;
            AdrType = ModImm;
         }
      }
      ChkAdr(Mask);
      return;
   }

   if (*Asc == '*') {
      AdrVals[0] = EvalIntExpression(Asc + 1, Int8, &OK);
      if (OK) {
         AdrCnt = 1;
         AdrType = ModIReg;
      }
      ChkAdr(Mask);
      return;
   }

   if (*Asc == '@') strmove(Asc, 1);

   if (Asc[strlen(Asc) - 1] == ')') {
      p = Asc + strlen(Asc) - 2;
      Lev = 0;
      while ((p >= Asc) && (Lev != -1)) {
         switch (*p) {
            case '(':
               Lev--;
               break;
            case ')':
               Lev++;
               break;
         }
         if (Lev != -1) p--;
      }
      if (p < Asc) {
         WrError(1300);
         p = NULL;
      }
   } else p = NULL;

   if (p == NULL) {
      HVal = EvalIntExpression(Asc, Int16, &OK);
      if (OK)
         if (((Mask & MModReg) != 0) && (Hi(HVal) == 0)) {
            AdrVals[0] = Lo(HVal);
            AdrCnt = 1;
            AdrType = ModReg;
         } else if (((Mask & MModPort) != 0) && (Hi(HVal) == 0x01)) {
            AdrVals[0] = Lo(HVal);
            AdrCnt = 1;
            AdrType = ModPort;
         } else {
            AdrVals[0] = Hi(HVal);
            AdrVals[1] = Lo(HVal);
            AdrCnt = 2;
            AdrType = ModAbs;
         }
      ChkAdr(Mask);
      return;
   } else {
      FirstPassUnknown = false;
      *p = '\0';
      HVal = EvalIntExpression(Asc, Int16, &OK);
      if (OK) {
         p++;
         p[strlen(p) - 1] = '\0';
         if (strcasecmp(p, "B") == 0) {
            AdrVals[0] = Hi(HVal);
            AdrVals[1] = Lo(HVal);
            AdrCnt = 2;
            AdrType = ModBRel;
         } else WrXError(1445, p);
      }
      ChkAdr(Mask);
      return;
   }
}

static bool DecodePseudo(void) {
   return false;
}

static void PutCode(Word Code) {
   if (Hi(Code) != 0) BAsmCode[CodeLen++] = Hi(Code);
   BAsmCode[CodeLen++] = Lo(Code);
}

static void MakeCode_TMS7(void) {
   Integer z, AdrInt;
   bool OK, Rela;

   CodeLen = 0;
   DontPrint = false;
   OpSize = 0;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(true)) return;

   for (z = 0; z < FixedOrderCount; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else PutCode(FixedOrders[z].Code);
         return;
      }

   if ((Memo("MOV")) || (Memo("MOVP"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], MModPort + MModAccA + MModAccB + ((Memo("MOVP")) ? 0 : MModReg + MModAbs + MModIReg + MModBRel));
         switch (AdrType) {
            case ModAccA:
               DecodeAdr(ArgStr[1], MModPort + ((Memo("MOVP")) ? 0 : MModReg + MModAbs + MModIReg + MModBRel + MModAccB + MModImm));
               switch (AdrType) {
                  case ModReg:
                     BAsmCode[0] = 0x12;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
                  case ModAbs:
                     BAsmCode[0] = 0x8a;
                     memcpy(BAsmCode + 1, AdrVals, 2);
                     CodeLen = 3;
                     break;
                  case ModIReg:
                     BAsmCode[0] = 0x9a;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
                  case ModBRel:
                     BAsmCode[0] = 0xaa;
                     memcpy(BAsmCode + 1, AdrVals, 2);
                     CodeLen = 3;
                     break;
                  case ModAccB:
                     BAsmCode[0] = 0x62;
                     CodeLen = 1;
                     break;
                  case ModPort:
                     BAsmCode[0] = 0x80;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0x22;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
               }
               break;
            case ModAccB:
               DecodeAdr(ArgStr[1], MModPort + ((Memo("MOVP")) ? 0 : MModAccA + MModReg + MModImm));
               switch (AdrType) {
                  case ModAccA:
                     BAsmCode[0] = 0xc0;
                     CodeLen = 1;
                     break;
                  case ModReg:
                     BAsmCode[0] = 0x32;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
                  case ModPort:
                     BAsmCode[0] = 0x91;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0x52;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
               }
               break;
            case ModReg:
               BAsmCode[1] = BAsmCode[2] = AdrVals[0];
               DecodeAdr(ArgStr[1], MModAccA + MModAccB + MModReg + MModPort + MModImm);
               switch (AdrType) {
                  case ModAccA:
                     BAsmCode[0] = 0xd0;
                     CodeLen = 2;
                     break;
                  case ModAccB:
                     BAsmCode[0] = 0xd1;
                     CodeLen = 2;
                     break;
                  case ModReg:
                     BAsmCode[0] = 0x42;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 3;
                     break;
                  case ModPort:
                     BAsmCode[0] = 0xa2;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 3;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0x72;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 3;
                     break;
               }
               break;
            case ModPort:
               BAsmCode[1] = AdrVals[0];
               BAsmCode[2] = AdrVals[0];
               DecodeAdr(ArgStr[1], MModAccA + MModAccB + MModImm);
               switch (AdrType) {
                  case ModAccA:
                     BAsmCode[0] = 0x82;
                     CodeLen = 2;
                     break;
                  case ModAccB:
                     BAsmCode[0] = 0x92;
                     CodeLen = 2;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0xa2;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 3;
                     break;
               }
               break;
            case ModAbs:
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               DecodeAdr(ArgStr[1], MModAccA);
               if (AdrType != ModNone) {
                  BAsmCode[0] = 0x8b;
                  CodeLen = 3;
               }
               break;
            case ModIReg:
               BAsmCode[1] = AdrVals[0];
               DecodeAdr(ArgStr[1], MModAccA);
               if (AdrType != ModNone) {
                  BAsmCode[0] = 0x9b;
                  CodeLen = 2;
               }
               break;
            case ModBRel:
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               DecodeAdr(ArgStr[1], MModAccA);
               if (AdrType != ModNone) {
                  BAsmCode[0] = 0xab;
                  CodeLen = 3;
               }
               break;
         }
      }
      return;
   }

   if (Memo("LDA")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModAbs + MModBRel + MModIReg);
         switch (AdrType) {
            case ModAbs:
               BAsmCode[0] = 0x8a;
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               CodeLen = 1 + AdrCnt;
               break;
            case ModBRel:
               BAsmCode[0] = 0xaa;
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               CodeLen = 1 + AdrCnt;
               break;
            case ModIReg:
               BAsmCode[0] = 0x9a;
               BAsmCode[1] = AdrVals[0];
               CodeLen = 2;
               break;
         }
      }
      return;
   }

   if (Memo("STA")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModAbs + MModBRel + MModIReg);
         switch (AdrType) {
            case ModAbs:
               BAsmCode[0] = 0x8b;
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               CodeLen = 1 + AdrCnt;
               break;
            case ModBRel:
               BAsmCode[0] = 0xab;
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               CodeLen = 1 + AdrCnt;
               break;
            case ModIReg:
               BAsmCode[0] = 0x9b;
               BAsmCode[1] = AdrVals[0];
               CodeLen = 2;
               break;
         }
      }
      return;
   }

   if ((Memo("MOVW")) || (Memo("MOVD"))) {
      OpSize = 1;
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], MModReg);
         if (AdrType != ModNone) {
            z = AdrVals[0];
            DecodeAdr(ArgStr[1], MModReg + MModImm + MModImmBRel);
            switch (AdrType) {
               case ModReg:
                  BAsmCode[0] = 0x98;
                  BAsmCode[1] = AdrVals[0];
                  BAsmCode[2] = z;
                  CodeLen = 3;
                  break;
               case ModImm:
                  BAsmCode[0] = 0x88;
                  memcpy(BAsmCode + 1, AdrVals, 2);
                  BAsmCode[3] = z;
                  CodeLen = 4;
                  break;
               case ModImmBRel:
                  BAsmCode[0] = 0xa8;
                  memcpy(BAsmCode + 1, AdrVals, 2);
                  BAsmCode[3] = z;
                  CodeLen = 4;
                  break;
            }
         }
      }
      return;
   }

   for (z = 0; z < Rel8OrderCount; z++)
      if (Memo(Rel8Orders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrInt = EvalIntExpression(ArgStr[1], UInt16, &OK) - (EProgCounter() + 2);
            if (OK)
               if ((AdrInt > 127) || (AdrInt < -128)) WrError(1370);
               else {
                  CodeLen = 2;
                  BAsmCode[0] = Rel8Orders[z].Code;
                  BAsmCode[1] = AdrInt & 0xff;
               }
         }
         return;
      }

   if ((Memo("CMP")) || (Memo("CMPA"))) {
      if (((Memo("CMP")) && (ArgCnt != 2)) || ((Memo("CMPA")) && (ArgCnt != 1))) WrError(1110);
      else {
         if (Memo("CMPA")) AdrType = ModAccA;
         else DecodeAdr(ArgStr[2], MModAccA + MModAccB + MModReg);
         switch (AdrType) {
            case ModAccA:
               DecodeAdr(ArgStr[1], MModAbs + MModIReg + MModBRel + MModAccB + MModReg + MModImm);
               switch (AdrType) {
                  case ModAbs:
                     BAsmCode[0] = 0x8d;
                     memcpy(BAsmCode + 1, AdrVals, 2);
                     CodeLen = 3;
                     break;
                  case ModIReg:
                     BAsmCode[0] = 0x9d;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
                  case ModBRel:
                     BAsmCode[0] = 0xad;
                     memcpy(BAsmCode + 1, AdrVals, 2);
                     CodeLen = 3;
                     break;
                  case ModAccB:
                     BAsmCode[0] = 0x6d;
                     CodeLen = 1;
                     break;
                  case ModReg:
                     BAsmCode[0] = 0x1d;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0x2d;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
               }
               break;
            case ModAccB:
               DecodeAdr(ArgStr[1], MModReg + MModImm);
               switch (AdrType) {
                  case ModReg:
                     BAsmCode[0] = 0x3d;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0x5d;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
               }
               break;
            case ModReg:
               BAsmCode[2] = AdrVals[0];
               DecodeAdr(ArgStr[1], MModReg + MModImm);
               switch (AdrType) {
                  case ModReg:
                     BAsmCode[0] = 0x4d;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 3;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0x7d;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 3;
                     break;
               }
               break;
         }
      }
      return;
   }

   for (z = 0; z < ALU1OrderCount; z++)
      if (Memo(ALU1Orders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else {
            DecodeAdr(ArgStr[2], MModAccA + MModAccB + MModReg);
            switch (AdrType) {
               case ModAccA:
                  DecodeAdr(ArgStr[1], MModAccB + MModReg + MModImm);
                  switch (AdrType) {
                     case ModAccB:
                        CodeLen = 1;
                        BAsmCode[0] = 0x60 + ALU1Orders[z].Code;
                        break;
                     case ModReg:
                        CodeLen = 2;
                        BAsmCode[0] = 0x10 + ALU1Orders[z].Code;
                        BAsmCode[1] = AdrVals[0];
                        break;
                     case ModImm:
                        CodeLen = 2;
                        BAsmCode[0] = 0x20 + ALU1Orders[z].Code;
                        BAsmCode[1] = AdrVals[0];
                        break;
                  }
                  break;
               case ModAccB:
                  DecodeAdr(ArgStr[1], MModReg + MModImm);
                  switch (AdrType) {
                     case ModReg:
                        CodeLen = 2;
                        BAsmCode[0] = 0x30 + ALU1Orders[z].Code;
                        BAsmCode[1] = AdrVals[0];
                        break;
                     case ModImm:
                        CodeLen = 2;
                        BAsmCode[0] = 0x50 + ALU1Orders[z].Code;
                        BAsmCode[1] = AdrVals[0];
                        break;
                  }
                  break;
               case ModReg:
                  BAsmCode[2] = AdrVals[0];
                  DecodeAdr(ArgStr[1], MModReg + MModImm);
                  switch (AdrType) {
                     case ModReg:
                        CodeLen = 3;
                        BAsmCode[0] = 0x40 + ALU1Orders[z].Code;
                        BAsmCode[1] = AdrVals[0];
                        break;
                     case ModImm:
                        CodeLen = 3;
                        BAsmCode[0] = 0x70 + ALU1Orders[z].Code;
                        BAsmCode[1] = AdrVals[0];
                        break;
                  }
                  break;
            }
         }
         return;
      }

   for (z = 0; z < ALU2OrderCount; z++)
      if ((strncmp(OpPart, ALU2Orders[z].Name, ALU2Orders[z].Length) == 0)
         && ((OpPart[ALU2Orders[z].Length] == 'P') || (OpPart[ALU2Orders[z].Length] == '\0'))) {
         Rela = strncmp(OpPart, "BTJ", 3) == 0;
         if (((Rela) && (ArgCnt != 3))
            || ((!Rela) && (ArgCnt != 2))) WrError(1110);
         else {
            DecodeAdr(ArgStr[2], MModPort + ((OpPart[strlen(OpPart) - 1] == 'P') ? 0 : MModAccA + MModAccB + MModReg));
            switch (AdrType) {
               case ModAccA:
                  DecodeAdr(ArgStr[1], MModAccB + MModReg + MModImm);
                  switch (AdrType) {
                     case ModAccB:
                        BAsmCode[0] = 0x60 + ALU2Orders[z].Code;
                        CodeLen = 1;
                        break;
                     case ModReg:
                        BAsmCode[0] = 0x10 + ALU2Orders[z].Code;
                        BAsmCode[1] = AdrVals[0];
                        CodeLen = 2;
                        break;
                     case ModImm:
                        BAsmCode[0] = 0x20 + ALU2Orders[z].Code;
                        BAsmCode[1] = AdrVals[0];
                        CodeLen = 2;
                        break;
                  }
                  break;
               case ModAccB:
                  DecodeAdr(ArgStr[1], MModReg + MModImm);
                  switch (AdrType) {
                     case ModReg:
                        BAsmCode[0] = 0x30 + ALU2Orders[z].Code;
                        BAsmCode[1] = AdrVals[0];
                        CodeLen = 2;
                        break;
                     case ModImm:
                        BAsmCode[0] = 0x50 + ALU2Orders[z].Code;
                        BAsmCode[1] = AdrVals[0];
                        CodeLen = 2;
                        break;
                  }
                  break;
               case ModReg:
                  BAsmCode[2] = AdrVals[0];
                  DecodeAdr(ArgStr[1], MModReg + MModImm);
                  switch (AdrType) {
                     case ModReg:
                        BAsmCode[0] = 0x40 + ALU2Orders[z].Code;
                        BAsmCode[1] = AdrVals[0];
                        CodeLen = 3;
                        break;
                     case ModImm:
                        BAsmCode[0] = 0x70 + ALU2Orders[z].Code;
                        BAsmCode[1] = AdrVals[0];
                        CodeLen = 3;
                  }
                  break;
               case ModPort:
                  BAsmCode[1] = AdrVals[0];
                  DecodeAdr(ArgStr[1], MModAccA + MModAccB + MModImm);
                  switch (AdrType) {
                     case ModAccA:
                        BAsmCode[0] = 0x80 + ALU2Orders[z].Code;
                        CodeLen = 2;
                        break;
                     case ModAccB:
                        BAsmCode[0] = 0x90 + ALU2Orders[z].Code;
                        CodeLen = 2;
                        break;
                     case ModImm:
                        BAsmCode[0] = 0xa0 + ALU2Orders[z].Code;
                        BAsmCode[2] = BAsmCode[1];
                        BAsmCode[1] = AdrVals[0];
                        CodeLen = 3;
                  }
                  break;
            }
            if ((CodeLen != 0) && (Rela)) {
               AdrInt = EvalIntExpression(ArgStr[3], Int16, &OK) - (EProgCounter() + CodeLen + 1);
               if (!OK) CodeLen = 0;
               else if ((!SymbolQuestionable) && ((AdrInt > 127) || (AdrInt < -128))) {
                  WrError(1370);
                  CodeLen = 0;
               } else BAsmCode[CodeLen++] = AdrInt & 0xff;
            }
         }
         return;
      }

   for (z = 0; z < JmpOrderCount; z++)
      if (Memo(JmpOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], MModAbs + MModIReg + MModBRel);
            switch (AdrType) {
               case ModAbs:
                  CodeLen = 3;
                  BAsmCode[0] = 0x80 + JmpOrders[z].Code;
                  memcpy(BAsmCode + 1, AdrVals, 2);
                  break;
               case ModIReg:
                  CodeLen = 2;
                  BAsmCode[0] = 0x90 + JmpOrders[z].Code;
                  BAsmCode[1] = AdrVals[0];
                  break;
               case ModBRel:
                  CodeLen = 3;
                  BAsmCode[0] = 0xa0 + JmpOrders[z].Code;
                  memcpy(BAsmCode + 1, AdrVals, 2);
                  break;
            }
         }
         return;
      }

   for (z = 0; z < ABRegOrderCount; z++)
      if (Memo(ABRegOrders[z].Name)) {
         if (((!Memo("DJNZ")) && (ArgCnt != 1))
            || ((Memo("DJNZ")) && (ArgCnt != 2))) WrError(1110);
         else if (strcasecmp(ArgStr[1], "ST") == 0) {
            if ((Memo("PUSH")) || (Memo("POP"))) {
               BAsmCode[0] = 8 + (Memo("PUSH") * 6);
               CodeLen = 1;
            } else WrError(1350);
         } else {
            DecodeAdr(ArgStr[1], MModAccA + MModAccB + MModReg);
            switch (AdrType) {
               case ModAccA:
                  BAsmCode[0] = 0xb0 + ABRegOrders[z].Code;
                  CodeLen = 1;
                  break;
               case ModAccB:
                  BAsmCode[0] = 0xc0 + ABRegOrders[z].Code;
                  CodeLen = 1;
                  break;
               case ModReg:
                  BAsmCode[0] = 0xd0 + ABRegOrders[z].Code;
                  BAsmCode[1] = AdrVals[0];
                  CodeLen = 2;
                  break;
            }
            if ((Memo("DJNZ")) && (CodeLen != 0)) {
               AdrInt = EvalIntExpression(ArgStr[2], UInt16, &OK) - (EProgCounter() + CodeLen + 1);
               if (!OK) CodeLen = 0;
               else if ((!SymbolQuestionable) && ((AdrInt > 127) || (AdrInt < -128))) {
                  WrError(1370);
                  CodeLen = 0;
               } else BAsmCode[CodeLen++] = AdrInt & 0xff;
            }
         }
         return;
      }

   if (Memo("TRAP")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         FirstPassUnknown = false;
         BAsmCode[0] = EvalIntExpression(ArgStr[1], UInt5, &OK);
         if (FirstPassUnknown) BAsmCode[0] &= 15;
         if (OK)
            if (BAsmCode[0] > 23) WrError(1320);
            else {
               BAsmCode[0] = 0xff - BAsmCode[0];
               CodeLen = 1;
            }
      }
      return;
   }

   if (Memo("TST")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModAccA + MModAccB);
         switch (AdrType) {
            case ModAccA:
               BAsmCode[0] = 0xb0;
               CodeLen = 1;
               break;
            case ModAccB:
               BAsmCode[0] = 0xc1;
               CodeLen = 1;
               break;
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_TMS7(void) {
   switch (ActPC) {
      case SegCode:
         return (ProgCounter() < 0x10000);
      default:
         return false;
   }
}

static bool IsDef_TMS7(void) {
   return false;
}

static void InternSymbol_TMS7(char *Asc, TempResult * Erg) {
   String h;
   bool Err;

   Erg->Typ = TempNone;
   if ((strlen(Asc) < 2) || ((toupper(*Asc) != 'R') && (toupper(*Asc) != 'P'))) return;

   strmaxcpy(h, Asc + 1, 255);
   if ((*h == '0') && (strlen(h) > 1)) *h = '$';
   Erg->Contents.Int = ConstLongInt(h, &Err);
   if ((!Err) || (Erg->Contents.Int < 0) || (Erg->Contents.Int > 255)) return;

   Erg->Typ = TempInt;
   if (toupper(*Asc) == 'P') Erg->Contents.Int += 0x100;
}

static void SwitchFrom_TMS7(void) {
   DeinitFields();
}

static void SwitchTo_TMS7(void) {
   TurnWords = false;
   ConstMode = ConstModeIntel;
   SetIsOccupied = false;

   PCSymbol = "$";
   HeaderID = 0x73;
   NOPCode = 0x00;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_TMS7;
   ChkPC = ChkPC_TMS7;
   IsDef = IsDef_TMS7;
   SwitchFrom = SwitchFrom_TMS7;
   InternSymbol = InternSymbol_TMS7;

   InitFields();
}

void codetms7_init(void) {
   CPU70C00 = AddCPU("TMS70C00", SwitchTo_TMS7);
   CPU70C20 = AddCPU("TMS70C20", SwitchTo_TMS7);
   CPU70C40 = AddCPU("TMS70C40", SwitchTo_TMS7);
   CPU70CT20 = AddCPU("TMS70CT20", SwitchTo_TMS7);
   CPU70CT40 = AddCPU("TMS70CT40", SwitchTo_TMS7);
   CPU70C02 = AddCPU("TMS70C02", SwitchTo_TMS7);
   CPU70C42 = AddCPU("TMS70C42", SwitchTo_TMS7);
   CPU70C82 = AddCPU("TMS70C82", SwitchTo_TMS7);
   CPU70C08 = AddCPU("TMS70C08", SwitchTo_TMS7);
   CPU70C48 = AddCPU("TMS70C48", SwitchTo_TMS7);
}
