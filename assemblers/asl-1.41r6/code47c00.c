/* code47c00.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Toshiba TLCS-47(0(A))                                       */
/*                                                                           */
/* Historie: 30.12.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

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
   Byte Code;
} FixedOrder;

#define FixedOrderCnt 3
#define BitOrderCnt 4

#define ModNone (-1)
#define ModAcc 0
#define MModAcc (1 << ModAcc)
#define ModL 1
#define MModL (1 << ModL)
#define ModH 2
#define MModH (1 << ModH)
#define ModHL 3
#define MModHL (1 << ModHL)
#define ModIHL 4
#define MModIHL (1 << ModIHL)
#define ModAbs 5
#define MModAbs (1 << ModAbs)
#define ModPort 6
#define MModPort (1 << ModPort)
#define ModImm 7
#define MModImm (1 << ModImm)
#define ModSAbs 8
#define MModSAbs (1 << ModSAbs)

static CPUVar CPU47C00, CPU470C00, CPU470AC00;
static ShortInt AdrType, OpSize;
static Byte AdrVal;
static LongInt DMBAssume;
static SimpProc SaveInitProc;

static FixedOrder *FixedOrders;
static char **BitOrders;

/*---------------------------------------------------------------------------*/

static void AddFixed(char *NName, Byte NCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("RET", 0x2a);
   AddFixed("RETI", 0x2b);
   AddFixed("NOP", 0x00);

   BitOrders = (char **)malloc(sizeof(char *) * BitOrderCnt);
   InstrZ = 0;
   BitOrders[InstrZ++] = "SET";
   BitOrders[InstrZ++] = "CLR";
   BitOrders[InstrZ++] = "TEST";
   BitOrders[InstrZ++] = "TESTP";
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(BitOrders);
}

/*---------------------------------------------------------------------------*/

static Word RAMEnd(void) {
   if (MomCPU == CPU47C00) return 0xff;
   else if (MomCPU == CPU470C00) return 0x1ff;
   else return 0x3ff;
}

static Word ROMEnd(void) {
   if (MomCPU == CPU47C00) return 0xfff;
   else if (MomCPU == CPU470C00) return 0x1fff;
   else return 0x3fff;
}

static Word PortEnd(void) {
   if (MomCPU == CPU47C00) return 0x0f;
   else return 0x1f;
}

static void SetOpSize(ShortInt NewSize) {
   if (OpSize == -1) OpSize = NewSize;
   else if (OpSize != NewSize) {
      WrError(1131);
      AdrType = ModNone;
   }
}

static void ChkAdr(Word Mask) {
   if ((AdrType != ModNone) && (((1 << AdrType) & Mask) == 0)) {
      WrError(1350);
      AdrType = ModNone;
   }
}

static void DecodeAdr(char *Asc, Word Mask) {
   static char *RegNames[ModIHL + 1] = { "A", "L", "H", "HL", "@HL" };

   Byte z;
   Word AdrWord;
   bool OK;

   AdrType = ModNone;

   for (z = 0; z <= ModIHL; z++)
      if (strcasecmp(Asc, RegNames[z]) == 0) {
         AdrType = z;
         if (z != ModIHL) SetOpSize(z == ModHL);
         ChkAdr(Mask);
         return;
      }

   if (*Asc == '#') {
      switch (OpSize) {
         case -1:
            WrError(1132);
            break;
         case 2:
            AdrVal = EvalIntExpression(Asc + 1, UInt2, &OK) & 3;
            if (OK) AdrType = ModImm;
            break;
         case 0:
            AdrVal = EvalIntExpression(Asc + 1, Int4, &OK) & 15;
            if (OK) AdrType = ModImm;
            break;
         case 1:
            AdrVal = EvalIntExpression(Asc + 1, Int8, &OK);
            if (OK) AdrType = ModImm;
            break;
      }
      ChkAdr(Mask);
      return;
   }

   if (*Asc == '%') {
      AdrVal = EvalIntExpression(Asc + 1, Int5, &OK);
      if (OK) {
         AdrType = ModPort;
         ChkSpace(SegIO);
      }
      ChkAdr(Mask);
      return;
   }

   FirstPassUnknown = false;
   AdrWord = EvalIntExpression(Asc, Int16, &OK);
   if (OK) {
      ChkSpace(SegData);

      if (FirstPassUnknown) AdrWord &= RAMEnd();
      else if (Hi(AdrWord) != DMBAssume) WrError(110);

      AdrVal = Lo(AdrWord);
      if (FirstPassUnknown) AdrVal &= 15;

      if (((Mask & MModSAbs) != 0) && (AdrVal < 16))
         AdrType = ModSAbs;
      else AdrType = ModAbs;
   }

   ChkAdr(Mask);
}

