/* code3201x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TMS3201x-Familie                                            */
/*                                                                           */
/* Historie: 28.11.1996 Grundsteinlegung                                     */
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

typedef struct {
   char *Name;
   Word Code;
} FixedOrder;

typedef struct {
   char *Name;
   Word Code;
   bool Must1;
} AdrOrder;

typedef struct {
   char *Name;
   Word Code;
   Word AllowShifts;
} AdrShiftOrder;

typedef struct {
   char *Name;
   Word Code;
   Integer Min, Max;
   Word Mask;
} ImmOrder;

#define FixedOrderCnt 14
#define JmpOrderCnt 11
#define AdrOrderCnt 21
#define AdrShiftOrderCnt 5
#define ImmOrderCnt 3

static Word AdrMode;
static bool AdrOK;

static CPUVar CPU32010, CPU32015;

static FixedOrder *FixedOrders;
static FixedOrder *JmpOrders;
static AdrOrder *AdrOrders;
static AdrShiftOrder *AdrShiftOrders;
static ImmOrder *ImmOrders;

/*----------------------------------------------------------------------------*/

static void AddFixed(char *NName, Word NCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddJmp(char *NName, Word NCode) {
   if (InstrZ >= JmpOrderCnt) exit(255);
   JmpOrders[InstrZ].Name = NName;
   JmpOrders[InstrZ++].Code = NCode;
}

static void AddAdr(char *NName, Word NCode, Word NMust1) {
   if (InstrZ >= AdrOrderCnt) exit(255);
   AdrOrders[InstrZ].Name = NName;
   AdrOrders[InstrZ].Code = NCode;
   AdrOrders[InstrZ++].Must1 = NMust1;
}

static void AddAdrShift(char *NName, Word NCode, Word NAllow) {
   if (InstrZ >= AdrShiftOrderCnt) exit(255);
   AdrShiftOrders[InstrZ].Name = NName;
   AdrShiftOrders[InstrZ].Code = NCode;
   AdrShiftOrders[InstrZ++].AllowShifts = NAllow;
}

static void AddImm(char *NName, Word NCode, Integer NMin, Integer NMax, Word NMask) {
   if (InstrZ >= ImmOrderCnt) exit(255);
   ImmOrders[InstrZ].Name = NName;
   ImmOrders[InstrZ].Code = NCode;
   ImmOrders[InstrZ].Min = NMin;
   ImmOrders[InstrZ].Max = NMax;
   ImmOrders[InstrZ++].Mask = NMask;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("ABS", 0x7f88);
   AddFixed("APAC", 0x7f8f);
   AddFixed("CALA", 0x7f8c);
   AddFixed("DINT", 0x7f81);
   AddFixed("EINT", 0x7f82);
   AddFixed("NOP", 0x7f80);
   AddFixed("PAC", 0x7f8e);
   AddFixed("POP", 0x7f9d);
   AddFixed("PUSH", 0x7f9c);
   AddFixed("RET", 0x7f8d);
   AddFixed("ROVM", 0x7f8a);
   AddFixed("SOVM", 0x7f8b);
   AddFixed("SPAC", 0x7f90);
   AddFixed("ZAC", 0x7f89);

   JmpOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * JmpOrderCnt);
   InstrZ = 0;
   AddJmp("B", 0xf900);
   AddJmp("BANZ", 0xf400);
   AddJmp("BGEZ", 0xfd00);
   AddJmp("BGZ", 0xfc00);
   AddJmp("BIOZ", 0xf600);
   AddJmp("BLEZ", 0xfb00);
   AddJmp("BLZ", 0xfa00);
   AddJmp("BNZ", 0xfe00);
   AddJmp("BV", 0xf500);
   AddJmp("BZ", 0xff00);
   AddJmp("CALL", 0xf800);

   AdrOrders = (AdrOrder *) malloc(sizeof(AdrOrder) * AdrOrderCnt);
   InstrZ = 0;
   AddAdr("ADDH", 0x6000, false);
   AddAdr("ADDS", 0x6100, false);
   AddAdr("AND", 0x7900, false);
   AddAdr("DMOV", 0x6900, false);
   AddAdr("LDP", 0x6f00, false);
   AddAdr("LST", 0x7b00, false);
   AddAdr("LT", 0x6a00, false);
   AddAdr("LTA", 0x6c00, false);
   AddAdr("LTD", 0x6b00, false);
   AddAdr("MAR", 0x6800, false);
   AddAdr("MPY", 0x6d00, false);
   AddAdr("OR", 0x7a00, false);
   AddAdr("SST", 0x7c00, true);
   AddAdr("SUBC", 0x6400, false);
   AddAdr("SUBH", 0x6200, false);
   AddAdr("SUBS", 0x6300, false);
   AddAdr("TBLR", 0x6700, false);
   AddAdr("TBLW", 0x7d00, false);
   AddAdr("XOR", 0x7800, false);
   AddAdr("ZALH", 0x6500, false);
   AddAdr("ZALS", 0x6600, false);

   AdrShiftOrders = (AdrShiftOrder *) malloc(sizeof(AdrShiftOrder) * AdrShiftOrderCnt);
   InstrZ = 0;
   AddAdrShift("ADD", 0x0000, 0xffff);
   AddAdrShift("LAC", 0x2000, 0xffff);
   AddAdrShift("SACH", 0x5800, 0x0013);
   AddAdrShift("SACL", 0x5000, 0x0001);
   AddAdrShift("SUB", 0x1000, 0xffff);

   ImmOrders = (ImmOrder *) malloc(sizeof(ImmOrder) * ImmOrderCnt);
   InstrZ = 0;
   AddImm("LACK", 0x7e00, 0, 255, 0xff);
   AddImm("LDPK", 0x6e00, 0, 1, 0x1);
   AddImm("MPYK", 0x8000, -4096, 4095, 0x1fff);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(JmpOrders);
   free(AdrOrders);
   free(AdrShiftOrders);
   free(ImmOrders);
}

