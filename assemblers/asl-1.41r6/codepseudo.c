/* codepseudo.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Pseudo-Befehle                                         */
/*                                                                           */
/* Historie: 23. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "nls.h"
#include "bpemu.h"
#include "endian.h"
#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"

#include "codepseudo.h"

int FindInst(void *Field, int Size, int Count) {
   char *cptr, **ptr;

#ifdef OPT
   int l = 0, r = Count - 1, m, res;

   while (true) {
      m = (l + r) >> 1;
      cptr = ((char *)Field) + (Size * m);
      ptr = (char **)cptr;
      res = strcmp(*ptr, OpPart);
      if (res == 0) return m;
      else if (l == r) return -1;
      else if (res < 0) {
         if (r - l == 1) return -1;
         else l = m;
      } else r = m;
   }

#else
   int z, res;

   cptr = Field;
   for (z = 0; z < Count; z++) {
      ptr = (char **)cptr;
      res = strcmp(*ptr, OpPart);
      if (res == 0) return z;
      if (res > 0) return -1;
      cptr += Size;
   }
   return -1;
#endif
}

bool IsIndirect(char *Asc) {
   Integer z, Level, l;

   if (((l = strlen(Asc)) <= 2) || (Asc[0] != '(') || (Asc[l - 1] != ')')) return false;

   Level = 0;
   for (z = 1; z <= l - 2; z++) {
      if (Asc[z] == '(') Level++;
      if (Asc[z] == ')') Level--;
      if (Level < 0) return false;
   }

   return true;
}

static enum { DSNone, DSConstant, DSSpace } DSFlag;
typedef bool (*TLayoutFunc)(char *Asc, Word * Cnt, bool Turn);

static bool LayoutByte(char *Asc, Word * Cnt, bool Turn) {
   bool Result;
   TempResult t;

   if (Turn); /* satisfy some compilers */

   Result = false;

   if (strcmp(Asc, "?") == 0) {
      if (DSFlag == DSConstant) WrError(1930);
      else {
         *Cnt = 1;
         Result = true;
         DSFlag = DSSpace;
         CodeLen++;
      }
      return Result;
   } else {
      if (DSFlag == DSSpace) {
         WrError(1930);
         return Result;
      } else DSFlag = DSConstant;
   }

   FirstPassUnknown = false;
   EvalExpression(Asc, &t);
   switch (t.Typ) {
      case TempInt:
         if (FirstPassUnknown) t.Contents.Int &= 0xff;
         if (!RangeCheck(t.Contents.Int, Int8)) WrError(1320);
         else {
            BAsmCode[CodeLen++] = t.Contents.Int;
            *Cnt = 1;
            Result = true;
         };
         break;
      case TempFloat:
         WrError(1135);
         break;
      case TempString:
         TranslateString(t.Contents.Ascii);
         memcpy(BAsmCode + CodeLen, t.Contents.Ascii, strlen(t.Contents.Ascii));
         CodeLen += (*Cnt = strlen(t.Contents.Ascii));
         Result = true;
         break;
      case TempNone:
         break;
   }

   return Result;
}

static bool LayoutWord(char *Asc, Word * Cnt, bool Turn) {
   bool OK, Result;
   Word erg;

   *Cnt = 2;
   Result = false;

   if (strcmp(Asc, "?") == 0) {
      if (DSFlag == DSConstant) WrError(1930);
      else {
         Result = true;
         DSFlag = DSSpace;
         CodeLen += 2;
      }
      return Result;
   } else {
      if (DSFlag == DSSpace) {
         WrError(1930);
         return Result;
      } else DSFlag = DSConstant;
   }

   if (CodeLen + 2 > MaxCodeLen) {
      WrError(1920);
      return Result;
   }
   erg = EvalIntExpression(Asc, Int16, &OK);
   if (OK) {
      if (Turn) erg = ((erg >> 8) & 0xff) + ((erg & 0xff) << 8);
      BAsmCode[CodeLen] = erg & 0xff;
      BAsmCode[CodeLen + 1] = erg >> 8;
      CodeLen += 2;
   }
   return OK;
}

