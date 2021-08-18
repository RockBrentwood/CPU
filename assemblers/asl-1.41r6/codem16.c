// AS-Portierung
// Codegenerator Mitsubishi M16
#include "stdinc.h"

#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "nls.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

#define ModNone      (-1)
#define ModReg       0
#define MModReg      (1 << ModReg)
#define ModIReg      1
#define MModIReg     (1 << ModIReg)
#define ModDisp16    2
#define MModDisp16   (1 << ModDisp16)
#define ModDisp32    3
#define MModDisp32   (1 << ModDisp32)
#define ModImm       4
#define MModImm      (1 << ModImm)
#define ModAbs16     5
#define MModAbs16    (1 << ModAbs16)
#define ModAbs32     6
#define MModAbs32    (1 << ModAbs32)
#define ModPCRel16   7
#define MModPCRel16  (1 << ModPCRel16)
#define ModPCRel32   8
#define MModPCRel32  (1 << ModPCRel32)
#define ModPop       9
#define MModPop      (1 << ModPop)
#define ModPush      10
#define MModPush     (1 << ModPush)
#define ModRegChain  11
#define MModRegChain (1 << ModRegChain)
#define ModPCChain   12
#define MModPCChain  (1 << ModPCChain)
#define ModAbsChain  13
#define MModAbsChain (1 << ModAbsChain)

#define Mask_RegOnly    (MModReg)
#define Mask_AllShort   (MModReg+MModIReg+MModDisp16+MModImm+MModAbs16+MModAbs32+MModPCRel16+MModPCRel32+MModPop+MModPush+MModPCChain+MModAbsChain)
#define Mask_AllGen     (Mask_AllShort+MModDisp32+MModRegChain)
#define Mask_NoImmShort (Mask_AllShort-MModImm)
#define Mask_NoImmGen   (Mask_AllGen-MModImm)
#define Mask_MemShort   (Mask_NoImmShort-MModReg)
#define Mask_MemGen     (Mask_NoImmGen-MModReg)

#define Mask_Source     (Mask_AllGen-MModPush)
#define Mask_Dest       (Mask_NoImmGen-MModPop)
#define Mask_PureDest   (Mask_NoImmGen-MModPush-MModPop)
#define Mask_PureMem    (Mask_MemGen-MModPush-MModPop)

#define FixedOrderCount 7
#define OneOrderCount 13
#define GE2OrderCount 11
#define BitOrderCount 6
#define GetPutOrderCount 8
#define BFieldOrderCount 4
#define MulOrderCount 4
#define ConditionCount 14
#define LogOrderCount 3

typedef struct {
   char *Name;
   Word Code;
} FixedOrder;

typedef struct {
   char *Name;
   Word Mask;
   Byte OpMask;
   Word Code;
} OneOrder;

typedef struct {
   char *Name;
   Word Mask1, Mask2;
   Word SMask1, SMask2;
   Word Code;
   bool Signed;
} GE2Order;

typedef struct {
   char *Name;
   bool MustByte;
   Word Code1, Code2;
} BitOrder;

typedef struct {
   char *Name;
   ShortInt Size;
   Word Code;
   bool Turn;
} GetPutOrder;

static CPUVar CPUM16;

static String Format;
static Byte FormatCode;
static ShortInt DOpSize, OpSize[5];
static Word AdrMode[5];
static ShortInt AdrType[5];
static Byte AdrCnt1[5], AdrCnt2[5];
static Word AdrVals[5][8];

static Byte OptionCnt;
static char Options[2][5];

static FixedOrder *FixedOrders;
static OneOrder *OneOrders;
static GE2Order *GE2Orders;
static BitOrder *BitOrders;
static GetPutOrder *GetPutOrders;
static char **BFieldOrders;
static char **MulOrders;
static char **Conditions;
static char **LogOrders;

/*------------------------------------------------------------------------*/

static void AddFixed(char *NName, Word NCode) {
   if (InstrZ >= FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name = NName;
   FixedOrders[InstrZ++].Code = NCode;
}

static void AddOne(char *NName, Byte NOpMask, Word NMask, Word NCode) {
   if (InstrZ >= OneOrderCount) exit(255);
   OneOrders[InstrZ].Name = NName;
   OneOrders[InstrZ].Code = NCode;
   OneOrders[InstrZ].Mask = NMask;
   OneOrders[InstrZ++].OpMask = NOpMask;
}

static void AddGE2(char *NName, Word NMask1, Word NMask2, Byte NSMask1, Byte NSMask2, Word NCode, bool NSigned) {
   if (InstrZ >= GE2OrderCount) exit(255);
   GE2Orders[InstrZ].Name = NName;
   GE2Orders[InstrZ].Mask1 = NMask1;
   GE2Orders[InstrZ].Mask2 = NMask2;
   GE2Orders[InstrZ].SMask1 = NSMask1;
   GE2Orders[InstrZ].SMask2 = NSMask2;
   GE2Orders[InstrZ].Code = NCode;
   GE2Orders[InstrZ++].Signed = NSigned;
}

static void AddBit(char *NName, bool NMust, Word NCode1, Word NCode2) {
   if (InstrZ >= BitOrderCount) exit(255);
   BitOrders[InstrZ].Name = NName;
   BitOrders[InstrZ].Code1 = NCode1;
   BitOrders[InstrZ].Code2 = NCode2;
   BitOrders[InstrZ++].MustByte = NMust;
}

static void AddGetPut(char *NName, Byte NSize, Word NCode, bool NTurn) {
   if (InstrZ >= GetPutOrderCount) exit(255);
   GetPutOrders[InstrZ].Name = NName;
   GetPutOrders[InstrZ].Code = NCode;
   GetPutOrders[InstrZ].Turn = NTurn;
   GetPutOrders[InstrZ++].Size = NSize;
}

static void InitFields(void) {
   FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCount);
   InstrZ = 0;
   AddFixed("NOP", 0x1bd6);
   AddFixed("PIB", 0x0bd6);
   AddFixed("RIE", 0x08f7);
   AddFixed("RRNG", 0x3bd6);
   AddFixed("RTS", 0x2bd6);
   AddFixed("STCTX", 0x07d6);
   AddFixed("REIT", 0x2fd6);

   OneOrders = (OneOrder *) malloc(sizeof(OneOrder) * OneOrderCount);
   InstrZ = 0;
   AddOne("ACS", 0x00, Mask_PureMem, 0x8300);
   AddOne("NEG", 0x07, Mask_PureDest, 0xc800);
   AddOne("NOT", 0x07, Mask_PureDest, 0xcc00);
   AddOne("JMP", 0x00, Mask_PureMem, 0x8200);
   AddOne("JSR", 0x00, Mask_PureMem, 0xaa00);
   AddOne("LDCTX", 0x00, MModIReg + MModDisp16 + MModDisp32 + MModAbs16 + MModAbs32 + MModPCRel16 + MModPCRel32, 0x8600);
   AddOne("LDPSB", 0x02, Mask_Source, 0xdb00);
   AddOne("LDPSM", 0x02, Mask_Source, 0xdc00);
   AddOne("POP", 0x04, Mask_PureDest, 0x9000);
   AddOne("PUSH", 0x04, Mask_Source - MModPop, 0xb000);
   AddOne("PUSHA", 0x00, Mask_PureMem, 0xa200);
   AddOne("STPSB", 0x02, Mask_Dest, 0xdd00);
   AddOne("STPSM", 0x02, Mask_Dest, 0xde00);

   GE2Orders = (GE2Order *) malloc(sizeof(GE2Order) * GE2OrderCount);
   InstrZ = 0;
   AddGE2("ADDU", Mask_Source, Mask_PureDest, 7, 7, 0x0400, false);
   AddGE2("ADDX", Mask_Source, Mask_PureDest, 7, 7, 0x1000, true);
   AddGE2("SUBU", Mask_Source, Mask_PureDest, 7, 7, 0x0c00, false);
   AddGE2("SUBX", Mask_Source, Mask_PureDest, 7, 7, 0x1800, true);
   AddGE2("CMPU", Mask_Source, Mask_PureDest, 7, 7, 0x8400, false);
   AddGE2("LDC", Mask_Source, Mask_PureDest, 7, 4, 0x9800, true);
   AddGE2("LDP", Mask_Source, Mask_PureMem, 7, 7, 0x9c00, true);
   AddGE2("MOVU", Mask_Source, Mask_Dest, 7, 7, 0x8c00, true);
   AddGE2("REM", Mask_Source, Mask_PureDest, 7, 7, 0x5800, true);
   AddGE2("REMU", Mask_Source, Mask_PureDest, 7, 7, 0x5c00, true);
   AddGE2("ROT", Mask_Source, Mask_PureDest, 1, 7, 0x3800, true);

   BitOrders = (BitOrder *) malloc(sizeof(BitOrder) * BitOrderCount);
   InstrZ = 0;
   AddBit("BCLR", false, 0xb400, 0xa180);
   AddBit("BCLRI", true, 0xa400, 0x0000);
   AddBit("BNOT", false, 0xb800, 0x0000);
   AddBit("BSET", false, 0xb000, 0x8180);
   AddBit("BSETI", true, 0xa000, 0x81c0);
   AddBit("BTST", false, 0xbc00, 0xa1c0);

   GetPutOrders = (GetPutOrder *) malloc(sizeof(GetPutOrder) * GetPutOrderCount);
   InstrZ = 0;
   AddGetPut("GETB0", 0, 0xc000, false);
   AddGetPut("GETB1", 0, 0xc400, false);
   AddGetPut("GETB2", 0, 0xc800, false);
   AddGetPut("GETH0", 1, 0xcc00, false);
   AddGetPut("PUTB0", 0, 0xd000, true);
   AddGetPut("PUTB1", 0, 0xd400, true);
   AddGetPut("PUTB2", 0, 0xd800, true);
   AddGetPut("PUTH0", 1, 0xdc00, true);

   BFieldOrders = (char **)malloc(sizeof(char *) * BFieldOrderCount);
   InstrZ = 0;
   BFieldOrders[InstrZ++] = "BFCMP";
   BFieldOrders[InstrZ++] = "BFCMPU";
   BFieldOrders[InstrZ++] = "BFINS";
   BFieldOrders[InstrZ++] = "BFINSU";

   MulOrders = (char **)malloc(sizeof(char *) * MulOrderCount);
   InstrZ = 0;
   MulOrders[InstrZ++] = "MUL";
   MulOrders[InstrZ++] = "MULU";
   MulOrders[InstrZ++] = "DIV";
   MulOrders[InstrZ++] = "DIVU";

   Conditions = (char **)malloc(sizeof(char *) * ConditionCount);
   InstrZ = 0;
   Conditions[InstrZ++] = "XS";
   Conditions[InstrZ++] = "XC";
   Conditions[InstrZ++] = "EQ";
   Conditions[InstrZ++] = "NE";
   Conditions[InstrZ++] = "LT";
   Conditions[InstrZ++] = "GE";
   Conditions[InstrZ++] = "LE";
   Conditions[InstrZ++] = "GT";
   Conditions[InstrZ++] = "VS";
   Conditions[InstrZ++] = "VC";
   Conditions[InstrZ++] = "MS";
   Conditions[InstrZ++] = "MC";
   Conditions[InstrZ++] = "FS";
   Conditions[InstrZ++] = "FC";

   LogOrders = (char **)malloc(sizeof(char *) * LogOrderCount);
   InstrZ = 0;
   LogOrders[InstrZ++] = "AND";
   LogOrders[InstrZ] = "OR";
   LogOrders[InstrZ++] = "XOR";
}

static void DeinitFields(void) {
   free(FixedOrders);
   free(OneOrders);
   free(GE2Orders);
   free(BitOrders);
   free(GetPutOrders);
   free(BFieldOrders);
   free(MulOrders);
   free(Conditions);
   free(LogOrders);
}

/*------------------------------------------------------------------------*/

typedef enum { DispSizeNone, DispSize4, DispSize4Eps, DispSize16, DispSize32 } DispSize;
typedef struct _TChainRec {
   struct _TChainRec *Next;
   Byte RegCnt;
   Word Regs[5], Scales[5];
   LongInt DispAcc;
   bool HasDisp;
   DispSize DSize;
} *PChainRec, TChainRec;
static bool ErrFlag;

static bool IsD4(LongInt inp) {
   return ((inp >= -32) && (inp <= 28));
}

static bool IsD16(LongInt inp) {
   return ((inp >= -0x8000) && (inp <= 0x7fff));
}

