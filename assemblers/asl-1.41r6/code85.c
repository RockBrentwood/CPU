// AS-Portierung
// Codegenerator 8080/8085
#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

/*--------------------------------------------------------------------------------------------------*/

typedef struct {
   char *Name;
   bool May80;
   Byte Code;
} FixedOrder;

typedef struct {
   char *Name;
   Byte Code;
} BaseOrder;

#define FixedOrderCnt 27
#define Op16OrderCnt 22
#define Op8OrderCnt 10
#define ALUOrderCnt 8

static FixedOrder *FixedOrders;
static BaseOrder *Op16Orders;
static BaseOrder *Op8Orders;
static BaseOrder *ALUOrders;

static CPUVar CPU8080, CPU8085;

/*--------------------------------------------------------------------------------------------------------*/

static void AddFixed(char *NName, bool NMay, Byte NCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ].May80 = NMay;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddOp16(char *NName, Byte NCode) {
   if (InstrZ >= Op16OrderCnt) exit(255);
   Op16Orders[InstrZ].Name = NName;
   Op16Orders[InstrZ++].Code = NCode;
}

static void AddOp8(char *NName, Byte NCode) {
   if (InstrZ >= Op8OrderCnt) exit(255);
   Op8Orders[InstrZ].Name = NName;
   Op8Orders[InstrZ++].Code = NCode;
}

static void AddALU(char *NName, Byte NCode) {
   if (InstrZ >= ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name = NName;
   ALUOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("XCHG", true, 0xeb);
   AddFixed("XTHL", true, 0xe3);
   AddFixed("SPHL", true, 0xf9);
   AddFixed("PCHL", true, 0xe9);
   AddFixed("RET", true, 0xc9);
   AddFixed("RC", true, 0xd8);
   AddFixed("RNC", true, 0xd0);
   AddFixed("RZ", true, 0xc8);
   AddFixed("RNZ", true, 0xc0);
   AddFixed("RP", true, 0xf0);
   AddFixed("RM", true, 0xf8);
   AddFixed("RPE", true, 0xe8);
   AddFixed("RPO", true, 0xe0);
   AddFixed("RLC", true, 0x07);
   AddFixed("RRC", true, 0x0f);
   AddFixed("RAL", true, 0x17);
   AddFixed("RAR", true, 0x1f);
   AddFixed("CMA", true, 0x2f);
   AddFixed("STC", true, 0x37);
   AddFixed("CMC", true, 0x3f);
   AddFixed("DAA", true, 0x27);
   AddFixed("EI", true, 0xfb);
   AddFixed("DI", true, 0xf3);
   AddFixed("NOP", true, 0x00);
   AddFixed("HLT", true, 0x76);
   AddFixed("RIM", false, 0x20);
   AddFixed("SIM", false, 0x30);

   Op16Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * Op16OrderCnt);
   InstrZ = 0;
   AddOp16("STA", 0x32);
   AddOp16("LDA", 0x3a);
   AddOp16("SHLD", 0x22);
   AddOp16("LHLD", 0x2a);
   AddOp16("JMP", 0xc3);
   AddOp16("JC", 0xda);
   AddOp16("JNC", 0xd2);
   AddOp16("JZ", 0xca);
   AddOp16("JNZ", 0xc2);
   AddOp16("JP", 0xf2);
   AddOp16("JM", 0xfa);
   AddOp16("JPE", 0xea);
   AddOp16("JPO", 0xe2);
   AddOp16("CALL", 0xcd);
   AddOp16("CC", 0xdc);
   AddOp16("CNC", 0xd4);
   AddOp16("CZ", 0xcc);
   AddOp16("CNZ", 0xc4);
   AddOp16("CP", 0xf4);
   AddOp16("CM", 0xfc);
   AddOp16("CPE", 0xec);
   AddOp16("CPO", 0xe4);

   Op8Orders = (BaseOrder *) malloc(sizeof(BaseOrder) * Op8OrderCnt);
   InstrZ = 0;
   AddOp8("IN", 0xdb);
   AddOp8("OUT", 0xd3);
   AddOp8("ADI", 0xc6);
   AddOp8("ACI", 0xce);
   AddOp8("SUI", 0xd6);
   AddOp8("SBI", 0xde);
   AddOp8("ANI", 0xe6);
   AddOp8("XRI", 0xee);
   AddOp8("ORI", 0xf6);
   AddOp8("CPI", 0xfe);

   ALUOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * ALUOrderCnt);
   InstrZ = 0;
   AddALU("ADD", 0x80);
   AddALU("ADC", 0x88);
   AddALU("SUB", 0x90);
   AddALU("SBB", 0x98);
   AddALU("ANA", 0xa0);
   AddALU("XRA", 0xa8);
   AddALU("ORA", 0xb0);
   AddALU("CMP", 0xb8);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(Op16Orders);
   free(Op8Orders);
   free(ALUOrders);
}

