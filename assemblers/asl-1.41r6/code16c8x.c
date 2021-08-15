/* code16c8x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegenerator PIC16C8x                                                 */
/*                                                                           */
/* Historie: 21.8.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>

#include "chunks.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

/*---------------------------------------------------------------------------*/

typedef struct {
   char *Name;
   Word Code;
} FixedOrder;

typedef struct {
   char *Name;
   Word Code;
   Byte DefaultDir;
} AriOrder;

#define D_CPU16C64 0
#define D_CPU16C84 1

#define FixedOrderCnt 7
#define LitOrderCnt 7
#define AriOrderCnt 14
#define BitOrderCnt 4
#define FOrderCnt 2

static FixedOrder *FixedOrders;
static FixedOrder *LitOrders;
static AriOrder *AriOrders;
static FixedOrder *BitOrders;
static FixedOrder *FOrders;

static CPUVar CPU16C64, CPU16C84;

/*--------------------------------------------------------------------------*/

static void AddFixed(char *NName, Word NCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddLit(char *NName, Word NCode) {
   if (InstrZ >= LitOrderCnt) exit(255);
   LitOrders[InstrZ].Name = NName;
   LitOrders[InstrZ++].Code = NCode;
}

static void AddAri(char *NName, Word NCode, Byte NDir) {
   if (InstrZ >= AriOrderCnt) exit(255);
   AriOrders[InstrZ].Name = NName;
   AriOrders[InstrZ].Code = NCode;
   AriOrders[InstrZ++].DefaultDir = NDir;
}

static void AddBit(char *NName, Word NCode) {
   if (InstrZ >= BitOrderCnt) exit(255);
   BitOrders[InstrZ].Name = NName;
   BitOrders[InstrZ++].Code = NCode;
}

static void AddF(char *NName, Word NCode) {
   if (InstrZ >= FOrderCnt) exit(255);
   FOrders[InstrZ].Name = NName;
   FOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("CLRW", 0x0100);
   AddFixed("NOP", 0x0000);
   AddFixed("CLRWDT", 0x0064);
   AddFixed("OPTION", 0x0062);
   AddFixed("SLEEP", 0x0063);
   AddFixed("RETFIE", 0x0009);
   AddFixed("RETURN", 0x0008);

   LitOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * LitOrderCnt);
   InstrZ = 0;
   AddLit("ADDLW", 0x3e00);
   AddLit("ANDLW", 0x3900);
   AddLit("IORLW", 0x3800);
   AddLit("MOVLW", 0x3000);
   AddLit("RETLW", 0x3400);
   AddLit("SUBLW", 0x3c00);
   AddLit("XORLW", 0x3a00);

   AriOrders = (AriOrder *) malloc(sizeof(AriOrder) * AriOrderCnt);
   InstrZ = 0;
   AddAri("ADDWF", 0x0700, 0);
   AddAri("ANDWF", 0x0500, 0);
   AddAri("COMF", 0x0900, 1);
   AddAri("DECF", 0x0300, 1);
   AddAri("DECFSZ", 0x0b00, 1);
   AddAri("INCF", 0x0a00, 1);
   AddAri("INCFSZ", 0x0f00, 1);
   AddAri("IORWF", 0x0400, 0);
   AddAri("MOVF", 0x0800, 0);
   AddAri("RLF", 0x0d00, 1);
   AddAri("RRF", 0x0c00, 1);
   AddAri("SUBWF", 0x0200, 0);
   AddAri("SWAPF", 0x0e00, 1);
   AddAri("XORWF", 0x0600, 0);

   BitOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * BitOrderCnt);
   InstrZ = 0;
   AddBit("BCF", 0x1000);
   AddBit("BSF", 0x1400);
   AddBit("BTFSC", 0x1800);
   AddBit("BTFSS", 0x1c00);

   FOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FOrderCnt);
   InstrZ = 0;
   AddF("CLRF", 0x0180);
   AddF("MOVWF", 0x0080);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(LitOrders);
   free(AriOrders);
   free(BitOrders);
   free(FOrders);
}

/*--------------------------------------------------------------------------*/

