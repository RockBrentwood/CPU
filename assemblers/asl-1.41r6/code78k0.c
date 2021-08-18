// AS-Portierung
// Codegenerator 78K0-Familie
#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

#define ModNone (-1)
#define ModReg8 0
#define MModReg8 (1 << ModReg8)
#define ModReg16 1
#define MModReg16 (1 << ModReg16)
#define ModImm 2
#define MModImm (1 << ModImm)
#define ModShort 3
#define MModShort (1 << ModShort)
#define ModSFR 4
#define MModSFR (1 << ModSFR)
#define ModAbs 5
#define MModAbs (1 << ModAbs)
#define ModIReg 6
#define MModIReg (1 << ModIReg)
#define ModIndex 7
#define MModIndex (1 << ModIndex)
#define ModDisp 8
#define MModDisp (1 << ModDisp)

#define AccReg 1
#define AccReg16 0

#define FixedOrderCount 11
#define AriOrderCount 8
#define Ari16OrderCount 3
#define ShiftOrderCount 4
#define Bit2OrderCount 3
#define RelOrderCount 4
#define BRelOrderCount 3

typedef struct {
   char *Name;
   Word Code;
} FixedOrder;

static FixedOrder *FixedOrders;
static char **AriOrders;
static char **Ari16Orders;
static char **ShiftOrders;
static char **Bit2Orders;
static char **RelOrders;
static char **BRelOrders;

static Byte OpSize, AdrPart;
static Byte AdrVals[2];
static ShortInt AdrMode;

static CPUVar CPU78070;

/*-------------------------------------------------------------------------*/
/* dynamische Codetabellenverwaltung */

static void AddFixed(char *NewName, Word NewCode) {
   if (InstrZ >= FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name = NewName;
   FixedOrders[InstrZ++].Code = NewCode;
}

static void AddAri(char *NewName) {
   if (InstrZ >= AriOrderCount) exit(255);
   AriOrders[InstrZ++] = NewName;
}

static void AddAri16(char *NewName) {
   if (InstrZ >= Ari16OrderCount) exit(255);
   Ari16Orders[InstrZ++] = NewName;
}

static void AddShift(char *NewName) {
   if (InstrZ >= ShiftOrderCount) exit(255);
   ShiftOrders[InstrZ++] = NewName;
}

static void AddBit2(char *NewName) {
   if (InstrZ >= Bit2OrderCount) exit(255);
   Bit2Orders[InstrZ++] = NewName;
}

static void AddRel(char *NewName) {
   if (InstrZ >= RelOrderCount) exit(255);
   RelOrders[InstrZ++] = NewName;
}

static void AddBRel(char *NewName) {
   if (InstrZ >= BRelOrderCount) exit(255);
   BRelOrders[InstrZ++] = NewName;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCount);
   InstrZ = 0;
   AddFixed("BRK", 0x00bf);
   AddFixed("RET", 0x00af);
   AddFixed("RETB", 0x009f);
   AddFixed("RETI", 0x008f);
   AddFixed("HALT", 0x7110);
   AddFixed("STOP", 0x7100);
   AddFixed("NOP", 0x0000);
   AddFixed("EI", 0x7a1e);
   AddFixed("DI", 0x7b1e);
   AddFixed("ADJBA", 0x6180);
   AddFixed("ADJBS", 0x6190);

   AriOrders = (char **)malloc(sizeof(char *) * AriOrderCount);
   InstrZ = 0;
   AddAri("ADD");
   AddAri("SUB");
   AddAri("ADDC");
   AddAri("SUBC");
   AddAri("CMP");
   AddAri("AND");
   AddAri("OR");
   AddAri("XOR");

   Ari16Orders = (char **)malloc(sizeof(char *) * Ari16OrderCount);
   InstrZ = 0;
   AddAri16("ADDW");
   AddAri16("SUBW");
   AddAri16("CMPW");

   ShiftOrders = (char **)malloc(sizeof(char *) * ShiftOrderCount);
   InstrZ = 0;
   AddShift("ROR");
   AddShift("RORC");
   AddShift("ROL");
   AddShift("ROLC");

   Bit2Orders = (char **)malloc(sizeof(char *) * Bit2OrderCount);
   InstrZ = 0;
   AddBit2("AND1");
   AddBit2("OR1");
   AddBit2("XOR1");

   RelOrders = (char **)malloc(sizeof(char *) * RelOrderCount);
   InstrZ = 0;
   AddRel("BC");
   AddRel("BNC");
   AddRel("BZ");
   AddRel("BNZ");

   BRelOrders = (char **)malloc(sizeof(char *) * BRelOrderCount);
   InstrZ = 0;
   AddBRel("BTCLR");
   AddBRel("BT");
   AddBRel("BF");
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(AriOrders);
   free(Ari16Orders);
   free(ShiftOrders);
   free(Bit2Orders);
   free(RelOrders);
   free(BRelOrders);
}

