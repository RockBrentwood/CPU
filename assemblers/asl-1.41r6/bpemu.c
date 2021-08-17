/* bpemu.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Emulation einiger Borland-Pascal-Funktionen                               */
/*                                                                           */
/* Historie: 20. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <sys/types.h>
#include <ctype.h>

#include "stringutil.h"

#ifdef __MSDOS__
#   include <dir.h>
#endif

#ifdef __EMX__
#   include <os2.h>
#endif

char *FExpand(char *Src) {
   static String CurrentDir;
   String Copy;
#ifdef DRSEP
   String DrvPart;
#   ifdef __EMX__
   ULONG DrvNum, Dummy;
#   else
   int DrvNum;
#   endif
#endif
   char *p, *p2;

   strmaxcpy(Copy, Src, 255);

#ifdef DRSEP
   p = strchr(Copy, DRSEP);
   if (p != NULL) {
      memcpy(DrvPart, Copy, p - Copy);
      DrvPart[p - Copy] = '\0';
      strcopy(Copy, p + 1);
   } else *DrvPart = '\0';
#endif

#ifdef __MSDOS__
   if (*DrvPart == '\0') {
      DrvNum = getdisk();
      *DrvPart = DrvNum + 'A';
      DrvPart[1] = '\0';
      DrvNum++;
   } else DrvNum = toupper(*DrvPart) - '@';
   getcurdir(DrvNum, CurrentDir);
#else
#   ifdef __EMX__
   if (*DrvPart == '\0') {
      DosQueryCurrentDisk(&DrvNum, &Dummy);
      *DrvPart = DrvNum + '@';
      DrvPart[1] = '\0';
   } else DrvNum = toupper(*DrvPart) - '@';
   Dummy = 255;
   DosQueryCurrentDir(DrvNum, (PBYTE) CurrentDir, &Dummy);
#   else
   getcwd(CurrentDir, 255);
#   endif
#endif

   if (CurrentDir[strlen(CurrentDir) - 1] != PATHSEP) strmaxcat(CurrentDir, SPATHSEP, 255);
   if (*CurrentDir != PATHSEP) strmaxprep(CurrentDir, SPATHSEP, 255);

   if (*Copy == PATHSEP) {
      strmaxcpy(CurrentDir, SPATHSEP, 255);
      strmove(Copy, 1);
   }
#ifdef DRSEP
   strmaxprep(CurrentDir, SDRSEP, 255);
   strmaxprep(CurrentDir, DrvPart, 255);
#endif

   while ((p = strchr(Copy, PATHSEP)) != NULL) {
      *p = '\0';
      if (strcmp(Copy, ".") == 0);
      else if ((strcmp(Copy, "..") == 0) && (strlen(CurrentDir) > 1)) {
         CurrentDir[strlen(CurrentDir) - 1] = '\0';
         p2 = strrchr(CurrentDir, PATHSEP);
         p2[1] = '\0';
      } else {
         strmaxcat(CurrentDir, Copy, 255);
         strmaxcat(CurrentDir, SPATHSEP, 255);
      }
      strcopy(Copy, p + 1);
   }

   strmaxcat(CurrentDir, Copy, 255);

   return CurrentDir;
}

char *FSearch(char *File, char *Path) {
   static String Component;
   char *p, *start, Save = '\0';
   FILE *Dummy;
   bool OK;

   Dummy = fopen(File, "r");
   OK = (Dummy != NULL);
   if (OK) {
      fclose(Dummy);
      strmaxcpy(Component, File, 255);
      return Component;
   }

   start = Path;
   do {
      if (*start == '\0') break;
      p = strchr(start, ':');
      if (p != NULL) {
         Save = (*p);
         *p = '\0';
      }
      strmaxcpy(Component, start, 255);
      strmaxcat(Component, SPATHSEP, 255);
      strmaxcat(Component, File, 255);
      if (p != NULL) *p = Save;
      Dummy = fopen(Component, "r");
      OK = (Dummy != NULL);
      if (OK) {
         fclose(Dummy);
         return Component;
      }
      start = p + 1;
   }
   while (p != NULL);

   *Component = '\0';
   return Component;
}

long FileSize(FILE * file) {
   long Save = ftell(file), Size;

   fseek(file, 0, SEEK_END);
   Size = ftell(file);
   fseek(file, Save, SEEK_SET);
   return Size;
}

Byte Lo(Word inp) {
   return (inp & 0xff);
}

Byte Hi(Word inp) {
   return ((inp >> 8) & 0xff);
}

bool Odd(int inp) {
   return ((inp & 1) == 1);
}

void bpemu_init(void) {
}
