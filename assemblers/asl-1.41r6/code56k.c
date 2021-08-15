/* code56k.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegeneratormodul fuer die DSP56K-Familie                             */
/*                                                                           */
/* Historie: 10. 6.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

#include "code56k.h"

typedef struct {
   char *Name;
   LongWord Code;
} FixedOrder;

typedef enum { ParAB, ParXYAB, ParABXYnAB, ParABBA, ParXYnAB, ParMul } ParTyp;
typedef struct {
   char *Name;
   ParTyp Typ;
   Byte Code;
} ParOrder;

#define FixedOrderCnt 9

#define ParOrderCnt 29

#define BitOrderCnt 4
static char *BitOrders[BitOrderCnt] = { "BCLR", "BSET", "BCHG", "BTST" };

#define BitJmpOrderCnt 4
static char *BitJmpOrders[BitOrderCnt] = { "JCLR", "JSET", "JSCLR", "JSSET" };

static Byte MacTable[4][4] = { { 0, 2, 5, 4 }, { 2, 0xff, 6, 7 }, { 5, 6, 1, 3 }, { 4, 7, 3, 0xff } };

#define ModNone (-1)
#define ModImm 0
#define MModImm (1 << ModImm)
#define ModAbs 1
#define MModAbs (1 << ModAbs)
#define ModIReg 2
#define MModIReg (1 << ModIReg)
#define ModPreDec 3
#define MModPreDec (1 << ModPreDec)
#define ModPostDec 4
#define MModPostDec (1 << ModPostDec)
#define ModPostInc 5
#define MModPostInc (1 << ModPostInc)
#define ModIndex 6
#define MModIndex (1 << ModIndex)
#define ModModDec 7
#define MModModDec (1 << ModModDec)
#define ModModInc 8
#define MModModInc (1 << ModModInc)

#define MModNoExt (MModIReg+MModPreDec+MModPostDec+MModPostInc+MModIndex+MModModDec+MModModInc)
#define MModNoImm (MModAbs+MModNoExt)
#define MModAll (MModNoImm+MModImm)

#define SegLData (SegYData+1)

#define MSegCode (1 << SegCode)
#define MSegXData (1 << SegXData)
#define MSegYData (1 << SegYData)
#define MSegLData (1 << SegLData)

static CPUVar CPU56000;
static ShortInt AdrType;
static Word AdrMode;
static LongInt AdrVal;
static Byte AdrSeg;

static FixedOrder *FixedOrders;
static ParOrder *ParOrders;

/*----------------------------------------------------------------------------------------------*/

static void AddFixed(char *Name, LongWord Code) {
   if (InstrZ >= FixedOrderCnt) exit(255);

   FixedOrders[InstrZ].Name = Name;
   FixedOrders[InstrZ++].Code = Code;
}

static void AddPar(char *Name, ParTyp Typ, LongWord Code) {
   if (InstrZ >= ParOrderCnt) exit(255);

   ParOrders[InstrZ].Name = Name;
   ParOrders[InstrZ].Typ = Typ;
   ParOrders[InstrZ++].Code = Code;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("NOP", 0x000000);
   AddFixed("ENDDO", 0x00008c);
   AddFixed("ILLEGAL", 0x000005);
   AddFixed("RESET", 0x000084);
   AddFixed("RTI", 0x000004);
   AddFixed("RTS", 0x00000c);
   AddFixed("STOP", 0x000087);
   AddFixed("SWI", 0x000006);
   AddFixed("WAIT", 0x000086);

   ParOrders = (ParOrder *) malloc(sizeof(ParOrder) * ParOrderCnt);
   InstrZ = 0;
   AddPar("ABS", ParAB, 0x26);
   AddPar("ASL", ParAB, 0x32);
   AddPar("ASR", ParAB, 0x22);
   AddPar("CLR", ParAB, 0x13);
   AddPar("LSL", ParAB, 0x33);
   AddPar("LSR", ParAB, 0x23);
   AddPar("NEG", ParAB, 0x36);
   AddPar("NOT", ParAB, 0x17);
   AddPar("RND", ParAB, 0x11);
   AddPar("ROL", ParAB, 0x37);
   AddPar("ROR", ParAB, 0x27);
   AddPar("TST", ParAB, 0x03);
   AddPar("ADC", ParXYAB, 0x21);
   AddPar("SBC", ParXYAB, 0x25);
   AddPar("ADD", ParABXYnAB, 0x00);
   AddPar("CMP", ParABXYnAB, 0x05);
   AddPar("CMPM", ParABXYnAB, 0x07);
   AddPar("SUB", ParABXYnAB, 0x04);
   AddPar("ADDL", ParABBA, 0x12);
   AddPar("ADDR", ParABBA, 0x02);
   AddPar("SUBL", ParABBA, 0x16);
   AddPar("SUBR", ParABBA, 0x06);
   AddPar("AND", ParXYnAB, 0x46);
   AddPar("EOR", ParXYnAB, 0x43);
   AddPar("OR", ParXYnAB, 0x42);
   AddPar("MAC", ParMul, 0x82);
   AddPar("MACR", ParMul, 0x83);
   AddPar("MPY", ParMul, 0x80);
   AddPar("MPYR", ParMul, 0x81);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(ParOrders);
}

/*----------------------------------------------------------------------------------------------*/

static void SplitArg(char *Orig, char *LDest, char *RDest) {
   char *p, *str;

   str = strdup(Orig);
   p = QuotPos(str, ',');
   if (p == NULL) {
      *RDest = '\0';
      strcpy(LDest, str);
   } else {
      *p = '\0';
      strcpy(LDest, str);
      strcpy(RDest, p + 1);
   }
   free(str);
}

static void ChkMask(Word Erl, Byte ErlSeg) {
   if ((AdrType != ModNone) && ((Erl & (1 << AdrType)) == 0)) {
      WrError(1350);
      AdrCnt = 0;
      AdrType = ModNone;
   }
   if ((AdrSeg != SegNone) && ((ErlSeg & (1 << AdrSeg)) == 0)) {
      WrError(1960);
      AdrCnt = 0;
      AdrType = ModNone;
   }
}

