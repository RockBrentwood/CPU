/* code96c141.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TLCS-900(L)                                                 */
/*                                                                           */
/* Historie: 27. 6.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "stringutil.h"

#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"


typedef struct
         {
          char *Name;
          Word Code;
          Byte CPUFlag;
          bool InSup;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Word Code;
          bool InSup;
          Byte MinMax,MaxMax;
          ShortInt Default;
         } ImmOrder;

typedef struct
         {
          char *Name;
          Word Code;
          Byte OpMask;
         } RegOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } ALU2Order;

typedef struct
         {
          char *Name;
          Byte Code;
         } Condition;

#define FixedOrderCnt 13
#define ImmOrderCnt 3
#define RegOrderCnt 8
#define ALU2OrderCnt 8
#define ShiftOrderCnt 8
#define MulDivOrderCnt 4
#define BitCFOrderCnt 5
#define BitOrderCnt 5
#define ConditionCnt 24

#define ModNone (-1)
#define ModReg 0
#define MModReg (1  << ModReg)
#define ModXReg 1
#define MModXReg (1 << ModXReg)
#define ModMem 2
#define MModMem (1  << ModMem)
#define ModImm 3
#define MModImm (1  << ModImm)
#define ModCReg 4
#define MModCReg (1 << ModCReg)

static FixedOrder *FixedOrders;
static RegOrder *RegOrders;
static ImmOrder *ImmOrders;
static ALU2Order *ALU2Orders;
static char **ShiftOrders;
static char **MulDivOrders;
static ALU2Order *BitCFOrders;
static char **BitOrders;
static Condition *Conditions;
static LongInt DefaultCondition;

static ShortInt AdrType;
static ShortInt OpSize;        /* -1/0/1/2 = nix/Byte/Word/Long */
static Byte AdrMode;
static Byte AdrVals[10];
static bool MinOneIs0;

static CPUVar CPU96C141,CPU93C141;

