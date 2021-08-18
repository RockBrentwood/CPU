// AS-Portierung
// Codegenerator SH7x00
#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

#define FixedOrderCount 10
#define OneRegOrderCount 22
#define TwoRegOrderCount 18
#define MulRegOrderCount 3
#define BWOrderCount 3
#define LogOrderCount 4

#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModIReg 1
#define MModIReg (1 << ModIReg)
#define ModPreDec 2
#define MModPreDec (1 << ModPreDec)
#define ModPostInc 3
#define MModPostInc (1 << ModPostInc)
#define ModIndReg 4
#define MModIndReg (1 << ModIndReg)
#define ModR0Base 5
#define MModR0Base (1 << ModR0Base)
#define ModGBRBase 6
#define MModGBRBase (1 << ModGBRBase)
#define ModGBRR0 7
#define MModGBRR0 (1 << ModGBRR0)
#define ModPCRel 8
#define MModPCRel (1 << ModPCRel)
#define ModImm 9
#define MModImm (1 << ModImm)

#define CompLiteralsName "COMPRESSEDLITERALS"

typedef struct {
   char *Name;
   Word Code;
} FixedOrder;

typedef struct {
   char *Name;
   CPUVar MinCPU;
   Word Code;
} FixedMinOrder;

typedef struct _TLiteral {
   struct _TLiteral *Next;
   LongInt Value, FCount;
   bool Is32, IsForward;
   Integer PassNo;
   LongInt DefSection;
} *PLiteral, TLiteral;

static ShortInt OpSize; /* Groesse=8*(2^OpSize) */
static ShortInt AdrMode; /* Ergebnisadressmodus */
static Word AdrPart; /* Adressierungsmodusbits im Opcode */

static PLiteral FirstLiteral;
static LongInt ForwardCount;
static SimpProc SaveInitProc;

static CPUVar CPU7000, CPU7600;

static FixedOrder *FixedOrders;
static FixedMinOrder *OneRegOrders;
static FixedOrder *TwoRegOrders;
static FixedMinOrder *MulRegOrders;
static FixedOrder *BWOrders;
static char **LogOrders;

static bool CurrDelayed, PrevDelayed, CompLiterals;
static LongInt DelayedAdr;

/*-------------------------------------------------------------------------*/
/* dynamische Belegung/Freigabe Codetabellen */

static void AddFixed(char *NName, Word NCode) {
   if (InstrZ >= FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddOneReg(char *NName, Word NCode, CPUVar NMin) {
   if (InstrZ >= OneRegOrderCount) exit(255);
   OneRegOrders[InstrZ].Name = NName;
   OneRegOrders[InstrZ].Code = NCode;
   OneRegOrders[InstrZ++].MinCPU = NMin;
}

static void AddTwoReg(char *NName, Word NCode) {
   if (InstrZ >= TwoRegOrderCount) exit(255);
   TwoRegOrders[InstrZ].Name = NName;
   TwoRegOrders[InstrZ++].Code = NCode;
}

static void AddMulReg(char *NName, Word NCode, CPUVar NMin) {
   if (InstrZ >= MulRegOrderCount) exit(255);
   MulRegOrders[InstrZ].Name = NName;
   MulRegOrders[InstrZ].Code = NCode;
   MulRegOrders[InstrZ++].MinCPU = NMin;
}

static void AddBW(char *NName, Word NCode) {
   if (InstrZ >= BWOrderCount) exit(255);
   BWOrders[InstrZ].Name = NName;
   BWOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCount);
   InstrZ = 0;
   AddFixed("CLRT", 0x0008);
   AddFixed("CLRMAC", 0x0028);
   AddFixed("NOP", 0x0009);
   AddFixed("RTE", 0x002b);
   AddFixed("SETT", 0x0018);
   AddFixed("SLEEP", 0x001b);
   AddFixed("RTS", 0x000b);
   AddFixed("DIV0U", 0x0019);
   AddFixed("BRK", 0x0000);
   AddFixed("RTB", 0x0001);

   OneRegOrders = (FixedMinOrder *) malloc(sizeof(FixedMinOrder) * OneRegOrderCount);
   InstrZ = 0;
   AddOneReg("MOVT", 0x0029, CPU7000);
   AddOneReg("CMP/PZ", 0x4011, CPU7000);
   AddOneReg("CMP/PL", 0x4015, CPU7000);
   AddOneReg("ROTL", 0x4004, CPU7000);
   AddOneReg("ROTR", 0x4005, CPU7000);
   AddOneReg("ROTCL", 0x4024, CPU7000);
   AddOneReg("ROTCR", 0x4025, CPU7000);
   AddOneReg("SHAL", 0x4020, CPU7000);
   AddOneReg("SHAR", 0x4021, CPU7000);
   AddOneReg("SHLL", 0x4000, CPU7000);
   AddOneReg("SHLR", 0x4001, CPU7000);
   AddOneReg("SHLL2", 0x4008, CPU7000);
   AddOneReg("SHLR2", 0x4009, CPU7000);
   AddOneReg("SHLL8", 0x4018, CPU7000);
   AddOneReg("SHLR8", 0x4019, CPU7000);
   AddOneReg("SHLL16", 0x4028, CPU7000);
   AddOneReg("SHLR16", 0x4029, CPU7000);
   AddOneReg("LDBR", 0x0021, CPU7000);
   AddOneReg("STBR", 0x0020, CPU7000);
   AddOneReg("DT", 0x4010, CPU7600);
   AddOneReg("BRAF", 0x0032, CPU7600);
   AddOneReg("BSRF", 0x0003, CPU7600);

   TwoRegOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * TwoRegOrderCount);
   InstrZ = 0;
   AddTwoReg("XTRCT", 0x200d);
   AddTwoReg("ADDC", 0x300e);
   AddTwoReg("ADDV", 0x300f);
   AddTwoReg("CMP/HS", 0x3002);
   AddTwoReg("CMP/GE", 0x3003);
   AddTwoReg("CMP/HI", 0x3006);
   AddTwoReg("CMP/GT", 0x3007);
   AddTwoReg("CMP/STR", 0x200c);
   AddTwoReg("DIV1", 0x3004);
   AddTwoReg("DIV0S", 0x2007);
   AddTwoReg("MULS", 0x200f);
   AddTwoReg("MULU", 0x200e);
   AddTwoReg("NEG", 0x600b);
   AddTwoReg("NEGC", 0x600a);
   AddTwoReg("SUB", 0x3008);
   AddTwoReg("SUBC", 0x300a);
   AddTwoReg("SUBV", 0x300b);
   AddTwoReg("NOT", 0x6007);

   MulRegOrders = (FixedMinOrder *) malloc(sizeof(FixedMinOrder) * MulRegOrderCount);
   InstrZ = 0;
   AddMulReg("MUL", 0x0007, CPU7600);
   AddMulReg("DMULU", 0x3005, CPU7600);
   AddMulReg("DMULS", 0x300d, CPU7600);

   BWOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * BWOrderCount);
   InstrZ = 0;
   AddBW("SWAP", 0x6008);
   AddBW("EXTS", 0x600e);
   AddBW("EXTU", 0x600c);

   LogOrders = (char **)malloc(sizeof(char *) * LogOrderCount);
   InstrZ = 0;
   LogOrders[InstrZ++] = "TST";
   LogOrders[InstrZ++] = "AND";
   LogOrders[InstrZ++] = "XOR";
   LogOrders[InstrZ++] = "OR";
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(OneRegOrders);
   free(TwoRegOrders);
   free(MulRegOrders);
   free(BWOrders);
   free(LogOrders);
}