/*-------------------------------------------------------------------------*/
/* Adressausdruck parsen */

static void ChkAdr(Word Mask) {
   if ((AdrMode != ModNone) && ((Mask & (1 << AdrMode)) == 0)) {
      WrError(1350);
      AdrMode = ModNone;
      AdrCnt = 0;
   }
}

static void DecodeAdr(char *Asc, Word Mask) {
   static char *RegNames[8] = { "X", "A", "C", "B", "E", "D", "L", "H" };

   Word AdrWord;
   Integer z;
   bool OK, LongFlag;

   AdrMode = ModNone;
   AdrCnt = 0;

/* Register */

   for (z = 0; z < 8; z++)
      if (strcasecmp(Asc, RegNames[z]) == 0) {
         AdrMode = ModReg8;
         AdrPart = z;
         ChkAdr(Mask);
         return;
      }

   if (toupper(*Asc) != 'R') ;
   else if ((strlen(Asc) == 2) && (Asc[1] >= '0') && (Asc[1] <= '7')) {
      AdrMode = ModReg8;
      AdrPart = Asc[1] - '0';
      ChkAdr(Mask);
      return;
   } else if ((strlen(Asc) == 3) && (toupper(Asc[1]) == 'P') && (Asc[2] >= '0') && (Asc[2] <= '3')) {
      AdrMode = ModReg16;
      AdrPart = Asc[2] - '0';
      ChkAdr(Mask);
      return;
   }

   if (strlen(Asc) == 2)
      for (z = 0; z < 4; z++)
         if ((toupper(*Asc) == *RegNames[(z << 1) + 1]) && (toupper(Asc[1]) == *RegNames[z << 1])) {
            AdrMode = ModReg16;
            AdrPart = z;
            ChkAdr(Mask);
            return;
         }

/* immediate */

   if (*Asc == '#') {
      switch (OpSize) {
         case 0:
            AdrVals[0] = EvalIntExpression(Asc + 1, Int8, &OK);
            break;
         case 1:
            AdrWord = EvalIntExpression(Asc + 1, Int16, &OK);
            if (OK) {
               AdrVals[0] = Lo(AdrWord);
               AdrVals[1] = Hi(AdrWord);
            }
            break;
      }
      if (OK) {
         AdrMode = ModImm;
         AdrCnt = OpSize + 1;
      }
      ChkAdr(Mask);
      return;
   }

/* indirekt */

   if ((*Asc == '[') && (Asc[strlen(Asc) - 1] == ']')) {
      strmove(Asc, 1);
      Asc[strlen(Asc) - 1] = '\0';

      if ((strcasecmp(Asc, "DE") == 0) || (strcasecmp(Asc, "RP2") == 0)) {
         AdrMode = ModIReg;
         AdrPart = 0;
      } else if ((strncasecmp(Asc, "HL", 2) != 0) && (strncasecmp(Asc, "RP3", 3) != 0)) WrXError(1445, Asc);
      else {
         strmove(Asc, 2);
         if (*Asc == '3') strmove(Asc, 1);
         if ((strcasecmp(Asc, "+B") == 0) || (strcasecmp(Asc, "+R3") == 0)) {
            AdrMode = ModIndex;
            AdrPart = 1;
         } else if ((strcasecmp(Asc, "+C") == 0) || (strcasecmp(Asc, "+R2") == 0)) {
            AdrMode = ModIndex;
            AdrPart = 0;
         } else {
            AdrVals[0] = EvalIntExpression(Asc, UInt8, &OK);
            if (!OK) ;
            else if (AdrVals[0] == 0) {
               AdrMode = ModIReg;
               AdrPart = 1;
            } else {
               AdrMode = ModDisp;
               AdrCnt = 1;
            }
         }
      }

      ChkAdr(Mask);
      return;
   }

/* erzwungen lang ? */

   if (*Asc == '!') {
      LongFlag = true;
      strmove(Asc, 1);
   } else LongFlag = false;

/* -->absolut */

   FirstPassUnknown = true;
   AdrWord = EvalIntExpression(Asc, UInt16, &OK);
   if (FirstPassUnknown) {
      AdrWord &= 0xffffe;
      if ((Mask & MModAbs) == 0)
         AdrWord = (AdrWord | 0xff00) & 0xff1f;
   }
   if (!OK) ;
   else if ((!LongFlag) && ((Mask & MModShort) != 0) && (AdrWord >= 0xfe20) && (AdrWord <= 0xff1f)) {
      AdrMode = ModShort;
      AdrCnt = 1;
      AdrVals[0] = Lo(AdrWord);
   } else if ((!LongFlag) && ((Mask & MModSFR) != 0) && (((AdrWord >= 0xff00) && (AdrWord <= 0xffcf)) || (AdrWord >= 0xffe0))) {
      AdrMode = ModSFR;
      AdrCnt = 1;
      AdrVals[0] = Lo(AdrWord);
   } else {
      AdrMode = ModAbs;
      AdrCnt = 2;
      AdrVals[0] = Lo(AdrWord);
      AdrVals[1] = Hi(AdrWord);
   }

   ChkAdr(Mask);
}