static bool DecodeReg(char *Asc, Word * Erg) {
   bool IO;

   if (strcasecmp(Asc, "SP") == 0) *Erg = 15;
   else if (strcasecmp(Asc, "FP") == 0) *Erg = 14;
   else if ((strlen(Asc) > 1) && (toupper(*Asc) == 'R')) {
      *Erg = ConstLongInt(Asc + 1, &IO);
      return ((IO) && (*Erg <= 15));
   } else return false;
   return true;
}

static void SplitSize(char *s, DispSize * Erg) {
   Integer l = strlen(s);

   if ((l > 2) && (s[l - 1] == '4') && (s[l - 2] == ':')) {
      *Erg = DispSize4;
      s[l - 2] = '\0';
   } else if ((l > 3) && (s[l - 1] == '6') && (s[l - 2] == '1') && (s[l - 3] == ':')) {
      *Erg = DispSize16;
      s[l - 3] = '\0';
   } else if ((l > 3) && (s[l - 1] == '2') && (s[l - 2] == '3') && (s[l - 3] == ':')) {
      *Erg = DispSize32;
      s[l - 3] = '\0';
   }
}

static void DecideAbs(LongInt Disp, DispSize Size, Word Mask, Integer Index) {
   switch (Size) {
      case DispSize4:
         Size = DispSize16;
         break;
      case DispSizeNone:
         if ((IsD16(Disp)) && ((Mask & MModAbs16) != 0)) Size = DispSize16;
         else Size = DispSize32;
         break;
      default:
         break;
   }

   switch (Size) {
      case DispSize16:
         if (ChkRange(Disp, -0x8000, 0x7fff)) {
            AdrType[Index] = ModAbs16;
            AdrMode[Index] = 0x09;
            AdrVals[Index][0] = Disp & 0xffff;
            AdrCnt1[Index] = 2;
         }
         break;
      case DispSize32:
         AdrType[Index] = ModAbs32;
         AdrMode[Index] = 0x0a;
         AdrVals[Index][0] = Disp >> 16;
         AdrVals[Index][1] = Disp & 0xffff;
         AdrCnt1[Index] = 4;
         break;
      default:
         WrError(10000);
   }
}

static void SetError(Word Code) {
   WrError(Code);
   ErrFlag = true;
}

static PChainRec DecodeChain(char *Asc) {
   PChainRec Rec;
   String Part, SReg;
   Integer z;
   char *p;
   bool OK;
   Byte Scale;

   ChkStack();
   Rec = (PChainRec) malloc(sizeof(TChainRec));
   Rec->Next = NULL;
   Rec->RegCnt = 0;
   Rec->DispAcc = 0;
   Rec->HasDisp = false;
   Rec->DSize = DispSizeNone;

   while ((*Asc != '\0') && (!ErrFlag)) {

   /* eine Komponente abspalten */

      p = QuotPos(Asc, ',');
      if (p == NULL) {
         strmaxcpy(Part, Asc, 255);
         *Asc = '\0';
      } else {
         *p = '\0';
         strmaxcpy(Part, Asc, 255);
         strcopy(Asc, p + 1);
      }

      strcopy(SReg, Part);
      p = QuotPos(SReg, '*');
      if (p != NULL) *p = '\0';

   /* weitere Indirektion ? */

      if (*Part == '@')
         if (Rec->Next != NULL) SetError(1350);
         else {
            strmove(Part, 1);
            if (IsIndirect(Part)) {
               strmove(Part, 1);
               Part[strlen(Part) - 1] = '\0';
            }
            Rec->Next = DecodeChain(Part);
         }

   /* Register, mit Skalierungsfaktor ? */

      else if (DecodeReg(SReg, Rec->Regs + Rec->RegCnt)) {
         if (Rec->RegCnt >= 5) SetError(1350);
         else {
            FirstPassUnknown = false;
            if (p == NULL) {
               OK = true;
               Scale = 1;
            } else Scale = EvalIntExpression(p + 1, UInt4, &OK);
            if (FirstPassUnknown) Scale = 1;
            if (!OK) ErrFlag = true;
            else if ((Scale != 1) && (Scale != 2) && (Scale != 4) && (Scale != 8)) SetError(1350);
            else {
               Rec->Scales[Rec->RegCnt] = 0;
               while (Scale > 1) {
                  Rec->Scales[Rec->RegCnt]++;
                  Scale = Scale >> 1;
               }
               Rec->RegCnt++;
            }
         }
      }

   /* PC, mit Skalierungsfaktor ? */

      else if (strcasecmp(SReg, "PC") == 0) {
         if (Rec->RegCnt >= 5) SetError(1350);
         else {
            FirstPassUnknown = false;
            if (p == NULL) {
               OK = true;
               Scale = 1;
            } else Scale = EvalIntExpression(p + 1, UInt4, &OK);
            if (FirstPassUnknown) Scale = 1;
            if (!OK) ErrFlag = true;
            else if ((Scale != 1) && (Scale != 2) && (Scale != 4) && (Scale != 8)) SetError(1350);
            else {
               for (z = Rec->RegCnt - 1; z >= 0; z--) {
                  Rec->Regs[z + 1] = Rec->Regs[z];
                  Rec->Scales[z + 1] = Rec->Scales[z];
               }
               Rec->Scales[0] = 0;
               while (Scale > 1) {
                  Rec->Scales[0]++;
                  Scale = Scale >> 1;
               }
               Rec->Regs[0] = 16;
               Rec->RegCnt++;
            }
         }
      }

   /* ansonsten Displacement */

      else {
         SplitSize(Part, &(Rec->DSize));
         Rec->DispAcc += EvalIntExpression(Part, Int32, &OK);
         if (!OK) ErrFlag = true;
         Rec->HasDisp = true;
      }
   }

   if (ErrFlag) {
      free(Rec);
      return NULL;
   } else return Rec;
}

static bool ChkAdr(Word Mask, Integer Index) {
   AdrCnt2[Index] = AdrCnt1[Index] >> 1;
   if ((AdrType[Index] != -1) && ((Mask & (1 << AdrType[Index])) == 0)) {
      AdrCnt1[Index] = AdrCnt2[Index] = 0;
      AdrType[Index] = ModNone;
      WrError(1350);
      return false;
   } else return (AdrType[Index] != ModNone);
}

static bool DecodeAdr(char *Asc, Integer Index, Word Mask) {
   LongInt AdrLong, MinReserve, MaxReserve;
   Integer z, z2, LastChain;
   bool OK;
   PChainRec RootChain, RunChain, PrevChain;
   DispSize DSize;

   AdrCnt1[Index] = 0;
   AdrType[Index] = ModNone;

/* Register ? */

   if (DecodeReg(Asc, AdrMode + Index)) {
      AdrType[Index] = ModReg;
      AdrMode[Index] += 0x10;
      return ChkAdr(Mask, Index);
   }

/* immediate ? */

   if (*Asc == '#') {
      switch (OpSize[Index]) {
         case -1:
            WrError(1132);
            OK = false;
            break;
         case 0:
            AdrVals[Index][0] = EvalIntExpression(Asc + 1, Int8, &OK) & 0xff;
            if (OK) AdrCnt1[Index] = 2;
            break;
         case 1:
            AdrVals[Index][0] = EvalIntExpression(Asc + 1, Int16, &OK);
            if (OK) AdrCnt1[Index] = 2;
            break;
         case 2:
            AdrLong = EvalIntExpression(Asc + 1, Int32, &OK);
            if (OK) {
               AdrVals[Index][0] = AdrLong >> 16;
               AdrVals[Index][1] = AdrLong & 0xffff;
               AdrCnt1[Index] = 4;
            }
            break;
      }
      if (OK) {
         AdrType[Index] = ModImm;
         AdrMode[Index] = 0x0c;
      }
      return ChkAdr(Mask, Index);
   }

/* indirekt ? */

   if (*Asc == '@') {
      strmove(Asc, 1);
      if (IsIndirect(Asc)) {
         strmove(Asc, 1);
         Asc[strlen(Asc) - 1] = '\0';
      }

   /* Stack Push ? */

      if ((strcasecmp(Asc, "-R15") == 0) || (strcasecmp(Asc, "-SP") == 0)) {
         AdrType[Index] = ModPush;
         AdrMode[Index] = 0x05;
         return ChkAdr(Mask, Index);
      }

   /* Stack Pop ? */

      if ((strcasecmp(Asc, "R15+") == 0) || (strcasecmp(Asc, "SP+") == 0)) {
         AdrType[Index] = ModPop;
         AdrMode[Index] = 0x04;
         return ChkAdr(Mask, Index);
      }

   /* Register einfach indirekt ? */

      if (DecodeReg(Asc, AdrMode + Index)) {
         AdrType[Index] = ModIReg;
         AdrMode[Index] += 0x30;
         return ChkAdr(Mask, Index);
      }

   /* zusammengesetzt indirekt ? */

      ErrFlag = false;
      RootChain = DecodeChain(Asc);

      if (ErrFlag);

      else if (RootChain == NULL);

   /* absolut ? */

      else if ((RootChain->Next == NULL) && (RootChain->RegCnt == 0)) {
         if (!RootChain->HasDisp) RootChain->DispAcc = 0;
         DecideAbs(RootChain->DispAcc, RootChain->DSize, Mask, Index);
         free(RootChain);
      }

   /* einfaches Register/PC mit Displacement ? */

      else if ((RootChain->Next == NULL) && (RootChain->RegCnt == 1) && (RootChain->Scales[0] == 0)) {
         if (RootChain->Regs[0] == 16) RootChain->DispAcc -= EProgCounter();

      /* Displacement-Groesse entscheiden */

         if (RootChain->DSize != DispSizeNone) ;
         else if ((RootChain->DispAcc == 0) && (RootChain->Regs[0] < 16));
         else if (IsD16(RootChain->DispAcc))
            RootChain->DSize = DispSize16;
         else RootChain->DSize = DispSize32;

         switch (RootChain->DSize) {

            /* kein Displacement mit Register */

            case DispSizeNone:
               if (!ChkRange(RootChain->DispAcc, 0, 0)) ;
               else if (RootChain->Regs[0] >= 16) WrError(1350);
               else {
                  AdrType[Index] = ModIReg;
                  AdrMode[Index] = 0x30 + RootChain->Regs[0];
               }
               break;

            /* 16-Bit-Displacement */

            case DispSize4:
            case DispSize16:
               if (ChkRange(RootChain->DispAcc, -0x8000, 0x7fff)) {
                  AdrVals[Index][0] = RootChain->DispAcc & 0xffff;
                  AdrCnt1[Index] = 2;
                  if (RootChain->Regs[0] == 16) {
                     AdrType[Index] = ModPCRel16;
                     AdrMode[Index] = 0x0d;
                  } else {
                     AdrType[Index] = ModDisp16;
                     AdrMode[Index] = 0x20 + RootChain->Regs[0];
                  }
               }
               break;

            /* 32-Bit-Displacement */

            default:
               AdrVals[Index][1] = RootChain->DispAcc & 0xffff;
               AdrVals[Index][0] = RootChain->DispAcc >> 16;
               AdrCnt1[Index] = 4;
               if (RootChain->Regs[0] == 16) {
                  AdrType[Index] = ModPCRel32;
                  AdrMode[Index] = 0x0e;
               } else {
                  AdrType[Index] = ModDisp32;
                  AdrMode[Index] = 0x40 + RootChain->Regs[0];
               }
         }

         free(RootChain);
      }

   /* komplex: dann chained iterieren */

      else {
      /* bis zum innersten Element der Indirektion als Basis laufen */

         RunChain = RootChain;
         while (RunChain->Next != NULL) RunChain = RunChain->Next;

      /* Entscheidung des Basismodus: die Basis darf nicht skaliert
         sein, und wenn ein Modus nicht erlaubt ist, muessen wir mit
         Base-none anfangen... */

         z = 0;
         while ((z < RunChain->RegCnt) && (RunChain->Scales[z] != 0)) z++;
         if (z >= RunChain->RegCnt) {
            AdrType[Index] = ModAbsChain;
            AdrMode[Index] = 0x0b;
         } else {
            if (RunChain->Regs[z] == 16) {
               AdrType[Index] = ModPCChain;
               AdrMode[Index] = 0x0f;
               RunChain->DispAcc -= EProgCounter();
            } else {
               AdrType[Index] = ModRegChain;
               AdrMode[Index] = 0x60 + RunChain->Regs[z];
            }
            for (z2 = z; z2 <= RunChain->RegCnt - 2; z2++) {
               RunChain->Regs[z2] = RunChain->Regs[z2 + 1];
               RunChain->Scales[z2] = RunChain->Scales[z2 + 1];
            }
            RunChain->RegCnt--;
         }

      /* Jetzt ueber die einzelnen Komponenten iterieren */

         LastChain = 0;
         while (RootChain != NULL) {
            RunChain = RootChain;
            PrevChain = NULL;
            while (RunChain->Next != NULL) {
               PrevChain = RunChain;
               RunChain = RunChain->Next;
            }

         /* noch etwas abzulegen ? */

            if ((RunChain->RegCnt != 0) || (RunChain->HasDisp)) {
               LastChain = AdrCnt1[Index] >> 1;

            /* Register ablegen */

               if (RunChain->RegCnt != 0) {
                  if (RunChain->Regs[0] == 16) AdrVals[Index][LastChain] = 0x0600;
                  else AdrVals[Index][LastChain] = RunChain->Regs[0] << 10;
                  AdrVals[Index][LastChain] += RunChain->Scales[0] << 5;
                  for (z2 = 0; z2 <= RunChain->RegCnt - 2; z2++) {
                     RunChain->Regs[z2] = RunChain->Regs[z2 + 1];
                     RunChain->Scales[z2] = RunChain->Scales[z2 + 1];
                  }
                  RunChain->RegCnt--;
               } else AdrVals[Index][LastChain] = 0x0200;
               AdrCnt1[Index] += 2;

            /* Displacement ablegen */

               if (RunChain->HasDisp) {
                  if ((AdrVals[Index][LastChain] & 0x3e00) == 0x0600)
                     RunChain->DispAcc -= EProgCounter();

                  if (RunChain->DSize == DispSizeNone) {
                     MinReserve = 32 * RunChain->RegCnt;
                     MaxReserve = 28 * RunChain->RegCnt;
                     if (IsD4(RunChain->DispAcc))
                        if ((RunChain->DispAcc & 3) == 0) DSize = DispSize4;
                        else DSize = DispSize16;
                     else if ((RunChain->DispAcc >= -32 - MinReserve) && (RunChain->DispAcc <= 28 + MaxReserve)) DSize = DispSize4Eps;
                     else if (IsD16(RunChain->DispAcc)) DSize = DispSize16;
                     else if ((RunChain->DispAcc >= -0x8000 - MinReserve) && (RunChain->DispAcc <= 0x7fff + MaxReserve)) DSize = DispSize4Eps;
                     else DSize = DispSize32;
                  } else DSize = RunChain->DSize;
                  RunChain->DSize = DispSizeNone;

                  switch (DSize) {

                     /* Fall 1: passt komplett in 4er-Displacement */

                     case DispSize4:
                        if (!ChkRange(RunChain->DispAcc, -32, 28)) ;
                        else if ((RunChain->DispAcc & 3) != 0) WrError(1325);
                        else {
                           AdrVals[Index][LastChain] += (RunChain->DispAcc >> 2) & 0x000f;
                           RunChain->HasDisp = false;
                        }
                        break;

                     /* Fall 2: passt nicht mehr in naechstkleineres Displacement, aber wir
                        koennen hier schon einen Teil ablegen, um im naechsten Iterations-
                        schritt ein kuerzeres Displacement zu bekommen */

                     case DispSize4Eps:
                        if (RunChain->DispAcc > 0) {
                           AdrVals[Index][LastChain] += 0x0007;
                           RunChain->DispAcc -= 28;
                        } else {
                           AdrVals[Index][LastChain] += 0x0008;
                           RunChain->DispAcc += 32;
                        }
                        break;

                     /* Fall 3: 16 Bit */

                     case DispSize16:
                        if (ChkRange(RunChain->DispAcc, -0x8000, 0x7fff)) {
                           AdrVals[Index][LastChain] += 0x0011;
                           AdrVals[Index][LastChain + 1] = RunChain->DispAcc & 0xffff;
                           AdrCnt1[Index] += 2;
                           RunChain->HasDisp = false;
                        }
                        break;

                     /* Fall 4: 32 Bit */

                     case DispSize32:
                        AdrVals[Index][LastChain] += 0x0012;
                        AdrVals[Index][LastChain + 1] = RunChain->DispAcc >> 16;
                        AdrVals[Index][LastChain + 2] = RunChain->DispAcc & 0xffff;
                        AdrCnt1[Index] += 4;
                        RunChain->HasDisp = false;
                        break;

                     default:
                        WrError(10000);
                  }
               }
            }

         /* nichts mehr drin: dann ein leeres Steuerwort erzeugen.  Tritt
            auf, falls alles schon im Basisadressierungsbyte verschwunden */

            else if (RunChain != RootChain) {
               LastChain = AdrCnt1[Index] >> 1;
               AdrVals[Index][LastChain] = 0x0200;
               AdrCnt1[Index] += 2;
            }

         /* nichts mehr drin: wegwerfen
            wenn wir noch nicht ganz vorne angekommen sind, dann ein
            Indirektionsflag setzen */

            if ((RunChain->RegCnt == 0) && (!RunChain->HasDisp)) {
               if (RunChain != RootChain) AdrVals[Index][LastChain] += 0x4000;
               if (PrevChain == NULL) RootChain = NULL;
               else PrevChain->Next = NULL;
               free(RunChain);
            }
         }

      /* Ende-Kennung fuer letztes Glied */

         AdrVals[Index][LastChain] += 0x8000;
      }

      return ChkAdr(Mask, Index);
   }

/* ansonsten absolut */

   DSize = DispSizeNone;
   SplitSize(Asc, &DSize);
   AdrLong = EvalIntExpression(Asc, Int32, &OK);
   if (OK) DecideAbs(AdrLong, DSize, Mask, Index);

   return ChkAdr(Mask, Index);
}

