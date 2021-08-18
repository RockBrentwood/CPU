// AS-Portierung
// AS-Codegenerator H8/500
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

#define FixedOrderCount 6
#define RelOrderCount 21
#define OneOrderCount 13
#define OneRegOrderCount 3
#define RegEAOrderCount 9
#define TwoRegOrderCount 3
#define LogOrderCount 3
#define BitOrderCount 4

#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModIReg 1
#define MModIReg (1 << ModIReg)
#define ModDisp8 2
#define MModDisp8 (1 << ModDisp8)
#define ModDisp16 3
#define MModDisp16 (1 << ModDisp16)
#define ModPredec 4
#define MModPredec (1 << ModPredec)
#define ModPostInc 5
#define MModPostInc (1 << ModPostInc)
#define ModAbs8 6
#define MModAbs8 (1 << ModAbs8)
#define ModAbs16 7
#define MModAbs16 (1 << ModAbs16)
#define ModImm 8
#define MModImm (1 << ModImm)

#define MModAll (MModReg|MModIReg|MModDisp8|MModDisp16|MModPredec|MModPostInc|MModAbs8|MModAbs16|MModImm)
#define MModNoImm (MModAll-MModImm)

typedef struct {
   char *Name;
   Word Code;
} FixedOrder;

typedef struct {
   char *Name;
   Word Code;
   Byte SizeMask;
   ShortInt DefSize;
} OneOrder;

static CPUVar CPU532, CPU534, CPU536, CPU538;
static SimpProc SaveInitProc;

static ShortInt OpSize;
static String Format;
static ShortInt AdrMode;
static Byte AdrByte, FormatCode;
static Byte AdrVals[3];
static Byte AbsBank;

static LongInt Reg_DP, Reg_EP, Reg_TP, Reg_BR;

static FixedOrder *FixedOrders;
static FixedOrder *RelOrders;
static OneOrder *OneOrders;
static OneOrder *OneRegOrders;
static OneOrder *RegEAOrders;
static OneOrder *TwoRegOrders;
static FixedOrder *LogOrders;
static FixedOrder *BitOrders;

/*-------------------------------------------------------------------------*/
/* dynamische Belegung/Freigabe Codetabellen */

static void AddFixed(char *NName, Word NCode) {
   if (InstrZ >= FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Code = NCode;
   FixedOrders[InstrZ++].Name = NName;
}

static void AddRel(char *NName, Word NCode) {
   if (InstrZ >= RelOrderCount) exit(255);
   RelOrders[InstrZ].Code = NCode;
   RelOrders[InstrZ++].Name = NName;
}

static void AddOne(char *NName, Word NCode, Byte NMask, ShortInt NDef) {
   if (InstrZ >= OneOrderCount) exit(255);
   OneOrders[InstrZ].Code = NCode;
   OneOrders[InstrZ].Name = NName;
   OneOrders[InstrZ].SizeMask = NMask;
   OneOrders[InstrZ++].DefSize = NDef;
}

static void AddOneReg(char *NName, Word NCode, Byte NMask, ShortInt NDef) {
   if (InstrZ >= OneRegOrderCount) exit(255);
   OneRegOrders[InstrZ].Code = NCode;
   OneRegOrders[InstrZ].Name = NName;
   OneRegOrders[InstrZ].SizeMask = NMask;
   OneRegOrders[InstrZ++].DefSize = NDef;
}

static void AddRegEA(char *NName, Word NCode, Byte NMask, ShortInt NDef) {
   if (InstrZ >= RegEAOrderCount) exit(255);
   RegEAOrders[InstrZ].Code = NCode;
   RegEAOrders[InstrZ].Name = NName;
   RegEAOrders[InstrZ].SizeMask = NMask;
   RegEAOrders[InstrZ++].DefSize = NDef;
}

static void AddTwoReg(char *NName, Word NCode, Byte NMask, ShortInt NDef) {
   if (InstrZ >= TwoRegOrderCount) exit(255);
   TwoRegOrders[InstrZ].Code = NCode;
   TwoRegOrders[InstrZ].Name = NName;
   TwoRegOrders[InstrZ].SizeMask = NMask;
   TwoRegOrders[InstrZ++].DefSize = NDef;
}

static void AddLog(char *NName, Word NCode) {
   if (InstrZ >= LogOrderCount) exit(255);
   LogOrders[InstrZ].Code = NCode;
   LogOrders[InstrZ++].Name = NName;
}

static void AddBit(char *NName, Word NCode) {
   if (InstrZ >= BitOrderCount) exit(255);
   BitOrders[InstrZ].Code = NCode;
   BitOrders[InstrZ++].Name = NName;
}

static void InitFields(void) {
   InstrZ = 0;
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCount);
   AddFixed("NOP", 0x0000);
   AddFixed("PRTS", 0x1119);
   AddFixed("RTE", 0x000a);
   AddFixed("RTS", 0x0019);
   AddFixed("SLEEP", 0x001a);
   AddFixed("TRAP/VS", 0x0009);

   InstrZ = 0;
   RelOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * RelOrderCount);
   AddRel("BRA", 0x20);
   AddRel("BT", 0x20);
   AddRel("BRN", 0x21);
   AddRel("BF", 0x21);
   AddRel("BHI", 0x22);
   AddRel("BLS", 0x23);
   AddRel("BCC", 0x24);
   AddRel("BHS", 0x24);
   AddRel("BCS", 0x25);
   AddRel("BLO", 0x25);
   AddRel("BNE", 0x26);
   AddRel("BEQ", 0x27);
   AddRel("BVC", 0x28);
   AddRel("BVS", 0x29);
   AddRel("BPL", 0x2a);
   AddRel("BMI", 0x2b);
   AddRel("BGE", 0x2c);
   AddRel("BLT", 0x2d);
   AddRel("BGT", 0x2e);
   AddRel("BLE", 0x2f);
   AddRel("BSR", 0x0e);

   InstrZ = 0;
   OneOrders = (OneOrder *) malloc(sizeof(OneOrder) * OneOrderCount);
   AddOne("CLR", 0x13, 3, 1);
   AddOne("NEG", 0x14, 3, 1);
   AddOne("NOT", 0x15, 3, 1);
   AddOne("ROTL", 0x1c, 3, 1);
   AddOne("ROTR", 0x1d, 3, 1);
   AddOne("ROTXL", 0x1e, 3, 1);
   AddOne("ROTXR", 0x1f, 3, 1);
   AddOne("SHAL", 0x18, 3, 1);
   AddOne("SHAR", 0x19, 3, 1);
   AddOne("SHLL", 0x1a, 3, 1);
   AddOne("SHLR", 0x1b, 3, 1);
   AddOne("TAS", 0x17, 1, 0);
   AddOne("TST", 0x16, 3, 1);

   InstrZ = 0;
   OneRegOrders = (OneOrder *) malloc(sizeof(OneOrder) * OneRegOrderCount);
   AddOneReg("EXTS", 0x11, 1, 0);
   AddOneReg("EXTU", 0x12, 1, 0);
   AddOneReg("SWAP", 0x10, 1, 0);

   InstrZ = 0;
   RegEAOrders = (OneOrder *) malloc(sizeof(OneOrder) * RegEAOrderCount);
   AddRegEA("ADDS", 0x28, 3, 1);
   AddRegEA("ADDX", 0xa0, 3, 1);
   AddRegEA("AND", 0x50, 3, 1);
   AddRegEA("DIVXU", 0xb8, 3, 1);
   AddRegEA("MULXU", 0xa8, 3, 1);
   AddRegEA("OR", 0x40, 3, 1);
   AddRegEA("SUBS", 0x38, 3, 1);
   AddRegEA("SUBX", 0xb0, 3, 1);
   AddRegEA("XOR", 0x60, 3, 1);

   InstrZ = 0;
   TwoRegOrders = (OneOrder *) malloc(sizeof(OneOrder) * TwoRegOrderCount);
   AddTwoReg("DADD", 0xa000, 1, 0);
   AddTwoReg("DSUB", 0xb000, 1, 0);
   AddTwoReg("XCH", 0x90, 2, 1);

   InstrZ = 0;
   LogOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * LogOrderCount);
   AddLog("ANDC", 0x58);
   AddLog("ORC", 0x48);
   AddLog("XORC", 0x68);

   InstrZ = 0;
   BitOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * BitOrderCount);
   AddBit("BCLR", 0x50);
   AddBit("BNOT", 0x60);
   AddBit("BSET", 0x40);
   AddBit("BTST", 0x70);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(RelOrders);
   free(OneOrders);
   free(OneRegOrders);
   free(RegEAOrders);
   free(TwoRegOrders);
   free(LogOrders);
   free(BitOrders);
}

