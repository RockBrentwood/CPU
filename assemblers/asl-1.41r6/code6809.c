/* code6809.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 6809/6309                                                   */
/*                                                                           */
/* Historie: 10.10.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "stringutil.h"
#include "bpemu.h"

#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct {
   char *Name;
   Word Code;
   CPUVar MinCPU;
} BaseOrder;

typedef struct {
   char *Name;
   Word Code;
   bool Inv;
   CPUVar MinCPU;
} FlagOrder;

typedef struct {
   char *Name;
   Word Code8;
   Word Code16;
   CPUVar MinCPU;
} RelOrder;

typedef struct {
   char *Name;
   Word Code;
   Byte Op16;
   bool MayImm;
   CPUVar MinCPU;
} ALUOrder;

#define ModNone (-1)
#define ModImm 1
#define ModDir 2
#define ModInd 3
#define ModExt 4

#define FixedOrderCnt 73
#define RelOrderCnt 19
#define ALUOrderCnt 65
#define ALU2OrderCnt 8
#define RMWOrderCnt 13
#define FlagOrderCnt 3
#define LEAOrderCnt 4
#define ImmOrderCnt 4
#define StackOrderCnt 4
#define BitOrderCnt 8

#define StackRegCnt 11
static char StackRegNames[StackRegCnt][4] = { "CCR", "A", "B", "DPR", "X", "Y", "S/U", "PC", "CC", "DP", "S" };
static Byte StackRegCodes[StackRegCnt] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 3, 6 };

static char *FlagChars = "CVZNIHFE";

static ShortInt AdrMode;
static Byte AdrVals[5];
static Byte OpSize;
static bool ExtFlag;
static LongInt DPRValue;

static BaseOrder *FixedOrders;
static RelOrder *RelOrders;
static ALUOrder *ALUOrders;
static char **ALU2Orders;
static BaseOrder *RMWOrders;
static FlagOrder *FlagOrders;
static BaseOrder *LEAOrders;
static BaseOrder *ImmOrders;
static BaseOrder *StackOrders;
static char **BitOrders;

static SimpProc SaveInitProc;

static CPUVar CPU6809, CPU6309;

/*-------------------------------------------------------------------------*/
/* Erzeugung/Aufloesung Codetabellen*/

static void AddFixed(char *NName, Word NCode, CPUVar NCPU) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ].Code = NCode;
   FixedOrders[InstrZ++].MinCPU = NCPU;
}

