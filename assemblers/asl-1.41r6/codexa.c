/* codexa.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegenerator Philips XA                                               */
/*                                                                           */
/* Historie: 25.10.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "stringutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

/*-------------------------------------------------------------------------*/

#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModMem 1
#define MModMem (1 << ModMem)
#define ModImm 2
#define MModImm (1 << ModImm)
#define ModAbs 3
#define MModAbs (1 << ModAbs)

#define FixedOrderCnt 5
#define JBitOrderCnt 3
#define ALUOrderCnt 8
#define RegOrderCnt 4
#define ShiftOrderCount 4
#define RotateOrderCount 4
#define RelOrderCount 17
#define StackOrderCount 4

typedef struct {
   char *Name;
   Word Code;
} FixedOrder;

typedef struct {
   char *Name;
   Byte SizeMask;
   Byte Code;
} RegOrder;

static CPUVar CPUXAG1, CPUXAG2, CPUXAG3;

static FixedOrder *FixedOrders;
static FixedOrder *JBitOrders;
static FixedOrder *StackOrders;
static char **ALUOrders;
static RegOrder *RegOrders;
static char **ShiftOrders;
static FixedOrder *RotateOrders;
static FixedOrder *RelOrders;

static LongInt Reg_DS;
static SimpProc SaveInitProc;

static ShortInt AdrMode;
static Byte AdrPart, MemPart;
static Byte AdrVals[4];
static ShortInt OpSize;

/*-------------------------------------------------------------------------*/

static void AddFixed(char *NName, Word NCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddJBit(char *NName, Word NCode) {
   if (InstrZ >= JBitOrderCnt) exit(255);
   JBitOrders[InstrZ].Name = NName;
   JBitOrders[InstrZ++].Code = NCode;
}

static void AddStack(char *NName, Word NCode) {
   if (InstrZ >= StackOrderCount) exit(255);
   StackOrders[InstrZ].Name = NName;
   StackOrders[InstrZ++].Code = NCode;
}

static void AddReg(char *NName, Byte NMask, Byte NCode) {
   if (InstrZ >= RegOrderCnt) exit(255);
   RegOrders[InstrZ].Name = NName;
   RegOrders[InstrZ].Code = NCode;
   RegOrders[InstrZ++].SizeMask = NMask;
}

static void AddRotate(char *NName, Word NCode) {
   if (InstrZ >= RotateOrderCount) exit(255);
   RotateOrders[InstrZ].Name = NName;
   RotateOrders[InstrZ++].Code = NCode;
}

static void AddRel(char *NName, Word NCode) {
   if (InstrZ >= RelOrderCount) exit(255);
   RelOrders[InstrZ].Name = NName;
   RelOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("NOP", 0x0000);
   AddFixed("RET", 0xd680);
   AddFixed("RETI", 0xd690);
   AddFixed("BKPT", 0x00ff);
   AddFixed("RESET", 0xd610);

   JBitOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * JBitOrderCnt);
   InstrZ = 0;
   AddJBit("JB", 0x80);
   AddJBit("JBC", 0xc0);
   AddJBit("JNB", 0xa0);

   StackOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * StackOrderCount);
   InstrZ = 0;
   AddStack("POP", 0x1027);
   AddStack("POPU", 0x0037);
   AddStack("PUSH", 0x3007);
   AddStack("PUSHU", 0x2017);

   ALUOrders = (char **)malloc(sizeof(char *) * ALUOrderCnt);
   InstrZ = 0;
   ALUOrders[InstrZ++] = "ADD";
   ALUOrders[InstrZ++] = "ADDC";
   ALUOrders[InstrZ++] = "SUB";
   ALUOrders[InstrZ++] = "SUBB";
   ALUOrders[InstrZ++] = "CMP";
   ALUOrders[InstrZ++] = "AND";
   ALUOrders[InstrZ++] = "OR";
   ALUOrders[InstrZ++] = "XOR";

   RegOrders = (RegOrder *) malloc(sizeof(RegOrder) * RegOrderCnt);
   InstrZ = 0;
   AddReg("NEG", 3, 0x0b);
   AddReg("CPL", 3, 0x0a);
   AddReg("SEXT", 3, 0x09);
   AddReg("DA", 1, 0x08);

   ShiftOrders = (char **)malloc(sizeof(char *) * ShiftOrderCount);
   InstrZ = 0;
   ShiftOrders[InstrZ++] = "LSR";
   ShiftOrders[InstrZ++] = "ASL";
   ShiftOrders[InstrZ++] = "ASR";
   ShiftOrders[InstrZ++] = "NORM";

   RotateOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * RotateOrderCount);
   InstrZ = 0;
   AddRotate("RR", 0xb0);
   AddRotate("RL", 0xd3);
   AddRotate("RRC", 0xb7);
   AddRotate("RLC", 0xd7);

   RelOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * RelOrderCount);
   InstrZ = 0;
   AddRel("BCC", 0xf0);
   AddRel("BCS", 0xf1);
   AddRel("BNE", 0xf2);
   AddRel("BEQ", 0xf3);
   AddRel("BNV", 0xf4);
   AddRel("BOV", 0xf5);
   AddRel("BPL", 0xf6);
   AddRel("BMI", 0xf7);
   AddRel("BG", 0xf8);
   AddRel("BL", 0xf9);
   AddRel("BGE", 0xfa);
   AddRel("BLT", 0xfb);
   AddRel("BGT", 0xfc);
   AddRel("BLE", 0xfd);
   AddRel("BR", 0xfe);
   AddRel("JZ", 0xec);
   AddRel("JNZ", 0xee);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(JBitOrders);
   free(StackOrders);
   free(ALUOrders);
   free(RegOrders);
   free(ShiftOrders);
   free(RotateOrders);
   free(RelOrders);
}

/*-------------------------------------------------------------------------*/

static void SetOpSize(ShortInt NSize) {
   if (OpSize == -1) OpSize = NSize;
   else if (OpSize != NSize) {
      AdrMode = ModNone;
      AdrCnt = 0;
      WrError(1131);
   }
}

static bool DecodeReg(char *Asc, ShortInt * NSize, Byte * Erg) {
   if (strcasecmp(Asc, "SP") == 0) {
      *Erg = 7;
      *NSize = 1;
      return true;
   } else if ((strlen(Asc) >= 2) && (toupper(*Asc) == 'R') && (Asc[1] >= '0') && (Asc[1] <= '7'))
      if (strlen(Asc) == 2) {
         *Erg = Asc[1] - '0';
         if (OpSize == 2) {
            if ((*Erg & 1) == 1) {
               WrError(1760);
               (*Erg)--;
            }
            *NSize = 2;
            return true;
         } else {
            *NSize = 1;
            return true;
         }
      } else if ((strlen(Asc) == 3) && (toupper(Asc[2]) == 'L')) {
         *Erg = (Asc[1] - '0') << 1;
         *NSize = 0;
         return true;
      } else if ((strlen(Asc) == 3) && (toupper(Asc[2]) == 'H')) {
         *Erg = ((Asc[1] - '0') << 1) + 1;
         *NSize = 0;
         return true;
      } else return false;
   return false;
}