/*---------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode, Byte NFlag, bool NSup)
{
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].Code=NCode;
   FixedOrders[InstrZ].CPUFlag=NFlag;
   FixedOrders[InstrZ++].InSup=NSup;
}

        static void AddReg(char *NName, Word NCode, Byte NMask)
{
   if (InstrZ>=RegOrderCnt) exit(255);
   RegOrders[InstrZ].Name=NName;
   RegOrders[InstrZ].Code=NCode;
   RegOrders[InstrZ++].OpMask=NMask;
}

        static void AddImm(char *NName, Word NCode, bool NInSup,
                           Byte NMinMax, Byte NMaxMax, ShortInt NDefault)
{
   if (InstrZ>=ImmOrderCnt) exit(255);
   ImmOrders[InstrZ].Name=NName;
   ImmOrders[InstrZ].Code=NCode;
   ImmOrders[InstrZ].InSup=NInSup;
   ImmOrders[InstrZ].MinMax=NMinMax;
   ImmOrders[InstrZ].MaxMax=NMaxMax;
   ImmOrders[InstrZ++].Default=NDefault;
}

        static void AddALU2(char *NName, Byte NCode)
{
   if (InstrZ>=ALU2OrderCnt) exit(255);
   ALU2Orders[InstrZ].Name=NName;
   ALU2Orders[InstrZ++].Code=NCode;
}

	static void AddShift(char *NName)
{
   if (InstrZ>=ShiftOrderCnt) exit(255);
   ShiftOrders[InstrZ++]=NName;
}

	static void AddMulDiv(char *NName)
{
   if (InstrZ>=MulDivOrderCnt) exit(255);
   MulDivOrders[InstrZ++]=NName;
}

        static void AddBitCF(char *NName, Byte NCode)
{
   if (InstrZ>=BitCFOrderCnt) exit(255);
   BitCFOrders[InstrZ].Name=NName;
   BitCFOrders[InstrZ++].Code=NCode;
}

	static void AddBit(char *NName)
{
   if (InstrZ>=BitOrderCnt) exit(255);
   BitOrders[InstrZ++]=NName;
}

        static void AddCondition(char *NName, Byte NCode)
{
   if (InstrZ>=ConditionCnt) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
}

        static void InitFields(void)
{
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("CCF"   , 0x0012, 3, false);
   AddFixed("DECF"  , 0x000d, 3, false);
   AddFixed("DI"    , 0x0607, 3, true );
   AddFixed("HALT"  , 0x0005, 3, true );
   AddFixed("INCF"  , 0x000c, 3, false);
   AddFixed("MAX"   , 0x0004, 1, true );
   AddFixed("MIN"   , 0x0004, 2, true );
   AddFixed("NOP"   , 0x0000, 3, false);
   AddFixed("NORMAL", 0x0001, 1, true );
   AddFixed("RCF"   , 0x0010, 3, false);
   AddFixed("RETI"  , 0x0007, 3, true );
   AddFixed("SCF"   , 0x0011, 3, false);
   AddFixed("ZCF"   , 0x0013, 3, false);

   RegOrders=(RegOrder *) malloc(sizeof(RegOrder)*RegOrderCnt); InstrZ=0;
   AddReg("CPL" , 0xc006, 3);
   AddReg("DAA" , 0xc010, 1);
   AddReg("EXTS", 0xc013, 6);
   AddReg("EXTZ", 0xc012, 6);
   AddReg("MIRR", 0xc016, 2);
   AddReg("NEG" , 0xc007, 3);
   AddReg("PAA" , 0xc014, 6);
   AddReg("UNLK", 0xc00d, 4);

   ImmOrders=(ImmOrder *) malloc(sizeof(ImmOrder)*ImmOrderCnt); InstrZ=0;
   AddImm("EI"  , 0x0600, true,  7, 7,  0);
   AddImm("LDF" , 0x1700, false, 7, 3, -1);
   AddImm("SWI" , 0x00f8, false, 7, 7,  7);

   ALU2Orders=(ALU2Order *) malloc(sizeof(ALU2Order)*ALU2OrderCnt); InstrZ=0;
   AddALU2("ADC", 1);
   AddALU2("ADD", 0);
   AddALU2("AND", 4);
   AddALU2("OR" , 6);
   AddALU2("SBC", 3);
   AddALU2("SUB", 2);
   AddALU2("XOR", 5);
   AddALU2("CP" , 7);

   ShiftOrders=(char **) malloc(sizeof(char*)*ShiftOrderCnt); InstrZ=0;
   AddShift("RLC");
   AddShift("RRC");
   AddShift("RL");
   AddShift("RR");
   AddShift("SLA");
   AddShift("SRA");
   AddShift("SLL");
   AddShift("SRL");

   MulDivOrders=(char **) malloc(sizeof(char*)*MulDivOrderCnt); InstrZ=0;
   AddMulDiv("MUL");
   AddMulDiv("MULS");
   AddMulDiv("DIV");
   AddMulDiv("DIVS");

   BitCFOrders=(ALU2Order *) malloc(sizeof(ALU2Order)*BitCFOrderCnt); InstrZ=0;
   AddBitCF("ANDCF" , 0);
   AddBitCF("LDCF"  , 3);
   AddBitCF("ORCF"  , 1);
   AddBitCF("STCF"  , 4);
   AddBitCF("XORCF" , 2);

   BitOrders=(char **) malloc(sizeof(char*)*BitOrderCnt); InstrZ=0;
   AddBit("RES");
   AddBit("SET");
   AddBit("CHG");
   AddBit("BIT");
   AddBit("TSET");

   Conditions=(Condition *) malloc(sizeof(Condition)*ConditionCnt); InstrZ=0;
   AddCondition("F"   ,  0);
   DefaultCondition=InstrZ;  AddCondition("T"   ,  8);
   AddCondition("Z"   ,  6); AddCondition("NZ"  , 14);
   AddCondition("C"   ,  7); AddCondition("NC"  , 15);
   AddCondition("PL"  , 13); AddCondition("MI"  ,  5);
   AddCondition("P"   , 13); AddCondition("M"   ,  5);
   AddCondition("NE"  , 14); AddCondition("EQ"  ,  6);
   AddCondition("OV"  ,  4); AddCondition("NOV" , 12);
   AddCondition("PE"  ,  4); AddCondition("PO"  , 12);
   AddCondition("GE"  ,  9); AddCondition("LT"  ,  1);
   AddCondition("GT"  , 10); AddCondition("LE"  ,  2);
   AddCondition("UGE" , 15); AddCondition("ULT" ,  7);
   AddCondition("UGT" , 11); AddCondition("ULE" ,  3);
}

        static void DeinitFields(void)
{
   free(FixedOrders);
   free(RegOrders);
   free(ImmOrders);
   free(ALU2Orders);
   free(ShiftOrders);
   free(MulDivOrders);
   free(BitCFOrders);
   free(BitOrders);
   free(Conditions);
}

/*---------------------------------------------------------------------------*/

	static bool IsRegBase(Byte No, Byte Size)
{
   return ((Size==2) || ((Size==1) && (No<0xf0) && (! Maximum) && ((No & 3)==0)));
}

	static void ChkMaximum(bool MustMax, Byte *Result)
{
   if (Maximum!=MustMax)
    {
     *Result=1;
     WrError((MustMax)?1997:1996);
    }
}

	static bool IsQuot(char Ch)
{
   return ((Ch=='\'') || (Ch=='`'));
}

	static Byte CodeEReg(char *Asc, Byte *ErgNo, Byte *ErgSize)
{
#define RegCnt 8
   static char *Reg8Names[RegCnt]=
	      {"A"  ,"W"  ,"C"  ,"B"  ,"E"  ,"D"  ,"L"  ,"H"  };
   static char *Reg16Names[RegCnt]=
	      {"WA" ,"BC" ,"DE" ,"HL" ,"IX" ,"IY" ,"IZ" ,"SP" };
   static char *Reg32Names[RegCnt]=
	      {"XWA","XBC","XDE","XHL","XIX","XIY","XIZ","XSP"};

   Integer z;
   String HAsc,Asc_N;
   Byte Result;

   strmaxcpy(Asc_N,Asc,255); NLS_UpString(Asc_N); Asc=Asc_N;

   Result=2;

   /* mom. Bank ? */

   for (z=0; z<RegCnt; z++)
    {
     if (strcmp(Asc,Reg8Names[z])==0)
      {
       *ErgNo=0xe0+((z & 6) << 1)+(z & 1); *ErgSize=0; return Result;
      }
     if (strcmp(Asc,Reg16Names[z])==0)
      {
       *ErgNo=0xe0+(z << 2); *ErgSize=1; return Result;
      }
     if (strcmp(Asc,Reg32Names[z])==0)
      {
       *ErgNo=0xe0+(z << 2); *ErgSize=2;
       if (z<4) ChkMaximum(true,&Result);
       return Result;
      }
    }

   /* Bankregister, 8 Bit ? */

   if ((strlen(Asc)==3) && ((*Asc=='Q') || (*Asc=='R')) && ((Asc[2]>='0') && (Asc[2]<='7')))
    for (z=0; z<RegCnt; z++)
     if (Asc[1]==*Reg8Names[z])
      {
       *ErgNo=((Asc[3]-'0') << 4)+((z & 6) << 1)+(z & 1);
       if (*Asc=='Q')
	{
	 ErgNo+=2; ChkMaximum(true,&Result);
	}
       if (((*Asc=='Q') || (Maximum)) && (Asc[2]>'3'))
	{
	 WrError(1320); Result=1;
	}
       *ErgSize=0; return Result;
      }

   /* Bankregister, 16 Bit ? */

   if ((strlen(Asc)==4) && ((*Asc=='Q') || (*Asc=='R')) && ((Asc[3]>='0') && (Asc[3]<='7')))
    {
     strmaxcpy(HAsc,Asc+2,255); HAsc[2]='\0';
     for (z=0; z<(RegCnt/2)-1; z++)
      if (strcmp(HAsc,Reg16Names[z])==0)
       {
	*ErgNo=((Asc[4]-'0') << 4)+(z << 2);
	if (*Asc=='Q')
	 {
	  ErgNo+=2; ChkMaximum(true,&Result);
	 }
	if (((*Asc=='Q') || (Maximum)) && (Asc[3]>'3'))
	 {
	  WrError(1320); Result=1;
	 }
	*ErgSize=1; return Result;
       }
    }

   /* Bankregister, 32 Bit ? */

   if ((strlen(Asc)==4) && ((Asc[3]>='0') && (Asc[3]<='7')))
    {
     strcpy(HAsc,Asc); HAsc[3]='\0';
     for (z=0; z<(RegCnt/2)-1; z++)
      if (strcmp(HAsc,Reg32Names[z])==0)
       {
	*ErgNo=((Asc[4]-'0') << 4)+(z << 2);
	ChkMaximum(true,&Result);
	if (Asc[3]>'3')
	 {
	  WrError(1320); Result=1;
	 }
	*ErgSize=2; return Result;
       }
    }

   /* obere 8-Bit-Haelften momentaner Bank ? */

   if ((strlen(Asc)==2) && (*Asc=='Q'))
    for (z=0; z<RegCnt; z++)
     if (strcmp(Asc+1,Reg8Names[z])==0)
      {
       *ErgNo=0xe2+((z & 6) << 1)+(z & 1);
       ChkMaximum(true,&Result);
       *ErgSize=0; return Result;
      }

   /* obere 16-Bit-Haelften momentaner Bank und von XIX..XSP ? */

   if ((strlen(Asc)==3) && (*Asc=='Q'))
    {
     for (z=0; z<RegCnt; z++)
      if (strcmp(Asc+1,Reg16Names[z])==0)
       {
	*ErgNo=0xe2+(z << 2);
	if (z<4) ChkMaximum(true,&Result);
	*ErgSize=1; return Result;
       }
    }

   /* 8-Bit-Teile von XIX..XSP ? */

   if (((strlen(Asc)==3) || ((strlen(Asc)==4) && (*Asc=='Q')))
   && ((Asc[strlen(Asc)-1]=='L') || (Asc[strlen(Asc)-1]=='H')))
    {
     strcpy(HAsc,Asc+1); HAsc[strlen(HAsc)-1]='\0';
     for (z=0; z<(RegCnt/2); z++)
      if (strcmp(Asc,Reg16Names[z+4])==0)
       {
	*ErgNo=0xf0+(z << 2)+((strlen(Asc)-3) << 1)+((Asc[strlen(Asc)-1]=='H'));
	*ErgSize=0; return Result;
       }
    }

   /* 8-Bit-Teile vorheriger Bank ? */

   if (((strlen(Asc)==2) || ((strlen(Asc)==3) && (*Asc=='Q'))) && (IsQuot(Asc[strlen(Asc)-1])))
    for (z=0; z<RegCnt; z++)
     if (Asc[strlen(Asc)-2]==*Reg8Names[z])
      {
       *ErgNo=0xd0+((z & 6) << 1)+((strlen(Asc)-2) << 1)+(z & 1);
       if (strlen(Asc)==3) ChkMaximum(true,&Result);
       *ErgSize=0; return Result;
      };

   /* 16-Bit-Teile vorheriger Bank ? */

   if (((strlen(Asc)==3) || ((strlen(Asc)==4) && (*Asc=='Q'))) && (IsQuot(Asc[strlen(Asc)-1])))
    {
     strcpy(HAsc,Asc+1); HAsc[strlen(HAsc)-1]='\0';
     for (z=0; z<(RegCnt/2); z++)
      if (strcmp(HAsc,Reg16Names[z])==0)
       {
	*ErgNo=0xd0+(z << 2)+((strlen(Asc)-3) << 1);
	if (strlen(Asc)==4) ChkMaximum(true,&Result);
	*ErgSize=1; return Result;
       }
    }

   /* 32-Bit-Register vorheriger Bank ? */

   if ((strlen(Asc)==4) && (IsQuot(Asc[3])))
    {
     strcpy(HAsc,Asc); HAsc[3]='\0';
     for (z=0; z<(RegCnt/2); z++)
      if (strcmp(HAsc,Reg32Names[z])==0)
       {
	*ErgNo=0xd0+(z << 2);
	ChkMaximum(true,&Result);
	*ErgSize=2; return Result;
       }
    }

   return (Result=0);
}

	static void ChkL(CPUVar Must, Byte *Result)
{
   if (MomCPU!=Must)
    {
     WrError(1440); *Result=0;
    }
}

	static Byte CodeCReg(char *Asc, Byte *ErgNo, Byte *ErgSize)
{
   Byte Result=2;

   if (strcasecmp(Asc,"NSP")==0)
    {
     *ErgNo=0x3c; *ErgSize=1;
     ChkL(CPU96C141,&Result);
     return Result;
    }
   if (strcasecmp(Asc,"XNSP")==0)
    {
     *ErgNo=0x3c; *ErgSize=2;
     ChkL(CPU96C141,&Result);
     return Result;
    }
   if (strcasecmp(Asc,"INTNEST")==0)
    {
     *ErgNo=0x3c; *ErgSize=1;
     ChkL(CPU93C141,&Result);
     return Result;
    }
   if ((strlen(Asc)==5) && (strncasecmp(Asc,"DMA",3)==0) && (Asc[4]>='0') && (Asc[4]<='3'))
   switch (toupper(Asc[3]))
    {
     case 'S':
      *ErgNo=(Asc[4]-'0')*4; *ErgSize=2; return Result;
     case 'D':
      *ErgNo=(Asc[4]-'0')*4+0x10; *ErgSize=2; return Result;
     case 'M':
      *ErgNo=(Asc[4]-'0')*4+0x22; *ErgSize=0; return Result;
     case 'C':
      *ErgNo=(Asc[4]-'0')*4+0x20; *ErgSize=1; return Result;
    }

   return (Result=0);
}


