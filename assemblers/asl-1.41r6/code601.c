// AS-Portierung
// Codegenerator PowerPC-Familie
#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "endian.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct {
   char *Name;
   LongWord Code;
   Byte CPUMask;
} BaseOrder;

#define FixedOrderCount      6
#define Reg1OrderCount       4
#define FReg1OrderCount      2
#define CReg1OrderCount      1
#define CBit1OrderCount      4
#define Reg2OrderCount       29
#define CReg2OrderCount      2
#define FReg2OrderCount      14
#define Reg2BOrderCount      2
#define Reg2SwapOrderCount   6
#define NoDestOrderCount     10
#define Reg3OrderCount       89
#define CReg3OrderCount      8
#define FReg3OrderCount      10
#define Reg3SwapOrderCount   49
#define MixedOrderCount      8
#define FReg4OrderCount      16
#define RegDispOrderCount    16
#define FRegDispOrderCount   8
#define Reg2ImmOrderCount    12
#define Imm16OrderCount      7
#define Imm16SwapOrderCount  6

static BaseOrder *FixedOrders;
static BaseOrder *Reg1Orders;
static BaseOrder *CReg1Orders;
static BaseOrder *CBit1Orders;
static BaseOrder *FReg1Orders;
static BaseOrder *Reg2Orders;
static BaseOrder *CReg2Orders;
static BaseOrder *FReg2Orders;
static BaseOrder *Reg2BOrders;
static BaseOrder *Reg2SwapOrders;
static BaseOrder *NoDestOrders;
static BaseOrder *Reg3Orders;
static BaseOrder *CReg3Orders;
static BaseOrder *FReg3Orders;
static BaseOrder *Reg3SwapOrders;
static BaseOrder *MixedOrders;
static BaseOrder *FReg4Orders;
static BaseOrder *RegDispOrders;
static BaseOrder *FRegDispOrders;
static BaseOrder *Reg2ImmOrders;
static BaseOrder *Imm16Orders;
static BaseOrder *Imm16SwapOrders;

static SimpProc SaveInitProc;
static bool BigEnd;

static CPUVar CPU403, CPU505, CPU601, CPU6000;

#if 0
void EnterByte(Byte b) {
   if (Odd(CodeLen)) {
      BAsmCode[CodeLen] = BAsmCode[CodeLen - 1];
      BAsmCode[CodeLen - 1] = b;
   } else {
      BAsmCode[CodeLen] = b;
   }
   CodeLen++;
}
#endif
/*-------------------------------------------------------------------------*/

