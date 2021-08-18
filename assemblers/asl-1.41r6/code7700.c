// AS-Portierung
// AS-Codegeneratormodul MELPS-7700
#include "stdinc.h"
#include <string.h>

#include "bpemu.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

#include "code7700.h"

typedef struct {
   char *Name;
   Word Code;
   Byte Allowed;
} FixedOrder;

typedef struct {
   char *Name;
   Word Code;
   ShortInt Disp8, Disp16;
} RelOrder;

typedef struct {
   char *Name;
   Byte Code;
} AccOrder;

typedef struct {
   char *Name;
   Byte ACode, MCode;
} RMWOrder;

typedef struct {
   char *Name;
   Byte CodeImm, CodeAbs8, CodeAbs16, CodeIdxX8, CodeIdxX16, CodeIdxY8, CodeIdxY16;
} XYOrder;

typedef struct {
   char *Name;
   Word Code;
   Byte Allowed;
} MulDivOrder;

#define ModNone    (-1)
#define ModImm      0
#define MModImm      (1l << ModImm)
#define ModAbs8     1
#define MModAbs8     (1l << ModAbs8)
#define ModAbs16    2
#define MModAbs16    (1l << ModAbs16)
#define ModAbs24    3
#define MModAbs24    (1l << ModAbs24)
#define ModIdxX8    4
#define MModIdxX8    (1l << ModIdxX8)
#define ModIdxX16   5
#define MModIdxX16   (1l << ModIdxX16)
#define ModIdxX24   6
#define MModIdxX24   (1l << ModIdxX24)
#define ModIdxY8    7
#define MModIdxY8    (1l << ModIdxY8)
#define ModIdxY16   8
#define MModIdxY16   (1l << ModIdxY16)
#define ModIdxY24   9
#define MModIdxY24   (1l << ModIdxY24)
#define ModInd8    10
#define MModInd8     (1l << ModInd8)
#define ModInd16   11
#define MModInd16    (1l << ModInd16)
#define ModInd24   12
#define MModInd24    (1l << ModInd24)
#define ModIndX8   13
#define MModIndX8    (1l << ModIndX8)
#define ModIndX16  14
#define MModIndX16   (1l << ModIndX16)
#define ModIndX24  15
#define MModIndX24   (1l << ModIndX24)
#define ModIndY8   16
#define MModIndY8    (1l << ModIndY8)
#define ModIndY16  17
#define MModIndY16   (1l << ModIndY16)
#define ModIndY24  18
#define MModIndY24   (1l << ModIndY24)
#define ModIdxS8   19
#define MModIdxS8    (1l << ModIdxS8)
#define ModIndS8   20
#define MModIndS8    (1l << ModIndS8)

#define FixedOrderCnt 64

#define RelOrderCnt 13

#define AccOrderCnt 9

#define RMWOrderCnt 6

#define Imm8OrderCnt 5

#define XYOrderCnt 6

#define MulDivOrderCnt 4

#define PushRegCnt 8
static char *PushRegs[PushRegCnt] = { "A", "B", "X", "Y", "DPR", "DT", "PG", "PS" };

#define PrefAccB 0x42

static LongInt Reg_PG, Reg_DT, Reg_X, Reg_M, Reg_DPR, BankReg;

static bool WordSize;
static Byte AdrVals[3];
static ShortInt AdrType;
static bool LFlag;

static FixedOrder *FixedOrders;
static RelOrder *RelOrders;
static AccOrder *AccOrders;
static RMWOrder *RMWOrders;
static FixedOrder *Imm8Orders;
static XYOrder *XYOrders;
static MulDivOrder *MulDivOrders;

static SimpProc SaveInitProc;

static CPUVar CPU65816, CPUM7700, CPUM7750, CPUM7751;

/*---------------------------------------------------------------------------*/

static void AddFixed(char *NName, Word NCode, Byte NAllowed) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ].Code = NCode;
   FixedOrders[InstrZ++].Allowed = NAllowed;
}

