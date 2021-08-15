/* codeallg.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* von allen Codegeneratoren benutzte Pseudobefehle                          */
/*                                                                           */
/* Historie:  10. 5.1996 Grundsteinlegung                                    */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "nls.h"
#include "stringutil.h"
#include "stringlists.h"
#include "bpemu.h"
#include "decodecmd.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmmac.h"
#include "asmcode.h"
#include "codepseudo.h"

void SetCPU(CPUVar NewCPU, bool NotPrev) {
   LongInt HCPU;
   char *z, *dest;
   bool ECPU;
   char s[11];
   PCPUDef Lauf;

   Lauf = FirstCPUDef;
   while ((Lauf != NULL) && (Lauf->Number != NewCPU)) Lauf = Lauf->Next;
   if (Lauf == NULL) return;

   strmaxcpy(MomCPUIdent, Lauf->Name, 11);
   MomCPU = Lauf->Orig;
   MomVirtCPU = Lauf->Number;
   strmaxcpy(s, MomCPUIdent, 11);
   for (z = dest = s; *z != '\0'; z++)
      if (((*z >= '0') && (*z <= '9')) || ((*z >= 'A') && (*z <= 'F')))
         *(dest++) = (*z);
   *dest = '\0';
   for (z = s; *z != '\0'; z++)
      if ((*z >= '0') && (*z <= '9')) break;
   if (*z != '\0') strcpy(s, z);
   strprep(s, "$");
   HCPU = ConstLongInt(s, &ECPU);
   if (ParamCount != 0) {
      EnterIntSymbol(MomCPUName, HCPU, SegNone, true);
      EnterStringSymbol(MomCPUIdentName, MomCPUIdent, true);
   }

   InternSymbol = Default_InternSymbol;
   if (!NotPrev) SwitchFrom();
   Lauf->SwitchProc();

   DontPrint = true;
}

char *IntLine(LongInt Inp) {
   static String s;

   switch (ConstMode) {
      case ConstModeIntel:
         sprintf(s, "%sH", HexString(Inp, 0));
         if (*s > '9') strmaxprep(s, "0", 255);
         break;
      case ConstModeMoto:
         sprintf(s, "$%s", HexString(Inp, 0));
         break;
      case ConstModeC:
         sprintf(s, "0x%s", HexString(Inp, 0));
         break;
   }

   return s;
}

static void CodeSECTION(void) {
   PSaveSection Neu;

   if (ArgCnt != 1) WrError(1110);
   else if (ExpandSymbol(ArgStr[1]))
      if (!ChkSymbName(ArgStr[1])) WrXError(1020, ArgStr[1]);
      else if ((PassNo == 1) && (GetSectionHandle(ArgStr[1], false, MomSectionHandle) != -2)) WrError(1483);
      else {
         Neu = (PSaveSection) malloc(sizeof(TSaveSection));
         Neu->Next = SectionStack;
         Neu->Handle = MomSectionHandle;
         Neu->LocSyms = NULL;
         Neu->GlobSyms = NULL;
         Neu->ExportSyms = NULL;
         SetMomSection(GetSectionHandle(ArgStr[1], true, MomSectionHandle));
         SectionStack = Neu;
      }
}

static void CodeENDSECTION_ChkEmptList(PForwardSymbol * Root) {
   PForwardSymbol Tmp;

   while (*Root != NULL) {
      WrXError(1488, (*Root)->Name);
      free((*Root)->Name);
      Tmp = (*Root);
      *Root = Tmp->Next;
      free(Tmp);
   }
}

static void CodeENDSECTION(void) {
   PSaveSection Tmp;

   if (ArgCnt > 1) WrError(1110);
   else if (SectionStack == NULL) WrError(1487);
   else if ((ArgCnt == 0) || (ExpandSymbol(ArgStr[1])))
      if ((ArgCnt == 1) && (GetSectionHandle(ArgStr[1], false, SectionStack->Handle) != MomSectionHandle)) WrError(1486);
      else {
         Tmp = SectionStack;
         SectionStack = Tmp->Next;
         CodeENDSECTION_ChkEmptList(&(Tmp->LocSyms));
         CodeENDSECTION_ChkEmptList(&(Tmp->GlobSyms));
         CodeENDSECTION_ChkEmptList(&(Tmp->ExportSyms));
         if (ArgCnt == 0)
            sprintf(ListLine, "[%s]", GetSectionName(MomSectionHandle));
         SetMomSection(Tmp->Handle);
         free(Tmp);
      }
}

