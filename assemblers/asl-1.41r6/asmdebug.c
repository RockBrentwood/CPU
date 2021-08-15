/* asmdebug.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung der Debug-Informationen zur Assemblierzeit                     */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmfnums.h"

#include "asmdebug.h"

typedef struct {
   bool InASM;
   LongInt LineNum;
   Integer FileName;
   ShortInt Space;
   LongInt Address;
} TLineInfo;

typedef struct _TLineInfoList {
   struct _TLineInfoList *Next;
   TLineInfo Contents;
} TLineInfoList, *PLineInfoList;

String TempFileName;
FILE *TempFile;
PLineInfoList LineInfoRoot;

void AddLineInfo(bool InASM, LongInt LineNum, char *FileName, ShortInt Space, LongInt Address) {
   PLineInfoList P, Run;

   P = (PLineInfoList) malloc(sizeof(TLineInfoList));
   P->Contents.InASM = InASM;
   P->Contents.LineNum = LineNum;
   P->Contents.FileName = GetFileNum(FileName);
   P->Contents.Space = Space;
   P->Contents.Address = Address;

   Run = LineInfoRoot;
   if (Run == NULL) {
      LineInfoRoot = P;
      P->Next = NULL;
   } else {
      while ((Run->Next != NULL) && (Run->Next->Contents.Space < Space)) Run = Run->Next;
      while ((Run->Next != NULL) && (Run->Next->Contents.FileName < P->Contents.FileName)) Run = Run->Next;
      while ((Run->Next != NULL) && (Run->Next->Contents.Address < Address)) Run = Run->Next;
      P->Next = Run->Next;
      Run->Next = P;
   }
}

void InitLineInfo(void) {
   TempFileName[0] = '\0';
   LineInfoRoot = NULL;
}

void ClearLineInfo(void) {
   PLineInfoList Run;

   if (TempFileName[0] != '\0') {
      fclose(TempFile);
      unlink(TempFileName);
   }

   while (LineInfoRoot != NULL) {
      Run = LineInfoRoot;
      LineInfoRoot = LineInfoRoot->Next;
      free(Run);
   }

   InitLineInfo();
}

static void DumpDebugInfo_MAP(void) {
   PLineInfoList Run;
   Integer ActFile, ModZ;
   ShortInt ActSeg;
   FILE *MAPFile;
   String MAPName;

   strmaxcpy(MAPName, SourceFile, 255);
   KillSuffix(MAPName);
   AddSuffix(MAPName, MapSuffix);
   MAPFile = fopen(MAPName, "w");
   if (MAPFile == NULL) ChkIO(10001);

   Run = LineInfoRoot;
   ActSeg = (-1);
   ActFile = (-1);
   ModZ = 0;
   while (Run != NULL) {
      if (Run->Contents.Space != ActSeg) {
         ActSeg = Run->Contents.Space;
         if (ModZ != 0) {
            errno = 0;
            fprintf(MAPFile, "\n");
            ChkIO(10004);
         }
         ModZ = 0;
         errno = 0;
         fprintf(MAPFile, "Segment %s\n", SegNames[ActSeg]);
         ChkIO(10004);
         ActFile = (-1);
      }
      if (Run->Contents.FileName != ActFile) {
         ActFile = Run->Contents.FileName;
         if (ModZ != 0) {
            errno = 0;
            fprintf(MAPFile, "\n");
            ChkIO(10004);
         }
         ModZ = 0;
         errno = 0;
         fprintf(MAPFile, "File %s\n", GetFileName(Run->Contents.FileName));
         ChkIO(10004);
      };
      errno = 0;
      fprintf(MAPFile, "%5d:%08x ", Run->Contents.LineNum, Run->Contents.Address);
      ChkIO(10004);
      if (++ModZ == 5) {
         errno = 0;
         fprintf(MAPFile, "\n");
         ChkIO(10004);
         ModZ = 0;
      }
      Run = Run->Next;
   }
   if (ModZ != 0) {
      errno = 0;
      fprintf(MAPFile, "\n");
      ChkIO(10004);
   }

   PrintDebSymbols(MAPFile);

   PrintDebSections(MAPFile);

   fclose(MAPFile);
}

void DumpDebugInfo(void) {
   switch (DebugMode) {
      case DebugMAP:
         DumpDebugInfo_MAP();
         break;
      default:
         break;
   }
}

void asmdebug_init(void) {
}