static void ChkEven(void) {
   if ((AdrMode == ModAbs) || (AdrMode == ModShort) || (AdrMode == ModSFR))
      if ((AdrVals[0] & 1) == 1) WrError(180);
}

static bool DecodeBitAdr(char *Asc, Byte * Erg) {
   char *p;
   bool OK;

   p = RQuotPos(Asc, '.');
   if (p == NULL) {
      WrError(1510);
      return false;
   }

   *p = '\0';
   *Erg = EvalIntExpression(p + 1, UInt3, &OK) << 4;
   if (!OK) return false;

   DecodeAdr(Asc, MModShort + MModSFR + MModIReg + MModReg8);
   switch (AdrMode) {
      case ModReg8:
         if (AdrPart != AccReg) {
            WrError(1350);
            return false;
         } else {
            *Erg += 0x88;
            return true;
         }
      case ModShort:
         return true;
      case ModSFR:
         *Erg += 0x08;
         return true;
      case ModIReg:
         if (AdrPart == 0) {
            WrError(1350);
            return false;
         } else {
            *Erg += 0x80;
            return true;
         }
      default:
         return false;
   }
}

/*-------------------------------------------------------------------------*/

static bool DecodePseudo(void) {
   return false;
}

static void MakeCode_78K0(void) {
   Integer z;
   Byte HReg;
   Word AdrWord;
   Integer AdrInt;
   bool OK;

   CodeLen = 0;
   DontPrint = false;
   OpSize = 0;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(false)) return;

/* ohne Argument */

   for (z = 0; z < FixedOrderCount; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else if (Hi(FixedOrders[z].Code) == 0) CodeLen = 1;
         else {
            BAsmCode[0] = Hi(FixedOrders[z].Code);
            CodeLen = 2;
         }
         BAsmCode[CodeLen - 1] = Lo(FixedOrders[z].Code);
         return;
      }

