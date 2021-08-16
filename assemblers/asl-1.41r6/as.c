/* as.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Hauptmodul                                                                */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#include "endian.h"
#include "bpemu.h"

#include "stdhandl.h"
#include "decodecmd.h"
#include "nls.h"
#include "stringutil.h"
#include "stringlists.h"
#include "asmitree.h"
#include "chunks.h"
#include "asminclist.h"
#include "asmfnums.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmmac.h"
#include "asmif.h"
#include "asmcode.h"
#include "asmdebug.h"
#include "codeallg.h"
#include "codepseudo.h"

#include "code68k.h"
#include "code56k.h"
#include "code601.h"
#include "code68.h"
#include "code6805.h"
#include "code6809.h"
#include "code6812.h"
#include "code6816.h"
#include "codeh8_3.h"
#include "codeh8_5.h"
#include "code7000.h"
#include "code65.h"
#include "code7700.h"
#include "code4500.h"
#include "codem16.h"
#include "codem16c.h"
#include "code48.h"
#include "code51.h"
#include "code96.h"
#include "code85.h"
#include "code86.h"
#include "code8x30x.h"
#include "codexa.h"
#include "codeavr.h"
#include "code29k.h"
#include "code166.h"
#include "codez80.h"
#include "codez8.h"
#include "code96c141.h"
#include "code90c141.h"
#include "code87c800.h"
#include "code47c00.h"
#include "code97c241.h"
#include "code16c5x.h"
#include "code16c8x.h"
#include "code17c4x.h"
#include "codest6.h"
#include "codest7.h"
#include "codest9.h"
#include "code6804.h"
#include "code3201x.h"
#include "code3202x.h"
#include "code3203x.h"
#include "code3205x.h"
#include "code9900.h"
#include "codetms7.h"
#include "code370.h"
#include "codemsp.h"
#include "code78c10.h"
#include "code75k0.h"
#include "code78k0.h"
#include "codescmp.h"
#include "codecop8.h"
#include "as1750.h"
/**          Code21xx};**/

/**
VAR
   ParCnt,k:Integer;
   CPU:CPUVar;**/
static String FileMask;
static long StartTime, StopTime;
static bool ErrFlag;
static bool MasterFile;

#define MAIN
#include "as.rsc"

/*=== Zeilen einlesen ======================================================*/

static void NULL_Restorer(PInputTag PInp) {
   if (PInp == NULL); /* satisfy some compilers */
}

static void GenerateProcessor(PInputTag * PInp) {
   *PInp = (PInputTag) malloc(sizeof(TInputTag));
   (*PInp)->IsMacro = false;
   (*PInp)->Next = NULL;
   (*PInp)->First = true;
   strmaxcpy((*PInp)->OrigPos, ErrorPos, 255);
   (*PInp)->OrigDoLst = DoLst;
   (*PInp)->StartLine = CurrLine;
   (*PInp)->ParCnt = 0;
   (*PInp)->ParZ = 0;
   InitStringList(&((*PInp)->Params));
   (*PInp)->LineCnt = 0;
   (*PInp)->LineZ = 1;
   (*PInp)->Lines = NULL;
   (*PInp)->SpecName[0] = '\0';
   (*PInp)->IsEmpty = false;
   (*PInp)->Buffer = NULL;
   (*PInp)->Datei = NULL;
   (*PInp)->IfLevel = SaveIFs();
   (*PInp)->Restorer = NULL_Restorer;
}

/*=========================================================================*/
/* Listing erzeugen */

static void MakeList_Gen2Line(char *h, Word EffLen, Word * n) {
   Integer z, Rest;

   Rest = EffLen - (*n);
   if (Rest > 8) Rest = 8;
   if (DontPrint) Rest = 0;
   for (z = 0; z < (Rest >> 1); z++) {
      strmaxcat(h, HexString(WAsmCode[(*n) >> 1], 4), 255);
      strmaxcat(h, " ", 255);
      (*n) += 2;
   }
   if ((Rest & 1) != 0) {
      strmaxcat(h, HexString(BAsmCode[*n], 2), 255);
      strmaxcat(h, "   ", 255);
      (*n)++;
   }
   for (z = 1; z <= (8 - Rest) >> 1; z++)
      strmaxcat(h, "     ", 255);
}

static void MakeList(void) {
   String h, h2;
   Byte i, k;
   Word n;
   Word EffLen;

   EffLen = CodeLen * Granularity();

   if ((strcmp(LstName, NULLDEV) != 0) && (DoLst) && ((ListMask & 1) != 0) && (!IFListMask())) {
   /* Zeilennummer / Programmzaehleradresse: */

      if (IncDepth == 0) strmaxcpy(h2, "   ", 255);
      else sprintf(h2, "(%d)", IncDepth);
      if ((ListMask & 0x10) != 0) {
         sprintf(h, "%5d/", CurrLine);
         strmaxcat(h2, h, 255);
      }
      strmaxcpy(h, h2, 255);
      strmaxcat(h, HexBlankString(EProgCounter() - CodeLen, 8), 255);
      strmaxcat(h, Retracted ? "  R " : " : ", 255);

   /* Extrawurst in Listing ? */

      if (*ListLine != '\0') {
         strmaxcat(h, ListLine, 255);
         strmaxcat(h, Blanks(20 - strlen(ListLine)), 255);
         strmaxcat(h, OneLine, 255);
         WrLstLine(h);
         *ListLine = '\0';
      }

   /* Code ausgeben */

      else
         switch (ListGran()) {
            case 4:
               for (i = 0; i < 2; i++)
                  if ((!DontPrint) && ((EffLen >> 2) > i)) {
                     strmaxcat(h, HexString(DAsmCode[i], 8), 255);
                     strmaxcat(h, " ", 255);
                  } else strmaxcat(h, "         ", 255);
               strmaxcat(h, "  ", 255);
               strmaxcat(h, OneLine, 255);
               WrLstLine(h);
               if ((EffLen > 8) && (!DontPrint)) {
                  EffLen -= 8;
                  n = EffLen >> 3;
                  if ((EffLen & 7) == 0) n--;
                  for (i = 0; i <= n; i++) {
                     strmaxcpy(h, "                    ", 255);
                     for (k = 0; k < 2; k++)
                        if ((EffLen >> 2) > i * 2 + k) {
                           strmaxcat(h, HexString(DAsmCode[i * 2 + k + 2], 8), 255);
                           strmaxcat(h, " ", 255);
                        }
                     WrLstLine(h);
                  }
               }
               break;
            case 2:
               n = 0;
               MakeList_Gen2Line(h, EffLen, &n);
               strmaxcat(h, OneLine, 255);
               WrLstLine(h);
               if (!DontPrint)
                  while (n < EffLen) {
                     strmaxcpy(h, "                    ", 255);
                     MakeList_Gen2Line(h, EffLen, &n);
                     WrLstLine(h);
                  }
               break;
            default:
               if ((TurnWords) && (Granularity != ListGran)) DreheCodes();
               for (i = 0; i < 6; i++)
                  if ((!DontPrint) && (EffLen > i)) {
                     strmaxcat(h, HexString(BAsmCode[i], 2), 255);
                     strmaxcat(h, " ", 255);
                  } else strmaxcat(h, "   ", 255);
               strmaxcat(h, "  ", 255);
               strmaxcat(h, OneLine, 255);
               WrLstLine(h);
               if ((EffLen > 6) && (!DontPrint)) {
                  EffLen -= 6;
                  n = EffLen / 6;
                  if ((EffLen % 6) == 0) n--;
                  for (i = 0; i <= n; i++) {
                     strmaxcpy(h, "                    ", 255);
                     for (k = 0; k < 6; k++)
                        if (EffLen > i * 6 + k) {
                           strmaxcat(h, HexString(BAsmCode[i * 6 + k + 6], 2), 255);
                           strmaxcat(h, " ", 255);
                        }
                     WrLstLine(h);
                  }
               }
               if ((TurnWords) && (Granularity != ListGran)) DreheCodes();
         }

   }
}

/*=========================================================================*/
/* Makroprozessor */

/*-------------------------------------------------------------------------*/
/* allgemein gebrauchte Subfunktionen */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* werden gebraucht, um festzustellen, ob innerhalb eines Makrorumpfes weitere
   Makroschachtelungen auftreten */

bool MacroStart(void) {
   return ((Memo("MACRO")) || (Memo("IRP")) || (Memo("REPT")) || (Memo("WHILE")));
}

bool MacroEnd(void) {
   return (Memo("ENDM"));
}

/*-------------------------------------------------------------------------*/
/* Dieser Einleseprozessor dient nur dazu, eine fehlerhafte Makrodefinition
  bis zum Ende zu ueberlesen */

static void WaitENDM_Processor(void) {
   POutputTag Tmp;

   if (MacroStart()) FirstOutputTag->NestLevel++;
   else if (MacroEnd()) FirstOutputTag->NestLevel--;
   if (FirstOutputTag->NestLevel <= -1) {
      Tmp = FirstOutputTag;
      FirstOutputTag = Tmp->Next;
      free(Tmp);
   }
}

static void AddWaitENDM_Processor(void) {
   POutputTag Neu;

   Neu = (POutputTag) malloc(sizeof(TOutputTag));
   Neu->Processor = WaitENDM_Processor;
   Neu->NestLevel = 0;
   Neu->Next = FirstOutputTag;
   FirstOutputTag = Neu;
}

/*-------------------------------------------------------------------------*/
/* normale Makros */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Diese Routine leitet die Quellcodezeilen bei der Makrodefinition in den
   Makro-Record um */