static void AddRel(char *NName, Word NCode8, Word NCode16) {
   if (InstrZ >= RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name = NName;
   RelOrders[InstrZ].Code8 = NCode8;
   RelOrders[InstrZ++].Code16 = NCode16;
}

static void AddALU(char *NName, Word NCode, Byte NSize, bool NImm, CPUVar NCPU) {
   if (InstrZ >= ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name = NName;
   ALUOrders[InstrZ].Code = NCode;
   ALUOrders[InstrZ].Op16 = NSize;
   ALUOrders[InstrZ].MayImm = NImm;
   ALUOrders[InstrZ++].MinCPU = NCPU;
}

static void AddRMW(char *NName, Word NCode, CPUVar NCPU) {
   if (InstrZ >= RMWOrderCnt) exit(255);
   RMWOrders[InstrZ].Name = NName;
   RMWOrders[InstrZ].Code = NCode;
   RMWOrders[InstrZ++].MinCPU = NCPU;
}

static void AddFlag(char *NName, Word NCode, bool NInv, CPUVar NCPU) {
   if (InstrZ >= FlagOrderCnt) exit(255);
   FlagOrders[InstrZ].Name = NName;
   FlagOrders[InstrZ].Code = NCode;
   FlagOrders[InstrZ].Inv = NInv;
   FlagOrders[InstrZ++].MinCPU = NCPU;
}

static void AddLEA(char *NName, Word NCode, CPUVar NCPU) {
   if (InstrZ >= LEAOrderCnt) exit(255);
   LEAOrders[InstrZ].Name = NName;
   LEAOrders[InstrZ].Code = NCode;
   LEAOrders[InstrZ++].MinCPU = NCPU;
}

static void AddImm(char *NName, Word NCode, CPUVar NCPU) {
   if (InstrZ >= ImmOrderCnt) exit(255);
   ImmOrders[InstrZ].Name = NName;
   ImmOrders[InstrZ].Code = NCode;
   ImmOrders[InstrZ++].MinCPU = NCPU;
}

static void AddStack(char *NName, Word NCode, CPUVar NCPU) {
   if (InstrZ >= StackOrderCnt) exit(255);
   StackOrders[InstrZ].Name = NName;
   StackOrders[InstrZ].Code = NCode;
   StackOrders[InstrZ++].MinCPU = NCPU;
}

static void InitFields(void) {
   FixedOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("NOP", 0x0012, CPU6809);
   AddFixed("SYNC", 0x0013, CPU6809);
   AddFixed("DAA", 0x0019, CPU6809);
   AddFixed("SEX", 0x001d, CPU6809);
   AddFixed("RTS", 0x0039, CPU6809);
   AddFixed("ABX", 0x003a, CPU6809);
   AddFixed("RTI", 0x003b, CPU6809);
   AddFixed("MUL", 0x003d, CPU6809);
   AddFixed("SWI2", 0x103f, CPU6809);
   AddFixed("SWI3", 0x113f, CPU6809);
   AddFixed("NEGA", 0x0040, CPU6809);
   AddFixed("COMA", 0x0043, CPU6809);
   AddFixed("LSRA", 0x0044, CPU6809);
   AddFixed("RORA", 0x0046, CPU6809);
   AddFixed("ASRA", 0x0047, CPU6809);
   AddFixed("ASLA", 0x0048, CPU6809);
   AddFixed("LSLA", 0x0048, CPU6809);
   AddFixed("ROLA", 0x0049, CPU6809);
   AddFixed("DECA", 0x004a, CPU6809);
   AddFixed("INCA", 0x004c, CPU6809);
   AddFixed("TSTA", 0x004d, CPU6809);
   AddFixed("CLRA", 0x004f, CPU6809);
   AddFixed("NEGB", 0x0050, CPU6809);
   AddFixed("COMB", 0x0053, CPU6809);
   AddFixed("LSRB", 0x0054, CPU6809);
   AddFixed("RORB", 0x0056, CPU6809);
   AddFixed("ASRB", 0x0057, CPU6809);
   AddFixed("ASLB", 0x0058, CPU6809);
   AddFixed("LSLB", 0x0058, CPU6809);
   AddFixed("ROLB", 0x0059, CPU6809);
   AddFixed("DECB", 0x005a, CPU6809);
   AddFixed("INCB", 0x005c, CPU6809);
   AddFixed("TSTB", 0x005d, CPU6809);
   AddFixed("CLRB", 0x005f, CPU6809);
   AddFixed("PSHSW", 0x1038, CPU6309);
   AddFixed("PULSW", 0x1039, CPU6309);
   AddFixed("PSHUW", 0x103a, CPU6309);
   AddFixed("PULUW", 0x103b, CPU6309);
   AddFixed("SEXW", 0x0014, CPU6309);
   AddFixed("NEGD", 0x1040, CPU6309);
   AddFixed("COMD", 0x1043, CPU6309);
   AddFixed("LSRD", 0x1044, CPU6309);
   AddFixed("RORD", 0x1046, CPU6309);
   AddFixed("ASRD", 0x1047, CPU6309);
   AddFixed("ASLD", 0x1048, CPU6309);
   AddFixed("LSLD", 0x1048, CPU6309);
   AddFixed("ROLD", 0x1049, CPU6309);
   AddFixed("DECD", 0x104a, CPU6309);
   AddFixed("INCD", 0x104c, CPU6309);
   AddFixed("TSTD", 0x104d, CPU6309);
   AddFixed("CLRD", 0x104f, CPU6309);
   AddFixed("COMW", 0x1053, CPU6309);
   AddFixed("LSRW", 0x1054, CPU6309);
   AddFixed("RORW", 0x1056, CPU6309);
   AddFixed("ROLW", 0x1059, CPU6309);
   AddFixed("DECW", 0x105a, CPU6309);
   AddFixed("INCW", 0x105c, CPU6309);
   AddFixed("TSTW", 0x105d, CPU6309);
   AddFixed("CLRW", 0x105f, CPU6309);
   AddFixed("COME", 0x1143, CPU6309);
   AddFixed("DECE", 0x114a, CPU6309);
   AddFixed("INCE", 0x114c, CPU6309);
   AddFixed("TSTE", 0x114d, CPU6309);
   AddFixed("CLRE", 0x114f, CPU6309);
   AddFixed("COMF", 0x1153, CPU6309);
   AddFixed("DECF", 0x115a, CPU6309);
   AddFixed("INCF", 0x115c, CPU6309);
   AddFixed("TSTF", 0x115d, CPU6309);
   AddFixed("CLRF", 0x115f, CPU6309);
   AddFixed("CLRS", 0x1fd4, CPU6309);
   AddFixed("CLRV", 0x1fd7, CPU6309);
   AddFixed("CLRX", 0x1fd1, CPU6309);
   AddFixed("CLRY", 0x1fd2, CPU6309);

   RelOrders = (RelOrder *) malloc(sizeof(RelOrder) * RelOrderCnt);
   InstrZ = 0;
   AddRel("BRA", 0x0020, 0x0016);
   AddRel("BRN", 0x0021, 0x1021);
   AddRel("BHI", 0x0022, 0x1022);
   AddRel("BLS", 0x0023, 0x1023);
   AddRel("BHS", 0x0024, 0x1024);
   AddRel("BCC", 0x0024, 0x1024);
   AddRel("BLO", 0x0025, 0x1025);
   AddRel("BCS", 0x0025, 0x1025);
   AddRel("BNE", 0x0026, 0x1026);
   AddRel("BEQ", 0x0027, 0x1027);
   AddRel("BVC", 0x0028, 0x1028);
   AddRel("BVS", 0x0029, 0x1029);
   AddRel("BPL", 0x002a, 0x102a);
   AddRel("BMI", 0x002b, 0x102b);
   AddRel("BGE", 0x002c, 0x102c);
   AddRel("BLT", 0x002d, 0x102d);
   AddRel("BGT", 0x002e, 0x102e);
   AddRel("BLE", 0x002f, 0x102f);
   AddRel("BSR", 0x008d, 0x0017);

   ALUOrders = (ALUOrder *) malloc(sizeof(ALUOrder) * ALUOrderCnt);
   InstrZ = 0;
   AddALU("LDA", 0x0086, 0, true, CPU6809);
   AddALU("STA", 0x0087, 0, false, CPU6809);
   AddALU("CMPA", 0x0081, 0, true, CPU6809);
   AddALU("ADDA", 0x008b, 0, true, CPU6809);
   AddALU("ADCA", 0x0089, 0, true, CPU6809);
   AddALU("SUBA", 0x0080, 0, true, CPU6809);
   AddALU("SBCA", 0x0082, 0, true, CPU6809);
   AddALU("ANDA", 0x0084, 0, true, CPU6809);
   AddALU("ORA", 0x008a, 0, true, CPU6809);
   AddALU("EORA", 0x0088, 0, true, CPU6809);
   AddALU("BITA", 0x0085, 0, true, CPU6809);

   AddALU("LDB", 0x00c6, 0, true, CPU6809);
   AddALU("STB", 0x00c7, 0, false, CPU6809);
   AddALU("CMPB", 0x00c1, 0, true, CPU6809);
   AddALU("ADDB", 0x00cb, 0, true, CPU6809);
   AddALU("ADCB", 0x00c9, 0, true, CPU6809);
   AddALU("SUBB", 0x00c0, 0, true, CPU6809);
   AddALU("SBCB", 0x00c2, 0, true, CPU6809);
   AddALU("ANDB", 0x00c4, 0, true, CPU6809);
   AddALU("ORB", 0x00ca, 0, true, CPU6809);
   AddALU("EORB", 0x00c8, 0, true, CPU6809);
   AddALU("BITB", 0x00c5, 0, true, CPU6809);

   AddALU("LDD", 0x00cc, 1, true, CPU6809);
   AddALU("STD", 0x00cd, 1, false, CPU6809);
   AddALU("CMPD", 0x1083, 1, true, CPU6809);
   AddALU("ADDD", 0x00c3, 1, true, CPU6809);
   AddALU("ADCD", 0x1089, 1, true, CPU6309);
   AddALU("SUBD", 0x0083, 1, true, CPU6809);
   AddALU("SBCD", 0x1082, 1, true, CPU6309);
   AddALU("MULD", 0x118f, 1, true, CPU6309);
   AddALU("DIVD", 0x118d, 1, true, CPU6309);
   AddALU("ANDD", 0x1084, 1, true, CPU6309);
   AddALU("ORD", 0x108a, 1, true, CPU6309);
   AddALU("EORD", 0x1088, 1, true, CPU6309);
   AddALU("BITD", 0x1085, 1, true, CPU6309);

   AddALU("LDW", 0x1086, 1, true, CPU6309);
   AddALU("STW", 0x1087, 1, false, CPU6309);
   AddALU("CMPW", 0x1081, 1, true, CPU6309);
   AddALU("ADDW", 0x108b, 1, true, CPU6309);
   AddALU("SUBW", 0x1080, 1, true, CPU6309);

   AddALU("STQ", 0x10cd, 1, true, CPU6309);
   AddALU("DIVQ", 0x118e, 1, true, CPU6309);

   AddALU("LDE", 0x1186, 0, true, CPU6309);
   AddALU("STE", 0x1187, 0, false, CPU6309);
   AddALU("CMPE", 0x1181, 0, true, CPU6309);
   AddALU("ADDE", 0x118b, 0, true, CPU6309);
   AddALU("SUBE", 0x1180, 0, true, CPU6309);

   AddALU("LDF", 0x11c6, 0, true, CPU6309);
   AddALU("STF", 0x11c7, 0, false, CPU6309);
   AddALU("CMPF", 0x11c1, 0, true, CPU6309);
   AddALU("ADDF", 0x11cb, 0, true, CPU6309);
   AddALU("SUBF", 0x11c0, 0, true, CPU6309);

   AddALU("LDX", 0x008e, 1, true, CPU6809);
   AddALU("STX", 0x008f, 1, false, CPU6809);
   AddALU("CMPX", 0x008c, 1, true, CPU6809);

   AddALU("LDY", 0x108e, 1, true, CPU6809);
   AddALU("STY", 0x108f, 1, false, CPU6809);
   AddALU("CMPY", 0x108c, 1, true, CPU6809);

   AddALU("LDU", 0x00ce, 1, true, CPU6809);
   AddALU("STU", 0x00cf, 1, false, CPU6809);
   AddALU("CMPU", 0x1183, 1, true, CPU6809);

   AddALU("LDS", 0x10ce, 1, true, CPU6809);
   AddALU("STS", 0x10cf, 1, false, CPU6809);
   AddALU("CMPS", 0x118c, 1, true, CPU6809);

   AddALU("JSR", 0x008d, 1, false, CPU6809);

   ALU2Orders = (char **)malloc(sizeof(char *) * ALU2OrderCnt);
   ALU2Orders[0] = "ADD";
   ALU2Orders[1] = "ADC";
   ALU2Orders[2] = "SUB";
   ALU2Orders[3] = "SBC";
   ALU2Orders[4] = "AND";
   ALU2Orders[5] = "OR";
   ALU2Orders[6] = "EOR";
   ALU2Orders[7] = "CMP";

   RMWOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * RMWOrderCnt);
   InstrZ = 0;
   AddRMW("NEG", 0x00, CPU6809);
   AddRMW("COM", 0x03, CPU6809);
   AddRMW("LSR", 0x04, CPU6809);
   AddRMW("ROR", 0x06, CPU6809);
   AddRMW("ASR", 0x07, CPU6809);
   AddRMW("ASL", 0x08, CPU6809);
   AddRMW("LSL", 0x08, CPU6809);
   AddRMW("ROL", 0x09, CPU6809);
   AddRMW("DEC", 0x0a, CPU6809);
   AddRMW("INC", 0x0c, CPU6809);
   AddRMW("TST", 0x0d, CPU6809);
   AddRMW("JMP", 0x0e, CPU6809);
   AddRMW("CLR", 0x0f, CPU6809);

   FlagOrders = (FlagOrder *) malloc(sizeof(FlagOrder) * FlagOrderCnt);
   InstrZ = 0;
   AddFlag("CWAI", 0x3c, true, CPU6809);
   AddFlag("ANDCC", 0x1c, true, CPU6809);
   AddFlag("ORCC", 0x1a, false, CPU6809);

   LEAOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * LEAOrderCnt);
   InstrZ = 0;
   AddLEA("LEAX", 0x30, CPU6809);
   AddLEA("LEAY", 0x31, CPU6809);
   AddLEA("LEAS", 0x32, CPU6809);
   AddLEA("LEAU", 0x33, CPU6809);

   ImmOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * ImmOrderCnt);
   InstrZ = 0;
   AddImm("AIM", 0x02, CPU6309);
   AddImm("OIM", 0x01, CPU6309);
   AddImm("EIM", 0x05, CPU6309);
   AddImm("TIM", 0x0b, CPU6309);

   StackOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * StackOrderCnt);
   InstrZ = 0;
   AddStack("PSHS", 0x34, CPU6809);
   AddStack("PULS", 0x35, CPU6809);
   AddStack("PSHU", 0x36, CPU6809);
   AddStack("PULU", 0x37, CPU6809);

   BitOrders = (char **)malloc(sizeof(char *) * BitOrderCnt);
   BitOrders[0] = "BAND";
   BitOrders[1] = "BIAND";
   BitOrders[2] = "BOR";
   BitOrders[3] = "BIOR";
   BitOrders[4] = "BEOR";
   BitOrders[5] = "BIEOR";
   BitOrders[6] = "LDBT";
   BitOrders[7] = "STBT";
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(RelOrders);
   free(ALUOrders);
   free(ALU2Orders);
   free(RMWOrders);
   free(FlagOrders);
   free(LEAOrders);
   free(ImmOrders);
   free(StackOrders);
   free(BitOrders);
}