/* Datentransfer */

   if (Memo("MOV")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg8 + MModShort + MModSFR + MModAbs + MModIReg + MModIndex + MModDisp);
         switch (AdrMode) {
            case ModReg8:
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], MModImm + MModReg8 + ((HReg == AccReg) ? MModShort + MModSFR + MModAbs + MModIReg + MModIndex + MModDisp : 0));
               switch (AdrMode) {
                  case ModReg8:
                     if ((HReg == AccReg) == (AdrPart == AccReg)) WrError(1350);
                     else if (HReg == AccReg) {
                        CodeLen = 1;
                        BAsmCode[0] = 0x60 + AdrPart;
                     } else {
                        CodeLen = 1;
                        BAsmCode[0] = 0x70 + HReg;
                     }
                     break;
                  case ModImm:
                     CodeLen = 2;
                     BAsmCode[0] = 0xa0 + HReg;
                     BAsmCode[1] = AdrVals[0];
                     break;
                  case ModShort:
                     CodeLen = 2;
                     BAsmCode[0] = 0xf0;
                     BAsmCode[1] = AdrVals[0];
                     break;
                  case ModSFR:
                     CodeLen = 2;
                     BAsmCode[0] = 0xf4;
                     BAsmCode[1] = AdrVals[0];
                     break;
                  case ModAbs:
                     CodeLen = 3;
                     BAsmCode[0] = 0xfe;
                     memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                     break;
                  case ModIReg:
                     CodeLen = 1;
                     BAsmCode[0] = 0x85 + (AdrPart << 1);
                     break;
                  case ModIndex:
                     CodeLen = 1;
                     BAsmCode[0] = 0xaa + AdrPart;
                     break;
                  case ModDisp:
                     CodeLen = 2;
                     BAsmCode[0] = 0xae;
                     BAsmCode[1] = AdrVals[0];
                     break;
               }
               break;
            case ModShort:
               BAsmCode[1] = AdrVals[0];
               DecodeAdr(ArgStr[2], MModReg8 + MModImm);
               switch (AdrMode) {
                  case ModReg8:
                     if (AdrPart != AccReg) WrError(1350);
                     else {
                        BAsmCode[0] = 0xf2;
                        CodeLen = 2;
                     }
                     break;
                  case ModImm:
                     BAsmCode[0] = 0x11;
                     BAsmCode[2] = AdrVals[0];
                     CodeLen = 3;
                     break;
               }
               break;
            case ModSFR:
               BAsmCode[1] = AdrVals[0];
               DecodeAdr(ArgStr[2], MModReg8 + MModImm);
               switch (AdrMode) {
                  case ModReg8:
                     if (AdrPart != AccReg) WrError(1350);
                     else {
                        BAsmCode[0] = 0xf6;
                        CodeLen = 2;
                     }
                     break;
                  case ModImm:
                     BAsmCode[0] = 0x13;
                     BAsmCode[2] = AdrVals[0];
                     CodeLen = 3;
                     break;
               }
               break;
            case ModAbs:
               memcpy(BAsmCode + 1, AdrVals, 2);
               DecodeAdr(ArgStr[2], MModReg8);
               if (AdrMode != ModReg8) ;
               else if (AdrPart != AccReg) WrError(1350);
               else {
                  BAsmCode[0] = 0x9e;
                  CodeLen = 3;
               }
               break;
            case ModIReg:
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], MModReg8);
               if (AdrMode != ModReg8) ;
               else if (AdrPart != AccReg) WrError(1350);
               else {
                  BAsmCode[0] = 0x95 + (AdrPart << 1);
                  CodeLen = 1;
               }
               break;
            case ModIndex:
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], MModReg8);
               if (AdrMode != ModReg8) ;
               else if (AdrPart != AccReg) WrError(1350);
               else {
                  BAsmCode[0] = 0xba + HReg;
                  CodeLen = 1;
               }
               break;
            case ModDisp:
               BAsmCode[1] = AdrVals[0];
               DecodeAdr(ArgStr[2], MModReg8);
               if (AdrMode != ModReg8) ;
               else if (AdrPart != AccReg) WrError(1350);
               else {
                  BAsmCode[0] = 0xbe;
                  CodeLen = 2;
               }
               break;
         }
      }
      return;
   }

   if (Memo("XCH")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         if ((strcasecmp(ArgStr[2], "A") == 0) || (strcasecmp(ArgStr[2], "RP1") == 0)) {
            strcopy(ArgStr[3], ArgStr[1]);
            strcopy(ArgStr[1], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[3]);
         }
         DecodeAdr(ArgStr[1], MModReg8);
         if (AdrMode == ModNone) ;
         else if (AdrPart != AccReg) WrError(1350);
         else {
            DecodeAdr(ArgStr[2], MModReg8 + MModShort + MModSFR + MModAbs + MModIReg + MModIndex + MModDisp);
            switch (AdrMode) {
               case ModReg8:
                  if (AdrPart == AccReg) WrError(1350);
                  else {
                     BAsmCode[0] = 0x30 + AdrPart;
                     CodeLen = 1;
                  }
                  break;
               case ModShort:
                  BAsmCode[0] = 0x83;
                  BAsmCode[1] = AdrVals[0];
                  CodeLen = 2;
                  break;
               case ModSFR:
                  BAsmCode[0] = 0x93;
                  BAsmCode[1] = AdrVals[0];
                  CodeLen = 2;
                  break;
               case ModAbs:
                  BAsmCode[0] = 0xce;
                  memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                  CodeLen = 3;
                  break;
               case ModIReg:
                  BAsmCode[0] = 0x05 + (AdrPart << 1);
                  CodeLen = 1;
                  break;
               case ModIndex:
                  BAsmCode[0] = 0x31;
                  BAsmCode[1] = 0x8a + AdrPart;
                  CodeLen = 2;
                  break;
               case ModDisp:
                  BAsmCode[0] = 0xde;
                  BAsmCode[1] = AdrVals[0];
                  CodeLen = 2;
                  break;
            }
         }
      }
      return;
   }

   if (Memo("MOVW")) {
      OpSize = 1;
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg16 + MModShort + MModSFR + MModAbs);
         switch (AdrMode) {
            case ModReg16:
               HReg = AdrPart;
               DecodeAdr(ArgStr[2], MModReg16 + MModImm + ((HReg == AccReg16) ? MModShort + MModSFR + MModAbs : 0));
               switch (AdrMode) {
                  case ModReg16:
                     if ((HReg == AccReg16) == (AdrPart == AccReg16)) WrError(1350);
                     else if (HReg == AccReg16) {
                        BAsmCode[0] = 0xc0 + (AdrPart << 1);
                        CodeLen = 1;
                     } else {
                        BAsmCode[0] = 0xd0 + (HReg << 1);
                        CodeLen = 1;
                     }
                     break;
                  case ModImm:
                     BAsmCode[0] = 0x10 + (HReg << 1);
                     memcpy(BAsmCode + 1, AdrVals, 2);
                     CodeLen = 3;
                     break;
                  case ModShort:
                     BAsmCode[0] = 0x89;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     ChkEven();
                     break;
                  case ModSFR:
                     BAsmCode[0] = 0xa9;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     ChkEven();
                     break;
                  case ModAbs:
                     BAsmCode[0] = 0x02;
                     memcpy(BAsmCode + 1, AdrVals, 2);
                     CodeLen = 3;
                     ChkEven();
                     break;
               }
               break;
            case ModShort:
               ChkEven();
               BAsmCode[1] = AdrVals[0];
               DecodeAdr(ArgStr[2], MModReg16 + MModImm);
               switch (AdrMode) {
                  case ModReg16:
                     if (AdrPart != AccReg16) WrError(1350);
                     else {
                        BAsmCode[0] = 0x99;
                        CodeLen = 2;
                     }
                     break;
                  case ModImm:
                     BAsmCode[0] = 0xee;
                     memcpy(BAsmCode + 2, AdrVals, 2);
                     CodeLen = 4;
                     break;
               }
               break;
            case ModSFR:
               ChkEven();
               BAsmCode[1] = AdrVals[0];
               DecodeAdr(ArgStr[2], MModReg16 + MModImm);
               switch (AdrMode) {
                  case ModReg16:
                     if (AdrPart != AccReg16) WrError(1350);
                     else {
                        BAsmCode[0] = 0xb9;
                        CodeLen = 2;
                     }
                     break;
                  case ModImm:
                     BAsmCode[0] = 0xfe;
                     memcpy(BAsmCode + 2, AdrVals, 2);
                     CodeLen = 4;
                     break;
               }
               break;
            case ModAbs:
               ChkEven();
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               DecodeAdr(ArgStr[2], MModReg16);
               if (AdrMode != ModReg16) ;
               else if (AdrPart != AccReg16) WrError(1350);
               else {
                  BAsmCode[0] = 0x03;
                  CodeLen = 3;
               }
               break;
         }
      }
      return;
   }

   if (Memo("XCHW")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg16);
         if (AdrMode == ModReg16) {
            HReg = AdrPart;
            DecodeAdr(ArgStr[2], MModReg16);
            if (AdrMode != ModReg16) ;
            else if ((HReg == AccReg16) == (AdrPart == AccReg16)) WrError(1350);
            else {
               BAsmCode[0] = (HReg == AccReg16) ? 0xe0 + (AdrPart << 1) : 0xe0 + (HReg << 1);
               CodeLen = 1;
            }
         }
      }
      return;
   }

   if ((Memo("PUSH")) || (Memo("POP"))) {
      z = Memo("POP");
      if (ArgCnt != 1) WrError(1110);
      else if (strcasecmp(ArgStr[1], "PSW") == 0) {
         BAsmCode[0] = 0x22 + z;
         CodeLen = 1;
      } else {
         DecodeAdr(ArgStr[1], MModReg16);
         if (AdrMode == ModReg16) {
            BAsmCode[0] = 0xb1 - z + (AdrPart << 1);
            CodeLen = 1;
         }
      }
      return;
   }