static void DecodeAdr(char *Asc_O, Word Erl, Byte ErlSeg) {
   static char *ModMasks[ModModInc + 1] = { "", "", "(Rx)", "-(Rx)", "(Rx)-", "(Rx)+", "(Rx+Nx)", "(Rx)-Nx", "(Rx)+Nx" };
   static Byte ModCodes[ModModInc + 1] = { 0, 0, 4, 7, 2, 3, 5, 0, 1 };
#define SegCount 4
   static char SegNames[SegCount] = { 'P', 'X', 'Y', 'L' };
   static Byte SegVals[SegCount] = { SegCode, SegXData, SegYData, SegLData };
   Integer z, l;
   bool OK;
   Byte OrdVal;
   String Asc;

   AdrType = ModNone;
   AdrCnt = 0;
   strmaxcpy(Asc, Asc_O, 255);

/* Defaultsegment herausfinden */

   if ((ErlSeg & MSegXData) != 0) AdrSeg = SegXData;
   else if ((ErlSeg & MSegYData) != 0) AdrSeg = SegYData;
   else if ((ErlSeg & MSegCode) != 0) AdrSeg = SegCode;
   else AdrSeg = SegNone;

/* Zielsegment vorgegeben ? */

   for (z = 0; z < SegCount; z++)
      if ((toupper(*Asc) == SegNames[z]) && (Asc[1] == ':')) {
         AdrSeg = SegVals[z];
         strcpy(Asc, Asc + 2);
      }

/* Adressausdruecke abklopfen: dazu mit Referenzstring vergleichen */

   for (z = ModIReg; z <= ModModInc; z++)
      if (strlen(Asc) == strlen(ModMasks[z])) {
         AdrMode = 0xffff;
         for (l = 0; l <= strlen(Asc); l++)
            if (ModMasks[z][l] == 'x') {
               OrdVal = Asc[l] - '0';
               if (OrdVal > 7) break;
               else if (AdrMode == 0xffff) AdrMode = OrdVal;
               else if (AdrMode != OrdVal) {
                  WrError(1760);
                  ChkMask(Erl, ErlSeg);
                  return;
               }
            } else if (ModMasks[z][l] != toupper(Asc[l])) break;
         if (l > strlen(Asc)) {
            AdrType = z;
            AdrMode += ModCodes[z] << 3;
            ChkMask(Erl, ErlSeg);
            return;
         }
      }

/* immediate ? */

   if (*Asc == '#') {
      AdrVal = EvalIntExpression(Asc + 1, Int24, &OK);
      if (OK) {
         AdrType = ModImm;
         AdrCnt = 1;
         AdrMode = 0x34;
         ChkMask(Erl, ErlSeg);
         return;
      }
   }

/* dann absolut */

   AdrVal = EvalIntExpression(Asc, Int16, &OK);
   if (OK) {
      AdrType = ModAbs;
      AdrMode = 0x30;
      AdrCnt = 1;
      if ((AdrSeg & ((1 << SegCode) | (1 << SegXData) | (1 << SegYData))) != 0) ChkSpace(AdrSeg);
      ChkMask(Erl, ErlSeg);
      return;
   }
}

static bool DecodeReg(char *Asc, LongInt * Erg) {
#define RegCount 12
   static char *RegNames[RegCount] = { "X0", "X1", "Y0", "Y1", "A0", "B0", "A2", "B2", "A1", "B1", "A", "B" };
   Word z;

   for (z = 0; z < RegCount; z++)
      if (strcasecmp(Asc, RegNames[z]) == 0) {
         *Erg = z + 4;
         return true;
      }
   if ((strlen(Asc) == 2) && (Asc[1] >= '0') && (Asc[1] <= '7'))
      switch (toupper(*Asc)) {
         case 'R':
            *Erg = 16 + Asc[1] - '0';
            return true;
         case 'N':
            *Erg = 24 + Asc[1] - '0';
            return true;
      }
   return false;
}

static bool DecodeALUReg(char *Asc, LongInt * Erg, bool MayX, bool MayY, bool MayAcc) {
   bool Result = false;

   if (!DecodeReg(Asc, Erg)) return Result;
   switch (*Erg) {
      case 4:
      case 5:
         if (MayX) {
            Result = true;
            (*Erg) -= 4;
         }
         break;
      case 6:
      case 7:
         if (MayY) {
            Result = true;
            (*Erg) -= 6;
         }
         break;
      case 14:
      case 15:
         if (MayAcc) {
            Result = true;
            if ((MayX) || (MayY)) (*Erg) -= 12;
            else (*Erg) -= 14;
         }
         break;
   }

   return Result;
}

static bool DecodeLReg(char *Asc, LongInt * Erg) {
#undef RegCount
#define RegCount 8
   static char *RegNames[RegCount] = { "A10", "B10", "X", "Y", "A", "B", "AB", "BA" };
   Word z;

   for (z = 0; z < RegCount; z++)
      if (strcasecmp(Asc, RegNames[z]) == 0) {
         *Erg = z;
         return true;
      }

   return false;
}

static bool DecodeXYABReg(char *Asc, LongInt * Erg) {
#undef RegCount
#define RegCount 8
   static char *RegNames[RegCount] = { "B", "A", "X", "Y", "X0", "Y0", "X1", "Y1" };
   Word z;

   for (z = 0; z < RegCount; z++)
      if (strcasecmp(Asc, RegNames[z]) == 0) {
         *Erg = z;
         return true;
      }

   return false;
}

static bool DecodePCReg(char *Asc, LongInt * Erg) {
#undef RegCount
#define RegCount 7
   static char *RegNames[RegCount] = { "SR", "OMR", "SP", "SSH", "SSL", "LA", "LC" };
   Word z;

   for (z = 0; z < RegCount; z++)
      if (strcasecmp(Asc, RegNames[z]) == 0) {
         (*Erg) = z + 1;
         return true;
      }

   return false;
}

static bool DecodeGeneralReg(char *Asc, LongInt * Erg) {
   if (DecodeReg(Asc, Erg)) return true;
   if (DecodePCReg(Asc, Erg)) {
      (*Erg) += 0x38;
      return true;
   }
   if ((strlen(Asc) == 2) && (toupper(*Asc) == 'M') && (Asc[1] >= '0') && (Asc[1] <= '7')) {
      *Erg = 0x20 + Asc[1] - '0';
      return true;;
   }
   return false;
}

static bool DecodeControlReg(char *Asc, LongInt * Erg) {
   bool Result = true;

   if (strcasecmp(Asc, "MR") == 0) *Erg = 0;
   else if (strcasecmp(Asc, "CCR") == 0) *Erg = 1;
   else if (strcasecmp(Asc, "OMR") == 0) *Erg = 2;
   else Result = false;

   return Result;
}

