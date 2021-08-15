/* codez80.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Zilog Z80/180/380                                           */
/*                                                                           */
/* Historie: 26.8.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "stringutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmcode.h"
#include "codepseudo.h"
#include "codevars.h"

/*-------------------------------------------------------------------------*/
/* Instruktionsgruppendefinitionen */

typedef struct {
   char *Name;
   CPUVar MinCPU;
   Byte Len;
   Word Code;
} BaseOrder;

typedef struct {
   char *Name;
   Byte Code;
} Condition;

typedef struct {
   char *Name;
   Byte Code;
} ALUOrder;

/*-------------------------------------------------------------------------*/
/* Praefixtyp */

typedef enum { Pref_IN_N, Pref_IN_W, Pref_IB_W, Pref_IW_W, Pref_IB_N,
   Pref_IN_LW, Pref_IB_LW, Pref_IW_LW, Pref_IW_N
} PrefType;

#define ExtFlagName    "INEXTMODE" /* Flag-Symbolnamen */
#define LWordFlagName  "INLWORDMODE"

#define ModNone (-1)
#define ModReg8 1
#define ModReg16 2
#define ModIndReg16 3
#define ModImm 4
#define ModAbs 5
#define ModRef 6
#define ModInt 7
#define ModSPRel 8

#define FixedOrderCnt 53
#define AccOrderCnt 3
#define HLOrderCnt 3
#define ALUOrderCnt 5
#define ShiftOrderCnt 8
#define BitOrderCnt 3
#define ConditionCnt 12

#define IXPrefix 0xdd
#define IYPrefix 0xfd

/*-------------------------------------------------------------------------*/

static Byte PrefixCnt;
static Byte AdrPart, OpSize;
static Byte AdrVals[4];
static ShortInt AdrMode;

static BaseOrder *FixedOrders;
static BaseOrder *AccOrders;
static BaseOrder *HLOrders;
static ALUOrder *ALUOrders;
static char **ShiftOrders;
static char **BitOrders;
static Condition *Conditions;

static SimpProc SaveInitProc;

static CPUVar CPUZ80, CPUZ80U, CPUZ180, CPUZ380;

static bool MayLW, /* Instruktion erlaubt 32 Bit */
 ExtFlag, /* Prozessor im 4GByte-Modus ? */
 LWordFlag; /* 32-Bit-Verarbeitung ? */

static PrefType CurrPrefix, /* mom. explizit erzeugter Praefix */
 LastPrefix; /* von der letzten Anweisung generierter Praefix */

/*==========================================================================*/
/* Codetabellenerzeugung */

static void AddFixed(char *NewName, CPUVar NewMin, Byte NewLen, Word NewCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NewName;
   FixedOrders[InstrZ].MinCPU = NewMin;
   FixedOrders[InstrZ].Len = NewLen;
   FixedOrders[InstrZ++].Code = NewCode;
}

static void AddAcc(char *NewName, CPUVar NewMin, Byte NewLen, Word NewCode) {
   if (InstrZ >= AccOrderCnt) exit(255);
   AccOrders[InstrZ].Name = NewName;
   AccOrders[InstrZ].MinCPU = NewMin;
   AccOrders[InstrZ].Len = NewLen;
   AccOrders[InstrZ++].Code = NewCode;
}

static void AddHL(char *NewName, CPUVar NewMin, Byte NewLen, Word NewCode) {
   if (InstrZ >= HLOrderCnt) exit(255);
   HLOrders[InstrZ].Name = NewName;
   HLOrders[InstrZ].MinCPU = NewMin;
   HLOrders[InstrZ].Len = NewLen;
   HLOrders[InstrZ++].Code = NewCode;
}

static void AddALU(char *NewName, Byte NCode) {
   if (InstrZ >= ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name = NewName;
   ALUOrders[InstrZ++].Code = NCode;
}

static void AddShift(char *NName) {
   if (InstrZ >= ShiftOrderCnt) exit(255);
   ShiftOrders[InstrZ++] = NName;
}

static void AddBit(char *NName) {
   if (InstrZ >= BitOrderCnt) exit(255);
   BitOrders[InstrZ++] = NName;
}

static void AddCondition(char *NewName, Byte NewCode) {
   if (InstrZ >= ConditionCnt) exit(255);
   Conditions[InstrZ].Name = NewName;
   Conditions[InstrZ++].Code = NewCode;
}

static void InitFields(void) {
   InstrZ = 0;
   Conditions = (Condition *) malloc(sizeof(Condition) * ConditionCnt);
   AddCondition("NZ", 0);
   AddCondition("Z", 1);
   AddCondition("NC", 2);
   AddCondition("C", 3);
   AddCondition("PO", 4);
   AddCondition("NV", 4);
   AddCondition("PE", 5);
   AddCondition("V", 5);
   AddCondition("P", 6);
   AddCondition("NS", 6);
   AddCondition("M", 7);
   AddCondition("S", 7);

   InstrZ = 0;
   FixedOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * FixedOrderCnt);
   AddFixed("EXX", CPUZ80, 1, 0x00d9);
   AddFixed("LDI", CPUZ80, 2, 0xeda0);
   AddFixed("LDIR", CPUZ80, 2, 0xedb0);
   AddFixed("LDD", CPUZ80, 2, 0xeda8);
   AddFixed("LDDR", CPUZ80, 2, 0xedb8);
   AddFixed("CPI", CPUZ80, 2, 0xeda1);
   AddFixed("CPIR", CPUZ80, 2, 0xedb1);
   AddFixed("CPD", CPUZ80, 2, 0xeda9);
   AddFixed("CPDR", CPUZ80, 2, 0xedb9);
   AddFixed("RLCA", CPUZ80, 1, 0x0007);
   AddFixed("RRCA", CPUZ80, 1, 0x000f);
   AddFixed("RLA", CPUZ80, 1, 0x0017);
   AddFixed("RRA", CPUZ80, 1, 0x001f);
   AddFixed("RLD", CPUZ80, 2, 0xed6f);
   AddFixed("RRD", CPUZ80, 2, 0xed67);
   AddFixed("DAA", CPUZ80, 1, 0x0027);
   AddFixed("CCF", CPUZ80, 1, 0x003f);
   AddFixed("SCF", CPUZ80, 1, 0x0037);
   AddFixed("NOP", CPUZ80, 1, 0x0000);
   AddFixed("HALT", CPUZ80, 1, 0x0076);
   AddFixed("RETI", CPUZ80, 2, 0xed4d);
   AddFixed("RETN", CPUZ80, 2, 0xed45);
   AddFixed("INI", CPUZ80, 2, 0xeda2);
   AddFixed("INIR", CPUZ80, 2, 0xedb2);
   AddFixed("IND", CPUZ80, 2, 0xedaa);
   AddFixed("INDR", CPUZ80, 2, 0xedba);
   AddFixed("OUTI", CPUZ80, 2, 0xeda3);
   AddFixed("OTIR", CPUZ80, 2, 0xedb3);
   AddFixed("OUTD", CPUZ80, 2, 0xedab);
   AddFixed("OTDR", CPUZ80, 2, 0xedbb);
   AddFixed("SLP", CPUZ180, 2, 0xed76);
   AddFixed("OTIM", CPUZ180, 2, 0xed83);
   AddFixed("OTIMR", CPUZ180, 2, 0xed93);
   AddFixed("OTDM", CPUZ180, 2, 0xed8b);
   AddFixed("OTDMR", CPUZ180, 2, 0xed9b);
   AddFixed("BTEST", CPUZ380, 2, 0xedcf);
   AddFixed("EXALL", CPUZ380, 2, 0xedd9);
   AddFixed("EXXX", CPUZ380, 2, 0xddd9);
   AddFixed("EXXY", CPUZ380, 2, 0xfdd9);
   AddFixed("INDW", CPUZ380, 2, 0xedea);
   AddFixed("INDRW", CPUZ380, 2, 0xedfa);
   AddFixed("INIW", CPUZ380, 2, 0xede2);
   AddFixed("INIRW", CPUZ380, 2, 0xedf2);
   AddFixed("LDDW", CPUZ380, 2, 0xede8);
   AddFixed("LDDRW", CPUZ380, 2, 0xedf8);
   AddFixed("LDIW", CPUZ380, 2, 0xede0);
   AddFixed("LDIRW", CPUZ380, 2, 0xedf0);
   AddFixed("MTEST", CPUZ380, 2, 0xddcf);
   AddFixed("OTDRW", CPUZ380, 2, 0xedfb);
   AddFixed("OTIRW", CPUZ380, 2, 0xedf3);
   AddFixed("OUTDW", CPUZ380, 2, 0xedeb);
   AddFixed("OUTIW", CPUZ380, 2, 0xede3);
   AddFixed("RETB", CPUZ380, 2, 0xed55);

   InstrZ = 0;
   AccOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * AccOrderCnt);
   AddAcc("CPL", CPUZ80, 1, 0x002f);
   AddAcc("NEG", CPUZ80, 2, 0xed44);
   AddAcc("EXTS", CPUZ380, 2, 0xed65);

   InstrZ = 0;
   HLOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * HLOrderCnt);
   AddHL("CPLW", CPUZ380, 2, 0xdd2f);
   AddHL("NEGW", CPUZ380, 2, 0xed54);
   AddHL("EXTSW", CPUZ380, 2, 0xed75);

   InstrZ = 0;
   ALUOrders = (ALUOrder *) malloc(sizeof(ALUOrder) * ALUOrderCnt);
   AddALU("SUB", 2);
   AddALU("AND", 4);
   AddALU("OR", 6);
   AddALU("XOR", 5);
   AddALU("CP", 7);

   InstrZ = 0;
   ShiftOrders = (char **)malloc(sizeof(char *) * ShiftOrderCnt);
   AddShift("RLC");
   AddShift("RRC");
   AddShift("RL");
   AddShift("RR");
   AddShift("SLA");
   AddShift("SRA");
   AddShift("SLIA");
   AddShift("SRL");

   InstrZ = 0;
   BitOrders = (char **)malloc(sizeof(char *) * BitOrderCnt);
   AddBit("BIT");
   AddBit("RES");
   AddBit("SET");
}

static void DeinitFields(void) {
   free(Conditions);
   free(FixedOrders);
   free(AccOrders);
   free(HLOrders);
   free(ALUOrders);
   free(ShiftOrders);
   free(BitOrders);
}

/*==========================================================================*/
/* Adressbereiche */

static LongInt CodeEnd(void) {
#ifdef __STDC__
   if (ExtFlag) return 0xffffffffu;
#else
   if (ExtFlag) return 0xffffffff;
#endif
   else if (MomCPU == CPUZ180) return 0x7ffff;
   else return 0xffff;
}

static LongInt PortEnd(void) {
#ifdef __STDC__
   if (ExtFlag) return 0xffffffffu;
#else
   if (ExtFlag) return 0xffffffff;
#endif
   else return 0xff;
}

/*==========================================================================*/
/* Praefix dazuaddieren */

static bool ExtendPrefix(PrefType * Dest, char *AddArg) {
   Byte SPart, IPart;

   switch (*Dest) {
      case Pref_IB_N:
      case Pref_IB_W:
      case Pref_IB_LW:
         IPart = 1;
         break;
      case Pref_IW_N:
      case Pref_IW_W:
      case Pref_IW_LW:
         IPart = 2;
         break;
      default:
         IPart = 0;
   }

   switch (*Dest) {
      case Pref_IN_W:
      case Pref_IB_W:
      case Pref_IW_W:
         SPart = 1;
         break;
      case Pref_IN_LW:
      case Pref_IB_LW:
      case Pref_IW_LW:
         SPart = 2;
         break;
      default:
         SPart = 0;
   }

   if (strcmp(AddArg, "W") == 0) /* Wortverarbeitung */
      SPart = 1;
   else if (strcmp(AddArg, "LW") == 0) /* Langwortverarbeitung */
      SPart = 2;
   else if (strcmp(AddArg, "IB") == 0) /* ein Byte im Argument mehr */
      IPart = 1;
   else if (strcmp(AddArg, "IW") == 0) /* ein Wort im Argument mehr */
      IPart = 2;
   else return false;

   switch ((IPart << 4) + SPart) {
      case 0x00:
         *Dest = Pref_IN_N;
         break;
      case 0x01:
         *Dest = Pref_IN_W;
         break;
      case 0x02:
         *Dest = Pref_IN_LW;
         break;
      case 0x10:
         *Dest = Pref_IB_N;
         break;
      case 0x11:
         *Dest = Pref_IB_W;
         break;
      case 0x12:
         *Dest = Pref_IB_LW;
         break;
      case 0x20:
         *Dest = Pref_IW_N;
         break;
      case 0x21:
         *Dest = Pref_IW_W;
         break;
      case 0x22:
         *Dest = Pref_IW_LW;
         break;
   }

   return true;
}