static void CodeCPU(void) {
   PCPUDef Lauf;

   if (ArgCnt != 1) WrError(1110);
   else if (*AttrPart != '\0') WrError(1100);
   else {
      NLS_UpString(ArgStr[1]);
      Lauf = FirstCPUDef;
      while ((Lauf != NULL) && (strcmp(ArgStr[1], Lauf->Name) != 0))
         Lauf = Lauf->Next;
      if (Lauf == NULL) WrXError(1430, ArgStr[1]);
      else {
         SetCPU(Lauf->Number, false);
         ActPC = SegCode;
      }
   }
}

static void CodeSETEQU(void) {
   TempResult t;
   bool MayChange;
   Integer DestSeg;

   FirstPassUnknown = false;
   MayChange = ((!Memo("EQU")) && (!Memo("=")));
   if (*AttrPart != '\0') WrError(1100);
   else if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
   else {
      EvalExpression(ArgStr[1], &t);
      if (!FirstPassUnknown) {
         if (ArgCnt == 1) DestSeg = SegNone;
         else {
            NLS_UpString(ArgStr[2]);
            if (strcmp(ArgStr[2], "MOMSEGMENT") == 0) DestSeg = ActPC;
            else {
               DestSeg = 0;
               while ((DestSeg <= PCMax) && (strcmp(ArgStr[2], SegNames[DestSeg]) != 0))
                  DestSeg++;
            }
         }
         if (DestSeg > PCMax) WrXError(1961, ArgStr[2]);
         else {
            SetListLineVal(&t);
            PushLocHandle(-1);
            switch (t.Typ) {
               case TempInt:
                  EnterIntSymbol(LabPart, t.Contents.Int, DestSeg, MayChange);
                  break;
               case TempFloat:
                  EnterFloatSymbol(LabPart, t.Contents.Float, MayChange);
                  break;
               case TempString:
                  EnterStringSymbol(LabPart, t.Contents.Ascii, MayChange);
                  break;
               case TempNone:
                  break;
            }
            PopLocHandle();
         }
      }
   }
}

static void CodeORG(void) {
   LargeInt HVal;
   bool ValOK;

   FirstPassUnknown = false;
   if (*AttrPart != '\0') WrError(1100);
   else if (ArgCnt != 1) WrError(1110);
   else {
#ifndef HAS64
      HVal = EvalIntExpression(ArgStr[1], UInt32, &ValOK);
#else
      HVal = EvalIntExpression(ArgStr[1], Int64, &ValOK);
#endif
      if (FirstPassUnknown) WrError(1820);
      if ((ValOK) && (!FirstPassUnknown)) {
         PCs[ActPC] = HVal;
         DontPrint = true;
      }
   }
}

static void CodeSHARED_BuildComment(char *c) {
   switch (ShareMode) {
      case 1:
         sprintf(c, "(* %s *)", CommPart);
         break;
      case 2:
         sprintf(c, "/* %s */", CommPart);
         break;
      case 3:
         sprintf(c, "; %s", CommPart);
         break;
   }
}

