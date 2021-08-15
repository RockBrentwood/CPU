/* code90c141.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Toshiba TLCS-90                                             */
/*                                                                           */
/* Historie: 30.10.1996 Grundsteinlegung                                     */
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
#include "asmitree.h"
#include "codepseudo.h"
#include "codevars.h"


typedef struct
         {
          Byte Code;
         } FixedOrder;

typedef struct
         {
          Byte Code;
          bool MayReg;
         } ShiftOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } Condition;


#define AccReg 6
#define HLReg 2

#define FixedOrderCnt 18
#define MoveOrderCnt 8
#define ShiftOrderCnt 10
#define BitOrderCnt 4
#define AccOrderCnt 3
#define ALU2OrderCnt 8
#define ConditionCnt 24

#define ModNone (-1)
#define ModReg8    0
#define MModReg8   (1 << ModReg8)
#define ModReg16   1
#define MModReg16  (1 << ModReg16)
#define ModIReg16  2
#define MModIReg16 (1 << ModIReg16)
#define ModIndReg  3
#define MModIndReg (1 << ModIndReg)
#define ModIdxReg  4
#define MModIdxReg (1 << ModIdxReg)
#define ModDir     5
#define MModDir    (1 << ModDir)
#define ModMem     6
#define MModMem    (1 << ModMem)
#define ModImm     7
#define MModImm    (1 << ModImm)

static int DefaultCondition;

static ShortInt AdrType;
static Byte AdrMode;
static ShortInt OpSize;
static Byte AdrVals[10];
static bool MinOneIs0;

static FixedOrder *FixedOrders;
static FixedOrder *MoveOrders;
static ShiftOrder *ShiftOrders;
static FixedOrder *BitOrders;
static FixedOrder *AccOrders;
static char **ALU2Orders;
static Condition *Conditions;
static PInstTreeNode ITree;

static CPUVar CPU90C141;

/*---------------------------------------------------------------------------*/

	static void ChkAdr(Byte Erl)
{
   if (AdrType!=ModNone)
    if (((1 << AdrType) & Erl)==0)
     {
      WrError(1350); AdrType=ModNone; AdrCnt=0;
     }
}

	static void SetOpSize(ShortInt New)
{
   if (OpSize==-1) OpSize=New;
   else if (OpSize!=New)
    {
     WrError(1131); AdrType=ModNone; AdrCnt=0;
    }
}

	static void DecodeAdr(char *Asc, Byte Erl)
{
#define Reg8Cnt 7
   static char *Reg8Names[Reg8Cnt]={"B","C","D","E","H","L","A"};
#define Reg16Cnt 7
   static char *Reg16Names[Reg16Cnt]={"BC","DE","HL","\0","IX","IY","SP"};
#define IReg16Cnt 3
   static char *IReg16Names[IReg16Cnt]={"IX","IY","SP"};

   Integer z;
   char *p,*ppos,*mpos;
   LongInt DispAcc,DispVal;
   Byte OccFlag,BaseReg;
   bool ok,fnd,NegFlag,NNegFlag,Unknown;
   String Part;

   AdrType=ModNone; AdrCnt=0;

   /* 1. 8-Bit-Register */

   for (z=0; z<Reg8Cnt; z++)
    if (strcasecmp(Asc,Reg8Names[z])==0)
     {
      AdrType=ModReg8; AdrMode=z; SetOpSize(0);
      ChkAdr(Erl); return;
     }

   /* 2. 16-Bit-Register, indiziert */

   if ((Erl & MModIReg16)!=0)
    for (z=0; z<IReg16Cnt; z++)
     if (strcasecmp(Asc,IReg16Names[z])==0)
      {
       AdrType=ModIReg16; AdrMode=z; SetOpSize(1);
       ChkAdr(Erl); return;
      }

   /* 3. 16-Bit-Register, normal */

   for (z=0; z<Reg16Cnt; z++)
    if (strcasecmp(Asc,Reg16Names[z])==0)
     {
      AdrType=ModReg16; AdrMode=z; SetOpSize(1);
      ChkAdr(Erl); return;
     }

   /* Speicheradresse */

   if (IsIndirect(Asc))
    {
     OccFlag=0; BaseReg=0; DispAcc=0; ok=true; NegFlag=false; Unknown=false;
     strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
     do
      {
       ppos=QuotPos(Asc,'+');
       mpos=QuotPos(Asc,'-');
       if (ppos==NULL) p=mpos;
       else if (mpos==NULL) p=ppos;
       else p=min(mpos,ppos);
       NNegFlag=((p!=NULL) && (*p=='-'));
       if (p==NULL)
        {
         strmaxcpy(Part,Asc,255); *Asc='\0';
        }
       else
        {
         *p='\0'; strmaxcpy(Part,Asc,255); strcpy(Asc,p+1);
        }
       fnd=false;
       if (strcasecmp(Part,"A")==0)
        {
 	 fnd=true;
	 ok=((! NegFlag) && ((OccFlag & 1)==0));
	 if (ok) OccFlag+=1; else WrError(1350);
        }
       if (! fnd)
        for (z=0; z<Reg16Cnt; z++)
	 if (strcasecmp(Part,Reg16Names[z])==0)
	  {
	   fnd=true; BaseReg=z;
	   ok=((! NegFlag) && ((OccFlag & 2)==0));
	   if (ok) OccFlag+=2; else WrError(1350);
	  }
       if (! fnd)
        {
	 FirstPassUnknown=false;
	 DispVal=EvalIntExpression(Part,Int32,&ok);
	 if (ok)
	  {
	   if (NegFlag) DispAcc-=DispVal; else DispAcc+=DispVal;
	   if (FirstPassUnknown) Unknown=true;
	  }
        }
       NegFlag=NNegFlag;
      }
     while ((*Asc!='\0') && (ok));
     if (! ok) return;
     if (Unknown) DispAcc&=0x7f;
     switch (OccFlag)
      {
       case 1:
        WrError(1350); break;
       case 3:
        if ((BaseReg!=2) || (DispAcc!=0)) WrError(1350);
        else
 	 {
	  AdrType=ModIdxReg; AdrMode=3;
	 }
        break;
       case 2:
        if ((DispAcc>127) || (DispAcc<-128)) WrError(1320);
        else if (DispAcc==0)
	 {
	  AdrType=ModIndReg; AdrMode=BaseReg;
 	 }
        else if (BaseReg<4) WrError(1350);
        else
 	 {
	  AdrType=ModIdxReg; AdrMode=BaseReg-4;
	  AdrCnt=1; AdrVals[0]=DispAcc & 0xff;
	 }
        break;
       case 0:
        if (DispAcc>0xffff) WrError(1925);
        else if ((Hi(DispAcc)==0xff) && ((Erl & MModDir)!=0))
	 {
	  AdrType=ModDir; AdrCnt=1; AdrVals[0]=Lo(DispAcc);
	 }
        else
	 {
	  AdrType=ModMem; AdrCnt=2;
          AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
	 }
        break;
      }
    }

   /* immediate */

   else
    {
     if ((OpSize==-1) && (MinOneIs0)) OpSize=0;
     switch (OpSize)
      {
       case -1:
        WrError(1130); break;
       case 0:
	AdrVals[0]=EvalIntExpression(Asc,Int8,&ok);
	if (ok)
	 {
	  AdrType=ModImm; AdrCnt=1;
	 }
        break;
       case 1:
	DispVal=EvalIntExpression(Asc,Int16,&ok);
	if (ok)
	 {
	  AdrType=ModImm; AdrCnt=2;
	  AdrVals[0]=Lo(DispVal); AdrVals[1]=Hi(DispVal);
	 }
        break;
      }
    }

   /* gefunden */

   ChkAdr(Erl);
}