static bool DecodeOpPair(char *Left, char *Right, Byte WorkSeg, LongInt * Dir, LongInt * Reg1, LongInt * Reg2, LongInt * AType, LongInt * AMode, LongInt * ACnt, LongInt * AVal) {
   bool Result = false;

   if (DecodeALUReg(Left, Reg1, WorkSeg == SegXData, WorkSeg == SegYData, true)) {
      if (DecodeALUReg(Right, Reg2, WorkSeg == SegXData, WorkSeg == SegYData, true)) {
         *Dir = 2;
         Result = true;
      } else {
         *Dir = 0;
         *Reg2 = (-1);
         DecodeAdr(Right, MModNoImm, 1 << WorkSeg);
         if (AdrType != ModNone) {
            *AType = AdrType;
            *AMode = AdrMode;
            *ACnt = AdrCnt;
            *AVal = AdrVal;
            Result = true;
         }
      }
   } else if (DecodeALUReg(Right, Reg1, WorkSeg == SegXData, WorkSeg == SegYData, true)) {
      *Dir = 1;
      *Reg2 = (-1);
      DecodeAdr(Left, MModAll, 1 << WorkSeg);
      if (AdrType != ModNone) {
         *AType = AdrType;
         *AMode = AdrMode;
         *ACnt = AdrCnt;
         *AVal = AdrVal;
         Result = true;
      }
   }

   return Result;
}

static LongInt TurnXY(LongInt Inp) {
   switch (Inp) {
      case 4:
      case 7:
         return Inp - 4;
      case 5:
      case 6:
         return 7 - Inp;
      default: /* wird nie erreicht */
         return 0;
   }
}

static bool DecodeTFR(char *Asc, LongInt * Erg) {
   LongInt Part1, Part2;
   String Left, Right;

   SplitArg(Asc, Left, Right);
   if (!DecodeALUReg(Right, &Part2, false, false, true)) return false;
   if (!DecodeReg(Left, &Part1)) return false;
   if ((Part1 < 4) || ((Part1 > 7) && (Part1 < 14)) || (Part1 > 15)) return false;
   if (Part1 > 13)
      if (((Part1 ^ Part2) & 1) == 0) return false;
      else Part1 = 0;
   else Part1 = TurnXY(Part1) + 4;
   *Erg = (Part1 << 1) + Part2;
   return true;
}

static bool DecodeMOVE_0(void) {
   DAsmCode[0] = 0x200000;
   CodeLen = 1;
   return true;
}

static bool DecodeMOVE_1(Integer Start) {
   String Left, Right;
   LongInt RegErg, RegErg2, IsY, MixErg;
   bool Result = false;

   SplitArg(ArgStr[Start], Left, Right);

/* 1. Register-Update */

   if (*Right == '\0') {
      DecodeAdr(Left, MModPostDec + MModPostInc + MModModDec + MModModInc, 0);
      if (AdrType != ModNone) {
         Result = true;
         DAsmCode[0] = 0x204000 + (AdrMode << 8);
         CodeLen = 1;
      }
      return Result;
   }

/* 2. Ziel ist Register */

   if (DecodeReg(Right, &RegErg)) {
      if (DecodeReg(Left, &RegErg2)) {
         Result = true;
         DAsmCode[0] = 0x200000 + (RegErg << 8) + (RegErg2 << 13);
         CodeLen = 1;
      } else {
         DecodeAdr(Left, MModAll, MSegXData + MSegYData);
         IsY = AdrSeg == SegYData;
         MixErg = ((RegErg & 0x18) << 17) + (IsY << 19) + ((RegErg & 7) << 16);
#ifdef __STDC__
         if ((AdrType == ModImm) && ((AdrVal & 0xffffff00u) == 0))
#else
         if ((AdrType == ModImm) && ((AdrVal & 0xffffff00) == 0))
#endif
         {
            Result = true;
            DAsmCode[0] = 0x200000 + (RegErg << 16) + ((AdrVal & 0xff) << 8);
            CodeLen = 1;
         } else if ((AdrType == ModAbs) && (AdrVal <= 63) && (AdrVal >= 0)) {
            Result = true;
            DAsmCode[0] = 0x408000 + MixErg + (AdrVal << 8);
            CodeLen = 1;
         } else if (AdrType != ModNone) {
            Result = true;
            DAsmCode[0] = 0x40c000 + MixErg + (AdrMode << 8);
            DAsmCode[1] = AdrVal;
            CodeLen = 1 + AdrCnt;
         }
      }
      return Result;
   }

/* 3. Quelle ist Register */

   if (DecodeReg(Left, &RegErg)) {
      DecodeAdr(Right, MModNoImm, MSegXData + MSegYData);
      IsY = AdrSeg == SegYData;
      MixErg = ((RegErg & 0x18) << 17) + (IsY << 19) + ((RegErg & 7) << 16);
      if ((AdrType == ModAbs) && (AdrVal <= 63) && (AdrVal >= 0)) {
         Result = true;
         DAsmCode[0] = 0x400000 + MixErg + (AdrVal << 8);
         CodeLen = 1;
      } else if (AdrType != ModNone) {
         Result = true;
         DAsmCode[0] = 0x404000 + MixErg + (AdrMode << 8);
         DAsmCode[1] = AdrVal;
         CodeLen = 1 + AdrCnt;
      }
      return Result;
   }

/* 4. Ziel ist langes Register */

   if (DecodeLReg(Right, &RegErg)) {
      DecodeAdr(Left, MModNoImm, MSegLData);
      MixErg = ((RegErg & 4) << 17) + ((RegErg & 3) << 16);
      if ((AdrType == ModAbs) && (AdrVal <= 63) && (AdrVal >= 0)) {
         Result = true;
         DAsmCode[0] = 0x408000 + MixErg + (AdrVal << 8);
         CodeLen = 1;
      } else {
         Result = true;
         DAsmCode[0] = 0x40c000 + MixErg + (AdrMode << 8);
         DAsmCode[1] = AdrVal;
         CodeLen = 1 + AdrCnt;
      }
      return Result;
   }

/* 5. Quelle ist langes Register */

   if (DecodeLReg(Left, &RegErg)) {
      DecodeAdr(Right, MModNoImm, MSegLData);
      MixErg = ((RegErg & 4) << 17) + ((RegErg & 3) << 16);
      if ((AdrType == ModAbs) && (AdrVal <= 63) && (AdrVal >= 0)) {
         Result = true;
         DAsmCode[0] = 0x400000 + MixErg + (AdrVal << 8);
         CodeLen = 1;
      } else {
         Result = true;
         DAsmCode[0] = 0x404000 + MixErg + (AdrMode << 8);
         DAsmCode[1] = AdrVal;
         CodeLen = 1 + AdrCnt;
      }
      return Result;
   }

   WrError(1350);
   return Result;
}

