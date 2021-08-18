// AS-Portierung
// Codegenerator SGS-Thomson ST9
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

typedef struct {
   char *Name;
   int len;
   Word Code;
} ALUOrder;

#define WorkOfs 0xd0

#define ModNone (-1)
#define ModReg 0
#define MModReg (1l << ModReg) /* Rn */
#define ModWReg 1
#define MModWReg (1l << ModWReg) /* rn */
#define ModRReg 2
#define MModRReg (1l << ModRReg) /* RRn */
#define ModWRReg 3
#define MModWRReg (1l << ModWRReg) /* rrn */
#define ModIReg 4
#define MModIReg (1l << ModIReg) /* (Rn) */
#define ModIWReg 5
#define MModIWReg (1l << ModIWReg) /* (rn) */
#define ModIRReg 6
#define MModIRReg (1l << ModIRReg) /* (RRn) */
#define ModIWRReg 7
#define MModIWRReg (1l << ModIWRReg) /* (rrn) */
#define ModIncWReg 8
#define MModIncWReg (1l << ModIncWReg) /* (rn)+ */
#define ModIncWRReg 9
#define MModIncWRReg (1l << ModIncWRReg) /* (rrn)+ */
#define ModDecWRReg 10
#define MModDecWRReg (1l << ModDecWRReg) /* -(rrn) */
#define ModDisp8WReg 11
#define MModDisp8WReg (1l << ModDisp8WReg) /* d8(rn) */
#define ModDisp8WRReg 12
#define MModDisp8WRReg (1l << ModDisp8WRReg) /* d8(rrn) */
#define ModDisp16WRReg 13
#define MModDisp16WRReg (1l << ModDisp16WRReg) /* d16(rrn) */
#define ModDispRWRReg 14
#define MModDispRWRReg (1l << ModDispRWRReg) /* rrm(rrn */
#define ModAbs 15
#define MModAbs (1l << ModAbs) /* NN */
#define ModImm 16
#define MModImm (1l << ModImm) /* #N/#NN */
#define ModDisp8RReg 17
#define MModDisp8RReg (1l << ModDisp8RReg) /* d8(RRn) */
#define ModDisp16RReg 18
#define MModDisp16RReg (1l << ModDisp16RReg) /* d16(RRn) */

#define FixedOrderCnt 12
#define ALUOrderCnt 10
#define RegOrderCnt 13
#define Reg16OrderCnt 8
#define Bit2OrderCnt 4
#define Bit1OrderCnt 4
#define ConditionCnt 20
#define LoadOrderCnt 4

static CPUVar CPUST9020, CPUST9030, CPUST9040, CPUST9050;

static FixedOrder *FixedOrders;
static ALUOrder *ALUOrders;
static FixedOrder *RegOrders;
static FixedOrder *Reg16Orders;
static FixedOrder *Bit2Orders;
static FixedOrder *Bit1Orders;
static FixedOrder *Conditions;
static FixedOrder *LoadOrders;

static ShortInt AdrMode, AbsSeg;
static Byte AdrPart, OpSize;
static Byte AdrVals[3];

static SimpProc SaveInitProc;
static LongInt DPAssume;

/*--------------------------------------------------------------------------*/

static void AddFixed(char *NName, Word NCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddALU(char *NName, Word NCode) {
   if (InstrZ >= ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name = NName;
   ALUOrders[InstrZ].len = strlen(NName);
   ALUOrders[InstrZ++].Code = NCode;
}

static void AddReg(char *NName, Word NCode) {
   if (InstrZ >= RegOrderCnt) exit(255);
   RegOrders[InstrZ].Name = NName;
   RegOrders[InstrZ++].Code = NCode;
}

static void AddReg16(char *NName, Word NCode) {
   if (InstrZ >= Reg16OrderCnt) exit(255);
   Reg16Orders[InstrZ].Name = NName;
   Reg16Orders[InstrZ++].Code = NCode;
}

static void AddBit2(char *NName, Word NCode) {
   if (InstrZ >= Bit2OrderCnt) exit(255);
   Bit2Orders[InstrZ].Name = NName;
   Bit2Orders[InstrZ++].Code = NCode;
}

static void AddBit1(char *NName, Word NCode) {
   if (InstrZ >= Bit1OrderCnt) exit(255);
   Bit1Orders[InstrZ].Name = NName;
   Bit1Orders[InstrZ++].Code = NCode;
}

static void AddCondition(char *NName, Word NCode) {
   if (InstrZ >= ConditionCnt) exit(255);
   Conditions[InstrZ].Name = NName;
   Conditions[InstrZ++].Code = NCode;
}

static void AddLoad(char *NName, Word NCode) {
   if (InstrZ >= LoadOrderCnt) exit(255);
   LoadOrders[InstrZ].Name = NName;
   LoadOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("CCF", 0x0061);
   AddFixed("DI", 0x0010);
   AddFixed("EI", 0x0000);
   AddFixed("HALT", 0xbf01);
   AddFixed("IRET", 0x00d3);
   AddFixed("NOP", 0x00ff);
   AddFixed("RCF", 0x0011);
   AddFixed("RET", 0x0046);
   AddFixed("SCF", 0x0001);
   AddFixed("SDM", 0x00fe);
   AddFixed("SPM", 0x00ee);
   AddFixed("WFI", 0xef01);

   ALUOrders = (ALUOrder *) malloc(sizeof(ALUOrder) * ALUOrderCnt);
   InstrZ = 0;
   AddALU("ADC", 3);
   AddALU("ADD", 4);
   AddALU("AND", 1);
   AddALU("CP", 9);
   AddALU("OR", 0);
   AddALU("SBC", 2);
   AddALU("SUB", 5);
   AddALU("TCM", 8);
   AddALU("TM", 10);
   AddALU("XOR", 6);

   RegOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * RegOrderCnt);
   InstrZ = 0;
   AddReg("CLR", 0x90);
   AddReg("CPL", 0x80);
   AddReg("DA", 0x70);
   AddReg("DEC", 0x40);
   AddReg("INC", 0x50);
   AddReg("POP", 0x76);
   AddReg("POPU", 0x20);
   AddReg("RLC", 0xb0);
   AddReg("ROL", 0xa0);
   AddReg("ROR", 0xc0);
   AddReg("RRC", 0xd0);
   AddReg("SRA", 0xe0);
   AddReg("SWAP", 0xf0);

   Reg16Orders = (FixedOrder *) malloc(sizeof(FixedOrder) * Reg16OrderCnt);
   InstrZ = 0;
   AddReg16("DECW", 0xcf);
   AddReg16("EXT", 0xc6);
   AddReg16("INCW", 0xdf);
   AddReg16("POPUW", 0xb7);
   AddReg16("POPW", 0x75);
   AddReg16("RLCW", 0x8f);
   AddReg16("RRCW", 0x36);
   AddReg16("SRAW", 0x2f);

   Bit2Orders = (FixedOrder *) malloc(sizeof(FixedOrder) * Bit2OrderCnt);
   InstrZ = 0;
   AddBit2("BAND", 0x1f);
   AddBit2("BLD", 0xf2);
   AddBit2("BOR", 0x0f);
   AddBit2("BXOR", 0x6f);

   Bit1Orders = (FixedOrder *) malloc(sizeof(FixedOrder) * Bit1OrderCnt);
   InstrZ = 0;
   AddBit1("BCPL", 0x6f);
   AddBit1("BRES", 0x1f);
   AddBit1("BSET", 0x0f);
   AddBit1("BTSET", 0xf2);

   Conditions = (FixedOrder *) malloc(sizeof(FixedOrder) * ConditionCnt);
   InstrZ = 0;
   AddCondition("F", 0x0);
   AddCondition("T", 0x8);
   AddCondition("C", 0x7);
   AddCondition("NC", 0xf);
   AddCondition("Z", 0x6);
   AddCondition("NZ", 0xe);
   AddCondition("PL", 0xd);
   AddCondition("MI", 0x5);
   AddCondition("OV", 0x4);
   AddCondition("NOV", 0xc);
   AddCondition("EQ", 0x6);
   AddCondition("NE", 0xe);
   AddCondition("GE", 0x9);
   AddCondition("LT", 0x1);
   AddCondition("GT", 0xa);
   AddCondition("LE", 0x2);
   AddCondition("UGE", 0xf);
   AddCondition("UL", 0x7);
   AddCondition("UGT", 0xb);
   AddCondition("ULE", 0x3);

   LoadOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * LoadOrderCnt);
   InstrZ = 0;
   AddLoad("LDPP", 0x00);
   AddLoad("LDDP", 0x10);
   AddLoad("LDPD", 0x01);
   AddLoad("LDDD", 0x11);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(ALUOrders);
   free(RegOrders);
   free(Reg16Orders);
   free(Bit2Orders);
   free(Bit1Orders);
   free(Conditions);
   free(LoadOrders);
}