/*--------------------------------------------------------------------------*/

	static bool DecodePseudo(void)
{
   return false;
}

	static bool WMemo(char *Name)
{
   String tmp;

   if (Memo(Name)) return true;

   sprintf(tmp,"%sW",Name);
   if (Memo(tmp))
    {
     OpSize=1; return true;
    }
   else return false;
}

	static bool ArgPair(char *Arg1, char *Arg2)
{
   return  (((strcasecmp(ArgStr[1],Arg1)==0) && (strcasecmp(ArgStr[2],Arg2)==0))
	 || ((strcasecmp(ArgStr[1],Arg2)==0) && (strcasecmp(ArgStr[2],Arg1)==0)));
}

/*-------------------------------------------------------------------------*/

/* ohne Argument */

	static void CodeFixed(Word Index)
{
   if (ArgCnt!=0) WrError(1110);
   else
    {
     CodeLen=1; BAsmCode[0]=FixedOrders[Index].Code;
    }
}

        static void CodeMove(Word Index)
{
   if (ArgCnt!=0) WrError(1110);
   else
    {
     CodeLen=2; BAsmCode[0]=0xfe; BAsmCode[1]=MoveOrders[Index].Code;
    }
}

        static void CodeShift(Word Index)
{
   if (ArgCnt!=1) WrError(1110);
   else
    {
     DecodeAdr(ArgStr[1],((ShiftOrders[Index].MayReg)?MModReg8:0)+MModIndReg+MModIdxReg+MModMem+MModDir);
     switch (AdrType)
      {
       case ModReg8:
        CodeLen=2; BAsmCode[0]=0xf8+AdrMode; BAsmCode[1]=ShiftOrders[Index].Code;
        if (AdrMode==AccReg) WrError(10);
        break;
       case ModIndReg:
        CodeLen=2; BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=ShiftOrders[Index].Code;
        break;
       case ModIdxReg:
        CodeLen=2+AdrCnt; BAsmCode[0]=0xf0+AdrMode;
        memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=ShiftOrders[Index].Code;
        break;
       case ModDir:
        CodeLen=3; BAsmCode[0]=0xe7; BAsmCode[1]=AdrVals[0];
        BAsmCode[2]=ShiftOrders[Index].Code;
        break;
       case ModMem:
        CodeLen=4; BAsmCode[0]=0xe3;
        memcpy(BAsmCode+1,AdrVals,AdrCnt);
        BAsmCode[3]=ShiftOrders[Index].Code;
        break;
      }
    }
}

