/* asmif.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Befehle zur bedingten Assemblierung                                       */
/*                                                                           */
/* Historie: 15. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "bpemu.h"
#include "chunks.h"
#include "stringutil.h"
#include "decodecmd.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"

#include "asmif.h"

PIfSave FirstIfSave;
bool IfAsm; /* false: in einer neg. IF-Sequenz-->kein Code */

static bool ActiveIF;

static LongInt GetIfVal(char *Cond) {
   bool IfOK;
   LongInt Tmp;

   FirstPassUnknown = false;
   Tmp = EvalIntExpression(Cond, Int32, &IfOK);
   if ((FirstPassUnknown) || (!IfOK)) {
      Tmp = 1;
      if (FirstPassUnknown) WrError(1820);
      else if (!IfOK) WrError(1135);
   }

   return Tmp;
}

static void AddBoolFlag(bool Flag) {
   strmaxcpy(ListLine, Flag ? "=>TRUE" : "=>FALSE", 255);
}

static void PushIF(LongInt IfExpr) {
   PIfSave NewSave;

   NewSave = (PIfSave) malloc(sizeof(TIfSave));
   NewSave->NestLevel = SaveIFs() + 1;
   NewSave->Next = FirstIfSave;
   NewSave->SaveIfAsm = IfAsm;
   NewSave->State = IfState_IFIF;
   NewSave->CaseFound = (IfExpr != 0);
   FirstIfSave = NewSave;
   IfAsm = (IfAsm && (IfExpr != 0));
}

static void CodeIF(void) {
   LongInt IfExpr;

   ActiveIF = IfAsm;

   if (!IfAsm) IfExpr = 1;
   else if (ArgCnt != 1) {
      WrError(1110);
      IfExpr = 1;
   } else IfExpr = GetIfVal(ArgStr[1]);
   if (IfAsm) AddBoolFlag(IfExpr != 0);
   PushIF(IfExpr);
}

static void CodeIFDEF(void) {
   LongInt IfExpr;
   bool Defined;

   ActiveIF = IfAsm;

   if (!IfAsm) IfExpr = 1;
   else if (ArgCnt != 1) {
      WrError(1110);
      IfExpr = 1;
   } else {
      Defined = IsSymbolDefined(ArgStr[1]);
      if (IfAsm)
         strmaxcpy(ListLine, (Defined) ? "=>DEFINED" : "=>UNDEFINED", 255);
      if (Memo("IFDEF")) IfExpr = (Defined) ? 1 : 0;
      else IfExpr = (Defined) ? 0 : 1;
   }
   PushIF(IfExpr);
}

static void CodeIFUSED(void) {
   LongInt IfExpr;
   bool Used;

   ActiveIF = IfAsm;

   if (!IfAsm) IfExpr = 1;
   else if (ArgCnt != 1) {
      WrError(1110);
      IfExpr = 1;
   } else {
      Used = IsSymbolUsed(ArgStr[1]);
      if (IfAsm)
         strmaxcpy(ListLine, (Used) ? "=>USED" : "=>UNUSED", 255);
      if (Memo("IFUSED")) IfExpr = (Used) ? 1 : 0;
      else IfExpr = (Used) ? 0 : 1;
   }
   PushIF(IfExpr);
}

void CodeIFEXIST(void) {
   LongInt IfExpr;
   bool Found;
   String NPath;

   ActiveIF = IfAsm;

   if (!IfAsm) IfExpr = 1;
   else if (ArgCnt != 1) {
      WrError(1110);
      IfExpr = 1;
   } else {
      strmaxcpy(ArgPart, ArgStr[1], 255);
      if (*ArgPart == '"') strmove(ArgPart, 1);
      if (ArgPart[strlen(ArgPart) - 1] == '"') ArgPart[strlen(ArgPart) - 1] = '\0';
      AddSuffix(ArgPart, IncSuffix);
      strmaxcpy(NPath, IncludeList, 255);
      strmaxprep(NPath, ".:", 255);
      Found = (*(FSearch(ArgPart, NPath)) != '\0');
      if (IfAsm)
         strmaxcpy(ListLine, (Found) ? "=>FOUND" : "=>NOT FOUND", 255);
      if (Memo("IFEXIST")) IfExpr = (Found) ? 1 : 0;
      else IfExpr = (Found) ? 0 : 1;
   }
   PushIF(IfExpr);
}

