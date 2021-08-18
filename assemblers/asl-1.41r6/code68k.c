// AS-Portierung
// Codegenerator 680x0-Familie
#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "endian.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct {
   char *Name;
   Word Code;
   CPUVar FirstCPU;
} CtReg;

typedef struct {
   char *Name;
   Byte Code;
   bool Dya;
} FPUOp;

typedef struct {
   char *Name;
   Byte Code;
} FPUCond;

#define CtRegCnt 8
#define CondCnt 20
#define FPUOpCnt 35
#define FPUCondCnt 26
#define PMMUCondCnt 16
#define PMMURegCnt 13

#define PMMUAvailName  "HASPMMU" /* PMMU-Befehle erlaubt */
#define FullPMMUName   "FULLPMMU" /* voller PMMU-Befehlssatz */

#define Mdata 1 /* Adressierungsmasken */
#define Madr 2
#define Madri 4
#define Mpost 8
#define Mpre 16
#define Mdadri 32
#define Maix 64
#define Mpc 128
#define Mpcidx 256
#define Mabs 512
#define Mimm 1024
#define Mfpn 2048
#define Mfpcr 4096

static Byte OpSize;
static ShortInt RelPos;
static bool PMMUAvail; /* PMMU-Befehle erlaubt? */
static bool FullPMMU; /* voller PMMU-Befehlssatz? */
static Byte AdrNum; /* Adressierungsnummer */
static Word AdrMode; /* Adressierungsmodus */
static Word AdrVals[10]; /* die Worte selber */

static CtReg *CtRegs;
static char **CondNams;
static Byte *CondVals;
static FPUOp *FPUOps;
static FPUCond *FPUConds;
static char **PMMUConds;
static char **PMMURegNames;
static Byte *PMMURegSizes;
static Word *PMMURegCodes;

static SimpProc SaveInitProc;
static CPUVar CPU68008, CPU68000, CPU68010, CPU68012, CPU68332, CPU68340, CPU68360, CPU68020, CPU68030;

static Word Masks[14] = { 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };

/* Codetabellenverwaltung : -----------------------------------------------*/

static void AddCtReg(char *NName, Word NCode, CPUVar NCPU) {
   if (InstrZ >= CtRegCnt) exit(255);
   CtRegs[InstrZ].Name = NName;
   CtRegs[InstrZ].Code = NCode;
   CtRegs[InstrZ++].FirstCPU = NCPU;
}

static void AddCond(char *NName, Byte NCode) {
   if (InstrZ >= CondCnt) exit(255);
   CondNams[InstrZ] = NName;
   CondVals[InstrZ++] = NCode;
}

static void AddFPUOp(char *NName, Byte NCode, bool NDya) {
   if (InstrZ >= FPUOpCnt) exit(255);
   FPUOps[InstrZ].Name = NName;
   FPUOps[InstrZ].Code = NCode;
   FPUOps[InstrZ++].Dya = NDya;
}

static void AddFPUCond(char *NName, Byte NCode) {
   if (InstrZ >= FPUCondCnt) exit(255);
   FPUConds[InstrZ].Name = NName;
   FPUConds[InstrZ++].Code = NCode;
}

static void AddPMMUCond(char *NName) {
   if (InstrZ >= PMMUCondCnt) exit(255);
   PMMUConds[InstrZ++] = NName;
}

static void AddPMMUReg(char *Name, Byte Size, Word Code) {
   if (InstrZ >= PMMURegCnt) exit(255);
   PMMURegNames[InstrZ] = Name;
   PMMURegSizes[InstrZ] = Size;
   PMMURegCodes[InstrZ++] = Code;
}

static void InitFields(void) {
   CtRegs = (CtReg *) malloc(sizeof(CtReg) * CtRegCnt);
   InstrZ = 0;
   AddCtReg("SFC", 0x000, CPU68010);
   AddCtReg("DFC", 0x001, CPU68010);
   AddCtReg("CACR", 0x002, CPU68020);
   AddCtReg("USP", 0x800, CPU68010);
   AddCtReg("VBR", 0x801, CPU68010);
   AddCtReg("CAAR", 0x802, CPU68020);
   AddCtReg("MSP", 0x803, CPU68020);
   AddCtReg("ISP", 0x804, CPU68020);

   CondNams = (char **)malloc(sizeof(char *) * CondCnt);
   CondVals = (Byte *) malloc(sizeof(Byte) * CondCnt);
   InstrZ = 0;
   AddCond("T", 0);
   AddCond("F", 1);
   AddCond("HI", 2);
   AddCond("LS", 3);
   AddCond("CC", 4);
   AddCond("CS", 5);
   AddCond("NE", 6);
   AddCond("EQ", 7);
   AddCond("VC", 8);
   AddCond("VS", 9);
   AddCond("PL", 10);
   AddCond("MI", 11);
   AddCond("GE", 12);
   AddCond("LT", 13);
   AddCond("GT", 14);
   AddCond("LE", 15);
   AddCond("HS", 4);
   AddCond("LO", 5);
   AddCond("RA", 0);
   AddCond("SR", 1);

   FPUOps = (FPUOp *) malloc(sizeof(FPUOp) * FPUOpCnt);
   InstrZ = 0;
   AddFPUOp("INT", 0x01, false);
   AddFPUOp("SINH", 0x02, false);
   AddFPUOp("INTRZ", 0x03, false);
   AddFPUOp("SQRT", 0x04, false);
   AddFPUOp("LOGNP1", 0x06, false);
   AddFPUOp("ETOXM1", 0x08, false);
   AddFPUOp("TANH", 0x09, false);
   AddFPUOp("ATAN", 0x0a, false);
   AddFPUOp("ASIN", 0x0c, false);
   AddFPUOp("ATANH", 0x0d, false);
   AddFPUOp("SIN", 0x0e, false);
   AddFPUOp("TAN", 0x0f, false);
   AddFPUOp("ETOX", 0x10, false);
   AddFPUOp("TWOTOX", 0x11, false);
   AddFPUOp("TENTOX", 0x12, false);
   AddFPUOp("LOGN", 0x14, false);
   AddFPUOp("LOG10", 0x15, false);
   AddFPUOp("LOG2", 0x16, false);
   AddFPUOp("ABS", 0x18, false);
   AddFPUOp("COSH", 0x19, false);
   AddFPUOp("NEG", 0x1a, false);
   AddFPUOp("ACOS", 0x1c, false);
   AddFPUOp("COS", 0x1d, false);
   AddFPUOp("GETEXP", 0x1e, false);
   AddFPUOp("GETMAN", 0x1f, false);
   AddFPUOp("DIV", 0x20, true);
   AddFPUOp("MOD", 0x21, true);
   AddFPUOp("ADD", 0x22, true);
   AddFPUOp("MUL", 0x23, true);
   AddFPUOp("SGLDIV", 0x24, true);
   AddFPUOp("REM", 0x25, true);
   AddFPUOp("SCALE", 0x26, true);
   AddFPUOp("SGLMUL", 0x27, true);
   AddFPUOp("SUB", 0x28, true);
   AddFPUOp("CMP", 0x38, true);

   FPUConds = (FPUCond *) malloc(sizeof(FPUCond) * FPUCondCnt);
   InstrZ = 0;
   AddFPUCond("EQ", 0x01);
   AddFPUCond("NE", 0x0e);
   AddFPUCond("GT", 0x12);
   AddFPUCond("NGT", 0x1d);
   AddFPUCond("GE", 0x13);
   AddFPUCond("NGE", 0x1c);
   AddFPUCond("LT", 0x14);
   AddFPUCond("NLT", 0x1b);
   AddFPUCond("LE", 0x15);
   AddFPUCond("NLE", 0x1a);
   AddFPUCond("GL", 0x16);
   AddFPUCond("NGL", 0x19);
   AddFPUCond("GLE", 0x17);
   AddFPUCond("NGLE", 0x18);
   AddFPUCond("OGT", 0x02);
   AddFPUCond("ULE", 0x0d);
   AddFPUCond("OGE", 0x03);
   AddFPUCond("ULT", 0x0c);
   AddFPUCond("OLT", 0x04);
   AddFPUCond("UGE", 0x0b);
   AddFPUCond("OLE", 0x05);
   AddFPUCond("UGT", 0x0a);
   AddFPUCond("OGL", 0x06);
   AddFPUCond("UEQ", 0x09);
   AddFPUCond("OR", 0x07);
   AddFPUCond("UN", 0x08);

   PMMUConds = (char **)malloc(sizeof(char *) * PMMUCondCnt);
   InstrZ = 0;
   AddPMMUCond("BS");
   AddPMMUCond("BC");
   AddPMMUCond("LS");
   AddPMMUCond("LC");
   AddPMMUCond("SS");
   AddPMMUCond("SC");
   AddPMMUCond("AS");
   AddPMMUCond("AC");
   AddPMMUCond("WS");
   AddPMMUCond("WC");
   AddPMMUCond("IS");
   AddPMMUCond("IC");
   AddPMMUCond("GS");
   AddPMMUCond("GC");
   AddPMMUCond("CS");
   AddPMMUCond("CC");

   PMMURegNames = (char **)malloc(sizeof(char *) * PMMURegCnt);
   PMMURegSizes = (Byte *) malloc(sizeof(Byte) * PMMURegCnt);
   PMMURegCodes = (Word *) malloc(sizeof(Word) * PMMURegCnt);
   InstrZ = 0;
   AddPMMUReg("TC", 2, 16);
   AddPMMUReg("DRP", 3, 17);
   AddPMMUReg("SRP", 3, 18);
   AddPMMUReg("CRP", 3, 19);
   AddPMMUReg("CAL", 0, 20);
   AddPMMUReg("VAL", 0, 21);
   AddPMMUReg("SCC", 0, 22);
   AddPMMUReg("AC", 1, 23);
   AddPMMUReg("PSR", 1, 24);
   AddPMMUReg("PCSR", 1, 25);
   AddPMMUReg("TT0", 2, 2);
   AddPMMUReg("TT1", 2, 3);
   AddPMMUReg("MMUSR", 1, 24);
}

static void DeinitFields(void) {
   free(CtRegs);
   free(CondNams);
   free(CondVals);
   free(FPUOps);
   free(FPUConds);
   free(PMMUConds);
   free(PMMURegNames);
   free(PMMURegSizes);
   free(PMMURegCodes);
}

/* Adressargument kopieren : ----------------------------------------------*/

#define CopyAdrVals(Dest) memcpy(Dest,AdrVals,AdrCnt)

static bool ValReg(char Ch) {
   return ((Ch >= '0') && (Ch <= '7'));
}

static bool CodeReg(char *s, Word * Erg) {
   bool Result = true;

   if (strcasecmp(s, "SP") == 0) *Erg = 15;
   else if (ValReg(s[1]))
      if (toupper(*s) == 'D') *Erg = s[1] - '0';
      else if (toupper(*s) == 'A') *Erg = s[1] - '0' + 8;
      else Result = false;
   else Result = false;

   return Result;
}

static bool CodeRegPair(char *Asc, Word * Erg1, Word * Erg2) {
   if (strlen(Asc) != 5) return false;
   if (toupper(*Asc) != 'D') return false;
   if (Asc[2] != ':') return false;
   if (toupper(Asc[3]) != 'D') return false;
   if (!(ValReg(Asc[1]) && ValReg(Asc[4]))) return false;

   *Erg1 = Asc[1] - '0';
   *Erg2 = Asc[4] - '0';

   return true;
}

static bool CodeIndRegPair(char *Asc, Word * Erg1, Word * Erg2) {
   if (strlen(Asc) != 9) return false;
   if (*Asc != '(') return false;
   if ((toupper(Asc[1]) != 'D') && (toupper(Asc[1]) != 'A')) return false;
   if (Asc[3] != ')') return false;
   if (Asc[4] != ':') return false;
   if (Asc[5] != '(') return false;
   if ((toupper(Asc[6]) != 'D') && (toupper(Asc[6]) != 'A')) return false;
   if (Asc[8] != ')') return false;
   if (!(ValReg(Asc[2]) && ValReg(Asc[7]))) return false;

   *Erg1 = Asc[2] - '0';
   if (toupper(Asc[1]) == 'A') *Erg1 += 8;
   *Erg2 = Asc[7] - '0';
   if (toupper(Asc[6]) == 'A') *Erg2 += 8;

   return true;
}

/*-------------------------------------------------------------------------*/

typedef enum { PC, AReg, Index, indir, Disp, None } CompType;
typedef struct {
   String Name;
   CompType Art;
   Word ANummer, INummer;
   bool Long;
   Word Scale;
   Word Size;
   LongInt Wert;
} AdrComp;

static bool ClassComp(AdrComp * C) {
   char sh[10];

   if ((*C->Name == '[') && (C->Name[strlen(C->Name) - 1] == ']')) {
      C->Art = indir;
      return true;
   }

   if (strcasecmp(C->Name, "PC") == 0) {
      C->Art = PC;
      return true;
   }

   sh[0] = C->Name[0];
   sh[1] = C->Name[1];
   sh[2] = '\0';
   if (!CodeReg(sh, &C->ANummer)) ;
   else if ((C->ANummer > 7) && (strlen(C->Name) == 2)) {
      C->Art = AReg;
      C->ANummer -= 8;
      return true;
   } else {
      if ((strlen(C->Name) > 3) && (C->Name[2] == '.')) {
         switch (toupper(C->Name[3])) {
            case 'L':
               C->Long = true;
               break;
            case 'W':
               C->Long = false;
               break;
            default:
               return false;
         }
         strmove(C->Name + 2, 2);
      } else C->Long = false;
      if ((strlen(C->Name) > 3) && (C->Name[2] == '*')) {
         switch (C->Name[3]) {
            case '1':
               C->Scale = 0;
               break;
            case '2':
               C->Scale = 1;
               break;
            case '4':
               C->Scale = 2;
               break;
            case '8':
               C->Scale = 3;
               break;
            default:
               return false;
         }
         strmove(C->Name + 2, 2);
      } else C->Scale = 0;
      C->INummer = C->ANummer;
      C->Art = Index;
      return true;
   }

   C->Art = Disp;
   if (C->Name[strlen(C->Name) - 2] == '.') {
      switch (toupper(C->Name[strlen(C->Name) - 1])) {
         case 'L':
            C->Size = 2;
            break;
         case 'W':
            C->Size = 1;
            break;
         default:
            return false;
      }
      C->Name[strlen(C->Name) - 2] = '\0';
   } else C->Size = 1;
   C->Art = Disp;
   return true;
}

static void ACheckCPU(CPUVar MinCPU) {
   if (MomCPU < MinCPU) {
      WrError(1505);
      AdrNum = 0;
      AdrCnt = 0;
   }
}

static void ChkAdr(Word Erl) {
   if ((Erl & Masks[AdrNum]) == 0) {
      WrError(1350);
      AdrNum = 0;
   }
}

static bool IsShortAdr(LongInt Adr) {
   Word WHi = (Adr >> 16) & 0xffff, WLo = Adr & 0xffff;

   return ((WHi == 0) && (WLo <= 0x7fff))
      || ((WHi == 0xffff) && (WLo >= 0x8000));
}

static bool IsDisp8(LongInt Disp) {
   return ((Disp >= -128) && (Disp <= 127));
}

static bool IsDisp16(LongInt Disp) {
   if (Disp < -32768) return false;
   if (Disp > 32767) return false;
   return true;
}

static void ChkEven(LongInt Adr) {
   if ((MomCPU <= CPU68340) && (Odd(Adr))) WrError(180);
}

