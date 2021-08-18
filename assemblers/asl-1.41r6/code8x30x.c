// AS-Portierung
// Codegenerator Signetics 8X30x
#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "chunks.h"
#include "bpemu.h"
#include "stringutil.h"

#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"

/*****************************************************************************/

#define AriOrderCnt 4

typedef struct {
   char *Name;
   Word Code;
} FixedOrder;

static CPUVar CPU8x300, CPU8x305;
static FixedOrder *AriOrders;

/*-------------------------------------------------------------------------*/

static int InstrZ;

static void AddAri(char *NName, Word NCode) {
   if (InstrZ >= AriOrderCnt) exit(255);
   AriOrders[InstrZ].Name = NName;
   AriOrders[InstrZ++].Code = NCode;
}

static void InitFields(void) {
   AriOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * AriOrderCnt);
   InstrZ = 0;
   AddAri("MOVE", 0);
   AddAri("ADD", 1);
   AddAri("AND", 2);
   AddAri("XOR", 3);
}

static void DeinitFields(void) {
   free(AriOrders);
}

/*-------------------------------------------------------------------------*/

static bool DecodeReg(char *Asc, Word * Erg, ShortInt * ErgLen) {
   bool OK;
   Word Acc;
   LongInt Adr;
   char *z;

   *ErgLen = (-1);

   if (strcasecmp(Asc, "AUX") == 0) {
      *Erg = 0;
      return true;
   }

   if (strcasecmp(Asc, "OVF") == 0) {
      *Erg = 8;
      return true;
   }

   if (strcasecmp(Asc, "IVL") == 0) {
      *Erg = 7;
      return true;
   }

   if (strcasecmp(Asc, "IVR") == 0) {
      *Erg = 15;
      return true;
   }

   if ((toupper(*Asc) == 'R') && (strlen(Asc) > 1) && (strlen(Asc) < 4)) {
      Acc = 0;
      OK = true;
      for (z = Asc + 1; *z != '\0'; z++)
         if (!OK) ;
         else if ((*z < '0') || (*z > '7')) OK = false;
         else Acc = (Acc << 3) + (*z - '0');
      if ((OK) && (Acc < 32)) {
         if ((MomCPU == CPU8x300) && (Acc > 9) && (Acc < 15)) {
            WrXError(1445, Asc);
            return false;
         } else *Erg = Acc;
         return true;
      }
   }

   if (strlen(Asc) == 4 && strncasecmp(Asc + 1, "IV", 2) == 0 && Asc[3] >= '0' && Asc[3] <= '7') {
      if (toupper(*Asc) == 'L') {
         *Erg = Asc[3] - '0' + 0x10;
         return true;
      } else if (toupper(*Asc) == 'R') {
         *Erg = Asc[3] - '0' + 0x18;
         return true;
      }
   }

/* IV - Objekte */

   Adr = EvalIntExpression(Asc, UInt24, &OK);
   if (OK) {
      *ErgLen = Adr & 7;
      *Erg = 0x10 + ((Adr & 0x10) >> 1) + ((Adr & 0x700) >> 8);
      return true;
   } else return false;
}

static char *HasDisp(char *Asc) {
   Integer Lev;
   char *z;
   int l = strlen(Asc);

   if (Asc[l - 1] == ')') {
      z = Asc + l - 2;
      Lev = 0;
      while ((z >= Asc) && (Lev != -1)) {
         switch (*z) {
            case '(':
               Lev--;
               break;
            case ')':
               Lev++;
               break;
         }
         if (Lev != -1) z--;
      }
      if (Lev != -1) {
         WrError(1300);
         return NULL;
      }
   } else z = NULL;

   return z;
}

static bool GetLen(char *Asc, Word * Erg) {
   bool OK;

   FirstPassUnknown = false;
   *Erg = EvalIntExpression(Asc, UInt4, &OK);
   if (!OK) return false;
   if (FirstPassUnknown) *Erg = 8;
   if (!ChkRange(*Erg, 1, 8)) return false;
   *Erg &= 7;
   return true;
}

