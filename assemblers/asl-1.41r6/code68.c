// AS-Portierung
// Codegenerator fuer 68xx Prozessoren
#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "codepseudo.h"
#include "codevars.h"

/*---------------------------------------------------------------------------*/

typedef struct {
   char *Name;
   CPUVar MinCPU, MaxCPU;
   Word Code;
} FixedOrder;

typedef struct {
   char *Name;
   Byte Code;
} BaseOrder;

typedef struct {
   char *Name;
   bool MayImm;
   Byte Code;
} ALU8Order;

typedef struct {
   char *Name;
   bool MayImm;
   CPUVar MinCPU; /* Shift  andere   ,Y   */
   Byte PageShift; /* 0 :     nix    Pg 2  */
   Byte Code; /* 1 :     Pg 3   Pg 4  */
} ALU16Order; /* 2 :     nix    Pg 4  */
                            /* 3 :     Pg 2   Pg 3  */

#define ModNone (-1)
#define ModAcc  0
#define MModAcc (1<<ModAcc)
#define ModDir  1
#define MModDir (1<<ModDir)
#define ModExt  2
#define MModExt (1<<ModExt)
#define ModInd  3
#define MModInd (1<<ModInd)
#define ModImm  4
#define MModImm (1<<ModImm)

#define Page2Prefix 0x18
#define Page3Prefix 0x1a
#define Page4Prefix 0xcd

#define FixedOrderCnt 45
#define RelOrderCnt 19
#define ALU8OrderCnt 11
#define ALU16OrderCnt 13
#define Sing8OrderCnt 12
#define Bit63OrderCnt 4

static ShortInt OpSize;
static Byte PrefCnt; /* Anzahl Befehlspraefixe */
static ShortInt AdrMode; /* Ergebnisadressmodus */
static Byte AdrPart; /* Adressierungsmodusbits im Opcode */
static Byte AdrVals[4]; /* Adressargument */

static FixedOrder *FixedOrders;
static BaseOrder *RelOrders;
static ALU8Order *ALU8Orders;
static ALU16Order *ALU16Orders;
static BaseOrder *Bit63Orders;
static BaseOrder *Sing8Orders;

static CPUVar CPU6800, CPU6301, CPU6811;

/*---------------------------------------------------------------------------*/

static void AddFixed(char *NName, CPUVar NMin, CPUVar NMax, Word NCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);

   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ].MinCPU = NMin;
   FixedOrders[InstrZ].MaxCPU = NMax;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddRel(char *NName, Byte NCode) {
   if (InstrZ >= RelOrderCnt) exit(255);

   RelOrders[InstrZ].Name = NName;
   RelOrders[InstrZ++].Code = NCode;
}

static void AddALU8(char *NName, bool NMay, Byte NCode) {
   if (InstrZ >= ALU8OrderCnt) exit(255);

   ALU8Orders[InstrZ].Name = NName;
   ALU8Orders[InstrZ].MayImm = NMay;
   ALU8Orders[InstrZ++].Code = NCode;
}

static void AddALU16(char *NName, bool NMay, CPUVar NMin, Byte NShift, Byte NCode) {
   if (InstrZ >= ALU16OrderCnt) exit(255);

   ALU16Orders[InstrZ].Name = NName;
   ALU16Orders[InstrZ].MayImm = NMay;
   ALU16Orders[InstrZ].MinCPU = NMin;
   ALU16Orders[InstrZ].PageShift = NShift;
   ALU16Orders[InstrZ++].Code = NCode;
}

static void AddSing8(char *NName, Byte NCode) {
   if (InstrZ >= Sing8OrderCnt) exit(255);

   Sing8Orders[InstrZ].Name = NName;
   Sing8Orders[InstrZ++].Code = NCode;
}

