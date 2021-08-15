/* code9900.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TMS99xx                                                     */
/*                                                                           */
/* Historie:  9. 3.1997 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "stringutil.h"
#include "endian.h"
#include "bpemu.h"
#include "nls.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

#define TwoOrderCount 6
#define OneOrderCount 6
#define SingOrderCount 14
#define SBitOrderCount 3
#define JmpOrderCount 13
#define ShiftOrderCount 4
#define ImmOrderCount 5
#define RegOrderCount 4
#define FixedOrderCount 6

typedef struct {
   char *Name;
   int NameLen;
   Word Code;
} FixedOrder;

static CPUVar CPU9900;

static bool IsWord;
static Word AdrVal, AdrPart;

static FixedOrder *TwoOrders;
static FixedOrder *OneOrders;
static FixedOrder *SingOrders;
static FixedOrder *SBitOrders;
static FixedOrder *JmpOrders;
static FixedOrder *ShiftOrders;
static FixedOrder *ImmOrders;
static FixedOrder *RegOrders;
static FixedOrder *FixedOrders;

/*-------------------------------------------------------------------------*/
/* dynamische Belegung/Freigabe Codetabellen */

static void AddTwo(char *NName, Word NCode) {
   if (InstrZ >= TwoOrderCount) exit(255);
   TwoOrders[InstrZ].Name = NName;
   TwoOrders[InstrZ].Code = NCode;
   TwoOrders[InstrZ++].NameLen = strlen(NName);
}

static void AddOne(char *NName, Word NCode) {
   if (InstrZ >= OneOrderCount) exit(255);
   OneOrders[InstrZ].Name = NName;
   OneOrders[InstrZ++].Code = NCode;
}

static void AddSing(char *NName, Word NCode) {
   if (InstrZ >= SingOrderCount) exit(255);
   SingOrders[InstrZ].Name = NName;
   SingOrders[InstrZ++].Code = NCode;
}

static void AddSBit(char *NName, Word NCode) {
   if (InstrZ >= SBitOrderCount) exit(255);
   SBitOrders[InstrZ].Name = NName;
   SBitOrders[InstrZ++].Code = NCode;
}

static void AddJmp(char *NName, Word NCode) {
   if (InstrZ >= JmpOrderCount) exit(255);
   JmpOrders[InstrZ].Name = NName;
   JmpOrders[InstrZ++].Code = NCode;
}

static void AddShift(char *NName, Word NCode) {
   if (InstrZ >= ShiftOrderCount) exit(255);
   ShiftOrders[InstrZ].Name = NName;
   ShiftOrders[InstrZ++].Code = NCode;
}

static void AddImm(char *NName, Word NCode) {
   if (InstrZ >= ImmOrderCount) exit(255);
   ImmOrders[InstrZ].Name = NName;
   ImmOrders[InstrZ++].Code = NCode;
}

static void AddReg(char *NName, Word NCode) {
   if (InstrZ >= RegOrderCount) exit(255);
   RegOrders[InstrZ].Name = NName;
   RegOrders[InstrZ++].Code = NCode;
}

