/* codez8.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Zilog Z8                                                    */
/*                                                                           */
/* Historie: 8.11.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "stringutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct {
   char *Name;
   Byte Code;
} FixedOrder;

typedef struct {
   char *Name;
   Byte Code;
   bool Is16;
} ALU1Order;

typedef struct {
   char *Name;
   Byte Code;
} Condition;

#define WorkOfs 0xe0

#define FixedOrderCnt 12

#define ALU2OrderCnt 10

#define ALU1OrderCnt 14

#define CondCnt 20

#define ModNone  (-1)
#define ModWReg   0
#define MModWReg   (1 << ModWReg)
#define ModReg    1
#define MModReg    (1 << ModReg)
#define ModIWReg  2
#define MModIWReg  (1 << ModIWReg)
#define ModIReg   3
#define MModIReg   (1 << ModIReg)
#define ModImm    4
#define MModImm    (1 << ModImm)
#define ModRReg   5
#define MModRReg   (1 << ModRReg)
#define ModIRReg  6
#define MModIRReg  (1 << ModIRReg)
#define ModInd    7
#define MModInd    (1 << ModInd)

static ShortInt AdrType;
static Byte AdrMode, AdrIndex;
static Word AdrWMode;

static FixedOrder *FixedOrders;
static FixedOrder *ALU2Orders;
static ALU1Order *ALU1Orders;
static Condition *Conditions;
static int TrueCond;

static CPUVar CPUZ8601, CPUZ8604, CPUZ8608, CPUZ8630, CPUZ8631;

/*--------------------------------------------------------------------------*/