/*--------------------------------------------------------------------------*/

static bool DecodeReg(char *Asc_O, Byte * Erg, Byte * Size) {
   bool Res;
   char *Asc;

   Asc = Asc_O;

   if (strlen(Asc) < 2) return false;
   if (*Asc != 'r') return false;
   Asc++;
   if (*Asc == 'r') {
      if (strlen(Asc) < 2) return false;
      *Size = 1;
      Asc++;
   } else *Size = 0;

   *Erg = ConstLongInt(Asc, &Res);
   if ((!Res) || (*Erg > 15)) return false;
   if ((*Size == 1) && (Odd(*Erg))) return false;

   return true;
}

static void ChkAdr(LongWord Mask) {
   if ((AdrMode != ModNone) && (((1l << AdrMode) & Mask) == 0)) {
      WrError(1350);
      AdrMode = ModNone;
      AdrCnt = 0;
   }
}

static void DecodeAdr(char *Asc_O, LongWord Mask) {
   Word AdrWord;
   Integer level;
   Byte flg, Size;
   bool OK;
   String Reg, Asc;
   char *p;

   AdrMode = ModNone;
   AdrCnt = 0;
   strmaxcpy(Asc, Asc_O, 255);

/* immediate */

   if (*Asc == '#') {
      switch (OpSize) {
         case 0:
            AdrVals[0] = EvalIntExpression(Asc + 1, Int8, &OK);
            break;
         case 1:
            AdrWord = EvalIntExpression(Asc + 1, Int16, &OK);
            AdrVals[0] = Hi(AdrWord);
            AdrVals[1] = Lo(AdrWord);
            break;
      }
      if (OK) {
         AdrMode = ModImm;
         AdrCnt = OpSize + 1;
      }
      ChkAdr(Mask);
      return;
   }

/* Arbeitsregister */

   if (DecodeReg(Asc, &AdrPart, &Size)) {
      if (Size == 0)
         if ((Mask & MModWReg) != 0) AdrMode = ModWReg;
         else {
            AdrVals[0] = WorkOfs + AdrPart;
            AdrCnt = 1;
            AdrMode = ModReg;
      } else if ((Mask & MModWRReg) != 0) AdrMode = ModWRReg;
      else {
         AdrVals[0] = WorkOfs + AdrPart;
         AdrCnt = 1;
         AdrMode = ModRReg;
      }
      ChkAdr(Mask);
      return;
   }

/* Postinkrement */

   if (Asc[strlen(Asc) - 1] == '+') {
      if ((*Asc != '(') || (Asc[strlen(Asc) - 2] != ')')) WrError(1350);
      else {
         strmove(Asc, 1);
         Asc[strlen(Asc) - 2] = '\0';
         if (!DecodeReg(Asc, &AdrPart, &Size)) WrXError(1445, Asc);
         AdrMode = (Size == 0) ? ModIncWReg : ModIncWRReg;
      }
      ChkAdr(Mask);
      return;
   }

/* Predekrement */

   if ((*Asc == '-') && (Asc[1] == '(') && (Asc[strlen(Asc) - 1] == ')')) {
      strcopy(Reg, Asc + 2);
      Reg[strlen(Reg) - 1] = '\0';
      if (DecodeReg(Reg, &AdrPart, &Size)) {
         if (Size == 0) WrError(1350);
         else AdrMode = ModDecWRReg;
         ChkAdr(Mask);
         return;
      }
   }

/* indirekt<->direkt */

   if ((Asc[strlen(Asc) - 1] != ')') || (strlen(Asc) < 3)) {
      OK = false;
      p = Asc;
   } else {
      level = 0;
      p = Asc + strlen(Asc) - 2;
      flg = 0;
      while ((p >= Asc) && (level >= 0)) {
         switch (*p) {
            case '(':
               if (flg == 0) level--;
               break;
            case ')':
               if (flg == 0) level++;
               break;
            case '\'':
               if (((flg & 2) == 0)) flg ^= 1;
               break;
            case '"':
               if (((flg & 1) == 0)) flg ^= 2;
               break;
         }
         p--;
      }
      OK = (level == -1) && ((p < Asc) || ((*p == '.') || (*p == '_') || (isdigit(*p)) || (isalpha(*p))));
   }

/* indirekt */

   if (OK) {
      strcopy(Reg, p + 2);
      Reg[strlen(Reg) - 1] = '\0';
      p[1] = '\0';
      if (DecodeReg(Reg, &AdrPart, &Size))
         if (Size == 0) { /* d(r) */
            AdrVals[0] = EvalIntExpression(Asc, Int8, &OK);
            if (OK) {
               if (((Mask & MModIWReg) != 0) && (AdrVals[0] == 0)) AdrMode = ModIWReg;
               else if (((Mask & MModIReg) != 0) && (AdrVals[0] == 0)) {
                  AdrVals[0] = WorkOfs + AdrPart;
                  AdrMode = ModIReg;
                  AdrCnt = 1;
               } else {
                  AdrMode = ModDisp8WReg;
                  AdrCnt = 1;
               }
            }
         } else { /* ...(rr) */
         if (DecodeReg(Asc, AdrVals, &Size)) { /* rr(rr) */
            if (Size != 1) WrError(1350);
            else {
               AdrMode = ModDispRWRReg;
               AdrCnt = 1;
            }
         } else { /* d(rr) */
            AdrWord = EvalIntExpression(Asc, Int16, &OK);
            if ((AdrWord == 0) && ((Mask & (MModIRReg + MModIWRReg)) != 0)) {
               if (((Mask & MModIWRReg) != 0)) AdrMode = ModIWRReg;
               else {
                  AdrMode = ModIRReg;
                  AdrVals[0] = AdrPart + WorkOfs;
                  AdrCnt = 1;
               }
            } else if ((AdrWord < 0x100) && ((Mask & (MModDisp8WRReg + MModDisp8RReg)) != 0)) {
               if (((Mask & MModDisp8WRReg) != 0)) {
                  AdrVals[0] = Lo(AdrWord);
                  AdrCnt = 1;
                  AdrMode = ModDisp8WRReg;
               } else {
                  AdrVals[0] = AdrPart + WorkOfs;
                  AdrVals[1] = Lo(AdrWord);
                  AdrCnt = 2;
                  AdrMode = ModDisp8RReg;
               }
            } else if (((Mask & MModDisp16WRReg) != 0)) {
               AdrVals[0] = Hi(AdrWord);
               AdrVals[1] = Lo(AdrWord);
               AdrCnt = 2;
               AdrMode = ModDisp16WRReg;
            } else {
               AdrVals[0] = AdrPart + WorkOfs;
               AdrVals[2] = Hi(AdrWord);
               AdrVals[1] = Lo(AdrWord);
               AdrCnt = 3;
               AdrMode = ModDisp16RReg;
            }
         }
         }

      else { /* ...(RR) */
         AdrWord = EvalIntExpression(Reg, UInt9, &OK);
         if (((TypeFlag & (1 << SegReg)) == 0)) WrError(1350);
         else if (AdrWord < 0xff) {
            AdrVals[0] = Lo(AdrWord);
            AdrWord = EvalIntExpression(Asc, Int8, &OK);
            if (AdrWord != 0) WrError(1320);
            else {
               AdrCnt = 1;
               AdrMode = ModIReg;
            }
         } else if ((AdrWord > 0x1ff) || (Odd(AdrWord))) WrError(1350);
         else {
            AdrVals[0] = Lo(AdrWord);
            AdrWord = EvalIntExpression(Asc, Int16, &OK);
            if ((AdrWord == 0) && ((Mask & MModIRReg) != 0)) {
               AdrCnt = 1;
               AdrMode = ModIRReg;
            } else if ((AdrWord < 0x100) && ((Mask & MModDisp8RReg) != 0)) {
               AdrVals[1] = Lo(AdrWord);
               AdrCnt = 2;
               AdrMode = ModDisp8RReg;
            } else {
               AdrVals[2] = Hi(AdrWord);
               AdrVals[1] = Lo(AdrWord);
               AdrCnt = 3;
               AdrMode = ModDisp16RReg;
            }
         }
      }
      ChkAdr(Mask);
      return;
   }

/* direkt */

   AdrWord = EvalIntExpression(Asc, UInt16, &OK);
   if (!OK) ;
   else if (((TypeFlag & (1 << SegReg))) == 0) {
      AdrMode = ModAbs;
      AdrVals[0] = Hi(AdrWord);
      AdrVals[1] = Lo(AdrWord);
      AdrCnt = 2;
      ChkSpace(AbsSeg);
   } else if (AdrWord < 0xff) {
      AdrMode = ModReg;
      AdrVals[0] = Lo(AdrWord);
      AdrCnt = 1;
   } else if ((AdrWord > 0x1ff) || (Odd(AdrWord))) WrError(1350);
   else {
      AdrMode = ModRReg;
      AdrVals[0] = Lo(AdrWord);
      AdrCnt = 1;
   }

   ChkAdr(Mask);
}