/*-------------------------------------------------------------------------*/

static bool CodeReg(char *ChIn, Byte * erg) {
   static char Regs[5] = "XYUS", *p;

   if (strlen(ChIn) != 1) return false;
   else {
      p = strchr(Regs, toupper(*ChIn));
      if (p == NULL) return false;
      *erg = p - Regs;
      return true;
   }
}

static void ChkZero(char *s, Byte * Erg) {
   if (*s == '>') {
      strmove(s, 1);
      *Erg = 1;
   } else if (*s == '<') {
      strmove(s, 1);
      *Erg = 2;
      if (*s == '<') {
         strmove(s, 1);
         *Erg = 3;
      }
   } else *Erg = 0;
}

static bool MayShort(Integer Arg) {
   return ((Arg >= -128) && (Arg < 127));
}

static void DecodeAdr(void) {
   String Asc, LAsc, temp;
   LongInt AdrLong;
   Word AdrWord;
   bool IndFlag, OK;
   Byte EReg, ZeroMode;
   char *p;
   Integer AdrInt;

   AdrMode = ModNone;
   AdrCnt = 0;
   strmaxcpy(Asc, ArgStr[1], 255);
   strmaxcpy(LAsc, ArgStr[ArgCnt], 255);

/* immediate */

   if (*Asc == '#') {
      switch (OpSize) {
         case 2:
            AdrLong = EvalIntExpression(Asc + 1, Int32, &OK);
            if (OK) {
               AdrVals[0] = Lo(AdrLong >> 24);
               AdrVals[1] = Lo(AdrLong >> 16);
               AdrVals[2] = Lo(AdrLong >> 8);
               AdrVals[3] = Lo(AdrLong);
               AdrCnt = 4;
            }
            break;
         case 1:
            AdrWord = EvalIntExpression(Asc + 1, Int16, &OK);
            if (OK) {
               AdrVals[0] = Hi(AdrWord);
               AdrVals[1] = Lo(AdrWord);
               AdrCnt = 2;
            }
            break;
         case 0:
            AdrVals[0] = EvalIntExpression(Asc + 1, Int8, &OK);
            if (OK) AdrCnt = 1;
            break;
      }
      if (OK) AdrMode = ModImm;
      return;
   }

/* indirekter Ausdruck ? */

   if ((*Asc == '[') && (Asc[strlen(Asc) - 1] == ']')) {
      IndFlag = true;
      strmove(Asc, 1);
      Asc[strlen(Asc) - 1] = '\0';
      ArgCnt = 0;
      while (*Asc != '\0') {
         ArgCnt++;
         p = QuotPos(Asc, ',');
         if (p != NULL) {
            *p = '\0';
            strmaxcpy(ArgStr[ArgCnt], Asc, 255);
            strcopy(Asc, p + 1);
         } else {
            strmaxcpy(ArgStr[ArgCnt], Asc, 255);
            *Asc = '\0';
         }
      }
      strmaxcpy(Asc, ArgStr[1], 255);
      strmaxcpy(LAsc, ArgStr[ArgCnt], 255);
   } else IndFlag = false;

/* Predekrement ? */

   if ((ArgCnt >= 1) && (ArgCnt <= 2) && (strlen(LAsc) == 2) && (*LAsc == '-') && (CodeReg(LAsc + 1, &EReg))) {
      if ((ArgCnt == 2) && (*Asc != '\0')) WrError(1350);
      else {
         AdrCnt = 1;
         AdrVals[0] = 0x82 + (EReg << 5) + (IndFlag << 4);
         AdrMode = ModInd;
      }
      return;
   }

   if ((ArgCnt >= 1) && (ArgCnt <= 2) && (strlen(LAsc) == 3) && (strncmp(LAsc, "--", 2) == 0) && (CodeReg(LAsc + 2, &EReg))) {
      if ((ArgCnt == 2) && (*Asc != '\0')) WrError(1350);
      else {
         AdrCnt = 1;
         AdrVals[0] = 0x83 + (EReg << 5) + (IndFlag << 4);
         AdrMode = ModInd;
      }
      return;
   }

   if ((ArgCnt >= 1) && (ArgCnt <= 2) && (strcasecmp(LAsc, "--W") == 0)) {
      if ((ArgCnt == 2) && (*Asc != '\0')) WrError(1350);
      else if (MomCPU < CPU6309) WrError(1505);
      else {
         AdrCnt = 1;
         AdrVals[0] = 0xef + IndFlag;
         AdrMode = ModInd;
      }
      return;
   }

/* Postinkrement ? */

   if ((ArgCnt >= 1) && (ArgCnt <= 2) && (strlen(LAsc) == 2) && (LAsc[1] == '+')) {
      temp[0] = (*LAsc);
      temp[1] = '\0';
      if (CodeReg(temp, &EReg)) {
         if ((ArgCnt == 2) && (*Asc != '\0')) WrError(1350);
         else {
            AdrCnt = 1;
            AdrVals[0] = 0x80 + (EReg << 5) + (IndFlag << 4);
            AdrMode = ModInd;
         }
         return;
      }
   }

   if ((ArgCnt >= 1) && (ArgCnt <= 2) && (strlen(LAsc) == 3) && (strncmp(LAsc + 1, "++", 2) == 0)) {
      temp[0] = (*LAsc);
      temp[1] = '\0';
      if (CodeReg(temp, &EReg)) {
         if ((ArgCnt == 2) && (*Asc != '\0')) WrError(1350);
         else {
            AdrCnt = 1;
            AdrVals[0] = 0x81 + (EReg << 5) + (IndFlag << 4);
            AdrMode = ModInd;
         }
         return;
      }
   }

   if ((ArgCnt >= 1) && (ArgCnt <= 2) && (strcasecmp(LAsc, "W++") == 0)) {
      if ((ArgCnt == 2) && (*Asc != '\0')) WrError(1350);
      else if (MomCPU < CPU6309) WrError(1505);
      else {
         AdrCnt = 1;
         AdrVals[0] = 0xcf + IndFlag;
         AdrMode = ModInd;
      }
      return;
   }

/* 16-Bit-Register (mit Index) ? */

   if ((ArgCnt <= 2) && (ArgCnt >= 1) && (CodeReg(LAsc, &EReg))) {
      AdrVals[0] = (EReg << 5) + (IndFlag << 4);

   /* nur 16-Bit-Register */

      if (ArgCnt == 1) {
         AdrCnt = 1;
         AdrVals[0] += 0x84;
         AdrMode = ModInd;
         return;
      }

   /* mit Index */

      if (strcasecmp(Asc, "A") == 0) {
         AdrCnt = 1;
         AdrVals[0] += 0x86;
         AdrMode = ModInd;
         return;
      }
      if (strcasecmp(Asc, "B") == 0) {
         AdrCnt = 1;
         AdrVals[0] += 0x85;
         AdrMode = ModInd;
         return;
      }
      if (strcasecmp(Asc, "D") == 0) {
         AdrCnt = 1;
         AdrVals[0] += 0x8b;
         AdrMode = ModInd;
         return;
      }
      if ((strcasecmp(Asc, "E") == 0) && (MomCPU >= CPU6309)) {
         if (EReg != 0) WrError(1350);
         else {
            AdrCnt = 1;
            AdrVals[0] += 0x87;
            AdrMode = ModInd;
         }
         return;
      }
      if ((strcasecmp(Asc, "F") == 0) && (MomCPU >= CPU6309)) {
         if (EReg != 0) WrError(1350);
         else {
            AdrCnt = 1;
            AdrVals[0] += 0x8a;
            AdrMode = ModInd;
         }
         return;
      }
      if ((strcasecmp(Asc, "W") == 0) && (MomCPU >= CPU6309)) {
         if (EReg != 0) WrError(1350);
         else {
            AdrCnt = 1;
            AdrVals[0] += 0x8e;
            AdrMode = ModInd;
         }
         return;
      }

   /* Displacement auswerten */

      ChkZero(Asc, &ZeroMode);
      if (ZeroMode > 1) {
         AdrInt = EvalIntExpression(Asc, Int8, &OK);
         if ((FirstPassUnknown) && (ZeroMode == 3)) AdrInt &= 0x0f;
      } else
         AdrInt = EvalIntExpression(Asc, Int16, &OK);

   /* Displacement 0 ? */

      if ((ZeroMode == 0) && (AdrInt == 0)) {
         AdrCnt = 1;
         AdrVals[0] += 0x84;
         AdrMode = ModInd;
         return;
      }

   /* 5-Bit-Displacement */

      else if ((ZeroMode == 3) || ((ZeroMode == 0) && (!IndFlag) && (AdrInt >= -16) && (AdrInt <= 15))) {
         if ((AdrInt < -16) || (AdrInt > 15)) WrError(1340);
         else if (IndFlag) WrError(1350);
         else {
            AdrMode = ModInd;
            AdrCnt = 1;
            AdrVals[0] += AdrInt & 0x1f;
         }
         return;
      }

   /* 8-Bit-Displacement */

      else if ((ZeroMode == 2) || ((ZeroMode == 0) && (MayShort(AdrInt)))) {
         if (!MayShort(AdrInt)) WrError(1340);
         else {
            AdrMode = ModInd;
            AdrCnt = 2;
            AdrVals[0] += 0x88;
            AdrVals[1] = Lo(AdrInt);
         };
         return;
      }

   /* 16-Bit-Displacement */

      else {
         AdrMode = ModInd;
         AdrCnt = 3;
         AdrVals[0] += 0x89;
         AdrVals[1] = Hi(AdrInt);
         AdrVals[2] = Lo(AdrInt);
         return;
      }
   }

   if ((ArgCnt <= 2) && (ArgCnt >= 1) && (MomCPU >= CPU6309) && (strcasecmp(ArgStr[ArgCnt], "W") == 0)) {
      AdrVals[0] = 0x8f + IndFlag;

   /* nur W-Register */

      if (ArgCnt == 1) {
         AdrCnt = 1;
         AdrMode = ModInd;
         return;
      }

   /* Displacement auswerten */
      ChkZero(Asc, &ZeroMode);
      AdrInt = EvalIntExpression(Asc, Int16, &OK);

   /* Displacement 0 ? */

      if ((ZeroMode == 0) && (AdrInt == 0)) {
         AdrCnt = 1;
         AdrMode = ModInd;
         return;
      }

   /* 16-Bit-Displacement */

      else {
         AdrMode = ModInd;
         AdrCnt = 3;
         AdrVals[0] += 0x20;
         AdrVals[1] = Hi(AdrInt);
         AdrVals[2] = Lo(AdrInt);
         return;
      }
   }

/* PC-relativ ? */

   if ((ArgCnt == 2) && ((strcasecmp(ArgStr[2], "PCR") == 0) || (strcasecmp(ArgStr[2], "PC") == 0))) {
      AdrVals[0] = IndFlag << 4;
      ChkZero(Asc, &ZeroMode);
      AdrInt = EvalIntExpression(Asc, Int16, &OK);
      if (OK) {
         AdrInt -= EProgCounter() + 3 + ExtFlag;

         if (ZeroMode == 3) WrError(1350);

         else if ((ZeroMode == 2) || ((ZeroMode == 0) && MayShort(AdrInt))) {
            if (!MayShort(AdrInt)) WrError(1320);
            else {
               AdrCnt = 2;
               AdrVals[0] += 0x8c;
               AdrVals[1] = Lo(AdrInt);
               AdrMode = ModInd;
            }
         }

         else {
            AdrInt--;
            AdrCnt = 3;
            AdrVals[0] += 0x8d;
            AdrVals[1] = Hi(AdrInt);
            AdrVals[2] = Lo(AdrInt);
            AdrMode = ModInd;
         }
      }
      return;
   }

   if (ArgCnt == 1) {
      ChkZero(Asc, &ZeroMode);
      FirstPassUnknown = false;
      AdrInt = EvalIntExpression(Asc, Int16, &OK);
      if ((FirstPassUnknown) && (ZeroMode == 2))
         AdrInt = (AdrInt & 0xff) | (DPRValue << 8);

      if (OK) {
         if (ZeroMode == 3) WrError(1350);

         else if ((ZeroMode == 2) || ((ZeroMode == 0) && (Hi(AdrInt) == DPRValue) && (!IndFlag))) {
            if (IndFlag) WrError(1990);
            else if (Hi(AdrInt) != DPRValue) WrError(1340);
            else {
               AdrCnt = 1;
               AdrMode = ModDir;
               AdrVals[0] = Lo(AdrInt);
            }
         }

         else {
            if (IndFlag) {
               AdrMode = ModInd;
               AdrCnt = 3;
               AdrVals[0] = 0x9f;
               AdrVals[1] = Hi(AdrInt);
               AdrVals[2] = Lo(AdrInt);
            } else {
               AdrMode = ModExt;
               AdrCnt = 2;
               AdrVals[0] = Hi(AdrInt);
               AdrVals[1] = Lo(AdrInt);
            }
         }
      }
      return;
   }

   if (AdrMode == ModNone) WrError(1350);
}

