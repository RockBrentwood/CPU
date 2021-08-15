/* code96.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator MCS/96-Familie                                              */
/*                                                                           */
/* Historie: 10.11.1996                                                      */
/*           16. 3.1997 80196N/80296                                         */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct
         {
          char *Name;
          Byte Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Byte Code;
          CPUVar MinCPU,MaxCPU;
         } BaseOrder;

typedef struct
         {
          char *Name;
          Byte Code;
          bool Reloc;
         } MacOrder;

#define FixedOrderCnt 16

#define ALU3OrderCnt 5

#define ALU2OrderCnt 9

#define ALU1OrderCnt 6

#define ShiftOrderCnt 3

#define RelOrderCnt 16

#define MacOrderCnt 8

#define RptOrderCnt 34


static char *ShiftOrders[ShiftOrderCnt]={"SHR","SHL","SHRA"};

#define ModNone (-1)
#define ModDir 0
#define MModDir (1 << ModDir)
#define ModMem 1
#define MModMem (1 << ModMem)
#define ModImm 2
#define MModImm (1 << ModImm)

#define SFRStart 2
#define SFRStop 0x17

static BaseOrder *FixedOrders;
static FixedOrder *ALU3Orders;
static FixedOrder *ALU2Orders;
static FixedOrder *ALU1Orders;
static FixedOrder *RelOrders;
static MacOrder *MacOrders;
static FixedOrder *RptOrders;

static CPUVar CPU8096,CPU80196,CPU80196N,CPU80296;
static SimpProc SaveInitProc;

static Byte AdrMode;
static ShortInt AdrType;
static Byte AdrVals[4];
static ShortInt OpSize;

static LongInt WSRVal,WSR1Val;
static Word WinStart,WinStop,WinEnd,WinBegin;
static Word Win1Start,Win1Stop,Win1Begin,Win1End;

IntType MemInt;