static void DecodeAdr(char *Asc_O, Word Erl) {
   Byte l, i;
   char *p;
   Word rerg;
   Byte lklamm, rklamm, lastrklamm;
   bool doklamm;

   AdrComp AdrComps[3], OneComp;
   Byte CompCnt;
   String OutDisp;
   Byte OutDispLen;
   bool PreInd;

#ifdef HAS64
   QuadInt QVal;
#endif
   LongInt HVal;
   Integer HVal16;
   ShortInt HVal8;
   Single FVal;
   Double DVal;
   bool ValOK;
   Word SwapField[6];
   String Asc;
   char CReg[10];

   strmaxcpy(Asc, Asc_O, 255);
   KillBlanks(Asc);
   l = strlen(Asc);
   AdrNum = 0;
   AdrCnt = 0;

/* immediate : */

   if (*Asc == '#') {
      strmove(Asc, 1);
      AdrNum = 11;
      AdrMode = 0x3c;
      switch (OpSize) {
         case 0:
            AdrCnt = 2;
            HVal8 = EvalIntExpression(Asc, Int8, &ValOK);
            if (ValOK) AdrVals[0] = (Word) ((Byte) HVal8);
            break;
         case 1:
            AdrCnt = 2;
            HVal16 = EvalIntExpression(Asc, Int16, &ValOK);
            if (ValOK) AdrVals[0] = (Word) HVal16;
            break;
         case 2:
            AdrCnt = 4;
            HVal = EvalIntExpression(Asc, Int32, &ValOK);
            if (ValOK) {
               AdrVals[0] = HVal >> 16;
               AdrVals[1] = HVal & 0xffff;
            }
            break;
#ifdef HAS64
         case 3:
            AdrCnt = 8;
            QVal = EvalIntExpression(Asc, Int64, &ValOK);
            if (ValOK) {
               AdrVals[0] = (QVal >> 48) & 0xffff;
               AdrVals[1] = (QVal >> 32) & 0xffff;
               AdrVals[2] = (QVal >> 16) & 0xffff;
               AdrVals[3] = (QVal) & 0xffff;
            }
            break;
#endif
         case 4:
            AdrCnt = 4;
            FVal = EvalFloatExpression(Asc, Float32, &ValOK);
            if (ValOK) {
               memcpy(SwapField, &FVal, 4);
               if (BigEndian) DWSwap((Byte *) SwapField, 4);
               AdrVals[0] = SwapField[1];
               AdrVals[1] = SwapField[0];
            }
            break;
         case 5:
            AdrCnt = 8;
            DVal = EvalFloatExpression(Asc, Float64, &ValOK);
            if (ValOK) {
               memcpy(SwapField, &DVal, 8);
               if (BigEndian) QWSwap((Byte *) SwapField, 8);
               AdrVals[0] = SwapField[3];
               AdrVals[1] = SwapField[2];
               AdrVals[2] = SwapField[1];
               AdrVals[3] = SwapField[0];
            }
            break;
         case 6:
            AdrCnt = 12;
            DVal = EvalFloatExpression(Asc, Float64, &ValOK);
            if (ValOK) {
               Double_2_TenBytes(DVal, (Byte *) SwapField);
               if (BigEndian) WSwap((Byte *) SwapField, 10);
               AdrVals[0] = SwapField[4];
               AdrVals[1] = 0;
               AdrVals[2] = SwapField[3];
               AdrVals[3] = SwapField[2];
               AdrVals[4] = SwapField[1];
               AdrVals[5] = SwapField[0];
            }
            break;
         case 7:
            AdrCnt = 12;
            DVal = EvalFloatExpression(Asc, Float64, &ValOK);
            if (ValOK) {
               ConvertDec(DVal, SwapField);
               AdrVals[0] = SwapField[5];
               AdrVals[1] = SwapField[4];
               AdrVals[2] = SwapField[3];
               AdrVals[3] = SwapField[2];
               AdrVals[4] = SwapField[1];
               AdrVals[5] = SwapField[0];
            }
            break;
      }
      ChkAdr(Erl);
      return;
   }

/* CPU-Register direkt: */

   if (CodeReg(Asc, &AdrMode)) {
      AdrCnt = 0;
      AdrNum = (AdrMode >> 3) + 1;
      ChkAdr(Erl);
      return;
   }

/* Gleitkommaregister direkt: */

   if (strncasecmp(Asc, "FP", 2) == 0) {
      if ((strlen(Asc) == 3) && (ValReg(Asc[2]))) {
         AdrMode = Asc[2] - '0';
         AdrCnt = 0;
         AdrNum = 12;
         ChkAdr(Erl);
         return;
      }
      if (strcasecmp(Asc, "FPCR") == 0) {
         AdrMode = 4;
         AdrNum = 13;
         ChkAdr(Erl);
         return;
      }
      if (strcasecmp(Asc, "FPSR") == 0) {
         AdrMode = 2;
         AdrNum = 13;
         ChkAdr(Erl);
         return;
      }
      if (strcasecmp(Asc, "FPIAR") == 0) {
         AdrMode = 1;
         AdrNum = 13;
         ChkAdr(Erl);
         return;
      }
   }

/* Adressregister indirekt mit Predekrement: */

   if ((l == 5) && (*Asc == '-') && (Asc[1] == '(') && (Asc[4] == ')')) {
      strcopy(CReg, Asc + 2);
      CReg[2] = '\0';
      if (CodeReg(CReg, &rerg))
         if (rerg > 7) {
            AdrMode = rerg + 24;
            AdrCnt = 0;
            AdrNum = 5;
            ChkAdr(Erl);
            return;
         }
   }

/* Adressregister indirekt mit Postinkrement */

   if ((l == 5) && (*Asc == '(') && (Asc[3] == ')') && (Asc[4] == '+')) {
      strcopy(CReg, Asc + 1);
      CReg[2] = '\0';
      if (CodeReg(CReg, &rerg))
         if (rerg > 7) {
            AdrMode = rerg + 16;
            AdrCnt = 0;
            AdrNum = 4;
            ChkAdr(Erl);
            return;
         }
   }

/* Unterscheidung direkt<->indirekt: */

   lklamm = 0;
   rklamm = 0;
   lastrklamm = 0;
   doklamm = true;
   for (p = Asc; *p != '\0'; p++) {
      if (*p == '[') doklamm = false;
      if (*p == ']') doklamm = true;
      if (!doklamm) ;
      else if (*p == '(') lklamm++;
      else if (*p == ')') {
         rklamm++;
         lastrklamm = p - Asc;
      }
   }

   if ((lklamm == 1) && (rklamm == 1) && (lastrklamm == strlen(Asc) - 1)) {

   /* aeusseres Displacement abspalten, Klammern loeschen: */

      p = strchr(Asc, '(');
      *p = '\0';
      strmaxcpy(OutDisp, Asc, 255);
      strcopy(Asc, p + 1);
      if ((strlen(OutDisp) > 2) && (OutDisp[strlen(OutDisp) - 2] == '.')) {
         switch (toupper(OutDisp[strlen(OutDisp) - 1])) {
            case 'B':
               OutDispLen = 0;
               break;
            case 'W':
               OutDispLen = 1;
               break;
            case 'L':
               OutDispLen = 2;
               break;
            default:
               WrError(1130);
               return;
         }
         OutDisp[strlen(OutDisp) - 2] = '\0';
      } else OutDispLen = 0;
      Asc[strlen(Asc) - 1] = '\0';

   /* in Komponenten zerteilen: */

      CompCnt = 0;
      do {
         doklamm = true;
         p = Asc;
         do {
            if (*p == '[') doklamm = false;
            else if (*p == ']') doklamm = true;
            p++;
         }
         while (((!doklamm) || (*p != ',')) && (*p != '\0'));
         if (*p == '\0') {
            strcopy(AdrComps[CompCnt].Name, Asc);
            *Asc = '\0';
         } else {
            *p = '\0';
            strcopy(AdrComps[CompCnt].Name, Asc);
            strcopy(Asc, p + 1);
         }
         if (!ClassComp(AdrComps + CompCnt)) {
            WrError(1350);
            return;
         }
         if ((CompCnt == 1) && (AdrComps[CompCnt].Art == AReg)) {
            AdrComps[CompCnt].Art = Index;
            AdrComps[CompCnt].INummer = AdrComps[CompCnt].ANummer + 8;
            AdrComps[CompCnt].Long = false;
            AdrComps[CompCnt].Scale = 0;
         }
         if ((AdrComps[CompCnt].Art == Disp) || ((AdrComps[CompCnt].Art != Index) && (CompCnt != 0))) {
            WrError(1350);
            return;
         }
         CompCnt++;
      }
      while (*Asc != '\0');
      if ((CompCnt > 2) || ((AdrComps[0].Art == Index) && (CompCnt != 1))) {
         WrError(1350);
         return;
      }

   /* 1. Variante (An....), d(An....) */

      if (AdrComps[0].Art == AReg) {

      /* 1.1. Variante (An), d(An) */

         if (CompCnt == 1) {

         /* 1.1.1. Variante (An) */

            if ((*OutDisp == '\0') && ((Madri & Erl) != 0)) {
               AdrMode = 0x10 + AdrComps[0].ANummer;
               AdrNum = 3;
               AdrCnt = 0;
               ChkAdr(Erl);
               return;
            }

         /* 1.1.2. Variante d(An) */

            else {
               if (OutDispLen >= 2) HVal = EvalIntExpression(OutDisp, Int32, &ValOK);
               else HVal = EvalIntExpression(OutDisp, SInt16, &ValOK);
               if (!ValOK) {
                  WrError(1350);
                  return;
               }
               if ((ValOK) && (HVal == 0) && ((Madri & Erl) != 0) && (OutDispLen == 0)) {
                  AdrMode = 0x10 + AdrComps[0].ANummer;
                  AdrNum = 3;
                  AdrCnt = 0;
                  ChkAdr(Erl);
                  return;
               }
               if (OutDispLen == 0) OutDispLen = 1;
               switch (OutDispLen) {
                  case 1: /* d16(An) */
                     AdrMode = 0x28 + AdrComps[0].ANummer;
                     AdrNum = 6;
                     AdrCnt = 2;
                     AdrVals[0] = HVal & 0xffff;
                     ChkAdr(Erl);
                     return;
                  case 2: /* d32(An) */
                     AdrMode = 0x30 + AdrComps[0].ANummer;
                     AdrNum = 7;
                     AdrCnt = 6;
                     AdrVals[0] = 0x0170;
                     AdrVals[1] = (HVal >> 16) & 0xffff;
                     AdrVals[2] = HVal & 0xffff;
                     ACheckCPU(CPU68332);
                     ChkAdr(Erl);
                     return;
               }
            }
         }

      /* 1.2. Variante d(An,Xi) */

         else {
            AdrVals[0] = (AdrComps[1].INummer << 12) + (AdrComps[1].Long << 11) + (AdrComps[1].Scale << 9);
            AdrMode = 0x30 + AdrComps[0].ANummer;
            switch (OutDispLen) {
               case 0:
                  HVal8 = EvalIntExpression(OutDisp, SInt8, &ValOK);
                  if (ValOK) {
                     AdrNum = 7;
                     AdrCnt = 2;
                     AdrVals[0] += ((Byte) HVal8);
                     if (AdrComps[1].Scale != 0) ACheckCPU(CPU68332);
                  }
                  ChkAdr(Erl);
                  return;
               case 1:
                  HVal16 = EvalIntExpression(OutDisp, SInt16, &ValOK);
                  if (ValOK) {
                     AdrNum = 7;
                     AdrCnt = 4;
                     AdrVals[0] += 0x120;
                     AdrVals[1] = HVal16;
                     ACheckCPU(CPU68332);
                  }
                  ChkAdr(Erl);
                  return;
               case 2:
                  HVal = EvalIntExpression(OutDisp, Int32, &ValOK);
                  if (ValOK) {
                     AdrNum = 7;
                     AdrCnt = 6;
                     AdrVals[0] += 0x130;
                     AdrVals[1] = HVal >> 16;
                     AdrVals[2] = HVal & 0xffff;
                     ACheckCPU(CPU68332);
                  }
                  ChkAdr(Erl);
                  return;
            }
         }
      }

   /* 2. Variante d(PC....) */

      else if (AdrComps[0].Art == PC) {

      /* 2.1. Variante d(PC) */

         if (CompCnt == 1) {
            if (OutDispLen == 0) OutDispLen = 1;
            HVal = EvalIntExpression(OutDisp, Int32, &ValOK) - (EProgCounter() + RelPos);
            if (!ValOK) {
               WrError(1350);
               return;
            }
            switch (OutDispLen) {
               case 1:
                  AdrMode = 0x3a;
                  HVal16 = HVal;
                  if (!IsDisp16(HVal)) {
                     WrError(1330);
                     return;
                  }
                  AdrNum = 8;
                  AdrCnt = 2;
                  AdrVals[0] = HVal16;
                  ChkAdr(Erl);
                  return;
               case 2:
                  AdrMode = 0x3b;
                  AdrNum = 9;
                  AdrCnt = 6;
                  AdrVals[0] = 0x170;
                  AdrVals[1] = HVal >> 16;
                  AdrVals[2] = HVal & 0xffff;
                  ACheckCPU(CPU68332);
                  ChkAdr(Erl);
                  return;
            }
         }

      /* 2.2. Variante d(PC,Xi) */

         else {
            AdrVals[0] = (AdrComps[1].INummer << 12) + (AdrComps[1].Long << 11) + (AdrComps[1].Scale << 9);
            HVal = EvalIntExpression(OutDisp, Int32, &ValOK) - (EProgCounter() + RelPos);
            if (!ValOK) {
               WrError(1350);
               return;
            }
            AdrMode = 0x3b;
            switch (OutDispLen) {
               case 0:
                  HVal8 = HVal;
                  if (!IsDisp8(HVal)) {
                     WrError(1330);
                     return;
                  }
                  AdrVals[0] += ((Byte) HVal8);
                  AdrCnt = 2;
                  AdrNum = 9;
                  if (AdrComps[1].Scale != 0) ACheckCPU(CPU68332);
                  ChkAdr(Erl);
                  return;
               case 1:
                  HVal16 = HVal;
                  if (!IsDisp16(HVal)) {
                     WrError(1330);
                     return;
                  }
                  AdrVals[0] += 0x120;
                  AdrCnt = 4;
                  AdrNum = 9;
                  AdrVals[1] = HVal16;
                  ACheckCPU(CPU68332);
                  ChkAdr(Erl);
                  return;
               case 2:
                  AdrVals[0] = AdrVals[0] + 0x120;
                  AdrCnt = 6;
                  AdrNum = 9;
                  AdrVals[1] = HVal >> 16;
                  AdrVals[2] = HVal & 0xffff;
                  ACheckCPU(CPU68332);
                  ChkAdr(Erl);
                  return;
            }
         }
      }

   /* 3. Variante (Xi), d(Xi) */

      else if (AdrComps[0].Art == Index) {
         AdrVals[0] = (AdrComps[0].INummer << 12) + (AdrComps[0].Long << 11) + (AdrComps[0].Scale << 9) + 0x180;
         AdrMode = 0x30;
         if (*OutDisp == '\0') {
            AdrVals[0] = AdrVals[0] + 0x0010;
            AdrCnt = 2;
            AdrNum = 7;
            ACheckCPU(CPU68332);
            ChkAdr(Erl);
            return;
         } else switch (OutDispLen) {
               case 0:
               case 1:
                  HVal16 = EvalIntExpression(OutDisp, Int16, &ValOK);
                  if (ValOK) {
                     AdrVals[0] = AdrVals[0] + 0x0020;
                     AdrVals[1] = HVal16;
                     AdrNum = 7;
                     AdrCnt = 4;
                     ACheckCPU(CPU68332);
                  }
                  ChkAdr(Erl);
                  return;
               case 2:
                  HVal = EvalIntExpression(OutDisp, Int32, &ValOK);
                  if (ValOK) {
                     AdrVals[0] = AdrVals[0] + 0x0030;
                     AdrNum = 7;
                     AdrCnt = 6;
                     AdrVals[1] = HVal >> 16;
                     AdrVals[2] = HVal & 0xffff;
                     ACheckCPU(CPU68332);
                  }
                  ChkAdr(Erl);
                  return;
         }
      }

   /* 4. Variante indirekt: */

      else if (AdrComps[0].Art == indir) {

      /* erst ab 68020 erlaubt */

         if (MomCPU < CPU68020) {
            WrError(1505);
            return;
         }

      /* Unterscheidung Vor- <---> Nachindizierung: */

         if (CompCnt == 2) {
            PreInd = false;
            AdrComps[2] = AdrComps[1];
         } else {
            PreInd = true;
            AdrComps[2].Art = None;
         }

      /* indirektes Argument herauskopieren: */

         strcopy(Asc, AdrComps[0].Name + 1);
         Asc[strlen(Asc) - 1] = '\0';

      /* Felder loeschen: */

         for (i = 0; i < 2; AdrComps[i++].Art = None);

      /* indirekten Ausdruck auseinanderfieseln: */

         do {

         /* abschneiden & klassifizieren: */

            p = strchr(Asc, ',');
            if (p == NULL) {
               strcopy(OneComp.Name, Asc);
               *Asc = '\0';
            } else {
               *p = '\0';
               strcopy(OneComp.Name, Asc);
               strcopy(Asc, p + 1);
            }
            if (!ClassComp(&OneComp)) {
               WrError(1350);
               return;
            }

         /* passend einsortieren: */

            if ((AdrComps[1].Art != None) && (OneComp.Art == AReg)) {
               OneComp.Art = Index;
               OneComp.INummer = OneComp.ANummer + 8;
               OneComp.Long = false;
               OneComp.Scale = 0;
            }
            switch (OneComp.Art) {
               case Disp:
                  i = 0;
                  break;
               case AReg:
               case PC:
                  i = 1;
                  break;
               case Index:
                  i = 2;
                  break;
               default:
                  i = (-1);
            }
            if (AdrComps[i].Art != None) {
               WrError(1350);
               return;
            } else AdrComps[i] = OneComp;
         }
         while (*Asc != '\0');

      /* Vor-oder Nachindizierung? */

         AdrVals[0] = 0x100 + (PreInd << 2);

      /* Indexregister eintragen */

         if (AdrComps[2].Art == None) AdrVals[0] += 0x40;
         else AdrVals[0] += (AdrComps[2].INummer << 12) + (AdrComps[2].Long << 11) + (AdrComps[2].Scale << 9);

      /* 4.1 Variante d([...PC...]...) */

         if (AdrComps[1].Art == PC) {
            HVal = EvalIntExpression(AdrComps[0].Name, Int32, &ValOK) - (EProgCounter() + RelPos);
            if (!ValOK) return;
            AdrMode = 0x3b;
            switch (AdrComps[0].Size) {
               case 1:
                  HVal16 = HVal;
                  if (!IsDisp16(HVal)) {
                     WrError(1330);
                     return;
                  }
                  AdrVals[1] = HVal16;
                  AdrVals[0] += 0x20;
                  AdrNum = 7;
                  AdrCnt = 4;
                  break;
               case 2:
                  AdrVals[1] = HVal >> 16;
                  AdrVals[2] = HVal & 0xffff;
                  AdrVals[0] += 0x30;
                  AdrNum = 7;
                  AdrCnt = 6;
                  break;
            }
         }

      /* 4.2 Variante d([...An...]...) */

         else {
            if (AdrComps[1].Art == None) {
               AdrMode = 0x30;
               AdrVals[0] += 0x80;
            } else AdrMode = 0x30 + AdrComps[1].ANummer;

            if (AdrComps[0].Art == None) {
               AdrNum = 7;
               AdrCnt = 2;
               AdrVals[0] += 0x10;
            } else switch (AdrComps[0].Size) {
                  case 1:
                     HVal16 = EvalIntExpression(AdrComps[0].Name, Int16, &ValOK);
                     if (!ValOK) return;
                     AdrNum = 7;
                     AdrVals[1] = HVal16;
                     AdrCnt = 4;
                     AdrVals[0] += 0x20;
                     break;
                  case 2:
                     HVal = EvalIntExpression(AdrComps[0].Name, Int32, &ValOK);
                     if (!ValOK) return;
                     AdrNum = 7;
                     AdrCnt = 6;
                     AdrVals[0] += 0x30;
                     AdrVals[1] = HVal >> 16;
                     AdrVals[2] = HVal & 0xffff;
                     break;
            }
         }

      /* aeusseres Displacement: */

         if (OutDispLen == 0) OutDispLen = 1;
         if (*OutDisp == '\0') {
            AdrVals[0]++;
            ChkAdr(Erl);
            return;
         } else switch (OutDispLen) {
               case 1:
                  HVal16 = EvalIntExpression(OutDisp, Int16, &ValOK);
                  if (!ValOK) {
                     AdrNum = 0;
                     AdrCnt = 0;
                     return;
                  }
                  AdrVals[AdrCnt >> 1] = HVal16;
                  AdrCnt += 2;
                  AdrVals[0] += 2;
                  break;
               case 2:
                  HVal = EvalIntExpression(OutDisp, Int32, &ValOK);
                  if (!ValOK) {
                     AdrNum = 0;
                     AdrCnt = 0;
                     return;
                  }
                  AdrVals[(AdrCnt >> 1)] = HVal >> 16;
                  AdrVals[(AdrCnt >> 1) + 1] = HVal & 0xffff;
                  AdrCnt += 4;
                  AdrVals[0] += 3;
                  break;
         }

         ChkAdr(Erl);
         return;

      }

   }

/* absolut: */

   else {
      AdrCnt = 0;
      if (strcasecmp(Asc + strlen(Asc) - 2, ".W") == 0) {
         AdrCnt = 2;
         Asc[strlen(Asc) - 2] = '\0';
      } else if (strcasecmp(Asc + strlen(Asc) - 2, ".L") == 0) {
         AdrCnt = 4;
         Asc[strlen(Asc) - 2] = '\0';
      }

      FirstPassUnknown = false;
      HVal = EvalIntExpression(Asc, Int32, &ValOK);
      if ((!FirstPassUnknown) && (OpSize > 0)) ChkEven(HVal);
      HVal16 = HVal;

      if (ValOK) {
         if (AdrCnt == 0) AdrCnt = (IsShortAdr(HVal)) ? 2 : 4;
         AdrNum = 10;

         if (AdrCnt == 2) {
            if (!IsShortAdr(HVal)) {
               WrError(1340);
               AdrNum = 0;
            } else {
               AdrMode = 0x38;
               AdrVals[0] = HVal16;
            }
         } else {
            AdrMode = 0x39;
            AdrVals[0] = HVal >> 16;
            AdrVals[1] = HVal & 0xffff;
         }
      }
   }

   ChkAdr(Erl);
}