static bool DecodeMOVE_2(Integer Start) {
   String Left1, Right1, Left2, Right2;
   LongInt RegErg, Reg1L, Reg1R, Reg2L, Reg2R;
   LongInt Mode1, Mode2, Dir1, Dir2, Type1, Type2, Cnt1, Cnt2, Val1, Val2;
   bool Result = false;

   SplitArg(ArgStr[Start], Left1, Right1);
   SplitArg(ArgStr[Start + 1], Left2, Right2);

/* 1. Spezialfall X auf rechter Seite ? */

   if (strcasecmp(Left2, "X0") == 0) {
      if (!DecodeALUReg(Right2, &RegErg, false, false, true)) WrError(1350);
      else if (strcmp(Left1, Right2) != 0) WrError(1350);
      else {
         DecodeAdr(Right1, MModNoImm, MSegXData);
         if (AdrType != ModNone) {
            CodeLen = 1 + AdrCnt;
            DAsmCode[0] = 0x080000 + (RegErg << 16) + (AdrMode << 8);
            DAsmCode[1] = AdrVal;
            Result = true;
         }
      }
      return Result;
   }

/* 2. Spezialfall Y auf linker Seite ? */

   if (strcasecmp(Left1, "Y0") == 0) {
      if (!DecodeALUReg(Right1, &RegErg, false, false, true)) WrError(1350);
      else if (strcmp(Left2, Right1) != 0) WrError(1350);
      else {
         DecodeAdr(Right2, MModNoImm, MSegYData);
         if (AdrType != ModNone) {
            CodeLen = 1 + AdrCnt;
            DAsmCode[0] = 0x088000 + (RegErg << 16) + (AdrMode << 8);
            DAsmCode[1] = AdrVal;
            Result = true;
         }
      }
      return Result;
   }

/* der Rest..... */

   if ((DecodeOpPair(Left1, Right1, SegXData, &Dir1, &Reg1L, &Reg1R, &Type1, &Mode1, &Cnt1, &Val1))
      && (DecodeOpPair(Left2, Right2, SegYData, &Dir2, &Reg2L, &Reg2R, &Type2, &Mode2, &Cnt2, &Val2))) {
      if ((Reg1R == -1) && (Reg2R == -1)) {
         if ((Mode1 >> 3 < 1) || (Mode1 >> 3 > 4) || (Mode2 >> 3 < 1) || (Mode2 >> 3 > 4)) WrError(1350);
         else if (((Mode1 ^ Mode2) & 4) == 0) WrError(1760);
         else {
            DAsmCode[0] = 0x800000 + (Dir2 << 22) + (Dir1 << 15) + (Reg1L << 18) + (Reg2L << 16) + ((Mode1 & 0x1f) << 8) + ((Mode2 & 3) << 13) + ((Mode2 & 24) << 17);
            CodeLen = 1;
            Result = true;
         }
      } else if (Reg1R == -1) {
         if ((Reg2L < 2) || (Reg2R > 1)) WrError(1350);
         else {
            DAsmCode[0] = 0x100000 + (Reg1L << 18) + ((Reg2L - 2) << 17) + (Reg2R << 16) + (Dir1 << 15) + (Mode1 << 8);
            DAsmCode[1] = Val1;
            CodeLen = 1 + Cnt1;
            Result = true;
         }
      } else if (Reg2R == -1) {
         if ((Reg1L < 2) || (Reg1R > 1)) WrError(1350);
         else {
            DAsmCode[0] = 0x104000 + (Reg2L << 16) + ((Reg1L - 2) << 19) + (Reg1R << 18) + (Dir2 << 15) + (Mode2 << 8);
            DAsmCode[1] = Val2;
            CodeLen = 1 + Cnt2;
            Result = true;
         }
      } else WrError(1350);
      return Result;
   }

   WrError(1350);
   return Result;
}

static bool DecodeMOVE(Integer Start) {
   switch (ArgCnt - Start + 1) {
      case 0:
         return DecodeMOVE_0();
      case 1:
         return DecodeMOVE_1(Start);
      case 2:
         return DecodeMOVE_2(Start);
      default:
         WrError(1110);
         return false;
   }
}

static bool DecodeCondition(char *Asc, Word * Erg) {
#define CondCount 18
   static char *CondNames[CondCount] = { "CC", "GE", "NE", "PL", "NN", "EC", "LC", "GT", "CS", "LT", "EQ", "MI",
      "NR", "ES", "LS", "LE", "HS", "LO"
   };
   bool Result;

   (*Erg) = 0;
   while ((*Erg < CondCount) && (strcasecmp(CondNames[*Erg], Asc) != 0)) (*Erg)++;
   if (*Erg == CondCount - 1) *Erg = 8;
   Result = (*Erg < CondCount);
   *Erg &= 15;
   return Result;
}

static bool DecodePseudo(void) {
   bool OK;
   Integer BCount;
   Word AdrWord, z, z2;
/*   Byte Segment;*/
   TempResult t;
   LongInt HInt;

   if (Memo("XSFR")) {
      CodeEquate(SegXData, 0, 0xffff);
      return true;
   }

   if (Memo("YSFR")) {
      CodeEquate(SegYData, 0, 0xffff);
      return true;
   }

/*  IF (Memo('XSFR')) || (Memo('YSFR')) THEN
    {
     FirstPassUnknown:=false;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      {
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF (OK) && (! FirstPassUnknown) THEN
	{
	 IF Memo('YSFR') THEN Segment:=SegYData ELSE Segment:=SegXData;
         PushLocHandle(-1);
	 EnterIntSymbol(LabPart,AdrWord,Segment,false);
         PopLocHandle;
	 IF MakeUseList THEN AddChunk(SegChunks[Segment],AdrWord,1,false);
	 ListLine:='='+'$'+HexString(AdrWord,4);
	};
      };
     Exit;
    };*/

   if (Memo("DS")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         FirstPassUnknown = false;
         AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
         if (FirstPassUnknown) WrError(1820);
         if ((OK) && (!FirstPassUnknown)) {
            CodeLen = AdrWord;
            DontPrint = true;
            if (MakeUseList)
               if (AddChunk(SegChunks + ActPC, ProgCounter(), CodeLen, ActPC == SegCode)) WrError(90);
         }
      }
      return true;
   }

   if (Memo("DC")) {
      if (ArgCnt < 1) WrError(1110);
      else {
         OK = true;
         for (z = 1; z <= ArgCnt; z++)
            if (OK) {
               FirstPassUnknown = false;
               EvalExpression(ArgStr[z], &t);
               switch (t.Typ) {
                  case TempInt:
                     if (FirstPassUnknown) t.Contents.Int &= 0xffffff;
                     if (!RangeCheck(t.Contents.Int, Int24)) {
                        WrError(1320);
                        OK = false;
                     } else {
                        DAsmCode[CodeLen++] = t.Contents.Int & 0xffffff;
                     }
                     break;
                  case TempString:
                     BCount = 2;
                     DAsmCode[CodeLen] = 0;
                     for (z2 = 0; z2 < strlen(t.Contents.Ascii); z2++) {
                        HInt = t.Contents.Ascii[z2];
                        HInt = CharTransTable[(Byte) HInt];
                        HInt <<= (BCount * 8);
                        DAsmCode[CodeLen] |= HInt;
                        if (--BCount < 0) {
                           BCount = 2;
                           DAsmCode[++CodeLen] = 0;
                        }
                     }
                     if (BCount != 2) CodeLen++;
                     break;
                  default:
                     WrError(1135);
                     OK = false;
               }
            }
      }
      if (!OK) CodeLen = 0;
      return true;
   }

   return false;
}