/* Arithmetik */

   for (z = 0; z < AriOrderCount; z++)
      if (Memo(AriOrders[z])) {
         if (ArgCnt != 2) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], MModReg8 + MModShort);
            switch (AdrMode) {
               case ModReg8:
                  HReg = AdrPart;
                  DecodeAdr(ArgStr[2], MModReg8 + ((HReg == AccReg) ? (MModImm + MModShort + MModAbs + MModIReg + MModIndex + MModDisp) : 0));
                  switch (AdrMode) {
                     case ModReg8:
                        if (AdrPart == AccReg) {
                           BAsmCode[0] = 0x61;
                           BAsmCode[1] = (z << 4) + HReg;
                           CodeLen = 2;
                        } else if (HReg == AccReg) {
                           BAsmCode[0] = 0x61;
                           BAsmCode[1] = 0x08 + (z << 4) + AdrPart;
                           CodeLen = 2;
                        } else WrError(1350);
                        break;
                     case ModImm:
                        BAsmCode[0] = (z << 4) + 0x0d;
                        BAsmCode[1] = AdrVals[0];
                        CodeLen = 2;
                        break;
                     case ModShort:
                        BAsmCode[0] = (z << 4) + 0x0e;
                        BAsmCode[1] = AdrVals[0];
                        CodeLen = 2;
                        break;
                     case ModAbs:
                        BAsmCode[0] = (z << 4) + 0x08;
                        memcpy(BAsmCode + 1, AdrVals, 2);
                        CodeLen = 3;
                        break;
                     case ModIReg:
                        if (AdrPart == 0) WrError(1350);
                        else {
                           BAsmCode[0] = (z << 4) + 0x0f;
                           CodeLen = 2;
                        }
                        break;
                     case ModIndex:
                        BAsmCode[0] = 0x31;
                        BAsmCode[1] = (z << 4) + 0x0a + AdrPart;
                        CodeLen = 2;
                        break;
                     case ModDisp:
                        BAsmCode[0] = (z << 4) + 0x09;
                        BAsmCode[1] = AdrVals[0];
                        CodeLen = 2;
                        break;
                  }
                  break;
               case ModShort:
                  BAsmCode[1] = AdrVals[0];
                  DecodeAdr(ArgStr[2], MModImm);
                  if (AdrMode == ModImm) {
                     BAsmCode[0] = (z << 4) + 0x88;
                     BAsmCode[2] = AdrVals[0];
                     CodeLen = 3;
                  }
                  break;
            }
         }
         return;
      }

   for (z = 0; z < Ari16OrderCount; z++)
      if (Memo(Ari16Orders[z])) {
         if (ArgCnt != 2) WrError(1110);
         else {
            OpSize = 1;
            DecodeAdr(ArgStr[1], MModReg16);
            if (AdrMode == ModReg16) {
               DecodeAdr(ArgStr[2], MModImm);
               if (AdrMode == ModImm) {
                  BAsmCode[0] = 0xca + (z << 4);
                  memcpy(BAsmCode + 1, AdrVals, 2);
                  CodeLen = 3;
               }
            }
         }
         return;
      }

   if (Memo("MULU")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg8);
         if (AdrMode != ModReg8) ;
         else if (AdrPart != 0) WrError(1350);
         else {
            BAsmCode[0] = 0x31;
            BAsmCode[1] = 0x88;
            CodeLen = 2;
         }
      }
      return;
   }

   if (Memo("DIVUW")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg8);
         if (AdrMode != ModReg8) ;
         else if (AdrPart != 2) WrError(1350);
         else {
            BAsmCode[0] = 0x31;
            BAsmCode[1] = 0x82;
            CodeLen = 2;
         }
      }
      return;
   }

   if ((Memo("INC")) || (Memo("DEC"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         z = Memo("DEC") << 4;
         DecodeAdr(ArgStr[1], MModReg8 + MModShort);
         switch (AdrMode) {
            case ModReg8:
               BAsmCode[0] = 0x40 + AdrPart + z;
               CodeLen = 1;
               break;
            case ModShort:
               BAsmCode[0] = 0x81 + z;
               BAsmCode[1] = AdrVals[0];
               CodeLen = 2;
               break;
         }
      }
      return;
   }

   if ((Memo("INCW")) || (Memo("DECW"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg16);
         if (AdrMode == ModReg16) {
            BAsmCode[0] = 0x80 + (Memo("DECW") << 4) + (AdrPart << 1);
            CodeLen = 1;
         }
      }
      return;
   }

   for (z = 0; z < ShiftOrderCount; z++)
      if (Memo(ShiftOrders[z])) {
         if (ArgCnt != 2) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], MModReg8);
            if (AdrMode != ModReg8) ;
            else if (AdrPart != AccReg) WrError(1350);
            else {
               HReg = EvalIntExpression(ArgStr[2], UInt1, &OK);
               if (!OK) ;
               else if (HReg != 1) WrError(1315);
               else {
                  BAsmCode[0] = 0x24 + z;
                  CodeLen = 1;
               }
            }
         }
         return;
      }

   if ((Memo("ROL4")) || (Memo("ROR4"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModIReg);
         if (AdrMode != ModIReg) ;
         else if (AdrPart == 0) WrError(1350);
         else {
            BAsmCode[0] = 0x31;
            BAsmCode[1] = 0x80 + (Memo("ROR4") << 4);
            CodeLen = 2;
         }
      }
      return;
   }