/*--------------------------------------------------------------------------*/
/* Code fuer Praefix bilden */

static void GetPrefixCode(PrefType inp, Byte * b1, Byte * b2) {
   Integer z;

   z = inp - 1;
   *b1 = 0xdd + ((z & 4) << 3);
   *b2 = 0xc0 + (z & 3);
}

/*--------------------------------------------------------------------------*/
/* DD-Praefix addieren, nur EINMAL pro Instruktion benutzen! */

static void ChangeDDPrefix(char *Add) {
   PrefType ActPrefix;
   Integer z;

   ActPrefix = LastPrefix;
   if (ExtendPrefix(&ActPrefix, Add))
      if (LastPrefix != ActPrefix) {
         if (LastPrefix != Pref_IN_N) RetractWords(2);
         for (z = PrefixCnt - 1; z >= 0; z--) BAsmCode[2 + z] = BAsmCode[z];
         PrefixCnt += 2;
         GetPrefixCode(ActPrefix, BAsmCode + 0, BAsmCode + 1);
      }
}

/*--------------------------------------------------------------------------*/
/* Wortgroesse ? */

static bool InLongMode(void) {
   switch (LastPrefix) {
      case Pref_IN_W:
      case Pref_IB_W:
      case Pref_IW_W:
         return false;
      case Pref_IN_LW:
      case Pref_IB_LW:
      case Pref_IW_LW:
         return MayLW;
      default:
         return LWordFlag && MayLW;
   }
}

/*--------------------------------------------------------------------------*/
/* absolute Adresse */

static LongWord EvalAbsAdrExpression(char *inp, bool *OK) {
   return EvalIntExpression(inp, ExtFlag ? Int32 : UInt16, OK);
}

/*==========================================================================*/
/* Adressparser */

static bool DecodeReg8(char *Asc, Byte * Erg) {
#define Reg8Cnt 7
   static char *Reg8Names[Reg8Cnt] = { "B", "C", "D", "E", "H", "L", "A" };
   Integer z;

   for (z = 0; z < Reg8Cnt; z++)
      if (strcasecmp(Asc, Reg8Names[z]) == 0) {
         *Erg = z;
         if (z == 6) (*Erg)++;
         return true;
      }

   return false;
}