static void AddFixed(char *NName, Byte NCode) {
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddALU2(char *NName, Byte NCode) {
   if (InstrZ >= ALU2OrderCnt) exit(255);
   ALU2Orders[InstrZ].Name = NName;
   ALU2Orders[InstrZ++].Code = NCode;
}

static void AddALU1(char *NName, Byte NCode, bool NIs) {
   if (InstrZ >= ALU1OrderCnt) exit(255);
   ALU1Orders[InstrZ].Name = NName;
   ALU1Orders[InstrZ].Is16 = NIs;
   ALU1Orders[InstrZ++].Code = NCode;
}

static void AddCondition(char *NName, Byte NCode) {
   if (InstrZ >= CondCnt) exit(255);
   Conditions[InstrZ].Name = NName;
   Conditions[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("CCF", 0xef);
   AddFixed("DI", 0x8f);
   AddFixed("EI", 0x9f);
   AddFixed("HALT", 0x7f);
   AddFixed("IRET", 0xbf);
   AddFixed("NOP", 0xff);
   AddFixed("RCF", 0xcf);
   AddFixed("RET", 0xaf);
   AddFixed("SCF", 0xdf);
   AddFixed("STOP", 0x6f);
   AddFixed("WDH", 0x4f);
   AddFixed("WDT", 0x5f);

   ALU2Orders = (FixedOrder *) malloc(sizeof(FixedOrder) * ALU2OrderCnt);
   InstrZ = 0;
   AddALU2("ADD", 0x00);
   AddALU2("ADC", 0x10);
   AddALU2("SUB", 0x20);
   AddALU2("SBC", 0x30);
   AddALU2("OR", 0x40);
   AddALU2("AND", 0x50);
   AddALU2("TCM", 0x60);
   AddALU2("TM", 0x70);
   AddALU2("CP", 0xa0);
   AddALU2("XOR", 0xb0);

   ALU1Orders = (ALU1Order *) malloc(sizeof(ALU1Order) * ALU1OrderCnt);
   InstrZ = 0;
   AddALU1("DEC", 0x00, false);
   AddALU1("RLC", 0x10, false);
   AddALU1("DA", 0x40, false);
   AddALU1("POP", 0x50, false);
   AddALU1("COM", 0x60, false);
   AddALU1("PUSH", 0x70, false);
   AddALU1("DECW", 0x80, true);
   AddALU1("RL", 0x90, false);
   AddALU1("INCW", 0xa0, true);
   AddALU1("CLR", 0xb0, false);
   AddALU1("RRC", 0xc0, false);
   AddALU1("SRA", 0xd0, false);
   AddALU1("RR", 0xe0, false);
   AddALU1("SWAP", 0xf0, false);

   Conditions = (Condition *) malloc(sizeof(Condition) * CondCnt);
   InstrZ = 0;
   AddCondition("F", 0);
   TrueCond = InstrZ;
   AddCondition("T", 8);
   AddCondition("C", 7);
   AddCondition("NC", 15);
   AddCondition("Z", 6);
   AddCondition("NZ", 14);
   AddCondition("MI", 5);
   AddCondition("PL", 13);
   AddCondition("OV", 4);
   AddCondition("NOV", 12);
   AddCondition("EQ", 6);
   AddCondition("NE", 14);
   AddCondition("LT", 1);
   AddCondition("GE", 9);
   AddCondition("LE", 2);
   AddCondition("GT", 10);
   AddCondition("ULT", 7);
   AddCondition("UGE", 15);
   AddCondition("ULE", 3);
   AddCondition("UGT", 11);
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(ALU2Orders);
   free(ALU1Orders);
   free(Conditions);
}

/*--------------------------------------------------------------------------*/

static bool IsWReg(char *Asc, Byte * Erg) {
   bool Err;

   if ((strlen(Asc) < 2) || (toupper(*Asc) != 'R')) return false;
   else {
      *Erg = ConstLongInt(Asc + 1, &Err);
      if (!Err) return false;
      else return (*Erg <= 15);
   }
}

static bool IsRReg(char *Asc, Byte * Erg) {
   bool Err;

   if ((strlen(Asc) < 3) || (strncasecmp(Asc, "RR", 2) != 0)) return false;
   else {
      *Erg = ConstLongInt(Asc + 2, &Err);
      if (!Err) return false;
      else return (*Erg <= 15);
   }
}

static void CorrMode(Byte Mask, ShortInt Old, ShortInt New) {
   if ((AdrType == Old) && ((Mask & (1 << Old)) == 0)) {
      AdrType = New;
      AdrMode += WorkOfs;
   }
}

static void ChkAdr(Byte Mask, bool Is16) {
   if (!Is16) {
      CorrMode(Mask, ModWReg, ModReg);
      CorrMode(Mask, ModIWReg, ModIReg);
   }

   if ((AdrType != ModNone) && ((Mask & (1 << AdrType)) == 0)) {
      WrError(1350);
      AdrType = ModNone;
   }
}

static void DecodeAdr(char *Asc, Byte Mask, bool Is16) {
   bool OK;
   char *p;

   AdrType = ModNone;

/* immediate ? */

   if (*Asc == '#') {
      AdrMode = EvalIntExpression(Asc + 1, Int8, &OK);
      if (OK) AdrType = ModImm;
      ChkAdr(Mask, Is16);
      return;
   };

/* Register ? */

   if (IsWReg(Asc, &AdrMode)) {
      AdrType = ModWReg;
      ChkAdr(Mask, Is16);
      return;
   }

   if (IsRReg(Asc, &AdrMode)) {
      if ((AdrMode & 1) == 1) WrError(1351);
      else AdrType = ModRReg;
      ChkAdr(Mask, Is16);
      return;
   }

/* indirekte Konstrukte ? */

   if (*Asc == '@') {
      strmove(Asc, 1);
      if (IsWReg(Asc, &AdrMode)) AdrType = ModIWReg;
      else if (IsRReg(Asc, &AdrMode)) {
         if ((AdrMode & 1) == 1) WrError(1351);
         else AdrType = ModIRReg;
      } else {
         AdrMode = EvalIntExpression(Asc, Int8, &OK);
         if (OK) {
            AdrType = ModIReg;
            ChkSpace(SegData);
         }
      }
      ChkAdr(Mask, Is16);
      return;
   }

/* indiziert ? */

   if ((Asc[strlen(Asc) - 1] == ')') && (strlen(Asc) > 4)) {
      p = Asc + strlen(Asc) - 1;
      *p = '\0';
      while ((p >= Asc) && (*p != '(')) p--;
      if (*p != '(') WrError(1300);
      else if (!IsWReg(p + 1, &AdrMode)) WrXError(1445, p + 1);
      else {
         *p = '\0';
         AdrIndex = EvalIntExpression(Asc, Int8, &OK);
         if (OK) {
            AdrType = ModInd;
            ChkSpace(SegData);
         }
         ChkAdr(Mask, Is16);
         return;
      }
   }

/* einfache direkte Adresse ? */

   if (Is16) AdrWMode = EvalIntExpression(Asc, UInt16, &OK);
   else AdrMode = EvalIntExpression(Asc, UInt8, &OK);
   if (OK) {
      AdrType = ModReg;
      ChkSpace((Is16) ? SegCode : SegData);
      ChkAdr(Mask, Is16);
      return;
   }

   ChkAdr(Mask, Is16);
}

/*---------------------------------------------------------------------*/

static bool DecodePseudo(void) {
   if (Memo("SFR")) {
      CodeEquate(SegData, 0, 0xff);
      return true;
   }

   return false;
}

static void MakeCode_Z8(void) {
   Integer z, AdrInt;
   Byte Save;
   bool OK;

   CodeLen = 0;
   DontPrint = false;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(true)) return;

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
      else {
         DecodeAdr(ArgStr[1], MModReg + MModWReg + MModIReg + MModIWReg + MModInd, false);
         switch (AdrType) {
            case ModReg:
               Save = AdrMode;
               DecodeAdr(ArgStr[2], MModReg + MModWReg + MModIReg + MModImm, false);
               switch (AdrType) {
                  case ModReg:
                     BAsmCode[0] = 0xe4;
                     BAsmCode[1] = AdrMode;
                     BAsmCode[2] = Save;
                     CodeLen = 3;
                     break;
                  case ModWReg:
                     BAsmCode[0] = (AdrMode << 4) + 9;
                     BAsmCode[1] = Save;
                     CodeLen = 2;
                     break;
                  case ModIReg:
                     BAsmCode[0] = 0xe5;
                     BAsmCode[1] = AdrMode;
                     BAsmCode[2] = Save;
                     CodeLen = 3;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0xe6;
                     BAsmCode[1] = Save;
                     BAsmCode[2] = AdrMode;
                     CodeLen = 3;
                     break;
               }
               break;
            case ModWReg:
               Save = AdrMode;
               DecodeAdr(ArgStr[2], MModWReg + MModReg + MModIWReg + MModIReg + MModImm + MModInd, false);
               switch (AdrType) {
                  case ModWReg:
                     BAsmCode[0] = (Save << 4) + 8;
                     BAsmCode[1] = AdrMode + WorkOfs;
                     CodeLen = 2;
                     break;
                  case ModReg:
                     BAsmCode[0] = (Save << 4) + 8;
                     BAsmCode[1] = AdrMode;
                     CodeLen = 2;
                     break;
                  case ModIWReg:
                     BAsmCode[0] = 0xe3;
                     BAsmCode[1] = (Save << 4) + AdrMode;
                     CodeLen = 2;
                     break;
                  case ModIReg:
                     BAsmCode[0] = 0xe5;
                     BAsmCode[1] = AdrMode;
                     BAsmCode[2] = WorkOfs + Save;
                     CodeLen = 3;
                     break;
                  case ModImm:
                     BAsmCode[0] = (Save << 4) + 12;
                     BAsmCode[1] = AdrMode;
                     CodeLen = 2;
                     break;
                  case ModInd:
                     BAsmCode[0] = 0xc7;
                     BAsmCode[1] = (Save << 4) + AdrMode;
                     BAsmCode[2] = AdrIndex;
                     CodeLen = 3;
                     break;
               }
               break;
            case ModIReg:
               Save = AdrMode;
               DecodeAdr(ArgStr[2], MModReg + MModImm, false);
               switch (AdrType) {
                  case ModReg:
                     BAsmCode[0] = 0xf5;
                     BAsmCode[1] = AdrMode;
                     BAsmCode[2] = Save;
                     CodeLen = 3;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0xe7;
                     BAsmCode[1] = Save;
                     BAsmCode[2] = AdrMode;
                     CodeLen = 3;
                     break;
               }
               break;
            case ModIWReg:
               Save = AdrMode;
               DecodeAdr(ArgStr[2], MModWReg + MModReg + MModImm, false);
               switch (AdrType) {
                  case ModWReg:
                     BAsmCode[0] = 0xf3;
                     BAsmCode[1] = (Save << 4) + AdrMode;
                     CodeLen = 2;
                     break;
                  case ModReg:
                     BAsmCode[0] = 0xf5;
                     BAsmCode[1] = AdrMode;
                     BAsmCode[2] = WorkOfs + Save;
                     CodeLen = 3;
                     break;
                  case ModImm:
                     BAsmCode[0] = 0xe7;
                     BAsmCode[1] = WorkOfs + Save;
                     BAsmCode[2] = AdrMode;
                     CodeLen = 3;
                     break;
               }
               break;
            case ModInd:
               Save = AdrMode;
               DecodeAdr(ArgStr[2], MModWReg, false);
               switch (AdrType) {
                  case ModWReg:
                     BAsmCode[0] = 0xd7;
                     BAsmCode[1] = (AdrMode << 4) + Save;
                     BAsmCode[2] = AdrIndex;
                     CodeLen = 3;
                     break;
               }
               break;
         }
      }
      return;
   }

   if ((Memo("LDC")) || (Memo("LDE"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModWReg + MModIRReg, false);
         switch (AdrType) {
            case ModWReg:
               Save = AdrMode;
               DecodeAdr(ArgStr[2], MModIRReg, false);
               if (AdrType != ModNone) {
                  BAsmCode[0] = (Memo("LDC")) ? 0xc2 : 0x82;
                  BAsmCode[1] = (Save << 4) + AdrMode;
                  CodeLen = 2;
               }
               break;
            case ModIRReg:
               Save = AdrMode;
               DecodeAdr(ArgStr[2], MModWReg, false);
               if (AdrType != ModNone) {
                  BAsmCode[0] = (Memo("LDC")) ? 0xd2 : 0x92;
                  BAsmCode[1] = (AdrMode << 4) + Save;
                  CodeLen = 2;
               }
               break;
         }
      }
      return;
   }

   if ((Memo("LDCI")) || (Memo("LDEI"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModIWReg + MModIRReg, false);
         switch (AdrType) {
            case ModIWReg:
               Save = AdrMode;
               DecodeAdr(ArgStr[2], MModIRReg, false);
               if (AdrType != ModNone) {
                  BAsmCode[0] = (Memo("LDCI")) ? 0xc3 : 0x83;
                  BAsmCode[1] = (Save << 4) + AdrMode;
                  CodeLen = 2;
               }
               break;
            case ModIRReg:
               Save = AdrMode;
               DecodeAdr(ArgStr[2], MModIWReg, false);
               if (AdrType != ModNone) {
                  BAsmCode[0] = (Memo("LDCI")) ? 0xd3 : 0x93;
                  BAsmCode[1] = (AdrMode << 4) + Save;
                  CodeLen = 2;
               }
               break;
         }
      }
      return;
   }

/* Arithmetik */

   for (z = 0; z < ALU2OrderCnt; z++)
      if (Memo(ALU2Orders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], MModReg + MModWReg + MModIReg, false);
            switch (AdrType) {
               case ModReg:
                  Save = AdrMode;
                  DecodeAdr(ArgStr[2], MModReg + MModIReg + MModImm, false);
                  switch (AdrType) {
                     case ModReg:
                        BAsmCode[0] = ALU2Orders[z].Code + 4;
                        BAsmCode[1] = AdrMode;
                        BAsmCode[2] = Save;
                        CodeLen = 3;
                        break;
                     case ModIReg:
                        BAsmCode[0] = ALU2Orders[z].Code + 5;
                        BAsmCode[1] = AdrMode;
                        BAsmCode[2] = Save;
                        CodeLen = 3;
                        break;
                     case ModImm:
                        BAsmCode[0] = ALU2Orders[z].Code + 6;
                        BAsmCode[1] = Save;
                        BAsmCode[2] = AdrMode;
                        CodeLen = 3;
                        break;
                  }
                  break;
               case ModWReg:
                  Save = AdrMode;
                  DecodeAdr(ArgStr[2], MModWReg + MModReg + MModIWReg + MModIReg + MModImm, false);
                  switch (AdrType) {
                     case ModWReg:
                        BAsmCode[0] = ALU2Orders[z].Code + 2;
                        BAsmCode[1] = (Save << 4) + AdrMode;
                        CodeLen = 2;
                        break;
                     case ModReg:
                        BAsmCode[0] = ALU2Orders[z].Code + 4;
                        BAsmCode[1] = AdrMode;
                        BAsmCode[2] = WorkOfs + Save;
                        CodeLen = 3;
                        break;
                     case ModIWReg:
                        BAsmCode[0] = ALU2Orders[z].Code + 3;
                        BAsmCode[1] = (Save << 4) + AdrMode;
                        CodeLen = 2;
                        break;
                     case ModIReg:
                        BAsmCode[0] = ALU2Orders[z].Code + 5;
                        BAsmCode[1] = AdrMode;
                        BAsmCode[2] = WorkOfs + Save;
                        CodeLen = 3;
                        break;
                     case ModImm:
                        BAsmCode[0] = ALU2Orders[z].Code + 6;
                        BAsmCode[1] = Save + WorkOfs;
                        BAsmCode[2] = AdrMode;
                        CodeLen = 3;
                        break;
                  }
                  break;
               case ModIReg:
                  Save = AdrMode;
                  DecodeAdr(ArgStr[2], MModImm, false);
                  switch (AdrType) {
                     case ModImm:
                        BAsmCode[0] = ALU2Orders[z].Code + 7;
                        BAsmCode[1] = Save;
                        BAsmCode[2] = AdrMode;
                        CodeLen = 3;
                        break;
                  }
                  break;
            }
         }
         return;
      }