/*----------------------------------------------------------------------------*/

static Word EvalARExpression(char *Asc, bool *OK) {
   *OK = true;
   if (strcasecmp(Asc, "AR0") == 0) return 0;
   if (strcasecmp(Asc, "AR1") == 0) return 1;
   return EvalIntExpression(Asc, UInt1, OK);
}

static void DecodeAdr(char *Arg, Integer Aux, bool Must1) {
   Byte h;
   char *p;

   AdrOK = false;

   if ((strcmp(Arg, "*") == 0) || (strcmp(Arg, "*-") == 0) || (strcmp(Arg, "*+") == 0)) {
      AdrMode = 0x88;
      if (strlen(Arg) == 2)
         AdrMode += (Arg[1] == '+') ? 0x20 : 0x10;
      if (Aux <= ArgCnt) {
         h = EvalARExpression(ArgStr[Aux], &AdrOK);
         if (AdrOK) {
            AdrMode &= 0xf7;
            AdrMode += h;
         }
      } else AdrOK = true;
   } else if (Aux <= ArgCnt) WrError(1110);
   else {
      h = 0;
      if ((strlen(Arg) > 3) && (strncasecmp(Arg, "DAT", 3) == 0)) {
         AdrOK = true;
         for (p = Arg + 3; *p != '\0'; p++)
            if ((*p > '9') || (*p < '0')) AdrOK = false;
         if (AdrOK) h = EvalIntExpression(Arg + 3, UInt8, &AdrOK);
      }
      if (!AdrOK) h = EvalIntExpression(Arg, Int8, &AdrOK);
      if (AdrOK)
         if ((Must1) && (h < 0x80) && (!FirstPassUnknown)) {
            WrError(1315);
            AdrOK = false;
         } else {
            AdrMode = h & 0x7f;
            ChkSpace(SegData);
         }
   }
}