/*-------------------------------------------------------------------------*/
/* die PC-relative Adresse: direkt nach verzoegerten Spruengen = Sprungziel+2 */

static LongInt PCRelAdr(void) {
   if (PrevDelayed) return DelayedAdr + 2;
   else return EProgCounter() + 4;
}

static void ChkDelayed(void) {
   if (PrevDelayed) WrError(200);
}

/*-------------------------------------------------------------------------*/
/* Adressparsing */

static char *LiteralName(PLiteral Lit) {
   String Tmp;
   static String Result;

   if (Lit->IsForward) sprintf(Tmp, "F_%s", HexString(Lit->FCount, 8));
   else if (Lit->Is32) sprintf(Tmp, "L_%s", HexString(Lit->Value, 8));
   else sprintf(Tmp, "W_%s", HexString(Lit->Value, 4));
   sprintf(Result, "LITERAL_%s_%s", Tmp, HexString(Lit->PassNo, 0));
   return Result;
}

/*
	static void PrintLiterals(void)
{
   PLiteral Lauf;

   WrLstLine("LiteralList");
   Lauf=FirstLiteral;
   while (Lauf!=NULL)
    {
     WrLstLine(LiteralName(Lauf)); Lauf=Lauf->Next;
    }
}
*/
static void SetOpSize(ShortInt Size) {
   if (OpSize == -1) OpSize = Size;
   else if (Size != OpSize) {
      WrError(1131);
      AdrMode = ModNone;
   }
}

static bool DecodeReg(char *Asc, Byte * Erg) {
   bool Err;

   if (strcasecmp(Asc, "SP") == 0) {
      *Erg = 15;
      return true;
   } else if ((strlen(Asc) < 2) || (strlen(Asc) > 3) || (toupper(*Asc) != 'R')) return false;
   else {
      *Erg = ConstLongInt(Asc + 1, &Err);
      return (Err && (*Erg <= 15));
   }
}

static void ChkAdr(Word Mask) {
   if ((AdrMode != ModNone) && ((Mask & (1 << AdrMode)) == 0)) {
      WrError(1350);
      AdrMode = ModNone;
   }
}

static LongInt ExtOp(LongInt Inp, Byte Src, bool Signed) {
   switch (Src) {
      case 0:
         Inp &= 0xff;
         break;
      case 1:
         Inp &= 0xffff;
         break;
   }
   if (Signed) {
      if (Src < 1)
         if ((Inp & 0x80) == 0x80) Inp += 0xff00;
      if (Src < 2)
         if ((Inp & 0x8000) == 0x8000) Inp += 0xffff0000;
   }
   return Inp;
}