static bool CodeCPUReg(char *Asc, Byte * Erg) {
#define RegCnt 18
   static char *RegNames[RegCnt] = { "D", "X", "Y", "U", "S", "SP", "PC", "W", "V", "A", "B", "CCR", "DPR", "CC", "DP", "Z", "E", "F" };
   static Byte RegVals[RegCnt] = { 0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 10, 11, 13, 14, 15 };

   Integer z;
   String Asc_N;

   strmaxcpy(Asc_N, Asc, 255);
   NLS_UpString(Asc_N);
   Asc = Asc_N;

   for (z = 0; z < RegCnt; z++)
      if (strcmp(Asc, RegNames[z]) == 0)
         if (((RegVals[z] & 6) == 6) && (MomCPU < CPU6309)) WrError(1505);
         else {
            *Erg = RegVals[z];
            return true;
         }
   return false;
}

static bool DecodePseudo(void) {
#define ASSUME09Count 1
   static ASSUMERec ASSUME09s[ASSUME09Count] = { { "DPR", &DPRValue, 0, 0xff, 0x100 } };

   if (Memo("ASSUME")) {
      CodeASSUME(ASSUME09s, ASSUME09Count);
      return true;
   }

   return false;
}

static void SplitPM(char *s, Integer * Erg) {
   int l = strlen(s);

   if (l == 0) *Erg = 0;
   else if (s[l - 1] == '+') {
      s[l - 1] = '\0';
      *Erg = 1;
   } else if (s[l - 1] == '-') {
      s[l - 1] = '\0';
      *Erg = (-1);
   } else *Erg = 0;
}