/* Logik */

        static void CodeBit(Word Index)
{
   Byte HReg;
   bool OK;

   if (ArgCnt!=2) WrError(1110);
   else
    {
     HReg=EvalIntExpression(ArgStr[1],UInt3,&OK);
     if (OK)
      {
       DecodeAdr(ArgStr[2],MModReg8+MModIndReg+MModIdxReg+MModMem+MModDir);
       switch (AdrType)
        {
         case ModReg8:
          CodeLen=2;
          BAsmCode[0]=0xf8+AdrMode; BAsmCode[1]=BitOrders[Index].Code+HReg;
          break;
         case ModIndReg:
          CodeLen=2;
          BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=BitOrders[Index].Code+HReg;
          break;
         case ModIdxReg:
          CodeLen=2+AdrCnt; memcpy(BAsmCode+1,AdrVals,AdrCnt);
          BAsmCode[0]=0xf0+AdrMode; BAsmCode[1+AdrCnt]=BitOrders[Index].Code+HReg;
          break;
         case ModMem:
          CodeLen=4; memcpy(BAsmCode+1,AdrVals,AdrCnt);
          BAsmCode[0]=0xe3; BAsmCode[1+AdrCnt]=BitOrders[Index].Code+HReg;
          break;
         case ModDir:
          BAsmCode[1]=AdrVals[0];
          if (Index==4)
           {
            BAsmCode[0]=0xe7; BAsmCode[2]=BitOrders[Index].Code+HReg; CodeLen=3;
           }
          else
           {
            BAsmCode[0]=BitOrders[Index].Code+HReg; CodeLen=2;
           }
          break;
        }
      }
    }
}

        static void CodeAcc(Word Index)
{
   if (ArgCnt!=1) WrError(1110);
   else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
   else
    {
     CodeLen=1; BAsmCode[0]=AccOrders[Index].Code;
    }
}

        static void MakeCode_90C141(void)
{
   Integer z;
   Integer AdrInt;
   bool OK;
   Byte HReg;

   CodeLen=0; DontPrint=false; OpSize=(-1);

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(false)) return;

   if (SearchInstTree(ITree)) return;

   /* Datentransfer */

   if (WMemo("LD"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModIndReg+MModIdxReg+MModDir+MModMem);
       switch (AdrType)
        {
         case ModReg8:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg8+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm);
	  switch (AdrType)
           {
	    case ModReg8:
	     if (HReg==AccReg)
	      {
	       CodeLen=1; BAsmCode[0]=0x20+AdrMode;
	      }
	     else if (AdrMode==AccReg)
	      {
	       CodeLen=1; BAsmCode[0]=0x28+HReg;
	      }
	     else
	      {
	       CodeLen=2; BAsmCode[0]=0xf8+AdrMode; BAsmCode[1]=0x30+HReg;
	      }
             break;
	    case ModIndReg:
	     CodeLen=2; BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=0x28+HReg;
	     break;
	    case ModIdxReg:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xf0+AdrMode;
	     memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=0x28+HReg;
	     break;
	    case ModDir:
	     if (HReg==AccReg)
	      {
	       CodeLen=2; BAsmCode[0]=0x27; BAsmCode[1]=AdrVals[0];
	      }
	     else
	      {
	       CodeLen=3; BAsmCode[0]=0xe7; BAsmCode[1]=AdrVals[0];
	       BAsmCode[2]=0x28+HReg;
	      }
             break;
	    case ModMem:
	     CodeLen=4; BAsmCode[0]=0xe3;
	     memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[3]=0x28+HReg;
	     break;
	    case ModImm:
	     CodeLen=2; BAsmCode[0]=0x30+HReg; BAsmCode[1]=AdrVals[0];
	     break;
	   }
	  break;
         case ModReg16:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg16+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm);
	  switch (AdrType)
           {
	    case ModReg16:
	     if (HReg==HLReg)
	      {
	       CodeLen=1; BAsmCode[0]=0x40+AdrMode;
	      }
	     else if (AdrMode==HLReg)
	      {
	       CodeLen=1; BAsmCode[0]=0x48+HReg;
	      }
	     else
	      {
	       CodeLen=2; BAsmCode[0]=0xf8+AdrMode; BAsmCode[1]=0x38+HReg;
	      }
             break;
	    case ModIndReg:
	     CodeLen=2;
	     BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=0x48+HReg;
	     break;
	    case ModIdxReg:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xf0+AdrMode;
	     memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=0x48+HReg;
	     break;
	    case ModDir:
	     if (HReg==HLReg)
	      {
	       CodeLen=2; BAsmCode[0]=0x47; BAsmCode[1]=AdrVals[0];
	      }
	     else
	      {
	       CodeLen=3; BAsmCode[0]=0xe7; BAsmCode[1]=AdrVals[0];
	       BAsmCode[2]=0x48+HReg;
	      }
             break;
	    case ModMem:
	     CodeLen=4; BAsmCode[0]=0xe3; BAsmCode[3]=0x48+HReg;
	     memcpy(BAsmCode+1,AdrVals,AdrCnt);
	     break;
            case ModImm:
             CodeLen=3; BAsmCode[0]=0x38+HReg;
             memcpy(BAsmCode+1,AdrVals,AdrCnt);
             break;
	   }
	  break;
         case ModIndReg:
         case ModIdxReg:
         case ModDir:
         case ModMem:
	  MinOneIs0=true; HReg=AdrCnt; memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  switch (AdrType)
           {
	    case ModIndReg: BAsmCode[0]=0xe8+AdrMode; break;
	    case ModIdxReg: BAsmCode[0]=0xf4+AdrMode; break;
	    case ModMem:    BAsmCode[0]=0xeb; break;
	    case ModDir:    BAsmCode[0]=0x0f; break;
	   }
	  DecodeAdr(ArgStr[2],MModReg16+MModReg8+MModImm);
	  if (BAsmCode[0]==0x0f)
	   switch (AdrType)
            {
	     case ModReg8:
	      if (AdrMode==AccReg)
	       {
	        CodeLen=2; BAsmCode[0]=0x2f;
	       }
	      else
	       {
	        CodeLen=3; BAsmCode[0]=0xef; BAsmCode[2]=0x20+AdrMode;
	       }
              break;
	     case ModReg16:
	      if (AdrMode==HLReg)
	       {
	        CodeLen=2; BAsmCode[0]=0x4f;
	       }
	      else
	       {
	        CodeLen=3; BAsmCode[0]=0xef; BAsmCode[2]=0x40+AdrMode;
	       }
              break;
	     case ModImm:
	      CodeLen=3+OpSize; BAsmCode[0]=0x37+(OpSize << 3);
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	      break;
	    }
	   else
	    {
	     switch (AdrType)
              {
	       case ModReg8:  BAsmCode[1+HReg]=0x20+AdrMode; break;
	       case ModReg16: BAsmCode[1+HReg]=0x40+AdrMode; break;
	       case ModImm:   BAsmCode[1+HReg]=0x37+(OpSize << 3); break;
	      }
	     memcpy(BAsmCode+2+HReg,AdrVals,AdrCnt);
	     CodeLen=1+HReg+1+AdrCnt;
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
       HReg=Memo("POP") << 3;
       if (strcasecmp(ArgStr[1],"AF")==0)
	{
	 CodeLen=1; BAsmCode[0]=0x56+HReg;
	}
       else
	{
	 DecodeAdr(ArgStr[1],MModReg16);
	 if (AdrType==ModReg16)
	  if (AdrMode==6) WrError(1350);
	  else
	   {
	    CodeLen=1; BAsmCode[0]=0x50+HReg+AdrMode;
	   }
	}
      }
     return;
    }

   if (Memo("LDA"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg16);
       if (AdrType==ModReg16)
	{
	 HReg=0x38+AdrMode;
	 DecodeAdr(ArgStr[2],MModIndReg+MModIdxReg);
	 switch (AdrType)
          {
	   case ModIndReg:
	    if (AdrMode<4) WrError(1350);
	    else
	     {
	      CodeLen=3; BAsmCode[0]=0xf0+AdrMode;
	      BAsmCode[1]=0; BAsmCode[2]=HReg;
	     }
            break;
	   case ModIdxReg:
	    CodeLen=2+AdrCnt; BAsmCode[0]=0xf4+AdrMode;
	    memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=HReg;
	    break;
	  }
	}
      }
     return;
    }

   if (Memo("LDAR"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"HL")!=0) WrError(1350);
     else
      {
       AdrInt=EvalIntExpression(ArgStr[2],Int16,&OK)-(EProgCounter()+2);
       if (OK)
	{
	 CodeLen=3; BAsmCode[0]=0x17;
	 BAsmCode[1]=Lo(AdrInt); BAsmCode[2]=Hi(AdrInt);
	}
      }
     return;
    }

   if (Memo("EX"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (ArgPair("DE","HL"))
      {
       CodeLen=1; BAsmCode[0]=0x08;
      }
     else if ((ArgPair("AF","AF\'") || ArgPair("AF","AF`")))
      {
       CodeLen=1; BAsmCode[0]=0x09;
      }
     else
      {
       DecodeAdr(ArgStr[1],MModReg16+MModIndReg+MModIdxReg+MModMem+MModDir);
       switch (AdrType)
        {
         case ModReg16:
	  HReg=0x50+AdrMode;
	  DecodeAdr(ArgStr[2],MModIndReg+MModIdxReg+MModMem+MModDir);
	  switch (AdrType)
           {
	    case ModIndReg:
	     CodeLen=2; BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=HReg;
	     break;
	    case ModIdxReg:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xf0+AdrMode;
	     memcpy(BAsmCode+1,AdrVals,AdrCnt);
	     BAsmCode[1+AdrCnt]=HReg;
	     break;
	    case ModDir:
	     CodeLen=3; BAsmCode[0]=0xe7; BAsmCode[1]=AdrVals[0];
	     BAsmCode[2]=HReg;
	     break;
	    case ModMem:
	     CodeLen=4; BAsmCode[0]=0xe3; memcpy(BAsmCode+1,AdrVals,AdrCnt);
	     BAsmCode[3]=HReg;
	     break;
 	   }
	  break;
         case ModIndReg:
         case ModIdxReg:
         case ModDir:
         case ModMem:
 	  switch (AdrType)
           {
 	    case ModIndReg:  BAsmCode[0]=0xe0+AdrMode; break;
	    case ModIdxReg:  BAsmCode[0]=0xf0+AdrMode; break;
	    case ModDir:     BAsmCode[0]=0xe7; break;
	    case ModMem:     BAsmCode[0]=0xe3; break;
	   }
	  memcpy(BAsmCode+1,AdrVals,AdrCnt); HReg=2+AdrCnt;
	  DecodeAdr(ArgStr[2],MModReg16);
	  if (AdrType==ModReg16)
	   {
	    BAsmCode[HReg-1]=0x50+AdrMode; CodeLen=HReg;
	   }
	  break;
        }
      }
     return;
    }

   /* Arithmetik */

   for (z=0; z<ALU2OrderCnt; z++)
    if (Memo(ALU2Orders[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
	DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModIdxReg+MModIndReg+MModDir+MModMem);
	switch (AdrType)
         {
	  case ModReg8:
           DecodeAdr(ArgStr[2],MModImm+(((HReg=AdrMode)==AccReg)?MModReg8+MModIndReg+MModIdxReg+MModDir+MModMem:0));
	   switch(AdrType)
            {
	     case ModReg8:
	      CodeLen=2;
	      BAsmCode[0]=0xf8+AdrMode; BAsmCode[1]=0x60+z;
	      break;
	     case ModIndReg:
	      CodeLen=2;
	      BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=0x60+z;
	      break;
	     case ModIdxReg:
	      CodeLen=2+AdrCnt;
	      BAsmCode[0]=0xf0+AdrMode;
	      memcpy(BAsmCode+1,AdrVals,AdrCnt);
	      BAsmCode[1+AdrCnt]=0x60+z;
	      break;
	     case ModDir:
	      CodeLen=2;
	      BAsmCode[0]=0x60+z;
	      BAsmCode[1]=AdrVals[0];
	      break;
	     case ModMem:
	      CodeLen=4;
	      BAsmCode[0]=0xe3; BAsmCode[3]=0x60+z;
	      memcpy(BAsmCode+1,AdrVals,AdrCnt);
	      break;
	     case ModImm:
	      if (HReg==AccReg)
	       {
	        CodeLen=2;
	        BAsmCode[0]=0x68+z; BAsmCode[1]=AdrVals[0];
	       }
	      else
	       {
	        CodeLen=3;
	        BAsmCode[0]=0xf8+HReg; BAsmCode[1]=0x68+z;
	        BAsmCode[2]=AdrVals[0];
	       }
	      break;
	    }
           break;
	  case ModReg16:
	   if ((AdrMode==2) || ((z==0) && (AdrMode>=4)))
	    {
	     HReg=AdrMode;
	     DecodeAdr(ArgStr[2],MModReg16+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm);
	     switch (AdrType)
              {
	       case ModReg16:
	        CodeLen=2;
	        BAsmCode[0]=0xf8+AdrMode;
	        BAsmCode[1]=(HReg>=4) ? 0x14+HReg-4 : 0x70+z;
	        break;
	       case ModIndReg:
	        CodeLen=2;
	        BAsmCode[0]=0xe0+AdrMode;
	        BAsmCode[1]=(HReg>=4) ? 0x14+HReg-4 : 0x70+z;
	        break;
	       case ModIdxReg:
	        CodeLen=2+AdrCnt;
	        BAsmCode[0]=0xf0+AdrMode;
	        memcpy(BAsmCode+1,AdrVals,AdrCnt);
                BAsmCode[1+AdrCnt]=(HReg>=4) ? 0x14+HReg-4 : 0x70+z;
	        break;
	       case ModDir:
	        if (HReg>=4)
	         {
	          CodeLen=3;
	          BAsmCode[0]=0xe7;
	          BAsmCode[1]=AdrVals[0];
	          BAsmCode[2]=0x10+HReg;
	         }
	        else
	         {
	          CodeLen=2;
	          BAsmCode[0]=0x70+z; BAsmCode[1]=AdrVals[0];
	         }
                break;
	       case ModMem:
	        CodeLen=4;
	        BAsmCode[0]=0xe3;
	        memcpy(BAsmCode+1,AdrVals,2);
                BAsmCode[3]=(HReg>=4) ? 0x14+HReg-4 : 0x70+z;
	        break;
	       case ModImm:
	        CodeLen=3;
                BAsmCode[0]=(HReg>=4) ? 0x14+HReg-4 : 0x78+z;
	        memcpy(BAsmCode+1,AdrVals,AdrCnt);
	        break;
	      }
	    }
	   else WrError(1350);
           break;
	  case ModIndReg:
          case ModIdxReg:
          case ModDir:
          case ModMem:
	   OpSize=0;
	   switch (AdrType)
            {
	     case ModIndReg:
	      HReg=3;
	      BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0x68+z;
	      break;
	     case ModIdxReg:
	      HReg=3+AdrCnt;
	      BAsmCode[0]=0xf4+AdrMode; BAsmCode[1+AdrCnt]=0x68+z;
	      memcpy(BAsmCode+1,AdrVals,AdrCnt);
	      break;
	     case ModDir:
	      HReg=4;
	      BAsmCode[0]=0xef; BAsmCode[1]=AdrVals[0]; BAsmCode[2]=0x68+z;
	      break;
	     case ModMem:
	      HReg=5;
	      BAsmCode[0]=0xeb; memcpy(BAsmCode+1,AdrVals,2); BAsmCode[3]=0x68+z;
	      break;
             default:
              HReg=0;
	    }
	   DecodeAdr(ArgStr[2],MModImm);
	   if (AdrType==ModImm)
	    {
	     BAsmCode[HReg-1]=AdrVals[0]; CodeLen=HReg;
	    }
	   break;
	 }
       }
      return;
     }

   if ((WMemo("INC")) || (WMemo("DEC")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       HReg=WMemo("DEC") << 3;
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModIndReg+MModIdxReg+MModDir+MModMem);
       if (OpSize==-1) OpSize=0;
       switch (AdrType)
        {
         case ModReg8:
	  CodeLen=1; BAsmCode[0]=0x80+HReg+AdrMode;
	  break;
         case ModReg16:
	  CodeLen=1; BAsmCode[0]=0x90+HReg+AdrMode;
	  break;
         case ModIndReg:
	  CodeLen=2; BAsmCode[0]=0xe0+AdrMode;
	  BAsmCode[1]=0x87+(OpSize << 4)+HReg;
	  break;
         case ModIdxReg:
	  CodeLen=2+AdrCnt; BAsmCode[0]=0xf0+AdrMode;
	  memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  BAsmCode[1+AdrCnt]=0x87+(OpSize << 4)+HReg;
	  break;
         case ModDir:
	  CodeLen=2; BAsmCode[0]=0x87+(OpSize << 4)+HReg;
	  BAsmCode[1]=AdrVals[0];
	  break;
         case ModMem:
	  CodeLen=4; BAsmCode[0]=0xe3;
	  memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  BAsmCode[3]=0x87+(OpSize << 4)+HReg;
	  BAsmCode[1]=AdrVals[0];
	  break;
        }
      }
     return;
    }

   if ((Memo("INCX")) || (Memo("DECX")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModDir);
       if (AdrType==ModDir)
	{
	 CodeLen=2;
	 BAsmCode[0]=0x07+(Memo("DECX") << 3);
	 BAsmCode[1]=AdrVals[0];
	}
      }
     return;
    }

   if ((Memo("MUL")) || (Memo("DIV")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"HL")!=0) WrError(1350);
     else
      {
       HReg=0x12+Memo("DIV"); OpSize=0;
       DecodeAdr(ArgStr[2],MModReg8+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm);
       switch (AdrType)
        {
         case ModReg8:
	  CodeLen=2;
	  BAsmCode[0]=0xf8+AdrMode; BAsmCode[1]=HReg;
	  break;
         case ModIndReg:
	  CodeLen=2;
	  BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=HReg;
	  break;
         case ModIdxReg:
	  CodeLen=2+AdrCnt;
	  BAsmCode[0]=0xf0+AdrMode; BAsmCode[1+AdrCnt]=HReg;
	  memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  break;
         case ModDir:
	  CodeLen=3; BAsmCode[0]=0xe7;
	  BAsmCode[1]=AdrVals[0]; BAsmCode[2]=HReg;
	  break;
         case ModMem:
	  CodeLen=4; BAsmCode[0]=0xe3; BAsmCode[3]=HReg;
	  memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  break;
         case ModImm:
          CodeLen=3; BAsmCode[0]=0xe7; BAsmCode[1]=AdrVals[0]; BAsmCode[2]=HReg;
          break;
        }
      }
     return;
    }

   /* Spruenge */

   if (Memo("JR"))
    {
     if ((ArgCnt==0) || (ArgCnt>2)) WrError(1110);
     else
      {
       if (ArgCnt==1) z=DefaultCondition;
       else
        {
         NLS_UpString(ArgStr[1]);
         for (z=0; z<ConditionCnt; z++)
          if (strcmp(ArgStr[1],Conditions[z].Name)==0) break;
        }
       if (z>=ConditionCnt) WrError(1360);
       else
	{
	 AdrInt=EvalIntExpression(ArgStr[ArgCnt],Int16,&OK)-(EProgCounter()+2);
	 if (OK)
	  if ((! SymbolQuestionable) && ((AdrInt>127) || (AdrInt<-128))) WrError(1370);
	  else
	   {
	    CodeLen=2;
	    BAsmCode[0]=0xc0+Conditions[z].Code;
	    BAsmCode[1]=AdrInt & 0xff;
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
       if (ArgCnt==1) z=DefaultCondition;
       else
        {
         NLS_UpString(ArgStr[1]);
         for (z=0; z<ConditionCnt; z++)
          if (strcmp(Conditions[z].Name,ArgStr[1])==0) break;
        }
       if (z>=ConditionCnt) WrError(1360);
       else
	{
	 OpSize=1; HReg=Memo("CALL");
	 DecodeAdr(ArgStr[ArgCnt],MModIndReg+MModIdxReg+MModMem+MModImm);
	 if (AdrType==ModImm) AdrType=ModMem;
	 switch (AdrType)
          {
	   case ModIndReg:
	    CodeLen=2;
	    BAsmCode[0]=0xe8+AdrMode;
	    BAsmCode[1]=0xc0+(HReg << 4)+Conditions[z].Code;
	    break;
	   case ModIdxReg:
	    CodeLen=2+AdrCnt;
	    BAsmCode[0]=0xf4+AdrMode;
	    memcpy(BAsmCode+1,AdrVals,AdrCnt);
	    BAsmCode[1+AdrCnt]=0xc0+(HReg << 4)+Conditions[z].Code;
	    break;
	   case ModMem:
	    if (z==DefaultCondition)
	     {
	      CodeLen=3;
	      BAsmCode[0]=0x1a+(HReg << 1);
	      memcpy(BAsmCode+1,AdrVals,AdrCnt);
	     }
	    else
	     {
	      CodeLen=4;
	      BAsmCode[0]=0xeb;
	      memcpy(BAsmCode+1,AdrVals,AdrCnt);
	      BAsmCode[3]=0xc0+(HReg << 4)+Conditions[z].Code;
	     }
	    break;
	  }
        }
      }
     return;
    }

   if (Memo("RET"))
    {
     if ((ArgCnt!=0) && (ArgCnt!=1)) WrError(1110);
     else
      {
       if (ArgCnt==0) z=DefaultCondition;
       else
        {
         NLS_UpString(ArgStr[1]);
         for (z=0; z<ConditionCnt; z++)
          if (strcmp(ArgStr[1],Conditions[z].Name)==0) break;
        }
       if (z>=ConditionCnt) WrError(1360);
       if (z==DefaultCondition)
	{
	 CodeLen=1; BAsmCode[0]=0x1e;
	}
       else if (z<ConditionCnt)
	{
	 CodeLen=2; BAsmCode[0]=0xfe;
	 BAsmCode[1]=0xd0+Conditions[z].Code;
	}
      }
     return;
    }

   if (Memo("DJNZ"))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else
      {
       if (ArgCnt==1)
	{
	 AdrType=ModReg8; AdrMode=0; OpSize=0;
	}
       else DecodeAdr(ArgStr[1],MModReg8+MModReg16);
       if (AdrType!=ModNone)
	if (AdrMode!=0) WrError(1350);
	else
	 {
	  AdrInt=EvalIntExpression(ArgStr[ArgCnt],Int16,&OK)-(EProgCounter()+2);
	  if (OK)
	   if ((! SymbolQuestionable) && ((AdrInt>127) || (AdrInt<-128))) WrError(1370);
	   else
	    {
	     CodeLen=2;
	     BAsmCode[0]=0x18+OpSize;
	     BAsmCode[1]=AdrInt & 0xff;
	    }
	 }
      }
     return;
    }

   if ((Memo("JRL")) || (Memo("CALR")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK)-(EProgCounter()+2);
       if (OK)
	{
	 CodeLen=3;
	 if (Memo("JRL"))
	  {
	   BAsmCode[0]=0x1b;
	   if ((AdrInt>=-128) && (AdrInt<=127)) WrError(20);
	  }
	 else BAsmCode[0]=0x1d;
	 BAsmCode[1]=Lo(AdrInt); BAsmCode[2]=Hi(AdrInt);
	}
      }
     return;
    }

   WrXError(1200,OpPart);
}