static LongInt OpMask(ShortInt OpSize) {
   switch (OpSize) {
      case 0:
         return 0xff;
      case 1:
         return 0xffff;
      case 2:
         return 0xffffffff;
      default:
         return 0;
   }
}

static void DecodeAdr(char *Asc, Word Mask, bool Signed) {
#define RegNone (-1)
#define RegPC (-2)
#define RegGBR (-3)

   Byte p, HReg;
   char *pos;
   ShortInt BaseReg, IndReg, DOpSize;
   LongInt DispAcc;
   String AdrStr, LStr;
   bool OK, FirstFlag, NIs32, Critical, Found, LDef;
   PLiteral Lauf, Last;

   AdrMode = ModNone;

   if (DecodeReg(Asc, &HReg)) {
      AdrPart = HReg;
      AdrMode = ModReg;
      ChkAdr(Mask);
      return;
   }

   if (*Asc == '@') {
      strmove(Asc, 1);
      if (IsIndirect(Asc)) {
         strmove(Asc, 1);
         Asc[strlen(Asc) - 1] = '\0';
         BaseReg = RegNone;
         IndReg = RegNone;
         DispAcc = 0;
         FirstFlag = false;
         OK = true;
         while ((*Asc != '\0') && (OK)) {
            pos = QuotPos(Asc, ',');
            if (pos == NULL) {
               strmaxcpy(AdrStr, Asc, 255);
               *Asc = '\0';
            } else {
               *pos = '\0';
               strmaxcpy(AdrStr, Asc, 255);
               strcopy(Asc, pos + 1);
            }
            if (strcasecmp(AdrStr, "PC") == 0)
               if (BaseReg == RegNone) BaseReg = RegPC;
               else {
                  WrError(1350);
                  OK = false;
            } else if (strcasecmp(AdrStr, "GBR") == 0)
               if (BaseReg == RegNone) BaseReg = RegGBR;
               else {
                  WrError(1350);
                  OK = false;
            } else if (DecodeReg(AdrStr, &HReg))
               if (IndReg == RegNone) IndReg = HReg;
               else if ((BaseReg == RegNone) && (HReg == 0)) BaseReg = 0;
               else if ((IndReg == 0) && (BaseReg == RegNone)) {
                  BaseReg = 0;
                  IndReg = HReg;
               } else {
                  WrError(1350);
                  OK = false;
            } else {
               FirstPassUnknown = false;
               DispAcc += EvalIntExpression(AdrStr, Int32, &OK);
               if (FirstPassUnknown) FirstFlag = true;
            }
         }
         if (FirstFlag) DispAcc = 0;
         if ((OK) && ((DispAcc & ((1 << OpSize) - 1)) != 0)) {
            WrError(1325);
            OK = false;
         } else if ((OK) && (DispAcc < 0)) {
            WrXError(1315, "Disp<0");
            OK = false;
         } else DispAcc = DispAcc >> OpSize;
         if (OK) {
            switch (BaseReg) {
               case 0:
                  if ((IndReg < 0) || (DispAcc != 0)) WrError(1350);
                  else {
                     AdrMode = ModR0Base;
                     AdrPart = IndReg;
                  }
                  break;
               case RegGBR:
                  if ((IndReg == 0) && (DispAcc == 0)) AdrMode = ModGBRR0;
                  else if (IndReg != RegNone) WrError(1350);
                  else if (DispAcc > 255) WrError(1320);
                  else {
                     AdrMode = ModGBRBase;
                     AdrPart = DispAcc;
                  }
                  break;
               case RegNone:
                  if (IndReg == RegNone) WrError(1350);
                  else if (DispAcc > 15) WrError(1320);
                  else {
                     AdrMode = ModIndReg;
                     AdrPart = (IndReg << 4) + DispAcc;
                  }
                  break;
               case RegPC:
                  if (IndReg != RegNone) WrError(1350);
                  else if (DispAcc > 255) WrError(1320);
                  else {
                     AdrMode = ModPCRel;
                     AdrPart = DispAcc;
                  }
                  break;
            }
         }
         ChkAdr(Mask);
         return;
      } else {
         if (DecodeReg(Asc, &HReg)) {
            AdrPart = HReg;
            AdrMode = ModIReg;
         } else if ((strlen(Asc) > 1) && (*Asc == '-') && (DecodeReg(Asc + 1, &HReg))) {
            AdrPart = HReg;
            AdrMode = ModPreDec;
         } else if ((strlen(Asc) > 1) && (Asc[strlen(Asc) - 1] == '+')) {
            strmaxcpy(AdrStr, Asc, 255);
            AdrStr[strlen(AdrStr) - 1] = '\0';
            if (DecodeReg(AdrStr, &HReg)) {
               AdrPart = HReg;
               AdrMode = ModPostInc;
            } else WrError(1350);
         } else WrError(1350);
         ChkAdr(Mask);
         return;
      }
   }

   if (*Asc == '#') {
      FirstPassUnknown = false;
      switch (OpSize) {
         case 0:
            DispAcc = EvalIntExpression(Asc + 1, Int8, &OK);
            break;
         case 1:
            DispAcc = EvalIntExpression(Asc + 1, Int16, &OK);
            break;
         case 2:
            DispAcc = EvalIntExpression(Asc + 1, Int32, &OK);
            break;
         default:
            DispAcc = 0;
            OK = true;
      }
      Critical = FirstPassUnknown || UsesForwards;
      if (OK) {
      /* minimale Groesse optimieren */
         DOpSize = (OpSize == 0) ? 0 : Critical;
         while (((ExtOp(DispAcc, DOpSize, Signed) ^ DispAcc) & OpMask(OpSize)) != 0) DOpSize++;
         if (DOpSize == 0) {
            AdrPart = DispAcc & 0xff;
            AdrMode = ModImm;
         } else if ((Mask & MModPCRel) != 0) {
         /* Literalgroesse ermitteln */
            NIs32 = (DOpSize == 2);
            if (!NIs32) DispAcc &= 0xffff;
         /* Literale sektionsspezifisch */
            strcpy(AdrStr, "[PARENT0]");
         /* schon vorhanden ? */
            Lauf = FirstLiteral;
            p = 0;
            OK = false;
            Last = NULL;
            Found = false;
            while ((Lauf != NULL) && (!Found)) {
               Last = Lauf;
               if (Critical || Lauf->IsForward || Lauf->DefSection != MomSectionHandle) ;
               else if (((Lauf->Is32 == NIs32) && (DispAcc == Lauf->Value))
                  || ((Lauf->Is32) && (!NIs32) && (DispAcc == (Lauf->Value >> 16)))) Found = true;
               else if ((Lauf->Is32) && (!NIs32) && (DispAcc == (Lauf->Value & 0xffff))) {
                  Found = true;
                  p = 2;
               }
               if (!Found) Lauf = Lauf->Next;
            }
         /* nein - erzeugen */
            if (!Found) {
               Lauf = (PLiteral) malloc(sizeof(TLiteral));
               Lauf->Is32 = NIs32;
               Lauf->Value = DispAcc;
               Lauf->IsForward = Critical;
               if (Critical) Lauf->FCount = ForwardCount++;
               Lauf->Next = NULL;
               Lauf->PassNo = 1;
               Lauf->DefSection = MomSectionHandle;
               do {
                  sprintf(LStr, "%s%s", LiteralName(Lauf), AdrStr);
                  LDef = IsSymbolDefined(LStr);
                  if (LDef) Lauf->PassNo++;
               }
               while (LDef);
               if (Last == NULL) FirstLiteral = Lauf;
               else Last->Next = Lauf;
            }
         /* Distanz abfragen - im naechsten Pass... */
            FirstPassUnknown = false;
            sprintf(LStr, "%s%s", LiteralName(Lauf), AdrStr);
            DispAcc = EvalIntExpression(LStr, Int32, &OK) + p;
            if (OK) {
               if (FirstPassUnknown)
                  DispAcc = 0;
               else if (NIs32)
                  DispAcc = (DispAcc - (PCRelAdr() & 0xfffffffc)) >> 2;
               else
                  DispAcc = (DispAcc - PCRelAdr()) >> 1;
               if (DispAcc < 0) {
                  WrXError(1315, "Disp<0");
                  OK = false;
               } else if ((DispAcc > 255) && (!SymbolQuestionable)) WrError(1330);
               else {
                  AdrMode = ModPCRel;
                  AdrPart = DispAcc;
                  OpSize = NIs32 + 1;
               }
            }
         } else WrError(1350);
      }
      ChkAdr(Mask);
      return;
   }

/* absolut ueber PC-relativ abwickeln */

   if ((OpSize != 1) && (OpSize != 2)) WrError(1130);
   else {
      FirstPassUnknown = false;
      DispAcc = EvalIntExpression(Asc, Int32, &OK);
      if (FirstPassUnknown) DispAcc = 0;
      else if (OpSize == 2) DispAcc -= (PCRelAdr() & 0xfffffffc);
      else DispAcc -= PCRelAdr();
      if (DispAcc < 0) WrXError(1315, "Disp<0");
      else if ((DispAcc & ((1 << OpSize) - 1)) != 0) WrError(1325);
      else {
         DispAcc = DispAcc >> OpSize;
         if (DispAcc > 255) WrError(1320);
         else {
            AdrMode = ModPCRel;
            AdrPart = DispAcc;
         }
      }
   }

   ChkAdr(Mask);
}