static void AddFixed(char *NName1, char *NName2, LongInt NCode, Byte NMask) {
   if (InstrZ >= FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name = (MomCPU == CPU6000) ? NName2 : NName1;
   FixedOrders[InstrZ].Code = NCode;
   FixedOrders[InstrZ++].CPUMask = NMask;
}

static void AddReg1(char *NName1, char *NName2, LongInt NCode, Byte NMask) {
   if (InstrZ >= Reg1OrderCount) exit(255);
   Reg1Orders[InstrZ].Name = (MomCPU == CPU6000) ? NName2 : NName1;
   Reg1Orders[InstrZ].Code = NCode;
   Reg1Orders[InstrZ++].CPUMask = NMask;
}

static void AddCReg1(char *NName1, char *NName2, LongInt NCode, Byte NMask) {
   if (InstrZ >= CReg1OrderCount) exit(255);
   CReg1Orders[InstrZ].Name = (MomCPU == CPU6000) ? NName2 : NName1;
   CReg1Orders[InstrZ].Code = NCode;
   CReg1Orders[InstrZ++].CPUMask = NMask;
}

static void AddCBit1(char *NName1, char *NName2, LongInt NCode, Byte NMask) {
   if (InstrZ >= CBit1OrderCount) exit(255);
   CBit1Orders[InstrZ].Name = (MomCPU == CPU6000) ? NName2 : NName1;
   CBit1Orders[InstrZ].Code = NCode;
   CBit1Orders[InstrZ++].CPUMask = NMask;
}

static void AddFReg1(char *NName1, char *NName2, LongInt NCode, Byte NMask) {
   if (InstrZ >= FReg1OrderCount) exit(255);
   FReg1Orders[InstrZ].Name = (MomCPU == CPU6000) ? NName2 : NName1;
   FReg1Orders[InstrZ].Code = NCode;
   FReg1Orders[InstrZ++].CPUMask = NMask;
}

static void AddSReg2(char *NName, LongInt NCode, Byte NMask) {
   if (InstrZ >= Reg2OrderCount) exit(255);
   if (NName == NULL) exit(255);
   Reg2Orders[InstrZ].Name = NName;
   Reg2Orders[InstrZ].Code = NCode;
   Reg2Orders[InstrZ++].CPUMask = NMask;
}

static void AddReg2(char *NName1, char *NName2, LongInt NCode, Byte NMask, bool WithOE, bool WithFL) {
   String NName;

   strcopy(NName, (MomCPU == CPU6000) ? NName1 : NName2);
   AddSReg2(strdup(NName), NCode, NMask);
   if (WithOE) {
      strcat(NName, "O");
      AddSReg2(strdup(NName), NCode | 0x400, NMask);
      NName[strlen(NName) - 1] = '\0';
   }
   if (WithFL) {
      strcat(NName, ".");
      AddSReg2(strdup(NName), NCode | 0x001, NMask);
      NName[strlen(NName) - 1] = '\0';
      if (WithOE) {
         strcat(NName, "O.");
         AddSReg2(strdup(NName), NCode | 0x401, NMask);
      }
   }
}

static void AddCReg2(char *NName1, char *NName2, LongWord NCode, Byte NMask) {
   if (InstrZ >= CReg2OrderCount) exit(255);
   CReg2Orders[InstrZ].Name = (MomCPU == CPU6000) ? NName2 : NName1;
   CReg2Orders[InstrZ].Code = NCode;
   CReg2Orders[InstrZ++].CPUMask = NMask;
}

static void AddSFReg2(char *NName, LongInt NCode, Byte NMask) {
   if (InstrZ >= FReg2OrderCount) exit(255);
   if (NName == NULL) exit(255);
   FReg2Orders[InstrZ].Name = NName;
   FReg2Orders[InstrZ].Code = NCode;
   FReg2Orders[InstrZ++].CPUMask = NMask;
}

static void AddFReg2(char *NName1, char *NName2, LongInt NCode, Byte NMask, bool WithFL) {
   String NName;

   strcopy(NName, (MomCPU == CPU6000) ? NName1 : NName2);
   AddSFReg2(strdup(NName), NCode, NMask);
   if (WithFL) {
      strcat(NName, ".");
      AddSFReg2(strdup(NName), NCode | 0x001, NMask);
   }
}

static void AddReg2B(char *NName1, char *NName2, LongInt NCode, Byte NMask) {
   if (InstrZ >= Reg2BOrderCount) exit(255);
   Reg2BOrders[InstrZ].Name = (MomCPU == CPU6000) ? NName2 : NName1;
   Reg2BOrders[InstrZ].Code = NCode;
   Reg2BOrders[InstrZ++].CPUMask = NMask;
}

static void AddSReg2Swap(char *NName, LongInt NCode, Byte NMask) {
   if (InstrZ >= Reg2SwapOrderCount) exit(255);
   if (NName == NULL) exit(255);
   Reg2SwapOrders[InstrZ].Name = NName;
   Reg2SwapOrders[InstrZ].Code = NCode;
   Reg2SwapOrders[InstrZ++].CPUMask = NMask;
}

static void AddReg2Swap(char *NName1, char *NName2, LongInt NCode, Byte NMask, bool WithOE, bool WithFL) {
   String NName;

   strcopy(NName, (MomCPU == CPU6000) ? NName1 : NName2);
   AddSReg2Swap(strdup(NName), NCode, NMask);
   if (WithOE) {
      strcat(NName, "O");
      AddSReg2Swap(strdup(NName), NCode | 0x400, NMask);
      NName[strlen(NName) - 1] = '\0';
   }
   if (WithFL) {
      strcat(NName, ".");
      AddSReg2Swap(strdup(NName), NCode | 0x001, NMask);
      NName[strlen(NName) - 1] = '\0';
      if (WithOE) {
         strcat(NName, "O.");
         AddSReg2Swap(strdup(NName), NCode | 0x401, NMask);
      }
   }
}

static void AddNoDest(char *NName1, char *NName2, LongInt NCode, Byte NMask) {
   if (InstrZ >= NoDestOrderCount) exit(255);
   NoDestOrders[InstrZ].Name = (MomCPU == CPU6000) ? NName2 : NName1;
   NoDestOrders[InstrZ].Code = NCode;
   NoDestOrders[InstrZ++].CPUMask = NMask;
}

static void AddSReg3(char *NName, LongInt NCode, Byte NMask) {
   if (InstrZ >= Reg3OrderCount) exit(255);
   if (NName == NULL) exit(255);
   Reg3Orders[InstrZ].Name = NName;
   Reg3Orders[InstrZ].Code = NCode;
   Reg3Orders[InstrZ++].CPUMask = NMask;
}

static void AddReg3(char *NName1, char *NName2, LongInt NCode, Byte NMask, bool WithOE, bool WithFL) {
   String NName;

   strcopy(NName, (MomCPU == CPU6000) ? NName1 : NName2);
   AddSReg3(strdup(NName), NCode, NMask);
   if (WithOE) {
      strcat(NName, "O");
      AddSReg3(strdup(NName), NCode | 0x400, NMask);
      NName[strlen(NName) - 1] = '\0';
   }
   if (WithFL) {
      strcat(NName, ".");
      AddSReg3(strdup(NName), NCode | 0x001, NMask);
      NName[strlen(NName) - 1] = '\0';
      if (WithOE) {
         strcat(NName, "O.");
         AddSReg3(strdup(NName), NCode | 0x401, NMask);
      }
   }
}

static void AddCReg3(char *NName, LongWord NCode, CPUVar NMask) {
   if (InstrZ >= CReg3OrderCount) exit(255);
   CReg3Orders[InstrZ].Name = NName;
   CReg3Orders[InstrZ].Code = NCode;
   CReg3Orders[InstrZ++].CPUMask = NMask;
}

static void AddSFReg3(char *NName, LongInt NCode, Byte NMask) {
   if (InstrZ >= FReg3OrderCount) exit(255);
   if (NName == NULL) exit(255);
   FReg3Orders[InstrZ].Name = NName;
   FReg3Orders[InstrZ].Code = NCode;
   FReg3Orders[InstrZ++].CPUMask = NMask;
}

static void AddFReg3(char *NName1, char *NName2, LongInt NCode, Byte NMask, bool WithFL) {
   String NName;

   strcopy(NName, (MomCPU == CPU6000) ? NName1 : NName2);
   AddSFReg3(strdup(NName), NCode, NMask);
   if (WithFL) {
      strcat(NName, ".");
      AddSFReg3(strdup(NName), NCode | 0x001, NMask);
   }
}

static void AddSReg3Swap(char *NName, LongInt NCode, Byte NMask) {
   if (InstrZ >= Reg3SwapOrderCount) exit(255);
   if (NName == NULL) exit(255);
   Reg3SwapOrders[InstrZ].Name = NName;
   Reg3SwapOrders[InstrZ].Code = NCode;
   Reg3SwapOrders[InstrZ++].CPUMask = NMask;
}

static void AddReg3Swap(char *NName1, char *NName2, LongInt NCode, Byte NMask, bool WithFL) {
   String NName;

   strcopy(NName, (MomCPU == CPU6000) ? NName1 : NName2);
   AddSReg3Swap(strdup(NName), NCode, NMask);
   if (WithFL) {
      strcat(NName, ".");
      AddSReg3Swap(strdup(NName), NCode | 0x001, NMask);
   }
}

static void AddMixed(char *NName1, char *NName2, LongWord NCode, Byte NMask) {
   if (InstrZ >= MixedOrderCount) exit(255);
   MixedOrders[InstrZ].Name = (MomCPU == CPU6000) ? NName1 : NName2;
   MixedOrders[InstrZ].Code = NCode;
   MixedOrders[InstrZ++].CPUMask = NMask;
}

static void AddSFReg4(char *NName, LongWord NCode, Byte NMask) {
   if (InstrZ >= FReg4OrderCount) exit(255);
   if (NName == NULL) exit(255);
   FReg4Orders[InstrZ].Name = NName;
   FReg4Orders[InstrZ].Code = NCode;
   FReg4Orders[InstrZ++].CPUMask = NMask;
}

static void AddFReg4(char *NName1, char *NName2, LongWord NCode, Byte NMask, bool WithFL) {
   String NName;

   strcopy(NName, (MomCPU == CPU6000) ? NName1 : NName2);
   AddSFReg4(strdup(NName), NCode, NMask);
   if (WithFL) {
      strcat(NName, ".");
      AddSFReg4(strdup(NName), NCode | 0x001, NMask);
   }
}

static void AddRegDisp(char *NName1, char *NName2, LongWord NCode, Byte NMask) {
   if (InstrZ >= RegDispOrderCount) exit(255);
   RegDispOrders[InstrZ].Name = (MomCPU == CPU6000) ? NName2 : NName1;
   RegDispOrders[InstrZ].Code = NCode;
   RegDispOrders[InstrZ++].CPUMask = NMask;
}

static void AddFRegDisp(char *NName1, char *NName2, LongWord NCode, Byte NMask) {
   if (InstrZ >= FRegDispOrderCount) exit(255);
   FRegDispOrders[InstrZ].Name = (MomCPU == CPU6000) ? NName2 : NName1;
   FRegDispOrders[InstrZ].Code = NCode;
   FRegDispOrders[InstrZ++].CPUMask = NMask;
}

static void AddSReg2Imm(char *NName, LongWord NCode, Byte NMask) {
   if (InstrZ >= Reg2ImmOrderCount) exit(255);
   if (NName == NULL) exit(255);
   Reg2ImmOrders[InstrZ].Name = NName;
   Reg2ImmOrders[InstrZ].Code = NCode;
   Reg2ImmOrders[InstrZ++].CPUMask = NMask;
}

static void AddReg2Imm(char *NName1, char *NName2, LongWord NCode, Byte NMask, bool WithFL) {
   String NName;

   strcopy(NName, (MomCPU == CPU6000) ? NName1 : NName2);
   AddSReg2Imm(strdup(NName), NCode, NMask);
   if (WithFL) {
      strcat(NName, ".");
      AddSReg2Imm(strdup(NName), NCode | 0x001, NMask);
   }
}

static void AddImm16(char *NName1, char *NName2, LongWord NCode, Byte NMask) {
   if (InstrZ >= Imm16OrderCount) exit(255);
   Imm16Orders[InstrZ].Name = (MomCPU == CPU6000) ? NName2 : NName1;
   Imm16Orders[InstrZ].Code = NCode;
   Imm16Orders[InstrZ++].CPUMask = NMask;
}

static void AddImm16Swap(char *NName1, char *NName2, LongWord NCode, Byte NMask) {
   if (InstrZ >= Imm16SwapOrderCount) exit(255);
   Imm16SwapOrders[InstrZ].Name = (MomCPU == CPU6000) ? NName2 : NName1;
   Imm16SwapOrders[InstrZ].Code = NCode;
   Imm16SwapOrders[InstrZ++].CPUMask = NMask;
}

#   define T1  1lu
#define T3  3lu
#define T4  4lu
#define T7  7lu
#define T8  8lu
#define T9  9lu
#define T10 10lu
#define T11 11lu
#define T12 12lu
#define T13 13lu
#define T14 14lu
#define T15 15lu
#define T16 16lu
#define T17 17lu
#define T18 18lu
#define T19 19lu
#define T20 20lu
#define T21 21lu
#define T22 22lu
#define T23 23lu
#define T24 24lu
#define T25 25lu
#define T26 26lu
#define T27 27lu
#define T28 28lu
#define T29 29lu
#define T31 31lu
#define T32 32lu
#define T33 33lu
#define T34 34lu
#define T35 35lu
#define T36 36lu
#define T37 37lu
#define T38 38lu
#define T39 39lu
#define T40 40lu
#define T41 41lu
#define T42 42lu
#define T43 43lu
#define T44 44lu
#define T45 45lu
#define T46 46lu
#define T47 47lu
#define T48 48lu
#define T49 49lu
#define T50 50lu
#define T51 51lu
#define T52 52lu
#define T53 53lu
#define T54 54lu
#define T55 55lu
#define T59 59lu
#define T63 63lu

static void InitFields(void) {
/* --> 0 0 0 */

   FixedOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * FixedOrderCount);
   InstrZ = 0;
   AddFixed("EIEIO", "EIEIO", (T31 << 26) + (854 << 1), 0x0f);
   AddFixed("ISYNC", "ICS", (T19 << 26) + (150 << 1), 0x0f);
   AddFixed("RFI", "RFI", (T19 << 26) + (50 << 1), 0x0f);
   AddFixed("SC", "SVCA", (T17 << 26) + (1 << 1), 0x0f);
   AddFixed("SYNC", "DCS", (T31 << 26) + (598 << 1), 0x0f);
   AddFixed("RFCI", "RFCI", (T19 << 26) + (51 << 1), 0x01);

/* D --> D 0 0 */

   Reg1Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * Reg1OrderCount);
   InstrZ = 0;
   AddReg1("MFCR", "MFCR", (T31 << 26) + (19 << 1), 0x0f);
   AddReg1("MFMSR", "MFMSR", (T31 << 26) + (83 << 1), 0x0f);
   AddReg1("MTMSR", "MTMSR", (T31 << 26) + (146 << 1), 0x0f);
   AddReg1("WRTEE", "WRTEE", (T31 << 26) + (131 << 1), 0x0f);

/* crD --> D 0 0 */

   CReg1Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * CReg1OrderCount);
   InstrZ = 0;
   AddCReg1("MCRXR", "MCRXR", (T31 << 26) + (512 << 1), 0x0f);

