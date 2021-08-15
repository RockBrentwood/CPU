/* code87c800.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TLCS-870                                                    */
/*                                                                           */
/* Historie: 29.12.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "nls.h"
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

typedef struct {
   char *Name;
   Byte Code;
} CondRec;

#define FixedOrderCnt 7
#define ConditionCnt 12
#define RegOrderCnt 7
#define ALUOrderCnt 8

#define ModNone (-1)
#define ModReg8 0
#define MModReg8 (1 << ModReg8)
#define ModReg16 1
#define MModReg16 (1 << ModReg16)
#define ModImm 2
#define MModImm (1 << ModImm)
#define ModAbs 3
#define MModAbs (1 << ModAbs)
#define ModMem 4
#define MModMem (1 << ModMem)

#define AccReg 0
#define WAReg 0

#define Reg8Cnt 8
static char *Reg8Names = "AWCBEDLH";

static CPUVar CPU87C00, CPU87C20, CPU87C40, CPU87C70;
static ShortInt OpSize;
static Byte AdrVals[4];
static ShortInt AdrType;
static Byte AdrMode;

static FixedOrder *FixedOrders;
static CondRec *Conditions;
static FixedOrder *RegOrders;
static char **ALUOrders;

/*--------------------------------------------------------------------------*/

static void AddFixed(char *NName, Word NCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddCond(char *NName, Byte NCode) {
   if (InstrZ >= ConditionCnt) exit(255);
   Conditions[InstrZ].Name = NName;
   Conditions[InstrZ++].Code = NCode;
}

static void AddReg(char *NName, Word NCode) {
   if (InstrZ >= RegOrderCnt) exit(255);
   RegOrders[InstrZ].Name = NName;
   RegOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("DI", 0x483a);
   AddFixed("EI", 0x403a);
   AddFixed("RET", 0x0005);
   AddFixed("RETI", 0x0004);
   AddFixed("RETN", 0xe804);
   AddFixed("SWI", 0x00ff);
   AddFixed("NOP", 0x0000);

   Conditions = (CondRec *) malloc(sizeof(CondRec) * ConditionCnt);
   InstrZ = 0;
   AddCond("EQ", 0);
   AddCond("Z", 0);
   AddCond("NE", 1);
   AddCond("NZ", 1);
   AddCond("CS", 2);
   AddCond("LT", 2);
   AddCond("CC", 3);
   AddCond("GE", 3);
   AddCond("LE", 4);
   AddCond("GT", 5);
   AddCond("T", 6);
   AddCond("F", 7);

   RegOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * RegOrderCnt);
   InstrZ = 0;
   AddReg("DAA", 0x0a);
   AddReg("DAS", 0x0b);
   AddReg("SHLC", 0x1c);
   AddReg("SHRC", 0x1d);
   AddReg("ROLC", 0x1e);
   AddReg("RORC", 0x1f);
   AddReg("SWAP", 0x01);

   ALUOrders = (char **)malloc(sizeof(char *) * ALUOrderCnt);
   InstrZ = 0;
   ALUOrders[InstrZ++] = "ADDC";
   ALUOrders[InstrZ++] = "ADD";
   ALUOrders[InstrZ++] = "SUBB";
   ALUOrders[InstrZ++] = "SUB";
   ALUOrders[InstrZ++] = "AND";
   ALUOrders[InstrZ++] = "XOR";
   ALUOrders[InstrZ++] = "OR";
   ALUOrders[InstrZ++] = "CMP";
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(Conditions);
   free(RegOrders);
   free(ALUOrders);
}

/*--------------------------------------------------------------------------*/

static void ChkAdr(Byte Erl) {
   if ((AdrType != ModNone) && (((1 << AdrType) & Erl) == 0)) {
      AdrType = ModNone;
      AdrCnt = 0;
      WrError(1350);
   }
}