static bool SplitBit(char *Asc, Byte * Erg) {
   char *p;
   Integer val;
   bool OK, Inv;

   p = RQuotPos(Asc, '.');
   if ((p == NULL) || (p == Asc + strlen(Asc) + 1)) {
      if (*Asc == '!') {
         Inv = true;
         strmove(Asc, 1);
      } else Inv = false;
      val = EvalIntExpression(Asc, UInt8, &OK);
      if (OK) {
         *Erg = val & 15;
         if (Inv) *Erg ^= 1;
         sprintf(Asc, "r%d", val >> 4);
         return true;
      }
      return false;
   }

   if (p[1] == '!')
      *Erg = 1 + (EvalIntExpression(p + 2, UInt3, &OK) << 1);
   else
      *Erg = EvalIntExpression(p + 1, UInt3, &OK) << 1;
   *p = '\0';
   return OK;
}

/*--------------------------------------------------------------------------*/

static bool DecodePseudo(void) {
#define ASSUMEST9Count 1
   static ASSUMERec ASSUMEST9s[ASSUMEST9Count] = { { "DP", &DPAssume, 0, 1, 0x0 } };
   Byte Bit;

   if (Memo("REG")) {
      CodeEquate(SegReg, 0, 0x1ff);
      return true;
   }

   if (Memo("BIT")) {
      if (ArgCnt != 1) WrError(1110);
      else if (SplitBit(ArgStr[1], &Bit)) {
         DecodeAdr(ArgStr[1], MModWReg);
         if (AdrMode == ModWReg) {
            PushLocHandle(-1);
            EnterIntSymbol(LabPart, (AdrPart << 4) + Bit, SegNone, false);
            PopLocHandle();
            sprintf(ListLine, "=r%d.%s%c", AdrPart, (Odd(Bit)) ? "NOT" : "", (Bit >> 1) + AscOfs);
         }
      }
      return true;
   }

   if (Memo("ASSUME")) {
      CodeASSUME(ASSUMEST9s, ASSUMEST9Count);
      return true;
   }

   return false;
}

static void MakeCode_ST9(void) {
   Integer z, AdrInt;
   bool OK;
   Byte HReg, HPart;
   Word Mask1, Mask2, AdrWord;

   CodeLen = 0;
   DontPrint = false;
   OpSize = 0;
   AbsSeg = (DPAssume == 1) ? SegData : SegCode;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(true)) return;

/* ohne Argument */

   for (z = 0; z < FixedOrderCnt; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else {
            OK = Hi(FixedOrders[z].Code) != 0;
            if (OK) BAsmCode[0] = Hi(FixedOrders[z].Code);
            CodeLen = OK;
            BAsmCode[CodeLen++] = Lo(FixedOrders[z].Code);
         }
         return;
      }

