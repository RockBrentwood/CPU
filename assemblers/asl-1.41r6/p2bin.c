/* p2bin.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Umwandlung von AS-Codefiles in Binaerfiles                                */
/*                                                                           */
/* Historie:  3. 6.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "endian.h"
#include "bpemu.h"
#include "stringutil.h"
#include "hex.h"
#include "nls.h"
#include "chunks.h"
#include "decodecmd.h"
#include "toolutils.h"

#define BinSuffix ".bin"


typedef void (*ProcessProc)(
#ifdef __PROTOS__
char *FileName, LongWord Offset
#endif
);


static CMDProcessed ParProcessed;
static Integer z;

static FILE *TargFile;
static String SrcName,TargName;

static LongWord StartAdr,StopAdr,RealFileLen;
static LongWord MaxGran,Dummy;
static bool StartAuto,StopAuto;

static Byte FillVal;
static bool DoCheckSum;

static Byte SizeDiv;
static LongInt ANDMask,ANDEq;

static ChunkList UsedList;

#include "ioerrors.rsc"
#include "tools.rsc"
#include "p2bin.rsc"

#ifdef DEBUG
#define ChkIO(s) ChkIO_L(s,__LINE__)
	static void ChkIO_L(char *s, int line)
{
   if (errno!=0)
    {
     fprintf(stderr,"%s %d\n",s,line); exit(3);
    }
}
#endif

        static void ParamError(bool InEnv, char *Arg)
{
   printf("%s%s\n%s\n",(InEnv)?ErrMsgInvEnvParam:ErrMsgInvParam,Arg,ErrMsgProgTerm);
   exit(1);
}

#define BufferSize 4096
static Byte Buffer[BufferSize];

        static void OpenTarget(void)
{
   LongWord Rest,Trans;

   TargFile=fopen(TargName,OPENWRMODE);
   if (TargFile==NULL) ChkIO(TargName);
   RealFileLen=((StopAdr-StartAdr+1)*MaxGran)/SizeDiv;

   memset(Buffer,FillVal,BufferSize);

   Rest=RealFileLen;
   while (Rest!=0)
    {
     Trans=min(Rest,BufferSize);
     if (fwrite(Buffer,1,Trans,TargFile)!=Trans) ChkIO(TargName);
     Rest-=Trans;
    }
}

	static void CloseTarget(void)
{
   LongWord Sum,Rest,Trans,Real,z;

   if (fclose(TargFile)==EOF) ChkIO(TargName);

   if (DoCheckSum)
    {
     TargFile=fopen(TargName,OPENUPMODE); if (TargFile==NULL) ChkIO(TargName);
     Rest=FileSize(TargFile)-1;
     Sum=0;
     while (Rest!=0)
      {
       Trans=min(Rest,BufferSize);
       Rest-=Trans;
       Real=fread(Buffer,1,Trans,TargFile);
       if (Real!=Trans) ChkIO(TargName);
       for (z=0; z<Trans; Sum+=Buffer[z++]);
      }
     errno=0; printf("%s%s\n",InfoMessChecksum,HexLong(Sum));
     Buffer[0]=0x100-(Sum&0xff); fflush(TargFile);
     if (fwrite(Buffer,1,1,TargFile)!=1) ChkIO(TargName); fflush(TargFile);
     if (fclose(TargFile)==EOF) ChkIO(TargName);
    }

   if (Magic!=0) unlink(TargName);
}

        static void ProcessFile(char *FileName, LongWord Offset)
{
   FILE *SrcFile;
   Word TestID;
   Byte InpHeader,InpSegment;
   LongWord InpStart,SumLen;
   Word InpLen,TransLen,ResLen;
   bool doit;
   LongWord ErgStart,ErgStop,NextPos;
   Word ErgLen=0;
   LongInt z;
   Byte Gran;

   SrcFile=fopen(FileName,OPENRDMODE);
   if (SrcFile==NULL) ChkIO(FileName);

   if (! Read2(SrcFile,&TestID)) ChkIO(FileName);
   if (TestID!=FileID) FormatError(FileName,FormatInvHeaderMsg);

   errno=0; printf("%s==>>%s",FileName,TargName); ChkIO(OutName);

   SumLen=0;

   do
    {
     ReadRecordHeader(&InpHeader,&InpSegment,&Gran,FileName,SrcFile);
     if (InpHeader==FileHeaderStartAdr)
      {
       if (! Read4(SrcFile,&ErgStart)) ChkIO(FileName);
      }
     else if (InpHeader!=FileHeaderEnd)
      {
       if (! Read4(SrcFile,&InpStart)) ChkIO(FileName);
       if (! Read2(SrcFile,&InpLen)) ChkIO(FileName);

       NextPos=ftell(SrcFile)+InpLen;
       if (NextPos>=FileSize(SrcFile)-1)
        FormatError(FileName,FormatInvRecordLenMsg);

       doit=(FilterOK(InpHeader) && (InpSegment==SegCode));

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

       if (doit)
        {
 	 /* an Anfang interessierender Daten */

 	 if (fseek(SrcFile,(ErgStart-InpStart)*Gran,SEEK_CUR)==-1) ChkIO(FileName);

 	 /* in Zieldatei an passende Stelle */

 	 if (fseek(TargFile,((ErgStart-StartAdr)*Gran)/SizeDiv,SEEK_SET)==-1) ChkIO(TargName);

 	 /* umkopieren */

 	 while (ErgLen>0)
 	  {
 	   TransLen=min(BufferSize,ErgLen);
 	   if (fread(Buffer,1,TransLen,SrcFile)!=TransLen) ChkIO(FileName);
 	   if (SizeDiv==1) ResLen=TransLen;
 	   else
 	    {
 	     ResLen=0;
 	     for (z=0; z<(LongInt)TransLen; z++)
 	      if (((ErgStart*Gran+z)&ANDMask)==ANDEq)
 	       Buffer[ResLen++]=Buffer[z];
 	    }
 	   if (fwrite(Buffer,1,ResLen,TargFile)!=ResLen) ChkIO(TargName);
 	   ErgLen-=TransLen; ErgStart+=TransLen; SumLen+=ResLen;
 	  }
        }
       if (fseek(SrcFile,NextPos,SEEK_SET)==-1) ChkIO(FileName);
      }
    }
   while (InpHeader!=0);

   errno=0; printf("  (%d Byte)\n",SumLen); ChkIO(OutName);

   if (fclose(SrcFile)==EOF) ChkIO(FileName);
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
   Byte Header,Gran,Segment;
   Word Length,TestID;
   LongWord Adr,EndAdr,NextPos;

   f=fopen(FileName,OPENRDMODE);
   if (f==NULL) ChkIO(FileName);

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

       if (FilterOK(Header) && (Segment==SegCode))
        {
         Adr+=Offset;
 	 EndAdr=Adr+(Length/Gran)-1;
         if (Gran>MaxGran) MaxGran=Gran;
         if (StartAuto) if (StartAdr>Adr) StartAdr=Adr;
 	 if (StopAuto) if (EndAdr>StopAdr) StopAdr=EndAdr;
        }

       fseek(f,NextPos,SEEK_SET);
      }
    }
   while(Header!=0);

   if (fclose(f)==EOF) ChkIO(FileName);
}

	static CMDResult CMD_AdrRange(bool Negate, char *Arg)
{
   char *p,Save;
   bool err;

   if (Arg==NULL); /* satisfy some compilers */

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

	static CMDResult CMD_ByteMode(bool Negate, char *Arg)
{
#define ByteModeCnt 9
   static char *ByteModeStrings[ByteModeCnt]={"ALL","EVEN","ODD","BYTE0","BYTE1","BYTE2","BYTE3","WORD0","WORD1"};
   static Byte ByteModeDivs[ByteModeCnt]={1,2,2,4,4,4,4,2,2};
   static Byte ByteModeMasks[ByteModeCnt]={0,1,1,3,3,3,3,2,2};
   static Byte ByteModeEqs[ByteModeCnt]={0,0,1,0,1,2,3,0,2};

   Integer z;

   if (Negate); /* satisfy some compilers */

   if (*Arg=='\0')
    {
     SizeDiv=1; ANDEq=0; ANDMask=0;
     return CMDOK;
    }
   else
    {
     for (z=0; z<strlen(Arg); z++) Arg[z]=toupper(Arg[z]);
     ANDEq=0xff;
     for (z=0; z<ByteModeCnt; z++)
      if (strcmp(Arg,ByteModeStrings[z])==0)
       {
        SizeDiv=ByteModeDivs[z];
        ANDMask=ByteModeMasks[z];
        ANDEq  =ByteModeEqs[z];
       }
      if (ANDEq==0xff) return CMDErr; else return CMDArg;
    }
}

	static CMDResult CMD_FillVal(bool Negate, char *Arg)
{
   bool err;

   if (Negate); /* satisfy some compilers */

   FillVal=ConstLongInt(Arg,&err);
   if (! err) return CMDErr; else return CMDArg;
}

	static CMDResult CMD_CheckSum(bool Negate, char *Arg)
{
   if (Arg==NULL); /* satisfy some compilers */

   DoCheckSum=! Negate;
   return CMDOK;
}

#define P2BINParamCnt 5
static CMDRec P2BINParams[P2BINParamCnt]=
	       {{"f", CMD_FilterList},
		{"r", CMD_AdrRange},
		{"s", CMD_CheckSum},
		{"m", CMD_ByteMode},
		{"l", CMD_FillVal}};

	int main(int argc, char **argv)
{
   ParamStr=argv; ParamCount=argc-1;
   endian_init();
   stringutil_init();
   bpemu_init();
   hex_init();
   nls_init();
   chunks_init();
   decodecmd_init();
   toolutils_init();

   NLS_Initialize(); WrCopyRight("P2BIN/C V1.41r5");

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

   StartAdr=0; StopAdr=0x7fff; StartAuto=false; StopAuto=false;
   FillVal=0xff; DoCheckSum=false; SizeDiv=1; ANDEq=0;
   ProcessCMD(P2BINParams,P2BINParamCnt,ParProcessed,"P2BINCMD",ParamError);

   if (ProcessedEmpty(ParProcessed))
    {
     errno=0;
     printf("%s\n",ErrMsgTargMissing);
     ChkIO(OutName);
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
   AddSuffix(TargName,BinSuffix);

   MaxGran=1;
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

   if (ProcessedEmpty(ParProcessed)) ProcessGroup(SrcName,ProcessFile);
   else for (z=1; z<=ParamCount; z++)
    if (ParProcessed[z]) ProcessGroup(ParamStr[z],ProcessFile);

   CloseTarget();
   return 0;
}