/*---------------------------------------------------------------------------*/

static void PutByte(Byte b) {
   if (((CodeLen & 1) == 1) && (!BigEndian)) {
      BAsmCode[CodeLen] = BAsmCode[CodeLen - 1];
      BAsmCode[CodeLen - 1] = b;
   } else {
      BAsmCode[CodeLen] = b;
   }
   CodeLen++;
}

static bool DecodePseudo(void) {
#define ONOFF68KCount 4
   static ONOFFRec ONOFF68Ks[ONOFF68KCount] = {
      { "PMMU", &PMMUAvail, PMMUAvailName },
      { "FULLPMMU", &FullPMMU, FullPMMUName },
      { "FPU", &FPUAvail, FPUAvailName },
      { "SUPMODE", &SupAllowed, SupAllowedName }
   };
   int z, l;

   if (Memo("STR")) {
      l = strlen(ArgStr[1]);
      if (ArgCnt != 1) WrError(1110);
      else if (l < 2) WrError(1135);
      else if (*ArgStr[1] != '\'') WrError(1135);
      else if (ArgStr[1][l - 1] != '\'') WrError(1135);
      else {
         PutByte(l - 2);
         for (z = 1; z < l - 1; z++)
            PutByte(CharTransTable[(unsigned int)ArgStr[1][z]]);
      }
      if ((Odd(CodeLen)) && (DoPadding)) PutByte(0);
      return true;
   }

   if (CodeONOFF(ONOFF68Ks, ONOFF68KCount)) return true;

   return false;
}

static Word ShiftCodes(char *s) {
   size_t l = strlen(s);
   char Ch = s[l - 1];
   Word Erg;

   s[l - 1] = '\0';
   if (strcmp(s, "AS") == 0) Erg = 0;
   else if (strcmp(s, "LS") == 0) Erg = 1;
   else if (strcmp(s, "RO") == 0) Erg = 3;
   else if (strcmp(s, "ROX") == 0) Erg = 2;
   else Erg = 0xffff;

   s[l - 1] = Ch;
   return Erg;
}

static Byte OneReg(char *Asc) {
   if (strlen(Asc) != 2) return 16;
   if ((toupper(*Asc) != 'A') && (toupper(*Asc) != 'D')) return 16;
   if (!ValReg(Asc[1])) return 16;
   return Asc[1] - '0' + ((toupper(*Asc) == 'D') ? 0 : 8);
}

static bool DecodeRegList(char *Asc_o, Word * Erg) {
   static Word Masks[16] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };
   Byte h, h2, z;
   char *p;
   String Asc, s;

   *Erg = 0;
   strmaxcpy(Asc, Asc_o, 255);
   do {
      p = strchr(Asc, '/');
      if (p == NULL) {
         strcopy(s, Asc);
         *Asc = '\0';
      } else {
         *p = '\0';
         strcopy(s, Asc);
         strcopy(Asc, p + 1);
      }
      if (*Asc == '/') strmove(Asc, 1);
      p = strchr(s, '-');
      if (p == NULL) {
         if ((h = OneReg(s)) == 16) return false;
         *Erg |= Masks[h];
      } else {
         *p = '\0';
         if ((h = OneReg(s)) == 16) return false;
         if ((h2 = OneReg(p + 1)) == 16) return false;
         for (z = h; z <= h2; *Erg |= Masks[z++]);
      }
   }
   while (*Asc != '\0');
   return true;
}

static bool DecodeCtrlReg(char *Asc, Word * Erg) {
   Byte z;
   String Asc_N;

   strmaxcpy(Asc_N, Asc, 255);
   NLS_UpString(Asc_N);
   Asc = Asc_N;

   z = 0;
   while ((z < CtRegCnt) && ((strcmp(CtRegs[z].Name, Asc) != 0) || (CtRegs[z].FirstCPU > MomCPU)))
      z++;
   if (z != CtRegCnt) *Erg = CtRegs[z].Code;
   return (z != CtRegCnt);
}

static bool OneField(char *Asc, Word * Erg, bool Ab1) {
   bool ValOK;

   if ((strlen(Asc) == 2) && (toupper(*Asc) == 'D') && (ValReg(Asc[1]))) {
      *Erg = 0x20 + (Asc[1] - '0');
      return true;
   } else {
      *Erg = EvalIntExpression(Asc, Int8, &ValOK);
      if ((Ab1) && (*Erg == 32)) *Erg = 0;
      return ((ValOK) && (*Erg < 32));
   }
}

static bool SplitBitField(char *Arg_o, Word * Erg) {
   char *p;
   Word OfsVal;
   String Desc, Arg;

   strmaxcpy(Arg, Arg_o, 255);
   p = strchr(Arg, '{');
   if (p == NULL) return false;
   *p = '\0';
   strcopy(Desc, p + 1);
   if (Desc[strlen(Desc) - 1] != '}') return false;
   Desc[strlen(Desc) - 1] = '\0';

   p = strchr(Desc, ':');
   if (p == NULL) return false;
   *p = '\0';
   if (!OneField(Desc, &OfsVal, false)) return false;
   if (!OneField(p + 1, Erg, true)) return false;
   Erg += OfsVal << 6;
   return true;
}

static void CheckCPU(CPUVar Level) {
   if (MomCPU < Level) {
      WrError(1500);
      CodeLen = 0;
   }
}

static void Check020(void) {
   if (MomCPU != CPU68020) {
      WrError(1500);
      CodeLen = 0;
   }
}

static void Check32(void) {
   if ((MomCPU < CPU68332) || (MomCPU > CPU68360)) {
      WrError(1500);
      CodeLen = 0;
   }
}

static void CheckSup(void) {
   if (!SupAllowed) WrError(50);
}

/*-------------------------------------------------------------------------*/

static bool DecodeOneFPReg(char *Asc, Byte * h) {
   if ((strlen(Asc) == 3) && (strncasecmp(Asc, "FP", 2) == 0) && ValReg(Asc[2])) {
      *h = Asc[2] - '0';
      return true;
   } else return false;
}

static void DecodeFRegList(char *Asc_o, Byte * Typ, Byte * Erg) {
   String s, Asc;
   Word hw;
   Byte h2, h3, z;
   char *h1;

   strmaxcpy(Asc, Asc_o, 255);
   *Typ = 0;
   if (*Asc == '\0') return;

   if ((strlen(Asc) == 2) && (*Asc == 'D') && ValReg(Asc[1])) {
      *Typ = 1;
      *Erg = (Asc[1] - '0') << 4;
      return;
   }

   hw = 0;
   do {
      h1 = strchr(Asc, '/');
      if (h1 == NULL) {
         strcopy(s, Asc);
         *Asc = '\0';
      } else {
         *h1 = '\0';
         strcopy(s, Asc);
         strcopy(Asc, h1 + 1);
      }
      if (strcasecmp(s, "FPCR") == 0) hw |= 0x400;
      else if (strcasecmp(s, "FPSR") == 0) hw |= 0x200;
      else if (strcasecmp(s, "FPIAR") == 0) hw |= 0x100;
      else {
         h1 = strchr(s, '-');
         if (h1 == NULL) {
            if (!DecodeOneFPReg(s, &h2)) return;
            hw |= (1 << (7 - h2));
         } else {
            *h1 = '\0';
            if (!DecodeOneFPReg(s, &h2)) return;
            if (!DecodeOneFPReg(h1 + 1, &h3)) return;
            for (z = h2; z <= h3; z++) hw |= (1 << (7 - z));
         }
      }
   }
   while (*Asc != '\0');
   if (Hi(hw) == 0) {
      *Typ = 2;
      *Erg = Lo(hw);
   } else if (Lo(hw) == 0) {
      *Typ = 3;
      *Erg = Hi(hw);
   }
}

static void GenerateMovem(Byte z1, Byte z2) {
   Byte hz2, z;

   if (AdrNum == 0) return;
   CodeLen = 4 + AdrCnt;
   CopyAdrVals(WAsmCode + 2);
   WAsmCode[0] = 0xf200 | AdrMode;
   switch (z1) {
      case 1:
      case 2:
         WAsmCode[1] |= 0xc000;
         if (z1 == 1) WAsmCode[1] |= 0x800;
         if (AdrNum != 5) WAsmCode[1] |= 0x1000;
         if ((AdrNum == 5) && (z1 == 2)) {
            hz2 = z2;
            z2 = 0;
            for (z = 0; z < 8; z++) {
               z2 = z2 << 1;
               if ((hz2 & 1) == 1) z2 |= 1;
               hz2 = hz2 >> 1;
            }
         }
         WAsmCode[1] |= z2;
         break;
      case 3:
         WAsmCode[1] |= 0x8000 | (((Word) z2) << 10);
   }
}