static void AddBit63(char *NName, Byte NCode) {
   if (InstrZ >= Bit63OrderCnt) exit(255);

   Bit63Orders[InstrZ].Name = NName;
   Bit63Orders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("ABA", CPU6800, CPU6811, 0x001b);
   AddFixed("ABX", CPU6301, CPU6811, 0x003a);
   AddFixed("ABY", CPU6811, CPU6811, 0x183a);
   AddFixed("ASLD", CPU6301, CPU6811, 0x0005);
   AddFixed("CBA", CPU6800, CPU6811, 0x0011);
   AddFixed("CLC", CPU6800, CPU6811, 0x000c);
   AddFixed("CLI", CPU6800, CPU6811, 0x000e);
   AddFixed("CLV", CPU6800, CPU6811, 0x000a);
   AddFixed("DAA", CPU6800, CPU6811, 0x0019);
   AddFixed("DES", CPU6800, CPU6811, 0x0034);
   AddFixed("DEX", CPU6800, CPU6811, 0x0009);
   AddFixed("DEY", CPU6811, CPU6811, 0x1809);
   AddFixed("FDIV", CPU6811, CPU6811, 0x0003);
   AddFixed("IDIV", CPU6811, CPU6811, 0x0002);
   AddFixed("INS", CPU6800, CPU6811, 0x0031);
   AddFixed("INX", CPU6800, CPU6811, 0x0008);
   AddFixed("INY", CPU6811, CPU6811, 0x1808);
   AddFixed("LSLD", CPU6301, CPU6811, 0x0005);
   AddFixed("LSRD", CPU6301, CPU6811, 0x0004);
   AddFixed("MUL", CPU6301, CPU6811, 0x003d);
   AddFixed("NOP", CPU6800, CPU6811, 0x0001);
   AddFixed("PSHX", CPU6301, CPU6811, 0x003c);
   AddFixed("PSHY", CPU6811, CPU6811, 0x183c);
   AddFixed("PULX", CPU6301, CPU6811, 0x0038);
   AddFixed("PULY", CPU6811, CPU6811, 0x1838);
   AddFixed("RTI", CPU6800, CPU6811, 0x003b);
   AddFixed("RTS", CPU6800, CPU6811, 0x0039);
   AddFixed("SBA", CPU6800, CPU6811, 0x0010);
   AddFixed("SEC", CPU6800, CPU6811, 0x000d);
   AddFixed("SEI", CPU6800, CPU6811, 0x000f);
   AddFixed("SEV", CPU6800, CPU6811, 0x000b);
   AddFixed("SLP", CPU6301, CPU6301, 0x001a);
   AddFixed("STOP", CPU6811, CPU6811, 0x00cf);
   AddFixed("SWI", CPU6800, CPU6811, 0x003f);
   AddFixed("TAB", CPU6800, CPU6811, 0x0016);
   AddFixed("TAP", CPU6800, CPU6811, 0x0006);
   AddFixed("TBA", CPU6800, CPU6811, 0x0017);
   AddFixed("TPA", CPU6800, CPU6811, 0x0007);
   AddFixed("TSX", CPU6800, CPU6811, 0x0030);
   AddFixed("TSY", CPU6811, CPU6811, 0x1830);
   AddFixed("TXS", CPU6800, CPU6811, 0x0035);
   AddFixed("TYS", CPU6811, CPU6811, 0x1835);
   AddFixed("WAI", CPU6800, CPU6811, 0x003e);
   AddFixed("XGDX", CPU6811, CPU6811, 0x008f);
   AddFixed("XGDY", CPU6811, CPU6811, 0x188f);

   RelOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * RelOrderCnt);
   InstrZ = 0;
   AddRel("BCC", 0x24);
   AddRel("BCS", 0x25);
   AddRel("BEQ", 0x27);
   AddRel("BGE", 0x2c);
   AddRel("BGT", 0x2e);
   AddRel("BHI", 0x22);
   AddRel("BHS", 0x24);
   AddRel("BLE", 0x2f);
   AddRel("BLO", 0x25);
   AddRel("BLS", 0x23);
   AddRel("BLT", 0x2d);
   AddRel("BMI", 0x2b);
   AddRel("BNE", 0x26);
   AddRel("BPL", 0x2a);
   AddRel("BRA", 0x20);
   AddRel("BRN", 0x21);
   AddRel("BSR", 0x8d);
   AddRel("BVC", 0x28);
   AddRel("BVS", 0x29);

   ALU8Orders = (ALU8Order *) malloc(sizeof(ALU8Order) * ALU8OrderCnt);
   InstrZ = 0;
   AddALU8("SUB", true, 0x80);
   AddALU8("CMP", true, 0x81);
   AddALU8("SBC", true, 0x82);
   AddALU8("AND", true, 0x84);
   AddALU8("BIT", true, 0x85);
   AddALU8("LDA", true, 0x86);
   AddALU8("STA", false, 0x87);
   AddALU8("EOR", true, 0x88);
   AddALU8("ADC", true, 0x89);
   AddALU8("ORA", true, 0x8a);
   AddALU8("ADD", true, 0x8b);

   ALU16Orders = (ALU16Order *) malloc(sizeof(ALU16Order) * ALU16OrderCnt);
   InstrZ = 0;
   AddALU16("ADDD", true, CPU6301, 0, 0xc3);
   AddALU16("SUBD", true, CPU6301, 0, 0x83);
   AddALU16("LDD", true, CPU6301, 0, 0xcc);
   AddALU16("STD", false, CPU6301, 0, 0xcd);
   AddALU16("CPD", true, CPU6811, 1, 0x83);
   AddALU16("CPX", true, CPU6800, 2, 0x8c);
   AddALU16("CPY", true, CPU6811, 3, 0x8c);
   AddALU16("LDS", true, CPU6800, 0, 0x8e);
   AddALU16("STS", false, CPU6800, 0, 0x8f);
   AddALU16("LDX", true, CPU6800, 2, 0xce);
   AddALU16("STX", false, CPU6800, 2, 0xcf);
   AddALU16("LDY", true, CPU6811, 3, 0xce);
   AddALU16("STY", false, CPU6811, 3, 0xcf);

   Sing8Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * Sing8OrderCnt);
   InstrZ = 0;
   AddSing8("ASL", 0x48);
   AddSing8("LSL", 0x48);
   AddSing8("ASR", 0x47);
   AddSing8("CLR", 0x4f);
   AddSing8("COM", 0x43);
   AddSing8("DEC", 0x4a);
   AddSing8("INC", 0x4c);
   AddSing8("LSR", 0x44);
   AddSing8("NEG", 0x40);
   AddSing8("ROL", 0x49);
   AddSing8("ROR", 0x46);
   AddSing8("TST", 0x4d);

   Bit63Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * Bit63OrderCnt);
   InstrZ = 0;
   AddBit63("AIM", 0x61);
   AddBit63("EIM", 0x65);
   AddBit63("OIM", 0x62);
   AddBit63("TIM", 0x6b);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(RelOrders);
   free(ALU8Orders);
   free(ALU16Orders);
   free(Sing8Orders);
   free(Bit63Orders);
}

