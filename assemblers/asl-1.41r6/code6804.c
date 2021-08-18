// AS-Portierung
// AS-Codeenerator Motorola/ST 6804
#include "stdinc.h"

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
   LongInt Code;
} BaseOrder;

#define FixedOrderCnt 19
#define RelOrderCnt 6
#define ALUOrderCnt 4

#define ModNone (-1)
#define ModInd 0
#define MModInd (1 << ModInd)
#define ModDir 1
#define MModDir (1 << ModDir)
#define ModImm 2
#define MModImm (1 << ModImm)

static ShortInt AdrMode;
static Byte AdrVal;

static CPUVar CPU6804;

static BaseOrder *FixedOrders;
static BaseOrder *RelOrders;
static BaseOrder *ALUOrders;

/*--------------------------------------------------------------------------*/

static void AddFixed(char *NName, LongInt NCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddRel(char *NName, LongInt NCode) {
   if (InstrZ >= RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name = NName;
   RelOrders[InstrZ++].Code = NCode;
}

static void AddALU(char *NName, LongInt NCode) {
   if (InstrZ >= ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name = NName;
   ALUOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   FixedOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("CLRA", 0x00fbff);
   AddFixed("CLRX", 0xb08000);
   AddFixed("CLRY", 0xb08100);
   AddFixed("COMA", 0x0000b4);
   AddFixed("ROLA", 0x0000b5);
   AddFixed("ASLA", 0x00faff);
   AddFixed("INCA", 0x00feff);
   AddFixed("INCX", 0x0000a8);
   AddFixed("INCY", 0x0000a9);
   AddFixed("DECA", 0x00ffff);
   AddFixed("DECX", 0x0000b8);
   AddFixed("DECY", 0x0000b9);
   AddFixed("TAX", 0x0000bc);
   AddFixed("TAY", 0x0000bd);
   AddFixed("TXA", 0x0000ac);
   AddFixed("TYA", 0x0000ad);
   AddFixed("RTS", 0x0000b3);
   AddFixed("RTI", 0x0000b2);
   AddFixed("NOP", 0x000020);

   RelOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * RelOrderCnt);
   InstrZ = 0;
   AddRel("BCC", 0x40);
   AddRel("BHS", 0x40);
   AddRel("BCS", 0x60);
   AddRel("BLO", 0x60);
   AddRel("BNE", 0x00);
   AddRel("BEQ", 0x20);

   ALUOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * ALUOrderCnt);
   InstrZ = 0;
   AddALU("ADD", 0x02);
   AddALU("SUB", 0x03);
   AddALU("CMP", 0x04);
   AddALU("AND", 0x05);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(RelOrders);
   free(ALUOrders);
}

/*--------------------------------------------------------------------------*/

static void ChkAdr(bool MayImm) {
   if ((AdrMode == ModImm) && (!MayImm)) {
      WrError(1350);
      AdrMode = ModNone;
   }
}

static void DecodeAdr(char *Asc, bool MayImm) {
   bool OK;

   AdrMode = ModNone;

   if (strcasecmp(Asc, "(X)") == 0) {
      AdrMode = ModInd;
      AdrVal = 0x00;
      ChkAdr(MayImm);
      return;
   }
   if (strcasecmp(Asc, "(Y)") == 0) {
      AdrMode = ModInd;
      AdrVal = 0x10;
      ChkAdr(MayImm);
      return;
   }

   if (*Asc == '#') {
      AdrVal = EvalIntExpression(Asc + 1, Int8, &OK);
      if (OK) AdrMode = ModImm;
      ChkAdr(MayImm);
      return;
   }

   AdrVal = EvalIntExpression(Asc, Int8, &OK);
   if (OK) {
      AdrMode = ModDir;
      ChkAdr(MayImm);
      return;
   }

   ChkAdr(MayImm);
}

/*--------------------------------------------------------------------------*/

static bool DecodePseudo(void) {
   if (Memo("SFR")) {
      CodeEquate(SegData, 0, 0xff);
      return true;
   }

   return false;
}

static bool IsShort(Byte Adr) {
   return ((Adr & 0xfc) == 0x80);
}

static void MakeCode_6804(void) {
   Integer z, AdrInt;
   bool OK;

   CodeLen = 0;
   DontPrint = false;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(true)) return;

/* Anweisungen ohne Argument */

   for (z = 0; z < FixedOrderCnt; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else {
            if ((FixedOrders[z].Code >> 16) != 0) CodeLen = 3;
            else CodeLen = 1 + (Hi(FixedOrders[z].Code) != 0);
            if (CodeLen == 3) BAsmCode[0] = FixedOrders[z].Code >> 16;
            if (CodeLen >= 2) BAsmCode[CodeLen - 2] = Hi(FixedOrders[z].Code);
            BAsmCode[CodeLen - 1] = Lo(FixedOrders[z].Code);
         }
         return;
      }

/* relative/absolute Spruenge */

   for (z = 0; z < RelOrderCnt; z++)
      if (Memo(RelOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrInt = EvalIntExpression(ArgStr[1], Int16, &OK) - (EProgCounter() + 1);
            if (!OK) ;
            else if ((!SymbolQuestionable) && ((AdrInt < -16) || (AdrInt > 15))) WrError(1370);
            else {
               CodeLen = 1;
               BAsmCode[0] = RelOrders[z].Code + (AdrInt & 0x1f);
               ChkSpace(SegCode);
            }
         }
         return;
      }

   if ((Memo("JSR")) || (Memo("JMP"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrInt = EvalIntExpression(ArgStr[1], UInt12, &OK);
         if (OK) {
            CodeLen = 2;
            BAsmCode[1] = Lo(AdrInt);
            BAsmCode[0] = 0x80 + (Memo("JMP") << 4) + (Hi(AdrInt) & 15);
            ChkSpace(SegCode);
         }
      }
      return;
   }

/* AKKU-Operationen */

   for (z = 0; z < ALUOrderCnt; z++)
      if (Memo(ALUOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], true);
            switch (AdrMode) {
               case ModInd:
                  CodeLen = 1;
                  BAsmCode[0] = 0xe0 + AdrVal + ALUOrders[z].Code;
                  break;
               case ModDir:
                  CodeLen = 2;
                  BAsmCode[0] = 0xf8 + ALUOrders[z].Code;
                  BAsmCode[1] = AdrVal;
                  break;
               case ModImm:
                  CodeLen = 2;
                  BAsmCode[0] = 0xe8 + ALUOrders[z].Code;
                  BAsmCode[1] = AdrVal;
                  break;
            }
         }
         return;
      }