static void DecodeFPUOrders(void) {
   static Byte SizeCodes[87] = { 6, 4, 0, 7, 1, 5, 2, 3 };
   Byte z, z1, z2;
   char *p;
   String sk;
   LongInt HVal;
   Integer HVal16;
   bool ValOK;

   for (z = 0; z < FPUOpCnt; z++)
      if (Memo(FPUOps[z].Name)) break;
   if (z < FPUOpCnt) {
      if ((ArgCnt == 1) && (!FPUOps[z].Dya)) {
         strcopy(ArgStr[2], ArgStr[1]);
         ArgCnt = 2;
      }
      if (*AttrPart == '\0') OpSize = 6;
      if (OpSize == 3) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], Mfpn);
         if (AdrNum == 12) {
            WAsmCode[0] = 0xf200;
            WAsmCode[1] = FPUOps[z].Code | (AdrMode << 7);
            RelPos = 4;
            DecodeAdr(ArgStr[1], ((OpSize <= 2) || (OpSize == 4)) ? Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm + Mfpn : Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm + Mfpn);
            if (AdrNum == 12) {
               WAsmCode[1] |= AdrMode << 10;
               if (OpSize == 6) CodeLen = 4;
               else WrError(1130);
            } else if (AdrNum != 0) {
               CodeLen = 4 + AdrCnt;
               CopyAdrVals(WAsmCode + 2);
               WAsmCode[0] |= AdrMode;
               WAsmCode[1] |= 0x4000 | (((Word) SizeCodes[OpSize]) << 10);
            }
         }
      }
      return;
   }

   if (Memo("SAVE")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1130);
      else {
         DecodeAdr(ArgStr[1], Madri + Mpre + Mdadri + Maix + Mabs);
         if (AdrNum != 0) {
            CodeLen = 2 + AdrCnt;
            WAsmCode[0] = 0xf300 | AdrMode;
            CopyAdrVals(WAsmCode + 1);
            CheckSup();
         }
      }
      return;
   }

   if (Memo("RESTORE")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1130);
      else {
         DecodeAdr(ArgStr[1], Madri + Mpost + Mdadri + Maix + Mabs);
         if (AdrNum != 0) {
            CodeLen = 2 + AdrCnt;
            WAsmCode[0] = 0xf340 | AdrMode;
            CopyAdrVals(WAsmCode + 1);
            CheckSup();
         }
      }
      return;
   }

   if (Memo("NOP")) {
      if (ArgCnt != 0) WrError(1110);
      else if (*AttrPart != '\0') WrError(1130);
      else {
         CodeLen = 4;
         WAsmCode[0] = 0xf280;
         WAsmCode[1] = 0;
      }
      return;
   }

   if (Memo("MOVE")) {
      if (ArgCnt != 2) WrError(1110);
      else if (OpSize == 3) WrError(1130);
      else {
         p = strchr(AttrPart, '{');
         if (p != 0) { /* k-Faktor abspalten */
            strcopy(sk, p);
            *p = '\0';
         } else *sk = '\0';
         DecodeAdr(ArgStr[2], Mdata + Madr + Madri + Mpost + Mpre + Mdadri + Maix + Mabs + Mfpn + Mfpcr);
         if (AdrNum == 12) { /* FMOVE.x <ea>/FPm,FPn ? */
            WAsmCode[0] = 0xf200;
            WAsmCode[1] = AdrMode << 7;
            RelPos = 4;
            if (*AttrPart == '\0') OpSize = 6;
            DecodeAdr(ArgStr[1], ((OpSize <= 2) || (OpSize == 4)) ? Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm + Mfpn : Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm + Mfpn);
            if (AdrNum == 12) { /* FMOVE.X FPm,FPn ? */
               WAsmCode[1] |= AdrMode << 10;
               if (OpSize == 6) CodeLen = 4;
               else WrError(1130);
            } else if (AdrNum != 0) { /* FMOVE.x <ea>,FPn ? */
               CodeLen = 4 + AdrCnt;
               CopyAdrVals(WAsmCode + 2);
               WAsmCode[0] |= AdrMode;
               WAsmCode[1] |= 0x4000 | (((Word) SizeCodes[OpSize]) << 10);
            }
         } else if (AdrNum == 13) { /* FMOVE.L <ea>,FPcr ? */
            if ((OpSize != 2) && (*AttrPart != '\0')) WrError(1130);
            else {
               RelPos = 4;
               WAsmCode[0] = 0xf200;
               WAsmCode[1] = 0x8000 | (AdrMode << 10);
               DecodeAdr(ArgStr[1], (AdrMode == 1) ? Mdata + Madr + Madri + Mpost + Mpre + Mdadri + Mpc + Mpcidx + Mabs + Mimm : Mdata + Madri + Mpost + Mpre + Mdadri + Mpc + Mpcidx + Mabs + Mimm);
               if (AdrNum != 0) {
                  WAsmCode[0] |= AdrMode;
                  CodeLen = 4 + AdrCnt;
                  CopyAdrVals(WAsmCode + 2);
               }
            }
         } else if (AdrNum != 0) { /* FMOVE.x ????,<ea> ? */
            WAsmCode[0] = 0xf200 | AdrMode;
            CodeLen = 4 + AdrCnt;
            CopyAdrVals(WAsmCode + 2);
            DecodeAdr(ArgStr[1], (AdrMode == 1) ? Mfpcr : Mfpn + Mfpcr);
            if (AdrNum == 12) { /* FMOVE.x FPn,<ea> ? */
               if (*AttrPart == '\0') OpSize = 6;
               WAsmCode[1] = 0x6000 | (((Word) SizeCodes[OpSize]) << 10) | (AdrMode << 7);
               if (OpSize != 7) ;
               else if (strlen(sk) > 2) {
                  OpSize = 0;
                  strmove(sk, 1);
                  sk[strlen(sk) - 1] = '\0';
                  DecodeAdr(sk, Mdata + Mimm);
                  if (AdrNum == 1) WAsmCode[1] |= (AdrMode << 4) | 0x1000;
                  else if (AdrNum == 11) WAsmCode[1] |= (AdrVals[0] & 127);
                  else CodeLen = 0;
               } else WAsmCode[1] |= 17;
            } else if (AdrNum == 13) { /* FMOVE.L FPcr,<ea> ? */
               if ((*AttrPart != '\0') && (OpSize != 2)) {
                  WrError(1130);
                  CodeLen = 0;
               } else {
                  WAsmCode[1] = 0xa000 | (AdrMode << 10);
                  if ((AdrMode != 1) && ((WAsmCode[0] & 0x38) == 8)) {
                     WrError(1350);
                     CodeLen = 0;
                  }
               }
            } else CodeLen = 0;
         }
      }
      return;
   }

   if (Memo("MOVECR")) {
      if (ArgCnt != 2) WrError(1110);
      else if ((*AttrPart != '\0') && (OpSize != 6)) WrError(1130);
      else {
         DecodeAdr(ArgStr[2], Mfpn);
         if (AdrNum == 12) {
            WAsmCode[0] = 0xf200;
            WAsmCode[1] = 0x5c00 | (AdrMode << 7);
            OpSize = 0;
            DecodeAdr(ArgStr[1], Mimm);
            if (AdrNum != 11) ;
            else if (AdrVals[0] > 63) WrError(1700);
            else {
               CodeLen = 4;
               WAsmCode[1] |= AdrVals[0];
            }
         }
      }
      return;
   }

   if (Memo("TST")) {
      if (*AttrPart == '\0') OpSize = 6;
      else if (OpSize == 3) WrError(1130);
      else if (ArgCnt != 1) WrError(1110);
      else {
         RelPos = 4;
         DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm + Mfpn);
         if (AdrNum == 12) {
            WAsmCode[0] = 0xf200;
            WAsmCode[1] = 0x3a | (AdrMode << 10);
            CodeLen = 4;
         } else if (AdrNum != 0) {
            WAsmCode[0] = 0xf200 | AdrMode;
            WAsmCode[1] = 0x403a | (((Word) SizeCodes[OpSize]) << 10);
            CodeLen = 4 + AdrCnt;
            CopyAdrVals(WAsmCode + 2);
         }
      }
      return;
   }

   if (Memo("SINCOS")) {
      if (*AttrPart == '\0') OpSize = 6;
      if (OpSize == 3) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         p = strrchr(ArgStr[2], ':');
         if (p != NULL) {
            *p = '\0';
            strcopy(sk, ArgStr[2]);
            strcopy(ArgStr[2], p + 1);
         } else *sk = '\0';
         DecodeAdr(sk, Mfpn);
         if (AdrNum == 12) {
            WAsmCode[1] = AdrMode | 0x30;
            DecodeAdr(ArgStr[2], Mfpn);
            if (AdrNum == 12) {
               WAsmCode[1] |= (AdrMode << 7);
               RelPos = 4;
               DecodeAdr(ArgStr[1], ((OpSize <= 2) || (OpSize == 4)) ? Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm + Mfpn : Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm + Mfpn);
               if (AdrNum == 12) {
                  WAsmCode[0] = 0xf200;
                  WAsmCode[1] |= (AdrMode << 10);
                  CodeLen = 4;
               } else if (AdrNum != 0) {
                  WAsmCode[0] = 0xf200 | AdrMode;
                  WAsmCode[1] |= 0x4000 | (((Word) SizeCodes[OpSize]) << 10);
                  CodeLen = 4 + AdrCnt;
                  CopyAdrVals(WAsmCode + 2);
               }
            }
         }
      }
      return;
   }

   if (*OpPart == 'B') {
      for (z = 0; z < FPUCondCnt; z++)
         if (strcmp(OpPart + 1, FPUConds[z].Name) == 0) break;
      if (z >= FPUCondCnt) WrError(1360);
      else {
         if ((OpSize != 1) && (OpSize != 2) && (OpSize != 6)) WrError(1130);
         else if (ArgCnt != 1) WrError(1110);
         else {
            HVal = EvalIntExpression(ArgStr[1], Int32, &ValOK) - (EProgCounter() + 2);
            HVal16 = HVal;

            if (OpSize == 1) {
               OpSize = (IsDisp16(HVal)) ? 2 : 6;
            }

            if (OpSize == 2) {
               if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(1370);
               else {
                  CodeLen = 4;
                  WAsmCode[0] = 0xf280 | FPUConds[z].Code;
                  WAsmCode[1] = HVal16;
               }
            } else {
               CodeLen = 6;
               WAsmCode[0] = 0xf2c0 | FPUConds[z].Code;
               WAsmCode[2] = HVal & 0xffff;
               WAsmCode[1] = HVal >> 16;
               if ((IsDisp16(HVal)) && (PassNo > 1) && (*AttrPart == '\0')) {
                  WrError(20);
                  WAsmCode[0] ^= 0x40;
                  CodeLen -= 2;
                  WAsmCode[1] = WAsmCode[2];
                  StopfZahl++;
               }
            }
         }
      }
      return;
   }

   if (strncmp(OpPart, "DB", 2) == 0) {
      for (z = 0; z < FPUCondCnt; z++)
         if (strcmp(OpPart + 2, FPUConds[z].Name) == 0) break;
      if (z >= FPUCondCnt) WrError(1360);
      else {
         if ((OpSize != 1) && (*AttrPart != '\0')) WrError(1130);
         else if (ArgCnt != 2) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], Mdata);
            if (AdrNum != 0) {
               WAsmCode[0] = 0xf248 | AdrMode;
               WAsmCode[1] = FPUConds[z].Code;
               HVal = EvalIntExpression(ArgStr[2], Int32, &ValOK) - (EProgCounter() + 4);
               if (ValOK) {
                  HVal16 = HVal;
                  WAsmCode[2] = HVal16;
                  if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(1370);
                  else CodeLen = 6;
               }
            }
         }
      }
      return;
   }

   if (*OpPart == 'S') {
      for (z = 0; z < FPUCondCnt; z++)
         if (strcmp(OpPart + 1, FPUConds[z].Name) == 0) break;
      if (z >= FPUCondCnt) WrError(1360);
      else {
         if ((OpSize != 0) && (*AttrPart != '\0')) WrError(1130);
         else if (ArgCnt != 1) WrError(1110);
         else {
            DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
            if (AdrNum != 0) {
               CodeLen = 4 + AdrCnt;
               WAsmCode[0] = 0xf240 | AdrMode;
               WAsmCode[1] = FPUConds[z].Code;
               CopyAdrVals(WAsmCode + 2);
            }
         }
      }
      return;
   }

   if (strncmp(OpPart, "TRAP", 4) == 0) {
      for (z = 0; z < FPUCondCnt; z++)
         if (strcmp(OpPart + 4, FPUConds[z].Name) == 0) break;
      if (z >= FPUCondCnt) WrError(1360);
      else {
         if (*AttrPart == '\0') OpSize = 0;
         if (OpSize > 2) WrError(1130);
         else if (((OpSize == 0) && (ArgCnt != 0)) || ((OpSize != 0) && (ArgCnt != 1))) WrError(1110);
         else {
            WAsmCode[0] = 0xf278;
            WAsmCode[1] = FPUConds[z].Code;
            if (OpSize == 0) {
               WAsmCode[0] |= 4;
               CodeLen = 4;
            } else {
               DecodeAdr(ArgStr[1], Mimm);
               if (AdrNum != 0) {
                  WAsmCode[0] |= (OpSize + 1);
                  CopyAdrVals(WAsmCode + 2);
                  CodeLen = 4 + AdrCnt;
               }
            }
         }
      }
      return;
   }

   if (Memo("MOVEM")) {
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeFRegList(ArgStr[2], &z1, &z2);
         if (z1 != 0) {
            if ((*AttrPart != '\0') && (((z1 < 3) && (OpSize != 6)) || ((z1 == 3) && (OpSize != 2))))
               WrError(1130);
            else {
               RelPos = 4;
               DecodeAdr(ArgStr[1], Madri + Mpost + (z1 == 3 ? 0 : Mpre) + Mdadri + Maix + Mpc + Mpcidx + Mabs);
               WAsmCode[1] = 0;
               GenerateMovem(z1, z2);
            }
         } else {
            DecodeFRegList(ArgStr[1], &z1, &z2);
            if (z1 != 0) {
               if ((*AttrPart != '\0') && (((z1 < 3) && (OpSize != 6)) || ((z1 == 3) && (OpSize != 2))))
                  WrError(1130);
               else {
                  DecodeAdr(ArgStr[2], Madri + Mpost + (z1 == 3 ? 0 : Mpre) + Mdadri + Maix + Mabs);
                  WAsmCode[1] = 0x2000;
                  GenerateMovem(z1, z2);
               }
            } else WrError(1410);
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

/*-------------------------------------------------------------------------*/

static bool DecodeFC(char *Asc, Word * erg) {
   bool OK;
   Word Val;
   String Asc_N;

   strmaxcpy(Asc_N, Asc, 255);
   NLS_UpString(Asc_N);
   Asc = Asc_N;

   if (strcmp(Asc, "SFC") == 0) {
      *erg = 0;
      return true;
   }

   if (strcmp(Asc, "DFC") == 0) {
      *erg = 1;
      return true;
   }

   if ((strlen(Asc) == 2) && (*Asc == 'D') && ValReg(Asc[1])) {
      *erg = Asc[2] - '0' + 8;
      return true;
   }

   if (*Asc == '#') {
      Val = EvalIntExpression(Asc + 1, Int4, &OK);
      if (OK) *erg = Val + 16;
      return OK;
   }

   return false;
}

static bool DecodePMMUReg(char *Asc, Word * erg, Byte * Size) {
   Byte z;

   if ((strlen(Asc) == 4) && (strncasecmp(Asc, "BAD", 3) == 0) && ValReg(Asc[3])) {
      *Size = 1;
      *erg = 0x7000 + ((Asc[3] - '0') << 2);
      return true;
   }
   if ((strlen(Asc) == 4) && (strncasecmp(Asc, "BAC", 3) == 0) && ValReg(Asc[3])) {
      *Size = 1;
      *erg = 0x7400 + ((Asc[3] - '0') << 2);
      return true;
   }

   for (z = 0; z < PMMURegCnt; z++)
      if (strcasecmp(Asc, PMMURegNames[z]) == 0) break;
   if (z < PMMURegCnt) {
      *Size = PMMURegSizes[z];
      *erg = PMMURegCodes[z] << 10;
   }
   return (z < PMMURegCnt);
}

static void DecodePMMUOrders(void) {
   Byte z;
   Word Mask;
   LongInt HVal;
   Integer HVal16;
   bool ValOK;

   if (Memo("SAVE")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1130);
      else if (!FullPMMU) WrError(1500);
      else {
         DecodeAdr(ArgStr[1], Madri + Mpre + Mdadri + Maix + Mabs);
         if (AdrNum != 0) {
            CodeLen = 2 + AdrCnt;
            WAsmCode[0] = 0xf100 | AdrMode;
            CopyAdrVals(WAsmCode + 1);
            CheckSup();
         }
      }
      return;
   }

   if (Memo("RESTORE")) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1130);
      else if (!FullPMMU) WrError(1500);
      else {
         DecodeAdr(ArgStr[1], Madri + Mpre + Mdadri + Maix + Mabs);
         if (AdrNum != 0) {
            CodeLen = 2 + AdrCnt;
            WAsmCode[0] = 0xf140 | AdrMode;
            CopyAdrVals(WAsmCode + 1);
            CheckSup();
         }
      }
      return;
   }

   if (Memo("FLUSHA")) {
      if (*AttrPart != '\0') WrError(1130);
      else if (ArgCnt != 0) WrError(1110);
      else {
         CodeLen = 4;
         WAsmCode[0] = 0xf000;
         WAsmCode[1] = 0x2400;
         CheckSup();
      }
      return;
   }

   if ((Memo("FLUSH")) || (Memo("FLUSHS"))) {
      if (*AttrPart != '\0') WrError(1130);
      else if ((ArgCnt != 2) && (ArgCnt != 3)) WrError(1110);
      else if ((Memo("FLUSHS")) && (!FullPMMU)) WrError(1500);
      else if (!DecodeFC(ArgStr[1], WAsmCode + 1)) WrError(1710);
      else {
         OpSize = 0;
         DecodeAdr(ArgStr[2], Mimm);
         if (AdrNum != 0) {
            if (AdrVals[0] > 15) WrError(1720);
            else {
               WAsmCode[1] |= (AdrVals[0] << 5) | 0x3000;
               if (Memo("FLUSHS")) WAsmCode[1] |= 0x400;
               WAsmCode[0] = 0xf000;
               CodeLen = 4;
               CheckSup();
               if (ArgCnt == 3) {
                  WAsmCode[1] |= 0x800;
                  DecodeAdr(ArgStr[3], Madri + Mdadri + Maix + Mabs);
                  if (AdrNum == 0) CodeLen = 0;
                  else {
                     WAsmCode[0] |= AdrMode;
                     CodeLen += AdrCnt;
                     CopyAdrVals(WAsmCode + 2);
                  }
               }
            }
         }
      }
      return;
   }

   if (Memo("FLUSHR")) {
      if (*AttrPart == '\0') OpSize = 3;
      if (OpSize != 3) WrError(1130);
      else if (ArgCnt != 1) WrError(1110);
      else if (!FullPMMU) WrError(1500);
      else {
         RelPos = 4;
         DecodeAdr(ArgStr[1], Madri + Mpre + Mpost + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm);
         if (AdrNum != 0) {
            WAsmCode[0] = 0xf000 | AdrMode;
            WAsmCode[1] = 0xa000;
            CopyAdrVals(WAsmCode + 2);
            CodeLen = 4 + AdrCnt;
            CheckSup();
         }
      }
      return;
   }

   if ((Memo("LOADR")) || (Memo("LOADW"))) {
      if (*AttrPart != '\0') WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else if (!DecodeFC(ArgStr[1], WAsmCode + 1)) WrError(1710);
      else {
         DecodeAdr(ArgStr[2], Madri + Mdadri + Maix + Mabs);
         if (AdrNum != 0) {
            WAsmCode[0] = 0xf000 | AdrMode;
            WAsmCode[1] |= 0x2000;
            if (Memo("LOADR")) WAsmCode[1] |= 0x200;
            CodeLen = 4 + AdrCnt;
            CopyAdrVals(WAsmCode + 2);
            CheckSup();
         }
      }
      return;
   }

   if ((Memo("MOVE")) || (Memo("MOVEFD"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         if (DecodePMMUReg(ArgStr[1], WAsmCode + 1, &z)) {
            WAsmCode[1] |= 0x200;
            if (*AttrPart == '\0') OpSize = z;
            if (OpSize != z) WrError(1130);
            else {
               Mask = Madri + Mdadri + Maix + Mabs;
               if (FullPMMU) {
                  Mask *= Mpost + Mpre;
                  if (z != 3) Mask += Mdata + Madr;
               }
               DecodeAdr(ArgStr[2], Mask);
               if (AdrNum != 0) {
                  WAsmCode[0] = 0xf000 | AdrMode;
                  CodeLen = 4 + AdrCnt;
                  CopyAdrVals(WAsmCode + 2);
                  CheckSup();
               }
            }
         } else if (DecodePMMUReg(ArgStr[2], WAsmCode + 1, &z)) {
            if (*AttrPart == '\0') OpSize = z;
            if (OpSize != z) WrError(1130);
            else {
               RelPos = 4;
               Mask = Madri + Mdadri + Maix + Mabs;
               if (FullPMMU) {
                  Mask += Mpost + Mpre + Mpc + Mpcidx + Mimm;
                  if (z != 3) Mask += Mdata + Madr;
               }
               DecodeAdr(ArgStr[1], Mask);
               if (AdrNum != 0) {
                  WAsmCode[0] = 0xf000 | AdrMode;
                  CodeLen = 4 + AdrCnt;
                  CopyAdrVals(WAsmCode + 2);
                  if (Memo("MOVEFD")) WAsmCode[1] += 0x100;
                  CheckSup();
               }
            }
         } else WrError(1730);
      }
      return;
   }

   if ((Memo("TESTR")) || (Memo("TESTW"))) {
      if (*AttrPart != '\0') WrError(1130);
      else if ((ArgCnt > 4) || (ArgCnt < 3)) WrError(1110);
      else {
         if (!DecodeFC(ArgStr[1], WAsmCode + 1)) WrError(1710);
         else {
            DecodeAdr(ArgStr[2], Madri + Mdadri + Maix + Mabs);
            if (AdrNum != 0) {
               WAsmCode[0] = 0xf000 | AdrMode;
               CodeLen = 4 + AdrCnt;
               WAsmCode[1] |= 0x8000;
               CopyAdrVals(WAsmCode + 2);
               if (Memo("TESTR")) WAsmCode[1] |= 0x200;
               DecodeAdr(ArgStr[3], Mimm);
               if (AdrNum != 0)
                  if (AdrVals[0] > 7) {
                     WrError(1740);
                     CodeLen = 0;
                  } else {
                     WAsmCode[1] |= AdrVals[0] << 10;
                     if (ArgCnt == 4) {
                        DecodeAdr(ArgStr[4], Madr);
                        if (AdrNum == 0) CodeLen = 0;
                        else WAsmCode[1] |= AdrMode << 5;
                        CheckSup();
                     }
               } else CodeLen = 0;
            }
         }
      }
      return;
   }

   if (Memo("VALID")) {
      if (ArgCnt != 2) WrError(1110);
      else if (!FullPMMU) WrError(1500);
      else if ((*AttrPart != '\0') && (OpSize != 2)) WrError(1130);
      else {
         DecodeAdr(ArgStr[2], Madri + Mdadri + Maix + Mabs);
         if (AdrNum != 0) {
            WAsmCode[0] = 0xf000 | AdrMode;
            WAsmCode[1] = 0x2800;
            CodeLen = 4 + AdrCnt;
            CopyAdrVals(WAsmCode + 1);
            if (strcasecmp(ArgStr[1], "VAL") == 0);
            else {
               DecodeAdr(ArgStr[1], Madr);
               if (AdrNum != 0) {
                  WAsmCode[1] |= 0x400 | (AdrMode & 7);
               } else CodeLen = 0;
            }
         }
      }
      return;
   }

   if (*OpPart == 'B') {
      for (z = 0; z < PMMUCondCnt; z++)
         if (strcmp(OpPart + 1, PMMUConds[z]) == 0) break;
      if (z == PMMUCondCnt) WrError(1360);
      else {
         if ((OpSize != 1) && (OpSize != 2) && (OpSize != 6)) WrError(1130);
         else if (ArgCnt != 1) WrError(1110);
         else if (!FullPMMU) WrError(1500);
         else {

            HVal = EvalIntExpression(ArgStr[1], Int32, &ValOK) - (EProgCounter() + 2);
            HVal16 = HVal;

            if (OpSize == 1) OpSize = (IsDisp16(HVal)) ? 2 : 6;

            if (OpSize == 2) {
               if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(1370);
               else {
                  CodeLen = 4;
                  WAsmCode[0] = 0xf080 | z;
                  WAsmCode[1] = HVal16;
                  CheckSup();
               }
            } else {
               CodeLen = 6;
               WAsmCode[0] = 0xf0c0 | z;
               WAsmCode[2] = HVal & 0xffff;
               WAsmCode[1] = HVal >> 16;
               CheckSup();
            }
         }
      }
      return;
   }

   if (strncmp(OpPart, "DB", 2) == 0) {
      for (z = 0; z < PMMUCondCnt; z++)
         if (strcmp(OpPart + 2, PMMUConds[z]) == 0) break;
      if (z == PMMUCondCnt) WrError(1360);
      else {
         if ((OpSize != 1) && (*AttrPart != '\0')) WrError(1130);
         else if (ArgCnt != 2) WrError(1110);
         else if (!FullPMMU) WrError(1500);
         else {
            DecodeAdr(ArgStr[1], Mdata);
            if (AdrNum != 0) {
               WAsmCode[0] = 0xf048 | AdrMode;
               WAsmCode[1] = z;
               HVal = EvalIntExpression(ArgStr[2], Int32, &ValOK) - (EProgCounter() + 4);
               if (ValOK) {
                  HVal16 = HVal;
                  WAsmCode[2] = HVal16;
                  if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(1370);
                  else CodeLen = 6;
                  CheckSup();
               }
            }
         }
      }
      return;
   }

   if (*OpPart == 'S') {
      for (z = 0; z < PMMUCondCnt; z++)
         if (strcmp(OpPart + 1, PMMUConds[z]) == 0) break;
      if (z == PMMUCondCnt) WrError(1360);
      else {
         if ((OpSize != 0) && (*AttrPart != '\0')) WrError(1130);
         else if (ArgCnt != 1) WrError(1110);
         else if (!FullPMMU) WrError(1500);
         else {
            DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
            if (AdrNum != 0) {
               CodeLen = 4 + AdrCnt;
               WAsmCode[0] = 0xf040 | AdrMode;
               WAsmCode[1] = z;
               CopyAdrVals(WAsmCode + 2);
               CheckSup();
            }
         }
      }
      return;
   }

   if (strncmp(OpPart, "TRAP", 4) == 0) {
      for (z = 0; z < PMMUCondCnt; z++)
         if (strcmp(OpPart + 4, PMMUConds[z]) == 0) break;
      if (z == PMMUCondCnt) WrError(1360);
      else {
         if (*AttrPart == '\0') OpSize = 0;
         if (OpSize > 2) WrError(1130);
         else if (((OpSize == 0) && (ArgCnt != 0)) || ((OpSize != 0) && (ArgCnt != 1))) WrError(1110);
         else if (!FullPMMU) WrError(1500);
         else {
            WAsmCode[0] = 0xf078;
            WAsmCode[1] = z;
            if (OpSize == 0) {
               WAsmCode[0] |= 4;
               CodeLen = 4;
               CheckSup();
            } else {
               DecodeAdr(ArgStr[1], Mimm);
               if (AdrNum != 0) {
                  WAsmCode[0] |= (OpSize + 1);
                  CopyAdrVals(WAsmCode + 2);
                  CodeLen = 4 + AdrCnt;
                  CheckSup();
               }
            }
         }
      }
      return;
   }

   WrError(1200);
}