static void CodeSHARED(void) {
   Integer z;
   bool ValOK;
   LargeInt HVal;
   Double FVal;
   String s, c;

   if (ShareMode == 0) WrError(30);
   else if ((ArgCnt == 0) && (*CommPart != '\0')) {
      CodeSHARED_BuildComment(c);
      errno = 0;
      fprintf(ShareFile, "%s\n", c);
      ChkIO(10004);
   } else
      for (z = 1; z <= ArgCnt; z++) {
         if (IsSymbolString(ArgStr[z])) {
            ValOK = GetStringSymbol(ArgStr[z], s);
            if (ShareMode == 1) {
               strmaxprep(s, "\'", 255);
               strmaxcat(s, "\'", 255);
            } else {
               strmaxprep(s, "\"", 255);
               strmaxcat(s, "\"", 255);
            }
         } else if (IsSymbolFloat(ArgStr[z])) {
            ValOK = GetFloatSymbol(ArgStr[z], &FVal);
            sprintf(s, "%0.17g", FVal);
         } else {
            ValOK = GetIntSymbol(ArgStr[z], &HVal);
            switch (ShareMode) {
               case 1:
                  sprintf(s, "$%s", HexString(HVal, 0));
                  break;
               case 2:
                  sprintf(s, "0x%s", HexString(HVal, 0));
                  break;
               case 3:
                  strmaxcpy(s, IntLine(HVal), 255);
                  break;
            }
         }
         if (ValOK) {
            if ((z == 1) && (*CommPart != '\0')) {
               CodeSHARED_BuildComment(c);
               strmaxprep(c, " ", 255);
            } else *c = '\0';
            errno = 0;
            switch (ShareMode) {
               case 1:
                  fprintf(ShareFile, "%s = %s;%s\n", ArgStr[z], s, c);
                  break;
               case 2:
                  fprintf(ShareFile, "#define %s %s%s\n", ArgStr[z], s, c);
                  break;
               case 3:
                  strmaxprep(s, IsSymbolChangeable(ArgStr[z]) ? "set " : "equ ", 255);
                  fprintf(ShareFile, "%s %s%s\n", ArgStr[z], s, c);
                  break;
            }
            ChkIO(10004);
         } else if (PassNo == 1) {
            Repass = true;
            if ((MsgIfRepass) && (PassNo >= PassNoForMessage)) WrXError(170, ArgStr[z]);
         }
      }
}

static void CodePAGE(void) {
   Integer LVal, WVal;
   bool ValOK;

   if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
   else if (*AttrPart != '\0') WrError(1100);
   else {
      LVal = EvalIntExpression(ArgStr[1], UInt8, &ValOK);
      if (ValOK) {
         if ((LVal < 5) && (LVal != 0)) LVal = 5;
         if (ArgCnt == 1) {
            WVal = 0;
            ValOK = true;
         } else WVal = EvalIntExpression(ArgStr[2], UInt8, &ValOK);
         if (ValOK) {
            if ((WVal < 5) && (WVal != 0)) WVal = 5;
            PageLength = LVal;
            PageWidth = WVal;
         }
      }
   }
}

static void CodeNEWPAGE(void) {
   ShortInt HVal8;
   bool ValOK;

   if (ArgCnt > 1) WrError(1110);
   else if (*AttrPart != '\0') WrError(1100);
   else {
      if (ArgCnt == 0) {
         HVal8 = 0;
         ValOK = true;
      } else HVal8 = EvalIntExpression(ArgStr[1], Int8, &ValOK);
      if ((ValOK) || (ArgCnt == 0)) {
         if (HVal8 > ChapMax) HVal8 = ChapMax;
         else if (HVal8 < 0) HVal8 = 0;
         NewPage(HVal8, true);
      }
   }
}

static void CodeString(char *erg) {
   String tmp;
   bool OK;

   if (ArgCnt != 1) WrError(1110);
   else {
      EvalStringExpression(ArgStr[1], &OK, tmp);
      if (!OK) WrError(1970);
      else strmaxcpy(erg, tmp, 255);
   }
}

static void CodePHASE(void) {
   bool OK;
   LongInt HVal;

   if (ArgCnt != 1) WrError(1110);
   else {
      HVal = EvalIntExpression(ArgStr[1], Int32, &OK);
      if (OK) Phases[ActPC] = HVal - ProgCounter();
   }
}

static void CodeDEPHASE(void) {
   if (ArgCnt != 0) WrError(1110);
   else Phases[ActPC] = 0;
}

static void CodeWARNING(void) {
   String mess;
   bool OK;

   if (ArgCnt != 1) WrError(1110);
   else {
      EvalStringExpression(ArgStr[1], &OK, mess);
      if (!OK) WrError(1970);
      else WrErrorString(mess, "", true, false);
   }
}

static void CodeMESSAGE(void) {
   String mess;
   bool OK;

   if (ArgCnt != 1) WrError(1110);
   else {
      EvalStringExpression(ArgStr[1], &OK, mess);
      if (!OK) WrError(1970);
      printf("%s%s\n", mess, ClrEol);
      if (strcmp(LstName, "/dev/null") != 0) WrLstLine(mess);
   }
}