/*--------------------------------------------------------------------------*/

static bool DecodePseudo(void) {
#define ASSUME47Count 1
   static ASSUMERec ASSUME47s[ASSUME47Count] = { { "DMB", &DMBAssume, 0, 3, 4 } };

   if (Memo("PORT")) {
      CodeEquate(SegIO, 0, 0x1f);
      return true;
   }

   if (Memo("ASSUME")) {
      CodeASSUME(ASSUME47s, ASSUME47Count);
      return true;
   }

   return false;
}

static void ChkCPU(Byte Mask) {
   Byte NMask = (1 << (MomCPU - CPU47C00));

/* Don't ask me why, but NetBSD/Sun3 doesn't like writing
   everything in one formula when using -O3 :-( */

   if ((Mask & NMask) == 0) {
      WrError(1500);
      CodeLen = 0;
   }
}

static bool DualOp(char *s1, char *s2) {
   return (((strcasecmp(ArgStr[1], s1) == 0) && (strcasecmp(ArgStr[2], s2) == 0))
      || ((strcasecmp(ArgStr[2], s1) == 0) && (strcasecmp(ArgStr[1], s2) == 0)));
}

static void MakeCode_47C00(void) {
   bool OK;
   Word AdrWord;
   Integer z;
   Byte HReg;

   CodeLen = 0;
   DontPrint = false;
   OpSize = (-1);

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodeIntelPseudo(false)) return;

   if (DecodePseudo()) return;

/* ohne Argument */

   for (z = 0; z < FixedOrderCnt; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else {
            CodeLen = 1;
            BAsmCode[0] = FixedOrders[z].Code;
         }
         return;
      }