static void DecodeAdr(char *Asc, Byte Erl) {
#define Reg16Cnt 4
   static char *Reg16Names[Reg16Cnt] = { "WA", "BC", "DE", "HL" };
#define AdrRegCnt 5
   static char *AdrRegs[AdrRegCnt] = { "HL", "DE", "C", "PC", "A" };

   Integer z;
   Byte RegFlag;
   LongInt DispAcc, DispPart;
   String AdrPart;
   bool OK, NegFlag, NNegFlag, FirstFlag;
   char *PPos, *NPos, *EPos;

   AdrType = ModNone;
   AdrCnt = 0;

   if (strlen(Asc) == 1)
      for (z = 0; z < Reg8Cnt; z++)
         if (toupper(*Asc) == Reg8Names[z]) {
            AdrType = ModReg8;
            OpSize = 0;
            AdrMode = z;
            ChkAdr(Erl);
            return;
         }

   for (z = 0; z < Reg16Cnt; z++)
      if (strcasecmp(Asc, Reg16Names[z]) == 0) {
         AdrType = ModReg16;
         OpSize = 1;
         AdrMode = z;
         ChkAdr(Erl);
         return;
      }

   if (IsIndirect(Asc)) {
      strcpy(Asc, Asc + 1);
      Asc[strlen(Asc) - 1] = '\0';
      if (strcasecmp(Asc, "-HL") == 0) {
         AdrType = ModMem;
         AdrMode = 7;
         ChkAdr(Erl);
         return;
      }
      if (strcasecmp(Asc, "HL+") == 0) {
         AdrType = ModMem;
         AdrMode = 6;
         ChkAdr(Erl);
         return;
      }
      RegFlag = 0;
      DispAcc = 0;
      NegFlag = false;
      OK = true;
      FirstFlag = false;
      while ((OK) && (*Asc != '\0')) {
         PPos = QuotPos(Asc, '+');
         NPos = QuotPos(Asc, '-');
         if (PPos == NULL) EPos = NPos;
         else if (NPos == NULL) EPos = PPos;
         else EPos = min(NPos, PPos);
         NNegFlag = ((EPos != NULL) && (*EPos == '-'));
         if (EPos == NULL) {
            strmaxcpy(AdrPart, Asc, 255);
            *Asc = '\0';
         } else {
            *EPos = '\0';
            strmaxcpy(AdrPart, Asc, 255);
            strcpy(Asc, EPos + 1);
         }
         for (z = 0; z < AdrRegCnt; z++)
            if (strcasecmp(AdrPart, AdrRegs[z]) == 0) break;
         if (z >= AdrRegCnt) {
            FirstPassUnknown = false;
            DispPart = EvalIntExpression(AdrPart, Int32, &OK);
            FirstFlag |= FirstPassUnknown;
            if (NegFlag) DispAcc -= DispPart;
            else DispAcc += DispPart;
         } else if ((NegFlag) || ((RegFlag & (1 << z)) != 0)) {
            WrError(1350);
            OK = false;
         } else RegFlag += 1 << z;
         NegFlag = NNegFlag;
      }
      if (DispAcc != 0) RegFlag += 1 << AdrRegCnt;
      if (OK)
         switch (RegFlag) {
            case 0x20:
               if (FirstFlag) DispAcc &= 0xff;
               if (DispAcc > 0xff) WrError(1320);
               else {
                  AdrType = ModAbs;
                  AdrMode = 0;
                  AdrCnt = 1;
                  AdrVals[0] = DispAcc & 0xff;
               }
               break;
            case 0x02:
               AdrType = ModMem;
               AdrMode = 2;
               break;
            case 0x01:
               AdrType = ModMem;
               AdrMode = 3;
               break;
            case 0x21:
               if (FirstFlag) DispAcc &= 0x7f;
               if (DispAcc > 127) WrError(1320);
               else if (DispAcc < -128) WrError(1315);
               else {
                  AdrType = ModMem;
                  AdrMode = 4;
                  AdrCnt = 1;
                  AdrVals[0] = DispAcc & 0xff;
               }
               break;
            case 0x05:
               AdrType = ModMem;
               AdrMode = 5;
               break;
            case 0x18:
               AdrType = ModMem;
               AdrMode = 1;
               break;
            default:
               WrError(1350);
         }
      ChkAdr(Erl);
      return;
   } else
      switch (OpSize) {
         case -1:
            WrError(1132);
            break;
         case 0:
            AdrVals[0] = EvalIntExpression(Asc, Int8, &OK);
            if (OK) {
               AdrType = ModImm;
               AdrCnt = 1;
            }
            break;
         case 1:
            DispAcc = EvalIntExpression(Asc, Int16, &OK);
            if (OK) {
               AdrType = ModImm;
               AdrCnt = 2;
               AdrVals[0] = DispAcc & 0xff;
               AdrVals[1] = (DispAcc >> 8) & 0xff;
            }
            break;
      }

   ChkAdr(Erl);
}

static bool SplitBit(char *Asc, Byte * Erg) {
   char *p;
   String Part;

   p = RQuotPos(Asc, '.');
   if (p == NULL) return false;
   *p = '\0';
   strmaxcpy(Part, p + 1, 255);

   if (strlen(Part) != 1) return false;
   else if ((*Part >= '0') && (*Part <= '7')) {
      *Erg = (*Part) - '0';
      return true;
   } else {
      for (*Erg = 0; *Erg < Reg8Cnt; (*Erg)++)
         if (toupper(*Part) == Reg8Names[*Erg]) break;
      if (*Erg < Reg8Cnt) {
         *Erg += 8;
         return true;
      } else return false;
   }
}

static bool DecodePseudo(void) {
   return false;
}

static void CodeMem(Byte Entry, Byte Opcode) {
   BAsmCode[0] = Entry + AdrMode;
   memcpy(BAsmCode + 1, AdrVals, AdrCnt);
   BAsmCode[1 + AdrCnt] = Opcode;
}

static void MakeCode_87C800(void) {
   Integer z, AdrInt, Condition;
   Byte HReg, HCnt, HMode, HVal;
   bool OK;

   CodeLen = 0;
   DontPrint = false;
   OpSize = (-1);

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(false)) return;

/* ohne Argument */

   for (z = 0; z < FixedOrderCnt; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else {
            CodeLen = 0;
            if (Hi(FixedOrders[z].Code) != 0) BAsmCode[CodeLen++] = Hi(FixedOrders[z].Code);
            BAsmCode[CodeLen++] = Lo(FixedOrders[z].Code);
         }
         return;
      }