static bool IsSym(char ch) {
   return ((ch == '_') || ((ch >= '0') && (ch <= '9')) || ((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z')));
}

static void DecodeAdr(char *Asc_O) {
#define Reg8XCnt 4
   static char *Reg8XNames[Reg8XCnt] = { "IXU", "IXL", "IYU", "IYL" };
#define Reg16Cnt 6
   static char *Reg16Names[Reg16Cnt] = { "BC", "DE", "HL", "SP", "IX", "IY" };

   Integer z, AdrInt;
   LongInt AdrLong;
   bool OK;
   String Asc;

   AdrMode = ModNone;
   AdrCnt = 0;
   AdrPart = 0;

/* 0. Sonderregister */

   if (strcasecmp(Asc_O, "R") == 0) {
      AdrMode = ModRef;
      return;
   }

   if (strcasecmp(Asc_O, "I") == 0) {
      AdrMode = ModInt;
      return;
   }

/* 1. 8-Bit-Register ? */

   if (DecodeReg8(Asc_O, &AdrPart)) {
      AdrMode = ModReg8;
      return;
   }

/* 1a. 8-Bit-Haelften von IX/IY ? (nur Z380, sonst als Symbole zulassen) */

   if ((MomCPU >= CPUZ380) || (MomCPU == CPUZ80U))
      for (z = 0; z < Reg8XCnt; z++)
         if (strcasecmp(Asc_O, Reg8XNames[z]) == 0) {
            AdrMode = ModReg8;
            BAsmCode[PrefixCnt++] = (z <= 1) ? IXPrefix : IYPrefix;
            AdrPart = 4 + (z & 1); /* = H /L */
            return;
         }

/* 2. 16-Bit-Register ? */

   for (z = 0; z < Reg16Cnt; z++)
      if (strcasecmp(Asc_O, Reg16Names[z]) == 0) {
         AdrMode = ModReg16;
         if (z <= 3) AdrPart = z;
         else {
            BAsmCode[PrefixCnt++] = (z == 4) ? IXPrefix : IYPrefix;
            AdrPart = 2; /* = HL */
         }
         return;
      }

/* 3. 16-Bit-Register indirekt ? */

   if ((strlen(Asc_O) >= 4) && (*Asc_O == '(') && (Asc_O[strlen(Asc_O) - 1] == ')'))
      for (z = 0; z < Reg16Cnt; z++)
         if ((strncasecmp(Asc_O + 1, Reg16Names[z], 2) == 0)
            && (!IsSym(Asc_O[3]))) {
            if (z < 3) {
               if (strlen(Asc_O) != 4) {
                  WrError(1350);
                  return;
               }
               switch (z) {
                  case 0:
                  case 1: /* BC,DE */
                     AdrMode = ModIndReg16;
                     AdrPart = z;
                     break;
                  case 2: /* HL=M-Register */
                     AdrMode = ModReg8;
                     AdrPart = 6;
                     break;
               }
            } else { /* SP,IX,IY */
               strmaxcpy(Asc, Asc_O + 3, 255);
               Asc[strlen(Asc) - 1] = '\0';
               if (*Asc == '+') strcpy(Asc, Asc + 1);
               AdrLong = EvalIntExpression(Asc, (MomCPU >= CPUZ380) ? SInt24 : SInt8, &OK);
               if (OK) {
                  if (z == 3) AdrMode = ModSPRel;
                  else {
                     AdrMode = ModReg8;
                     AdrPart = 6;
                     BAsmCode[PrefixCnt++] = (z == 4) ? IXPrefix : IYPrefix;
                  }
                  AdrVals[AdrCnt++] = AdrLong & 0xff;
                  if ((AdrLong >= -0x80l) && (AdrLong <= 0x7fl));
                  else {
                     AdrVals[AdrCnt++] = (AdrLong >> 8) & 0xff;
                     if ((AdrLong >= -0x8000l) && (AdrLong <= 0x7fffl)) ChangeDDPrefix("IB");
                     else {
                        AdrVals[AdrCnt++] = (AdrLong >> 16) & 0xff;
                        ChangeDDPrefix("IW");
                     }
                  }
               }
            }
            return;
         }

/* absolut ? */

   if (IsIndirect(Asc_O)) {
      AdrLong = EvalAbsAdrExpression(Asc_O, &OK);
      if (OK) {
         ChkSpace(SegCode);
         AdrMode = ModAbs;
         AdrVals[0] = AdrLong & 0xff;
         AdrVals[1] = (AdrLong >> 8) & 0xff;
         AdrCnt = 2;
#ifdef __STDC__
         if ((AdrLong & 0xffff0000u) == 0);
#else
         if ((AdrLong & 0xffff0000) == 0);
#endif
         else {
            AdrVals[AdrCnt++] = ((AdrLong >> 16) & 0xff);
#ifdef __STDC__
            if ((AdrLong & 0xff000000u) == 0) ChangeDDPrefix("IB");
#else
            if ((AdrLong & 0xff000000) == 0) ChangeDDPrefix("IB");
#endif
            else {
               AdrVals[AdrCnt++] = ((AdrLong >> 24) & 0xff);
               ChangeDDPrefix("IW");
            }
         }
      }
      return;
   }

/* ...immediate */

   switch (OpSize) {
      case 0xff:
         WrError(1132);
         break;
      case 0:
         AdrVals[0] = EvalIntExpression(Asc_O, Int8, &OK);
         if (OK) {
            AdrMode = ModImm;
            AdrCnt = 1;
         };
         break;
      case 1:
         if (InLongMode()) {
            AdrLong = EvalIntExpression(Asc_O, Int32, &OK);
            if (OK) {
               AdrVals[0] = Lo(AdrLong);
               AdrVals[1] = Hi(AdrLong);
               AdrMode = ModImm;
               AdrCnt = 2;
#ifdef __STDC__
               if ((AdrLong & 0xffff0000u) == 0);
#else
               if ((AdrLong & 0xffff0000) == 0);
#endif
               else {
                  AdrVals[AdrCnt++] = (AdrLong >> 16) & 0xff;
#ifdef __STDC__
                  if ((AdrLong & 0xff000000u) == 0) ChangeDDPrefix("IB");
#else
                  if ((AdrLong & 0xff000000) == 0) ChangeDDPrefix("IB");
#endif
                  else {
                     AdrVals[AdrCnt++] = (AdrLong >> 24) & 0xff;
                     ChangeDDPrefix("IW");
                  }
               }
            }
         } else {
            AdrInt = EvalIntExpression(Asc_O, Int16, &OK);
            if (OK) {
               AdrVals[0] = Lo(AdrInt);
               AdrVals[1] = Hi(AdrInt);
               AdrMode = ModImm;
               AdrCnt = 2;
            }
         }
         break;
   }
}

/*-------------------------------------------------------------------------*/
/* Bedingung entschluesseln */

static bool DecodeCondition(char *Name, Integer * Erg) {
   Integer z;
   String Name_N;

   strmaxcpy(Name_N, Name, 255);
   NLS_UpString(Name_N);

   z = 0;
   while ((z < ConditionCnt) && (strcmp(Conditions[z].Name, Name_N) != 0)) z++;
   if (z > ConditionCnt) return false;
   else {
      *Erg = Conditions[z].Code;
      return true;
   }
}

/*-------------------------------------------------------------------------*/
/* Sonderregister dekodieren */

static bool DecodeSFR(char *Inp, Byte * Erg) {
   if (strcasecmp(Inp, "SR") == 0) *Erg = 1;
   else if (strcasecmp(Inp, "XSR") == 0) *Erg = 5;
   else if (strcasecmp(Inp, "DSR") == 0) *Erg = 6;
   else if (strcasecmp(Inp, "YSR") == 0) *Erg = 7;
   else return false;
   return true;
}

/*=========================================================================*/

static bool DecodePseudo(void) {
#define ONOFFZ80Count 2
   static ONOFFRec ONOFFZ80s[ONOFFZ80Count] = { { "EXTMODE", &ExtFlag, ExtFlagName },
   { "LWORDMODE", &LWordFlag, LWordFlagName }
   };

   if (Memo("PORT")) {
      CodeEquate(SegIO, 0, PortEnd());
      return true;
   }

/* erweiterte Modi nur bei Z380 */

   if (CodeONOFF(ONOFFZ80s, ONOFFZ80Count)) {
      if (MomCPU < CPUZ380) {
         WrError(1500);
         SetFlag(&ExtFlag, ExtFlagName, false);
         SetFlag(&LWordFlag, LWordFlagName, false);
      }
      return true;
   }

/* Kompatibilitaet zum M80 */

   if (Memo("DEFB")) strmaxcpy(OpPart, "DB", 255);
   if (Memo("DEFW")) strmaxcpy(OpPart, "DW", 255);

   return false;
}

static void DecodeLD(void) {
   Byte AdrByte, HLen;
   Integer z;
   Byte HVals[5];

   if (ArgCnt != 2) WrError(1110);
   else {
      DecodeAdr(ArgStr[1]);
      switch (AdrMode) {
         case ModReg8:
            if (AdrPart == 7) { /* LD A,... */
               OpSize = 0;
               DecodeAdr(ArgStr[2]);
               switch (AdrMode) {
                  case ModReg8: /* LD A,R8/RX8/(HL)/(XY+D) */
                     BAsmCode[PrefixCnt] = 0x78 + AdrPart;
                     memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                     CodeLen = PrefixCnt + 1 + AdrCnt;
                     break;
                  case ModIndReg16: /* LD A,(BC)/(DE) */
                     BAsmCode[0] = 0x0a + (AdrPart << 4);
                     CodeLen = 1;
                     break;
                  case ModImm: /* LD A,imm8 */
                     BAsmCode[0] = 0x3e;
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
                  case ModAbs: /* LD a,(adr) */
                     BAsmCode[PrefixCnt] = 0x3a;
                     memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                     CodeLen = PrefixCnt + 1 + AdrCnt;
                     break;
                  case ModRef: /* LD A,R */
                     BAsmCode[0] = 0xed;
                     BAsmCode[1] = 0x5f;
                     CodeLen = 2;
                     break;
                  case ModInt: /* LD A,I */
                     BAsmCode[0] = 0xed;
                     BAsmCode[1] = 0x57;
                     CodeLen = 2;
                     break;
                  default:
                     if (AdrMode != ModNone) WrError(1350);
               }
            } else if ((AdrPart != 6) && (PrefixCnt == 0)) { /* LD R8,... */
               AdrByte = AdrPart;
               OpSize = 0;
               DecodeAdr(ArgStr[2]);
               switch (AdrMode) {
                  case ModReg8: /* LD R8,R8/RX8/(HL)/(XY+D) */
                     if (((AdrByte == 4) || (AdrByte == 5)) && (PrefixCnt == 1) && (AdrCnt == 0)) WrError(1350);
                     else {
                        BAsmCode[PrefixCnt] = 0x40 + (AdrByte << 3) + AdrPart;
                        memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                        CodeLen = PrefixCnt + 1 + AdrCnt;
                     }
                     break;
                  case ModImm: /* LD R8,imm8 */
                     BAsmCode[0] = 0x06 + (AdrByte << 3);
                     BAsmCode[1] = AdrVals[0];
                     CodeLen = 2;
                     break;
                  default:
                     if (AdrMode != ModNone) WrError(1350);
               }
            } else if ((AdrPart == 4) || (AdrPart == 5)) { /* LD RX8,... */
               AdrByte = AdrPart;
               OpSize = 0;
               DecodeAdr(ArgStr[2]);
               switch (AdrMode) {
                  case ModReg8: /* LD RX8,R8/RX8 */
                     if (AdrPart == 6) WrError(1350);
                     else if ((AdrPart >= 4) && (AdrPart <= 5) && (PrefixCnt != 2)) WrError(1350);
                     else if ((AdrPart >= 4) && (AdrPart <= 5) && (BAsmCode[0] != BAsmCode[1])) WrError(1350);
                     else {
                        if (PrefixCnt == 2) PrefixCnt--;
                        BAsmCode[PrefixCnt] = 0x40 + (AdrByte << 3) + AdrPart;
                        CodeLen = PrefixCnt + 1;
                     }
                     break;
                  case ModImm: /* LD RX8,imm8 */
                     BAsmCode[PrefixCnt] = 0x06 + (AdrByte << 3);
                     BAsmCode[PrefixCnt + 1] = AdrVals[0];
                     CodeLen = PrefixCnt + 2;
                     break;
                  default:
                     if (AdrMode != ModNone) WrError(1350);
               }
            } else { /* LD (HL)/(XY+d),... */
               HLen = AdrCnt;
               memcpy(HVals, AdrVals, AdrCnt);
               z = PrefixCnt;
               if ((z == 0) && (Memo("LDW"))) {
                  OpSize = 1;
                  MayLW = true;
               } else OpSize = 0;
               DecodeAdr(ArgStr[2]);
               switch (AdrMode) {
                  case ModReg8: /* LD (HL)/(XY+D),R8 */
                     if ((PrefixCnt != z) || (AdrPart == 6)) WrError(1350);
                     else {
                        BAsmCode[PrefixCnt] = 0x70 + AdrPart;
                        memcpy(BAsmCode + PrefixCnt + 1, HVals, HLen);
                        CodeLen = PrefixCnt + 1 + HLen;
                     }
                     break;
                  case ModImm: /* LD (HL)/(XY+D),imm8:16:32 */
                     if ((z == 0) && (Memo("LDW")))
                        if (MomCPU < CPUZ380) WrError(1500);
                        else {
                           BAsmCode[PrefixCnt] = 0xed;
                           BAsmCode[PrefixCnt + 1] = 0x36;
                           memcpy(BAsmCode + PrefixCnt + 2, AdrVals, AdrCnt);
                           CodeLen = PrefixCnt + 2 + AdrCnt;
                     } else {
                        BAsmCode[PrefixCnt] = 0x36;
                        memcpy(BAsmCode + 1 + PrefixCnt, HVals, HLen);
                        BAsmCode[PrefixCnt + 1 + HLen] = AdrVals[0];
                        CodeLen = PrefixCnt + 1 + HLen + AdrCnt;
                     }
                     break;
                  case ModReg16: /* LD (HL)/(XY+D),R16/XY */
                     if (MomCPU < CPUZ380) WrError(1500);
                     else if (AdrPart == 3) WrError(1350);
                     else if (HLen == 0)
                        if (PrefixCnt == z) { /* LD (HL),R16 */
                           if (AdrPart == 2) AdrPart = 3;
                           BAsmCode[0] = 0xfd;
                           BAsmCode[1] = 0x0f + (AdrPart << 4);
                           CodeLen = 2;
                        } else { /* LD (HL),XY */
                           CodeLen = PrefixCnt + 1;
                           BAsmCode[PrefixCnt] = 0x31;
                           CodeLen = 1 + PrefixCnt;
                     } else if (PrefixCnt == z) { /* LD (XY+D),R16 */
                        if (AdrPart == 2) AdrPart = 3;
                        BAsmCode[PrefixCnt] = 0xcb;
                        memcpy(BAsmCode + PrefixCnt + 1, HVals, HLen);
                        BAsmCode[PrefixCnt + 1 + HLen] = 0x0b + (AdrPart << 4);
                        CodeLen = PrefixCnt + 1 + HLen + 1;
                     } else if (BAsmCode[0] == BAsmCode[1]) WrError(1350);
                     else {
                        PrefixCnt--;
                        BAsmCode[PrefixCnt] = 0xcb;
                        memcpy(BAsmCode + PrefixCnt + 1, HVals, HLen);
                        BAsmCode[PrefixCnt + 1 + HLen] = 0x2b;
                        CodeLen = PrefixCnt + 1 + HLen + 1;
                     }
                     break;
                  default:
                     if (AdrMode != ModNone) WrError(1350);
               }
            }
            break;
         case ModReg16:
            if (AdrPart == 3) { /* LD SP,... */
               OpSize = 1;
               MayLW = true;
               DecodeAdr(ArgStr[2]);
               switch (AdrMode) {
                  case ModReg16: /* LD SP,HL/XY */
                     if (AdrPart != 2) WrError(1350);
                     else {
                        BAsmCode[PrefixCnt] = 0xf9;
                        CodeLen = PrefixCnt + 1;
                     }
                     break;
                  case ModImm: /* LD SP,imm16:32 */
                     BAsmCode[PrefixCnt] = 0x31;
                     memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                     CodeLen = PrefixCnt + 1 + AdrCnt;
                     break;
                  case ModAbs: /* LD SP,(adr) */
                     BAsmCode[PrefixCnt] = 0xed;
                     BAsmCode[PrefixCnt + 1] = 0x7b;
                     memcpy(BAsmCode + PrefixCnt + 2, AdrVals, AdrCnt);
                     CodeLen = PrefixCnt + 2 + AdrCnt;
                     break;
                  default:
                     if (AdrMode != ModNone) WrError(1350);
               }
            } else if (PrefixCnt == 0) { /* LD R16,... */
               AdrByte = (AdrPart == 2) ? 3 : AdrPart;
               OpSize = 1;
               MayLW = true;
               DecodeAdr(ArgStr[2]);
               switch (AdrMode) {
                  case ModInt: /* LD HL,I */
                     if (MomCPU < CPUZ380) WrError(1500);
                     else if (AdrByte != 3) WrError(1350);
                     else {
                        BAsmCode[0] = 0xdd;
                        BAsmCode[1] = 0x57;
                        CodeLen = 2;
                     }
                     break;
                  case ModReg8:
                     if (AdrPart != 6) WrError(1350);
                     else if (MomCPU < CPUZ380) WrError(1500);
                     else if (PrefixCnt == 0) { /* LD R16,(HL) */
                        BAsmCode[0] = 0xdd;
                        BAsmCode[1] = 0x0f + (AdrByte << 4);
                        CodeLen = 2;
                     } else { /* LD R16,(XY+d) */
                        BAsmCode[PrefixCnt] = 0xcb;
                        memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                        BAsmCode[PrefixCnt + 1 + AdrCnt] = 0x03 + (AdrByte << 4);
                        CodeLen = PrefixCnt + 1 + AdrCnt + 1;
                     }
                     break;
                  case ModReg16:
                     if (AdrPart == 3) WrError(1350);
                     else if (MomCPU < CPUZ380) WrError(1500);
                     else if (PrefixCnt == 0) { /* LD R16,R16 */
                        if (AdrPart == 2) AdrPart = 3;
                        else if (AdrPart == 0) AdrPart = 2;
                        BAsmCode[0] = 0xcd + (AdrPart << 4);
                        BAsmCode[1] = 0x02 + (AdrByte << 4);
                        CodeLen = 2;
                     } else { /* LD R16,XY */
                        BAsmCode[PrefixCnt] = 0x0b + (AdrByte << 4);
                        CodeLen = PrefixCnt + 1;
                     }
                     break;
                  case ModIndReg16: /* LD R16,(R16) */
                     if (MomCPU < CPUZ380) WrError(1500);
                     else {
                        CodeLen = 2;
                        BAsmCode[0] = 0xdd;
                        BAsmCode[1] = 0x0c + (AdrByte << 4) + AdrPart;
                     }
                     break;
                  case ModImm: /* LD R16,imm */
                     if (AdrByte == 3) AdrByte = 2;
                     CodeLen = PrefixCnt + 1 + AdrCnt;
                     BAsmCode[PrefixCnt] = 0x01 + (AdrByte << 4);
                     memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                     break;
                  case ModAbs: /* LD R16,(adr) */
                     if (AdrByte == 3) {
                        BAsmCode[PrefixCnt] = 0x2a;
                        memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                        CodeLen = 1 + PrefixCnt + AdrCnt;
                     } else {
                        BAsmCode[PrefixCnt] = 0xed;
                        BAsmCode[PrefixCnt + 1] = 0x4b + (AdrByte << 4);
                        memcpy(BAsmCode + PrefixCnt + 2, AdrVals, AdrCnt);
                        CodeLen = PrefixCnt + 2 + AdrCnt;
                     }
                     break;
                  case ModSPRel: /* LD R16,(SP+D) */
                     if (MomCPU < CPUZ380) WrError(1500);
                     else {
                        BAsmCode[PrefixCnt] = 0xdd;
                        BAsmCode[PrefixCnt + 1] = 0xcb;
                        memcpy(BAsmCode + PrefixCnt + 2, AdrVals, AdrCnt);
                        BAsmCode[PrefixCnt + 2 + AdrCnt] = 0x01 + (AdrByte << 4);
                        CodeLen = PrefixCnt + 3 + AdrCnt;
                     }
                     break;
                  default:
                     if (AdrMode != ModNone) WrError(1350);
               }
            } else { /* LD XY,... */
               OpSize = 1;
               MayLW = true;
               DecodeAdr(ArgStr[2]);
               switch (AdrMode) {
                  case ModReg8:
                     if (AdrPart != 6) WrError(1350);
                     else if (MomCPU < CPUZ380) WrError(1500);
                     else if (AdrCnt == 0) { /* LD XY,(HL) */
                        BAsmCode[PrefixCnt] = 0x33;
                        CodeLen = PrefixCnt + 1;
                     } else if (BAsmCode[0] == BAsmCode[1]) WrError(1350);
                     else { /* LD XY,(XY+D) */
                        BAsmCode[0] = BAsmCode[1];
                        PrefixCnt--;
                        BAsmCode[PrefixCnt] = 0xcb;
                        memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                        BAsmCode[PrefixCnt + 1 + AdrCnt] = 0x23;
                        CodeLen = PrefixCnt + 1 + AdrCnt + 1;
                     }
                     break;
                  case ModReg16:
                     if (MomCPU < CPUZ380) WrError(1500);
                     else if (AdrPart == 3) WrError(1350);
                     else if (PrefixCnt == 1) { /* LD XY,R16 */
                        if (AdrPart == 2) AdrPart = 3;
                        CodeLen = 1 + PrefixCnt;
                        BAsmCode[PrefixCnt] = 0x07 + (AdrPart << 4);
                     } else if (BAsmCode[0] == BAsmCode[1]) WrError(1350);
                     else { /* LD XY,XY */
                        BAsmCode[--PrefixCnt] = 0x27;
                        CodeLen = 1 + PrefixCnt;
                     }
                     break;
                  case ModIndReg16:
                     if (MomCPU < CPUZ380) WrError(1500);
                     else { /* LD XY,(R16) */
                        BAsmCode[PrefixCnt] = 0x03 + (AdrPart << 4);
                        CodeLen = PrefixCnt + 1;
                     }
                     break;
                  case ModImm: /* LD XY,imm16:32 */
                     BAsmCode[PrefixCnt] = 0x21;
                     memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                     CodeLen = PrefixCnt + 1 + AdrCnt;
                     break;
                  case ModAbs: /* LD XY,(adr) */
                     BAsmCode[PrefixCnt] = 0x2a;
                     memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                     CodeLen = PrefixCnt + 1 + AdrCnt;
                     break;
                  case ModSPRel: /* LD XY,(SP+D) */
                     if (MomCPU < CPUZ380) WrError(1500);
                     else {
                        BAsmCode[PrefixCnt] = 0xcb;
                        memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                        BAsmCode[PrefixCnt + 1 + AdrCnt] = 0x21;
                        CodeLen = PrefixCnt + 1 + AdrCnt + 1;
                     }
                     break;
                  default:
                     if (AdrMode != ModNone) WrError(1350);
               }
            }
            break;
         case ModIndReg16:
            AdrByte = AdrPart;
            if (Memo("LDW")) {
               OpSize = 1;
               MayLW = true;
            } else OpSize = 0;
            DecodeAdr(ArgStr[2]);
            switch (AdrMode) {
               case ModReg8: /* LD (R16),A */
                  if (AdrPart != 7) WrError(1350);
                  else {
                     CodeLen = 1;
                     BAsmCode[0] = 0x02 + (AdrByte << 4);
                  }
                  break;
               case ModReg16:
                  if (AdrPart == 3) WrError(1350);
                  else if (MomCPU < CPUZ380) WrError(1500);
                  else if (PrefixCnt == 0) { /* LD (R16),R16 */
                     if (AdrPart == 2) AdrPart = 3;
                     BAsmCode[0] = 0xfd;
                     BAsmCode[1] = 0x0c + AdrByte + (AdrPart << 4);
                     CodeLen = 2;
                  } else { /* LD (R16),XY */
                     BAsmCode[PrefixCnt] = 0x01 + (AdrByte << 4);
                     CodeLen = PrefixCnt + 1;
                  }
                  break;
               case ModImm:
                  if (!Memo("LDW")) WrError(1350);
                  else if (MomCPU < CPUZ380) WrError(1500);
                  else {
                     BAsmCode[PrefixCnt] = 0xed;
                     BAsmCode[PrefixCnt + 1] = 0x06 + (AdrByte << 4);
                     memcpy(BAsmCode + PrefixCnt + 2, AdrVals, AdrCnt);
                     CodeLen = PrefixCnt + 2 + AdrCnt;
                  }
                  break;
               default:
                  if (AdrMode != ModNone) WrError(1350);
            }
            break;
         case ModAbs:
            HLen = AdrCnt;
            memcpy(HVals, AdrVals, AdrCnt);
            OpSize = 0;
            DecodeAdr(ArgStr[2]);
            switch (AdrMode) {
               case ModReg8: /* LD (adr),A */
                  if (AdrPart != 7) WrError(1350);
                  else {
                     BAsmCode[PrefixCnt] = 0x32;
                     memcpy(BAsmCode + PrefixCnt + 1, HVals, HLen);
                     CodeLen = PrefixCnt + 1 + HLen;
                  }
                  break;
               case ModReg16:
                  if (AdrPart == 2) { /* LD (adr),HL/XY */
                     BAsmCode[PrefixCnt] = 0x22;
                     memcpy(BAsmCode + PrefixCnt + 1, HVals, HLen);
                     CodeLen = PrefixCnt + 1 + HLen;
                  } else { /* LD (adr),R16 */
                     BAsmCode[PrefixCnt] = 0xed;
                     BAsmCode[PrefixCnt + 1] = 0x43 + (AdrPart << 4);
                     memcpy(BAsmCode + PrefixCnt + 2, HVals, HLen);
                     CodeLen = PrefixCnt + 2 + HLen;
                  }
                  break;
               default:
                  if (AdrMode != ModNone) WrError(1350);
            }
            break;
         case ModInt:
            if (strcasecmp(ArgStr[2], "A") == 0) { /* LD I,A */
               CodeLen = 2;
               BAsmCode[0] = 0xed;
               BAsmCode[1] = 0x47;
            } else if (strcasecmp(ArgStr[2], "HL") == 0) /* LD I,HL */
               if (MomCPU < CPUZ380) WrError(1500);
               else {
                  CodeLen = 2;
                  BAsmCode[0] = 0xdd;
                  BAsmCode[1] = 0x47;
            } else WrError(1350);
            break;
         case ModRef:
            if (strcasecmp(ArgStr[2], "A") == 0) { /* LD R,A */
               CodeLen = 2;
               BAsmCode[0] = 0xed;
               BAsmCode[1] = 0x4f;
            } else WrError(1350);
            break;
         case ModSPRel:
            if (MomCPU < CPUZ380) WrError(1500);
            else {
               HLen = AdrCnt;
               memcpy(HVals, AdrVals, AdrCnt);
               OpSize = 0;
               DecodeAdr(ArgStr[2]);
               switch (AdrMode) {
                  case ModReg16:
                     if (AdrPart == 3) WrError(1350);
                     else if (PrefixCnt == 0) { /* LD (SP+D),R16 */
                        if (AdrPart == 2) AdrPart = 3;
                        BAsmCode[PrefixCnt] = 0xdd;
                        BAsmCode[PrefixCnt + 1] = 0xcb;
                        memcpy(BAsmCode + PrefixCnt + 2, HVals, HLen);
                        BAsmCode[PrefixCnt + 2 + HLen] = 0x09 + (AdrPart << 4);
                        CodeLen = PrefixCnt + 2 + HLen + 1;
                     } else { /* LD (SP+D),XY */
                        BAsmCode[PrefixCnt] = 0xcb;
                        memcpy(BAsmCode + PrefixCnt + 1, HVals, HLen);
                        BAsmCode[PrefixCnt + 1 + HLen] = 0x29;
                        CodeLen = PrefixCnt + 1 + HLen + 1;
                     }
                     break;
                  default:
                     if (AdrMode != ModNone) WrError(1350);
               }
            }
            break;
         default:
            if (AdrMode != ModNone) WrError(1350);
      } /* outer switch */
   }
}

static bool ParPair(char *Name1, char *Name2) {
   return (((strcasecmp(ArgStr[1], Name1) == 0) && (strcasecmp(ArgStr[2], Name2) == 0)) || ((strcasecmp(ArgStr[1], Name2) == 0) && (strcasecmp(ArgStr[2], Name1) == 0)));
}

static bool ImmIs8(void) {
   Word tmp = (Word) AdrVals[AdrCnt - 2];

   return ((tmp <= 255) || (tmp >= 0xff80));
}

static bool CodeAri(void) {
   Integer z;
   Byte AdrByte;
   bool OK;

   for (z = 0; z < ALUOrderCnt; z++)
      if (Memo(ALUOrders[z].Name)) {
         if (ArgCnt == 1) {
            strcpy(ArgStr[2], ArgStr[1]);
            strmaxcpy(ArgStr[1], "A", 255);
            ArgCnt = 2;
         }
         if (ArgCnt != 2) WrError(1110);
         else if (strcasecmp(ArgStr[1], "HL") == 0) {
            if (!Memo("SUB")) WrError(1350);
            else {
               OpSize = 1;
               DecodeAdr(ArgStr[2]);
               switch (AdrMode) {
                  case ModAbs:
                     BAsmCode[PrefixCnt] = 0xed;
                     BAsmCode[PrefixCnt + 1] = 0xd6;
                     memcpy(BAsmCode + PrefixCnt + 2, AdrVals, AdrCnt);
                     CodeLen = PrefixCnt + 2 + AdrCnt;
                     break;
                  default:
                     if (AdrMode != ModNone) WrError(1350);
               }
            }
         } else if (strcasecmp(ArgStr[1], "SP") == 0) {
            if (!Memo("SUB")) WrError(1350);
            else {
               OpSize = 1;
               DecodeAdr(ArgStr[2]);
               switch (AdrMode) {
                  case ModImm:
                     BAsmCode[0] = 0xed;
                     BAsmCode[1] = 0x92;
                     memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                     CodeLen = 2 + AdrCnt;
                     break;
                  default:
                     if (AdrMode != ModNone) WrError(1350);
               }
            }
         } else if (strcasecmp(ArgStr[1], "A") != 0) WrError(1350);
         else {
            OpSize = 0;
            DecodeAdr(ArgStr[2]);
            switch (AdrMode) {
               case ModReg8:
                  CodeLen = PrefixCnt + 1 + AdrCnt;
                  BAsmCode[PrefixCnt] = 0x80 + (ALUOrders[z].Code << 3) + AdrPart;
                  memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                  break;
               case ModImm:
                  if (!ImmIs8()) WrError(1320);
                  else {
                     CodeLen = 2;
                     BAsmCode[0] = 0xc6 + (ALUOrders[z].Code << 3);
                     BAsmCode[1] = AdrVals[0];
                  }
                  break;
               default:
                  if (AdrMode != ModNone) WrError(1350);
            }
         }
         return true;
      } else if ((strncmp(ALUOrders[z].Name, OpPart, strlen(ALUOrders[z].Name)) == 0) && (OpPart[strlen(OpPart) - 1] == 'W')) {
         if ((ArgCnt != 2) && (ArgCnt != 1)) WrError(1110);
         else if (MomCPU < CPUZ380) WrError(1500);
         else if ((ArgCnt == 2) && (strcasecmp(ArgStr[1], "HL") != 0)) WrError(1350);
         else {
            OpSize = 1;
            DecodeAdr(ArgStr[ArgCnt]);
            switch (AdrMode) {
               case ModReg16:
                  if (PrefixCnt > 0) { /* wenn Register, dann nie DDIR! */
                     BAsmCode[PrefixCnt] = 0x87 + (ALUOrders[z].Code << 3);
                     CodeLen = 1 + PrefixCnt;
                  } else if (AdrPart == 3) WrError(1350);
                  else {
                     if (AdrPart == 2) AdrPart = 3;
                     BAsmCode[0] = 0xed;
                     BAsmCode[1] = 0x84 + (ALUOrders[z].Code << 3) + AdrPart;
                     CodeLen = 2;
                  }
                  break;
               case ModReg8:
                  if ((AdrPart != 6) || (AdrCnt == 0)) WrError(1350);
                  else {
                     BAsmCode[PrefixCnt] = 0xc6 + (ALUOrders[z].Code << 3);
                     memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                     CodeLen = PrefixCnt + 1 + AdrCnt;
                  }
                  break;
               case ModImm:
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 0x86 + (ALUOrders[z].Code << 3);
                  memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                  CodeLen = 2 + AdrCnt;
                  break;
               default:
                  if (AdrMode != ModNone) WrError(1350);
            }
         }
         return true;
      }

   if (Memo("ADD")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1]);
         switch (AdrMode) {
            case ModReg8:
               if (AdrPart != 7) WrError(1350);
               else {
                  OpSize = 0;
                  DecodeAdr(ArgStr[2]);
                  switch (AdrMode) {
                     case ModReg8:
                        CodeLen = PrefixCnt + 1 + AdrCnt;
                        BAsmCode[PrefixCnt] = 0x80 + AdrPart;
                        memcpy(BAsmCode + 1 + PrefixCnt, AdrVals, AdrCnt);
                        break;
                     case ModImm:
                        CodeLen = PrefixCnt + 1 + AdrCnt;
                        BAsmCode[PrefixCnt] = 0xc6;
                        memcpy(BAsmCode + 1 + PrefixCnt, AdrVals, AdrCnt);
                        break;
                     default:
                        if (AdrMode != ModNone) WrError(1350);
                  }
               }
               break;
            case ModReg16:
               if (AdrPart == 3) { /* SP */
                  OpSize = 1;
                  DecodeAdr(ArgStr[2]);
                  switch (AdrMode) {
                     case ModImm:
                        if (MomCPU < CPUZ380) WrError(1500);
                        else {
                           BAsmCode[0] = 0xed;
                           BAsmCode[1] = 0x82;
                           memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                           CodeLen = 2 + AdrCnt;
                        }
                        break;
                     default:
                        if (AdrMode != ModNone) WrError(1350);
                  }
               } else if (AdrPart != 2) WrError(1350);
               else {
                  z = PrefixCnt; /* merkt, ob Indexregister */
                  OpSize = 1;
                  DecodeAdr(ArgStr[2]);
                  switch (AdrMode) {
                     case ModReg16:
                        if ((AdrPart == 2) && (PrefixCnt != 0) && ((PrefixCnt != 2) || (BAsmCode[0] != BAsmCode[1]))) WrError(1350);
                        else {
                           if (PrefixCnt == 2) PrefixCnt--;
                           CodeLen = 1 + PrefixCnt;
                           BAsmCode[PrefixCnt] = 0x09 + (AdrPart << 4);
                        }
                        break;
                     case ModAbs:
                        if (z != 0) WrError(1350);
                        else if (MomCPU < CPUZ380) WrError(1500);
                        else {
                           BAsmCode[PrefixCnt] = 0xed;
                           BAsmCode[PrefixCnt + 1] = 0xc2;
                           memcpy(BAsmCode + PrefixCnt + 2, AdrVals, AdrCnt);
                           CodeLen = PrefixCnt + 2 + AdrCnt;
                        }
                        break;
                     default:
                        if (AdrMode != ModNone) WrError(1350);
                  }
               }
               break;
            default:
               if (AdrMode != ModNone) WrError(1350);
         }
      }
      return true;
   }

   if (Memo("ADDW")) {
      if ((ArgCnt != 2) && (ArgCnt != 1)) WrError(1110);
      else if (MomCPU < CPUZ380) WrError(1500);
      else if ((ArgCnt == 2) && (strcasecmp(ArgStr[1], "HL") != 0)) WrError(1350);
      else {
         OpSize = 1;
         DecodeAdr(ArgStr[ArgCnt]);
         switch (AdrMode) {
            case ModReg16:
               if (PrefixCnt > 0) { /* wenn Register, dann nie DDIR! */
                  BAsmCode[PrefixCnt] = 0x87;
                  CodeLen = 1 + PrefixCnt;
               } else if (AdrPart == 3) WrError(1350);
               else {
                  if (AdrPart == 2) AdrPart = 3;
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 0x84 + AdrPart;
                  CodeLen = 2;
               }
               break;
            case ModReg8:
               if ((AdrPart != 6) || (AdrCnt == 0)) WrError(1350);
               else {
                  BAsmCode[PrefixCnt] = 0xc6;
                  memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                  CodeLen = PrefixCnt + 1 + AdrCnt;
               }
               break;
            case ModImm:
               BAsmCode[0] = 0xed;
               BAsmCode[1] = 0x86;
               memcpy(BAsmCode + 2, AdrVals, AdrCnt);
               CodeLen = 2 + AdrCnt;
               break;
            default:
               if (AdrMode != ModNone) WrError(1350);
         }
      }
      return true;
   }

   if ((Memo("ADC")) || (Memo("SBC"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1]);
         switch (AdrMode) {
            case ModReg8:
               if (AdrPart != 7) WrError(1350);
               else {
                  OpSize = 0;
                  DecodeAdr(ArgStr[2]);
                  switch (AdrMode) {
                     case ModReg8:
                        CodeLen = PrefixCnt + 1 + AdrCnt;
                        BAsmCode[PrefixCnt] = 0x88 + AdrPart;
                        memcpy(BAsmCode + 1 + PrefixCnt, AdrVals, AdrCnt);
                        break;
                     case ModImm:
                        CodeLen = PrefixCnt + 1 + AdrCnt;
                        BAsmCode[PrefixCnt] = 0xce;
                        memcpy(BAsmCode + 1 + PrefixCnt, AdrVals, AdrCnt);
                        break;
                     default:
                        if (AdrMode != ModNone) WrError(1350);
                  }
                  if ((Memo("SBC")) && (CodeLen != 0)) BAsmCode[PrefixCnt] += 0x10;
               }
               break;
            case ModReg16:
               if ((AdrPart != 2) || (PrefixCnt != 0)) WrError(1350);
               else {
                  OpSize = 1;
                  DecodeAdr(ArgStr[2]);
                  switch (AdrMode) {
                     case ModReg16:
                        if (PrefixCnt != 0) WrError(1350);
                        else {
                           CodeLen = 2;
                           BAsmCode[0] = 0xed;
                           BAsmCode[1] = 0x42 + (AdrPart << 4);
                           if (Memo("ADC")) BAsmCode[1] += 8;
                        }
                        break;
                     default:
                        if (AdrMode != ModNone) WrError(1350);
                  }
               }
               break;
            default:
               if (AdrMode != ModNone) WrError(1350);
         }
      }
      return true;
   }

   if ((Memo("ADCW")) || (Memo("SBCW"))) {
      if ((ArgCnt != 2) && (ArgCnt != 1)) WrError(1110);
      else if (MomCPU < CPUZ380) WrError(1500);
      else if ((ArgCnt == 2) && (strcasecmp(ArgStr[1], "HL") != 0)) WrError(1350);
      else {
         z = Memo("SBCW") << 4;
         OpSize = 1;
         DecodeAdr(ArgStr[ArgCnt]);
         switch (AdrMode) {
            case ModReg16:
               if (PrefixCnt > 0) { /* wenn Register, dann nie DDIR! */
                  BAsmCode[PrefixCnt] = 0x8f + z;
                  CodeLen = 1 + PrefixCnt;
               } else if (AdrPart == 3) WrError(1350);
               else {
                  if (AdrPart == 2) AdrPart = 3;
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 0x8c + z + AdrPart;
                  CodeLen = 2;
               }
               break;
            case ModReg8:
               if ((AdrPart != 6) || (AdrCnt == 0)) WrError(1350);
               else {
                  BAsmCode[PrefixCnt] = 0xce + z; /* ANSI :-0 */
                  memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                  CodeLen = PrefixCnt + 1 + AdrCnt;
               }
               break;
            case ModImm:
               BAsmCode[0] = 0xed;
               BAsmCode[1] = 0x8e + z;
               memcpy(BAsmCode + 2, AdrVals, AdrCnt);
               CodeLen = 2 + AdrCnt;
               break;
            default:
               if (AdrMode != ModNone) WrError(1350);
         }
      }
      return true;
   }

   if ((Memo("INC")) || (Memo("DEC")) || (Memo("INCW")) || (Memo("DECW"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         z = (Memo("DEC")) || (Memo("DECW"));
         DecodeAdr(ArgStr[1]);
         switch (AdrMode) {
            case ModReg8:
               if (OpPart[strlen(OpPart) - 1] == 'W') WrError(1350);
               else {
                  CodeLen = PrefixCnt + 1 + AdrCnt;
                  BAsmCode[PrefixCnt] = 0x04 + (AdrPart << 3) + z;
                  memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
               }
               break;
            case ModReg16:
               CodeLen = 1 + PrefixCnt;
               BAsmCode[PrefixCnt] = 0x03 + (AdrPart << 4) + (z << 3);
               break;
            default:
               if (AdrMode != ModNone) WrError(1350);
         }
      }
      return true;
   }

   for (z = 0; z < ShiftOrderCnt; z++)
      if (Memo(ShiftOrders[z])) {
         if ((ArgCnt == 0) || (ArgCnt > 2)) WrError(1110);
         else if ((z == 6) && (MomCPU != CPUZ80U)) WrError(1500); /* SLIA undok. Z80 */
         else {
            OpSize = 0;
            DecodeAdr(ArgStr[ArgCnt]);
            switch (AdrMode) {
               case ModReg8:
                  if ((PrefixCnt > 0) && (AdrPart != 6)) WrError(1350); /* IXL..IYU verbieten */
                  else {
                     if (ArgCnt == 1) OK = true;
                     else if (MomCPU != CPUZ80U) {
                        WrError(1500);
                        OK = false;
                     } else if ((AdrPart != 6) || (PrefixCnt != 1) || (!DecodeReg8(ArgStr[1], &AdrPart))) {
                        WrError(1350);
                        OK = false;
                     } else OK = true;
                     if (OK) {
                        CodeLen = PrefixCnt + 1 + AdrCnt + 1;
                        BAsmCode[PrefixCnt] = 0xcb;
                        memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                        BAsmCode[PrefixCnt + 1 + AdrCnt] = AdrPart + (z << 3);
                        if ((AdrPart == 7) && (z < 4)) WrError(10);
                     }
                  }
                  break;
               default:
                  if (AdrMode != ModNone) WrError(1350);
            }
         }
         return true;
      } else if ((strncmp(OpPart, ShiftOrders[z], strlen(ShiftOrders[z])) == 0) && (OpPart[strlen(OpPart) - 1] == 'W')) {
         if (ArgCnt != 1) WrError(1110);
         else if ((MomCPU < CPUZ380) || (z == 6)) WrError(1500);
         else {
            OpSize = 1;
            DecodeAdr(ArgStr[1]);
            switch (AdrMode) {
               case ModReg16:
                  if (PrefixCnt > 0) {
                     BAsmCode[2] = 0x04 + (z << 3) + ((BAsmCode[0] >> 5) & 1);
                     BAsmCode[0] = 0xed;
                     BAsmCode[1] = 0xcb;
                     CodeLen = 3;
                  } else if (AdrPart == 3) WrError(1350);
                  else {
                     if (AdrPart == 2) AdrPart = 3;
                     BAsmCode[0] = 0xed;
                     BAsmCode[1] = 0xcb;
                     BAsmCode[2] = (z << 3) + AdrPart;
                     CodeLen = 3;
                  }
                  break;
               case ModReg8:
                  if (AdrPart != 6) WrError(1350);
                  else {
                     if (AdrCnt == 0) {
                        BAsmCode[0] = 0xed;
                        PrefixCnt = 1;
                     }
                     BAsmCode[PrefixCnt] = 0xcb;
                     memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                     BAsmCode[PrefixCnt + 1 + AdrCnt] = 0x02 + (z << 3);
                     CodeLen = PrefixCnt + 1 + AdrCnt + 1;
                  }
                  break;
               default:
                  if (AdrMode != ModNone) WrError(1350);
            }
         }
         return true;
      }

   for (z = 0; z < BitOrderCnt; z++)
      if (Memo(BitOrders[z])) {
         if ((ArgCnt != 2) && (ArgCnt != 3)) WrError(1110);
         else {
            DecodeAdr(ArgStr[ArgCnt]);
            switch (AdrMode) {
               case ModReg8:
                  if ((AdrPart != 6) && (PrefixCnt != 0)) WrError(1350);
                  else {
                     AdrByte = EvalIntExpression(ArgStr[ArgCnt - 1], UInt3, &OK);
                     if (OK) {
                        if (ArgCnt == 2) OK = true;
                        else if (MomCPU != CPUZ80U) {
                           WrError(1500);
                           OK = false;
                        } else if ((AdrPart != 6) || (PrefixCnt != 1) || (z == 0) || (!DecodeReg8(ArgStr[1], &AdrPart))) {
                           WrError(1350);
                           OK = false;
                        } else OK = true;
                        if (OK) {
                           CodeLen = PrefixCnt + 2 + AdrCnt;
                           BAsmCode[PrefixCnt] = 0xcb;
                           memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                           BAsmCode[PrefixCnt + 1 + AdrCnt] = AdrPart + (AdrByte << 3) + ((z + 1) << 6);
                        }
                     }
                  }
                  break;
               default:
                  if (AdrMode != ModNone) WrError(1350);
            }
         }
         return true;
      }

   if (Memo("MLT")) {
      if (ArgCnt != 1) WrError(1110);
      else if (MomCPU < CPUZ180) WrError(1500);
      else {
         DecodeAdr(ArgStr[1]);
         if ((AdrMode != ModReg16) || (PrefixCnt != 0)) WrError(1350);
         else {
            BAsmCode[CodeLen] = 0xed;
            BAsmCode[CodeLen + 1] = 0x4c + (AdrPart << 4);
            CodeLen = 2;
         }
      }
      return true;
   }

   if ((Memo("DIVUW")) || (Memo("MULTW")) || (Memo("MULTUW"))) {
      if (ArgCnt == 1) {
         strcpy(ArgStr[2], ArgStr[1]);
         strmaxcpy(ArgStr[1], "HL", 255);
         ArgCnt = 2;
      }
      if (MomCPU < CPUZ380) WrError(1500);
      else if (ArgCnt != 2) WrError(1110);
      else if (strcasecmp(ArgStr[1], "HL") != 0) WrError(1350);
      else {
         AdrByte = *OpPart == 'D';
         z = OpPart[strlen(OpPart) - 2] == 'U';
         OpSize = 1;
         DecodeAdr(ArgStr[ArgCnt]);
         switch (AdrMode) {
            case ModReg8:
               if ((AdrPart != 6) || (PrefixCnt == 0)) WrError(1350);
               else {
                  BAsmCode[PrefixCnt] = 0xcb;
                  memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                  BAsmCode[PrefixCnt + 1 + AdrCnt] = 0x92 + (z << 3) + (AdrByte << 5);
                  CodeLen = PrefixCnt + 1 + AdrCnt + 1;
               }
               break;
            case ModReg16:
               if (AdrPart == 3) WrError(1350);
               else if (PrefixCnt == 0) {
                  if (AdrPart == 2) AdrPart = 3;
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 0xcb;
                  BAsmCode[2] = 0x90 + AdrPart + (z << 3) + (AdrByte << 5);
                  CodeLen = 3;
               } else {
                  BAsmCode[2] = 0x94 + ((BAsmCode[0] >> 5) & 1) + (z << 3) + (AdrByte << 5);
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 0xcb;
                  CodeLen = 3;
               }
               break;
            case ModImm:
               BAsmCode[0] = 0xed;
               BAsmCode[1] = 0xcb;
               BAsmCode[2] = 0x97 + (z << 3) + (AdrByte << 5);
               memcpy(BAsmCode + 3, AdrVals, AdrCnt);
               CodeLen = 3 + AdrCnt;
               break;
            default:
               if (AdrMode != ModNone) WrError(1350);
         }
      }
      return true;
   }

   if (Memo("TST")) {
      if (ArgCnt != 1) WrError(1110);
      else if (MomCPU < CPUZ180) WrError(1500);
      else {
         OpSize = 0;
         DecodeAdr(ArgStr[1]);
         switch (AdrMode) {
            case ModReg8:
               if (PrefixCnt != 0) WrError(1350);
               else {
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 4 + (AdrPart << 3);
                  CodeLen = 2;
               }
               break;
            case ModImm:
               BAsmCode[0] = 0xed;
               BAsmCode[1] = 0x64;
               BAsmCode[2] = AdrVals[0];
               CodeLen = 3;
               break;
            default:
               if (AdrMode != ModNone) WrError(1350);
         }
      }
      return true;
   }

   if (Memo("SWAP")) {
      if (ArgCnt != 1) WrError(1110);
      else if (MomCPU < CPUZ380) WrError(1500);
      else {
         DecodeAdr(ArgStr[1]);
         switch (AdrMode) {
            case ModReg16:
               if (AdrPart == 3) WrError(1350);
               else if (PrefixCnt == 0) {
                  if (AdrPart == 2) AdrPart = 3;
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 0x0e + (AdrPart << 4); /*? */
                  CodeLen = 2;
               } else {
                  BAsmCode[PrefixCnt] = 0x3e;
                  CodeLen = PrefixCnt + 1;
               }
               break;
            default:
               if (AdrMode != ModNone) WrError(1350);
         }
      }
      return true;
   }

   return false;
}