static void ChkAdr(Word Mask) {
   if ((AdrMode != ModNone) && ((Mask & (1 << AdrMode)) == 0)) {
      WrError(1350);
      AdrMode = ModNone;
      AdrCnt = 0;
   }
}

static void DecodeAdr(char *Asc, Word Mask) {
   ShortInt NSize;
   LongInt DispAcc, DispPart, AdrLong;
   bool FirstFlag, NegFlag, NextFlag, ErrFlag, OK;
   char *PPos, *MPos;
   Word AdrInt;
   Byte Reg;
   String Part;

   AdrMode = ModNone;
   AdrCnt = 0;
   KillBlanks(Asc);

   if (DecodeReg(Asc, &NSize, &AdrPart)) {
      if ((Mask & MModReg) != 0) {
         AdrMode = ModReg;
         SetOpSize(NSize);
      } else {
         AdrMode = ModMem;
         MemPart = 1;
         SetOpSize(NSize);
      }
      ChkAdr(Mask);
      return;
   }

   if (*Asc == '#') {
      switch (OpSize) {
         case -4:
            AdrVals[0] = EvalIntExpression(Asc + 1, UInt5, &OK);
            if (OK) {
               AdrCnt = 1;
               AdrMode = ModImm;
            }
            break;
         case -3:
            AdrVals[0] = EvalIntExpression(Asc + 1, SInt4, &OK);
            if (OK) {
               AdrCnt = 1;
               AdrMode = ModImm;
            }
            break;
         case -2:
            AdrVals[0] = EvalIntExpression(Asc + 1, UInt4, &OK);
            if (OK) {
               AdrCnt = 1;
               AdrMode = ModImm;
            }
            break;
         case -1:
            WrError(1132);
            break;
         case 0:
            AdrVals[0] = EvalIntExpression(Asc + 1, Int8, &OK);
            if (OK) {
               AdrCnt = 1;
               AdrMode = ModImm;
            }
            break;
         case 1:
            AdrInt = EvalIntExpression(Asc + 1, Int16, &OK);
            if (OK) {
               AdrVals[0] = Hi(AdrInt);
               AdrVals[1] = Lo(AdrInt);
               AdrCnt = 2;
               AdrMode = ModImm;
            }
            break;
         case 2:
            AdrLong = EvalIntExpression(Asc + 1, Int32, &OK);
            if (OK) {
               AdrVals[0] = (AdrLong >> 24) & 0xff;
               AdrVals[1] = (AdrLong >> 16) & 0xff;
               AdrVals[2] = (AdrLong >> 8) & 0xff;
               AdrVals[3] = AdrLong & 0xff;
               AdrCnt = 4;
               AdrMode = ModImm;
            }
            break;
      }
      ChkAdr(Mask);
      return;
   }

   if ((*Asc == '[') && (Asc[strlen(Asc) - 1] == ']')) {
      strcpy(Asc, Asc + 1);
      Asc[strlen(Asc) - 1] = '\0';
      if (Asc[strlen(Asc) - 1] == '+') {
         Asc[strlen(Asc) - 1] = '\0';
         if (!DecodeReg(Asc, &NSize, &AdrPart)) WrXError(1445, Asc);
         else if (NSize != 1) WrError(1350);
         else {
            AdrMode = ModMem;
            MemPart = 3;
         }
      } else {
         FirstFlag = false;
         ErrFlag = false;
         DispAcc = 0;
         AdrPart = 0xff;
         NegFlag = false;
         while ((*Asc != '\0') && (!ErrFlag)) {
            PPos = QuotPos(Asc, '+');
            MPos = QuotPos(Asc, '-');
            if (PPos == NULL) PPos = MPos;
            else if ((MPos != NULL) && (PPos > MPos)) PPos = MPos;
            NextFlag = ((PPos != NULL) && (*PPos == '-'));
            if (PPos == NULL) {
               strmaxcpy(Part, Asc, 255);
               *Asc = '\0';
            } else {
               *PPos = '\0';
               strmaxcpy(Part, Asc, 255);
               strcpy(Asc, PPos + 1);
            }
            if (DecodeReg(Part, &NSize, &Reg))
               if ((NSize != 1) || (AdrPart != 0xff) || (NegFlag)) {
                  WrError(1350);
                  ErrFlag = true;
               } else AdrPart = Reg;
            else {
               FirstPassUnknown = false;
               DispPart = EvalIntExpression(Part, Int32, &ErrFlag);
               ErrFlag = !ErrFlag;
               if (!ErrFlag) {
                  FirstFlag = FirstFlag || FirstPassUnknown;
                  if (NegFlag) DispAcc -= DispPart;
                  else DispAcc += DispPart;
               }
            }
            NegFlag = NextFlag;
         }
         if (FirstFlag) DispAcc &= 0x7fff;
         if (AdrPart == 0xff) WrError(1350);
         else if (DispAcc == 0) {
            AdrMode = ModMem;
            MemPart = 2;
         } else if ((DispAcc >= -128) && (DispAcc < 127)) {
            AdrMode = ModMem;
            MemPart = 4;
            AdrVals[0] = DispAcc & 0xff;
            AdrCnt = 1;
         } else if (ChkRange(DispAcc, -0x8000l, 0x7fffl)) {
            AdrMode = ModMem;
            MemPart = 5;
            AdrVals[0] = (DispAcc >> 8) & 0xff;
            AdrVals[1] = DispAcc & 0xff;
            AdrCnt = 2;
         }
      }
      ChkAdr(Mask);
      return;
   }

   FirstPassUnknown = false;
   AdrLong = EvalIntExpression(Asc, UInt24, &OK);
   if (OK) {
      if (FirstPassUnknown) {
         if ((Mask & MModAbs) == 0) AdrLong &= 0x3ff;
      }
      if ((AdrLong & 0xffff) > 0x7ff) WrError(1925);
      else if ((AdrLong & 0xffff) <= 0x3ff) {
         if ((AdrLong >> 16) != Reg_DS) WrError(110);
         ChkSpace(SegData);
         AdrMode = ModMem;
         MemPart = 6;
         AdrPart = Hi(AdrLong);
         AdrVals[0] = Lo(AdrLong);
         AdrCnt = 1;
      } else if (AdrLong > 0x7ff) WrError(1925);
      else {
         ChkSpace(SegIO);
         AdrMode = ModMem;
         MemPart = 6;
         AdrPart = Hi(AdrLong);
         AdrVals[0] = Lo(AdrLong);
         AdrCnt = 1;
      }
   }

   ChkAdr(Mask);
}

