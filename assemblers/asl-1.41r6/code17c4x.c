/* code17c4x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator PIC17C4x                                                    */
/*                                                                           */
/* Historie: 21.8.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "stringutil.h"
#include "chunks.h"
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
   Word DefaultDir;
   Word Code;
} AriOrder;

#define FixedOrderCnt 5
#define LittOrderCnt 8
#define AriOrderCnt 23
#define BitOrderCnt 5
#define FOrderCnt 5

/*---------------------------------------------------------------------------*/

static FixedOrder *FixedOrders;
static FixedOrder *LittOrders;
static AriOrder *AriOrders;
static FixedOrder *BitOrders;
static FixedOrder *FOrders;

static CPUVar CPU17C42;

/*---------------------------------------------------------------------------*/

static void AddFixed(char *NName, Word NCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddLitt(char *NName, Word NCode) {
   if (InstrZ >= LittOrderCnt) exit(255);
   LittOrders[InstrZ].Name = NName;
   LittOrders[InstrZ++].Code = NCode;
}

static void AddAri(char *NName, Word NDef, Word NCode) {
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
   AddFixed("RETFIE", 0x0005);
   AddFixed("RETURN", 0x0002);
   AddFixed("CLRWDT", 0x0004);
   AddFixed("NOP", 0x0000);
   AddFixed("SLEEP", 0x0003);

   LittOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * LittOrderCnt);
   InstrZ = 0;
   AddLitt("MOVLB", 0xb800);
   AddLitt("ADDLW", 0xb100);
   AddLitt("ANDLW", 0xb500);
   AddLitt("IORLW", 0xb300);
   AddLitt("MOVLW", 0xb000);
   AddLitt("SUBLW", 0xb200);
   AddLitt("XORLW", 0xb400);
   AddLitt("RETLW", 0xb600);

   AriOrders = (AriOrder *) malloc(sizeof(AriOrder) * AriOrderCnt);
   InstrZ = 0;
   AddAri("ADDWF", 0, 0x0e00);
   AddAri("ADDWFC", 0, 0x1000);
   AddAri("ANDWF", 0, 0x0a00);
   AddAri("CLRF", 1, 0x2800);
   AddAri("COMF", 1, 0x1200);
   AddAri("DAW", 1, 0x2e00);
   AddAri("DECF", 1, 0x0600);
   AddAri("INCF", 1, 0x1400);
   AddAri("IORWF", 0, 0x0800);
   AddAri("NEGW", 1, 0x2c00);
   AddAri("RLCF", 1, 0x1a00);
   AddAri("RLNCF", 1, 0x2200);
   AddAri("RRCF", 1, 0x1800);
   AddAri("RRNCF", 1, 0x2000);
   AddAri("SETF", 1, 0x2a00);
   AddAri("SUBWF", 0, 0x0400);
   AddAri("SUBWFB", 0, 0x0200);
   AddAri("SWAPF", 1, 0x1c00);
   AddAri("XORWF", 0, 0x0c00);
   AddAri("DECFSZ", 1, 0x1600);
   AddAri("DCFSNZ", 1, 0x2600);
   AddAri("INCFSZ", 1, 0x1e00);
   AddAri("INFSNZ", 1, 0x2400);

   BitOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * BitOrderCnt);
   InstrZ = 0;
   AddBit("BCF", 0x8800);
   AddBit("BSF", 0x8000);
   AddBit("BTFSC", 0x9800);
   AddBit("BTFSS", 0x9000);
   AddBit("BTG", 0x3800);

   FOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FOrderCnt);
   InstrZ = 0;
   AddF("MOVWF", 0x0100);
   AddF("CPFSEQ", 0x3100);
   AddF("CPFSGT", 0x3200);
   AddF("CPFSLT", 0x3000);
   AddF("TSTFSZ", 0x3300);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(LittOrders);
   free(AriOrders);
   free(BitOrders);
   free(FOrders);
}

/*---------------------------------------------------------------------------*/