/*-------------------------------------------------------------------------*/
/* Adressparsing */

static void SetOpSize(ShortInt NSize) {
   if (OpSize == -1) OpSize = NSize;
   else if (OpSize != NSize) WrError(1132);
}

static bool DecodeReg(char *Asc, Byte * Erg) {
   if (strcasecmp(Asc, "SP") == 0) *Erg = 7;
   else if (strcasecmp(Asc, "FP") == 0) *Erg = 6;
   else if ((strlen(Asc) == 2) && (toupper(*Asc) == 'R') && (Asc[1] >= '0') && (Asc[1] <= '7'))
      *Erg = Asc[1] - '0';
   else return false;
   return true;
}

static bool DecodeRegList(char *Asc, Byte * Erg) {
   String Part;
   Byte Reg1, Reg2, z;
   char *p;

   if (IsIndirect(Asc)) {
      strmove(Asc, 1);
      Asc[strlen(Asc) - 1] = '\0';
      KillBlanks(Asc);
   }

   *Erg = 0;
   while (*Asc != '\0') {
      p = QuotPos(Asc, ',');
      if (p == NULL) {
         strmaxcpy(Part, Asc, 255);
         *Asc = '\0';
      } else {
         *p = '\0';
         strmaxcpy(Part, Asc, 255);
         strcopy(Asc, p + 1);
      }
      if (DecodeReg(Part, &Reg1)) *Erg |= (1 << Reg1);
      else {
         p = strchr(Part, '-');
         if (p == NULL) return false;
         *p = '\0';
         if (!DecodeReg(Part, &Reg1)) return false;
         if (!DecodeReg(p + 1, &Reg2)) return false;
         if (Reg1 > Reg2) Reg2 += 8;
         for (z = Reg1; z <= Reg2; z++) *Erg |= (1 << (z & 7));
      }
   }

   return true;
}

static bool DecodeCReg(char *Asc, Byte * Erg) {
   if (strcasecmp(Asc, "SR") == 0) {
      *Erg = 0;
      SetOpSize(1);
   } else if (strcasecmp(Asc, "CCR") == 0) {
      *Erg = 1;
      SetOpSize(0);
   } else if (strcasecmp(Asc, "BR") == 0) {
      *Erg = 3;
      SetOpSize(0);
   } else if (strcasecmp(Asc, "EP") == 0) {
      *Erg = 4;
      SetOpSize(0);
   } else if (strcasecmp(Asc, "DP") == 0) {
      *Erg = 5;
      SetOpSize(0);
   } else if (strcasecmp(Asc, "TP") == 0) {
      *Erg = 7;
      SetOpSize(0);
   } else return false;
   return true;
}

static void SplitDisp(char *Asc, ShortInt * Size) {
   int l = strlen(Asc);

   if ((l > 2) && (Asc[l - 1] == '8') && (Asc[l - 2] == ':')) {
      Asc[l - 2] = '\0';
      *Size = 0;
   } else if ((l > 3) && (Asc[l - 1] == '6') && (Asc[l - 2] == '1') && (Asc[l - 3] == ':')) {
      Asc[l - 3] = '\0';
      *Size = 1;
   }
}

