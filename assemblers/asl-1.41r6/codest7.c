/* codest7.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator SGS-Thomson ST7                                             */
/*                                                                           */
/* Historie: 21.5.1997 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "stringutil.h"
#include "nls.h"
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
   bool MayImm;
} AriOrder;

#define FixedOrderCnt 11
#define AriOrderCnt 8
#define RMWOrderCnt 13
#define RelOrderCnt 20

#define ModNone (-1)
#define ModImm 0
#define MModImm (1l << ModImm)
#define ModAbs8 1
#define MModAbs8 (1l << ModAbs8)
#define ModAbs16 2
#define MModAbs16 (1l << ModAbs16)
#define ModIX 3
#define MModIX (1l << ModIX)
#define ModIX8 4
#define MModIX8 (1l << ModIX8)
#define ModIX16 5
#define MModIX16 (1l << ModIX16)
#define ModIY 6
#define MModIY (1l << ModIY)
#define ModIY8 7
#define MModIY8 (1l << ModIY8)
#define ModIY16 8
#define MModIY16 (1l << ModIY16)
#define ModIAbs8 9
#define MModIAbs8 (1l << ModIAbs8)
#define ModIAbs16 10
#define MModIAbs16 (1l << ModIAbs16)
#define ModIXAbs8 11
#define MModIXAbs8 (1l << ModIXAbs8)
#define ModIXAbs16 12
#define MModIXAbs16 (1l << ModIXAbs16)
#define ModIYAbs8 13
#define MModIYAbs8 (1l << ModIYAbs8)
#define ModIYAbs16 14
#define MModIYAbs16 (1l << ModIYAbs16)
#define ModA 15
#define MModA (1l << ModA)
#define ModX 16
#define MModX (1l << ModX)
#define ModY 17
#define MModY (1l << ModY)
#define ModS 18
#define MModS (1l << ModS)
#define ModCCR 19
#define MModCCR (1l << ModCCR)

static CPUVar CPUST7;

static FixedOrder *FixedOrders;
static AriOrder *AriOrders;
static FixedOrder *RMWOrders;
static FixedOrder *RelOrders;

static ShortInt AdrType;
static Byte AdrPart, OpSize, PrefixCnt;
static Byte AdrVals[3];

/*--------------------------------------------------------------------------*/

static void AddFixed(char *NName, Byte NCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddAri(char *NName, Byte NCode, bool NMay) {
   if (InstrZ >= AriOrderCnt) exit(255);
   AriOrders[InstrZ].Name = NName;
   AriOrders[InstrZ].Code = NCode;
   AriOrders[InstrZ++].MayImm = NMay;
}

static void AddRMW(char *NName, Byte NCode) {
   if (InstrZ >= RMWOrderCnt) exit(255);
   RMWOrders[InstrZ].Name = NName;
   RMWOrders[InstrZ++].Code = NCode;
}

static void AddRel(char *NName, Byte NCode) {
   if (InstrZ >= RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name = NName;
   RelOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   InstrZ = 0;
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   AddFixed("HALT", 0x8e);
   AddFixed("IRET", 0x80);
   AddFixed("NOP", 0x9d);
   AddFixed("RCF", 0x98);
   AddFixed("RET", 0x81);
   AddFixed("RIM", 0x9a);
   AddFixed("RSP", 0x9c);
   AddFixed("SCF", 0x99);
   AddFixed("SIM", 0x9b);
   AddFixed("TRAP", 0x83);
   AddFixed("WFI", 0x8f);

   InstrZ = 0;
   AriOrders = (AriOrder *) malloc(sizeof(AriOrder) * AriOrderCnt);
   AddAri("ADC", 0x09, true);
   AddAri("ADD", 0x0b, true);
   AddAri("AND", 0x04, true);
   AddAri("BCP", 0x05, true);
   AddAri("OR", 0x0a, true);
   AddAri("SBC", 0x02, true);
   AddAri("SUB", 0x00, true);
   AddAri("XOR", 0x08, true);

   InstrZ = 0;
   RMWOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * RMWOrderCnt);
   AddRMW("CLR", 0x0f);
   AddRMW("CPL", 0x03);
   AddRMW("DEC", 0x0a);
   AddRMW("INC", 0x0c);
   AddRMW("NEG", 0x00);
   AddRMW("RLC", 0x09);
   AddRMW("RRC", 0x06);
   AddRMW("SLA", 0x08);
   AddRMW("SLL", 0x08);
   AddRMW("SRA", 0x07);
   AddRMW("SRL", 0x04);
   AddRMW("SWAP", 0x0e);
   AddRMW("TNZ", 0x0d);

   InstrZ = 0;
   RelOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * RelOrderCnt);
   AddRel("CALLR", 0xad);
   AddRel("JRA", 0x20);
   AddRel("JRC", 0x25);
   AddRel("JREQ", 0x27);
   AddRel("JRF", 0x21);
   AddRel("JRH", 0x29);
   AddRel("JRIH", 0x2f);
   AddRel("JRIL", 0x2e);
   AddRel("JRM", 0x2d);
   AddRel("JRMI", 0x2b);
   AddRel("JRNC", 0x24);
   AddRel("JRNE", 0x26);
   AddRel("JRNH", 0x28);
   AddRel("JRNM", 0x2c);
   AddRel("JRPL", 0x2a);
   AddRel("JRT", 0x20);
   AddRel("JRUGE", 0x24);
   AddRel("JRUGT", 0x22);
   AddRel("JRULE", 0x23);
   AddRel("JRULT", 0x25);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(AriOrders);
   free(RMWOrders);
   free(RelOrders);
}