static bool LayoutDoubleWord(char *Asc, Word * Cnt, bool Turn) {
   TempResult erg;
   Integer z;
   Byte Exg;
   Single copy;
   bool Result = false;

   *Cnt = 4;

   if (strcmp(Asc, "?") == 0) {
      if (DSFlag == DSConstant) WrError(1930);
      else {
         Result = true;
         DSFlag = DSSpace;
         CodeLen += 4;
      }
      return Result;
   } else {
      if (DSFlag == DSSpace) {
         WrError(1930);
         return Result;
      } else DSFlag = DSConstant;
   }

   if (CodeLen + 4 > MaxCodeLen) {
      WrError(1920);
      return Result;
   }

   KillBlanks(Asc);
   EvalExpression(Asc, &erg);
   switch (erg.Typ) {
      case TempNone:
         return Result;
      case TempInt:
         if (RangeCheck(erg.Contents.Int, Int32)) {
            BAsmCode[CodeLen] = ((erg.Contents.Int) & 0xff);
            BAsmCode[CodeLen + 1] = ((erg.Contents.Int >> 8) & 0xff);
            BAsmCode[CodeLen + 2] = ((erg.Contents.Int >> 16) & 0xff);
            BAsmCode[CodeLen + 3] = ((erg.Contents.Int >> 24) & 0xff);
            CodeLen += 4;
         } else {
            WrError(1320);
            return Result;
         }
         break;
      case TempFloat:
         if (FloatRangeCheck(erg.Contents.Float, Float32)) {
            copy = erg.Contents.Float;
            memcpy(BAsmCode + CodeLen, &copy, 4);
            if (BigEndian) DSwap(BAsmCode + CodeLen, 4);
            CodeLen += 4;
         } else {
            WrError(1320);
            return Result;
         }
         break;
      case TempString:
         WrError(1135);
         return Result;
   }

   if (Turn)
      for (z = 0; z < 2; z++) {
         Exg = BAsmCode[CodeLen - 4 + z];
         BAsmCode[CodeLen - 4 + z] = BAsmCode[CodeLen - 1 - z];
         BAsmCode[CodeLen - 1 - z] = Exg;
      }
   return true;
}

static bool LayoutQuadWord(char *Asc, Word * Cnt, bool Turn) {
   bool Result;
   TempResult erg;
   Integer z;
   Byte Exg;
   Double Copy;

   Result = false;
   *Cnt = 8;

   if (strcmp(Asc, "?") == 0) {
      if (DSFlag == DSConstant) WrError(1930);
      else {
         Result = true;
         DSFlag = DSSpace;
         CodeLen += 8;
      }
      return Result;
   } else {
      if (DSFlag == DSSpace) {
         WrError(1930);
         return Result;
      } else DSFlag = DSConstant;
   }

   if (CodeLen + 8 > MaxCodeLen) {
      WrError(1920);
      return Result;
   }

   KillBlanks(Asc);
   EvalExpression(Asc, &erg);
   switch (erg.Typ) {
      case TempNone:
         return Result;
      case TempInt:
         memcpy(BAsmCode + CodeLen, &(erg.Contents.Int), sizeof(LargeInt));
#ifdef HAS64
         if (BigEndian) QSwap(BAsmCode + CodeLen, 8);
#else
         if (BigEndian) DSwap(BAsmCode + CodeLen, 4);
         for (z = 4; z < 8; BAsmCode[CodeLen + (z++)] = (BAsmCode[CodeLen + 3] >= 0x80) ? 0xff : 0x00);
#endif
         CodeLen += 8;
         break;
      case TempFloat:
         Copy = erg.Contents.Float;
         memcpy(BAsmCode + CodeLen, &Copy, 8);
         if (BigEndian) QSwap(BAsmCode + CodeLen, 8);
         CodeLen += 8;
         break;
      case TempString:
         WrError(1135);
         return Result;
   }

   if (Turn)
      for (z = 0; z < 4; z++) {
         Exg = BAsmCode[CodeLen - 8 + z];
         BAsmCode[CodeLen - 8 + z] = BAsmCode[CodeLen - 1 - z];
         BAsmCode[CodeLen - 1 - z] = Exg;
      }
   return true;
}

static bool LayoutTenBytes(char *Asc, Word * Cnt, bool Turn) {
   bool OK, Result;
   Double erg;
   Integer z;
   Byte Exg;

   Result = false;
   *Cnt = 10;

   if (strcmp(Asc, "?") == 0) {
      if (DSFlag == DSConstant) WrError(1930);
      else {
         Result = true;
         DSFlag = DSSpace;
         CodeLen += 10;
      }
      return Result;
   } else {
      if (DSFlag == DSSpace) {
         WrError(1930);
         return Result;
      } else DSFlag = DSConstant;
   }

   if (CodeLen + 10 > MaxCodeLen) {
      WrError(1920);
      return Result;
   }
   erg = EvalFloatExpression(Asc, Float64, &OK);
   if (OK) {
      Double_2_TenBytes(erg, BAsmCode + CodeLen);
      CodeLen += 10;
      if (Turn)
         for (z = 0; z < 5; z++) {
            Exg = BAsmCode[CodeLen - 10 + z];
            BAsmCode[CodeLen - 10 + z] = BAsmCode[CodeLen - 1 - z];
            BAsmCode[CodeLen - 1 - z] = Exg;
         }
   }
   return OK;
}

