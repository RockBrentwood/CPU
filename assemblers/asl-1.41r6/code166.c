/* code166.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegenerator Siemens 80C16x                                           */
/*                                                                           */
/* Historie: 11.11.1996 (alaaf) Grundsteinlegung                             */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "stringutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct
         {
          char *Name;
          CPUVar MinCPU;
          Word Code1,Code2;
         } BaseOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } SimpOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } Condition;


#define FixedOrderCount 10
#define ConditionCount 20
#define ALU2OrderCount 8
#define ShiftOrderCount 5
#define Bit2OrderCount 6
#define LoopOrderCount 4
#define DivOrderCount 4
#define BJmpOrderCount 4
#define MulOrderCount 3

#define DPPCount 4
static char *RegNames[6]={"DPP0","DPP1","DPP2","DPP3","CP","SP"};

static CPUVar CPU80C166,CPU80C167;

static BaseOrder *FixedOrders;
static Condition *Conditions;
static int TrueCond;
static char **ALU2Orders;
static SimpOrder *ShiftOrders;
static SimpOrder *Bit2Orders;
static SimpOrder *LoopOrders;
static char **DivOrders;
static char **BJmpOrders;
static char **MulOrders;

static SimpProc SaveInitProc;
static LongInt DPPAssumes[DPPCount];
static IntType MemInt,MemInt2;
static Byte OpSize;

static bool DPPChanged[DPPCount],N_DPPChanged[DPPCount];
static bool SPChanged,CPChanged,N_SPChanged,N_CPChanged;

static ShortInt ExtCounter;
static enum {MemModeStd,MemModeNoCheck,MemModeZeroPage,MemModeFixedBank,MemModeFixedPage} MemMode;
	     /* normal    EXTS Rn        EXTP Rn         EXTS nn          EXTP nn        */
static Word MemPage;
static bool ExtSFRs;

/*-------------------------------------------------------------------------*/

	static void AddFixed(char *NName, CPUVar NMin, Word NCode1, Word NCode2)
{
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].MinCPU=NMin;
   FixedOrders[InstrZ].Code1=NCode1;
   FixedOrders[InstrZ++].Code2=NCode2;
}

	static void AddShift(char *NName, Byte NCode)
{
   if (InstrZ>=ShiftOrderCount) exit(255);
   ShiftOrders[InstrZ].Name=NName;
   ShiftOrders[InstrZ++].Code=NCode;
}

	static void AddBit2(char *NName, Byte NCode)
{
   if (InstrZ>=Bit2OrderCount) exit(255);
   Bit2Orders[InstrZ].Name=NName;
   Bit2Orders[InstrZ++].Code=NCode;
}

	static void AddLoop(char *NName, Byte NCode)
{
   if (InstrZ>=LoopOrderCount) exit(255);
   LoopOrders[InstrZ].Name=NName;
   LoopOrders[InstrZ++].Code=NCode;
}

	static void AddCondition(char *NName, Byte NCode)
{
   if (InstrZ>=ConditionCount) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
}

	static void InitFields(void)
{
   FixedOrders=(BaseOrder *) malloc(FixedOrderCount*sizeof(BaseOrder)); InstrZ=0;
   AddFixed("DISWDT",CPU80C166,0x5aa5,0xa5a5);
   AddFixed("EINIT" ,CPU80C166,0x4ab5,0xb5b5);
   AddFixed("IDLE"  ,CPU80C166,0x7887,0x8787);
   AddFixed("NOP"   ,CPU80C166,0x00cc,0x0000);
   AddFixed("PWRDN" ,CPU80C166,0x6897,0x9797);
   AddFixed("RET"   ,CPU80C166,0x00cb,0x0000);
   AddFixed("RETI"  ,CPU80C166,0x88fb,0x0000);
   AddFixed("RETS"  ,CPU80C166,0x00db,0x0000);
   AddFixed("SRST"  ,CPU80C166,0x48b7,0xb7b7);
   AddFixed("SRVWDT",CPU80C166,0x58a7,0xa7a7);

   Conditions=(Condition *) malloc(sizeof(Condition)*ConditionCount); InstrZ=0;
   TrueCond=InstrZ; AddCondition("UC" ,0x0); AddCondition("Z"  ,0x2);
   AddCondition("NZ" ,0x3); AddCondition("V"  ,0x4);
   AddCondition("NV" ,0x5); AddCondition("N"  ,0x6);
   AddCondition("NN" ,0x7); AddCondition("C"  ,0x8);
   AddCondition("NC" ,0x9); AddCondition("EQ" ,0x2);
   AddCondition("NE" ,0x3); AddCondition("ULT",0x8);
   AddCondition("ULE",0xf); AddCondition("UGE",0x9);
   AddCondition("UGT",0xe); AddCondition("SLT",0xc);
   AddCondition("SLE",0xb); AddCondition("SGE",0xd);
   AddCondition("SGT",0xa); AddCondition("NET",0x1);

   ALU2Orders=(char **) malloc(sizeof(char *)*ALU2OrderCount); InstrZ=0;
   ALU2Orders[InstrZ++]="ADD" ; ALU2Orders[InstrZ++]="ADDC";
   ALU2Orders[InstrZ++]="SUB" ; ALU2Orders[InstrZ++]="SUBC";
   ALU2Orders[InstrZ++]="CMP" ; ALU2Orders[InstrZ++]="XOR" ;
   ALU2Orders[InstrZ++]="AND" ; ALU2Orders[InstrZ++]="OR"  ;

   ShiftOrders=(SimpOrder *) malloc(sizeof(SimpOrder)*ShiftOrderCount); InstrZ=0;
   AddShift("ASHR",0xac); AddShift("ROL" ,0x0c);
   AddShift("ROR" ,0x2c); AddShift("SHL" ,0x4c);
   AddShift("SHR" ,0x6c);

   Bit2Orders=(SimpOrder *) malloc(sizeof(SimpOrder)*Bit2OrderCount); InstrZ=0;
   AddBit2("BAND",0x6a); AddBit2("BCMP" ,0x2a);
   AddBit2("BMOV",0x4a); AddBit2("BMOVN",0x3a);
   AddBit2("BOR" ,0x5a); AddBit2("BXOR" ,0x7a);

   LoopOrders=(SimpOrder *) malloc(sizeof(SimpOrder)*LoopOrderCount); InstrZ=0;
   AddLoop("CMPD1",0xa0); AddLoop("CMPD2",0xb0);
   AddLoop("CMPI1",0x80); AddLoop("CMPI2",0x90);

   DivOrders=(char **) malloc(sizeof(char *)*DivOrderCount); InstrZ=0;
   DivOrders[InstrZ++]="DIV";  DivOrders[InstrZ++]="DIVU";
   DivOrders[InstrZ++]="DIVL"; DivOrders[InstrZ++]="DIVLU";

   BJmpOrders=(char **) malloc(sizeof(char *)*BJmpOrderCount);  InstrZ=0;
   BJmpOrders[InstrZ++]="JB";  BJmpOrders[InstrZ++]="JNB";
   BJmpOrders[InstrZ++]="JBC"; BJmpOrders[InstrZ++]="JNBS";

   MulOrders=(char **) malloc(sizeof(char *)*MulOrderCount); InstrZ=0;
   MulOrders[InstrZ++]="MUL";   MulOrders[InstrZ++]="MULU";
   MulOrders[InstrZ++]="PRIOR";
}

	static void DeinitFields(void)
{
   free(FixedOrders);
   free(Conditions);
   free(ALU2Orders);
   free(ShiftOrders);
   free(Bit2Orders);
   free(LoopOrders);
   free(DivOrders);
   free(BJmpOrders);
   free(MulOrders);
}

