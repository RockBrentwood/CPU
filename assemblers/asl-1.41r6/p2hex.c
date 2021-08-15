/* p2hex.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Konvertierung von AS-P-Dateien nach Hex                                   */
/*                                                                           */
/* Historie:  1. 6.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "endian.h"
#include "bpemu.h"
#include "hex.h"
#include "nls.h"
#include "stringutil.h"
#include "chunks.h"
#include "decodecmd.h"

#include "toolutils.h"

static char *HexSuffix=".hex";
#define MaxLineLen 254

typedef enum {Default,MotoS,IntHex,IntHex16,IntHex32,MOSHex,TekHex,TiDSK} OutFormat;
typedef void (*ProcessProc)(
#ifdef __PROTOS__
char *FileName, LongWord Offset
#endif
);

static CMDProcessed ParProcessed;
static Integer z,z2;
static FILE *TargFile;
static String SrcName,TargName;

static LongWord StartAdr,StopAdr,LineLen;
static LongWord StartData,StopData,EntryAdr;
static bool StartAuto,StopAuto,EntryAdrPresent;
static Word Seg,Ofs;
static LongWord Dummy;
static Byte IntelMode;
static Byte MultiMode;   /* 0=8M, 1=16, 2=8L, 3=8H */
static bool Rec5;
static bool SepMoto;

static bool RelAdr,MotoOccured,IntelOccured,MOSOccured,DSKOccured;
static Byte MaxMoto,MaxIntel;

static OutFormat DestFormat;

static ChunkList UsedList;