/*-------------------------------------------------------------------------*/

/* Symbol: 00AA0ORL */

static bool DecodePseudo(void) {
   LongInt Adr, Ofs, Erg;
   Word Len;
   bool OK;

   if ((Memo("LIV")) || (Memo("RIV"))) {
      Erg = 0x10 * Memo("RIV");
      if (ArgCnt != 3) WrError(1110);
      else {
         Adr = EvalIntExpression(ArgStr[1], UInt8, &OK);
         if (OK) {
            Ofs = EvalIntExpression(ArgStr[2], UInt3, &OK);
            if (OK)
               if (GetLen(ArgStr[3], &Len)) {
                  PushLocHandle(-1);
                  EnterIntSymbol(LabPart, Erg + (Adr << 16) + (Ofs << 8) + (Len & 7), SegNone, false);
                  PopLocHandle();
               }
         }
      }
      return true;
   }

   return false;
}

static void MakeCode_8x30X(void) {
   bool OK;
   Word SrcReg, DestReg;
   ShortInt SrcLen, DestLen;
   LongInt Op;
   Word Rot, Adr;
   Integer z;
   char *p;
   String tmp;

   CodeLen = 0;
   DontPrint = false;

/* zu ignorierendes */

   if (Memo("")) return;

/* Pseudoanweisungen */

   if (DecodePseudo()) return;

/* eingebaute Makros */

   if (Memo("NOP")) { /* NOP = MOVE AUX,AUX */
      if (ArgCnt != 0) WrError(1110);
      else {
         WAsmCode[0] = 0x0000;
         CodeLen = 1;
      }
      return;
   }

   if (Memo("HALT")) { /* HALT = JMP * */
      if (ArgCnt != 0) WrError(1110);
      else {
         WAsmCode[0] = 0xe000 + (EProgCounter() & 0x1fff);
         CodeLen = 1;
      }
      return;
   }

   if ((Memo("XML")) || (Memo("XMR"))) {
      if (ArgCnt != 1) WrError(1110);
      else if (MomCPU < CPU8x305) WrError(1500);
      else {
         Adr = EvalIntExpression(ArgStr[1], Int8, &OK);
         if (OK) {
            WAsmCode[0] = 0xca00 + (Memo("XER") << 8) + (Adr & 0xff);
            CodeLen = 1;
         }
      }
      return;
   }

/* Datentransfer */

   if (Memo("SEL")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         Op = EvalIntExpression(ArgStr[1], UInt24, &OK);
         if (OK) {
            WAsmCode[0] = 0xc700 + ((Op & 0x10) << 7) + ((Op >> 16) & 0xff);
            CodeLen = 1;
         }
      }
      return;
   }

   if (Memo("XMIT")) {
      if ((ArgCnt != 2) && (ArgCnt != 3)) WrError(1110);
      else if (!DecodeReg(ArgStr[2], &SrcReg, &SrcLen)) ;
      else if (SrcReg < 16) {
         if (ArgCnt != 2) WrError(1110);
         else {
            Adr = EvalIntExpression(ArgStr[1], Int8, &OK);
            if (OK) {
               WAsmCode[0] = 0xc000 + (SrcReg << 8) + (Adr & 0xff);
               CodeLen = 1;
            }
         }
      } else {
         if (ArgCnt == 2) {
            Rot = 0xffff;
            OK = true;
         } else OK = GetLen(ArgStr[3], &Rot);
         if (OK) {
            if (Rot == 0xffff)
               Rot = (SrcLen == -1) ? 0 : SrcLen;
            if ((SrcLen != -1) && (Rot != SrcLen)) WrError(1131);
            else {
               Adr = EvalIntExpression(ArgStr[1], Int5, &OK);
               if (OK) {
                  WAsmCode[0] = 0xc000 + (SrcReg << 8) + (Rot << 5) + (Adr & 0x1f);
                  CodeLen = 1;
               }
            }
         }
      }
      return;
   }