static bool DecodeIntelPseudo_ValidSymChar(char ch) {
   return (((ch >= 'A') && (ch <= 'Z')) || ((ch >= '0') && (ch <= '9')) || (ch == '_') || (ch == '.'));
}

static bool DecodeIntelPseudo_LayoutMult(char *Asc_O, Word * Cnt, TLayoutFunc LayoutFunc, bool Turn) {
   Integer z, Depth, Fnd, ALen;
   String Asc, Part;
   Word Rep, SumCnt, ECnt, SInd;
   bool OK, Hyp;

   strmaxcpy(Asc, Asc_O, 255);

/* nach DUP suchen */

   Depth = 0;
   Fnd = 0;
   ALen = strlen(Asc);
   for (z = 0; z < ALen - 2; z++) {
      if (Asc[z] == '(') Depth++;
      else if (Asc[z] == ')') Depth--;
      else if (Depth == 0)
         if (((z == 0) || (!DecodeIntelPseudo_ValidSymChar(Asc[z - 1])))
            && (!DecodeIntelPseudo_ValidSymChar(Asc[z + 3]))
            && (strncasecmp(Asc + z, "DUP", 3) == 0)) Fnd = z;
   }

/* DUP gefunden: */

   if (Fnd != 0) {
   /* Anzahl ausrechnen */

      FirstPassUnknown = false;
      Asc[Fnd] = '\0';
      Rep = EvalIntExpression(Asc, Int32, &OK);
      Asc[Fnd] = 'D';
      if (FirstPassUnknown) {
         WrError(1820);
         return false;
      }
      if (!OK) return false;

   /* Einzelteile bilden & evaluieren */

      strmove(Asc, Fnd + 3);
      KillPrefBlanks(Asc);
      SumCnt = 0;
      if ((strlen(Asc) >= 2) && (*Asc == '(') && (Asc[strlen(Asc) - 1] == ')')) {
         strcpy(Asc, Asc + 1);
         Asc[strlen(Asc) - 1] = '\0';
      }
      do {
         Fnd = 0;
         z = 0;
         Hyp = false;
         Depth = 0;
         do {
            if (Asc[z] == '\'') Hyp = !Hyp;
            else if (!Hyp) {
               if (Asc[z] == '(') Depth++;
               else if (Asc[z] == ')') Depth--;
               else if ((Depth == 0) && (Asc[z] == ',')) Fnd = z;
            }
            z++;
         }
         while ((z < strlen(Asc)) && (Fnd == 0));
         if (Fnd == 0) {
            strmaxcpy(Part, Asc, 255);
            *Asc = '\0';
         } else {
            Asc[Fnd] = '\0';
            strmaxcpy(Part, Asc, 255);
            strmove(Asc, Fnd + 1);
         }
         if (!DecodeIntelPseudo_LayoutMult(Part, &ECnt, LayoutFunc, Turn))
            return false;
         SumCnt += ECnt;
      }
      while (*Asc != '\0');

   /* Ergebnis vervielfachen */

      if (DSFlag == DSConstant) {
         SInd = CodeLen - SumCnt;
         if (CodeLen + SumCnt * (Rep - 1) > MaxCodeLen) {
            WrError(1920);
            return false;
         }
         for (z = 1; z <= Rep - 1; z++) {
            if (CodeLen + SumCnt > MaxCodeLen) return false;
            memcpy(BAsmCode + CodeLen, BAsmCode + SInd, SumCnt);
            CodeLen += SumCnt;
         }
      } else CodeLen += SumCnt * (Rep - 1);
      *Cnt = SumCnt * Rep;
      return true;
   }

/* kein DUP: einfacher Ausdruck */

   else return LayoutFunc(Asc, Cnt, Turn);
}