static bool DecodeBitAddr(char *Asc, LongInt * Erg) {
   char *p;
   Byte BPos, Reg;
   ShortInt Size, Res;
   LongInt AdrLong;
   bool OK;

   p = RQuotPos(Asc, '.');
   Res = 0;
   if (p == NULL) {
      FirstPassUnknown = false;
      AdrLong = EvalIntExpression(Asc, UInt24, &OK);
      if (FirstPassUnknown) AdrLong &= 0x3ff;
      *Erg = AdrLong;
      Res = 1;
   } else {
      FirstPassUnknown = false;
      *p = '\0';
      BPos = EvalIntExpression(p + 1, UInt4, &OK);
      if (FirstPassUnknown) BPos &= 7;
      if (OK) {
         if (DecodeReg(Asc, &Size, &Reg))
            if ((Size == 0) && (BPos > 7)) WrError(1320);
            else {
               if (Size == 0) *Erg = (Reg << 3) + BPos;
               else *Erg = (Reg << 4) + BPos;
               Res = 1;
         } else if (BPos > 7) WrError(1320);
         else {
            FirstPassUnknown = false;
            AdrLong = EvalIntExpression(Asc, UInt24, &OK);
            if ((TypeFlag & (1 << SegIO)) != 0) {
               ChkSpace(SegIO);
               if (FirstPassUnknown) AdrLong = (AdrLong & 0x3f) | 0x400;
               if (ChkRange(AdrLong, 0x400, 0x43f)) {
                  *Erg = 0x200 + ((AdrLong & 0x3f) << 3) + BPos;
                  Res = 1;
               } else Res = (-1);
            } else {
               ChkSpace(SegData);
               if (FirstPassUnknown) AdrLong = (AdrLong & 0x00ff003f) | 0x20;
               if (ChkRange(AdrLong & 0xff, 0x20, 0x3f)) {
                  *Erg = 0x100 + ((AdrLong & 0x1f) << 3) + BPos + (AdrLong & 0xff0000);
                  Res = 1;
               } else Res = (-1);
            }
         }
      }
      *p = '.';
   }
   if (Res == 0) WrError(1350);
   return (Res == 1);
}

static void ChkBitPage(LongInt Adr) {
   if ((Adr >> 16) != Reg_DS) WrError(110);
}

/*-------------------------------------------------------------------------*/

static bool DecodePseudo(void) {
#define ASSUMEXACount 1
   static ASSUMERec ASSUMEXAs[ASSUMEXACount] = { { "DS", &Reg_DS, 0, 0xff, 0x100 } };
#define ONOFFXACount 1
   static ONOFFRec ONOFFXAs[ONOFFXACount] = { { "SUPMODE", &SupAllowed, SupAllowedName } };

   LongInt BAdr;

   if (Memo("PORT")) {
      CodeEquate(SegIO, 0x400, 0x7ff);
      return true;
   }

   if (Memo("BIT")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (DecodeBitAddr(ArgStr[1], &BAdr)) {
         EnterIntSymbol(LabPart, BAdr, SegNone, false);
         switch ((BAdr & 0x3ff) >> 8) {
            case 0:
               sprintf(ListLine, "=R%d.%d", (BAdr >> 4) & 15, BAdr & 15);
               break;
            case 1:
               sprintf(ListLine, "=%x:%x.%d", (BAdr >> 16) & 255, (BAdr & 0x1f8) >> 3, BAdr & 7);
               break;
            default:
               sprintf(ListLine, "=S:%x.%d", ((BAdr >> 3) & 0x3f) + 0x400, BAdr & 7);
               break;
         }
      }
      return true;
   }

   if (CodeONOFF(ONOFFXAs, ONOFFXACount)) return true;

   if (Memo("ASSUME")) {
      CodeASSUME(ASSUMEXAs, ASSUMEXACount);
      return true;
   }

   return false;
}

static bool IsRealDef(void) {
   return ((Memo("PORT")) || (Memo("BIT")));
}

static void ForceAlign(void) {
   if ((EProgCounter() & 1) == 1) {
      BAsmCode[0] = NOPCode;
      CodeLen = 1;
   }
}