/*-------------------------------------------------------------------------*/

static bool CodeSingle(void) {
   if (Memo("PEA")) {
      if ((*AttrPart != '\0') && (OpSize != 2)) WrError(1100);
      else if (ArgCnt != 1) WrError(1110);
      else {
         OpSize = 0;
         DecodeAdr(ArgStr[1], Madri + Mdadri + Maix + Mpc + Mpcidx + Mabs);
         if (AdrNum != 0) {
            CodeLen = 2 + AdrCnt;
            WAsmCode[0] = 0x4840 | AdrMode;
            CopyAdrVals(WAsmCode + 1);
         }
      }
      return true;
   }

   if ((Memo("CLR")) || (Memo("TST"))) {
      if (OpSize > 2) WrError(1130);
      else if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
         if (AdrNum != 0) {
            CodeLen = 2 + AdrCnt;
            WAsmCode[0] = (Memo("TST")) ? 0x4a00 : 0x4200;
            WAsmCode[0] |= ((OpSize << 6) | AdrMode);
            CopyAdrVals(WAsmCode + 1);
         }
      }
      return true;
   }

   if ((Memo("JMP")) || (Memo("JSR"))) {
      if (*AttrPart != '\0') WrError(1130);
      else if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Madri + Mdadri + Maix + Mpc + Mpcidx + Mabs);
         if (AdrNum != 0) {
            CodeLen = 2 + AdrCnt;
            WAsmCode[0] = (Memo("JMP")) ? 0x4ec0 : 0x4e80;
            WAsmCode[0] |= AdrMode;
            CopyAdrVals(WAsmCode + 1);
         }
      }
      return true;
   }

   if ((Memo("NBCD")) || (Memo("TAS"))) {
      if ((*AttrPart != '\0') && (OpSize != 0)) WrError(1130);
      else if (ArgCnt != 1) WrError(1110);
      else {
         OpSize = 0;
         DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
         if (AdrNum != 0) {
            CodeLen = 2 + AdrCnt;
            WAsmCode[0] = (Memo("NBCD")) ? 0x4800 : 0x4ac0;
            WAsmCode[0] |= AdrMode;
            CopyAdrVals(WAsmCode + 1);
         }
      }
      return true;
   }

   if ((Memo("NEG")) || (Memo("NOT")) || (Memo("NEGX"))) {
      if (OpSize > 2) WrError(1130);
      else if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
         if (AdrNum != 0) {
            CodeLen = 2 + AdrCnt;
            if (Memo("NOT")) WAsmCode[0] = 0x4600;
            else WAsmCode[0] = (Memo("NEG")) ? 0x4400 : 0x4000;
            WAsmCode[0] |= ((OpSize << 6) | AdrMode);
            CopyAdrVals(WAsmCode + 1);
         }
      }
      return true;
   }

   if (Memo("SWAP")) {
      if ((*AttrPart != '\0') && (OpSize != 2)) WrError(1130);
      else if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Mdata);
         if (AdrNum != 0) {
            CodeLen = 2;
            WAsmCode[0] = 0x4840 | AdrMode;
         }
      }
      return true;
   }

   if (Memo("UNLK")) {
      if (*AttrPart != '\0') WrError(1130);
      else if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Madr);
         if (AdrNum != 0) {
            CodeLen = 2;
            WAsmCode[0] = 0x4e58 | AdrMode;
         }
      }
      return true;
   }

   if (Memo("EXT")) {
      if (ArgCnt != 1) WrError(1110);
      else if ((OpSize == 0) || (OpSize > 2)) WrError(1130);
      else {
         DecodeAdr(ArgStr[1], Mdata);
         if (AdrNum == 1) {
            WAsmCode[0] = 0x4880 | AdrMode | (((Word) OpSize - 1) << 6);
            CodeLen = 2;
         }
      }
      return true;
   }

   return false;
}