/* INC hat eine Optimierungsmoeglichkeit */

   if (Memo("INC")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModWReg + MModReg + MModIReg, false);
         switch (AdrType) {
            case ModWReg:
               BAsmCode[0] = (AdrMode << 4) + 0x0e;
               CodeLen = 1;
               break;
            case ModReg:
               BAsmCode[0] = 0x20;
               BAsmCode[1] = AdrMode;
               CodeLen = 2;
               break;
            case ModIReg:
               BAsmCode[0] = 0x21;
               BAsmCode[1] = AdrMode;
               CodeLen = 2;
               break;
         }
      }
      return;
   }

/* ...alle anderen nicht */

   for (z = 0; z < ALU1OrderCnt; z++)
      if (Memo(ALU1Orders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], ((ALU1Orders[z].Is16) ? MModRReg : 0) + MModReg + MModIReg, false);
            switch (AdrType) {
               case ModReg:
                  BAsmCode[0] = ALU1Orders[z].Code;
                  BAsmCode[1] = AdrMode;
                  CodeLen = 2;
                  break;
               case ModRReg:
                  BAsmCode[0] = ALU1Orders[z].Code;
                  BAsmCode[1] = WorkOfs + AdrMode;
                  CodeLen = 2;
                  break;
               case ModIReg:
                  BAsmCode[0] = ALU1Orders[z].Code + 1;
                  BAsmCode[1] = AdrMode;
                  CodeLen = 2;
                  break;
            }
         }
         return;
      }