/* crbD --> D 0 0 */

   CBit1Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * CBit1OrderCount);
   InstrZ = 0;
   AddCBit1("MTFSB0", "MTFSB0", (T63 << 26) + (70 << 1), 0x0c);
   AddCBit1("MTFSB0.", "MTFSB0.", (T63 << 26) + (70 << 1) + 1, 0x0c);
   AddCBit1("MTFSB1", "MTFSB1", (T63 << 26) + (38 << 1), 0x0c);
   AddCBit1("MTFSB1.", "MTFSB1.", (T63 << 26) + (38 << 1) + 1, 0x0c);

/* frD --> D 0 0 */

   FReg1Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * FReg1OrderCount);
   InstrZ = 0;
   AddFReg1("MFFS", "MFFS", (T63 << 26) + (583 << 1), 0x0c);
   AddFReg1("MFFS.", "MFFS.", (T63 << 26) + (583 << 1) + 1, 0x0c);

/* D,A --> D A 0 */

   Reg2Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * Reg2OrderCount);
   InstrZ = 0;
   AddReg2("ABS", "ABS", (T31 << 26) + (360 << 1), 0x08, true, true);
   AddReg2("ADDME", "AME", (T31 << 26) + (234 << 1), 0x0f, true, true);
   AddReg2("ADDZE", "AZE", (T31 << 26) + (202 << 1), 0x0f, true, true);
   AddReg2("CLCS", "CLCS", (T31 << 26) + (531 << 1), 0x08, false, false);
   AddReg2("NABS", "NABS", (T31 << 26) + (488 << 1), 0x08, true, true);
   AddReg2("NEG", "NEG", (T31 << 26) + (104 << 1), 0x0f, true, true);
   AddReg2("SUBFME", "SFME", (T31 << 26) + (232 << 1), 0x0f, true, true);
   AddReg2("SUBFZE", "SFZE", (T31 << 26) + (200 << 1), 0x0f, true, true);

/* cD,cS --> D S 0 */

   CReg2Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * CReg2OrderCount);
   InstrZ = 0;
   AddCReg2("MCRF", "MCRF", (T19 << 26) + (0 << 1), 0x0f);
   AddCReg2("MCRFS", "MCRFS", (T63 << 26) + (64 << 1), 0x0c);

/* fD,fB --> D 0 B */

   FReg2Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * FReg2OrderCount);
   InstrZ = 0;
   AddFReg2("FABS", "FABS", (T63 << 26) + (264 << 1), 0x0c, true);
   AddFReg2("FCTIW", "FCTIW", (T63 << 26) + (14 << 1), 0x0c, true);
   AddFReg2("FCTIWZ", "FCTIWZ", (T63 << 26) + (15 << 1), 0x0c, true);
   AddFReg2("FMR", "FMR", (T63 << 26) + (72 << 1), 0x0c, true);
   AddFReg2("FNABS", "FNABS", (T63 << 26) + (136 << 1), 0x0c, true);
   AddFReg2("FNEG", "FNEG", (T63 << 26) + (40 << 1), 0x0c, true);
   AddFReg2("FRSP", "FRSP", (T63 << 26) + (12 << 1), 0x0c, true);

/* D,B --> D 0 B */

   Reg2BOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * Reg2BOrderCount);
   InstrZ = 0;
   AddReg2B("MFSRIN", "MFSRIN", (T31 << 26) + (659 << 1), 0x0c);
   AddReg2B("MTSRIN", "MTSRI", (T31 << 26) + (242 << 1), 0x0c);

/* A,S --> S A 0 */

   Reg2SwapOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * Reg2SwapOrderCount);
   InstrZ = 0;
   AddReg2Swap("CNTLZW", "CNTLZ", (T31 << 26) + (26 << 1), 0x0f, false, true);
   AddReg2Swap("EXTSB ", "EXTSB", (T31 << 26) + (954 << 1), 0x0f, false, true);
   AddReg2Swap("EXTSH ", "EXTS", (T31 << 26) + (922 << 1), 0x0f, false, true);

/* A,B --> 0 A B */

   NoDestOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * NoDestOrderCount);
   InstrZ = 0;
   AddNoDest("DCBF", "DCBF", (T31 << 26) + (86 << 1), 0x0f);
   AddNoDest("DCBI", "DCBI", (T31 << 26) + (470 << 1), 0x0f);
   AddNoDest("DCBST", "DCBST", (T31 << 26) + (54 << 1), 0x0f);
   AddNoDest("DCBT", "DCBT", (T31 << 26) + (278 << 1), 0x0f);
   AddNoDest("DCBTST", "DCBTST", (T31 << 26) + (246 << 1), 0x0f);
   AddNoDest("DCBZ", "DCLZ", (T31 << 26) + (1014 << 1), 0x0f);
   AddNoDest("DCCCI", "DCCCI", (T31 << 26) + (454 << 1), 0x01);
   AddNoDest("ICBI", "ICBI", (T31 << 26) + (982 << 1), 0x0f);
   AddNoDest("ICBT", "ICBT", (T31 << 26) + (262 << 1), 0x01);
   AddNoDest("ICCCI", "ICCCI", (T31 << 26) + (966 << 1), 0x01);

/* D,A,B --> D A B */

   Reg3Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * Reg3OrderCount);
   InstrZ = 0;
   AddReg3("ADD", "CAX", (T31 << 26) + (266 << 1), 0x0f, true, true);
   AddReg3("ADDC", "A", (T31 << 26) + (10 << 1), 0x0f, true, true);
   AddReg3("ADDE", "AE", (T31 << 26) + (138 << 1), 0x0f, true, true);
   AddReg3("DIV", "DIV", (T31 << 26) + (331 << 1), 0x08, true, true);
   AddReg3("DIVS", "DIVS", (T31 << 26) + (363 << 1), 0x08, true, true);
   AddReg3("DIVW", "DIVW", (T31 << 26) + (491 << 1), 0x0f, true, true);
   AddReg3("DIVWU", "DIVWU", (T31 << 26) + (459 << 1), 0x0f, true, true);
   AddReg3("DOZ", "DOZ", (T31 << 26) + (264 << 1), 0x08, true, true);
   AddReg3("ECIWX", "ECIWX", (T31 << 26) + (310 << 1), 0x08, false, false);
   AddReg3("LBZUX", "LBZUX", (T31 << 26) + (119 << 1), 0x0f, false, false);
   AddReg3("LBZX", "LBZX", (T31 << 26) + (87 << 1), 0x0f, false, false);
   AddReg3("LHAUX", "LHAUX", (T31 << 26) + (375 << 1), 0x0f, false, false);
   AddReg3("LHAX", "LHAX", (T31 << 26) + (343 << 1), 0x0f, false, false);
   AddReg3("LHBRX", "LHBRX", (T31 << 26) + (790 << 1), 0x0f, false, false);
   AddReg3("LHZUX", "LHZUX", (T31 << 26) + (311 << 1), 0x0f, false, false);
   AddReg3("LHZX", "LHZX", (T31 << 26) + (279 << 1), 0x0f, false, false);
   AddReg3("LSCBX", "LSCBX", (T31 << 26) + (277 << 1), 0x08, false, true);
   AddReg3("LSWX", "LSX", (T31 << 26) + (533 << 1), 0x0f, false, false);
   AddReg3("LWARX", "LWARX", (T31 << 26) + (20 << 1), 0x0f, false, false);
   AddReg3("LWBRX", "LBRX", (T31 << 26) + (534 << 1), 0x0f, false, false);
   AddReg3("LWZUX", "LUX", (T31 << 26) + (55 << 1), 0x0f, false, false);
   AddReg3("LWZX", "LX", (T31 << 26) + (23 << 1), 0x0f, false, false);
   AddReg3("MUL", "MUL", (T31 << 26) + (107 << 1), 0x08, true, true);
   AddReg3("MULHW", "MULHW", (T31 << 26) + (75 << 1), 0x0f, false, true);
   AddReg3("MULHWU", "MULHWU", (T31 << 26) + (11 << 1), 0x0f, false, true);
   AddReg3("MULLW", "MULS", (T31 << 26) + (235 << 1), 0x0f, true, true);
   AddReg3("STBUX", "STBUX", (T31 << 26) + (247 << 1), 0x0f, false, false);
   AddReg3("STBX", "STBX", (T31 << 26) + (215 << 1), 0x0f, false, false);
   AddReg3("STHBRX", "STHBRX", (T31 << 26) + (918 << 1), 0x0f, false, false);
   AddReg3("STHUX", "STHUX", (T31 << 26) + (439 << 1), 0x0f, false, false);
   AddReg3("STHX", "STHX", (T31 << 26) + (407 << 1), 0x0f, false, false);
   AddReg3("STSWX", "STSX", (T31 << 26) + (661 << 1), 0x0f, false, false);
   AddReg3("STWBRX", "STBRX", (T31 << 26) + (662 << 1), 0x0f, false, false);
   AddReg3("STWCX.", "STWCX.", (T31 << 26) + (150 << 1), 0x0f, false, false);
   AddReg3("STWUX", "STUX", (T31 << 26) + (183 << 1), 0x0f, false, false);
   AddReg3("STWX", "STX", (T31 << 26) + (151 << 1), 0x0f, false, false);
   AddReg3("SUBF", "SUBF", (T31 << 26) + (40 << 1), 0x0f, true, true);
   AddReg3("SUB", "SUB", (T31 << 26) + (40 << 1), 0x0f, true, true);
   AddReg3("SUBFC", "SF", (T31 << 26) + (8 << 1), 0x0f, true, true);
   AddReg3("SUBC", "SUBC", (T31 << 26) + (8 << 1), 0x0f, true, true);
   AddReg3("SUBFE", "SFE", (T31 << 26) + (136 << 1), 0x0f, true, true);