static bool CodeDual(void) {
   bool ValOK;
   ShortInt HVal8;

   if ((Memo("MOVE")) || (Memo("MOVEA"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (strcasecmp(ArgStr[1], "USP") == 0) {
         if ((*AttrPart != '\0') && (OpSize != 2)) WrError(1130);
         else {
            DecodeAdr(ArgStr[2], Madr);
            if (AdrNum != 0) {
               CodeLen = 2;
               WAsmCode[0] = 0x4e68 | (AdrMode & 7);
               CheckSup();
            }
         }
      } else if (strcasecmp(ArgStr[2], "USP") == 0) {
         if ((*AttrPart != '\0') && (OpSize != 2)) WrError(1130);
         else {
            DecodeAdr(ArgStr[1], Madr);
            if (AdrNum != 0) {
               CodeLen = 2;
               WAsmCode[0] = 0x4e60 | (AdrMode & 7);
               CheckSup();
            }
         }
      } else if (strcasecmp(ArgStr[1], "SR") == 0) {
         if (OpSize != 1) WrError(1130);
         else {
            DecodeAdr(ArgStr[2], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
            if (AdrNum != 0) {
               CodeLen = 2 + AdrCnt;
               WAsmCode[0] = 0x40c0 | AdrMode;
               CopyAdrVals(WAsmCode + 1);
               if (MomCPU >= CPU68010) CheckSup();
            }
         }
      } else if (strcasecmp(ArgStr[1], "CCR") == 0) {
         if ((*AttrPart != '\0') && (OpSize != 0)) WrError(1130);
         else {
            OpSize = 0;
            DecodeAdr(ArgStr[2], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
            if (AdrNum != 0) {
               CodeLen = 2 + AdrCnt;
               WAsmCode[0] = 0x42c0 | AdrMode;
               CopyAdrVals(WAsmCode + 1);
               CheckCPU(CPU68010);
            }
         }
      } else if (strcasecmp(ArgStr[2], "SR") == 0) {
         if (OpSize != 1) WrError(1130);
         else {
            DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm);
            if (AdrNum != 0) {
               CodeLen = 2 + AdrCnt;
               WAsmCode[0] = 0x46c0 | AdrMode;
               CopyAdrVals(WAsmCode + 1);
               CheckSup();
            }
         }
      } else if (strcasecmp(ArgStr[2], "CCR") == 0) {
         if ((*AttrPart != '\0') && (OpSize != 0)) WrError(1130);
         else {
            OpSize = 0;
            DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm);
            if (AdrNum != 0) {
               CodeLen = 2 + AdrCnt;
               WAsmCode[0] = 0x44c0 | AdrMode;
               CopyAdrVals(WAsmCode + 1);
            }
         }
      } else {
         if (OpSize > 2) WrError(1130);
         else {
            DecodeAdr(ArgStr[1], Mdata + Madr + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm);
            if (AdrNum != 0) {
               CodeLen = 2 + AdrCnt;
               CopyAdrVals(WAsmCode + 1);
               if (OpSize == 0) WAsmCode[0] = 0x1000;
               else if (OpSize == 1) WAsmCode[0] = 0x3000;
               else WAsmCode[0] = 0x2000;
               WAsmCode[0] |= AdrMode;
               DecodeAdr(ArgStr[2], Mdata + Madr + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
               if (AdrMode != 0) {
                  AdrMode = ((AdrMode & 7) << 3) | (AdrMode >> 3);
                  WAsmCode[0] |= AdrMode << 6;
                  CopyAdrVals(WAsmCode + (CodeLen >> 1));
                  CodeLen += AdrCnt;
               }
            }
         }
      }
      return true;
   }

   if (Memo("LEA")) {
      if ((*AttrPart != '\0') && (OpSize != 2)) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], Madr);
         if (AdrNum != 0) {
            OpSize = 0;
            WAsmCode[0] = 0x41c0 | ((AdrMode & 7) << 9);
            DecodeAdr(ArgStr[1], Madri + Mdadri + Maix + Mpc + Mpcidx + Mabs);
            if (AdrNum != 0) {
               WAsmCode[0] |= AdrMode;
               CodeLen = 2 + AdrCnt;
               CopyAdrVals(WAsmCode + 1);
            }
         }
      }
      return true;
   }

   if ((Memo("ASL")) || (Memo("ASR")) || (Memo("LSL")) || (Memo("LSR"))
      || (Memo("ROL")) || (Memo("ROR")) || (Memo("ROXL")) || (Memo("ROXR"))) {
      if (ArgCnt == 1) {
         strcopy(ArgStr[2], ArgStr[1]);
         strcpy(ArgStr[1], "#1");
         ArgCnt = 2;
      }
      if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
         if (AdrNum == 1) {
            if (OpSize > 2) WrError(1130);
            else {
               WAsmCode[0] = 0xe000 | AdrMode | (ShiftCodes(OpPart) << 3) | (OpSize << 6);
               if (OpPart[strlen(OpPart) - 1] == 'L') WAsmCode[0] |= 0x100;
               OpSize = 1;
               DecodeAdr(ArgStr[1], Mdata + Mimm);
               if ((AdrNum == 1) || ((AdrNum == 11) && (Lo(AdrVals[0]) >= 1) && (Lo(AdrVals[0]) <= 8))) {
                  CodeLen = 2;
                  WAsmCode[0] |= (AdrNum == 1) ? 0x20 | (AdrMode << 9) : ((AdrVals[0] & 7) << 9);
               } else WrError(1380);
            }
         } else if (AdrNum != 0) {
            if (OpSize != 1) WrError(1130);
            else {
               WAsmCode[0] = 0xe0c0 | AdrMode | (ShiftCodes(OpPart) << 9);
               if (OpPart[strlen(OpPart) - 1] == 'L') WAsmCode[0] |= 0x100;
               CopyAdrVals(WAsmCode + 1);
               if (*ArgStr[1] == '#') strmove(ArgStr[1], 1);
               HVal8 = EvalIntExpression(ArgStr[1], Int8, &ValOK);
               if ((ValOK) && (HVal8 == 1)) CodeLen = 2 + AdrCnt;
               else WrError(1390);
            }
         }
      }
      return true;
   }

   if ((Memo("ADDQ")) || (Memo("SUBQ"))) {
      if (OpSize > 2) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], Mdata + Madr + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
         if (AdrNum != 0) {
            WAsmCode[0] = 0x5000 | AdrMode | (OpSize << 6);
            if (Memo("SUBQ")) WAsmCode[0] |= 0x100;
            CopyAdrVals(WAsmCode + 1);
            if (*ArgStr[1] == '#') strmove(ArgStr[1], 1);
            HVal8 = EvalIntExpression(ArgStr[1], Int8, &ValOK);
            if ((ValOK) && (HVal8 >= 1) && (HVal8 <= 8)) {
               CodeLen = 2 + AdrCnt;
               WAsmCode[0] |= (((Word) HVal8 & 7) << 9);
            } else WrError(1390);
         }
      }
      return true;
   }

   if ((Memo("ADDX")) || (Memo("SUBX"))) {
      if (OpSize > 2) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Mdata + Mpre);
         if (AdrNum != 0) {
            WAsmCode[0] = 0x9100 | (OpSize << 6) || (AdrMode & 7);
            if (AdrNum == 5) WAsmCode[0] |= 8;
            if (strcmp(OpPart, "ADDX") == 0) WAsmCode[0] |= 0x4000;
            DecodeAdr(ArgStr[2], Masks[AdrNum]);
            if (AdrNum != 0) {
               CodeLen = 2;
               WAsmCode[0] |= (AdrMode & 7) << 9;
            }
         }
      }
      return true;
   }

   if (Memo("CMPM")) {
      if (OpSize > 2) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Mpost);
         if (AdrNum == 4) {
            WAsmCode[0] = 0xb108 | (OpSize << 6) | (AdrMode & 7);
            DecodeAdr(ArgStr[2], Mpost);
            if (AdrNum == 4) {
               WAsmCode[0] |= (AdrMode & 7) << 9;
               CodeLen = 2;
            }
         }
      }
      return true;
   }

   if ((strncmp(OpPart, "ADD", 3) == 0) || (strncmp(OpPart, "SUB", 3) == 0) || ((strncmp(OpPart, "CMP", 3) == 0) && (OpPart[3] != '2'))) {
      OpPart[3] = '\0';
      if (OpSize > 2) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], Mdata + Madr + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
         if (AdrNum == 2) /* ADDA ? */
            if (OpSize == 0) WrError(1130);
            else {
               WAsmCode[0] = 0x90c0 | ((AdrMode & 7) << 9);
               if (strcmp(OpPart, "ADD") == 0) WAsmCode[0] |= 0x4000;
               else if (strcmp(OpPart, "CMP") == 0) WAsmCode[0] |= 0x2000;
               if (OpSize == 2) WAsmCode[0] |= 0x100;
               DecodeAdr(ArgStr[1], Mdata + Madr + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm);
               if (AdrNum != 0) {
                  WAsmCode[0] |= AdrMode;
                  CodeLen = 2 + AdrCnt;
                  CopyAdrVals(WAsmCode + 1);
               }
         } else if (AdrNum == 1) { /* ADD <EA>,Dn ? */
            WAsmCode[0] = 0x9000 | (OpSize << 6) | (AdrMode << 9);
            if (strcmp(OpPart, "ADD") == 0) WAsmCode[0] |= 0x4000;
            else if (strcmp(OpPart, "CMP") == 0) WAsmCode[0] |= 0x2000;
            DecodeAdr(ArgStr[1], Mdata + Madr + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm);
            if (AdrNum != 0) {
               CodeLen = 2 + AdrCnt;
               CopyAdrVals(WAsmCode + 1);
               WAsmCode[0] |= AdrMode;
            }
         } else {
            DecodeAdr(ArgStr[1], Mdata + Mimm);
            if (AdrNum == 11) { /* ADDI ? */
               WAsmCode[0] = 0x400 | (OpSize << 6);
               if (strcmp(OpPart, "ADD") == 0) WAsmCode[0] |= 0x200;
               else if (strcmp(OpPart, "CMP") == 0) WAsmCode[0] |= 0x800;
               CodeLen = 2 + AdrCnt;
               CopyAdrVals(WAsmCode + 1);
               DecodeAdr(ArgStr[2], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
               if (AdrNum != 0) {
                  WAsmCode[0] |= AdrMode;
                  CopyAdrVals(WAsmCode + (CodeLen >> 1));
                  CodeLen += AdrCnt;
               } else CodeLen = 0;
            } else if (AdrNum != 0) { /* ADD Dn,<EA> ? */
               if (strcmp(OpPart, "CMP") == 0) WrError(1420);
               else {
                  WAsmCode[0] = 0x9100 | (OpSize << 6) | (AdrMode << 9);
                  if (strcmp(OpPart, "ADD") == 0) WAsmCode[0] |= 0x4000;
                  DecodeAdr(ArgStr[2], Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
                  if (AdrNum != 0) {
                     CodeLen = 2 + AdrCnt;
                     CopyAdrVals(WAsmCode + 1);
                     WAsmCode[0] |= AdrMode;
                  }
               }
            }
         }
      }
      return true;
   }

   if ((strncmp(OpPart, "AND", 3) == 0) || (strncmp(OpPart, "OR", 2) == 0)) {
      if (ArgCnt != 2) WrError(1110);
      else if (OpSize > 2) WrError(1130);
      else {
         if ((strcasecmp(ArgStr[2], "CCR") != 0) && (strcasecmp(ArgStr[2], "SR") != 0))
            DecodeAdr(ArgStr[2], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
         if (strcasecmp(ArgStr[2], "CCR") == 0) { /* && #...,CCR */
            if ((*AttrPart != '\0') && (OpSize != 0)) WrError(1130);
            else {
               WAsmCode[0] = (strncmp(OpPart, "AND", 3) == 0) ? 0x023c : 0x003c;
               OpSize = 0;
               DecodeAdr(ArgStr[1], Mimm);
               if (AdrNum != 0) {
                  CodeLen = 4;
                  WAsmCode[1] = AdrVals[0];
                  CheckCPU(CPU68000);
               }
            }
         } else if (strcasecmp(ArgStr[2], "SR") == 0) { /* && #...,SR */
            if (OpSize != 1) WrError(1130);
            else {
               WAsmCode[0] = (strncmp(OpPart, "AND", 3) == 0) ? 0x027c : 0x007c;
               OpSize = 1;
               DecodeAdr(ArgStr[1], Mimm);
               if (AdrNum != 0) {
                  CodeLen = 4;
                  WAsmCode[1] = AdrVals[0];
                  CheckSup();
                  CheckCPU(CPU68000);
               }
            }
         } else if (AdrNum == 1) { /* && <EA>,Dn */
            WAsmCode[0] = 0x8000 | (OpSize << 6) | (AdrMode << 9);
            if (strncmp(OpPart, "AND", 3) == 0) WAsmCode[0] |= 0x4000;
            DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm);
            if (AdrNum != 0) {
               WAsmCode[0] |= AdrMode;
               CodeLen = 2 + AdrCnt;
               CopyAdrVals(WAsmCode + 1);
            }
         } else if (AdrNum != 0) { /* && ...,<EA> */
            DecodeAdr(ArgStr[1], Mdata + Mimm);
            if (AdrNum == 11) { /* && #..,<EA> */
               WAsmCode[0] = OpSize << 6;
               if (strncmp(OpPart, "AND", 3) == 0) WAsmCode[0] |= 0x200;
               CodeLen = 2 + AdrCnt;
               CopyAdrVals(WAsmCode + 1);
               DecodeAdr(ArgStr[2], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
               if (AdrNum != 0) {
                  WAsmCode[0] |= AdrMode;
                  CopyAdrVals(WAsmCode + (CodeLen >> 1));
                  CodeLen += AdrCnt;
               } else CodeLen = 0;
            } else if (AdrNum != 0) { /* && Dn,<EA> ? */
               WAsmCode[0] = 0x8100 | (OpSize << 6) | (AdrMode << 9);
               if (strncmp(OpPart, "AND", 3) == 0) WAsmCode[0] |= 0x4000;
               DecodeAdr(ArgStr[2], Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
               if (AdrNum != 0) {
                  CodeLen = 2 + AdrCnt;
                  CopyAdrVals(WAsmCode + 1);
                  WAsmCode[0] |= AdrMode;
               }
            }
         }
      }
      return true;
   }

   if (strncmp(OpPart, "EOR", 3) == 0) {
      if (ArgCnt != 2) WrError(1110);
      else if (strcasecmp(ArgStr[2], "CCR") == 0) {
         if ((*AttrPart != '\0') && (OpSize != 0)) WrError(1130);
         else {
            WAsmCode[0] = 0xa3c;
            OpSize = 0;
            DecodeAdr(ArgStr[1], Mimm);
            if (AdrNum != 0) {
               CodeLen = 4;
               WAsmCode[1] = AdrVals[0];
            }
         }
      } else if (strcasecmp(ArgStr[2], "SR") == 0) {
         if (OpSize != 1) WrError(1130);
         else {
            WAsmCode[0] = 0xa7c;
            DecodeAdr(ArgStr[1], Mimm);
            if (AdrNum != 0) {
               CodeLen = 4;
               WAsmCode[1] = AdrVals[0];
               CheckSup();
               CheckCPU(CPU68000);
            }
         }
      } else if (OpSize > 2) WrError(1130);
      else {
         DecodeAdr(ArgStr[1], Mdata + Mimm);
         if (AdrNum == 1) {
            WAsmCode[0] = 0xb100 | (AdrMode << 9) | (OpSize << 6);
            DecodeAdr(ArgStr[2], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
            if (AdrNum != 0) {
               CodeLen = 2 + AdrCnt;
               CopyAdrVals(WAsmCode + 1);
               WAsmCode[0] |= AdrMode;
            }
         } else if (AdrNum == 11) {
            WAsmCode[0] = 0x0a00 | (OpSize << 6);
            CopyAdrVals(WAsmCode + 1);
            CodeLen = 2 + AdrCnt;
            DecodeAdr(ArgStr[2], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
            if (AdrNum != 0) {
               CopyAdrVals(WAsmCode + (CodeLen >> 1));
               CodeLen += AdrCnt;
               WAsmCode[0] |= AdrMode;
            } else CodeLen = 0;
         }
      }
      return true;
   }

   return false;
}

static bool CodeBits(void) {
   if ((Memo("BSET")) || (Memo("BCLR")) || (Memo("BCHG")) || (Memo("BTST"))) {
      if (ArgCnt != 2) WrError(1110);
      else {
         if (*AttrPart == '\0') OpSize = 0;
         if (strcmp(OpPart, "BTST") != 0) DecodeAdr(ArgStr[2], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
         else DecodeAdr(ArgStr[2], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs);
         if (*AttrPart == '\0') OpSize = (AdrNum == 1) ? 2 : 0;
         if (AdrNum != 0) {
            if (((AdrNum == 1) && (OpSize != 2)) || ((AdrNum != 1) && (OpSize != 0))) WrError(1130);
            else {
               WAsmCode[0] = AdrMode;
               if (Memo("BSET")) WAsmCode[0] |= 0xc0;
               else if (Memo("BCLR")) WAsmCode[0] |= 0x80;
               else if (Memo("BCHG")) WAsmCode[0] |= 0x40;
               CodeLen = 2 + AdrCnt;
               CopyAdrVals(WAsmCode + 1);
               OpSize = 0;
               DecodeAdr(ArgStr[1], Mdata + Mimm);
               if (AdrNum == 1) {
                  WAsmCode[0] |= 0x100 | (AdrMode << 9);
               } else if (AdrNum == 11) {
                  memmove(WAsmCode + 2, WAsmCode + 1, CodeLen - 2);
                  WAsmCode[1] = AdrVals[0];
                  WAsmCode[0] |= 0x800;
                  CodeLen += 2;
                  if ((AdrVals[0] > 31)
                     || (((WAsmCode[0] & 0x38) != 0) && (AdrVals[0] > 7))) {
                     CodeLen = 0;
                     WrError(1510);
                  }
               } else CodeLen = 0;
            }
         }
      }
      return true;
   }

   if ((Memo("BFSET")) || (Memo("BFCLR"))
      || (Memo("BFCHG")) || (Memo("BFTST"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (*AttrPart != '\0') WrError(1130);
      else if (!SplitBitField(ArgStr[1], WAsmCode + 1)) WrError(1750);
      else {
         RelPos = 4;
         OpSize = 0;
         if (Memo("BFTST")) DecodeAdr(ArgStr[1], Mdata + Madri + Mdadri + Maix + Mpc + Mpcidx + Mabs);
         else DecodeAdr(ArgStr[1], Mdata + Madri + Mdadri + Maix + Mabs);
         if (AdrNum != 0) {
            WAsmCode[0] = 0xe8c0 | AdrMode;
            CopyAdrVals(WAsmCode + 2);
            CodeLen = 4 + AdrCnt;
            if (Memo("BFSET")) WAsmCode[0] |= 0x600;
            else if (Memo("BFCLR")) WAsmCode[0] |= 0x400;
            else if (Memo("BFCHG")) WAsmCode[0] |= 0x200;
            CheckCPU(CPU68020);
         }
      }
      return true;
   }

   if ((Memo("BFEXTS")) || (Memo("BFEXTU")) || (Memo("BFFFO"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (*AttrPart != '\0') WrError(1130);
      else if (!SplitBitField(ArgStr[1], WAsmCode + 1)) WrError(1750);
      else {
         RelPos = 4;
         OpSize = 0;
         DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs);
         if (AdrNum != 0) {
            WAsmCode[0] = 0xe9c0 + AdrMode;
            CopyAdrVals(WAsmCode + 2);
            if (Memo("BFEXTS")) WAsmCode[0] += 0x200;
            else if (Memo("BFFFO")) WAsmCode[0] += 0x400;
            CodeLen = 4 + AdrCnt;
            DecodeAdr(ArgStr[2], Mdata);
            if (AdrNum != 0) {
               WAsmCode[1] |= AdrMode << 12;
               CheckCPU(CPU68020);
            } else CodeLen = 0;
         }
      }
      return true;
   }

   if (Memo("BFINS")) {
      if (ArgCnt != 2) WrError(1110);
      else if (*AttrPart != '\0') WrError(1130);
      else if (!SplitBitField(ArgStr[2], WAsmCode + 1)) WrError(1750);
      else {
         OpSize = 0;
         DecodeAdr(ArgStr[2], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
         if (AdrNum != 0) {
            WAsmCode[0] = 0xefc0 + AdrMode;
            CopyAdrVals(WAsmCode + 2);
            CodeLen = 4 + AdrCnt;
            DecodeAdr(ArgStr[1], Mdata);
            if (AdrNum != 0) {
               WAsmCode[1] |= AdrMode << 12;
               CheckCPU(CPU68020);
            } else CodeLen = 0;
         }
      }
      return true;
   }

   return false;
}

static void MakeCode_68K(void) {
   bool ValOK;
   char *p;
   LongInt HVal;
   Integer i, HVal16;
   ShortInt HVal8;
   Byte z;
   Word w1, w2;

   CodeLen = 0;
   OpSize = 1;
   DontPrint = false;
   RelPos = 2;

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

/* Nullanweisung */

   if ((Memo("")) && (*AttrPart == '\0') && (ArgCnt == 0)) return;

/* Pseudoanweisungen */

   if (DecodeMoto16Pseudo(OpSize, true)) return;

   if (DecodePseudo()) return;

/* Befehlserweiterungen */

   if ((*OpPart == 'F') && (FPUAvail)) {
      strmove(OpPart, 1);
      DecodeFPUOrders();
      return;
   }

   if ((*OpPart == 'P') && (!Memo("PEA")) && (PMMUAvail)) {
      strmove(OpPart, 1);
      DecodePMMUOrders();
      return;
   }

/* Anweisungen ohne Argument */

   if (Memo("NOP")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 0) WrError(1110);
      else {
         CodeLen = 2;
         WAsmCode[0] = 0x4e71;
      }
      return;
   }

   if (Memo("RESET")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 0) WrError(1110);
      else {
         CodeLen = 2;
         WAsmCode[0] = 0x4e70;
         CheckSup();
      }
      return;
   }

   if (Memo("ILLEGAL")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 0) WrError(1110);
      else {
         CodeLen = 2;
         WAsmCode[0] = 0x4afc;
      }
      return;
   }

   if (Memo("TRAPV")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 0) WrError(1110);
      else {
         CodeLen = 2;
         WAsmCode[0] = 0x4e76;
      }
      return;
   }

   if (Memo("RTE")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 0) WrError(1110);
      else {
         CodeLen = 2;
         WAsmCode[0] = 0x4e73;
         CheckSup();
      }
      return;
   }

   if (Memo("RTR")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 0) WrError(1110);
      else {
         CodeLen = 2;
         WAsmCode[0] = 0x4e77;
      }
      return;
   }

   if (Memo("RTS")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 0) WrError(1110);
      else {
         CodeLen = 2;
         WAsmCode[0] = 0x4e75;
      }
      return;
   }

   if (Memo("BGND")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 0) WrError(1110);
      else {
         CodeLen = 2;
         WAsmCode[0] = 0x4afa;
         Check32();
      }
      return;
   }

/* Anweisungen mit konstantem Argument */

   if (Memo("STOP")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 1) WrError(1110);
      else if (*ArgStr[1] != '#') WrError(1120);
      else {
         HVal = EvalIntExpression(ArgStr[1] + 1, Int16, &ValOK);
         if (ValOK) {
            CodeLen = 4;
            WAsmCode[0] = 0x4e72;
            WAsmCode[1] = HVal;
            CheckSup();
         }
      }
      return;
   }

   if (Memo("LPSTOP")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 1) WrError(1110);
      else if (*ArgStr[1] != '#') WrError(1120);
      else {
         HVal = EvalIntExpression(ArgStr[1] + 1, Int16, &ValOK);
         if (ValOK) {
            CodeLen = 6;
            WAsmCode[0] = 0xf800;
            WAsmCode[1] = 0x01c0;
            WAsmCode[2] = HVal;
            CheckSup();
            Check32();
         }
      }
      return;
   }

   if (Memo("TRAP")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 1) WrError(1110);
      else if (*ArgStr[1] != '#') WrError(1120);
      else {
         HVal = EvalIntExpression(ArgStr[1] + 1, Int4, &ValOK);
         if (ValOK) {
            HVal8 = HVal;
            CodeLen = 2;
            WAsmCode[0] = 0x4e40 + (HVal8 & 15);
         }
      }
      return;
   }

   if (Memo("BKPT")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 1) WrError(1110);
      else if (*ArgStr[1] != '#') WrError(1120);
      else {
         HVal = EvalIntExpression(ArgStr[1] + 1, UInt3, &ValOK);
         if (ValOK) {
            HVal8 = HVal;
            CodeLen = 2;
            WAsmCode[0] = 0x4848 + (HVal8 & 7);
            CheckCPU(CPU68010);
         }
      }
      return;
   }

   if (Memo("RTD")) {
      if (*AttrPart != '\0') WrError(1100);
      else if (ArgCnt != 1) WrError(1110);
      else if (*ArgStr[1] != '#') WrError(1120);
      else {
         HVal = EvalIntExpression(ArgStr[1] + 1, Int16, &ValOK);
         if (ValOK) {
            CodeLen = 4;
            WAsmCode[0] = 0x4e74;
            WAsmCode[1] = HVal;
            CheckCPU(CPU68010);
         }
      }
      return;
   }