static bool SplitBit(char *Asc, Integer * Erg) {
   char *p;
   bool OK;

   p = QuotPos(Asc, '.');
   if (p == NULL) {
      WrError(1510);
      return false;
   }
   *Erg = EvalIntExpression(p + 1, UInt3, &OK);
   if (!OK) return false;
   *p = '\0';
   return true;
}

static void MakeCode_6809(void) {
   char *p;
   Integer z, z2, z3, AdrInt;
   bool LongFlag, OK, Extent;

   CodeLen = 0;
   DontPrint = false;
   OpSize = 0;
   ExtFlag = false;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(true)) return;

/* Anweisungen ohne Argument */

   for (z = 0; z < FixedOrderCnt; z++)
      if Memo
         (FixedOrders[z].Name) {
         if (ArgCnt != 0) WrError(1110);
         else if (MomCPU < FixedOrders[z].MinCPU) WrError(1500);
         else if (Hi(FixedOrders[z].Code) == 0) {
            BAsmCode[0] = Lo(FixedOrders[z].Code);
            CodeLen = 1;
         } else {
            BAsmCode[0] = Hi(FixedOrders[z].Code);
            BAsmCode[1] = Lo(FixedOrders[z].Code);
            CodeLen = 2;
         }
         return;
         };

/* Specials... */

   if (Memo("SWI")) {
      if (ArgCnt == 0) {
         BAsmCode[0] = 0x3f;
         CodeLen = 1;
      } else if (ArgCnt != 1) WrError(1110);
      else if (strcasecmp(ArgStr[1], "2") == 0) {
         BAsmCode[0] = 0x10;
         BAsmCode[1] = 0x3f;
         CodeLen = 2;
      } else if (strcasecmp(ArgStr[1], "3") == 0) {
         BAsmCode[0] = 0x11;
         BAsmCode[1] = 0x3f;
         CodeLen = 2;
      } else WrError(1135);
      return;
   }