/* cD,cA,cB --> D A B */

   CReg3Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * CReg3OrderCount);
   InstrZ = 0;
   AddCReg3("CRAND", (T19 << 26) + (257 << 1), 0x0f);
   AddCReg3("CRANDC", (T19 << 26) + (129 << 1), 0x0f);
   AddCReg3("CREQV", (T19 << 26) + (289 << 1), 0x0f);
   AddCReg3("CRNAND", (T19 << 26) + (225 << 1), 0x0f);
   AddCReg3("CRNOR", (T19 << 26) + (33 << 1), 0x0f);
   AddCReg3("CROR", (T19 << 26) + (449 << 1), 0x0f);
   AddCReg3("CRORC", (T19 << 26) + (417 << 1), 0x0f);
   AddCReg3("CRXOR", (T19 << 26) + (193 << 1), 0x0f);

/* fD,fA,fB --> D A B */

   FReg3Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * FReg3OrderCount);
   InstrZ = 0;
   AddFReg3("FADD", "FA", (T63 << 26) + (21 << 1), 0x0c, true);
   AddFReg3("FADDS", "FADDS", (T59 << 26) + (21 << 1), 0x0c, true);
   AddFReg3("FDIV", "FD", (T63 << 26) + (18 << 1), 0x0c, true);
   AddFReg3("FDIVS", "FDIVS", (T59 << 26) + (18 << 1), 0x0c, true);
   AddFReg3("FSUB", "FS", (T63 << 26) + (20 << 1), 0x0c, true);

/* A,S,B --> S A B */

   Reg3SwapOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * Reg3SwapOrderCount);
   InstrZ = 0;
   AddReg3Swap("AND", "AND", (T31 << 26) + (28 << 1), 0x0f, true);
   AddReg3Swap("ANDC", "ANDC", (T31 << 26) + (60 << 1), 0x0f, true);
   AddReg3Swap("ECOWX", "ECOWX", (T31 << 26) + (438 << 1), 0x0c, false);
   AddReg3Swap("EQV", "EQV", (T31 << 26) + (284 << 1), 0x0f, true);
   AddReg3Swap("MASKG", "MASKG", (T31 << 26) + (29 << 1), 0x08, true);
   AddReg3Swap("MASKIR", "MASKIR", (T31 << 26) + (541 << 1), 0x08, true);
   AddReg3Swap("NAND", "NAND", (T31 << 26) + (476 << 1), 0x0f, true);
   AddReg3Swap("NOR", "NOR", (T31 << 26) + (124 << 1), 0x0f, true);
   AddReg3Swap("OR", "OR", (T31 << 26) + (444 << 1), 0x0f, true);
   AddReg3Swap("ORC", "ORC", (T31 << 26) + (412 << 1), 0x0f, true);
   AddReg3Swap("RRIB", "RRIB", (T31 << 26) + (537 << 1), 0x08, true);
   AddReg3Swap("SLE", "SLE", (T31 << 26) + (153 << 1), 0x08, true);
   AddReg3Swap("SLEQ", "SLEQ", (T31 << 26) + (217 << 1), 0x08, true);
   AddReg3Swap("SLLQ", "SLLQ", (T31 << 26) + (216 << 1), 0x08, true);
   AddReg3Swap("SLQ", "SLQ", (T31 << 26) + (152 << 1), 0x08, true);
   AddReg3Swap("SLW", "SL", (T31 << 26) + (24 << 1), 0x0f, true);
   AddReg3Swap("SRAQ", "SRAQ", (T31 << 26) + (920 << 1), 0x08, true);
   AddReg3Swap("SRAW", "SRA", (T31 << 26) + (792 << 1), 0x0f, true);
   AddReg3Swap("SRE", "SRE", (T31 << 26) + (665 << 1), 0x08, true);
   AddReg3Swap("SREA", "SREA", (T31 << 26) + (921 << 1), 0x08, true);
   AddReg3Swap("SREQ", "SREQ", (T31 << 26) + (729 << 1), 0x08, true);
   AddReg3Swap("SRLQ", "SRLQ", (T31 << 26) + (728 << 1), 0x08, true);
   AddReg3Swap("SRQ", "SRQ", (T31 << 26) + (664 << 1), 0x08, true);
   AddReg3Swap("SRW", "SR", (T31 << 26) + (536 << 1), 0x0f, true);
   AddReg3Swap("XOR", "XOR", (T31 << 26) + (316 << 1), 0x0f, true);

/* fD,A,B --> D A B */

   MixedOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * MixedOrderCount);
   InstrZ = 0;
   AddMixed("LFDUX", "LFDUX", (T31 << 26) + (631 << 1), 0x0c);
   AddMixed("LFDX", "LFDX", (T31 << 26) + (599 << 1), 0x0c);
   AddMixed("LFSUX", "LFSUX", (T31 << 26) + (567 << 1), 0x0c);
   AddMixed("LFSX", "LFSX", (T31 << 26) + (535 << 1), 0x0c);
   AddMixed("STFDUX", "STFDUX", (T31 << 26) + (759 << 1), 0x0c);
   AddMixed("STFDX", "STFDX", (T31 << 26) + (727 << 1), 0x0c);
   AddMixed("STFSUX", "STFSUX", (T31 << 26) + (695 << 1), 0x0c);
   AddMixed("STFSX", "STFSX", (T31 << 26) + (663 << 1), 0x0c);

/* fD,fA,fC,fB --> D A B C */

   FReg4Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * FReg4OrderCount);
   InstrZ = 0;
   AddFReg4("FMADD", "FMA", (T63 << 26) + (29 << 1), 0x0c, true);
   AddFReg4("FMADDS", "FMADDS", (T59 << 26) + (29 << 1), 0x0c, true);
   AddFReg4("FMSUB", "FMS", (T63 << 26) + (28 << 1), 0x0c, true);
   AddFReg4("FMSUBS", "FMSUBS", (T59 << 26) + (28 << 1), 0x0c, true);
   AddFReg4("FNMADD", "FNMA", (T63 << 26) + (31 << 1), 0x0c, true);
   AddFReg4("FNMADDS", "FNMADDS", (T59 << 26) + (31 << 1), 0x0c, true);
   AddFReg4("FNMSUB", "FNMS", (T63 << 26) + (30 << 1), 0x0c, true);
   AddFReg4("FNMSUBS", "FNMSUBS", (T59 << 26) + (30 << 1), 0x0c, true);

/* D,d(A) --> D A d */

   RegDispOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * RegDispOrderCount);
   InstrZ = 0;
   AddRegDisp("LBZ", "LBZ", (T34 << 26), 0x0f);
   AddRegDisp("LBZU", "LBZU", (T35 << 26), 0x0f);
   AddRegDisp("LHA", "LHA", (T42 << 26), 0x0f);
   AddRegDisp("LHAU", "LHAU", (T43 << 26), 0x0f);
   AddRegDisp("LHZ", "LHZ", (T40 << 26), 0x0f);
   AddRegDisp("LHZU", "LHZU", (T41 << 26), 0x0f);
   AddRegDisp("LMW", "LM", (T46 << 26), 0x0f);
   AddRegDisp("LWZ", "L", (T32 << 26), 0x0f);
   AddRegDisp("LWZU", "LU", (T33 << 26), 0x0f);
   AddRegDisp("STB", "STB", (T38 << 26), 0x0f);
   AddRegDisp("STBU", "STBU", (T39 << 26), 0x0f);
   AddRegDisp("STH", "STH", (T44 << 26), 0x0f);
   AddRegDisp("STHU", "STHU", (T45 << 26), 0x0f);
   AddRegDisp("STMW", "STM", (T47 << 26), 0x0f);
   AddRegDisp("STW", "ST", (T36 << 26), 0x0f);
   AddRegDisp("STWU", "STU", (T37 << 26), 0x0f);

/* fD,d(A) --> D A d */

   FRegDispOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * FRegDispOrderCount);
   InstrZ = 0;
   AddFRegDisp("LFD", "LFD", (T50 << 26), 0x0c);
   AddFRegDisp("LFDU", "LFDU", (T51 << 26), 0x0c);
   AddFRegDisp("LFS", "LFS", (T48 << 26), 0x0c);
   AddFRegDisp("LFSU", "LFSU", (T49 << 26), 0x0c);
   AddFRegDisp("STFD", "STFD", (T54 << 26), 0x0c);
   AddFRegDisp("STFDU", "STFDU", (T55 << 26), 0x0c);
   AddFRegDisp("STFS", "STFS", (T52 << 26), 0x0c);
   AddFRegDisp("STFSU", "STFSU", (T53 << 26), 0x0c);

