/* toolutils.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterroutinen fuer die AS-Tools                                           */
/*                                                                           */
/* Historie: 31. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "stringutil.h"
#include "decodecmd.h"
#include "stdhandl.h"
#include "ioerrors.h"

#include "toolutils.h"

LongWord Magic = 0x1b342b4d;

#include "tools.rsc"

/****************************************************************************/

static bool DoFilter;
static Integer FilterCnt;
static Byte FilterBytes[100];

static char *InfoMessCopyright = "(C) 1992,1997 Alfred Arnold";

Word FileID = 0x1489; /* Dateiheader Eingabedateien */
char *OutName = "STDOUT"; /* Pseudoname Output */

/****************************************************************************/

void WrCopyRight(char *Msg) {
   printf("%s\n%s\n", Msg, InfoMessCopyright);
}

void DelSuffix(char *Name) {
   char *p, *z, *Part;

   p = NULL;
      for (z = Name; *z != '\0'; z++) if (*z == '\\') p = z;
   Part = (p != NULL) ? (p) : (Name);
   Part = strchr(Part, '.');
   if (Part != NULL) *Part = '\0';
}

void AddSuffix(char *s, char *Suff) {
   char *p, *z, *Part;

   p = NULL;
      for (z = s; *z != '\0'; z++) if (*z == '\\') p = z;
   Part = (p != NULL) ? (p) : (s);
   if (strchr(Part, '.') == NULL) strmaxcat(s, Suff, 255);
}

void FormatError(char *Name, char *Detail) {
   fprintf(stderr, "%s%s%s (%s)\n", FormatErr1aMsg, Name, FormatErr1bMsg, Detail);
   fprintf(stderr, "%s\n", FormatErr2Msg);
   exit(3);
}

void ChkIO(char *Name) {
   int io;

   io = errno;

   if (io == 0) return;

   fprintf(stderr, "%s%s%s\n", IOErrAHeaderMsg, Name, IOErrBHeaderMsg);

   fprintf(stderr, "%s.\n", GetErrorMsg(io));

   fprintf(stderr, "%s\n", ErrMsgTerminating);

   exit(2);
}

Word Granularity(Byte Header) {
   switch (Header) {
      case 0x09:
      case 0x76:
         return 4;
      case 0x70:
      case 0x71:
      case 0x72:
      case 0x74:
      case 0x75:
      case 0x77:
      case 0x12:
      case 0x3b:
         return 2;
      default:
         return 1;
   }
}

void ReadRecordHeader(Byte * Header, Byte * Segment, Byte * Gran, char *Name, FILE * f) {
   if (fread(Header, 1, 1, f) != 1) ChkIO(Name);
   if ((*Header != FileHeaderEnd) && (*Header != FileHeaderStartAdr))
      if (*Header == FileHeaderDataRec) {
         if (fread(Header, 1, 1, f) != 1) ChkIO(Name);
         if (fread(Segment, 1, 1, f) != 1) ChkIO(Name);
         if (fread(Gran, 1, 1, f) != 1) ChkIO(Name);
      } else {
         *Segment = SegCode;
         *Gran = Granularity(*Header);
      }
}

void WriteRecordHeader(Byte * Header, Byte * Segment, Byte * Gran, char *Name, FILE * f) {
   Byte h;

   if ((*Header == FileHeaderEnd) || (*Header == FileHeaderStartAdr)) {
      if (fwrite(Header, 1, 1, f) != 1) ChkIO(Name);
   } else if ((*Segment != SegCode) || (*Gran != Granularity(*Header))) {
      h = FileHeaderDataRec;
      if (fwrite(&h, 1, 1, f)) ChkIO(Name);
      if (fwrite(Header, 1, 1, f)) ChkIO(Name);
      if (fwrite(Segment, 1, 1, f)) ChkIO(Name);
      if (fwrite(Gran, 1, 1, f)) ChkIO(Name);
   } else {
      if (fwrite(Header, 1, 1, f)) ChkIO(Name);
   }
}

CMDResult CMD_FilterList(bool Negate, char *Arg) {
   Byte FTemp;
   bool err;
   char *p;
   Integer Search;
   String Copy;

   if (*Arg == '\0') return CMDErr;
   strmaxcpy(Copy, Arg, 255);

   do {
      p = strchr(Copy, ',');
      if (p != NULL) *p = '\0';
      FTemp = ConstLongInt(Copy, &err);
      if (!err) return CMDErr;

      for (Search = 0; Search < FilterCnt; Search++)
         if (FilterBytes[Search] == FTemp) break;

      if ((Negate) && (Search < FilterCnt))
         FilterBytes[Search] = FilterBytes[--FilterCnt];

      else if ((!Negate) && (Search >= FilterCnt))
         FilterBytes[FilterCnt++] = FTemp;

      if (p != NULL) strcpy(Copy, p + 1);
   }
   while (p != NULL);

   DoFilter = (FilterCnt != 0);

   return CMDArg;
}

bool FilterOK(Byte Header) {
   Integer z;

   if (DoFilter) {
      for (z = 0; z < FilterCnt; z++)
         if (Header == FilterBytes[z]) return true;
      return false;
   } else return true;
}

bool RemoveOffset(char *Name, LongWord * Offset) {
   Integer z, Nest;
   bool err;

   *Offset = 0;
   if (Name[strlen(Name) - 1] == ')') {
      z = strlen(Name) - 2;
      Nest = 0;
      while ((z >= 0) && (Nest >= 0)) {
         switch (Name[z]) {
            case '(':
               Nest--;
               break;
            case ')':
               Nest++;
               break;
         }
         if (Nest != -1) z--;
      }
      if (Nest != -1) return false;
      else {
         Name[strlen(Name) - 1] = '\0';
         *Offset = ConstLongInt(Name + z + 1, &err);
         Name[z] = '\0';
         return err;
      }
   } else return true;
}

void toolutils_init(void) {
   Word z;
   LongWord XORVal;

   FilterCnt = 0;
   DoFilter = false;
   for (z = 0; z < strlen(InfoMessCopyright); z++) {
      XORVal = InfoMessCopyright[z];
      XORVal = XORVal << (((z + 1) % 4) * 8);
      Magic = Magic ^ XORVal;
   }
}