static void MakeCode_Z80(void) {
   bool OK;
   LongWord AdrLong;
   LongInt AdrLInt;
   Byte AdrByte;
   Integer z;

   CodeLen = 0;
   DontPrint = false;
   PrefixCnt = 0;
   OpSize = 0xff;
   MayLW = false;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

/* letzten Praefix umkopieren */

   LastPrefix = CurrPrefix;
   CurrPrefix = Pref_IN_N;

/* evtl. Datenablage */

   if (DecodeIntelPseudo(false)) return;

/*--------------------------------------------------------------------------*/
/* Instruktionspraefix */

   if (Memo("DDIR")) {
      if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
      else if (MomCPU < CPUZ380) WrError(1500);
      else {
         OK = true;
         for (z = 1; z <= ArgCnt; z++)
            if (OK) {
               NLS_UpString(ArgStr[z]);
               OK = ExtendPrefix(&CurrPrefix, ArgStr[z]);
               if (!OK) WrError(1135);
            }
         if (OK) {
            GetPrefixCode(CurrPrefix, BAsmCode + 0, BAsmCode + 1);
            CodeLen = 2;
         }
      }
      return;
   }

                                  /*--------------------------------------------------------------------------*/
/* mit Sicherheit am haeufigsten... */

   if ((Memo("LD")) || (Memo("LDW"))) {
      DecodeLD();
      return;
   }

                                  /*--------------------------------------------------------------------------*/
/* ohne Operanden */

   for (z = 0; z < FixedOrderCnt; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else if (MomCPU < FixedOrders[z].MinCPU) WrError(1500);
         else {
            if ((CodeLen = FixedOrders[z].Len) == 2) {
               BAsmCode[0] = Hi(FixedOrders[z].Code);
               BAsmCode[1] = Lo(FixedOrders[z].Code);
            } else BAsmCode[0] = Lo(FixedOrders[z].Code);
         };
         return;
      }

/* nur Akku zugelassen */

   for (z = 0; z < AccOrderCnt; z++)
      if (Memo(AccOrders[z].Name)) {
         if (ArgCnt == 0) {
            ArgCnt = 1;
            strmaxcpy(ArgStr[1], "A", 255);
         }
         if (ArgCnt != 1) WrError(1110);
         else if (strcasecmp(ArgStr[1], "A") != 0) WrError(1350);
         else if (MomCPU < AccOrders[z].MinCPU) WrError(1500);
         else {
            if ((CodeLen = AccOrders[z].Len) == 2) {
               BAsmCode[0] = Hi(AccOrders[z].Code);
               BAsmCode[1] = Lo(AccOrders[z].Code);
            } else BAsmCode[0] = Lo(AccOrders[z].Code);
         }
         return;
      }

   for (z = 0; z < HLOrderCnt; z++)
      if (Memo(HLOrders[z].Name)) {
         if (ArgCnt == 0) {
            ArgCnt = 1;
            strmaxcpy(ArgStr[1], "HL", 255);
         };
         if (ArgCnt != 1) WrError(1110);
         else if (strcasecmp(ArgStr[1], "HL") != 0) WrError(1350);
         else if (MomCPU < HLOrders[z].MinCPU) WrError(1500);
         else {
            if ((CodeLen = HLOrders[z].Len) == 2) {
               BAsmCode[0] = Hi(HLOrders[z].Code);
               BAsmCode[1] = Lo(HLOrders[z].Code);
            } else BAsmCode[0] = Lo(HLOrders[z].Code);
         }
         return;
      }

                                  /*-------------------------------------------------------------------------*/
/* Datentransfer */

   if ((Memo("PUSH")) || (Memo("POP"))) {
      z = Memo("PUSH") << 2;
      if (ArgCnt != 1) WrError(1110);
      else if (strcasecmp(ArgStr[1], "SR") == 0)
         if (MomCPU < CPUZ380) WrError(1500);
         else {
            CodeLen = 2;
            BAsmCode[0] = 0xed;
            BAsmCode[1] = 0xc1 + z;
      } else {
         if (strcasecmp(ArgStr[1], "SP") == 0) strmaxcpy(ArgStr[1], "A", 255);
         if (strcasecmp(ArgStr[1], "AF") == 0) strmaxcpy(ArgStr[1], "SP", 255);
         OpSize = 1;
         MayLW = true;
         DecodeAdr(ArgStr[1]);
         switch (AdrMode) {
            case ModReg16:
               CodeLen = 1 + PrefixCnt;
               BAsmCode[PrefixCnt] = 0xc1 + (AdrPart << 4) + z;
               break;
            case ModImm:
               if (MomCPU < CPUZ380) WrError(1500);
               else {
                  BAsmCode[PrefixCnt] = 0xfd;
                  BAsmCode[PrefixCnt + 1] = 0xf5;
                  memcpy(BAsmCode + PrefixCnt + 2, AdrVals, AdrCnt);
                  CodeLen = PrefixCnt + 2 + AdrCnt;
               }
               break;
            default:
               if (AdrMode != ModNone) WrError(1350);
         }
      }
      return;
   }

   if (Memo("EX")) {
      if (ArgCnt != 2) WrError(1110);
      else if (ParPair("DE", "HL")) {
         CodeLen = 1;
         BAsmCode[0] = 0xeb;
      } else if (ParPair("AF", "AF\'")) {
         CodeLen = 1;
         BAsmCode[0] = 0x08;
      } else if (ParPair("AF", "AF`")) {
         CodeLen = 1;
         BAsmCode[0] = 0x08;
      } else if (ParPair("(SP)", "HL")) {
         CodeLen = 1;
         BAsmCode[0] = 0xe3;
      } else if (ParPair("(SP)", "IX")) {
         CodeLen = 2;
         BAsmCode[0] = IXPrefix;
         BAsmCode[1] = 0xe3;
      } else if (ParPair("(SP)", "IY")) {
         CodeLen = 2;
         BAsmCode[0] = IYPrefix;
         BAsmCode[1] = 0xe3;
      } else if (ParPair("(HL)", "A")) {
         if (MomCPU < CPUZ380) WrError(1500);
         else {
            CodeLen = 2;
            BAsmCode[0] = 0xed;
            BAsmCode[1] = 0x37;
         }
      } else {
         if (ArgStr[2][strlen(ArgStr[2]) - 1] == '\'') {
            OK = true;
            ArgStr[2][strlen(ArgStr[2]) - 1] = '\0';
         } else OK = false;
         DecodeAdr(ArgStr[1]);
         switch (AdrMode) {
            case ModReg8:
               if ((AdrPart == 6) || (PrefixCnt != 0)) WrError(1350);
               else {
                  AdrByte = AdrPart;
                  DecodeAdr(ArgStr[2]);
                  switch (AdrMode) {
                     case ModReg8:
                        if ((AdrPart == 6) || (PrefixCnt != 0)) WrError(1350);
                        else if (MomCPU < CPUZ380) WrError(1500);
                        else if ((AdrByte == 7) && (!OK)) {
                           BAsmCode[0] = 0xed;
                           BAsmCode[1] = 0x07 + (AdrPart << 3);
                           CodeLen = 2;
                        } else if ((AdrPart == 7) && (!OK)) {
                           BAsmCode[0] = 0xed;
                           BAsmCode[1] = 0x07 + (AdrByte << 3);
                           CodeLen = 2;
                        } else if ((OK) && (AdrPart == AdrByte)) {
                           BAsmCode[0] = 0xcb;
                           BAsmCode[1] = 0x30 + AdrPart;
                           CodeLen = 2;
                        } else WrError(1350);
                        break;
                     default:
                        if (AdrMode != ModNone) WrError(1350);
                  }
               }
               break;
            case ModReg16:
               if (AdrPart == 3) WrError(1350);
               else if (PrefixCnt == 0) { /* EX R16,... */
                  if (AdrPart == 2) AdrByte = 3;
                  else AdrByte = AdrPart;
                  DecodeAdr(ArgStr[2]);
                  switch (AdrMode) {
                     case ModReg16:
                        if (AdrPart == 3) WrError(1350);
                        else if (MomCPU < CPUZ380) WrError(1500);
                        else if (OK) {
                           if (AdrPart == 2) AdrPart = 3;
                           if ((PrefixCnt != 0) || (AdrPart != AdrByte)) WrError(1350);
                           else {
                              CodeLen = 3;
                              BAsmCode[0] = 0xed;
                              BAsmCode[1] = 0xcb;
                              BAsmCode[2] = 0x30 + AdrByte;
                           }
                        } else if (PrefixCnt == 0) {
                           if (AdrByte == 0) {
                              if (AdrPart == 2) AdrPart = 3;
                              BAsmCode[0] = 0xed;
                              BAsmCode[1] = 0x01 + (AdrPart << 2);
                              CodeLen = 2;
                           } else if (AdrPart == 0) {
                              BAsmCode[0] = 0xed;
                              BAsmCode[1] = 0x01 + (AdrByte << 2);
                              CodeLen = 2;
                           }
                        } else {
                           if (AdrPart == 2) AdrPart = 3;
                           BAsmCode[1] = 0x03 + ((BAsmCode[0] >> 2) & 8) + (AdrByte << 4);
                           BAsmCode[0] = 0xed;
                           CodeLen = 2;
                        }
                        break;
                     default:
                        if (AdrMode != ModNone) WrError(1350);
                  }
               } else { /* EX XY,... */
                  DecodeAdr(ArgStr[2]);
                  switch (AdrMode) {
                     case ModReg16:
                        if (AdrPart == 3) WrError(1350);
                        else if (MomCPU < CPUZ380) WrError(1500);
                        else if (OK)
                           if ((PrefixCnt != 2) || (BAsmCode[0] != BAsmCode[1])) WrError(1350);
                           else {
                              CodeLen = 3;
                              BAsmCode[2] = ((BAsmCode[0] >> 5) & 1) + 0x34;
                              BAsmCode[0] = 0xed;
                              BAsmCode[1] = 0xcb;
                        } else if (PrefixCnt == 1) {
                           if (AdrPart == 2) AdrPart = 3;
                           BAsmCode[1] = ((BAsmCode[0] >> 2) & 8) + 3 + (AdrPart << 4);
                           BAsmCode[0] = 0xed;
                           CodeLen = 2;
                        } else if (BAsmCode[0] == BAsmCode[1]) WrError(1350);
                        else {
                           BAsmCode[0] = 0xed;
                           BAsmCode[1] = 0x2b;
                           CodeLen = 2;
                        }
                        break;
                     default:
                        if (AdrMode != ModNone) WrError(1350);
                  }
               }
               break;
            default:
               if (AdrMode != ModNone) WrError(1350);
         }
      }
      return;
   }

                 /*-------------------------------------------------------------------------*/
/* Arithmetik */

   if (CodeAri()) return;

/*-------------------------------------------------------------------------*/
/* Ein/Ausgabe */

   if (Memo("TSTI")) {
      if (MomCPU != CPUZ80U) WrError(1500);
      else if (ArgCnt != 0) WrError(1110);
      else {
         BAsmCode[0] = 0xed;
         BAsmCode[1] = 0x70;
         CodeLen = 2;
      }
      return;
   }

   if ((Memo("IN")) || (Memo("OUT"))) {
      if ((ArgCnt == 1) && (Memo("IN"))) {
         if (MomCPU != CPUZ80U) WrError(1500);
         else if (strcasecmp(ArgStr[1], "(C)") != 0) WrError(1350);
         else {
            BAsmCode[0] = 0xed;
            BAsmCode[1] = 0x70;
            CodeLen = 2;
         }
      } else if (ArgCnt != 2) WrError(1110);
      else {
         if (Memo("OUT")) {
            strcpy(ArgStr[3], ArgStr[1]);
            strcpy(ArgStr[1], ArgStr[2]);
            strcpy(ArgStr[2], ArgStr[3]);
         }
         if (strcasecmp(ArgStr[2], "(C)") == 0) {
            OpSize = 0;
            DecodeAdr(ArgStr[1]);
            switch (AdrMode) {
               case ModReg8:
                  if ((AdrPart == 6) || (PrefixCnt != 0)) WrError(1350);
                  else {
                     CodeLen = 2;
                     BAsmCode[0] = 0xed;
                     BAsmCode[1] = 0x40 + (AdrPart << 3);
                     if (Memo("OUT")) BAsmCode[1]++;
                  }
                  break;
               case ModImm:
                  if (Memo("IN")) WrError(1350);
                  else if ((MomCPU == CPUZ80U) && (AdrVals[0] == 0)) {
                     BAsmCode[0] = 0xed;
                     BAsmCode[1] = 0x71;
                     CodeLen = 2;
                  } else if (MomCPU < CPUZ380) WrError(1500);
                  else {
                     BAsmCode[0] = 0xed;
                     BAsmCode[1] = 0x71;
                     BAsmCode[2] = AdrVals[0];
                     CodeLen = 3;
                  }
                  break;
               default:
                  if (AdrMode != ModNone) WrError(1350);
            }
         } else if (strcasecmp(ArgStr[1], "A") != 0) WrError(1350);
         else {
            BAsmCode[1] = EvalIntExpression(ArgStr[2], UInt8, &OK);
            if (OK) {
               ChkSpace(SegIO);
               CodeLen = 2;
               BAsmCode[0] = (Memo("OUT")) ? 0xd3 : 0xdb;
            }
         }
      }
      return;
   }

   if ((Memo("INW")) || (Memo("OUTW"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (MomCPU < CPUZ380) WrError(1500);
      else {
         if (Memo("OUTW")) {
            strcpy(ArgStr[3], ArgStr[1]);
            strcpy(ArgStr[1], ArgStr[2]);
            strcpy(ArgStr[2], ArgStr[3]);
         };
         if (strcasecmp(ArgStr[2], "(C)") != 0) WrError(1350);
         else {
            OpSize = 1;
            DecodeAdr(ArgStr[1]);
            switch (AdrMode) {
               case ModReg16:
                  if ((AdrPart == 3) || (PrefixCnt > 0)) WrError(1350);
                  else {
                     switch (AdrPart) {
                        case 1:
                           AdrPart = 2;
                           break;
                        case 2:
                           AdrPart = 7;
                           break;
                     }
                     BAsmCode[0] = 0xdd;
                     BAsmCode[1] = 0x40 + (AdrPart << 3);
                     if (Memo("OUTW")) BAsmCode[1]++;
                     CodeLen = 2;
                  }
                  break;
               case ModImm:
                  if (Memo("INW")) WrError(1350);
                  else {
                     BAsmCode[0] = 0xfd;
                     BAsmCode[1] = 0x79;
                     memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                     CodeLen = 2 + AdrCnt;
                  }
                  break;
               default:
                  if (AdrMode != ModNone) WrError(1350);
            }
         }
      }
      return;
   }

   if ((Memo("IN0")) || (Memo("OUT0"))) {
      if ((ArgCnt != 2) && (ArgCnt != 1)) WrError(1110);
      else if ((ArgCnt == 1) && (Memo("OUT0"))) WrError(1110);
      else if (MomCPU < CPUZ180) WrError(1500);
      else {
         if (Memo("OUT0")) {
            strcpy(ArgStr[3], ArgStr[1]);
            strcpy(ArgStr[1], ArgStr[2]);
            strcpy(ArgStr[2], ArgStr[3]);
         }
         OpSize = 0;
         if (ArgCnt == 1) {
            AdrPart = 6;
            OK = true;
         } else {
            DecodeAdr(ArgStr[1]);
            if ((AdrMode == ModReg8) && (AdrPart != 6) && (PrefixCnt == 0)) OK = true;
            else {
               OK = false;
               if (AdrMode != ModNone) WrError(1350);
            }
         }
         if (OK) {
            BAsmCode[2] = EvalIntExpression(ArgStr[ArgCnt], UInt8, &OK);
            if (OK) {
               BAsmCode[0] = 0xed;
               BAsmCode[1] = AdrPart << 3;
               if (Memo("OUT0")) BAsmCode[1]++;
               CodeLen = 3;
            }
         }
      }
      return;
   }

   if ((Memo("INA")) || (Memo("INAW")) || (Memo("OUTA")) || (Memo("OUTAW"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (MomCPU < CPUZ380) WrError(1500);
      else {
         if (*OpPart == 'O') {
            strcpy(ArgStr[3], ArgStr[1]);
            strcpy(ArgStr[1], ArgStr[2]);
            strcpy(ArgStr[2], ArgStr[3]);
         }
         OpSize = OpPart[strlen(OpPart) - 1] == 'W';
         if (((OpSize == 0) && (strcasecmp(ArgStr[1], "A") != 0))
            || ((OpSize == 1) && (strcasecmp(ArgStr[1], "HL") != 0))) WrError(1350);
         else {
            AdrLong = EvalIntExpression(ArgStr[2], ExtFlag ? Int32 : UInt8, &OK);
            if (OK) {
               ChkSpace(SegIO);
               if (AdrLong > 0xffffff) ChangeDDPrefix("IW");
               else if (AdrLong > 0xffff) ChangeDDPrefix("IB");
               BAsmCode[PrefixCnt] = 0xed + (OpSize << 4);
               BAsmCode[PrefixCnt + 1] = 0xd3 + ((*OpPart == 'I') << 3);
               BAsmCode[PrefixCnt + 2] = AdrLong & 0xff;
               BAsmCode[PrefixCnt + 3] = (AdrLong >> 8) & 0xff;
               CodeLen = PrefixCnt + 4;
               if (AdrLong > 0xffff)
                  BAsmCode[CodeLen++] = (AdrLong >> 16) & 0xff;
               if (AdrLong > 0xffffff)
                  BAsmCode[CodeLen++] = (AdrLong >> 24) & 0xff;
            }
         }
      }
      return;
   }

   if (Memo("TSTIO")) {
      if (ArgCnt != 1) WrError(1110);
      else if (MomCPU < CPUZ180) WrError(1500);
      else {
         BAsmCode[2] = EvalIntExpression(ArgStr[1], Int8, &OK);
         if (OK) {
            BAsmCode[0] = 0xed;
            BAsmCode[1] = 0x74;
            CodeLen = 3;
         }
      }
      return;
   }

                 /*-------------------------------------------------------------------------*/
/* Spruenge */

   if (Memo("RET")) {
      if (ArgCnt == 0) {
         CodeLen = 1;
         BAsmCode[0] = 0xc9;
      } else if (ArgCnt != 1) WrError(1110);
      else if (!DecodeCondition(ArgStr[1], &z)) WrError(1360);
      else {
         CodeLen = 1;
         BAsmCode[0] = 0xc0 + (z << 3);
      }
      return;
   }

   if (Memo("JP")) {
      if (ArgCnt == 1) {
         if (strcasecmp(ArgStr[1], "(HL)") == 0) {
            CodeLen = 1;
            BAsmCode[0] = 0xe9;
            OK = false;
         } else if (strcasecmp(ArgStr[1], "(IX)") == 0) {
            CodeLen = 2;
            BAsmCode[0] = IXPrefix;
            BAsmCode[1] = 0xe9;
            OK = false;
         } else if (strcasecmp(ArgStr[1], "(IY)") == 0) {
            CodeLen = 2;
            BAsmCode[0] = IYPrefix;
            BAsmCode[1] = 0xe9;
            OK = false;
         } else {
            z = 1;
            OK = true;
         }
      } else if (ArgCnt == 2) {
         OK = DecodeCondition(ArgStr[1], &z);
         if (OK) z <<= 3;
         else WrError(1360);
      } else {
         WrError(1110);
         OK = false;
      }
      if (OK) {
         AdrLong = EvalAbsAdrExpression(ArgStr[ArgCnt], &OK);
         if (OK)
#ifdef __STDC__
            if ((AdrLong & 0xffff0000u) == 0)
#else
            if ((AdrLong & 0xffff0000) == 0)
#endif
            {
               CodeLen = 3;
               BAsmCode[0] = 0xc2 + z;
               BAsmCode[1] = Lo(AdrLong);
               BAsmCode[2] = Hi(AdrLong);
            }
#ifdef __STDC__
            else if ((AdrLong & 0xff000000u) == 0)
#else
            else if ((AdrLong & 0xff000000) == 0)
#endif
            {
               ChangeDDPrefix("IB");
               CodeLen = 4 + PrefixCnt;
               BAsmCode[PrefixCnt] = 0xc2 + z;
               BAsmCode[PrefixCnt + 1] = Lo(AdrLong);
               BAsmCode[PrefixCnt + 2] = Hi(AdrLong);
               BAsmCode[PrefixCnt + 3] = Hi(AdrLong >> 8);
            } else {
               ChangeDDPrefix("IW");
               CodeLen = 5 + PrefixCnt;
               BAsmCode[PrefixCnt] = 0xc2 + z;
               BAsmCode[PrefixCnt + 1] = Lo(AdrLong);
               BAsmCode[PrefixCnt + 2] = Hi(AdrLong);
               BAsmCode[PrefixCnt + 3] = Hi(AdrLong >> 8);
               BAsmCode[PrefixCnt + 4] = Hi(AdrLong >> 16);
            }
      }
      return;
   }

   if (Memo("CALL")) {
      if (ArgCnt == 1) {
         z = 9;
         OK = true;
      } else if (ArgCnt == 2) {
         OK = DecodeCondition(ArgStr[1], &z);
         if (OK) z <<= 3;
         else WrError(1360);
      } else {
         WrError(1110);
         OK = false;
      }
      if (OK) {
         AdrLong = EvalAbsAdrExpression(ArgStr[ArgCnt], &OK);
         if (OK)
#ifdef __STDC__
            if ((AdrLong & 0xffff0000u) == 0)
#else
            if ((AdrLong & 0xffff0000) == 0)
#endif
            {
               CodeLen = 3;
               BAsmCode[0] = 0xc4 + z;
               BAsmCode[1] = Lo(AdrLong);
               BAsmCode[2] = Hi(AdrLong);
            }
#ifdef __STDC__
            else if ((AdrLong & 0xff000000u) == 0)
#else
            else if ((AdrLong & 0xff000000) == 0)
#endif
            {
               ChangeDDPrefix("IB");
               CodeLen = 4 + PrefixCnt;
               BAsmCode[PrefixCnt] = 0xc4 + z;
               BAsmCode[PrefixCnt + 1] = Lo(AdrLong);
               BAsmCode[PrefixCnt + 2] = Hi(AdrLong);
               BAsmCode[PrefixCnt + 3] = Hi(AdrLong >> 8);
            } else {
               ChangeDDPrefix("IW");
               CodeLen = 5 + PrefixCnt;
               BAsmCode[PrefixCnt] = 0xc4 + z;
               BAsmCode[PrefixCnt + 1] = Lo(AdrLong);
               BAsmCode[PrefixCnt + 2] = Hi(AdrLong);
               BAsmCode[PrefixCnt + 3] = Hi(AdrLong >> 8);
               BAsmCode[PrefixCnt + 4] = Hi(AdrLong >> 16);
            }
      }
      return;
   }

   if (Memo("JR")) {
      if (ArgCnt == 1) {
         z = 3;
         OK = true;
      } else if (ArgCnt == 2) {
         OK = DecodeCondition(ArgStr[1], &z);
         if ((OK) && (z > 3)) OK = false;
         if (OK) z += 4;
         else WrError(1360);
      } else {
         WrError(1110);
         OK = false;
      }
      if (OK) {
         AdrLInt = EvalAbsAdrExpression(ArgStr[ArgCnt], &OK);
         if (OK) {
            AdrLInt -= EProgCounter() + 2;
            if ((AdrLInt <= 0x7fl) && (AdrLInt >= -0x80l)) {
               CodeLen = 2;
               BAsmCode[0] = z << 3;
               BAsmCode[1] = AdrLInt & 0xff;
            } else if (MomCPU < CPUZ380) WrError(1370);
            else {
               AdrLInt -= 2;
               if ((AdrLInt <= 0x7fffl) && (AdrLInt >= -0x8000l)) {
                  CodeLen = 4;
                  BAsmCode[0] = 0xdd;
                  BAsmCode[1] = z << 3;
                  BAsmCode[2] = AdrLInt & 0xff;
                  BAsmCode[3] = (AdrLInt >> 8) & 0xff;
               } else {
                  AdrLInt--;
                  if ((AdrLInt <= 0x7fffffl) && (AdrLInt >= -0x800000l)) {
                     CodeLen = 5;
                     BAsmCode[0] = 0xfd;
                     BAsmCode[1] = z << 3;
                     BAsmCode[2] = AdrLInt & 0xff;
                     BAsmCode[3] = (AdrLInt >> 8) & 0xff;
                     BAsmCode[4] = (AdrLInt >> 16) & 0xff;
                  } else WrError(1370);
               }
            }
         }
      }
      return;
   }

   if (Memo("CALR")) {
      if (ArgCnt == 1) {
         z = 9;
         OK = true;
      } else if (ArgCnt == 2) {
         OK = DecodeCondition(ArgStr[1], &z);
         if (OK) z <<= 3;
         else WrError(1360);
      } else {
         WrError(1110);
         OK = false;
      }
      if (OK)
         if (MomCPU < CPUZ380) WrError(1500);
         else {
            AdrLInt = EvalAbsAdrExpression(ArgStr[ArgCnt], &OK);
            if (OK) {
               AdrLInt -= EProgCounter() + 3;
               if ((AdrLInt <= 0x7fl) && (AdrLInt >= -0x80l)) {
                  CodeLen = 3;
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 0xc4 + z;
                  BAsmCode[2] = AdrLInt & 0xff;
               } else {
                  AdrLInt--;
                  if ((AdrLInt <= 0x7fffl) && (AdrLInt >= -0x8000l)) {
                     CodeLen = 4;
                     BAsmCode[0] = 0xdd;
                     BAsmCode[1] = 0xc4 + z;
                     BAsmCode[2] = AdrLInt & 0xff;
                     BAsmCode[3] = (AdrLInt >> 8) & 0xff;
                  } else {
                     AdrLInt--;
                     if ((AdrLInt <= 0x7fffffl) && (AdrLInt >= -0x800000l)) {
                        CodeLen = 5;
                        BAsmCode[0] = 0xfd;
                        BAsmCode[1] = 0xc4 + z;
                        BAsmCode[2] = AdrLInt & 0xff;
                        BAsmCode[3] = (AdrLInt >> 8) & 0xff;
                        BAsmCode[4] = (AdrLInt >> 16) & 0xff;
                     } else WrError(1370);
                  }
               }
            }
         }
      return;
   }

   if (Memo("DJNZ")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrLInt = EvalAbsAdrExpression(ArgStr[1], &OK);
         if (OK) {
            AdrLInt -= EProgCounter() + 2;
            if ((AdrLInt <= 0x7fl) && (AdrLInt >= -0x80l)) {
               CodeLen = 2;
               BAsmCode[0] = 0x10;
               BAsmCode[1] = Lo(AdrLInt);
            } else if (MomCPU < CPUZ380) WrError(1370);
            else {
               AdrLInt -= 2;
               if ((AdrLInt <= 0x7fffl) && (AdrLInt >= -0x8000l)) {
                  CodeLen = 4;
                  BAsmCode[0] = 0xdd;
                  BAsmCode[1] = 0x10;
                  BAsmCode[2] = AdrLInt & 0xff;
                  BAsmCode[3] = (AdrLInt >> 8) & 0xff;
               } else {
                  AdrLInt--;
                  if ((AdrLInt <= 0x7fffffl) && (AdrLInt >= -0x800000l)) {
                     CodeLen = 5;
                     BAsmCode[0] = 0xfd;
                     BAsmCode[1] = 0x10;
                     BAsmCode[2] = AdrLInt & 0xff;
                     BAsmCode[3] = (AdrLInt >> 8) & 0xff;
                     BAsmCode[4] = (AdrLInt >> 16) & 0xff;
                  } else WrError(1370);
               }
            }
         }
      }
      return;
   }

   if (Memo("RST")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         FirstPassUnknown = false;
         AdrByte = EvalIntExpression(ArgStr[1], Int8, &OK);
         if (FirstPassUnknown) AdrByte = AdrByte & 0x38;
         if (OK)
            if ((AdrByte > 0x38) || ((AdrByte & 7) != 0)) WrError(1320);
            else {
               CodeLen = 1;
               BAsmCode[0] = 0xc7 + AdrByte;
            }
      }
      return;
   }

                 /*-------------------------------------------------------------------------*/
/* Sonderbefehle */

   if ((Memo("EI")) || (Memo("DI"))) {
      if (ArgCnt == 0) {
         BAsmCode[0] = 0xf3 + (Memo("EI") << 3);
         CodeLen = 1;
      } else if (ArgCnt != 1) WrError(1110);
      else if (MomCPU < CPUZ380) WrError(1500);
      else {
         BAsmCode[2] = EvalIntExpression(ArgStr[1], UInt8, &OK);
         if (OK) {
            BAsmCode[0] = 0xdd;
            BAsmCode[1] = 0xf3 + (Memo("EI") << 3);
            CodeLen = 3;
         }
      }
      return;
   }

   if (Memo("IM")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         AdrByte = EvalIntExpression(ArgStr[1], UInt2, &OK);
         if (OK)
            if (AdrByte > 3) WrError(1320);
            else if ((AdrByte == 3) && (MomCPU < CPUZ380)) WrError(1500);
            else {
               if (AdrByte == 3) AdrByte = 1;
               else if (AdrByte >= 1) AdrByte++;
               CodeLen = 2;
               BAsmCode[0] = 0xed;
               BAsmCode[1] = 0x46 + (AdrByte << 3);
            }
      }
      return;
   }

   if (Memo("LDCTL")) {
      OpSize = 0;
      if (ArgCnt != 2) WrError(1110);
      else if (MomCPU < CPUZ380) WrError(1500);
      else if (DecodeSFR(ArgStr[1], &AdrByte)) {
         DecodeAdr(ArgStr[2]);
         switch (AdrMode) {
            case ModReg8:
               if (AdrPart != 7) WrError(1350);
               else {
                  BAsmCode[0] = 0xcd + ((AdrByte & 3) << 4);
                  BAsmCode[1] = 0xc8 + ((AdrByte & 4) << 2);
                  CodeLen = 2;
               }
               break;
            case ModReg16:
               if ((AdrByte != 1) || (AdrPart != 2) || (PrefixCnt != 0)) WrError(1350);
               else {
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 0xc8;
                  CodeLen = 2;
               }
               break;
            case ModImm:
               BAsmCode[0] = 0xcd + ((AdrByte & 3) << 4);
               BAsmCode[1] = 0xca + ((AdrByte & 4) << 2);
               BAsmCode[2] = AdrVals[0];
               CodeLen = 3;
               break;
            default:
               if (AdrMode != ModNone) WrError(1350);
         }
      } else if (DecodeSFR(ArgStr[2], &AdrByte)) {
         DecodeAdr(ArgStr[1]);
         switch (AdrMode) {
            case ModReg8:
               if ((AdrPart != 7) || (AdrByte == 1)) WrError(1350);
               else {
                  BAsmCode[0] = 0xcd + ((AdrByte & 3) << 4);
                  BAsmCode[1] = 0xd0;
                  CodeLen = 2;
               }
               break;
            case ModReg16:
               if ((AdrByte != 1) || (AdrPart != 2) || (PrefixCnt != 0)) WrError(1350);
               else {
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 0xc0;
                  CodeLen = 2;
               }
               break;
            default:
               if (AdrMode != ModNone) WrError(1350);
         }
      } else WrError(1350);
      return;
   }

   if ((Memo("RESC")) || (Memo("SETC"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (MomCPU < CPUZ380) WrError(1500);
      else {
         AdrByte = 0xff;
         NLS_UpString(ArgStr[1]);
         if (strcmp(ArgStr[1], "LW") == 0) AdrByte = 1;
         else if (strcmp(ArgStr[1], "LCK") == 0) AdrByte = 2;
         else if (strcmp(ArgStr[1], "XM") == 0) AdrByte = 3;
         else WrError(1440);
         if (AdrByte != 0xff) {
            CodeLen = 2;
            BAsmCode[0] = 0xcd + (AdrByte << 4);
            BAsmCode[1] = 0xf7 + (Memo("RESC") << 3);
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static void InitCode_Z80(void) {
   SaveInitProc();
   SetFlag(&ExtFlag, ExtFlagName, false);
   SetFlag(&LWordFlag, LWordFlagName, false);
}

static bool ChkPC_Z80(void) {
   switch (ActPC) {
      case SegCode:
         return (CodeEnd() >= ProgCounter());
      case SegIO:
         return (PortEnd() >= ProgCounter());
      default:
         return false;
   }
}

static bool IsDef_Z80(void) {
   return Memo("PORT");
}

static void SwitchFrom_Z80(void) {
   DeinitFields();
}

static void SwitchTo_Z80(void) {
   TurnWords = false;
   ConstMode = ConstModeIntel;
   SetIsOccupied = true;

   PCSymbol = "$";
   HeaderID = 0x51;
   NOPCode = 0x00;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = (1 << SegCode) + (1 << SegIO);
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;
   Grans[SegIO] = 1;
   ListGrans[SegIO] = 1;
   SegInits[SegIO] = 0;

   MakeCode = MakeCode_Z80;
   ChkPC = ChkPC_Z80;
   IsDef = IsDef_Z80;
   SwitchFrom = SwitchFrom_Z80;
   InitFields();
}

void codez80_init(void) {
   CPUZ80 = AddCPU("Z80", SwitchTo_Z80);
   CPUZ80U = AddCPU("Z80UNDOC", SwitchTo_Z80);
   CPUZ180 = AddCPU("Z180", SwitchTo_Z80);
   CPUZ380 = AddCPU("Z380", SwitchTo_Z80);

   SaveInitProc = InitPassProc;
   InitPassProc = InitCode_Z80;
}
