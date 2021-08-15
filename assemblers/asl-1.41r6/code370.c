/* code370.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 370-Familie                                                 */
/*                                                                           */
/* Historie: 10.12.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct {
   char *Name;
   Word Code;
} FixedOrder;

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
#define ModSPRel 6
#define MModSPRel (1 << ModSPRel) /* nn(SP) */
#define ModIReg 7
#define MModIReg (1 << ModIReg) /* @Rn */
#define ModRegRel 8
#define MModRegRel (1 << ModRegRel) /* nn(Rn) */
#define ModImm 9
#define MModImm (1 << ModImm) /* #nn */
#define ModImmBRel 10
#define MModImmBRel (1 << ModImmBRel) /* #nnnn(B) */
#define ModImmRegRel 11
#define MModImmRegRel (1 << ModImmRegRel) /* #nn(Rm) */

#define FixedOrderCount 12
#define Rel8OrderCount 18
#define ALU1OrderCount 7
#define ALU2OrderCount 5
#define JmpOrderCount 4
#define ABRegOrderCount 14
#define BitOrderCount 5

static CPUVar CPU37010, CPU37020, CPU37030, CPU37040, CPU37050;

static Byte OpSize;
static ShortInt AdrType;
static Byte AdrVals[2];
static bool AddrRel;

static FixedOrder *FixedOrders;
static FixedOrder *Rel8Orders;
static FixedOrder *ALU1Orders;
static FixedOrder *ALU2Orders;
static FixedOrder *JmpOrders;
static FixedOrder *ABRegOrders;
static FixedOrder *BitOrders;

/****************************************************************************/

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

static void InitBit(char *NName, Word NCode) {
   if (InstrZ >= BitOrderCount) exit(255);
   BitOrders[InstrZ].Name = NName;
   BitOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCount);
   InstrZ = 0;
   InitFixed("CLRC", 0x00b0);
   InitFixed("DINT", 0xf000);
   InitFixed("EINT", 0xf00c);
   InitFixed("EINTH", 0xf004);
   InitFixed("EINTL", 0xf008);
   InitFixed("IDLE", 0x00f6);
   InitFixed("LDSP", 0x00fd);
   InitFixed("NOP", 0x00ff);
   InitFixed("RTI", 0x00fa);
   InitFixed("RTS", 0x00f9);
   InitFixed("SETC", 0x00f8);
   InitFixed("STSP", 0x00fe);

   Rel8Orders = (FixedOrder *) malloc(sizeof(FixedOrder) * Rel8OrderCount);
   InstrZ = 0;
   InitRel8("JMP", 0x00);
   InitRel8("JC", 0x03);
   InitRel8("JEQ", 0x02);
   InitRel8("JG", 0x0e);
   InitRel8("JGE", 0x0d);
   InitRel8("JHS", 0x0b);
   InitRel8("JL", 0x09);
   InitRel8("JLE", 0x0a);
   InitRel8("JLO", 0x0f);
   InitRel8("JN", 0x01);
   InitRel8("JNC", 0x07);
   InitRel8("JNE", 0x06);
   InitRel8("JNV", 0x0c);
   InitRel8("JNZ", 0x06);
   InitRel8("JP", 0x04);
   InitRel8("JPZ", 0x05);
   InitRel8("JV", 0x08);
   InitRel8("JZ", 0x02);

   ALU1Orders = (FixedOrder *) malloc(sizeof(FixedOrder) * ALU1OrderCount);
   InstrZ = 0;
   InitALU1("ADC", 9);
   InitALU1("ADD", 8);
   InitALU1("DAC", 14);
   InitALU1("DSB", 15);
   InitALU1("SBB", 11);
   InitALU1("SUB", 10);
   InitALU1("MPY", 12);

   ALU2Orders = (FixedOrder *) malloc(sizeof(FixedOrder) * ALU2OrderCount);
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
   InitJmp("JMPL", 9);
   InitJmp("CALLR", 15);

   ABRegOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * ABRegOrderCount);
   InstrZ = 0;
   InitABReg("CLR", 5);
   InitABReg("COMPL", 11);
   InitABReg("DEC", 2);
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

   BitOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * BitOrderCount);
   InstrZ = 0;
   InitBit("CMPBIT", 5);
   InitBit("JBIT0", 7);
   InitBit("JBIT1", 6);
   InitBit("SBIT0", 3);
   InitBit("SBIT1", 4);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(Rel8Orders);
   free(ALU1Orders);
   free(ALU2Orders);
   free(JmpOrders);
   free(ABRegOrders);
   free(BitOrders);
}