static void CodeIFB(void) {
   bool Blank = true;
   LongInt IfExpr;
   Integer z;

   ActiveIF = IfAsm;

   if (!IfAsm) IfExpr = 1;
   else {
         for (z = 1; z <= ArgCnt; z++) if (strlen(ArgStr[z++]) > 0) Blank = false;
      if (IfAsm)
         strmaxcpy(ListLine, (Blank) ? "=>BLANK" : "=>NOT BLANK", 255);
      if (Memo("IFB")) IfExpr = (Blank) ? 1 : 0;
      else IfExpr = (Blank) ? 0 : 1;
   }
   PushIF(IfExpr);
}

static void CodeELSEIF(void) {
   LongInt IfExpr;

   if (FirstIfSave == NULL) WrError(1840);
   else if (ArgCnt == 0) {
      if (FirstIfSave->State != IfState_IFIF) WrError(1480);
      else if (FirstIfSave->SaveIfAsm) AddBoolFlag(IfAsm = (!FirstIfSave->CaseFound));
      FirstIfSave->State = IfState_IFELSE;
   } else if (ArgCnt == 1) {
      if (FirstIfSave->State != IfState_IFIF) WrError(1480);
      else {
         if (!FirstIfSave->SaveIfAsm) IfExpr = 1;
         else if (FirstIfSave->CaseFound) IfExpr = 0;
         else IfExpr = GetIfVal(ArgStr[1]);
         IfAsm = ((FirstIfSave->SaveIfAsm) && (IfExpr != 0) && (!FirstIfSave->CaseFound));
         if (FirstIfSave->SaveIfAsm) AddBoolFlag(IfExpr != 0);
         if (IfExpr != 0) FirstIfSave->CaseFound = true;
      }
   } else WrError(1110);

   ActiveIF = (FirstIfSave == NULL) || (FirstIfSave->SaveIfAsm);
}

static void CodeENDIF(void) {
   PIfSave NewSave;

   if (ArgCnt != 0) WrError(1110);
   if (FirstIfSave == NULL) WrError(1840);
   else {
      if ((FirstIfSave->State != IfState_IFIF) && (FirstIfSave->State != IfState_IFELSE)) WrError(1480);
      else {
         IfAsm = FirstIfSave->SaveIfAsm;
         NewSave = FirstIfSave;
         FirstIfSave = NewSave->Next;
         free(NewSave);
      }
   }

   ActiveIF = IfAsm;
}

static void EvalIfExpression(char *Cond, TempResult * erg) {
   FirstPassUnknown = false;
   EvalExpression(Cond, erg);
   if ((erg->Typ == TempNone) || (FirstPassUnknown)) {
      erg->Typ = TempInt;
      erg->Contents.Int = 1;
      if (FirstPassUnknown) WrError(1820);
   }
}

static void CodeSWITCH(void) {
   PIfSave NewSave;

   ActiveIF = IfAsm;

   NewSave = (PIfSave) malloc(sizeof(TIfSave));
   NewSave->NestLevel = SaveIFs() + 1;
   NewSave->Next = FirstIfSave;
   NewSave->SaveIfAsm = IfAsm;
   NewSave->CaseFound = false;
   NewSave->State = IfState_CASESWITCH;
   if (ArgCnt != 1) {
      NewSave->SaveExpr.Typ = TempInt;
      NewSave->SaveExpr.Contents.Int = 1;
      if (IfAsm) WrError(1110);
   } else {
      EvalIfExpression(ArgStr[1], &(NewSave->SaveExpr));
      SetListLineVal(&(NewSave->SaveExpr));
   }
   FirstIfSave = NewSave;
}