/*-------------------------------------------------------------------------*/

#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModImm 1
#define MModImm (1 << ModImm)
#define ModIReg 2
#define MModIReg (1 << ModIReg)
#define ModPreDec 3
#define MModPreDec (1 << ModPreDec)
#define ModPostInc 4
#define MModPostInc (1 << ModPostInc)
#define ModIndex 5
#define MModIndex (1 << ModIndex)
#define ModAbs 6
#define MModAbs (1 << ModAbs)
#define ModMReg 7
#define MModMReg (1 << ModMReg)
#define ModLAbs 8
#define MModLAbs (1 << ModLAbs)

static Byte AdrMode;
static Byte AdrVals[2];
static ShortInt AdrType;

	static bool IsReg(char *Asc, Byte *Erg, bool WordWise)
{
   bool err;

   if ((strlen(Asc)<2) || (toupper(*Asc)!='R')) return false;
   else if ((strlen(Asc)>2) && (toupper(Asc[1])=='L') && (! WordWise))
    {
     *Erg=ConstLongInt(Asc+2,&err); *Erg<<=1;
     return ((err) && (*Erg<=15));
    }
   else if ((strlen(Asc)>2) && (toupper(Asc[1])=='H') && (! WordWise))
    {
     *Erg=ConstLongInt(Asc+2,&err); *Erg<<=1; (*Erg)++;
     return ((err) && (*Erg<=15));
    }
   else
    {
     *Erg=ConstLongInt(Asc+1,&err);
     return ((err) && (*Erg<=15));
    }
}

	static bool IsRegM1(char *Asc, Byte *Erg, bool WordWise)
{
   char tmp;
   int l;
   bool b;

   if (*Asc!='\0')
    {
     tmp=Asc[l=(strlen(Asc)-1)]; Asc[l]='\0';
     b=IsReg(Asc,Erg,WordWise);
     Asc[l]=tmp;
     return b;
    }
   else return false;
}

	static LongInt SFRStart(void)
{
   return (ExtSFRs) ? 0xf000 : 0xfe00;
}

	static LongInt SFREnd(void)
{
   return (ExtSFRs) ? 0xf1de : 0xffde;
}

	static bool CalcPage(LongInt *Adr, bool DoAnyway)
{
   Integer z;
   Word Bank;

   switch (MemMode)
    {
     case MemModeStd:
      z=0;
      while ((z<=3) && (((*Adr) >> 14)!=DPPAssumes[z])) z++;
      if (z>3)
       {
        WrError(110); (*Adr)&=0xffff; return (DoAnyway);
       }
      else
       {
        *Adr=((*Adr) & 0x3fff)+(z << 14);
        if (DPPChanged[z]) WrXError(200,RegNames[z]);
        return true;
       }
     case MemModeZeroPage:
      (*Adr)&=0x3fff;
      return true;
     case MemModeFixedPage:
      Bank=(*Adr) >> 14; (*Adr)&=0x3fff;
      if (Bank!=MemPage)
       {
        WrError(110); return (DoAnyway);
       }
      else return true;
     case MemModeNoCheck:
      (*Adr)&=0xffff;
      return true;
     case MemModeFixedBank:
      Bank=(*Adr) >> 16; (*Adr)&=0xffff;
      if (Bank!=MemPage)
       {
        WrError(110); return (DoAnyway);
       }
      else return true;
     default:
      return false;
    }
}

	static void DecideAbsolute(bool InCode, LongInt DispAcc, Word Mask, bool Dest)
{
#define DPPAdr 0xfe00
#define SPAdr 0xfe12
#define CPAdr 0xfe10

   Integer z;

   if (InCode)
    if (((EProgCounter() >> 16)==(DispAcc >> 16)) && ((Mask & MModAbs)!=0))
     {
      AdrType=ModAbs; AdrCnt=2;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
     }
    else
     {
      AdrType=ModLAbs; AdrCnt=2; AdrMode=DispAcc >> 16;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
     }
   else if (((Mask & MModMReg)!=0) && (DispAcc>=SFRStart()) && (DispAcc<=SFREnd()) && ((DispAcc&1)==0))
    {
     AdrType=ModMReg; AdrCnt=1; AdrVals[0]=(DispAcc-SFRStart()) >> 1;
    }
   else switch (MemMode)
    {
     case MemModeStd:
      z=0;
      while ((z<=3) && ((DispAcc >> 14)!=DPPAssumes[z])) z++;
      if (z>3)
       {
        WrError(110); z=(DispAcc >> 14) & 3;
       }
      AdrType=ModAbs; AdrCnt=2;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=(Hi(DispAcc) & 0x3f)+(z << 6);
      if (DPPChanged[z]) WrXError(200,RegNames[z]);
      break;
     case MemModeZeroPage:
      AdrType=ModAbs; AdrCnt=2;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc) & 0x3f;
      break;
     case MemModeFixedPage:
      if ((DispAcc >> 14)!=MemPage) WrError(110);
      AdrType=ModAbs; AdrCnt=2;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc) & 0x3f;
      break;
     case MemModeNoCheck:
      AdrType=ModAbs; AdrCnt=2;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
      break;
     case MemModeFixedBank:
      if ((DispAcc >> 16)!=MemPage) WrError(110);
      AdrType=ModAbs; AdrCnt=2;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
      break;
    }

   if ((AdrType!=ModNone) && (Dest))
    switch ((Word)DispAcc)
     {
      case SPAdr    : N_SPChanged=true; break;
      case CPAdr    : N_CPChanged=true; break;
      case DPPAdr   :
      case DPPAdr+1 : N_DPPChanged[0]=true; break;
      case DPPAdr+2 :
      case DPPAdr+3 : N_DPPChanged[1]=true; break;
      case DPPAdr+4 :
      case DPPAdr+5 : N_DPPChanged[2]=true; break;
      case DPPAdr+6 :
      case DPPAdr+7 : N_DPPChanged[3]=true; break;
     }
}

	static void DecodeAdr(char *Asc, Word Mask, bool InCode, bool Dest)
{
   LongInt HDisp,DispAcc;
   char *PPos,*MPos;
   String Part;
   bool OK,NegFlag,NNegFlag;
   Byte HReg;

   AdrType=ModNone; AdrCnt=0;

   /* immediate ? */

   if (*Asc=='#')
    {
     switch (OpSize)
      {
       case 0:
	AdrVals[0]=EvalIntExpression(Asc+1,Int8,&OK);
	AdrVals[1]=0;
        break;
       case 1:
	HDisp=EvalIntExpression(Asc+1,Int16,&OK);
	AdrVals[0]=Lo(HDisp); AdrVals[1]=Hi(HDisp);
        break;
      }
     if (OK)
      {
       AdrType=ModImm; AdrCnt=OpSize+1;
      }
    }

   /* Register ? */

   else if (IsReg(Asc,&AdrMode,OpSize==1))
    {
     if ((Mask & MModReg)!=0) AdrType=ModReg;
     else
      {
       AdrType=ModMReg; AdrVals[0]=0xf0+AdrMode; AdrCnt=1;
      }
     if (CPChanged) WrXError(200,RegNames[4]);
    }

   /* indirekt ? */

   else if ((*Asc=='[') && (Asc[strlen(Asc)-1]==']'))
    {
     strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';

     /* Predekrement ? */

     if ((strlen(Asc)>2) && (*Asc=='-') && (IsReg(Asc+1,&AdrMode,true)))
      AdrType=ModPreDec;

     /* Postinkrement ? */

     else if ((strlen(Asc)>2) && (Asc[strlen(Asc)-1]=='+') && (IsRegM1(Asc,&AdrMode,true)))
      AdrType=ModPostInc;

     /* indiziert ? */

     else
      {
       NegFlag=false; DispAcc=0; AdrMode=0xff;
       while (*Asc!='\0')
	{
	 MPos=QuotPos(Asc,'-'); PPos=QuotPos(Asc,'+');
	 if (((MPos<PPos) || (PPos==NULL)) && (MPos!=NULL))
	  {
	   PPos=MPos; NNegFlag=true;
	  }
	 else NNegFlag=false;
         if (PPos==NULL)
          {
           strmaxcpy(Part,Asc,255); *Asc='\0';
          }
         else
          {
           *PPos='\0'; strmaxcpy(Part,Asc,255); strcpy(Asc,PPos+1);
          }
	 if (IsReg(Part,&HReg,true))
	  if ((NegFlag) || (AdrMode!=0xff)) WrError(1350); else AdrMode=HReg;
	 else
	  {
	   if (*Part=='#') strcpy(Part,Part+1);
	   HDisp=EvalIntExpression(Part,Int32,&OK);
	   if (OK)
	    if (NegFlag) DispAcc-=HDisp; else DispAcc+=HDisp;
	  }
	 NegFlag=NNegFlag;
	}
       if (AdrMode==0xff) DecideAbsolute(InCode,DispAcc,Mask,Dest);
       else if (DispAcc==0) AdrType=ModIReg;
       else if (DispAcc>0xffff) WrError(1320);
       else if (DispAcc<-0x8000l) WrError(1315);
       else
	{
	 AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
	 AdrType=ModIndex; AdrCnt=2;
	}
      }
    }
   else
    {
     DispAcc=EvalIntExpression(Asc,MemInt,&OK);
     if (OK) DecideAbsolute(InCode,DispAcc,Mask,Dest);
    }

   if ((AdrType!=ModNone) && (((1 << AdrType) & Mask)==0))
    {
     WrError(1350); AdrType=ModNone; AdrCnt=0;
    }
}

	static Integer DecodeCondition(char *Name)
{
   Integer z;

   NLS_UpString(Name);
   for (z=0; z<ConditionCount; z++)
    if (strcmp(Conditions[z].Name,Name)==0) break;
   return z;
}

	static bool DecodeBitAddr(char *Asc, Word *Adr, Byte *Bit, bool MayBeOut)
{
   char *p;
   Word LAdr;
   Byte Reg;
   bool OK;

   p=QuotPos(Asc,'.');
   if (p==NULL)
    {
     LAdr=EvalIntExpression(Asc,UInt16,&OK) & 0x1fff;
     if (OK)
      {
       if ((! MayBeOut) && ((LAdr >> 12)!=ExtSFRs))
	{
	 WrError(1335); return false;
	}
       *Adr=LAdr >> 4; *Bit=LAdr & 15;
       if (! MayBeOut) *Adr=Lo(*Adr);
       return true;
      }
     else return false;
    }
   else if (p<=Asc+1) return false;
   else
    {
     *p='\0';
     if (IsReg(Asc,&Reg,true)) *Adr=0xf0+Reg;
     else
      {
       FirstPassUnknown=false;
       LAdr=EvalIntExpression(Asc,UInt16,&OK); if (! OK) return false;
       if (FirstPassUnknown) LAdr=0xfd00;
       if ((LAdr&1)==1)
	{
	 WrError(1325); return false;
	}
       if ((LAdr>=0xfd00) && (LAdr<=0xfdfe)) *Adr=(LAdr-0xfd00)/2;
       else if ((LAdr>=0xff00) && (LAdr<=0xffde))
	{
	 if ((ExtSFRs) && (! MayBeOut))
	  {
	   WrError(1335); return false;
	  }
	 *Adr=0x80+((LAdr-0xff00)/2);
	}
       else if ((LAdr>=0xf100) && (LAdr<=0xf1de))
	{
	 if ((! ExtSFRs) && (! MayBeOut))
	  {
	   WrError(1335); return false;
	  }
	 *Adr=0x80+((LAdr-0xf100)/2);
	 if (MayBeOut) (*Adr)+=0x100;
	}
       else
	{
	 WrError(1320); return false;
	}
      }

     *Bit=EvalIntExpression(p+1,UInt4,&OK);
     return OK;
    }
}

	static Word WordVal(void)
{
   return AdrVals[0]+(((Word)AdrVals[1]) << 8);
}

	static bool DecodePref(char *Asc, Byte *Erg)
{
   bool OK;

   if (*Asc!='#')
    {
     WrError(1350); return false;
    }
   strcpy(Asc,Asc+1);
   FirstPassUnknown=false;
   *Erg=EvalIntExpression(Asc,UInt3,&OK);
   if (FirstPassUnknown) *Erg=1;
   if (! OK) return false;
   if (*Erg<1) WrError(1315);
   else if (*Erg>4) WrError(1320);
   else
    {
     (*Erg)--; return true;
    }
   return false;
}