/* Datentransfer */

   if ((Memo("LD")) || (Memo("LDW"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         if (OpPart[strlen(OpPart) - 1] == 'W') {
            OpSize = 1;
            Mask1 = MModWRReg;
            Mask2 = MModRReg;
         } else {
            Mask1 = MModWReg;
            Mask2 = MModReg;
         }
         DecodeAdr(ArgStr[1], Mask1 + Mask2 + MModIWReg + MModDisp8WReg + MModIncWReg + MModIWRReg + MModIncWRReg + MModDecWRReg + MModDisp8WRReg + MModDisp16WRReg + MModDispRWRReg + MModAbs + MModIRReg);
         switch (AdrMode) {
            case ModWReg:
            case ModWRReg:
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], Mask1 + Mask2 + MModIWReg + MModDisp8WReg + MModIWRReg + MModIncWRReg + MModDecWRReg + MModDispRWRReg + MModDisp8WRReg + MModDisp16WRReg + MModAbs + MModImm);
               switch (AdrMode) {
                  case ModWReg:
                     BAsmCode[0] = (HReg << 4) + 8;
                     BAsmCode[1] = WorkOfs + AdrPart;
                     CodeLen = 2;
                     break;
                  case ModWRReg:
                     BAsmCode[0] = 0xe3;
                     BAsmCode[1] = (HReg << 4) + AdrPart;
                     CodeLen = 2;
                     break;
                  case ModReg:
                     BAsmCode[0] = (HReg << 4) + 8;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
                  case ModRReg:
                     BAsmCode[0] = 0xef;
                     BAsmCode[1] = AdrVals[0];
                     BAsmCode[2] = HReg + WorkOfs;
                     CodeLen = 3;
                     break;
                  case ModIWReg:
                     if (OpSize == 0) {
                        BAsmCode[0] = 0xe4;
                        BAsmCode[1] = (HReg << 4) + AdrPart;
                        CodeLen = 2;
                     } else {
                        BAsmCode[0] = 0xa6;
                        BAsmCode[1] = 0xf0 + AdrPart;
                        BAsmCode[2] = WorkOfs + HReg;
                        CodeLen = 3;
                     }
                     break;
                  case ModDisp8WReg:
                     BAsmCode[0] = 0xb3 + (OpSize * 0x2b);
                     BAsmCode[1] = (HReg << 4) + AdrPart;
                     BAsmCode[2] = AdrVals[0];
                     CodeLen = 3;
                     break;
                  case ModIWRReg:
                     BAsmCode[0] = 0xb5 + (OpSize * 0x2e);
                     BAsmCode[1] = (HReg << 4) + AdrPart + OpSize;
                     CodeLen = 2;
                     break;
                  case ModIncWRReg:
                     BAsmCode[0] = 0xb4 + (OpSize * 0x21);
                     BAsmCode[1] = 0xf1 + AdrPart;
                     BAsmCode[2] = WorkOfs + HReg;
                     CodeLen = 3;
                     break;
                  case ModDecWRReg:
                     BAsmCode[0] = 0xc2 + OpSize;
                     BAsmCode[1] = 0xf1 + AdrPart;
                     BAsmCode[2] = WorkOfs + HReg;
                     CodeLen = 3;
                     break;
                  case ModDispRWRReg:
                     BAsmCode[0] = 0x60;
                     BAsmCode[1] = (0x10 * (1 - OpSize)) + (AdrVals[0] << 4) + AdrPart;
                     BAsmCode[2] = 0xf0 + HReg;
                     CodeLen = 3;
                     break;
                  case ModDisp8WRReg:
                     BAsmCode[0] = 0x7f + (OpSize * 7);
                     BAsmCode[1] = 0xf1 + AdrPart;
                     BAsmCode[2] = AdrVals[0];
                     BAsmCode[3] = HReg + WorkOfs;
                     CodeLen = 4;
                     break;
                  case ModDisp16WRReg:
                     BAsmCode[0] = 0x7f + (OpSize * 7);
                     BAsmCode[1] = 0xf0 + AdrPart;
                     memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                     BAsmCode[2 + AdrCnt] = HReg + WorkOfs;
                     CodeLen = 3 + AdrCnt;
                     break;
                  case ModAbs:
                     BAsmCode[0] = 0xc4 + (OpSize * 0x1e);
                     BAsmCode[1] = 0xf0 + AdrPart;
                     memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                     CodeLen = 2 + AdrCnt;
                     break;
                  case ModImm:
                     if (OpSize == 0) {
                        BAsmCode[0] = (HReg << 4) + 0x0c;
                        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                        CodeLen = 1 + AdrCnt;
                     } else {
                        BAsmCode[0] = 0xbf;
                        BAsmCode[1] = WorkOfs + HReg;
                        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                        CodeLen = 2 + AdrCnt;
                     }
                     break;
               }
               break;
            case ModReg:
            case ModRReg:
               HReg = AdrVals[0];
               DecodeAdr(ArgStr[2], Mask1 + Mask2 + MModIWReg + MModIWRReg + MModIncWRReg + MModDecWRReg + MModDispRWRReg + MModDisp8WRReg + MModDisp16WRReg + MModImm);
               switch (AdrMode) {
                  case ModWReg:
                     BAsmCode[0] = (AdrPart << 4) + 0x09;
                     BAsmCode[1] = HReg;
                     CodeLen = 2;
                     break;
                  case ModWRReg:
                     BAsmCode[0] = 0xef;
                     BAsmCode[1] = WorkOfs + AdrPart;
                     BAsmCode[2] = HReg;
                     CodeLen = 3;
                     break;
                  case ModReg:
                  case ModRReg:
                     BAsmCode[0] = 0xf4 - (OpSize * 5);
                     BAsmCode[1] = AdrVals[0];
                     BAsmCode[2] = HReg;
                     CodeLen = 3;
                     break;
                  case ModIWReg:
                     BAsmCode[0] = 0xe7 - (0x41 * OpSize);
                     BAsmCode[1] = 0xf0 + AdrPart;
                     BAsmCode[2] = HReg;
                     CodeLen = 3;
                     break;
                  case ModIWRReg:
                     BAsmCode[0] = 0x72 + (OpSize * 12);
                     BAsmCode[1] = 0xf1 + AdrPart - OpSize;
                     BAsmCode[2] = HReg;
                     CodeLen = 3;
                     break;
                  case ModIncWRReg:
                     BAsmCode[0] = 0xb4 + (0x21 * OpSize);
                     BAsmCode[1] = 0xf1 + AdrPart;
                     BAsmCode[2] = HReg;
                     CodeLen = 3;
                     break;
                  case ModDecWRReg:
                     BAsmCode[0] = 0xc2 + OpSize;
                     BAsmCode[1] = 0xf1 + AdrPart;
                     BAsmCode[2] = HReg;
                     CodeLen = 3;
                     break;
                  case ModDisp8WRReg:
                     BAsmCode[0] = 0x7f + (OpSize * 7);
                     BAsmCode[1] = 0xf1 + AdrPart;
                     BAsmCode[2] = AdrVals[0];
                     BAsmCode[3] = HReg;
                     CodeLen = 4;
                     break;
                  case ModDisp16WRReg:
                     BAsmCode[0] = 0x7f + (OpSize * 7);
                     BAsmCode[1] = 0xf0 + AdrPart;
                     memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                     BAsmCode[2 + AdrCnt] = HReg;
                     CodeLen = 3 + AdrCnt;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0xf5 - (OpSize * 0x36);
                     BAsmCode[1] = HReg;
                     memcpy(BAsmCode + 2, AdrVals, BAsmCode[2]);
                     CodeLen = 2 + AdrCnt;
                     break;
               }
               break;
            case ModIWReg:
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], Mask2 + Mask1);
               switch (AdrMode) {
                  case ModWReg:
                     BAsmCode[0] = 0xe5;
                     BAsmCode[1] = (HReg << 4) + AdrPart;
                     CodeLen = 2;
                     break;
                  case ModWRReg:
                     BAsmCode[0] = 0x96;
                     BAsmCode[1] = WorkOfs + AdrPart;
                     BAsmCode[2] = 0xf0 + HReg;
                     CodeLen = 3;
                     break;
                  case ModReg:
                  case ModRReg:
                     BAsmCode[0] = 0xe6 - (0x50 * OpSize);
                     BAsmCode[1] = AdrVals[0];
                     BAsmCode[2] = 0xf0 + HReg;
                     CodeLen = 3;
                     break;
               }
               break;
            case ModDisp8WReg:
               BAsmCode[2] = AdrVals[0];
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], Mask1);
               switch (AdrMode) {
                  case ModWReg:
                  case ModWRReg:
                     BAsmCode[0] = 0xb2 + (OpSize * 0x2c);
                     BAsmCode[1] = (AdrPart << 4) + (OpSize << 4) + HReg;
                     CodeLen = 3;
                     break;
               }
               break;
            case ModIncWReg:
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], MModIncWRReg * (1 - OpSize));
               switch (AdrMode) {
                  case ModIncWRReg:
                     BAsmCode[0] = 0xd7;
                     BAsmCode[1] = (HReg << 4) + AdrPart + 1;
                     CodeLen = 2;
                     break;
               }
               break;
            case ModIWRReg:
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], (MModIWReg * (1 - OpSize)) + Mask1 + Mask2 + MModIWRReg + MModImm);
               switch (AdrMode) {
                  case ModIWReg:
                     BAsmCode[0] = 0xb5;
                     BAsmCode[1] = (AdrPart << 4) + HReg + 1;
                     CodeLen = 2;
                     break;
                  case ModWReg:
                     BAsmCode[0] = 0x72;
                     BAsmCode[1] = 0xf0 + HReg;
                     BAsmCode[2] = AdrPart + WorkOfs;
                     CodeLen = 3;
                     break;
                  case ModWRReg:
                     BAsmCode[0] = 0xe3;
                     BAsmCode[1] = (HReg << 4) + 0x10 + AdrPart;
                     CodeLen = 2;
                     break;
                  case ModReg:
                  case ModRReg:
                     BAsmCode[0] = 0x72 + (OpSize * 0x4c);
                     BAsmCode[1] = 0xf0 + HReg + OpSize;
                     BAsmCode[2] = AdrVals[0];
                     CodeLen = 3;
                     break;
                  case ModIWRReg:
                     if (OpSize == 0) {
                        BAsmCode[0] = 0x73;
                        BAsmCode[1] = 0xf0 + AdrPart;
                        BAsmCode[2] = WorkOfs + HReg;
                        CodeLen = 3;
                     } else {
                        BAsmCode[0] = 0xe3;
                        BAsmCode[1] = 0x11 + (HReg << 4) + AdrPart;
                        CodeLen = 2;
                     }
                     break;
                  case ModImm:
                     BAsmCode[0] = 0xf3 - (OpSize * 0x35);
                     BAsmCode[1] = 0xf0 + HReg;
                     memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                     CodeLen = 2 + AdrCnt;
                     break;
               }
               break;
            case ModIncWRReg:
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], Mask2 + (MModIncWReg * (1 - OpSize)));
               switch (AdrMode) {
                  case ModReg:
                  case ModRReg:
                     BAsmCode[0] = 0xb4 + (OpSize * 0x21);
                     BAsmCode[1] = 0xf0 + HReg;
                     BAsmCode[2] = AdrVals[0];
                     CodeLen = 3;
                     break;
                  case ModIncWReg:
                     BAsmCode[0] = 0xd7;
                     BAsmCode[1] = (AdrPart << 4) + HReg;
                     CodeLen = 2;
                     break;
               }
               break;
            case ModDecWRReg:
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], Mask2);
               switch (AdrMode) {
                  case ModReg:
                  case ModRReg:
                     BAsmCode[0] = 0xc2 + OpSize;
                     BAsmCode[1] = 0xf0 + HReg;
                     BAsmCode[2] = AdrVals[0];
                     CodeLen = 3;
                     break;
               }
               break;
            case ModDispRWRReg:
               HReg = AdrPart;
               HPart = AdrVals[0];
               DecodeAdr(ArgStr[2], Mask1);
               switch (AdrMode) {
                  case ModWReg:
                  case ModWRReg:
                     BAsmCode[0] = 0x60;
                     BAsmCode[1] = (0x10 * (1 - OpSize)) + 0x01 + (HPart << 4) + HReg;
                     BAsmCode[2] = 0xf0 + AdrPart;
                     CodeLen = 3;
                     break;
               }
               break;
            case ModDisp8WRReg:
               BAsmCode[2] = AdrVals[0];
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], Mask2 + (OpSize * MModImm));
               switch (AdrMode) {
                  case ModReg:
                  case ModRReg:
                     BAsmCode[0] = 0x26 + (OpSize * 0x60);
                     BAsmCode[1] = 0xf1 + HReg;
                     BAsmCode[3] = AdrVals[0];
                     CodeLen = 4;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0x06;
                     BAsmCode[1] = 0xf1 + HReg;
                     memcpy(BAsmCode + 3, AdrVals, AdrCnt);
                     CodeLen = 3 + AdrCnt;
                     break;
               }
               break;
            case ModDisp16WRReg:
               memcpy(BAsmCode + 2, AdrVals, 2);
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], Mask2 + (OpSize * MModImm));
               switch (AdrMode) {
                  case ModReg:
                  case ModRReg:
                     BAsmCode[0] = 0x26 + (OpSize * 0x60);
                     BAsmCode[1] = 0xf0 + HReg;
                     BAsmCode[4] = AdrVals[0];
                     CodeLen = 5;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0x06;
                     BAsmCode[1] = 0xf0 + HReg;
                     memcpy(BAsmCode + 4, AdrVals, AdrCnt);
                     CodeLen = 4 + AdrCnt;
                     break;
               }
               break;
            case ModAbs:
               memcpy(BAsmCode + 2, AdrVals, 2);
               DecodeAdr(ArgStr[2], Mask1 + MModImm);
               switch (AdrMode) {
                  case ModWReg:
                  case ModWRReg:
                     BAsmCode[0] = 0xc5 + (OpSize * 0x1d);
                     BAsmCode[1] = 0xf0 + AdrPart + OpSize;
                     CodeLen = 4;
                     break;
                  case ModImm:
                     memmove(BAsmCode + 2 + AdrCnt, BAsmCode + 2, 2);
                     BAsmCode[0] = 0x2f + (OpSize * 7);
                     BAsmCode[1] = 0xf1;
                     memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                     CodeLen = 4 + AdrCnt;
                     break;
               }
               break;
            case ModIRReg:
               HReg = AdrVals[0];
               DecodeAdr(ArgStr[2], MModIWRReg * (1 - OpSize));
               switch (AdrMode) {
                  case ModIWRReg:
                     BAsmCode[0] = 0x73;
                     BAsmCode[1] = 0xf0 + AdrPart;
                     BAsmCode[2] = HReg;
                     CodeLen = 3;
                     break;
               }
               break;
         }
      }
      return;
   }

   for (z = 0; z < LoadOrderCnt; z++)
      if (Memo(LoadOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], MModIncWRReg);
            if (AdrMode == ModIncWRReg) {
               HReg = AdrPart << 4;
               DecodeAdr(ArgStr[2], MModIncWRReg);
               if (AdrMode == ModIncWRReg) {
                  BAsmCode[0] = 0xd6;
                  BAsmCode[1] = LoadOrders[z].Code + HReg + AdrPart;
                  CodeLen = 2;
               }
            }
         }
         return;
      }

   if ((Memo("PEA")) || (Memo("PEAU"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModDisp8RReg + MModDisp16RReg);
         if (AdrMode != ModNone) {
            BAsmCode[0] = 0x8f;
            BAsmCode[1] = 0x01 + (2 * Memo("PEAU"));
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
            BAsmCode[2] += AdrCnt - 2;
            CodeLen = 2 + AdrCnt;
         }
      }
      return;
   }

   if ((Memo("PUSH")) || (Memo("PUSHU"))) {
      z = Memo("PUSHU");
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg + MModIReg + MModImm);
         switch (AdrMode) {
            case ModReg:
               BAsmCode[0] = 0x66 - (z * 0x36);
               BAsmCode[1] = AdrVals[0];
               CodeLen = 2;
               break;
            case ModIReg:
               BAsmCode[0] = 0xf7 - (z * 0xc6);
               BAsmCode[1] = AdrVals[0];
               CodeLen = 2;
               break;
            case ModImm:
               BAsmCode[0] = 0x8f;
               BAsmCode[1] = 0xf1 + (z * 2);
               BAsmCode[2] = AdrVals[0];
               CodeLen = 3;
               break;
         }
      }
      return;
   }

   if ((Memo("PUSHW")) || (Memo("PUSHUW"))) {
      z = Memo("PUSHUW");
      OpSize = 1;
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModRReg + MModImm);
         switch (AdrMode) {
            case ModRReg:
               BAsmCode[0] = 0x74 + (z * 0x42);
               BAsmCode[1] = AdrVals[0];
               CodeLen = 2;
               break;
            case ModImm:
               BAsmCode[0] = 0x8f;
               BAsmCode[1] = 0xc1 + (z * 2);
               memcpy(BAsmCode + 2, AdrVals, AdrCnt);
               CodeLen = 2 + AdrCnt;
               break;
         }
      }
      return;
   }

   if (Memo("XCH")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg);
         if (AdrMode == ModReg) {
            BAsmCode[2] = AdrVals[0];
            DecodeAdr(ArgStr[2], MModReg);
            if (AdrMode == ModReg) {
               BAsmCode[1] = AdrVals[0];
               BAsmCode[0] = 0x16;
               CodeLen = 3;
            }
         }
      }
      return;
   }