static LongInt ImmVal(Integer Index) {
   switch (OpSize[Index]) {
      case 0:
         return (ShortInt) (AdrVals[Index][0] & 0xff);
      case 1:
         return (Integer) (AdrVals[Index][0]);
      case 2:
         return (((LongInt) AdrVals[Index][0]) << 16) + ((Integer) AdrVals[Index][1]);
      default:
         WrError(10000);
         return 0;
   }
}

static bool IsShort(Integer Index) {
   return ((AdrMode[Index] & 0xc0) == 0);
}

static void AdaptImm(Integer Index, Byte NSize, bool Signed) {
   switch (OpSize[Index]) {
      case 0:
         if (NSize != 0) {
            if (((AdrVals[Index][0] & 0x80) == 0x80) && (Signed))
               AdrVals[Index][0] |= 0xff00;
            else AdrVals[Index][0] &= 0xff;
            if (NSize == 2) {
               if (((AdrVals[Index][0] & 0x8000) == 0x8000) && (Signed))
                  AdrVals[Index][1] = 0xffff;
               else AdrVals[Index][1] = 0;
               AdrCnt1[Index] += 2;
               AdrCnt2[Index]++;
            }
         }
         break;
      case 1:
         if (NSize == 0) AdrVals[Index][0] &= 0xff;
         else if (NSize == 2) {
            if (((AdrVals[Index][0] & 0x8000) == 0x8000) && (Signed))
               AdrVals[Index][1] = 0xffff;
            else AdrVals[Index][1] = 0;
            AdrCnt1[Index] += 2;
            AdrCnt2[Index]++;
         }
         break;
      case 2:
         if (NSize != 2) {
            AdrCnt1[Index] -= 2;
            AdrCnt2[Index]--;
            if (NSize == 0) AdrVals[Index][0] &= 0xff;
         }
         break;
   }
   OpSize[Index] = NSize;
}

static ShortInt DefSize(Byte Mask) {
   ShortInt z;

   z = 2;
   while ((z >= 0) && ((Mask & 4) == 0)) {
      Mask = (Mask << 1) & 7;
      z--;
   }
   return z;
}

static Word RMask(Word No, bool Turn) {
   return (Turn) ? (0x8000 >> No) : (1 << No);
}

static bool DecodeRegList(char *Asc, Word * Erg, bool Turn) {
   char Part[11];
   char *p, *p1, *p2;
   Word r1, r2, z;

   if (IsIndirect(Asc)) {
      strmove(Asc, 1);
      Asc[strlen(Asc) - 1] = '\0';
   }
   *Erg = 0;
   while (*Asc != '\0') {
      p1 = strchr(Asc, ',');
      p2 = strchr(Asc, '/');
      if ((p1 != NULL) && (p1 < p2)) p = p1;
      else p = p2;
      if (p == NULL) {
         strmaxcpy(Part, Asc, 11);
         *Asc = '\0';
      } else {
         *p = '\0';
         strmaxcpy(Part, Asc, 11);
         strcopy(Asc, p + 1);
      }
      p = strchr(Part, '-');
      if (p == NULL) {
         if (!DecodeReg(Part, &r1)) {
            WrXError(1410, Part);
            return false;
         }
         *Erg |= RMask(r1, Turn);
      } else {
         *p = '\0';
         if (!DecodeReg(Part, &r1)) {
            WrXError(1410, Part);
            return false;
         }
         if (!DecodeReg(p + 1, &r2)) {
            WrXError(1410, p + 1);
            return false;
         }
         if (r1 <= r2)
            for (z = r1; z <= r2; z++) *Erg |= RMask(z, Turn);
         else {
            for (z = r2; z <= 15; z++) *Erg |= RMask(z, Turn);
            for (z = 0; z <= r1; z++) *Erg |= RMask(z, Turn);
         }
      }
   }
   return true;
}

static bool DecodeCondition(char *Asc, Word * Erg) {
   Integer z;
   String Asc_N;

   strmaxcpy(Asc_N, Asc, 255);
   NLS_UpString(Asc_N);
   Asc = Asc_N;

   for (z = 0; z < ConditionCount; z++)
      if (strcmp(Asc, Conditions[z]) == 0) break;
   *Erg = z;
   return (z < ConditionCount);
}

/*------------------------------------------------------------------------*/

static bool CheckFormat(char *FSet) {
   char *p;

   if (strcmp(Format, " ") == 0) FormatCode = 0;
   else {
      p = strchr(FSet, *Format);
      if (p != NULL) FormatCode = p - FSet + 1;
      else WrError(1090);
      return (p != NULL);
   }
   return true;
}

static bool CheckBFieldFormat(void) {
   if ((strcmp(Format, "G:R") == 0) || (strcmp(Format, "R:G") == 0)) FormatCode = 1;
   else if ((strcmp(Format, "G:I") == 0) || (strcmp(Format, "I:G") == 0)) FormatCode = 2;
   else if ((strcmp(Format, "E:R") == 0) || (strcmp(Format, "R:E") == 0)) FormatCode = 3;
   else if ((strcmp(Format, "E:I") == 0) || (strcmp(Format, "I:E") == 0)) FormatCode = 4;
   else {
      WrError(1090);
      return false;
   }
   return true;
}

static bool GetOpSize(char *Asc, Byte Index) {
   char *p;
   int l = strlen(Asc);

   p = RQuotPos(Asc, '.');
   if (p == NULL) {
      OpSize[Index] = DOpSize;
      return true;
   } else if (p == Asc + l - 2) {
      switch (p[1]) {
         case 'B':
            OpSize[Index] = 0;
            break;
         case 'H':
            OpSize[Index] = 1;
            break;
         case 'W':
            OpSize[Index] = 2;
            break;
         default:
            WrError(1107);
            return false;
      }
      *p = '\0';
      return true;
   } else {
      WrError(1107);
      return false;
   }
}

static void SplitOptions(void) {
   char *p;
   Integer z;

   OptionCnt = 0;
   *Options[0] = '\0';
   *Options[1] = '\0';
   do {
      p = RQuotPos(OpPart, '/');
      if (p != NULL) {
         if (OptionCnt < 2) {
            for (z = OptionCnt - 1; z >= 0; z--) strcopy(Options[z + 1], Options[z]);
            OptionCnt++;
            strmaxcpy(Options[0], p + 1, 255);
         }
         *p = '\0';
      }
   }
   while (p != NULL);
}

/*------------------------------------------------------------------------*/

static bool DecodePseudo(void) {
   return false;
}

static void DecideBranch(LongInt Adr, Byte Index) {
   LongInt Dist = Adr - EProgCounter();

   if (FormatCode == 0) {
   /* Groessenangabe erzwingt G-Format */
      if (OpSize[Index] != -1) FormatCode = 1;
   /* gerade 9-Bit-Zahl kurz darstellbar */
      else if (((Dist & 1) == 0) && (Dist <= 254) && (Dist >= -256)) FormatCode = 2;
   /* ansonsten allgemein */
      else FormatCode = 1;
   }
   if (FormatCode != 1 || OpSize[Index] != -1) ;
   else if ((Dist <= 127) && (Dist >= -128)) OpSize[Index] = 0;
   else if ((Dist <= 32767) && (Dist >= -32768)) OpSize[Index] = 1;
   else OpSize[Index] = 2;
}