static bool DecodePseudo(void) {
   Word Size;
   Integer z, z2;
   char *p;
   TempResult t;
   bool OK;

   if (Memo("PORT")) {
      CodeEquate(SegIO, 0, 7);
      return true;
   }

   if (Memo("RES")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         FirstPassUnknown = false;
         Size = EvalIntExpression(ArgStr[1], Int16, &OK);
         if (FirstPassUnknown) WrError(1820);
         if ((OK) && (!FirstPassUnknown)) {
            DontPrint = true;
            CodeLen = Size;
            if (MakeUseList)
               if (AddChunk(SegChunks + ActPC, ProgCounter(), CodeLen, ActPC == SegCode)) WrError(90);
         }
      }
      return true;
   }

   if (Memo("DATA")) {
      if (ArgCnt == 0) WrError(1110);
      else {
         OK = true;
         for (z = 1; z <= ArgCnt; z++)
            if (OK) {
               EvalExpression(ArgStr[z], &t);
               switch (t.Typ) {
                  case TempInt:
                     if ((t.Contents.Int < -32768) || (t.Contents.Int > 0xffff)) {
                        WrError(1320);
                        OK = false;
                     } else WAsmCode[CodeLen++] = t.Contents.Int;
                     break;
                  case TempFloat:
                     WrError(1135);
                     OK = false;
                     break;
                  case TempString:
                     for (p = t.Contents.Ascii, z2 = 0; *p != '\0'; p++, z2++) {
                        if ((z2 & 1) == 0)
                           WAsmCode[CodeLen] = CharTransTable[(int)*p];
                        else
                           WAsmCode[CodeLen++] += ((Word) CharTransTable[(int)*p]) << 8;
                     }
                     if ((z2 & 1) == 0) CodeLen++;
                     break;
                  default:
                     OK = false;
               }
            }
         if (!OK) CodeLen = 0;
      }
      return true;
   }

   return false;
}

static void MakeCode_3201X(void) {
   bool OK, HasSh;
   Word AdrWord;
   LongInt AdrLong;
   Integer z, Cnt;

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

/* Spruenge */

   for (z = 0; z < JmpOrderCnt; z++)
      if (Memo(JmpOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            WAsmCode[1] = EvalIntExpression(ArgStr[1], UInt12, &OK);
            if (OK) {
               CodeLen = 2;
               WAsmCode[0] = JmpOrders[z].Code;
            }
         }
         return;
      }

/* nur Adresse */

   for (z = 0; z < AdrOrderCnt; z++)
      if (Memo(AdrOrders[z].Name)) {
         if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], 2, AdrOrders[z].Must1);
            if (AdrOK) {
               CodeLen = 1;
               WAsmCode[0] = AdrOrders[z].Code + AdrMode;
            }
         }
         return;
      }

/* Adresse & schieben */

   for (z = 0; z < AdrShiftOrderCnt; z++)
      if (Memo(AdrShiftOrders[z].Name)) {
         if ((ArgCnt < 1) || (ArgCnt > 3)) WrError(1110);
         else {
            if (*ArgStr[1] == '*')
               if (ArgCnt == 2)
                  if (strncasecmp(ArgStr[2], "AR", 2) == 0) {
                     HasSh = false;
                     Cnt = 2;
                  } else {
                     HasSh = true;
                     Cnt = 3;
               } else {
                  HasSh = true;
                  Cnt = 3;
            } else {
               Cnt = 3;
               HasSh = (ArgCnt == 2);
            }
            DecodeAdr(ArgStr[1], Cnt, false);
            if (AdrOK) {
               if (!HasSh) {
                  OK = true;
                  AdrWord = 0;
               } else {
                  AdrWord = EvalIntExpression(ArgStr[2], Int4, &OK);
                  if ((OK) && (FirstPassUnknown)) AdrWord = 0;
               }
               if (OK)
                  if ((AdrShiftOrders[z].AllowShifts & (1 << AdrWord)) == 0) WrError(1380);
                  else {
                     CodeLen = 1;
                     WAsmCode[0] = AdrShiftOrders[z].Code + AdrMode + (AdrWord << 8);
                  }
            }
         }
         return;
      }