/* Datentransfer */

   if (Memo("LD")) {
      if (ArgCnt != 2) WrError(1110);
      else if (strcasecmp(ArgStr[1], "DMB") == 0) {
         SetOpSize(2);
         DecodeAdr(ArgStr[2], MModImm + MModIHL);
         switch (AdrType) {
            case ModIHL:
               CodeLen = 3;
               BAsmCode[0] = 0x03;
               BAsmCode[1] = 0x3a;
               BAsmCode[2] = 0xe9;
               ChkCPU(4);
               break;
            case ModImm:
               CodeLen = 3;
               BAsmCode[0] = 0x03;
               BAsmCode[1] = 0x2c;
               BAsmCode[2] = 0x09 + (AdrVal << 4);
               ChkCPU(4);
               break;
         }
      } else {
         DecodeAdr(ArgStr[1], MModAcc + MModHL + MModH + MModL);
         switch (AdrType) {
            case ModAcc:
               DecodeAdr(ArgStr[2], MModIHL + MModAbs + MModImm);
               switch (AdrType) {
                  case ModIHL:
                     CodeLen = 1;
                     BAsmCode[0] = 0x0c;
                     break;
                  case ModAbs:
                     CodeLen = 2;
                     BAsmCode[0] = 0x3c;
                     BAsmCode[1] = AdrVal;
                     break;
                  case ModImm:
                     CodeLen = 1;
                     BAsmCode[0] = 0x40 + AdrVal;
                     break;
               }
               break;
            case ModHL:
               DecodeAdr(ArgStr[2], MModAbs + MModImm);
               switch (AdrType) {
                  case ModAbs:
                     if ((AdrVal & 3) != 0) WrError(1325);
                     else {
                        CodeLen = 2;
                        BAsmCode[0] = 0x28;
                        BAsmCode[1] = AdrVal;
                     }
                     break;
                  case ModImm:
                     CodeLen = 2;
                     BAsmCode[0] = 0xc0 + (AdrVal >> 4);
                     BAsmCode[1] = 0xe0 + (AdrVal & 15);
                     break;
               }
               break;
            case ModH:
            case ModL:
               BAsmCode[0] = 0xc0 + ((AdrType == ModL) << 5);
               DecodeAdr(ArgStr[2], MModImm);
               if (AdrType != ModNone) {
                  CodeLen = 1;
                  BAsmCode[0] += AdrVal;
               }
               break;
         }
      }
      return;
   }

   if (Memo("LDL")) {
      if (ArgCnt != 2) WrError(1110);
      else if ((strcasecmp(ArgStr[1], "A") != 0) || (strcasecmp(ArgStr[2], "@DC") != 0)) WrError(1350);
      else {
         CodeLen = 1;
         BAsmCode[0] = 0x33;
      }
      return;
   }

   if (Memo("LDH")) {
      if (ArgCnt != 2) WrError(1110);
      else if ((strcasecmp(ArgStr[1], "A") != 0) || (strcasecmp(ArgStr[2], "@DC+") != 0)) WrError(1350);
      else {
         CodeLen = 1;
         BAsmCode[0] = 0x32;
      }
      return;
   }

   if (Memo("ST")) {
      if (ArgCnt != 2) WrError(1110);
      else if (strcasecmp(ArgStr[1], "DMB") == 0) {
         DecodeAdr(ArgStr[2], MModIHL);
         if (AdrType != ModNone) {
            CodeLen = 3;
            BAsmCode[0] = 0x03;
            BAsmCode[1] = 0x3a;
            BAsmCode[2] = 0x69;
            ChkCPU(4);
         }
      } else {
         OpSize = 0;
         DecodeAdr(ArgStr[1], MModImm + MModAcc);
         switch (AdrType) {
            case ModAcc:
               if (strcasecmp(ArgStr[2], "@HL+") == 0) {
                  CodeLen = 1;
                  BAsmCode[0] = 0x1a;
               } else if (strcasecmp(ArgStr[2], "@HL-") == 0) {
                  CodeLen = 1;
                  BAsmCode[0] = 0x1b;
               } else {
                  DecodeAdr(ArgStr[2], MModAbs + MModIHL);
                  switch (AdrType) {
                     case ModAbs:
                        CodeLen = 2;
                        BAsmCode[0] = 0x3f;
                        BAsmCode[1] = AdrVal;
                        break;
                     case ModIHL:
                        CodeLen = 1;
                        BAsmCode[0] = 0x0f;
                        break;
                  }
               }
               break;
            case ModImm:
               HReg = AdrVal;
               if (strcasecmp(ArgStr[2], "@HL+") == 0) {
                  CodeLen = 1;
                  BAsmCode[0] = 0xf0 + HReg;
               } else {
                  DecodeAdr(ArgStr[2], MModSAbs);
                  if (AdrType != ModNone)
                     if (AdrVal > 0x0f) WrError(1320);
                     else {
                        CodeLen = 2;
                        BAsmCode[0] = 0x2d;
                        BAsmCode[1] = (HReg << 4) + AdrVal;
                     }
               }
               break;
         }
      }
      return;
   }

   if (Memo("MOV")) {
      if (ArgCnt != 2) WrError(1110);
      else if ((strcasecmp(ArgStr[1], "A") == 0) && (strcasecmp(ArgStr[2], "DMB") == 0)) {
         CodeLen = 3;
         BAsmCode[0] = 0x03;
         BAsmCode[1] = 0x3a;
         BAsmCode[2] = 0xa9;
         ChkCPU(4);
      } else if ((strcasecmp(ArgStr[1], "DMB") == 0) && (strcasecmp(ArgStr[2], "A") == 0)) {
         CodeLen = 3;
         BAsmCode[0] = 0x03;
         BAsmCode[1] = 0x3a;
         BAsmCode[2] = 0x29;
         ChkCPU(4);
      } else if ((strcasecmp(ArgStr[1], "A") == 0) && (strcasecmp(ArgStr[2], "SPW13") == 0)) {
         CodeLen = 2;
         BAsmCode[0] = 0x3a;
         BAsmCode[1] = 0x84;
         ChkCPU(4);
      } else if ((strcasecmp(ArgStr[1], "STK13") == 0) && (strcasecmp(ArgStr[2], "A") == 0)) {
         CodeLen = 2;
         BAsmCode[0] = 0x3a;
         BAsmCode[1] = 0x04;
         ChkCPU(4);
      } else if (strcasecmp(ArgStr[2], "A") != 0) WrError(1350);
      else {
         DecodeAdr(ArgStr[1], MModH + MModL);
         if (AdrType != ModNone) {
            CodeLen = 1;
            BAsmCode[0] = 0x10 + (AdrType == ModL);
         }
      }
      return;
   }

   if (Memo("XCH")) {
      if (ArgCnt != 2) WrError(1110);
      else if (DualOp("A", "EIR")) {
         CodeLen = 1;
         BAsmCode[0] = 0x13;
      } else if (DualOp("A", "@HL")) {
         CodeLen = 1;
         BAsmCode[0] = 0x0d;
      } else if (DualOp("A", "H")) {
         CodeLen = 1;
         BAsmCode[0] = 0x30;
      } else if (DualOp("A", "L")) {
         CodeLen = 1;
         BAsmCode[0] = 0x31;
      } else {
         if ((strcasecmp(ArgStr[1], "A") != 0) && (strcasecmp(ArgStr[1], "HL") != 0)) {
            strcpy(ArgStr[3], ArgStr[1]);
            strcpy(ArgStr[1], ArgStr[2]);
            strcpy(ArgStr[2], ArgStr[3]);
         }
         if ((strcasecmp(ArgStr[1], "A") != 0) && (strcasecmp(ArgStr[1], "HL") != 0)) WrError(1350);
         else {
            DecodeAdr(ArgStr[2], MModAbs);
            if (AdrType != ModNone)
               if ((strcasecmp(ArgStr[1], "HL") == 0) && ((AdrVal & 3) != 0)) WrError(1325);
               else {
                  CodeLen = 2;
                  BAsmCode[0] = 0x29 + (0x14 * (strcasecmp(ArgStr[1], "A") == 0));
                  BAsmCode[1] = AdrVal;
               }
         }
      }
      return;
   }

   if (Memo("IN")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModPort);
         if (AdrType != ModNone) {
            HReg = AdrVal;
            DecodeAdr(ArgStr[2], MModAcc + MModIHL);
            switch (AdrType) {
               case ModAcc:
                  CodeLen = 2;
                  BAsmCode[0] = 0x3a;
                  BAsmCode[1] = (HReg & 0x0f) + (((HReg & 0x10) ^ 0x10) << 1);
                  break;
               case ModIHL:
                  CodeLen = 2;
                  BAsmCode[0] = 0x3a;
                  BAsmCode[1] = 0x40 + (HReg & 0x0f) + (((HReg & 0x10) ^ 0x10) << 1);
                  break;
            }
         }
      }
      return;
   }

   if (Memo("OUT")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], MModPort);
         if (AdrType != ModNone) {
            HReg = AdrVal;
            OpSize = 0;
            DecodeAdr(ArgStr[1], MModAcc + MModIHL + MModImm);
            switch (AdrType) {
               case ModAcc:
                  CodeLen = 2;
                  BAsmCode[0] = 0x3a;
                  BAsmCode[1] = 0x80 + ((HReg & 0x10) << 1) + ((HReg & 0x0f) ^ 4);
                  break;
               case ModIHL:
                  CodeLen = 2;
                  BAsmCode[0] = 0x3a;
                  BAsmCode[1] = 0xc0 + ((HReg & 0x10) << 1) + ((HReg & 0x0f) ^ 4);
                  break;
               case ModImm:
                  if (HReg > 0x0f) WrError(1110);
                  else {
                     CodeLen = 2;
                     BAsmCode[0] = 0x2c;
                     BAsmCode[1] = (AdrVal << 4) + HReg;
                  }
                  break;
            }
         }
      }
      return;
   }

   if (Memo("OUTB")) {
      if (ArgCnt != 1) WrError(1110);
      else if (strcasecmp(ArgStr[1], "@HL") != 0) WrError(1350);
      else {
         CodeLen = 1;
         BAsmCode[0] = 0x12;
      }
      return;
   }