static void MACRO_OutProcessor(void) {
   POutputTag Tmp;
   Integer z;
   StringRecPtr l;
   PMacroRec GMacro;
   String s;

   if ((MacroOutput) && (FirstOutputTag->DoExport)) {
      errno = 0;
      fprintf(MacroFile, "%s\n", OneLine);
      ChkIO(10004);
   }
   if (MacroStart()) FirstOutputTag->NestLevel++;
   else if (MacroEnd()) FirstOutputTag->NestLevel--;
   if (FirstOutputTag->NestLevel != -1) {
      strmaxcpy(s, OneLine, 255);
      KillCtrl(s);
      l = FirstOutputTag->Params;
      for (z = 1; z <= FirstOutputTag->Mac->ParamCount; z++)
         CompressLine(GetStringListNext(&l), z, s);
      if (HasAttrs) CompressLine(AttrName, ParMax + 1, s);
      AddStringListLast(&(FirstOutputTag->Mac->FirstLine), s);
   }

   if (FirstOutputTag->NestLevel == -1) {
      if (IfAsm) {
         AddMacro(FirstOutputTag->Mac, FirstOutputTag->PubSect, true);
         if ((FirstOutputTag->DoGlobCopy) && (SectionStack != NULL)) {
            GMacro = (PMacroRec) malloc(sizeof(MacroRec));
            GMacro->Name = strdup(FirstOutputTag->GName);
            GMacro->ParamCount = FirstOutputTag->Mac->ParamCount;
            GMacro->FirstLine = DuplicateStringList(FirstOutputTag->Mac->FirstLine);
            AddMacro(GMacro, FirstOutputTag->GlobSect, false);
         }
      } else ClearMacroRec(&(FirstOutputTag->Mac));

      Tmp = FirstOutputTag;
      FirstOutputTag = Tmp->Next;
      ClearStringList(&(Tmp->Params));
      free(Tmp);
   }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Hierher kommen bei einem Makroaufruf die expandierten Zeilen */

bool MACRO_Processor(PInputTag PInp, char *erg) {
   StringRecPtr Lauf;
   Integer z;
   bool Result;

   Result = true;

   Lauf = PInp->Lines;
   for (z = 1; z <= PInp->LineZ - 1; z++) Lauf = Lauf->Next;
   strcpy(erg, Lauf->Content);
   Lauf = PInp->Params;
   for (z = 1; z <= PInp->ParCnt; z++) {
      ExpandLine(Lauf->Content, z, erg);
      Lauf = Lauf->Next;
   }
   if (HasAttrs) ExpandLine(PInp->SaveAttr, ParMax + 1, erg);

   CurrLine = PInp->StartLine;
   sprintf(ErrorPos, "%s %s(%d)", PInp->OrigPos, PInp->SpecName, PInp->LineZ);

   if (PInp->LineZ == 1) PushLocHandle(GetLocHandle());

   if (++(PInp->LineZ) > PInp->LineCnt) Result = false;

   return Result;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Initialisierung des Makro-Einleseprozesses */

static bool ReadMacro_SearchArg(char *Test, char *Comp, bool *Erg) {
   if (strcasecmp(Test, Comp) == 0) {
      *Erg = true;
      return true;
   } else if ((strlen(Test) > 2) && (strncasecmp(Test, "NO", 2) == 0) && (strcasecmp(Test + 2, Comp) == 0)) {
      *Erg = false;
      return true;
   } else return false;
}

static bool ReadMacro_SearchSect(char *Test_O, char *Comp, bool *Erg, LongInt * Section) {
   char *p;
   String Test, Sect;

   strmaxcpy(Test, Test_O, 255);
   KillBlanks(Test);
   p = strchr(Test, ':');
   if (p == NULL) *Sect = '\0';
   else {
      strmaxcpy(Sect, p + 1, 255);
      *p = '\0';
   }
   if ((strlen(Test) > 2) && (strncasecmp(Test, "NO", 2) == 0) && (strcasecmp(Test + 2, Comp) == 0)) {
      *Erg = false;
      return true;
   } else if (strcasecmp(Test, Comp) == 0) {
      *Erg = false;
      return (IdentifySection(Sect, Section));
   } else return false;
}

static void ReadMacro(void) {
   String PList;
   PSaveSection RunSection;
   PMacroRec OneMacro;
   Integer z1, z2;
   POutputTag Neu;

   bool DoMacExp, DoPublic;
   LongInt HSect;
   bool ErrFlag;

   CodeLen = 0;
   ErrFlag = false;

/* Makronamen pruefen */
/* Definition nur im ersten Pass */

   if (PassNo != 1) ErrFlag = true;
   else if (!ExpandSymbol(LabPart)) ErrFlag = true;
   else if (!ChkSymbName(LabPart)) {
      WrXError(1020, LabPart);
      ErrFlag = true;
   }

   Neu = (POutputTag) malloc(sizeof(TOutputTag));
   Neu->Processor = MACRO_OutProcessor;
   Neu->NestLevel = 0;
   Neu->Params = NULL;
   Neu->DoExport = false;
   Neu->DoGlobCopy = false;
   Neu->Next = FirstOutputTag;

/* Argumente ueberpruefen */

   DoMacExp = LstMacroEx;
   DoPublic = false;
   *PList = '\0';
   z2 = 0;
   for (z1 = 1; z1 <= ArgCnt; z1++)
      if ((ArgStr[z1][0] == '{') && (ArgStr[z1][strlen(ArgStr[z1]) - 1] == '}')) {
         strcpy(ArgStr[z1], ArgStr[z1] + 1);
         ArgStr[z1][strlen(ArgStr[z1]) - 1] = '\0';
         if (ReadMacro_SearchArg(ArgStr[z1], "EXPORT", &(Neu->DoExport)));
         else if (ReadMacro_SearchArg(ArgStr[z1], "EXPAND", &DoMacExp)) {
            strmaxcat(PList, ",", 255);
            strmaxcat(PList, ArgStr[z1], 255);
         } else if (ReadMacro_SearchSect(ArgStr[z1], "GLOBAL", &(Neu->DoGlobCopy), &(Neu->GlobSect)));
         else if (ReadMacro_SearchSect(ArgStr[z1], "PUBLIC", &DoPublic, &(Neu->PubSect)));
         else {
            WrXError(1465, ArgStr[z1]);
            ErrFlag = true;
         }
      } else {
         strmaxcat(PList, ",", 255);
         strmaxcat(PList, ArgStr[z1], 255);
         z2++;
         if (!ChkMacSymbName(ArgStr[z1])) {
            WrXError(1020, ArgStr[z1]);
            ErrFlag = true;
         }
         AddStringListLast(&(Neu->Params), ArgStr[z1]);
      }

/* Abbruch bei Fehler */

   if (ErrFlag) {
      ClearStringList(&(Neu->Params));
      free(Neu);
      AddWaitENDM_Processor();
      return;
   }

/* Bei Globalisierung Namen des Extramakros ermitteln */

   if (Neu->DoGlobCopy) {
      strmaxcpy(Neu->GName, LabPart, 255);
      RunSection = SectionStack;
      HSect = MomSectionHandle;
      while ((HSect != Neu->GlobSect) && (RunSection != NULL)) {
         strmaxprep(Neu->GName, "_", 255);
         strmaxprep(Neu->GName, GetSectionName(HSect), 255);
         HSect = RunSection->Handle;
         RunSection = RunSection->Next;
      }
   }
   if (!DoPublic) Neu->PubSect = MomSectionHandle;

   OneMacro = (PMacroRec) malloc(sizeof(MacroRec));
   Neu->Mac = OneMacro;
   if ((MacroOutput) && (Neu->DoExport)) {
      if (strlen(PList) != 0) strcpy(PList, PList + 1);
      errno = 0;
      if (Neu->DoGlobCopy) fprintf(MacroFile, "%s MACRO %s\n", Neu->GName, PList);
      else fprintf(MacroFile, "%s MACRO %s\n", LabPart, PList);
      ChkIO(10004);
   }
   OneMacro->Used = false;
   OneMacro->Name = strdup(LabPart);
   OneMacro->ParamCount = z2;
   OneMacro->FirstLine = NULL;
   OneMacro->LocMacExp = DoMacExp;

   FirstOutputTag = Neu;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Beendigung der Expansion eines Makros */

static void MACRO_Cleanup(PInputTag PInp) {
   ClearStringList(&(PInp->Params));
}

static void MACRO_Restorer(PInputTag PInp) {
   PopLocHandle();
   strmaxcpy(ErrorPos, PInp->OrigPos, 255);
   DoLst = PInp->OrigDoLst;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Dies initialisiert eine Makroexpansion */

static void ExpandMacro(PMacroRec OneMacro) {
   Integer z1;
   StringRecPtr Lauf;
   PInputTag Tag;

   CodeLen = 0;

/* if (OneMacro->Used) WrError(1850);
   else */

   {
      OneMacro->Used = true;

   /* 1. Tag erzeugen */

      GenerateProcessor(&Tag);
      Tag->Processor = MACRO_Processor;
      Tag->Restorer = MACRO_Restorer;
      Tag->Cleanup = MACRO_Cleanup;
      strmaxcpy(Tag->SpecName, OneMacro->Name, 255);
      strmaxcpy(Tag->SaveAttr, AttrPart, 255);
      Tag->IsMacro = true;

   /* 2. Parameterzahl anpassen */

      if (ArgCnt < OneMacro->ParamCount)
         for (z1 = ArgCnt + 1; z1 <= OneMacro->ParamCount; z1++) *(ArgStr[z1]) = '\0';
      ArgCnt = OneMacro->ParamCount;

   /* 3. Parameterliste aufbauen - umgekehrt einfacher */

      for (z1 = ArgCnt; z1 >= 1; z1--) {
         if (!CaseSensitive) UpString(ArgStr[z1]);
         AddStringListFirst(&(Tag->Params), ArgStr[z1]);
      }
      Tag->ParCnt = ArgCnt;

   /* 4. Zeilenliste anhaengen */

      Tag->Lines = OneMacro->FirstLine;
      Tag->IsEmpty = (OneMacro->FirstLine == NULL);
      Lauf = OneMacro->FirstLine;
      while (Lauf != NULL) {
         Tag->LineCnt++;
         Lauf = Lauf->Next;
      }
   }

/* 5. anhaengen */

   if (IfAsm) {
      NextDoLst = (DoLst && OneMacro->LocMacExp);
      Tag->Next = FirstInputTag;
      FirstInputTag = Tag;
   } else {
      ClearStringList(&(Tag->Params));
      free(Tag);
   }
}

/*-------------------------------------------------------------------------*/
/* vorzeitiger Abbruch eines Makros */

static void ExpandEXITM(void) {
   if (ArgCnt != 0) WrError(1110);
   else if (FirstInputTag == NULL) WrError(1805);
   else if (!FirstInputTag->IsMacro) WrError(1805);
   else if (IfAsm) {
      FirstInputTag->Cleanup(FirstInputTag);
      RestoreIFs(FirstInputTag->IfLevel);
      FirstInputTag->IsEmpty = true;
   }
}

/*-------------------------------------------------------------------------*/
/*--- IRP (was das bei MASM auch immer heissen mag...)
      Ach ja: Individual Repeat! Danke Bernhard, jetzt hab'
      ich's gerafft! -----------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Aufraeumroutine */

static void IRP_Cleanup(PInputTag PInp) {
   ClearStringList(&(PInp->Lines));
   ClearStringList(&(PInp->Params));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Diese Routine liefert bei der Expansion eines IRP-Statements die expan-
  dierten Zeilen */

bool IRP_Processor(PInputTag PInp, char *erg) {
   StringRecPtr Lauf;
   Integer z;
   bool Result;

   Result = true;

   Lauf = PInp->Lines;
   for (z = 1; z <= PInp->LineZ - 1; z++) Lauf = Lauf->Next;
   strcpy(erg, Lauf->Content);
   Lauf = PInp->Params;
   for (z = 1; z <= PInp->ParZ - 1; z++) Lauf = Lauf->Next;
   ExpandLine(Lauf->Content, 1, erg);
   CurrLine = PInp->StartLine + PInp->LineZ;

   sprintf(ErrorPos, "%s IRP:%s/%d", PInp->OrigPos, Lauf->Content, PInp->LineZ);

   if (PInp->LineZ == 1) {
      if (!PInp->First) PopLocHandle();
      PInp->First = false;
      PushLocHandle(GetLocHandle());
   }

   if (++(PInp->LineZ) > PInp->LineCnt) {
      PInp->LineZ = 1;
      if (++(PInp->ParZ) > PInp->ParCnt) Result = false;
   }

   return Result;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Diese Routine sammelt waehrend der Definition eines IRP-Statements die
  Quellzeilen ein */

static void IRP_OutProcessor(void) {
   POutputTag Tmp;
   StringRecPtr Dummy;
   String s;

/* Schachtelungen mitzaehlen */

   if (MacroStart()) FirstOutputTag->NestLevel++;
   else if (MacroEnd()) FirstOutputTag->NestLevel--;

/* falls noch nicht zuende, weiterzaehlen */

   if (FirstOutputTag->NestLevel > -1) {
      strmaxcpy(s, OneLine, 255);
      KillCtrl(s);
      CompressLine(GetStringListFirst(FirstOutputTag->Params, &Dummy), 1, s);
      AddStringListLast(&(FirstOutputTag->Tag->Lines), s);
      FirstOutputTag->Tag->LineCnt++;
   }

/* alles zusammen? Dann umhaengen */

   if (FirstOutputTag->NestLevel == -1) {
      Tmp = FirstOutputTag;
      FirstOutputTag = FirstOutputTag->Next;
      Tmp->Tag->IsEmpty = (Tmp->Tag->Lines == NULL);
      if (IfAsm) {
         NextDoLst = DoLst && LstMacroEx;
         Tmp->Tag->Next = FirstInputTag;
         FirstInputTag = Tmp->Tag;
      } else {
         ClearStringList(&(Tmp->Tag->Lines));
         ClearStringList(&(Tmp->Tag->Params));
         free(Tmp->Tag);
      }
      ClearStringList(&(Tmp->Params));
      free(Tmp);
   }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Initialisierung der IRP-Bearbeitung */

static void ExpandIRP(void) {
   String Parameter;
   Integer z1;
   PInputTag Tag;
   POutputTag Neu;
   bool ErrFlag;

/* 1.Parameter pruefen */

   if (ArgCnt < 2) {
      WrError(1110);
      ErrFlag = true;
   } else {
      strmaxcpy(Parameter, ArgStr[1], 255);
      if (!ChkMacSymbName(ArgStr[1])) {
         WrXError(1020, Parameter);
         ErrFlag = true;
      } else ErrFlag = false;
   }
   if (ErrFlag) {
      AddWaitENDM_Processor();
      return;
   }

/* 2. Tag erzeugen */

   GenerateProcessor(&Tag);
   Tag->ParCnt = ArgCnt - 1;
   Tag->Processor = IRP_Processor;
   Tag->Restorer = MACRO_Restorer;
   Tag->Cleanup = IRP_Cleanup;
   Tag->ParZ = 1;
   Tag->IsMacro = true;

/* 3. Parameterliste aufbauen; rueckwaerts einen Tucken schneller */

   for (z1 = ArgCnt; z1 >= 2; z1--) {
      UpString(ArgStr[z1]);
      AddStringListFirst(&(Tag->Params), ArgStr[z1]);
   }

/* 4. einbetten */

   Neu = (POutputTag) malloc(sizeof(TOutputTag));
   Neu->Next = FirstOutputTag;
   Neu->Processor = IRP_OutProcessor;
   Neu->NestLevel = 0;
   Neu->Tag = Tag;
   Neu->Params = NULL;
   AddStringListFirst(&(Neu->Params), ArgStr[1]);
   FirstOutputTag = Neu;
}

/*--- Repetition -----------------------------------------------------------*/

static void REPT_Cleanup(PInputTag PInp) {
   ClearStringList(&(PInp->Lines));
}

bool REPT_Processor(PInputTag PInp, char *erg) {
   StringRecPtr Lauf;
   Integer z;
   bool Result;

   Result = true;

   Lauf = PInp->Lines;
   for (z = 1; z <= PInp->LineZ - 1; z++) Lauf = Lauf->Next;
   strcpy(erg, Lauf->Content);
   CurrLine = PInp->StartLine + PInp->LineZ;

   sprintf(ErrorPos, "%s REPT %d/%d", PInp->OrigPos, PInp->ParZ, PInp->LineZ);

   if (PInp->LineZ == 1) {
      if (!PInp->First) PopLocHandle();
      PInp->First = false;
      PushLocHandle(GetLocHandle());
   }

   if ((++PInp->LineZ) > PInp->LineCnt) {
      PInp->LineZ = 1;
      if ((++PInp->ParZ) > PInp->ParCnt) Result = false;
   }

   return Result;
}

static void REPT_OutProcessor(void) {
   POutputTag Tmp;

/* Schachtelungen mitzaehlen */

   if (MacroStart()) FirstOutputTag->NestLevel++;
   else if (MacroEnd()) FirstOutputTag->NestLevel--;

/* falls noch nicht zuende, weiterzaehlen */

   if (FirstOutputTag->NestLevel > -1) {
      AddStringListLast(&(FirstOutputTag->Tag->Lines), OneLine);
      FirstOutputTag->Tag->LineCnt++;
   }

/* alles zusammen? Dann umhaengen */

   if (FirstOutputTag->NestLevel == -1) {
      Tmp = FirstOutputTag;
      FirstOutputTag = FirstOutputTag->Next;
      Tmp->Tag->IsEmpty = (Tmp->Tag->Lines == NULL);
      if ((IfAsm) && (Tmp->Tag->ParCnt > 0)) {
         NextDoLst = (DoLst && LstMacroEx);
         Tmp->Tag->Next = FirstInputTag;
         FirstInputTag = Tmp->Tag;
      } else {
         ClearStringList(&(Tmp->Tag->Lines));
         free(Tmp->Tag);
      }
      free(Tmp);
   }
}

static void ExpandREPT(void) {
   bool ValOK;
   LongInt ReptCount = 0;
   PInputTag Tag;
   POutputTag Neu;
   bool ErrFlag;

/* 1.Repetitionszahl ermitteln */

   if (ArgCnt != 1) {
      WrError(1110);
      ErrFlag = true;
   } else {
      FirstPassUnknown = false;
      ReptCount = EvalIntExpression(ArgStr[1], Int32, &ValOK);
      if (FirstPassUnknown) WrError(1820);
      ErrFlag = ((!ValOK) || (FirstPassUnknown));
   }
   if (ErrFlag) {
      AddWaitENDM_Processor();
      return;
   }

/* 2. Tag erzeugen */

   GenerateProcessor(&Tag);
   Tag->ParCnt = ReptCount;
   Tag->Processor = REPT_Processor;
   Tag->Restorer = MACRO_Restorer;
   Tag->Cleanup = REPT_Cleanup;
   Tag->IsMacro = true;
   Tag->ParZ = 1;

/* 3. einbetten */

   Neu = (POutputTag) malloc(sizeof(TOutputTag));
   Neu->Processor = REPT_OutProcessor;
   Neu->NestLevel = 0;
   Neu->Next = FirstOutputTag;
   Neu->Tag = Tag;
   FirstOutputTag = Neu;
}

/*- bedingte Wiederholung -------------------------------------------------------*/

static void WHILE_Cleanup(PInputTag PInp) {
   ClearStringList(&(PInp->Lines));
}

bool WHILE_Processor(PInputTag PInp, char *erg) {
   StringRecPtr Lauf;
   Integer z;
   bool OK, Result;

   sprintf(ErrorPos, "%s WHILE %d/%d", PInp->OrigPos, PInp->ParZ, PInp->LineZ);
   CurrLine = PInp->StartLine + PInp->LineZ;

   if (PInp->LineZ == 1) {
      if (!PInp->First) PopLocHandle();
      PInp->First = false;
      PushLocHandle(GetLocHandle());
   } else OK = true;

   Lauf = PInp->Lines;
   for (z = 1; z <= PInp->LineZ - 1; z++) Lauf = Lauf->Next;
   strcpy(erg, Lauf->Content);

   if ((++PInp->LineZ) > PInp->LineCnt) {
      PInp->LineZ = 1;
      PInp->ParZ++;
      z = EvalIntExpression(PInp->SpecName, Int32, &OK);
      OK = (OK && (z != 0));
      Result = OK;
   } else Result = true;

   return Result;
}

static void WHILE_OutProcessor(void) {
   POutputTag Tmp;
   bool OK;
   LongInt Erg;

/* Schachtelungen mitzaehlen */

   if (MacroStart()) FirstOutputTag->NestLevel++;
   else if (MacroEnd()) FirstOutputTag->NestLevel--;

/* falls noch nicht zuende, weiterzaehlen */

   if (FirstOutputTag->NestLevel > -1) {
      AddStringListLast(&(FirstOutputTag->Tag->Lines), OneLine);
      FirstOutputTag->Tag->LineCnt++;
   }

/* alles zusammen? Dann umhaengen */

   if (FirstOutputTag->NestLevel == -1) {
      Tmp = FirstOutputTag;
      FirstOutputTag = FirstOutputTag->Next;
      Tmp->Tag->IsEmpty = (Tmp->Tag->Lines == NULL);
      FirstPassUnknown = false;
      Erg = EvalIntExpression(Tmp->Tag->SpecName, Int32, &OK);
      if (FirstPassUnknown) WrError(1820);
      OK = (OK && (!FirstPassUnknown) && (Erg != 0));
      if ((IfAsm) && (OK)) {
         NextDoLst = (DoLst && LstMacroEx);
         Tmp->Tag->Next = FirstInputTag;
         FirstInputTag = Tmp->Tag;
      } else {
         ClearStringList(&(Tmp->Tag->Lines));
         free(Tmp->Tag);
      }
      free(Tmp);
   }
}

static void ExpandWHILE(void) {
   PInputTag Tag;
   POutputTag Neu;
   bool ErrFlag;

/* 1.Bedingung ermitteln */

   if (ArgCnt != 1) {
      WrError(1110);
      ErrFlag = true;
   } else ErrFlag = false;
   if (ErrFlag) {
      AddWaitENDM_Processor();
      return;
   }

/* 2. Tag erzeugen */

   GenerateProcessor(&Tag);
   Tag->Processor = WHILE_Processor;
   Tag->Restorer = MACRO_Restorer;
   Tag->Cleanup = WHILE_Cleanup;
   Tag->IsMacro = true;
   Tag->ParZ = 1;
   strmaxcpy(Tag->SpecName, ArgStr[1], 255);

/* 3. einbetten */

   Neu = (POutputTag) malloc(sizeof(TOutputTag));
   Neu->Processor = WHILE_OutProcessor;
   Neu->NestLevel = 0;
   Neu->Next = FirstOutputTag;
   Neu->Tag = Tag;
   FirstOutputTag = Neu;
}

/*--------------------------------------------------------------------------*/
/* Einziehen von Include-Files */

static void INCLUDE_Cleanup(PInputTag PInp) {
   fclose(PInp->Datei);
   free(PInp->Buffer);
   LineSum += MomLineCounter;
   if ((*LstName != '\0') && (!QuietMode)) {
      printf("%s(%d)", NamePart(CurrFileName), MomLineCounter);
      printf("%s\n", ClrEol);
   }
   if (MakeIncludeList) PopInclude();
}

bool INCLUDE_Processor(PInputTag PInp, char *Erg) {
   bool Result;

   Result = true;

   if (feof(PInp->Datei)) *Erg = '\0';
   else {
      ReadLn(PInp->Datei, Erg);
     /**ChkIO(10003);**/
   }
   sprintf(ErrorPos, "%s(%d)", NamePart(CurrFileName), CurrLine = (++MomLineCounter));
   if (feof(PInp->Datei)) Result = false;

   return Result;
}

static void INCLUDE_Restorer(PInputTag PInp) {
   MomLineCounter = PInp->StartLine;
   strmaxcpy(CurrFileName, PInp->SpecName, 255);
   strmaxcpy(ErrorPos, PInp->OrigPos, 255);
   IncDepth--;
}

static void ExpandINCLUDE(bool SearchPath) {
   PInputTag Tag;

   if (!IfAsm) return;

   if (ArgCnt != 1) {
      WrError(1110);
      return;
   }

   strmaxcpy(ArgPart, ArgStr[1], 255);
   if (*ArgPart == '"') strcpy(ArgPart, ArgPart + 1);
   if (ArgPart[strlen(ArgPart) - 1] == '"') ArgPart[strlen(ArgPart) - 1] = '\0';
   AddSuffix(ArgPart, IncSuffix);
   strmaxcpy(ArgStr[1], ArgPart, 255);
   if (SearchPath) {
      strmaxcpy(ArgPart, FExpand(FSearch(ArgPart, IncludeList)), 255);
      if (ArgPart[strlen(ArgPart) - 1] == '/') strmaxcat(ArgPart, ArgStr[1], 255);
   }

/* Tag erzeugen */

   GenerateProcessor(&Tag);
   Tag->Processor = INCLUDE_Processor;
   Tag->Restorer = INCLUDE_Restorer;
   Tag->Cleanup = INCLUDE_Cleanup;
   Tag->Buffer = malloc(BufferArraySize);

/* Sicherung alter Daten */

   Tag->StartLine = MomLineCounter;
   strmaxcpy(Tag->SpecName, CurrFileName, 255);

/* Datei oeffnen */

   Tag->Datei = fopen(ArgPart, "r");
   if (Tag->Datei == NULL) ChkIO(10001);
   setvbuf(Tag->Datei, Tag->Buffer, _IOFBF, BufferArraySize);

/* neu besetzen */

   strmaxcpy(CurrFileName, ArgPart, 255);
   MomLineCounter = 0;
   NextIncDepth++;
   AddFile(ArgPart);
   PushInclude(ArgPart);

/* einhaengen */

   Tag->Next = FirstInputTag;
   FirstInputTag = Tag;
}

/*=========================================================================*/
/* Einlieferung von Zeilen */

static void GetNextLine(char *Line) {
   PInputTag HTag;

   while ((FirstInputTag != NULL) && (FirstInputTag->IsEmpty)) {
      FirstInputTag->Restorer(FirstInputTag);
      HTag = FirstInputTag;
      FirstInputTag = HTag->Next;
      free(HTag);
   }

   if (FirstInputTag == NULL) {
      *Line = '\0';
      return;
   }

   if (!FirstInputTag->Processor(FirstInputTag, Line)) {
      FirstInputTag->Cleanup(FirstInputTag);
      FirstInputTag->IsEmpty = true;
   }

   MacLineSum++;
}

static bool InputEnd(void) {
   PInputTag Lauf;

   Lauf = FirstInputTag;
   while (Lauf != NULL) {
      if (!Lauf->IsEmpty) return false;
      Lauf = Lauf->Next;
   }

   return true;
}

/*=== Eine Quelldatei ( Haupt-oder Includedatei ) bearbeiten ===============*/

/*--- aus der zerlegten Zeile Code erzeugen --------------------------------*/

bool HasLabel(void) {
   return ((*LabPart != '\0')
      && ((!Memo("SET")) || (SetIsOccupied))
      && ((!Memo("EVAL")) || (!SetIsOccupied))
      && (!Memo("EQU"))
      && (!Memo("="))
      && (!Memo(":="))
      && (!Memo("MACRO"))
      && (!Memo("FUNCTION"))
      && (!Memo("LABEL"))
      && (!IsDef()));
}

static void Produce_Code(void) {
   Byte z;
   PMacroRec OneMacro;
   bool SearchMacros;

/* Makrosuche unterdruecken ? */

   if (*OpPart == '!') {
      SearchMacros = false;
      strcpy(OpPart, OpPart + 1);
   } else {
      SearchMacros = true;
      ExpandSymbol(OpPart);
   }
   strcpy(LOpPart, OpPart);
   NLS_UpString(OpPart);

/* Prozessor eingehaengt ? */

   if (FirstOutputTag != NULL) {
      FirstOutputTag->Processor();
      return;
   }

/* ansonsten Code erzeugen */

/* evtl. voranstehendes Label ablegen */

   if (IfAsm)
      if (HasLabel()) EnterIntSymbol(LabPart, EProgCounter(), ActPC, false);

/* Makroliste ? */

   if (Memo("IRP")) ExpandIRP();

/* Repetition ? */

   else if (Memo("REPT")) ExpandREPT();

/* bedingte Repetition ? */

   else if (Memo("WHILE")) ExpandWHILE();

/* bedingte Assemblierung ? */

   else if (CodeIFs());

/* Makrodefinition ? */

   else if (Memo("MACRO")) ReadMacro();

/* Abbruch Makroexpansion ? */

   else if (Memo("EXITM")) ExpandEXITM();

/* Includefile? */

   else if Memo
      ("INCLUDE") {
      ExpandINCLUDE(true);
      MasterFile = false;
      }

/* Makroaufruf ? */

   else if ((SearchMacros) && (FoundMacro(&OneMacro))) {
      if (IfAsm) ExpandMacro(OneMacro);
      if (IfAsm) strmaxcpy(ListLine, "(MACRO)", 255);
   }

   else {
      StopfZahl = 0;
      CodeLen = 0;
      DontPrint = false;

      if (IfAsm) {
         if (!CodeGlobalPseudo()) MakeCode();
         if ((MacProOutput) && (strlen(OpPart) != 0)) {
            errno = 0;
            fprintf(MacProFile, "%s\n", OneLine);
            ChkIO(10002);
         }
      }

      for (z = 0; z < StopfZahl; z++) {
         switch (ListGran()) {
            case 4:
               DAsmCode[CodeLen >> 2] = NOPCode;
               break;
            case 2:
               WAsmCode[CodeLen >> 1] = NOPCode;
               break;
            case 1:
               BAsmCode[CodeLen] = NOPCode;
               break;
         }
         CodeLen += ListGran() / Granularity();
      }

      if ((!ChkPC()) && (CodeLen != 0)) WrError(1925);
      else {
         if (!DontPrint) {
            if (MakeUseList)
               if (AddChunk(SegChunks + ActPC, ProgCounter(), CodeLen, ActPC == SegCode)) WrError(90);
            if (DebugMode != DebugNone) AddSectionUsage(ProgCounter(), CodeLen);
         }
         PCs[ActPC] += CodeLen;
      /* if (ActPC!=SegCode)
         {
         if ((CodeLen!=0) && (! DontPrint)) WrError(1940);
         }
         else */ if (CodeOutput) {
            if (DontPrint) NewRecord();
            else WriteBytes();
         }
         if ((DebugMode != DebugNone) && (CodeLen > 0) && (!DontPrint))
            AddLineInfo(true, CurrLine, CurrFileName, ActPC, PCs[ActPC] - CodeLen);
      }
   }
}

/*--- Zeile in Listing zerteilen -------------------------------------------*/

static void SplitLine(void) {
   jmp_buf Retry;
   String h;
   char *i, *k, *p;
   Integer z;
   bool lpos;

   Retracted = false;

/* Kommentar loeschen */

   strmaxcpy(h, OneLine, 255);
   i = QuotPos(h, ';');
   if (i != NULL) {
      strmaxcpy(CommPart, i + 1, 255);
      *i = '\0';
   } else *CommPart = '\0';

/* alles in Grossbuchstaben wandeln, Praeprozessor laufen lassen */

   ExpandDefines(h);

/* Label abspalten */

   if ((*h != '\0') && (!isspace(h[0]))) {
      i = FirstBlank(h);
      k = strchr(h, ':');
      if ((k != NULL) && ((k < i) || (i == NULL))) i = k;
      SplitString(h, LabPart, h, i);
      if (LabPart[strlen(LabPart) - 1] == ':') LabPart[strlen(LabPart) - 1] = '\0';
   } else *LabPart = '\0';

/* Opcode & Argument trennen */
   setjmp(Retry);
   KillPrefBlanks(h);
   i = FirstBlank(h);
   SplitString(h, OpPart, ArgPart, i);

/* Falls noch kein Label da war, kann es auch ein Label sein */

   i = strchr(OpPart, ':');
   if ((*LabPart == '\0') && (i != NULL) && (i == OpPart + strlen(OpPart) - 1)) {
      *i = '\0';
      strmaxcpy(LabPart, OpPart, 255);
      strcpy(OpPart, i + 1);
      if (*OpPart == '\0') {
         strmaxcpy(h, ArgPart, 255);
         longjmp(Retry, 1);
      }
   }

/* Attribut abspalten */

   if (HasAttrs) {
      k = NULL;
      AttrSplit = ' ';
      for (z = 0; z < strlen(AttrChars); z++) {
         p = strchr(OpPart, AttrChars[z]);
            if (p != NULL) if ((k == NULL) || (p < k)) k = p;
      }
      if (k != NULL) {
         AttrSplit = (*k);
         strmaxcpy(AttrPart, k + 1, 255);
         *k = '\0';
         if ((*OpPart == '\0') && (*AttrPart != '\0')) {
            strmaxcpy(OpPart, AttrPart, 255);
            *AttrPart = '\0';
         }
      } else *AttrPart = '\0';
   } else *AttrPart = '\0';

   KillPostBlanks(ArgPart);

/* Argumente zerteilen: Da alles aus einem String kommt und die Teile alle auch
   so lang sind, koennen wir uns Laengenabfragen sparen */
   ArgCnt = 0;
   strcpy(h, ArgPart);
   if (*h != '\0')
      do {
         KillPrefBlanks(h);
         i = h + strlen(h);
         for (z = 0; z < strlen(DivideChars); z++) {
            p = QuotPos(h, DivideChars[z]);
            if ((p != NULL) && (p < i)) i = p;
         }
         lpos = (i == h + strlen(h) - 1);
         if (i >= h) SplitString(h, ArgStr[++ArgCnt], h, i);
         if ((lpos) && (ArgCnt != ParMax)) *ArgStr[++ArgCnt] = '\0';
         KillPostBlanks(ArgStr[ArgCnt]);
      }
      while ((*h != '\0') && (ArgCnt != ParMax));

   if (*h != '\0') WrError(1140);

   Produce_Code();
}

/**
CONST
   LineBuffer:String='';
   InComment:bool=false;

	static void C_SplitLine(void)
{
   p,p2:Integer;
   SaveLine,h:String;

   { alten Inhalt sichern }

   SaveLine:=OneLine; h:=OneLine;

   { Falls in Kommentar, nach schliessender Klammer suchen und den Teil bis
     dahin verwerfen; falls keine Klammer drin ist, die ganze Zeile weg-
     schmeissen; da wir dann OneLine bisher noch nicht veraendert hatten,
     stoert der Abbruch ohne Wiederherstellung von Oneline nicht. }

   IF InComment THEN
    {
     p:=Pos('}',h);
     IF p>Length(h) THEN Exit
     ELSE
      {
       Delete(h,1,p); InComment:=false;
      };
    };

   { in der Zeile befindliche Teile loeschen; falls am Ende keine
     schliessende Klammer kommt, muessen wir das Kommentarflag setzen. }

   REPEAT
    p:=QuotPos(h,'{');
    IF p>Length(h) THEN p:=0
    ELSE
     {
      p2:=QuotPos(h,'}');
      IF (p2>p) && (Length(h)>=p2) THEN Delete(h,p,p2-p+1)
      ELSE
       {
        Byte(h[0]):=Pred(p);
        InComment:=true;
        p:=0;
       };
     };
   UNTIL p=0;

   { alten Inhalt zurueckkopieren }

   OneLine:=SaveLine;
};**/

/*------------------------------------------------------------------------*/

static void ProcessFile(String FileName) {
   long NxtTime, ListTime;
   String Num;
/*Integer z; */
   char *Name;

   sprintf(OneLine, " INCLUDE \"%s\"", FileName);
   MasterFile = false;
   NextIncDepth = IncDepth;
   SplitLine();
   IncDepth = NextIncDepth;

   ListTime = GTime();

   while ((!InputEnd()) && (!ENDOccured)) {
   /* Zeile lesen */

      GetNextLine(OneLine);

   /* Ergebnisfelder vorinitialisieren */

      DontPrint = false;
      CodeLen = 0;
      *ListLine = '\0';

      NextDoLst = DoLst;
      NextIncDepth = IncDepth;

      if (*OneLine == '#') Preprocess();
      else SplitLine();

      MakeList();
      DoLst = NextDoLst;
      IncDepth = NextIncDepth;

   /* Zeilenzaehler */

      if (!QuietMode) {
         NxtTime = GTime();
         if (((strcmp(LstName, "!1") != 0) || ((ListMask & 1) == 0)) && (DTime(ListTime, NxtTime) > 50)) {
            sprintf(Num, "%d", MomLineCounter);
            Name = NamePart(CurrFileName);
            printf("%s(%s)%s", Name, Num, ClrEol);
         /*for (z=0; z<strlen(Name)+strlen(Num)+2; z++) putchar('\b'); */
            putchar('\r');
            ListTime = NxtTime;
         }
      }

   /* bei Ende Makroprozessor ausraeumen
      OK - das ist eine Hauruckmethode... */

      if (ENDOccured)
         while (FirstInputTag != NULL) GetNextLine(OneLine);
   }

   while (FirstInputTag != NULL) GetNextLine(OneLine);

/* irgendeine Makrodefinition nicht abgeschlossen ? */

   if (FirstOutputTag != NULL) WrError(1800);
}

/****************************************************************************/

static char *TWrite_Plur(Integer n) {
   return (n != 1) ? ListPlurName : "";
}

static void TWrite_RWrite(char *dest, Double r, Byte Stellen) {
   String s, form;

   sprintf(form, "%%20.%df", Stellen);
   sprintf(s, form, r);
   while (*s == ' ') strcpy(s, s + 1);
   strcat(dest, s);
}

static void TWrite(Double DTime, char *dest) {
   Integer h;
   String s;

   *dest = '\0';
   h = (int)floor(DTime / 3600.0);
   if (h > 0) {
      sprintf(s, "%d", h);
      strcat(dest, s);
      strcat(dest, ListHourName);
      strcat(dest, TWrite_Plur(h));
      strcat(dest, ", ");
      DTime -= 3600.0 * h;
   }
   h = (int)floor(DTime / 60.0);
   if (h > 0) {
      sprintf(s, "%d", h);
      strcat(dest, s);
      strcat(dest, ListMinuName);
      strcat(dest, TWrite_Plur(h));
      strcat(dest, ", ");
      DTime -= 60.0 * h;
   }
   TWrite_RWrite(dest, DTime, 2);
   strcat(dest, ListSecoName);
   if (DTime != 1) strcat(dest, ListPlurName);
}

/*--------------------------------------------------------------------------*/

static void AssembleFile_InitPass(void) {
   static char DateS[31], TimeS[31];
   Integer z;

   FirstInputTag = NULL;
   FirstOutputTag = NULL;

   ErrorPos[0] = '\0';
   MomLineCounter = 0;
   MomLocHandle = (-1);
   LocHandleCnt = 0;

   SectionStack = NULL;
   FirstIfSave = NULL;
   FirstSaveState = NULL;

   InitPassProc();

   ActPC = SegCode;
   PCs[ActPC] = 0;
   ENDOccured = false;
   ErrorCount = 0;
   WarnCount = 0;
   LineSum = 0;
   MacLineSum = 0;
   for (z = 1; z <= PCMax; z++) {
      PCsUsed[z] = (z == SegCode);
      Phases[z] = 0;
      InitChunk(&SegChunks[z]);
   }
   for (z = 0; z < 256; z++) CharTransTable[z] = z;

   strmaxcpy(CurrFileName, "INTERNAL", 255);
   AddFile(CurrFileName);
   CurrLine = 0;

   IncDepth = (-1);
   DoLst = true;

/* Pseudovariablen initialisieren */

   ResetSymbolDefines();
   ResetMacroDefines();
   EnterIntSymbol(FlagTrueName, 1, 0, true);
   EnterIntSymbol(FlagFalseName, 0, 0, true);
   EnterFloatSymbol(PiName, 4.0 * atan(1.0), true);
   EnterIntSymbol(VerName, VerNo, 0, true);
#ifdef HAS64
   EnterIntSymbol(Has64Name, 1, 0, true);
#else
   EnterIntSymbol(Has64Name, 0, 0, true);
#endif
   EnterIntSymbol(CaseSensName, CaseSensitive, 0, true);
   if (PassNo == 0) {
      NLS_CurrDateString(DateS);
      NLS_CurrTimeString(false, TimeS);
   }
   EnterStringSymbol(DateName, DateS, true);
   EnterStringSymbol(TimeName, TimeS, true);
   SetCPU(0, true);
   SetFlag(&SupAllowed, SupAllowedName, false);
   SetFlag(&FPUAvail, FPUAvailName, false);
   SetFlag(&DoPadding, DoPaddingName, true);
   SetFlag(&Maximum, MaximumName, false);
   EnterIntSymbol(ListOnName, ListOn = 1, 0, true);
   SetFlag(&LstMacroEx, LstMacroExName, true);
   SetFlag(&RelaxedMode, RelaxedName, false);
   CopyDefSymbols();

   ResetPageCounter();

   StartAdrPresent = false;

   Repass = false;
   PassNo++;
}

static void AssembleFile_ExitPass(void) {
   SwitchFrom();
   ClearLocStack();
   ClearStacks();
   if (FirstIfSave != NULL) WrError(1470);
   if (FirstSaveState != NULL) WrError(1460);
   if (SectionStack != NULL) WrError(1485);
}

static void AssembleFile(void) {
   String s;

   if (MakeDebug) fprintf(Debug, "File %s\n", SourceFile);

/* Untermodule initialisieren */

   AsmDefInit();
   AsmParsInit();
   AsmIFInit();
   InitFileList();
   ResetStack();

/* Kommandozeilenoptionen verarbeiten */

   strmaxcpy(OutName, GetFromOutList(), 255);
   if (OutName[0] == '\0') {
      strmaxcpy(OutName, SourceFile, 255);
      KillSuffix(OutName);
      AddSuffix(OutName, PrgSuffix);
   }

   if (ErrorPath[0] == '\0') {
      strmaxcpy(ErrorName, SourceFile, 255);
      KillSuffix(ErrorName);
      AddSuffix(ErrorName, LogSuffix);
   }

   switch (ListMode) {
      case 0:
         strmaxcpy(LstName, NULLDEV, 255);
         break;
      case 1:
         strmaxcpy(LstName, "!1", 255);
         break;
      case 2:
         strmaxcpy(LstName, SourceFile, 255);
         KillSuffix(LstName);
         AddSuffix(LstName, LstSuffix);
         break;
   }

   if (ShareMode != 0) {
      strmaxcpy(ShareName, SourceFile, 255);
      KillSuffix(ShareName);
      switch (ShareMode) {
         case 1:
            AddSuffix(ShareName, ".inc");
            break;
         case 2:
            AddSuffix(ShareName, ".h");
            break;
         case 3:
            AddSuffix(ShareName, IncSuffix);
            break;
      }
   }

   if (MacProOutput) {
      strmaxcpy(MacProName, SourceFile, 255);
      KillSuffix(MacProName);
      AddSuffix(MacProName, PreSuffix);
   }

   if (MacroOutput) {
      strmaxcpy(MacroName, SourceFile, 255);
      KillSuffix(MacroName);
      AddSuffix(MacroName, MacSuffix);
   }

   ClearIncludeList();

   if (DebugMode != DebugNone) InitLineInfo();

/* Variablen initialisieren */

   StartTime = GTime();

   PassNo = 0;
   MomLineCounter = 0;

/* Listdatei eroeffnen */

   if (!QuietMode) printf("%s%s\n", InfoMessAssembling, SourceFile);

   do {
   /* Durchlauf initialisieren */

      AssembleFile_InitPass();
      AsmSubInit();
      if (!QuietMode) printf("%s%d%s\n", InfoMessPass, PassNo, ClrEol);

   /* Dateien oeffnen */

      if (CodeOutput) OpenFile();

      if (ShareMode != 0) {
         ShareFile = fopen(ShareName, "w");
         if (ShareFile == NULL) ChkIO(10001);
         errno = 0;
         switch (ShareMode) {
            case 1:
               fprintf(ShareFile, "(* %s-Includefile f%sr CONST-Sektion *)\n", SourceFile, CH_ue);
               break;
            case 2:
               fprintf(ShareFile, "/* %s-Includefile f%sr C-Programm */\n", SourceFile, CH_ue);
               break;
            case 3:
               fprintf(ShareFile, "; %s-Includefile f%sr Assembler-Programm\n", SourceFile, CH_ue);
               break;
         }
         ChkIO(10002);
      }

      if (MacProOutput) {
         MacProFile = fopen(MacProName, "w");
         if (MacProFile == NULL) ChkIO(10001);
      }

      if ((MacroOutput) && (PassNo == 1)) {
         MacroFile = fopen(MacroName, "w");
         if (MacroFile == NULL) ChkIO(10001);
      }

   /* Listdatei oeffnen */

      RewriteStandard(&LstFile, LstName);
      if (LstFile == NULL) ChkIO(10001);
      errno = 0;
      fprintf(LstFile, "%s", PrtInitString);
      ChkIO(10002);
      if ((ListMask & 1) != 0) NewPage(0, false);

   /* assemblieren */

      ProcessFile(SourceFile);
      AssembleFile_ExitPass();

   /* Dateien schliessen */

      if (CodeOutput) CloseFile();

      if (ShareMode != 0) {
         errno = 0;
         switch (ShareMode) {
            case 1:
               fprintf(ShareFile, "(* Ende Includefile f%sr CONST-Sektion *)\n", CH_ue);
               break;
            case 2:
               fprintf(ShareFile, "/* Ende Includefile f%sr C-Programm */\n", CH_ue);
               break;
            case 3:
               fprintf(ShareFile, "; Ende Includefile f%sr Assembler-Programm\n", CH_ue);
               break;
         }
         ChkIO(10002);
         fclose(ShareFile);
      }

      if (MacProOutput) fclose(MacProFile);
      if ((MacroOutput) && (PassNo == 1)) fclose(MacroFile);

   /* evtl. fuer naechsten Durchlauf aufraeumen */

      if ((ErrorCount == 0) && (Repass)) {
         fclose(LstFile);
         if (CodeOutput) unlink(OutName);
         if (MakeUseList) ClearUseList();
         if (MakeCrossList) ClearCrossList();
         ClearDefineList();
         if (DebugMode != DebugNone) ClearLineInfo();
         ClearIncludeList();
      }
   }
   while ((ErrorCount == 0) && (Repass));

/* bei Fehlern loeschen */

   if (ErrorCount != 0) {
      if (CodeOutput) unlink(OutName);
      if (MacProOutput) unlink(MacProName);
      if ((MacroOutput) && (PassNo == 1)) unlink(MacroName);
      if (ShareMode != 0) unlink(ShareName);
      ErrFlag = true;
   }

/* Debug-Ausgabe muss VOR die Symbollistenausgabe, weil letztere die
   Symbolliste loescht */

   if (DebugMode != DebugNone) {
      if (ErrorCount == 0) DumpDebugInfo();
      ClearLineInfo();
   }

/* Listdatei abschliessen */

   if (strcmp(LstName, NULLDEV) != 0) {
      if ((ListMask & 2) != 0) PrintSymbolList();

      if ((ListMask & 4) != 0) PrintMacroList();

      if ((ListMask & 8) != 0) PrintFunctionList();

      if ((ListMask & 32) != 0) PrintDefineList();

      if (MakeUseList) {
         NewPage(ChapDepth, true);
         PrintUseList();
      }

      if (MakeCrossList) {
         NewPage(ChapDepth, true);
         PrintCrossList();
      }

      if (MakeSectionList) PrintSectionList();

      if (MakeIncludeList) PrintIncludeList();

      errno = 0;
      fprintf(LstFile, "%s", PrtExitString);
      ChkIO(10002);
   }

   if (MakeUseList) ClearUseList();

   if (MakeCrossList) ClearCrossList();

   ClearSectionList();

   ClearIncludeList();

   if ((*ErrorPath == '\0') && (IsErrorOpen)) {
      fclose(ErrorFile);
      IsErrorOpen = false;
   }

   ClearUpProc();

/* Statistik ausgeben */

   StopTime = GTime();
   TWrite(DTime(StartTime, StopTime) / 100.0, s);
   if (!QuietMode) printf("\n%s%s%s\n\n", s, InfoMessAssTime, ClrEol);
   if (ListMode == 2) {
      WrLstLine("");
      strmaxcat(s, InfoMessAssTime, 255);
      WrLstLine(s);
      WrLstLine("");
   }

   strcpy(s, Dec32BlankString(LineSum, 7));
   strmaxcat(s, (LineSum == 1) ? InfoMessAssLine : InfoMessAssLines, 255);
   if (!QuietMode) printf("%s%s\n", s, ClrEol);
   if (ListMode == 2) WrLstLine(s);

   if (LineSum != MacLineSum) {
      strcpy(s, Dec32BlankString(MacLineSum, 7));
      strmaxcat(s, (MacLineSum == 1) ? InfoMessMacAssLine : InfoMessMacAssLines, 255);
      if (!QuietMode) printf("%s%s\n", s, ClrEol);
      if (ListMode == 2) WrLstLine(s);
   }

   strcpy(s, Dec32BlankString(PassNo, 7));
   strmaxcat(s, (PassNo == 1) ? InfoMessPassCnt : InfoMessPPassCnt, 255);
   if (!QuietMode) printf("%s%s\n", s, ClrEol);
   if (ListMode == 2) WrLstLine(s);

   if ((ErrorCount > 0) && (Repass) && (ListMode != 0))
      WrLstLine(InfoMessNoPass);

   sprintf(s, "%s%s", Dec32BlankString(ErrorCount, 7), InfoMessErrCnt);
   if (ErrorCount != 1) strmaxcat(s, InfoMessErrPCnt, 255);
   if (!QuietMode) printf("%s%s\n", s, ClrEol);
   if (ListMode == 2) WrLstLine(s);

   sprintf(s, "%s%s", Dec32BlankString(WarnCount, 7), InfoMessWarnCnt);
   if (WarnCount != 1) strmaxcat(s, InfoMessWarnPCnt, 255);
   if (!QuietMode) printf("%s%s\n", s, ClrEol);
   if (ListMode == 2) WrLstLine(s);

#ifdef __TURBOC__
   sprintf(s, "%s%s", Dec32BlankString(coreleft() >> 10, 7), InfoMessRemainMem);
   if (!QuietMode) printf("%s%s\n", s, ClrEol);
   if (ListMode == 2) WrLstLine(s);

   sprintf(s, "%s%s", Dec32BlankString(StackRes(), 7), InfoMessRemainStack);
   if (!QuietMode) printf("%s%s\n", s, ClrEol);
   if (ListMode == 2) WrLstLine(s);
#endif

   fclose(LstFile);

/* verstecktes */

   if (MakeDebug) PrintSymbolDepth();

/* Speicher freigeben */

   ClearSymbolList();
   ClearMacroList();
   ClearFunctionList();
   ClearDefineList();
   ClearFileList();
}

static void AssembleGroup(void) {
/**   String PathPrefix;
      Search:SearchRec;**/

   AddSuffix(FileMask, SrcSuffix);
   strmaxcpy(SourceFile, FileMask, 255);
   AssembleFile();

/**   FileMask:=FExpand(FileMask);
   AddSuffix(FileMask,SrcSuffix);
   FindFirst(FileMask,AnyFile,Search);
   PathPrefix:=PathPart(FileMask);

   IF DosError<>0 THEN WriteLn(StdErr,FileMask,InfoMessNFilesFound,Char_LF)
   ELSE
    REPEAT
     IF (Search.Attr && (Hidden || SysFile || VolumeID || Directory)=0) THEN
      {
       SourceFile:=PathPrefix+Search.Name;
       AssembleFile;
      };
     FindNext(Search);
    UNTIL DosError<>0**/
}

/*-------------------------------------------------------------------------*/

static CMDResult CMD_SharePascal(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   if (!Negate) ShareMode = 1;
   else if (ShareMode == 1) ShareMode = 0;
   return CMDOK;
}

static CMDResult CMD_ShareC(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   if (!Negate) ShareMode = 2;
   else if (ShareMode == 2) ShareMode = 0;
   return CMDOK;
}

static CMDResult CMD_ShareAssembler(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   if (!Negate) ShareMode = 3;
   else if (ShareMode == 3) ShareMode = 0;
   return CMDOK;
}

static CMDResult CMD_DebugMode(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

/*UpString(Arg);

   if (Negate)
   if (Arg[0]!='\0') return CMDErr;
   else
   {
   DebugMode=DebugNone; return CMDOK;
   }
   else if (strcmp(Arg,"")==0)
   {
   DebugMode=DebugMAP; return CMDOK;
   }
   else if (strcmp(Arg,"MAP")==0)
   {
   DebugMode=DebugMAP; return CMDArg;
   }
   else if (strcmp(Arg,"A.OUT")==0)
   {
   DebugMode=DebugAOUT; return CMDArg;
   }
   else if (strcmp(Arg,"COFF")==0)
   {
   DebugMode=DebugCOFF; return CMDArg;
   }
   else if (strcmp(Arg,"ELF")==0)
   {
   DebugMode=DebugELF; return CMDArg;
   }
   else return CMDErr; */

   if (Negate) DebugMode = DebugNone;
   else DebugMode = DebugMAP;
   return CMDOK;
}

static CMDResult CMD_ListConsole(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   if (!Negate) ListMode = 1;
   else if (ListMode == 1) ListMode = 0;
   return CMDOK;
}

static CMDResult CMD_ListFile(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   if (!Negate) ListMode = 2;
   else if (ListMode == 2) ListMode = 0;
   return CMDOK;
}

static CMDResult CMD_SuppWarns(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   SuppWarns = !Negate;
   return CMDOK;
}

static CMDResult CMD_UseList(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   MakeUseList = !Negate;
   return CMDOK;
}

static CMDResult CMD_CrossList(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   MakeCrossList = !Negate;
   return CMDOK;
}

static CMDResult CMD_SectionList(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   MakeSectionList = !Negate;
   return CMDOK;
}

static CMDResult CMD_BalanceTree(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   BalanceTree = !Negate;
   return CMDOK;
}

static CMDResult CMD_MakeDebug(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   if (!Negate) {
      MakeDebug = true;
      errno = 0;
      Debug = fopen("as.deb", "w");
      if (Debug == NULL) ChkIO(10002);
   } else if (MakeDebug) {
      MakeDebug = false;
      fclose(Debug);
   }
   return CMDOK;
}

static CMDResult CMD_MacProOutput(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   MacProOutput = !Negate;
   return CMDOK;
}

static CMDResult CMD_MacroOutput(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   MacroOutput = !Negate;
   return CMDOK;
}

static CMDResult CMD_MakeIncludeList(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   MakeIncludeList = !Negate;
   return CMDOK;
}

static CMDResult CMD_CodeOutput(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   CodeOutput = !Negate;
   return CMDOK;
}

static CMDResult CMD_MsgIfRepass(bool Negate, String Arg) {
   bool OK;

   MsgIfRepass = !Negate;
   if (MsgIfRepass)
      if (Arg[0] == '\0') {
         PassNoForMessage = 1;
         return CMDOK;
      } else {
         PassNoForMessage = ConstLongInt(Arg, &OK);
         if (!OK) {
            PassNoForMessage = 1;
            return CMDOK;
         } else if (PassNoForMessage < 1) return CMDErr;
         else return CMDArg;
   } else return CMDOK;
}

static CMDResult CMD_ExtendErrors(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   ExtendErrors = !Negate;
   return CMDOK;
}

static CMDResult CMD_NumericErrors(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   NumericErrors = !Negate;
   return CMDOK;
}

static CMDResult CMD_HexLowerCase(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   HexLowerCase = !Negate;
   return CMDOK;
}

static CMDResult CMD_QuietMode(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   QuietMode = !Negate;
   return CMDOK;
}

static CMDResult CMD_ThrowErrors(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   ThrowErrors = !Negate;
   return CMDOK;
}

static CMDResult CMD_CaseSensitive(bool Negate, char *Arg) {
   if (Arg == NULL); /* satisfy some compilers */

   CaseSensitive = !Negate;
   return CMDOK;
}

static CMDResult CMD_IncludeList(bool Negate, char *Arg) {
   char *p;
   String Copy, part;

   if (*Arg == '\0') return CMDErr;
   else {
      strncpy(Copy, Arg, 255);
      do {
         p = strrchr(Copy, ':');
         if (p == NULL) {
            strmaxcpy(part, Copy, 255);
            *Copy = '\0';
         } else {
            *p = '\0';
            strmaxcpy(part, p + 1, 255);
         }
         if (Negate) RemoveIncludeList(part);
         else AddIncludeList(part);
      }
      while (Copy[0] != '\0');
      return CMDArg;
   }
}

static CMDResult CMD_ListMask(bool Negate, char *Arg) {
   Byte erg;
   bool OK;

   if (Arg[0] == '\0') return CMDErr;
   else {
      erg = ConstLongInt(Arg, &OK);
   // OK = ConstLongInt(Arg, &erg); //(@) Formerly: which was a bug.
      if ((!OK) || (erg > 31)) return CMDErr;
      else {
         if (Negate) ListMask &= (~erg);
         else ListMask |= erg;
         return CMDArg;
      }
   }
}

static CMDResult CMD_DefSymbol(bool Negate, char *Arg) {
   String Copy, Part, Name;
   char *p;
   TempResult t;

   if (Arg[0] == '\0') return CMDErr;

   strmaxcpy(Copy, Arg, 255);
   do {
      p = QuotPos(Copy, ',');
      if (p == NULL) {
         strmaxcpy(Part, Copy, 255);
         Copy[0] = '\0';
      } else {
         *p = '\0';
         strmaxcpy(Part, Copy, 255);
         strcpy(Copy, p + 1);
      }
      UpString(Part);
      p = QuotPos(Part, '=');
      if (p == NULL) {
         strmaxcpy(Name, Part, 255);
         Part[0] = '\0';
      } else {
         *p = '\0';
         strmaxcpy(Name, Part, 255);
         strcpy(Part, p + 1);
      }
      if (!ChkSymbName(Name)) return CMDErr;
      if (Negate) RemoveDefSymbol(Name);
      else {
         AsmParsInit();
         if (Part[0] != '\0') {
            FirstPassUnknown = false;
            EvalExpression(Part, &t);
            if ((t.Typ == TempNone) || (FirstPassUnknown)) return CMDErr;
         } else {
            t.Typ = TempInt;
            t.Contents.Int = 1;
         }
         AddDefSymbol(Name, &t);
      }
   }
   while (Copy[0] != '\0');

   return CMDArg;
}

static CMDResult CMD_ErrorPath(bool Negate, String Arg) {
   if (Negate) return CMDErr;
   else if (Arg[0] == '\0') {
      ErrorPath[0] = '\0';
      return CMDOK;
   } else {
      strncpy(ErrorPath, Arg, 255);
      return CMDArg;
   }
}

static CMDResult CMD_OutFile(bool Negate, char *Arg) {
   if (Arg[0] == '\0')
      if (Negate) {
         ClearOutList();
         return CMDOK;
      } else return CMDErr;
   else {
      if (Negate) RemoveFromOutList(Arg);
      else AddToOutList(Arg);
      return CMDArg;
   }
}

static bool CMD_CPUAlias_ChkCPUName(char *s) {
   Integer z;

   for (z = 0; z < strlen(s); z++)
      if (!isalnum(s[z])) return false;
   return true;
}

static CMDResult CMD_CPUAlias(bool Negate, char *Arg) {
   char *p;
   String s1, s2;

   if (Negate) return CMDErr;
   else if (Arg[0] == '\0') return CMDErr;
   else {
      p = strchr(Arg, '=');
      if (p == NULL) return CMDErr;
      else {
         *p = '\0';
         strmaxcpy(s1, Arg, 255);
         UpString(s1);
         strmaxcpy(s2, p + 1, 255);
         UpString(s2);
         *p = '=';
         if (!(CMD_CPUAlias_ChkCPUName(s1) && CMD_CPUAlias_ChkCPUName(s2)))
            return CMDErr;
         else if (!AddCPUAlias(s2, s1)) return CMDErr;
         else return CMDArg;
      }
   }
}

static void ParamError(bool InEnv, char *Arg) {
   printf("%s%s\n", (InEnv) ? (ErrMsgInvEnvParam) : (ErrMsgInvParam), Arg);
   exit(4);
}

#define ASParamCnt 30
static CMDRec ASParams[ASParamCnt] = { { "A", CMD_BalanceTree },
{ "ALIAS", CMD_CPUAlias },
{ "a", CMD_ShareAssembler },
{ "C", CMD_CrossList },
{ "c", CMD_ShareC },
{ "D", CMD_DefSymbol },
{ "E", CMD_ErrorPath },
{ "g", CMD_DebugMode },
{ "G", CMD_CodeOutput },
{ "h", CMD_HexLowerCase },
{ "i", CMD_IncludeList },
{ "I", CMD_MakeIncludeList },
{ "L", CMD_ListFile },
{ "l", CMD_ListConsole },
{ "M", CMD_MacroOutput },
{ "n", CMD_NumericErrors },
{ "o", CMD_OutFile },
{ "P", CMD_MacProOutput },
{ "p", CMD_SharePascal },
{ "q", CMD_QuietMode },
{ "QUIET", CMD_QuietMode },
{ "r", CMD_MsgIfRepass },
{ "s", CMD_SectionList },
{ "t", CMD_ListMask },
{ "u", CMD_UseList },
{ "U", CMD_CaseSensitive },
{ "w", CMD_SuppWarns },
{ "x", CMD_ExtendErrors },
{ "X", CMD_MakeDebug },
{ "Y", CMD_ThrowErrors }
};

/*--------------------------------------------------------------------------*/

#ifdef __sunos__

extern void on_exit(void (*procp)(int status, caddr_t arg), caddr_t arg);

static void GlobExitProc(int status, caddr_t arg) {
   if (MakeDebug) fclose(Debug);
}

#else

static void GlobExitProc(void) {
   if (MakeDebug) fclose(Debug);
}

#endif

static Integer LineZ;

static void NxtLine(void) {
   if (++LineZ == 23) {
      LineZ = 0;
      if (Redirected != NoRedir) return;
      printf("%s", KeyWaitMsg);
      while (getchar() != '\n');
      printf("%s%s", CursUp, ClrEol);
   }
}

static void WrHead(void) {
   if (!QuietMode) {
      setbuf(stdout, NULL);
      printf("%s%s\n", InfoMessMacroAss, Version);
      NxtLine();
      printf("C-Version\n");
      NxtLine();
      printf("%s\n", InfoMessCopyright);
      NxtLine();
      WriteCopyrights(NxtLine);
      printf("\n");
      NxtLine();
   }
}

int main(int argc, char **argv) {
   char *Env;
   String Dummy;
   Integer i;
   CMDProcessed ParUnprocessed; /* bearbeitete Kommandozeilenparameter */

   ParamCount = argc - 1;
   ParamStr = argv;

/* in Pascal geht soetwas automatisch - Bauernsprache! */

   endian_init();
   nls_init();
   bpemu_init();
   stdhandl_init();
   stringutil_init();
   stringlists_init();
   chunks_init();

   asmfnums_init();
   asminclist_init();
   asmitree_init();

   asmdef_init();
   asmsub_init();
   asmpars_init();

   asmmac_init();
   asmif_init();
   asmcode_init();
   asmdebug_init();

   codeallg_init();
   codepseudo_init();

   code68k_init();
   code56k_init();
   code601_init();
   code68_init();
   code6805_init();
   code6809_init();
   code6812_init();
   code6816_init();
   codeh8_3_init();
   codeh8_5_init();
   code7000_init();
   code65_init();
   code7700_init();
   code4500_init();
   codem16_init();
   codem16c_init();
   code48_init();
   code51_init();
   code96_init();
   code85_init();
   code86_init();
   code8x30x_init();
   codexa_init();
   codeavr_init();
   code29k_init();
   code166_init();
   codez80_init();
   codez8_init();
   code96c141_init();
   code90c141_init();
   code87c800_init();
   code47c00_init();
   code97c241_init();
   code16c5x_init();
   code16c8x_init();
   code17c4x_init();
   codest6_init();
   codest7_init();
   codest9_init();
   code6804_init();
   code3201x_init();
   code3202x_init();
   code3203x_init();
   code3205x_init();
   code9900_init();
   codetms7_init();
   code370_init();
   codemsp_init();
   code78c10_init();
   code75k0_init();
   code78k0_init();
   codescmp_init();
   codecop8_init();
/*as1750_init(); */

#ifdef __sunos__
   on_exit(GlobExitProc, (caddr_t) NULL);
#else
#   ifndef __MUNIX__
   atexit(GlobExitProc);
#   endif
#endif

   NLS_Initialize();

   *CursUp = '\0';
   *ClrEol = '\0';
   switch (Redirected) {
      case NoRedir:
         Env = getenv("USEANSI");
         strncpy(Dummy, (Env != NULL) ? Env : "Y", 255);
         if (toupper(Dummy[0]) == 'N') {
         } else {
            strcpy(ClrEol, " [K");
            ClrEol[0] = Char_ESC; /* ANSI-Sequenzen */
            strcpy(CursUp, " [A");
            CursUp[0] = Char_ESC;
         }
         break;
      case RedirToDevice:
      /* Basissteuerzeichen fuer Geraete */
         for (i = 1; i <= 20; i++) strcat(ClrEol, " ");
         for (i = 1; i <= 20; i++) strcat(ClrEol, "\b");
         break;
      case RedirToFile:
         strcpy(ClrEol, "\n"); /* CRLF auf Datei */
   }

   ShareMode = 0;
   ListMode = 0;
   IncludeList[0] = '\0';
   SuppWarns = false;
   MakeUseList = false;
   MakeCrossList = false;
   MakeSectionList = false;
   MakeIncludeList = false;
   ListMask = 0x3f;
   MakeDebug = false;
   ExtendErrors = false;
   MacroOutput = false;
   MacProOutput = false;
   CodeOutput = true;
   strcpy(ErrorPath, "!2");
   MsgIfRepass = false;
   QuietMode = false;
   NumericErrors = false;
   DebugMode = DebugNone;
   CaseSensitive = false;
   ThrowErrors = false;

   LineZ = 0;

   if (ParamCount == 0) {
      WrHead();
      printf("%s%s%s\n", InfoMessHead1, GetEXEName(), InfoMessHead2);
      NxtLine();
      for (i = 0; i < InfoMessHelpCnt; i++) {
         printf("%s\n", InfoMessHelp[i]);
         NxtLine();
      }
      PrintCPUList(NxtLine);
      ClearCPUList();
      exit(1);
   }
#if defined(STDINCLUDES)
   CMD_IncludeList(false, STDINCLUDES);
#endif
   ProcessCMD(ASParams, ASParamCnt, ParUnprocessed, EnvName, ParamError);

/* wegen QuietMode dahinter */

   WrHead();

   ErrFlag = false;
   if (ErrorPath[0] != '\0') strcpy(ErrorName, ErrorPath);
   IsErrorOpen = false;

   for (i = 1; i <= ParamCount; i++)
      if (ParUnprocessed[i]) break;
   if (i > ParamCount) {
      printf("%s [%s] ", InvMsgSource, SrcSuffix);
      fgets(FileMask, 255, stdin);
      AssembleGroup();
   } else
      for (i = 1; i <= ParamCount; i++)
         if (ParUnprocessed[i]) {
            strmaxcpy(FileMask, ParamStr[i], 255);
            AssembleGroup();
         }

   if ((ErrorPath[0] != '\0') && (IsErrorOpen)) {
      fclose(ErrorFile);
      IsErrorOpen = false;
   }

   ClearCPUList();

   if (ErrFlag) return (2);
   else return (0);
}