/* Arithmetik */

   for (z = 0; z < AriOrderCnt; z++)
      if (Memo(AriOrders[z].Name)) {
         if ((ArgCnt != 2) && (ArgCnt != 3)) WrError(1110);
         else if (!DecodeReg(ArgStr[ArgCnt], &DestReg, &DestLen)) ;
         else if (DestReg < 16) { /* Ziel Register */
            if (ArgCnt == 2) { /* wenn nur zwei Operanden und Ziel Register... */
               p = HasDisp(ArgStr[1]); /* kann eine Rotation dabei sein */
               if (p != NULL) { /* jau! */
                  strcopy(tmp, p + 1);
                  tmp[strlen(tmp) - 1] = '\0';
                  Rot = EvalIntExpression(tmp, UInt3, &OK);
                  if (OK) {
                     *p = '\0';
                     if (!DecodeReg(ArgStr[1], &SrcReg, &SrcLen)) ;
                     else if (SrcReg >= 16) WrXError(1445, ArgStr[1]);
                     else {
                        WAsmCode[0] = (AriOrders[z].Code << 13) + (SrcReg << 8) + (Rot << 5) + DestReg;
                        CodeLen = 1;
                     }
                  }
               } else { /* noi! */
                  if (DecodeReg(ArgStr[1], &SrcReg, &SrcLen)) {
                     WAsmCode[0] = (AriOrders[z].Code << 13) + (SrcReg << 8) + DestReg;
                     if ((SrcReg >= 16) && (SrcLen != -1)) WAsmCode[0] += SrcLen << 5;
                     CodeLen = 1;
                  }
               }
            } else { /* 3 Operanden --> Quelle ist I/O */
               if (!GetLen(ArgStr[2], &Rot)) ;
               else if (!DecodeReg(ArgStr[1], &SrcReg, &SrcLen)) ;
               else if (SrcReg < 16) WrXError(1445, ArgStr[1]);
               else if ((SrcLen != -1) && (SrcLen != Rot)) WrError(1131);
               else {
                  WAsmCode[0] = (AriOrders[z].Code << 13) + (SrcReg << 8) + (Rot << 5) + DestReg;
                  CodeLen = 1;
               }
            }
         } else { /* Ziel I/O */
            if (ArgCnt == 2) { /* 2 Argumente: Laenge=Laenge Ziel */
               Rot = DestLen;
               OK = true;
            } else { /* 3 Argumente: Laenge=Laenge Ziel+Angabe */
               OK = GetLen(ArgStr[2], &Rot);
               if (OK) {
                  if (FirstPassUnknown) Rot = DestLen;
                  if (DestLen == -1) DestLen = Rot;
                  OK = Rot == DestLen;
                  if (!OK) WrError(1131);
               }
            }
            if (!OK) ;
            else if (DecodeReg(ArgStr[1], &SrcReg, &SrcLen)) {
               if ((Rot == 0xffff))
                  Rot = ((SrcLen == -1)) ? 0 : SrcLen;
               if ((DestReg >= 16) && (SrcLen != -1) && (SrcLen != Rot)) WrError(1131);
               else {
                  WAsmCode[0] = (AriOrders[z].Code << 13) + (SrcReg << 8) + (Rot << 5) + DestReg;
                  CodeLen = 1;
               }
            }
         }
         return;
      }

   if (Memo("XEC")) {
      if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
      else {
         p = HasDisp(ArgStr[1]);
         if (p == NULL) WrError(1350);
         else {
            strcopy(tmp, p + 1);
            tmp[strlen(tmp) - 1] = '\0';
            if (DecodeReg(tmp, &SrcReg, &SrcLen)) {
               *p = '\0';
               if (SrcReg < 16) {
                  if (ArgCnt != 1) WrError(1110);
                  else {
                     WAsmCode[0] = EvalIntExpression(ArgStr[1], UInt8, &OK);
                     if (OK) {
                        WAsmCode[0] += 0x8000 + (SrcReg << 8);
                        CodeLen = 1;
                     }
                  }
               } else {
                  if (ArgCnt == 1) {
                     Rot = 0xffff;
                     OK = true;
                  } else OK = GetLen(ArgStr[2], &Rot);
                  if (OK) {
                     if (Rot == 0xffff)
                        Rot = (SrcLen == -1) ? 0 : SrcLen;
                     if ((SrcLen != -1) && (Rot != SrcLen)) WrError(1131);
                     else {
                        WAsmCode[0] = EvalIntExpression(ArgStr[1], UInt5, &OK);
                        if (OK) {
                           WAsmCode[0] += 0x8000 + (SrcReg << 8) + (Rot << 5);
                           CodeLen = 1;
                        }
                     }
                  }
               }
            }
         }
      }
      return;
   }