static void CodeERROR(void) {
   String mess;
   bool OK;

   if (ArgCnt != 1) WrError(1110);
   else {
      EvalStringExpression(ArgStr[1], &OK, mess);
      if (!OK) WrError(1970);
      else WrErrorString(mess, "", false, false);
   }
}

static void CodeFATAL(void) {
   String mess;
   bool OK;

   if (ArgCnt != 1) WrError(1110);
   else {
      EvalStringExpression(ArgStr[1], &OK, mess);
      if (!OK) WrError(1970);
      else WrErrorString(mess, "", false, true);
   }
}

static void CodeCHARSET(void) {
   Byte w1, w2, w3;
   Integer ch;
   bool OK;

   if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
   else {
      w1 = EvalIntExpression(ArgStr[1], Int8, &OK);
      if (OK) {
         w3 = EvalIntExpression(ArgStr[ArgCnt], Int8, &OK);
         if (OK) {
            if (ArgCnt == 2) {
               w2 = w1;
               OK = true;
            } else w2 = EvalIntExpression(ArgStr[2], Int8, &OK);
            if (OK) {
               if (w1 > w2) WrError(1320);
               else
                  for (ch = w1; ch <= w2; ch++)
                     CharTransTable[ch] = ch - w1 + w3;
            }
         }
      }
   }
}

static void CodeFUNCTION(void) {
   String FName;
   bool OK;
   Integer z;

   if (ArgCnt < 2) WrError(1110);
   else {
      OK = true;
      z = 1;
      do {
         OK = (OK && ChkMacSymbName(ArgStr[z]));
         if (!OK) WrXError(1020, ArgStr[z]);
         z++;
      }
      while ((z < ArgCnt) && (OK));
      if (OK) {
         strmaxcpy(FName, ArgStr[ArgCnt], 255);
         for (z = 1; z < ArgCnt; z++)
            CompressLine(ArgStr[z], z, FName);
         EnterFunction(LabPart, FName, ArgCnt - 1);
      }
   }
}

static void CodeSAVE(void) {
   PSaveState Neu;

   if (ArgCnt != 0) WrError(1110);
   else {
      Neu = (PSaveState) malloc(sizeof(TSaveState));
      Neu->Next = FirstSaveState;
      Neu->SaveCPU = MomCPU;
      Neu->SavePC = ActPC;
      Neu->SaveListOn = ListOn;
      Neu->SaveLstMacroEx = LstMacroEx;
      FirstSaveState = Neu;
   }
}

static void CodeRESTORE(void) {
   PSaveState Old;

   if (ArgCnt != 0) WrError(1110);
   else if (FirstSaveState == NULL) WrError(1450);
   else {
      Old = FirstSaveState;
      FirstSaveState = Old->Next;
      if (Old->SavePC != ActPC) {
         ActPC = Old->SavePC;
         DontPrint = true;
      }
      if (Old->SaveCPU != MomCPU) SetCPU(Old->SaveCPU, false);
      EnterIntSymbol(ListOnName, ListOn = Old->SaveListOn, 0, true);
      SetFlag(&LstMacroEx, LstMacroExName, Old->SaveLstMacroEx);
      free(Old);
   }
}

static void CodeSEGMENT(void) {
   Byte SegZ;
   Word Mask;
   bool Found;

   if (ArgCnt != 1) WrError(1110);
   else {
      Found = false;
      NLS_UpString(ArgStr[1]);
      for (SegZ = 1, Mask = 2; SegZ <= PCMax; SegZ++, Mask <<= 1)
         if (((ValidSegs & Mask) != 0) && (strcmp(ArgStr[1], SegNames[SegZ]) == 0)) {
            Found = true;
            if (ActPC != SegZ) {
               ActPC = SegZ;
               if (!PCsUsed[ActPC]) PCs[ActPC] = SegInits[ActPC];
               PCsUsed[ActPC] = true;
               DontPrint = true;
            }
         }
      if (!Found) WrXError(1961, ArgStr[1]);
   }
}

static void CodeLABEL(void) {
   LongInt Erg;
   bool OK;

   FirstPassUnknown = false;
   if (ArgCnt != 1) WrError(1110);
   else {
      Erg = EvalIntExpression(ArgStr[1], Int32, &OK);
      if ((OK) && (!FirstPassUnknown)) {
         PushLocHandle(-1);
         EnterIntSymbol(LabPart, Erg, SegCode, false);
         sprintf(ListLine, "=%s", IntLine(Erg));
         PopLocHandle();
      }
   }
}