/* Arithmetik */

   for (z = 0; z < ALUOrderCnt; z++)
      if ((strncmp(OpPart, ALUOrders[z].Name, ALUOrders[z].len) == 0) && ((OpPart[ALUOrders[z].len] == '\0') || (OpPart[ALUOrders[z].len] == 'W'))) {
         if (ArgCnt != 2) WrError(1110);
         else {
            if (OpPart[strlen(OpPart) - 1] == 'W') {
               OpSize = 1;
               Mask1 = MModWRReg;
               Mask2 = MModRReg;
            } else {
               Mask1 = MModWReg;
               Mask2 = MModReg;
            }
            DecodeAdr(ArgStr[1], Mask1 + Mask2 + MModIWReg + MModIWRReg + MModIncWRReg + MModDecWRReg + MModDispRWRReg + MModDisp8WRReg + MModDisp16WRReg + MModAbs + MModIRReg);
            switch (AdrMode) {
               case ModWReg:
               case ModWRReg:
                  HReg = AdrPart;
                  DecodeAdr(ArgStr[2], Mask1 + MModIWReg + Mask2 + MModIWRReg + MModIncWRReg + MModDecWRReg + MModDispRWRReg + MModDisp8WRReg + MModDisp16WRReg + MModAbs + MModImm);
                  switch (AdrMode) {
                     case ModWReg:
                     case ModWRReg:
                        BAsmCode[0] = (ALUOrders[z].Code << 4) + 2 + (OpSize * 12);
                        BAsmCode[1] = (HReg << 4) + AdrPart;
                        CodeLen = 2;
                        break;
                     case ModIWReg:
                        if (OpSize == 0) {
                           BAsmCode[0] = (ALUOrders[z].Code << 4) + 3;
                           BAsmCode[1] = (HReg << 4) + AdrPart;
                           CodeLen = 2;
                        } else {
                           BAsmCode[0] = 0xa6;
                           BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart;
                           BAsmCode[2] = WorkOfs + HReg;
                           CodeLen = 3;
                        }
                        break;
                     case ModReg:
                     case ModRReg:
                        BAsmCode[0] = (ALUOrders[z].Code << 4) + 4 + (OpSize * 3);
                        BAsmCode[1] = AdrVals[0];
                        BAsmCode[2] = HReg + WorkOfs;
                        CodeLen = 3;
                        break;
                     case ModIWRReg:
                        if (OpSize == 0) {
                           BAsmCode[0] = 0x72;
                           BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart + 1;
                           BAsmCode[2] = HReg + WorkOfs;
                           CodeLen = 3;
                        } else {
                           BAsmCode[0] = (ALUOrders[z].Code << 4) + 0x0e;
                           BAsmCode[1] = (HReg << 4) + AdrPart + 1;
                           CodeLen = 2;
                        }
                        break;
                     case ModIncWRReg:
                        BAsmCode[0] = 0xb4 + (OpSize * 0x21);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart + 1;
                        BAsmCode[2] = HReg + WorkOfs;
                        CodeLen = 3;
                        break;
                     case ModDecWRReg:
                        BAsmCode[0] = 0xc2 + OpSize;
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart + 1;
                        BAsmCode[2] = HReg + WorkOfs;
                        CodeLen = 3;
                        break;
                     case ModDispRWRReg:
                        BAsmCode[0] = 0x60;
                        BAsmCode[1] = 0x10 * (1 - OpSize) + (AdrVals[0] << 4) + AdrPart;
                        BAsmCode[2] = (ALUOrders[z].Code << 4) + HReg;
                        CodeLen = 3;
                        break;
                     case ModDisp8WRReg:
                        BAsmCode[0] = 0x7f + (OpSize * 7);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart + 1;
                        BAsmCode[2] = AdrVals[0];
                        BAsmCode[3] = WorkOfs + HReg;
                        CodeLen = 4;
                        break;
                     case ModDisp16WRReg:
                        BAsmCode[0] = 0x7f + (OpSize * 7);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart;
                        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                        BAsmCode[2 + AdrCnt] = WorkOfs + HReg;
                        CodeLen = 3 + AdrCnt;
                        break;
                     case ModAbs:
                        BAsmCode[0] = 0xc4 + (OpSize * 30);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + HReg;
                        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                        CodeLen = 2 + AdrCnt;
                        break;
                     case ModImm:
                        BAsmCode[0] = (ALUOrders[z].Code << 4) + 5 + (OpSize * 2);
                        BAsmCode[1] = WorkOfs + HReg + OpSize;
                        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                        CodeLen = 2 + AdrCnt;
                        break;
                  }
                  break;
               case ModReg:
               case ModRReg:
                  HReg = AdrVals[0];
                  DecodeAdr(ArgStr[2], Mask2 + MModIWReg + MModIWRReg + MModIncWRReg + MModDecWRReg + MModDisp8WRReg + MModDisp16WRReg + MModImm);
                  switch (AdrMode) {
                     case ModReg:
                     case ModRReg:
                        BAsmCode[0] = (ALUOrders[z].Code << 4) + 4 + (OpSize * 3);
                        CodeLen = 3;
                        BAsmCode[1] = AdrVals[0];
                        BAsmCode[2] = HReg;
                        break;
                     case ModIWReg:
                        BAsmCode[0] = 0xa6 + 65 * (1 - OpSize);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart;
                        BAsmCode[2] = HReg;
                        CodeLen = 3;
                        break;
                     case ModIWRReg:
                        BAsmCode[0] = 0x72 + (OpSize * 12);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart + (1 - OpSize);
                        BAsmCode[2] = HReg;
                        CodeLen = 3;
                        break;
                     case ModIncWRReg:
                        BAsmCode[0] = 0xb4 + (OpSize * 0x21);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart + 1;
                        BAsmCode[2] = HReg;
                        CodeLen = 3;
                        break;
                     case ModDecWRReg:
                        BAsmCode[0] = 0xc2 + OpSize;
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart + 1;
                        BAsmCode[2] = HReg;
                        CodeLen = 3;
                        break;
                     case ModDisp8WRReg:
                        BAsmCode[0] = 0x7f + (OpSize * 7);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart + 1;
                        BAsmCode[2] = AdrVals[0];
                        BAsmCode[3] = HReg;
                        CodeLen = 4;
                        break;
                     case ModDisp16WRReg:
                        BAsmCode[0] = 0x7f + (OpSize * 7);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart;
                        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                        BAsmCode[2 + AdrCnt] = HReg;
                        CodeLen = 3 + AdrCnt;
                        break;
                     case ModImm:
                        BAsmCode[0] = (ALUOrders[z].Code << 4) + 5 + (OpSize * 2);
                        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                        BAsmCode[1] = HReg + OpSize;
                        CodeLen = 2 + AdrCnt;
                        break;
                  }
                  break;
               case ModIWReg:
                  HReg = AdrPart;
                  DecodeAdr(ArgStr[2], Mask2);
                  switch (AdrMode) {
                     case ModReg:
                     case ModRReg:
                        BAsmCode[0] = 0xe6 - (OpSize * 0x50);
                        BAsmCode[1] = AdrVals[0];
                        BAsmCode[2] = (ALUOrders[z].Code << 4) + HReg;
                        CodeLen = 3;
                        break;
                  }
                  break;
               case ModIWRReg:
                  HReg = AdrPart;
                  DecodeAdr(ArgStr[2], (OpSize * MModWRReg) + Mask2 + MModIWRReg + MModImm);
                  switch (AdrMode) {
                     case ModWRReg:
                        BAsmCode[0] = (ALUOrders[z].Code << 4) + 0x0e;
                        BAsmCode[1] = (HReg << 4) + 0x10 + AdrPart;
                        CodeLen = 2;
                        break;
                     case ModReg:
                     case ModRReg:
                        BAsmCode[0] = 0x72 + (OpSize * 76);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + HReg + OpSize;
                        BAsmCode[2] = AdrVals[0];
                        CodeLen = 3;
                        break;
                     case ModIWRReg:
                        if (OpSize == 0) {
                           BAsmCode[0] = 0x73;
                           BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart;
                           BAsmCode[2] = HReg + WorkOfs;
                           CodeLen = 3;
                        } else {
                           BAsmCode[0] = (ALUOrders[z].Code << 4) + 0x0e;
                           BAsmCode[1] = 0x11 + (HReg << 4) + AdrPart;
                           CodeLen = 2;
                        }
                        break;
                     case ModImm:
                        BAsmCode[0] = 0xf3 - (OpSize * 0x35);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + HReg;
                        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                        CodeLen = 2 + AdrCnt;
                        break;
                  }
                  break;
               case ModIncWRReg:
                  HReg = AdrPart;
                  DecodeAdr(ArgStr[2], Mask2);
                  switch (AdrMode) {
                     case ModReg:
                     case ModRReg:
                        BAsmCode[0] = 0xb4 + (OpSize * 0x21);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + HReg;
                        BAsmCode[2] = AdrVals[0];
                        CodeLen = 3;
                        break;
                  }
                  break;
               case ModDecWRReg:
                  HReg = AdrPart;
                  DecodeAdr(ArgStr[2], Mask2);
                  switch (AdrMode) {
                     case ModReg:
                     case ModRReg:
                        BAsmCode[0] = 0xc2 + OpSize;
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + HReg;
                        BAsmCode[2] = AdrVals[0];
                        CodeLen = 3;
                        break;
                  }
                  break;
               case ModDispRWRReg:
                  HReg = AdrPart;
                  HPart = AdrVals[0];
                  DecodeAdr(ArgStr[2], Mask1);
                  switch (AdrMode) {
                     case ModWReg:
                     case ModWRReg:
                        BAsmCode[0] = 0x60;
                        BAsmCode[1] = 0x11 - (OpSize * 0x10) + (HPart << 4) + HReg;
                        BAsmCode[2] = (ALUOrders[z].Code << 4) + AdrPart;
                        CodeLen = 3;
                        break;
                  }
                  break;
               case ModDisp8WRReg:
                  HReg = AdrPart;
                  BAsmCode[2] = AdrVals[0];
                  DecodeAdr(ArgStr[2], Mask2 + (OpSize * MModImm));
                  switch (AdrMode) {
                     case ModReg:
                     case ModRReg:
                        BAsmCode[0] = 0x26 + (OpSize * 0x60);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + HReg + 1;
                        BAsmCode[3] = AdrVals[0] + OpSize;
                        CodeLen = 4;
                        break;
                     case ModImm:
                        BAsmCode[0] = 0x06;
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + HReg + 1;
                        memcpy(BAsmCode + 3, AdrVals, AdrCnt);
                        CodeLen = 3 + AdrCnt;
                        break;
                  }
                  break;
               case ModDisp16WRReg:
                  HReg = AdrPart;
                  memcpy(BAsmCode + 2, AdrVals, 2);
                  DecodeAdr(ArgStr[2], Mask2 + (OpSize * MModImm));
                  switch (AdrMode) {
                     case ModReg:
                     case ModRReg:
                        BAsmCode[0] = 0x26 + (OpSize * 0x60);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + HReg;
                        BAsmCode[4] = AdrVals[0] + OpSize;
                        CodeLen = 5;
                        break;
                     case ModImm:
                        BAsmCode[0] = 0x06;
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + HReg;
                        memcpy(BAsmCode + 4, AdrVals, AdrCnt);
                        CodeLen = 4 + AdrCnt;
                        break;
                  }
                  break;
               case ModAbs:
                  memcpy(BAsmCode + 2, AdrVals, 2);
                  DecodeAdr(ArgStr[2], Mask1 + MModImm);
                  switch (AdrMode) {
                     case ModWReg:
                     case ModWRReg:
                        BAsmCode[0] = 0xc5 + (OpSize * 29);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart + OpSize;
                        CodeLen = 4;
                        break;
                     case ModImm:
                        memmove(BAsmCode + 2 + AdrCnt, BAsmCode + 2, 2);
                        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                        BAsmCode[0] = 0x2f + (OpSize * 7);
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + 1;
                        CodeLen = 4 + AdrCnt;
                        break;
                  }
                  break;
               case ModIRReg:
                  HReg = AdrVals[0];
                  DecodeAdr(ArgStr[2], MModIWRReg * (1 - OpSize));
                  switch (AdrMode) {
                     case ModIWRReg:
                        BAsmCode[0] = 0x73;
                        BAsmCode[1] = (ALUOrders[z].Code << 4) + AdrPart;
                        BAsmCode[2] = HReg;
                        CodeLen = 3;
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
            DecodeAdr(ArgStr[1], MModReg + MModIReg);
            switch (AdrMode) {
               case ModReg:
                  BAsmCode[0] = RegOrders[z].Code;
                  BAsmCode[1] = AdrVals[0];
                  CodeLen = 2;
                  break;
               case ModIReg:
                  BAsmCode[0] = RegOrders[z].Code + 1;
                  BAsmCode[1] = AdrVals[0];
                  CodeLen = 2;
                  break;
            }
         }
         return;
      }

   for (z = 0; z < Reg16OrderCnt; z++)
      if (Memo(Reg16Orders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], MModRReg);
            switch (AdrMode) {
               case ModRReg:
                  BAsmCode[0] = Reg16Orders[z].Code;
                  BAsmCode[1] = AdrVals[0] + Memo("EXT");
                  CodeLen = 2;
                  break;
            }
         }
         return;
      }

   if ((Memo("DIV")) || (Memo("MUL"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModWRReg);
         if (AdrMode == ModWRReg) {
            HReg = AdrPart;
            DecodeAdr(ArgStr[2], MModWReg);
            if (AdrMode == ModWReg) {
               BAsmCode[0] = 0x5f - (0x10 * Memo("MUL"));
               BAsmCode[1] = (HReg << 4) + AdrPart;
               CodeLen = 2;
            }
         }
      }
      return;
   }

   if (Memo("DIVWS")) {
      if (ArgCnt != 3) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModWRReg);
         if (AdrMode == ModWRReg) {
            HReg = AdrPart;
            DecodeAdr(ArgStr[2], MModWRReg);
            if (AdrMode == ModWRReg) {
               BAsmCode[2] = (HReg << 4) + AdrPart;
               DecodeAdr(ArgStr[3], MModRReg);
               if (AdrMode == ModRReg) {
                  BAsmCode[0] = 0x56;
                  BAsmCode[1] = AdrVals[0];
                  CodeLen = 3;
               }
            }
         }
      }
      return;
   }