/* relative Spruenge */

   for (z = 0; z < RelOrderCnt; z++)
      if ((Memo(RelOrders[z].Name)) || ((*OpPart == 'L') && (strcmp(OpPart + 1, RelOrders[z].Name) == 0))) {
         if (ArgCnt != 1) WrError(1110);
         else {
            LongFlag = (*OpPart == 'L');
            ExtFlag = (LongFlag) && (Hi(RelOrders[z].Code16) != 0);
            AdrInt = EvalIntExpression(ArgStr[1], UInt16, &OK);
            if (OK) {
               AdrInt -= EProgCounter() + 2 + LongFlag + ExtFlag;
               if ((!SymbolQuestionable) && (!LongFlag) && ((AdrInt < -128) || (AdrInt > 127))) WrError(1370);
               else {
                  CodeLen = 1 + ExtFlag;
                  if (LongFlag)
                     if (ExtFlag) {
                        BAsmCode[0] = Hi(RelOrders[z].Code16);
                        BAsmCode[1] = Lo(RelOrders[z].Code16);
                     } else BAsmCode[0] = Lo(RelOrders[z].Code16);
                  else BAsmCode[0] = Lo(RelOrders[z].Code8);
                  if (LongFlag) {
                     BAsmCode[CodeLen] = Hi(AdrInt);
                     BAsmCode[CodeLen + 1] = Lo(AdrInt);
                     CodeLen += 2;
                  } else {
                     BAsmCode[CodeLen] = Lo(AdrInt);
                     CodeLen++;
                  }
               }
            }
         }
         return;
      }