/*--------------------------------------------------------------------------*/

static void AddPrefix(Byte Pref) {
   BAsmCode[PrefixCnt++] = Pref;
}

static void DecideSize(LongInt Mask, char *Asc, LongInt Type1, LongInt Type2, Byte Part1, Byte Part2) {
   enum { None, I8, I16 } Size;
   Word Value;
   bool OK;
   int l = strlen(Asc);

   if ((l >= 3) && (Asc[l - 2] == '.')) {
      if (toupper(Asc[l - 1]) == 'B') {
         Size = I8;
         Asc[l - 2] = '\0';
      } else if (toupper(Asc[l - 1]) == 'W') {
         Size = I16;
         Asc[l - 2] = '\0';
      } else Size = None;
   } else Size = None;

   if (Size == I8) Value = EvalIntExpression(Asc, UInt8, &OK);
   else Value = EvalIntExpression(Asc, Int16, &OK);

   if (OK)
      if ((Size == I8) || (((Mask & (1l << Type1)) != 0) && (Size == None) && (Hi(Value) == 0))) {
         AdrVals[0] = Lo(Value);
         AdrCnt = 1;
         AdrPart = Part1;
         AdrType = Type1;
      } else {
         AdrVals[0] = Hi(Value);
         AdrVals[1] = Lo(Value);
         AdrCnt = 2;
         AdrPart = Part2;
         AdrType = Type2;
      }
}

static void DecideASize(LongInt Mask, char *Asc, LongInt Type1, LongInt Type2, Byte Part1, Byte Part2) {
   bool I16;
   bool OK;
   int l = strlen(Asc);

   if ((l >= 3) && (Asc[l - 2] == '.') && (toupper(Asc[l - 1]) == 'W')) {
      I16 = true;
      Asc[l - 2] = '\0';
   } else if (((Mask & (1l << Type1))) == 0) I16 = true;
   else I16 = false;

   AdrVals[0] = EvalIntExpression(Asc, UInt8, &OK);
   if (OK) {
      AdrCnt = 1;
      if (I16) {
         AdrPart = Part2;
         AdrType = Type2;
      } else {
         AdrPart = Part1;
         AdrType = Type1;
      }
   }
}

static void ChkAdr(LongInt Mask) {
   if ((AdrType != ModNone) && ((Mask & (1l << AdrType)) == 0)) {
      WrError(1350);
      AdrType = ModNone;
      AdrCnt = 0;
   }
}