/*--------------------------------------------------------------------------------------------------------*/

static bool DecodeReg8(char *Asc, Byte * Erg) {
   static char *RegNames = "BCDEHLMA";
   char *p;

   if (strlen(Asc) != 1) return false;
   else {
      p = strchr(RegNames, toupper(*Asc));
      if (p == 0) return false;
      else {
         *Erg = p - RegNames;
         return true;
      }
   }
}

static bool DecodeReg16(char *Asc, Byte * Erg) {
   static char *RegNames[4] = { "B", "D", "H", "SP" };

   for (*Erg = 0; (*Erg) < 4; (*Erg)++)
      if (strcasecmp(Asc, RegNames[*Erg]) == 0) break;

   return ((*Erg) < 4);
}

static bool DecodePseudo(void) {
   if (Memo("PORT")) {
      CodeEquate(SegIO, 0, 0xff);
      return true;
   }

   return false;
}

static void MakeCode_85(void) {
   bool OK;
   Word AdrWord;
   Byte AdrByte;
   Integer z;

   CodeLen = 0;
   DontPrint = false;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(false)) return;

/* Anweisungen ohne Operanden */

   for (z = 0; z < FixedOrderCnt; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else if ((MomCPU < CPU8085) && (!FixedOrders[z].May80)) WrError(1500);
         else {
            CodeLen = 1;
            BAsmCode[0] = FixedOrders[z].Code;
         }
         return;
      }