/*-------------------------------------------------------------------------*/

static void LTORG_16(void) {
   PLiteral Lauf;

   Lauf = FirstLiteral;
   while (Lauf != NULL) {
      if ((!Lauf->Is32) && (Lauf->DefSection == MomSectionHandle)) {
         WAsmCode[CodeLen >> 1] = Lauf->Value;
         EnterIntSymbol(LiteralName(Lauf), EProgCounter() + CodeLen, SegCode, false);
         Lauf->PassNo = (-1);
         CodeLen += 2;
      }
      Lauf = Lauf->Next;
   }
}

static void LTORG_32(void) {
   PLiteral Lauf, EqLauf;

   Lauf = FirstLiteral;
   while (Lauf != NULL) {
      if ((Lauf->Is32) && (Lauf->DefSection == MomSectionHandle) && (Lauf->PassNo >= 0)) {
         if (((EProgCounter() + CodeLen) & 2) != 0) {
            WAsmCode[CodeLen >> 1] = 0;
            CodeLen += 2;
         }
         WAsmCode[CodeLen >> 1] = (Lauf->Value >> 16);
         WAsmCode[(CodeLen >> 1) + 1] = (Lauf->Value & 0xffff);
         EnterIntSymbol(LiteralName(Lauf), EProgCounter() + CodeLen, SegCode, false);
         Lauf->PassNo = (-1);
         if (CompLiterals) {
            EqLauf = Lauf->Next;
            while (EqLauf != NULL) {
               if ((EqLauf->Is32) && (EqLauf->PassNo >= 0) && (EqLauf->DefSection == MomSectionHandle) && (EqLauf->Value == Lauf->Value)) {
                  EnterIntSymbol(LiteralName(EqLauf), EProgCounter() + CodeLen, SegCode, false);
                  EqLauf->PassNo = (-1);
               }
               EqLauf = EqLauf->Next;
            }
         }
         CodeLen += 4;
      }
      Lauf = Lauf->Next;
   }
}