/* Bitoperationen */

   if (Memo("MOV1")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         if (strcasecmp(ArgStr[2], "CY") == 0) {
            strcopy(ArgStr[3], ArgStr[1]);
            strcopy(ArgStr[1], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[3]);
            z = 1;
         } else z = 4;
         if (strcasecmp(ArgStr[1], "CY") != 0) WrError(1110);
         else if (DecodeBitAdr(ArgStr[2], &HReg)) {
            BAsmCode[0] = 0x61 + (((HReg & 0x88) != 0x88) << 4);
            BAsmCode[1] = z + HReg;
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
            CodeLen = 2 + AdrCnt;
         }
      }
      return;
   }

   for (z = 0; z < Bit2OrderCount; z++)
      if (Memo(Bit2Orders[z])) {
         if (ArgCnt != 2) WrError(1110);
         else if (strcasecmp(ArgStr[1], "CY") != 0) WrError(1110);
         else if (DecodeBitAdr(ArgStr[2], &HReg)) {
            BAsmCode[0] = 0x61 + (((HReg & 0x88) != 0x88) << 4);
            BAsmCode[1] = z + 5 + HReg;
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
            CodeLen = 2 + AdrCnt;
         }
         return;
      }

   if ((Memo("SET1")) || (Memo("CLR1"))) {
      z = Memo("CLR1");
      if (ArgCnt != 1) WrError(1110);
      else if (strcasecmp(ArgStr[1], "CY") == 0) {
         BAsmCode[0] = 0x20 + z;
         CodeLen = 1;
      } else if (!DecodeBitAdr(ArgStr[1], &HReg)) ;
      else if ((HReg & 0x88) == 0) {
         BAsmCode[0] = 0x0a + z + (HReg & 0x70);
         BAsmCode[1] = AdrVals[0];
         CodeLen = 2;
      } else {
         BAsmCode[0] = 0x61 + (((HReg & 0x88) != 0x88) << 4);
         BAsmCode[1] = HReg + 2 + z;
         memcpy(BAsmCode + 2, AdrVals, AdrCnt);
         CodeLen = 2 + AdrCnt;
      }
      return;
   }

   if (Memo("NOT1")) {
      if (ArgCnt != 1) WrError(1110);
      else if (strcasecmp(ArgStr[1], "CY") != 0) WrError(1350);
      else {
         BAsmCode[0] = 0x01;
         CodeLen = 1;
      }
      return;
   }