bool DecodeIntelPseudo(bool Turn) {
   Word Dummy;
   Integer z;
   TLayoutFunc LayoutFunc = NULL;
   bool OK;
   LongInt HVal;

   if ((Memo("DB")) || (Memo("DW")) || (Memo("DD")) || (Memo("DQ")) || (Memo("DT"))) {
      DSFlag = DSNone;
      if (Memo("DB")) {
         LayoutFunc = LayoutByte;
         if (*LabPart != '\0') SetSymbolSize(LabPart, 0);
      } else if (Memo("DW")) {
         LayoutFunc = LayoutWord;
         if (*LabPart != '\0') SetSymbolSize(LabPart, 1);
      } else if (Memo("DD")) {
         LayoutFunc = LayoutDoubleWord;
         if (*LabPart != '\0') SetSymbolSize(LabPart, 2);
      } else if (Memo("DQ")) {
         LayoutFunc = LayoutQuadWord;
         if (*LabPart != '\0') SetSymbolSize(LabPart, 3);
      } else if (Memo("DT")) {
         LayoutFunc = LayoutTenBytes;
         if (*LabPart != '\0') SetSymbolSize(LabPart, 4);
      }
      z = 1;
      do {
         OK = DecodeIntelPseudo_LayoutMult(ArgStr[z], &Dummy, LayoutFunc, Turn);
         if (!OK) CodeLen = 0;
         z++;
      }
      while ((OK) && (z <= ArgCnt));
      DontPrint = (DSFlag == DSSpace);
      if ((MakeUseList) && (DontPrint))
         if (AddChunk(SegChunks + ActPC, ProgCounter(), CodeLen, ActPC == SegCode)) WrError(90);
      return true;
   }

   if (Memo("DS")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         FirstPassUnknown = false;
         HVal = EvalIntExpression(ArgStr[1], Int32, &OK);
         if (FirstPassUnknown) WrError(1820);
         else if (OK) {
            DontPrint = true;
            CodeLen = HVal;
            if (MakeUseList)
               if (AddChunk(SegChunks + ActPC, ProgCounter(), CodeLen, ActPC == SegCode)) WrError(90);
         }
      }
      return true;
   }

   return false;
}