static bool DecideBranchLength(LongInt * Addr, Integer Index) {
   *Addr -= EProgCounter();
   if (OpSize[Index] == -1) {
      if ((*Addr >= -128) && (*Addr <= 127)) OpSize[Index] = 0;
      else if ((*Addr >= -32768) && (*Addr <= 32767)) OpSize[Index] = 1;
      else OpSize[Index] = 2;
   }

   if ((!SymbolQuestionable) && (((OpSize[Index] == 0) && ((*Addr < -128) || (*Addr > 127)))
         || ((OpSize[Index] == 1) && ((*Addr < -32768) || (*Addr > 32767))))) {
      WrError(1370);
      return false;
   } else return true;
}

static void Make_G(Word Code) {
   WAsmCode[0] = 0xd000 + (OpSize[1] << 8) + AdrMode[1];
   memcpy(WAsmCode + 1, AdrVals[1], AdrCnt1[1]);
   WAsmCode[1 + AdrCnt2[1]] = Code + (OpSize[2] << 8) + AdrMode[2];
   memcpy(WAsmCode + 2 + AdrCnt2[1], AdrVals[2], AdrCnt1[2]);
   CodeLen = 4 + AdrCnt1[1] + AdrCnt1[2];
}

static void Make_E(Word Code, bool Signed) {
   LongInt HVal, Min, Max;

   Min = 128 * (-Signed);
   Max = Min + 255;
   if (AdrType[1] != ModImm) WrError(1350);
   else {
      HVal = ImmVal(1);
      if (ChkRange(HVal, Min, Max)) {
         WAsmCode[0] = 0xbf00 + (HVal & 0xff);
         WAsmCode[1] = Code + (OpSize[2] << 8) + AdrMode[2];
         memcpy(WAsmCode + 2, AdrVals[2], AdrCnt1[2]);
         CodeLen = 4 + AdrCnt1[2];
      }
   }
}

static void Make_I(Word Code, bool Signed) {
   if ((AdrType[1] != ModImm) || (!IsShort(2))) WrError(1350);
   else {
      AdaptImm(1, OpSize[2], Signed);
      WAsmCode[0] = Code + (OpSize[2] << 8) + AdrMode[2];
      memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
      memcpy(WAsmCode + 1 + AdrCnt2[2], AdrVals[1], AdrCnt1[1]);
      CodeLen = 2 + AdrCnt1[1] + AdrCnt1[2];
   }
}