/* Spruenge */

   if (Memo("JR")) {
      if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
      else {
         if (ArgCnt == 1) z = TrueCond;
         else {
            z = 0;
            NLS_UpString(ArgStr[1]);
            while ((z < CondCnt) && (strcmp(Conditions[z].Name, ArgStr[1]) != 0)) z++;
            if (z >= CondCnt) WrError(1360);
         }
         if (z < CondCnt) {
            AdrInt = EvalIntExpression(ArgStr[ArgCnt], Int16, &OK) - (EProgCounter() + 2);
            if (OK)
               if ((!SymbolQuestionable) && ((AdrInt > 127) || (AdrInt < -128))) WrError(1370);
               else {
                  ChkSpace(SegCode);
                  BAsmCode[0] = (Conditions[z].Code << 4) + 0x0b;
                  BAsmCode[1] = Lo(AdrInt);
                  CodeLen = 2;
               }
         }
      }
      return;
   }

   if (Memo("DJNZ")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModWReg, false);
         if (AdrType != ModNone) {
            AdrInt = EvalIntExpression(ArgStr[2], Int16, &OK) - (EProgCounter() + 2);
            if (OK)
               if ((!SymbolQuestionable) && ((AdrInt > 127) || (AdrInt < -128))) WrError(1370);
               else {
                  BAsmCode[0] = (AdrMode << 4) + 0x0a;
                  BAsmCode[1] = Lo(AdrInt);
                  CodeLen = 2;
               }
         }
      }
      return;
   }

   if (Memo("CALL")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModIRReg + MModIReg + MModReg, true);
         switch (AdrType) {
            case ModIRReg:
               BAsmCode[0] = 0xd4;
               BAsmCode[1] = 0xe0 + AdrMode;
               CodeLen = 2;
               break;
            case ModIReg:
               BAsmCode[0] = 0xd4;
               BAsmCode[1] = AdrMode;
               CodeLen = 2;
               break;
            case ModReg:
               BAsmCode[0] = 0xd6;
               BAsmCode[1] = Hi(AdrWMode);
               BAsmCode[2] = Lo(AdrWMode);
               CodeLen = 3;
               break;
         }
      }
      return;
   }

   if (Memo("JP")) {
      if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
      else {
         if (ArgCnt == 1) z = TrueCond;
         else {
            z = 0;
            NLS_UpString(ArgStr[1]);
            while ((z < CondCnt) && (strcmp(Conditions[z].Name, ArgStr[1]) != 0)) z++;
            if (z >= CondCnt) WrError(1360);
         }
         if (z < CondCnt) {
            DecodeAdr(ArgStr[ArgCnt], MModIRReg + MModIReg + MModReg, true);
            switch (AdrType) {
               case ModIRReg:
                  if (z != TrueCond) WrError(1350);
                  else {
                     BAsmCode[0] = 0x30;
                     BAsmCode[1] = 0xe0 + AdrMode;
                     CodeLen = 2;
                  }
                  break;
               case ModIReg:
                  if (z != TrueCond) WrError(1350);
                  else {
                     BAsmCode[0] = 0x30;
                     BAsmCode[1] = AdrMode;
                     CodeLen = 2;
                  }
                  break;
               case ModReg:
                  BAsmCode[0] = (Conditions[z].Code << 4) + 0x0d;
                  BAsmCode[1] = Hi(AdrWMode);
                  BAsmCode[2] = Lo(AdrWMode);
                  CodeLen = 3;
                  break;
            }
         }
      }
      return;
   }