static bool DecodePseudo(void) {
   Word Size;
   bool ValOK;
   Integer z, z2;
   TempResult t;
   LongInt MinV, MaxV;

   if (Memo("SFR")) {
      CodeEquate(SegData, 0, 0xff);
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
      MaxV = (ActPC == SegCode) ? 65535 : 255;
      MinV = (-((MaxV + 1) >> 1));
      if (ArgCnt == 0) WrError(1110);
      else {
         ValOK = true;
         for (z = 1; z <= ArgCnt; z++)
            if (ValOK) {
               FirstPassUnknown = false;
               EvalExpression(ArgStr[z], &t);
               if ((FirstPassUnknown) && (t.Typ == TempInt)) t.Contents.Int &= MaxV;
               switch (t.Typ) {
                  case TempInt:
                     if (ChkRange(t.Contents.Int, MinV, MaxV))
                        if (ActPC == SegCode) WAsmCode[CodeLen++] = t.Contents.Int;
                        else BAsmCode[CodeLen++] = t.Contents.Int;
                     break;
                  case TempFloat:
                     WrError(1135);
                     ValOK = false;
                     break;
                  case TempString:
                     for (z2 = 0; z2 < strlen(t.Contents.Ascii); z2++) {
                        Size = CharTransTable[(Byte) t.Contents.Ascii[z2]];
                        if (ActPC == SegData) BAsmCode[CodeLen++] = Size;
                        else if ((z2 & 1) == 0) WAsmCode[CodeLen++] = Size;
                        else WAsmCode[CodeLen - 1] += Size << 8;
                     }
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

static void MakeCode_17c4x(void) {
   bool OK;
   Word AdrWord;
   Integer z;

   CodeLen = 0;
   DontPrint = false;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

/* kein Argument */

   for (z = 0; z < FixedOrderCnt; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else {
            CodeLen = 1;
            WAsmCode[0] = FixedOrders[z].Code;
         }
         return;
      }

/* konstantes Argument */

   for (z = 0; z < LittOrderCnt; z++)
      if (Memo(LittOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrWord = EvalIntExpression(ArgStr[1], Int8, &OK);
            if (OK) {
               WAsmCode[0] = LittOrders[z].Code + (AdrWord & 0xff);
               CodeLen = 1;
            }
         }
         return;
      }

/* W-mit-f-Operationen */

   for (z = 0; z < AriOrderCnt; z++)
      if (Memo(AriOrders[z].Name)) {
         if ((ArgCnt == 0) || (ArgCnt > 2)) WrError(1110);
         else {
            AdrWord = EvalIntExpression(ArgStr[1], Int8, &OK);
            if (OK) {
               ChkSpace(SegData);
               WAsmCode[0] = AriOrders[z].Code + (AdrWord & 0xff);
               if (ArgCnt == 1) {
                  CodeLen = 1;
                  WAsmCode[0] += AriOrders[z].DefaultDir << 8;
               } else if (strcasecmp(ArgStr[2], "W") == 0) CodeLen = 1;
               else if (strcasecmp(ArgStr[2], "F") == 0) {
                  CodeLen = 1;
                  WAsmCode[0] += 0x100;
               } else {
                  AdrWord = EvalIntExpression(ArgStr[2], UInt1, &OK);
                  if (OK) {
                     CodeLen = 1;
                     WAsmCode[0] += (AdrWord << 8);
                  }
               }
            }
         }
         return;
      }

/* Bitoperationen */

   for (z = 0; z < BitOrderCnt; z++)
      if (Memo(BitOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else {
            AdrWord = EvalIntExpression(ArgStr[2], UInt3, &OK);
            if (OK) {
               WAsmCode[0] = EvalIntExpression(ArgStr[1], Int8, &OK);
               if (OK) {
                  CodeLen = 1;
                  WAsmCode[0] += BitOrders[z].Code + (AdrWord << 8);
                  ChkSpace(SegData);
               }
            }
         }
         return;
      }

/* Register als Operand */

   for (z = 0; z < FOrderCnt; z++)
      if (Memo(FOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrWord = EvalIntExpression(ArgStr[1], Int8, &OK);
            if (OK) {
               CodeLen = 1;
               WAsmCode[0] = FOrders[z].Code + AdrWord;
               ChkSpace(SegData);
            }
         }
         return;
      }

   if ((Memo("MOVFP")) || (Memo("MOVPF"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         if (Memo("MOVFP")) {
            strcpy(ArgStr[3], ArgStr[1]);
            strcpy(ArgStr[1], ArgStr[2]);
            strcpy(ArgStr[2], ArgStr[3]);
         }
         AdrWord = EvalIntExpression(ArgStr[1], UInt5, &OK);
         if (OK) {
            WAsmCode[0] = EvalIntExpression(ArgStr[2], Int8, &OK);
            if (OK) {
               WAsmCode[0] = Lo(WAsmCode[0]) + (AdrWord << 8) + 0x4000;
               if (Memo("MOVFP")) WAsmCode[0] += 0x2000;
               CodeLen = 1;
            }
         }
      }
      return;
   }

   if ((Memo("TABLRD")) || (Memo("TABLWT"))) {
      if (ArgCnt != 3) WrError(1110);
      else {
         WAsmCode[0] = Lo(EvalIntExpression(ArgStr[3], Int8, &OK));
         if (OK) {
            AdrWord = EvalIntExpression(ArgStr[2], UInt1, &OK);
            if (OK) {
               WAsmCode[0] += AdrWord << 8;
               AdrWord = EvalIntExpression(ArgStr[1], UInt1, &OK);
               if (OK) {
                  WAsmCode[0] += 0xa800 + (AdrWord << 9);
                  if (Memo("TABLWT")) WAsmCode[0] += 0x400;
                  CodeLen = 1;
               }
            }
         }
      };
      return;
   }

   if ((Memo("TLRD")) || (Memo("TLWT"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         WAsmCode[0] = Lo(EvalIntExpression(ArgStr[2], Int8, &OK));
         if (OK) {
            AdrWord = EvalIntExpression(ArgStr[1], UInt1, &OK);
            if (OK) {
               WAsmCode[0] += (AdrWord << 9) + 0xa000;
               if (Memo("TLWT")) WAsmCode[0] += 0x400;
               CodeLen = 1;
            }
         }
      }
      return;
   }

   if ((Memo("CALL")) || (Memo("GOTO"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrWord = EvalIntExpression(ArgStr[1], UInt16, &OK);
         if (OK)
            if (((ProgCounter() ^ AdrWord) & 0xe000) != 0) WrError(1910);
            else {
               WAsmCode[0] = 0xc000 + (AdrWord & 0x1fff);
               if (Memo("CALL")) WAsmCode[0] += 0x2000;
               CodeLen = 1;
            }
      }
      return;
   }

   if (Memo("LCALL")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrWord = EvalIntExpression(ArgStr[1], UInt16, &OK);
         if (OK) {
            CodeLen = 3;
            WAsmCode[0] = 0xb000 + Hi(AdrWord);
            WAsmCode[1] = 0x0103;
            WAsmCode[2] = 0xb700 + Lo(AdrWord);
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_17c4x(void) {
   bool ok;

   switch (ActPC) {
      case SegCode:
         ok = ProgCounter() <= 0xffff;
         break;
      case SegData:
         ok = ProgCounter() <= 0xff;
         break;
      default:
         ok = false;
   }
   return (ok);
}

static bool IsDef_17c4x(void) {
   return Memo("SFR");
}

static void SwitchFrom_17c4x(void) {
   DeinitFields();
}

static void SwitchTo_17c4x(void) {
   TurnWords = false;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "*";
   HeaderID = 0x72;
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

   MakeCode = MakeCode_17c4x;
   ChkPC = ChkPC_17c4x;
   IsDef = IsDef_17c4x;
   SwitchFrom = SwitchFrom_17c4x;
   InitFields();
}

void code17c4x_init(void) {
   CPU17C42 = AddCPU("17C42", SwitchTo_17c4x);
}