static bool DecodePseudo(void) {
#define ONOFF7000Count 2
   static ONOFFRec ONOFF7000s[ONOFF7000Count] = {
      { "SUPMODE", &SupAllowed, SupAllowedName },
      { "COMPLITERALS", &CompLiterals, CompLiteralsName }
   };
   PLiteral Lauf, Tmp, Last;
   if (CodeONOFF(ONOFF7000s, ONOFF7000Count)) return true;

/* ab hier (und weiter in der Hauptroutine) stehen die Befehle,
   die Code erzeugen, deshalb wird der Merker fuer verzoegerte
   Spruenge hier weiter geschaltet. */

   PrevDelayed = CurrDelayed;
   CurrDelayed = false;

   if (Memo("LTORG")) {
      if (ArgCnt != 0) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else {
         if ((EProgCounter() & 3) == 0) {
            LTORG_32();
            LTORG_16();
         } else {
            LTORG_16();
            LTORG_32();
         }
         Lauf = FirstLiteral;
         Last = NULL;
         while (Lauf != NULL) {
            if ((Lauf->DefSection == MomSectionHandle) && (Lauf->PassNo < 0)) {
               Tmp = Lauf->Next;
               if (Last == NULL) FirstLiteral = Tmp;
               else Last->Next = Tmp;
               free(Lauf);
               Lauf = Tmp;
            } else {
               Last = Lauf;
               Lauf = Lauf->Next;
            }
         }
      }
      return true;
   }

   return false;
}

static void SetCode(Word Code) {
   CodeLen = 2;
   WAsmCode[0] = Code;
}