/* Sonderbefehle */

   if (Memo("SRP")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], MModImm, false);
         if (AdrType == ModImm)
            if (((AdrMode & 15) != 0) || ((AdrMode > 0x70) && (AdrMode < 0xf0))) WrError(120);
            else {
               BAsmCode[0] = 0x31;
               BAsmCode[1] = AdrMode;
               CodeLen = 2;
            }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_Z8(void) {
   switch (ActPC) {
      case SegCode:
         return (ProgCounter() < 0x10000);
      case SegData:
         return (ProgCounter() < 0x100);
      default:
         return false;
   }
}

static bool IsDef_Z8(void) {
   return (Memo("SFR"));
}

static void SwitchFrom_Z8(void) {
   DeinitFields();
}

static void SwitchTo_Z8(void) {
   TurnWords = false;
   ConstMode = ConstModeIntel;
   SetIsOccupied = false;

   PCSymbol = "$";
   HeaderID = 0x79;
   NOPCode = 0xff;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = (1 << SegCode) | (1 << SegData);
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 1;
   SegInits[SegCode] = 0;
   Grans[SegData] = 1;
   ListGrans[SegData] = 1;
   SegInits[SegData] = 0;

   MakeCode = MakeCode_Z8;
   ChkPC = ChkPC_Z8;
   IsDef = IsDef_Z8;
   SwitchFrom = SwitchFrom_Z8;
   InitFields();
}

void codez8_init(void) {
   CPUZ8601 = AddCPU("Z8601", SwitchTo_Z8);
   CPUZ8604 = AddCPU("Z8604", SwitchTo_Z8);
   CPUZ8608 = AddCPU("Z8608", SwitchTo_Z8);
   CPUZ8630 = AddCPU("Z8630", SwitchTo_Z8);
   CPUZ8631 = AddCPU("Z8631", SwitchTo_Z8);
}