/*-------------------------------------------------------------------------*/

	static void AddFixed(char *NName, Byte NCode)
{
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Code=NCode;
   AddInstTree(&ITree,NName,CodeFixed,InstrZ++);
}

        static void AddMove(char *NName, Byte NCode)
{
   if (InstrZ>=MoveOrderCnt) exit(255);
   MoveOrders[InstrZ].Code=NCode;
   AddInstTree(&ITree,NName,CodeMove,InstrZ++);
}

        static void AddShift(char *NName, Byte NCode, bool NMay)
{
   if (InstrZ>=ShiftOrderCnt) exit(255);
   ShiftOrders[InstrZ].Code=NCode;
   ShiftOrders[InstrZ].MayReg=NMay;
   AddInstTree(&ITree,NName,CodeShift,InstrZ++);
}

        static void AddBit(char *NName, Byte NCode)
{
   if (InstrZ>=BitOrderCnt) exit(255);
   BitOrders[InstrZ].Code=NCode;
   AddInstTree(&ITree,NName,CodeBit,InstrZ++);
}

        static void AddAcc(char *NName, Byte NCode)
{
   if (InstrZ>=AccOrderCnt) exit(255);
   AccOrders[InstrZ].Code=NCode;
   AddInstTree(&ITree,NName,CodeAcc,InstrZ++);
}

        static void AddCondition(char *NName, Byte NCode)
{
   if (InstrZ>=ConditionCnt) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
}

	static void InitFields(void)
{
   ITree=NULL;

   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("EXX" ,0x0a); AddFixed("CCF" ,0x0e);
   AddFixed("SCF" ,0x0d); AddFixed("RCF" ,0x0c);
   AddFixed("NOP" ,0x00); AddFixed("HALT",0x01);
   AddFixed("DI"  ,0x02); AddFixed("EI"  ,0x03);
   AddFixed("SWI" ,0xff); AddFixed("RLCA",0xa0);
   AddFixed("RRCA",0xa1); AddFixed("RLA" ,0xa2);
   AddFixed("RRA" ,0xa3); AddFixed("SLAA",0xa4);
   AddFixed("SRAA",0xa5); AddFixed("SLLA",0xa6);
   AddFixed("SRLA",0xa7); AddFixed("RETI",0x1f);

   MoveOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*MoveOrderCnt); InstrZ=0;
   AddMove("LDI" ,0x58);
   AddMove("LDIR",0x59);
   AddMove("LDD" ,0x5a);
   AddMove("LDDR",0x5b);
   AddMove("CPI" ,0x5c);
   AddMove("CPIR",0x5d);
   AddMove("CPD" ,0x5e);
   AddMove("CPDR",0x5f);

   ShiftOrders=(ShiftOrder *) malloc(sizeof(ShiftOrder)*ShiftOrderCnt); InstrZ=0;
   AddShift("RLC",0xa0,true );
   AddShift("RRC",0xa1,true );
   AddShift("RL" ,0xa2,true );
   AddShift("RR" ,0xa3,true );
   AddShift("SLA",0xa4,true );
   AddShift("SRA",0xa5,true );
   AddShift("SLL",0xa6,true );
   AddShift("SRL",0xa7,true );
   AddShift("RLD",0x10,false);
   AddShift("RRD",0x11,false);

   BitOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*BitOrderCnt); InstrZ=0;
   AddBit("BIT" ,0xa8);
   AddBit("SET" ,0xb8);
   AddBit("RES" ,0xb0);
   AddBit("TSET",0x18);

   AccOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*AccOrderCnt); InstrZ=0;
   AddAcc("DAA",0x0b);
   AddAcc("CPL",0x10);
   AddAcc("NEG",0x11);

   ALU2Orders=(char **) malloc(sizeof(char *)*ALU2OrderCnt); InstrZ=0;
   ALU2Orders[InstrZ++]="ADD"; ALU2Orders[InstrZ++]="ADC";
   ALU2Orders[InstrZ++]="SUB"; ALU2Orders[InstrZ++]="SBC";
   ALU2Orders[InstrZ++]="AND"; ALU2Orders[InstrZ++]="XOR";
   ALU2Orders[InstrZ++]="OR";  ALU2Orders[InstrZ++]="CP";

   Conditions=(Condition *) malloc(sizeof(Condition)*ConditionCnt); InstrZ=0;
   AddCondition("F"  ,  0); DefaultCondition=InstrZ; AddCondition("T"  ,  8);
   AddCondition("Z"  ,  6); AddCondition("NZ" , 14);
   AddCondition("C"  ,  7); AddCondition("NC" , 15);
   AddCondition("PL" , 13); AddCondition("MI" ,  5);
   AddCondition("P"  , 13); AddCondition("M"  ,  5);
   AddCondition("NE" , 14); AddCondition("EQ" ,  6);
   AddCondition("OV" ,  4); AddCondition("NOV", 12);
   AddCondition("PE" ,  4); AddCondition("PO" , 12);
   AddCondition("GE" ,  9); AddCondition("LT" ,  1);
   AddCondition("GT" , 10); AddCondition("LE" ,  2);
   AddCondition("UGE", 15); AddCondition("ULT",  7);
   AddCondition("UGT", 11); AddCondition("ULE",  3);
}

	static void DeinitFields(void)
{
   ClearInstTree(&ITree);

   free(FixedOrders);
   free(MoveOrders);
   free(ShiftOrders);
   free(BitOrders);
   free(AccOrders);
   free(ALU2Orders);
   free(Conditions);
}

/*-------------------------------------------------------------------------*/

	static bool ChkPC_90C141(void)
{
   switch (ActPC)
    {
     case SegCode: return (ProgCounter()<=0xffff);
     default: return false;
    }
}

	static bool IsDef_90C141(void)
{
   return false;
}

        static void SwitchFrom_90C141(void)
{
   DeinitFields();
}

	static void SwitchTo_90C141(void)
{
   TurnWords=false; ConstMode=ConstModeIntel; SetIsOccupied=true;

   PCSymbol="$"; HeaderID=0x53; NOPCode=0x00;
   DivideChars=","; HasAttrs=false;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_90C141; ChkPC=ChkPC_90C141; IsDef=IsDef_90C141;
   SwitchFrom=SwitchFrom_90C141; InitFields();
}

	void code90c141_init(void)
{
   CPU90C141=AddCPU("90C141",SwitchTo_90C141);
}
