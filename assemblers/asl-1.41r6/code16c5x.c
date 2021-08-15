/* code16c5x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS - Codegenerator fuer PIC16C5x                                          */
/*                                                                           */
/* Historie: 19.8.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>

#include "stringutil.h"
#include "chunks.h"
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
   Word Code;
   Byte DefaultDir;
} AriOrder;

#define FixedOrderCnt 5
#define LitOrderCnt 5
#define AriOrderCnt 14
#define BitOrderCnt 4
#define FOrderCnt 2

#define D_CPU16C54 0
#define D_CPU16C55 1
#define D_CPU16C56 2
#define D_CPU16C57 3

static FixedOrder *FixedOrders;
static FixedOrder *LitOrders;
static AriOrder *AriOrders;
static FixedOrder *BitOrders;
static FixedOrder *FOrders;

static CPUVar CPU16C54, CPU16C55, CPU16C56, CPU16C57;

/*-------------------------------------------------------------------------*/

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

static void AddAri(char *NName, Word NCode, Byte NDef) {
   if (InstrZ >= AriOrderCnt) exit(255);
   AriOrders[InstrZ].Name = NName;
   AriOrders[InstrZ].Code = NCode;
   AriOrders[InstrZ++].DefaultDir = NDef;
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
   AddFixed("CLRW", 0x040);
   AddFixed("NOP", 0x000);
   AddFixed("CLRWDT", 0x004);
   AddFixed("OPTION", 0x002);
   AddFixed("SLEEP", 0x003);

   LitOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * LitOrderCnt);
   InstrZ = 0;
   AddLit("ANDLW", 0xe00);
   AddLit("IORLW", 0xd00);
   AddLit("MOVLW", 0xc00);
   AddLit("RETLW", 0x800);
   AddLit("XORLW", 0xf00);

   AriOrders = (AriOrder *) malloc(sizeof(AriOrder) * AriOrderCnt);
   InstrZ = 0;
   AddAri("ADDWF", 0x1c0, 0);
   AddAri("ANDWF", 0x140, 0);
   AddAri("COMF", 0x240, 1);
   AddAri("DECF", 0x0c0, 1);
   AddAri("DECFSZ", 0x2c0, 1);
   AddAri("INCF", 0x280, 1);
   AddAri("INCFSZ", 0x3c0, 1);
   AddAri("IORWF", 0x100, 0);
   AddAri("MOVF", 0x200, 0);
   AddAri("RLF", 0x340, 1);
   AddAri("RRF", 0x300, 1);
   AddAri("SUBWF", 0x080, 0);
   AddAri("SWAPF", 0x380, 1);
   AddAri("XORWF", 0x180, 0);

   BitOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * BitOrderCnt);
   InstrZ = 0;
   AddBit("BCF", 0x400);
   AddBit("BSF", 0x500);
   AddBit("BTFSC", 0x600);
   AddBit("BTFSS", 0x700);

   FOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FOrderCnt);
   InstrZ = 0;
   AddF("CLRF", 0x060);
   AddF("MOVWF", 0x020);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(LitOrders);
   free(AriOrders);
   free(BitOrders);
   free(FOrders);
}

/*-------------------------------------------------------------------------*/

static Word ROMEnd(void) {
   switch (MomCPU - CPU16C54) {
      case D_CPU16C54:
      case D_CPU16C55:
         return 511;
      case D_CPU16C56:
         return 1023;
      case D_CPU16C57:
         return 2047;
      default:
         return 0;
   }
}

static bool DecodePseudo(void) {
   Word Size;
   bool ValOK;
   Integer z;
   TempResult t;
   char *p;
   LongInt MinV, MaxV;

   if (Memo("SFR")) {
      CodeEquate(SegData, 0, 0x1f);
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
      MaxV = (ActPC == SegCode) ? 4095 : 255;
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
            if ((Size << 1) > MaxCodeLen) WrError(1920);
            else {
               CodeLen = Size;
               memset(WAsmCode, 0, 2 * Size);
            }
      }
      return true;
   }

   return false;
}