static bool CodeAri(void) {
   Integer z;
   Word AdrWord, Mask, Mask2;
   char Form[6];
   LongInt HVal;

   if ((Memo("ADD")) || (Memo("SUB"))) {
      z = Memo("SUB");
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("GELQI"))
         if (GetOpSize(ArgStr[2], 2))
            if (GetOpSize(ArgStr[1], 1)) {
               if (OpSize[2] == -1) OpSize[2] = 2;
               if (OpSize[1] == -1) OpSize[1] = OpSize[2];
               if (DecodeAdr(ArgStr[1], 1, Mask_Source))
                  if (DecodeAdr(ArgStr[2], 2, Mask_PureDest)) {
                     if (FormatCode == 0) {
                        if (AdrType[1] == ModImm) {
                           HVal = ImmVal(1);
                           if (IsShort(2))
                              if ((HVal >= 1) && (HVal <= 8)) FormatCode = 4;
                              else FormatCode = 5;
                           else if ((HVal >= -128) && (HVal < 127)) FormatCode = 2;
                           else FormatCode = 1;
                        } else if (IsShort(1) && (AdrType[2] == ModReg) && (OpSize[1] == 2) && (OpSize[2] == 2)) FormatCode = 3;
                        else FormatCode = 1;
                     }
                     switch (FormatCode) {
                        case 1:
                           Make_G(z << 11);
                           break;
                        case 2:
                           Make_E(z << 11, true);
                           break;
                        case 3:
                           if ((!IsShort(1)) || (AdrType[2] != ModReg)) WrError(1350);
                           else if ((OpSize[1] != 2) || (OpSize[2] != 2)) WrError(1130);
                           else {
                              WAsmCode[0] = 0x8100 + (z << 6) + ((AdrMode[2] & 15) << 10) + AdrMode[1];
                              memcpy(WAsmCode + 1, AdrVals[1], AdrCnt1[1]);
                              CodeLen = 2 + AdrCnt1[1];
                              if ((AdrMode[1] == 0x04) & (AdrMode[2] == 15)) WrError(140);
                           }
                           break;
                        case 4:
                           if ((AdrType[1] != ModImm) || (!IsShort(2))) WrError(1350);
                           else {
                              HVal = ImmVal(1);
                              if (ChkRange(HVal, 1, 8)) {
                                 WAsmCode[0] = 0x4040 + (z << 13) + ((HVal & 7) << 10) + (OpSize[2] << 8) + AdrMode[2];
                                 memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
                                 CodeLen = 2 + AdrCnt1[2];
                              }
                           }
                           break;
                        case 5:
                           Make_I(0x44c0 + (z << 11), true);
                           break;
                     }
                  }
            }
      return true;
   }

   if (Memo("CMP")) {
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("GELZQI"))
         if (GetOpSize(ArgStr[1], 1))
            if (GetOpSize(ArgStr[2], 2)) {
               if (OpSize[2] == -1) OpSize[2] = 2;
               if (OpSize[1] == -1) OpSize[1] = OpSize[2];
               if (DecodeAdr(ArgStr[1], 1, Mask_Source))
                  if (DecodeAdr(ArgStr[2], 2, Mask_NoImmGen - MModPush)) {
                     if (FormatCode == 0) {
                        if (AdrType[1] == ModImm) {
                           HVal = ImmVal(1);
                           if (HVal == 0) FormatCode = 4;
                           else if ((HVal >= 1) && (HVal <= 8) && (IsShort(2))) FormatCode = 5;
                           else if ((HVal >= -128) && (HVal <= 127)) FormatCode = 2;
                           else if (AdrType[2] == ModReg) FormatCode = 3;
                           else if (IsShort(2)) FormatCode = 5;
                           else FormatCode = 1;
                        } else if ((IsShort(1)) && (AdrType[2] == ModReg)) FormatCode = 3;
                        else FormatCode = 1;
                     }
                     switch (FormatCode) {
                        case 1:
                           Make_G(0x8000);
                           break;
                        case 2:
                           Make_E(0x8000, true);
                           break;
                        case 3:
                           if ((!IsShort(1)) || (AdrType[2] != ModReg)) WrError(1350);
                           else if (OpSize[1] != 2) WrError(1130);
                           else {
                              WAsmCode[0] = ((AdrMode[2] & 15) << 10) + (OpSize[2] << 8) + AdrMode[1];
                              memcpy(WAsmCode + 1, AdrVals[1], AdrCnt1[1]);
                              CodeLen = 2 + AdrCnt1[1];
                           }
                           break;
                        case 4:
                           if (AdrType[1] != ModImm) WrError(1350);
                           else {
                              HVal = ImmVal(1);
                              if (ChkRange(HVal, 0, 0)) {
                                 WAsmCode[0] = 0xc000 + (OpSize[2] << 8) + AdrMode[2];
                                 memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
                                 CodeLen = 2 + AdrCnt1[2];
                              }
                           }
                           break;
                        case 5:
                           if ((AdrType[1] != ModImm) || (!IsShort(2))) WrError(1350);
                           else {
                              HVal = ImmVal(1);
                              if (ChkRange(HVal, 1, 8)) {
                                 WAsmCode[0] = 0x4000 + (OpSize[2] << 8) + AdrMode[2] + ((HVal & 7) << 10);
                                 memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
                                 CodeLen = 2 + AdrCnt1[2];
                              }
                           }
                           break;
                        case 6:
                           Make_I(0x40c0, true);
                           break;
                     }
                  }
            }
      return true;
   }

   for (z = 0; z < GE2OrderCount; z++)
      if (Memo(GE2Orders[z].Name)) {
         if (ArgCnt != 2) WrError(1110);
         else if (CheckFormat("GE"))
            if (GetOpSize(ArgStr[2], 2))
               if (GetOpSize(ArgStr[1], 1)) {
                  if (OpSize[2] == -1) OpSize[2] = DefSize(GE2Orders[z].SMask2);
                  if (OpSize[1] == -1) OpSize[1] = DefSize(GE2Orders[z].SMask1);
                  if (((GE2Orders[z].SMask1 & (1 << OpSize[1])) == 0) || ((GE2Orders[z].SMask2 & (1 << OpSize[2])) == 0)) WrError(1130);
                  else if (DecodeAdr(ArgStr[1], 1, GE2Orders[z].Mask1))
                     if (DecodeAdr(ArgStr[2], 2, GE2Orders[z].Mask2)) {
                        if (FormatCode == 0) {
                           if (AdrType[1] == ModImm) {
                              HVal = ImmVal(1);
                              if ((GE2Orders[z].Signed) && (HVal >= -128) && (HVal <= 127)) FormatCode = 2;
                              else if ((!GE2Orders[z].Signed) && (HVal >= 0) && (HVal <= 255)) FormatCode = 2;
                              else FormatCode = 1;
                           } else FormatCode = 1;
                        }
                        switch (FormatCode) {
                           case 1:
                              Make_G(GE2Orders[z].Code);
                              break;
                           case 2:
                              Make_E(GE2Orders[z].Code, GE2Orders[z].Signed);
                              break;
                        }
                     }
               }
         return true;
      }

   for (z = 0; z < LogOrderCount; z++)
      if (Memo(LogOrders[z])) {
         if (ArgCnt != 2) WrError(1110);
         else if (CheckFormat("GERI"))
            if (GetOpSize(ArgStr[1], 1))
               if (GetOpSize(ArgStr[2], 2)) {
                  if (OpSize[2] == -1) OpSize[2] = 2;
                  if (OpSize[1] == -1) OpSize[1] = OpSize[2];
                  if (DecodeAdr(ArgStr[1], 1, Mask_Source))
                     if (DecodeAdr(ArgStr[2], 2, Mask_Dest - MModPush)) {
                        if (FormatCode == 0) {
                           if (AdrType[1] == ModImm) {
                              HVal = ImmVal(1);
                              if ((HVal >= 0) && (HVal <= 255)) FormatCode = 2;
                              else if (IsShort(2)) FormatCode = 4;
                              else FormatCode = 1;
                           } else if ((AdrType[1] == ModReg) && (AdrType[2] == ModReg) && (OpSize[1] == 2) && (OpSize[2] == 2))
                              FormatCode = 3;
                           else FormatCode = 1;
                        }
                        switch (FormatCode) {
                           case 1:
                              Make_G(0x2000 + (z << 10));
                              break;
                           case 2:
                              Make_E(0x2000 + (z << 10), false);
                              break;
                           case 3:
                              if ((AdrType[1] != ModReg) || (AdrType[2] != ModReg)) WrError(1350);
                              else if ((OpSize[1] != 2) || (OpSize[2] != 2)) WrError(1130);
                              else {
                                 WAsmCode[0] = 0x00c0 + (z << 8) + (AdrMode[1] & 15) + ((AdrMode[2] & 15) << 10);
                                 CodeLen = 2;
                              }
                              break;
                           case 4:
                              if ((AdrType[1] != ModImm) || (!IsShort(2))) WrError(1350);
                              else {
                                 WAsmCode[0] = 0x50c0 + (OpSize[2] << 8) + (z << 10) + AdrMode[2];
                                 memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
                                 AdaptImm(1, OpSize[2], false);
                                 memcpy(WAsmCode + 1 + AdrCnt2[2], AdrVals[1], AdrCnt1[1]);
                                 CodeLen = 2 + AdrCnt1[1] + AdrCnt1[2];
                              }
                              break;
                        }
                        if (OpSize[1] > OpSize[2]) WrError(140);
                     }
               }
         return true;
      }

   for (z = 0; z < MulOrderCount; z++)
      if (Memo(MulOrders[z])) {
         strcpy(Form, (Odd(z)) ? "GE" : "GER");
         if (ArgCnt != 2) WrError(1110);
         else if (CheckFormat(Form))
            if (GetOpSize(ArgStr[1], 1))
               if (GetOpSize(ArgStr[2], 2)) {
                  if (OpSize[2] == -1) OpSize[2] = 2;
                  if (OpSize[1] == -1) OpSize[1] = OpSize[2];
                  if (DecodeAdr(ArgStr[1], 1, Mask_Source))
                     if (DecodeAdr(ArgStr[2], 2, Mask_PureDest)) {
                        if (FormatCode == 0) {
                           if (AdrType[1] == ModImm) {
                              HVal = ImmVal(1);
                              if ((HVal >= -128 + (Odd(z) << 7)) && (HVal <= 127 + (Odd(z) << 7))) FormatCode = 2;
                              else FormatCode = 1;
                           } else if ((!Odd(z)) && (AdrType[1] == ModReg) && (OpSize[1] == 2)
                              && (AdrType[2] == ModReg) && (OpSize[2] == 2)) FormatCode = 3;
                           else FormatCode = 1;
                        }
                        switch (FormatCode) {
                           case 1:
                              Make_G(0x4000 + (z << 10));
                              break;
                           case 2:
                              Make_E(0x4000 + (z << 10), !Odd(z));
                              break;
                           case 3:
                              if ((AdrType[1] != ModReg) || (AdrType[2] != ModReg)) WrError(1350);
                              else if ((OpSize[1] != 2) || (OpSize[2] != 2)) WrError(1130);
                              else {
                                 WAsmCode[0] = 0x00d0 + ((AdrMode[2] & 15) << 10) + (z << 7) + (AdrMode[1] & 15);
                                 CodeLen = 2;
                              }
                        }
                     }
               }
         return true;
      }

   for (z = 0; z < GetPutOrderCount; z++)
      if (Memo(GetPutOrders[z].Name)) {
         if (GetPutOrders[z].Turn) {
            Mask = Mask_Source;
            Mask2 = MModReg;
            AdrWord = 1;
         } else {
            Mask = MModReg;
            Mask2 = Mask_Dest;
            AdrWord = 2;
         }
         if (ArgCnt != 2) WrError(1110);
         else if (CheckFormat("G"))
            if (GetOpSize(ArgStr[1], 1))
               if (GetOpSize(ArgStr[2], 2)) {
                  if (OpSize[AdrWord] == -1) OpSize[AdrWord] = GetPutOrders[z].Size;
                  if (OpSize[3 - AdrWord] == -1) OpSize[3 - AdrWord] = 2;
                  if ((OpSize[AdrWord] != GetPutOrders[z].Size) || (OpSize[3 - AdrWord] != 2)) WrError(1130);
                  else if (DecodeAdr(ArgStr[1], 1, Mask))
                     if (DecodeAdr(ArgStr[2], 2, Mask2)) {
                        Make_G(GetPutOrders[z].Code);
                        WAsmCode[0] += 0x0400;
                     }
               }
         return true;
      }

   if (Memo("MOVA")) {
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("GR"))
         if (GetOpSize(ArgStr[2], 2)) {
            if (OpSize[2] == -1) OpSize[2] = 2;
            OpSize[1] = 0;
            if (OpSize[2] != 2) WrError(1110);
            else if (DecodeAdr(ArgStr[1], 1, Mask_PureMem))
               if (DecodeAdr(ArgStr[2], 2, Mask_Dest)) {
                  if (FormatCode != 0) ;
                  else if ((AdrType[1] == ModDisp16) && (AdrType[2] == ModReg)) FormatCode = 2;
                  else FormatCode = 1;
                  switch (FormatCode) {
                     case 1:
                        Make_G(0xb400);
                        WAsmCode[0] += 0x800;
                        break;
                     case 2:
                        if ((AdrType[1] != ModDisp16) || (AdrType[2] != ModReg)) WrError(1350);
                        else {
                           WAsmCode[0] = 0x03c0 + ((AdrMode[2] & 15) << 10) + (AdrMode[1] & 15);
                           WAsmCode[1] = AdrVals[1][0];
                           CodeLen = 4;
                        }
                        break;
                  }
               }
         }
      return true;
   }

   if ((Memo("QINS")) || (Memo("QDEL"))) {
      z = Memo("QINS") << 11;
      Mask = Mask_PureMem;
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("G"))
         if ((Memo("QINS")) || (GetOpSize(ArgStr[2], 2))) {
            if (OpSize[2] == -1) OpSize[2] = 2;
            OpSize[1] = 0;
            if (OpSize[2] != 2) WrError(1130);
            else if (DecodeAdr(ArgStr[1], 1, Mask))
               if (DecodeAdr(ArgStr[2], 2, Mask + (Memo("QDEL") * MModReg))) {
                  Make_G(0xb000 + z);
                  WAsmCode[0] += 0x800;
               }
         }
      return true;
   }

   if (Memo("RVBY")) {
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("G"))
         if (GetOpSize(ArgStr[1], 1))
            if (GetOpSize(ArgStr[2], 2)) {
               if (OpSize[2] == -1) OpSize[2] = 2;
               if (OpSize[1] == -1) OpSize[1] = OpSize[2];
               if (DecodeAdr(ArgStr[1], 1, Mask_Source))
                  if (DecodeAdr(ArgStr[2], 2, Mask_Dest)) {
                     Make_G(0x4000);
                     WAsmCode[0] += 0x400;
                  }
            }
      return true;
   }

   if ((Memo("SHL")) || (Memo("SHA"))) {
      z = Memo("SHA");
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("GEQ"))
         if (GetOpSize(ArgStr[1], 1))
            if (GetOpSize(ArgStr[2], 2)) {
               if (OpSize[1] == -1) OpSize[1] = 0;
               if (OpSize[2] == -1) OpSize[2] = 2;
               if (OpSize[1] != 0) WrError(1130);
               else if (DecodeAdr(ArgStr[1], 1, Mask_Source))
                  if (DecodeAdr(ArgStr[2], 2, Mask_PureDest)) {
                     if (FormatCode == 0) {
                        if (AdrType[1] == ModImm) {
                           HVal = ImmVal(1);
                           if ((IsShort(2)) && (abs(HVal) >= 1) && (abs(HVal) <= 8) && ((z == 0) || (HVal < 0))) FormatCode = 3;
                           else if ((HVal >= -128) && (HVal <= 127)) FormatCode = 2;
                           else FormatCode = 1;
                        } else FormatCode = 1;
                     }
                     switch (FormatCode) {
                        case 1:
                           Make_G(0x3000 + (z << 10));
                           break;
                        case 2:
                           Make_E(0x3000 + (z << 10), true);
                           break;
                        case 3:
                           if ((AdrType[1] != ModImm) || (!IsShort(2))) WrError(1350);
                           else {
                              HVal = ImmVal(1);
                              if (!ChkRange(HVal, -8, (1 - z) << 3)) ;
                              else if (HVal == 0) WrError(1135);
                              else {
                                 if (HVal < 0) HVal += 16;
                                 else HVal &= 7;
                                 WAsmCode[0] = 0x4080 + (HVal << 10) + (z << 6) + (OpSize[2] << 8) + AdrMode[2];
                                 memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
                                 CodeLen = 2 + AdrCnt1[2];
                              }
                           }
                           break;
                     }
                  }
            }
      return true;
   }

   if ((Memo("SHXL")) || (Memo("SHXR"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (CheckFormat("G"))
         if (GetOpSize(ArgStr[1], 1)) {
            if (OpSize[1] == -1) OpSize[1] = 2;
            if (OpSize[1] != 2) WrError(1130);
            else if (DecodeAdr(ArgStr[1], 1, Mask_PureDest)) {
               WAsmCode[0] = 0x02f7;
               WAsmCode[1] = 0x8a00 + (Memo("SHXR") << 12) + AdrMode[1];
               memcpy(WAsmCode + 1, AdrVals, AdrCnt1[1]);
               CodeLen = 4 + AdrCnt1[1];
            }
         }
      return true;
   }

   return false;
}

static bool CodeBits(void) {
   Integer z;
   char Form[6];
   LongInt HVal, AdrLong;
   Word Mask;

   for (z = 0; z < BitOrderCount; z++)
      if (Memo(BitOrders[z].Name)) {
         strcpy(Form, (BitOrders[z].Code2 != 0) ? "GER" : "GE");
         if (ArgCnt != 2) WrError(1110);
         else if (CheckFormat(Form) && GetOpSize(ArgStr[1], 1) && GetOpSize(ArgStr[2], 2)) {
            if (OpSize[1] == -1) OpSize[1] = 2;
            if (DecodeAdr(ArgStr[1], 1, Mask_Source) && DecodeAdr(ArgStr[2], 2, Mask_PureDest)) {
               if (OpSize[2] != -1) ;
               else if ((AdrType[2] == ModReg) && (!BitOrders[z].MustByte)) OpSize[2] = 2;
               else OpSize[2] = 0;
               if (((AdrType[2] != ModReg) || (BitOrders[z].MustByte)) && (OpSize[2] != 0)) WrError(1130);
               else {
                  if (FormatCode == 0) {
                     if (AdrType[1] == ModImm) {
                        HVal = ImmVal(1);
                        if ((HVal >= 0) && (HVal <= 7) && (IsShort(2)) && (BitOrders[z].Code2 != 0) && (OpSize[2] == 0)) FormatCode = 3;
                        else if ((HVal >= -128) && (HVal < 127)) FormatCode = 2;
                        else FormatCode = 1;
                     } else FormatCode = 1;
                  }
                  switch (FormatCode) {
                     case 1:
                        Make_G(BitOrders[z].Code1);
                        break;
                     case 2:
                        Make_E(BitOrders[z].Code1, true);
                        break;
                     case 3:
                        if ((AdrType[1] != ModImm) || (!IsShort(2))) WrError(1350);
                        else if (OpSize[2] != 0) WrError(1130);
                        else {
                           HVal = ImmVal(1);
                           if (ChkRange(HVal, 0, 7)) {
                              WAsmCode[0] = BitOrders[z].Code2 + ((HVal & 7) << 10) + AdrMode[2];
                              memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
                              CodeLen = 2 + AdrCnt1[2];
                           }
                        }
                        break;
                  }
               }
            }
         }
         return true;
      }

   for (z = 0; z < BFieldOrderCount; z++)
      if (Memo(BFieldOrders[z])) {
         if (ArgCnt != 4) WrError(1110);
         else if (CheckBFieldFormat())
            if (GetOpSize(ArgStr[1], 1))
               if (GetOpSize(ArgStr[2], 2))
                  if (GetOpSize(ArgStr[3], 3))
                     if (GetOpSize(ArgStr[4], 4)) {
                        if (OpSize[1] == -1) OpSize[1] = 2;
                        if (OpSize[2] == -1) OpSize[2] = 2;
                        if (OpSize[3] == -1) OpSize[3] = 2;
                        if (OpSize[4] == -1) OpSize[4] = 2;
                        if (DecodeAdr(ArgStr[1], 1, MModReg + MModImm))
                           if (DecodeAdr(ArgStr[3], 3, MModReg + MModImm)) {
                              Mask = (AdrType[3] == ModReg) ? Mask_Source : MModImm;
                              if (DecodeAdr(ArgStr[2], 2, Mask))
                                 if (DecodeAdr(ArgStr[4], 4, Mask_PureMem)) {
                                    if (FormatCode == 0) {
                                       if (AdrType[3] == ModReg)
                                          if (AdrType[1] == ModReg) FormatCode = 1;
                                          else FormatCode = 2;
                                       else if (AdrType[1] == ModReg) FormatCode = 3;
                                       else FormatCode = 4;
                                    }
                                    switch (FormatCode) {
                                       case 1:
                                          if ((AdrType[1] != ModReg) || (AdrType[3] != ModReg)) WrError(1350);
                                          else if ((OpSize[1] != 2) || (OpSize[3] != 2) || (OpSize[4] != 2)) WrError(1130);
                                          else {
                                             WAsmCode[0] = 0xd000 + (OpSize[2] << 8) + AdrMode[2];
                                             memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
                                             WAsmCode[1 + AdrCnt2[2]] = 0xc200 + (z << 10) + AdrMode[4];
                                             memcpy(WAsmCode + 2 + AdrCnt2[2], AdrVals[4], AdrCnt1[4]);
                                             WAsmCode[2 + AdrCnt2[2] + AdrCnt2[4]] = ((AdrMode[3] & 15) << 10) + (AdrMode[1] & 15);
                                             CodeLen = 6 + AdrCnt1[2] + AdrCnt1[4];
                                          }
                                          break;
                                       case 2:
                                          if ((AdrType[1] != ModImm) || (AdrType[3] != ModReg)) WrError(1350);
                                          else if ((OpSize[3] != 2) || (OpSize[4] != 2)) WrError(1130);
                                          else {
                                             WAsmCode[0] = 0xd000 + (OpSize[2] << 8) + AdrMode[2];
                                             memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
                                             WAsmCode[1 + AdrCnt2[2]] = 0xd200 + (z << 10) + AdrMode[4];
                                             memcpy(WAsmCode + 2 + AdrCnt2[2], AdrVals[4], AdrCnt1[4]);
                                             WAsmCode[2 + AdrCnt2[2] + AdrCnt2[4]] = ((AdrMode[3] & 15) << 10) + (OpSize[1] << 8);
                                             CodeLen = 6 + AdrCnt1[2] + AdrCnt1[4];
                                             if (OpSize[1] == 0) WAsmCode[(CodeLen - 2) >> 1] += AdrVals[1][0] & 0xff;
                                             else {
                                                memcpy(WAsmCode + (CodeLen >> 1), AdrVals[1], AdrCnt1[1]);
                                                CodeLen += AdrCnt1[1];
                                             }
                                          }
                                          break;
                                       case 3:
                                          if ((AdrType[1] != ModReg) || (AdrType[2] != ModImm) || (AdrType[3] != ModImm)) WrError(1350);
                                          else if ((OpSize[1] != 2) || (OpSize[4] != 2)) WrError(1130);
                                          else {
                                             HVal = ImmVal(2);
                                             if (ChkRange(HVal, -128, -127)) {
                                                AdrLong = ImmVal(3);
                                                if (ChkRange(AdrLong, 1, 32)) {
                                                   WAsmCode[0] = 0xbf00 + (HVal & 0xff);
                                                   WAsmCode[1] = 0xc200 + (z << 10) + AdrMode[4];
                                                   memcpy(WAsmCode + 2, AdrVals[4], AdrCnt1[4]);
                                                   WAsmCode[2 + AdrCnt2[4]] = ((AdrLong & 31) << 10) + (AdrMode[1] & 15);
                                                   CodeLen = 6 + AdrCnt1[4];
                                                }
                                             }
                                          }
                                          break;
                                       case 4:
                                          if ((AdrType[1] != ModImm) || (AdrType[2] != ModImm) || (AdrType[3] != ModImm)) WrError(1350);
                                          else if (OpSize[4] != 2) WrError(1130);
                                          else {
                                             HVal = ImmVal(2);
                                             if (ChkRange(HVal, -128, -127)) {
                                                AdrLong = ImmVal(3);
                                                if (ChkRange(AdrLong, 1, 32)) {
                                                   WAsmCode[0] = 0xbf00 + (HVal & 0xff);
                                                   WAsmCode[1] = 0xd200 + (z << 10) + AdrMode[4];
                                                   memcpy(WAsmCode + 2, AdrVals[4], AdrCnt1[4]);
                                                   WAsmCode[2 + AdrCnt2[4]] = ((AdrLong & 31) << 10) + (OpSize[1] << 8);
                                                   CodeLen = 6 + AdrCnt1[4];
                                                   if (OpSize[1] == 0) WAsmCode[(CodeLen - 1) >> 1] += AdrVals[1][0] & 0xff;
                                                   else {
                                                      memcpy(WAsmCode + (CodeLen >> 1), AdrVals[1], AdrCnt1[1]);
                                                      CodeLen += AdrCnt1[1];
                                                   }
                                                }
                                             }
                                          }
                                          break;
                                    }
                                 }
                           }
                     }
         return true;
      }

   if ((Memo("BFEXT")) || (Memo("BFEXTU"))) {
      z = Memo("BFEXTU");
      if (ArgCnt != 4) WrError(1110);
      else if (CheckFormat("GE"))
         if (GetOpSize(ArgStr[1], 1))
            if (GetOpSize(ArgStr[2], 2))
               if (GetOpSize(ArgStr[3], 3))
                  if (GetOpSize(ArgStr[4], 4)) {
                     if (OpSize[1] == -1) OpSize[1] = 2;
                     if (OpSize[2] == -1) OpSize[2] = 2;
                     if (OpSize[3] == -1) OpSize[3] = 2;
                     if (OpSize[4] == -1) OpSize[4] = 2;
                     if (DecodeAdr(ArgStr[4], 4, MModReg))
                        if (DecodeAdr(ArgStr[3], 3, Mask_MemGen - MModPop - MModPush))
                           if (DecodeAdr(ArgStr[2], 2, MModReg + MModImm)) {
                              if (AdrType[2] == ModReg) Mask = Mask_Source;
                              else Mask = MModImm;
                              if (DecodeAdr(ArgStr[1], 1, Mask)) {
                                 if (FormatCode == 0) {
                                    if (AdrType[2] == ModReg) FormatCode = 1;
                                    else FormatCode = 2;
                                 }
                                 switch (FormatCode) {
                                    case 1:
                                       if ((OpSize[2] != 2) || (OpSize[3] != 2) || (OpSize[4] != 2)) WrError(1130);
                                       else {
                                          WAsmCode[0] = 0xd000 + (OpSize[1] << 8) + AdrMode[1];
                                          memcpy(WAsmCode + 1, AdrVals[1], AdrCnt1[1]);
                                          WAsmCode[1 + AdrCnt2[1]] = 0xea00 + (z << 10) + AdrMode[3];
                                          memcpy(WAsmCode + 2 + AdrCnt2[1], AdrVals[3], AdrCnt1[3]);
                                          WAsmCode[2 + AdrCnt2[1] + AdrCnt2[3]] = ((AdrMode[2] & 15) << 10) + (AdrMode[4] & 15);
                                          CodeLen = 6 + AdrCnt1[1] + AdrCnt1[3];
                                       }
                                       break;
                                    case 2:
                                       if ((AdrType[1] != ModImm) || (AdrType[2] != ModImm)) WrError(1350);
                                       else if ((OpSize[3] != 2) || (OpSize[4] != 2)) WrError(1350);
                                       else {
                                          HVal = ImmVal(1);
                                          if (ChkRange(HVal, -128, 127)) {
                                             AdrLong = ImmVal(2);
                                             if (ChkRange(AdrLong, 1, 32)) {
                                                WAsmCode[0] = 0xbf00 + (HVal & 0xff);
                                                WAsmCode[1] = 0xea00 + (z << 10) + AdrMode[3];
                                                memcpy(WAsmCode + 2, AdrVals[3], AdrCnt1[3]);
                                                WAsmCode[2 + AdrCnt2[3]] = ((AdrLong & 31) << 10) + (AdrMode[4] & 15);
                                                CodeLen = 6 + AdrCnt1[3];
                                             }
                                          }
                                       }
                                       break;
                                 }
                              }
                           }
                  }
      return true;
   }

   if ((Memo("BSCH/0")) || (Memo("BSCH/1"))) {
      z = OpPart[5] - '0';
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("G"))
         if (GetOpSize(ArgStr[1], 1))
            if (GetOpSize(ArgStr[2], 2)) {
               if (OpSize[1] == -1) OpSize[1] = 2;
               if (OpSize[2] == -1) OpSize[2] = 2;
               if (OpSize[1] != 2) WrError(1130);
               else if (DecodeAdr(ArgStr[1], 1, Mask_Source))
                  if (DecodeAdr(ArgStr[2], 2, Mask_PureDest)) {
                  /* immer G-Format */
                     WAsmCode[0] = 0xd600 + AdrMode[1];
                     memcpy(WAsmCode + 1, AdrVals[1], AdrCnt1[1]);
                     WAsmCode[1 + AdrCnt2[1]] = 0x5000 + (z << 10) + (OpSize[2] << 8) + AdrMode[2];
                     memcpy(WAsmCode + 2 + AdrCnt2[1], AdrVals[2], AdrCnt1[2]);
                     CodeLen = 4 + AdrCnt1[1] + AdrCnt1[2];
                  }
            }
      return true;
   }

   return false;
}