static void AddFixed(char *NName, Word NCode) {
   if (InstrZ >= FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   TwoOrders = (FixedOrder *) malloc(TwoOrderCount * sizeof(FixedOrder));
   InstrZ = 0;
   AddTwo("A", 5);
   AddTwo("C", 4);
   AddTwo("S", 3);
   AddTwo("SOC", 7);
   AddTwo("SZC", 2);
   AddTwo("MOV", 6);

   OneOrders = (FixedOrder *) malloc(OneOrderCount * sizeof(FixedOrder));
   InstrZ = 0;
   AddOne("COC", 0x08);
   AddOne("CZC", 0x09);
   AddOne("XOR", 0x0a);
   AddOne("MPY", 0x0e);
   AddOne("DIV", 0x0f);
   AddOne("XOP", 0x0b);

   SingOrders = (FixedOrder *) malloc(SingOrderCount * sizeof(FixedOrder));
   InstrZ = 0;
   AddSing("B", 0x0440);
   AddSing("BL", 0x0680);
   AddSing("BLWP", 0x0400);
   AddSing("CLR", 0x04c0);
   AddSing("SETO", 0x0700);
   AddSing("INV", 0x0540);
   AddSing("NEG", 0x0500);
   AddSing("ABS", 0x0740);
   AddSing("SWPB", 0x06c0);
   AddSing("INC", 0x0580);
   AddSing("INCT", 0x05c0);
   AddSing("DEC", 0x0600);
   AddSing("DECT", 0x0640);
   AddSing("X", 0x0480);

   SBitOrders = (FixedOrder *) malloc(SBitOrderCount * sizeof(FixedOrder));
   InstrZ = 0;
   AddSBit("SBO", 0x1d);
   AddSBit("SBZ", 0x1e);
   AddSBit("TB", 0x1f);

   JmpOrders = (FixedOrder *) malloc(JmpOrderCount * sizeof(FixedOrder));
   InstrZ = 0;
   AddJmp("JEQ", 0x13);
   AddJmp("JGT", 0x15);
   AddJmp("JH", 0x1b);
   AddJmp("JHE", 0x14);
   AddJmp("JL", 0x1a);
   AddJmp("JLE", 0x12);
   AddJmp("JLT", 0x11);
   AddJmp("JMP", 0x10);
   AddJmp("JNC", 0x17);
   AddJmp("JNE", 0x16);
   AddJmp("JNO", 0x19);
   AddJmp("JOC", 0x18);
   AddJmp("JOP", 0x1c);

   ShiftOrders = (FixedOrder *) malloc(ShiftOrderCount * sizeof(FixedOrder));
   InstrZ = 0;
   AddShift("SLA", 0x0a);
   AddShift("SRA", 0x08);
   AddShift("SRC", 0x0b);
   AddShift("SRL", 0x09);

   ImmOrders = (FixedOrder *) malloc(ImmOrderCount * sizeof(FixedOrder));
   InstrZ = 0;
   AddImm("AI", 0x011);
   AddImm("ANDI", 0x012);
   AddImm("CI", 0x014);
   AddImm("LI", 0x010);
   AddImm("ORI", 0x013);

   RegOrders = (FixedOrder *) malloc(RegOrderCount * sizeof(FixedOrder));
   InstrZ = 0;
   AddReg("STST", 0x02c);
   AddReg("LST", 0x008);
   AddReg("STWP", 0x02a);
   AddReg("LWP", 0x009);

   FixedOrders = (FixedOrder *) malloc(FixedOrderCount * sizeof(FixedOrder));
   InstrZ = 0;
   AddFixed("RTWP", 0x0380);
   AddFixed("IDLE", 0x0340);
   AddFixed("RSET", 0x0360);
   AddFixed("CKOF", 0x03c0);
   AddFixed("CKON", 0x03a0);
   AddFixed("LREX", 0x03e0);
}

static void DeinitFields(void) {
   free(TwoOrders);
   free(OneOrders);
   free(SingOrders);
   free(SBitOrders);
   free(JmpOrders);
   free(ShiftOrders);
   free(ImmOrders);
   free(RegOrders);
   free(FixedOrders);
}

/*-------------------------------------------------------------------------*/
/* Adressparser */

static bool DecodeReg(char *Asc, Word * Erg) {
   bool Err;
   int l = strlen(Asc);

   if ((l >= 2) && (toupper(*Asc) == 'R')) {
      *Erg = ConstLongInt(Asc + 1, &Err);
      return ((Err) && (*Erg <= 15));
   } else if ((l >= 3) && (toupper(*Asc) == 'W') && (toupper(Asc[1]) == 'R')) {
      *Erg = ConstLongInt(Asc + 2, &Err);
      return ((Err) && (*Erg <= 15));
   } else return false;
}

static char *HasDisp(char *Asc) {
   char *p;
   Integer Lev;

   if (Asc[strlen(Asc) - 1] == ')') {
      p = Asc + strlen(Asc) - 2;
      Lev = 0;
      while ((p >= Asc) && (Lev != -1)) {
         switch (*p) {
            case '(':
               Lev--;
               break;
            case ')':
               Lev++;
               break;
         }
         if (Lev != -1) p--;
      }
      if (Lev != -1) {
         WrError(1300);
         p = NULL;
      }
   } else p = NULL;

   return p;
}

static bool DecodeAdr(char *Asc) {
   bool IncFlag;
   String Reg;
   bool OK;
   char *p;

   AdrCnt = 0;

   if (DecodeReg(Asc, &AdrPart)) return true;

   if (*Asc == '*') {
      Asc++;
      if (Asc[strlen(Asc) - 1] == '+') {
         IncFlag = true;
         Asc[strlen(Asc) - 1] = '\0';
      } else IncFlag = false;
      if (!DecodeReg(Asc, &AdrPart)) WrXError(1445, Asc);
      else {
         AdrPart += 0x10 + (IncFlag << 5);
         return true;
      }
      return false;
   }

   if (*Asc == '@') Asc++;

   p = HasDisp(Asc);
   if (p == NULL) {
      FirstPassUnknown = false;
      AdrVal = EvalIntExpression(Asc, UInt16, &OK);
      if (OK) {
         AdrPart = 0x20;
         AdrCnt = 1;
         if ((!FirstPassUnknown) && (IsWord) && (Odd(AdrVal))) WrError(180);
         return true;
      }
   } else {
      strmaxcpy(Reg, p + 1, 255);
      Reg[strlen(Reg) - 1] = '\0';
      if (!DecodeReg(Reg, &AdrPart)) WrXError(1445, Reg);
      else if (AdrPart == 0) WrXError(1445, Reg);
      else {
         *p = '\0';
         AdrVal = EvalIntExpression(Asc, Int16, &OK);
         if (OK) {
            AdrPart += 0x20;
            AdrCnt = 1;
            return true;
         }
      }
   }

   return false;
}

/*-------------------------------------------------------------------------*/

static void PutByte(Byte Value) {
   if (((CodeLen & 1) == 1) && (!BigEndian)) {
      BAsmCode[CodeLen] = BAsmCode[CodeLen - 1];
      BAsmCode[CodeLen - 1] = Value;
   } else {
      BAsmCode[CodeLen] = Value;
   }
   CodeLen++;
}

static bool DecodePseudo(void) {
#define ONOFF9900Count 1
   static ONOFFRec ONOFF9900s[ONOFF9900Count] = { { "PADDING", &DoPadding, DoPaddingName } };

   Integer z;
   char *p;
   bool OK;
   TempResult t;
   Word HVal16;

   if (CodeONOFF(ONOFF9900s, ONOFF9900Count)) return true;

   if (Memo("BYTE")) {
      if (ArgCnt == 0) WrError(1110);
      else {
         z = 1;
         OK = true;
         do {
            KillBlanks(ArgStr[z]);
            FirstPassUnknown = false;
            EvalExpression(ArgStr[z], &t);
            switch (t.Typ) {
               case TempInt:
                  if (FirstPassUnknown) t.Contents.Int &= 0xff;
                  if (!RangeCheck(t.Contents.Int, Int8)) WrError(1320);
                  else if (CodeLen == MaxCodeLen) {
                     WrError(1920);
                     OK = false;
                  } else PutByte(t.Contents.Int);
                  break;
               case TempFloat:
                  WrError(1135);
                  OK = false;
                  break;
               case TempString:
                  if (strlen(t.Contents.Ascii) + CodeLen >= MaxCodeLen) {
                     WrError(1920);
                     OK = false;
                  } else {
                     TranslateString(t.Contents.Ascii);
                     for (p = t.Contents.Ascii; *p != '\0'; PutByte(*(p++)));
                  }
                  break;
               case TempNone:
                  OK = false;
                  break;
            }
            z++;
         }
         while ((z <= ArgCnt) && (OK));
         if (!OK) CodeLen = 0;
         else if ((Odd(CodeLen)) && (DoPadding)) PutByte(0);
      }
      return true;
   }

   if (Memo("WORD")) {
      if (ArgCnt == 0) WrError(1110);
      else {
         z = 1;
         OK = true;
         do {
            HVal16 = EvalIntExpression(ArgStr[z], Int16, &OK);
            if (OK) {
               WAsmCode[CodeLen >> 1] = HVal16;
               CodeLen += 2;
            }
            z++;
         }
         while ((z <= ArgCnt) && (OK));
         if (!OK) CodeLen = 0;
      }
      return true;
   }

   if (Memo("BSS")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         FirstPassUnknown = false;
         HVal16 = EvalIntExpression(ArgStr[1], Int16, &OK);
         if (FirstPassUnknown) WrError(1820);
         else if (OK) {
            if ((DoPadding) && (Odd(HVal16))) HVal16++;
            DontPrint = true;
            CodeLen = HVal16;
            if (MakeUseList)
               if (AddChunk(SegChunks + ActPC, ProgCounter(), HVal16, ActPC == SegCode)) WrError(90);
         }
      }
      return true;
   }

   return false;
}