/* Bitoperationen */

   for (z = 0; z < Bit2OrderCnt; z++)
      if (Memo(Bit2Orders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (!SplitBit(ArgStr[1], &HReg)) ;
         else if (Odd(HReg)) WrError(1350);
         else {
            DecodeAdr(ArgStr[1], MModWReg);
            if (AdrMode == ModWReg) {
               HReg = (HReg << 4) + AdrPart;
               if (SplitBit(ArgStr[2], &HPart)) {
                  DecodeAdr(ArgStr[2], MModWReg);
                  if (AdrMode == ModWReg) {
                     HPart = (HPart << 4) + AdrPart;
                     BAsmCode[0] = Bit2Orders[z].Code;
                     if (Memo("BLD")) {
                        BAsmCode[1] = HPart | 0x10;
                        BAsmCode[2] = HReg | (HPart & 0x10);
                     } else if (Memo("BXOR")) {
                        BAsmCode[1] = 0x10 + HReg;
                        BAsmCode[2] = HPart;
                     } else {
                        BAsmCode[1] = 0x10 + HReg;
                        BAsmCode[2] = HPart ^ 0x10;
                     }
                     CodeLen = 3;
                  }
               }
            }
         }
         return;
      }

   for (z = 0; z < Bit1OrderCnt; z++)
      if (Memo(Bit1Orders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (!SplitBit(ArgStr[1], &HReg)) ;
         else if (Odd(HReg)) WrError(1350);
         else {
            DecodeAdr(ArgStr[1], MModWReg + (Memo("BTSET") * MModIWRReg));
            switch (AdrMode) {
               case ModWReg:
                  BAsmCode[0] = Bit1Orders[z].Code;
                  BAsmCode[1] = (HReg << 4) + AdrPart;
                  CodeLen = 2;
                  break;
               case ModIWRReg:
                  BAsmCode[0] = 0xf6;
                  BAsmCode[1] = (HReg << 4) + AdrPart;
                  CodeLen = 2;
                  break;
            }
         }
         return;
      }

/* Spruenge */

   if ((Memo("BTJF")) || (Memo("BTJT"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (!SplitBit(ArgStr[1], &HReg)) ;
      else if (Odd(HReg)) WrError(1350);
      else {
         DecodeAdr(ArgStr[1], MModWReg);
         if (AdrMode == ModWReg) {
            BAsmCode[1] = (HReg << 4) + AdrPart + (Memo("BTJF") << 4);
            AdrInt = EvalIntExpression(ArgStr[2], UInt16, &OK) - (EProgCounter() + 3);
            if (!OK) ;
            else if ((!SymbolQuestionable) && ((AdrInt < -128) || (AdrInt > 127))) WrError(1370);
            else {
               BAsmCode[0] = 0xaf;
               BAsmCode[2] = AdrInt & 0xff;
               CodeLen = 3;
               ChkSpace(SegCode);
            }
         }
      }
      return;
   }

   if ((Memo("JP")) || (Memo("CALL"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AbsSeg = SegCode;
         DecodeAdr(ArgStr[1], MModIRReg + MModAbs);
         switch (AdrMode) {
            case ModIRReg:
               BAsmCode[0] = 0x74 + (Memo("JP") * 0x60);
               BAsmCode[1] = AdrVals[0] + Memo("CALL");
               CodeLen = 2;
               break;
            case ModAbs:
               BAsmCode[0] = 0x8d + (Memo("CALL") * 0x45);
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               CodeLen = 1 + AdrCnt;
               break;
         }
      }
      return;
   }

   if ((Memo("CPJFI")) || (Memo("CPJTI"))) {
      if (ArgCnt != 3) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModWReg);
         if (AdrMode == ModWReg) {
            HReg = AdrPart;
            DecodeAdr(ArgStr[2], MModIWRReg);
            if (AdrMode == ModIWRReg) {
               BAsmCode[1] = (AdrPart << 4) + (Memo("CPJTI") << 4) + HReg;
               AdrInt = EvalIntExpression(ArgStr[3], UInt16, &OK) - (EProgCounter() + 3);
               if (!OK) ;
               else if ((!SymbolQuestionable) && ((AdrInt < -128) || (AdrInt > 127))) WrError(1370);
               else {
                  ChkSpace(SegCode);
                  BAsmCode[0] = 0x9f;
                  BAsmCode[2] = AdrInt & 0xff;
                  CodeLen = 3;
               }
            }
         }
      }
      return;
   }

   if (Memo("DJNZ")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModWReg);
         if (AdrMode == ModWReg) {
            BAsmCode[0] = (AdrPart << 4) + 0x0a;
            AdrInt = EvalIntExpression(ArgStr[2], UInt16, &OK) - (EProgCounter() + 2);
            if (!OK) ;
            else if ((!SymbolQuestionable) && ((AdrInt < -128) || (AdrInt > 127))) WrError(1370);
            else {
               ChkSpace(SegCode);
               BAsmCode[1] = AdrInt & 0xff;
               CodeLen = 2;
            }
         }
      }
      return;
   }

   if (Memo("DWJNZ")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModRReg);
         if (AdrMode == ModRReg) {
            BAsmCode[1] = AdrVals[0];
            AdrInt = EvalIntExpression(ArgStr[2], UInt16, &OK) - (EProgCounter() + 3);
            if (!OK) ;
            else if ((!SymbolQuestionable) && ((AdrInt < -128) || (AdrInt > 127))) WrError(1370);
            else {
               ChkSpace(SegCode);
               BAsmCode[0] = 0xc6;
               BAsmCode[2] = AdrInt & 0xff;
               CodeLen = 3;
            }
         }
      }
      return;
   }

   for (z = 0; z < ConditionCnt; z++)
      if ((*OpPart == 'J') && (OpPart[1] == 'P') && (strcmp(OpPart + 2, Conditions[z].Name) == 0)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrWord = EvalIntExpression(ArgStr[1], UInt16, &OK);
            if (OK) {
               ChkSpace(SegCode);
               BAsmCode[0] = 0x0d + (Conditions[z].Code << 4);
               BAsmCode[1] = Hi(AdrWord);
               BAsmCode[2] = Lo(AdrWord);
               CodeLen = 3;
            }
         }
         return;
      } else if ((*OpPart == 'J') && (OpPart[1] == 'R') && (strcmp(OpPart + 2, Conditions[z].Name) == 0)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrInt = EvalIntExpression(ArgStr[1], UInt16, &OK) - (EProgCounter() + 2);
            if (!OK) ;
            else if ((!SymbolQuestionable) && ((AdrInt < -128) || (AdrInt > 127))) WrError(1370);
            else {
               ChkSpace(SegCode);
               BAsmCode[0] = 0x0b + (Conditions[z].Code << 4);
               BAsmCode[1] = AdrInt & 0xff;
               CodeLen = 2;
            }
         }
         return;
      }