/* ALU-Operationen */

   for (z = 0; z < ALUOrderCnt; z++)
      if Memo
         (ALUOrders[z].Name) {
         if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
         else if (MomCPU < ALUOrders[z].MinCPU) WrError(1500);
         else {
            OpSize = ALUOrders[z].Op16;
            ExtFlag = (Hi(ALUOrders[z].Code) != 0);
            DecodeAdr();
            if (AdrMode != ModNone)
               if ((!ALUOrders[z].MayImm) && (AdrMode == ModImm)) WrError(1350);
               else {
                  CodeLen = ExtFlag + 1 + AdrCnt;
                  if (ExtFlag) BAsmCode[0] = Hi(ALUOrders[z].Code);
                  BAsmCode[ExtFlag] = Lo(ALUOrders[z].Code) + ((AdrMode - 1) << 4);
                  memcpy(BAsmCode + 1 + ExtFlag, AdrVals, AdrCnt);
               }
         }
         return;
         }

   if (Memo("LDQ")) {
      if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
      else if (MomCPU < CPU6309) WrError(1500);
      else {
         OpSize = 2;
         DecodeAdr();
         if (AdrMode == ModImm) {
            BAsmCode[0] = 0xcd;
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            CodeLen = 1 + AdrCnt;
         } else {
            BAsmCode[0] = 0x10;
            BAsmCode[1] = 0xcc + ((AdrMode - 1) << 4);
            CodeLen = 2 + AdrCnt;
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
         }
      }
      return;
   }

/* Read-Modify-Write-Operationen */

   for (z = 0; z < RMWOrderCnt; z++)
      if Memo
         (RMWOrders[z].Name) {
         if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
         else if (MomCPU < RMWOrders[z].MinCPU) WrError(1500);
         else {
            DecodeAdr();
            if (AdrMode != ModNone)
               if (AdrMode == ModImm) WrError(1350);
               else {
                  CodeLen = 1 + AdrCnt;
                  switch (AdrMode) {
                     case ModDir:
                        BAsmCode[0] = RMWOrders[z].Code;
                        break;
                     case ModInd:
                        BAsmCode[0] = RMWOrders[z].Code + 0x60;
                        break;
                     case ModExt:
                        BAsmCode[0] = RMWOrders[z].Code + 0x70;
                        break;
                  }
                  memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               }
         }
         return;
         }

/* Anweisungen mit Flag-Operand */

   for (z = 0; z < FlagOrderCnt; z++)
      if (Memo(FlagOrders[z].Name)) {
         if (ArgCnt < 1) WrError(1110);
         else {
            OK = true;
            if (FlagOrders[z].Inv) BAsmCode[1] = 0xff;
            else BAsmCode[1] = 0x00;
            for (z2 = 1; z2 <= ArgCnt; z2++)
               if (OK) {
                  p = (strlen(ArgStr[z2]) == 1) ? strchr(FlagChars, toupper(*ArgStr[z2])) : NULL;
                  if (p != NULL) {
                     z3 = p - FlagChars;
                     if (FlagOrders[z].Inv) BAsmCode[1] &= (0xff ^ (1 << z3));
                     else BAsmCode[1] |= (1 << z3);
                  } else if (*ArgStr[z2] != '#') {
                     WrError(1120);
                     OK = false;
                  } else {
                     BAsmCode[2] = EvalIntExpression(ArgStr[z2] + 1, Int8, &OK);
                     if (OK)
                        if (FlagOrders[z].Inv) BAsmCode[1] &= BAsmCode[2];
                        else BAsmCode[1] |= BAsmCode[2];
                  }
               }
            if (OK) {
               CodeLen = 2;
               BAsmCode[0] = FlagOrders[z].Code;
            }
         }
         return;
      }

/* Bit-Befehle */

   for (z = 0; z < ImmOrderCnt; z++)
      if (Memo(ImmOrders[z].Name)) {
         if ((ArgCnt != 2) && (ArgCnt != 3)) WrError(1110);
         else if (MomCPU < ImmOrders[z].MinCPU) WrError(1500);
         else if (*ArgStr[1] != '#') WrError(1120);
         else {
            BAsmCode[1] = EvalIntExpression(ArgStr[1] + 1, Int8, &OK);
            if (OK) {
               for (z2 = 1; z2 < ArgCnt; z2++) strcopy(ArgStr[z2], ArgStr[z2 + 1]);
               ArgCnt--;
               DecodeAdr();
               if (AdrMode == ModImm) WrError(1350);
               else {
                  switch (AdrMode) {
                     case ModDir:
                        BAsmCode[0] = ImmOrders[z].Code;
                        break;
                     case ModExt:
                        BAsmCode[0] = ImmOrders[z].Code + 0x70;
                        break;
                     case ModInd:
                        BAsmCode[0] = ImmOrders[z].Code + 0x60;
                        break;
                  }
                  memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                  CodeLen = 2 + AdrCnt;
               }
            }
         }
         return;
      }

   for (z = 0; z < BitOrderCnt; z++)
      if (Memo(BitOrders[z])) {
         if (ArgCnt != 2) WrError(1110);
         else if (MomCPU < CPU6309) WrError(1500);
         else if (SplitBit(ArgStr[1], &z2))
            if (SplitBit(ArgStr[2], &z3))
               if (!CodeCPUReg(ArgStr[1], BAsmCode + 2)) WrError(1980);
               else if ((BAsmCode[2] < 8) || (BAsmCode[2] > 11)) WrError(1980);
               else {
                  strcopy(ArgStr[1], ArgStr[2]);
                  ArgCnt = 1;
                  DecodeAdr();
                  if (AdrMode != ModDir) WrError(1350);
                  else {
                     BAsmCode[2] -= 7;
                     if (BAsmCode[2] == 3) BAsmCode[2] = 0;
                     BAsmCode[0] = 0x11;
                     BAsmCode[1] = 0x30 + z;
                     BAsmCode[2] = (BAsmCode[2] << 6) + (z3 << 3) + z2;
                     BAsmCode[3] = AdrVals[0];
                     CodeLen = 4;
                  }
               }
         return;
      }