/* Datentransfer */

   if ((Memo("LDA")) || (Memo("STA"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Memo("LDA"));
         AdrInt = Memo("STA");
         switch (AdrMode) {
            case ModInd:
               CodeLen = 1;
               BAsmCode[0] = 0xe0 + AdrInt + AdrVal;
               break;
            case ModDir:
               if (IsShort(AdrVal)) {
                  CodeLen = 1;
                  BAsmCode[0] = 0xac + (AdrInt << 4) + (AdrVal & 3);
               } else {
                  CodeLen = 2;
                  BAsmCode[0] = 0xf8 + AdrInt;
                  BAsmCode[1] = AdrVal;
               }
               break;
            case ModImm:
               CodeLen = 2;
               BAsmCode[0] = 0xe8 + AdrInt;
               BAsmCode[1] = AdrVal;
               break;
         }
      }
      return;
   }

   if ((Memo("LDXI")) || (Memo("LDYI"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (*ArgStr[1] != '#') WrError(1350);
      else {
         BAsmCode[2] = EvalIntExpression(ArgStr[1] + 1, Int8, &OK);
         if (OK) {
            CodeLen = 3;
            BAsmCode[0] = 0xb0;
            BAsmCode[1] = 0x80 + Memo("LDYI");
         }
      }
      return;
   }

   if (Memo("MVI")) {
      if (ArgCnt != 2) WrError(1110);
      else if (*ArgStr[2] != '#') WrError(1350);
      else {
         BAsmCode[1] = EvalIntExpression(ArgStr[1], Int8, &OK);
         if (OK) {
            ChkSpace(SegData);
            BAsmCode[2] = EvalIntExpression(ArgStr[2] + 1, Int8, &OK);
            if (OK) {
               BAsmCode[0] = 0xb0;
               CodeLen = 3;
            }
         }
      }
      return;
   }

/* Read/Modify/Write-Operationen */

   if ((Memo("INC")) || (Memo("DEC"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], false);
         AdrInt = Memo("DEC");
         switch (AdrMode) {
            case ModInd:
               CodeLen = 1;
               BAsmCode[0] = 0xe6 + AdrInt + AdrVal;
               break;
            case ModDir:
               if (IsShort(AdrVal)) {
                  CodeLen = 1;
                  BAsmCode[0] = 0xa8 + (AdrInt << 4) + (AdrVal & 3);
               } else {
                  CodeLen = 2;
                  BAsmCode[0] = 0xfe + AdrInt; /* ANSI :-O */
                  BAsmCode[1] = AdrVal;
               }
               break;
         }
      }
      return;
   }

/* Bitbefehle */

   if ((Memo("BSET")) || (Memo("BCLR"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         AdrVal = EvalIntExpression(ArgStr[1], UInt3, &OK);
         if (OK) {
            BAsmCode[0] = 0xd0 + (Memo("BSET") << 3) + AdrVal;
            BAsmCode[1] = EvalIntExpression(ArgStr[2], Int8, &OK);
            if (OK) {
               CodeLen = 2;
               ChkSpace(SegData);
            }
         }
      }
      return;
   }

   if ((Memo("BRSET")) || (Memo("BRCLR"))) {
      if (ArgCnt != 3) WrError(1110);
      else {
         AdrVal = EvalIntExpression(ArgStr[1], UInt3, &OK);
         if (OK) {
            BAsmCode[0] = 0xc0 + (Memo("BRSET") << 3) + AdrVal;
            BAsmCode[1] = EvalIntExpression(ArgStr[2], Int8, &OK);
            if (OK) {
               ChkSpace(SegData);
               AdrInt = EvalIntExpression(ArgStr[3], Int16, &OK) - (EProgCounter() + 3);
               if (!OK) ;
               else if ((!SymbolQuestionable) && ((AdrInt < -128) || (AdrInt > 127))) WrError(1370);
               else {
                  ChkSpace(SegCode);
                  BAsmCode[2] = AdrInt & 0xff;
                  CodeLen = 3;
               }
            }
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_6804(void) {
   switch (ActPC) {
      case SegCode:
         return ProgCounter() <= 0xfff;
      case SegData:
         return ProgCounter() <= 0xff;
      default:
         return false;
   }
}

static bool IsDef_6804(void) {
   return (Memo("SFR"));
}

static void SwitchFrom_6804(void) {
   DeinitFields();
}

static void SwitchTo_6804(void) {
   TurnWords = false;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "PC";
   HeaderID = 0x64;
   NOPCode = 0x20;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = (1 << SegCode) | (1 << SegData);
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;
   Grans[SegData] = 1;
   ListGrans[SegData] = 1;
   SegInits[SegData] = 0;

   MakeCode = MakeCode_6804;
   ChkPC = ChkPC_6804;
   IsDef = IsDef_6804;
   SwitchFrom = SwitchFrom_6804;
   InitFields();
}

void code6804_init(void) {
   CPU6804 = AddCPU("6804", SwitchTo_6804);
}