/* A,S,Imm5 --> S A Imm */

   Reg2ImmOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * Reg2ImmOrderCount);
   InstrZ = 0;
   AddReg2Imm("SLIQ", "SLIQ", (T31 << 26) + (184 << 1), 0x08, true);
   AddReg2Imm("SLLIQ", "SLLIQ", (T31 << 26) + (248 << 1), 0x08, true);
   AddReg2Imm("SRAIQ", "SRAIQ", (T31 << 26) + (952 << 1), 0x08, true);
   AddReg2Imm("SRAWI", "SRAI", (T31 << 26) + (824 << 1), 0x0f, true);
   AddReg2Imm("SRIQ", "SRIQ", (T31 << 26) + (696 << 1), 0x08, true);
   AddReg2Imm("SRLIQ", "SRLIQ", (T31 << 26) + (760 << 1), 0x08, true);

/* D,A,Imm --> D A Imm */

   Imm16Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * Imm16OrderCount);
   InstrZ = 0;
   AddImm16("ADDI", "CAL", T14 << 26, 0x0f);
   AddImm16("ADDIC", "AI", T12 << 26, 0x0f);
   AddImm16("ADDIC.", "AI.", T13 << 26, 0x0f);
   AddImm16("ADDIS", "CAU", T15 << 26, 0x0f);
   AddImm16("DOZI", "DOZI", T9 << 26, 0x08);
   AddImm16("MULLI", "MULI", T7 << 26, 0x0f);
   AddImm16("SUBFIC", "SFI", T8 << 26, 0x0c);

/* A,S,Imm --> S A Imm */

   Imm16SwapOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * Imm16SwapOrderCount);
   InstrZ = 0;
   AddImm16Swap("ANDI.", "ANDIL.", T28 << 26, 0x0f);
   AddImm16Swap("ANDIS.", "ANDIU.", T29 << 26, 0x0f);
   AddImm16Swap("ORI", "ORIL", T24 << 26, 0x0f);
   AddImm16Swap("ORIS", "ORIU", T25 << 26, 0x0f);
   AddImm16Swap("XORI", "XORIL", T26 << 26, 0x0f);
   AddImm16Swap("XORIS", "XORIU", T27 << 26, 0x0f);
}

static void DeinitNames(BaseOrder * Orders, int OrderCount) {
   int z;

   for (z = 0; z < OrderCount; free(Orders[z++].Name));
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(Reg1Orders);
   free(FReg1Orders);
   free(CReg1Orders);
   free(CBit1Orders);
   DeinitNames(Reg2Orders, Reg2OrderCount);
   free(Reg2Orders);
   free(CReg2Orders);
   DeinitNames(FReg2Orders, FReg2OrderCount);
   free(FReg2Orders);
   free(Reg2BOrders);
   DeinitNames(Reg2SwapOrders, Reg2SwapOrderCount);
   free(Reg2SwapOrders);
   free(NoDestOrders);
   DeinitNames(Reg3Orders, Reg3OrderCount);
   free(Reg3Orders);
   free(CReg3Orders);
   DeinitNames(FReg3Orders, FReg3OrderCount);
   free(FReg3Orders);
   DeinitNames(Reg3SwapOrders, Reg3SwapOrderCount);
   free(Reg3SwapOrders);
   free(MixedOrders);
   DeinitNames(FReg4Orders, FReg4OrderCount);
   free(FReg4Orders);
   free(RegDispOrders);
   free(FRegDispOrders);
   DeinitNames(Reg2ImmOrders, Reg2ImmOrderCount);
   free(Reg2ImmOrders);
   free(Imm16Orders);
   free(Imm16SwapOrders);
}

/*-------------------------------------------------------------------------*/

static void PutCode(LongWord Code) {
   memcpy(BAsmCode, &Code, 4);
   if (!BigEndian) DSwap((void *)BAsmCode, 4);
}

static void IncCode(LongWord Code) {
   BAsmCode[0] += (Code >> 24) & 0xff;
   BAsmCode[1] += (Code >> 16) & 0xff;
   BAsmCode[2] += (Code >> 8) & 0xff;
   BAsmCode[3] += (Code) & 0xff;
}

/*-------------------------------------------------------------------------*/

static bool DecodeGenReg(char *Asc, LongWord * Erg) {
   bool io;

   if ((strlen(Asc) < 2) || (toupper(*Asc) != 'R')) return false;
   else {
      *Erg = ConstLongInt(Asc + 1, &io);
      return ((io) && (*Erg <= 31));
   }
}

static bool DecodeFPReg(char *Asc, LongWord * Erg) {
   bool io;

   if ((strlen(Asc) < 3) || (toupper(*Asc) != 'F') || (toupper(Asc[1]) != 'R')) return false;
   else {
      *Erg = ConstLongInt(Asc + 2, &io);
      return ((io) && (*Erg <= 31));
   }
}

static bool DecodeCondReg(char *Asc, LongWord * Erg) {
   bool OK;

   *Erg = EvalIntExpression(Asc, UInt3, &OK) << 2;
   return ((OK) && (*Erg <= 31));
}

static bool DecodeCondBit(char *Asc, LongWord * Erg) {
   bool OK;

   *Erg = EvalIntExpression(Asc, UInt5, &OK);
   return ((OK) && (*Erg <= 31));
}

static bool DecodeRegDisp(char *Asc, LongWord * Erg) {
   char *p;
   int l = strlen(Asc);
   Integer Disp;
   bool OK;

   if (Asc[l - 1] != ')') return false;
   Asc[l - 1] = '\0';
   l--;
   p = Asc + l - 1;
   while ((p >= Asc) && (*p != '(')) p--;
   if (p < Asc) return false;
   if (!DecodeGenReg(p + 1, Erg)) return false;
   *p = '\0';
   Disp = EvalIntExpression(Asc, Int16, &OK);
   if (!OK) return false;
   *Erg = (*Erg << 16) + (Disp & 0xffff);
   return true;
}

/*-------------------------------------------------------------------------*/

static bool Convert6000(char *Name1, char *Name2) {
   if (!Memo(Name1)) return true;
   if (MomCPU == CPU6000) {
      strmaxcpy(OpPart, Name2, 255);
      return true;
   } else {
      WrError(1200);
      return false;
   }
}

static bool PMemo(char *Name) {
   String tmp;

   if (Memo(Name)) return true;

   strmaxcpy(tmp, Name, 255);
   strmaxcat(tmp, ".", 255);
   return (Memo(tmp));
}

static void IncPoint(void) {
   if (OpPart[strlen(OpPart) - 1] == '.') IncCode(1);
}

static void ChkSup(void) {
   if (!SupAllowed) WrError(50);
}

static bool ChkCPU(Byte Mask) {
   return (((Mask >> (MomCPU - CPU403)) & 1) == 1);
}

/*-------------------------------------------------------------------------*/

static bool DecodePseudo(void) {
#define ONOFF601Count 2
   static ONOFFRec ONOFF601s[ONOFF601Count] = {
      { "SUPMODE", &SupAllowed, SupAllowedName },
      { "BIGENDIAN", &BigEnd, BigEndianName }
   };

   if (CodeONOFF(ONOFF601s, ONOFF601Count)) return true;

   return false;
}

static void SwapCode(LongWord * Code) {
   *Code = ((*Code & 0x1f) << 5) | ((*Code >> 5) & 0x1f);
}

static void MakeCode_601(void) {
   Integer z, Imm;
   LongWord Dest, Src1, Src2, Src3;
   LongInt Dist;
   bool OK;

   CodeLen = 0;
   DontPrint = false;

/* Nullanweisung */

   if ((Memo("")) && (*AttrPart == '\0') && (ArgCnt == 0)) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(BigEnd)) return;

/* ohne Argument */

   for (z = 0; z < FixedOrderCount; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else if (!ChkCPU(FixedOrders[z].CPUMask)) WrXError(1500, OpPart);
         else {
            CodeLen = 4;
            PutCode(FixedOrders[z].Code);
            if (Memo("RFI")) ChkSup();
         }
         return;
      }