static Word ROMEnd(void) {
   switch (MomCPU - CPU16C64) {
      case D_CPU16C64:
         return 0x7ff;
      case D_CPU16C84:
         return 0x3ff;
      default:
         return 0;
   }
}

static Word EvalFExpression(char *Asc, bool *OK) {
   LongInt h;

   h = EvalIntExpression(Asc, UInt9, OK);
   if (*OK) {
      ChkSpace(SegData);
      return (h & 0x7f);
   } else return 0;
}

static bool DecodePseudo(void) {
   Word Size;
   bool ValOK;
   Integer z;
   char *p;
   TempResult t;
   LongInt MinV, MaxV;

   if (Memo("SFR")) {
      CodeEquate(SegData, 0, 511);
      return true;
   }

   if (Memo("RES")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         FirstPassUnknown = false;
         Size = EvalIntExpression(ArgStr[1], Int16, &ValOK);
         if (FirstPassUnknown) WrError(1820);
         if ((ValOK) && (!FirstPassUnknown)) {
            DontPrint = true;
            CodeLen = Size;
            if (MakeUseList)
               if (AddChunk(SegChunks + ActPC, ProgCounter(), CodeLen, ActPC == SegCode)) WrError(90);
         }
      }
      return true;
   }

   if (Memo("DATA")) {
      MaxV = (ActPC == SegCode) ? 16383 : 255;
      MinV = (-((MaxV + 1) >> 1));
      if (ArgCnt == 0) WrError(1110);
      else {
         ValOK = true;
         for (z = 1; z <= ArgCnt; z++)
            if (ValOK) {
               FirstPassUnknown = false;
               EvalExpression(ArgStr[z], &t);
               if ((FirstPassUnknown) && (t.Typ == TempInt)) t.Typ &= MaxV;
               switch (t.Typ) {
                  case TempInt:
                     if (ChkRange(t.Contents.Int, MinV, MaxV))
                        if (ActPC == SegCode) WAsmCode[CodeLen++] = t.Contents.Int & MaxV;
                        else BAsmCode[CodeLen++] = t.Contents.Int & MaxV;
                     break;
                  case TempFloat:
                     WrError(1135);
                     ValOK = false;
                     break;
                  case TempString:
                     for (p = t.Contents.Ascii; *p != '\0'; p++)
                        if (ActPC == SegCode)
                           WAsmCode[CodeLen++] = CharTransTable[(Byte) * p];
                        else
                           BAsmCode[CodeLen++] = CharTransTable[(Byte) * p];
                     break;
                  default:
                     ValOK = false;
               }
            }
         if (!ValOK) CodeLen = 0;
      }
      return true;
   }

   if (Memo("ZERO")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         FirstPassUnknown = false;
         Size = EvalIntExpression(ArgStr[1], Int16, &ValOK);
         if (FirstPassUnknown) WrError(1820);
         if ((ValOK) && (!FirstPassUnknown))
            if (Size << 1 > MaxCodeLen) WrError(1920);
            else {
               CodeLen = Size;
               memset(WAsmCode, 0, 2 * Size);
            }
      }
      return true;
   }

   return false;
}