static void ChkAdr(Word Mask) {
   if ((AdrType != -1) && ((Mask & (1 << AdrType)) == 0)) {
      WrError(1350);
      AdrType = ModNone;
      AdrCnt = 0;
   }
}

static char *HasDisp(char *Asc) {
   char *p;
   Integer Lev;

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
         WrXError(1300, Asc);
         return NULL;
      }
   } else p = NULL;

   return p;
}

static void DecodeAdr(char *Asc, Word Mask) {
   Integer HVal;
   char *p;
   bool OK;

   AdrType = ModNone;
   AdrCnt = 0;

   if (strcasecmp(Asc, "A") == 0) {
      if ((Mask & MModAccA) != 0) AdrType = ModAccA;
      else if ((Mask & MModReg) != 0) {
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
      if ((Mask & MModAccB) != 0) AdrType = ModAccB;
      else if ((Mask & MModReg) != 0) {
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

   if (*Asc == '#') {
      strcpy(Asc, Asc + 1);
      p = HasDisp(Asc);
      if (p == NULL) {
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
      } else {
         *p = '\0';
         FirstPassUnknown = false;
         HVal = EvalIntExpression(Asc, Int16, &OK);
         if (OK) {
            *p = '(';
            if (strcasecmp(p, "(B)") == 0) {
               AdrVals[0] = Hi(HVal);
               AdrVals[1] = Lo(HVal);
               AdrCnt = 2;
               AdrType = ModImmBRel;
            } else {
               if (FirstPassUnknown) HVal &= 127;
               if (ChkRange(HVal, -128, 127)) {
                  AdrVals[0] = HVal & 0xff;
                  AdrCnt = 1;
                  AdrVals[1] = EvalIntExpression(Asc, UInt8, &OK);
                  if (OK) {
                     AdrCnt = 2;
                     AdrType = ModImmRegRel;
                  }
               }
            }
         }
      }
      ChkAdr(Mask);
      return;
   }

   if (*Asc == '@') {
      AdrVals[0] = EvalIntExpression(Asc + 1, Int8, &OK);
      if (OK) {
         AdrCnt = 1;
         AdrType = ModIReg;
      }
      ChkAdr(Mask);
      return;
   }

   p = HasDisp(Asc);

   if (p == NULL) {
      HVal = EvalIntExpression(Asc, Int16, &OK);
      if (OK)
         if (((Mask & MModReg) != 0) && (Hi(HVal) == 0)) {
            AdrVals[0] = Lo(HVal);
            AdrCnt = 1;
            AdrType = ModReg;
         } else if (((Mask & MModPort) != 0) && (Hi(HVal) == 0x10)) {
            AdrVals[0] = Lo(HVal);
            AdrCnt = 1;
            AdrType = ModPort;
         } else {
            if (AddrRel) HVal -= EProgCounter() + 3;
            AdrVals[0] = Hi(HVal);
            AdrVals[1] = Lo(HVal);
            AdrCnt = 2;
            AdrType = ModAbs;
         }
      ChkAdr(Mask);
      return;
   } else {
      *(p++) = '\0';
      FirstPassUnknown = false;
      HVal = EvalIntExpression(Asc, Int16, &OK);
      if (FirstPassUnknown) HVal &= 0x7f;
      if (OK) {
         p[strlen(p) - 1] = '\0';
         if (strcasecmp(p, "B") == 0) {
            if (AddrRel) HVal -= EProgCounter() + 3;
            AdrVals[0] = Hi(HVal);
            AdrVals[1] = Lo(HVal);
            AdrCnt = 2;
            AdrType = ModBRel;
         } else if (strcasecmp(p, "SP") == 0) {
            if (AddrRel) HVal -= EProgCounter() + 3;
            if (HVal > 127) WrError(1320);
            else if (HVal < -128) WrError(1315);
            else {
               AdrVals[0] = HVal & 0xff;
               AdrCnt = 1;
               AdrType = ModSPRel;
            }
         } else if (HVal > 127) WrError(1320);
         else if (HVal < -128) WrError(1315);
         else {
            AdrVals[0] = HVal & 0xff;
            AdrVals[1] = EvalIntExpression(p, Int8, &OK);
            if (OK) {
               AdrCnt = 2;
               AdrType = ModRegRel;
            }
         }
      }
      ChkAdr(Mask);
      return;
   }
}

static bool DecodePseudo(void) {
   bool OK;
   Byte Bit;
   Word Adr;

   if (Memo("DBIT")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         FirstPassUnknown = false;
         Bit = EvalIntExpression(ArgStr[1], UInt3, &OK);
         if ((OK) && (!FirstPassUnknown)) {
            if ((strcasecmp(ArgStr[2], "A") == 0) || (strcasecmp(ArgStr[2], "B") == 0)) {
               Adr = (*ArgStr[2]) - 'A';
               OK = true;
            } else Adr = EvalIntExpression(ArgStr[2], Int16, &OK);
            if ((OK) && (!FirstPassUnknown)) {
               PushLocHandle(-1);
               EnterIntSymbol(LabPart, (((LongInt) Bit) << 16) + Adr, SegNone, false);
               sprintf(ListLine, "=%s:%c", HexString(Adr, 0), Bit + '0');
               PopLocHandle();
            }
         }
      }
      return true;
   }

   return false;
}

static void PutCode(Word Code) {
   if (Hi(Code) == 0) {
      CodeLen = 1;
      BAsmCode[0] = Code;
   } else {
      CodeLen = 2;
      BAsmCode[0] = Hi(Code);
      BAsmCode[1] = Lo(Code);
   }
}

static void MakeCode_370(void) {
   Integer z, AdrInt;
   LongInt Bit;
   bool OK, Rela;

   CodeLen = 0;
   DontPrint = false;
   OpSize = 0;
   AddrRel = false;

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

   if (Memo("MOV")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], MModAccB + MModReg + MModPort + MModAbs + MModIReg + MModBRel + MModSPRel + MModRegRel + MModAccA);
         switch (AdrType) {
            case ModAccA:
               DecodeAdr(ArgStr[1], MModReg + MModAbs + MModIReg + MModBRel + MModRegRel + MModSPRel + MModAccB + MModPort + MModImm);
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
                  case ModRegRel:
                     BAsmCode[0] = 0xf4;
                     BAsmCode[1] = 0xea;
                     memcpy(BAsmCode + 2, AdrVals, 2);
                     CodeLen = 4;
                     break;
                  case ModSPRel:
                     BAsmCode[0] = 0xf1;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
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
               DecodeAdr(ArgStr[1], MModAccA + MModReg + MModPort + MModImm);
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
               BAsmCode[1] = BAsmCode[2] = AdrVals[0];
               DecodeAdr(ArgStr[1], MModAccA + MModAccB + MModReg + MModImm);
               switch (AdrType) {
                  case ModAccA:
                     BAsmCode[0] = 0x21;
                     CodeLen = 2;
                     break;
                  case ModAccB:
                     BAsmCode[0] = 0x51;
                     CodeLen = 2;
                     break;
                  case ModReg:
                     BAsmCode[0] = 0x71;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 3;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0xf7;
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
            case ModSPRel:
               BAsmCode[1] = AdrVals[0];
               DecodeAdr(ArgStr[1], MModAccA);
               if (AdrType != ModNone) {
                  BAsmCode[0] = 0xf2;
                  CodeLen = 2;
               }
               break;
            case ModRegRel:
               memcpy(BAsmCode + 2, AdrVals, AdrCnt);
               DecodeAdr(ArgStr[1], MModAccA);
               if (AdrType != ModNone) {
                  BAsmCode[0] = 0xf4;
                  BAsmCode[1] = 0xeb;
                  CodeLen = 4;
               }
               break;
         }
      }
      return;
   }

   if (Memo("MOVW")) {
      OpSize = 1;
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], MModReg);
         if (AdrType != ModNone) {
            z = AdrVals[0];
            DecodeAdr(ArgStr[1], MModReg + MModImm + MModImmBRel + MModImmRegRel);
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
               case ModImmRegRel:
                  BAsmCode[0] = 0xf4;
                  BAsmCode[1] = 0xe8;
                  memcpy(BAsmCode + 2, AdrVals, 2);
                  BAsmCode[4] = z;
                  CodeLen = 5;
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
            AdrInt = EvalIntExpression(ArgStr[1], Int16, &OK) - (EProgCounter() + 2);
            if (OK)
               if ((!SymbolQuestionable) && ((AdrInt > 127) || (AdrInt < -128))) WrError(1370);
               else {
                  CodeLen = 2;
                  BAsmCode[0] = Rel8Orders[z].Code;
                  BAsmCode[1] = AdrInt & 0xff;
               }
         }
         return;
      }

   if (Memo("CMP")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], MModAccA + MModAccB + MModReg);
         switch (AdrType) {
            case ModAccA:
               DecodeAdr(ArgStr[1], MModAbs + MModIReg + MModBRel + MModRegRel + MModSPRel + MModAccB + MModReg + MModImm);
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
                  case ModRegRel:
                     BAsmCode[0] = 0xf4;
                     BAsmCode[1] = 0xed;
                     memcpy(BAsmCode + 2, AdrVals, 2);
                     CodeLen = 4;
                     break;
                  case ModSPRel:
                     BAsmCode[0] = 0xf3;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
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
      if (Memo(ALU2Orders[z].Name)) {
         Rela = ((Memo("BTJO")) || (Memo("BTJZ")));
         if (((Rela) && (ArgCnt != 3))
            || ((!Rela) && (ArgCnt != 2))) WrError(1110);
         else {
            DecodeAdr(ArgStr[2], MModAccA + MModAccB + MModReg + MModPort);
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
                        break;
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
                        break;
                  }
                  break;
            }
            if ((CodeLen != 0) && (Rela)) {
               AdrInt = EvalIntExpression(ArgStr[3], UInt16, &OK) - (EProgCounter() + CodeLen + 1);
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
         AddrRel = (Memo("CALLR") || Memo("JMPL"));
         if (ArgCnt != 1) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], MModAbs + MModIReg + MModBRel + MModRegRel);
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
               case ModRegRel:
                  CodeLen = 4;
                  BAsmCode[0] = 0xf4;
                  BAsmCode[1] = 0xe0 + JmpOrders[z].Code;
                  memcpy(BAsmCode + 2, AdrVals, 2);
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
               BAsmCode[0] = 0xf3 + ABRegOrders[z].Code;
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
                  BAsmCode[CodeLen + 1] = AdrVals[0];
                  CodeLen = 2;
                  break;
            }
            if ((Memo("DJNZ")) && (CodeLen != 0)) {
               AdrInt = EvalIntExpression(ArgStr[2], Int16, &OK) - (EProgCounter() + CodeLen + 1);
               if (!OK) CodeLen = 0;
               else if ((!FirstPassUnknown) && ((AdrInt > 127) || (AdrInt < -128))) {
                  WrError(1370);
                  CodeLen = 0;
               } else BAsmCode[CodeLen++] = AdrInt & 0xff;
            }
         }
         return;
      }

   for (z = 0; z < BitOrderCount; z++)
      if (Memo(BitOrders[z].Name)) {
         Rela = ((Memo("JBIT0")) || (Memo("JBIT1")));
         if (((Rela) && (ArgCnt != 2))
            || ((!Rela) && (ArgCnt != 1))) WrError(1110);
         else {
            FirstPassUnknown = false;
            Bit = EvalIntExpression(ArgStr[1], Int32, &OK);
            if (OK) {
               if (FirstPassUnknown) Bit &= 0x000710ff;
               BAsmCode[1] = 1 << ((Bit >> 16) & 7);
               BAsmCode[2] = Lo(Bit);
               switch (Hi(Bit)) {
                  case 0:
                     BAsmCode[0] = 0x70 + BitOrders[z].Code;
                     CodeLen = 3;
                     break;
                  case 16:
                     BAsmCode[0] = 0xa0 + BitOrders[z].Code;
                     CodeLen = 3;
                     break;
                  default:
                     WrError(1350);
               }
               if ((CodeLen != 0) && (Rela)) {
                  AdrInt = EvalIntExpression(ArgStr[2], Int16, &OK) - (EProgCounter() + CodeLen + 1);
                  if (!OK) CodeLen = 0;
                  else if ((!FirstPassUnknown) && ((AdrInt > 127) || (AdrInt < -128))) {
                     WrError(1370);
                     CodeLen = 0;
                  } else BAsmCode[CodeLen++] = AdrInt & 0xff;
               }
            }
         }
         return;
      }

   if (Memo("DIV")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], MModAccA);
         if (AdrType != ModNone) {
            DecodeAdr(ArgStr[1], MModReg);
            if (AdrType != ModNone) {
               BAsmCode[0] = 0xf4;
               BAsmCode[1] = 0xf8;
               BAsmCode[2] = AdrVals[0];
               CodeLen = 3;
            }
         }
      }
      return;
   }

   if (Memo("INCW")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], MModReg);
         if (AdrType != ModNone) {
            BAsmCode[2] = AdrVals[0];
            DecodeAdr(ArgStr[1], MModImm);
            if (AdrType != ModNone) {
               BAsmCode[0] = 0x70;
               BAsmCode[1] = AdrVals[0];
               CodeLen = 3;
            }
         }
      }
      return;
   }

   if (Memo("LDST")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModImm);
         if (AdrType != ModNone) {
            BAsmCode[0] = 0xf0;
            BAsmCode[1] = AdrVals[0];
            CodeLen = 2;
         }
      }
      return;
   }

   if (Memo("TRAP")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         BAsmCode[0] = EvalIntExpression(ArgStr[1], Int4, &OK);
         if (OK) {
            BAsmCode[0] = 0xef - BAsmCode[0];
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
               BAsmCode[0] = 0xc6;
               CodeLen = 1;
               break;
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_370(void) {
   switch (ActPC) {
      case SegCode:
         return (ProgCounter() < 0x10000);
      default:
         return false;
   }
}

static bool IsDef_370(void) {
   return (Memo("DBIT"));
}

static void InternSymbol_370(char *Asc, TempResult * Erg) {
   bool err;
   String h;

   Erg->Typ = TempNone;
   if ((strlen(Asc) < 2) || ((toupper(*Asc) != 'R') && (toupper(*Asc) != 'P'))) return;

   strcpy(h, Asc + 1);
   if ((*h == '0') && (strlen(h) > 1)) *h = '$';
   Erg->Contents.Int = ConstLongInt(h, &err);
   if ((!err) || (Erg->Contents.Int < 0) || (Erg->Contents.Int > 255)) return;

   Erg->Typ = TempInt;
   if (toupper(*Asc) == 'P') Erg->Contents.Int += 0x1000;
}

static void SwitchFrom_370(void) {
   DeinitFields();
}

static void SwitchTo_370(void) {
   TurnWords = false;
   ConstMode = ConstModeIntel;
   SetIsOccupied = false;

   PCSymbol = "$";
   HeaderID = 0x49;
   NOPCode = 0xff;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_370;
   ChkPC = ChkPC_370;
   IsDef = IsDef_370;
   SwitchFrom = SwitchFrom_370;
   InternSymbol = InternSymbol_370;

   InitFields();
}

void code370_init(void) {
   CPU37010 = AddCPU("370C010", SwitchTo_370);
   CPU37020 = AddCPU("370C020", SwitchTo_370);
   CPU37030 = AddCPU("370C030", SwitchTo_370);
   CPU37040 = AddCPU("370C040", SwitchTo_370);
   CPU37050 = AddCPU("370C050", SwitchTo_370);
}