static void CodeREAD(void) {
   String Exp;
   TempResult Erg;
   bool OK;
   LongInt SaveLocHandle;

   if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
   else {
      if (ArgCnt == 2) EvalStringExpression(ArgStr[1], &OK, Exp);
      else {
         sprintf(Exp, "Read %s ? ", ArgStr[1]);
         OK = true;
      }
      if (OK) {
         setbuf(stdout, NULL);
         printf("%s", Exp);
         fgets(Exp, 255, stdin);
         UpString(Exp);
         FirstPassUnknown = false;
         EvalExpression(Exp, &Erg);
         if (OK) {
            SetListLineVal(&Erg);
            SaveLocHandle = MomLocHandle;
            MomLocHandle = (-1);
            if (FirstPassUnknown) WrError(1820);
            else switch (Erg.Typ) {
                  case TempInt:
                     EnterIntSymbol(ArgStr[ArgCnt], Erg.Contents.Int, SegNone, true);
                     break;
                  case TempFloat:
                     EnterFloatSymbol(ArgStr[ArgCnt], Erg.Contents.Float, true);
                     break;
                  case TempString:
                     EnterStringSymbol(ArgStr[ArgCnt], Erg.Contents.Ascii, true);
                     break;
                  case TempNone:
                     break;
            }
            MomLocHandle = SaveLocHandle;
         }
      }
   }
}

static void CodeALIGN(void) {
   Word Dummy;
   bool OK;
   LongInt NewPC;

   if (ArgCnt != 1) WrError(1110);
   else {
      FirstPassUnknown = false;
      Dummy = EvalIntExpression(ArgStr[1], Int16, &OK);
      if (OK)
         if (FirstPassUnknown) WrError(1820);
         else {
            NewPC = ProgCounter() + Dummy - 1;
            NewPC -= NewPC % Dummy;
            CodeLen = NewPC - ProgCounter();
            DontPrint = (CodeLen != 0);
            if ((MakeUseList) && (DontPrint))
               if (AddChunk(SegChunks + ActPC, ProgCounter(), CodeLen, ActPC == SegCode)) WrError(90);
         }
   }
}

static void CodeENUM(void) {
   Integer z;
   char *p = NULL;
   bool OK;
   LongInt Counter, First = 0, Last = 0;
   String SymPart;

   Counter = 0;
   if (ArgCnt == 0) WrError(1110);
   else
      for (z = 1; z <= ArgCnt; z++) {
         p = QuotPos(ArgStr[z], '=');
         if (p != NULL) {
            strmaxcpy(SymPart, p + 1, 255);
            FirstPassUnknown = false;
            Counter = EvalIntExpression(SymPart, Int32, &OK);
            if (!OK) return;
            if (FirstPassUnknown) {
               WrXError(1820, SymPart);
               return;
            }
            *p = '\0';
         }
         EnterIntSymbol(ArgStr[z], Counter, SegNone, false);
         if (z == 1) First = Counter;
         else if (z == ArgCnt) Last = Counter;
         Counter++;
      }
   sprintf(ListLine, "=%s", IntLine(First));
   if (ArgCnt != 1) {
      strmaxcat(ListLine, "..", 255);
      strmaxcat(ListLine, IntLine(Last), 255);
   }
}

static void CodeEND(void) {
   LongInt HVal;
   bool OK;

   if (ArgCnt > 1) WrError(1110);
   else {
      if (ArgCnt == 1) {
         FirstPassUnknown = false;
         HVal = EvalIntExpression(ArgStr[1], Int32, &OK);
         if ((OK) && (!FirstPassUnknown)) {
            ChkSpace(SegCode);
            StartAdr = HVal;
            StartAdrPresent = true;
         }
      }
      ENDOccured = true;
   }
}

static void CodeLISTING(void) {
   Byte Value = 0xff;
   bool OK;

   if (ArgCnt != 1) WrError(1110);
   else if (*AttrPart != '\0') WrError(1100);
   else {
      OK = true;
      NLS_UpString(ArgStr[1]);
      if (strcmp(ArgStr[1], "OFF") == 0) Value = 0;
      else if (strcmp(ArgStr[1], "ON") == 0) Value = 1;
      else if (strcmp(ArgStr[1], "NOSKIPPED") == 0) Value = 2;
      else if (strcmp(ArgStr[1], "PURECODE") == 0) Value = 3;
      else OK = false;
      if (!OK) WrError(1520);
      else EnterIntSymbol(ListOnName, ListOn = Value, 0, true);
   }
}