/* Register-Register-Operationen */

   if ((Memo("TFR")) || (Memo("TFM")) || (Memo("EXG"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         SplitPM(ArgStr[1], &z2);
         SplitPM(ArgStr[2], &z3);
         if ((z2 != 0) || (z3 != 0)) {
            if (Memo("EXG")) WrError(1350);
            else if (!CodeCPUReg(ArgStr[1], BAsmCode + 3)) WrError(1980);
            else if (!CodeCPUReg(ArgStr[2], BAsmCode + 2)) WrError(1980);
            else if ((BAsmCode[2] < 1) || (BAsmCode[2] > 4)) WrError(1980);
            else if ((BAsmCode[3] < 1) || (BAsmCode[3] > 4)) WrError(1980);
            else {
               BAsmCode[0] = 0x11;
               BAsmCode[1] = 0;
               BAsmCode[2] += BAsmCode[3] << 4;
               if ((z2 == 1) && (z3 == 1)) BAsmCode[1] = 0x38;
               else if ((z2 == -1) && (z3 == -1)) BAsmCode[1] = 0x39;
               else if ((z2 == 1) && (z3 == 0)) BAsmCode[1] = 0x3a;
               else if ((z2 == 0) && (z3 == 1)) BAsmCode[1] = 0x3b;
               if (BAsmCode[1] == 0) WrError(1350);
               else CodeLen = 3;
            }
         } else if (Memo("TFM")) WrError(1350);
         else if (!CodeCPUReg(ArgStr[1], BAsmCode + 2)) WrError(1980);
         else if (!CodeCPUReg(ArgStr[2], BAsmCode + 1)) WrError(1980);
         else if ((BAsmCode[1] != 13) && (BAsmCode[2] != 13) && /* Z-Register mit allen kompatibel */
            (((BAsmCode[1] ^ BAsmCode[2]) & 0x08) != 0)) WrError(1131);
         else {
            CodeLen = 2;
            BAsmCode[0] = 0x1e + Memo("TFR");
            BAsmCode[1] += BAsmCode[2] << 4;
         }
      }
      return;
   }

   for (z = 0; z < ALU2OrderCnt; z++)
      if ((strncmp(OpPart, ALU2Orders[z], strlen(ALU2Orders[z])) == 0) && ((OpPart[strlen(OpPart)] == '\0') || (OpPart[strlen(OpPart) - 1] == 'R'))) {
         if (ArgCnt != 2) WrError(1110);
         else if (!CodeCPUReg(ArgStr[1], BAsmCode + 3)) WrError(1980);
         else if (!CodeCPUReg(ArgStr[2], BAsmCode + 2)) WrError(1980);
         else if ((BAsmCode[1] != 13) && (BAsmCode[2] != 13) && /* Z-Register mit allen kompatibel */
            (((BAsmCode[2] ^ BAsmCode[3]) & 0x08) != 0)) WrError(1131);
         else {
            CodeLen = 3;
            BAsmCode[0] = 0x10;
            BAsmCode[1] = 0x30 + z;
            BAsmCode[2] += BAsmCode[3] << 4;
         }
         return;
      }

/* Berechnung effektiver Adressen */

   for (z = 0; z < LEAOrderCnt; z++)
      if Memo
         (LEAOrders[z].Name) {
         if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
         else {
            DecodeAdr();
            if (AdrMode != ModNone)
               if (AdrMode != ModInd) WrError(1350);
               else {
                  CodeLen = 1 + AdrCnt;
                  BAsmCode[0] = LEAOrders[z].Code;
                  memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               };
         };
         return;
         }

/* Push/Pull */

   for (z = 0; z < StackOrderCnt; z++)
      if Memo
         (StackOrders[z].Name) {
         BAsmCode[1] = 0;
         OK = true;
         Extent = false;
      /* S oder U einsetzen, entsprechend Opcode */
         *StackRegNames[StackRegCnt - 1] = OpPart[strlen(OpPart) - 1] ^ 'S' ^ 'U';
         for (z2 = 1; z2 <= ArgCnt; z2++)
            if (OK) {
               if (strcasecmp(ArgStr[z2], "W") == 0) {
                  if (MomCPU < CPU6309) {
                     WrError(1500);
                     OK = false;
                  } else if (ArgCnt != 1) {
                     WrError(1335);
                     OK = false;
                  } else Extent = true;
               } else {
                  for (z3 = 0; z3 < StackRegCnt; z3++)
                     if (strcasecmp(ArgStr[z2], StackRegNames[z3]) == 0) {
                        BAsmCode[1] |= (1 << StackRegCodes[z3]);
                        break;
                     }
                  if (z3 >= StackRegCnt)
                     if (strcasecmp(ArgStr[z2], "ALL") == 0) BAsmCode[1] = 0xff;
                     else if (*ArgStr[z2] != '#') OK = false;
                     else {
                        BAsmCode[2] = EvalIntExpression(ArgStr[z2] + 1, Int8, &OK);
                        if (OK) BAsmCode[1] |= BAsmCode[2];
                     }
               }
            }
         if (OK)
            if (Extent) {
               CodeLen = 2;
               BAsmCode[0] = 0x10;
               BAsmCode[1] = StackOrders[z].Code + 4;
            } else {
               CodeLen = 2;
               BAsmCode[0] = StackOrders[z].Code;
         } else WrError(1980);
         return;
         }

   if ((Memo("BITMD")) || (Memo("LDMD"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (MomCPU < CPU6309) WrError(1500);
      else if (*ArgStr[1] != '#') WrError(1120);
      else {
         BAsmCode[2] = EvalIntExpression(ArgStr[1] + 1, Int8, &OK);
         if (OK) {
            BAsmCode[0] = 0x11;
            BAsmCode[1] = 0x3c + Memo("LDMD");
            CodeLen = 3;
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static void InitCode_6809() {
   SaveInitProc();
   DPRValue = 0;
}

static bool ChkPC_6809(void) {
   return ((ActPC == SegCode) && (ProgCounter() < 0x10000));
}

static bool IsDef_6809(void) {
   return false;
}

static void SwitchFrom_6809(void) {
   DeinitFields();
}

static void SwitchTo_6809(void) {
   TurnWords = false;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "*";
   HeaderID = 0x63;
   NOPCode = 0x9d;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = (1 << SegCode);
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_6809;
   ChkPC = ChkPC_6809;
   IsDef = IsDef_6809;

   SwitchFrom = SwitchFrom_6809;
   InitFields();
}

void code6809_init(void) {
   CPU6809 = AddCPU("6809", SwitchTo_6809);
   CPU6309 = AddCPU("6309", SwitchTo_6809);

   SaveInitProc = InitPassProc;
   InitPassProc = InitCode_6809;
}