#include "tools.rsc"
#include "p2hex.rsc"

        static void ParamError(bool InEnv, char *Arg)
{
   printf("%s%s\n",InEnv?ErrMsgInvEnvParam:ErrMsgInvParam,Arg);
   printf("%s\n",ErrMsgProgTerm);
   exit(1);
}

        static void OpenTarget(void)
{
   TargFile=fopen(TargName,"w");
   if (TargFile==NULL) ChkIO(TargName);
}

	static void CloseTarget(void)
{
   errno=0; fclose(TargFile); ChkIO(TargName);
   if (Magic!=0) unlink(TargName);
}

        static void ProcessFile(char *FileName, LongWord Offset)
{
   FILE *SrcFile;
   Word TestID;
   Byte InpHeader,InpSegment,InpGran,BSwap;
   LongInt InpStart,SumLen;
   Word InpLen,TransLen;
   bool doit,FirstBank=0;
   Byte Buffer[MaxLineLen];
   Word *WBuffer=(Word *) Buffer;
   LongWord ErgStart,
#ifdef __STDC__
            ErgStop=0xffffffffu,
#else
            ErgStop=0xffffffff,
#endif
            NextPos,IntOffset=0,MaxAdr;
   Word ErgLen=0,ChkSum=0,RecCnt,Gran,SwapBase,HSeg;

   LongInt z;

   Byte MotRecType=0;

   OutFormat ActFormat;

   SrcFile=fopen(FileName,OPENRDMODE);
   if (SrcFile==NULL) ChkIO(FileName);

   if (! Read2(SrcFile,&TestID)) ChkIO(FileName);
   if (TestID!=FileMagic) FormatError(FileName,FormatInvHeaderMsg);

   errno=0; printf("%s==>>%s",FileName,TargName); ChkIO(OutName);

   SumLen=0;

   do
    {
     ReadRecordHeader(&InpHeader,&InpSegment,&InpGran,FileName,SrcFile);
     if (InpHeader==FileHeaderStartAdr)
      {
       if (! Read4(SrcFile,&ErgStart)) ChkIO(FileName);
       if (! EntryAdrPresent)
        {
         EntryAdr=ErgStart; EntryAdrPresent=true;
        }
      }
     else if (InpHeader!=FileHeaderEnd)
      {
       Gran=InpGran;

       if ((ActFormat=DestFormat)==Default)
        switch (InpHeader)
         {
          case 0x01: case 0x05: case 0x09: case 0x52: case 0x56: case 0x61:
          case 0x62: case 0x63: case 0x64: case 0x65: case 0x68: case 0x69:
          case 0x6c:
           ActFormat=MotoS; break;
          case 0x12: case 0x21: case 0x31: case 0x32: case 0x33: case 0x39: case 0x3a: case 0x41:
          case 0x48: case 0x49: case 0x51: case 0x53: case 0x54: case 0x55: case 0x6e: case 0x70:
          case 0x71: case 0x72: case 0x73: case 0x78: case 0x79: case 0x7a: case 0x7b:
           ActFormat=IntHex; break;
          case 0x42: case 0x4c:
           ActFormat=IntHex16; break;
          case 0x13: case 0x29: case 0x76:
           ActFormat=IntHex32; break;
          case 0x11: case 0x19:
           ActFormat=MOSHex; break;
          case 0x74: case 0x75: case 0x77:
           ActFormat=TiDSK; break;
          default:
           FormatError(FileName,FormatInvRecordHeaderMsg);
         }

       switch (ActFormat)
        {
         case MotoS:
         case IntHex32:
#ifdef __STDC__
          MaxAdr=0xffffffffu; break;
#else
          MaxAdr=0xffffffff; break;
#endif
         case IntHex16:
          MaxAdr=0xffff0+0xffff; break;
         default:
          MaxAdr=0xffff;
        }

       if (! Read4(SrcFile,&InpStart)) ChkIO(FileName);
       if (! Read2(SrcFile,&InpLen)) ChkIO(FileName);

       NextPos=ftell(SrcFile)+InpLen;
       if (NextPos>=FileSize(SrcFile)-1)
        FormatError(FileName,FormatInvRecordLenMsg);

       doit=(FilterOK(InpHeader)) && (InpSegment==SegCode);

       if (doit)
        {
         InpStart+=Offset;
 	 ErgStart=max(StartAdr,InpStart);
 	 ErgStop=min(StopAdr,InpStart+(InpLen/Gran)-1);
 	 doit=(ErgStop>=ErgStart);
 	 if (doit)
 	  {
 	   ErgLen=(ErgStop+1-ErgStart)*Gran;
           if (AddChunk(&UsedList,ErgStart,ErgStop-ErgStart+1,true))
            {
             errno=0; printf(" %s\n",ErrMsgOverlap); ChkIO(OutName);
            }
          }
        }

       if (ErgStop>MaxAdr)
        {
         errno=0; printf(" %s\n",ErrMsgAdrOverflow); ChkIO(OutName);
        }

       if (doit)
        {
 	 /* an Anfang interessierender Daten */

 	 if (fseek(SrcFile,(ErgStart-InpStart)*Gran,SEEK_CUR)==-1) ChkIO(FileName);

 	 /* Statistik, Anzahl Datenzeilen ausrechnen */

         RecCnt=ErgLen/LineLen;
         if ((ErgLen%LineLen)!=0) RecCnt++;

 	 /* relative Angaben ? */

 	 if (RelAdr) ErgStart-=StartAdr;

 	 /* Kopf einer Datenzeilengruppe */

  	 switch (ActFormat)
          {
   	   case MotoS:
  	    if ((! MotoOccured) || (SepMoto))
  	     {
  	      errno=0; fprintf(TargFile,"S0030000FC\n"); ChkIO(TargName);
  	     }
  	    if ((ErgStop>>24)!=0) MotRecType=2;
  	    else if ((ErgStop>>16)!=0) MotRecType=1;
  	    else MotRecType=0;
            if (MaxMoto<MotRecType) MaxMoto=MotRecType;
  	    if (Rec5)
  	     {
  	      ChkSum=Lo(RecCnt)+Hi(RecCnt)+3;
              errno=0;
  	      fprintf(TargFile,"S503%s%s\n",HexWord(RecCnt),HexByte(Lo(ChkSum^0xff)));
  	      ChkIO(TargName);
  	     }
  	    MotoOccured=true;
  	    break;
  	   case MOSHex:
            MOSOccured=true;
            break;
  	   case IntHex:
  	    IntelOccured=true;
  	    IntOffset=0;
  	    break;
  	   case IntHex16:
  	    IntelOccured=true;
#ifdef __STDC__
  	    IntOffset=ErgStart&0xfffffff0u;
#else
            IntOffset=ErgStart&0xfffffff0;
#endif
  	    HSeg=IntOffset>>4; ChkSum=4+Lo(HSeg)+Hi(HSeg);
            errno=0;
  	    fprintf(TargFile,":02000002%s%s\n",HexWord(HSeg),HexByte(0x100-ChkSum));
            if (MaxIntel<1) MaxIntel=1;
  	    ChkIO(TargName);
  	    break;
           case IntHex32:
  	    IntelOccured=true;
#ifdef __STDC__
            IntOffset=ErgStart&0xffff0000u;
#else
            IntOffset=ErgStart&0xffff0000;
#endif
            HSeg=IntOffset>>16; ChkSum=6+Lo(HSeg)+Hi(HSeg);
            fprintf(TargFile,":02000004%s%s\n",HexWord(HSeg),HexByte(0x100-ChkSum));
            if (MaxIntel<2) MaxIntel=2;
  	    ChkIO(TargName);
            FirstBank=false;
            break;
           case TekHex:
            break;
  	   case TiDSK:
  	    if (! DSKOccured)
  	     {
  	      DSKOccured=true;
              errno=0; fprintf(TargFile,"%s%s\n",DSKHeaderLine,TargName); ChkIO(TargName);
  	     }
            break;
           default:
            break;
 	  }

 	 /* Datenzeilen selber */

 	 while (ErgLen>0)
 	  {
           /* evtl. Folgebank fuer Intel32 ausgeben */

           if ((ActFormat==IntHex32) && (FirstBank))
            {
             IntOffset+=0x10000;
             HSeg=IntOffset>>16; ChkSum=6+Lo(HSeg)+Hi(HSeg);
             errno=0;
             fprintf(TargFile,":02000004%s%s\n",HexWord(HSeg),HexByte(0x100-ChkSum));
             ChkIO(TargName);
             FirstBank=false;
            }

           /* Recordlaenge ausrechnen, fuer Intel32 auf 64K-Grenze begrenzen */

           TransLen=min(LineLen,ErgLen);
           if ((ActFormat==IntHex32) && ((ErgStart&0xffff)+(TransLen/Gran)>=0x10000))
            {
             TransLen=Gran*(0x10000-(ErgStart&0xffff));
             FirstBank=true;
            }

 	   /* Start der Datenzeile */

 	   switch (ActFormat)
            {
 	     case MotoS:
              errno=0;
              fprintf(TargFile,"S%c%s",'1'+MotRecType,HexByte(TransLen+3+MotRecType));
 	      ChkIO(TargName);
 	      ChkSum=TransLen+3+MotRecType;
 	      if (MotRecType>=2)
 	       {
 	        errno=0; fprintf(TargFile,"%s",HexByte((ErgStart>>24)&0xff)); ChkIO(TargName);
 	        ChkSum+=((ErgStart>>24)&0xff);
 	       }
 	      if (MotRecType>=1)
 	       {
 	        errno=0; fprintf(TargFile,"%s",HexByte((ErgStart>>16)&0xff)); ChkIO(TargName);
 	        ChkSum+=((ErgStart>>16)&0xff);
 	       }
 	      errno=0; fprintf(TargFile,"%s",HexWord(ErgStart&0xffff)); ChkIO(TargName);
 	      ChkSum+=Hi(ErgStart)+Lo(ErgStart);
 	      break;
 	     case MOSHex:
 	      errno=0; fprintf(TargFile,";%s%s",HexByte(TransLen),HexWord(ErgStart && 0xffff)); ChkIO(TargName);
 	      ChkSum+=TransLen+Lo(ErgStart)+Hi(ErgStart);
 	      break;
             case IntHex:
             case IntHex16:
             case IntHex32:
 	      errno=0; fprintf(TargFile,":"); ChkIO(TargName); ChkSum=0;
 	      if (MultiMode==0)
 	       {
 	        errno=0; fprintf(TargFile,"%s",HexByte(TransLen)); ChkIO(TargName);
 	        errno=0; fprintf(TargFile,"%s",HexWord((ErgStart-IntOffset)*Gran)); ChkIO(TargName);
 	        ChkSum+=TransLen;
 	        ChkSum+=Lo((ErgStart-IntOffset)*Gran);
 	        ChkSum+=Hi((ErgStart-IntOffset)*Gran);
 	       }
 	      else
 	       {
 	        errno=0; fprintf(TargFile,"%s",HexByte(TransLen/Gran)); ChkIO(TargName);
 	        errno=0; fprintf(TargFile,"%s",HexWord(ErgStart-IntOffset)); ChkIO(TargName);
 	        ChkSum+=TransLen/Gran;
 	        ChkSum+=Lo(ErgStart-IntOffset);
 	        ChkSum+=Hi(ErgStart-IntOffset);
 	       }
 	      errno=0; fprintf(TargFile,"00"); ChkIO(TargName);
 	      break;
 	     case TekHex:
 	      errno=0;
              fprintf(TargFile,"/%s%s%s",HexWord(ErgStart),HexByte(TransLen),
 		                         HexByte(Lo(ErgStart)+Hi(ErgStart)+TransLen));
 	      ChkIO(TargName);
 	      ChkSum=0;
 	      break;
 	     case TiDSK:
              errno=0; fprintf(TargFile,"9%s",HexWord(/*Gran**/ErgStart));
 	      ChkIO(TargName);
 	      ChkSum=0;
 	      break;
             default:
              break;
 	    }

 	   /* Daten selber */

 	   if (fread(Buffer,1,TransLen,SrcFile)!=TransLen) ChkIO(FileName);
 	   if ((Gran!=1) && (MultiMode==1))
 	    for (z=0; z<(TransLen/Gran); z++)
 	     {
 	      SwapBase=z*Gran;
 	      for (z2=0; z2<(Gran/2); z++)
 	       {
 	        BSwap=Buffer[SwapBase+z2];
 	        Buffer[SwapBase+z2]=Buffer[SwapBase+Gran-1-z2];
 	        Buffer[SwapBase+Gran-1-z2]=BSwap;
 	       }
 	     }
 	   if (ActFormat==TiDSK)
            {
             if (BigEndian) WSwap(WBuffer,TransLen);
 	     for (z=0; z<(TransLen/2); z++)
 	      {
               errno=0;
 	       if ((ErgStart+z >= StartData) && (ErgStart+z <= StopData))
 	        fprintf(TargFile,"M%s",HexWord(WBuffer[z]));
 	       else
 	        fprintf(TargFile,"B%s",HexWord(WBuffer[z]));
 	       ChkIO(TargName);
 	       ChkSum+=WBuffer[z];
 	       SumLen+=Gran;
 	      }
            }
 	   else
 	    for (z=0; z<(LongInt)TransLen; z++)
 	     if ((MultiMode<2) || (z%Gran==MultiMode-2))
 	      {
 	       errno=0; fprintf(TargFile,"%s",HexByte(Buffer[z])); ChkIO(TargName);
 	       ChkSum+=Buffer[z];
 	       SumLen++;
 	      }

 	   /* Ende Datenzeile */

 	   switch (ActFormat)
            {
 	     case MotoS:
 	      errno=0;
 	      fprintf(TargFile,"%s\n",HexByte(Lo(ChkSum^0xff)));
 	      ChkIO(TargName);
 	      break;
 	     case MOSHex:
 	      errno=0;
              fprintf(TargFile,"%s\n",HexWord(ChkSum));
              break;
             case IntHex:
             case IntHex16:
             case IntHex32:
              errno=0;
 	      fprintf(TargFile,"%s\n",HexByte(Lo(1+(ChkSum^0xff))));
 	      ChkIO(TargName);
 	      break;
 	     case TekHex:
 	      errno=0;
              fprintf(TargFile,"%s\n",HexByte(Lo(ChkSum)));
 	      ChkIO(TargName);
 	      break;
 	     case TiDSK:
 	      errno=0;
              fprintf(TargFile,"7%sF\n",HexWord(ChkSum));
 	      ChkIO(TargName);
 	      break;
             default:
              break;
 	    }

 	   /* Zaehler rauf */

 	   ErgLen-=TransLen;
 	   ErgStart+=TransLen/Gran;
 	  }

         /* Ende der Datenzeilengruppe */

         switch (ActFormat)
          {
           case MotoS:
            if (SepMoto)
             {
              errno=0;
 	      fprintf(TargFile,"S%c%s",'9'-MotRecType,HexByte(3+MotRecType));
              ChkIO(TargName);
              for (z=1; z<=2+MotRecType; z++)
               {
 	        errno=0; fprintf(TargFile,"%s",HexByte(0)); ChkIO(TargName);
               }
              errno=0;
              fprintf(TargFile,"%s\n",HexByte(0xff-3-MotRecType));
              ChkIO(TargName);
             }
            break;
           case MOSHex:
            break;
           case IntHex:
           case IntHex16:
           case IntHex32:
            break;
           case TekHex:
            break;
           case TiDSK:
            break;
           default:
            break;
          };
        }
       if (fseek(SrcFile,NextPos,SEEK_SET)==-1) ChkIO(FileName);
      }
    }
   while (InpHeader!=0);

   errno=0; printf("  (%d Byte)\n",SumLen); ChkIO(OutName);

   errno=0; fclose(SrcFile); ChkIO(FileName);
}

	static void ProcessGroup(char *GroupName_O, ProcessProc Processor)
{
/**   s:SearchRec;**/
   String /**Path,Name,**/Ext,GroupName;
   LongWord Offset;

   strmaxcpy(GroupName,GroupName_O,255); strmaxcpy(Ext,GroupName,255);
   if (! RemoveOffset(GroupName,&Offset)) ParamError(false,Ext);
   AddSuffix(GroupName,Suffix);

   Processor(GroupName,Offset);
/**   FSplit(GroupName,Path,Name,Ext);

   FindFirst(GroupName,Archive,s);
   IF DosError<>0 THEN
    WriteLn(ErrMsgNullMaskA,GroupName,ErrMsgNullMaskB)
   ELSE
    WHILE DosError=0 DO
     {
      Processor(Path+s.Name,Offset);
      FindNext(s);
     };**/
}

        static void MeasureFile(char *FileName, LongWord Offset)
{
   FILE *f;
   Byte Header,Segment,Gran;
   Word Length,TestID;
   LongWord Adr,EndAdr,NextPos;

   f=fopen(FileName,OPENRDMODE); if (f==NULL) ChkIO(FileName);

   if (! Read2(f,&TestID)) ChkIO(FileName);
   if (TestID!=FileMagic) FormatError(FileName,FormatInvHeaderMsg);

   do
    {
     ReadRecordHeader(&Header,&Segment,&Gran,FileName,f);

     if (Header==FileHeaderStartAdr)
      {
       if (fseek(f,sizeof(LongWord),SEEK_CUR)==-1) ChkIO(FileName);
      }
     else if (Header!=FileHeaderEnd)
      {
       if (! Read4(f,&Adr)) ChkIO(FileName);
       if (! Read2(f,&Length)) ChkIO(FileName);
       NextPos=ftell(f)+Length;
       if (NextPos>FileSize(f))
        FormatError(FileName,FormatInvRecordLenMsg);

       if (FilterOK(Header))
        {
         Adr+=Offset;
 	 EndAdr=Adr+(Length/Gran)-1;
         if (StartAuto) if (StartAdr>Adr) StartAdr=Adr;
 	 if (StopAuto) if (EndAdr>StopAdr) StopAdr=EndAdr;
        }

       fseek(f,NextPos,SEEK_SET);
      }
    }
   while(Header!=0);

   fclose(f);
}

	static CMDResult CMD_AdrRange(bool Negate, char *Arg)
{
   char *p,Save;
   bool err;

   if (Negate)
    {
     StartAdr=0; StopAdr=0x7fff;
     return CMDOK;
    }
   else
    {
     p=strchr(Arg,'-'); if (p==NULL) return CMDErr;

     Save=(*p); *p='\0';
     if ((StartAuto=(strcmp(Arg,"$")==0))) err=true;
     else StartAdr=ConstLongInt(Arg,&err);
     *p=Save;
     if (! err) return CMDErr;

     if ((StopAuto=(strcmp(p+1,"$")==0))) err=true;
     else StopAdr=ConstLongInt(p+1,&err);
     if (! err) return CMDErr;

     if ((! StartAuto) && (! StopAuto) && (StartAdr>StopAdr)) return CMDErr;

     return CMDArg;
    }
}

	static CMDResult CMD_RelAdr(bool Negate, char *Arg)
{
   if (Arg==NULL); /* satisfy some compilers */

   RelAdr=(! Negate);
   return CMDOK;
}

        static CMDResult CMD_Rec5(bool Negate, char *Arg)
{
   if (Arg==NULL); /* satisfy some compilers */

   Rec5=(! Negate);
   return CMDOK;
}

        static CMDResult CMD_SepMoto(bool Negate, char *Arg)
{
   if (Arg==NULL); /* satisfy some compilers */

   SepMoto=(! Negate);
   return CMDOK;
}

        static CMDResult CMD_IntelMode(bool Negate, char *Arg)
{
   Integer Mode;
   bool err;

   if (*Arg=='\0') return CMDErr;
   else
    {
     Mode=ConstLongInt(Arg,&err);
     if ((! err) || (Mode<0) || (Mode>2)) return CMDErr;
     else
      {
       if (! Negate) IntelMode=Mode;
       else if (IntelMode==Mode) IntelMode=0;
       return CMDArg;
      }
    }
}

	static CMDResult CMD_MultiMode(bool Negate, char *Arg)
{
   Integer Mode;
   bool err;

   if (*Arg=='\0') return CMDErr;
   else
    {
     Mode=ConstLongInt(Arg,&err);
     if ((! err) || (Mode<0) || (Mode>3)) return CMDErr;
     else
      {
       if (! Negate) MultiMode=Mode;
       else if (MultiMode==Mode) MultiMode=0;
       return CMDArg;
      }
    }
}

        static CMDResult CMD_DestFormat(bool Negate, char *Arg)
{
#define NameCnt 8

   static char *Names[NameCnt]={"DEFAULT","MOTO","INTEL","INTEL16","INTEL32","MOS","TEK","DSK"};
   static OutFormat Format[NameCnt]={Default,MotoS,IntHex,IntHex16,IntHex32,MOSHex,TekHex,TiDSK};
   Integer z;

   for (z=0; z<strlen(Arg); z++) Arg[z]=toupper(Arg[z]);

   z=0;
   while ((z<NameCnt) && (strcmp(Arg,Names[z])!=0)) z++;
   if (z>=NameCnt) return CMDErr;

   if (! Negate) DestFormat=Format[z];
   else if (DestFormat==Format[z]) DestFormat=Default;

   return CMDArg;
}

	static CMDResult CMD_DataAdrRange(bool Negate, char *Arg)
{
   char *p,Save;
   bool err;

   if (Negate)
    {
     StartData=0; StopData=0x1fff;
     return CMDOK;
    }
   else
    {
     p=strchr(Arg,'-'); if (p==NULL) return CMDErr;

     Save=(*p); *p='\0';
     StartData=ConstLongInt(Arg,&err);
     *p=Save;
     if (! err) return CMDErr;

     StopData=ConstLongInt(p+1,&err);
     if (! err) return CMDErr;

     if (StartData>StopData) return CMDErr;

     return CMDArg;
    }
}

	static CMDResult CMD_EntryAdr(bool Negate, char *Arg)
{
   bool err;

   if (Negate)
    {
     EntryAdrPresent=false;
     return CMDOK;
    }
   else
    {
     EntryAdr=ConstLongInt(Arg,&err);
     if ((! err) || (EntryAdr>0xffff)) return CMDErr;
     return CMDArg;
    }
}

        static CMDResult CMD_LineLen(bool Negate, char *Arg)
{
   bool err;

   if (Negate)
    if (*Arg!='\0') return CMDErr;
    else
     {
      LineLen=16; return CMDOK;
     }
   else if (*Arg=='\0') return CMDErr;
   else
    {
     LineLen=ConstLongInt(Arg,&err);
     if ((err!=0) || (LineLen<1) || (LineLen>MaxLineLen)) return CMDErr;
     else
      {
       LineLen+=(LineLen&1); return CMDArg;
      }
    }
}