/*---------------------------------------------------------------------------*/

static bool SplitAcc(char *Op) {
   char Ch;
   Integer z;
   int OpLen = strlen(Op), OpPartLen = strlen(OpPart);

   Ch = OpPart[OpPartLen - 1];
   if ((OpLen + 1 == OpPartLen) && (strncmp(OpPart, Op, OpLen) == 0) && ((Ch == 'A') || (Ch == 'B'))) {
      for (z = ArgCnt; z >= 1; z--) strcopy(ArgStr[z + 1], ArgStr[z]);
      ArgStr[1][0] = Ch;
      ArgStr[1][1] = '\0';
      OpPart[OpPartLen - 1] = '\0';
      ArgCnt++;
   }
   return (Memo(Op));
}

static void DecodeAdr(Integer StartInd, Integer StopInd, Byte Erl) {
   String Asc;
   bool OK, ErrOcc;
   Word AdrWord;
   Byte Bit8;

   AdrMode = ModNone;
   AdrPart = 0;
   strmaxcpy(Asc, ArgStr[StartInd], 255);
   ErrOcc = false;

/* eine Komponente ? */

   if (StartInd == StopInd) {

   /* Akkumulatoren ? */

      if (strcasecmp(Asc, "A") == 0) {
         if ((MModAcc & Erl) != 0) AdrMode = ModAcc;
      } else if (strcasecmp(Asc, "B") == 0) {
         if ((MModAcc & Erl) != 0) {
            AdrMode = ModAcc;
            AdrPart = 1;
         }
      }

   /* immediate ? */

      else if ((strlen(Asc) > 1) && (*Asc == '#')) {
         if ((MModImm & Erl) != 0) {
            if (OpSize == 1) {
               AdrWord = EvalIntExpression(Asc + 1, Int16, &OK);
               if (OK) {
                  AdrMode = ModImm;
                  AdrVals[AdrCnt++] = Hi(AdrWord);
                  AdrVals[AdrCnt++] = Lo(AdrWord);
               } else ErrOcc = true;
            } else {
               AdrVals[AdrCnt] = EvalIntExpression(Asc + 1, Int8, &OK);
               if (OK) {
                  AdrMode = ModImm;
                  AdrCnt++;
               } else ErrOcc = true;
            }
         }
      }

   /* absolut ? */

      else {
         Bit8 = 0;
         if (*Asc == '<') {
            Bit8 = 2;
            strmove(Asc, 1);
         } else if (*Asc == '>') {
            Bit8 = 1;
            strmove(Asc, 1);
         }
         if ((Bit8 == 2) || ((MModExt & Erl) == 0))
            AdrWord = EvalIntExpression(Asc, Int8, &OK);
         else
            AdrWord = EvalIntExpression(Asc, Int16, &OK);
         if (OK) {
            if (((MModDir & Erl) != 0) && (Bit8 != 1) && ((Bit8 == 2) || ((MModExt & Erl) == 0) || (Hi(AdrWord) == 0))) {
               if (Hi(AdrWord) != 0) {
                  WrError(1340);
                  ErrOcc = true;
               } else {
                  AdrMode = ModDir;
                  AdrPart = 1;
                  AdrVals[AdrCnt++] = Lo(AdrWord);
               }
            } else if ((MModExt & Erl) != 0) {
               AdrMode = ModExt;
               AdrPart = 3;
               AdrVals[AdrCnt++] = Hi(AdrWord);
               AdrVals[AdrCnt++] = Lo(AdrWord);
            }
         } else ErrOcc = true;
      }
   }

/* zwei Komponenten ? */

   else if (StartInd + 1 == StopInd) {

   /* indiziert ? */

      if (((strcasecmp(ArgStr[StopInd], "X") == 0) || (strcasecmp(ArgStr[StopInd], "Y") == 0))) {
         if ((MModInd & Erl) != 0) {
            AdrWord = EvalIntExpression(Asc, Int8, &OK);
            if (OK)
               if ((MomCPU < CPU6811) && (strcasecmp(ArgStr[StartInd + 1], "Y") == 0)) {
                  WrError(1505);
                  ErrOcc = true;
               } else {
                  AdrVals[AdrCnt++] = Lo(AdrWord);
                  AdrMode = ModInd;
                  AdrPart = 2;
                  if (strcasecmp(ArgStr[StartInd + 1], "Y") == 0) {
                     BAsmCode[PrefCnt++] = 0x18;
                  }
            } else ErrOcc = true;
         }
      } else {
         WrXError(1445, ArgStr[StopInd]);
         ErrOcc = true;
      }

   } else {
      WrError(1110);
      ErrOcc = true;
   }

   if ((!ErrOcc) && (AdrMode == ModNone)) WrError(1350);
}