typedef struct
         {
          char *Name;
          Byte Num;
          bool InMax,InMin;
         } RegDesc;


	static void SetOpSize(ShortInt NewSize)
{
   if (OpSize==-1) OpSize=NewSize;
   else if (OpSize!=NewSize)
    {
     WrError(1131); AdrType=ModNone;
    }
}

	static bool IsRegCurrent(Byte No, Byte Size, Byte *Erg)
{
   switch (Size)
    {
     case 0:
      if ((No & 0xf2)==0xe0)
       {
        *Erg=((No & 0x0c) >> 1)+((No & 1) ^ 1);
        return true;
       }
      else return false;
     case 1:
     case 2:
      if ((No & 0xe3)==0xe0)
       {
        *Erg=((No & 0x1c) >> 2);
        return true;
       }
      else return false;
     default:
      return false;
    }
}

	static void ChkAdr(Byte Erl)
{
   if (AdrType!=ModNone)
    if ((Erl&(1 << AdrType))==0)
     {
      WrError(1350); AdrType=ModNone;
     }
}

	static void DecodeAdr(char *Asc, Byte Erl)
{
   String HAsc,Rest;
   Byte HNum,HSize;
   bool OK,NegFlag,NNegFlag,MustInd,FirstFlag;
   Byte BaseReg,BaseSize;
   Byte IndReg,IndSize;
   Byte PartMask;
   LongInt DispPart,DispAcc;
   char *MPos,*PPos,*EPos;

   AdrType=ModNone;

   /* Register ? */

   switch (CodeEReg(Asc,&HNum,&HSize))
    {
     case 1:
      ChkAdr(Erl); return;
     case 2:
      if (IsRegCurrent(HNum,HSize,&AdrMode)) AdrType=ModReg;
      else
       {
	AdrType=ModXReg; AdrMode=HNum;
       }
      SetOpSize(HSize);
      ChkAdr(Erl); return;
    }

   /* Steuerregister ? */

   if (CodeCReg(Asc,&HNum,&HSize)==2)
    {
     AdrType=ModCReg; AdrMode=HNum;
     SetOpSize(HSize);
     ChkAdr(Erl); return;
    }

   /* Predekrement ? */

   if ((strlen(Asc)>4) && (Asc[strlen(Asc)-1]==')') && (strncmp(Asc,"(-",2)==0))
    {
     strcpy(HAsc,Asc+2); HAsc[strlen(HAsc)-1]='\0';
     if (CodeEReg(HAsc,&HNum,&HSize)!=2) WrError(1350);
     else if (! IsRegBase(HNum,HSize)) WrError(1350);
     else
      {
       AdrType=ModMem; AdrMode=0x44;
       AdrCnt=1; AdrVals[0]=HNum; if (OpSize!=-1) AdrVals[0]+=OpSize;
      }
     ChkAdr(Erl); return;
    }

   /* Postinkrement ? */

   if ((strlen(Asc)>4) && (Asc[0]=='(') && (strncmp(Asc+strlen(Asc)-2,"+)",2)==0))
    {
     strcpy(HAsc,Asc+1); HAsc[strlen(HAsc)-2]='\0';
     if (CodeEReg(HAsc,&HNum,&HSize)!=2) WrError(1350);
     else if (! IsRegBase(HNum,HSize)) WrError(1350);
     else
      {
       AdrType=ModMem; AdrMode=0x45;
       AdrCnt=1; AdrVals[0]=HNum; if (OpSize!=-1) AdrVals[0]+=OpSize;
      }
     ChkAdr(Erl); return;
    }

   /* Speicheroperand ? */

   if (IsIndirect(Asc))
    {
     NegFlag=false; NNegFlag=false; FirstFlag=false;
     PartMask=0; DispAcc=0; BaseReg=IndReg=BaseSize=IndSize=0xff;
     strcpy(Rest,Asc+1); Rest[strlen(Rest)-1]='\0';

     do
      {
       MPos=QuotPos(Rest,'-'); PPos=QuotPos(Rest,'+');
       if ((PPos!=NULL) && ((MPos==NULL) || (PPos<MPos)))
        {
 	 EPos=PPos; NNegFlag=false;
        }
       else if ((MPos!=NULL) && ((PPos==NULL) || (MPos<PPos)))
        {
 	 EPos=MPos; NNegFlag=true;
        }
       else EPos=Rest+strlen(Rest);
       if ((EPos==Rest) || (EPos==Rest+strlen(Rest)-1))
        {
         WrError(1350); return;
        }
       strncpy(HAsc,Rest,EPos-Rest); HAsc[EPos-Rest]='\0';
       if (EPos<Rest+strlen(Rest)) strcpy(Rest,EPos+1); else *Rest='\0';

       switch (CodeEReg(HAsc,&HNum,&HSize))
        {
         case 0:
          FirstPassUnknown=false;
 	  DispPart=EvalIntExpression(HAsc,Int32,&OK);
          if (FirstPassUnknown) FirstFlag=true;
 	  if (! OK) return;
 	  if (NegFlag) DispAcc-=DispPart; else DispAcc+=DispPart;
 	  PartMask|=1;
 	  break;
         case 1:
          break;
         case 2:
          if (NegFlag)
 	   {
 	    WrError(1350); return;
 	   }
 	  else
 	   {
 	    if (HSize==0) MustInd=true;
 	    else if (HSize==2) MustInd=false;
 	    else if (! IsRegBase(HNum,HSize)) MustInd=true;
 	    else if ((PartMask & 4)!=0) MustInd=true;
 	    else MustInd=false;
 	    if (MustInd)
 	     if ((PartMask & 2)!=0)
 	      {
 	       WrError(1350); return;
 	      }
 	     else
 	      {
 	       IndReg=HNum; PartMask|=2;
 	       IndSize=HSize;
 	      }
 	    else
 	     if ((PartMask & 4)!=0)
 	      {
 	       WrError(1350); return;
 	      }
 	     else
 	      {
 	       BaseReg=HNum; PartMask|=4;
 	       BaseSize=HSize;
 	      }
 	   }
          break;
        }

       NegFlag=NNegFlag; NNegFlag=false;
      }
     while (*Rest!='\0');

     if ((DispAcc==0) && (PartMask!=1)) PartMask&=6;
     if ((PartMask==5) && (FirstFlag)) DispAcc&=0x7fff;

     switch (PartMask)
      {
       case 0:
       case 2:
       case 3:
       case 7:
        WrError(1350);
        break;
       case 1:
        if (DispAcc<=0xff)
	 {
	  AdrType=ModMem; AdrMode=0x40; AdrCnt=1;
	  AdrVals[0]=DispAcc;
	 }
        else if (DispAcc<0xffff)
	 {
	  AdrType=ModMem; AdrMode=0x41; AdrCnt=2;
	  AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
	 }
        else if (DispAcc<0xffffff)
	 {
	  AdrType=ModMem; AdrMode=0x42; AdrCnt=3;
	  AdrVals[0]=DispAcc         & 0xff;
	  AdrVals[1]=(DispAcc >>  8) & 0xff;
	  AdrVals[2]=(DispAcc >> 16) & 0xff;
	 }
        else WrError(1925);
        break;
       case 4:
        if (IsRegCurrent(BaseReg,BaseSize,&AdrMode))
	 {
	  AdrType=ModMem; AdrCnt=0;
	 }
        else
	 {
	  AdrType=ModMem; AdrMode=0x43; AdrCnt=1;
	  AdrVals[0]=BaseReg;
	 }
        break;
       case 5:
        if ((DispAcc<=127) && (DispAcc>=-128) && (IsRegCurrent(BaseReg,BaseSize,&AdrMode)))
	 {
	  AdrType=ModMem; AdrMode+=8; AdrCnt=1;
	  AdrVals[0]=DispAcc & 0xff;
	 }
        else if ((DispAcc<=32767) && (DispAcc>=-32768))
	 {
	  AdrType=ModMem; AdrMode=0x43; AdrCnt=3;
	  AdrVals[0]=BaseReg+1;
	  AdrVals[1]=DispAcc & 0xff;
	  AdrVals[2]=(DispAcc >> 8) & 0xff;
	 }
        else WrError(1320);
        break;
       case 6:
	AdrType=ModMem; AdrMode=0x43; AdrCnt=3;
	AdrVals[0]=3+(IndSize << 2);
	AdrVals[1]=BaseReg;
	AdrVals[2]=IndReg;
        break;
      }
     ChkAdr(Erl); return;
    }

   /* bleibt nur noch immediate... */

   if ((MinOneIs0) && (OpSize==-1)) OpSize=0;
   switch (OpSize)
    {
     case -1:
      WrError(1132);
      break;
     case 0:
      AdrVals[0]=EvalIntExpression(Asc,Int8,&OK);
      if (OK)
       {
	AdrType=ModImm; AdrCnt=1;
       }
      break;
     case 1:
      DispAcc=EvalIntExpression(Asc,Int16,&OK);
      if (OK)
       {
	AdrType=ModImm; AdrCnt=2;
	AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
       }
      break;
     case 2:
      DispAcc=EvalIntExpression(Asc,Int32,&OK);
      if (OK)
       {
	AdrType=ModImm; AdrCnt=4;
	AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
	AdrVals[2]=Lo(DispAcc >> 16); AdrVals[3]=Hi(DispAcc >> 16);
       }
      break;
    }
}

	static void CheckSup(void)
{
   if (MomCPU==CPU96C141)
    if (! SupAllowed) WrError(50);
}

	static bool WMemo(char *Asc_O)
{
   String Asc;

   strmaxcpy(Asc,Asc_O,255);

   if (Memo(Asc)) return true;

   strmaxcat(Asc,"W",255);
   if (Memo(Asc))
    {
     OpSize=1; return true;
    }

   Asc[strlen(Asc)-1]='L';
   if (Memo(Asc))
    {
     OpSize=2; return true;
    }

   Asc[strlen(Asc)-1]='B';
   if (Memo(Asc))
    {
     OpSize=0; return true;
    }

   return false;
}

	static bool DecodePseudo(void)
{
#define ONOFF96C1Count 2
   static ONOFFRec ONOFF96C1s[ONOFF96C1Count]=
  	        {{"MAXMODE", &Maximum   , MaximumName   },
	         {"SUPMODE", &SupAllowed, SupAllowedName}};

   if (CodeONOFF(ONOFF96C1s,ONOFF96C1Count)) return true;

   return false;
}

	static void CorrMode(Byte Ref, Byte Adr)
{
   if ((BAsmCode[Ref] & 0x4e)==0x44)
    BAsmCode[Adr]=(BAsmCode[Adr] & 0xfc) | OpSize;
}

	static bool ArgPair(const char *Val1, const char *Val2)
{
   return  (((strcasecmp(ArgStr[1],Val1)==0) && (strcasecmp(ArgStr[2],Val2)==0)) ||
	    ((strcasecmp(ArgStr[1],Val2)==0) && (strcasecmp(ArgStr[2],Val1)==0)));
}

	static LongInt ImmVal(void)
{
   LongInt tmp;

   tmp=AdrVals[0];
   if (OpSize>=1) tmp+=((LongInt)AdrVals[1]) << 8;
   if (OpSize==2)
    {
     tmp+=((LongInt)AdrVals[2]) << 16;
     tmp+=((LongInt)AdrVals[3]) << 24;
    }
   return tmp;
}

	static bool IsPwr2(LongInt Inp, Byte *Erg)
{
   LongInt Shift;

   Shift=1; *Erg=0;
   do
    {
     if (Inp==Shift) return true;
     Shift+=Shift; (*Erg)++;
    }
   while (Shift!=0);
   return false;
}

	static bool IsShort(Byte Code)
{
   return ((Code & 0x4e)==40);
}

	static bool CodeMove(void)
{
   bool ShSrc,ShDest,OK;
   Byte HReg;

   if (WMemo("LD"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem);
       switch (AdrType)
        {
         case ModReg:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem+MModImm);
	  switch (AdrType)
           {
	    case ModReg:
	     CodeLen=2;
	     BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
	     BAsmCode[1]=0x88+HReg;
	     break;
	    case ModXReg:
	     CodeLen=3;
	     BAsmCode[0]=0xc7+(OpSize << 4);
	     BAsmCode[1]=AdrMode;
	     BAsmCode[2]=0x88+HReg;
	     break;
	    case ModMem:
	     CodeLen=2+AdrCnt;
	     BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
	     memcpy(BAsmCode+1,AdrVals,AdrCnt);
	     BAsmCode[1+AdrCnt]=0x20+HReg;
	     break;
	    case ModImm:
	     if ((ImmVal()<=7) && (ImmVal()>=0))
	      {
	       CodeLen=2;
	       BAsmCode[0]=0xc8+(OpSize << 4)+HReg;
	       BAsmCode[1]=0xa8+AdrVals[0];
	      }
	     else
	      {
	       CodeLen=1+AdrCnt;
	       BAsmCode[0]=((OpSize+2) << 4)+HReg;
	       memcpy(BAsmCode+1,AdrVals,AdrCnt);
	      }
	     break;
           }
	  break;
         case ModXReg:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg+MModImm);
	  switch (AdrType)
           {
	    case ModReg:
	     CodeLen=3;
	     BAsmCode[0]=0xc7+(OpSize << 4);
	     BAsmCode[1]=HReg;
	     BAsmCode[2]=0x98+AdrMode;
	     break;
	    case ModImm:
	     if ((ImmVal()<=7) && (ImmVal()>=0))
	      {
	       CodeLen=3;
	       BAsmCode[0]=0xc7+(OpSize << 4);
	       BAsmCode[1]=HReg;
	       BAsmCode[2]=0xa8+AdrVals[0];
	      }
	     else
	      {
	       CodeLen=3+AdrCnt;
	       BAsmCode[0]=0xc7+(OpSize << 4);
	       BAsmCode[1]=HReg;
	       BAsmCode[2]=3;
	       memcpy(BAsmCode+3,AdrVals,AdrCnt);
	      }
             break;
	   }
	  break;
         case ModMem:
	  BAsmCode[0]=AdrMode;
	  HReg=AdrCnt; MinOneIs0=true;
	  memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  DecodeAdr(ArgStr[2],MModReg+MModMem+MModImm);
	  switch (AdrType)
           {
	    case ModReg:
	     CodeLen=2+HReg;
	     BAsmCode[0]+=0xb0; CorrMode(0,1);
	     BAsmCode[1+HReg]=0x40+(OpSize << 4)+AdrMode;
	     break;
	    case ModMem:
	     if (OpSize==-1) OpSize=0;
	     ShDest=IsShort(BAsmCode[0]); ShSrc=IsShort(AdrMode);

  	     if (! (ShDest || ShSrc)) WrError(1350);
	     else
	      {
	       if ((ShDest && (! ShSrc))) OK=true;
	       else if (ShSrc && (! ShDest)) OK=false;
	       else if (AdrMode==0x40) OK=true;
	       else OK=false;

	       if (OK)   /* dest=(dir8/16) */
	        {
	         CodeLen=4+AdrCnt; HReg=BAsmCode[0];
	         if (BAsmCode[0]==0x40) BAsmCode[3+AdrCnt]=0;
				   else BAsmCode[3+AdrCnt]=BAsmCode[2];
	         BAsmCode[2+AdrCnt]=BAsmCode[1];
	         BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
	         AdrMode=HReg; CorrMode(0,1);
	         memcpy(BAsmCode+1,AdrVals,AdrCnt);
	         BAsmCode[1+AdrCnt]=0x19;
	        }
	       else
	        {
	         CodeLen=4+HReg;
	         BAsmCode[2+HReg]=AdrVals[0];
	         if (AdrMode==0x40) BAsmCode[3+HReg]=0;
			       else BAsmCode[3+HReg]=AdrVals[1];
	         BAsmCode[0]+=0xb0; CorrMode(0,1);
	         BAsmCode[1+HReg]=0x14+(OpSize << 1);
	        }
	      }
	     break;
	    case ModImm:
	     if (BAsmCode[0]==0x40)
	      {
	       CodeLen=2+AdrCnt;
	       BAsmCode[0]=0x08+(OpSize << 1);
	       memcpy(BAsmCode+2,AdrVals,AdrCnt);
	      }
	     else
	      {
	       CodeLen=2+HReg+AdrCnt;
	       BAsmCode[0]+=0xb0;
	       BAsmCode[1+HReg]=OpSize << 1;
	       memcpy(BAsmCode+2+HReg,AdrVals,AdrCnt);
	      }
	     break;
           }
	  break;
        }
      }
     return true;
    }

   if ((WMemo("POP")) || (WMemo("PUSH")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"F")==0)
      {
       CodeLen=1;
       BAsmCode[0]=0x18+Memo("POP");
      }
     else if (strcasecmp(ArgStr[1],"A")==0)
      {
       CodeLen=1;
       BAsmCode[0]=0x14+Memo("POP");
      }
     else if (strcasecmp(ArgStr[1],"SR")==0)
      {
       CodeLen=1;
       BAsmCode[0]=0x02+Memo("POP");
       CheckSup();
      }
     else
      {
       MinOneIs0=true;
       DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem+
                           (WMemo("PUSH")?MModImm:0));
       switch (AdrType)
        {
         case ModReg:
	  if (OpSize==0)
	   {
	    CodeLen=2;
	    BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
	    BAsmCode[1]=0x04+Memo("POP");
	   }
	  else
	   {
	    CodeLen=1;
	    BAsmCode[0]=0x28+(Memo("POP") << 5)+((OpSize-1) << 4)+AdrMode;
	   }
          break;
         case ModXReg:
	  CodeLen=3;
	  BAsmCode[0]=0xc7+(OpSize << 4);
	  BAsmCode[1]=AdrMode;
	  BAsmCode[2]=0x04+Memo("POP");
          break;
         case ModMem:
	  if (OpSize==-1) OpSize=0;
	  CodeLen=2+AdrCnt;
	  if (strncmp(OpPart,"POP",3)==0)
	   BAsmCode[0]=0xb0+AdrMode;
	   else BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
	  memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  if (strncmp(OpPart,"POP",3)==0)
	   BAsmCode[1+AdrCnt]=0x04+(OpSize << 1);
	   else BAsmCode[1+AdrCnt]=0x04;
	  break;
         case ModImm:
          if (OpSize==-1) OpSize=0;
          BAsmCode[0]=9+(OpSize << 1);
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          CodeLen=1+AdrCnt;
          break;
        }
      }
     return true;
    }

   return false;
}

	static void MakeCode_96C141(void)
{
   Integer z;
   Word AdrWord;
   LongInt AdrLong;
   bool OK;
   Byte HReg;
   char *CmpStr;
   char LChar;

   CodeLen=0; DontPrint=false; OpSize=(-1); MinOneIs0=false;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(false)) return;

   if (CodeMove()) return;


   for (z=0; z<FixedOrderCnt; z++)
    if Memo(FixedOrders[z].Name)
     {
      if (ArgCnt!=0) WrError(1110);
      else if ((FixedOrders[z].CPUFlag & (1 << (MomCPU-CPU96C141)))==0) WrError(1500);
      else
       {
	if (Hi(FixedOrders[z].Code)==0)
	 {
	  CodeLen=1;
          BAsmCode[0]=Lo(FixedOrders[z].Code);
	 }
	else
	 {
	  CodeLen=2;
          BAsmCode[0]=Hi(FixedOrders[z].Code);
          BAsmCode[1]=Lo(FixedOrders[z].Code);
	 }
	if (FixedOrders[z].InSup) CheckSup();
       }
      return;
     }

   for (z=0; z<ImmOrderCnt; z++)
    if Memo(ImmOrders[z].Name)
     {
      if ((ArgCnt>1) || ((ImmOrders[z].Default==-1) && (ArgCnt==0))) WrError(1110);
      else
       {
	if (ArgCnt==0)
	 {
	  AdrWord=ImmOrders[z].Default; OK=true;
	 }
	else AdrWord=EvalIntExpression(ArgStr[1],Int8,&OK);
	if (OK)
	if (((Maximum) && (AdrWord>ImmOrders[z].MaxMax)) || ((! Maximum) && (AdrWord>ImmOrders[z].MinMax))) WrError(1320);
	else if (Hi(ImmOrders[z].Code)==0)
	 {
	  CodeLen=1; BAsmCode[0]=Lo(ImmOrders[z].Code)+AdrWord;
	 }
	else
	 {
	  CodeLen=2; BAsmCode[0]=Hi(ImmOrders[z].Code);
	  BAsmCode[1]=Lo(ImmOrders[z].Code)+AdrWord;
	 }
	if (ImmOrders[z].InSup) CheckSup();
       }
      return;
     }

   for (z=0; z<RegOrderCnt; z++)
    if Memo(RegOrders[z].Name)
     {
      if (ArgCnt!=1) WrError(1110);
      else
       {
	DecodeAdr(ArgStr[1],MModReg+MModXReg);
	if (AdrType!=ModNone)
	 if (((1 << OpSize) & RegOrders[z].OpMask)==0) WrError(1130);
	 else if (AdrType==ModReg)
	  {
	   BAsmCode[0]=Hi(RegOrders[z].Code)+8+(OpSize << 4)+AdrMode;
	   BAsmCode[1]=Lo(RegOrders[z].Code);
	   CodeLen=2;
	  }
	 else
	  {
	   BAsmCode[0]=Hi(RegOrders[z].Code)+7+(OpSize << 4);
	   BAsmCode[1]=AdrMode;
	   BAsmCode[2]=Lo(RegOrders[z].Code);
	   CodeLen=3;
	  }
       }
      return;
     }

   for (z=0; z<ALU2OrderCnt; z++)
    if (WMemo(ALU2Orders[z].Name))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
        DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem);
        switch (AdrType)
         {
          case ModReg:
 	   HReg=AdrMode;
 	   DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem+MModImm);
 	   switch (AdrType)
            {
 	     case ModReg:
 	      CodeLen=2;
 	      BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
 	      BAsmCode[1]=0x80+(ALU2Orders[z].Code << 4)+HReg;
 	      break;
 	     case ModXReg:
 	      CodeLen=3;
 	      BAsmCode[0]=0xc7+(OpSize << 4);
 	      BAsmCode[1]=AdrMode;
 	      BAsmCode[2]=0x80+(ALU2Orders[z].Code << 4)+HReg;
 	      break;
 	     case ModMem:
 	      CodeLen=2+AdrCnt;
 	      BAsmCode[0]=0x80+AdrMode+(OpSize << 4);
 	      memcpy(BAsmCode+1,AdrVals,AdrCnt);
 	      BAsmCode[1+AdrCnt]=0x80+HReg+(ALU2Orders[z].Code << 4);
 	      break;
 	     case ModImm:
 	      if ((ALU2Orders[z].Code==7) && (OpSize!=2) && (ImmVal()<=7) && (ImmVal()>=0))
 	       {
 	        CodeLen=2;
 	        BAsmCode[0]=0xc8+(OpSize << 4)+HReg;
 	        BAsmCode[1]=0xd8+AdrVals[0];
 	       }
 	      else
 	       {
 	        CodeLen=2+AdrCnt;
 	        BAsmCode[0]=0xc8+(OpSize << 4)+HReg;
 	        BAsmCode[1]=0xc8+ALU2Orders[z].Code;
 	        memcpy(BAsmCode+2,AdrVals,AdrCnt);
 	       }
              break;
 	    }
 	   break;
          case ModXReg:
 	   HReg=AdrMode;
 	   DecodeAdr(ArgStr[2],MModImm);
 	   switch (AdrType)
            {
 	     case ModImm:
 	      if ((ALU2Orders[z].Code==7) && (OpSize!=2) && (ImmVal()<=7) && (ImmVal()>=0))
 	       {
 	        CodeLen=3;
 	        BAsmCode[0]=0xc7+(OpSize << 4);
 	        BAsmCode[1]=HReg;
 	        BAsmCode[2]=0xd8+AdrVals[0];
 	       }
 	      else
 	       {
 	        CodeLen=3+AdrCnt;
 	        BAsmCode[0]=0xc7+(OpSize << 4);
 	        BAsmCode[1]=HReg;
 	        BAsmCode[2]=0xc8+ALU2Orders[z].Code;
 	        memcpy(BAsmCode+3,AdrVals,AdrCnt);
 	       }
              break;
 	    }
 	   break;
          case ModMem:
 	   MinOneIs0=true;
 	   HReg=AdrCnt; BAsmCode[0]=AdrMode;
 	   memcpy(BAsmCode+1,AdrVals,AdrCnt);
 	   DecodeAdr(ArgStr[2],MModReg+MModImm);
 	   switch (AdrType)
            {
 	     case ModReg:
 	      CodeLen=2+HReg; CorrMode(0,1);
 	      BAsmCode[0]+=0x80+(OpSize << 4);
 	      BAsmCode[1+HReg]=0x88+(ALU2Orders[z].Code << 4)+AdrMode;
 	      break;
 	     case ModImm:
 	      CodeLen=2+HReg+AdrCnt;
 	      BAsmCode[0]+=0x80+(OpSize << 4);
 	      BAsmCode[1+HReg]=0x38+ALU2Orders[z].Code;
 	      memcpy(BAsmCode+2+HReg,AdrVals,AdrCnt);
 	      break;
 	    };
 	   break;
         }
       }
      return;
     }

   for (z=0; z<ShiftOrderCnt; z++)
    if (WMemo(ShiftOrders[z]))
     {
      if ((ArgCnt!=1) && (ArgCnt!=2))  WrError(1110);
      else
       {
	OK=true;
	if (ArgCnt==1) HReg=1;
	else if (strcasecmp(ArgStr[1],"A")==0) HReg=0xff;
	else
	 {
          FirstPassUnknown=false;
	  HReg=EvalIntExpression(ArgStr[1],Int8,&OK);
	  if (OK)
	   if (FirstPassUnknown) HReg&=0x0f;
           else
            if ((HReg==0) || (HReg>16))
             {
              WrError(1320); OK=false;
             }
            else HReg&=0x0f;
	 }
	if (OK)
	 {
	  DecodeAdr(ArgStr[ArgCnt],MModReg+MModXReg+((HReg==0xff)?0:MModMem));
	  switch (AdrType)
           {
	    case ModReg:
	     CodeLen=2+(HReg!=0xff);
	     BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
	     BAsmCode[1]=0xe8+z;
	     if (HReg==0xff) BAsmCode[1]+=0x10;
	     else BAsmCode[2]=HReg;
	     break;
	    case ModXReg:
	     CodeLen=3+(HReg!=0xff);
	     BAsmCode[0]=0xc7+(OpSize << 4);
	     BAsmCode[1]=AdrMode;
	     BAsmCode[2]=0xe8+z;
	     if (HReg==0xff) BAsmCode[2]+=0x10;
	     else BAsmCode[3]=HReg;
	     break;
	    case ModMem:
	     if (HReg!=1) WrError(1350);
	     else
	      {
	       if (OpSize==-1) OpSize=0;
	       CodeLen=2+AdrCnt;
	       BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
	       memcpy(BAsmCode+1,AdrVals,AdrCnt);
	       BAsmCode[1+AdrCnt]=0x78+z;
	      }
             break;
	   }
	 }
       }
      return;
     }

   for (z=0;z<MulDivOrderCnt; z++)
    if (Memo(MulDivOrders[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
	DecodeAdr(ArgStr[1],MModReg+MModXReg);
	if (OpSize==0) WrError(1130);
	else
	 {
	  if ((AdrType==ModReg) && (OpSize==1))
	   if (AdrMode>3)
	    {
	     AdrType=ModXReg; AdrMode=0xe0+(AdrMode << 2);
	    }
	   else AdrMode+=1+AdrMode;
	  OpSize--;
	  HReg=AdrMode;
	  switch (AdrType)
           {
	    case ModReg:
	     DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem+MModImm);
	     switch (AdrType)
              {
	       case ModReg:
	        CodeLen=2;
	        BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
	        BAsmCode[1]=0x40+(z << 3)+HReg;
	        break;
	       case ModXReg:
	        CodeLen=3;
	        BAsmCode[0]=0xc7+(OpSize << 4);
	        BAsmCode[1]=AdrMode;
	        BAsmCode[2]=0x40+(z << 3)+HReg;
	        break;
	       case ModMem:
	        CodeLen=2+AdrCnt;
	        BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
	        memcpy(BAsmCode+1,AdrVals,AdrCnt);
	        BAsmCode[1+AdrCnt]=0x40+(z << 3)+HReg;
	        break;
	       case ModImm:
	        CodeLen=2+AdrCnt;
	        BAsmCode[0]=0xc8+(OpSize << 4)+HReg;
	        BAsmCode[1]=0x08+z;
	        memcpy(BAsmCode+2,AdrVals,AdrCnt);
	        break;
	      }
	     break;
	    case ModXReg:
	     DecodeAdr(ArgStr[2],MModImm);
	     if (AdrType==ModImm)
	      {
	       CodeLen=3+AdrCnt;
	       BAsmCode[0]=0xc7+(OpSize << 4);
	       BAsmCode[1]=HReg;
	       BAsmCode[2]=0x08+z;
	       memcpy(BAsmCode+3,AdrVals,AdrCnt);
	      }
	     break;
	   }
	 }
       }
      return;
     }

   if (Memo("MULA"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg+MModXReg);
       if ((AdrType!=ModNone) && (OpSize!=2)) WrError(1130);
       else
        switch (AdrType)
         {
          case ModReg:
	   CodeLen=2;
	   BAsmCode[0]=0xd8+AdrMode;
	   BAsmCode[1]=0x19;
	   break;
          case ModXReg:
	   CodeLen=3;
	   BAsmCode[0]=0xd7;
	   BAsmCode[1]=AdrMode;
	   BAsmCode[2]=0x19;
	   break;
         }
      }
     return;
    }

   for (z=0; z<BitCFOrderCnt; z++)
    if Memo(BitCFOrders[z].Name)
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
	DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem);
	if (AdrType!=ModNone)
	 if (OpSize==2) WrError(1130);
	 else
	  {
	   if (AdrType==ModMem) OpSize=0;
	   if (strcasecmp(ArgStr[1],"A")==0)
	    {
	     HReg=0xff; OK=true;
	    }
	   else
	    {
	     FirstPassUnknown=false;
             HReg=EvalIntExpression(ArgStr[1],(OpSize==0)?UInt3:Int4,&OK);
	    }
	   if (OK)
	    switch (AdrType)
	     {
              case ModReg:
	       CodeLen=2+(HReg!=0xff);
	       BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
	       BAsmCode[1]=0x20+((HReg==0xff) << 3)+BitCFOrders[z].Code;
	       if (HReg!=0xff) BAsmCode[2]=HReg;
	       break;
	      case ModXReg:
	       CodeLen=3+(HReg!=0xff);
	       BAsmCode[0]=0xc7+(OpSize << 4);
	       BAsmCode[1]=AdrMode;
	       BAsmCode[2]=0x20+((HReg==0xff) << 3)+BitCFOrders[z].Code;
	       if (HReg!=0xff) BAsmCode[3]=HReg;
	       break;
	      case ModMem:
	       CodeLen=2+AdrCnt;
	       BAsmCode[0]=0xb0+AdrMode;
	       memcpy(BAsmCode+1,AdrVals,AdrCnt);
	       if (HReg==0xff) BAsmCode[1+AdrCnt]=0x28+BitCFOrders[z].Code;
	       else BAsmCode[1+AdrCnt]=0x80+(BitCFOrders[z].Code << 3)+HReg;
	       break;
	     }
	  }
       }
      return;
     }

   for (z=0; z<BitOrderCnt; z++)
    if (Memo(BitOrders[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
	DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem);
	if (AdrType==ModMem) OpSize=0;
	if (AdrType!=ModNone)
	 if (OpSize==2) WrError(1130);
	 else
	  {
           HReg=EvalIntExpression(ArgStr[1],(OpSize==0)?UInt3:Int4,&OK);
	   if (OK)
	    switch (AdrType)
             {
	      case ModReg:
	       CodeLen=3;
	       BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
	       BAsmCode[1]=0x30+z;
	       BAsmCode[2]=HReg;
	       break;
	      case ModXReg:
	       CodeLen=4;
	       BAsmCode[0]=0xc7+(OpSize << 4);
	       BAsmCode[1]=AdrMode;
	       BAsmCode[2]=0x30+z;
	       BAsmCode[3]=HReg;
	       break;
	      case ModMem:
	       CodeLen=2+AdrCnt;
	       if (z==4) z=0; else z++;
	       BAsmCode[0]=0xb0+AdrMode;
	       memcpy(BAsmCode+1,AdrVals,AdrCnt);
	       BAsmCode[1+AdrCnt]=0xa8+(z << 3)+HReg;
	       break;
	     }
	  }
       }
      return;
     }

   if ((Memo("CALL")) || (Memo("JP")))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else
      {
       if (ArgCnt==1)  z=DefaultCondition;
       else
	{
	 z=0; NLS_UpString(ArgStr[1]);
	 while ((z<ConditionCnt) && (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
	}
       if (z>=ConditionCnt) WrError(1360);
       else
	{
	 OpSize=2;
	 DecodeAdr(ArgStr[ArgCnt],MModMem+MModImm);
	 if (AdrType==ModImm)
	  if (AdrVals[3]!=0)
	   {
	    WrError(1320); AdrType=ModNone;
	   }
	  else if (AdrVals[2]!=0)
	   {
	    AdrType=ModMem; AdrMode=0x42; AdrCnt=3;
	   }
	  else
	   {
	    AdrType=ModMem; AdrMode=0x41; AdrCnt=2;
	   }
	 if (AdrType==ModMem)
	  if ((z==DefaultCondition) && ((AdrMode==0x41) || (AdrMode==0x42)))
	   {
	    CodeLen=1+AdrCnt;
	    BAsmCode[0]=0x1a+2*Memo("CALL")+(AdrCnt-2);
	    memcpy(BAsmCode+1,AdrVals,AdrCnt);
	   }
	  else
	   {
	    CodeLen=2+AdrCnt;
	    BAsmCode[0]=0xb0+AdrMode;
	    memcpy(BAsmCode+1,AdrVals,AdrCnt);
	    BAsmCode[1+AdrCnt]=0xd0+(Memo("CALL") << 4)+(Conditions[z].Code);
	   }
	}
      }
     return;
    }

   if ((Memo("JR")) || (Memo("JRL")))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else
      {
       if (ArgCnt==1) z=DefaultCondition;
       else
	{
	 z=0; NLS_UpString(ArgStr[1]);
	 while ((z<ConditionCnt) && (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
	}
       if (z>=ConditionCnt) WrError(1360);
       else
	{
	 AdrLong=EvalIntExpression(ArgStr[ArgCnt],Int32,&OK);
	 if (OK)
	  if (Memo("JRL"))
	   {
	    AdrLong-=EProgCounter()+3;
            if (((AdrLong>0x7fffl) || (AdrLong<-0x8000l)) && (! SymbolQuestionable)) WrError(1330);
	    else
	     {
	      CodeLen=3; BAsmCode[0]=0x70+Conditions[z].Code;
	      BAsmCode[1]=Lo(AdrLong); BAsmCode[2]=Hi(AdrLong);
	      if (! FirstPassUnknown)
	       {
		AdrLong++;
		if ((AdrLong>=-128) && (AdrLong<=127)) WrError(20);
	       }
	     }
	   }
	  else
	   {
	    AdrLong-=EProgCounter()+2;
            if (((AdrLong>127) || (AdrLong<-128)) && (! SymbolQuestionable)) WrError(1330);
	    else
	     {
	      CodeLen=2; BAsmCode[0]=0x60+Conditions[z].Code;
	      BAsmCode[1]=Lo(AdrLong);
	     }
	   }
	}
      }
     return;
    }

   if (Memo("CALR"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrLong=EvalIntExpression(ArgStr[1],Int32,&OK)-(EProgCounter()+3);
       if (OK)
        if (((AdrLong<-32768) || (AdrLong>32767)) && (! SymbolQuestionable)) WrError(1330);
	else
	 {
	  CodeLen=3; BAsmCode[0]=0x1e;
	  BAsmCode[1]=Lo(AdrLong); BAsmCode[2]=Hi(AdrLong);
	 }
      }
     return;
    }

   if (Memo("RET"))
    {
     if (ArgCnt>1) WrError(1110);
     else
      {
       if (ArgCnt==0) z=DefaultCondition;
       else
	{
	 z=0; NLS_UpString(ArgStr[1]);
	 while ((z<ConditionCnt) && (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
	}
       if (z>=ConditionCnt) WrError(1360);
       else if (z==DefaultCondition)
	{
	 CodeLen=1; BAsmCode[0]=0x0e;
	}
       else
	{
	 CodeLen=2; BAsmCode[0]=0xb0;
	 BAsmCode[1]=0xf0+Conditions[z].Code;
	}
      }
     return;
    }

   if (Memo("RETD"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
	{
	 CodeLen=3; BAsmCode[0]=0x0f;
	 BAsmCode[1]=Lo(AdrWord); BAsmCode[2]=Hi(AdrWord);
	}
      }
     return;
    }

   if (Memo("DJNZ"))
    {
     if ((ArgCnt!=2) && (ArgCnt!=1)) WrError(1110);
     else
      {
       if (ArgCnt==1)
	{
	 AdrType=ModReg; AdrMode=2; OpSize=0;
	}
       else DecodeAdr(ArgStr[1],MModReg+MModXReg);
       if (AdrType!=ModNone)
	if (OpSize==2) WrError(1130);
	else
	 {
	  AdrLong=EvalIntExpression(ArgStr[ArgCnt],Int32,&OK)-(EProgCounter()+3+(AdrType==ModXReg));
	  if (OK)
           if (((AdrLong<-128) || (AdrLong>127)) && (! SymbolQuestionable)) WrError(1370);
	   else
            switch (AdrType)
             {
	      case ModReg:
  	       CodeLen=3;
	       BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
	       BAsmCode[1]=0x1c;
	       BAsmCode[2]=AdrLong & 0xff;
	       break;
	      case ModXReg:
	       CodeLen=4;
	       BAsmCode[0]=0xc7+(OpSize << 4);
	       BAsmCode[1]=AdrMode;
	       BAsmCode[2]=0x1c;
	       BAsmCode[3]=AdrLong & 0xff;
	       break;
	     }
	 }
      }
     return;
    }

   if (Memo("EX"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if ((ArgPair("F","F\'")) || (ArgPair("F`","F")))
      {
       CodeLen=1; BAsmCode[0]=0x16;
      }
     else
      {
       DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem);
       if (OpSize==2) WrError(1130);
       else
        switch (AdrType)
         {
          case ModReg:
  	   HReg=AdrMode;
	   DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem);
	   switch (AdrType)
            {
	     case ModReg:
	      CodeLen=2;
	      BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
	      BAsmCode[1]=0xb8+HReg;
	      break;
	     case ModXReg:
	      CodeLen=3;
	      BAsmCode[0]=0xc7+(OpSize << 4);
	      BAsmCode[1]=AdrMode;
	      BAsmCode[2]=0xb8+HReg;
	      break;
	     case ModMem:
	      CodeLen=2+AdrCnt;
	      BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
	      memcpy(BAsmCode+1,AdrVals,AdrCnt);
	      BAsmCode[1+AdrCnt]=0x30+HReg;
	      break;
	    }
	   break;
          case ModXReg:
	   HReg=AdrMode;
	   DecodeAdr(ArgStr[2],MModReg);
	   if (AdrType==ModReg)
	    {
	     CodeLen=3;
	     BAsmCode[0]=0xc7+(OpSize << 4);
	     BAsmCode[1]=HReg;
	     BAsmCode[2]=0xb8+AdrMode;
	    }
	   break;
          case ModMem:
	   MinOneIs0=true;
	   HReg=AdrCnt; BAsmCode[0]=AdrMode;
	   memcpy(BAsmCode+1,AdrVals,AdrCnt);
	   DecodeAdr(ArgStr[2],MModReg);
	   if (AdrType==ModReg)
	    {
	     CodeLen=2+HReg; CorrMode(0,1);
	     BAsmCode[0]+=0x80+(OpSize << 4);
	     BAsmCode[1+HReg]=0x30+AdrMode;
	    }
	   break;
         }
      }
     return;
    }

   if ((WMemo("INC")) || (WMemo("DEC")))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else
      {
       FirstPassUnknown=false;
       if (ArgCnt==1)
	{
	 HReg=1; OK=true;
	}
       else HReg=EvalIntExpression(ArgStr[1],Int4,&OK);
       if (OK)
	if (FirstPassUnknown) HReg&=7;
	else if ((HReg<1) || (HReg>8))
	 {
	  WrError(1320); OK=false;
	 }
       if (OK)
	{
	 HReg&=7;    /* 8-->0 */
	 DecodeAdr(ArgStr[ArgCnt],MModReg+MModXReg+MModMem);
	 switch (AdrType)
          {
	   case ModReg:
	    CodeLen=2;
	    BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
            BAsmCode[1]=0x60+(WMemo("DEC") << 3)+HReg;
	    break;
	   case ModXReg:
	    CodeLen=3;
	    BAsmCode[0]=0xc7+(OpSize << 4);
	    BAsmCode[1]=AdrMode;
            BAsmCode[2]=0x60+(WMemo("DEC") << 3)+HReg;
	    break;
	   case ModMem:
            if (OpSize==-1) OpSize=0;
	    CodeLen=2+AdrCnt;
            BAsmCode[0]=0x80+AdrMode+(OpSize << 4);
	    memcpy(BAsmCode+1,AdrVals,AdrCnt);
            BAsmCode[1+AdrCnt]=0x60+(WMemo("DEC") << 3)+HReg;
	    break;
	  }
	}
      }
     return;
    }

   if ((Memo("BS1B")) || (Memo("BS1F")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1135);
     else
      {
       DecodeAdr(ArgStr[2],MModReg+MModXReg);
       if (OpSize!=1) WrError(1130);
       else
        switch (AdrType)
         {
          case ModReg:
  	   CodeLen=2;
	   BAsmCode[0]=0xd8+AdrMode;
	   BAsmCode[1]=0x0e +Memo("BS1B");
	   break;
          case ModXReg:
	   CodeLen=3;
	   BAsmCode[0]=0xd7;
	   BAsmCode[1]=AdrMode;
	   BAsmCode[2]=0x0e +Memo("BS1B");
	   break;
         };
      }
     return;
    }

   if ((Memo("CPD")) || (Memo("CPDR")) || (Memo("CPI")) || (Memo("CPIR")))
    {
     if ((ArgCnt!=0) && (ArgCnt!=2)) WrError(1110);
     else
      {
       if (ArgCnt==0)
	{
	 OK=true; OpSize=0; AdrMode=3;
	}
       else
	{
	 OK=true;
	 if (strcasecmp(ArgStr[1],"A")==0) OpSize=0;
	 else if (strcasecmp(ArgStr[1],"WA")==0) OpSize=1;
	 if (OpPart[2]=='I') CmpStr="+)"; else CmpStr="-)";
	 if (OpSize==-1) OK=false;
	 else if ((*ArgStr[2]!='(') || (strcasecmp(ArgStr[2]+strlen(ArgStr[2])-2,CmpStr)!=0)) OK=false;
	 else
          {
           ArgStr[2][strlen(ArgStr[2])-2]='\0';
           if (CodeEReg(ArgStr[2]+1,&AdrMode,&HReg)!=2) OK=false;
	   else if (! IsRegBase(AdrMode,HReg)) OK=false;
	   else if (! IsRegCurrent(AdrMode,HReg,&AdrMode)) OK=false;
          }
	 if (! OK) WrError(1135);
	}
       if (OK)
	{
	 CodeLen=2;
	 BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
	 BAsmCode[1]=0x14+((OpPart[2]=='D') << 1)+(strlen(OpPart)-3);
	}
      }
     return;
    }

   if ((WMemo("LDD")) || (WMemo("LDDR")) || (WMemo("LDI")) || (WMemo("LDIR")))
    {
     if (OpSize==-1) OpSize=0;
     if (OpSize==2) WrError(1130);
     else if ((ArgCnt!=0) && (ArgCnt!=2)) WrError(1110);
     else
      {
       if (ArgCnt==0)
	{
	 OK=true; HReg=0;
	}
       else
	{
	 OK=true;
	 if (OpPart[2]=='I') CmpStr="+)"; else CmpStr="-)";
	 if ((*ArgStr[1]!='(') || (*ArgStr[2]!='(') ||
	     (strcasecmp(ArgStr[1]+strlen(ArgStr[1])-2,CmpStr)!=0) ||
	     (strcasecmp(ArgStr[2]+strlen(ArgStr[2])-2,CmpStr)!=0))  OK=false;
	 else
	  {
           strcpy(ArgStr[1],ArgStr[1]+1); ArgStr[1][strlen(ArgStr[1])-2]='\0';
           strcpy(ArgStr[2],ArgStr[2]+1); ArgStr[2][strlen(ArgStr[2])-2]='\0';
	   if ((strcasecmp(ArgStr[1],"XIX")==0) && (strcasecmp(ArgStr[2],"XIY")==0)) HReg=2;
	   else if ((Maximum) && (strcasecmp(ArgStr[1],"XDE")==0) && (strcasecmp(ArgStr[2],"XHL")==0)) HReg=0;
	   else if ((! Maximum) && (strcasecmp(ArgStr[1],"DE")==0) && (strcasecmp(ArgStr[2],"HL")==0)) HReg=0;
	   else OK=false;
	  }
	}
       if (! OK) WrError(1350);
       else
	{
	 CodeLen=2;
	 BAsmCode[0]=0x83+(OpSize << 4)+HReg;
	 BAsmCode[1]=0x10+((OpPart[2]=='D') << 1)+(strchr(OpPart,'R')!=NULL);
	}
      }
     return;
    }

   if (Memo("LDA"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg);
       if (AdrType!=ModNone)
	if (OpSize<1) WrError(1130);
	else
	 {
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModMem);
	  if (AdrType!=ModNone)
	   {
	    CodeLen=2+AdrCnt;
	    BAsmCode[0]=0xb0+AdrMode;
	    memcpy(BAsmCode+1,AdrVals,AdrCnt);
	    BAsmCode[1+AdrCnt]=0x20+((OpSize-1) << 4)+HReg;
	   }
	 }
      }
     return;
    }

   if (Memo("LDAR"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       AdrLong=EvalIntExpression(ArgStr[2],Int32,&OK)-(EProgCounter()+4);
       if (OK)
        if (((AdrLong<-32768) || (AdrLong>32767)) && (! SymbolQuestionable)) WrError(1330);
	else
	 {
	  DecodeAdr(ArgStr[1],MModReg);
	  if (AdrType!=ModNone)
	   if (OpSize<1) WrError(1130);
	   else
	    {
	     CodeLen=5;
	     BAsmCode[0]=0xf3; BAsmCode[1]=0x13;
	     BAsmCode[2]=Lo(AdrLong); BAsmCode[3]=Hi(AdrLong);
	     BAsmCode[4]=0x20+((OpSize-1) << 4)+AdrMode;
	    }
	 }
      }
     return;
    }

   if (Memo("LDC"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg+MModXReg+MModCReg);
       HReg=AdrMode;
       switch (AdrType)
        {
         case ModReg:
	  DecodeAdr(ArgStr[2],MModCReg);
	  if (AdrType!=ModNone)
	   {
	    CodeLen=3;
	    BAsmCode[0]=0xc8+(OpSize << 4)+HReg;
	    BAsmCode[1]=0x2f;
	    BAsmCode[2]=AdrMode;
	   }
	  break;
         case ModXReg:
	  DecodeAdr(ArgStr[2],MModCReg);
	  if (AdrType!=ModNone)
	   {
	    CodeLen=4;
	    BAsmCode[0]=0xc7+(OpSize << 4);
	    BAsmCode[1]=HReg;
	    BAsmCode[2]=0x2f;
	    BAsmCode[3]=AdrMode;
	   };
	  break;
         case ModCReg:
	  DecodeAdr(ArgStr[2],MModReg+MModXReg);
	  switch (AdrType)
           {
	    case ModReg:
	     CodeLen=3;
	     BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
	     BAsmCode[1]=0x2e;
	     BAsmCode[2]=HReg;
	     break;
	    case ModXReg:
	     CodeLen=4;
	     BAsmCode[0]=0xc7+(OpSize << 4);
	     BAsmCode[1]=AdrMode;
	     BAsmCode[2]=0x2e;
	     BAsmCode[3]=HReg;
	     break;
	   }
	  break;
        }
      }
     return;
    }

   if (Memo("LDX"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModMem);
       if (AdrType!=ModNone)
	if (AdrMode!=0x40) WrError(1350);
	else
	 {
	  BAsmCode[4]=EvalIntExpression(ArgStr[2],Int8,&OK);
	  if (OK)
	   {
	    CodeLen=6;
	    BAsmCode[0]=0xf7; BAsmCode[1]=0;
	    BAsmCode[2]=AdrVals[0]; BAsmCode[3]=0;
	    BAsmCode[5]=0;
	   }
	 }
      }
     return;
    }

   if (Memo("LINK"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       AdrWord=EvalIntExpression(ArgStr[2],Int16,&OK);
       if (OK)
	{
	 DecodeAdr(ArgStr[1],MModReg+MModXReg);
	 if ((AdrType!=ModNone) && (OpSize!=2)) WrError(1130);
	 else
          switch (AdrType)
           {
	    case ModReg:
	     CodeLen=4;
	     BAsmCode[0]=0xe8+AdrMode;
	     BAsmCode[1]=0x0c;
	     BAsmCode[2]=Lo(AdrWord);
	     BAsmCode[3]=Hi(AdrWord);
	     break;
	    case ModXReg:
	     CodeLen=5;
	     BAsmCode[0]=0xe7;
	     BAsmCode[1]=AdrMode;
	     BAsmCode[2]=0x0c;
	     BAsmCode[3]=Lo(AdrWord);
	     BAsmCode[4]=Hi(AdrWord);
	     break;
	   }
	}
      }
     return;
    }

   LChar=OpPart[strlen(OpPart)-1];
   if (((strncmp(OpPart,"MDEC",4)==0) || (strncmp(OpPart,"MINC",4)==0)) && (LChar>='1') && (LChar<='4'))
    {
     if (LChar=='3') WrError(1135);
     else if (ArgCnt!=2) WrError(1110);
     else
      {
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
	if (! IsPwr2(AdrWord,&HReg)) WrError(1135);
	else if ((HReg==0) || ((LChar=='2') && (HReg<2)) || ((LChar=='4') && (HReg<3))) WrError(1135);
	else
	 {
	  AdrWord-=LChar-'0';
	  IsPwr2(LChar-'0',&HReg);
	  DecodeAdr(ArgStr[2],MModReg+MModXReg);
	  if ((AdrType!=ModNone) && (OpSize!=1)) WrError(1130);
	  else
           switch (AdrType)
            {
	     case ModReg:
	      CodeLen=4;
	      BAsmCode[0]=0xd8+AdrMode;
	      BAsmCode[1]=0x38+((OpPart[2]=='D') << 2)+HReg;
	      BAsmCode[2]=Lo(AdrWord);
	      BAsmCode[3]=Hi(AdrWord);
	      break;
	     case ModXReg:
	      CodeLen=5;
	      BAsmCode[0]=0xd7;
	      BAsmCode[1]=AdrMode;
	      BAsmCode[2]=0x38+((OpPart[2]=='D') << 2)+HReg;
	      BAsmCode[3]=Lo(AdrWord);
	      BAsmCode[4]=Hi(AdrWord);
	      break;
	    }
	 }
      }
     return;
    }

   if ((Memo("RLD")) || (Memo("RRD")))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else if ((ArgCnt==2) && (strcasecmp(ArgStr[1],"A")!=0)) WrError(1350);
     else
      {
       DecodeAdr(ArgStr[ArgCnt],MModMem);
       if (AdrType!=ModNone)
	{
	 CodeLen=2+AdrCnt;
	 BAsmCode[0]=0x80+AdrMode;
	 memcpy(BAsmCode+1,AdrVals,AdrCnt);
	 BAsmCode[1+AdrCnt]=0x06+Memo("RRD");
	}
      }
     return;
    }

   if (Memo("SCC"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       z=0; NLS_UpString(ArgStr[1]);
       while ((z<ConditionCnt) && (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
       if (z>=ConditionCnt) WrError(1360);
       else
	{
	 DecodeAdr(ArgStr[2],MModReg+MModXReg);
	 if (OpSize>1) WrError(1110);
	 else
          switch (AdrType)
           {
	    case ModReg:
  	     CodeLen=2;
	     BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
	     BAsmCode[1]=0x70+Conditions[z].Code;
	     break;
	    case ModXReg:
	     CodeLen=3;
	     BAsmCode[0]=0xc7+(OpSize << 4);
	     BAsmCode[1]=AdrMode;
	     BAsmCode[2]=0x70+Conditions[z].Code;
	     break;
	   }
	}
      }
     return;
    }

   WrXError(1200,OpPart);
}

	static bool ChkPC_96C141(void)
{
   bool ok;

   switch (ActPC)
    {
     case SegCode:
      if (Maximum) ok=(ProgCounter()<=0xffffff);
	      else ok=(ProgCounter()<=0xffff);
      break;
     default:
      ok=false;
    }
   return (ok);
}


	static bool IsDef_96C141(void)
{
   return false;
}

        static void SwitchFrom_96C141(void)
{
   DeinitFields();
}

	static void SwitchTo_96C141(void)
{
   TurnWords=false; ConstMode=ConstModeIntel; SetIsOccupied=true;

   PCSymbol="$"; HeaderID=0x52; NOPCode=0x00;
   DivideChars=","; HasAttrs=false;

   ValidSegs=(1<<SegCode);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_96C141; ChkPC=ChkPC_96C141; IsDef=IsDef_96C141;
   SwitchFrom=SwitchFrom_96C141;

   InitFields();
}

	void code96c141_init(void)
{
   CPU96C141=AddCPU("96C141",SwitchTo_96C141);
   CPU93C141=AddCPU("93C141",SwitchTo_96C141);
}