#define P2HEXParamCnt 11
static CMDRec P2HEXParams[P2HEXParamCnt]=
	       {{"f", CMD_FilterList},
		{"r", CMD_AdrRange},
		{"a", CMD_RelAdr},
		{"i", CMD_IntelMode},
		{"m", CMD_MultiMode},
		{"F", CMD_DestFormat},
		{"5", CMD_Rec5},
		{"s", CMD_SepMoto},
		{"d", CMD_DataAdrRange},
                {"e", CMD_EntryAdr},
                {"l", CMD_LineLen}};

static Word ChkSum;

	int main(int argc, char **argv)
{
   ParamCount=argc-1; ParamStr=argv;
   hex_init();
   endian_init();
   bpemu_init();
   hex_init();
   nls_init();
   chunks_init();
   decodecmd_init();
   toolutils_init();

   NLS_Initialize(); WrCopyRight("P2HEX/C V1.41r5");

   InitChunk(&UsedList);

   if (ParamCount==0)
    {
     errno=0; printf("%s%s%s\n",InfoMessHead1,GetEXEName(),InfoMessHead2); ChkIO(OutName);
     for (z=0; z<InfoMessHelpCnt; z++)
      {
       errno=0; printf("%s\n",InfoMessHelp[z]); ChkIO(OutName);
      }
     exit(1);
    }

   StartAdr=0; StopAdr=0x7fff;
   StartAuto=false; StopAuto=false;
   StartData=0; StopData=0x1fff;
   EntryAdr=(-1); EntryAdrPresent=false;
   RelAdr=false; Rec5=true; LineLen=16;
   IntelMode=0; MultiMode=0; DestFormat=Default;
   *TargName='\0';
   ProcessCMD(P2HEXParams,P2HEXParamCnt,ParProcessed,"P2HEXCMD",ParamError);

   if (ProcessedEmpty(ParProcessed))
    {
     errno=0; printf("%s\n",ErrMsgTargMissing); ChkIO(OutName);
     exit(1);
    }

   z=ParamCount;
   while ((z>0) && (! ParProcessed[z])) z--;
   strmaxcpy(TargName,ParamStr[z],255);
   if (! RemoveOffset(TargName,&Dummy)) ParamError(false,ParamStr[z]);
   ParProcessed[z]=false;
   if (ProcessedEmpty(ParProcessed))
    {
     strmaxcpy(SrcName,ParamStr[z],255); DelSuffix(TargName);
    }
   AddSuffix(TargName,HexSuffix);

   if ((StartAuto) || (StopAuto))
    {
#ifdef __STDC__
     if (StartAuto) StartAdr=0xffffffffu;
#else
     if (StartAuto) StartAdr=0xffffffff;
#endif
     if (StopAuto) StopAdr=0;
     if (ProcessedEmpty(ParProcessed)) ProcessGroup(SrcName,MeasureFile);
     else for (z=1; z<=ParamCount; z++)
      if (ParProcessed[z]) ProcessGroup(ParamStr[z],MeasureFile);
     if (StartAdr>StopAdr)
      {
       errno=0; printf("%s\n",ErrMsgAutoFailed); ChkIO(OutName); exit(1);
      }
    }

   OpenTarget();
   MotoOccured=false; IntelOccured=false;
   MOSOccured=false;  DSKOccured=false;
   MaxMoto=0; MaxIntel=0;

   if (ProcessedEmpty(ParProcessed)) ProcessGroup(SrcName,ProcessFile);
   else for (z=1; z<=ParamCount; z++)
    if (ParProcessed[z]) ProcessGroup(ParamStr[z],ProcessFile);

   if ((MotoOccured) && (! SepMoto))
    {
     errno=0; fprintf(TargFile,"S%c%s",'9'-MaxMoto,HexByte(3+MaxMoto)); ChkIO(TargName);
     ChkSum=3+MaxMoto;
     if (! EntryAdrPresent) EntryAdr=0;
     if (MaxMoto>=2)
      {
       errno=0; fprintf(TargFile,HexByte((EntryAdr>>24)&0xff)); ChkIO(TargName);
       ChkSum+=(EntryAdr>>24)&0xff;
      }
     if (MaxMoto>=1)
      {
       errno=0; fprintf(TargFile,HexByte((EntryAdr>>16)&0xff)); ChkIO(TargName);
       ChkSum+=(EntryAdr>>16)&0xff;
      }
     errno=0; fprintf(TargFile,"%s",HexWord(EntryAdr&0xffff)); ChkIO(TargName);
     ChkSum+=(EntryAdr>>8)&0xff;
     ChkSum+=EntryAdr&0xff;
     errno=0; fprintf(TargFile,"%s\n",HexByte(0xff-(ChkSum&0xff))); ChkIO(TargName);
    }

   if (IntelOccured)
    {
     if (EntryAdrPresent)
      {
       if (MaxIntel==2)
        {
         errno=0; fprintf(TargFile,":04000003"); ChkIO(TargName); ChkSum=4+3;
         errno=0; fprintf(TargFile,"%s",HexLong(EntryAdr)); ChkIO(TargName);
         ChkSum+=((EntryAdr>>24)&0xff)+
                 ((EntryAdr>>16)&0xff)+
                 ((EntryAdr>>8) &0xff)+
                 ( EntryAdr     &0xff);
        }
       else if (MaxIntel==1)
        {
         errno=0; fprintf(TargFile,":04000003"); ChkIO(TargName); ChkSum=4+3;
         Seg=(EntryAdr>>4)&0xffff;
         Ofs=EntryAdr&0x000f;
         errno=0; fprintf(TargFile,"%s%s",HexWord(Seg),HexWord(Ofs));
         ChkIO(TargName);
         ChkSum+=Lo(Seg)+Hi(Seg)+Ofs;
        }
       else
        {
         errno=0; fprintf(TargFile,":02000003%s",HexWord(EntryAdr&0xffff));
         ChkIO(TargName); ChkSum=2+3+Lo(EntryAdr)+Hi(EntryAdr);
        }
       errno=0; fprintf(TargFile,"%s\n",HexByte(0x100-ChkSum)); ChkIO(TargName);
      }
     errno=0;
     switch (IntelMode)
      {
       case 0: fprintf(TargFile,":00000001FF\n"); break;
       case 1: fprintf(TargFile,":00000001\n"); break;
       case 2: fprintf(TargFile,":0000000000\n"); break;
      }
     ChkIO(TargName);
    }

   if (MOSOccured)
    {
     errno=0; fprintf(TargFile,";0000040004\n"); ChkIO(TargName);
    }

   if (DSKOccured)
    {
     if (EntryAdrPresent)
      {
       errno=0;
       fprintf(TargFile,"1%s7%sF\n",HexWord(EntryAdr),HexWord(EntryAdr));
       ChkIO(TargName);
      }
     errno=0; fprintf(TargFile,":\n"); ChkIO(TargName);
    }

   CloseTarget();
   return 0;
}