static void DecideAbsolute(LongInt Value, ShortInt Size, bool Unknown, Word Mask) {
   LongInt Base;

   if (Size != -1) ;
   else if (((Value >> 8) == Reg_BR) && ((Mask & MModAbs8) != 0)) Size = 0;
   else Size = 1;

   switch (Size) {
      case 0:
         if (Unknown) Value = (Value & 0xff) | (Reg_BR << 8);
         if ((Value >> 8) != Reg_BR) WrError(110);
         AdrMode = ModAbs8;
         AdrByte = 0x05;
         AdrVals[0] = Value & 0xff;
         AdrCnt = 1;
         break;
      case 1:
         Base = (Maximum) ? ((LongInt) AbsBank) << 16 : 0;
         if (Unknown) Value = (Value & 0xffff) | Base;
         if ((Value >> 16) != (Base >> 16)) WrError(110);
         AdrMode = ModAbs16;
         AdrByte = 0x15;
         AdrVals[0] = (Value >> 8) & 0xff;
         AdrVals[1] = Value & 0xff;
         AdrCnt = 2;
         break;
   }
}

static void ChkAdr(Word Mask) {
   if ((AdrMode != ModNone) && ((Mask & (1 << AdrMode)) == 0)) {
      WrError(1350);
      AdrMode = ModNone;
      AdrCnt = 0;
   }
}

static void DecodeAdr(char *Asc, Word Mask) {
   Word AdrWord;
   bool OK, Unknown;
   LongInt DispAcc;
   Byte HReg;
   ShortInt DispSize, RegPart;
   String Part;
   char *p;

   AdrMode = ModNone;
   AdrCnt = 0;

/* einfaches Register ? */

   if (DecodeReg(Asc, &AdrByte)) {
      AdrMode = ModReg;
      AdrByte += 0xa0;
      ChkAdr(Mask);
      return;
   }

/* immediate ? */

   if (*Asc == '#') {
      switch (OpSize) {
         case -1:
            OK = false;
            WrError(1131);
            break;
         case 0:
            AdrVals[0] = EvalIntExpression(Asc + 1, Int8, &OK);
            break;
         case 1:
            AdrWord = EvalIntExpression(Asc + 1, Int16, &OK);
            AdrVals[0] = Hi(AdrWord);
            AdrVals[1] = Lo(AdrWord);
            break;
      }
      if (OK) {
         AdrMode = ModImm;
         AdrByte = 0x04;
         AdrCnt = OpSize + 1;
      }
      ChkAdr(Mask);
      return;
   }

/* indirekt ? */

   if (*Asc == '@') {
      strmove(Asc, 1);
      if (IsIndirect(Asc)) {
         strmove(Asc, 1);
         Asc[strlen(Asc) - 1] = '\0';
      }

   /* Predekrement ? */

      if ((*Asc == '-') && (DecodeReg(Asc + 1, &AdrByte))) {
         AdrMode = ModPredec;
         AdrByte += 0xb0;
      }

   /* Postinkrement ? */

      else if (Asc[strlen(Asc) - 1] == '+') {
         strmaxcpy(Part, Asc, 255);
         Part[strlen(Part) - 1] = '\0';
         if (DecodeReg(Part, &AdrByte)) {
            AdrMode = ModPostInc;
            AdrByte += 0xc0;
         }
      }

   /* zusammengesetzt */

      else {
         DispAcc = 0;
         DispSize = (-1);
         RegPart = (-1);
         OK = true;
         Unknown = false;
         while ((*Asc != '\0') && (OK)) {
            p = QuotPos(Asc, ',');
            if (p == NULL) {
               strmaxcpy(Part, Asc, 255);
               *Asc = '\0';
            } else {
               *p = '\0';
               strmaxcpy(Part, Asc, 255);
               strcopy(Asc, p + 1);
            }
            if (DecodeReg(Part, &HReg))
               if (RegPart != -1) {
                  WrError(1350);
                  OK = false;
               } else RegPart = HReg;
            else {
               SplitDisp(Part, &DispSize);
               if (*Part == '#') strmove(Part, 1);
               FirstPassUnknown = false;
               DispAcc += EvalIntExpression(Part, Int32, &OK);
               if (FirstPassUnknown) Unknown = true;
            }
         }
         if (OK) {
            if (RegPart == -1) DecideAbsolute(DispAcc, DispSize, Unknown, Mask);
            else if (DispAcc == 0)
               switch (DispSize) {
                  case -1:
                     AdrMode = ModIReg;
                     AdrByte = 0xd0 + RegPart;
                     break;
                  case 0:
                     AdrMode = ModDisp8;
                     AdrByte = 0xe0 + RegPart;
                     AdrVals[0] = 0;
                     AdrCnt = 1;
                     break;
                  case 1:
                     AdrMode = ModDisp16;
                     AdrByte = 0xf0 + RegPart;
                     AdrVals[0] = 0;
                     AdrVals[1] = 0;
                     AdrCnt = 2;
                     break;
            } else {
               if (DispSize != -1) ;
               else if ((DispAcc >= -128) && (DispAcc < 127)) DispSize = 0;
               else DispSize = 1;
               switch (DispSize) {
                  case 0:
                     if (Unknown) DispAcc &= 0x7f;
                     if (ChkRange(DispAcc, -128, 127)) {
                        AdrMode = ModDisp8;
                        AdrByte = 0xe0 + RegPart;
                        AdrVals[0] = DispAcc & 0xff;
                        AdrCnt = 1;
                     }
                     break;
                  case 1:
                     if (Unknown) DispAcc &= 0x7fff;
                     if (ChkRange(DispAcc, -0x8000l, 0x7fffl)) {
                        AdrMode = ModDisp16;
                        AdrByte = 0xf0 + RegPart;
                        AdrVals[1] = DispAcc & 0xff;
                        AdrVals[0] = (DispAcc >> 8) & 0xff;
                        AdrCnt = 2;
                     }
                     break;
               }
            }
         }
      }
      ChkAdr(Mask);
      return;
   }

/* absolut */

   DispSize = (-1);
   SplitDisp(Asc, &DispSize);
   FirstPassUnknown = false;
   DispAcc = EvalIntExpression(Asc, UInt24, &OK);
   DecideAbsolute(DispAcc, DispSize, FirstPassUnknown, Mask);

   ChkAdr(Mask);
}