/* Spruenge */

   if (Memo("CALL")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModAbs);
         if (AdrMode == ModAbs) {
            BAsmCode[0] = 0x9a;
            memcpy(BAsmCode + 1, AdrVals, 2);
            CodeLen = 3;
         }
      }
      return;
   }

   if (Memo("CALLF")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         if (*ArgStr[1] == '!') strmove(ArgStr[1], 1);
         AdrWord = EvalIntExpression(ArgStr[1], UInt11, &OK);
         if (OK) {
            BAsmCode[0] = 0x0c + (Hi(AdrWord) << 4);
            BAsmCode[1] = Lo(AdrWord);
            CodeLen = 2;
         }
      }
      return;
   }

   if (Memo("CALLT")) {
      if (ArgCnt != 1) WrError(1110);
      else if ((*ArgStr[1] != '[') || (ArgStr[1][strlen(ArgStr[1]) - 1] != ']')) WrError(1350);
      else {
         FirstPassUnknown = false;
         ArgStr[1][strlen(ArgStr[1]) - 1] = '\0';
         AdrWord = EvalIntExpression(ArgStr[1] + 1, UInt6, &OK);
         if (FirstPassUnknown) AdrWord &= 0xfffe;
         if (!OK) ;
         else if (Odd(AdrWord)) WrError(1325);
         else {
            BAsmCode[0] = 0xc1 + (AdrWord & 0x3e);
            CodeLen = 1;
         }
      }
      return;
   }

   if (Memo("BR")) {
      if (ArgCnt != 1) WrError(1110);
      else if ((strcasecmp(ArgStr[1], "AX") == 0) || (strcasecmp(ArgStr[1], "RP0") == 0)) {
         BAsmCode[0] = 0x31;
         BAsmCode[1] = 0x98;
         CodeLen = 2;
      } else {
         if (*ArgStr[1] == '!') {
            strmove(ArgStr[1], 1);
            HReg = 1;
         } else if (*ArgStr[1] == '$') {
            strmove(ArgStr[1], 1);
            HReg = 2;
         } else HReg = 0;
         AdrWord = EvalIntExpression(ArgStr[1], UInt16, &OK);
         if (OK) {
            if (HReg == 0) {
               AdrInt = AdrWord - (EProgCounter() - 2);
               HReg = ((AdrInt >= -128) && (AdrInt < 127)) ? 2 : 1;
            }
            switch (HReg) {
               case 1:
                  BAsmCode[0] = 0x9b;
                  BAsmCode[1] = Lo(AdrWord);
                  BAsmCode[2] = Hi(AdrWord);
                  CodeLen = 3;
                  break;
               case 2:
                  AdrInt = AdrWord - (EProgCounter() + 2);
                  if (((AdrInt < -128) || (AdrInt > 127)) && (!SymbolQuestionable)) WrError(1370);
                  else {
                     BAsmCode[0] = 0xfa;
                     BAsmCode[1] = AdrInt & 0xff;
                     CodeLen = 2;
                  }
                  break;
            }
         }
      }
      return;
   }

   for (z = 0; z < RelOrderCount; z++)
      if (Memo(RelOrders[z])) {
         if (ArgCnt != 1) WrError(1110);
         else {
            if (*ArgStr[1] == '$') strmove(ArgStr[1], 1);
            AdrInt = EvalIntExpression(ArgStr[1], UInt16, &OK) - (EProgCounter() + 2);
            if (!OK) ;
            else if (((AdrInt < -128) || (AdrInt > 127)) && (!SymbolQuestionable)) WrError(1370);
            else {
               BAsmCode[0] = 0x8b + (z << 4);
               BAsmCode[1] = AdrInt & 0xff;
               CodeLen = 2;
            }
         }
         return;
      }

   for (z = 0; z < BRelOrderCount; z++)
      if (Memo(BRelOrders[z])) {
         if (ArgCnt != 2) WrError(1110);
         else if (DecodeBitAdr(ArgStr[1], &HReg)) {
            if ((z == 1) && ((HReg & 0x88) == 0)) {
               BAsmCode[0] = 0x8c + HReg;
               BAsmCode[1] = AdrVals[0];
               HReg = 2;
            } else {
               BAsmCode[0] = 0x31;
               switch (HReg & 0x88) {
                  case 0x00:
                     BAsmCode[1] = 0x00;
                     break;
                  case 0x08:
                     BAsmCode[1] = 0x04;
                     break;
                  case 0x80:
                     BAsmCode[1] = 0x84;
                     break;
                  case 0x88:
                     BAsmCode[1] = 0x0c;
                     break;
               }
               BAsmCode[1] += (HReg & 0x70) + z + 1;
               BAsmCode[2] = AdrVals[0];
               HReg = 2 + AdrCnt;
            }
            if (*ArgStr[2] == '$') strmove(ArgStr[2], 1);
            AdrInt = EvalIntExpression(ArgStr[2], UInt16, &OK) - (EProgCounter() + HReg + 1);
            if (!OK) ;
            else if (((AdrInt < -128) || (AdrInt > 127)) && (!SymbolQuestionable)) WrError(1370);
            else {
               BAsmCode[HReg] = AdrInt & 0xff;
               CodeLen = HReg + 1;
            }
         }
         return;
      }

   if (Memo("DBNZ")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModReg8 + MModShort);
         if ((AdrMode == ModReg8) && ((AdrPart & 6) != 2)) WrError(1350);
         else if (AdrMode != ModNone) {
            BAsmCode[0] = (AdrMode == ModReg8) ? 0x88 + AdrPart : 0x04;
            BAsmCode[1] = AdrVals[0];
            if (*ArgStr[2] == '$') strmove(ArgStr[2], 1);
            AdrInt = EvalIntExpression(ArgStr[2], UInt16, &OK) - (EProgCounter() + AdrCnt + 2);
            if (!OK) ;
            else if (((AdrInt < -128) || (AdrInt > 127)) && (!SymbolQuestionable)) WrError(1370);
            else {
               BAsmCode[AdrCnt + 1] = AdrInt & 0xff;
               CodeLen = AdrCnt + 2;
            }
         }
      }
      return;
   }