/* Spruenge */

   if (Memo("JMP")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         WAsmCode[0] = EvalIntExpression(ArgStr[1], UInt13, &OK);
         if (OK) {
            WAsmCode[0] += 0xe000;
            CodeLen = 1;
         }
      }
      return;
   }

   if (Memo("NZT")) {
      if ((ArgCnt != 2) && (ArgCnt != 3)) WrError(1110);
      else if (!DecodeReg(ArgStr[1], &SrcReg, &SrcLen)) ;
      else if (SrcReg < 16) {
         if (ArgCnt != 2) WrError(1110);
         else {
            Adr = EvalIntExpression(ArgStr[2], UInt13, &OK);
            if (!OK) ;
            else if ((!SymbolQuestionable) && ((Adr >> 8) != (EProgCounter() >> 8))) WrError(1910);
            else {
               WAsmCode[0] = 0xa000 + (SrcReg << 8) + (Adr & 0xff);
               CodeLen = 1;
            }
         }
      } else {
         if (ArgCnt == 2) {
            Rot = 0xffff;
            OK = true;
         } else OK = GetLen(ArgStr[2], &Rot);
         if (OK) {
            if (Rot == 0xffff)
               Rot = (SrcLen == -1) ? 0 : SrcLen;
            if ((SrcLen != -1) && (Rot != SrcLen)) WrError(1131);
            else {
               Adr = EvalIntExpression(ArgStr[ArgCnt], UInt13, &OK);
               if (!OK) ;
               else if ((!SymbolQuestionable) && ((Adr >> 5) != (EProgCounter() >> 5))) WrError(1910);
               else {
                  WAsmCode[0] = 0xa000 + (SrcReg << 8) + (Rot << 5) + (Adr & 0x1f);
                  CodeLen = 1;
               }
            }
         }
      }
      return;
   }

   WrXError(1200, OpPart);
}

static bool ChkPC_8x30X(void) {
   switch (ActPC) {
      case SegCode:
         return (ProgCounter() <= 0x1fff);
      default:
         return false;
   }
}

static bool IsDef_8x30X(void) {
   return (Memo("LIV") || Memo("RIV"));
}

static void SwitchFrom_8x30X() {
   DeinitFields();
}

static void SwitchTo_8x30X(void) {
   TurnWords = false;
   ConstMode = ConstModeMoto;
   SetIsOccupied = false;

   PCSymbol = "*";
   HeaderID = 0x3a;
   NOPCode = 0x0000;
   DivideChars = ",";
   HasAttrs = false;

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 2;
   ListGrans[SegCode] = 2;
   SegInits[SegCode] = 0;

   MakeCode = MakeCode_8x30X;
   ChkPC = ChkPC_8x30X;
   IsDef = IsDef_8x30X;
   SwitchFrom = SwitchFrom_8x30X;
   InitFields();
}

void code8x30x_init(void) {
   CPU8x300 = AddCPU("8x300", SwitchTo_8x30X);
   CPU8x305 = AddCPU("8x305", SwitchTo_8x30X);
}