static void MakeCode_M16(void) {
   Integer z;
   char *p;
   Word AdrWord, HReg, Mask;
   LongInt AdrLong, HVal;
   bool OK;

   DOpSize = (-1);
   for (z = 1; z <= ArgCnt; OpSize[z++] = (-1));

/* zu ignorierendes */

   if (Memo("")) return;

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
         if (p == NULL) {
            strmaxcpy(Format, AttrPart, 255);
            *AttrPart = '\0';
         } else {
            *p = '\0';
            if (p == AttrPart) strcpy(Format, " ");
            else strmaxcpy(Format, AttrPart, 255);
         }
         break;
      default:
         strcpy(Format, " ");
   }
   NLS_UpString(Format);

/* Attribut abarbeiten */

   if (*AttrPart == '\0') DOpSize = (-1);
   else
      switch (toupper(*AttrPart)) {
         case 'B':
            DOpSize = 0;
            break;
         case 'H':
            DOpSize = 1;
            break;
         case 'W':
            DOpSize = 2;
            break;
         default:
            WrError(1107);
            return;
      }

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(false)) return;

/* ohne Argument */

   for (z = 0; z < FixedOrderCount; z++)
      if (Memo(FixedOrders[z].Name)) {
         if (ArgCnt != 0) WrError(1110);
         else if (*AttrPart != '\0') WrError(1100);
         else if (strcmp(Format, " ") != 0) WrError(1090);
         else {
            CodeLen = 2;
            WAsmCode[0] = FixedOrders[z].Code;
         }
         return;
      }

   if ((Memo("STOP")) || (Memo("SLEEP"))) {
      if (ArgCnt != 0) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else {
         CodeLen = 10;
         WAsmCode[0] = 0xd20c;
         if (Memo("STOP")) {
            WAsmCode[1] = 0x5374;
            WAsmCode[2] = 0x6f70;
         } else {
            WAsmCode[1] = 0x5761;
            WAsmCode[2] = 0x6974;
         }
         WAsmCode[3] = 0x9e09;
         WAsmCode[4] = 0x0700;
      }
      return;
   }

/* Datentransfer */

   if (Memo("MOV")) {
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("GELSZQI"))
         if (GetOpSize(ArgStr[1], 1))
            if (GetOpSize(ArgStr[2], 2)) {
               if (OpSize[2] == -1) OpSize[2] = 2;
               if (OpSize[1] == -1) OpSize[1] = OpSize[2];
               if (DecodeAdr(ArgStr[1], 1, Mask_Source))
                  if (DecodeAdr(ArgStr[2], 2, Mask_AllGen - MModPop)) {
                     if (FormatCode == 0) {
                        if (AdrType[1] == ModImm) {
                           HVal = ImmVal(1);
                           if (HVal == 0) FormatCode = 5;
                           else if ((HVal >= 1) && (HVal <= 8) && (IsShort(2))) FormatCode = 6;
                           else if ((HVal >= -128) && (HVal <= 127)) FormatCode = 2;
                           else if (IsShort(2)) FormatCode = 7;
                           else FormatCode = 1;
                        } else if ((AdrType[1] == ModReg) && (OpSize[1] == 2) && (IsShort(2))) FormatCode = 4;
                        else if ((AdrType[2] == ModReg) && (OpSize[2] == 2) && (IsShort(1))) FormatCode = 3;
                        else FormatCode = 1;
                     }
                     switch (FormatCode) {
                        case 1:
                           Make_G(0x8800);
                           break;
                        case 2:
                           Make_E(0x8800, true);
                           break;
                        case 3:
                           if ((!IsShort(1)) || (AdrType[2] != ModReg)) WrError(1350);
                           else if (OpSize[2] != 2) WrError(1130);
                           else {
                              WAsmCode[0] = 0x0040 + ((AdrMode[2] & 15) << 10) + (OpSize[1] << 8) + AdrMode[1];
                              memcpy(WAsmCode + 1, AdrVals[1], AdrCnt1[1]);
                              CodeLen = 2 + AdrCnt1[1];
                           }
                           break;
                        case 4:
                           if ((!IsShort(2)) || (AdrType[1] != ModReg)) WrError(1350);
                           else if (OpSize[1] != 2) WrError(1130);
                           else {
                              WAsmCode[0] = 0x0080 + ((AdrMode[1] & 15) << 10) + (OpSize[2] << 8) + AdrMode[2];
                              memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
                              CodeLen = 2 + AdrCnt1[2];
                           }
                           break;
                        case 5:
                           if (AdrType[1] != ModImm) WrError(1350);
                           else {
                              HVal = ImmVal(1);
                              if (ChkRange(HVal, 0, 0)) {
                                 WAsmCode[0] = 0xc400 + (OpSize[2] << 8) + AdrMode[2];
                                 memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
                                 CodeLen = 2 + AdrCnt1[2];
                              }
                           }
                           break;
                        case 6:
                           if ((AdrType[1] != ModImm) || (!IsShort(2))) WrError(1350);
                           else {
                              HVal = ImmVal(1);
                              if (ChkRange(HVal, 1, 8)) {
                                 WAsmCode[0] = 0x6000 + ((HVal & 7) << 10) + (OpSize[2] << 8) + AdrMode[2];
                                 memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
                                 CodeLen = 2 + AdrCnt1[2];
                              }
                           }
                           break;
                        case 7:
                           Make_I(0x48c0, true);
                           break;
                     }
                  }
            }
      return;
   }