bool DecodeMotoPseudo(bool Turn) {
   bool OK;
   Integer z;
   Word HVal16;
   TempResult t;
   String SVal;

   if ((Memo("BYT")) || (Memo("FCB"))) {
      if (ArgCnt == 0) WrError(1110);
      else {
         z = 1;
         OK = true;
         do {
            KillBlanks(ArgStr[z]);
            EvalExpression(ArgStr[z], &t);
            switch (t.Typ) {
               case TempInt:
                  if (!RangeCheck(t.Contents.Int, Int8)) {
                     WrError(1320);
                     OK = false;
                  } else if (CodeLen == MaxCodeLen) {
                     WrError(1920);
                     OK = false;
                  } else BAsmCode[CodeLen++] = t.Contents.Int;
                  break;
               case TempFloat:
                  WrError(1135);
                  OK = false;
                  break;
               case TempString:
                  TranslateString(t.Contents.Ascii);
                  if (CodeLen + strlen(t.Contents.Ascii) > MaxCodeLen) {
                     WrError(1920);
                     OK = false;
                  } else {
                     memcpy(BAsmCode + CodeLen, t.Contents.Ascii, strlen(t.Contents.Ascii));
                     CodeLen += strlen(t.Contents.Ascii);
                  }
                  break;
               default:
                  OK = false;
                  break;
            }
            z++;
         }
         while ((z <= ArgCnt) && (OK));
         if (!OK) CodeLen = 0;
      }
      return true;
   }

   if ((Memo("ADR")) || (Memo("FDB"))) {
      if (ArgCnt == 0) WrError(1110);
      else {
         z = 1;
         OK = true;
         do {
            HVal16 = EvalIntExpression(ArgStr[z], Int16, &OK);
            if (OK) {
               if (Turn) {
                  BAsmCode[CodeLen++] = Hi(HVal16);
                  BAsmCode[CodeLen++] = Lo(HVal16);
               } else {
                  BAsmCode[CodeLen++] = Lo(HVal16);
                  BAsmCode[CodeLen++] = Hi(HVal16);
               }
            }
            z++;
         }
         while ((z <= ArgCnt) && (OK));
         if (!OK) CodeLen = 0;
      }
      return true;
   }

   if (Memo("FCC")) {
      if (ArgCnt == 0) WrError(1110);
      else {
         z = 1;
         OK = true;
         do {
            EvalStringExpression(ArgStr[z], &OK, SVal);
            if (OK)
               if (CodeLen + strlen(SVal) >= MaxCodeLen) {
                  WrError(1920);
                  OK = false;
               } else {
                  TranslateString(SVal);
                  memcpy(BAsmCode + CodeLen, SVal, strlen(SVal));
                  CodeLen += strlen(SVal);
               }
            z++;
         }
         while ((z <= ArgCnt) && (OK));
         if (!OK) CodeLen = 0;
      }
      return true;
   }

   if ((Memo("DFS")) || (Memo("RMB"))) {
      if (ArgCnt != 1) WrError(1110);
      else {
         FirstPassUnknown = false;
         HVal16 = EvalIntExpression(ArgStr[1], Int16, &OK);
         if (FirstPassUnknown) WrError(1820);
         else if (OK) {
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

static void DigIns(char Ch, Byte Pos, Word * w) {
   Byte wpos = Pos >> 2, bpos = (Pos & 3) * 4;
   Word dig = Ch - '0';

   w[wpos] |= (dig << bpos);
}

void ConvertDec(Double F, Word * w) {
   char s[30], Man[30], Exp[30];
   char *h;
   LongInt z;
   Byte epos;

   sprintf(s, "%0.16e", F);
   h = strchr(s, 'e');
   if (h == NULL) {
      strcpy(Man, s);
      strcpy(Exp, "+0000");
   } else {
      *h = '\0';
      strcpy(Man, s);
      strcpy(Exp, h + 1);
   }
   memset(w, 0, 12);
   if (*Man == '-') {
      w[5] |= 0x8000;
      strcpy(Man, Man + 1);
   } else if (*Man == '+') strcpy(Man, Man + 1);
   if (*Exp == '-') {
      w[5] |= 0x4000;
      strcpy(Exp, Exp + 1);
   } else if (*Exp == '+') strcpy(Exp, Exp + 1);
   DigIns(*Man, 16, w);
   strcpy(Man, Man + 2);
   if (strlen(Man) > 16) Man[16] = '\0';
   for (z = 0; z < strlen(Man); z++) DigIns(Man[z], 15 - z, w);
   if (strlen(Exp) > 4) strmove(Exp, strlen(Exp) - 4);
   for (z = strlen(Exp) - 1; z >= 0; z--) {
      epos = strlen(Exp) - 1 - z;
      if (epos == 3) DigIns(Exp[z], 19, w);
      else DigIns(Exp[z], epos + 20, w);
   }
}

static void EnterByte(Byte b) {
   if (((CodeLen & 1) == 1) && (!BigEndian) && (ListGran() != 1)) {
      BAsmCode[CodeLen] = BAsmCode[CodeLen - 1];
      BAsmCode[CodeLen - 1] = b;
   } else {
      BAsmCode[CodeLen] = b;
   }
   CodeLen++;
}

bool DecodeMoto16Pseudo(ShortInt OpSize, bool Turn) {
#define ONOFFMoto16Count 1
   static ONOFFRec ONOFFMoto16s[ONOFFMoto16Count] = { { "PADDING", &DoPadding, DoPaddingName } };

   Byte z;
   Word TurnField[8];
   char *p, *zp;
   LongInt z2;
   LongInt WSize, Rep = 0;
   LongInt NewPC, HVal, WLen;
#ifdef HAS64
   QuadInt QVal;
#endif
   Integer HVal16;
   Double DVal;
   Single FVal;
   TempResult t;
   bool OK, ValOK;

   if (Turn); /* satisfy some compilers */

   if (OpSize < 0) OpSize = 1;

   if (CodeONOFF(ONOFFMoto16s, ONOFFMoto16Count)) return true;

   if (Memo("DC")) {
      if (ArgCnt == 0) WrError(1110);
      else {
         OK = true;
         z = 1;
         WLen = 0;
         do {
            FirstPassUnknown = false;
            if (*ArgStr[z] != '[') Rep = 1;
            else {
               strcpy(ArgStr[z], ArgStr[z] + 1);
               p = QuotPos(ArgStr[z], ']');
               if (p == NULL) {
                  WrError(1300);
                  OK = false;
               } else {
                  *p = '\0';
                  Rep = EvalIntExpression(ArgStr[z], Int32, &OK);
                  strcpy(ArgStr[z], p + 1);
               }
            }
            if (OK)
               if (FirstPassUnknown) WrError(1820);
               else {
                  switch (OpSize) {
                     case 0:
                        FirstPassUnknown = false;
                        EvalExpression(ArgStr[z], &t);
                        if ((FirstPassUnknown) && (t.Typ == TempInt)) t.Contents.Int &= 0xff;
                        switch (t.Typ) {
                           case TempInt:
                              if (!RangeCheck(t.Contents.Int, Int8)) {
                                 WrError(1320);
                                 OK = false;
                              } else if (CodeLen + Rep > MaxCodeLen) {
                                 WrError(1920);
                                 OK = false;
                                 } else for (z2 = 0; z2 < Rep; z2++) EnterByte(t.Contents.Int);
                              break;
                           case TempFloat:
                              WrError(1135);
                              OK = false;
                              break;
                           case TempString:
                              if (CodeLen + Rep * strlen(t.Contents.Ascii) > MaxCodeLen) {
                                 WrError(1920);
                                 OK = false;
                              } else
                                 for (z2 = 0; z2 < Rep; z2++)
                                    for (zp = t.Contents.Ascii; *zp != '\0'; EnterByte(CharTransTable[(unsigned int)*(zp++)]));
                              break;
                           default:
                              OK = false;
                        }
                        break;
                     case 1:
                        HVal16 = EvalIntExpression(ArgStr[z], Int16, &OK);
                        if (OK)
                           if (CodeLen + (Rep << 1) > MaxCodeLen) {
                              WrError(1920);
                              OK = false;
                           } else {
                              if (ListGran() == 1)
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    BAsmCode[(WLen << 1)] = Hi(HVal16);
                                    BAsmCode[(WLen << 1) + 1] = Lo(HVal16);
                                    WLen++;
                              } else
                                 for (z2 = 0; z2 < Rep; z2++) WAsmCode[WLen++] = HVal16;
                              CodeLen += Rep << 1;
                           }
                        break;
                     case 2:
                        HVal = EvalIntExpression(ArgStr[z], Int32, &OK);
                        if (OK)
                           if (CodeLen + (Rep << 2) > MaxCodeLen) {
                              WrError(1920);
                              OK = false;
                           } else {
                              if (ListGran() == 1)
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    BAsmCode[(WLen << 1)] = (HVal >> 24) & 0xff;
                                    BAsmCode[(WLen << 1) + 1] = (HVal >> 16) & 0xff;
                                    BAsmCode[(WLen << 1) + 2] = (HVal >> 8) & 0xff;
                                    BAsmCode[(WLen << 1) + 3] = (HVal) & 0xff;
                                    WLen += 2;
                              } else
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    WAsmCode[WLen++] = HVal >> 16;
                                    WAsmCode[WLen++] = HVal & 0xffff;
                                 }
                              CodeLen += Rep << 2;
                           }
                        break;
#ifdef HAS64
                     case 3:
                        QVal = EvalIntExpression(ArgStr[z], Int64, &OK);
                        if (OK)
                           if (CodeLen + (Rep << 3) > MaxCodeLen) {
                              WrError(1920);
                              OK = false;
                           } else {
                              if (ListGran() == 1)
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    BAsmCode[(WLen << 1)] = (QVal >> 56) & 0xff;
                                    BAsmCode[(WLen << 1) + 1] = (QVal >> 48) & 0xff;
                                    BAsmCode[(WLen << 1) + 2] = (QVal >> 40) & 0xff;
                                    BAsmCode[(WLen << 1) + 3] = (QVal >> 32) & 0xff;
                                    BAsmCode[(WLen << 1) + 4] = (QVal >> 24) & 0xff;
                                    BAsmCode[(WLen << 1) + 5] = (QVal >> 16) & 0xff;
                                    BAsmCode[(WLen << 1) + 6] = (QVal >> 8) & 0xff;
                                    BAsmCode[(WLen << 1) + 7] = (QVal) & 0xff;
                                    WLen += 4;
                              } else
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    WAsmCode[WLen++] = (QVal >> 48) & 0xffff;
                                    WAsmCode[WLen++] = (QVal >> 32) & 0xffff;
                                    WAsmCode[WLen++] = (QVal >> 16) & 0xffff;
                                    WAsmCode[WLen++] = QVal & 0xffff;
                                 }
                              CodeLen += Rep << 3;
                           }
                        break;
#endif
                     case 4:
                        FVal = EvalFloatExpression(ArgStr[z], Float32, &OK);
                        if (OK)
                           if (CodeLen + (Rep << 2) > MaxCodeLen) {
                              WrError(1920);
                              OK = false;
                           } else {
                              memcpy(TurnField, &FVal, 4);
                              if (BigEndian) DWSwap((void *)TurnField, 4);
                              if (ListGran() == 1)
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    BAsmCode[(WLen << 1)] = Hi(TurnField[1]);
                                    BAsmCode[(WLen << 1) + 1] = Lo(TurnField[1]);
                                    BAsmCode[(WLen << 1) + 2] = Hi(TurnField[0]);
                                    BAsmCode[(WLen << 1) + 3] = Lo(TurnField[0]);
                                    WLen += 2;
                              } else
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    WAsmCode[WLen++] = TurnField[1];
                                    WAsmCode[WLen++] = TurnField[0];
                                 }
                              CodeLen += Rep << 2;
                           }
                        break;
                     case 5:
                        DVal = EvalFloatExpression(ArgStr[z], Float64, &OK);
                        if (OK)
                           if (CodeLen + (Rep << 3) > MaxCodeLen) {
                              WrError(1920);
                              OK = false;
                           } else {
                              memcpy(TurnField, &DVal, 8);
                              if (BigEndian) QWSwap((void *)TurnField, 8);
                              if (ListGran() == 1)
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    BAsmCode[(WLen << 1)] = Hi(TurnField[3]);
                                    BAsmCode[(WLen << 1) + 1] = Lo(TurnField[3]);
                                    BAsmCode[(WLen << 1) + 2] = Hi(TurnField[2]);
                                    BAsmCode[(WLen << 1) + 3] = Lo(TurnField[2]);
                                    BAsmCode[(WLen << 1) + 4] = Hi(TurnField[1]);
                                    BAsmCode[(WLen << 1) + 5] = Lo(TurnField[1]);
                                    BAsmCode[(WLen << 1) + 6] = Hi(TurnField[0]);
                                    BAsmCode[(WLen << 1) + 7] = Lo(TurnField[0]);
                                    WLen += 4;
                              } else
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    WAsmCode[WLen++] = TurnField[3];
                                    WAsmCode[WLen++] = TurnField[2];
                                    WAsmCode[WLen++] = TurnField[1];
                                    WAsmCode[WLen++] = TurnField[0];
                                 }
                              CodeLen += Rep << 3;
                           }
                        break;
                     case 6:
                        DVal = EvalFloatExpression(ArgStr[z], Float64, &OK);
                        if (OK)
                           if (CodeLen + (Rep * 12) > MaxCodeLen) {
                              WrError(1920);
                              OK = false;
                           } else {
                              Double_2_TenBytes(DVal, (Byte *) TurnField);
                              if (BigEndian) WSwap((void *)TurnField, 10);
                              if (ListGran() == 1)
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    BAsmCode[(WLen << 1)] = Hi(TurnField[4]);
                                    BAsmCode[(WLen << 1) + 1] = Lo(TurnField[4]);
                                    BAsmCode[(WLen << 1) + 2] = 0;
                                    BAsmCode[(WLen << 1) + 3] = 0;
                                    BAsmCode[(WLen << 1) + 4] = Hi(TurnField[3]);
                                    BAsmCode[(WLen << 1) + 5] = Lo(TurnField[3]);
                                    BAsmCode[(WLen << 1) + 6] = Hi(TurnField[2]);
                                    BAsmCode[(WLen << 1) + 7] = Lo(TurnField[2]);
                                    BAsmCode[(WLen << 1) + 8] = Hi(TurnField[1]);
                                    BAsmCode[(WLen << 1) + 9] = Lo(TurnField[1]);
                                    BAsmCode[(WLen << 1) + 10] = Hi(TurnField[0]);
                                    BAsmCode[(WLen << 1) + 11] = Lo(TurnField[0]);
                                    WLen += 6;
                              } else
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    WAsmCode[WLen++] = TurnField[4];
                                    WAsmCode[WLen++] = 0;
                                    WAsmCode[WLen++] = TurnField[3];
                                    WAsmCode[WLen++] = TurnField[2];
                                    WAsmCode[WLen++] = TurnField[1];
                                    WAsmCode[WLen++] = TurnField[0];
                                 }
                              CodeLen += Rep * 12;
                           }
                        break;
                     case 7:
                        DVal = EvalFloatExpression(ArgStr[z], Float64, &OK);
                        if (OK)
                           if (CodeLen + (Rep * 12) > MaxCodeLen) {
                              WrError(1920);
                              OK = false;
                           } else {
                              ConvertDec(DVal, TurnField);
                              if (ListGran() == 1)
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    BAsmCode[(WLen << 1)] = Hi(TurnField[5]);
                                    BAsmCode[(WLen << 1) + 1] = Lo(TurnField[5]);
                                    BAsmCode[(WLen << 1) + 2] = Hi(TurnField[4]);
                                    BAsmCode[(WLen << 1) + 3] = Lo(TurnField[4]);
                                    BAsmCode[(WLen << 1) + 4] = Hi(TurnField[3]);
                                    BAsmCode[(WLen << 1) + 5] = Lo(TurnField[3]);
                                    BAsmCode[(WLen << 1) + 6] = Hi(TurnField[2]);
                                    BAsmCode[(WLen << 1) + 7] = Lo(TurnField[2]);
                                    BAsmCode[(WLen << 1) + 8] = Hi(TurnField[1]);
                                    BAsmCode[(WLen << 1) + 9] = Lo(TurnField[1]);
                                    BAsmCode[(WLen << 1) + 10] = Hi(TurnField[0]);
                                    BAsmCode[(WLen << 1) + 11] = Lo(TurnField[0]);
                                    WLen += 6;
                              } else
                                 for (z2 = 0; z2 < Rep; z2++) {
                                    WAsmCode[WLen++] = TurnField[5];
                                    WAsmCode[WLen++] = TurnField[4];
                                    WAsmCode[WLen++] = TurnField[3];
                                    WAsmCode[WLen++] = TurnField[2];
                                    WAsmCode[WLen++] = TurnField[1];
                                    WAsmCode[WLen++] = TurnField[0];
                                 }
                              CodeLen += Rep * 12;
                           }
                        break;
                  }
               }
            z++;
         }
         while ((z <= ArgCnt) && (OK));
         if (!OK) CodeLen = 0;
         if ((DoPadding) && ((CodeLen & 1) == 1)) EnterByte(0);
      }
      return true;
   }

   if (Memo("DS")) {
      if (ArgCnt != 1) WrError(1110);
      else {
         FirstPassUnknown = false;
         HVal = EvalIntExpression(ArgStr[1], Int32, &ValOK);
         if (FirstPassUnknown) WrError(1820);
         if ((ValOK) && (!FirstPassUnknown)) {
            DontPrint = true;
            switch (OpSize) {
               case 0:
                  WSize = 1;
                  if (((HVal & 1) == 1) && (DoPadding)) HVal++;
                  break;
               case 1:
                  WSize = 2;
                  break;
               case 2:
               case 4:
                  WSize = 4;
                  break;
               case 3:
               case 5:
                  WSize = 8;
                  break;
               case 6:
               case 7:
                  WSize = 12;
                  break;
               default:
                  WSize = 0;
            }
            if (HVal == 0) {
               NewPC = ProgCounter() + WSize - 1;
               NewPC = NewPC - (NewPC % WSize);
               CodeLen = NewPC - ProgCounter();
               if (CodeLen == 0) DontPrint = false;
            } else CodeLen = HVal * WSize;
            if ((MakeUseList) && (DontPrint))
               if (AddChunk(SegChunks + ActPC, ProgCounter(), CodeLen, ActPC == SegCode)) WrError(90);
         }
      }
      return true;
   }

   return false;
}