/* ein Register */

   for (z = 0; z < Reg1OrderCount; z++)
      if (Memo(Reg1Orders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (!ChkCPU(Reg1Orders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(Reg1Orders[z].Code + (Dest << 21));
            if (Memo("MTMSR")) ChkSup();
         }
         return;
      }

/* ein Steuerregister */

   for (z = 1; z < CReg1OrderCount; z++)
      if (Memo(CReg1Orders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (!ChkCPU(CReg1Orders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeCondReg(ArgStr[1], &Dest)) WrError(1350);
         else if ((Dest & 3) != 0) WrError(1351);
         else {
            CodeLen = 4;
            PutCode(CReg1Orders[z].Code + (Dest << 21));
         }
         return;
      }

/* ein Steuerregisterbit */

   for (z = 0; z < CBit1OrderCount; z++)
      if (Memo(CBit1Orders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (!ChkCPU(CBit1Orders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeCondBit(ArgStr[1], &Dest)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(CBit1Orders[z].Code + (Dest << 21));
         }
         return;
      }

/* ein Gleitkommaregister */

   for (z = 0; z < FReg1OrderCount; z++)
      if (Memo(FReg1Orders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (!ChkCPU(FReg1Orders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeFPReg(ArgStr[1], &Dest)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(FReg1Orders[z].Code + (Dest << 21));
         }
         return;
      }

/* 1/2 Integer-Register */

   for (z = 0; z < Reg2OrderCount; z++)
      if (Memo(Reg2Orders[z].Name)) {
         if (ArgCnt == 1) {
            ArgCnt = 2;
            strcopy(ArgStr[2], ArgStr[1]);
         }
         if (ArgCnt != 2) WrError(1110);
         else if (!ChkCPU(Reg2Orders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(Reg2Orders[z].Code + (Dest << 21) + (Src1 << 16));
         }
         return;
      }

/* 2 Bedingungs-Bits */

   for (z = 0; z < CReg2OrderCount; z++)
      if (Memo(CReg2Orders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (!ChkCPU(CReg2Orders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeCondReg(ArgStr[1], &Dest)) WrError(1350);
         else if ((Dest & 3) != 0) WrError(1351);
         else if (!DecodeCondReg(ArgStr[2], &Src1)) WrError(1350);
         else if ((Src1 & 3) != 0) WrError(1351);
         else {
            CodeLen = 4;
            PutCode(CReg2Orders[z].Code + (Dest << 21) + (Src1 << 16));
         }
         return;
      }

/* 1/2 Float-Register */

   for (z = 0; z < FReg2OrderCount; z++)
      if (Memo(FReg2Orders[z].Name)) {
         if (ArgCnt == 1) {
            ArgCnt = 2;
            strcopy(ArgStr[2], ArgStr[1]);
         }
         if (ArgCnt != 2) WrError(1110);
         else if (!ChkCPU(FReg2Orders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeFPReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeFPReg(ArgStr[2], &Src1)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(FReg2Orders[z].Code + (Dest << 21) + (Src1 << 11));
         }
         return;
      }

/* 1/2 Integer-Register, Quelle in B */

   for (z = 0; z < Reg2BOrderCount; z++)
      if (Memo(Reg2BOrders[z].Name)) {
         if (ArgCnt == 1) {
            ArgCnt = 2;
            strcopy(ArgStr[2], ArgStr[1]);
         }
         if (ArgCnt != 2) WrError(1110);
         else if (!ChkCPU(Reg2BOrders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(Reg2BOrders[z].Code + (Dest << 21) + (Src1 << 11));
            ChkSup();
         }
         return;
      }

/* 1/2 Integer-Register, getauscht */

   for (z = 0; z < Reg2SwapOrderCount; z++)
      if (Memo(Reg2SwapOrders[z].Name)) {
         if (ArgCnt == 1) {
            ArgCnt = 2;
            strcopy(ArgStr[2], ArgStr[1]);
         }
         if (ArgCnt != 2) WrError(1110);
         else if (!ChkCPU(Reg2SwapOrders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(Reg2SwapOrders[z].Code + (Dest << 16) + (Src1 << 21));
         }
         return;
      }

/* 2 Integer-Register, kein Ziel */

   for (z = 0; z < NoDestOrderCount; z++)
      if (Memo(NoDestOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (!ChkCPU(NoDestOrders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeGenReg(ArgStr[1], &Src1)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[2], &Src2)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(NoDestOrders[z].Code + (Src1 << 16) + (Src2 << 11));
         }
         return;
      }

/* 2/3 Integer-Register */

   for (z = 0; z < Reg3OrderCount; z++)
      if (Memo(Reg3Orders[z].Name)) {
         if (ArgCnt == 2) {
            ArgCnt = 3;
            strcopy(ArgStr[3], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[1]);
         }
         if (ArgCnt != 3) WrError(1110);
         else if (!ChkCPU(Reg3Orders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[3], &Src2)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(Reg3Orders[z].Code + (Dest << 21) + (Src1 << 16) + (Src2 << 11));
         }
         return;
      }

/* 2/3 Bedingungs-Bits */

   for (z = 0; z < CReg3OrderCount; z++)
      if (Memo(CReg3Orders[z].Name)) {
         if (ArgCnt == 2) {
            ArgCnt = 3;
            strcopy(ArgStr[3], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[1]);
         }
         if (ArgCnt != 3) WrError(1110);
         else if (!ChkCPU(CReg3Orders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeCondBit(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeCondBit(ArgStr[2], &Src1)) WrError(1350);
         else if (!DecodeCondBit(ArgStr[3], &Src2)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(CReg3Orders[z].Code + (Dest << 21) + (Src1 << 16) + (Src2 << 11));
         }
         return;
      }

/* 2/3 Float-Register */

   for (z = 0; z < FReg3OrderCount; z++)
      if (Memo(FReg3Orders[z].Name)) {
         if (ArgCnt == 2) {
            ArgCnt = 3;
            strcopy(ArgStr[3], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[1]);
         }
         if (ArgCnt != 3) WrError(1110);
         else if (!ChkCPU(FReg3Orders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeFPReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeFPReg(ArgStr[2], &Src1)) WrError(1350);
         else if (!DecodeFPReg(ArgStr[3], &Src2)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(FReg3Orders[z].Code + (Dest << 21) + (Src1 << 16) + (Src2 << 11));
         }
         return;
      }

/* 2/3 Integer-Register, Ziel & Quelle 1 getauscht */

   for (z = 0; z < Reg3SwapOrderCount; z++)
      if (Memo(Reg3SwapOrders[z].Name)) {
         if (ArgCnt == 2) {
            ArgCnt = 3;
            strcopy(ArgStr[3], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[1]);
         }
         if (ArgCnt != 3) WrError(1110);
         else if (!ChkCPU(Reg3SwapOrders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[3], &Src2)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(Reg3SwapOrders[z].Code + (Dest << 16) + (Src1 << 21) + (Src2 << 11));
         }
         return;
      }

/* 1 Float und 2 Integer-Register */

   for (z = 0; z < MixedOrderCount; z++)
      if (Memo(MixedOrders[z].Name)) {
         if (ArgCnt != 3) WrError(1110);
         else if (!ChkCPU(MixedOrders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeFPReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[3], &Src2)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(MixedOrders[z].Code + (Dest << 21) + (Src1 << 16) + (Src2 << 11));
         }
         return;
      }

/* 3/4 Float-Register */

   for (z = 0; z < FReg4OrderCount; z++)
      if (Memo(FReg4Orders[z].Name)) {
         if (ArgCnt == 3) {
            ArgCnt = 4;
            strcopy(ArgStr[4], ArgStr[3]);
            strcopy(ArgStr[3], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[1]);
         }
         if (ArgCnt != 4) WrError(1110);
         else if (!ChkCPU(FReg4Orders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeFPReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeFPReg(ArgStr[2], &Src1)) WrError(1350);
         else if (!DecodeFPReg(ArgStr[3], &Src3)) WrError(1350);
         else if (!DecodeFPReg(ArgStr[4], &Src2)) WrError(1350);
         else {
            CodeLen = 4;
            PutCode(FReg4Orders[z].Code + (Dest << 21) + (Src1 << 16) + (Src2 << 11) + (Src3 << 6));
         }
         return;
      }

/* Register mit indiziertem Speicheroperandem */

   for (z = 0; z < RegDispOrderCount; z++)
      if (Memo(RegDispOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeRegDisp(ArgStr[2], &Src1)) WrError(1350);
         else {
            PutCode(RegDispOrders[z].Code + (Dest << 21) + Src1);
            CodeLen = 4;
         }
         return;
      }

/* Gleitkommaregister mit indiziertem Speicheroperandem */

   for (z = 0; z < FRegDispOrderCount; z++)
      if (Memo(FRegDispOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (!DecodeFPReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeRegDisp(ArgStr[2], &Src1)) WrError(1350);
         else {
            PutCode(FRegDispOrders[z].Code + (Dest << 21) + Src1);
            CodeLen = 4;
         }
         return;
      }

/* 2 verdrehte Register mit immediate */

   for (z = 0; z < Reg2ImmOrderCount; z++)
      if (Memo(Reg2ImmOrders[z].Name)) {
         if (ArgCnt != 3) WrError(1110);
         else if (!ChkCPU(Reg2ImmOrders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
         else {
            Src2 = EvalIntExpression(ArgStr[3], UInt5, &OK);
            if (OK) {
               PutCode(Reg2ImmOrders[z].Code + (Src1 << 21) + (Dest << 16) + (Src2 << 11));
               CodeLen = 4;
            }
         }
         return;
      }

/* 2 Register+immediate */

   for (z = 0; z < Imm16OrderCount; z++)
      if (Memo(Imm16Orders[z].Name)) {
         if (ArgCnt == 2) {
            ArgCnt = 3;
            strcopy(ArgStr[3], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[1]);
         }
         if (ArgCnt != 3) WrError(1110);
         else if (!ChkCPU(Imm16Orders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
         else {
            Imm = EvalIntExpression(ArgStr[3], Int16, &OK);
            if (OK) {
               CodeLen = 4;
               PutCode(Imm16Orders[z].Code + (Dest << 21) + (Src1 << 16) + (Imm && 0xffff));
            }
         }
         return;
      }

/* 2 Register+immediate, Ziel & Quelle 1 getauscht */

   for (z = 0; z < Imm16SwapOrderCount; z++)
      if (Memo(Imm16SwapOrders[z].Name)) {
         if (ArgCnt == 2) {
            ArgCnt = 3;
            strcopy(ArgStr[3], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[1]);
         }
         if (ArgCnt != 3) WrError(1110);
         else if (!ChkCPU(Imm16SwapOrders[z].CPUMask)) WrXError(1500, OpPart);
         else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
         else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
         else {
            Imm = EvalIntExpression(ArgStr[3], Int16, &OK);
            if (OK) {
               CodeLen = 4;
               PutCode(Imm16SwapOrders[z].Code + (Dest << 16) + (Src1 << 21) + (Imm && 0xffff));
            }
         }
         return;
      }

/* Ausreisser... */

   if (!Convert6000("FM", "FMUL")) return;
   if (!Convert6000("FM.", "FMUL.")) return;

   if ((PMemo("FMUL")) || (PMemo("FMULS"))) {
      if (ArgCnt == 2) {
         strcopy(ArgStr[3], ArgStr[2]);
         strcopy(ArgStr[2], ArgStr[1]);
         ArgCnt = 3;
      }
      if (ArgCnt != 3) WrError(1110);
      else if (!DecodeFPReg(ArgStr[1], &Dest)) WrError(1350);
      else if (!DecodeFPReg(ArgStr[2], &Src1)) WrError(1350);
      else if (!DecodeFPReg(ArgStr[3], &Src2)) WrError(1350);
      else {
         PutCode((T59 << 26) + (25 << 1) + (Dest << 21) + (Src1 << 16) + (Src2 << 6));
         if (PMemo("FMUL")) IncCode(T4 << 26);
         IncPoint();
         CodeLen = 4;
      }
      return;
   }

   if (!Convert6000("LSI", "LSWI")) return;
   if (!Convert6000("STSI", "STSWI")) return;

   if ((Memo("LSWI")) || (Memo("STSWI"))) {
      if (ArgCnt != 3) WrError(1110);
      else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
      else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
      else {
         Src2 = EvalIntExpression(ArgStr[3], UInt5, &OK);
         if (OK) {
            PutCode((T31 << 26) + (597 << 1) + (Dest << 21) + (Src1 << 16) + (Src2 << 11));
            if (Memo("STSWI")) IncCode(128 << 1);
            CodeLen = 4;
         }
      }
      return;
   }

   if ((Memo("MFSPR")) || (Memo("MTSPR"))) {
      if (Memo("MTSPR")) {
         strcopy(ArgStr[3], ArgStr[1]);
         strcopy(ArgStr[1], ArgStr[2]);
         strcopy(ArgStr[2], ArgStr[3]);
      }
      if (ArgCnt != 2) WrError(1110);
      else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
      else {
         Src1 = EvalIntExpression(ArgStr[2], UInt10, &OK);
         if (OK) {
            SwapCode(&Src1);
            PutCode((T31 << 26) + (Dest << 21) + (Src1 << 11));
            IncCode((Memo("MFSPR") ? 339 : 467) << 1);
            CodeLen = 4;
         }
      }
      return;
   }

   if ((Memo("MFDCR")) || (Memo("MTDCR"))) {
      if (Memo("MTDCR")) {
         strcopy(ArgStr[3], ArgStr[1]);
         strcopy(ArgStr[1], ArgStr[2]);
         strcopy(ArgStr[2], ArgStr[3]);
      }
      if (ArgCnt != 2) WrError(1110);
      else if (MomCPU != CPU403) WrXError(1500, OpPart);
      else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
      else {
         Src1 = EvalIntExpression(ArgStr[2], UInt10, &OK);
         if (OK) {
            SwapCode(&Src1);
            PutCode((T31 << 26) + (Dest << 21) + (Src1 << 11));
            IncCode((Memo("MFDCR") ? 323 : 451) << 1);
            CodeLen = 4;
         }
      }
      return;
   }

   if ((Memo("MFSR")) || (Memo("MTSR"))) {
      if (Memo("MTSR")) {
         strcopy(ArgStr[3], ArgStr[1]);
         strcopy(ArgStr[1], ArgStr[2]);
         strcopy(ArgStr[2], ArgStr[3]);
      }
      if (ArgCnt != 2) WrError(1110);
      else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
      else {
         Src1 = EvalIntExpression(ArgStr[2], UInt4, &OK);
         if (OK) {
            PutCode((T31 << 26) + (Dest << 21) + (Src1 << 16));
            IncCode((Memo("MFSR") ? 595 : 210) << 1);
            CodeLen = 4;
            ChkSup();
         }
      }
      return;
   }

   if (Memo("MTCRF")) {
      if (ArgCnt != 2) WrError(1110);
      else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
      else {
         Dest = EvalIntExpression(ArgStr[1], UInt9, &OK);
         if (!OK) ;
         else if ((Dest & 1) == 1) WrError(1351);
         else {
            PutCode((T31 << 26) + (Src1 << 26) + (Dest << 11) + (144 << 1));
            CodeLen = 4;
         }
      }
      return;
   }

   if (PMemo("MTFSF")) {
      if (ArgCnt != 2) WrError(1110);
      else if (!DecodeFPReg(ArgStr[2], &Src1)) WrError(1350);
      else {
         Dest = EvalIntExpression(ArgStr[1], UInt8, &OK);
         if (OK) {
            PutCode((T63 << 26) + (Dest << 17) + (Src1 << 11) + (711 << 1));
            IncPoint();
            CodeLen = 4;
         }
      }
      return;
   }

   if (PMemo("MTFSFI")) {
      if (ArgCnt != 2) WrError(1110);
      else if (!DecodeCondReg(ArgStr[1], &Dest)) WrError(1350);
      else if ((Dest & 3) != 0) WrError(1351);
      else {
         Src1 = EvalIntExpression(ArgStr[2], UInt4, &OK);
         if (OK) {
            PutCode((T63 << 26) + (Dest << 21) + (Src1 << 12) + (134 << 1));
            IncPoint();
            CodeLen = 4;
         }
      }
      return;
   }

   if (PMemo("RLMI")) {
      if (ArgCnt != 5) WrError(1110);
      else if (MomCPU < CPU6000) WrXError(1500, OpPart);
      else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
      else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
      else if (!DecodeGenReg(ArgStr[3], &Src2)) WrError(1350);
      else {
         Src3 = EvalIntExpression(ArgStr[4], UInt5, &OK);
         if (OK) {
            Imm = EvalIntExpression(ArgStr[5], UInt5, &OK);
            if (OK) {
               PutCode((T22 << 26) + (Src1 << 21) + (Dest << 16)
                  + (Src2 << 11) + (Src3 << 6) + (Imm << 1));
               IncPoint();
               CodeLen = 4;
            }
         }
      }
      return;
   }

   if (!Convert6000("RLNM", "RLWNM")) return;
   if (!Convert6000("RLNM.", "RLWNM.")) return;

   if (PMemo("RLWNM")) {
      if (ArgCnt != 5) WrError(1110);
      else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
      else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
      else if (!DecodeGenReg(ArgStr[3], &Src2)) WrError(1350);
      else {
         Src3 = EvalIntExpression(ArgStr[4], UInt5, &OK);
         if (OK) {
            Imm = EvalIntExpression(ArgStr[5], UInt5, &OK);
            if (OK) {
               PutCode((T23 << 26) + (Src1 << 21) + (Dest << 16)
                  + (Src2 << 11) + (Src3 << 6) + (Imm << 1));
               IncPoint();
               CodeLen = 4;
            }
         }
      }
      return;
   }

   if (!Convert6000("RLIMI", "RLWIMI")) return;
   if (!Convert6000("RLIMI.", "RLWIMI.")) return;
   if (!Convert6000("RLINM", "RLWINM")) return;
   if (!Convert6000("RLINM.", "RLWINM.")) return;

   if ((PMemo("RLWIMI")) || (PMemo("RLWINM"))) {
      if (ArgCnt != 5) WrError(1110);
      else if (!DecodeGenReg(ArgStr[1], &Dest)) WrError(1350);
      else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
      else {
         Src2 = EvalIntExpression(ArgStr[3], UInt5, &OK);
         if (OK) {
            Src3 = EvalIntExpression(ArgStr[4], UInt5, &OK);
            if (OK) {
               Imm = EvalIntExpression(ArgStr[5], UInt5, &OK);
               if (OK) {
                  PutCode((T20 << 26) + (Dest << 16) + (Src1 << 21)
                     + (Src2 << 11) + (Src3 << 6) + (Imm << 1));
                  if (PMemo("RLWINM")) IncCode(T1 << 26);
                  IncPoint();
                  CodeLen = 4;
               }
            }
         }
      }
      return;
   }

   if (!Convert6000("TLBI", "TLBIE")) return;

   if (Memo("TLBIE")) {
      if (ArgCnt != 1) WrError(1110);
      else if (!DecodeGenReg(ArgStr[1], &Src1)) WrError(1350);
      else {
         PutCode((T31 << 26) + (Src1 << 11) + (306 << 1));
         CodeLen = 4;
         ChkSup();
      }
      return;
   }

   if (!Convert6000("T", "TW")) return;

   if (Memo("TW")) {
      if (ArgCnt != 3) WrError(1110);
      else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
      else if (!DecodeGenReg(ArgStr[3], &Src2)) WrError(1350);
      else {
         Dest = EvalIntExpression(ArgStr[1], UInt5, &OK);
         if (OK) {
            PutCode((T31 << 26) + (Dest << 21) + (Src1 << 16) + (Src2 << 11) + (4 << 1));
            CodeLen = 4;
         }
      }
      return;
   }

   if (!Convert6000("TI", "TWI")) return;

   if (Memo("TWI")) {
      if (ArgCnt != 3) WrError(1110);
      else if (!DecodeGenReg(ArgStr[2], &Src1)) WrError(1350);
      else {
         Imm = EvalIntExpression(ArgStr[3], Int16, &OK);
         if (OK) {
            Dest = EvalIntExpression(ArgStr[1], UInt5, &OK);
            if (OK) {
               PutCode((T3 << 26) + (Dest << 21) + (Src1 << 16) + (Imm & 0xffff));
               CodeLen = 4;
            }
         }
      }
      return;
   }

   if (Memo("WRTEEI")) {
      if (ArgCnt != 1) WrError(1110);
      else if (MomCPU != CPU403) WrXError(1500, OpPart);
      else {
         Src1 = EvalIntExpression(ArgStr[1], UInt1, &OK) << 15;
         if (OK) {
            PutCode((T31 << 26) + Src1 + (163 << 1));
            CodeLen = 4;
         }
      }
      return;
   }

/* Vergleiche */

   if ((Memo("CMP")) || (Memo("CMPL"))) {
      if (ArgCnt == 3) {
         strcopy(ArgStr[4], ArgStr[3]);
         strcopy(ArgStr[3], ArgStr[2]);
         strmaxcpy(ArgStr[2], "0", 255);
         ArgCnt = 4;
      }
      if (ArgCnt != 4) WrError(1110);
      else if (!DecodeGenReg(ArgStr[4], &Src2)) WrError(1350);
      else if (!DecodeGenReg(ArgStr[3], &Src1)) WrError(1350);
      else if (!DecodeCondReg(ArgStr[1], &Dest)) WrError(1350);
      else if ((Dest & 3) != 0) WrError(1351);
      else {
         Src3 = EvalIntExpression(ArgStr[2], UInt1, &OK);
         if (OK) {
            PutCode((T31 << 26) + (Dest << 21) + (Src3 << 21) + (Src1 << 16)
               + (Src2 << 11));
            if (Memo("CMPL")) IncCode(32 << 1);
            CodeLen = 4;
         }
      }
      return;
   }

   if ((Memo("FCMPO")) || (Memo("FCMPU"))) {
      if (ArgCnt != 3) WrError(1110);
      else if (!DecodeFPReg(ArgStr[3], &Src2)) WrError(1350);
      else if (!DecodeFPReg(ArgStr[2], &Src1)) WrError(1350);
      else if (!DecodeCondReg(ArgStr[1], &Dest)) WrError(1350);
      else if ((Dest & 3) != 0) WrError(1351);
      else {
         PutCode((T63 << 26) + (Dest << 21) + (Src1 << 16) + (Src2 << 11));
         if (Memo("FCMPO")) IncCode(32 << 1);
         CodeLen = 4;
      }
      return;
   }

   if ((Memo("CMPI")) || (Memo("CMPLI"))) {
      if (ArgCnt == 3) {
         strcopy(ArgStr[4], ArgStr[3]);
         strcopy(ArgStr[3], ArgStr[2]);
         strmaxcpy(ArgStr[2], "0", 255);
         ArgCnt = 4;
      }
      if (ArgCnt != 4) WrError(1110);
      else {
         Src2 = EvalIntExpression(ArgStr[4], Int16, &OK);
         if (!OK) ;
         else if (!DecodeGenReg(ArgStr[3], &Src1)) WrError(1350);
         else if (!DecodeCondReg(ArgStr[1], &Dest)) WrError(1350);
         else if ((Dest & 3) != 0) WrError(1351);
         else {
            Src3 = EvalIntExpression(ArgStr[2], UInt1, &OK);
            if (OK) {
               PutCode((T10 << 26) + (Dest << 21) + (Src3 << 21) + (Src1 << 16) + (Src2 && 0xffff));
               if (Memo("CMPI")) IncCode(T1 << 26);
               CodeLen = 4;
            }
         }
      }
      return;
   }

/* Spruenge */

   if ((Memo("B")) || (Memo("BL")) || (Memo("BA")) || (Memo("BLA"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         Dist = EvalIntExpression(ArgStr[1], Int32, &OK);
         if (OK) {
            if ((Memo("B")) || (Memo("BL"))) Dist -= EProgCounter();
            if ((!SymbolQuestionable) && (Dest > 0x1ffffff)) WrError(1320);
            else if ((!SymbolQuestionable) && (Dist < -0x2000000l)) WrError(1315);
            else if ((Dist & 3) != 0) WrError(1375);
            else {
               PutCode((T18 << 26) + (Dist & 0x03fffffc));
               if ((Memo("BA")) || (Memo("BLA"))) IncCode(2);
               if ((Memo("BL")) || (Memo("BLA"))) IncCode(1);
               CodeLen = 4;
            }
         }
      }
      return;
   }

   if ((Memo("BC")) || (Memo("BCL")) || (Memo("BCA")) || (Memo("BCLA"))) {
      if (ArgCnt != 3) WrError(1110);
      else {
         Src1 = EvalIntExpression(ArgStr[1], UInt5, &OK); /* BO */
         if (OK) {
            Src2 = EvalIntExpression(ArgStr[2], UInt5, &OK); /* BI */
            if (OK) {
               Dist = EvalIntExpression(ArgStr[3], Int32, &OK); /* ADR */
               if (OK) {
                  if ((Memo("BC")) || (Memo("BCL"))) Dist -= EProgCounter();
                  if ((!SymbolQuestionable) && (Dist > 0x7fff)) WrError(1320);
                  else if ((!SymbolQuestionable) && (Dist < -0x8000l)) WrError(1315);
                  else if ((Dist & 3) != 0) WrError(1375);
                  else {
                     PutCode((T16 << 26) + (Src1 << 21) + (Src2 << 16) + (Dist & 0xfffc));
                     if ((Memo("BCA")) || (Memo("BCLA"))) IncCode(2);
                     if ((Memo("BCL")) || (Memo("BCLA"))) IncCode(1);
                     CodeLen = 4;
                  }
               }
            }
         }
      }
      return;
   }

   if (!Convert6000("BCC", "BCCTR")) return;
   if (!Convert6000("BCCL", "BCCTRL")) return;
   if (!Convert6000("BCR", "BCLR")) return;
   if (!Convert6000("BCRL", "BCLRL")) return;

   if ((Memo("BCCTR")) || (Memo("BCCTRL")) || (Memo("BCLR")) || (Memo("BCLRL"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         Src1 = EvalIntExpression(ArgStr[1], UInt5, &OK);
         if (OK) {
            Src2 = EvalIntExpression(ArgStr[2], UInt5, &OK);
            if (OK) {
               PutCode((T19 << 26) + (Src1 << 21) + (Src2 << 16));
               if ((Memo("BCCTR")) || (Memo("BCCTRL")))
                  IncCode(528 << 1);
               else
                  IncCode(16 << 1);
               if ((Memo("BCCTRL")) || (Memo("BCLRL"))) IncCode(1);
               CodeLen = 4;
            }
         }
      }
      return;
   }

/* unbekannter Befehl */

   WrXError(1200, OpPart);
}

static bool ChkPC_601(void) {
#ifdef HAS64
   return ((ActPC == SegCode) && (ProgCounter() <= 0xffffffffll));
#else
   return (ActPC == SegCode);
#endif
}

static bool IsDef_601(void) {
   return false;
}

static void InitPass_601(void) {
   SaveInitProc();
   SetFlag(&BigEnd, BigEndianName, false);
}

static void InternSymbol_601(char *Asc, TempResult * Erg) {
   int l = strlen(Asc);

   Erg->Typ = TempNone;
   if ((l == 3) || (l == 4))
      if ((toupper(*Asc) == 'C') && (toupper(Asc[1]) == 'R'))
         if ((Asc[l - 1] >= '0') && (Asc[l - 1] <= '7'))
            if ((l == 3) != ((toupper(Asc[2]) == 'F') || (toupper(Asc[3]) == 'B'))) {
               Erg->Typ = TempInt;
               Erg->Contents.Int = Asc[l - 1] - '0';
            }
}

static void SwitchFrom_601(void) {
   DeinitFields();
}

static void SwitchTo_601(void) {
   TurnWords = false;
   ConstMode = ConstModeC;
   SetIsOccupied = false;

   PCSymbol = "*";
   HeaderID = 0x05;
   NOPCode = 0x000000000;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = (1 << SegCode);
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_601;
   ChkPC = ChkPC_601;
   IsDef = IsDef_601;
   SwitchFrom = SwitchFrom_601;
   InternSymbol = InternSymbol_601;

   InitFields();
}

void code601_init(void) {
   CPU403 = AddCPU("PPC403", SwitchTo_601);
   CPU505 = AddCPU("MPC505", SwitchTo_601);
   CPU601 = AddCPU("MPC601", SwitchTo_601);
   CPU6000 = AddCPU("RS6000", SwitchTo_601);

   SaveInitProc = InitPassProc;
   InitPassProc = InitPass_601;
}