static void AddPrefix(Byte Prefix) {
   BAsmCode[PrefCnt++] = Prefix;
}

static void Try2Split(Integer Src) {
   Integer z;
   char *p;

   KillPrefBlanks(ArgStr[Src]);
   KillPostBlanks(ArgStr[Src]);
   p = ArgStr[Src] + strlen(ArgStr[Src]) - 1;
   while ((p > ArgStr[Src]) && (!isspace(*p))) p--;
   if (p > ArgStr[Src]) {
      for (z = ArgCnt; z >= Src; z--) strcopy(ArgStr[z + 1], ArgStr[z]);
      ArgCnt++;
      strcopy(ArgStr[Src + 1], p + 1);
      *p = '\0';
      KillPostBlanks(ArgStr[Src]);
      KillPrefBlanks(ArgStr[Src + 1]);
   }
}

static bool DecodePseudo(void) {
   return false;
}

static void MakeCode_68(void) {
   Integer z, AdrInt;
   bool OK;
   Byte AdrByte, Mask;

   int erg;
   FixedOrder *forder;

   CodeLen = 0;
   DontPrint = false;
   PrefCnt = 0;
   AdrCnt = 0;
   OpSize = 0;

/* Operandengroesse festlegen */

   if (*AttrPart != '\0')
      switch (toupper(*AttrPart)) {
         case 'B':
            OpSize = 0;
            break;
         case 'W':
            OpSize = 1;
            break;
         case 'L':
            OpSize = 2;
            break;
         case 'Q':
            OpSize = 3;
            break;
         case 'S':
            OpSize = 4;
            break;
         case 'D':
            OpSize = 5;
            break;
         case 'X':
            OpSize = 6;
            break;
         case 'P':
            OpSize = 7;
            break;
         default:
            WrError(1107);
            return;
      }

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(true)) return;
   if (DecodeMoto16Pseudo(OpSize, true)) return;