/* Ein/Ausgabe */

   if ((Memo("IN")) || (Memo("OUT"))) {
      if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], 3, false);
         if (AdrOK) {
            AdrWord = EvalIntExpression(ArgStr[2], UInt3, &OK);
            if (OK) {
               ChkSpace(SegIO);
               CodeLen = 1;
               WAsmCode[0] = 0x4000 + AdrMode + (AdrWord << 8);
               if (Memo("OUT")) WAsmCode[0] += 0x800;
            }
         }
      }
      return;
   }

/* konstantes Argument */

   for (z = 0; z < ImmOrderCnt; z++)
      if (Memo(ImmOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrLong = EvalIntExpression(ArgStr[1], Int32, &OK);
            if (OK) {
               if (FirstPassUnknown) AdrLong &= ImmOrders[z].Mask;
               if (AdrLong < ImmOrders[z].Min) WrError(1315);
               else if (AdrLong > ImmOrders[z].Max) WrError(1320);
               else {
                  CodeLen = 1;
                  WAsmCode[0] = ImmOrders[z].Code + (AdrLong & ImmOrders[z].Mask);
               }
            }
         }
         return;
      }

/* mit Hilfsregistern */

   if (Memo("LARP")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrWord = EvalARExpression(ArgStr[1], &OK);
         if (OK) {
            CodeLen = 1;
            WAsmCode[0] = 0x6880 + AdrWord;
         }
      }
      return;
   }

   if ((Memo("LAR")) || (Memo("SAR"))) {
      if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
      else {
         AdrWord = EvalARExpression(ArgStr[1], &OK);
         if (OK) {
            DecodeAdr(ArgStr[2], 3, false);
            if (AdrOK) {
               CodeLen = 1;
               WAsmCode[0] = 0x3000 + AdrMode + (AdrWord << 8);
               if (Memo("LAR")) WAsmCode[0] += 0x800;
            }
         }
      }
      return;
   }

   if (Memo("LARK")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         AdrWord = EvalARExpression(ArgStr[1], &OK);
         if (OK) {
            WAsmCode[0] = EvalIntExpression(ArgStr[2], Int8, &OK);
            if (OK) {
               CodeLen = 1;
               WAsmCode[0] = Lo(WAsmCode[0]) + 0x7000 + (AdrWord << 8);
            }
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_3201X(void) {
   switch (ActPC) {
      case SegCode:
         return (ProgCounter() <= 0xfff);
      case SegData:
         return (ProgCounter() <= ((MomCPU == CPU32010) ? 0x8f : 0xff));
      case SegIO:
         return (ProgCounter() <= 7);
      default:
         return false;
   }
}

static bool IsDef_3201X(void) {
   return (Memo("PORT"));
}

static void SwitchFrom_3201X(void) {
   DeinitFields();
}

static void SwitchTo_3201X(void) {
   TurnWords = false;
   ConstMode = ConstModeIntel;
   SetIsOccupied = false;

   PCSymbol = "$";
   HeaderID = 0x74;
   NOPCode = 0x7f80;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = (1 << SegCode) | (1 << SegData) | (1 << SegIO);
   Grans[SegCode] = 2;
   ListGrans[SegCode] = 2;
   SegInits[SegCode] = 0;
   Grans[SegData] = 2;
   ListGrans[SegData] = 2;
   SegInits[SegData] = 0;
   Grans[SegIO] = 2;
   ListGrans[SegIO] = 2;
   SegInits[SegIO] = 0;

   MakeCode = MakeCode_3201X;
   ChkPC = ChkPC_3201X;
   IsDef = IsDef_3201X;
   SwitchFrom = SwitchFrom_3201X;
   InitFields();
}

void code3201x_init(void) {
   CPU32010 = AddCPU("32010", SwitchTo_3201X);
   CPU32015 = AddCPU("32015", SwitchTo_3201X);
}