/* Besonderheiten */

   if (Memo("SPP")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*ArgStr[1] != '#') WrError(1350);
      else {
         BAsmCode[1] = (EvalIntExpression(ArgStr[1] + 1, UInt6, &OK) << 2) + 0x02;
         if (OK) {
            BAsmCode[0] = 0xc7;
            CodeLen = 2;
         }
      }
      return;
   }

   if ((Memo("SRP")) || (Memo("SRP0")) || (Memo("SRP1"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (*ArgStr[1] != '#') WrError(1350);
      else {
         BAsmCode[1] = EvalIntExpression(ArgStr[1] + 1, UInt5, &OK) << 3;
         if (OK) {
            BAsmCode[0] = 0xc7;
            CodeLen = 2;
            if (strlen(OpPart) == 4) BAsmCode[1] += 4;
            if (OpPart[strlen(OpPart) - 1] == '1') BAsmCode[1]++;
         }
      }
      return;
   }

/* Fakes... */

   if (Memo("SLA")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModWReg + MModReg + MModIWRReg);
         switch (AdrMode) {
            case ModWReg:
               BAsmCode[0] = 0x42;
               BAsmCode[1] = (AdrPart << 4) + AdrPart;
               CodeLen = 2;
               break;
            case ModReg:
               BAsmCode[0] = 0x44;
               BAsmCode[1] = AdrVals[0];
               BAsmCode[2] = AdrVals[0];
               CodeLen = 3;
               break;
            case ModIWRReg:
               BAsmCode[0] = 0x73;
               BAsmCode[1] = 0x40 + AdrPart;
               BAsmCode[2] = WorkOfs + AdrPart;
               CodeLen = 3;
               break;
         }
      }
      return;
   }

   if (Memo("SLAW")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModWRReg + MModRReg + MModIWRReg);
         switch (AdrMode) {
            case ModWRReg:
               BAsmCode[0] = 0x4e;
               BAsmCode[1] = (AdrPart << 4) + AdrPart;
               CodeLen = 2;
               break;
            case ModRReg:
               BAsmCode[0] = 0x47;
               BAsmCode[1] = AdrVals[0];
               BAsmCode[2] = AdrVals[0];
               CodeLen = 3;
               break;
            case ModIWRReg:
               BAsmCode[0] = 0x4e;
               BAsmCode[1] = 0x11 + (AdrPart << 4) + AdrPart;
               CodeLen = 2;
               break;
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static void InitCode_ST9(void) {
   SaveInitProc();

   DPAssume = 0;
}

static bool ChkPC_ST9(void) {
   switch (ActPC) {
      case SegCode:
      case SegData:
         return (ProgCounter() < 0x10000);
      case SegReg:
         return (ProgCounter() < 0x100);
      default:
         return false;
   }
}

static bool IsDef_ST9(void) {
   return (Memo("REG") || Memo("BIT"));
}

static void SwitchFrom_ST9(void) {
   DeinitFields();
}

static void InternSymbol_ST9(char *Asc, TempResult * Erg) {
   String h;
   bool Err;
   bool Pair;

   Erg->Typ = TempNone;
   if ((strlen(Asc) < 2) || (*Asc != 'R')) return;

   strmaxcpy(h, Asc + 1, 255);
   if (*h == 'R') {
      if (strlen(h) < 2) return;
      Pair = true;
      strmove(h, 1);
   } else Pair = false;
   Erg->Contents.Int = ConstLongInt(h, &Err);
   if ((!Err) || (Erg->Contents.Int < 0) || (Erg->Contents.Int > 255)) return;
   if ((Erg->Contents.Int & 0xf0) == 0xd0) return;
   if ((Pair) && (Odd(Erg->Contents.Int))) return;

   if (Pair) Erg->Contents.Int += 0x100;
   Erg->Typ = TempInt;
   TypeFlag |= (1 << SegReg);
}

static void SwitchTo_ST9(void) {
   TurnWords = false;
   ConstMode = ConstModeIntel;
   SetIsOccupied = false;

   PCSymbol = "PC";
   HeaderID = 0x32;
   NOPCode = 0xff;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = (1 << SegCode) | (1 << SegData) | (1 << SegReg);
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;
   Grans[SegData] = 1;
   ListGrans[SegData] = 1;
   SegInits[SegData] = 0;
   Grans[SegReg] = 1;
   ListGrans[SegReg] = 1;
   SegInits[SegReg] = 0;

   MakeCode = MakeCode_ST9;
   ChkPC = ChkPC_ST9;
   IsDef = IsDef_ST9;
   SwitchFrom = SwitchFrom_ST9;
   InternSymbol = InternSymbol_ST9;

   InitFields();
}

void codest9_init(void) {
   CPUST9020 = AddCPU("ST9020", SwitchTo_ST9);
   CPUST9030 = AddCPU("ST9030", SwitchTo_ST9);
   CPUST9040 = AddCPU("ST9040", SwitchTo_ST9);
   CPUST9050 = AddCPU("ST9050", SwitchTo_ST9);
   SaveInitProc = InitPassProc;
   InitPassProc = InitCode_ST9;
}