void CodeEquate(ShortInt DestSeg, LargeInt Min, LargeInt Max) {
   bool OK;
   TempResult t;
   LargeInt Erg;

   FirstPassUnknown = false;
   if (ArgCnt != 1) WrError(1110);
   else {
      Erg = EvalIntExpression(ArgStr[1], Int32, &OK);
      if ((OK) && (!FirstPassUnknown))
         if (Min > Erg) WrError(1315);
         else if (Erg > Max) WrError(1320);
         else {
            PushLocHandle(-1);
            EnterIntSymbol(LabPart, Erg, DestSeg, false);
            PopLocHandle();
            if ((MakeUseList) && (MakeUseList))
               if (AddChunk(SegChunks + DestSeg, Erg, 1, false)) WrError(90);
            t.Typ = TempInt;
            t.Contents.Int = Erg;
            SetListLineVal(&t);
         }
   }
}

void CodeASSUME(ASSUMERec * Def, Integer Cnt) {
   Integer z1, z2;
   bool OK;
   LongInt HVal;
   String RegPart, ValPart;

   if (ArgCnt == 0) WrError(1110);
   else {
      z1 = 1;
      OK = true;
      while ((z1 <= ArgCnt) && (OK)) {
         SplitString(ArgStr[z1], RegPart, ValPart, QuotPos(ArgStr[z1], ':'));
         z2 = 0;
         NLS_UpString(RegPart);
         while ((z2 < Cnt) && (strcmp(Def[z2].Name, RegPart) != 0)) z2++;
         OK = (z2 < Cnt);
         if (!OK) WrXError(1980, RegPart);
         else if (strcmp(ValPart, "NOTHING") == 0)
            if (Def[z2].NothingVal == -1) WrError(1350);
            else *Def[z2].Dest = Def[z2].NothingVal;
         else {
            FirstPassUnknown = false;
            HVal = EvalIntExpression(ValPart, Int32, &OK);
            if (OK)
               if (FirstPassUnknown) {
                  WrError(1820);
                  OK = false;
               } else if (ChkRange(HVal, Def[z2].Min, Def[z2].Max)) *Def[z2].Dest = HVal;
         }
         z1++;
      }
   }
}

bool CodeONOFF(ONOFFRec * Def, Integer Cnt) {
   Integer z;
   bool OK;

   for (z = 0; z < Cnt; z++)
      if (Memo(Def[z].Name)) {
         if (ArgCnt != 1) WrError(1110);
         else {
            NLS_UpString(ArgStr[1]);
            if (*AttrPart != '\0') WrError(1100);
            else if ((strcmp(ArgStr[1], "ON") != 0) && (strcmp(ArgStr[1], "OFF") != 0)) WrError(1520);
            else {
               OK = (strcmp(ArgStr[1], "ON") == 0);
               SetFlag(Def[z].Dest, Def[z].FlagName, OK);
            }
         }
         return true;
      }

   return false;
}

void codepseudo_init(void) {
}