/* ein 16-Bit-Operand */

   for (z = 0; z < Op16OrderCnt; z++)
      if (Memo(Op16Orders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
            if (OK) {
               CodeLen = 3;
               BAsmCode[0] = Op16Orders[z].Code;
               BAsmCode[1] = Lo(AdrWord);
               BAsmCode[2] = Hi(AdrWord);
               ChkSpace(SegCode);
            }
         }
         return;
      }

   for (z = 0; z < Op8OrderCnt; z++)
      if (Memo(Op8Orders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrByte = EvalIntExpression(ArgStr[1], Int8, &OK);
            if (OK) {
               CodeLen = 2;
               BAsmCode[0] = Op8Orders[z].Code;
               BAsmCode[1] = AdrByte;
               if (z < 2) ChkSpace(SegIO);
            }
         }
         return;
      }

   if (Memo("MOV")) {
      if (ArgCnt != 2) WrError(1110);
      else if (!DecodeReg8(ArgStr[1], &AdrByte)) WrError(1980);
      else if (!DecodeReg8(ArgStr[2], BAsmCode + 0)) WrError(1980);
      else {
         BAsmCode[0] += 0x40 + (AdrByte << 3);
         if (BAsmCode[0] == 0x76) WrError(1760);
         else CodeLen = 1;
      }
      return;
   }

   if (Memo("MVI")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         BAsmCode[1] = EvalIntExpression(ArgStr[2], Int8, &OK);
         if (!OK) ;
         else if (!DecodeReg8(ArgStr[1], &AdrByte)) WrError(1980);
         else {
            BAsmCode[0] = 0x06 + (AdrByte << 3);
            CodeLen = 2;
         }
      }
      return;
   }

   if (Memo("LXI")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         AdrWord = EvalIntExpression(ArgStr[2], Int16, &OK);
         if (!OK) ;
         else if (!DecodeReg16(ArgStr[1], &AdrByte)) WrError(1980);
         else {
            BAsmCode[0] = 0x01 + (AdrByte << 4);
            BAsmCode[1] = Lo(AdrWord);
            BAsmCode[2] = Hi(AdrWord);
            CodeLen = 3;
         }
      }
      return;
   }

   if ((Memo("LDAX")) || (Memo("STAX"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (!DecodeReg16(ArgStr[1], &AdrByte)) WrError(1980);
      else switch (AdrByte) {
            case 3: /* SP */
               WrError(1135);
               break;
            case 2: /* H --> MOV A,M oder M,A */
               CodeLen = 1;
               BAsmCode[0] = (Memo("LDAX")) ? 0x7e : 0x77;
               break;
            default:
               CodeLen = 1;
               BAsmCode[0] = 0x02 + (AdrByte << 4);
               if (Memo("LDAX")) BAsmCode[0] += 8;
               break;
      }
      return;
   }

   if ((Memo("PUSH")) || (Memo("POP"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         if (strcasecmp(ArgStr[1], "PSW") == 0) strmaxcpy(ArgStr[1], "SP", 255);
         if (!DecodeReg16(ArgStr[1], &AdrByte)) WrError(1980);
         else {
            CodeLen = 1;
            BAsmCode[0] = 0xc1 + (AdrByte << 4);
            if (Memo("PUSH")) BAsmCode[0] += 4;
         }
      }
      return;
   }

   if (Memo("RST")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrByte = EvalIntExpression(ArgStr[1], UInt3, &OK);
         if (OK) {
            CodeLen = 1;
            BAsmCode[0] = 0xc7 + (AdrByte << 3);
         }
      }
      return;
   }

   if ((Memo("INR")) || (Memo("DCR"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (!DecodeReg8(ArgStr[1], &AdrByte)) WrError(1980);
      else {
         CodeLen = 1;
         BAsmCode[0] = 0x04 + (AdrByte << 3);
         if (Memo("DCR")) BAsmCode[0]++;
      }
      return;
   }

   if ((Memo("INX")) || (Memo("DCX"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (!DecodeReg16(ArgStr[1], &AdrByte)) WrError(1980);
      else {
         CodeLen = 1;
         BAsmCode[0] = 0x03 + (AdrByte << 4);
         if (Memo("DCX")) BAsmCode[0] += 8;
      }
      return;
   }

   if (Memo("DAD")) {
      if (ArgCnt != 1) WrError(1110);
      else if (!DecodeReg16(ArgStr[1], &AdrByte)) WrError(1980);
      else {
         CodeLen = 1;
         BAsmCode[0] = 0x09 + (AdrByte << 4);
      }
      return;
   }

   for (z = 0; z < ALUOrderCnt; z++)
      if (Memo(ALUOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (!DecodeReg8(ArgStr[1], &AdrByte)) WrError(1980);
         else {
            CodeLen = 1;
            BAsmCode[0] = ALUOrders[z].Code + AdrByte;
         }
         return;
      }

   WrXError(1200, OpPart);
}

static bool ChkPC_85(void) {
   switch (ActPC) {
      case SegCode:
         return (ProgCounter() < 0x10000);
      case SegIO:
         return (ProgCounter() < 0x100);
      default:
         return false;
   }
}

static bool IsDef_85(void) {
   return (Memo("PORT"));
}

static void SwitchFrom_85(void) {
   DeinitFields();
}

static void SwitchTo_85(void) {
   TurnWords = false;
   ConstMode = ConstModeIntel;
   SetIsOccupied = false;

   PCSymbol = "$";
   HeaderID = 0x41;
   NOPCode = 0x00;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = (1 << SegCode) | (1 << SegIO);
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;
   Grans[SegIO] = 1;
   ListGrans[SegIO] = 1;
   SegInits[SegIO] = 0;

   MakeCode = MakeCode_85;
   ChkPC = ChkPC_85;
   IsDef = IsDef_85;
   SwitchFrom = SwitchFrom_85;
   InitFields();
}

void code85_init(void) {
   CPU8080 = AddCPU("8080", SwitchTo_85);
   CPU8085 = AddCPU("8085", SwitchTo_85);
}
