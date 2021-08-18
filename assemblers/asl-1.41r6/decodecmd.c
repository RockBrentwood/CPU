// AS-Portierung
// Verarbeitung Kommandozeilenparameter
#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "stringutil.h"
#include "decodecmd.h"
#include "decodecmd.rsc"

LongInt ParamCount; /* Kommandozeilenparameter */
char **ParamStr;

static void ClrBlanks(char *tmp) {
   Integer cnt;

   for (cnt = 0; isspace(tmp[cnt]); cnt++);
   if (cnt > 0) strmove(tmp, cnt);
}

bool ProcessedEmpty(CMDProcessed Processed) {
   Integer z;

   for (z = 1; z <= ParamCount; z++)
      if (Processed[z]) return false;
   return true;
}

static CMDResult ProcessParam(CMDRec * Def, Integer Cnt, char *O_Param, char *O_Next) {
   Integer Start;
   bool Negate;
   Integer z, Search;
   CMDResult TempRes;
   String s, Param, Next;

   strncpy(Param, O_Param, 255);
   strncpy(Next, O_Next, 255);

   if ((*Next == '-') || (*Next == '+')) *Next = '\0';
   if ((*Param == '-') || (*Param == '+')) {
      Negate = (*Param == '+');
      Start = 1;

      if (Param[Start] == '#') {
         for (z = Start + 1; z < strlen(Param); z++) Param[z] = toupper(Param[z]);
         Start++;
      } else if (Param[Start] == '~') {
         for (z = Start + 1; z < strlen(Param); z++) Param[z] = tolower(Param[z]);
         Start++;
      }

      TempRes = CMDOK;

      Search = 0;
      strncpy(s, Param + Start, 255);
      for (z = 0; z < strlen(s); z++) s[z] = toupper(s[z]);
      for (Search = 0; Search < Cnt; Search++)
         if ((strlen(Def[Search].Ident) > 1) && (strcmp(s, Def[Search].Ident) == 0)) break;
      if (Search < Cnt)
         TempRes = Def[Search].Callback(Negate, Next);

      else
         for (z = Start; z < strlen(Param); z++)
            if (TempRes != CMDErr) {
               Search = 0;
               for (Search = 0; Search < Cnt; Search++)
                  if ((strlen(Def[Search].Ident) == 1) && (Def[Search].Ident[0] == Param[z])) break;
               if (Search >= Cnt) TempRes = CMDErr;
               else
                  switch (Def[Search].Callback(Negate, Next)) {
                     case CMDErr:
                        TempRes = CMDErr;
                        break;
                     case CMDArg:
                        TempRes = CMDArg;
                        break;
                     case CMDOK:
                        break;
                     case CMDFile:
                        break;   /** **/
                  }
            }
      return TempRes;
   } else return CMDFile;
}

static void DecodeLine(CMDRec * Def, Integer Cnt, char *OneLine, CMDErrCallback ErrProc) {
   Integer z;
   char *EnvStr[256], *start, *p;
   Integer EnvCnt = 0;

   ClrBlanks(OneLine);
   if ((*OneLine != '\0') && (*OneLine != ';')) {
      start = OneLine;
      while (*start != '\0') {
         EnvStr[EnvCnt++] = start;
         p = strchr(start, ' ');
         if (p == NULL) p = strchr(start, 9);
         if (p != NULL) {
            *p = '\0';
            start = p + 1;
            while (isspace(*start)) start++;
         } else start += strlen(start);
      }
      EnvStr[EnvCnt] = start;

      for (z = 0; z < EnvCnt; z++)
         switch (ProcessParam(Def, Cnt, EnvStr[z], EnvStr[z + 1])) {
            case CMDErr:
            case CMDFile:
               ErrProc(true, EnvStr[z]);
               break;
            case CMDArg:
               z++;
               break;
            case CMDOK:
               break;
         }
   }
}

void ProcessCMD(CMDRec * Def, Integer Cnt, CMDProcessed Unprocessed, char *EnvName, CMDErrCallback ErrProc) {
   Integer z;
   String OneLine;
   FILE *KeyFile;

   if (getenv(EnvName) == NULL) OneLine[0] = '\0';
   else strncpy(OneLine, getenv(EnvName), 255);

   if (OneLine[0] == '@') {
      strmove(OneLine, 1);
      ClrBlanks(OneLine);
      KeyFile = fopen(OneLine, "r");
      if (KeyFile == NULL) ErrProc(true, ErrMsgKeyFileNotFound);
      while (!feof(KeyFile)) {
         errno = 0;
         ReadLn(KeyFile, OneLine);
         if (errno != 0) ErrProc(true, ErrMsgKeyFileError);
         DecodeLine(Def, Cnt, OneLine, ErrProc);
      }
      fclose(KeyFile);
   }

   else DecodeLine(Def, Cnt, OneLine, ErrProc);

   for (z = 0; z <= ParamCount; z++) Unprocessed[z] = z != 0;
   for (z = 1; z <= ParamCount; z++)
      if (Unprocessed[z]) {
         switch (ProcessParam(Def, Cnt, ParamStr[z], (z < ParamCount) ? ParamStr[z + 1] : "")) {
            case CMDErr:
               ErrProc(false, ParamStr[z]);
               break;
            case CMDOK:
               Unprocessed[z] = false;
               break;
            case CMDArg:
               Unprocessed[z] = Unprocessed[z + 1] = false;
               break;
            case CMDFile:
               break;       /** **/
         }
      }
}

char *GetEXEName(void) {
   static String s;
   char *pos;

   strcopy(s, ParamStr[0]);
   do {
      pos = strchr(s, '/');
      if (pos != NULL) strcopy(s, pos + 1);
   }
   while (pos != NULL);
   pos = strchr(s, '.');
   if (pos != NULL) *pos = '\0';
   return s;
}

void decodecmd_init(void) {
}