/* Anweisungen mit einem Speicheroperanden */

   if (CodeSingle()) return;

/* zwei Operanden */

   if (CodeDual()) return;

   if (CodeBits()) return;

   if (Memo("MOVEM")) {
      if (ArgCnt != 2) WrError(1110);
      else if ((OpSize != 1) && (OpSize != 2)) WrError(1130);
      else {
         if (DecodeRegList(ArgStr[2], WAsmCode + 1)) {
            DecodeAdr(ArgStr[1], Madri + Mpost + Mdadri + Maix + Mpc + Mpcidx + Mabs);
            if (AdrNum != 0) {
               WAsmCode[0] = 0x4c80 | AdrMode | ((OpSize - 1) << 6);
               CodeLen = 4 + AdrCnt;
               CopyAdrVals(WAsmCode + 2);
            }
         } else if (DecodeRegList(ArgStr[1], WAsmCode + 1)) {
            DecodeAdr(ArgStr[2], Madri + Mpre + Mdadri + Maix + Mabs);
            if (AdrNum != 0) {
               WAsmCode[0] = 0x4880 | AdrMode | ((OpSize - 1) << 6);
               CodeLen = 4 + AdrCnt;
               CopyAdrVals(WAsmCode + 2);
               if (AdrNum == 5) {
                  WAsmCode[9] = WAsmCode[1];
                  WAsmCode[1] = 0;
                  for (z = 0; z < 16; z++) {
                     WAsmCode[1] = WAsmCode[1] << 1;
                     if ((WAsmCode[9] & 1) == 1) WAsmCode[1]++;
                     WAsmCode[9] = WAsmCode[9] >> 1;
                  }
               }
            }
         } else WrError(1410);
      }
      return;
   }

   if (Memo("MOVEQ")) {
      if (ArgCnt != 2) WrError(1110);
      else if ((*AttrPart != '\0') && (OpSize != 2)) WrError(1130);
      else {
         DecodeAdr(ArgStr[2], Mdata);
         if (AdrNum != 0) {
            WAsmCode[0] = 0x7000 | (AdrMode << 9);
            OpSize = 0;
            DecodeAdr(ArgStr[1], Mimm);
            if (AdrNum != 0) {
               CodeLen = 2;
               WAsmCode[0] |= AdrVals[0];
            }
         }
      }
      return;
   }

   if (Memo("EXG")) {
      if ((*AttrPart != '\0') && (OpSize != 2)) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Mdata + Madr);
         if (AdrNum == 1) {
            WAsmCode[0] = 0xc100 | (AdrMode << 9);
            DecodeAdr(ArgStr[2], Mdata + Madr);
            if (AdrNum == 1) {
               WAsmCode[0] |= 0x40 | AdrMode;
               CodeLen = 2;
            } else if (AdrNum == 2) {
               WAsmCode[0] |= 0x88 | (AdrMode & 7);
               CodeLen = 2;
            }
         } else if (AdrNum == 2) {
            WAsmCode[0] = 0xc100 | (AdrMode & 7);
            DecodeAdr(ArgStr[2], Mdata + Madr);
            if (AdrNum == 1) {
               WAsmCode[0] |= 0x88 | (AdrMode << 9);
            // WAsmCode[0] |= 0x88 || (AdrMode << 9); //(@) Formerly: which was a bug.
               CodeLen = 2;
            } else {
               WAsmCode[0] |= 0x48 | ((AdrMode & 7) << 9);
               CodeLen = 2;
            }
         }
      }
      return;
   }

   if ((Memo("DIVSL")) || (Memo("DIVUL"))) {
      if (*AttrPart == '\0') OpSize = 2;
      if (ArgCnt != 2) WrError(1110);
      else if (OpSize != 2) WrError(1130);
      else if (!CodeRegPair(ArgStr[2], &w1, &w2)) WrError(1760);
      else {
         RelPos = 4;
         WAsmCode[1] = w1 + (w2 << 12);
         if (OpPart[3] == 'S') WAsmCode[1] |= 0x800;
         DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm);
         if (AdrNum != 0) {
            WAsmCode[0] = 0x4c40 + AdrMode;
            CopyAdrVals(WAsmCode + 2);
            CodeLen = 4 + AdrCnt;
            CheckCPU(CPU68332);
         }
      }
      return;
   }

   if ((strncmp(OpPart, "MUL", 3) == 0) || (strncmp(OpPart, "DIV", 3) == 0)) {
      if (ArgCnt != 2) WrError(1110);
      else if (OpSize == 1) {
         DecodeAdr(ArgStr[2], Mdata);
         if (AdrNum != 0) {
            WAsmCode[0] = 0x80c0 | (AdrMode << 9);
            if (strncmp(OpPart, "MUL", 3) == 0) WAsmCode[0] |= 0x4000;
            if (OpPart[3] == 'S') WAsmCode[0] |= 0x100;
            DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm);
            if (AdrNum != 0) {
               WAsmCode[0] |= AdrMode;
               CodeLen = 2 + AdrCnt;
               CopyAdrVals(WAsmCode + 1);
            }
         }
      } else if (OpSize == 2) {
         if (strchr(ArgStr[2], ':') == NULL) {
            strcat(ArgStr[2], ":");
            stradd(ArgStr[2], ArgStr[2]);
            ArgStr[2][strlen(ArgStr[2]) - 1] = '\0';
         }
         if (!CodeRegPair(ArgStr[2], &w1, &w2)) WrError(1760);
         else {
            WAsmCode[1] = w1 + (w2 << 12);
            RelPos = 4;
            if (w1 != w2) WAsmCode[1] |= 0x400;
            if (OpPart[3] == 'S') WAsmCode[1] |= 0x800;
            DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm);
            if (AdrNum != 0) {
               WAsmCode[0] = 0x4c00 + AdrMode;
               if (strncmp(OpPart, "DIV", 3) == 0) WAsmCode[0] |= 0x40;
               CopyAdrVals(WAsmCode + 2);
               CodeLen = 4 + AdrCnt;
               CheckCPU(CPU68332);
            }
         }
      } else WrError(1130);
      return;
   }

   if ((Memo("ABCD")) || (Memo("SBCD"))) {
      if ((OpSize != 0) && (*AttrPart != '\0')) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         OpSize = 0;
         DecodeAdr(ArgStr[1], Mdata + Mpre);
         if (AdrNum != 0) {
            WAsmCode[0] = 0x8100 | (AdrMode & 7);
            if (AdrNum == 5) WAsmCode[0] |= 8;
            if (Memo("ABCD")) WAsmCode[0] |= 0x4000;
            DecodeAdr(ArgStr[2], Masks[AdrNum]);
            if (AdrNum != 0) {
               CodeLen = 2;
               WAsmCode[0] |= (AdrMode & 7) << 9;
            }
         }
      }
      return;
   }

   if (Memo("CHK")) {
      if (OpSize != 1) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm);
         if (AdrNum != 0) {
            WAsmCode[0] = 0x4180 | AdrMode;
            CodeLen = 2 + AdrCnt;
            CopyAdrVals(WAsmCode + 1);
            DecodeAdr(ArgStr[2], Mdata);
            if (AdrNum == 1) WAsmCode[0] |= WAsmCode[0] | (AdrMode << 9);
            else CodeLen = 0;
         }
      }
      return;
   }

   if (Memo("LINK")) {
      if ((OpSize < 1) || (OpSize > 2)) WrError(1130);
      else if ((OpSize == 2) && (MomCPU < CPU68332)) WrError(1500);
      else if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Madr);
         if (AdrNum != 0) {
            WAsmCode[0] = (OpSize == 1) ? 0x4e50 : 0x4808;
            WAsmCode[0] += AdrMode & 7;
            DecodeAdr(ArgStr[2], Mimm);
            if (AdrNum == 11) {
               CodeLen = 2 + AdrCnt;
               memcpy(WAsmCode + 1, AdrVals, AdrCnt);
            }
         }
      }
      return;
   }

   if (Memo("MOVEP")) {
      if ((OpSize == 0) || (OpSize > 2)) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Mdata + Mdadri);
         if (AdrNum == 1) {
            WAsmCode[0] = 0x188 | ((OpSize - 1) << 6) | (AdrMode << 9);
            DecodeAdr(ArgStr[2], Mdadri);
            if (AdrNum == 6) {
               WAsmCode[0] |= AdrMode & 7;
               CodeLen = 4;
               WAsmCode[1] = AdrVals[0];
            }
         } else if (AdrNum == 6) {
            WAsmCode[0] = 0x108 | ((OpSize - 1) << 6) | (AdrMode & 7);
            WAsmCode[1] = AdrVals[0];
            DecodeAdr(ArgStr[2], Mdata);
            if (AdrNum == 1) {
               WAsmCode[0] |= (AdrMode & 7) << 9;
               CodeLen = 4;
            }
         }
      }
      return;
   }

   if (Memo("MOVEC")) {
      if ((*AttrPart != '\0') && (OpSize != 2)) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      {
         if (DecodeCtrlReg(ArgStr[1], WAsmCode + 1)) {
            DecodeAdr(ArgStr[2], Mdata + Madr);
            if (AdrNum != 0) {
               CodeLen = 4;
               WAsmCode[0] = 0x4e7a;
               WAsmCode[1] |= AdrMode << 12;
               CheckSup();
            }
         } else if (DecodeCtrlReg(ArgStr[2], WAsmCode + 1)) {
            DecodeAdr(ArgStr[1], Mdata + Madr);
            if (AdrNum != 0) {
               CodeLen = 4;
               WAsmCode[0] = 0x4e7b;
               WAsmCode[1] |= AdrMode << 12;
               CheckSup();
            }
         } else WrError(1440);
      }
      return;
   }

   if (Memo("MOVES")) {
      if (ArgCnt != 2) WrError(1110);
      else if (OpSize > 2) WrError(1130);
      else {
         DecodeAdr(ArgStr[1], Mdata + Madr + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
         if ((AdrNum == 1) || (AdrNum == 2)) {
            WAsmCode[1] = 0x800 | (AdrMode << 12);
            DecodeAdr(ArgStr[2], Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
            if (AdrNum != 0) {
               WAsmCode[0] = 0xe00 | AdrMode | (OpSize << 6);
               CodeLen = 4 + AdrCnt;
               CopyAdrVals(WAsmCode + 2);
               CheckSup();
               CheckCPU(CPU68010);
            }
         } else if (AdrNum != 0) {
            WAsmCode[0] = 0xe00 | AdrMode | (OpSize << 6);
            CodeLen = 4 + AdrCnt;
            CopyAdrVals(WAsmCode + 2);
            DecodeAdr(ArgStr[2], Mdata + Madr);
            if (AdrNum != 0) {
               WAsmCode[1] = AdrMode << 12;
               CheckSup();
               CheckCPU(CPU68010);
            } else CodeLen = 0;
         }
      }
      return;
   }

   if (Memo("CALLM")) {
      if (*AttrPart != '\0') WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         OpSize = 0;
         DecodeAdr(ArgStr[1], Mimm);
         if (AdrNum != 0) {
            WAsmCode[1] = AdrVals[0];
            RelPos = 4;
            DecodeAdr(ArgStr[2], Madri + Mdadri + Maix + Mpc + Mpcidx + Mabs);
            if (AdrNum != 0) {
               WAsmCode[0] = 0x06c0 + AdrMode;
               CopyAdrVals(WAsmCode + 2);
               CodeLen = 4 + AdrCnt;
               CheckCPU(CPU68020);
               Check020();
            }
         }
      }
      return;
   }

   if (Memo("CAS")) {
      if (OpSize > 2) WrError(1130);
      else if (ArgCnt != 3) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Mdata);
         if (AdrNum != 0) {
            WAsmCode[1] = AdrMode;
            DecodeAdr(ArgStr[2], Mdata);
            if (AdrNum != 0) {
               RelPos = 4;
               WAsmCode[1] += (((Word) AdrMode) << 6);
               DecodeAdr(ArgStr[3], Mdata + Madr + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
               if (AdrNum != 0) {
                  WAsmCode[0] = 0x08c0 + AdrMode + (((Word) OpSize + 1) << 9);
                  CopyAdrVals(WAsmCode + 2);
                  CodeLen = 4 + AdrCnt;
                  CheckCPU(CPU68020);
               }
            }
         }
      }
      return;
   }

   if (Memo("CAS2")) {
      if ((OpSize != 1) && (OpSize != 2)) WrError(1130);
      else if (ArgCnt != 3) WrError(1110);
      else if (!CodeRegPair(ArgStr[1], WAsmCode + 1, WAsmCode + 2)) WrError(1760);
      else if (!CodeRegPair(ArgStr[2], &w1, &w2)) WrError(1760);
      else {
         WAsmCode[1] += (w1 << 6);
         WAsmCode[2] += (w2 << 6);
         if (!CodeIndRegPair(ArgStr[3], &w1, &w2)) WrError(1760);
         else {
            WAsmCode[1] += (w1 << 12);
            WAsmCode[2] += (w2 << 12);
            WAsmCode[0] = 0x0cfc + (((Word) OpSize - 1) << 9);
            CodeLen = 6;
            CheckCPU(CPU68020);
         }
      }
      return;
   }

   if ((Memo("CHK2")) || (Memo("CMP2"))) {
      if (OpSize > 2) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         DecodeAdr(ArgStr[2], Mdata + Madr);
         if (AdrNum != 0) {
            RelPos = 4;
            WAsmCode[1] = ((Word) AdrMode) << 12;
            DecodeAdr(ArgStr[1], Madri + Mdadri + Maix + Mpc + Mpcidx + Mabs);
            if (AdrNum != 0) {
               WAsmCode[0] = 0x00c0 + (((Word) OpSize) << 9) + AdrMode;
               if (Memo("CHK2")) WAsmCode[1] |= 0x0800;
               CopyAdrVals(WAsmCode + 2);
               CodeLen = 4 + AdrCnt;
               CheckCPU(CPU68332);
            }
         }
      }
      return;
   }

   if (Memo("EXTB")) {
      if ((OpSize != 2) && (*AttrPart != '\0')) WrError(1130);
      else if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Mdata);
         if (AdrNum != 0) {
            WAsmCode[0] = 0x49c0 + AdrMode;
            CodeLen = 2;
            CheckCPU(CPU68332);
         }
      }
      return;
   }

   if ((Memo("PACK")) || (Memo("UNPK"))) {
      if (ArgCnt != 3) WrError(1110);
      else if (*AttrPart != '\0') WrError(1130);
      else {
         DecodeAdr(ArgStr[1], Mdata + Mpre);
         if (AdrNum != 0) {
            WAsmCode[0] = (Memo("PACK") ? 0x8140 : 0x8180) | (AdrMode & 7);
            if (AdrNum == 5) WAsmCode[0] += 8;
            DecodeAdr(ArgStr[2], Masks[AdrNum]);
            if (AdrNum != 0) {
               WAsmCode[0] |= ((AdrMode & 7) << 9);
               DecodeAdr(ArgStr[3], Mimm);
               if (AdrNum != 0) {
                  WAsmCode[1] = AdrVals[0];
                  CodeLen = 4;
                  CheckCPU(CPU68020);
               }
            }
         }
      }
      return;
   }

   if (Memo("RTM")) {
      if (*AttrPart != '\0') WrError(1130);
      else if (ArgCnt != 1) WrError(1110);
      else {
         DecodeAdr(ArgStr[1], Mdata + Madr);
         if (AdrNum != 0) {
            WAsmCode[0] = 0x06c0 + AdrMode;
            CodeLen = 2;
            CheckCPU(CPU68020);
            Check020();
         }
      }
      return;
   }

   if ((Memo("TBLS")) || (Memo("TBLSN")) || (Memo("TBLU")) || (Memo("TBLUN"))) {
      if (ArgCnt != 2) WrError(1110);
      else if (OpSize > 2) WrError(1130);
      else if (MomCPU < CPU68332) WrError(1500);
      else {
         DecodeAdr(ArgStr[2], Mdata);
         if (AdrNum != 0) {
            HVal = AdrMode;
            p = strchr(ArgStr[1], ':');
            if (p == 0) {
               DecodeAdr(ArgStr[1], Madri + Mdadri + Maix + Mabs + Mpc + Mpcidx);
               if (AdrNum != 0) {
                  WAsmCode[0] = 0xf800 + AdrMode;
                  WAsmCode[1] = 0x0100 + (OpSize << 6) + (HVal << 12);
                  if (OpPart[3] == 'S') WAsmCode[1] += 0x0800;
                  if (OpPart[strlen(OpPart) - 1] == 'N') WAsmCode[1] += 0x0400;
                  memcpy(WAsmCode + 2, AdrVals, AdrCnt);
                  CodeLen = 4 + AdrCnt;
                  Check32();
               }
            } else {
               strcopy(ArgStr[3], p + 1);
               *p = '\0';
               DecodeAdr(ArgStr[1], Mdata);
               if (AdrNum != 0) {
                  w2 = AdrMode;
                  DecodeAdr(ArgStr[3], Mdata);
                  if (AdrNum != 0) {
                     WAsmCode[0] = 0xf800 + w2;
                     WAsmCode[1] = 0x0000 + (OpSize << 6) + (HVal << 12) + AdrMode;
                     if (OpPart[3] == 'S') WAsmCode[1] += 0x0800;
                     if (OpPart[strlen(OpPart) - 1] == 'N') WAsmCode[1] += 0x0400;
                     CodeLen = 4;
                     Check32();
                  }
               }
            }
         }
      }
      return;
   }