/* Anweisungen ohne Argument */

/* Sonderfall : XGDX hat anderen Code bei 6301 !!!! */

   if ((MomCPU == CPU6301) && (Memo("XGDX"))) {
      if (ArgCnt != 0) WrError(1110);
      else {
         CodeLen = 1;
         BAsmCode[0] = 0x18;
      }
      return;
   }

   for (z = 0, forder = FixedOrders; z < FixedOrderCnt; z++, forder++)
      if ((erg = strcmp(OpPart, forder->Name)) == 0) {
         if (ArgCnt != 0) WrError(1110);
         else if ((MomCPU < forder->MinCPU) || (MomCPU > forder->MaxCPU)) WrError(1500);
         else if (Hi(forder->Code) != 0) {
            CodeLen = 2;
            BAsmCode[0] = Hi(forder->Code);
            BAsmCode[1] = Lo(forder->Code);
         } else {
            CodeLen = 1;
            BAsmCode[0] = Lo(forder->Code);
         }
         return;
      } else if (erg < 0) break;

/* rel. Spruenge */

   for (z = 0; z < RelOrderCnt; z++)
      if ((erg = strcmp(OpPart, RelOrders[z].Name)) == 0) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrInt = EvalIntExpression(ArgStr[1], Int16, &OK);
            if (OK) {
               AdrInt -= EProgCounter() + 2;
               if (((AdrInt < -128) || (AdrInt > 127)) && (!SymbolQuestionable)) WrError(1370);
               else {
                  CodeLen = 2;
                  BAsmCode[0] = RelOrders[z].Code;
                  BAsmCode[1] = Lo(AdrInt);
               }
            }
         }
         return;
      } else if (erg < 0) break;