/* Datentransfer */

   if (Memo("LD")) {
      if (ArgCnt != 2) WrError(1110);
      else if (strcasecmp(ArgStr[1], "SP") == 0) {
         OpSize = 1;
         DecodeAdr(ArgStr[2], MModImm + MModReg16);
         switch (AdrType) {
            case ModReg16:
               CodeLen = 2;
               BAsmCode[0] = 0xe8 + AdrMode;
               BAsmCode[1] = 0xfa;
               break;
            case ModImm:
               CodeLen = 3;
               BAsmCode[0] = 0xfa;
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               break;
         }
      } else if (strcasecmp(ArgStr[2], "SP") == 0) {
         DecodeAdr(ArgStr[1], MModReg16);
         switch (AdrType) {
            case ModReg16:
               CodeLen = 2;
               BAsmCode[0] = 0xe8 + AdrMode;
               BAsmCode[1] = 0xfb;
               break;
         }
      } else if (strcasecmp(ArgStr[1], "RBS") == 0) {
         BAsmCode[1] = EvalIntExpression(ArgStr[2], Int4, &OK);
         if (OK) {
            CodeLen = 2;
            BAsmCode[0] = 0x0f;
         }
      } else if (strcasecmp(ArgStr[1], "CF") == 0) {
         if (!SplitBit(ArgStr[2], &HReg)) WrError(1510);
         else {
            DecodeAdr(ArgStr[2], MModReg8 + MModAbs + MModMem);
            switch (AdrType) {
               case ModReg8:
                  if (HReg >= 8) WrError(1350);
                  else {
                     CodeLen = 2;
                     BAsmCode[0] = 0xe8 + AdrMode;
                     BAsmCode[1] = 0xd8 + HReg;
                  }
                  break;
               case ModAbs:
                  if (HReg >= 8) WrError(1350);
                  else {
                     CodeLen = 2;
                     BAsmCode[0] = 0xd8 + HReg;
                     BAsmCode[1] = AdrVals[0];
                  }
                  break;
               case ModMem:
                  if (HReg < 8) {
                     CodeLen = 2 + AdrCnt;
                     CodeMem(0xe0, 0xd8 + HReg);
                  } else if ((AdrMode != 2) && (AdrMode != 3)) WrError(1350);
                  else {
                     CodeLen = 2;
                     BAsmCode[0] = 0xe0 + HReg;
                     BAsmCode[1] = 0x9c + AdrMode;
                  }
                  break;
            }
         }
      } else if (strcasecmp(ArgStr[2], "CF") == 0) {
         if (!SplitBit(ArgStr[1], &HReg)) WrError(1510);
         else {
            DecodeAdr(ArgStr[1], MModReg8 + MModAbs + MModMem);
            switch (AdrType) {
               case ModReg8:
                  if (HReg >= 8) WrError(1350);
                  else {
                     CodeLen = 2;
                     BAsmCode[0] = 0xe8 + AdrMode;
                     BAsmCode[1] = 0xc8 + HReg;
                  }
                  break;
               case ModAbs:
               case ModMem:
                  if (HReg < 8) {
                     CodeLen = 2 + AdrCnt;
                     CodeMem(0xe0, 0xc8 + HReg);
                  } else if ((AdrMode != 2) && (AdrMode != 3)) WrError(1350);
                  else {
                     CodeLen = 2;
                     BAsmCode[0] = 0xe0 + HReg;
                     BAsmCode[1] = 0x98 + AdrMode;
                  }
                  break;
            }
         }
      } else {
         DecodeAdr(ArgStr[1], MModReg8 + MModReg16 + MModAbs + MModMem);
         switch (AdrType) {
            case ModReg8:
               HReg = AdrMode;
               DecodeAdr(ArgStr[2], MModReg8 + MModAbs + MModMem + MModImm);
               switch (AdrType) {
                  case ModReg8:
                     if (HReg == AccReg) {
                        CodeLen = 1;
                        BAsmCode[0] = 0x50 + AdrMode;
                     } else if (AdrMode == AccReg) {
                        CodeLen = 1;
                        BAsmCode[0] = 0x58 + HReg;
                     } else {
                        CodeLen = 2;
                        BAsmCode[0] = 0xe8 + AdrMode;
                        BAsmCode[1] = 0x58 + HReg;
                     }
                     break;
                  case ModAbs:
                     if (HReg == AccReg) {
                        CodeLen = 2;
                        BAsmCode[0] = 0x22;
                        BAsmCode[1] = AdrVals[0];
                     } else {
                        CodeLen = 3;
                        BAsmCode[0] = 0xe0;
                        BAsmCode[1] = AdrVals[0];
                        BAsmCode[2] = 0x58 + HReg;
                     }
                     break;
                  case ModMem:
                     if ((HReg == AccReg) && (AdrMode == 3)) { /* A,(HL) */
                        CodeLen = 1;
                        BAsmCode[0] = 0x23;
                     } else {
                        CodeLen = 2 + AdrCnt;
                        CodeMem(0xe0, 0x58 + HReg);
                        if ((HReg >= 6) && (AdrMode == 6)) WrError(140);
                     }
                     break;
                  case ModImm:
                     CodeLen = 2;
                     BAsmCode[0] = 0x30 + HReg;
                     BAsmCode[1] = AdrVals[0];
                     break;
               }
               break;
            case ModReg16:
               HReg = AdrMode;
               DecodeAdr(ArgStr[2], MModReg16 + MModAbs + MModMem + MModImm);
               switch (AdrType) {
                  case ModReg16:
                     CodeLen = 2;
                     BAsmCode[0] = 0xe8 + AdrMode;
                     BAsmCode[1] = 0x14 + HReg;
                     break;
                  case ModAbs:
                     CodeLen = 3;
                     BAsmCode[0] = 0xe0;
                     BAsmCode[1] = AdrVals[0];
                     BAsmCode[2] = 0x14 + HReg;
                     break;
                  case ModMem:
                     if (AdrMode > 5) WrError(1350); /* (-HL),(HL+) */
                     else {
                        CodeLen = 2 + AdrCnt;
                        BAsmCode[0] = 0xe0 + AdrMode;
                        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                        BAsmCode[1 + AdrCnt] = 0x14 + HReg;
                     }
                     break;
                  case ModImm:
                     CodeLen = 3;
                     BAsmCode[0] = 0x14 + HReg;
                     memcpy(BAsmCode + 1, AdrVals, 2);
                     break;
               }
               break;
            case ModAbs:
               HReg = AdrVals[0];
               OpSize = 0;
               DecodeAdr(ArgStr[2], MModReg8 + MModReg16 + MModAbs + MModMem + MModImm);
               switch (AdrType) {
                  case ModReg8:
                     if (AdrMode == AccReg) {
                        CodeLen = 2;
                        BAsmCode[0] = 0x2a;
                        BAsmCode[1] = HReg;
                     } else {
                        CodeLen = 3;
                        BAsmCode[0] = 0xf0;
                        BAsmCode[1] = HReg;
                        BAsmCode[2] = 0x50 + AdrMode;
                     }
                     break;
                  case ModReg16:
                     CodeLen = 3;
                     BAsmCode[0] = 0xf0;
                     BAsmCode[1] = HReg;
                     BAsmCode[2] = 0x10 + AdrMode;
                     break;
                  case ModAbs:
                     CodeLen = 3;
                     BAsmCode[0] = 0x26;
                     BAsmCode[1] = AdrVals[0];
                     BAsmCode[2] = HReg;
                     break;
                  case ModMem:
                     if (AdrMode > 5) WrError(1350); /* (-HL),(HL+) */
                     else {
                        CodeLen = 3 + AdrCnt;
                        BAsmCode[0] = 0xe0 + AdrMode;
                        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                        BAsmCode[1 + AdrCnt] = 0x26;
                        BAsmCode[2 + AdrCnt] = HReg;
                     }
                     break;
                  case ModImm:
                     CodeLen = 3;
                     BAsmCode[0] = 0x2c;
                     BAsmCode[1] = HReg;
                     BAsmCode[2] = AdrVals[0];
                     break;
               }
               break;
            case ModMem:
               HVal = AdrVals[0];
               HCnt = AdrCnt;
               HMode = AdrMode;
               OpSize = 0;
               DecodeAdr(ArgStr[2], MModReg8 + MModReg16 + MModAbs + MModMem + MModImm);
               switch (AdrType) {
                  case ModReg8:
                     if ((HMode == 3) && (AdrMode == AccReg)) { /* (HL),A */
                        CodeLen = 1;
                        BAsmCode[0] = 0x2b;
                     } else if ((HMode == 1) || (HMode == 5)) WrError(1350);
                     else {
                        CodeLen = 2 + HCnt;
                        BAsmCode[0] = 0xf0 + HMode;
                        memcpy(BAsmCode + 1, &HVal, HCnt);
                        BAsmCode[1 + HCnt] = 0x50 + AdrMode;
                        if ((HMode == 6) && (AdrMode >= 6)) WrError(140);
                     }
                     break;
                  case ModReg16:
                     if ((HMode < 2) || (HMode > 4)) WrError(1350); /* (HL),(DE),(HL+d) */
                     else {
                        CodeLen = 2 + HCnt;
                        BAsmCode[0] = 0xf0 + HMode;
                        memcpy(BAsmCode + 1, &HVal, HCnt);
                        BAsmCode[1 + HCnt] = 0x10 + AdrMode;
                     }
                     break;
                  case ModAbs:
                     if (HMode != 3) WrError(1350); /* (HL) */
                     else {
                        CodeLen = 3;
                        BAsmCode[0] = 0xe0;
                        BAsmCode[1] = AdrVals[0];
                        BAsmCode[2] = 0x27;
                     }
                     break;
                  case ModMem:
                     if (HMode != 3) WrError(1350); /* (HL) */
                     else if (AdrMode > 5) WrError(1350); /* (-HL),(HL+) */
                     else {
                        CodeLen = 2 + AdrCnt;
                        BAsmCode[0] = 0xe0 + AdrMode;
                        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                        BAsmCode[1 + AdrCnt] = 0x27;
                     }
                     break;
                  case ModImm:
                     if ((HMode == 1) || (HMode == 5)) WrError(1350); /* (HL+C),(PC+A) */
                     else if (HMode == 3) { /* (HL) */
                        CodeLen = 2;
                        BAsmCode[0] = 0x2d;
                        BAsmCode[1] = AdrVals[0];
                     } else {
                        CodeLen = 3 + HCnt;
                        BAsmCode[0] = 0xf0 + HMode;
                        memcpy(BAsmCode + 1, &HVal, HCnt);
                        BAsmCode[1 + HCnt] = 0x2c;
                        BAsmCode[2 + HCnt] = AdrVals[0];
                     }
                     break;
               }
               break;
         }
      }
      return;
   }

   if (Memo("XCH")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg8 + MModReg16 + MModAbs + MModMem);
         switch (AdrType) {
            case ModReg8:
               HReg = AdrMode;
               DecodeAdr(ArgStr[2], MModReg8 + MModAbs + MModMem);
               switch (AdrType) {
                  case ModReg8:
                     CodeLen = 2;
                     BAsmCode[0] = 0xe8 + AdrMode;
                     BAsmCode[1] = 0xa8 + HReg;
                     break;
                  case ModAbs:
                  case ModMem:
                     CodeLen = 2 + AdrCnt;
                     CodeMem(0xe0, 0xa8 + HReg);
                     if ((HReg >= 6) && (AdrMode == 6)) WrError(140);
                     break;
               }
               break;
            case ModReg16:
               HReg = AdrMode;
               DecodeAdr(ArgStr[2], MModReg16);
               if (AdrType != ModNone) {
                  CodeLen = 2;
                  BAsmCode[0] = 0xe8 + AdrMode;
                  BAsmCode[1] = 0x10 + HReg;
               }
               break;
            case ModAbs:
               BAsmCode[1] = AdrVals[0];
               DecodeAdr(ArgStr[2], MModReg8);
               if (AdrType != ModNone) {
                  CodeLen = 3;
                  BAsmCode[0] = 0xe0;
                  BAsmCode[2] = 0xa8 + AdrMode;
               }
               break;
            case ModMem:
               BAsmCode[0] = 0xe0 + AdrMode;
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               HReg = AdrCnt;
               DecodeAdr(ArgStr[2], MModReg8);
               if (AdrType != ModNone) {
                  CodeLen = 2 + HReg;
                  BAsmCode[1 + HReg] = 0xa8 + AdrMode;
                  if ((AdrMode >= 6) && ((BAsmCode[0] & 0x0f) == 6)) WrError(140);
               }
               break;
         }
      }
      return;
   }

   if (Memo("CLR")) {
      if (ArgCnt != 1) WrError(1110);
      else if (strcasecmp(ArgStr[1], "CF") == 0) {
         CodeLen = 1;
         BAsmCode[0] = 0x0c;
      } else if (SplitBit(ArgStr[1], &HReg)) {
         DecodeAdr(ArgStr[1], MModReg8 + MModAbs + MModMem);
         switch (AdrType) {
            case ModReg8:
               if (HReg >= 8) WrError(1350);
               else {
                  CodeLen = 2;
                  BAsmCode[0] = 0xe8 + AdrMode;
                  BAsmCode[1] = 0x48 + HReg;
               }
               break;
            case ModAbs:
               if (HReg >= 8) WrError(1350);
               else {
                  CodeLen = 2;
                  BAsmCode[0] = 0x48 + HReg;
                  BAsmCode[1] = AdrVals[0];
               }
               break;
            case ModMem:
               if (HReg <= 8) {
                  CodeLen = 2 + AdrCnt;
                  CodeMem(0xe0, 0x48 + HReg);
               } else if ((AdrMode != 2) && (AdrMode != 3)) WrError(1350);
               else {
                  CodeLen = 2;
                  BAsmCode[0] = 0xe0 + HReg;
                  BAsmCode[1] = 0x88 + AdrMode;
               }
               break;
         }
      } else {
         DecodeAdr(ArgStr[1], MModReg8 + MModReg16 + MModAbs + MModMem);
         switch (AdrType) {
            case ModReg8:
               CodeLen = 2;
               BAsmCode[0] = 0x30 + AdrMode;
               BAsmCode[1] = 0;
               break;
            case ModReg16:
               CodeLen = 3;
               BAsmCode[0] = 0x14 + AdrMode;
               BAsmCode[1] = 0;
               BAsmCode[2] = 0;
               break;
            case ModAbs:
               CodeLen = 2;
               BAsmCode[0] = 0x2e;
               BAsmCode[1] = AdrVals[0];
               break;
            case ModMem:
               if ((AdrMode == 5) || (AdrMode == 1)) WrError(1350); /* (PC+A, HL+C) */
               else if (AdrMode == 3) { /* (HL) */
                  CodeLen = 1;
                  BAsmCode[0] = 0x2f;
               } else {
                  CodeLen = 3 + AdrCnt;
                  BAsmCode[0] = 0xf0 + AdrMode;
                  memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                  BAsmCode[1 + AdrCnt] = 0x2c;
                  BAsmCode[2 + AdrCnt] = 0;
               }
               break;
         }
      }
      return;
   }

   if (Memo("LDW")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         AdrInt = EvalIntExpression(ArgStr[2], Int16, &OK);
         if (OK) {
            DecodeAdr(ArgStr[1], MModReg16 + MModAbs + MModMem);
            switch (AdrType) {
               case ModReg16:
                  CodeLen = 3;
                  BAsmCode[0] = 0x14 + AdrMode;
                  BAsmCode[1] = AdrInt & 0xff;
                  BAsmCode[2] = AdrInt >> 8;
                  break;
               case ModAbs:
                  CodeLen = 4;
                  BAsmCode[0] = 0x24;
                  BAsmCode[1] = AdrVals[0];
                  BAsmCode[2] = AdrInt & 0xff;
                  BAsmCode[3] = AdrInt >> 8;
                  break;
               case ModMem:
                  if (AdrMode != 3) WrError(1350); /* (HL) */
                  else {
                     CodeLen = 3;
                     BAsmCode[0] = 0x25;
                     BAsmCode[1] = AdrInt & 0xff;
                     BAsmCode[2] = AdrInt >> 8;
                  }
                  break;
            }
         }
      }
      return;
   }

   if ((Memo("PUSH")) || (Memo("POP"))) {
      HReg = Memo("PUSH") + 6;
      if (ArgCnt != 1) WrError(1110);
      else if (strcasecmp(ArgStr[1], "PSW") == 0) {
         CodeLen = 1;
         BAsmCode[0] = HReg;
      } else {
         DecodeAdr(ArgStr[1], MModReg16);
         if (AdrType != ModNone) {
            CodeLen = 2;
            BAsmCode[0] = 0xe8 + AdrMode;
            BAsmCode[1] = HReg;
         }
      }
      return;
   }

   if ((Memo("TEST")) || (Memo("CPL")) || (Memo("SET"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (strcasecmp(ArgStr[1], "CF") == 0) {
         if (Memo("TEST")) WrError(1350);
         else {
            CodeLen = 1;
            BAsmCode[0] = 0x0d + Memo("CPL");
         }
      } else if (!SplitBit(ArgStr[1], &HReg)) WrError(1510);
      else {
         if (Memo("TEST")) HVal = 0xd8;
         else if (Memo("SET")) HVal = 0x40;
         else HVal = 0xc0;
         DecodeAdr(ArgStr[1], MModReg8 + MModAbs + MModMem);
         switch (AdrType) {
            case ModReg8:
               if (HReg >= 8) WrError(1350);
               else {
                  CodeLen = 2;
                  BAsmCode[0] = 0xe8 + AdrMode;
                  BAsmCode[1] = HVal + HReg;
               }
               break;
            case ModAbs:
               if (HReg >= 8) WrError(1350);
               else if (Memo("CPL")) {
                  CodeLen = 3;
                  CodeMem(0xe0, HVal + HReg);
               } else {
                  CodeLen = 2;
                  BAsmCode[0] = HVal + HReg;
                  BAsmCode[1] = AdrVals[0];
               }
               break;
            case ModMem:
               if (HReg < 8) {
                  CodeLen = 2 + AdrCnt;
                  CodeMem(0xe0, HVal + HReg);
               } else if ((AdrMode != 2) && (AdrMode != 3)) WrError(1350);
               else {
                  CodeLen = 2;
                  BAsmCode[0] = 0xe0 + HReg;
                  BAsmCode[1] = ((HVal & 0x18) >> 1) + ((HVal & 0x80) >> 3) + 0x80 + AdrMode;
               }
               break;
         }
      }
      return;
   }

/* Arithmetik */

   for (z = 0; z < RegOrderCnt; z++)
      if (Memo(RegOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], MModReg8);
            if (AdrType != ModNone)
               if (AdrMode == AccReg) {
                  CodeLen = 1;
                  BAsmCode[0] = RegOrders[z].Code;
               } else {
                  CodeLen = 2;
                  BAsmCode[0] = 0xe8 + AdrMode;
                  BAsmCode[1] = RegOrders[z].Code;
               }
         }
         return;
      }

   for (z = 0; z < ALUOrderCnt; z++)
      if (Memo(ALUOrders[z])) {
         if (ArgCnt != 2) WrError(1110);
         else if (strcasecmp(ArgStr[1], "CF") == 0) {
            if (!Memo("XOR")) WrError(1350);
            else if (!SplitBit(ArgStr[2], &HReg)) WrError(1510);
            else if (HReg >= 8) WrError(1350);
            else {
               DecodeAdr(ArgStr[2], MModReg8 + MModAbs + MModMem);
               switch (AdrType) {
                  case ModReg8:
                     CodeLen = 2;
                     BAsmCode[0] = 0xe8 + AdrMode;
                     BAsmCode[1] = 0xd0 + HReg;
                     break;
                  case ModAbs:
                  case ModMem:
                     CodeLen = 2 + AdrCnt;
                     CodeMem(0xe0, 0xd0 + HReg);
                     break;
               }
            }
         } else {
            DecodeAdr(ArgStr[1], MModReg8 + MModReg16 + MModMem + MModAbs);
            switch (AdrType) {
               case ModReg8:
                  HReg = AdrMode;
                  DecodeAdr(ArgStr[2], MModReg8 + MModMem + MModAbs + MModImm);
                  switch (AdrType) {
                     case ModReg8:
                        if (HReg == AccReg) {
                           CodeLen = 2;
                           BAsmCode[0] = 0xe8 + AdrMode;
                           BAsmCode[1] = 0x60 + z;
                        } else if (AdrMode == AccReg) {
                           CodeLen = 2;
                           BAsmCode[0] = 0xe8 + HReg;
                           BAsmCode[1] = 0x68 + z;
                        } else WrError(1350);
                        break;
                     case ModMem:
                        if (HReg != AccReg) WrError(1350);
                        else {
                           CodeLen = 2 + AdrCnt;
                           BAsmCode[0] = 0xe0 + AdrMode;
                           memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                           BAsmCode[1 + AdrCnt] = 0x78 + z;
                        }
                        break;
                     case ModAbs:
                        if (HReg != AccReg) WrError(1350);
                        else {
                           CodeLen = 2;
                           BAsmCode[0] = 0x78 + z;
                           BAsmCode[1] = AdrVals[0];
                        }
                        break;
                     case ModImm:
                        if (HReg == AccReg) {
                           CodeLen = 2;
                           BAsmCode[0] = 0x70 + z;
                           BAsmCode[1] = AdrVals[0];
                        } else {
                           CodeLen = 3;
                           BAsmCode[0] = 0xe8 + HReg;
                           BAsmCode[1] = 0x70 + z;
                           BAsmCode[2] = AdrVals[0];
                        }
                        break;
                  }
                  break;
               case ModReg16:
                  HReg = AdrMode;
                  DecodeAdr(ArgStr[2], MModImm + MModReg16);
                  switch (AdrType) {
                     case ModImm:
                        CodeLen = 4;
                        BAsmCode[0] = 0xe8 + HReg;
                        BAsmCode[1] = 0x38 + z;
                        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                        break;
                     case ModReg16:
                        if (HReg != WAReg) WrError(1350);
                        else {
                           CodeLen = 2;
                           BAsmCode[0] = 0xe8 + AdrMode;
                           BAsmCode[1] = 0x30 + z;
                        }
                        break;
                  }
                  break;
               case ModAbs:
                  if (strcasecmp(ArgStr[2], "(HL)") == 0) {
                     CodeLen = 3;
                     BAsmCode[0] = 0xe0;
                     BAsmCode[1] = AdrVals[0];
                     BAsmCode[2] = 0x60 + z;
                  } else {
                     BAsmCode[3] = EvalIntExpression(ArgStr[2], Int8, &OK);
                     if (OK) {
                        CodeLen = 4;
                        BAsmCode[0] = 0xe0;
                        BAsmCode[1] = AdrVals[0];
                        BAsmCode[2] = 0x70 + z;
                     }
                  }
                  break;
               case ModMem:
                  if (strcasecmp(ArgStr[2], "(HL)") == 0) {
                     CodeLen = 2 + AdrCnt;
                     BAsmCode[0] = 0xe0 + AdrMode;
                     memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                     BAsmCode[1 + AdrCnt] = 0x60 + z;
                  } else {
                     BAsmCode[2 + AdrCnt] = EvalIntExpression(ArgStr[2], Int8, &OK);
                     if (OK) {
                        CodeLen = 3 + AdrCnt;
                        BAsmCode[0] = 0xe0 + AdrMode;
                        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                        BAsmCode[1 + AdrCnt] = 0x70 + z;
                     }
                  }
                  break;
            }
         }
         return;
      }

   if (Memo("MCMP")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         HReg = EvalIntExpression(ArgStr[2], Int8, &OK);
         if (OK) {
            DecodeAdr(ArgStr[1], MModMem + MModAbs);
            if (AdrType != ModNone) {
               CodeLen = 3 + AdrCnt;
               CodeMem(0xe0, 0x2f);
               BAsmCode[2 + AdrCnt] = HReg;
            }
         }
      }
      return;
   }

   if ((Memo("DEC")) || (Memo("INC"))) {
      HReg = Memo("DEC") << 3;
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg8 + MModReg16 + MModAbs + MModMem);
         switch (AdrType) {
            case ModReg8:
               CodeLen = 1;
               BAsmCode[0] = 0x60 + HReg + AdrMode;
               break;
            case ModReg16:
               CodeLen = 1;
               BAsmCode[0] = 0x10 + HReg + AdrMode;
               break;
            case ModAbs:
               CodeLen = 2;
               BAsmCode[0] = 0x20 + HReg;
               BAsmCode[1] = AdrVals[0];
               break;
            case ModMem:
               if (AdrMode == 3) { /* (HL) */
                  CodeLen = 1;
                  BAsmCode[0] = 0x21 + HReg;
               } else {
                  CodeLen = 2 + AdrCnt;
                  BAsmCode[0] = 0xe0 + AdrMode;
                  memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                  BAsmCode[1 + AdrCnt] = 0x20 + HReg;
               }
               break;
         }
      }
      return;
   }

   if (Memo("MUL")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg8);
         if (AdrType == ModReg8) {
            HReg = AdrMode;
            DecodeAdr(ArgStr[2], MModReg8);
            if (AdrType == ModReg8)
               if ((HReg ^ AdrMode) != 1) WrError(1760);
               else {
                  HReg = HReg >> 1;
                  if (HReg == 0) {
                     CodeLen = 1;
                     BAsmCode[0] = 0x02;
                  } else {
                     CodeLen = 2;
                     BAsmCode[0] = 0xe8 + HReg;
                     BAsmCode[1] = 0x02;
                  }
               }
         }
      }
      return;
   }

   if (Memo("DIV")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg16);
         if (AdrType == ModReg16) {
            HReg = AdrMode;
            DecodeAdr(ArgStr[2], MModReg8);
            if (AdrType == ModReg8)
               if (AdrMode != 2) WrError(1350); /* C */
               else if (HReg == 0) {
                  CodeLen = 1;
                  BAsmCode[0] = 0x03;
               } else {
                  CodeLen = 2;
                  BAsmCode[0] = 0xe8 + HReg;
                  BAsmCode[1] = 0x03;
                  if (HReg == 1) WrError(140);
               }
         }
      }
      return;
   }

   if ((Memo("ROLD")) || (Memo("RORD"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (strcasecmp(ArgStr[1], "A") != 0) WrError(1350);
      else {
         HReg = Memo("RORD") + 8;
         DecodeAdr(ArgStr[2], MModAbs + MModMem);
         if (AdrType != ModNone) {
            CodeLen = 2 + AdrCnt;
            CodeMem(0xe0, HReg);
            if (AdrMode == 1) WrError(140);
         }
      }
      return;
   }

/* Spruenge */

   if (Memo("JRS")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         NLS_UpString(ArgStr[1]);
         for (Condition = ConditionCnt - 2; Condition < ConditionCnt; Condition++)
            if (strcmp(ArgStr[1], Conditions[Condition].Name) == 0) break;
         if (Condition >= ConditionCnt) WrXError(1360, ArgStr[1]);
         else {
            AdrInt = EvalIntExpression(ArgStr[2], Int16, &OK) - (EProgCounter() + 2);
            if (OK)
               if (((AdrInt < -16) || (AdrInt > 15)) && (!SymbolQuestionable)) WrError(1370);
               else {
                  CodeLen = 1;
                  BAsmCode[0] = ((Conditions[Condition].Code - 2) << 5) + (AdrInt & 0x1f);
               }
         }
      }
      return;
   }

   if (Memo("JR")) {
      if ((ArgCnt != 2) && (ArgCnt != 1)) WrError(1110);
      else {
         if (ArgCnt == 1) Condition = (-1);
         else {
            NLS_UpString(ArgStr[1]);
            for (Condition = 0; Condition < ConditionCnt; Condition++)
               if (strcmp(ArgStr[1], Conditions[Condition].Name) == 0) break;
         }
         if (Condition >= ConditionCnt) WrXError(1360, ArgStr[1]);
         else {
            AdrInt = EvalIntExpression(ArgStr[2], Int16, &OK) - (EProgCounter() + 2);
            if (OK)
               if (((AdrInt < -128) || (AdrInt > 127)) && (!SymbolQuestionable)) WrError(1370);
               else {
                  CodeLen = 2;
                  if (Condition == -1) BAsmCode[0] = 0xfb;
                  else BAsmCode[0] = 0xd0 + Conditions[Condition].Code;
                  BAsmCode[1] = AdrInt & 0xff;
               }
         }
      }
      return;
   }

   if ((Memo("JP")) || (Memo("CALL"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         OpSize = 1;
         HReg = 0xfc + 2 * Memo("JP");
         DecodeAdr(ArgStr[1], MModReg16 + MModAbs + MModMem + MModImm);
         switch (AdrType) {
            case ModReg16:
               CodeLen = 2;
               BAsmCode[0] = 0xe8 + AdrMode;
               BAsmCode[1] = HReg;
               break;
            case ModAbs:
               CodeLen = 3;
               BAsmCode[0] = 0xe0;
               BAsmCode[1] = AdrVals[0];
               BAsmCode[2] = HReg;
               break;
            case ModMem:
               if (AdrMode > 5) WrError(1350);
               else {
                  CodeLen = 2 + AdrCnt;
                  BAsmCode[0] = 0xe0 + AdrMode;
                  memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                  BAsmCode[1 + AdrCnt] = HReg;
               }
               break;
            case ModImm:
               if ((AdrVals[1] == 0xff) && (Memo("CALL"))) {
                  CodeLen = 2;
                  BAsmCode[0] = 0xfd;
                  BAsmCode[1] = AdrVals[0];
               } else {
                  CodeLen = 3;
                  BAsmCode[0] = HReg;
                  memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               }
               break;
         }
      }
      return;
   }

   if (Memo("CALLV")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         HVal = EvalIntExpression(ArgStr[1], Int4, &OK);
         if (OK) {
            CodeLen = 1;
            BAsmCode[0] = 0xc0 + (HVal & 15);
         }
      }
      return;
   }

   if (Memo("CALLP")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrInt = EvalIntExpression(ArgStr[1], Int16, &OK);
         if (OK)
            if ((Hi(AdrInt) != 0xff) && (Hi(AdrInt) != 0)) WrError(1320);
            else {
               CodeLen = 2;
               BAsmCode[0] = 0xfd;
               BAsmCode[1] = Lo(AdrInt);
            }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_87C800(void) {
   switch (ActPC) {
      case SegCode:
         return (ProgCounter() <= 0xffff);
      default:
         return false;
   }
}

static bool IsDef_87C800(void) {
   return false;
}

static void SwitchFrom_87C800(void) {
   DeinitFields();
}

static void SwitchTo_87C800(void) {
   TurnWords = false;
   ConstMode = ConstModeIntel;
   SetIsOccupied = true;

   PCSymbol = "$";
   HeaderID = 0x54;
   NOPCode = 0x00;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_87C800;
   ChkPC = ChkPC_87C800;
   IsDef = IsDef_87C800;
   SwitchFrom = SwitchFrom_87C800;
   InitFields();
}

void code87c800_init(void) {
   CPU87C00 = AddCPU("87C00", SwitchTo_87C800);
   CPU87C20 = AddCPU("87C20", SwitchTo_87C800);
   CPU87C40 = AddCPU("87C40", SwitchTo_87C800);
   CPU87C70 = AddCPU("87C70", SwitchTo_87C800);
}