static void MakeCode_XA(void) {
   Byte HReg, HMem, HCnt, HPart;
   Byte HVals[3];
   Integer z, i;
   Word Mask;
   LongInt AdrLong;
   bool OK;

   CodeLen = 0;
   DontPrint = false;
   OpSize = (-1);

/* Operandengroesse */

   if (*AttrPart != '\0')
      switch (toupper(*AttrPart)) {
         case 'B':
            SetOpSize(0);
            break;
         case 'W':
            SetOpSize(1);
            break;
         case 'D':
            SetOpSize(2);
            break;
         default:
            WrError(1107);
            return;
      }

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

/* Labels muessen auf geraden Adressen liegen */

   if ((ActPC == SegCode) && (!IsRealDef()) && (*LabPart != '\0')) {
      ForceAlign();
      EnterIntSymbol(LabPart, EProgCounter() + CodeLen, ActPC, false);
   }

   if (DecodeMoto16Pseudo(OpSize, false)) return;
   if (DecodeIntelPseudo(false)) return;

/* zu ignorierendes */

   if (Memo("")) return;

/* Anweisungen ohne Operanden */

   for (z = 0; z < FixedOrderCnt; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else {
            if (Hi(FixedOrders[z].Code) != 0) BAsmCode[CodeLen++] = Hi(FixedOrders[z].Code);
            BAsmCode[CodeLen++] = Lo(FixedOrders[z].Code);
            if ((Memo("RETI")) && (!SupAllowed)) WrError(50);
         }
         return;
      }

/* Datentransfer */

   if (Memo("MOV")) {
      if (ArgCnt != 2) WrError(1110);
      else if (strcasecmp(ArgStr[1], "C") == 0) {
         if (DecodeBitAddr(ArgStr[2], &AdrLong))
            if (*AttrPart != '\0') WrError(1100);
            else {
               ChkBitPage(AdrLong);
               BAsmCode[CodeLen++] = 0x08;
               BAsmCode[CodeLen++] = 0x20 + Hi(AdrLong);
               BAsmCode[CodeLen++] = Lo(AdrLong);
            }
      } else if (strcasecmp(ArgStr[2], "C") == 0) {
         if (DecodeBitAddr(ArgStr[1], &AdrLong))
            if (*AttrPart != '\0') WrError(1100);
            else {
               ChkBitPage(AdrLong);
               BAsmCode[CodeLen++] = 0x08;
               BAsmCode[CodeLen++] = 0x30 + Hi(AdrLong);
               BAsmCode[CodeLen++] = Lo(AdrLong);
            }
      } else if (strcasecmp(ArgStr[1], "USP") == 0) {
         SetOpSize(1);
         DecodeAdr(ArgStr[2], MModReg);
         if (AdrMode == ModReg) {
            BAsmCode[CodeLen++] = 0x98;
            BAsmCode[CodeLen++] = (AdrPart << 4) + 0x0f;
         }
      } else if (strcasecmp(ArgStr[2], "USP") == 0) {
         SetOpSize(1);
         DecodeAdr(ArgStr[1], MModReg);
         if (AdrMode == ModReg) {
            BAsmCode[CodeLen++] = 0x90;
            BAsmCode[CodeLen++] = (AdrPart << 4) + 0x0f;
         }
      } else {
         DecodeAdr(ArgStr[1], MModReg + MModMem);
         switch (AdrMode) {
            case ModReg:
               if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
               else {
                  HReg = AdrPart;
                  DecodeAdr(ArgStr[2], MModMem + MModImm);
                  switch (AdrMode) {
                     case ModMem:
                        BAsmCode[CodeLen++] = 0x80 + (OpSize << 3) + MemPart;
                        BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                        memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                        CodeLen += AdrCnt;
                        if ((MemPart == 3) && ((HReg >> (1 - OpSize)) == AdrPart)) WrError(140);
                        break;
                     case ModImm:
                        BAsmCode[CodeLen++] = 0x91 + (OpSize << 3);
                        BAsmCode[CodeLen++] = 0x08 + (HReg << 4);
                        memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                        CodeLen += AdrCnt;
                        break;
                  }
               }
               break;
            case ModMem:
               memcpy(HVals, AdrVals, AdrCnt);
               HCnt = AdrCnt;
               HPart = MemPart;
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], MModReg + MModMem + MModImm);
               switch (AdrMode) {
                  case ModReg:
                     if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
                     else {
                        BAsmCode[CodeLen++] = 0x80 + (OpSize << 3) + HPart;
                        BAsmCode[CodeLen++] = (AdrPart << 4) + 0x08 + HReg;
                        memcpy(BAsmCode + CodeLen, HVals, HCnt);
                        CodeLen += HCnt;
                        if ((HPart == 3) && ((AdrPart >> (1 - OpSize)) == HReg)) WrError(140);
                     }
                     break;
                  case ModMem:
                     if (OpSize == -1) WrError(1132);
                     else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
                     else if ((HPart == 6) && (MemPart == 6)) {
                        BAsmCode[CodeLen++] = 0x97 + (OpSize << 3);
                        BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                        BAsmCode[CodeLen++] = HVals[0];
                        BAsmCode[CodeLen++] = AdrVals[0];
                     } else if ((HPart == 6) && (MemPart == 2)) {
                        BAsmCode[CodeLen++] = 0xa0 + (OpSize << 3);
                        BAsmCode[CodeLen++] = 0x80 + (AdrPart << 4) + HReg;
                        BAsmCode[CodeLen++] = HVals[0];
                     } else if ((HPart == 2) && (MemPart == 6)) {
                        BAsmCode[CodeLen++] = 0xa0 + (OpSize << 3);
                        BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                        BAsmCode[CodeLen++] = AdrVals[0];
                     } else if ((HPart == 3) && (MemPart == 3)) {
                        BAsmCode[CodeLen++] = 0x90 + (OpSize << 3);
                        BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                        if (HReg == AdrPart) WrError(140);
                     } else WrError(1350);
                     break;
                  case ModImm:
                     if (OpSize == -1) WrError(1132);
                     else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
                     else {
                        BAsmCode[CodeLen++] = 0x90 + (OpSize << 3) + HPart;
                        BAsmCode[CodeLen++] = 0x08 + (HReg << 4);
                        memcpy(BAsmCode + CodeLen, HVals, HCnt);
                        memcpy(BAsmCode + CodeLen + HCnt, AdrVals, AdrCnt);
                        CodeLen += HCnt + AdrCnt;
                     }
                     break;
               }
               break;
         }
      }
      return;
   }

   if (Memo("MOVC")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         if ((*AttrPart == '\0') && (strcasecmp(ArgStr[1], "A") == 0)) OpSize = 0;
         if (strcasecmp(ArgStr[2], "[A+DPTR]") == 0)
            if (strcasecmp(ArgStr[1], "A") != 0) WrError(1350);
            else if (OpSize != 0) WrError(1130);
            else {
               BAsmCode[CodeLen++] = 0x90;
               BAsmCode[CodeLen++] = 0x4e;
         } else if (strcasecmp(ArgStr[2], "[A+PC]") == 0)
            if (strcasecmp(ArgStr[1], "A") != 0) WrError(1350);
            else if (OpSize != 0) WrError(1130);
            else {
               BAsmCode[CodeLen++] = 0x90;
               BAsmCode[CodeLen++] = 0x4c;
         } else {
            DecodeAdr(ArgStr[1], MModReg);
            if (AdrMode != ModNone)
               if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
               else {
                  HReg = AdrPart;
                  DecodeAdr(ArgStr[2], MModMem);
                  if (AdrMode != ModNone)
                     if (MemPart != 3) WrError(1350);
                     else {
                        BAsmCode[CodeLen++] = 0x80 + (OpSize << 3);
                        BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                        if ((MemPart == 3) && ((HReg >> (1 - OpSize)) == AdrPart)) WrError(140);
                     }
               }
         }
      }
      return;
   }

   if (Memo("MOVX")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModMem);
         if (AdrMode == ModMem)
            switch (MemPart) {
               case 1:
                  if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
                  else {
                     HReg = AdrPart;
                     DecodeAdr(ArgStr[2], MModMem);
                     if (AdrMode == ModMem)
                        if (MemPart != 2) WrError(1350);
                        else {
                           BAsmCode[CodeLen++] = 0xa7 + (OpSize << 3);
                           BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                        }
                  }
                  break;
               case 2:
                  HReg = AdrPart;
                  DecodeAdr(ArgStr[2], MModReg);
                  if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
                  else {
                     BAsmCode[CodeLen++] = 0xa7 + (OpSize << 3);
                     BAsmCode[CodeLen++] = 0x08 + (AdrPart << 4) + HReg;
                  }
                  break;
               default:
                  WrError(1350);
            }
      }
      return;
   }

   for (z = 0; z < StackOrderCount; z++)
      if (Memo(StackOrders[z].Name)) {
         if (ArgCnt < 1) WrError(1110);
         else {
            HReg = 0xff;
            OK = true;
            Mask = 0;
            for (i = 1; i <= ArgCnt; i++)
               if (OK) {
                  DecodeAdr(ArgStr[i], MModMem);
                  if (AdrMode == ModNone) OK = false;
                  else switch (MemPart) {
                        case 1:
                           if (HReg == 0) {
                              WrError(1350);
                              OK = false;
                           } else {
                              HReg = 1;
                              Mask |= (1 << AdrPart);
                           }
                           break;
                        case 6:
                           if (HReg != 0xff) {
                              WrError(1350);
                              OK = false;
                           } else HReg = 0;
                           break;
                        default:
                           WrError(1350);
                           OK = false;
                  }
               }
            if (OK)
               if (OpSize == -1) WrError(1132);
               else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
               else if (HReg == 0) {
                  BAsmCode[CodeLen++] = 0x87 + (OpSize << 3);
                  BAsmCode[CodeLen++] = Hi(StackOrders[z].Code) + AdrPart;
                  BAsmCode[CodeLen++] = AdrVals[0];
               } else if (z < 2) { /* POP: obere Register zuerst */
                  if (Hi(Mask) != 0) {
                     BAsmCode[CodeLen++] = Lo(StackOrders[z].Code) + (OpSize << 3) + 0x40;
                     BAsmCode[CodeLen++] = Hi(Mask);
                  }
                  if (Lo(Mask) != 0) {
                     BAsmCode[CodeLen++] = Lo(StackOrders[z].Code) + (OpSize << 3);
                     BAsmCode[CodeLen++] = Lo(Mask);
                  }
                  if ((OpSize == 1) && (Memo("POP")) && ((Mask & 0x80) != 0)) WrError(140);
               } else { /* PUSH: untere Register zuerst */
               if (Lo(Mask) != 0) {
                  BAsmCode[CodeLen++] = Lo(StackOrders[z].Code) + (OpSize << 3);
                  BAsmCode[CodeLen++] = Lo(Mask);
               }
               if (Hi(Mask) != 0) {
                  BAsmCode[CodeLen++] = Lo(StackOrders[z].Code) + (OpSize << 3) + 0x40;
                  BAsmCode[CodeLen++] = Hi(Mask);
               }
               }
         }
         return;
      }

   if (Memo("XCH")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModMem);
         if (AdrMode == ModMem)
            switch (MemPart) {
               case 1:
                  HReg = AdrPart;
                  DecodeAdr(ArgStr[2], MModMem);
                  if (AdrMode == ModMem)
                     if ((OpSize != 1) && (OpSize != 0)) WrError(1130);
                     else switch (MemPart) {
                           case 1:
                              BAsmCode[CodeLen++] = 0x60 + (OpSize << 3);
                              BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                              if (HReg == AdrPart) WrError(140);
                              break;
                           case 2:
                              BAsmCode[CodeLen++] = 0x50 + (OpSize << 3);
                              BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                              break;
                           case 6:
                              BAsmCode[CodeLen++] = 0xa0 + (OpSize << 3);
                              BAsmCode[CodeLen++] = 0x08 + (HReg << 4) + AdrPart;
                              BAsmCode[CodeLen++] = AdrVals[0];
                              break;
                           default:
                              WrError(1350);
                     }
                  break;
               case 2:
                  HReg = AdrPart;
                  DecodeAdr(ArgStr[2], MModReg);
                  if (AdrMode == ModReg)
                     if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
                     else {
                        BAsmCode[CodeLen++] = 0x50 + (OpSize << 3);
                        BAsmCode[CodeLen++] = (AdrPart << 4) + HReg;
                     }
                  break;
               case 6:
                  HPart = AdrPart;
                  HVals[0] = AdrVals[0];
                  DecodeAdr(ArgStr[2], MModReg);
                  if (AdrMode == ModReg)
                     if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
                     else {
                        BAsmCode[CodeLen++] = 0xa0 + (OpSize << 3);
                        BAsmCode[CodeLen++] = 0x08 + (AdrPart << 4) + HPart;
                        BAsmCode[CodeLen++] = HVals[0];
                     }
                  break;
               default:
                  WrError(1350);
            }
      }
      return;
   }