/* Arithmetik */

   for (z = 0; z < ALU8OrderCnt; z++)
      if (SplitAcc(ALU8Orders[z].Name)) {
         if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
         else {
            DecodeAdr(2, ArgCnt, ((ALU8Orders[z].MayImm) ? MModImm : 0) + MModInd + MModExt + MModDir);
            if (AdrMode != ModNone) {
               BAsmCode[PrefCnt] = ALU8Orders[z].Code + (AdrPart << 4);
               DecodeAdr(1, 1, 1);
               if (AdrMode != ModNone) {
                  BAsmCode[PrefCnt] += AdrPart << 6;
                  CodeLen = PrefCnt + 1 + AdrCnt;
                  memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
               }
            }
         }
         return;
      }

   for (z = 0; z < ALU16OrderCnt; z++)
      if (Memo(ALU16Orders[z].Name)) {
         OpSize = 1;
         if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
         else if (MomCPU < ALU16Orders[z].MinCPU) WrError(1500);
         else {
            DecodeAdr(1, ArgCnt, (ALU16Orders[z].MayImm ? MModImm : 0) + MModInd + MModExt + MModDir);
            if (AdrMode != ModNone) {
               switch (ALU16Orders[z].PageShift) {
                  case 1:
                     if (PrefCnt == 1) BAsmCode[PrefCnt - 1] = Page4Prefix;
                     else AddPrefix(Page3Prefix);
                     break;
                  case 2:
                     if (PrefCnt == 1) BAsmCode[PrefCnt - 1] = Page4Prefix;
                     break;
                  case 3:
                     if (PrefCnt == 0) AddPrefix((AdrMode == ModInd) ? Page3Prefix : Page2Prefix);
                     break;
               }
               BAsmCode[PrefCnt] = ALU16Orders[z].Code + (AdrPart << 4);
               CodeLen = PrefCnt + 1 + AdrCnt;
               memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
            }
         }
         return;
      }

   for (z = 0; z < Bit63OrderCnt; z++)
      if (Memo(Bit63Orders[z].Name)) {
         if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
         else if (MomCPU != CPU6301) WrError(1500);
         else {
            DecodeAdr(1, 1, MModImm);
            if (AdrMode != ModNone) {
               DecodeAdr(2, ArgCnt, MModDir + MModInd);
               if (AdrMode != ModNone) {
                  BAsmCode[PrefCnt] = Bit63Orders[z].Code;
                  if (AdrMode == ModDir) BAsmCode[PrefCnt] += 0x10;
                  CodeLen = PrefCnt + 1 + AdrCnt;
                  memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
               }
            }
         }
         return;
      }

   for (z = 0; z < Sing8OrderCnt; z++)
      if (SplitAcc(Sing8Orders[z].Name)) {
         if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
         else {
            DecodeAdr(1, ArgCnt, MModAcc + MModExt + MModInd);
            if (AdrMode != ModNone) {
               CodeLen = PrefCnt + 1 + AdrCnt;
               BAsmCode[PrefCnt] = Sing8Orders[z].Code + (AdrPart << 4);
               memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
            }
         }
         return;
      }

   if (Memo("JMP")) {
      if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
      else {
         DecodeAdr(1, ArgCnt, MModExt + MModInd);
         if (AdrMode != ModImm) {
            CodeLen = PrefCnt + 1 + AdrCnt;
            BAsmCode[PrefCnt] = 0x4e + (AdrPart << 4);
            memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
         }
      }
      return;
   }

   if (Memo("JSR")) {
      if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
      else {
         DecodeAdr(1, ArgCnt, MModDir + MModExt + MModInd);
         if (AdrMode != ModImm) {
            CodeLen = PrefCnt + 1 + AdrCnt;
            BAsmCode[PrefCnt] = 0x8d + (AdrPart << 4);
            memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
         }
      }
      return;
   }

   if ((SplitAcc("PSH")) || (SplitAcc("PUL"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(1, 1, MModAcc);
         if (AdrMode != ModNone) {
            CodeLen = 1;
            BAsmCode[0] = 0x32 + AdrPart;
            if (Memo("PSH")) BAsmCode[0] += 4;
         }
      }
      return;
   }

   if ((Memo("BRSET")) || (Memo("BRCLR"))) {
      if (ArgCnt == 1) {
         Try2Split(1);
         Try2Split(1);
      } else if (ArgCnt == 2) {
         Try2Split(ArgCnt);
         Try2Split(2);
      }
      if ((ArgCnt < 3) || (ArgCnt > 4)) WrError(1110);
      else if (MomCPU < CPU6811) WrError(1500);
      else {
         if (ArgStr[ArgCnt - 1][0] == '#') strmove(ArgStr[ArgCnt - 1], 1);
         Mask = EvalIntExpression(ArgStr[ArgCnt - 1], Int8, &OK);
         if (OK) {
            DecodeAdr(1, ArgCnt - 2, MModDir + MModInd);
            if (AdrMode != ModNone) {
               AdrInt = EvalIntExpression(ArgStr[ArgCnt], Int16, &OK);
               if (OK) {
                  AdrInt -= EProgCounter() + 3 + PrefCnt + AdrCnt;
                  if ((AdrInt < -128) || (AdrInt > 127)) WrError(1370);
                  else {
                     CodeLen = PrefCnt + 3 + AdrCnt;
                     BAsmCode[PrefCnt] = 0x12;
                     if (AdrMode == ModInd) BAsmCode[PrefCnt] += 12;
                     if (Memo("BRCLR")) BAsmCode[PrefCnt]++;
                     memcpy(BAsmCode + PrefCnt + 1, AdrVals, AdrCnt);
                     BAsmCode[PrefCnt + 1 + AdrCnt] = Mask;
                     BAsmCode[PrefCnt + 2 + AdrCnt] = Lo(AdrInt);
                  }
               }
            }
         }
      }
      return;
   }

   if ((Memo("BSET")) || (Memo("BCLR"))) {
      if (MomCPU == CPU6301) {
         strcopy(ArgStr[ArgCnt + 1], ArgStr[1]);
         for (z = 1; z <= ArgCnt - 1; z++) strcopy(ArgStr[z], ArgStr[z + 1]);
         strcopy(ArgStr[ArgCnt], ArgStr[ArgCnt + 1]);
      }
      if ((ArgCnt >= 1) && (ArgCnt <= 2)) Try2Split(ArgCnt);
      if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
      else if (MomCPU < CPU6301) WrError(1500);
      else {
         if (ArgStr[ArgCnt][0] == '#') strmove(ArgStr[ArgCnt], 1);
         Mask = EvalIntExpression(ArgStr[ArgCnt], Int8, &OK);
         if (!OK || MomCPU != CPU6301) ;
         else if (Mask > 7) {
            WrError(1320);
            OK = false;
         } else {
            Mask = 1 << Mask;
            if (Memo("BCLR")) Mask = 0xff - Mask;
         }
         if (OK) {
            DecodeAdr(1, ArgCnt - 1, MModDir + MModInd);
            if (AdrMode != ModNone) {
               CodeLen = PrefCnt + 2 + AdrCnt;
               if (MomCPU == CPU6301) {
                  BAsmCode[PrefCnt] = 0x61;
                  if (Memo("BSET")) BAsmCode[PrefCnt]++;
                  if (AdrMode == ModDir) BAsmCode[PrefCnt] += 0x10;
                  BAsmCode[1 + PrefCnt] = Mask;
                  memcpy(BAsmCode + 2 + PrefCnt, AdrVals, AdrCnt);
               } else {
                  BAsmCode[PrefCnt] = 0x14;
                  if (Memo("BCLR")) BAsmCode[PrefCnt]++;
                  if (AdrMode == ModInd) BAsmCode[PrefCnt] += 8;
                  memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
                  BAsmCode[1 + PrefCnt + AdrCnt] = Mask;
               }
            }
         }
      }
      return;
   }

   if ((Memo("BTST")) || (Memo("BTGL"))) {
      if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
      else if (MomCPU != CPU6301) WrError(1500);
      else {
         AdrByte = EvalIntExpression(ArgStr[1], Int8, &OK);
         if (!OK) ;
         else if (AdrByte > 7) WrError(1320);
         else {
            DecodeAdr(2, ArgCnt, MModDir + MModInd);
            if (AdrMode != ModNone) {
               CodeLen = PrefCnt + 2 + AdrCnt;
               BAsmCode[1 + PrefCnt] = 1 << AdrByte;
               memcpy(BAsmCode + 2 + PrefCnt, AdrVals, AdrCnt);
               BAsmCode[PrefCnt] = 0x65;
               if (Memo("BTST")) BAsmCode[PrefCnt] += 6;
               if (AdrMode == ModDir) BAsmCode[PrefCnt] += 0x10;
            }
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_68(void) {
   if (ActPC == SegCode) return (ProgCounter() < 0x10000);
   else return false;
}

static bool IsDef_68(void) {
   return false;
}

static void SwitchFrom_68() {
   DeinitFields();
}

static void SwitchTo_68(void) {
   TurnWords = false;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "*";
   HeaderID = 0x61;
   NOPCode = 0x01;
   DivideChars = ",";
   HasAttrs = true;
   AttrChars = ".";

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_68;
   ChkPC = ChkPC_68;
   IsDef = IsDef_68;
   SwitchFrom = SwitchFrom_68;
   InitFields();
}

void code68_init(void) {
   CPU6800 = AddCPU("6800", SwitchTo_68);
   CPU6301 = AddCPU("6301", SwitchTo_68);
   CPU6811 = AddCPU("6811", SwitchTo_68);
}