static void MakeCode_56K(void) {
   Integer z;
   LongInt AddVal, h = 0, Reg1, Reg2, Reg3;
   LongInt HVal, HCnt, HMode, HSeg;
   Word Condition;
   bool OK;
   String Left, Mid, Right;

   CodeLen = 0;
   DontPrint = false;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

/* ohne Argument */

   for (z = 0; z < FixedOrderCnt; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else {
            CodeLen = 1;
            DAsmCode[0] = FixedOrders[z].Code;
         };
         return;
      }

/* ALU */

   for (z = 0; z < ParOrderCnt; z++)
      if (Memo(ParOrders[z].Name)) {
         if (DecodeMOVE(2)) {
            OK = true;
            switch (ParOrders[z].Typ) {
               case ParAB:
                  if (DecodeALUReg(ArgStr[1], &Reg1, false, false, true)) h = Reg1 << 3;
                  else OK = false;
                  break;
               case ParXYAB:
                  SplitArg(ArgStr[1], Left, Right);
                  if (!DecodeALUReg(Right, &Reg2, false, false, true)) OK = false;
                  else if (!DecodeLReg(Left, &Reg1)) OK = false;
                  else if ((Reg1 < 2) || (Reg1 > 3)) OK = false;
                  else h = (Reg2 << 3) + ((Reg1 - 2) << 4);
                  break;
               case ParABXYnAB:
                  SplitArg(ArgStr[1], Left, Right);
                  if (!DecodeALUReg(Right, &Reg2, false, false, true)) OK = false;
                  else if (!DecodeXYABReg(Left, &Reg1)) OK = false;
                  else if ((Reg1 ^ Reg2) == 1) OK = false;
                  else {
                     if (Reg1 == 0) Reg1 = 1;
                     h = (Reg2 << 3) + (Reg1 << 4);
                  }
                  break;
               case ParABBA:
                  if (strcasecmp(ArgStr[1], "B,A") == 0) h = 0;
                  else if (strcasecmp(ArgStr[1], "A,B") == 0) h = 8;
                  else OK = false;
                  break;
               case ParXYnAB:
                  SplitArg(ArgStr[1], Left, Right);
                  if (!DecodeALUReg(Right, &Reg2, false, false, true)) OK = false;
                  else if (!DecodeReg(Left, &Reg1)) OK = false;
                  else if ((Reg1 < 4) || (Reg1 > 7)) OK = false;
                  else h = (Reg2 << 3) + (TurnXY(Reg1) << 4);
                  break;
               case ParMul:
                  SplitArg(ArgStr[1], Left, Mid);
                  SplitArg(Mid, Mid, Right);
                  h = 0;
                  if (*Left == '-') {
                     strcpy(Left, Left + 1);
                     h += 4;
                  } else if (*Left == '+') strcpy(Left, Left + 1);
                  if (!DecodeALUReg(Right, &Reg3, false, false, true)) OK = false;
                  else if (!DecodeReg(Left, &Reg1)) OK = false;
                  else if ((Reg1 < 4) || (Reg1 > 7)) OK = false;
                  else if (!DecodeReg(Mid, &Reg2)) OK = false;
                  else if ((Reg2 < 4) || (Reg2 > 7)) OK = false;
                  else if (MacTable[Reg1 - 4][Reg2 - 4] == 0xff) OK = false;
                  else h += (Reg3 << 3) + (MacTable[Reg1 - 4][Reg2 - 4] << 4);
                  break;
            }
            if (OK) DAsmCode[0] += ParOrders[z].Code + h;
            else {
               WrError(1350);
               CodeLen = 0;
            }
         }
         return;
      }

   if (Memo("DIV")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         SplitArg(ArgStr[1], Left, Right);
         if ((*Left == '\0') || (*Right == '\0')) WrError(1110);
         else if (!DecodeALUReg(Right, &Reg2, false, false, true)) WrError(1350);
         else if (!DecodeReg(Left, &Reg1)) WrError(1350);
         else if ((Reg1 < 4) || (Reg1 > 7)) WrError(1350);
         else {
            CodeLen = 1;
            DAsmCode[0] = 0x018040 + (Reg2 << 3) + (TurnXY(Reg1) << 4);
         }
      }
      return;
   }

   if ((Memo("ANDI")) || (Memo("ORI"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         SplitArg(ArgStr[1], Left, Right);
         if ((*Left == '\0') || (*Right == '\0')) WrError(1110);
         else if (!DecodeControlReg(Right, &Reg1)) WrError(1350);
         else if (*Left != '#') WrError(1120);
         else {
            h = EvalIntExpression(Left + 1, Int8, &OK);
            if (OK) {
               CodeLen = 1;
               DAsmCode[0] = 0x0000b8 + ((h & 0xff) << 8) + (Memo("ORI") << 6) + Reg1;
            }
         }
      }
      return;
   }

   if (Memo("NORM")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         SplitArg(ArgStr[1], Left, Right);
         if ((*Left == '\0') || (*Right == '\0')) WrError(1110);
         else if (!DecodeALUReg(Right, &Reg2, false, false, true)) WrError(1350);
         else if (!DecodeReg(Left, &Reg1)) WrError(1350);
         else if ((Reg1 < 16) || (Reg1 > 23)) WrError(1350);
         else {
            CodeLen = 1;
            DAsmCode[0] = 0x01d815 + ((Reg1 & 7) << 8) + (Reg2 << 3);
         }
      }
      return;
   }

   for (z = 0; z < BitOrderCnt; z++)
      if (Memo(BitOrders[z])) {
         if (ArgCnt != 1) WrError(1110);
         else {
            Reg2 = ((z & 1) << 5) + (((LongInt) z >> 1) << 16);
            SplitArg(ArgStr[1], Left, Right);
            if ((*Left == '\0') || (*Right == '\0')) WrError(1110);
            else if (*Left != '#') WrError(1120);
            else {
               h = EvalIntExpression(Left + 1, Int8, &OK);
               if (FirstPassUnknown) h &= 15;
               if (OK)
                  if ((h < 0) || (h > 23)) WrError(1320);
                  else if (DecodeGeneralReg(Right, &Reg1)) {
                     CodeLen = 1;
                     DAsmCode[0] = 0x0ac040 + h + (Reg1 << 8) + Reg2;
                  } else {
                     DecodeAdr(Right, MModNoImm, MSegXData + MSegYData);
                     Reg3 = (AdrSeg == SegYData) << 6;
                     if ((AdrType == ModAbs) && (AdrVal <= 63) && (AdrVal >= 0)) {
                        CodeLen = 1;
                        DAsmCode[0] = 0x0a0000 + h + (AdrVal << 8) + Reg3 + Reg2;
                     } else if ((AdrType == ModAbs) && (AdrVal >= 0xffc0) && (AdrVal <= 0xffff)) {
                        CodeLen = 1;
                        DAsmCode[0] = 0x0a8000 + h + ((AdrVal & 0x3f) << 8) + Reg3 + Reg2;
                     } else if (AdrType != ModNone) {
                        CodeLen = 1 + AdrCnt;
                        DAsmCode[0] = 0x0a4000 + h + (AdrMode << 8) + Reg3 + Reg2;
                        DAsmCode[1] = AdrVal;
                     }
                  }
            }
         }
         return;
      }