/* Steueranweisungen */

   if (Memo("SEL")) {
      if (ArgCnt != 1) WrError(1350);
      else if ((strlen(ArgStr[1]) != 3) || (strncasecmp(ArgStr[1], "RB", 2) != 0)) WrError(1350);
      else {
         HReg = ArgStr[1][2] - '0';
         if (ChkRange(HReg, 0, 3)) {
            BAsmCode[0] = 0x61;
            BAsmCode[1] = 0xd0 + ((HReg & 1) << 3) + ((HReg & 2) << 4);
            CodeLen = 2;
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_78K0(void) {
   switch (ActPC) {
      case SegCode:
         return (ProgCounter() < 0x10000);
      default:
         return false;
   }
}

static bool IsDef_78K0(void) {
   return false;
}

static void SwitchFrom_78K0(void) {
   DeinitFields();
}

static void SwitchTo_78K0(void) {
   TurnWords = false;
   ConstMode = ConstModeIntel;
   SetIsOccupied = false;

   PCSymbol = "PC";
   HeaderID = 0x7c;
   NOPCode = 0x00;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_78K0;
   ChkPC = ChkPC_78K0;
   IsDef = IsDef_78K0;
   SwitchFrom = SwitchFrom_78K0;
   InitFields();
}

void code78k0_init(void) {
   CPU78070 = AddCPU("78070", SwitchTo_78K0);
}