static void MakeCode_7000(void) {
   Integer z;
   LongInt AdrLong;
   bool OK;
   Byte HReg;

   CodeLen = 0;
   DontPrint = false;
   OpSize = (-1);

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

/* Attribut verwursten */

   if (*AttrPart != '\0') {
      if (strlen(AttrPart) != 1) {
         WrError(1105);
         return;
      }
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
   }

   if (DecodeMoto16Pseudo(OpSize, true)) return;

/* Anweisungen ohne Argument */

   for (z = 0; z < FixedOrderCount; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else if (*AttrPart != '\0') WrError(1100);
         else SetCode(FixedOrders[z].Code);
         if ((!SupAllowed) && ((Memo("RTB")) || (Memo("BRK")))) WrError(50);
         return;
      }

/* Datentransfer */

   if (Memo("MOV")) {
      if (OpSize == -1) SetOpSize(2);
      if (ArgCnt != 2) WrError(1110);
      else if (OpSize > 2) WrError(1130);
      else if (DecodeReg(ArgStr[1], &HReg)) {
         DecodeAdr(ArgStr[2], MModReg + MModIReg + MModPreDec + MModIndReg + MModR0Base + MModGBRBase, true);
         switch (AdrMode) {
            case ModReg:
               if (OpSize != 2) WrError(1130);
               else SetCode(0x6003 + (HReg << 4) + (AdrPart << 8));
               break;
            case ModIReg:
               SetCode(0x2000 + (HReg << 4) + (AdrPart << 8) + OpSize);
               break;
            case ModPreDec:
               SetCode(0x2004 + (HReg << 4) + (AdrPart << 8) + OpSize);
               break;
            case ModIndReg:
               if (OpSize == 2)
                  SetCode(0x1000 + (HReg << 4) + (AdrPart & 15) + ((AdrPart & 0xf0) << 4));
               else if (HReg != 0) WrError(1350);
               else SetCode(0x8000 + AdrPart + (((Word) OpSize) << 8));
               break;
            case ModR0Base:
               SetCode(0x0004 + (AdrPart << 8) + (HReg << 4) + OpSize);
               break;
            case ModGBRBase:
               if (HReg != 0) WrError(1350);
               else SetCode(0xc000 + AdrPart + (((Word) OpSize) << 8));
               break;
         }
      } else if (DecodeReg(ArgStr[2], &HReg)) {
         DecodeAdr(ArgStr[1], MModImm + MModPCRel + MModIReg + MModPostInc + MModIndReg + MModR0Base + MModGBRBase, true);
         switch (AdrMode) {
            case ModIReg:
               SetCode(0x6000 + (AdrPart << 4) + (((Word) HReg) << 8) + OpSize);
               break;
            case ModPostInc:
               SetCode(0x6004 + (AdrPart << 4) + (((Word) HReg) << 8) + OpSize);
               break;
            case ModIndReg:
               if (OpSize == 2)
                  SetCode(0x5000 + (((Word) HReg) << 8) + AdrPart);
               else if (HReg != 0) WrError(1350);
               else SetCode(0x8400 + AdrPart + (((Word) OpSize) << 8));
               break;
            case ModR0Base:
               SetCode(0x000c + (AdrPart << 4) + (((Word) HReg) << 8) + OpSize);
               break;
            case ModGBRBase:
               if (HReg != 0) WrError(1350);
               else SetCode(0xc400 + AdrPart + (((Word) OpSize) << 8));
               break;
            case ModPCRel:
               if (OpSize == 0) WrError(1350);
               else SetCode(0x9000 + (((Word) OpSize - 1) << 14) + (((Word) HReg) << 8) + AdrPart);
               break;
            case ModImm:
               SetCode(0xe000 + (((Word) HReg) << 8) + AdrPart);
               break;
         }
      } else WrError(1350);
      return;
   }

   if (Memo("MOVA")) {
      if (ArgCnt != 2) WrError(1110);
      else if (!DecodeReg(ArgStr[2], &HReg)) WrError(1350);
      else if (HReg != 0) WrError(1350);
      else {
         SetOpSize(2);
         DecodeAdr(ArgStr[1], MModPCRel, false);
         if (AdrMode != ModNone) {
            CodeLen = 2;
            WAsmCode[0] = 0xc700 + AdrPart;
         }
      }
      return;
   }

   if ((Memo("LDC")) || (Memo("STC"))) {
      if (OpSize == -1) SetOpSize(2);
      if (ArgCnt != 2) WrError(1110);
      else {
         if (Memo("LDC")) {
            strcopy(ArgStr[3], ArgStr[1]);
            strcopy(ArgStr[1], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[3]);
         }
         if (strcasecmp(ArgStr[1], "SR") == 0) HReg = 0;
         else if (strcasecmp(ArgStr[1], "GBR") == 0) HReg = 1;
         else if (strcasecmp(ArgStr[1], "VBR") == 0) HReg = 2;
         else {
            WrError(1440);
            HReg = 0xff;
         }
         if (HReg < 0xff) {
            DecodeAdr(ArgStr[2], MModReg + ((Memo("LDC")) ? MModPostInc : MModPreDec), false);
            switch (AdrMode) {
               case ModReg:
                  if (Memo("LDC")) SetCode(0x400e + (AdrPart << 8) + (HReg << 4)); /* ANSI :-0 */
                  else SetCode(0x0002 + (AdrPart << 8) + (HReg << 4));
                  break;
               case ModPostInc:
                  SetCode(0x4007 + (AdrPart << 8) + (HReg << 4));
                  break;
               case ModPreDec:
                  SetCode(0x4003 + (AdrPart << 8) + (HReg << 4));
                  break;
            }
         }
      }
      return;
   }

   if ((Memo("LDS")) || (Memo("STS"))) {
      if (OpSize == -1) SetOpSize(2);
      if (ArgCnt != 2) WrError(1110);
      else {
         if (Memo("LDS")) {
            strcopy(ArgStr[3], ArgStr[1]);
            strcopy(ArgStr[1], ArgStr[2]);
            strcopy(ArgStr[2], ArgStr[3]);
         }
         if (strcasecmp(ArgStr[1], "MACH") == 0) HReg = 0;
         else if (strcasecmp(ArgStr[1], "MACL") == 0) HReg = 1;
         else if (strcasecmp(ArgStr[1], "PR") == 0) HReg = 2;
         else {
            WrError(1440);
            HReg = 0xff;
         }
         if (HReg < 0xff) {
            DecodeAdr(ArgStr[2], MModReg + ((Memo("LDS")) ? MModPostInc : MModPreDec), false);
            switch (AdrMode) {
               case ModReg:
                  if (Memo("LDS")) SetCode(0x400a + (AdrPart << 8) + (HReg << 4));
                  else SetCode(0x000a + (AdrPart << 8) + (HReg << 4));
                  break;
               case ModPostInc:
                  SetCode(0x4006 + (AdrPart << 8) + (HReg << 4));
                  break;
               case ModPreDec:
                  SetCode(0x4002 + (AdrPart << 8) + (HReg << 4));
                  break;
            }
         }
      }
      return;
   }

/* nur ein Register als Argument */

   for (z = 0; z < OneRegOrderCount; z++)
      if (Memo(OneRegOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (*AttrPart != '\0') WrError(1100);
         else if (MomCPU < OneRegOrders[z].MinCPU) WrError(1500);
         else {
            DecodeAdr(ArgStr[1], MModReg, false);
            if (AdrMode != ModNone)
               SetCode(OneRegOrders[z].Code + (AdrPart << 8));
            if ((!SupAllowed) && ((Memo("STBR")) || (Memo("LDBR")))) WrError(50);
            if (*OpPart == 'B') {
               CurrDelayed = true;
               DelayedAdr = 0x7fffffff;
               ChkDelayed();
            }
         }
         return;
      }

   if (Memo("TAS")) {
      if (OpSize == -1) SetOpSize(0);
      if (ArgCnt != 1) WrError(1110);
      else if (OpSize != 0) WrError(1130);
      else {
         DecodeAdr(ArgStr[1], MModIReg, false);
         if (AdrMode != ModNone) SetCode(0x401b + (AdrPart << 8));
      }
      return;
   }

/* zwei Register */

   for (z = 0; z < TwoRegOrderCount; z++)
      if (Memo(TwoRegOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (*AttrPart != '\0') WrError(1100);
         else {
            DecodeAdr(ArgStr[1], MModReg, false);
            if (AdrMode != ModNone) {
               WAsmCode[0] = TwoRegOrders[z].Code + (AdrPart << 4);
               DecodeAdr(ArgStr[2], MModReg, false);
               if (AdrMode != ModNone) SetCode(WAsmCode[0] + (((Word) AdrPart) << 8));
            }
         }
         return;
      }

   for (z = 0; z < MulRegOrderCount; z++)
      if (Memo(MulRegOrders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (MomCPU < MulRegOrders[z].MinCPU) WrError(1500);
         else {
            if (*AttrPart == '\0') OpSize = 2;
            if (OpSize != 2) WrError(1130);
            else {
               DecodeAdr(ArgStr[1], MModReg, false);
               if (AdrMode != ModNone) {
                  WAsmCode[0] = MulRegOrders[z].Code + (AdrPart << 4);
                  DecodeAdr(ArgStr[2], MModReg, false);
                  if (AdrMode != ModNone) SetCode(WAsmCode[0] + (((Word) AdrPart) << 8));
               }
            }
         }
         return;
      }

   for (z = 0; z < BWOrderCount; z++)
      if (Memo(BWOrders[z].Name)) {
         if (OpSize == -1) SetOpSize(1);
         if (ArgCnt != 2) WrError(1110);
         else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
         else {
            DecodeAdr(ArgStr[1], MModReg, false);
            if (AdrMode != ModNone) {
               WAsmCode[0] = BWOrders[z].Code + OpSize + (AdrPart << 4);
               DecodeAdr(ArgStr[2], MModReg, false);
               if (AdrMode != ModNone) SetCode(WAsmCode[0] + (((Word) AdrPart) << 8));
            }
         }
         return;
      }

   if (Memo("MAC")) {
      if (OpSize == -1) SetOpSize(1);
      if (ArgCnt != 2) WrError(1110);
      else if ((OpSize != 1) && (OpSize != 2)) WrError(1130);
      else if ((OpSize == 2) && (MomCPU < CPU7600)) WrError(1500);
      else {
         DecodeAdr(ArgStr[1], MModPostInc, false);
         if (AdrMode != ModNone) {
            WAsmCode[0] = 0x000f + (AdrPart << 4) + (((Word) 2 - OpSize) << 14);
            DecodeAdr(ArgStr[2], MModPostInc, false);
            if (AdrMode != ModNone) SetCode(WAsmCode[0] + (((Word) AdrPart) << 8));
         }
      }
      return;
   }

   if (Memo("ADD")) {
      if (ArgCnt != 2) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else {
         DecodeAdr(ArgStr[2], MModReg, false);
         if (AdrMode != ModNone) {
            HReg = AdrPart;
            OpSize = 2;
            DecodeAdr(ArgStr[1], MModReg + MModImm, true);
            switch (AdrMode) {
               case ModReg:
                  SetCode(0x300c + (((Word) HReg) << 8) + (AdrPart << 4));
                  break;
               case ModImm:
                  SetCode(0x7000 + AdrPart + (((Word) HReg) << 8));
                  break;
            }
         }
      }
      return;
   }

   if (Memo("CMP/EQ")) {
      if (ArgCnt != 2) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else {
         DecodeAdr(ArgStr[2], MModReg, false);
         if (AdrMode != ModNone) {
            HReg = AdrPart;
            OpSize = 2;
            DecodeAdr(ArgStr[1], MModReg + MModImm, true);
            switch (AdrMode) {
               case ModReg:
                  SetCode(0x3000 + (((Word) HReg) << 8) + (AdrPart << 4));
                  break;
               case ModImm:
                  if (HReg != 0) WrError(1350);
                  else SetCode(0x8800 + AdrPart);
                  break;
            }
         }
      }
      return;
   }

   for (z = 0; z < LogOrderCount; z++)
      if (Memo(LogOrders[z])) {
         if (ArgCnt != 2) WrError(1110);
         else {
            DecodeAdr(ArgStr[2], MModReg + MModGBRR0, false);
            switch (AdrMode) {
               case ModReg:
                  if ((*AttrPart != '\0') && (OpSize != 2)) WrError(1130);
                  else {
                     OpSize = 2;
                     HReg = AdrPart;
                     DecodeAdr(ArgStr[1], MModReg + MModImm, false);
                     switch (AdrMode) {
                        case ModReg:
                           SetCode(0x2008 + z + (((Word) HReg) << 8) + (AdrPart << 4));
                           break;
                        case ModImm:
                           if (HReg != 0) WrError(1350);
                           else SetCode(0xc800 + (z << 8) + AdrPart);
                           break;
                     }
                  }
                  break;
               case ModGBRR0:
                  DecodeAdr(ArgStr[1], MModImm, false);
                  if (AdrMode != ModNone)
                     SetCode(0xcc00 + (z << 8) + AdrPart);
                  break;
            }
         }
         return;
      }

/* Miszellaneen.. */

   if (Memo("TRAPA")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else {
         OpSize = 0;
         DecodeAdr(ArgStr[1], MModImm, false);
         if (AdrMode == ModImm) SetCode(0xc300 + AdrPart);
         ChkDelayed();
      }
      return;
   }

/* Spruenge */

   if ((Memo("BF")) || (Memo("BT"))
      || (Memo("BF/S")) || (Memo("BT/S"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1110);
      else if ((strlen(OpPart) == 4) && (MomCPU < CPU7600)) WrError(1500);
      else {
         DelayedAdr = EvalIntExpression(ArgStr[1], Int32, &OK);
         AdrLong = DelayedAdr - (EProgCounter() + 4);
         if (!OK) ;
         else if (Odd(AdrLong)) WrError(1375);
         else if (((AdrLong < -256) || (AdrLong > 254)) && (!SymbolQuestionable)) WrError(1370);
         else {
            WAsmCode[0] = 0x8900 + ((AdrLong >> 1) & 0xff);
            if (OpPart[1] == 'F') WAsmCode[0] += 0x200;
            if (strlen(OpPart) == 4) {
               WAsmCode[0] += 0x400;
               CurrDelayed = true;
            }
            CodeLen = 2;
            ChkDelayed();
         }
      }
      return;
   }

   if ((Memo("BRA")) || (Memo("BSR"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1110);
      else {
         DelayedAdr = EvalIntExpression(ArgStr[1], Int32, &OK);
         AdrLong = DelayedAdr - (EProgCounter() + 4);
         if (!OK) ;
         else if (Odd(AdrLong)) WrError(1375);
         else if (((AdrLong < -4096) || (AdrLong > 4094)) && (!SymbolQuestionable)) WrError(1370);
         else {
            WAsmCode[0] = 0xa000 + ((AdrLong >> 1) & 0xfff);
            if (Memo("BSR")) WAsmCode[0] += 0x1000;
            CodeLen = 2;
            CurrDelayed = true;
            ChkDelayed();
         }
      }
      return;
   }

   if ((Memo("JSR")) || (Memo("JMP"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1130);
      else {
         DecodeAdr(ArgStr[1], MModIReg, false);
         if (AdrMode != ModNone) {
            SetCode(0x400b + (AdrPart << 8) + (Memo("JMP") << 5));
            CurrDelayed = true;
            DelayedAdr = 0x7fffffff;
            ChkDelayed();
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static void InitCode_7000(void) {
   SaveInitProc();
   FirstLiteral = NULL;
   ForwardCount = 0;
}

static bool ChkPC_7000(void) {
#ifdef HAS64
   return ((ActPC == SegCode) && (ProgCounter() <= 0xffffffffll));
#else
   return (ActPC == SegCode);
#endif
}

static bool IsDef_7000(void) {
   return false;
}

static void SwitchFrom_7000(void) {
   DeinitFields();
   if (FirstLiteral != NULL) WrError(1495);
}

static void SwitchTo_7000(void) {
   TurnWords = true;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "*";
   HeaderID = 0x6c;
   NOPCode = 0x0009;
   DivideChars = ",";
   HasAttrs = true;
   AttrChars = ".";

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 2;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_7000;
   ChkPC = ChkPC_7000;
   IsDef = IsDef_7000;
   SwitchFrom = SwitchFrom_7000;

   CurrDelayed = false;
   PrevDelayed = false;

   InitFields();
}

void code7000_init(void) {
   CPU7000 = AddCPU("SH7000", SwitchTo_7000);
   CPU7600 = AddCPU("SH7600", SwitchTo_7000);

   SaveInitProc = InitPassProc;
   InitPassProc = InitCode_7000;
   FirstLiteral = NULL;
}