static LongInt ImmVal(void) {
   LongInt t;

   switch (OpSize) {
      case 0:
         t = AdrVals[0];
         if (t > 127) t -= 256;
         break;
      case 1:
         t = (((Word) AdrVals[0]) << 8) + AdrVals[1];
         if (t > 0x7fff) t -= 0x10000;
         break;
      default:
         t = 0;
         WrError(10000);
   }
   return t;
}

/*-------------------------------------------------------------------------*/

static bool CheckFormat(char *FSet) {
   char *p;

   if (strcmp(Format, " ") == 0) FormatCode = 0;
   else {
      p = strchr(FSet, *Format);
      if (p == NULL) {
         WrError(1090);
         return false;
      } else FormatCode = p - FSet + 1;
   }
   return true;
}

static bool DecodePseudo(void) {
#define ONOFFH8_5Count 1
   static ONOFFRec ONOFFH8_5s[ONOFFH8_5Count] = { { "MAXMODE", &Maximum, MaximumName } };
#define ASSUMEH8_5Count 4
   static ASSUMERec ASSUMEH8_5s[ASSUMEH8_5Count] = {
      { "DP", &Reg_DP, 0, 0xff, -1 },
      { "EP", &Reg_EP, 0, 0xff, -1 },
      { "TP", &Reg_TP, 0, 0xff, -1 },
      { "BR", &Reg_BR, 0, 0xff, -1 }
   };

   if (CodeONOFF(ONOFFH8_5s, ONOFFH8_5Count)) return true;

   if (Memo("ASSUME")) {
      CodeASSUME(ASSUMEH8_5s, ASSUMEH8_5Count);
      return true;
   }

   return false;
}

static ShortInt Adr2Mode;
static Byte Adr2Byte, Adr2Cnt;
static Byte Adr2Vals[3];

static void CopyAdr(void) {
   Adr2Mode = AdrMode;
   Adr2Byte = AdrByte;
   Adr2Cnt = AdrCnt;
   memcpy(Adr2Vals, AdrVals, AdrCnt);
}