/*---------------------------------------------------------------------------*/


   	static void AddFixed(char *NName, Byte NCode, CPUVar NMin, CPUVar NMax)
{
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].Code=NCode;
   FixedOrders[InstrZ].MinCPU=NMin;
   FixedOrders[InstrZ++].MaxCPU=NMax;
}

        static void AddALU3(char *NName, Byte NCode)
{
   if (InstrZ>=ALU3OrderCnt) exit(255);
   ALU3Orders[InstrZ].Name=NName;
   ALU3Orders[InstrZ++].Code=NCode;
}

        static void AddALU2(char *NName, Byte NCode)
{
   if (InstrZ>=ALU2OrderCnt) exit(255);
   ALU2Orders[InstrZ].Name=NName;
   ALU2Orders[InstrZ++].Code=NCode;
}

        static void AddALU1(char *NName, Byte NCode)
{
   if (InstrZ>=ALU1OrderCnt) exit(255);
   ALU1Orders[InstrZ].Name=NName;
   ALU1Orders[InstrZ++].Code=NCode;
}

        static void AddRel(char *NName, Byte NCode)
{
   if (InstrZ>=RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ++].Code=NCode;
}

        static void AddMac(char *NName, Byte NCode, bool NRel)
{
   if (InstrZ>=MacOrderCnt) exit(255);
   MacOrders[InstrZ].Name=NName;
   MacOrders[InstrZ].Code=NCode;
   MacOrders[InstrZ++].Reloc=NRel;
}

        static void AddRpt(char *NName, Byte NCode)
{
   if (InstrZ>=RptOrderCnt) exit(255);
   RptOrders[InstrZ].Name=NName;
   RptOrders[InstrZ++].Code=NCode;
}

   	static void InitFields(void)
{
   FixedOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("CLRC" ,0xf8,CPU8096  ,CPU80296 );
   AddFixed("CLRVT",0xfc,CPU8096  ,CPU80296 );
   AddFixed("DI"   ,0xfa,CPU8096  ,CPU80296 );
   AddFixed("DPTS" ,0xea,CPU80196 ,CPU80196N);
   AddFixed("EI"   ,0xfb,CPU8096  ,CPU80296 );
   AddFixed("EPTS" ,0xeb,CPU80196 ,CPU80196N);
   AddFixed("NOP"  ,0xfd,CPU8096  ,CPU80296 );
   AddFixed("POPA" ,0xf5,CPU80196 ,CPU80296 );
   AddFixed("POPF" ,0xf3,CPU8096  ,CPU80296 );
   AddFixed("PUSHA",0xf4,CPU80196 ,CPU80296 );
   AddFixed("PUSHF",0xf2,CPU8096  ,CPU80296 );
   AddFixed("RET"  ,0xf0,CPU8096  ,CPU80296 );
   AddFixed("RSC"  ,0xff,CPU8096  ,CPU80296 );
   AddFixed("SETC" ,0xf9,CPU8096  ,CPU80296 );
   AddFixed("TRAP" ,0xf7,CPU8096  ,CPU80296 );
   AddFixed("RETI" ,0xe5,CPU80196N,CPU80296 );

   ALU3Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*ALU3OrderCnt); InstrZ=0;
   AddALU3("ADD" , 0x01);
   AddALU3("AND" , 0x00);
   AddALU3("MUL" , 0x83);   /* ** */
   AddALU3("MULU", 0x03);
   AddALU3("SUB" , 0x02);

   ALU2Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*ALU2OrderCnt); InstrZ=0;
   AddALU2("ADDC", 0xa4);
   AddALU2("CMP" , 0x88);
   AddALU2("DIV" , 0x8c);   /* ** */
   AddALU2("DIVU", 0x8c);
   AddALU2("LD"  , 0xa0);
   AddALU2("OR"  , 0x80);
   AddALU2("ST"  , 0xc0);
   AddALU2("SUBC", 0xa8);
   AddALU2("XOR" , 0x84);

   ALU1Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*ALU1OrderCnt); InstrZ=0;
   AddALU1("CLR", 0x01);
   AddALU1("DEC", 0x05);
   AddALU1("EXT", 0x06);
   AddALU1("INC", 0x07);
   AddALU1("NEG", 0x03);
   AddALU1("NOT", 0x02);

   RelOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RelOrderCnt); InstrZ=0;
   AddRel("JC"   , 0xdb);
   AddRel("JE"   , 0xdf);
   AddRel("JGE"  , 0xd6);
   AddRel("JGT"  , 0xd2);
   AddRel("JH"   , 0xd9);
   AddRel("JLE"  , 0xda);
   AddRel("JLT"  , 0xde);
   AddRel("JNC"  , 0xd3);
   AddRel("JNE"  , 0xd7);
   AddRel("JNH"  , 0xd1);
   AddRel("JNST" , 0xd0);
   AddRel("JNV"  , 0xd5);
   AddRel("JNVT" , 0xd4);
   AddRel("JST"  , 0xd8);
   AddRel("JV"   , 0xdd);
   AddRel("JVT"  , 0xdc);

   MacOrders=(MacOrder *) malloc(sizeof(MacOrder)*MacOrderCnt); InstrZ=0;
   AddMac("MAC"   ,0x00,false); AddMac("SMAC"  ,0x01,false);
   AddMac("MACR"  ,0x04,true ); AddMac("SMACR" ,0x05,true );
   AddMac("MACZ"  ,0x08,false); AddMac("SMACZ" ,0x09,false);
   AddMac("MACRZ" ,0x0c,true ); AddMac("SMACRZ",0x0d,true );

   RptOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RptOrderCnt); InstrZ=0;
   AddRpt("RPT"    ,0x00); AddRpt("RPTNST" ,0x10); AddRpt("RPTNH"  ,0x11);
   AddRpt("RPTGT"  ,0x12); AddRpt("RPTNC"  ,0x13); AddRpt("RPTNVT" ,0x14);
   AddRpt("RPTNV"  ,0x15); AddRpt("RPTGE"  ,0x16); AddRpt("RPTNE"  ,0x17);
   AddRpt("RPTST"  ,0x18); AddRpt("RPTH"   ,0x19); AddRpt("RPTLE"  ,0x1a);
   AddRpt("RPTC"   ,0x1b); AddRpt("RPTVT"  ,0x1c); AddRpt("RPTV"   ,0x1d);
   AddRpt("RPTLT"  ,0x1e); AddRpt("RPTE"   ,0x1f); AddRpt("RPTI"   ,0x20);
   AddRpt("RPTINST",0x30); AddRpt("RPTINH" ,0x31); AddRpt("RPTIGT" ,0x32);
   AddRpt("RPTINC" ,0x33); AddRpt("RPTINVT",0x34); AddRpt("RPTINV" ,0x35);
   AddRpt("RPTIGE" ,0x36); AddRpt("RPTINE" ,0x37); AddRpt("RPTIST" ,0x38);
   AddRpt("RPTIH"  ,0x39); AddRpt("RPTILE" ,0x3a); AddRpt("RPTIC"  ,0x3b);
   AddRpt("RPTIVT" ,0x3c); AddRpt("RPTIV"  ,0x3d); AddRpt("RPTILT" ,0x3e);
   AddRpt("RPTIE"  ,0x3f);
}

	static void DeinitFields(void)
{
   free(FixedOrders);
   free(ALU3Orders);
   free(ALU2Orders);
   free(ALU1Orders);
   free(RelOrders);
   free(MacOrders);
   free(RptOrders);
}