static void DecodeAdr(char *Asc_O, LongInt Mask) {
   bool OK, YReg;
   String Asc, Asc2;
   char *p;

   strmaxcpy(Asc, Asc_O, 255);

   AdrType = ModNone;
   AdrCnt = 0;

/* Register ? */

   if (strcasecmp(Asc, "A") == 0) {
      AdrType = ModA;
      ChkAdr(Mask);
      return;
   }

   if (strcasecmp(Asc, "X") == 0) {
      AdrType = ModX;
      ChkAdr(Mask);
      return;
   }

   if (strcasecmp(Asc, "Y") == 0) {
      AdrType = ModY;
      AddPrefix(0x90);
      ChkAdr(Mask);
      return;
   }

   if (strcasecmp(Asc, "S") == 0) {
      AdrType = ModS;
      ChkAdr(Mask);
      return;
   }

   if (strcasecmp(Asc, "CC") == 0) {
      AdrType = ModCCR;
      ChkAdr(Mask);
      return;
   }

/* immediate ? */

   if (*Asc == '#') {
      AdrVals[0] = EvalIntExpression(Asc + 1, Int8, &OK);
      if (OK) {
         AdrType = ModImm;
         AdrPart = 0xa;
         AdrCnt = 1;
      }
      ChkAdr(Mask);
      return;
   }

/* speicherindirekt ? */

   if ((*Asc == '[') && (Asc[strlen(Asc) - 1] == ']')) {
      strcpy(Asc, Asc + 1);
      Asc[strlen(Asc) - 1] = '\0';
      DecideASize(Mask, Asc, ModIAbs8, ModIAbs16, 0xb, 0xc);
      if (AdrType != ModNone) AddPrefix(0x92);
      ChkAdr(Mask);
      return;
   }

/* sonstwie indirekt ? */

   if (IsIndirect(Asc)) {
      strcpy(Asc, Asc + 1);
      Asc[strlen(Asc) - 1] = '\0';

   /* ein oder zwei Argumente ? */

      p = QuotPos(Asc, ',');
      if (p == NULL) {
         AdrPart = 0xf;
         if (strcasecmp(Asc, "X") == 0) AdrType = ModIX;
         else if (strcasecmp(Asc, "Y") == 0) {
            AdrType = ModIY;
            AddPrefix(0x90);
         } else WrXError(1445, Asc);
         ChkAdr(Mask);
         return;
      }

      strmaxcpy(Asc2, p + 1, 255);
      *p = '\0';

      if (strcasecmp(Asc, "X") == 0) {
         strmaxcpy(Asc, Asc2, 255);
         YReg = false;
      } else if (strcasecmp(Asc2, "X") == 0) YReg = false;
      else if (strcasecmp(Asc, "Y") == 0) {
         strmaxcpy(Asc, Asc2, 255);
         YReg = true;
      } else if (strcasecmp(Asc2, "Y") == 0) YReg = true;
      else {
         WrError(1350);
         return;
      }

   /* speicherindirekt ? */

      if ((*Asc == '[') && (Asc[strlen(Asc) - 1] == ']')) {
         strcpy(Asc, Asc + 1);
         Asc[strlen(Asc) - 1] = '\0';
         if (YReg) {
            DecideASize(Mask, Asc, ModIYAbs8, ModIYAbs16, 0xe, 0xd);
            if (AdrType != ModNone) AddPrefix(0x91);
         } else {
            DecideASize(Mask, Asc, ModIXAbs8, ModIXAbs16, 0xe, 0xd);
            if (AdrType != ModNone) AddPrefix(0x92);
         }
      } else {
         if (YReg) DecideSize(Mask, Asc, ModIY8, ModIY16, 0xe, 0xd);
         else DecideSize(Mask, Asc, ModIX8, ModIX16, 0xe, 0xd);
         if ((AdrType != ModNone) && (YReg)) AddPrefix(0x90);
      }

      ChkAdr(Mask);
      return;
   }

/* dann absolut */

   DecideSize(Mask, Asc, ModAbs8, ModAbs16, 0xb, 0xc);

   ChkAdr(Mask);
}

/*--------------------------------------------------------------------------*/

static bool DecodePseudo(void) {
   return false;
}