/* bedingte Befehle */

   if ((strlen(OpPart) <= 3) && (*OpPart == 'B')) {
   /* .W, .S, .L, .X erlaubt */

      if ((OpSize != 1) && (OpSize != 2) && (OpSize != 4) && (OpSize != 6))
         WrError(1130);

   /* nur ein Operand erlaubt */

      else if (ArgCnt != 1) WrError(1110);
      else {

      /* Bedingung finden, evtl. meckern */

         for (i = 0; i < CondCnt; i++)
            if (strcmp(OpPart + 1, CondNams[i]) == 0) break;
         if (i == CondCnt) WrError(1360);
         else {

         /* Zieladresse ermitteln, zum Programmzaehler relativieren */

            HVal = EvalIntExpression(ArgStr[1], Int32, &ValOK);
            HVal = HVal - (EProgCounter() + 2);

         /* Bei Automatik Groesse festlegen */

            if (OpSize == 1) {
               if (IsDisp8(HVal)) OpSize = 4;
               else if (IsDisp16(HVal)) OpSize = 2;
               else OpSize = 6;
            }

            if (ValOK) {

            /* 16 Bit ? */

               if (OpSize == 2) {

               /* zu weit ? */

                  HVal16 = HVal;
                  if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(1370);
                  else {

                  /* Code erzeugen */

                     CodeLen = 4;
                     WAsmCode[0] = 0x6000 | (CondVals[i] << 8);
                     WAsmCode[1] = HVal16;
                  }
               }

            /* 8 Bit ? */

               else if (OpSize == 4) {

               /* zu weit ? */

                  HVal8 = HVal;
                  if ((!IsDisp8(HVal)) && (!SymbolQuestionable)) WrError(1370);

               /* Code erzeugen */
                  else {
                     CodeLen = 2;
                     if (HVal8 != 0) {
                        WAsmCode[0] = 0x6000 | (CondVals[i] << 8) | ((Byte) HVal8);
                     } else {
                        WAsmCode[0] = NOPCode;
                        if ((!Repass) && (*AttrPart != '\0')) WrError(60);
                     }
                  }
               }

            /* 32 Bit ? */

               else {
                  CodeLen = 6;
                  WAsmCode[0] = 0x60ff | (CondVals[i] << 8);
                  WAsmCode[1] = HVal >> 16;
                  WAsmCode[2] = HVal & 0xffff;
                  CheckCPU(CPU68332);
               }
            }
         }
         return;
      }
   }

   if ((strlen(OpPart) <= 3) && (*OpPart == 'S')) {
      if ((*AttrPart != '\0') && (OpSize != 0)) WrError(1130);
      else if (ArgCnt != 1) WrError(1130);
      else {
         for (i = 0; i < CondCnt - 2; i++)
            if (strcmp(OpPart + 1, CondNams[i]) == 0) break;
         if (i == CondCnt - 2) WrError(1360);
         else {
            OpSize = 0;
            DecodeAdr(ArgStr[1], Mdata + Madri + Mpost + Mpre + Mdadri + Maix + Mabs);
            if (AdrNum != 0) {
               WAsmCode[0] = 0x50c0 | (CondVals[i] << 8) | AdrMode;
               CodeLen = 2 + AdrCnt;
               CopyAdrVals(WAsmCode + 1);
            }
         }
      }
      return;
   }

   if ((strlen(OpPart) <= 4) && (strncmp(OpPart, "DB", 2) == 0)) {
      if (OpSize != 1) WrError(1130);
      else if (ArgCnt != 2) WrError(1110);
      else {
         for (i = 0; i < CondCnt - 1; i++)
            if (strcmp(OpPart + 2, CondNams[i]) == 0) break;
         if (i == 18) i = 1;
         if (i == CondCnt - 1) WrError(1360);
         else {
            HVal = EvalIntExpression(ArgStr[2], Int32, &ValOK);
            if (ValOK) {
               HVal -= (EProgCounter() + 2);
               HVal16 = HVal;
               if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(1370);
               else {
                  CodeLen = 4;
                  WAsmCode[0] = 0x50c8 | (CondVals[i] << 8);
                  WAsmCode[1] = HVal16;
                  DecodeAdr(ArgStr[1], Mdata);
                  if (AdrNum == 1) WAsmCode[0] |= AdrMode;
                  else CodeLen = 0;
               }
            }
         }
         return;
      }
   }

   if ((strlen(OpPart) <= 6) && (strncmp(OpPart, "TRAP", 4) == 0)) {
      if (*AttrPart == '\0') OpSize = 0;
      i = (OpSize == 0) ? 0 : 1;
      if (OpSize > 2) WrError(1130);
      else if (ArgCnt != i) WrError(1110);
      else {
         for (i = 0; i < CondCnt - 2; i++)
            if (strcmp(OpPart + 4, CondNams[i]) == 0) break;
         if (i == 18) WrError(1360);
         else {
            WAsmCode[0] = 0x50f8 + (i << 8);
            if (OpSize == 0) {
               WAsmCode[0] += 4;
               CodeLen = 2;
            } else {
               DecodeAdr(ArgStr[1], Mimm);
               if (AdrNum != 0) {
                  WAsmCode[0] += OpSize + 1;
                  CopyAdrVals(WAsmCode + 1);
                  CodeLen = 2 + AdrCnt;
               }
            }
            CheckCPU(CPU68332);
         }
      }
      return;
   }

/* unbekannter Befehl */

   WrXError(1200, OpPart);
}

static void InitCode_68K(void) {
   SaveInitProc();
   SetFlag(&PMMUAvail, PMMUAvailName, false);
   SetFlag(&FullPMMU, FullPMMUName, true);
}

static bool ChkPC_68K(void) {
#ifdef HAS64
   return ((ActPC == SegCode) && (ProgCounter() <= 0xffffffffll));
#else
   return (ActPC == SegCode);
#endif
}

static bool IsDef_68K(void) {
   return false;
}

static void SwitchFrom_68K(void) {
   DeinitFields();
}

static void SwitchTo_68K(void) {
   TurnWords = true;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "*";
   HeaderID = 0x01;
   NOPCode = 0x4e71;
   DivideChars = ",";
   HasAttrs = true;
   AttrChars = ".";

   ValidSegs = (1 << SegCode);
   Grans[SegCode] = 1;
   ListGrans[SegCode] = 2;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_68K;
   ChkPC = ChkPC_68K;
   IsDef = IsDef_68K;
   SwitchFrom = SwitchFrom_68K;
   InitFields();

   SetFlag(&FullPMMU, FullPMMUName, MomCPU <= CPU68020);
}

void code68k_init(void) {
   CPU68008 = AddCPU("68008", SwitchTo_68K);
   CPU68000 = AddCPU("68000", SwitchTo_68K);
   CPU68010 = AddCPU("68010", SwitchTo_68K);
   CPU68012 = AddCPU("68012", SwitchTo_68K);
   CPU68332 = AddCPU("68332", SwitchTo_68K);
   CPU68340 = AddCPU("68340", SwitchTo_68K);
   CPU68360 = AddCPU("68360", SwitchTo_68K);
   CPU68020 = AddCPU("68020", SwitchTo_68K);
   CPU68030 = AddCPU("68030", SwitchTo_68K);

   SaveInitProc = InitPassProc;
   InitPassProc = InitCode_68K;
}