/*-------------------------------------------------------------------------*/

	static void ChkSFR(Word Adr)
{
   if ((Adr>=SFRStart) & (Adr<=SFRStop)) WrError(190);
}

        static void Chk296(Word Adr)
{
   if ((MomCPU==CPU80296) && (Adr<=1)) WrError(190);
}

	static bool ChkWork(Word *Adr)
{
   /* Registeradresse, die von Fenstern ueberdeckt wird ? */

   if ((*Adr>=WinBegin) && (*Adr<=WinEnd)) return false;

   else if ((*Adr>=Win1Begin) && (*Adr<=Win1End)) return false;

   /* Speicheradresse in Fenster ? */

   else if ((*Adr>=WinStart) && (*Adr<=WinStop))
    {
     *Adr=(*Adr)-WinStart+WinBegin; return true;
    }

   else if ((*Adr>=Win1Start) && (*Adr<=Win1Stop))
    {
     *Adr=(*Adr)-Win1Start+Win1Begin; return true;
    }

   /* Default */

   else return (*Adr<=0xff);
}

	static void ChkAlign(Byte Adr)
{
   if (((OpSize==0) && ((Adr & 1)!=0))
   ||  ((OpSize==1) && ((Adr & 3)!=0))) WrError(180);
}

	static void ChkAdr(Byte Mask)
{
   if ((AdrType==ModDir) && ((Mask & MModDir)==0))
    {
     AdrType=ModMem; AdrMode=0;
    }

   if ((AdrType!=ModNone) && ((Mask & (1 << AdrType))==0))
    {
     WrError(1350); AdrType=ModNone; AdrCnt=0;
    }
}

	static void DecodeAdr(char *Asc, Byte Mask, bool AddrWide)
{
   LongInt AdrInt;
   LongWord AdrWord;
   Word BReg;
   bool OK;
   char *p,*p2;
   int l;
   Byte Reg;
   LongWord OMask;

   AdrType=ModNone; AdrCnt=0;
   OMask=(1 << OpSize)-1;

   if (*Asc=='#')
    {
     switch (OpSize)
      {
       case -1:
        WrError(1132); break;
       case 0:
	AdrVals[0]=EvalIntExpression(Asc+1,Int8,&OK);
	if (OK)
	 {
	  AdrType=ModImm; AdrCnt=1; AdrMode=1;
	 }
        break;
       case 1:
	AdrWord=EvalIntExpression(Asc+1,Int16,&OK);
	if (OK)
	 {
	  AdrType=ModImm; AdrCnt=2; AdrMode=1;
	  AdrVals[0]=Lo(AdrWord); AdrVals[1]=Hi(AdrWord);
	 }
        break;
      }
     ChkAdr(Mask); return;
    }

   p=QuotPos(Asc,'[');
   if (p!=NULL)
    {
     p2=RQuotPos(Asc,']'); l=strlen(Asc);
     if ((p2>Asc+l-1) || (p2<Asc+l-2)) WrError(1350);
     else
      {
       FirstPassUnknown=false; *p2='\0';
       BReg=EvalIntExpression(p+1,Int16,&OK);
       if (FirstPassUnknown) BReg=0;
       if (OK)
        if (! ChkWork(&BReg)) WrError(1320);
        else
	 {
	  Reg=Lo(BReg); ChkSFR(Reg);
	  if ((Reg&1)==1) WrError(1351);
	  else if ((p==Asc) && (p2==Asc+l-2) && (Asc[l-1]=='+'))
	   {
	    AdrType=ModMem; AdrMode=2; AdrCnt=1; AdrVals[0]=Reg+1;
	   }
	  else if (p2!=Asc+l-1) WrError(1350);
	  else if (p==Asc)
	   {
	    AdrType=ModMem; AdrMode=2; AdrCnt=1; AdrVals[0]=Reg;
	   }
	  else
	   {
            *p='\0';
	    if (! AddrWide) AdrInt=EvalIntExpression(Asc,Int16,&OK);
            else AdrInt=EvalIntExpression(Asc,Int24,&OK);
	    if (OK)
	     if (AdrInt==0)
	      {
	       AdrType=ModMem; AdrMode=2; AdrCnt=1; AdrVals[0]=Reg;
	      }
             else if (AddrWide)
              {
               AdrType=ModMem; AdrMode=3; AdrCnt=4;
               AdrVals[0]=Reg; AdrVals[1]=AdrInt & 0xff;
               AdrVals[2]=(AdrInt >> 8) & 0xff;
               AdrVals[3]=(AdrInt >> 16) & 0xff;
              }
	     else if ((AdrInt>=-128) && (AdrInt<127))
	      {
	       AdrType=ModMem; AdrMode=3; AdrCnt=2;
	       AdrVals[0]=Reg; AdrVals[1]=Lo(AdrInt);
	      }
	     else
	      {
	       AdrType=ModMem; AdrMode=3; AdrCnt=3;
	       AdrVals[0]=Reg+1; AdrVals[1]=Lo(AdrInt); AdrVals[2]=Hi(AdrInt);
	      }
	   }
	 }
      }
    }
   else
    {
     FirstPassUnknown=false;
     AdrWord=EvalIntExpression(Asc,MemInt,&OK);
     if (FirstPassUnknown) AdrWord&=(0xffffffff-OMask);
     if (OK)
      if ((AdrWord & OMask)!=0) WrError(1325);
      else
       {
        BReg=AdrWord & 0xffff;
        if (((BReg & 0xffff0000)==0) && (ChkWork(&BReg)))
 	 {
 	  AdrType=ModDir; AdrCnt=1; AdrVals[0]=Lo(BReg);
 	 }
        else if (AddrWide)
         {
          AdrType=ModMem; AdrMode=3; AdrCnt=4; AdrVals[0]=0;
          AdrVals[1]=AdrWord & 0xff;
          AdrVals[2]=(AdrWord >> 8) & 0xff;
          AdrVals[3]=(AdrWord >> 16) & 0xff;
         }
        else if (AdrWord>=0xff80)
 	 {
 	  AdrType=ModMem; AdrMode=3; AdrCnt=2; AdrVals[0]=0;
 	  AdrVals[1]=Lo(AdrWord);
 	 }
        else
 	 {
 	  AdrType=ModMem; AdrMode=3; AdrCnt=3; AdrVals[0]=1;
 	  AdrVals[1]=Lo(AdrWord); AdrVals[2]=Hi(AdrWord);
 	 }
       }
    }

   ChkAdr(Mask);
}

	static void CalcWSRWindow(void)
{
   if (WSRVal<=0x0f)
    {
     WinStart=0xffff; WinStop=0; WinBegin=0xff; WinEnd=0;
    }
   else if (WSRVal<=0x1f)
    {
     WinBegin=0x80; WinEnd=0xff;
     if (WSRVal<0x18) WinStart=(WSRVal-0x10) << 7;
     else WinStart=(WSRVal+0x20) << 7;
     WinStop=WinStart+0x7f;
    }
   else if (WSRVal<=0x3f)
    {
     WinBegin=0xc0; WinEnd=0xff;
     if (WSRVal<0x30) WinStart=(WSRVal-0x20) << 6;
     else WinStart=(WSRVal+0x40) << 6;
     WinStop=WinStart+0x3f;
    }
   else if (WSRVal<=0x7f)
    {
     WinBegin=0xe0; WinEnd=0xff;
     if (WSRVal<0x60) WinStart=(WSRVal-0x40) << 5;
     else WinStart=(WSRVal+0x80) << 5;
     WinStop=WinStart+0x1f;
    }
   if ((WinStop>0x1fdf) && (MomCPU<CPU80296)) WinStop=0x1fdf;
}

        static void CalcWSR1Window(void)
{
   if (WSR1Val<=0x1f)
    {
     Win1Start=0xffff; Win1Stop=0; Win1Begin=0xff; Win1End=0;
    }
   else if (WSR1Val<=0x3f)
    {
     Win1Begin=0x40; Win1End=0x7f;
     if (WSR1Val<0x30) Win1Start=(WSR1Val-0x20) << 6;
     else Win1Start=(WSR1Val+0x40) << 6;
     Win1Stop=Win1Start+0x3f;
    }
   else if (WSR1Val<=0x7f)
    {
     Win1Begin=0x60; Win1End=0x7f;
     if (WSR1Val<0x60) Win1Start=(WSR1Val-0x40) << 5;
     else Win1Start=(WSR1Val+0x80) << 5;
     Win1Stop=Win1Start+0x1f;
    }
   else
    {
     Win1Begin=0x40; Win1End=0x7f;
     Win1Start=(WSR1Val+0x340) << 6;
     Win1Stop=Win1Start+0x3f;
    }
}

	static bool DecodePseudo(void)
{
#define ASSUME96Count 2
   static ASSUMERec ASSUME96s[ASSUME96Count]=
	     {{"WSR", &WSRVal, 0, 0xff, 0x00},
              {"WSR1", &WSR1Val, 0, 0xbf, 0x00}};

   if (Memo("ASSUME"))
    {
     if (MomCPU<CPU80196) WrError(1500);
     else CodeASSUME(ASSUME96s,(MomCPU>=CPU80296)?ASSUME96Count:1);
     WSRVal&=0x7f;
     CalcWSRWindow(); CalcWSR1Window();
     return true;
    }

   return false;
}

	static bool BMemo(char *Name)
{
   int l;

   if (strncmp(OpPart,Name,l=strlen(Name))!=0) return false;
   switch (OpPart[l])
    {
     case '\0':
      OpSize=1; return true;
     case 'B':
      if (OpPart[l+1]=='\0')
       {
        OpSize=0; return true;
       }
      else return false;
     default:
      return false;
    }
}

	static bool LMemo(char *Name)
{
   int l;

   if (strncmp(OpPart,Name,l=strlen(Name))!=0) return false;
   switch (OpPart[l])
    {
     case '\0':
      OpSize=1; return true;
     case 'B':
      if (OpPart[l+1]=='\0')
       {
        OpSize=0; return true;
       }
      else return false;
     case 'L':
      if (OpPart[l+1]=='\0')
       {
        OpSize=2; return true;
       }
      else return false;
     default:
      return false;
    }
}

	static void MakeCode_96(void)
{
   bool OK,Special,IsShort;
   Word AdrWord;
   Integer z;
   LongInt AdrInt;
   Byte Start,HReg,Mask;

   CodeLen=0; DontPrint=false; OpSize=(-1);

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(false)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     {
      if (ArgCnt!=0) WrError(1110);
      else if (MomCPU<FixedOrders[z].MinCPU) WrError(1500);
      else BAsmCode[(CodeLen=1)-1]=FixedOrders[z].Code;
      return;
     }

   /* Arithmetik */

   for (z=0; z<ALU3OrderCnt; z++)
    if (BMemo(ALU3Orders[z].Name))
     {
      if ((ArgCnt!=2) && (ArgCnt!=3)) WrError(1110);
      else
       {
        Start=0; Special=(strncmp(ALU3Orders[z].Name,"MUL",3)==0);
        if ((ALU3Orders[z].Code & 0x80)!=0) BAsmCode[Start++]=0xfe;
        BAsmCode[Start++]=0x40+((ArgCnt==2) << 5)
       		          +((1-OpSize) << 4)
       		          +((ALU3Orders[z].Code & 0x7f) << 2);
        DecodeAdr(ArgStr[ArgCnt],MModImm+MModMem,false);
        if (AdrType!=ModNone)
         {
          BAsmCode[Start-1]+=AdrMode;
          memcpy(BAsmCode+Start,AdrVals,AdrCnt); Start+=AdrCnt;
          if ((Special) && (AdrMode==0)) ChkSFR(AdrVals[0]);
          if (ArgCnt==3)
           {
            DecodeAdr(ArgStr[2],MModDir,false);
            OK=(AdrType!=ModNone);
            if (OK)
             {
              BAsmCode[Start++]=AdrVals[0];
              if (Special) ChkSFR(AdrVals[0]);
             }
           }
          else OK=true;
          if (OK)
           {
            DecodeAdr(ArgStr[1],MModDir,false);
            if (AdrType!=ModNone)
             {
              BAsmCode[Start]=AdrVals[0]; CodeLen=Start+1;
              if (Special)
       	       {
       	        ChkSFR(AdrVals[0]);
                Chk296(AdrVals[0]);
                ChkAlign(AdrVals[0]);
       	       }
             }
           }
         }
       }
      return;
     }

   for (z=0; z<ALU2OrderCnt; z++)
    if (BMemo(ALU2Orders[z].Name))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
        Start=0; Special=(strncmp(OpPart,"DIV",3)==0);
        if (strcmp(ALU2Orders[z].Name,"DIV")==0) BAsmCode[Start++]=0xfe;
        HReg=(1+(strcmp(ALU2Orders[z].Name,"ST")!=0)) << 1;
        BAsmCode[Start++]=ALU2Orders[z].Code+((1-OpSize) << HReg);
        Mask=MModMem; if (! BMemo("ST")) Mask+=MModImm;
        DecodeAdr(ArgStr[2],Mask,false);
        if (AdrType!=ModNone)
         {
          BAsmCode[Start-1]+=AdrMode;
          memcpy(BAsmCode+Start,AdrVals,AdrCnt); Start+=AdrCnt;
          if ((Special) && (AdrMode==0)) ChkSFR(AdrVals[0]);
          DecodeAdr(ArgStr[1],MModDir,false);
          if (AdrType!=ModNone)
           {
            BAsmCode[Start]=AdrVals[0]; CodeLen=1+Start;
            if (Special)
             {
              ChkSFR(AdrVals[0]); ChkAlign(AdrVals[0]);
             }
           }
         }
       }
      return;
     }

   if (Memo("CMPL"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80196) WrError(1500);
     else
      {
       OpSize=2;
       DecodeAdr(ArgStr[1],MModDir,false);
       if (AdrType!=ModNone)
        {
         BAsmCode[2]=AdrVals[0];
         DecodeAdr(ArgStr[2],MModDir,false);
         if (AdrType!=ModNone)
          {
           BAsmCode[1]=AdrVals[0]; BAsmCode[0]=0xc5; CodeLen=3;
          }
        }
      }
     return;
    }

   if ((Memo("PUSH")) || (Memo("POP")))
    {
     OpSize=1;
     if (ArgCnt!=1) WrError(1110);
     else
      {
       Mask=MModMem; if (Memo("PUSH")) Mask+=MModImm;
       DecodeAdr(ArgStr[1],Mask,false);
       if (AdrType!=ModNone)
	{
	 CodeLen=1+AdrCnt;
	 BAsmCode[0]=0xc8+AdrMode+(Memo("POP") << 2);
	 memcpy(BAsmCode+1,AdrVals,AdrCnt);
	}
      }
     return;
    }

   if ((Memo("BMOV")) || (Memo("BMOVI")) || (Memo("EBMOVI")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80196) WrError(1500);
     else if ((MomCPU<CPU80196N) && (Memo("EBMOVI"))) WrError(1500);
     else
      {
       OpSize=2; DecodeAdr(ArgStr[1],MModDir,false);
       if (AdrType!=ModNone)
        {
         BAsmCode[2]=AdrVals[0];
         OpSize=1; DecodeAdr(ArgStr[2],MModDir,false);
         if (AdrType!=ModNone)
          {
           BAsmCode[1]=AdrVals[0];
           if (Memo("BMOVI")) BAsmCode[0]=0xad;
           else if (Memo("BMOV")) BAsmCode[0]=0xc1;
           else BAsmCode[0]=0xe4;
           CodeLen=3;
          }
        }
      }
     return;
    }

   for (z=0; z<ALU1OrderCnt; z++)
    if (BMemo(ALU1Orders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else
       {
        DecodeAdr(ArgStr[1],MModDir,false);
        if (AdrType!=ModNone)
         {
          CodeLen=1+AdrCnt;
          BAsmCode[0]=ALU1Orders[z].Code+((1-OpSize) << 4);
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          if (BMemo("EXT")) ChkAlign(AdrVals[0]);
         }
       }
      return;
     }

   if (BMemo("XCH"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80196) WrError(1500);
     else
      {
       DecodeAdr(ArgStr[1],MModMem+MModDir,false);
       switch (AdrType)
        {
         case ModMem:
          if (AdrMode==1) WrError(1350);
          else
           {
            memcpy(BAsmCode+1,AdrVals,AdrCnt); HReg=AdrCnt;
            BAsmCode[0]=0x04+((1-OpSize) << 4)+AdrMode;
            DecodeAdr(ArgStr[2],MModDir,false);
            if (AdrType!=ModNone)
             {
              BAsmCode[1+HReg]=AdrVals[0]; CodeLen=2+HReg;
             }
           }
          break;
         case ModDir:
          HReg=AdrVals[0];
          DecodeAdr(ArgStr[2],MModMem,false);
          if (AdrType!=ModNone)
           if (AdrMode==1) WrError(1350);
           else
            {
             BAsmCode[0]=0x04+((1-OpSize) << 4)+AdrMode;
             memcpy(BAsmCode+1,AdrVals,AdrCnt);
             BAsmCode[1+AdrCnt]=HReg; CodeLen=2+AdrCnt;
            }
          break;
        }
      }
     return;
    }

   if ((Memo("LDBZE")) || (Memo("LDBSE")))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       OpSize=0;
       DecodeAdr(ArgStr[2],MModMem+MModImm,false);
       if (AdrType!=ModNone)
	{
	 BAsmCode[0]=0xac+(Memo("LDBSE") << 4)+AdrMode;
	 memcpy(BAsmCode+1,AdrVals,AdrCnt); Start=1+AdrCnt;
	 OpSize=1; DecodeAdr(ArgStr[1],MModDir,false);
	 if (AdrType!=ModNone)
	  {
	   BAsmCode[Start]=AdrVals[0]; CodeLen=1+Start;
	  }
	}
      }
     return;
    }

   if (Memo("NORML"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       OpSize=0; DecodeAdr(ArgStr[2],MModDir,false);
       if (AdrType!=ModNone)
	{
	 BAsmCode[1]=AdrVals[0];
	 OpSize=1; DecodeAdr(ArgStr[1],MModDir,false);
	 if (AdrType!=ModNone)
	  {
	   CodeLen=3; BAsmCode[0]=0x0f; BAsmCode[2]=AdrVals[0];
	   ChkAlign(AdrVals[0]);
	  }
	}
      }
     return;
    }

   if (Memo("IDLPD"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU80196) WrError(1500);
     else
      {
       OpSize=0; DecodeAdr(ArgStr[1],MModImm,false);
       if (AdrType!=ModNone)
        {
         CodeLen=2; BAsmCode[0]=0xf6; BAsmCode[1]=AdrVals[0];
        }
      }
     return;
    }

   for (z=0; z<ShiftOrderCnt; z++)
    if (LMemo(ShiftOrders[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
	DecodeAdr(ArgStr[1],MModDir,false);
	if (AdrType!=ModNone)
	 {
	  BAsmCode[0]=0x08+z+((OpSize==0) << 4)+((OpSize==2) << 2);
	  BAsmCode[2]=AdrVals[0];
	  OpSize=0; DecodeAdr(ArgStr[2],MModDir+MModImm,false);
	  if (AdrType!=ModNone)
	   if ((AdrType==ModImm) && (AdrVals[0]>15)) WrError(1320);
	   else if ((AdrType==ModDir) && (AdrVals[0]<16)) WrError(1315);
	   else
	    {
	     BAsmCode[1]=AdrVals[0]; CodeLen=3;
	    }
	 }
       }
      return;
     }

   if (Memo("SKIP"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       OpSize=0; DecodeAdr(ArgStr[1],MModDir,false);
       if (AdrType!=ModNone)
	{
	 CodeLen=2; BAsmCode[0]=0; BAsmCode[1]=AdrVals[0];
	}
      }
     return;
    }

   if ((BMemo("ELD")) || (BMemo("EST")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80196N) WrError(1500);
     else
      {
       DecodeAdr(ArgStr[2],MModMem,true);
       if (AdrType==ModMem)
        if ((AdrMode==2) && (Odd(AdrVals[0]))) WrError(1350); /* kein Autoincrement */
        else
         {
          BAsmCode[0]=(AdrMode & 1)+((1-OpSize) << 1);
          if (OpPart[1]=='L') BAsmCode[0]+=0xe8;
                         else BAsmCode[0]+=0x1c;
          memcpy(BAsmCode+1,AdrVals,AdrCnt); HReg=1+AdrCnt;
          DecodeAdr(ArgStr[1],MModDir,false);
          if (AdrType==ModDir)
           {
            BAsmCode[HReg]=AdrVals[0]; CodeLen=HReg+1;
           };
         };
      };
     return;
    }

   for (z=0; z<MacOrderCnt; z++)
    if (Memo(MacOrders[z].Name))
     {
      if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
      else if (MomCPU<CPU80296) WrError(1500);
      else
       {
        OpSize=1; BAsmCode[0]=0x4c+((ArgCnt==1) << 5);
        if (MacOrders[z].Reloc) DecodeAdr(ArgStr[ArgCnt],MModMem,false);
        else DecodeAdr(ArgStr[ArgCnt],MModMem+MModImm,false);
        if (AdrType!=ModNone)
         {
          BAsmCode[0]+=AdrMode;
          memcpy(BAsmCode+1,AdrVals,AdrCnt); HReg=1+AdrCnt;
          if (ArgCnt==2)
           {
            DecodeAdr(ArgStr[1],MModDir,false);
            if (AdrType==ModDir)
             {
              BAsmCode[HReg]=AdrVals[0]; HReg++;
             }
           }
          if (AdrType!=ModNone)
           {
            BAsmCode[HReg]=MacOrders[z].Code; CodeLen=1+HReg;
           }
         }
       }
      return;
     }

   if ((Memo("MVAC")) || (Memo("MSAC")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80296) WrError(1500);
     else
      {
       OpSize=2; DecodeAdr(ArgStr[1],MModDir,false);
       if (AdrType==ModDir)
        {
         BAsmCode[0]=0x0d; BAsmCode[2]=AdrVals[0]+1+(Memo("MSAC") << 1);
         OpSize=0; DecodeAdr(ArgStr[2],MModImm+MModDir,false);
         BAsmCode[1]=AdrVals[0];
         switch (AdrType)
          {
           case ModImm:
            if (AdrVals[0]>31) WrError(1320); else CodeLen=3;
            break;
           case ModDir:
            if (AdrVals[0]<32) WrError(1315); else CodeLen=3;
          }
        }
      }
     return;
    }

   for (z=0; z<RptOrderCnt; z++)
    if (Memo(RptOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (MomCPU<CPU80296) WrError(1500);
      else
       {
        OpSize=1; DecodeAdr(ArgStr[1],MModImm+MModMem,false);
        if (AdrType!=ModNone)
         if (AdrMode==3) WrError(1350);
         else
          {
           BAsmCode[0]=0x40+AdrMode;
           memcpy(BAsmCode+1,AdrVals,AdrCnt);
           BAsmCode[1+AdrCnt]=RptOrders[z].Code;
           BAsmCode[2+AdrCnt]=4;
           CodeLen=3+AdrCnt;
          }
       }
      return;
     }

   /* Spruenge */

   for (z=0; z<RelOrderCnt; z++)
    if (Memo(RelOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else
       {
        AdrInt=EvalIntExpression(ArgStr[1],MemInt,&OK)-(EProgCounter()+2);
        if (OK)
         if ((! SymbolQuestionable) && ((AdrInt<-128) || (AdrInt>127))) WrError(1370);
         else
          {
           CodeLen=2; BAsmCode[0]=RelOrders[z].Code; BAsmCode[1]=AdrInt & 0xff;
          }
       }
      return;
     }

   if ((Memo("SCALL")) || (Memo("LCALL")) || (Memo("CALL")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrWord=EvalIntExpression(ArgStr[1],MemInt,&OK);
       if (OK)
	{
	 AdrInt=AdrWord-(EProgCounter()+2);
	 if (Memo("SCALL")) IsShort=true;
	 else if (Memo("LCALL")) IsShort=false;
	 else IsShort=((AdrInt>=-1024) && (AdrInt<1023));
	 if (IsShort)
	  {
	   if ((! SymbolQuestionable) && ((AdrInt<-1024) || (AdrInt>1023))) WrError(1370);
	   else
	    {
	     CodeLen=2; BAsmCode[1]=AdrInt & 0xff;
	     BAsmCode[0]=0x28+((AdrInt & 0x700) >> 8);
	    }
	  }
	 else
	  {
           CodeLen=3; BAsmCode[0]=0xef; AdrInt--;
           BAsmCode[1]=Lo(AdrInt); BAsmCode[2]=Hi(AdrInt);
	   if ((! SymbolQuestionable) && (AdrInt>=-1024) && (AdrInt<=1023)) WrError(20);
	  }
	}
      }
     return;
    }

   if ((Memo("BR")) || (Memo("LJMP")) || (Memo("SJMP")))
    {
     OpSize=1;
     if (ArgCnt!=1) WrError(1110);
     else if ((Memo("BR")) && (QuotPos(ArgStr[1],'[')!=NULL))
      {
       DecodeAdr(ArgStr[1],MModMem,false);
       if (AdrType!=ModNone)
	if ((AdrMode!=2) || ((AdrVals[0]&1)==1)) WrError(1350);
	else
	 {
	  CodeLen=2; BAsmCode[0]=0xe3; BAsmCode[1]=AdrVals[0];
	 }
      }
     else
      {
       AdrWord=EvalIntExpression(ArgStr[1],MemInt,&OK);
       if (OK)
	{
	 AdrInt=AdrWord-(EProgCounter()+2);
	 if (Memo("SJMP")) IsShort=true;
	 else if (Memo("LJMP")) IsShort=false;
	 else IsShort=((AdrInt>=-1024) && (AdrInt<1023));
	 if (IsShort)
	  {
	   if ((! SymbolQuestionable) && ((AdrInt<-1024) || (AdrInt>1023))) WrError(1370);
	   else
	    {
	     CodeLen=2; BAsmCode[1]=AdrInt & 0xff;
	     BAsmCode[0]=0x20+((AdrInt & 0x700) >> 8);
	    }
	  }
	 else
	  {
           CodeLen=3; BAsmCode[0]=0xe7; AdrInt--;
           BAsmCode[1]=Lo(AdrInt); BAsmCode[2]=Hi(AdrInt);
	   if ((! SymbolQuestionable) && (AdrInt>=-1024) && (AdrInt<=1023)) WrError(20);
	  }
	}
      }
     return;
    }

   if (Memo("TIJMP"))
    {
     if (ArgCnt!=3) WrError(1110);
     else if (MomCPU<CPU80196) WrError(1500);
     else
      {
       OpSize=1; DecodeAdr(ArgStr[1],MModDir,false);
       if (AdrType!=ModNone)
        {
         BAsmCode[3]=AdrVals[0];
         DecodeAdr(ArgStr[2],MModDir,false);
         if (AdrType!=ModNone)
          {
           BAsmCode[1]=AdrVals[0];
           OpSize=0; DecodeAdr(ArgStr[3],MModImm,false);
           if (AdrType!=ModNone)
            {
             BAsmCode[2]=AdrVals[0]; BAsmCode[0]=0xe2; CodeLen=4;
            }
          }
        }
      }
     return;
    }

   if ((Memo("DJNZ")) || (Memo("DJNZW")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if ((Memo("DJNZW")) && (MomCPU<CPU80196)) WrError(1500);
     else
      {
       OpSize=Memo("DJNZW");
       DecodeAdr(ArgStr[1],MModDir,false);
       if (AdrType!=ModNone)
	{
	 BAsmCode[0]=0xe0+OpSize; BAsmCode[1]=AdrVals[0];
	 AdrInt=EvalIntExpression(ArgStr[2],MemInt,&OK)-(EProgCounter()+3);
	 if (OK)
	  if ((! SymbolQuestionable) && ((AdrInt<-128) || (AdrInt>127))) WrError(1370);
	  else
	   {
	    CodeLen=3; BAsmCode[2]=AdrInt & 0xff;
	   }
	}
      }
     return;
    }

   if ((Memo("JBC")) || (Memo("JBS")))
    {
     if (ArgCnt!=3) WrError(1110);
     else
      {
       BAsmCode[0]=EvalIntExpression(ArgStr[2],UInt3,&OK);
       if (OK)
	{
	 BAsmCode[0]+=0x30+(Memo("JBS") << 3);
	 OpSize=0; DecodeAdr(ArgStr[1],MModDir,false);
	 if (AdrType!=ModNone)
	  {
	   BAsmCode[1]=AdrVals[0];
	   AdrInt=EvalIntExpression(ArgStr[3],MemInt,&OK)-(EProgCounter()+3);
	   if (OK)
	    if ((! SymbolQuestionable) && ((AdrInt<-128) || (AdrInt>127))) WrError(1370);
	    else
	     {
	      CodeLen=3; BAsmCode[2]=AdrInt & 0xff;
	     }
	  }
	}
      }
     return;
    }

   if (Memo("ECALL"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU80196N) WrError(1500);
     else
      {
       AdrInt=EvalIntExpression(ArgStr[1],MemInt,&OK)-(EProgCounter()+4);
       if (OK)
        {
         BAsmCode[0]=0xf1;
         BAsmCode[1]=AdrInt & 0xff;
         BAsmCode[2]=(AdrInt >> 8) & 0xff;
         BAsmCode[3]=(AdrInt >> 16) & 0xff;
         CodeLen=4;
        }
      }
     return;
    }

   if ((Memo("EJMP")) || (Memo("EBR")))
    {
     OpSize=1;
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU80196N) WrError(1500);
     else if (*ArgStr[1]=='[')
      {
       DecodeAdr(ArgStr[1],MModMem,false);
       if (AdrType==ModMem)
        if (AdrMode!=2) WrError(1350);
        else if (Odd(AdrVals[0])) WrError(1350);
        else
         {
          BAsmCode[0]=0xe3; BAsmCode[1]=AdrVals[0]+1;
          CodeLen=2;
         }
      }
     else
      {
       AdrInt=EvalIntExpression(ArgStr[1],MemInt,&OK)-(EProgCounter()+4);
       if (OK)
        {
         BAsmCode[0]=0xe6;
         BAsmCode[1]=AdrInt & 0xff;
         BAsmCode[2]=(AdrInt >> 8) & 0xff;
         BAsmCode[3]=(AdrInt >> 16) & 0xff;
         CodeLen=4;
        }
      }
     return;
    }

   WrXError(1200,OpPart);
}

	static void InitCode_96(void)
{
   SaveInitProc();
   WSRVal=0; CalcWSRWindow();
   WSR1Val=0; CalcWSR1Window();
}

	static bool ChkPC_96(void)
{
   switch (ActPC)
    {
     case SegCode:
      if (MomCPU>=CPU80196N) return (ProgCounter()<0x1000000);
                        else return (ProgCounter()<0x10000);
     default:
      return false;
    }
}


	static bool IsDef_96(void)
{
   return false;
}

        static void SwitchFrom_96(void)
{
   DeinitFields();
}

	static void SwitchTo_96(void)
{
   TurnWords=false; ConstMode=ConstModeIntel; SetIsOccupied=false;

   PCSymbol="$"; HeaderID=0x39; NOPCode=0xfd;
   DivideChars=","; HasAttrs=false;

   ValidSegs=1<<SegCode;
   Grans[SegCode ]=1; ListGrans[SegCode ]=1; SegInits[SegCode ]=0;

   MakeCode=MakeCode_96; ChkPC=ChkPC_96; IsDef=IsDef_96;
   SwitchFrom=SwitchFrom_96;

   if (MomCPU>=CPU80196N) MemInt=UInt24;
   else MemInt=UInt16;

   InitFields();
}

	void code96_init(void)
{
   CPU8096  =AddCPU("8096"  ,SwitchTo_96);
   CPU80196 =AddCPU("80196" ,SwitchTo_96);
   CPU80196N=AddCPU("80196N",SwitchTo_96);
   CPU80296 =AddCPU("80296" ,SwitchTo_96);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_96;
}