/* ein Operand */

   for (z = 0; z < OneOrderCount; z++)
      if (Memo(OneOrders[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else if (CheckFormat("G"))
            if (GetOpSize(ArgStr[1], 1)) {
               if ((OpSize[1] == -1) && (OneOrders[z].OpMask != 0)) OpSize[1] = DefSize(OneOrders[z].OpMask);
               if ((OpSize[1] != -1) && (((1 << OpSize[1]) & OneOrders[z].OpMask) == 0)) WrError(1130);
               else {
                  if (DecodeAdr(ArgStr[1], 1, OneOrders[z].Mask)) {
                  /* da nur G, Format ignorieren */
                     WAsmCode[0] = OneOrders[z].Code + AdrMode[1];
                     if (OneOrders[z].OpMask != 0) WAsmCode[0] += OpSize[1] << 8;
                     memcpy(WAsmCode + 1, AdrVals[1], AdrCnt1[1]);
                     CodeLen = 2 + AdrCnt1[1];
                  }
               }
            }
         return;
      }

/* zwei Operanden */

   if (CodeAri()) return;

/* drei Operanden */

   if ((Memo("CHK/N")) || (Memo("CHK/S")) || (Memo("CHK"))) {
      z = OpPart[strlen(OpPart) - 1] == 'S';
      if (ArgCnt != 3) WrError(1110);
      else if (CheckFormat("G"))
         if (GetOpSize(ArgStr[1], 2))
            if (GetOpSize(ArgStr[2], 1))
               if (GetOpSize(ArgStr[3], 3)) {
                  if (OpSize[3] == -1) OpSize[3] = 2;
                  if (OpSize[2] == -1) OpSize[2] = OpSize[3];
                  if (OpSize[1] == -1) OpSize[1] = OpSize[3];
                  if ((OpSize[1] != OpSize[2]) || (OpSize[2] != OpSize[3])) WrError(1131);
                  else if (DecodeAdr(ArgStr[1], 2, Mask_MemGen - MModPop - MModPush))
                     if (DecodeAdr(ArgStr[2], 1, Mask_Source))
                        if (DecodeAdr(ArgStr[3], 3, MModReg)) {
                           OpSize[2] = 2 + z;
                           Make_G((AdrMode[3] & 15) << 10);
                           WAsmCode[0] += 0x400;
                        }
               }
      return;
   }

   if (Memo("CSI")) {
      if (ArgCnt != 3) WrError(1110);
      else if (CheckFormat("G"))
         if (GetOpSize(ArgStr[1], 3))
            if (GetOpSize(ArgStr[2], 1))
               if (GetOpSize(ArgStr[3], 2)) {
                  if (OpSize[3] == -1) OpSize[3] = 2;
                  if (OpSize[2] == -1) OpSize[2] = OpSize[3];
                  if (OpSize[1] == -1) OpSize[1] = OpSize[2];
                  if ((OpSize[1] != OpSize[2]) || (OpSize[2] != OpSize[3])) WrError(1131);
                  else if (DecodeAdr(ArgStr[1], 3, MModReg))
                     if (DecodeAdr(ArgStr[2], 1, Mask_Source))
                        if (DecodeAdr(ArgStr[3], 2, Mask_PureMem)) {
                           OpSize[2] = 0;
                           Make_G((AdrMode[3] & 15) << 10);
                           WAsmCode[0] += 0x400;
                        }
               }
      return;
   }

   if ((Memo("DIVX")) || (Memo("MULX"))) {
      z = Memo("DIVX");
      if (ArgCnt != 3) WrError(1110);
      else if (CheckFormat("G"))
         if (GetOpSize(ArgStr[1], 1))
            if (GetOpSize(ArgStr[2], 2))
               if (GetOpSize(ArgStr[3], 3)) {
                  if (OpSize[3] == -1) OpSize[3] = 2;
                  if (OpSize[2] == -1) OpSize[2] = OpSize[3];
                  if (OpSize[1] == -1) OpSize[1] = OpSize[2];
                  if ((OpSize[1] != 2) || (OpSize[2] != 2) || (OpSize[3] != 2)) WrError(1130);
                  else if (DecodeAdr(ArgStr[1], 1, Mask_Source))
                     if (DecodeAdr(ArgStr[2], 2, Mask_PureDest))
                        if (DecodeAdr(ArgStr[3], 3, MModReg)) {
                           OpSize[2] = 0;
                           Make_G(0x8200 + ((AdrMode[3] & 15) << 10) + (z << 8));
                           WAsmCode[0] += 0x400;
                        }
               }
      return;
   }

/* Bitoperationen */

   if (CodeBits()) return;

/* Spruenge */

   if ((Memo("BSR")) || (Memo("BRA"))) {
      z = Memo("BSR");
      if (ArgCnt != 1) WrError(1110);
      else if (CheckFormat("GD"))
         if (GetOpSize(ArgStr[1], 1)) {
            AdrLong = EvalIntExpression(ArgStr[1], Int32, &OK);
            if (OK) {
               DecideBranch(AdrLong, 1);
               switch (FormatCode) {
                  case 2:
                     if (OpSize[1] != -1) WrError(1100);
                     else {
                        AdrLong -= EProgCounter();
                        if ((!SymbolQuestionable) && ((AdrLong < -256) || (AdrLong > 254))) WrError(1370);
                        else if (Odd(AdrLong)) WrError(1375);
                        else {
                           CodeLen = 2;
                           WAsmCode[0] = 0xae00 + (z << 8) + Lo(AdrLong >> 1);
                        }
                     }
                     break;
                  case 1:
                     WAsmCode[0] = 0x20f7 + (z << 11) + (((Word) OpSize[1]) << 8);
                     AdrLong -= EProgCounter();
                     switch (OpSize[1]) {
                        case 0:
                           if ((!SymbolQuestionable) && ((AdrLong < -128) || (AdrLong > 127))) WrError(1370);
                           else {
                              CodeLen = 4;
                              WAsmCode[1] = Lo(AdrLong);
                           }
                           break;
                        case 1:
                           if ((!SymbolQuestionable) && ((AdrLong < -32768) || (AdrLong > 32767))) WrError(1370);
                           else {
                              CodeLen = 4;
                              WAsmCode[1] = AdrLong & 0xffff;
                           }
                           break;
                        case 2:
                           CodeLen = 6;
                           WAsmCode[1] = AdrLong >> 16;
                           WAsmCode[2] = AdrLong & 0xffff;
                           break;
                     }
                     break;
               }
            }
         }
      return;
   }

   if (*OpPart == 'B')
      for (z = 0; z < ConditionCount; z++)
         if (strcmp(OpPart + 1, Conditions[z]) == 0) {
            if (ArgCnt != 1) WrError(1110);
            else if (CheckFormat("GD"))
               if (GetOpSize(ArgStr[1], 1)) {
                  AdrLong = EvalIntExpression(ArgStr[1], Int32, &OK);
                  if (OK) {
                     DecideBranch(AdrLong, 1);
                     switch (FormatCode) {
                        case 2:
                           if (OpSize[1] != -1) WrError(1100);
                           else {
                              AdrLong -= EProgCounter();
                              if ((!SymbolQuestionable) && ((AdrLong < -256) || (AdrLong > 254))) WrError(1370);
                              else if (Odd(AdrLong)) WrError(1375);
                              else {
                                 CodeLen = 2;
                                 WAsmCode[0] = 0x8000 + (z << 10) + Lo(AdrLong >> 1);
                              }
                           }
                           break;
                        case 1:
                           WAsmCode[0] = 0x00f6 + (z << 10) + (((Word) OpSize[1]) << 8);
                           AdrLong -= EProgCounter();
                           switch (OpSize[1]) {
                              case 0:
                                 if ((AdrLong < -128) || (AdrLong > 127)) WrError(1370);
                                 else {
                                    CodeLen = 4;
                                    WAsmCode[1] = Lo(AdrLong);
                                 }
                                 break;
                              case 1:
                                 if ((AdrLong < -32768) || (AdrLong > 32767)) WrError(1370);
                                 else {
                                    CodeLen = 4;
                                    WAsmCode[1] = AdrLong & 0xffff;
                                 }
                                 break;
                              case 2:
                                 CodeLen = 6;
                                 WAsmCode[1] = AdrLong >> 16;
                                 WAsmCode[2] = AdrLong & 0xffff;
                                 break;
                           }
                           break;
                     }
                  }
               }
            return;
         }

   if ((Memo("ACB")) || (Memo("SCB"))) {
      AdrWord = Memo("SCB");
      if (ArgCnt != 4) WrError(1110);
      else if (CheckFormat("GEQR"))
         if (GetOpSize(ArgStr[2], 3))
            if (GetOpSize(ArgStr[4], 4))
               if (GetOpSize(ArgStr[1], 1))
                  if (GetOpSize(ArgStr[3], 2)) {
                     if ((OpSize[3] == -1) && (OpSize[2] == -1)) OpSize[3] = 2;
                     if ((OpSize[3] == -1) && (OpSize[2] != -1)) OpSize[3] = OpSize[2];
                     else if ((OpSize[3] != -1) && (OpSize[2] == -1)) OpSize[2] = OpSize[3];
                     if (OpSize[1] == -1) OpSize[1] = OpSize[2];
                     if (OpSize[3] != OpSize[2]) WrError(1131);
                     else if (!DecodeReg(ArgStr[2], &HReg)) WrError(1350);
                     else {
                        AdrLong = EvalIntExpression(ArgStr[4], Int32, &OK);
                        if (OK) {
                           if (DecodeAdr(ArgStr[1], 1, Mask_Source))
                              if (DecodeAdr(ArgStr[3], 2, Mask_Source)) {
                                 if (FormatCode == 0) {
                                    if (AdrType[1] != ModImm) FormatCode = 1;
                                    else {
                                       HVal = ImmVal(1);
                                       if ((HVal == 1) && (AdrType[2] == ModReg)) FormatCode = 4;
                                       else if ((HVal == 1) && (AdrType[2] == ModImm)) {
                                          HVal = ImmVal(2);
                                          if ((HVal >= 1 - AdrWord) && (HVal <= 64 - AdrWord)) FormatCode = 3;
                                          else FormatCode = 2;
                                       } else if ((HVal >= -128) && (HVal <= 127)) FormatCode = 2;
                                       else FormatCode = 1;
                                    }
                                 }
                                 switch (FormatCode) {
                                    case 1:
                                       if (DecideBranchLength(&AdrLong, 4)) { /* ??? */
                                          WAsmCode[0] = 0xd000 + (OpSize[1] << 8) + AdrMode[1];
                                          memcpy(WAsmCode + 1, AdrVals[1], AdrCnt1[1]);
                                          WAsmCode[1 + AdrCnt2[1]] = 0xf000 + (AdrWord << 11) + (OpSize[2] << 8) + AdrMode[2];
                                          memcpy(WAsmCode + 2 + AdrCnt2[1], AdrVals[2], AdrCnt1[2]);
                                          WAsmCode[2 + AdrCnt2[1] + AdrCnt2[2]] = (HReg << 10) + (OpSize[4] << 8);
                                          CodeLen = 6 + AdrCnt1[1] + AdrCnt1[2];
                                       }
                                       break;
                                    case 2:
                                       if (!DecideBranchLength(&AdrLong, 4)) ; /* ??? */
                                       else if (AdrType[1] != ModImm) WrError(1350);
                                       else {
                                          HVal = ImmVal(1);
                                          if (ChkRange(HVal, -128, 127)) {
                                             WAsmCode[0] = 0xbf00 + (HVal & 0xff);
                                             WAsmCode[1] = 0xf000 + (AdrWord << 11) + (OpSize[2] << 8) + AdrMode[2];
                                             memcpy(WAsmCode + 2, AdrVals[2], AdrCnt1[2]);
                                             WAsmCode[2 + AdrCnt2[2]] = (HReg << 10) + (OpSize[4] << 8);
                                             CodeLen = 6 + AdrCnt1[2];
                                          }
                                       }
                                       break;
                                    case 3:
                                       if (!DecideBranchLength(&AdrLong, 4)) ; /* ??? */
                                       else if (AdrType[1] != ModImm) WrError(1350);
                                       else if (ImmVal(1) != 1) WrError(1135);
                                       else if (AdrType[2] != ModImm) WrError(1350);
                                       else {
                                          HVal = ImmVal(2);
                                          if (ChkRange(HVal, 1 - AdrWord, 64 - AdrWord)) {
                                             WAsmCode[0] = 0x03d1 + (HReg << 10) + (AdrWord << 1);
                                             WAsmCode[1] = ((HVal & 0x3f) << 10) + (OpSize[4] << 8);
                                             CodeLen = 4;
                                          }
                                       }
                                       break;
                                    case 4:
                                       if (!DecideBranchLength(&AdrLong, 4)) ; /* ??? */
                                       else if (AdrType[1] != ModImm) WrError(1350);
                                       else if (ImmVal(1) != 1) WrError(1135);
                                       else if (OpSize[2] != 2) WrError(1130);
                                       else if (AdrType[2] != ModReg) WrError(1350);
                                       else {
                                          WAsmCode[0] = 0x03d0 + (HReg << 10) + (AdrWord << 1);
                                          WAsmCode[1] = ((AdrMode[2] & 15) << 10) + (OpSize[4] << 8);
                                          CodeLen = 4;
                                       }
                                       break;
                                 }
                                 if (CodeLen > 0)
                                    switch (OpSize[4]) {
                                       case 0:
                                          WAsmCode[(CodeLen >> 1) - 1] += AdrLong & 0xff;
                                          break;
                                       case 1:
                                          WAsmCode[CodeLen >> 1] = AdrLong & 0xffff;
                                          CodeLen += 2;
                                          break;
                                       case 2:
                                          WAsmCode[CodeLen >> 1] = AdrLong >> 16;
                                          WAsmCode[(CodeLen >> 1) + 1] = AdrLong & 0xffff;
                                          CodeLen += 4;
                                          break;
                                    }
                              }
                        }
                     }
                  }
      return;
   }

   if (Memo("TRAPA")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else if (*ArgStr[1] != '#') WrError(1350);
      else {
         AdrWord = EvalIntExpression(ArgStr[1] + 1, UInt4, &OK);
         if (OK) {
            CodeLen = 2;
            WAsmCode[0] = 0x03d5 + (AdrWord << 10);
         }
      }
      return;
   }

   if (strncmp(OpPart, "TRAP", 4) == 0) {
      if (ArgCnt != 0) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else {
         SplitOptions();
         if (OptionCnt != 1) WrError(1115);
         else if (!DecodeCondition(Options[0], &AdrWord)) WrError(1360);
         else {
            CodeLen = 2;
            WAsmCode[0] = 0x03d4 + (AdrWord << 10);
         }
      }
      return;
   }

/* Specials */

   if ((Memo("ENTER")) || (Memo("EXITD"))) {
      if (Memo("EXITD")) {
         z = 1;
         strcopy(ArgStr[3], ArgStr[1]);
         strcopy(ArgStr[1], ArgStr[2]);
         strcopy(ArgStr[2], ArgStr[3]);
      } else z = 0;
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("GE") && GetOpSize(ArgStr[1], 1) && GetOpSize(ArgStr[2], 2)) {
         if (OpSize[1] == -1) OpSize[1] = 2;
         if (OpSize[2] == -1) OpSize[2] = 2;
         if (OpSize[2] != 2) WrError(1130);
         else if (!DecodeAdr(ArgStr[1], 1, MModReg + MModImm)) ;
         else if (!DecodeRegList(ArgStr[2], &AdrWord, z == 1)) ;
         else if ((z & 0xc000) != 0) WrXError(1410, "SP/FP");
         else {
            if (FormatCode == 0) {
               if (AdrType[1] == ModImm) {
                  HVal = ImmVal(1);
                  if ((HVal >= -128) && (HVal <= 127)) FormatCode = 2;
                  else FormatCode = 1;
               } else FormatCode = 1;
            }
            switch (FormatCode) {
               case 1:
                  WAsmCode[0] = 0x02f7;
                  WAsmCode[1] = 0x8c00 + (z << 12) + (OpSize[1] << 8) + AdrMode[1];
                  memcpy(WAsmCode + 2, AdrVals[1], AdrCnt1[1]);
                  WAsmCode[2 + AdrCnt2[1]] = AdrWord;
                  CodeLen = 6 + AdrCnt1[1];
                  break;
               case 2:
                  if (AdrType[1] != ModImm) WrError(1350);
                  else {
                     HVal = ImmVal(1);
                     if (ChkRange(HVal, -128, 127)) {
                        WAsmCode[0] = 0x8e00 + (z << 12) + (HVal & 0xff);
                        WAsmCode[1] = AdrWord;
                        CodeLen = 4;
                     }
                  }
                  break;
            }
         }
      }
      return;
   }

   if (strncmp(OpPart, "SCMP", 4) == 0) {
      if (DOpSize == -1) DOpSize = 2;
      if (ArgCnt != 0) WrError(1110);
      else {
         SplitOptions();
         if (OptionCnt > 1) WrError(1115);
         else {
            OK = true;
            if (OptionCnt == 0) AdrWord = 6;
            else if (strcasecmp(Options[0], "LTU") == 0) AdrWord = 0;
            else if (strcasecmp(Options[0], "GEU") == 0) AdrWord = 1;
            else OK = (DecodeCondition(Options[0], &AdrWord) && (AdrWord > 1) && (AdrWord < 6));
            if (!OK) WrXError(1360, Options[0]);
            else {
               WAsmCode[0] = 0x00e0 + (DOpSize << 8) + (AdrWord << 10);
               CodeLen = 2;
            }
         }
      }
      return;
   }

   if ((strncmp(OpPart, "SMOV", 4) == 0) || (strncmp(OpPart, "SSCH", 4) == 0)) {
      if (DOpSize == -1) DOpSize = 2;
      z = (OpPart[1] == 'S') << 4;
      if (ArgCnt != 0) WrError(1110);
      else {
         SplitOptions();
         if (strcasecmp(Options[0], "F") == 0) {
            Mask = 0;
            strcopy(Options[0], Options[1]);
            OptionCnt--;
         } else if (strcasecmp(Options[0], "B") == 0) {
            Mask = 1;
            strcopy(Options[0], Options[1]);
            OptionCnt--;
         } else if (strcasecmp(Options[1], "F") == 0) {
            Mask = 0;
            OptionCnt--;
         } else if (strcasecmp(Options[1], "B") == 0) {
            Mask = 1;
            OptionCnt--;
         } else Mask = 0;
         if (OptionCnt > 1) WrError(1115);
         else {
            OK = true;
            if (OptionCnt == 0) AdrWord = 6;
            else if (strcasecmp(Options[0], "LTU") == 0) AdrWord = 0;
            else if (strcasecmp(Options[0], "GEU") == 0) AdrWord = 1;
            else OK = (DecodeCondition(Options[0], &AdrWord)) && (AdrWord > 1) && (AdrWord < 6);
            if (!OK) WrXError(1360, Options[0]);
            else {
               WAsmCode[0] = 0x00e4 + (DOpSize << 8) + (AdrWord << 10) + Mask + z;
               CodeLen = 2;
            }
         }
      }
      return;
   }

   if (Memo("SSTR")) {
      if (DOpSize == -1) DOpSize = 2;
      if (ArgCnt != 0) WrError(1110);
      else {
         WAsmCode[0] = 0x24f7 + (DOpSize << 8);
         CodeLen = 2;
      }
      return;
   }

   if ((Memo("LDM")) || (Memo("STM"))) {
      Mask = MModIReg + MModDisp16 + MModDisp32 + MModAbs16 + MModAbs32 + MModPCRel16 + MModPCRel32;
      if (Memo("LDM")) {
         z = 0x1000;
         Mask += MModPop;
         strcopy(ArgStr[3], ArgStr[1]);
         strcopy(ArgStr[1], ArgStr[2]);
         strcopy(ArgStr[2], ArgStr[3]);
      } else {
         z = 0;
         Mask += MModPush;
      }
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("G"))
         if (GetOpSize(ArgStr[1], 1))
            if (GetOpSize(ArgStr[2], 2)) {
               if (OpSize[1] == -1) OpSize[1] = 2;
               if (OpSize[2] == -1) OpSize[2] = 2;
               if ((OpSize[1] != 2) || (OpSize[2] != 2)) WrError(1130);
               else if (DecodeAdr(ArgStr[2], 2, Mask))
                  if (DecodeRegList(ArgStr[1], &AdrWord, AdrType[2] != ModPush)) {
                     WAsmCode[0] = 0x8a00 + z + AdrMode[2];
                     memcpy(WAsmCode + 1, AdrVals[2], AdrCnt1[2]);
                     WAsmCode[1 + AdrCnt2[2]] = AdrWord;
                     CodeLen = 4 + AdrCnt1[2];
                  }
            }
      return;
   }

   if ((Memo("STC")) || (Memo("STP"))) {
      z = Memo("STP") << 10;
      if (ArgCnt != 2) WrError(1110);
      else if (CheckFormat("G"))
         if (GetOpSize(ArgStr[1], 1))
            if (GetOpSize(ArgStr[2], 2)) {
               if (OpSize[2] == -1) OpSize[2] = 2;
               if (OpSize[1] == -1) OpSize[1] = OpSize[1];
               if (OpSize[1] != OpSize[2]) WrError(1132);
               else if ((z == 0) && (OpSize[2] != 2)) WrError(1130);
               else if (DecodeAdr(ArgStr[1], 1, Mask_PureMem))
                  if (DecodeAdr(ArgStr[2], 2, Mask_Dest)) {
                     OpSize[1] = 0;
                     Make_G(0xa800 + z);
                     WAsmCode[0] += 0x800;
                  }
            }
      return;
   }

   if (Memo("WAIT")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1100);
      else if (strcmp(Format, " ") != 0) WrError(1090);
      else if (*ArgStr[1] != '#') WrError(1350);
      else {
         WAsmCode[1] = EvalIntExpression(ArgStr[1] + 1, UInt3, &OK);
         if (OK) {
            WAsmCode[0] = 0x0fd6;
            CodeLen = 4;
         }
      }
      return;
   }

   if (Memo("JRNG")) {
      if (ArgCnt != 1) WrError(1110);
      else if (CheckFormat("GE"))
         if (GetOpSize(ArgStr[1], 1)) {
            if (OpSize[1] == -1) OpSize[1] = 1;
            if (OpSize[1] != 1) WrError(1130);
            else if (DecodeAdr(ArgStr[1], 1, MModReg + MModImm)) {
               if (FormatCode == 0) {
                  if (AdrType[1] == ModImm) {
                     HVal = ImmVal(1);
                     if ((HVal >= 0) && (HVal <= 255)) FormatCode = 2;
                     else FormatCode = 1;
                  } else FormatCode = 1;
               }
               switch (FormatCode) {
                  case 1:
                     WAsmCode[0] = 0xba00 + AdrMode[1];
                     memcpy(WAsmCode + 1, AdrVals[1], AdrCnt1[1]);
                     CodeLen = 2 + AdrCnt1[1];
                     break;
                  case 2:
                     if (AdrType[1] != ModImm) WrError(1350);
                     else {
                        HVal = ImmVal(1);
                        if (ChkRange(HVal, 0, 255)) {
                           WAsmCode[0] = 0xbe00 + (HVal & 0xff);
                           CodeLen = 2;
                        }
                     }
                     break;
               }
            }
         }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_M16(void) {
#ifdef HAS64
   return ((ActPC == SegCode) && (ProgCounter() <= 0xffffffffll));
#else
   return (ActPC == SegCode);
#endif
}

static bool IsDef_M16(void) {
   return false;
}

static void SwitchFrom_M16(void) {
   DeinitFields();
}

static void SwitchTo_M16(void) {
   TurnWords = true;
   ConstMode = ConstModeIntel;
   SetIsOccupied = false;

   PCSymbol = "$";
   HeaderID = 0x13;
   NOPCode = 0x1bd6;
   DivideChars = ",";
   HasAttrs = true;
   AttrChars = ".:";

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 2;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_M16;
   ChkPC = ChkPC_M16;
   IsDef = IsDef_M16;
   SwitchFrom = SwitchFrom_M16;
   InitFields();
}

void codem16_init(void) {
   CPUM16 = AddCPU("M16", SwitchTo_M16);
}