/* Arithmetik */

   if (Memo("CMPR")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModAcc + MModSAbs + MModH + MModL);
         switch (AdrType) {
            case ModAcc:
               DecodeAdr(ArgStr[2], MModIHL + MModAbs + MModImm);
               switch (AdrType) {
                  case ModIHL:
                     CodeLen = 1;
                     BAsmCode[0] = 0x16;
                     break;
                  case ModAbs:
                     CodeLen = 2;
                     BAsmCode[0] = 0x3e;
                     BAsmCode[1] = AdrVal;
                     break;
                  case ModImm:
                     CodeLen = 1;
                     BAsmCode[0] = 0xd0 + AdrVal;
                     break;
               }
               break;
            case ModSAbs:
               OpSize = 0;
               HReg = AdrVal;
               DecodeAdr(ArgStr[2], MModImm);
               if (AdrType != ModNone) {
                  CodeLen = 2;
                  BAsmCode[0] = 0x2e;
                  BAsmCode[1] = (AdrVal << 4) + HReg;
               }
               break;
            case ModH:
            case ModL:
               HReg = AdrType;
               DecodeAdr(ArgStr[2], MModImm);
               if (AdrType != ModNone) {
                  CodeLen = 2;
                  BAsmCode[0] = 0x38;
                  BAsmCode[1] = 0x90 + ((HReg == ModH) << 6) + AdrVal;
               }
               break;
         }
      }
      return;
   }

   if (Memo("ADD")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModAcc + MModIHL + MModSAbs + MModL + MModH);
         switch (AdrType) {
            case ModAcc:
               DecodeAdr(ArgStr[2], MModIHL + MModImm);
               switch (AdrType) {
                  case ModIHL:
                     CodeLen = 1;
                     BAsmCode[0] = 0x17;
                     break;
                  case ModImm:
                     CodeLen = 2;
                     BAsmCode[0] = 0x38;
                     BAsmCode[1] = AdrVal;
                     break;
               }
               break;
            case ModIHL:
               OpSize = 0;
               DecodeAdr(ArgStr[2], MModImm);
               if (AdrType != ModNone) {
                  CodeLen = 2;
                  BAsmCode[0] = 0x38;
                  BAsmCode[1] = 0x40 + AdrVal;
               }
               break;
            case ModSAbs:
               HReg = AdrVal;
               OpSize = 0;
               DecodeAdr(ArgStr[2], MModImm);
               if (AdrType != ModNone) {
                  CodeLen = 2;
                  BAsmCode[0] = 0x2f;
                  BAsmCode[1] = (AdrVal << 4) + HReg;
               }
               break;
            case ModH:
            case ModL:
               HReg = AdrType == ModH;
               DecodeAdr(ArgStr[2], MModImm);
               if (AdrType != ModNone) {
                  CodeLen = 2;
                  BAsmCode[0] = 0x38;
                  BAsmCode[1] = 0x80 + (HReg << 6) + AdrVal;
               }
               break;
         }
      }
      return;
   }

   if (Memo("ADDC")) {
      if (ArgCnt != 2) WrError(1110);
      else if ((strcasecmp(ArgStr[1], "A") != 0) || (strcasecmp(ArgStr[2], "@HL") != 0)) WrError(1350);
      else {
         CodeLen = 1;
         BAsmCode[0] = 0x15;
      }
      return;
   }

   if (Memo("SUBRC")) {
      if (ArgCnt != 2) WrError(1110);
      else if ((strcasecmp(ArgStr[1], "A") != 0) || (strcasecmp(ArgStr[2], "@HL") != 0)) WrError(1350);
      else {
         CodeLen = 1;
         BAsmCode[0] = 0x14;
      }
      return;
   }

   if (Memo("SUBR")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         OpSize = 0;
         DecodeAdr(ArgStr[2], MModImm);
         if (AdrType != ModNone) {
            HReg = AdrVal;
            DecodeAdr(ArgStr[1], MModAcc + MModIHL);
            switch (AdrType) {
               case ModAcc:
                  CodeLen = 2;
                  BAsmCode[0] = 0x38;
                  BAsmCode[1] = 0x10 + HReg;
                  break;
               case ModIHL:
                  CodeLen = 2;
                  BAsmCode[0] = 0x38;
                  BAsmCode[1] = 0x50 + HReg;
                  break;
            }
         }
      }
      return;
   }

   if ((Memo("INC")) || (Memo("DEC"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         HReg = Memo("DEC");
         DecodeAdr(ArgStr[1], MModAcc + MModIHL + MModL);
         switch (AdrType) {
            case ModAcc:
               CodeLen = 1;
               BAsmCode[0] = 0x08 + HReg;
               break;
            case ModL:
               CodeLen = 1;
               BAsmCode[0] = 0x18 + HReg;
               break;
            case ModIHL:
               CodeLen = 1;
               BAsmCode[0] = 0x0a + HReg;
               break;
         }
      }
      return;
   }

/* Logik */

   if ((Memo("AND")) || (Memo("OR"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         HReg = Memo("OR");
         DecodeAdr(ArgStr[1], MModAcc + MModIHL);
         switch (AdrType) {
            case ModAcc:
               DecodeAdr(ArgStr[2], MModImm + MModIHL);
               switch (AdrType) {
                  case ModIHL:
                     CodeLen = 1;
                     BAsmCode[0] = 0x1e - HReg; /* ANSI :-0 */
                     break;
                  case ModImm:
                     CodeLen = 2;
                     BAsmCode[0] = 0x38;
                     BAsmCode[1] = 0x30 - (HReg << 4) + AdrVal;
                     break;
               }
               break;
            case ModIHL:
               SetOpSize(0);
               DecodeAdr(ArgStr[2], MModImm);
               if (AdrType != ModNone) {
                  CodeLen = 2;
                  BAsmCode[0] = 0x38;
                  BAsmCode[1] = 0x70 - (HReg << 4) + AdrVal;
               }
               break;
         }
      }
      return;
   }

   if (Memo("XOR")) {
      if (ArgCnt != 2) WrError(1110);
      else if ((strcasecmp(ArgStr[1], "A") != 0) || (strcasecmp(ArgStr[2], "@HL") != 0)) WrError(1350);
      else {
         CodeLen = 1;
         BAsmCode[0] = 0x1f;
      }
      return;
   }

   if ((Memo("ROLC")) || (Memo("RORC"))) {
      if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
      else if (strcasecmp(ArgStr[1], "A") != 0) WrError(1350);
      else {
         if (ArgCnt == 1) {
            HReg = 1;
            OK = true;
         } else HReg = EvalIntExpression(ArgStr[2], Int8, &OK);
         if (OK) {
            BAsmCode[0] = 0x05 + (Memo("RORC") << 1);
            for (z = 1; z < HReg; z++) BAsmCode[z] = BAsmCode[0];
            CodeLen = HReg;
            if (HReg >= 4) WrError(160);
         }
      }
      return;
   }

   for (z = 0; z < BitOrderCnt; z++)
      if (Memo(BitOrders[z])) {
         if (ArgCnt == 1)
            if (strcasecmp(ArgStr[1], "@L") == 0) {
               if (Memo("TESTP")) WrError(1350);
               else {
                  if (z == 2) z = 3;
                  CodeLen = 1;
                  BAsmCode[0] = 0x34 + z;
               }
            } else if (strcasecmp(ArgStr[1], "CF") == 0) {
               if (z < 2) WrError(1350);
               else {
                  CodeLen = 1;
                  BAsmCode[0] = 10 - 2 * z;
               }
            } else if (strcasecmp(ArgStr[1], "ZF") == 0) {
               if (z != 3) WrError(1350);
               else {
                  CodeLen = 1;
                  BAsmCode[0] = 0x0e;
               }
            } else if (strcasecmp(ArgStr[1], "GF") == 0) {
               if (z == 2) WrError(1350);
               else {
                  CodeLen = 1;
                  BAsmCode[0] = (z == 3) ? 1 : 3 - z;
                  ChkCPU(1);
               }
            } else if ((strcasecmp(ArgStr[1], "DMB") == 0) || (strcasecmp(ArgStr[1], "DMB0") == 0)) {
               CodeLen = 2;
               BAsmCode[0] = 0x3b;
               BAsmCode[1] = 0x39 + (z << 6);
               ChkCPU(1 << (1 + (strcasecmp(ArgStr[1], "DMB0") == 0)));
            } else if (strcasecmp(ArgStr[1], "DMB1") == 0) {
               CodeLen = 3;
               BAsmCode[0] = 3;
               BAsmCode[1] = 0x3b;
               BAsmCode[2] = 0x19 + (z << 6);
               ChkCPU(4);
            } else if (strcasecmp(ArgStr[1], "STK13") == 0) {
               if (z > 1) WrError(1350);
               else {
                  CodeLen = 3;
                  BAsmCode[0] = 3 - z;
                  BAsmCode[1] = 0x3a;
                  BAsmCode[2] = 0x84;
                  ChkCPU(4);
               }
            } else WrError(1350);
         else if (ArgCnt == 2)
            if (strcasecmp(ArgStr[1], "IL") == 0) {
               if (z != 1) WrError(1350);
               else {
                  HReg = EvalIntExpression(ArgStr[2], UInt6, &OK);
                  if (OK) {
                     CodeLen = 2;
                     BAsmCode[0] = 0x36;
                     BAsmCode[1] = 0xc0 + HReg;
                  }
               }
            } else {
               HReg = EvalIntExpression(ArgStr[2], UInt2, &OK);
               if (OK) {
                  DecodeAdr(ArgStr[1], MModAcc + MModIHL + MModPort + MModSAbs);
                  switch (AdrType) {
                     case ModAcc:
                        if (z != 2) WrError(1350);
                        else {
                           CodeLen = 1;
                           BAsmCode[0] = 0x5c + HReg;
                        }
                        break;
                     case ModIHL:
                        if (z == 3) WrError(1350);
                        else {
                           CodeLen = 1;
                           BAsmCode[0] = 0x50 + HReg + (z << 2);
                        }
                        break;
                     case ModPort:
                        if (AdrVal > 15) WrError(1320);
                        else {
                           CodeLen = 2;
                           BAsmCode[0] = 0x3b;
                           BAsmCode[1] = (z << 6) + (HReg << 4) + AdrVal;
                        }
                        break;
                     case ModSAbs:
                        CodeLen = 2;
                        BAsmCode[0] = 0x39;
                        BAsmCode[1] = (z << 6) + (HReg << 4) + AdrVal;
                        break;
                  }
               }
         } else WrError(1110);
         return;
      }

   if ((Memo("EICLR")) || (Memo("DICLR"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (strcasecmp(ArgStr[1], "IL") != 0) WrError(1350);
      else {
         BAsmCode[1] = EvalIntExpression(ArgStr[2], UInt6, &OK);
         if (OK) {
            CodeLen = 2;
            BAsmCode[0] = 0x36;
            BAsmCode[1] += 0x40 * (1 + Memo("DICLR"));
         }
      }
      return;
   }

/* Spruenge */

   if (Memo("BSS")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
         if (OK)
            if ((!SymbolQuestionable) && ((AdrWord >> 6) != ((EProgCounter() + 1) >> 6))) WrError(1910);
            else {
               ChkSpace(SegCode);
               CodeLen = 1;
               BAsmCode[0] = 0x80 + (AdrWord & 0x3f);
            }
      }
      return;
   }

   if (Memo("BS")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
         if (OK)
            if (((!SymbolQuestionable) && (AdrWord >> 12) != ((EProgCounter() + 2) >> 12))) WrError(1910);
            else {
               ChkSpace(SegCode);
               CodeLen = 2;
               BAsmCode[0] = 0x60 + (Hi(AdrWord) & 15);
               BAsmCode[1] = Lo(AdrWord);
            }
      }
      return;
   }

   if (Memo("BSL")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
         if (OK)
            if (AdrWord > ROMEnd()) WrError(1320);
            else {
               ChkSpace(SegCode);
               CodeLen = 3;
               switch (AdrWord >> 12) {
                  case 0:
                     BAsmCode[0] = 0x02;
                     break;
                  case 1:
                     BAsmCode[0] = 0x03;
                     break;
                  case 2:
                     BAsmCode[0] = 0x1c;
                     break;
                  case 3:
                     BAsmCode[0] = 0x01;
                     break;
               }
               BAsmCode[1] = 0x60 + (Hi(AdrWord) & 0x0f);
               BAsmCode[2] = Lo(AdrWord);
            }
      }
      return;
   }

   if (Memo("B")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
         if (OK)
            if (AdrWord > ROMEnd()) WrError(1320);
            else {
               ChkSpace(SegCode);
               if ((AdrWord >> 6) == ((EProgCounter() + 1) >> 6)) {
                  CodeLen = 1;
                  BAsmCode[0] = 0x80 + (AdrWord & 0x3f);
               } else if ((AdrWord >> 12) == ((EProgCounter() + 2) >> 12)) {
                  CodeLen = 2;
                  BAsmCode[0] = 0x60 + (Hi(AdrWord) & 0x0f);
                  BAsmCode[1] = Lo(AdrWord);
               } else {
                  CodeLen = 3;
                  switch (AdrWord >> 12) {
                     case 0:
                        BAsmCode[0] = 0x02;
                        break;
                     case 1:
                        BAsmCode[0] = 0x03;
                        break;
                     case 2:
                        BAsmCode[0] = 0x1c;
                        break;
                     case 3:
                        BAsmCode[0] = 0x01;
                        break;
                  }
                  BAsmCode[1] = 0x60 + (Hi(AdrWord) & 0x0f);
                  BAsmCode[2] = Lo(AdrWord);
               }
            }
      }
      return;
   }

   if (Memo("CALLS")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
         if (OK) {
            if (AdrWord == 0x86) AdrWord = 0x06;
            if ((AdrWord & 0xff87) != 6) WrError(1135);
            else {
               CodeLen = 1;
               BAsmCode[0] = (AdrWord >> 3) + 0x70;
            }
         }
      }
      return;
   }

   if (Memo("CALL")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
         if (OK)
            if ((!SymbolQuestionable) && (((AdrWord ^ EProgCounter()) & 0x3800) != 0)) WrError(1910);
            else {
               ChkSpace(SegCode);
               CodeLen = 2;
               BAsmCode[0] = 0x20 + (Hi(AdrWord) & 7);
               BAsmCode[1] = Lo(AdrWord);
            }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_47C00(void) {
   switch (ActPC) {
      case SegCode:
         return (ProgCounter() <= ROMEnd());
      case SegData:
         return (ProgCounter() <= RAMEnd());
      case SegIO:
         return (ProgCounter() <= PortEnd());
      default:
         return false;
   }
}

static bool IsDef_47C00(void) {
   return (Memo("PORT"));
}

static void SwitchFrom_47C00(void) {
   DeinitFields();
}

static void SwitchTo_47C00(void) {
   TurnWords = false;
   ConstMode = ConstModeIntel;
   SetIsOccupied = true;

   PCSymbol = "$";
   HeaderID = 0x55;
   NOPCode = 0x00;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = (1 << SegCode) | (1 << SegData) | (1 << SegIO);
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;
   Grans[SegData] = 1;
   ListGrans[SegData] = 1;
   SegInits[SegData] = 0;
   Grans[SegIO] = 1;
   ListGrans[SegIO] = 1;
   SegInits[SegIO] = 0;

   MakeCode = MakeCode_47C00;
   ChkPC = ChkPC_47C00;
   IsDef = IsDef_47C00;
   SwitchFrom = SwitchFrom_47C00;
   InitFields();
}

static void InitCode_47C00(void) {
   SaveInitProc();

   DMBAssume = 0;
}

void code47c00_init(void) {
   CPU47C00 = AddCPU("47C00", SwitchTo_47C00);
   CPU470C00 = AddCPU("470C00", SwitchTo_47C00);
   CPU470AC00 = AddCPU("470AC00", SwitchTo_47C00);

   SaveInitProc = InitPassProc;
   InitPassProc = InitCode_47C00;
}