/* Datentransfer */

   if (Memo("MOVE")) {
      DecodeMOVE(1);
      return;
   }

   if (Memo("MOVEC")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         SplitArg(ArgStr[1], Left, Right);
         if ((*Left == '\0') || (*Right == '\0')) WrError(1110);
         else if (DecodeGeneralReg(Left, &Reg1))
            if (DecodeGeneralReg(Right, &Reg2))
               if (Reg1 >= 0x20) { /* S1,D2 */
                  CodeLen = 1;
                  DAsmCode[0] = 0x044080 + (Reg2 << 8) + Reg1;
               } else if (Reg2 >= 0x20) { /* S2,D1 */
                  CodeLen = 1;
                  DAsmCode[0] = 0x04c080 + (Reg1 << 8) + Reg2;
               } else WrError(1350);
            else if (Reg1 < 0x20) WrError(1350);
            else { /* S1,ea/aa */
               DecodeAdr(Right, MModNoImm, MSegXData + MSegYData);
               if ((AdrType == ModAbs) && (AdrVal >= 0) && (AdrVal <= 63)) {
                  CodeLen = 1;
                  DAsmCode[0] = 0x050000 + (AdrVal << 8) + ((AdrSeg == SegYData) << 6) + Reg1;
               } else if (AdrType != ModNone) {
                  CodeLen = 1 + AdrCnt;
                  DAsmCode[1] = AdrVal;
                  DAsmCode[0] = 0x054000 + (AdrMode << 8) + ((AdrSeg == SegYData) << 6) + Reg1;
               }
         } else if (!DecodeGeneralReg(Right, &Reg2)) WrError(1350);
         else if (Reg2 < 0x20) WrError(1350);
         else { /* ea/aa,D1 */
            DecodeAdr(Left, MModAll, MSegXData + MSegYData);
            if ((AdrType == ModImm) && (AdrVal <= 0xff) && (AdrVal >= 0)) {
               CodeLen = 1;
               DAsmCode[0] = 0x050080 + (AdrVal << 8) + Reg2;
            } else if ((AdrType == ModAbs) && (AdrVal >= 0) && (AdrVal <= 63)) {
               CodeLen = 1;
               DAsmCode[0] = 0x058000 + (AdrVal << 8) + ((AdrSeg == SegYData) << 6) + Reg2;
            } else if (AdrType != ModNone) {
               CodeLen = 1 + AdrCnt;
               DAsmCode[1] = AdrVal;
               DAsmCode[0] = 0x05c000 + (AdrMode << 8) + ((AdrSeg == SegYData) << 6) + Reg2;
            }
         }
      }
      return;
   }

   if (Memo("MOVEM")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         SplitArg(ArgStr[1], Left, Right);
         if ((*Left == '\0') || (*Right == '\0')) WrError(1110);
         else if (DecodeGeneralReg(Left, &Reg1)) {
            DecodeAdr(Right, MModNoImm, MSegCode);
            if ((AdrType == ModAbs) && (AdrVal >= 0) && (AdrVal <= 63)) {
               CodeLen = 1;
               DAsmCode[0] = 0x070000 + Reg1 + (AdrVal << 8);
            } else if (AdrType != ModNone) {
               CodeLen = 1 + AdrCnt;
               DAsmCode[1] = AdrVal;
               DAsmCode[0] = 0x074080 + Reg1 + (AdrMode << 8);
            }
         } else if (!DecodeGeneralReg(Right, &Reg2)) WrError(1350);
         else {
            DecodeAdr(Left, MModNoImm, MSegCode);
            if ((AdrType == ModAbs) && (AdrVal >= 0) && (AdrVal <= 63)) {
               CodeLen = 1;
               DAsmCode[0] = 0x078000 + Reg2 + (AdrVal << 8);
            } else if (AdrType != ModNone) {
               CodeLen = 1 + AdrCnt;
               DAsmCode[1] = AdrVal;
               DAsmCode[0] = 0x07c080 + Reg2 + (AdrMode << 8);
            }
         }
      }
      return;
   }

   if (Memo("MOVEP")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         SplitArg(ArgStr[1], Left, Right);
         if ((*Left == '\0') || (*Right == '\0')) WrError(1110);
         else if (DecodeGeneralReg(Left, &Reg1)) {
            DecodeAdr(Right, MModAbs, MSegXData + MSegYData);
            if (AdrType != ModNone)
               if ((AdrVal > 0xffff) || (AdrVal < 0xffc0)) WrError(1315);
               else {
                  CodeLen = 1;
                  DAsmCode[0] = 0x08c000 + ((AdrSeg == SegYData) << 16) + (AdrVal & 0x3f) + (Reg1 << 8);
               }
         } else if (DecodeGeneralReg(Right, &Reg2)) {
            DecodeAdr(Left, MModAbs, MSegXData + MSegYData);
            if (AdrType != ModNone)
               if ((AdrVal > 0xffff) || (AdrVal < 0xffc0)) WrError(1315);
               else {
                  CodeLen = 1;
                  DAsmCode[0] = 0x084000 + ((AdrSeg == SegYData) << 16) + (AdrVal & 0x3f) + (Reg2 << 8);
               }
         } else {
            DecodeAdr(Left, MModAll, MSegXData + MSegYData + MSegCode);
            if ((AdrType == ModAbs) && (AdrSeg != SegCode) && (AdrVal >= 0xffc0) && (AdrVal <= 0xffff)) {
               HVal = AdrVal & 0x3f;
               HSeg = AdrSeg;
               DecodeAdr(Right, MModNoImm, MSegXData + MSegYData + MSegCode);
               if (AdrType != ModNone)
                  if (AdrSeg == SegCode) {
                     CodeLen = 1 + AdrCnt;
                     DAsmCode[1] = AdrVal;
                     DAsmCode[0] = 0x084040 + HVal + (AdrMode << 8) + ((HSeg == SegYData) << 16);
                  } else {
                     CodeLen = 1 + AdrCnt;
                     DAsmCode[1] = AdrVal;
                     DAsmCode[0] = 0x084080 + HVal + (AdrMode << 8) + ((HSeg == SegYData) << 16) + ((AdrSeg == SegYData) << 6);
                  }
            } else if (AdrType != ModNone) {
               HVal = AdrVal;
               HCnt = AdrCnt;
               HMode = AdrMode;
               HSeg = AdrSeg;
               DecodeAdr(Right, MModAbs, MSegXData + MSegYData);
               if (AdrType != ModNone)
                  if ((AdrVal < 0xffc0) || (AdrVal > 0xffff)) WrError(1315);
                  else if (HSeg == SegCode) {
                     CodeLen = 1 + HCnt;
                     DAsmCode[1] = HVal;
                     DAsmCode[0] = 0x08c040 + (AdrVal & 0x3f) + (HMode << 8) + ((AdrSeg == SegYData) << 16);
                  } else {
                     CodeLen = 1 + HCnt;
                     DAsmCode[1] = HVal;
                     DAsmCode[0] = 0x08c080 + (((Word) AdrVal) & 0x3f) + (HMode << 8) + ((AdrSeg == SegYData) << 16) + ((HSeg == SegYData) << 6);
                  }
            }
         }
      }
      return;
   }

   if (Memo("TFR")) {
      if (ArgCnt < 1) WrError(1110);
      else if (DecodeMOVE(2))
         if (DecodeTFR(ArgStr[1], &Reg1)) {
            DAsmCode[0] += 0x01 + (Reg1 << 3);
         } else {
            WrError(1350);
            CodeLen = 0;
         }
      return;
   }

   if ((*OpPart == 'T') && (DecodeCondition(OpPart + 1, &Condition))) {
      if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
      else if (!DecodeTFR(ArgStr[1], &Reg1)) WrError(1350);
      else if (ArgCnt == 1) {
         CodeLen = 1;
         DAsmCode[0] = 0x020000 + (Condition << 12) + (Reg1 << 3);
      } else {
         SplitArg(ArgStr[2], Left, Right);
         if ((*Left == '\0') || (*Right == '\0')) WrError(1110);
         else if (!DecodeReg(Left, &Reg2)) WrError(1350);
         else if ((Reg2 < 16) || (Reg2 > 23)) WrError(1350);
         else if (!DecodeReg(Right, &Reg3)) WrError(1350);
         else if ((Reg2 < 16) || (Reg2 > 23)) WrError(1350);
         else {
            Reg2 -= 16;
            Reg3 -= 16;
            CodeLen = 1;
            DAsmCode[0] = 0x030000 + (Condition << 12) + (Reg2 << 8) + (Reg1 << 3) + Reg3;
         }
      }
      return;
   }

   if (Memo("LUA")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         SplitArg(ArgStr[1], Left, Right);
         if ((*Left == '\0') || (*Right == '\0')) WrError(1110);
         else {
            DecodeAdr(Left, MModModInc + MModModDec + MModPostInc + MModPostDec, MSegXData);
            if (AdrType != ModNone)
               if (!DecodeReg(Right, &Reg1)) WrError(1350);
               else if ((Reg1 < 16) || (Reg1 > 31)) WrError(1350);
               else {
                  CodeLen = 1;
                  DAsmCode[0] = 0x044000 + (AdrMode << 8) + Reg1;
               }
         }
      }
      return;
   }