static void AddRel(char *NName, Word NCode, ShortInt NDisp8, ShortInt NDisp16) {
   if (InstrZ >= RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name = NName;
   RelOrders[InstrZ].Code = NCode;
   RelOrders[InstrZ].Disp8 = NDisp8;
   RelOrders[InstrZ++].Disp16 = NDisp16;
}

static void AddAcc(char *NName, Byte NCode) {
   if (InstrZ >= AccOrderCnt) exit(255);
   AccOrders[InstrZ].Name = NName;
   AccOrders[InstrZ++].Code = NCode;
}

static void AddRMW(char *NName, Byte NACode, Byte NMCode) {
   if (InstrZ >= RMWOrderCnt) exit(255);
   RMWOrders[InstrZ].Name = NName;
   RMWOrders[InstrZ].MCode = NMCode;
   RMWOrders[InstrZ++].ACode = NACode;
}

static void AddImm8(char *NName, Word NCode, Byte NAllowed) {
   if (InstrZ >= Imm8OrderCnt) exit(255);
   Imm8Orders[InstrZ].Name = NName;
   Imm8Orders[InstrZ].Code = NCode;
   Imm8Orders[InstrZ++].Allowed = NAllowed;
}

static void AddXY(char *NName, Byte NCodeImm, Byte NCodeAbs8, Byte NCodeAbs16, Byte NCodeIdxX8, Byte NCodeIdxX16, Byte NCodeIdxY8, Byte NCodeIdxY16) {
   if (InstrZ >= XYOrderCnt) exit(255);
   XYOrders[InstrZ].Name = NName;
   XYOrders[InstrZ].CodeImm = NCodeImm;
   XYOrders[InstrZ].CodeAbs8 = NCodeAbs8;
   XYOrders[InstrZ].CodeAbs16 = NCodeAbs16;
   XYOrders[InstrZ].CodeIdxX8 = NCodeIdxX8;
   XYOrders[InstrZ].CodeIdxX16 = NCodeIdxX16;
   XYOrders[InstrZ].CodeIdxY8 = NCodeIdxY8;
   XYOrders[InstrZ++].CodeIdxY16 = NCodeIdxY16;
}

static void AddMulDiv(char *NName, Word NCode, Byte NAllowed) {
   if (InstrZ >= MulDivOrderCnt) exit(255);
   MulDivOrders[InstrZ].Name = NName;
   MulDivOrders[InstrZ].Code = NCode;
   MulDivOrders[InstrZ++].Allowed = NAllowed;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("CLC", 0x0018, 15);
   AddFixed("CLI", 0x0058, 15);
   AddFixed("CLM", 0x00d8, 14);
   AddFixed("CLV", 0x00b8, 15);
   AddFixed("DEX", 0x00ca, 15);
   AddFixed("DEY", 0x0088, 15);
   AddFixed("INX", 0x00e8, 15);
   AddFixed("INY", 0x00c8, 15);
   AddFixed("NOP", 0x00ea, 15);
   AddFixed("PHA", 0x0048, 15);
   AddFixed("PHD", 0x000b, 15);
   AddFixed("PHG", 0x004b, 14);
   AddFixed("PHP", 0x0008, 15);
   AddFixed("PHT", 0x008b, 14);
   AddFixed("PHX", 0x00da, 15);
   AddFixed("PHY", 0x005a, 15);
   AddFixed("PLA", 0x0068, 15);
   AddFixed("PLD", 0x002b, 15);
   AddFixed("PLP", 0x0028, 15);
   AddFixed("PLT", 0x00ab, 14);
   AddFixed("PLX", 0x00fa, 15);
   AddFixed("PLY", 0x007a, 15);
   AddFixed("RTI", 0x0040, 15);
   AddFixed("RTL", 0x006b, 15);
   AddFixed("RTS", 0x0060, 15);
   AddFixed("SEC", 0x0038, 15);
   AddFixed("SEI", 0x0078, 15);
   AddFixed("SEM", 0x00f8, 14);
   AddFixed("STP", 0x00db, 15);
   AddFixed("TAD", 0x005b, 15);
   AddFixed("TAS", 0x001b, 15);
   AddFixed("TAX", 0x00aa, 15);
   AddFixed("TAY", 0x00a8, 15);
   AddFixed("TBD", 0x425b, 14);
   AddFixed("TBS", 0x421b, 14);
   AddFixed("TBX", 0x42aa, 14);
   AddFixed("TBY", 0x42a8, 14);
   AddFixed("TDA", 0x007b, 15);
   AddFixed("TDB", 0x427b, 14);
   AddFixed("TSA", 0x003b, 15);
   AddFixed("TSX", 0x00ba, 15);
   AddFixed("TXA", 0x008a, 15);
   AddFixed("TXB", 0x428a, 14);
   AddFixed("TXS", 0x009a, 15);
   AddFixed("TXY", 0x009b, 15);
   AddFixed("TYA", 0x0098, 15);
   AddFixed("TYB", 0x4298, 15);
   AddFixed("TYX", 0x00bb, 15);
   AddFixed("WIT", 0x00cb, 14);
   AddFixed("XAB", 0x8928, 14);
   AddFixed("COP", 0x0002, 1);
   AddFixed("CLD", 0x00d8, 1);
   AddFixed("SED", 0x00f8, 1);
   AddFixed("TCS", 0x001b, 15);
   AddFixed("TSC", 0x003b, 15);
   AddFixed("TCD", 0x005b, 15);
   AddFixed("TDC", 0x007b, 15);
   AddFixed("PHK", 0x004b, 1);
   AddFixed("WAI", 0x00cb, 1);
   AddFixed("XBA", 0x00eb, 1);
   AddFixed("SWA", 0x00eb, 1);
   AddFixed("XCE", 0x00fb, 1);
   AddFixed("DEA", (MomCPU >= CPUM7700) ? 0x001a : 0x003a, 15);
   AddFixed("INA", (MomCPU >= CPUM7700) ? 0x003a : 0x001a, 15);

   RelOrders = (RelOrder *) malloc(sizeof(RelOrder) * RelOrderCnt);
   InstrZ = 0;
   AddRel("BCC", 0x0090, 2, -1);
   AddRel("BLT", 0x0090, 2, -1);
   AddRel("BCS", 0x00b0, 2, -1);
   AddRel("BGE", 0x00b0, 2, -1);
   AddRel("BEQ", 0x00f0, 2, -1);
   AddRel("BMI", 0x0030, 2, -1);
   AddRel("BNE", 0x00d0, 2, -1);
   AddRel("BPL", 0x0010, 2, -1);
   AddRel("BRA", 0x8280, 2, 3);
   AddRel("BVC", 0x0050, 2, -1);
   AddRel("BVS", 0x0070, 2, -1);
   AddRel("BRL", 0x8200, -1, 3);
   AddRel("BRAL", 0x8200, -1, 3);

   AccOrders = (AccOrder *) malloc(sizeof(AccOrder) * AccOrderCnt);
   InstrZ = 0;
   AddAcc("ADC", 0x60);
   AddAcc("AND", 0x20);
   AddAcc("CMP", 0xc0);
   AddAcc("CPA", 0xc0);
   AddAcc("EOR", 0x40);
   AddAcc("LDA", 0xa0);
   AddAcc("ORA", 0x00);
   AddAcc("SBC", 0xe0);
   AddAcc("STA", 0x80);

   RMWOrders = (RMWOrder *) malloc(sizeof(RMWOrder) * RMWOrderCnt);
   InstrZ = 0;
   AddRMW("ASL", 0x0a, 0x06);
   AddRMW("DEC", (MomCPU >= CPUM7700) ? 0x1a : 0x3a, 0xc6);
   AddRMW("ROL", 0x2a, 0x26);
   AddRMW("INC", (MomCPU >= CPUM7700) ? 0x3a : 0x1a, 0xe6);
   AddRMW("LSR", 0x4a, 0x46);
   AddRMW("ROR", 0x6a, 0x66);

   Imm8Orders = (FixedOrder *) malloc(sizeof(FixedOrder) * Imm8OrderCnt);
   InstrZ = 0;
   AddImm8("CLP", 0x00c2, 15);
   AddImm8("REP", 0x00c2, 15);
   AddImm8("LDT", 0x89c2, 14);
   AddImm8("SEP", 0x00e2, 15);
   AddImm8("RMPA", 0x89e2, 8);

   XYOrders = (XYOrder *) malloc(sizeof(XYOrder) * XYOrderCnt);
   InstrZ = 0;
   AddXY("CPX", 0xe0, 0xe4, 0xec, 0xff, 0xff, 0xff, 0xff);
   AddXY("CPY", 0xc0, 0xc4, 0xcc, 0xff, 0xff, 0xff, 0xff);
   AddXY("LDX", 0xa2, 0xa6, 0xae, 0xff, 0xff, 0xb6, 0xbe);
   AddXY("LDY", 0xa0, 0xa4, 0xac, 0xb4, 0xbc, 0xff, 0xff);
   AddXY("STX", 0xff, 0x86, 0x8e, 0xff, 0xff, 0x96, 0xff);
   AddXY("STY", 0xff, 0x84, 0x8c, 0x94, 0xff, 0xff, 0xff);

   MulDivOrders = (MulDivOrder *) malloc(sizeof(MulDivOrder) * MulDivOrderCnt);
   InstrZ = 0;
   AddMulDiv("MPY", 0x0000, 14);
   AddMulDiv("MPYS", 0x0080, 12);
   AddMulDiv("DIV", 0x0020, 14);
   AddMulDiv("DIVS", 0x00a0, 12); /*??? */
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(RelOrders);
   free(AccOrders);
   free(RMWOrders);
   free(Imm8Orders);
   free(XYOrders);
   free(MulDivOrders);
}

/*---------------------------------------------------------------------------*/

static void ChkAdr(LongWord Mask) {
   if (AdrType != ModNone)
      if ((Mask & (1l << ((LongWord) AdrType))) == 0) {
         AdrType = ModNone;
         AdrCnt = 0;
         WrError(1350);
      }
}

static void CodeDisp(char *Asc, LongInt Start, LongWord Mask) {
   bool OK;
   LongInt Adr;
   ShortInt DType;
   int l = strlen(Asc);

   if ((l > 1) && (*Asc == '<')) {
      Asc++;
      DType = 0;
   } else if ((l > 1) && (*Asc == '>'))
      if ((l > 2) && (Asc[1] == '>')) {
         Asc += 2;
         DType = 2;
      } else {
         Asc++;
         DType = 1;
   } else DType = (-1);

   Adr = EvalIntExpression(Asc, UInt24, &OK);

   if (!OK) return;

   if (DType == -1) {
      if ((((Mask & (1l << Start))) != 0) && (Adr >= Reg_DPR) && (Adr < Reg_DPR + 0x100)) DType = 0;
      else if ((((Mask & (2l << Start))) != 0) && ((Adr >> 16) == BankReg)) DType = 1;
      else DType = 2;
   }

   if ((Mask & (1l << (Start + DType))) == 0) WrError(1350);
   else switch (DType) {
         case 0:
            if ((FirstPassUnknown) || (ChkRange(Adr, Reg_DPR, Reg_DPR + 0xff))) {
               AdrCnt = 1;
               AdrType = Start;
               AdrVals[0] = Lo(Adr - Reg_DPR);
            }
            break;
         case 1:
            if ((!FirstPassUnknown) && ((Adr >> 16) != BankReg)) WrError(1320);
            else {
               AdrCnt = 2;
               AdrType = Start + 1;
               AdrVals[0] = Lo(Adr);
               AdrVals[1] = Hi(Adr);
            }
            break;
         case 2:
            AdrCnt = 3;
            AdrType = Start + 2;
            AdrVals[0] = Lo(Adr);
            AdrVals[1] = Hi(Adr);
            AdrVals[2] = Adr >> 16;
            break;
   }
}

static void SplitArg(char *Src, String * HStr, Integer * HCnt) {
   char *p;

   strmove(Src, 1);
   Src[strlen(Src) - 1] = '\0';
   p = QuotPos(Src, ',');
   if (p == NULL) {
      strmaxcpy(HStr[0], Src, 255);
      *HCnt = 1;
   } else {
      *p = '\0';
      strmaxcpy(HStr[0], Src, 255);
      strmaxcpy(HStr[1], p + 1, 255);
      *p = ',';
      *HCnt = 2;
   }
}

static void DecodeAdr(Integer Start, LongWord Mask) {
   Word AdrWord;
   bool OK;
   Integer HCnt;
   String HStr[2];

   AdrType = ModNone;
   AdrCnt = 0;
   BankReg = Reg_DT;

/* I. 1 Parameter */

   if (Start == ArgCnt) {
   /* I.1. immediate */

      if (*ArgStr[Start] == '#') {
         if (WordSize) {
            AdrWord = EvalIntExpression(ArgStr[Start] + 1, Int16, &OK);
            AdrVals[0] = Lo(AdrWord);
            AdrVals[1] = Hi(AdrWord);
         } else AdrVals[0] = EvalIntExpression(ArgStr[Start] + 1, Int8, &OK);
         if (OK) {
            AdrCnt = 1 + WordSize;
            AdrType = ModImm;
         }
         ChkAdr(Mask);
         return;
      }

   /* I.2. indirekt */

      if (IsIndirect(ArgStr[Start])) {
         SplitArg(ArgStr[Start], HStr, &HCnt);

      /* I.2.i. einfach indirekt */

         if (HCnt == 1) {
            CodeDisp(HStr[0], ModInd8, Mask);
            ChkAdr(Mask);
            return;
         }

      /* I.2.ii indirekt mit Vorindizierung */

         else if (strcasecmp(HStr[1], "X") == 0) {
            CodeDisp(HStr[0], ModIndX8, Mask);
            ChkAdr(Mask);
            return;
         }

         else {
            WrError(1350);
            ChkAdr(Mask);
            return;
         }
      }

   /* I.3. absolut */

      else {
         CodeDisp(ArgStr[Start], ModAbs8, Mask);
         ChkAdr(Mask);
         return;
      }
   }

/* II. 2 Parameter */

   else if (Start + 1 == ArgCnt) {
   /* II.1 indirekt mit Nachindizierung */

      if (IsIndirect(ArgStr[Start])) {
         if (strcasecmp(ArgStr[Start + 1], "Y") != 0) WrError(1350);
         else {
            SplitArg(ArgStr[Start], HStr, &HCnt);

         /* II.1.i. (d),Y */

            if (HCnt == 1) {
               CodeDisp(HStr[0], ModIndY8, Mask);
               ChkAdr(Mask);
               return;
            }

         /* II.1.ii. (d,S),Y */

            else if (strcasecmp(HStr[1], "S") == 0) {
               AdrVals[0] = EvalIntExpression(HStr[0], Int8, &OK);
               if (OK) {
                  AdrType = ModIndS8;
                  AdrCnt = 1;
               }
               ChkAdr(Mask);
               return;
            }

            else WrError(1350);
         }
         ChkAdr(Mask);
         return;
      }

   /* II.2. einfach indiziert */

      else {
      /* II.2.i. d,X */

         if (strcasecmp(ArgStr[Start + 1], "X") == 0) {
            CodeDisp(ArgStr[Start], ModIdxX8, Mask);
            ChkAdr(Mask);
            return;
         }

      /* II.2.ii. d,Y */

         else if (strcasecmp(ArgStr[Start + 1], "Y") == 0) {
            CodeDisp(ArgStr[Start], ModIdxY8, Mask);
            ChkAdr(Mask);
            return;
         }

      /* II.2.iii. d,S */

         else if (strcasecmp(ArgStr[Start + 1], "S") == 0) {
            AdrVals[0] = EvalIntExpression(ArgStr[Start], Int8, &OK);
            if (OK) {
               AdrType = ModIdxS8;
               AdrCnt = 1;
            }
            ChkAdr(Mask);
            return;
         }

         else WrError(1350);
      }
   }

   else WrError(1110);
}

static bool DecodePseudo(void) {
#define ASSUME7700Count 5
   static ASSUMERec ASSUME7700s[ASSUME7700Count] = {
      { "PG", &Reg_PG, 0, 0xff, 0x100 },
      { "DT", &Reg_DT, 0, 0xff, 0x100 },
      { "X", &Reg_X, 0, 1, -1 },
      { "M", &Reg_M, 0, 1, -1 },
      { "DPR", &Reg_DPR, 0, 0xffff, 0x10000 }
   };

   if (Memo("ASSUME")) {
      CodeASSUME(ASSUME7700s, ASSUME7700Count);
      return true;
   }

   return false;
}

static bool LMemo(char *s) {
   String tmp;

   if (Memo(s)) {
      LFlag = false;
      return true;
   } else {
      strmaxcpy(tmp, s, 255);
      strmaxcat(tmp, "L", 255);
      if (Memo(tmp)) {
         LFlag = true;
         return true;
      } else return false;
   }
}

static void MakeCode_7700(void) {
   Integer z, Start;
   LongInt AdrLong, Mask;
   bool OK, Rel;

   CodeLen = 0;
   DontPrint = false;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(false)) return;
   if (DecodeIntelPseudo(false)) return;

/* ohne Argument */

   if (Memo("BRK")) {
      if (ArgCnt != 0) WrError(1110);
      else {
         CodeLen = 2;
         BAsmCode[0] = 0x00;
         BAsmCode[1] = NOPCode;
      }
      return;
   }

   for (z = 0; z < FixedOrderCnt; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else if (((FixedOrders[z].Allowed >> (MomCPU - CPU65816)) & 1) == 0) WrError(1500);
         else {
            CodeLen = 1 + (Hi(FixedOrders[z].Code) != 0);
            if (CodeLen == 2) BAsmCode[0] = Hi(FixedOrders[z].Code);
            BAsmCode[CodeLen - 1] = Lo(FixedOrders[z].Code);
         }
         return;
      }

   if ((Memo("PHB")) || (Memo("PLB"))) {
      if (ArgCnt != 0) WrError(1110);
      else {
         if (MomCPU >= CPUM7700) {
            CodeLen = 2;
            BAsmCode[0] = PrefAccB;
            BAsmCode[1] = 0x48;
         } else {
            CodeLen = 1;
            BAsmCode[0] = 0x8b;
         }
         if (Memo("PLB")) BAsmCode[CodeLen - 1] += 0x20;
      }
      return;
   }

/* relative Adressierung */

   for (z = 0; z < RelOrderCnt; z++)
      if (Memo(RelOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            if (*ArgStr[1] == '#') strmove(ArgStr[1], 1);
            AdrLong = EvalIntExpression(ArgStr[1], Int32, &OK);
            if (OK) {
               OK = RelOrders[z].Disp8 == -1;
               if (OK) AdrLong -= EProgCounter() + RelOrders[z].Disp16;
               else {
                  AdrLong -= EProgCounter() + RelOrders[z].Disp8;
                  if (((AdrLong > 127) || (AdrLong < -128)) && (!SymbolQuestionable) && (RelOrders[z].Disp16 != -1)) {
                     OK = true;
                     AdrLong -= RelOrders[z].Disp16 - RelOrders[z].Disp8;
                  }
               }
               if (OK) /* d16 */
                  if (((AdrLong < -32768) || (AdrLong > 32767)) && (!SymbolQuestionable)) WrError(1330);
                  else {
                     CodeLen = 3;
                     BAsmCode[0] = Hi(RelOrders[z].Code);
                     BAsmCode[1] = Lo(AdrLong);
                     BAsmCode[2] = Hi(AdrLong);
               } else /* d8 */ if (((AdrLong < -128) || (AdrLong > 127)) && (!SymbolQuestionable)) WrError(1370);
               else {
                  CodeLen = 2;
                  BAsmCode[0] = Lo(RelOrders[z].Code);
                  BAsmCode[1] = Lo(AdrLong);
               }
            }
         }
         return;
      }

/* mit Akku */

   for (z = 0; z < AccOrderCnt; z++)
      if (LMemo(AccOrders[z].Name)) {
         if ((ArgCnt == 0) || (ArgCnt > 3)) WrError(1110);
         else {
            WordSize = (Reg_M == 0);
            if (strcasecmp(ArgStr[1], "A") == 0) Start = 2;
            else if (strcasecmp(ArgStr[1], "B") == 0) {
               Start = 2;
               BAsmCode[0] = PrefAccB;
               CodeLen++;
               if (MomCPU == CPU65816) {
                  WrError(1505);
                  return;
               }
            } else Start = 1;
            Mask = MModAbs8 + MModAbs16 + MModAbs24 + MModIdxX8 + MModIdxX16 + MModIdxX24 + MModIdxY16 + MModInd8 + MModIndX8 + MModIndY8 + MModIdxS8 + MModIndS8;
            if (!LMemo("STA")) Mask += MModImm;
            DecodeAdr(Start, Mask);
            if (AdrType == ModNone) ;
            else if ((LFlag) && (AdrType != ModInd8) && (AdrType != ModIndY8)) WrError(1350);
            else {
               switch (AdrType) {
                  case ModImm:
                     BAsmCode[CodeLen] = AccOrders[z].Code + 0x09;
                     break;
                  case ModAbs8:
                     BAsmCode[CodeLen] = AccOrders[z].Code + 0x05;
                     break;
                  case ModAbs16:
                     BAsmCode[CodeLen] = AccOrders[z].Code + 0x0d;
                     break;
                  case ModAbs24:
                     BAsmCode[CodeLen] = AccOrders[z].Code + 0x0f;
                     break;
                  case ModIdxX8:
                     BAsmCode[CodeLen] = AccOrders[z].Code + 0x15;
                     break;
                  case ModIdxX16:
                     BAsmCode[CodeLen] = AccOrders[z].Code + 0x1d;
                     break;
                  case ModIdxX24:
                     BAsmCode[CodeLen] = AccOrders[z].Code + 0x1f;
                     break;
                  case ModIdxY16:
                     BAsmCode[CodeLen] = AccOrders[z].Code + 0x19;
                     break;
                  case ModInd8:
                     if (LFlag) BAsmCode[CodeLen] = AccOrders[z].Code + 0x07;
                     else BAsmCode[CodeLen] = AccOrders[z].Code + 0x12;
                     break;
                  case ModIndX8:
                     BAsmCode[CodeLen] = AccOrders[z].Code + 0x01;
                     break;
                  case ModIndY8:
                     if (LFlag) BAsmCode[CodeLen] = AccOrders[z].Code + 0x17;
                     else BAsmCode[CodeLen] = AccOrders[z].Code + 0x11;
                     break;
                  case ModIdxS8:
                     BAsmCode[CodeLen] = AccOrders[z].Code + 0x03;
                     break;
                  case ModIndS8:
                     BAsmCode[CodeLen] = AccOrders[z].Code + 0x13;
                     break;
               }
               memcpy(BAsmCode + CodeLen + 1, AdrVals, AdrCnt);
               CodeLen += 1 + AdrCnt;
            }
         }
         return;
      }

   if ((Memo("EXTS")) || (Memo("EXTZ"))) {
      if (ArgCnt == 0) {
         strmaxcpy(ArgStr[1], "A", 255);
         ArgCnt = 1;
      }
      if (ArgCnt != 1) WrError(1110);
      else if (MomCPU < CPUM7750) WrError(1500);
      else {
         BAsmCode[1] = 0x8b + (Memo("EXTZ") << 5);
         BAsmCode[0] = 0;
         if (strcasecmp(ArgStr[1], "A") == 0) BAsmCode[0] = 0x89;
         else if (strcasecmp(ArgStr[1], "B") == 0) BAsmCode[0] = 0x42;
         else WrError(1350);
         if (BAsmCode[0] != 0) CodeLen = 2;
      }
      return;
   }

   for (z = 0; z < RMWOrderCnt; z++)
      if (Memo(RMWOrders[z].Name)) {
         if ((ArgCnt == 0) || ((ArgCnt == 1) && (strcasecmp(ArgStr[1], "A") == 0))) {
            CodeLen = 1;
            BAsmCode[0] = RMWOrders[z].ACode;
         } else if ((ArgCnt == 1) && (strcasecmp(ArgStr[1], "B") == 0)) {
            CodeLen = 2;
            BAsmCode[0] = PrefAccB;
            BAsmCode[1] = RMWOrders[z].ACode;
            if (MomCPU == CPU65816) {
               WrError(1505);
               return;
            }
         } else if (ArgCnt > 2) WrError(1110);
         else {
            DecodeAdr(1, MModAbs8 + MModAbs16 + MModIdxX8 + MModIdxX16);
            if (AdrType != ModNone) {
               switch (AdrType) {
                  case ModAbs8:
                     BAsmCode[0] = RMWOrders[z].MCode;
                     break;
                  case ModAbs16:
                     BAsmCode[0] = RMWOrders[z].MCode + 8;
                     break;
                  case ModIdxX8:
                     BAsmCode[0] = RMWOrders[z].MCode + 16;
                     break;
                  case ModIdxX16:
                     BAsmCode[0] = RMWOrders[z].MCode + 24;
                     break;
               }
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               CodeLen = 1 + AdrCnt;
            }
         }
         return;
      }

   if (Memo("ASR")) {
      if (MomCPU < CPUM7750) WrError(1500);
      else if ((ArgCnt == 0) || ((ArgCnt == 1) && (strcasecmp(ArgStr[1], "A") == 0))) {
         BAsmCode[0] = 0x89;
         BAsmCode[1] = 0x08;
         CodeLen = 2;
      } else if ((ArgCnt == 1) && (strcasecmp(ArgStr[1], "B") == 0)) {
         BAsmCode[0] = 0x42;
         BAsmCode[1] = 0x08;
         CodeLen = 2;
      } else if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
      else {
         DecodeAdr(1, MModAbs8 + MModIdxX8 + MModAbs16 + MModIdxX16);
         if (AdrType != ModNone) {
            BAsmCode[0] = 0x89;
            switch (AdrType) {
               case ModAbs8:
                  BAsmCode[1] = 0x06;
                  break;
               case ModIdxX8:
                  BAsmCode[1] = 0x16;
                  break;
               case ModAbs16:
                  BAsmCode[1] = 0x0e;
                  break;
               case ModIdxX16:
                  BAsmCode[1] = 0x1e;
                  break;
            }
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
            CodeLen = 2 + AdrCnt;
         }
      }
      return;
   }

   if ((Memo("BBC")) || (Memo("BBS"))) {
      if (ArgCnt != 3) WrError(1110);
      else if (MomCPU < CPUM7700) WrError(1500);
      else {
         WordSize = (Reg_M == 0);
         ArgCnt = 2;
         DecodeAdr(2, MModAbs8 + MModAbs16);
         if (AdrType != ModNone) {
            BAsmCode[0] = 0x24;
            if (Memo("BBC")) BAsmCode[0] += 0x10;
            if (AdrType == ModAbs16) BAsmCode[0] += 8;
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            CodeLen = 1 + AdrCnt;
            ArgCnt = 1;
            DecodeAdr(1, MModImm);
            if (AdrType == ModNone) CodeLen = 0;
            else {
               memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
               CodeLen += AdrCnt;
               AdrLong = EvalIntExpression(ArgStr[3], UInt24, &OK) - (EProgCounter() + CodeLen + 1);
               if (!OK) CodeLen = 0;
               else if ((!SymbolQuestionable) && ((AdrLong < -128) || (AdrLong > 127))) {
                  WrError(1370);
                  CodeLen = 0;
               } else {
                  BAsmCode[CodeLen] = Lo(AdrLong);
                  CodeLen++;
               }
            }
         }
      }
      return;
   }

   if (Memo("BIT")) {
      if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
      else if (MomCPU != CPU65816) WrError(1500);
      else {
         WordSize = false;
         DecodeAdr(1, MModAbs8 + MModAbs16 + MModIdxX8 + MModIdxX16 + MModImm);
         if (AdrType != ModNone) {
            switch (AdrType) {
               case ModAbs8:
                  BAsmCode[0] = 0x24;
                  break;
               case ModAbs16:
                  BAsmCode[0] = 0x2c;
                  break;
               case ModIdxX8:
                  BAsmCode[0] = 0x34;
                  break;
               case ModIdxX16:
                  BAsmCode[0] = 0x3c;
                  break;
               case ModImm:
                  BAsmCode[0] = 0x89;
                  break;
            }
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            CodeLen = 1 + AdrCnt;
         }
      }
      return;
   }

   if ((Memo("CLB")) || (Memo("SEB"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (MomCPU < CPUM7700) WrError(1500);
      else {
         WordSize = (Reg_M == 0);
         DecodeAdr(2, MModAbs8 + MModAbs16);
         if (AdrType != ModNone) {
            BAsmCode[0] = 0x04;
            if (Memo("CLB")) BAsmCode[0] += 0x10;
            if (AdrType == ModAbs16) BAsmCode[0] += 8;
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            CodeLen = 1 + AdrCnt;
            ArgCnt = 1;
            DecodeAdr(1, MModImm);
            if (AdrType == ModNone) CodeLen = 0;
            else {
               memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
               CodeLen += AdrCnt;
            }
         }
      }
      return;
   }

   if ((Memo("TSB")) || (Memo("TRB"))) {
      if (MomCPU == CPU65816) {
         if (ArgCnt != 1) WrError(1110);
         else {
            DecodeAdr(1, MModAbs8 + MModAbs16);
            if (AdrType != ModNone) {
               BAsmCode[0] = 0x04;
               if (Memo("TRB")) BAsmCode[0] += 0x10;
               if (AdrType == ModAbs16) BAsmCode[0] += 8;
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               CodeLen = 1 + AdrCnt;
            }
         }
      } else if (Memo("TRB")) WrError(1500);
      else if (ArgCnt != 0) WrError(1110);
      else {
         CodeLen = 2;
         BAsmCode[0] = 0x42;
         BAsmCode[1] = 0x3b;
      }
      return;
   }

   for (z = 0; z < Imm8OrderCnt; z++)
      if (Memo(Imm8Orders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (((Imm8Orders[z].Allowed >> (MomCPU - CPU65816)) & 1) == 0) WrError(1500);
         else {
            WordSize = false;
            DecodeAdr(1, MModImm);
            if (AdrType == ModImm) {
               CodeLen = 1 + (Hi(Imm8Orders[z].Code) != 0);
               if (CodeLen == 2) BAsmCode[0] = Hi(Imm8Orders[z].Code);
               BAsmCode[CodeLen - 1] = Lo(Imm8Orders[z].Code);
               memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
               CodeLen += AdrCnt;
            }
         }
         return;
      }

   if (Memo("RLA")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         WordSize = (Reg_M == 0);
         DecodeAdr(1, MModImm);
         if (AdrType != ModNone) {
            CodeLen = 2 + AdrCnt;
            BAsmCode[0] = 0x89;
            BAsmCode[1] = 0x49;
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
         }
      }
      return;
   }

   for (z = 0; z < XYOrderCnt; z++)
      if (Memo(XYOrders[z].Name)) {
         if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
         else {
            WordSize = (Reg_X == 0);
            Mask = 0;
            if (XYOrders[z].CodeImm != 0xff) Mask += MModImm;
            if (XYOrders[z].CodeAbs8 != 0xff) Mask += MModAbs8;
            if (XYOrders[z].CodeAbs16 != 0xff) Mask += MModAbs16;
            if (XYOrders[z].CodeIdxX8 != 0xff) Mask += MModIdxX8;
            if (XYOrders[z].CodeIdxX16 != 0xff) Mask += MModIdxX16;
            if (XYOrders[z].CodeIdxY8 != 0xff) Mask += MModIdxY8;
            if (XYOrders[z].CodeIdxY16 != 0xff) Mask += MModIdxY16;
            DecodeAdr(1, Mask);
            if (AdrType != ModNone) {
               switch (AdrType) {
                  case ModImm:
                     BAsmCode[0] = XYOrders[z].CodeImm;
                     break;
                  case ModAbs8:
                     BAsmCode[0] = XYOrders[z].CodeAbs8;
                     break;
                  case ModAbs16:
                     BAsmCode[0] = XYOrders[z].CodeAbs16;
                     break;
                  case ModIdxX8:
                     BAsmCode[0] = XYOrders[z].CodeIdxX8;
                     break;
                  case ModIdxY8:
                     BAsmCode[0] = XYOrders[z].CodeIdxY8;
                     break;
                  case ModIdxX16:
                     BAsmCode[0] = XYOrders[z].CodeIdxX16;
                     break;
                  case ModIdxY16:
                     BAsmCode[0] = XYOrders[z].CodeIdxY16;
                     break;
               }
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               CodeLen = 1 + AdrCnt;
            }
         }
         return;
      }

   for (z = 0; z < MulDivOrderCnt; z++)
      if (LMemo(MulDivOrders[z].Name)) {
         if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
         else if (((MulDivOrders[z].Allowed >> (MomCPU - CPU65816)) & 1) == 0) WrError(1500);
         else {
            WordSize = (Reg_M == 0);
            DecodeAdr(1, MModImm + MModAbs8 + MModAbs16 + MModAbs24 + MModIdxX8 + MModIdxX16 + MModIdxX24 + MModIdxY16 + MModInd8 + MModIndX8 + MModIndY8 + MModIdxS8 + MModIndS8);
            if (AdrType == ModNone) ;
            else if ((LFlag) && (AdrType != ModInd8) && (AdrType != ModIndY8)) WrError(1350);
            else {
               BAsmCode[0] = 0x89;
               switch (AdrType) {
                  case ModImm:
                     BAsmCode[1] = 0x09;
                     break;
                  case ModAbs8:
                     BAsmCode[1] = 0x05;
                     break;
                  case ModAbs16:
                     BAsmCode[1] = 0x0d;
                     break;
                  case ModAbs24:
                     BAsmCode[1] = 0x0f;
                     break;
                  case ModIdxX8:
                     BAsmCode[1] = 0x15;
                     break;
                  case ModIdxX16:
                     BAsmCode[1] = 0x1d;
                     break;
                  case ModIdxX24:
                     BAsmCode[1] = 0x1f;
                     break;
                  case ModIdxY16:
                     BAsmCode[1] = 0x19;
                     break;
                  case ModInd8:
                     BAsmCode[1] = (LFlag) ? 0x07 : 0x12;
                     break;
                  case ModIndX8:
                     BAsmCode[1] = 0x01;
                     break;
                  case ModIndY8:
                     BAsmCode[1] = (LFlag) ? 0x17 : 0x11;
                     break;
                  case ModIdxS8:
                     BAsmCode[1] = 0x03;
                     break;
                  case ModIndS8:
                     BAsmCode[1] = 0x13;
                     break;
               }
               BAsmCode[1] += MulDivOrders[z].Code;
               memcpy(BAsmCode + 2, AdrVals, AdrCnt);
               CodeLen = 2 + AdrCnt;
            }
         }
         return;
      }

   if ((Memo("JML")) || (Memo("JSL"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
         if (OK) {
            CodeLen = 4;
            BAsmCode[0] = (Memo("JSL")) ? 0x22 : 0x5c;
            BAsmCode[1] = AdrLong >> 16;
            BAsmCode[2] = Hi(AdrLong);
            BAsmCode[3] = Lo(AdrLong);
         }
      }
      return;
   }

   if ((LMemo("JMP")) || (LMemo("JSR"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         BankReg = Reg_PG;
         Mask = MModAbs24 + MModIndX16;
         if (!LFlag) Mask += MModAbs16;
         DecodeAdr(1, Mask + ((LMemo("JSR")) ? 0 : MModInd16));
         if (AdrType != ModNone) {
            switch (AdrType) {
               case ModAbs16:
                  BAsmCode[0] = (LMemo("JSR")) ? 0x20 : 0x4c;
                  break;
               case ModAbs24:
                  BAsmCode[0] = (LMemo("JSR")) ? 0x22 : 0x5c;
                  break;
               case ModIndX16:
                  BAsmCode[0] = (LMemo("JSR")) ? 0xfc : 0x7c;
                  break;
               case ModInd16:
                  BAsmCode[0] = (LFlag) ? 0xdc : 0x6c;
                  break;
            }
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            CodeLen = 1 + AdrCnt;
         }
      }
      return;
   }

   if (Memo("LDM")) {
      if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
      else if (MomCPU < CPUM7700) WrError(1500);
      else {
         DecodeAdr(2, MModAbs8 + MModAbs16 + MModIdxX8 + MModIdxX16);
         if (AdrType != ModNone) {
            switch (AdrType) {
               case ModAbs8:
                  BAsmCode[0] = 0x64;
                  break;
               case ModAbs16:
                  BAsmCode[0] = 0x9c;
                  break;
               case ModIdxX8:
                  BAsmCode[0] = 0x74;
                  break;
               case ModIdxX16:
                  BAsmCode[0] = 0x9e;
                  break;
            }
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            CodeLen = 1 + AdrCnt;
            WordSize = (Reg_M == 0);
            ArgCnt = 1;
            DecodeAdr(1, MModImm);
            if (AdrType == ModNone) CodeLen = 0;
            else {
               memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
               CodeLen += AdrCnt;
            }
         }
      }
      return;
   }

   if (Memo("STZ")) {
      if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
      else if (MomCPU != CPU65816) WrError(1500);
      else {
         DecodeAdr(1, MModAbs8 + MModAbs16 + MModIdxX8 + MModIdxX16);
         if (AdrType != ModNone) {
            switch (AdrType) {
               case ModAbs8:
                  BAsmCode[0] = 0x64;
                  break;
               case ModAbs16:
                  BAsmCode[0] = 0x9c;
                  break;
               case ModIdxX8:
                  BAsmCode[0] = 0x74;
                  break;
               case ModIdxX16:
                  BAsmCode[0] = 0x9e;
                  break;
            }
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            CodeLen = 1 + AdrCnt;
         }
      }
      return;
   }

   if ((Memo("MVN")) || (Memo("MVP"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         AdrLong = EvalIntExpression(ArgStr[1], Int32, &OK);
         if (OK) {
            Mask = EvalIntExpression(ArgStr[2], Int32, &OK);
            if (!OK) ;
            else if (((Mask & 0xff000000) != 0) || ((AdrLong & 0xff000000) != 0)) WrError(1320);
            else {
               BAsmCode[0] = (Memo("MVN")) ? 0x54 : 0x44;
               BAsmCode[1] = AdrLong >> 16;
               BAsmCode[2] = Mask >> 16;
               CodeLen = 3;
            }
         }
      }
      return;
   }

   if ((Memo("PSH")) || (Memo("PUL"))) {
      if (ArgCnt == 0) WrError(1110);
      else if (MomCPU < CPUM7700) WrError(1500);
      else {
         BAsmCode[0] = 0xeb + (Memo("PUL") << 4);
         BAsmCode[1] = 0;
         OK = true;
         z = 1;
         while ((z <= ArgCnt) && (OK)) {
            if (*ArgStr[z] == '#')
               BAsmCode[1] |= EvalIntExpression(ArgStr[z] + 1, Int8, &OK);
            else {
               Start = 0;
               while ((Start < PushRegCnt) && (strcasecmp(PushRegs[Start], ArgStr[z]) != 0)) Start++;
               OK = (Start < PushRegCnt);
               if (OK) BAsmCode[1] |= 1l << Start;
               else WrXError(1980, ArgStr[z]);
            }
            z++;
         }
         if (OK) CodeLen = 2;
      }
      return;
   }

   if (Memo("PEA")) {
      WordSize = true;
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(1, MModImm);
         if (AdrType != ModNone) {
            CodeLen = 1 + AdrCnt;
            BAsmCode[0] = 0xf4;
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
         }
      }
      return;
   }

   if (Memo("PEI")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         if (*ArgStr[1] == '#') strmove(ArgStr[1], 1);
         DecodeAdr(1, MModAbs8);
         if (AdrType != ModNone) {
            CodeLen = 1 + AdrCnt;
            BAsmCode[0] = 0xd4;
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
         }
      }
      return;
   }

   if (Memo("PER")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         Rel = true;
         if (*ArgStr[1] == '#') {
            strmove(ArgStr[1], 1);
            Rel = false;
         }
         BAsmCode[0] = 0x62;
         if (Rel) {
            AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK) - (EProgCounter() + 2);
            if (!OK) ;
            else if ((AdrLong < -32768) || (AdrLong > 32767)) WrError(1370);
            else {
               CodeLen = 3;
               BAsmCode[1] = AdrLong & 0xff;
               BAsmCode[2] = (AdrLong >> 8) & 0xff;
            }
         } else {
            z = EvalIntExpression(ArgStr[1], Int16, &OK);
            if (OK) {
               CodeLen = 3;
               BAsmCode[1] = Lo(z);
               BAsmCode[2] = Hi(z);
            }
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static void InitCode_7700(void) {
   SaveInitProc();
   Reg_PG = 0;
   Reg_DT = 0;
   Reg_X = 0;
   Reg_M = 0;
   Reg_DPR = 0;
}

static bool ChkPC_7700(void) {
   switch (ActPC) {
      case SegCode:
         return (ProgCounter() <= 0xffffff);
      default:
         return false;
   }
}

static bool IsDef_7700(void) {
   return false;
}

static void SwitchFrom_7700(void) {
   DeinitFields();
}

static void SwitchTo_7700(void) {
   TurnWords = false;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "*";
   HeaderID = 0x19;
   NOPCode = 0xea;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_7700;
   ChkPC = ChkPC_7700;
   IsDef = IsDef_7700;
   SwitchFrom = SwitchFrom_7700;
   InitFields();
}

void code7700_init(void) {
   CPU65816 = AddCPU("65816", SwitchTo_7700);
   CPUM7700 = AddCPU("MELPS7700", SwitchTo_7700);
   CPUM7750 = AddCPU("MELPS7750", SwitchTo_7700);
   CPUM7751 = AddCPU("MELPS7751", SwitchTo_7700);

   SaveInitProc = InitPassProc;
   InitPassProc = InitCode_7700;
}