static void CodeCASE(void) {
   bool eq;
   Integer z;
   TempResult t;

   if (FirstIfSave == NULL) WrError(1840);
   else if (ArgCnt == 0) WrError(1110);
   else {
      if ((FirstIfSave->State != IfState_CASESWITCH) && (FirstIfSave->State != IfState_CASECASE)) WrError(1480);
      else {
         if (!FirstIfSave->SaveIfAsm) eq = true;
         else if (FirstIfSave->CaseFound) eq = false;
         else {
            eq = false;
            z = 1;
            do {
               EvalIfExpression(ArgStr[z], &t);
               eq = (FirstIfSave->SaveExpr.Typ == t.Typ);
               if (eq)
                  switch (t.Typ) {
                     case TempInt:
                        eq = (t.Contents.Int == FirstIfSave->SaveExpr.Contents.Int);
                        break;
                     case TempFloat:
                        eq = (t.Contents.Float == FirstIfSave->SaveExpr.Contents.Float);
                        break;
                     case TempString:
                        eq = (strcmp(t.Contents.Ascii, FirstIfSave->SaveExpr.Contents.Ascii) == 0);
                        break;
                     case TempNone:
                        eq = false;
                        break;
                  }
               z++;
            }
            while ((!eq) && (z <= ArgCnt));
         };
         IfAsm = ((FirstIfSave->SaveIfAsm) && (eq) && (!FirstIfSave->CaseFound));
         if (FirstIfSave->SaveIfAsm) AddBoolFlag(eq && (!FirstIfSave->CaseFound));
         if (eq) FirstIfSave->CaseFound = true;
         FirstIfSave->State = IfState_CASECASE;
      }
   }

   ActiveIF = (FirstIfSave == NULL) || (FirstIfSave->SaveIfAsm);
}

static void CodeELSECASE(void) {
   if (ArgCnt != 0) WrError(1110);
   else {
      if ((FirstIfSave->State != IfState_CASESWITCH) && (FirstIfSave->State != IfState_CASECASE)) WrError(1480);
      else IfAsm = (FirstIfSave->SaveIfAsm && (!FirstIfSave->CaseFound));
      if (FirstIfSave->SaveIfAsm) AddBoolFlag(!FirstIfSave->CaseFound);
      FirstIfSave->CaseFound = true;
      FirstIfSave->State = IfState_CASEELSE;
   }

   ActiveIF = (FirstIfSave == NULL) || (FirstIfSave->SaveIfAsm);
}

static void CodeENDCASE(void) {
   PIfSave NewSave;

   if (ArgCnt != 0) WrError(1110);
   if (FirstIfSave == NULL) WrError(1840);
   else {
      if ((FirstIfSave->State != IfState_CASESWITCH)
         && (FirstIfSave->State != IfState_CASECASE)
         && (FirstIfSave->State != IfState_CASEELSE)) WrError(1480);
      else {
         IfAsm = FirstIfSave->SaveIfAsm;
         if (!FirstIfSave->CaseFound) WrError(100);
         NewSave = FirstIfSave;
         FirstIfSave = NewSave->Next;
         free(NewSave);
      }
   }

   ActiveIF = IfAsm;
}

bool CodeIFs(void) {
   bool Result = true;

   ActiveIF = false;

   if (Memo("IF")) CodeIF();
   else if ((Memo("IFDEF")) || (Memo("IFNDEF"))) CodeIFDEF();
   else if ((Memo("IFUSED")) || (Memo("IFNUSED"))) CodeIFUSED();
   else if ((Memo("IFEXIST")) || (Memo("IFNEXIST"))) CodeIFEXIST();
   else if ((Memo("IFB")) || (Memo("IFNB"))) CodeIFB();
   else if (Memo("ELSEIF")) CodeELSEIF();
   else if (Memo("ENDIF")) CodeENDIF();
   else if (Memo("SWITCH")) CodeSWITCH();
   else if (Memo("CASE")) CodeCASE();
   else if (Memo("ELSECASE")) CodeELSECASE();
   else if (Memo("ENDCASE")) CodeENDCASE();
   else Result = false;

   return Result;
}

Integer SaveIFs(void) {
   return (FirstIfSave == NULL) ? 0 : FirstIfSave->NestLevel;
}

void RestoreIFs(Integer Level) {
   PIfSave OldSave;

   while ((FirstIfSave != NULL) && (FirstIfSave->NestLevel != Level)) {
      OldSave = FirstIfSave;
      FirstIfSave = OldSave->Next;
      IfAsm = OldSave->SaveIfAsm;
      free(OldSave);
   }
}

bool IFListMask(void) {
   switch (ListOn) {
      case 0:
         return true;
      case 1:
         return false;
      case 2:
         return ((!ActiveIF) && (!IfAsm));
      case 3:
         return (ActiveIF || (!IfAsm));
   }
   return true;
}

void AsmIFInit(void) {
   IfAsm = true;
}

void asmif_init(void) {
}