/* Spruenge */

   if ((Memo("JMP")) || (Memo("JSR"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AddVal = (Memo("JSR")) << 16;
         DecodeAdr(ArgStr[1], MModNoImm, MSegCode);
         if (AdrType == ModAbs)
            if ((AdrVal & 0xf000) == 0) {
               CodeLen = 1;
               DAsmCode[0] = 0x0c0000 + AddVal + AdrVal;
            } else {
               CodeLen = 2;
               DAsmCode[0] = 0x0af080 + AddVal;
               DAsmCode[1] = AdrVal;
         } else if (AdrType != ModNone) {
            CodeLen = 1;
            DAsmCode[0] = 0x0ac080 + AddVal + (AdrMode << 8);
         }
      }
      return;
   }

   if ((*OpPart == 'J') && (DecodeCondition(OpPart + 1, &Condition))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModNoImm, MSegCode);
         if (AdrType == ModAbs)
            if ((AdrVal & 0xf000) == 0) {
               CodeLen = 1;
               DAsmCode[0] = 0x0e0000 + (Condition << 12) + AdrVal;
            } else {
               CodeLen = 2;
               DAsmCode[0] = 0x0af0a0 + Condition;
               DAsmCode[1] = AdrVal;
         } else if (AdrType != ModNone) {
            CodeLen = 1;
            DAsmCode[0] = 0x0ac0a0 + Condition + (AdrMode << 8);
         }
      }
      return;
   }

   if ((strncmp(OpPart, "JS", 2) == 0) && (DecodeCondition(OpPart + 2, &Condition))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModNoImm, MSegCode);
         if (AdrType == ModAbs)
            if ((AdrVal & 0xf000) == 0) {
               CodeLen = 1;
               DAsmCode[0] = 0x0f0000 + (Condition << 12) + AdrVal;
            } else {
               CodeLen = 2;
               DAsmCode[0] = 0x0bf0a0 + Condition;
               DAsmCode[1] = AdrVal;
         } else if (AdrType != ModNone) {
            CodeLen = 1;
            DAsmCode[0] = 0x0bc0a0 + Condition + (AdrMode << 8);
         }
      }
      return;
   }

   for (z = 0; z < BitJmpOrderCnt; z++)
      if (Memo(BitJmpOrders[z])) {
         if (ArgCnt != 1) WrError(1110);
         else {
            SplitArg(ArgStr[1], Left, Mid);
            SplitArg(Mid, Mid, Right);
            if ((*Left == '\0') || (*Mid == '\0') || (*Right == '\0')) WrError(1110);
            else if (*Left != '#') WrError(1120);
            else {
               DAsmCode[1] = EvalIntExpression(Right, Int16, &OK);
               if (OK) {
                  h = EvalIntExpression(Left + 1, Int8, &OK);
                  if (FirstPassUnknown) h &= 15;
                  if (OK)
                     if ((h < 0) || (h > 23)) WrError(1320);
                     else {
                        Reg2 = ((z & 1) << 5) + (((LongInt) (z >> 1)) << 16);
                        if (DecodeGeneralReg(Mid, &Reg1)) {
                           CodeLen = 2;
                           DAsmCode[0] = 0x0ac080 + h + Reg2 + (Reg1 << 8);
                        } else {
                           DecodeAdr(Mid, MModNoImm, MSegXData + MSegYData);
                           Reg3 = (AdrSeg == SegYData) << 6;
                           if (AdrType == ModAbs)
                              if ((AdrVal >= 0) && (AdrVal <= 63)) {
                                 CodeLen = 2;
                                 DAsmCode[0] = 0x0a0080 + h + Reg2 + Reg3 + (AdrVal << 8);
                              } else if ((AdrVal >= 0xffc0) && (AdrVal <= 0xffff)) {
                                 CodeLen = 2;
                                 DAsmCode[0] = 0x0a8080 + h + Reg2 + Reg3 + ((AdrVal & 0x3f) << 8);
                              } else WrError(1320);
                           else {
                              CodeLen = 2;
                              DAsmCode[0] = 0x0a4080 + h + Reg2 + Reg3 + (AdrMode << 8);
                           }
                        }
                     }
               }
            }
         }
         return;
      }

   if (Memo("DO")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         SplitArg(ArgStr[1], Left, Right);
         if ((*Left == '\0') || (*Right == '\0')) WrError(1110);
         else {
            DAsmCode[1] = EvalIntExpression(Right, Int16, &OK);
            if (OK) {
               ChkSpace(SegCode);
               if (DecodeGeneralReg(Left, &Reg1)) {
                  CodeLen = 2;
                  DAsmCode[0] = 0x06c000 + (Reg1 << 8);
               } else if (*Left == '#') {
                  Reg1 = EvalIntExpression(Left + 1, Int12, &OK);
                  if (OK) {
                     CodeLen = 2;
                     DAsmCode[0] = 0x060080 + (Reg1 >> 8) + ((Reg1 & 0xff) << 8);
                  }
               } else {
                  DecodeAdr(Left, MModNoImm, MSegXData + MSegYData);
                  if (AdrType == ModAbs)
                     if ((AdrVal < 0) || (AdrVal > 63)) WrError(1320);
                     else {
                        CodeLen = 2;
                        DAsmCode[0] = 0x060000 + (AdrVal << 8) + ((AdrSeg == SegYData) << 6);
                  } else {
                     CodeLen = 2;
                     DAsmCode[0] = 0x064000 + (AdrMode << 8) + ((AdrSeg == SegYData) << 6);
                  }
               }
            }
         }
      }
      return;
   }

   if (Memo("REP")) {
      if (ArgCnt != 1) WrError(1110);
      else if (DecodeGeneralReg(ArgStr[1], &Reg1)) {
         CodeLen = 1;
         DAsmCode[0] = 0x06c020 + (Reg1 << 8);
      } else {
         DecodeAdr(ArgStr[1], MModAll, MSegXData + MSegYData);
         if (AdrType == ModImm)
            if ((AdrVal < 0) || (AdrVal > 0xfff)) WrError(1320);
            else {
               CodeLen = 1;
               DAsmCode[0] = 0x0600a0 + (AdrVal >> 8) + ((AdrVal & 0xff) << 8);
         } else if ((AdrType == ModAbs) && (AdrVal >= 0) && (AdrVal <= 63)) {
            CodeLen = 1;
            DAsmCode[0] = 0x060020 + (AdrVal << 8) + ((AdrSeg == SegYData) << 6);
         } else {
            CodeLen = 1 + AdrCnt;
            DAsmCode[1] = AdrVal;
            DAsmCode[0] = 0x064020 + (AdrMode << 8) + ((AdrSeg == SegYData) << 6);
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_56K(void) {
   bool ok;

   switch (ActPC) {
      case SegCode:
      case SegXData:
      case SegYData:
         ok = (ProgCounter() <= 0xffff);
         break;
      default:
         ok = false;
   }
   return (ok);
}

static bool IsDef_56K(void) {
   return ((Memo("XSFR")) || (Memo("YSFR")));
}

static void SwitchFrom_56K(void) {
   DeinitFields();
}

static void SwitchTo_56K(void) {
   TurnWords = true;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "*";
   HeaderID = 0x09;
   NOPCode = 0x000000;
   DivideChars = " \009";
   HasAttrs = false;

   ValidSegs = (1 << SegCode) | (1 << SegXData) | (1 << SegYData);
   Grans[SegCode] = 4;
   ListGrans[SegCode] = 4;
   SegInits[SegCode] = 0;
   Grans[SegXData] = 4;
   ListGrans[SegXData] = 4;
   SegInits[SegXData] = 0;
   Grans[SegYData] = 4;
   ListGrans[SegYData] = 4;
   SegInits[SegYData] = 0;

   MakeCode = MakeCode_56K;
   ChkPC = ChkPC_56K;
   IsDef = IsDef_56K;
   SwitchFrom = SwitchFrom_56K;
   InitFields();
}

void code56k_init(void) {
   CPU56000 = AddCPU("56000", SwitchTo_56K);
}