static void MakeCode_ST7(void) {
   Integer z, AdrInt;
   LongInt Mask;
   bool OK;

   CodeLen = 0;
   DontPrint = false;
   OpSize = 1;
   PrefixCnt = 0;

/* zu ignorierendes */

   if (Memo("")) return;

/* Attribut verarbeiten */

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

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(true)) return;
   if (DecodeMoto16Pseudo(OpSize, true)) return;

/* ohne Argument */

   for (z = 0; z < FixedOrderCnt; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else if (*AttrPart != '\0') WrError(1100);
         else {
            BAsmCode[PrefixCnt] = FixedOrders[z].Code;
            CodeLen = PrefixCnt + 1;
         }
         return;
      }

/* Datentransfer */

   if (Memo("LD")) {
      if (ArgCnt != 2) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else {
         DecodeAdr(ArgStr[1], MModA + MModX + MModY + MModS + MModImm + MModAbs8 + MModAbs16 + MModIX + MModIX8 + MModIX16 + MModIY + MModIY8 + MModIY16 + MModIAbs8 + MModIAbs16 + MModIXAbs8 + MModIXAbs16 + MModIYAbs8 + MModIYAbs16);

         switch (AdrType) {
            case ModA:
               DecodeAdr(ArgStr[2], MModImm + MModAbs8 + MModAbs16 + MModIX + MModIX8 + MModIX16 + MModIY + MModIY8 + MModIY16 + MModIAbs8 + MModIAbs16 + MModIXAbs8 + MModIXAbs16 + MModIYAbs8 + MModIYAbs16 + MModX + MModY + MModS);
               switch (AdrType) {
                  case ModX:
                  case ModY:
                     BAsmCode[PrefixCnt] = 0x9f;
                     CodeLen = PrefixCnt + 1;
                     break;
                  case ModS:
                     BAsmCode[PrefixCnt] = 0x9e;
                     CodeLen = PrefixCnt + 1;
                     break;
                  default:
                     if (AdrType != ModNone) {
                        BAsmCode[PrefixCnt] = 0x06 + (AdrPart << 4);
                        memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                        CodeLen = PrefixCnt + 1 + AdrCnt;
                     }
               }
               break;
            case ModX:
               DecodeAdr(ArgStr[2], MModImm + MModAbs8 + MModAbs16 + MModIX + MModIX8 + MModIX16 + MModIAbs8 + MModIAbs16 + MModIXAbs8 + MModIXAbs16 + MModA + MModY + MModS);
               switch (AdrType) {
                  case ModA:
                     BAsmCode[PrefixCnt] = 0x97;
                     CodeLen = PrefixCnt + 1;
                     break;
                  case ModY:
                     BAsmCode[0] = 0x93;
                     CodeLen = 1;
                     break;
                  case ModS:
                     BAsmCode[PrefixCnt] = 0x96;
                     CodeLen = PrefixCnt + 1;
                     break;
                  default:
                     if (AdrType != ModNone) {
                        BAsmCode[PrefixCnt] = 0x0e + (AdrPart << 4); /* ANSI :-O */
                        memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                        CodeLen = PrefixCnt + 1 + AdrCnt;
                     }
               }
               break;
            case ModY:
               PrefixCnt = 0;
               DecodeAdr(ArgStr[2], MModImm + MModAbs8 + MModAbs16 + MModIY + MModIY8 + MModIY16 + MModIAbs8 + MModIAbs16 + MModIYAbs8 + MModIYAbs16 + MModA + MModX + MModS);
               switch (AdrType) {
                  case ModA:
                     AddPrefix(0x90);
                     BAsmCode[PrefixCnt] = 0x97;
                     CodeLen = PrefixCnt + 1;
                     break;
                  case ModX:
                     AddPrefix(0x90);
                     BAsmCode[PrefixCnt] = 0x93;
                     CodeLen = PrefixCnt + 1;
                     break;
                  case ModS:
                     AddPrefix(0x90);
                     BAsmCode[PrefixCnt] = 0x96;
                     CodeLen = PrefixCnt + 1;
                     break;
                  default:
                     if (AdrType != ModNone) {
                        if (PrefixCnt == 0) AddPrefix(0x90);
                        if (BAsmCode[0] == 0x92) BAsmCode[0]--;
                        BAsmCode[PrefixCnt] = 0x0e + (AdrPart << 4); /* ANSI :-O */
                        memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                        CodeLen = PrefixCnt + 1 + AdrCnt;
                     }
               }
               break;
            case ModS:
               DecodeAdr(ArgStr[2], MModA + MModX + MModY);
               switch (AdrType) {
                  case ModA:
                     BAsmCode[PrefixCnt] = 0x95;
                     CodeLen = PrefixCnt + 1;
                     break;
                  case ModX:
                  case ModY:
                     BAsmCode[PrefixCnt] = 0x94;
                     CodeLen = PrefixCnt + 1;
                     break;
               }
               break;
            default:
               if (AdrType != ModNone) {
                  PrefixCnt = 0;
                  DecodeAdr(ArgStr[2], MModA + MModX + MModY);
                  switch (AdrType) {
                     case ModA:
                        Mask = MModAbs8 + MModAbs16 + MModIX + MModIX8 + MModIX16 + MModIY + MModIY8 + MModIY16 + MModIAbs8 + MModIAbs16 + MModIXAbs8 + MModIXAbs16 + MModIYAbs8 + MModIYAbs16;
                        DecodeAdr(ArgStr[1], Mask);
                        if (AdrType != ModNone) {
                           BAsmCode[PrefixCnt] = 0x07 + (AdrPart << 4);
                           memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                           CodeLen = PrefixCnt + 1 + AdrCnt;
                        }
                        break;
                     case ModX:
                        DecodeAdr(ArgStr[1], MModAbs8 + MModAbs16 + MModIX + MModIX8 + MModIX16 + MModIAbs8 + MModIAbs16 + MModIXAbs8 + MModIXAbs16);
                        if (AdrType != ModNone) {
                           BAsmCode[PrefixCnt] = 0x0f + (AdrPart << 4);
                           memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                           CodeLen = PrefixCnt + 1 + AdrCnt;
                        }
                        break;
                     case ModY:
                        PrefixCnt = 0;
                        DecodeAdr(ArgStr[1], MModAbs8 + MModAbs16 + MModIY + MModIY8 + MModIY16 + MModIAbs8 + MModIAbs16 + MModIYAbs8 + MModIYAbs16);
                        if (AdrType != ModNone) {
                           if (PrefixCnt == 0) AddPrefix(0x90);
                           if (BAsmCode[0] == 0x92) BAsmCode[0]--;
                           BAsmCode[PrefixCnt] = 0x0f + (AdrPart << 4);
                           memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                           CodeLen = PrefixCnt + 1 + AdrCnt;
                        }
                        break;
                  }
               }
         }
      }
      return;
   }

   if ((Memo("PUSH")) || (Memo("POP"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else {
         DecodeAdr(ArgStr[1], MModA + MModX + MModY + MModCCR);
         if (AdrType != ModNone) {
            switch (AdrType) {
               case ModA:
                  BAsmCode[PrefixCnt] = 0x84;
                  break;
               case ModX:
               case ModY:
                  BAsmCode[PrefixCnt] = 0x85;
                  break;
               case ModCCR:
                  BAsmCode[PrefixCnt] = 0x86;
                  break;
            }
            if (Memo("PUSH")) BAsmCode[PrefixCnt] += 4;
            CodeLen = PrefixCnt + 1;
         }
      }
      return;
   }

/* Arithmetik */

   if (Memo("CP")) {
      if (ArgCnt != 2) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else {
         DecodeAdr(ArgStr[1], MModA + MModX + MModY);
         switch (AdrType) {
            case ModA:
               Mask = MModImm + MModAbs8 + MModAbs16 + MModIX + MModIX8 + MModIX16 + MModIY + MModIY8 + MModIY16 + MModIAbs8 + MModIAbs16 + MModIXAbs8 + MModIXAbs16 + MModIYAbs8 + MModIYAbs16;
               DecodeAdr(ArgStr[2], Mask);
               if (AdrType != ModNone) {
                  BAsmCode[PrefixCnt] = 0x01 + (AdrPart << 4);
                  memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                  CodeLen = PrefixCnt + 1 + AdrCnt;
               }
               break;
            case ModX:
               DecodeAdr(ArgStr[2], MModImm + MModAbs8 + MModAbs16 + MModIX + MModIX8 + MModIX16 + MModIAbs8 + MModIAbs16 + MModIXAbs8 + MModIXAbs16);
               if (AdrType != ModNone) {
                  BAsmCode[PrefixCnt] = 0x03 + (AdrPart << 4);
                  memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                  CodeLen = PrefixCnt + 1 + AdrCnt;
               }
               break;
            case ModY:
               PrefixCnt = 0;
               DecodeAdr(ArgStr[2], MModImm + MModAbs8 + MModAbs16 + MModIY + MModIY8 + MModIY16 + MModIAbs8 + MModIAbs16 + MModIYAbs8 + MModIYAbs16);
               if (AdrType != ModNone) {
                  if (PrefixCnt == 0) AddPrefix(0x90);
                  if (BAsmCode[0] == 0x92) BAsmCode[0]--;
                  BAsmCode[PrefixCnt] = 0x03 + (AdrPart << 4);
                  memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                  CodeLen = PrefixCnt + 1 + AdrCnt;
               }
               break;
         }
      }
      return;
   }

   for (z = 0; z < AriOrderCnt; z++)
      if (Memo(AriOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (*AttrPart != '\0') WrError(1100);
         else {
            DecodeAdr(ArgStr[1], MModA);
            if (AdrType == ModA) {
               Mask = MModAbs8 + MModAbs16 + MModIX + MModIX8 + MModIX16 + MModIY + MModIY8 + MModIY16 + MModIAbs8 + MModIAbs16 + MModIXAbs8 + MModIXAbs16 + MModIYAbs8 + MModIYAbs16;
               if (AriOrders[z].MayImm) Mask += MModImm;
               DecodeAdr(ArgStr[2], Mask);
               if (AdrType != ModNone) {
                  BAsmCode[PrefixCnt] = AriOrders[z].Code + (AdrPart << 4);
                  memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                  CodeLen = PrefixCnt + 1 + AdrCnt;
               }
            }
         }
         return;
      }

   for (z = 0; z < RMWOrderCnt; z++)
      if (Memo(RMWOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (*AttrPart != '\0') WrError(1100);
         else {
            DecodeAdr(ArgStr[1], MModA + MModX + MModY + MModAbs8 + MModIX + MModIX8 + MModIY + MModIY8 + MModIAbs8 + MModIXAbs8 + MModIYAbs8);
            switch (AdrType) {
               case ModA:
                  BAsmCode[PrefixCnt] = 0x40 + RMWOrders[z].Code;
                  CodeLen = PrefixCnt + 1;
                  break;
               case ModX:
               case ModY:
                  BAsmCode[PrefixCnt] = 0x50 + RMWOrders[z].Code;
                  CodeLen = PrefixCnt + 1;
                  break;
               default:
                  if (AdrType != ModNone) {
                     BAsmCode[PrefixCnt] = RMWOrders[z].Code + ((AdrPart - 8) << 4);
                     memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                     CodeLen = PrefixCnt + 1 + AdrCnt;
                  }
            }
         }
         return;
      }

   if (Memo("MUL")) {
      if (ArgCnt != 2) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else {
         DecodeAdr(ArgStr[2], MModA);
         if (AdrType != ModNone) {
            DecodeAdr(ArgStr[1], MModX + MModY);
            if (AdrType != ModNone) {
               BAsmCode[PrefixCnt] = 0x42;
               CodeLen = PrefixCnt + 1;
            }
         }
      }
      return;
   }

/* Bitbefehle */

   if ((Memo("BRES")) || (Memo("BSET"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (*ArgStr[2] != '#') WrError(1350);
      else {
         z = EvalIntExpression(ArgStr[2] + 1, UInt3, &OK);
         if (OK) {
            DecodeAdr(ArgStr[1], MModAbs8 + MModIAbs8);
            if (AdrType != ModNone) {
               BAsmCode[PrefixCnt] = 0x10 + Memo("BRES") + (z << 1);
               memcpy(BAsmCode + 1 + PrefixCnt, AdrVals, AdrCnt);
               CodeLen = PrefixCnt + 1 + AdrCnt;
            }
         }
      }
      return;
   }

   if ((Memo("BTJF")) || (Memo("BTJT"))) {
      if (ArgCnt != 3) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (*ArgStr[2] != '#') WrError(1350);
      else {
         z = EvalIntExpression(ArgStr[2] + 1, UInt3, &OK);
         if (OK) {
            DecodeAdr(ArgStr[1], MModAbs8 + MModIAbs8);
            if (AdrType != ModNone) {
               BAsmCode[PrefixCnt] = 0x00 + Memo("BTJF") + (z << 1);
               memcpy(BAsmCode + 1 + PrefixCnt, AdrVals, AdrCnt);
               AdrInt = EvalIntExpression(ArgStr[3], UInt16, &OK) - (EProgCounter() + PrefixCnt + 1 + AdrCnt);
               if (OK)
                  if ((!SymbolQuestionable) && ((AdrInt < -128) || (AdrInt > 127))) WrError(1370);
                  else {
                     BAsmCode[PrefixCnt + 1 + AdrCnt] = AdrInt & 0xff;
                     CodeLen = PrefixCnt + 1 + AdrCnt + 1;
                  }
            }
         }
      }
      return;
   }

/* Spruenge */

   if ((Memo("JP")) || (Memo("CALL"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else {
         Mask = MModAbs8 + MModAbs16 + MModIX + MModIX8 + MModIX16 + MModIY + MModIY8 + MModIY16 + MModIAbs8 + MModIAbs16 + MModIXAbs8 + MModIXAbs16 + MModIYAbs8 + MModIYAbs16;
         DecodeAdr(ArgStr[1], Mask);
         if (AdrType != ModNone) {
            BAsmCode[PrefixCnt] = 0x0c + Memo("CALL") + (AdrPart << 4);
            memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
            CodeLen = PrefixCnt + 1 + AdrCnt;
         }
      }
      return;
   }

   for (z = 0; z < RelOrderCnt; z++)
      if (Memo(RelOrders[z].Name)) {
         if (*AttrPart != '\0') WrError(1100);
         else if (ArgCnt != 1) WrError(1110);
         else if (*ArgStr[1] == '[') {
            DecodeAdr(ArgStr[1], MModIAbs8);
            if (AdrType != ModNone) {
               BAsmCode[PrefixCnt] = RelOrders[z].Code;
               memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
               CodeLen = PrefixCnt + 1 + AdrCnt;
            }
         } else {
            AdrInt = EvalIntExpression(ArgStr[1], UInt16, &OK) - (EProgCounter() + 2);
            if (OK)
               if ((!SymbolQuestionable) && ((AdrInt < -128) || (AdrInt > 127))) WrError(1370);
               else {
                  BAsmCode[0] = RelOrders[z].Code;
                  BAsmCode[1] = AdrInt & 0xff;
                  CodeLen = 2;
               }
         }
         return;
      }

/* nix gefunden */

   WrXError(1200, OpPart);
}

static bool ChkPC_ST7(void) {
   switch (ActPC) {
      case SegCode:
         return (ProgCounter() < 0x10000);
      default:
         return false;
   }
}

static bool IsDef_ST7(void) {
   return false;
}

static void SwitchFrom_ST7(void) {
   DeinitFields();
}

static void SwitchTo_ST7(void) {
   TurnWords = false;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "PC";
   HeaderID = 0x33;
   NOPCode = 0x9d;
   DivideChars = ",";
   HasAttrs = true;
   AttrChars = ".";

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_ST7;
   ChkPC = ChkPC_ST7;
   IsDef = IsDef_ST7;
   SwitchFrom = SwitchFrom_ST7;

   InitFields();
}

void codest7_init(void) {
   CPUST7 = AddCPU("ST7", SwitchTo_ST7);
}