/* Arithmetik */

   for (z = 0; z < ALUOrderCnt; z++)
      if (Memo(ALUOrders[z])) {
         if (ArgCnt != 2) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], MModReg + MModMem);
            switch (AdrMode) {
               case ModReg:
                  if (OpSize >= 2) WrError(1130);
                  else if (OpSize == -1) WrError(1132);
                  else {
                     HReg = AdrPart;
                     DecodeAdr(ArgStr[2], MModMem + MModImm);
                     switch (AdrMode) {
                        case ModMem:
                           BAsmCode[CodeLen++] = (z << 4) + (OpSize << 3) + MemPart;
                           BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                           memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                           CodeLen += AdrCnt;
                           if ((MemPart == 3) && ((HReg >> (1 - OpSize)) == AdrPart)) WrError(140);
                           break;
                        case ModImm:
                           BAsmCode[CodeLen++] = 0x91 + (OpSize << 3);
                           BAsmCode[CodeLen++] = (HReg << 4) + z;
                           memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                           CodeLen += AdrCnt;
                           break;
                     }
                  }
                  break;
               case ModMem:
                  HReg = AdrPart;
                  HMem = MemPart;
                  HCnt = AdrCnt;
                  memcpy(HVals, AdrVals, AdrCnt);
                  DecodeAdr(ArgStr[2], MModReg + MModImm);
                  switch (AdrMode) {
                     case ModReg:
                        if (OpSize == 2) WrError(1130);
                        else if (OpSize == -1) WrError(1132);
                        else {
                           BAsmCode[CodeLen++] = (z << 4) + (OpSize << 3) + HMem;
                           BAsmCode[CodeLen++] = (AdrPart << 4) + 8 + HReg;
                           memcpy(BAsmCode + CodeLen, HVals, HCnt);
                           CodeLen += HCnt;
                           if ((HMem == 3) && ((AdrPart >> (1 - OpSize)) == HReg)) WrError(140);
                        }
                        break;
                     case ModImm:
                        if (OpSize == 2) WrError(1130);
                        else if (OpSize == -1) WrError(1132);
                        else {
                           BAsmCode[CodeLen++] = 0x90 + HMem + (OpSize << 3);
                           BAsmCode[CodeLen++] = (HReg << 4) + z;
                           memcpy(BAsmCode + CodeLen, HVals, HCnt);
                           memcpy(BAsmCode + CodeLen + HCnt, AdrVals, AdrCnt);
                           CodeLen += AdrCnt + HCnt;
                        }
                        break;
                  }
                  break;
            }
         }
         return;
      }

   for (z = 0; z < RegOrderCnt; z++)
      if (Memo(RegOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], MModReg);
            switch (AdrMode) {
               case ModReg:
                  if ((RegOrders[z].SizeMask & (1 << OpSize)) == 0) WrError(1130);
                  else {
                     BAsmCode[CodeLen++] = 0x90 + (OpSize << 3);
                     BAsmCode[CodeLen++] = (AdrPart << 4) + RegOrders[z].Code;
                  }
                  break;
            }
         }
         return;
      }

   if ((Memo("ADDS")) || (Memo("MOVS"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         HMem = OpSize;
         OpSize = (-3);
         DecodeAdr(ArgStr[2], MModImm);
         switch (AdrMode) {
            case ModImm:
               HReg = AdrVals[0];
               OpSize = HMem;
               DecodeAdr(ArgStr[1], MModMem);
               switch (AdrMode) {
                  case ModMem:
                     if (OpSize == 2) WrError(1130);
                     else if (OpSize == -1) WrError(1132);
                     else {
                        BAsmCode[CodeLen++] = 0xa0 + (Memo("MOVS") << 4) + (OpSize << 3) + MemPart;
                        BAsmCode[CodeLen++] = (AdrPart << 4) + (HReg & 0x0f);
                        memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                        CodeLen += AdrCnt;
                     }
                     break;
               }
               break;
         }
      }
      return;
   }

   if (Memo("DIV")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg);
         if (AdrMode == ModReg)
            if ((OpSize != 1) && (OpSize != 2)) WrError(1130);
            else {
               HReg = AdrPart;
               OpSize--;
               DecodeAdr(ArgStr[2], MModReg + MModImm);
               switch (AdrMode) {
                  case ModReg:
                     BAsmCode[CodeLen++] = 0xe7 + (OpSize << 3);
                     BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                     break;
                  case ModImm:
                     BAsmCode[CodeLen++] = 0xe8 + OpSize;
                     BAsmCode[CodeLen++] = (HReg << 4) + 0x0b - (OpSize << 1);
                     memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                     CodeLen += AdrCnt;
                     break;
               }
            }
      }
      return;
   }

   if (Memo("DIVU")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg);
         if (AdrMode == ModReg)
            if ((OpSize == 0) && ((AdrPart & 1) == 1)) WrError(1445);
            else {
               HReg = AdrPart;
               z = OpSize;
               if (OpSize != 0) OpSize--;
               DecodeAdr(ArgStr[2], MModReg + MModImm);
               switch (AdrMode) {
                  case ModReg:
                     BAsmCode[CodeLen++] = 0xe1 + (z << 2);
                     if (z == 2) BAsmCode[CodeLen - 1] += 4;
                     BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                     break;
                  case ModImm:
                     BAsmCode[CodeLen++] = 0xe8 + (z == 2);
                     BAsmCode[CodeLen++] = (HReg << 4) + 0x01 + ((z == 1) << 1);
                     memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                     CodeLen += AdrCnt;
                     break;
               }
            }
      }
      return;
   }

   if (Memo("MUL")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg);
         if (AdrMode == ModReg)
            if (OpSize != 1) WrError(1130);
            else if ((AdrPart & 1) == 1) WrError(1445);
            else {
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], MModReg + MModImm);
               switch (AdrMode) {
                  case ModReg:
                     BAsmCode[CodeLen++] = 0xe6;
                     BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                     break;
                  case ModImm:
                     BAsmCode[CodeLen++] = 0xe9;
                     BAsmCode[CodeLen++] = (HReg << 4) + 0x08;
                     memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                     CodeLen += AdrCnt;
                     break;
               }
            }
      }
      return;
   }

   if (Memo("MULU")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg);
         if (AdrMode == ModReg)
            if ((AdrPart & 1) == 1) WrError(1445);
            else {
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], MModReg + MModImm);
               switch (AdrMode) {
                  case ModReg:
                     BAsmCode[CodeLen++] = 0xe0 + (OpSize << 2);
                     BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                     break;
                  case ModImm:
                     BAsmCode[CodeLen++] = 0xe8 + OpSize;
                     BAsmCode[CodeLen++] = (HReg << 4);
                     memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                     CodeLen += AdrCnt;
                     break;
               }
            }
      }
      return;
   }

   if (Memo("LEA")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg);
         if (AdrMode == ModReg)
            if (OpSize != 1) WrError(1130);
            else {
               HReg = AdrPart;
               strmaxprep(ArgStr[2], "[", 255);
               strmaxcat(ArgStr[2], "]", 255);
               DecodeAdr(ArgStr[2], MModMem);
               if (AdrMode == ModMem)
                  switch (MemPart) {
                     case 4:
                     case 5:
                        BAsmCode[CodeLen++] = 0x20 + (MemPart << 3);
                        BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                        memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                        CodeLen += AdrCnt;
                        break;
                     default:
                        WrError(1350);
                  }
            }
      }
      return;
   }