/*-------------------------------------------------------------------------*/

#define ASSUME166Count 4
static ASSUMERec ASSUME166s[ASSUME166Count]=
	     {{"DPP0", DPPAssumes+0, 0, 15, -1},
	      {"DPP1", DPPAssumes+1, 0, 15, -1},
	      {"DPP2", DPPAssumes+2, 0, 15, -1},
	      {"DPP3", DPPAssumes+3, 0, 15, -1}};

	static bool DecodePseudo(void)
{
   Word Adr;
   Byte Bit;

   if (Memo("ASSUME"))
    {
     CodeASSUME(ASSUME166s,ASSUME166Count);
     return true;
    }

   if (Memo("BIT"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (DecodeBitAddr(ArgStr[1],&Adr,&Bit,true))
      {
       PushLocHandle(-1);
       EnterIntSymbol(LabPart,(Adr << 4)+Bit,SegNone,false);
       PopLocHandle();
       sprintf(ListLine,"=%02xH.%1x",Adr,Bit);
      }
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

	static void MakeCode_166(void)
{
   Integer z,Cond;
   Word AdrWord;
   Byte AdrBank,HReg;
   Byte BOfs1,BOfs2;
   Word BAdr1,BAdr2;
   LongInt AdrLong;
   bool OK;

   CodeLen=0; DontPrint=false; OpSize=1;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(false)) return;

   /* Pipeline-Flags weiterschalten */

   SPChanged=N_SPChanged; N_SPChanged=false;
   CPChanged=N_CPChanged; N_CPChanged=false;
   for (z=0; z<DPPCount; z++)
    {
     DPPChanged[z]=N_DPPChanged[z];
     N_DPPChanged[z]=false;
    }

   /* Praefixe herunterzaehlen */

   if (ExtCounter>=0)
    if (--ExtCounter<0)
     {
      MemMode=MemModeStd;
      ExtSFRs=false;
     }

   /* ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     {
      if (ArgCnt!=0) WrError(1110);
      else
       {
	CodeLen=2;
	BAsmCode[0]=Lo(FixedOrders[z].Code1);
        BAsmCode[1]=Hi(FixedOrders[z].Code1);
	if (FixedOrders[z].Code2!=0)
	 {
	  CodeLen=4;
	  BAsmCode[2]=Lo(FixedOrders[z].Code2);
          BAsmCode[3]=Hi(FixedOrders[z].Code2);
	  if ((strncmp(OpPart,"RET",3)==0) && (SPChanged)) WrXError(200,RegNames[5]);
	 }
       }
      return;
     }

   /* Datentransfer */

   if (BMemo("MOV"))
    {
     Cond=1-OpSize;
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg+MModMReg+MModIReg+MModPreDec+MModPostInc+MModIndex+MModAbs,false,true);
       switch (AdrType)
        {
         case ModReg:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg+MModImm+MModIReg+MModPostInc+MModIndex+MModAbs,false,false);
	  switch (AdrType)
           {
	    case ModReg:
	     CodeLen=2; BAsmCode[0]=0xf0+Cond;
	     BAsmCode[1]=(HReg << 4)+AdrMode;
	     break;
	    case ModImm:
	     if (WordVal()<=15)
	      {
	       CodeLen=2; BAsmCode[0]=0xe0+Cond;
	       BAsmCode[1]=(WordVal() << 4)+HReg;
	      }
	     else
	      {
	       CodeLen=4; BAsmCode[0]=0xe6+Cond; BAsmCode[1]=HReg+0xf0;
	       memcpy(BAsmCode+2,AdrVals,2);
	      }
             break;
	    case ModIReg:
	     CodeLen=2; BAsmCode[0]=0xa8+Cond;
	     BAsmCode[1]=(HReg << 4)+AdrMode;
	     break;
	    case ModPostInc:
	     CodeLen=2; BAsmCode[0]=0x98+Cond;
	     BAsmCode[1]=(HReg << 4)+AdrMode;
	     break;
	    case ModIndex:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xd4+(Cond << 5);
	     BAsmCode[1]=(HReg << 4)+AdrMode; memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     break;
	    case ModAbs:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xf2+Cond;
	     BAsmCode[1]=0xf0+HReg; memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     break;
	   }
	  break;
         case ModMReg:
	  BAsmCode[1]=AdrVals[0];
          DecodeAdr(ArgStr[2],MModImm+MModMReg+((DPPAssumes[3]==3)?MModIReg:0)+MModAbs,false,false);
	  switch (AdrType)
           {
	    case ModImm:
	     CodeLen=4; BAsmCode[0]=0xe6+Cond;
	     memcpy(BAsmCode+2,AdrVals,2);
	     break;
            case ModMReg: /* BAsmCode[1] sicher absolut darstellbar, da Rn vorher */
                          /* abgefangen wird! */
             BAsmCode[0]=0xf6+Cond;
             AdrLong=0xfe00+(((Word)BAsmCode[1]) << 1);
             CalcPage(&AdrLong,true);
	     BAsmCode[2]=Lo(AdrLong);
             BAsmCode[3]=Hi(AdrLong);
             BAsmCode[1]=AdrVals[0];
             CodeLen=4;
             break;
	    case ModIReg:
	     CodeLen=4; BAsmCode[0]=0x94+(Cond << 5);
	     BAsmCode[2]=BAsmCode[1] << 1;
	     BAsmCode[3]=0xfe + (BAsmCode[1] >> 7); /* ANSI :-0 */
	     BAsmCode[1]=AdrMode;
	     break;
	    case ModAbs:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xf2+Cond;
	     memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     break;
	   }
	  break;
         case ModIReg:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg+MModIReg+MModPostInc+MModAbs,false,false);
	  switch (AdrType)
           {
	    case ModReg:
	     CodeLen=2; BAsmCode[0]=0xb8+Cond;
	     BAsmCode[1]=HReg+(AdrMode << 4);
	     break;
	    case ModIReg:
	     CodeLen=2; BAsmCode[0]=0xc8+Cond;
	     BAsmCode[1]=(HReg << 4)+AdrMode;
	     break;
	    case ModPostInc:
	     CodeLen=2; BAsmCode[0]=0xe8+Cond;
	     BAsmCode[1]=(HReg << 4)+AdrMode;
	     break;
	    case ModAbs:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0x84+(Cond << 5);
	     BAsmCode[1]=HReg; memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     break;
	   }
	  break;
         case ModPreDec:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg,false,false);
	  switch (AdrType)
           {
	    case ModReg:
	     CodeLen=2; BAsmCode[0]=0x88+Cond;
	     BAsmCode[1]=HReg+(AdrMode << 4);
	     break;
	   }
	  break;
         case ModPostInc:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModIReg,false,false);
	  switch (AdrType)
           {
	    case ModIReg:
 	     CodeLen=2; BAsmCode[0]=0xd8+Cond;
	     BAsmCode[1]=(HReg << 4)+AdrMode;
	     break;
	   }
	  break;
         case ModIndex:
	  BAsmCode[1]=AdrMode; memcpy(BAsmCode+2,AdrVals,AdrCnt);
	  DecodeAdr(ArgStr[2],MModReg,false,false);
	  switch (AdrType)
           {
	    case ModReg:
	     BAsmCode[0]=0xc4+(Cond << 5);
	     CodeLen=4; BAsmCode[1]+=AdrMode << 4;
	     break;
	   }
	  break;
         case ModAbs:
	  memcpy(BAsmCode+2,AdrVals,AdrCnt);
	  DecodeAdr(ArgStr[2],MModIReg+MModMReg,false,false);
	  switch (AdrType)
           {
	    case ModIReg:
	     CodeLen=4; BAsmCode[0]=0x94+(Cond << 5);
	     BAsmCode[1]=AdrMode;
	     break;
	    case ModMReg:
	     CodeLen=4; BAsmCode[0]=0xf6+Cond;
	     BAsmCode[1]=AdrVals[0];
	     break;
	   }
	  break;
        }
      }
     return;
    }

   if ((Memo("MOVBS")) || (Memo("MOVBZ")))
    {
     Cond=Memo("MOVBS") << 4;
     if (ArgCnt!=2) WrError(1110);
     else
      {
       OpSize=1;
       DecodeAdr(ArgStr[1],MModReg+MModMReg+MModAbs,false,true);
       OpSize=0;
       switch (AdrType)
        {
         case ModReg:
	  HReg=AdrMode; DecodeAdr(ArgStr[2],MModReg,false,false);
	  switch (AdrType)
           {
	    case ModReg:
	     CodeLen=2; BAsmCode[0]=0xc0+Cond;
	     BAsmCode[1]=HReg+(AdrMode << 4);
	     break;
	   }
	  break;
         case ModMReg:
	  BAsmCode[1]=AdrVals[0];
	  DecodeAdr(ArgStr[2],MModAbs+MModMReg,false,false);
	  switch (AdrType)
           {
            case ModMReg: /* BAsmCode[1] sicher absolut darstellbar, da Rn vorher */
                          /* abgefangen wird! */
             BAsmCode[0]=0xc5+Cond;
             AdrLong=0xfe00+(((Word)BAsmCode[1]) << 1);
             CalcPage(&AdrLong,true);
	     BAsmCode[2]=Lo(AdrLong);
             BAsmCode[3]=Hi(AdrLong);
             BAsmCode[1]=AdrVals[0];
             CodeLen=4;
             break;
	    case ModAbs:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xc2+Cond;
	     memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     break;
	   }
	  break;
         case ModAbs:
	  memcpy(BAsmCode+2,AdrVals,AdrCnt);
	  DecodeAdr(ArgStr[2],MModMReg,false,false);
	  switch (AdrType)
           {
	    case ModMReg:
	     CodeLen=4; BAsmCode[0]=0xc5+Cond;
	     BAsmCode[1]=AdrVals[0];
	     break;
	   }
	  break;
        }
      }
     return;
    }

   if ((Memo("PUSH")) || (Memo("POP")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModMReg,false,Memo("POP"));
       switch (AdrType)
        {
         case ModMReg:
	  CodeLen=2; BAsmCode[0]=0xec+(Memo("POP") << 4);
	  BAsmCode[1]=AdrVals[0];
	  if (SPChanged) WrXError(200,RegNames[5]);
	  break;
        }
      }
     return;
    }

   if (Memo("SCXT"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModMReg,false,true);
       switch (AdrType)
        {
         case ModMReg:
	  BAsmCode[1]=AdrVals[0];
	  DecodeAdr(ArgStr[2],MModAbs+MModImm,false,false);
	  if (AdrType!=ModNone)
	   {
	    CodeLen=4; BAsmCode[0]=0xc6+((AdrType==ModAbs) << 4);
	    memcpy(BAsmCode+2,AdrVals,2);
	   }
	  break;
        }
      }
     return;
    }

   /* Arithmetik */

   for (z=0; z<ALU2OrderCount; z++)
    if (BMemo(ALU2Orders[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
	Cond=(1-OpSize)+(z << 4);
	DecodeAdr(ArgStr[1],MModReg+MModMReg+MModAbs,false,true);
	switch (AdrType)
         {
	  case ModReg:
	   HReg=AdrMode;
	   DecodeAdr(ArgStr[2],MModReg+MModIReg+MModPostInc+MModAbs+MModImm,false,false);
	   switch (AdrType)
            {
	     case ModReg:
	      CodeLen=2; BAsmCode[0]=Cond;
	      BAsmCode[1]=(HReg << 4)+AdrMode;
	      break;
	     case ModIReg:
	      if (AdrMode>3) WrError(1350);
	      else
	       {
	        CodeLen=2; BAsmCode[0]=0x08+Cond;
	        BAsmCode[1]=(HReg << 4)+8+AdrMode;
	       }
              break;
	     case ModPostInc:
	      if (AdrMode>3) WrError(1350);
	      else
	       {
	        CodeLen=2; BAsmCode[0]=0x08+Cond;
	        BAsmCode[1]=(HReg << 4)+12+AdrMode;
	       }
              break;
	     case ModAbs:
	      CodeLen=4; BAsmCode[0]=0x02+Cond; BAsmCode[1]=0xf0+HReg;
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	      break;
	     case ModImm:
	      if (WordVal()<=7)
	       {
	        CodeLen=2; BAsmCode[0]=0x08+Cond;
	        BAsmCode[1]=(HReg << 4)+AdrVals[0];
	       }
	      else
	       {
	        CodeLen=4; BAsmCode[0]=0x06+Cond; BAsmCode[1]=0xf0+HReg;
	        memcpy(BAsmCode+2,AdrVals,2);
	       }
              break;
	    }
	   break;
	  case ModMReg:
	   BAsmCode[1]=AdrVals[0];
	   DecodeAdr(ArgStr[2],MModAbs+MModMReg+MModImm,false,false);
	   switch (AdrType)
            {
	     case ModAbs:
	      CodeLen=4; BAsmCode[0]=0x02+Cond;
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	      break;
             case ModMReg: /* BAsmCode[1] sicher absolut darstellbar, da Rn vorher */
                           /* abgefangen wird! */
              BAsmCode[0]=0x04+Cond;
              AdrLong=0xfe00+(((Word)BAsmCode[1]) << 1);
              CalcPage(&AdrLong,true);
	      BAsmCode[2]=Lo(AdrLong);
              BAsmCode[3]=Hi(AdrLong);
              BAsmCode[1]=AdrVals[0];
              CodeLen=4;
              break;
	     case ModImm:
	      CodeLen=4; BAsmCode[0]=0x06+Cond;
	      memcpy(BAsmCode+2,AdrVals,2);
	      break;
	    }
	   break;
	  case ModAbs:
	   memcpy(BAsmCode+2,AdrVals,AdrCnt);
	   DecodeAdr(ArgStr[2],MModMReg,false,false);
	   switch (AdrType)
            {
	     case ModMReg:
	      CodeLen=4; BAsmCode[0]=0x04+Cond; BAsmCode[1]=AdrVals[0];
	      break;
	    }
	   break;
	 }
       }
      return;
     }

   if ((BMemo("CPL")) || (BMemo("NEG")))
    {
     Cond=0x81+((1-OpSize) << 5);
     if (ArgCnt!=1) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg,false,true);
       if (AdrType==ModReg)
	{
	 CodeLen=2; BAsmCode[0]=Cond+(BMemo("CPL") << 4);
	 BAsmCode[1]=AdrMode << 4;
	}
      }
     return;
    }

   for (z=0; z<DivOrderCount; z++)
    if (Memo(DivOrders[z]))
     {
      if (ArgCnt!=1) WrError(1110);
      else
       {
	DecodeAdr(ArgStr[1],MModReg,false,false);
	if (AdrType==ModReg)
	 {
	  CodeLen=2; BAsmCode[0]=0x4b+(z << 4);
	  BAsmCode[1]=AdrMode*0x11;
	 }
       }
      return;
     }

   for (z=0; z<LoopOrderCount; z++)
    if (Memo(LoopOrders[z].Name))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
        DecodeAdr(ArgStr[1],MModReg,false,true);
        if (AdrType==ModReg)
         {
          BAsmCode[1]=AdrMode;
          DecodeAdr(ArgStr[2],MModAbs+MModImm,false,false);
          switch (AdrType)
           {
            case ModAbs:
             CodeLen=4; BAsmCode[0]=LoopOrders[z].Code+2; BAsmCode[1]+=0xf0;
             memcpy(BAsmCode+2,AdrVals,2);
             break;
            case ModImm:
             if (WordVal()<16)
              {
               CodeLen=2; BAsmCode[0]=LoopOrders[z].Code;
               BAsmCode[1]+=(WordVal() << 4);
              }
             else
              {
               CodeLen=4; BAsmCode[0]=LoopOrders[z].Code+6; BAsmCode[1]+=0xf0;
               memcpy(BAsmCode+2,AdrVals,2);
              }
             break;
           }
         }
       }
      return;
     }

   for (z=0; z<MulOrderCount; z++)
    if (Memo(MulOrders[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
	DecodeAdr(ArgStr[1],MModReg,false,false);
	switch (AdrType)
         {
	  case ModReg:
	   HReg=AdrMode;
	   DecodeAdr(ArgStr[2],MModReg,false,false);
	   switch (AdrType)
            {
	     case ModReg:
	      CodeLen=2; BAsmCode[0]=0x0b+(z << 4);
	      BAsmCode[1]=(HReg << 4)+AdrMode;
	      break;
	    }
	   break;
 	 }
       }
      return;
     }

   /* Logik */

   for (z=0; z<ShiftOrderCount; z++)
    if (Memo(ShiftOrders[z].Name))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
        OpSize=1;
        DecodeAdr(ArgStr[1],MModReg,false,true);
        switch (AdrType)
         {
          case ModReg:
           HReg=AdrMode;
           DecodeAdr(ArgStr[2],MModReg+MModImm,false,true);
           switch (AdrType)
            {
             case ModReg:
              BAsmCode[0]=ShiftOrders[z].Code; BAsmCode[1]=AdrMode+(HReg << 4);
              CodeLen=2;
              break;
             case ModImm:
              if (WordVal()>15) WrError(1320);
              else
               {
                BAsmCode[0]=ShiftOrders[z].Code+0x10;
                BAsmCode[1]=(WordVal() << 4)+HReg;
                CodeLen=2;
               }
              break;
            }
           break;
         }
       }
      return;
     }

   for (z=0; z<Bit2OrderCount; z++)
    if (Memo(Bit2Orders[z].Name))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       if (DecodeBitAddr(ArgStr[1],&BAdr1,&BOfs1,false))
       if (DecodeBitAddr(ArgStr[2],&BAdr2,&BOfs2,false))
        {
         CodeLen=4; BAsmCode[0]=Bit2Orders[z].Code;
         BAsmCode[1]=BAdr2; BAsmCode[2]=BAdr1;
         BAsmCode[3]=(BOfs2 << 4)+BOfs1;
        }
      return;
     }

   if ((Memo("BCLR")) || (Memo("BSET")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (DecodeBitAddr(ArgStr[1],&BAdr1,&BOfs1,false))
      {
       CodeLen=2; BAsmCode[0]=(BOfs1 << 4)+0x0e + Memo("BSET"); /* ANSI :-0 */
       BAsmCode[1]=BAdr1;
      }
     return;
    }

   if ((Memo("BFLDH")) || (Memo("BFLDL")))
    {
     if (ArgCnt!=3) WrError(1110);
     else
      {
       strmaxcat(ArgStr[1],".0",255);
       if (DecodeBitAddr(ArgStr[1],&BAdr1,&BOfs1,false))
        {
         OpSize=0; BAsmCode[1]=BAdr1;
         DecodeAdr(ArgStr[2],MModImm,false,false);
         if (AdrType==ModImm)
  	  {
  	   BAsmCode[2]=AdrVals[0];
  	   DecodeAdr(ArgStr[3],MModImm,false,false);
  	   if (AdrType==ModImm)
  	    {
  	     BAsmCode[3]=AdrVals[0];
  	     CodeLen=4; BAsmCode[0]=0x0a;
  	     if (Memo("BFLDH"))
  	      {
  	       BAdr1=BAsmCode[2]; BAsmCode[2]=BAsmCode[3]; BAsmCode[3]=BAdr1;
  	       BAsmCode[0]+=0x10;
  	      }
  	    }
  	  }
        }
      }
     return;
    }

   /*Spruenge */

   if (Memo("JMP"))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else
      {
       Cond=(ArgCnt==1) ? TrueCond : DecodeCondition(ArgStr[1]);
       if (Cond>=ConditionCount) WrXError(1360,ArgStr[1]);
       else
	{
	 DecodeAdr(ArgStr[ArgCnt],MModAbs+MModLAbs+MModIReg,true,false);
	 switch (AdrType)
          {
	   case ModLAbs:
	    if (Cond!=TrueCond) WrXError(1360,ArgStr[1]);
	    else
	     {
	      CodeLen=2+AdrCnt; BAsmCode[0]=0xfa; BAsmCode[1]=AdrMode;
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     }
            break;
	   case ModAbs:
	    AdrLong=WordVal()-(EProgCounter()+2);
	    if ((AdrLong<=254) && (AdrLong>=-256) && ((AdrLong&1)==0))
	     {
	      CodeLen=2; BAsmCode[0]=0x0d+(Conditions[Cond].Code << 4);
	      BAsmCode[1]=(AdrLong/2)&0xff;
	     }
	    else
	     {
	      CodeLen=2+AdrCnt; BAsmCode[0]=0xea;
	      BAsmCode[1]=Conditions[Cond].Code << 4;
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     }
	    break;
	   case ModIReg:
	    CodeLen=2; BAsmCode[0]=0x9c;
	    BAsmCode[1]=(Conditions[Cond].Code << 4)+AdrMode;
	    break;
	  }
	}
      }
     return;
    }

   if (Memo("CALL"))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else
      {
       Cond=(ArgCnt==1) ? TrueCond : DecodeCondition(ArgStr[1]);
       if (Cond>=ConditionCount) WrXError(1360,ArgStr[1]);
       else
	{
	 DecodeAdr(ArgStr[ArgCnt],MModAbs+MModLAbs+MModIReg,true,false);
	 switch (AdrType)
          {
	   case ModLAbs:
	    if (Cond!=TrueCond) WrXError(1360,ArgStr[1]);
	    else
	     {
	      CodeLen=2+AdrCnt; BAsmCode[0]=0xda; BAsmCode[1]=AdrMode;
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     }
            break;
	   case ModAbs:
	    AdrLong=WordVal()-(EProgCounter()+2);
	    if ((AdrLong<=254) && (AdrLong>=-256) && ((AdrLong&1)==0) && (Cond==TrueCond))
	     {
	      CodeLen=2; BAsmCode[0]=0xbb;
	      BAsmCode[1]=(AdrLong/2) & 0xff;
	     }
	    else
	     {
	      CodeLen=2+AdrCnt; BAsmCode[0]=0xca;
	      BAsmCode[1]=0xc0+(Conditions[Cond].Code << 4);
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     }
	    break;
	   case ModIReg:
	    CodeLen=2; BAsmCode[0]=0xab;
	    BAsmCode[1]=(Conditions[Cond].Code << 4)+AdrMode;
	    break;
	  }
	}
      }
     return;
    }

   if (Memo("JMPR"))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else
      {
       Cond=(ArgCnt==1) ? TrueCond : DecodeCondition(ArgStr[1]);
       if (Cond>=ConditionCount) WrXError(1360,ArgStr[1]);
       else
	{
	 AdrLong=EvalIntExpression(ArgStr[ArgCnt],MemInt,&OK)-(EProgCounter()+2);
	 if (OK)
	  if ((AdrLong&1)==1) WrError(1375);
	  else if ((! SymbolQuestionable) && ((AdrLong>254) || (AdrLong<-256))) WrError(1370);
	  else
	   {
	    CodeLen=2; BAsmCode[0]=0x0d+(Conditions[Cond].Code << 4);
	    BAsmCode[1]=(AdrLong/2) & 0xff;
	   }
	}
      }
     return;
    }

   if (Memo("CALLR"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrLong=EvalIntExpression(ArgStr[ArgCnt],MemInt,&OK)-(EProgCounter()+2);
       if (OK)
	if ((AdrLong&1)==1) WrError(1375);
	else if ((! SymbolQuestionable) && ((AdrLong>254) || (AdrLong<-256))) WrError(1370);
	else
	 {
	  CodeLen=2; BAsmCode[0]=0xbb;
	  BAsmCode[1]=(AdrLong/2) & 0xff;
	 }
      }
     return;
    }

   if ((Memo("JMPA")) || (Memo("CALLA")))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else
      {
       Cond=(ArgCnt==1) ? TrueCond : DecodeCondition(ArgStr[1]);
       if (Cond>=ConditionCount) WrXError(1360,ArgStr[1]);
       else
	{
	 AdrLong=EvalIntExpression(ArgStr[ArgCnt],MemInt,&OK);
	 if (OK)
	  if ((AdrLong >> 16)!=(EProgCounter() >> 16)) WrError(1910);
	  else
	   {
	    CodeLen=4;
            BAsmCode[0]=(Memo("JMPA")) ? 0xea : 0xca;
	    BAsmCode[1]=0x00+(Conditions[Cond].Code << 4);
	    BAsmCode[2]=Lo(AdrLong); BAsmCode[3]=Hi(AdrLong);
	   }
	}
      }
     return;
    }

   if ((Memo("JMPS")) || (Memo("CALLS")))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else
      {
       if (ArgCnt==1)
	{
	 AdrLong=EvalIntExpression(ArgStr[1],MemInt,&OK);
	 AdrWord=AdrLong & 0xffff; AdrBank=AdrLong >> 16;
	}
       else
	{
	 AdrWord=EvalIntExpression(ArgStr[2],UInt16,&OK);
	 if (OK) AdrBank=EvalIntExpression(ArgStr[1],MemInt2,&OK); else AdrBank=0;
	}
       if (OK)
	{
	 CodeLen=4;
         BAsmCode[0]=(Memo("JMPS")) ? 0xfa : 0xda;
	 BAsmCode[1]=AdrBank;
	 BAsmCode[2]=Lo(AdrWord); BAsmCode[3]=Hi(AdrWord);
	}
      }
     return;
    }

   if ((Memo("JMPI")) || (Memo("CALLI")))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else
      {
       Cond=(ArgCnt==1) ? TrueCond : DecodeCondition(ArgStr[1]);
       if (Cond>=ConditionCount) WrXError(1360,ArgStr[1]);
       else
	{
	 DecodeAdr(ArgStr[ArgCnt],MModIReg,true,false);
	 switch (AdrType)
          {
	   case ModIReg:
	    CodeLen=2;
            BAsmCode[0]=(Memo("JMPI")) ? 0x9c : 0xab;
	    BAsmCode[1]=AdrMode+(Conditions[Cond].Code << 4);
	    break;
	  }
	}
      }
     return;
    }

   for (z=0; z<BJmpOrderCount; z++)
    if (Memo(BJmpOrders[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else if (DecodeBitAddr(ArgStr[1],&BAdr1,&BOfs1,false))
       {
	AdrLong=EvalIntExpression(ArgStr[2],MemInt,&OK)-(EProgCounter()+4);
	if (OK)
	 if ((AdrLong&1)==1) WrError(1375);
	 else if ((! SymbolQuestionable) && ((AdrLong<-256) || (AdrLong>254))) WrError(1370);
	 else
	  {
	   CodeLen=4; BAsmCode[0]=0x8a+(z << 4);
	   BAsmCode[1]=BAdr1;
	   BAsmCode[2]=(AdrLong/2) & 0xff;
	   BAsmCode[3]=BOfs1 << 4;
	  }
       }
      return;
     }

   if (Memo("PCALL"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModMReg,false,false);
       switch (AdrType)
        {
         case ModMReg:
	  BAsmCode[1]=AdrVals[0];
	  DecodeAdr(ArgStr[2],MModAbs,true,false);
	  switch (AdrType)
           {
	    case ModAbs:
	     CodeLen=4; BAsmCode[0]=0xe2; memcpy(BAsmCode+2,AdrVals,2);
	     break;
	   }
	  break;
        }
      }
     return;
    }

   if (Memo("RETP"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModMReg,false,false);
       switch (AdrType)
        {
         case ModMReg:
	  BAsmCode[1]=AdrVals[0]; BAsmCode[0]=0xeb; CodeLen=2;
	  if (SPChanged) WrXError(200,RegNames[5]);
	  break;
        }
      }
     return;
    }

   if (Memo("TRAP"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      {
       BAsmCode[1]=EvalIntExpression(ArgStr[1]+1,UInt7,&OK) << 1;
       if (OK)
	{
	 BAsmCode[0]=0x9b; CodeLen=2;
	}
      }
     return;
    }

   /* spezielle Steuerbefehle */

   if (Memo("ATOMIC"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU80C167) WrError(1500);
     else if (DecodePref(ArgStr[1],&HReg))
      {
       CodeLen=2; BAsmCode[0]=0xd1; BAsmCode[1]=HReg << 4;
      }
     return;
    }

   if (Memo("EXTR"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU80C167) WrError(1500);
     else if (DecodePref(ArgStr[1],&HReg))
      {
       CodeLen=2; BAsmCode[0]=0xd1; BAsmCode[1]=0x80+(HReg << 4);
       ExtCounter=HReg+1; ExtSFRs=true;
      }
     return;
    }

   if ((Memo("EXTP")) || (Memo("EXTPR")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80C167) WrError(1500);
     else if (DecodePref(ArgStr[2],&HReg))
      {
       DecodeAdr(ArgStr[1],MModReg+MModImm,false,false);
       switch (AdrType)
        {
         case ModReg:
	  CodeLen=2; BAsmCode[0]=0xdc; BAsmCode[1]=0x40+(HReg << 4)+AdrMode;
	  if (Memo("EXTPR")) BAsmCode[1]+=0x80;
	  ExtCounter=HReg+1; MemMode=MemModeZeroPage;
	  break;
         case ModImm:
	  CodeLen=4; BAsmCode[0]=0xd7; BAsmCode[1]=0x40+(HReg << 4);
	  if (Memo("EXTPR")) BAsmCode[1]+=0x80;
	  BAsmCode[2]=(WordVal() >> 2) & 0x7f; BAsmCode[3]=WordVal() & 3;
	  ExtCounter=HReg+1; MemMode=MemModeFixedPage; MemPage=WordVal() & 0x3ff;
	  break;
        }
      }
     return;
    }

   if ((Memo("EXTS")) || (Memo("EXTSR")))
    {
     OpSize=0;
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80C167) WrError(1500);
     else if (DecodePref(ArgStr[2],&HReg))
      {
       DecodeAdr(ArgStr[1],MModReg+MModImm,false,false);
       switch (AdrType)
        {
         case ModReg:
	  CodeLen=2; BAsmCode[0]=0xdc; BAsmCode[1]=0x00+(HReg << 4)+AdrMode;
	  if (Memo("EXTSR")) BAsmCode[1]+=0x80;
	  ExtCounter=HReg+1; MemMode=MemModeNoCheck;
	  break;
         case ModImm:
	  CodeLen=4; BAsmCode[0]=0xd7; BAsmCode[1]=0x00+(HReg << 4);
	  if (Memo("EXTSR")) BAsmCode[1]+=0x80;
	  BAsmCode[2]=AdrVals[0]; BAsmCode[3]=0;
	  ExtCounter=HReg+1; MemMode=MemModeFixedBank; MemPage=AdrVals[0];
	  break;
        }
      }
     return;
    }

   WrXError(1200,OpPart);
}

	static void InitCode_166(void)
{
   Integer z;

   SaveInitProc();
   for (z=0; z<DPPCount; z++)
    {
     DPPAssumes[z]=z; N_DPPChanged[z]=false;
    }
   N_CPChanged=false; N_SPChanged=false;

   MemMode=MemModeStd; ExtSFRs=false; ExtCounter=(-1);
}

	static bool ChkPC_166(void)
{
   if (ActPC==SegCode) return (ProgCounter()<0x40000);
   else return false;
}

	static bool IsDef_166(void)
{
   return (Memo("BIT"));
}

	static void SwitchFrom_166(void)
{
   DeinitFields();
}

	static void SwitchTo_166(void)
{
   Byte z;

   TurnWords=false; ConstMode=ConstModeIntel; SetIsOccupied=false;
   OpSize=1;

   PCSymbol="$"; HeaderID=0x4c; NOPCode=0xcc00;
   DivideChars=","; HasAttrs=false;

   ValidSegs=(1<<SegCode);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_166; ChkPC=ChkPC_166; IsDef=IsDef_166;
   SwitchFrom=SwitchFrom_166;

   if (MomCPU==CPU80C166)
    {
     MemInt=UInt18; MemInt2=UInt2; ASSUME166s[0].Max=15;
    }
   else
    {
     MemInt=UInt24; MemInt2=UInt8; ASSUME166s[0].Max=1023;
    }
   for (z=1; z<4; z++) ASSUME166s[z].Max=ASSUME166s[0].Max;

   InitFields();
}

	void code166_init(void)
{
   CPU80C166=AddCPU("80C166",SwitchTo_166);
   CPU80C167=AddCPU("80C167",SwitchTo_166);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_166;
}