static void MakeCode_H8_5(void) {
   Integer z, AdrInt;
   char *p;
   bool OK;
   LongInt AdrLong;
   Byte HReg;
   ShortInt HSize;

   CodeLen = 0;
   DontPrint = false;
   OpSize = (-1);
   AbsBank = Reg_DP;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

/* Formatangabe abspalten */

   switch (AttrSplit) {
      case '.':
         p = strchr(AttrPart, ':');
         if (p != NULL) {
            if (p < AttrPart + strlen(AttrPart) - 1) strmaxcpy(Format, p + 1, 255);
            else strcpy(Format, " ");
            *p = '\0';
         } else strcpy(Format, " ");
         break;
      case ':':
         p = strchr(AttrPart, '.');
         if (p == 0) {
            strmaxcpy(Format, AttrPart, 255);
            *AttrPart = '\0';
         } else {
            *p = '\0';
            if (p == AttrPart) strcpy(Format, " ");
            else strmaxcpy(Format, AttrPart, 255);
            strcopy(AttrPart, p + 1);
         }
         break;
      default:
         strcpy(Format, " ");
   }

/* Attribut abarbeiten */

   if (*AttrPart == '\0') SetOpSize(-1);
   else
      switch (toupper(*AttrPart)) {
         case 'B':
            SetOpSize(0);
            break;
         case 'W':
            SetOpSize(1);
            break;
         case 'L':
            SetOpSize(2);
            break;
         case 'Q':
            SetOpSize(3);
            break;
         case 'S':
            SetOpSize(4);
            break;
         case 'D':
            SetOpSize(5);
            break;
         case 'X':
            SetOpSize(6);
            break;
         case 'P':
            SetOpSize(7);
            break;
         default:
            WrError(1107);
            return;
      }
   NLS_UpString(Format);

   if (DecodeMoto16Pseudo(OpSize, true)) return;

/* Anweisungen ohne Argument */

   for (z = 0; z < FixedOrderCount; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else if (OpSize != -1) WrError(1100);
         else if (strcmp(Format, " ") != 0) WrError(1090);
         else {
            CodeLen = 0;
            if (Hi(FixedOrders[z].Code) != 0) BAsmCode[CodeLen++] = Hi(FixedOrders[z].Code);
            BAsmCode[CodeLen++] = Lo(FixedOrders[z].Code);
         }
         return;
      }

/* Datentransfer */

   if (Memo("MOV")) {
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("GEIFLS")) {
         if (OpSize == -1)
            SetOpSize((FormatCode == 2) ? 0 : 1);
         if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
         else {
            DecodeAdr(ArgStr[2], MModNoImm);
            if (AdrMode != ModNone) {
               CopyAdr();
               DecodeAdr(ArgStr[1], MModAll);
               if (AdrMode != ModNone) {
                  if (FormatCode != 0) ;
                  else if ((AdrMode == ModImm) && (Adr2Mode == ModReg)) FormatCode = 2 + OpSize;
                  else if ((AdrMode == ModReg) && (Adr2Byte == 0xe6)) FormatCode = 4;
                  else if ((Adr2Mode == ModReg) && (AdrByte == 0xe6)) FormatCode = 4;
                  else if ((AdrMode == ModReg) && (Adr2Mode == ModAbs8)) FormatCode = 6;
                  else if ((AdrMode == ModAbs8) && (Adr2Mode == ModReg)) FormatCode = 5;
                  else FormatCode = 1;
                  switch (FormatCode) {
                     case 1:
                        if (AdrMode == ModReg) {
                           BAsmCode[0] = Adr2Byte + (OpSize << 3);
                           memcpy(BAsmCode + 1, Adr2Vals, Adr2Cnt);
                           BAsmCode[1 + Adr2Cnt] = 0x90 + (AdrByte & 7);
                           CodeLen = 2 + Adr2Cnt;
                        } else if (Adr2Mode == ModReg) {
                           BAsmCode[0] = AdrByte + (OpSize << 3);
                           memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                           BAsmCode[1 + AdrCnt] = 0x80 + (Adr2Byte & 7);
                           CodeLen = 2 + AdrCnt;
                        } else if (AdrMode == ModImm) {
                           BAsmCode[0] = Adr2Byte + (OpSize << 3);
                           memcpy(BAsmCode + 1, Adr2Vals, Adr2Cnt);
                           if ((OpSize == 0) || ((ImmVal() >= -128) && (ImmVal() < 127))) {
                              BAsmCode[1 + Adr2Cnt] = 0x06;
                              BAsmCode[2 + Adr2Cnt] = AdrVals[OpSize];
                              CodeLen = 3 + Adr2Cnt;
                           } else {
                              BAsmCode[1 + Adr2Cnt] = 0x07;
                              memcpy(BAsmCode + 2 + Adr2Cnt, AdrVals, AdrCnt);
                              CodeLen = 2 + Adr2Cnt + AdrCnt;
                           }
                        } else WrError(1350);
                        break;
                     case 2:
                        if ((AdrMode != ModImm) || (Adr2Mode != ModReg)) WrError(1350);
                        else if (OpSize != 0) WrError(1130);
                        else {
                           BAsmCode[0] = 0x50 + (Adr2Byte & 7);
                           memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                           CodeLen = 1 + AdrCnt;
                        }
                        break;
                     case 3:
                        if ((AdrMode != ModImm) || (Adr2Mode != ModReg)) WrError(1350);
                        else if (OpSize != 1) WrError(1130);
                        else {
                           BAsmCode[0] = 0x58 + (Adr2Byte & 7);
                           memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                           CodeLen = 1 + AdrCnt;
                        }
                        break;
                     case 4:
                        if ((AdrMode == ModReg) && (Adr2Byte == 0xe6)) {
                           BAsmCode[0] = 0x90 + (OpSize << 3) + (AdrByte & 7);
                           memcpy(BAsmCode + 1, Adr2Vals, Adr2Cnt);
                           CodeLen = 1 + Adr2Cnt;
                        } else if ((Adr2Mode == ModReg) && (AdrByte == 0xe6)) {
                           BAsmCode[0] = 0x80 + (OpSize << 3) + (Adr2Byte & 7);
                           memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                           CodeLen = 1 + AdrCnt;
                        } else WrError(1350);
                        break;
                     case 5:
                        if ((AdrMode != ModAbs8) || (Adr2Mode != ModReg)) WrError(1350);
                        else {
                           BAsmCode[0] = 0x60 + (OpSize << 3) + (Adr2Byte & 7);
                           memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                           CodeLen = 1 + AdrCnt;
                        }
                        break;
                     case 6:
                        if ((Adr2Mode != ModAbs8) || (AdrMode != ModReg)) WrError(1350);
                        else {
                           BAsmCode[0] = 0x70 + (OpSize << 3) + (AdrByte & 7);
                           memcpy(BAsmCode + 1, Adr2Vals, Adr2Cnt);
                           CodeLen = 1 + Adr2Cnt;
                        }
                        break;
                  }
               }
            }
         }
      }
      return;
   }

   if ((Memo("LDC")) || (Memo("STC"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else {
         if (Memo("STC")) {
            strcopy(ArgStr[3], ArgStr[1]);
            strcopy(ArgStr[1], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[3]);
         }
         if (!DecodeCReg(ArgStr[2], &HReg)) WrXError(1440, ArgStr[2]);
         else {
            DecodeAdr(ArgStr[1], (Memo("LDC")) ? MModAll : MModNoImm);
            if (AdrMode != ModNone) {
               BAsmCode[0] = AdrByte + (OpSize << 3);
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               BAsmCode[1 + AdrCnt] = 0x88 + (Memo("STC") << 4) + HReg;
               CodeLen = 2 + AdrCnt;
            }
         }
      }
      return;
   }

   if (Memo("LDM")) {
      if (OpSize == -1) OpSize = 1;
      if (ArgCnt != 2) WrError(1110);
      else if (OpSize != 1) WrError(1130);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else if (!DecodeRegList(ArgStr[2], BAsmCode + 1)) WrError(1410);
      else {
         DecodeAdr(ArgStr[1], MModPostInc);
         if (AdrMode == ModNone) ;
         else if ((AdrByte & 7) != 7) WrError(1350);
         else {
            BAsmCode[0] = 0x02;
            CodeLen = 2;
         }
      }
      return;
   }

   if (Memo("STM")) {
      if (OpSize == -1) OpSize = 1;
      if (ArgCnt != 2) WrError(1110);
      else if (OpSize != 1) WrError(1130);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else if (!DecodeRegList(ArgStr[1], BAsmCode + 1)) WrError(1410);
      else {
         DecodeAdr(ArgStr[2], MModPredec);
         if (AdrMode == ModNone) ;
         else if ((AdrByte & 7) != 7) WrError(1350);
         else {
            BAsmCode[0] = 0x12;
            CodeLen = 2;
         }
      }
      return;
   }

   if ((Memo("MOVTPE")) || (Memo("MOVFPE"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("G")) {
         if (Memo("MOVTPE")) {
            strcopy(ArgStr[3], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[1]);
            strcopy(ArgStr[1], ArgStr[3]);
         }
         if (OpSize == -1) SetOpSize(0);
         if (OpSize != 0) WrError(1130);
         else if (!DecodeReg(ArgStr[2], &HReg)) WrError(1350);
         else {
            DecodeAdr(ArgStr[1], MModNoImm - MModReg);
            if (AdrMode != ModNone) {
               BAsmCode[0] = AdrByte + (OpSize << 3);
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               BAsmCode[1 + AdrCnt] = 0;
               BAsmCode[2 + AdrCnt] = 0x80 + HReg + (Memo("MOVTPE") << 4);
               CodeLen = 3 + AdrCnt;
            }
         }
      }
      return;
   }

/* Arithmetik mit 2 Operanden */

   if ((Memo("ADD")) || (Memo("SUB"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("GQ")) {
         if (OpSize == -1) SetOpSize(1);
         if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
         else {
            DecodeAdr(ArgStr[2], MModNoImm);
            if (AdrMode != ModNone) {
               CopyAdr();
               DecodeAdr(ArgStr[1], MModAll);
               if (AdrMode != ModNone) {
                  AdrLong = ImmVal();
                  if (FormatCode != 0) ;
                  else if ((AdrMode == ModImm) && (abs(AdrLong) >= 1) && (abs(AdrLong) <= 2)) FormatCode = 2;
                  else FormatCode = 1;
                  switch (FormatCode) {
                     case 1:
                        if (Adr2Mode != ModReg) WrError(1350);
                        else {
                           BAsmCode[0] = AdrByte + (OpSize << 3);
                           memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                           BAsmCode[1 + AdrCnt] = 0x20 + (Memo("SUB") << 4) + (Adr2Byte & 7);
                           CodeLen = 2 + AdrCnt;
                        }
                        break;
                     case 2:
                        if (!ChkRange(AdrLong, -2, 2)) ;
                        else if (AdrLong == 0) WrError(1315);
                        else {
                           if (Memo("SUB")) AdrLong = (-AdrLong);
                           BAsmCode[0] = Adr2Byte + (OpSize << 3);
                           memcpy(BAsmCode + 1, Adr2Vals, Adr2Cnt);
                           BAsmCode[1 + Adr2Cnt] = 0x08 + abs(AdrLong) - 1;
                           if (AdrLong < 0) BAsmCode[1 + Adr2Cnt] += 4;
                           CodeLen = 2 + Adr2Cnt;
                        }
                        break;
                  }
               }
            }
         }
      }
      return;
   }

   if (Memo("CMP")) {
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("GEI")) {
         if (OpSize == -1)
            SetOpSize((FormatCode == 2) ? 0 : 1);
         if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
         else {
            DecodeAdr(ArgStr[2], MModNoImm);
            if (AdrMode != ModNone) {
               CopyAdr();
               DecodeAdr(ArgStr[1], MModAll);
               if (AdrMode != ModNone) {
                  if (FormatCode != 0) ;
                  else if ((AdrMode == ModImm) && (Adr2Mode == ModReg)) FormatCode = 2 + OpSize;
                  else FormatCode = 1;
                  switch (FormatCode) {
                     case 1:
                        if (Adr2Mode == ModReg) {
                           BAsmCode[0] = AdrByte + (OpSize << 3);
                           memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                           BAsmCode[1 + AdrCnt] = 0x70 + (Adr2Byte & 7);
                           CodeLen = 2 + AdrCnt;
                        } else if (AdrMode == ModImm) {
                           BAsmCode[0] = Adr2Byte + (OpSize << 3);
                           memcpy(BAsmCode + 1, Adr2Vals, Adr2Cnt);
                           BAsmCode[1 + Adr2Cnt] = 0x04 + OpSize;
                           memcpy(BAsmCode + 2 + Adr2Cnt, AdrVals, AdrCnt);
                           CodeLen = 2 + AdrCnt + Adr2Cnt;
                        } else WrError(1350);
                        break;
                     case 2:
                        if ((AdrMode != ModImm) || (Adr2Mode != ModReg)) WrError(1350);
                        else if (OpSize != 0) WrError(1130);
                        else {
                           BAsmCode[0] = 0x40 + (Adr2Byte & 7);
                           memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                           CodeLen = 1 + AdrCnt;
                        }
                        break;
                     case 3:
                        if ((AdrMode != ModImm) || (Adr2Mode != ModReg)) WrError(1350);
                        else if (OpSize != 1) WrError(1130);
                        else {
                           BAsmCode[0] = 0x48 + (Adr2Byte & 7);
                           memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                           CodeLen = 1 + AdrCnt;
                        }
                        break;
                  }
               }
            }
         }
      }
      return;
   }

   for (z = 0; z < RegEAOrderCount; z++)
      if (Memo(RegEAOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (CheckFormat("G")) {
            if (OpSize == -1) SetOpSize(RegEAOrders[z].DefSize);
            if (((1 << OpSize) & RegEAOrders[z].SizeMask) == 0) WrError(1130);
            else if (!DecodeReg(ArgStr[2], &HReg)) WrError(1350);
            else {
               DecodeAdr(ArgStr[1], MModAll);
               if (AdrMode != ModNone) {
                  BAsmCode[0] = AdrByte + (OpSize << 3);
                  memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                  BAsmCode[1 + AdrCnt] = RegEAOrders[z].Code + HReg;
                  CodeLen = 2 + AdrCnt;
               }
            }
         }
         return;
      }

   for (z = 0; z < TwoRegOrderCount; z++)
      if (Memo(TwoRegOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (strcmp(Format, " ") != 0) WrError(1090);
         else if (!DecodeReg(ArgStr[1], &HReg)) WrError(1350);
         else if (!DecodeReg(ArgStr[2], &AdrByte)) WrError(1350);
         else {
            if (OpSize == -1) SetOpSize(TwoRegOrders[z].DefSize);
            if (((1 << OpSize) & TwoRegOrders[z].SizeMask) == 0) WrError(1130);
            else {
               BAsmCode[0] = 0xa0 + HReg + (OpSize << 3);
               if (Hi(TwoRegOrders[z].Code) != 0) {
                  BAsmCode[1] = Lo(TwoRegOrders[z].Code);
                  BAsmCode[2] = Hi(TwoRegOrders[z].Code) + AdrByte;
                  CodeLen = 3;
               } else {
                  BAsmCode[1] = TwoRegOrders[z].Code + AdrByte;
                  CodeLen = 2;
               }
            }
         }
         return;
      }

   for (z = 0; z < LogOrderCount; z++)
      if (Memo(LogOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (!DecodeCReg(ArgStr[2], &HReg)) WrXError(1440, ArgStr[2]);
         else {
            DecodeAdr(ArgStr[1], MModImm);
            if (AdrMode != ModNone) {
               BAsmCode[0] = AdrByte + (OpSize << 3);
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               BAsmCode[1 + AdrCnt] = LogOrders[z].Code + HReg;
               CodeLen = 2 + AdrCnt;
            }
         }
         return;
      }

/* Arithmetik mit einem Operanden */

   for (z = 0; z < OneOrderCount; z++)
      if (Memo(OneOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (CheckFormat("G")) {
            if (OpSize == -1) SetOpSize(OneOrders[z].DefSize);
            if (((1 << OpSize) & OneOrders[z].SizeMask) == 0) WrError(1130);
            else {
               DecodeAdr(ArgStr[1], MModNoImm);
               if (AdrMode != ModNone) {
                  BAsmCode[0] = AdrByte + (OpSize << 3);
                  memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                  BAsmCode[1 + AdrCnt] = OneOrders[z].Code;
                  CodeLen = 2 + AdrCnt;
               }
            }
         }
         return;
      }

   for (z = 0; z < OneRegOrderCount; z++)
      if (Memo(OneRegOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (strcmp(Format, " ") != 0) WrError(1090);
         else if (!DecodeReg(ArgStr[1], &HReg)) WrError(1350);
         else {
            if (OpSize == -1) SetOpSize(OneRegOrders[z].DefSize);
            if (((1 << OpSize) & OneRegOrders[z].SizeMask) == 0) WrError(1130);
            else {
               BAsmCode[0] = 0xa0 + HReg + (OpSize << 3);
               BAsmCode[1] = OneRegOrders[z].Code;
               CodeLen = 2;
            }
         }
         return;
      }

/* Bitoperationen */

   for (z = 0; z < BitOrderCount; z++)
      if (Memo(BitOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else {
            if (OpSize == -1) OpSize = 0;
            if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
            else {
               DecodeAdr(ArgStr[2], MModNoImm);
               if (AdrMode != ModNone) {
                  if (DecodeReg(ArgStr[1], &HReg)) {
                     OK = true;
                     HReg += 8;
                  } else {
                     if (*ArgStr[1] == '#') strmove(ArgStr[1], 1);
                     HReg = EvalIntExpression(ArgStr[1], (OpSize == 0) ? UInt3 : UInt4, &OK);
                     if (OK) HReg += 0x80;
                  }
                  if (OK) {
                     BAsmCode[0] = AdrByte + (OpSize << 3);
                     memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                     BAsmCode[1 + AdrCnt] = BitOrders[z].Code + HReg;
                     CodeLen = 2 + AdrCnt;
                  }
               }
            }
         }
         return;
      }

/* Spruenge */

   for (z = 0; z < RelOrderCount; z++)
      if (Memo(RelOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (strcmp(Format, " ") != 0) WrError(1090);
         else {
            FirstPassUnknown = false;
            AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
            if (!OK) ;
            else if (((AdrLong >> 16) != (EProgCounter() >> 16)) && (!FirstPassUnknown) && (!SymbolQuestionable)) WrError(1910);
            else if ((EProgCounter() & 0xffff) >= 0xfffc) WrError(1905);
            else {
               AdrLong -= EProgCounter() + 2;
               if (AdrLong > 0x7fff) AdrLong -= 0x10000;
               else if (AdrLong < -0x8000l) AdrLong += 0x10000;
               if (OpSize != -1) ;
               else if ((AdrLong <= 127) && (AdrLong >= -128)) OpSize = 4;
               else OpSize = 2;
               switch (OpSize) {
                  case 2:
                     AdrLong--;
                     BAsmCode[0] = RelOrders[z].Code + 0x10;
                     BAsmCode[1] = (AdrLong >> 8) & 0xff;
                     BAsmCode[2] = AdrLong & 0xff;
                     CodeLen = 3;
                     break;
                  case 4:
                     if (((AdrLong < -128) || (AdrLong > 127)) && (!SymbolQuestionable)) WrError(1370);
                     else {
                        BAsmCode[0] = RelOrders[z].Code;
                        BAsmCode[1] = AdrLong & 0xff;
                        CodeLen = 2;
                     }
                     break;
                  default:
                     WrError(1130);
               }
            }
         }
         return;
      }

   if ((Memo("JMP")) || (Memo("JSR"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (CheckFormat("G")) {
         AbsBank = EProgCounter() >> 16;
         HReg = Memo("JSR") << 3;
         DecodeAdr(ArgStr[1], MModIReg + MModReg + MModDisp8 + MModDisp16 + MModAbs16);
         switch (AdrMode) {
            case ModReg:
            case ModIReg:
               BAsmCode[0] = 0x11;
               BAsmCode[1] = 0xd0 + HReg + (AdrByte & 7);
               CodeLen = 2;
               break;
            case ModDisp8:
            case ModDisp16:
               BAsmCode[0] = 0x11;
               BAsmCode[1] = AdrByte + HReg;
               memcpy(BAsmCode + 2, AdrVals, AdrCnt);
               CodeLen = 2 + AdrCnt;
               break;
            case ModAbs16:
               BAsmCode[0] = 0x10 + HReg;
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               CodeLen = 1 + AdrCnt;
               break;
         }
      }
      return;
   }

   if (strncmp(OpPart, "SCB/", 4) == 0) {
      if (ArgCnt != 2) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else if (!DecodeReg(ArgStr[1], &HReg)) WrError(1350);
      else {
         strmaxcpy(ArgStr[3], OpPart + 4, 255);
         OK = true;
         NLS_UpString(ArgStr[3]);
         if (strcmp(ArgStr[3], "F") == 0) BAsmCode[0] = 0x01;
         else if (strcmp(ArgStr[3], "NE") == 0) BAsmCode[0] = 0x06;
         else if (strcmp(ArgStr[3], "EQ") == 0) BAsmCode[0] = 0x07;
         else OK = false;
         if (!OK) WrError(1360);
         else {
            FirstPassUnknown = false;
            AdrLong = EvalIntExpression(ArgStr[2], UInt24, &OK);
            if (!OK) ;
            else if (((AdrLong >> 16) != (EProgCounter() >> 16)) && (!FirstPassUnknown) && (!SymbolQuestionable)) WrError(1910);
            else if ((EProgCounter() & 0xffff) >= 0xfffc) WrError(1905);
            else {
               AdrLong -= EProgCounter() + 3;
               if ((!SymbolQuestionable) && ((AdrLong > 127) || (AdrLong < -128))) WrError(1370);
               else {
                  BAsmCode[1] = 0xb8 + HReg;
                  BAsmCode[2] = AdrLong & 0xff;
                  CodeLen = 3;
               }
            }
         }
      }
      return;
   }

   if ((Memo("PJMP")) || (Memo("PJSR"))) {
      z = Memo("PJMP");
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else if (!Maximum) WrError(1997);
      else {
         if (*ArgStr[1] == '@') strmove(ArgStr[1], 1);
         if (DecodeReg(ArgStr[1], &HReg)) {
            BAsmCode[0] = 0x11;
            BAsmCode[1] = 0xc0 + ((1 - z) << 3) + HReg;
            CodeLen = 2;
         } else {
            AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
            if (OK) {
               BAsmCode[0] = 0x03 + (z << 4);
               BAsmCode[1] = (AdrLong >> 16) & 0xff;
               BAsmCode[2] = (AdrLong >> 8) & 0xff;
               BAsmCode[3] = AdrLong & 0xff;
               CodeLen = 4;
            }
         }
      }
      return;
   }

   if ((Memo("PRTD")) || (Memo("RTD"))) {
      HReg = Memo("PRTD");
      if (ArgCnt != 1) WrError(1110);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else if (*ArgStr[1] != '#') WrError(1120);
      else {
         strmove(ArgStr[1], 1);
         HSize = (-1);
         SplitDisp(ArgStr[1], &HSize);
         if (HSize != -1) SetOpSize(HSize);
         FirstPassUnknown = false;
         AdrInt = EvalIntExpression(ArgStr[1], SInt16, &OK);
         if (FirstPassUnknown) AdrInt &= 127;
         if (OK) {
            if (OpSize != -1) ;
            else if ((AdrInt < 127) && (AdrInt > -128)) OpSize = 0;
            else OpSize = 1;
            if (Memo("PRTD")) BAsmCode[0] = 0x11;
            switch (OpSize) {
               case 0:
                  if (ChkRange(AdrInt, -128, 127)) {
                     BAsmCode[HReg] = 0x14;
                     BAsmCode[1 + HReg] = AdrInt & 0xff;
                     CodeLen = 2 + HReg;
                  }
                  break;
               case 1:
                  BAsmCode[HReg] = 0x1c;
                  BAsmCode[1 + HReg] = (AdrInt >> 8) & 0xff;
                  BAsmCode[2 + HReg] = AdrInt & 0xff;
                  CodeLen = 3 + HReg;
                  break;
               default:
                  WrError(1130);
            }
         }
      }
      return;
   }

/* Sonderfaelle */

   if (Memo("LINK")) {
      if (ArgCnt != 2) WrError(1110);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else {
         DecodeAdr(ArgStr[1], MModReg);
         if (AdrMode == ModNone) ;
         else if ((AdrByte & 7) != 6) WrError(1350);
         else if (*ArgStr[2] != '#') WrError(1120);
         else {
            strmove(ArgStr[2], 1);
            HSize = (-1);
            SplitDisp(ArgStr[2], &HSize);
            if (HSize != -1) SetOpSize(HSize);
            FirstPassUnknown = false;
            AdrInt = EvalIntExpression(ArgStr[2], SInt16, &OK);
            if (FirstPassUnknown) AdrInt &= 127;
            if (OK) {
               if (OpSize != -1) ;
               else if ((AdrInt < 127) && (AdrInt > -128)) OpSize = 0;
               else OpSize = 1;
               switch (OpSize) {
                  case 0:
                     if (ChkRange(AdrInt, -128, 127)) {
                        BAsmCode[0] = 0x17;
                        BAsmCode[1] = AdrInt & 0xff;
                        CodeLen = 2;
                     }
                     break;
                  case 1:
                     BAsmCode[0] = 0x1f;
                     BAsmCode[1] = (AdrInt >> 8) & 0xff;
                     BAsmCode[2] = AdrInt & 0xff;
                     CodeLen = 3;
                     break;
                  default:
                     WrError(1130);
               }
            }
         }
      }
      return;
   }

   if (Memo("UNLK")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else {
         DecodeAdr(ArgStr[1], MModReg);
         if (AdrMode == ModNone) ;
         else if ((AdrByte & 7) != 6) WrError(1350);
         else {
            BAsmCode[0] = 0x0f;
            CodeLen = 1;
         }
      }
      return;
   }

   if (Memo("TRAPA")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else if (*ArgStr[1] != '#') WrError(1120);
      else {
         BAsmCode[1] = 0x10 + EvalIntExpression(ArgStr[1] + 1, UInt4, &OK);
         if (OK) {
            BAsmCode[0] = 0x08;
            CodeLen = 2;
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_H8_5(void) {
   if (ActPC == SegCode)
      return (ProgCounter() < (Maximum ? 0x1000000 : 0x10000));
   else return false;
}

static bool IsDef_H8_5(void) {
   return false;
}

static void SwitchFrom_H8_5(void) {
   DeinitFields();
}

static void InitCode_H8_5(void) {
   SaveInitProc();
   Reg_DP = (-1);
   Reg_EP = (-1);
   Reg_TP = (-1);
   Reg_BR = (-1);
}

static void SwitchTo_H8_5(void) {
   TurnWords = true;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "*";
   HeaderID = 0x69;
   NOPCode = 0x00;
   DivideChars = ",";
   HasAttrs = true;
   AttrChars = ".:";

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_H8_5;
   ChkPC = ChkPC_H8_5;
   IsDef = IsDef_H8_5;
   SwitchFrom = SwitchFrom_H8_5;

   InitFields();
}

void codeh8_5_init(void) {
   CPU532 = AddCPU("HD6475328", SwitchTo_H8_5);
   CPU534 = AddCPU("HD6475348", SwitchTo_H8_5);
   CPU536 = AddCPU("HD6475368", SwitchTo_H8_5);
   CPU538 = AddCPU("HD6475388", SwitchTo_H8_5);

   SaveInitProc = InitPassProc;
   InitPassProc = InitCode_H8_5;
}