static void CodeBINCLUDE(void) {
   FILE *F;
   LongInt Len = (-1);
   LongWord Ofs = 0, Curr, Rest;
   Word RLen;
   bool OK, SaveTurnWords;
   String Name;

   if ((ArgCnt < 1) || (ArgCnt > 3)) WrError(1110);
   else if (ActPC != SegCode) WrError(1940);
   else {
      if (ArgCnt == 1) OK = true;
      else {
         FirstPassUnknown = false;
         Ofs = EvalIntExpression(ArgStr[2], Int32, &OK);
         if (FirstPassUnknown) {
            WrError(1820);
            OK = false;
         }
         if (OK)
            if (ArgCnt == 2) Len = (-1);
            else {
               Len = EvalIntExpression(ArgStr[3], Int32, &OK);
               if (FirstPassUnknown) {
                  WrError(1820);
                  OK = false;
               }
            }
      }
      if (OK) {
         strmaxcpy(Name, ArgStr[1], 255);
         if (*Name == '"') strcpy(Name, Name + 1);
         if (Name[strlen(Name) - 1] == '"') Name[strlen(Name) - 1] = '\0';
         strmaxcpy(ArgStr[1], Name, 255);
         strmaxcpy(Name, FExpand(FSearch(Name, IncludeList)), 255);
         if (Name[strlen(Name) - 1] == '/') strmaxcat(Name, ArgStr[1], 255);
         F = fopen(Name, "r");
         ChkIO(10001);
         if (Len == -1) {
            Len = FileSize(F);
            ChkIO(10003);
         }
         fseek(F, SEEK_SET, Ofs);
         ChkIO(10003);
         Rest = Len;
         SaveTurnWords = TurnWords;
         TurnWords = false;
         do {
            if (Rest < MaxCodeLen) Curr = Rest;
            else Curr = MaxCodeLen;
            RLen = fread(BAsmCode, 1, Curr, F);
            ChkIO(10003);
            CodeLen = RLen;
            WriteBytes();
            Rest -= RLen;
         }
         while ((Rest != 0) && (RLen == Curr));
         if (Rest != 0) WrError(1600);
         TurnWords = SaveTurnWords;
         DontPrint = true;
         CodeLen = Len - Rest;
         fclose(F);
      }
   }
}

static void CodePUSHV(void) {
   Integer z;

   if (ArgCnt < 2) WrError(1110);
   else {
      if (!CaseSensitive) NLS_UpString(ArgStr[1]);
      for (z = 2; z <= ArgCnt; z++)
         PushSymbol(ArgStr[z], ArgStr[1]);
   }
}

static void CodePOPV(void) {
   Integer z;

   if (ArgCnt < 2) WrError(1110);
   else {
      if (!CaseSensitive) NLS_UpString(ArgStr[1]);
      for (z = 2; z <= ArgCnt; z++)
         PopSymbol(ArgStr[z], ArgStr[1]);
   }
}

static PForwardSymbol CodePPSyms_SearchSym(PForwardSymbol Root, char *Comp) {
   PForwardSymbol Lauf = Root;

   while ((Lauf != NULL) && (strcmp(Lauf->Name, Comp) != 0)) Lauf = Lauf->Next;
   return Lauf;
}

static void CodePPSyms(PForwardSymbol * Orig, PForwardSymbol * Alt1, PForwardSymbol * Alt2) {
   PForwardSymbol Lauf;
   Integer z;
   String Sym, Section;

   if (ArgCnt == 0) WrError(1110);
   else
      for (z = 1; z <= ArgCnt; z++) {
         SplitString(ArgStr[z], Sym, Section, QuotPos(ArgStr[z], ':'));
         if (!ExpandSymbol(Sym)) return;
         if (!ExpandSymbol(Section)) return;
         if (!CaseSensitive) NLS_UpString(Sym);
         Lauf = CodePPSyms_SearchSym(*Alt1, Sym);
         if (Lauf != NULL) WrXError(1489, ArgStr[z]);
         else {
            Lauf = CodePPSyms_SearchSym(*Alt2, Sym);
            if (Lauf != NULL) WrXError(1489, ArgStr[z]);
            else {
               Lauf = CodePPSyms_SearchSym(*Orig, Sym);
               if (Lauf == NULL) {
                  Lauf = (PForwardSymbol) malloc(sizeof(TForwardSymbol));
                  Lauf->Next = (*Orig);
                  *Orig = Lauf;
                  Lauf->Name = strdup(Sym);
               }
               IdentifySection(Section, &(Lauf->DestSection));
            }
         }
      }
}