static void MakeCode_16c8x(void) {
   bool OK;
   Word AdrWord;
   Integer z;

   CodeLen = 0;
   DontPrint = false;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

/* Anweisungen ohne Argument */

   for (z = 0; z < FixedOrderCnt; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else {
            CodeLen = 1;
            WAsmCode[0] = FixedOrders[z].Code;
            if (Memo("OPTION")) WrError(130);
         }
         return;
      }

/* nur ein Literal als Argument */

   for (z = 0; z < LitOrderCnt; z++)
      if (Memo(LitOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrWord = EvalIntExpression(ArgStr[1], Int8, &OK);
            if (OK) {
               CodeLen = 1;
               WAsmCode[0] = LitOrders[z].Code + (AdrWord & 0xff);
            }
         }
         return;
      }

/* W-mit-f-Operationen */

   for (z = 0; z < AriOrderCnt; z++)
      if (Memo(AriOrders[z].Name)) {
         if ((ArgCnt == 0) || (ArgCnt > 2)) WrError(1110);
         else {
            AdrWord = EvalFExpression(ArgStr[1], &OK);
            if (OK) {
               WAsmCode[0] = AriOrders[z].Code + AdrWord;
               if (ArgCnt == 1) {
                  CodeLen = 1;
                  WAsmCode[0] += AriOrders[z].DefaultDir << 7;
               } else if (strcasecmp(ArgStr[2], "W") == 0) CodeLen = 1;
               else if (strcasecmp(ArgStr[2], "F") == 0) {
                  CodeLen = 1;
                  WAsmCode[0] += 0x80;
               } else {
                  AdrWord = EvalIntExpression(ArgStr[2], UInt1, &OK);
                  if (OK) {
                     CodeLen = 1;
                     WAsmCode[0] += AdrWord << 7;
                  }
               }
            }
         }
         return;
      }

   for (z = 0; z < BitOrderCnt; z++)
      if (Memo(BitOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else {
            AdrWord = EvalIntExpression(ArgStr[2], UInt3, &OK);
            if (OK) {
               WAsmCode[0] = EvalFExpression(ArgStr[1], &OK);
               if (OK) {
                  CodeLen = 1;
                  WAsmCode[0] += BitOrders[z].Code + (AdrWord << 7);
               }
            }
         }
         return;
      }

   for (z = 0; z < FOrderCnt; z++)
      if (Memo(FOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrWord = EvalFExpression(ArgStr[1], &OK);
            if (OK) {
               CodeLen = 1;
               WAsmCode[0] = FOrders[z].Code + AdrWord;
            }
         }
         return;
      }

   if (Memo("TRIS")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         FirstPassUnknown = false;
         AdrWord = EvalIntExpression(ArgStr[1], UInt3, &OK);
         if (FirstPassUnknown) AdrWord = 5;
         if (OK)
            if (ChkRange(AdrWord, 5, 6)) {
               CodeLen = 1;
               WAsmCode[0] = 0x0060 + AdrWord;
               ChkSpace(SegData);
               WrError(130);
            }
      }
      return;
   }

   if ((Memo("CALL")) || (Memo("GOTO"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
         if (OK)
            if (AdrWord > ROMEnd()) WrError(1320);
            else {
               ChkSpace(SegCode);
               if (((ProgCounter() ^ AdrWord) & 0x800) != 0)
                  WAsmCode[CodeLen++] = 0x118a + ((AdrWord & 0x800) >> 1); /* BCF/BSF 10,3 */
               if (((ProgCounter() ^ AdrWord) & 0x1000) != 0)
                  WAsmCode[CodeLen++] = 0x120a + ((AdrWord & 0x400) >> 2); /* BCF/BSF 10,4 */
               if (Memo("CALL")) WAsmCode[CodeLen++] = 0x2000 + (AdrWord & 0x7ff);
               else WAsmCode[CodeLen++] = 0x2800 + (AdrWord & 0x7ff);
            }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_16c8x(void) {
   bool ok;

   switch (ActPC) {
      case SegCode:
         ok = ProgCounter() <= ROMEnd() + 0x300;
         break;
      case SegData:
         ok = ProgCounter() <= 0x1ff;
         break;
      default:
         ok = false;
   }
   return (ok);
}

static bool IsDef_16c8x(void) {
   return (Memo("SFR"));
}

static void SwitchFrom_16c8x(void) {
   DeinitFields();
}

static void SwitchTo_16c8x(void) {
   TurnWords = false;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "*";
   HeaderID = 0x70;
   NOPCode = 0x0000;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = (1 << SegCode) + (1 << SegData);
   Grans[SegCode] = 2;
   ListGrans[SegCode] = 2;
   SegInits[SegCode] = 0;
   Grans[SegData] = 1;
   ListGrans[SegData] = 1;
   SegInits[SegData] = 0;

   MakeCode = MakeCode_16c8x;
   ChkPC = ChkPC_16c8x;
   IsDef = IsDef_16c8x;
   SwitchFrom = SwitchFrom_16c8x;
   InitFields();
}

void code16c8x_init(void) {
   CPU16C64 = AddCPU("16C64", SwitchTo_16c8x);
   CPU16C84 = AddCPU("16C84", SwitchTo_16c8x);
}