static void MakeCode_9900(void) {
   Word HPart;
   Integer AdrInt;
   Integer z;
   bool OK;

   CodeLen = 0;
   DontPrint = false;
   IsWord = false;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

/* zwei Operanden */

   for (z = 0; z < TwoOrderCount; z++)
      if ((strncmp(OpPart, TwoOrders[z].Name, TwoOrders[z].NameLen) == 0) && (((OpPart[TwoOrders[z].NameLen] == 'B') && (OpPart[TwoOrders[z].NameLen + 1] == '\0')) || (OpPart[TwoOrders[z].NameLen] == '\0'))) {
         if (ArgCnt != 2) WrError(1110);
         else if (DecodeAdr(ArgStr[1])) {
            WAsmCode[0] = AdrPart;
            WAsmCode[1] = AdrVal;
            HPart = AdrCnt;
            if (DecodeAdr(ArgStr[2])) {
               WAsmCode[0] += AdrPart << 6;
               WAsmCode[1 + HPart] = AdrVal;
               CodeLen = (1 + HPart + AdrCnt) << 1;
               if (OpPart[strlen(OpPart) - 1] == 'B') WAsmCode[0] += 0x1000;
               WAsmCode[0] += TwoOrders[z].Code << 13;
            }
         }
         return;
      }

   for (z = 0; z < OneOrderCount; z++)
      if (Memo(OneOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (DecodeAdr(ArgStr[1])) {
            WAsmCode[0] = AdrPart;
            WAsmCode[1] = AdrVal;
            if (!DecodeReg(ArgStr[2], &HPart)) WrXError(1445, ArgStr[2]);
            else {
               WAsmCode[0] += (HPart << 6) + (OneOrders[z].Code << 10);
               CodeLen = (1 + AdrCnt) << 1;
            }
         }
         return;
      }

   if ((Memo("LDCR")) || (Memo("STCR"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (DecodeAdr(ArgStr[1])) {
         WAsmCode[0] = 0x3000 + (Memo("STCR") << 10) + AdrPart;
         WAsmCode[1] = AdrVal;
         FirstPassUnknown = false;
         HPart = EvalIntExpression(ArgStr[2], UInt5, &OK);
         if (FirstPassUnknown) HPart = 1;
         if (OK)
            if (ChkRange(HPart, 1, 16)) {
               WAsmCode[0] += (HPart & 15) << 6;
               CodeLen = (1 + AdrCnt) << 1;
            }
      }
      return;
   }

   for (z = 0; z < ShiftOrderCount; z++)
      if (Memo(ShiftOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (!DecodeReg(ArgStr[2], WAsmCode + 0)) WrXError(1445, ArgStr[2]);
         else {
            if (DecodeReg(ArgStr[1], &HPart)) {
               OK = HPart == 0;
               if (!OK) WrXError(1445, ArgStr[1]);
            } else {
               FirstPassUnknown = false;
               HPart = EvalIntExpression(ArgStr[1], UInt4, &OK);
               if ((OK) && (!FirstPassUnknown) && (HPart == 0)) {
                  WrError(1315);
                  OK = false;
               }
            }
            if (OK) {
               WAsmCode[0] += (HPart << 4) + (ShiftOrders[z].Code << 8);
               CodeLen = 2;
            }
         }
         return;
      }

   for (z = 0; z < ImmOrderCount; z++)
      if (Memo(ImmOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (!DecodeReg(ArgStr[2], WAsmCode + 0)) WrXError(1445, ArgStr[2]);
         else {
            WAsmCode[1] = EvalIntExpression(ArgStr[1], Int16, &OK);
            if (OK) {
               WAsmCode[0] += (ImmOrders[z].Code << 5);
               CodeLen = 4;
            }
         }
         return;
      }

   for (z = 0; z < RegOrderCount; z++)
      if (Memo(RegOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (!DecodeReg(ArgStr[1], WAsmCode + 0)) WrXError(1445, ArgStr[1]);
         else {
            WAsmCode[0] += RegOrders[z].Code << 4;
            CodeLen = 2;
         }
         return;
      };

/* ein Operand */

   if ((Memo("MPYS")) || (Memo("DIVS"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (DecodeAdr(ArgStr[1])) {
         WAsmCode[0] = 0x0180 + (Memo("MPYS") << 6) + AdrPart;
         WAsmCode[1] = AdrVal;
         CodeLen = (1 + AdrCnt) << 1;
      }
      return;
   }

   for (z = 0; z < SingOrderCount; z++)
      if (Memo(SingOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (DecodeAdr(ArgStr[1])) {
            WAsmCode[0] = SingOrders[z].Code + AdrPart;
            WAsmCode[1] = AdrVal;
            CodeLen = (1 + AdrCnt) << 1;
         }
         return;
      }

   for (z = 0; z < SBitOrderCount; z++)
      if (Memo(SBitOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            WAsmCode[0] = EvalIntExpression(ArgStr[1], SInt8, &OK);
            if (OK) {
               WAsmCode[0] = (WAsmCode[0] & 0xff) | (SBitOrders[z].Code << 8);
               CodeLen = 2;
            }
         }
         return;
      }

   for (z = 0; z < JmpOrderCount; z++)
      if (Memo(JmpOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            AdrInt = EvalIntExpression(ArgStr[1], UInt16, &OK) - (EProgCounter() + 2);
            if (OK)
               if (Odd(AdrInt)) WrError(1375);
               else if ((!SymbolQuestionable) && ((AdrInt < -256) || (AdrInt > 254))) WrError(1370);
               else {
                  WAsmCode[0] = ((AdrInt >> 1) & 0xff) | (JmpOrders[z].Code << 8);
                  CodeLen = 2;
               }
         };
         return;
      }

   if ((Memo("LWPI")) || (Memo("LIMI"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         WAsmCode[1] = EvalIntExpression(ArgStr[1], UInt16, &OK);
         if (OK) {
            WAsmCode[0] = (0x017 + Memo("LIMI")) << 5;
            CodeLen = 4;
         }
      }
      return;
   }

/* kein Operand */

   for (z = 0; z < FixedOrderCount; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else {
            WAsmCode[0] = FixedOrders[z].Code;
            CodeLen = 2;
         }
         return;
      }

   WrXError(1200, OpPart);
}

static bool ChkPC_9900(void) {
   return ((ActPC == SegCode) && (ProgCounter() < 0x10000));
}

static bool IsDef_9900(void) {
   return false;
}

static void SwitchFrom_9900(void) {
   DeinitFields();
}

static void SwitchTo_9900() {
   TurnWords = true;
   ConstMode = ConstModeIntel;
   SetIsOccupied = false;

   PCSymbol = "$";
   HeaderID = 0x48;
   NOPCode = 0x0000;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 2;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_9900;
   ChkPC = ChkPC_9900;
   IsDef = IsDef_9900;
   SwitchFrom = SwitchFrom_9900;

   InitFields();
}

void code9900_init(void) {
   CPU9900 = AddCPU("TMS9900", SwitchTo_9900);
}