#define ONOFFAllgCount 2
static ONOFFRec ONOFFAllgs[ONOFFAllgCount] = { { "MACEXP", &LstMacroEx, LstMacroExName },
{ "RELAXED", &RelaxedMode, RelaxedName }
};

typedef struct {
   char *Name;
   void (*Proc)(void);
} PseudoOrder;
static PseudoOrder Pseudos[] = { { "ALIGN", CodeALIGN },
{ "BINCLUDE", CodeBINCLUDE },
{ "CHARSET", CodeCHARSET },
{ "CPU", CodeCPU },
{ "DEPHASE", CodeDEPHASE },
{ "END", CodeEND },
{ "ENDSECTION", CodeENDSECTION },
{ "ENUM", CodeENUM },
{ "ERROR", CodeERROR },
{ "FATAL", CodeFATAL },
{ "FUNCTION", CodeFUNCTION },
{ "LABEL", CodeLABEL },
{ "LISTING", CodeLISTING },
{ "MESSAGE", CodeMESSAGE },
{ "NEWPAGE", CodeNEWPAGE },
{ "ORG", CodeORG },
{ "PAGE", CodePAGE },
{ "PHASE", CodePHASE },
{ "POPV", CodePOPV },
{ "PUSHV", CodePUSHV },
{ "READ", CodeREAD },
{ "RESTORE", CodeRESTORE },
{ "SAVE", CodeSAVE },
{ "SECTION", CodeSECTION },
{ "SEGMENT", CodeSEGMENT },
{ "SHARED", CodeSHARED },
{ "WARNING", CodeWARNING },
{ "", NULL }
};

typedef struct {
   char *Name;
   char *p;
} PseudoStrOrder;
static PseudoStrOrder PseudoStrs[] = { { "PRTINIT", PrtInitString },
{ "PRTEXIT", PrtExitString },
{ "TITLE", PrtTitleString },
{ "", NULL }
};

bool CodeGlobalPseudo(void) {
   PseudoOrder *POrder;
   PseudoStrOrder *PSOrder;

   if ((Memo("EQU"))
      || (Memo("="))
      || (Memo(":="))
      || (((!SetIsOccupied) && (Memo("SET"))))
      || (((SetIsOccupied) && (Memo("EVAL"))))) {
      CodeSETEQU();
      return true;
   }

   if (CodeONOFF(ONOFFAllgs, ONOFFAllgCount)) return true;

   POrder = Pseudos;
   while ((POrder->Proc != NULL) && (strcmp(POrder->Name, OpPart) <= 0)) {
      if (strcmp(POrder->Name, OpPart) == 0) {
         POrder->Proc();
         return true;
      }
      POrder++;
   }

   for (PSOrder = PseudoStrs; PSOrder->p != NULL; PSOrder++)
      if (Memo(PSOrder->Name)) {
         CodeString(PSOrder->p);
         return true;
      }

   if (SectionStack != NULL) {
      if (Memo("FORWARD")) {
         if (PassNo <= MaxSymPass)
            CodePPSyms(&(SectionStack->LocSyms), &(SectionStack->GlobSyms), &(SectionStack->ExportSyms));
         return true;
      }
      if (Memo("PUBLIC")) {
         CodePPSyms(&(SectionStack->GlobSyms), &(SectionStack->LocSyms), &(SectionStack->ExportSyms));
         return true;
      }
      if (Memo("GLOBAL")) {
         CodePPSyms(&(SectionStack->ExportSyms), &(SectionStack->LocSyms), &(SectionStack->GlobSyms));
         return true;
      }
   }

   return false;
}

void codeallg_init(void) {
}