/* Logik */

   if ((Memo("ANL")) || (Memo("ORL"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (strcasecmp(ArgStr[1], "C") != 0) WrError(1350);
      else {
         if (*ArgStr[2] == '/') {
            OK = true;
            strcpy(ArgStr[2], ArgStr[2] + 1);
         } else OK = false;
         if (DecodeBitAddr(ArgStr[2], &AdrLong)) {
            ChkBitPage(AdrLong);
            BAsmCode[CodeLen++] = 0x08;
            BAsmCode[CodeLen++] = 0x40 + (Memo("ORL") << 5) + (OK << 4) + (Hi(AdrLong) & 3);
            BAsmCode[CodeLen++] = Lo(AdrLong);
         }
      }
      return;
   }

   if ((Memo("CLR")) || (Memo("SETB"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (DecodeBitAddr(ArgStr[1], &AdrLong)) {
         ChkBitPage(AdrLong);
         BAsmCode[CodeLen++] = 0x08;
         BAsmCode[CodeLen++] = (Memo("SETB") << 4) + (Hi(AdrLong) & 3);
         BAsmCode[CodeLen++] = Lo(AdrLong);
      }
      return;
   }

   for (z = 0; z < ShiftOrderCount; z++)
      if Memo
         (ShiftOrders[z]) {
         if (ArgCnt != 2) WrError(1110);
         else if (OpSize > 2) WrError(1130);
         else {
            DecodeAdr(ArgStr[1], MModReg);
            switch (AdrMode) {
               case ModReg:
                  HReg = AdrPart;
                  HMem = OpSize;
                  if (*ArgStr[2] == '#') OpSize = (HMem == 2) ? -4 : -2;
                  else OpSize = 0;
                  DecodeAdr(ArgStr[2], MModReg + ((z == 3) ? 0 : MModImm));
                  switch (AdrMode) {
                     case ModReg:
                        BAsmCode[CodeLen++] = 0xc0 + ((HMem & 1) << 3) + z;
                        if (HMem == 2) BAsmCode[CodeLen - 1] += 12;
                        BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                        if (Memo("NORM"))
                           if (HMem == 2) {
                              if ((AdrPart >> 2) == (HReg >> 1)) WrError(140);
                           } else if ((AdrPart >> HMem) == HReg) WrError(140);
                        break;
                     case ModImm:
                        BAsmCode[CodeLen++] = 0xd0 + ((HMem & 1) << 3) + z;
                        if (HMem == 2) {
                           BAsmCode[CodeLen - 1] += 12;
                           BAsmCode[CodeLen++] = ((HReg & 14) << 4) + AdrVals[0];
                        } else BAsmCode[CodeLen++] = (HReg << 4) + AdrVals[0];
                        break;
                  }
                  break;
            }
         }
         return;
         }

   for (z = 0; z < RotateOrderCount; z++)
      if (Memo(RotateOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], MModReg);
            switch (AdrMode) {
               case ModReg:
                  if (OpSize == 2) WrError(1130);
                  else {
                     HReg = AdrPart;
                     HMem = OpSize;
                     OpSize = (-2);
                     DecodeAdr(ArgStr[2], MModImm);
                     switch (AdrMode) {
                        case ModImm:
                           BAsmCode[CodeLen++] = RotateOrders[z].Code + (HMem << 3);
                           BAsmCode[CodeLen++] = (HReg << 4) + AdrVals[0];
                           break;
                     }
                  }
                  break;
            }
         }
         return;
      }

/* vermischtes */

   if (Memo("TRAP")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else {
         OpSize = (-2);
         DecodeAdr(ArgStr[1], MModImm);
         switch (AdrMode) {
            case ModImm:
               BAsmCode[CodeLen++] = 0xd6;
               BAsmCode[CodeLen++] = 0x30 + AdrVals[0];
               break;
         }
      }
      return;
   }

/* Spruenge */

   for (z = 0; z < RelOrderCount; z++)
      if (Memo(RelOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (*AttrPart != '\0') WrError(1100);
         else {
            FirstPassUnknown = true;
            AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
            if (OK) {
               ChkSpace(SegCode);
#ifdef __STDC__
               if (FirstPassUnknown) AdrLong &= 0xfffffffeu;
#else
               if (FirstPassUnknown) AdrLong &= 0xfffffffe;
#endif
               AdrLong -= (EProgCounter() + CodeLen + 2) & 0xfffffe;
               if ((!SymbolQuestionable) && ((AdrLong > 254) || (AdrLong < -256))) WrError(1370);
               else if ((AdrLong & 1) == 1) WrError(1325);
               else {
                  BAsmCode[CodeLen++] = RelOrders[z].Code;
                  BAsmCode[CodeLen++] = (AdrLong >> 1) & 0xff;
               }
            }
         }
         return;
      }

   if (Memo("CALL")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (*ArgStr[1] == '[') {
         DecodeAdr(ArgStr[1], MModMem);
         if (AdrMode != ModNone)
            if (MemPart != 2) WrError(1350);
            else {
               BAsmCode[CodeLen++] = 0xc6;
               BAsmCode[CodeLen++] = AdrPart;
            }
      } else {
         FirstPassUnknown = false;
         AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
         if (OK) {
            ChkSpace(SegCode);
#ifdef __STDC__
            if (FirstPassUnknown) AdrLong &= 0xfffffffeu;
#else
            if (FirstPassUnknown) AdrLong &= 0xfffffffe;
#endif
            AdrLong -= (EProgCounter() + CodeLen + 3) & 0xfffffe;
            if ((!SymbolQuestionable) && ((AdrLong > 65534) || (AdrLong < -65536))) WrError(1370);
            else if ((AdrLong & 1) == 1) WrError(1325);
            else {
               AdrLong >>= 1;
               BAsmCode[CodeLen++] = 0xc5;
               BAsmCode[CodeLen++] = (AdrLong >> 8) & 0xff;
               BAsmCode[CodeLen++] = AdrLong & 0xff;
            }
         }
      }
      return;
   }

   if (Memo("JMP")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (strcasecmp(ArgStr[1], "[A+DPTR]") == 0) {
         BAsmCode[CodeLen++] = 0xd6;
         BAsmCode[CodeLen++] = 0x46;
      } else if (strncmp(ArgStr[1], "[[", 2) == 0) {
         ArgStr[1][strlen(ArgStr[1]) - 1] = '\0';
         DecodeAdr(ArgStr[1] + 1, MModMem);
         if (AdrMode == ModMem)
            switch (MemPart) {
               case 3:
                  BAsmCode[CodeLen++] = 0xd6;
                  BAsmCode[CodeLen++] = 0x60 + AdrPart;
                  break;
               default:
                  WrError(1350);
            }
      } else if (*ArgStr[1] == '[') {
         DecodeAdr(ArgStr[1], MModMem);
         if (AdrMode == ModMem)
            switch (MemPart) {
               case 2:
                  BAsmCode[CodeLen++] = 0xd6;
                  BAsmCode[CodeLen++] = 0x70 + AdrPart;
                  break;
               default:
                  WrError(1350);
            }
      } else {
         FirstPassUnknown = false;
         AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
         if (OK) {
            ChkSpace(SegCode);
#ifdef __STDC__
            if (FirstPassUnknown) AdrLong &= 0xfffffffeu;
#else
            if (FirstPassUnknown) AdrLong &= 0xfffffffe;
#endif
            AdrLong -= (EProgCounter() + CodeLen + 3) & 0xfffffe;
            if ((!SymbolQuestionable) && ((AdrLong > 65534) || (AdrLong < -65536))) WrError(1370);
            else if ((AdrLong & 1) == 1) WrError(1325);
            else {
               AdrLong >>= 1;
               BAsmCode[CodeLen++] = 0xd5;
               BAsmCode[CodeLen++] = (AdrLong >> 8) & 0xff;
               BAsmCode[CodeLen++] = AdrLong & 0xff;
            }
         }
      }
      return;
   }

   if (Memo("CJNE")) {
      if (ArgCnt != 3) WrError(1110);
      else {
         FirstPassUnknown = false;
         AdrLong = EvalIntExpression(ArgStr[3], UInt24, &OK);
         if (FirstPassUnknown) AdrLong &= 0xfffffe;
         if (OK) {
            ChkSpace(SegCode);
            OK = false;
            HReg = 0;
            DecodeAdr(ArgStr[1], MModMem);
            if (AdrMode == ModMem)
               switch (MemPart) {
                  case 1:
                     if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
                     else {
                        HReg = AdrPart;
                        DecodeAdr(ArgStr[2], MModMem + MModImm);
                        switch (AdrMode) {
                           case ModMem:
                              if (MemPart != 6) WrError(1350);
                              else {
                                 BAsmCode[CodeLen] = 0xe2 + (OpSize << 3);
                                 BAsmCode[CodeLen + 1] = (HReg << 4) + AdrPart;
                                 BAsmCode[CodeLen + 2] = AdrVals[0];
                                 HReg = CodeLen + 3;
                                 CodeLen += 4;
                                 OK = true;
                              }
                              break;
                           case ModImm:
                              BAsmCode[CodeLen] = 0xe3 + (OpSize << 3);
                              BAsmCode[CodeLen + 1] = HReg << 4;
                              HReg = CodeLen + 2;
                              memcpy(BAsmCode + CodeLen + 3, AdrVals, AdrCnt);
                              CodeLen += 3 + AdrCnt;
                              OK = true;
                              break;
                        }
                     }
                     break;
                  case 2:
                     if ((OpSize != -1) && (OpSize != 0) && (OpSize != 1)) WrError(1130);
                     else {
                        HReg = AdrPart;
                        DecodeAdr(ArgStr[2], MModImm);
                        if (AdrMode == ModImm) {
                           BAsmCode[CodeLen] = 0xe3 + (OpSize << 3);
                           BAsmCode[CodeLen + 1] = (HReg << 4) + 8;
                           HReg = CodeLen + 2;
                           memcpy(BAsmCode + CodeLen + 3, AdrVals, AdrCnt);
                           CodeLen += 3 + AdrCnt;
                           OK = true;
                        }
                     }
                     break;
                  default:
                     WrError(1350);
               }
            if (OK) {
               AdrLong -= (EProgCounter() + CodeLen) & 0xfffffe;
               OK = false;
               if ((!SymbolQuestionable) && ((AdrLong > 254) || (AdrLong < -256))) WrError(1370);
               else if ((AdrLong & 1) == 1) WrError(1325);
               else {
                  BAsmCode[HReg] = (AdrLong >> 1) & 0xff;
                  OK = true;
               }
            }
            if (!OK) CodeLen = 0;
         }
      }
      return;
   }

   if (Memo("DJNZ")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         FirstPassUnknown = false;
         AdrLong = EvalIntExpression(ArgStr[2], UInt24, &OK);
         if (FirstPassUnknown) AdrLong &= 0xfffffe;
         if (OK) {
            ChkSpace(SegCode);
            HReg = 0;
            DecodeAdr(ArgStr[1], MModMem);
            OK = false;
            DecodeAdr(ArgStr[1], MModMem);
            if (AdrMode == ModMem)
               switch (MemPart) {
                  case 1:
                     if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
                     else {
                        BAsmCode[CodeLen] = 0x87 + (OpSize << 3);
                        BAsmCode[CodeLen + 1] = (AdrPart << 4) + 0x08;
                        HReg = CodeLen + 2;
                        CodeLen += 3;
                        OK = true;
                     }
                     break;
                  case 6:
                     if (OpSize == -1) WrError(1132);
                     else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
                     else {
                        BAsmCode[CodeLen] = 0xe2 + (OpSize << 3);
                        BAsmCode[CodeLen + 1] = 0x08 + AdrPart;
                        BAsmCode[CodeLen + 2] = AdrVals[0];
                        HReg = CodeLen + 3;
                        CodeLen += 4;
                        OK = true;
                     }
                     break;
                  default:
                     WrError(1350);
               }
            if (OK) {
               AdrLong -= (EProgCounter() + CodeLen) & 0xfffffe;
               OK = false;
               if ((!SymbolQuestionable) && ((AdrLong > 254) || (AdrLong < -256))) WrError(1370);
               else if ((AdrLong & 1) == 1) WrError(1325);
               else {
                  BAsmCode[HReg] = (AdrLong >> 1) & 0xff;
                  OK = true;
               }
            }
            if (!OK) CodeLen = 0;
         }
      }
      return;
   }

   if ((Memo("FCALL")) || (Memo("FJMP"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else {
         FirstPassUnknown = false;
         AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
         if (FirstPassUnknown) AdrLong &= 0xfffffe;
         if (OK)
            if ((AdrLong & 1) == 1) WrError(1325);
            else {
               BAsmCode[CodeLen++] = 0xc4 + (Memo("FJMP") << 4);
               BAsmCode[CodeLen++] = (AdrLong >> 8) & 0xff;
               BAsmCode[CodeLen++] = AdrLong & 0xff;
               BAsmCode[CodeLen++] = (AdrLong >> 16) & 0xff;
            }
      }
      return;
   }

   for (z = 0; z < JBitOrderCnt; z++)
      if (Memo(JBitOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (*AttrPart != '\0') WrError(1100);
         else if (DecodeBitAddr(ArgStr[1], &AdrLong)) {
            BAsmCode[CodeLen] = 0x97;
            BAsmCode[CodeLen + 1] = JBitOrders[z].Code + Hi(AdrLong);
            BAsmCode[CodeLen + 2] = Lo(AdrLong);
            FirstPassUnknown = false;
            AdrLong = EvalIntExpression(ArgStr[2], UInt24, &OK);
            if (FirstPassUnknown) AdrLong &= 0xfffffe;
            AdrLong -= (EProgCounter() + CodeLen + 4) & 0xfffffe;
            if (OK)
               if ((!SymbolQuestionable) && ((AdrLong > 254) || (AdrLong < -256))) WrError(1370);
               else if ((AdrLong & 1) == 1) WrError(1325);
               else {
                  BAsmCode[CodeLen + 3] = (AdrLong >> 1) & 0xff;
                  CodeLen += 4;
               }
         }
         return;
      }

   WrXError(1200, OpPart);
}

static void InitCode_XA(void) {
   SaveInitProc();
   Reg_DS = 0;
}

static bool ChkPC_XA(void) {
   switch (ActPC) {
      case SegCode:
      case SegData:
         return (ProgCounter() < 0x1000000);
      case SegIO:
         return ((ProgCounter() > 0x3ff) && (ProgCounter() < 0x800));
      default:
         return false;
   }
}

static bool IsDef_XA(void) {
   return (ActPC == SegCode);
}

static void SwitchFrom_XA(void) {
   DeinitFields();
}

static void SwitchTo_XA(void) {
   TurnWords = false;
   ConstMode = ConstModeIntel;
   SetIsOccupied = false;

   PCSymbol = "$";
   HeaderID = 0x3c;
   NOPCode = 0x00;
   DivideChars = ",";
   HasAttrs = true;
   AttrChars = ".";

   ValidSegs = (1 << SegCode) | (1 << SegData) | (1 << SegIO);
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;
   Grans[SegData] = 1;
   ListGrans[SegData] = 1;
   SegInits[SegData] = 0;
   Grans[SegIO] = 1;
   ListGrans[SegIO] = 1;
   SegInits[SegIO] = 0x400;

   MakeCode = MakeCode_XA;
   ChkPC = ChkPC_XA;
   IsDef = IsDef_XA;
   SwitchFrom = SwitchFrom_XA;
   InitFields();
}

void codexa_init(void) {
   CPUXAG1 = AddCPU("XAG1", SwitchTo_XA);
   CPUXAG2 = AddCPU("XAG2", SwitchTo_XA);
   CPUXAG3 = AddCPU("XAG3", SwitchTo_XA);

   SaveInitProc = InitPassProc;
   InitPassProc = InitCode_XA;
}