static void MakeCode_16C5X(void) {
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
            AdrWord = EvalIntExpression(ArgStr[1], UInt5, &OK);
            if (OK) {
               ChkSpace(SegData);
               WAsmCode[0] = AriOrders[z].Code + (AdrWord & 0x1f);
               if (ArgCnt == 1) {
                  CodeLen = 1;
                  WAsmCode[0] += AriOrders[z].DefaultDir << 5;
               } else if (strcasecmp(ArgStr[2], "W") == 0) CodeLen = 1;
               else if (strcasecmp(ArgStr[2], "F") == 0) {
                  CodeLen = 1;
                  WAsmCode[0] += 0x20;
               } else {
                  AdrWord = EvalIntExpression(ArgStr[2], UInt1, &OK);
                  if (OK) {
                     CodeLen = 1;
                     WAsmCode[0] += AdrWord << 5;
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
               WAsmCode[0] = EvalIntExpression(ArgStr[1], UInt5, &OK);
               if (OK) {
                  CodeLen = 1;
                  WAsmCode[0] += BitOrders[z].Code + (AdrWord << 5);
                  ChkSpace(SegData);
               }
            }
         }
         return;
      }

   for (z = 0; z < FOrderCnt; z++)
      if (Memo(FOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrWord = EvalIntExpression(ArgStr[1], UInt5, &OK);
            if (OK) {
               CodeLen = 1;
               WAsmCode[0] = FOrders[z].Code + AdrWord;
               ChkSpace(SegData);
            }
         }
         return;
      }

   if (Memo("TRIS")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrWord = EvalIntExpression(ArgStr[1], UInt3, &OK);
         if (OK)
            if (ChkRange(AdrWord, 5, 7)) {
               CodeLen = 1;
               WAsmCode[0] = 0x000 + AdrWord;
               ChkSpace(SegData);
            }
      }
      return;
   }

   if ((Memo("CALL")) || (Memo("GOTO"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrWord = EvalIntExpression(ArgStr[1], UInt16, &OK);
         if (OK)
            if (AdrWord > ROMEnd()) WrError(1320);
            else if ((Memo("CALL")) && ((AdrWord & 0x100) != 0)) WrError(1905);
            else {
               ChkSpace(SegCode);
               if (((ProgCounter() ^ AdrWord) & 0x200) != 0)
                  WAsmCode[CodeLen++] = 0x4a3 + ((AdrWord & 0x200) >> 1); /* BCF/BSF 3,5 */
               if (((ProgCounter() ^ AdrWord) & 0x400) != 0)
                  WAsmCode[CodeLen++] = 0x4c3 + ((AdrWord & 0x400) >> 2); /* BCF/BSF 3,6 */
               if (Memo("CALL")) WAsmCode[CodeLen++] = 0x900 + (AdrWord & 0xff);
               else WAsmCode[CodeLen++] = 0xa00 + (AdrWord & 0x1ff);
            }
      }
      return;
   };

   WrXError(1200, OpPart);
}

static bool ChkPC_16C5X(void) {
   bool ok;

   switch (ActPC) {
      case SegCode:
         ok = ProgCounter() <= ROMEnd();
         break;
      case SegData:
         ok = ProgCounter() <= 0x1f;
         break;
      default:
         ok = false;
   }
   return (ok);
}

static bool IsDef_16C5X(void) {
   return Memo("SFR");
}

static void SwitchFrom_16C5X() {
   DeinitFields();
}

static void SwitchTo_16C5X(void) {
   TurnWords = false;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "*";
   HeaderID = 0x71;
   NOPCode = 0x000;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = (1 << SegCode) + (1 << SegData);
   Grans[SegCode] = 2;
   ListGrans[SegCode] = 2;
   SegInits[SegCode] = 0;
   Grans[SegData] = 1;
   ListGrans[SegData] = 1;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_16C5X;
   ChkPC = ChkPC_16C5X;
   IsDef = IsDef_16C5X;
   SwitchFrom = SwitchFrom_16C5X;
   InitFields();
}

void code16c5x_init(void) {
   CPU16C54 = AddCPU("16C54", SwitchTo_16C5X);
   CPU16C55 = AddCPU("16C55", SwitchTo_16C5X);
   CPU16C56 = AddCPU("16C56", SwitchTo_16C5X);
   CPU16C57 = AddCPU("16C57", SwitchTo_16C5X);
}
