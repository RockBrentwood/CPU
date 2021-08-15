/* codecop8.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegeneratormodul COP8-Familie                                           */
/*                                                                           */
/* Historie: 7.10.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>

#include "bpemu.h"
#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

#define ModNone (-1)
#define ModAcc 0
#define MModAcc (1 << ModAcc)
#define ModBInd 1
#define MModBInd (1 << ModBInd)
#define ModBInc 2
#define MModBInc (1 << ModBInc)
#define ModBDec 3
#define MModBDec (1 << ModBDec)
#define ModXInd 4
#define MModXInd (1 << ModXInd)
#define ModXInc 5
#define MModXInc (1 << ModXInc)
#define ModXDec 6
#define MModXDec (1 << ModXDec)
#define ModDir 7
#define MModDir (1 << ModDir)
#define ModImm 8
#define MModImm (1 << ModImm)

#define DirPrefix 0xbd
#define BReg 0xfe

#define FixedOrderCnt 13
#define AccOrderCnt 9
#define AccMemOrderCnt 7
#define BitOrderCnt 3

   typedef struct
            {
             char *Name;
             Byte Code;
            } FixedOrder;

static CPUVar CPUCOP87L84;

static FixedOrder *FixedOrders;
static FixedOrder *AccOrders;
static FixedOrder *AccMemOrders;
static FixedOrder *BitOrders;

static ShortInt AdrMode;
static Byte AdrVal;
static bool BigFlag;

/*---------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Byte NCode)
{
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
}

        static void AddAcc(char *NName, Byte NCode)
{
   if (InstrZ>=AccOrderCnt) exit(255);
   AccOrders[InstrZ].Name=NName;
   AccOrders[InstrZ++].Code=NCode;
}

        static void AddAccMem(char *NName, Byte NCode)
{
   if (InstrZ>=AccMemOrderCnt) exit(255);
   AccMemOrders[InstrZ].Name=NName;
   AccMemOrders[InstrZ++].Code=NCode;
}

        static void AddBit(char *NName, Byte NCode)
{
   if (InstrZ>=BitOrderCnt) exit(255);
   BitOrders[InstrZ].Name=NName;
   BitOrders[InstrZ++].Code=NCode;
}

        static void InitFields(void)
{
   FixedOrders=(FixedOrder *) malloc(FixedOrderCnt*sizeof(FixedOrder)); InstrZ=0;
   AddFixed("LAID" ,0xa4);  AddFixed("SC"   ,0xa1);  AddFixed("RC"   ,0xa0);
   AddFixed("IFC"  ,0x88);  AddFixed("IFNC" ,0x89);  AddFixed("VIS"  ,0xb4);
   AddFixed("JID"  ,0xa5);  AddFixed("RET"  ,0x8e);  AddFixed("RETSK",0x8d);
   AddFixed("RETI" ,0x8f);  AddFixed("INTR" ,0x00);  AddFixed("NOP"  ,0xb8);
   AddFixed("RPND" ,0xb5);

   AccOrders=(FixedOrder *) malloc(AccOrderCnt*sizeof(FixedOrder)); InstrZ=0;
   AddAcc("CLR"  ,0x64);  AddAcc("INC"  ,0x8a);  AddAcc("DEC"  ,0x8b);
   AddAcc("DCOR" ,0x66);  AddAcc("RRC"  ,0xb0);  AddAcc("RLC"  ,0xa8);
   AddAcc("SWAP" ,0x65);  AddAcc("POP"  ,0x8c);  AddAcc("PUSH" ,0x67);

   AccMemOrders=(FixedOrder *) malloc(AccMemOrderCnt*sizeof(FixedOrder)); InstrZ=0;
   AddAccMem("ADD"  ,0x84);  AddAccMem("ADC"  ,0x80);  AddAccMem("SUBC" ,0x81);
   AddAccMem("AND"  ,0x85);  AddAccMem("OR"   ,0x87);  AddAccMem("XOR"  ,0x86);
   AddAccMem("IFGT" ,0x83);

   BitOrders=(FixedOrder *) malloc(BitOrderCnt*sizeof(FixedOrder)); InstrZ=0;
   AddBit("IFBIT",0x70); AddBit("SBIT",0x78); AddBit("RBIT",0x68);
}

        static void DeinitFields(void)
{
   free(FixedOrders);
   free(AccOrders);
   free(AccMemOrders);
   free(BitOrders);
}

/*---------------------------------------------------------------------------*/

	static void ChkAdr(Word Mask)
{
   if ((AdrMode!=ModNone) && ((Mask && (1 << AdrMode))==0))
    {
     AdrMode=ModNone; WrError(1350);
    }
}

        static void DecodeAdr(char *Asc, Word Mask)
{
   static char *ModStrings[ModXDec+1]=
              {"A","[B]","[B+]","[B-]","[X]","[X+]","[X-]"};

   Integer z;
   bool OK;

   AdrMode=ModNone;

   /* indirekt/Akku */

   for (z=ModAcc; z<=ModXDec; z++)
    if (strcasecmp(Asc,ModStrings[z])==0)
     {
      AdrMode=z; ChkAdr(Mask); return;
     }

   /* immediate */

   if (*Asc=='#')
    {
     AdrVal=EvalIntExpression(Asc+1,Int8,&OK);
     if (OK) AdrMode=ModImm;
     ChkAdr(Mask); return;
    }

   /* direkt */

   AdrVal=EvalIntExpression(Asc,Int8,&OK);
   if (OK)
    {
     AdrMode=ModDir; ChkSpace(SegData);
    }

   ChkAdr(Mask);
}

/*---------------------------------------------------------------------------*/

	static bool DecodePseudo(void)
{
   bool ValOK;
   Word Size,Value,t,z;

   if (Memo("SFR"))
    {
     CodeEquate(SegData,0,0xff);
     return true;
    };

   if (Memo("ADDR"))
    {
     strcpy(OpPart,"DB"); BigFlag=true;
    }

   if (Memo("ADDRW"))
    {
     strcpy(OpPart,"DW"); BigFlag=true;
    }

   if (Memo("BYTE")) strcpy(OpPart,"DB");

   if (Memo("WORD")) strcpy(OpPart,"DW");

   if ((Memo("DSB")) || (Memo("DSW")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       FirstPassUnknown=false;
       Size=EvalIntExpression(ArgStr[1],UInt16,&ValOK);
       if (FirstPassUnknown) WrError(1820);
       if ((ValOK) && (! FirstPassUnknown))
	{
	 DontPrint=true;
         if (Memo("DSW")) Size+=Size;
         CodeLen=Size;
	 if (MakeUseList)
	  if (AddChunk(SegChunks+ActPC,ProgCounter(),CodeLen,ActPC==SegCode)) WrError(90);
	}
      }
     return true;
    }

   if (Memo("FB"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       FirstPassUnknown=false;
       Size=EvalIntExpression(ArgStr[1],UInt16,&ValOK);
       if (FirstPassUnknown) WrError(1820);
       if ((ValOK) && (! FirstPassUnknown))
        if (Size>MaxCodeLen) WrError(1920);
	else
	 {
          BAsmCode[0]=EvalIntExpression(ArgStr[2],Int8,&ValOK);
          if (ValOK)
           {
            CodeLen=Size;
            memset(BAsmCode+1,BAsmCode[0],Size-1);
           }
	 }
      }
     return true;
    }

   if (Memo("FW"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       FirstPassUnknown=false;
       Size=EvalIntExpression(ArgStr[1],UInt16,&ValOK);
       if (FirstPassUnknown) WrError(1820);
       if ((ValOK) && (! FirstPassUnknown))
        if ((Size << 1)>MaxCodeLen) WrError(1920);
	else
	 {
          Value=EvalIntExpression(ArgStr[2],Int16,&ValOK);
          if (ValOK)
           {
            CodeLen=Size << 1; t=0;
            for (z=0; z<Size; z++)
             {
              BAsmCode[t++]=Lo(Value); BAsmCode[t++]=Hi(Value);
             }
           }
	 }
      }
     return true;
    }

   return false;
}

        static void MakeCode_COP8(void)
{
   Integer z,AdrInt;
   Byte HReg;
   bool OK;
   Word AdrWord;

   CodeLen=0; DontPrint=false; BigFlag=false;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(BigFlag)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if Memo(FixedOrders[z].Name)
     {
      if (ArgCnt!=0) WrError(1110);
      else
       {
        BAsmCode[0]=FixedOrders[z].Code; CodeLen=1;
       }
      return;
     }

   /* Datentransfer */

   if (Memo("LD"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModAcc+MModDir+MModBInd+MModBInc+MModBDec);
       switch (AdrMode)
        {
         case ModAcc:
          DecodeAdr(ArgStr[2],MModDir+MModImm+MModBInd+MModXInd+MModBInc+MModXInc+MModBDec+MModXDec);
          switch (AdrMode)
           {
            case ModDir:
             BAsmCode[0]=0x9d; BAsmCode[1]=AdrVal; CodeLen=2;
             break;
            case ModImm:
             BAsmCode[0]=0x98; BAsmCode[1]=AdrVal; CodeLen=2;
             break;
            case ModBInd:
             BAsmCode[0]=0xae; CodeLen=1;
             break;
            case ModXInd:
             BAsmCode[0]=0xbe; CodeLen=1;
             break;
            case ModBInc:
             BAsmCode[0]=0xaa; CodeLen=1;
             break;
            case ModXInc:
             BAsmCode[0]=0xba; CodeLen=1;
             break;
            case ModBDec:
             BAsmCode[0]=0xab; CodeLen=1;
             break;
            case ModXDec:
             BAsmCode[0]=0xbb; CodeLen=1;
             break;
           }
          break;
         case ModDir:
          HReg=AdrVal; DecodeAdr(ArgStr[2],MModImm);
          if (AdrMode==ModImm)
           if (HReg==BReg)
            if (AdrVal<=15)
             {
              BAsmCode[0]=0x5f-AdrVal; CodeLen=1;
             }
            else
             {
              BAsmCode[0]=0x9f; BAsmCode[1]=AdrVal; CodeLen=2;
             }
           else if (HReg>=0xf0)
            {
             BAsmCode[0]=HReg-0x20; BAsmCode[1]=AdrVal; CodeLen=2;
            }
           else
            {
             BAsmCode[0]=0xbc; BAsmCode[1]=HReg; BAsmCode[2]=AdrVal; CodeLen=3;
            }
          break;
         case ModBInd:
          DecodeAdr(ArgStr[2],MModImm);
          if (AdrMode!=ModNone)
           {
            BAsmCode[0]=0x9e; BAsmCode[1]=AdrVal; CodeLen=2;
           }
          break;
         case ModBInc:
          DecodeAdr(ArgStr[2],MModImm);
          if (AdrMode!=ModNone)
           {
            BAsmCode[0]=0x9a; BAsmCode[1]=AdrVal; CodeLen=2;
           }
          break;
         case ModBDec:
          DecodeAdr(ArgStr[2],MModImm);
          if (AdrMode!=ModNone)
           {
            BAsmCode[0]=0x9b; BAsmCode[1]=AdrVal; CodeLen=2;
           }
          break;
        }
      }
     return;
    }

   if (Memo("X"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       if (strcasecmp(ArgStr[1],"A")!=0)
        {
         strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
        }
       DecodeAdr(ArgStr[1],MModAcc);
       if (AdrMode!=ModNone)
        {
         DecodeAdr(ArgStr[2],MModDir+MModBInd+MModXInd+MModBInc+MModXInc+MModBDec+MModXDec);
         switch (AdrMode)
          {
           case ModDir:
            BAsmCode[0]=0x9c; BAsmCode[1]=AdrVal; CodeLen=2;
            break;
           case ModBInd:
            BAsmCode[0]=0xa6; CodeLen=1;
            break;
           case ModBInc:
            BAsmCode[0]=0xa2; CodeLen=1;
            break;
           case ModBDec:
            BAsmCode[0]=0xa3; CodeLen=1;
            break;
           case ModXInd:
            BAsmCode[0]=0xb6; CodeLen=1;
            break;
           case ModXInc:
            BAsmCode[0]=0xb2; CodeLen=1;
            break;
           case ModXDec:
            BAsmCode[0]=0xb3; CodeLen=1;
            break;
          }
        }
      }
     return;
    }

   /* Arithmetik */

   for (z=0; z<AccOrderCnt; z++)
    if Memo(AccOrders[z].Name)
     {
      if (ArgCnt!=1) WrError(1110);
      else
       {
        DecodeAdr(ArgStr[1],MModAcc);
        if (AdrMode!=ModNone)
         {
          BAsmCode[0]=AccOrders[z].Code; CodeLen=1;
         }
       }
      return;
     }

   for (z=0; z<AccMemOrderCnt; z++)
    if Memo(AccMemOrders[z].Name)
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
        DecodeAdr(ArgStr[1],MModAcc);
        if (AdrMode!=ModNone)
         {
          DecodeAdr(ArgStr[2],MModDir+MModImm+MModBInd);
          switch (AdrMode)
           {
            case ModBInd:
             BAsmCode[0]=AccMemOrders[z].Code; CodeLen=1;
             break;
            case ModImm:
             BAsmCode[0]=AccMemOrders[z].Code+0x10; BAsmCode[1]=AdrVal;
             CodeLen=2;
             break;
            case ModDir:
             BAsmCode[0]=DirPrefix; BAsmCode[1]=AdrVal;
             BAsmCode[2]=AccMemOrders[z].Code;
             CodeLen=3;
             break;
           }
         }
       }
      return;
     }

   if (Memo("ANDSZ"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModAcc);
       if (AdrMode!=ModNone)
        {
         DecodeAdr(ArgStr[2],MModImm);
         if (AdrMode==ModImm)
          {
           BAsmCode[0]=0x60; BAsmCode[1]=AdrVal; CodeLen=2;
          }
        }
      }
     return;
    }

   /* Bedingungen */

   if (Memo("IFEQ"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModAcc+MModDir);
       switch (AdrMode)
        {
         case ModAcc:
          DecodeAdr(ArgStr[2],MModDir+MModBInd+MModImm);
          switch (AdrMode)
           {
            case ModDir:
             BAsmCode[0]=DirPrefix; BAsmCode[1]=AdrVal; BAsmCode[2]=0x82;
             CodeLen=3;
             break;
            case ModBInd:
             BAsmCode[0]=0x82; CodeLen=1;
             break;
            case ModImm:
             BAsmCode[0]=0x92; BAsmCode[1]=AdrVal; CodeLen=2;
             break;
           }
          break;
         case ModDir:
          BAsmCode[1]=AdrVal;
          DecodeAdr(ArgStr[2],MModImm);
          if (AdrMode==ModImm)
           {
            BAsmCode[0]=0xa9; BAsmCode[2]=AdrVal; CodeLen=3;
           }
          break;
        }
      }
     return;
    }

   if (Memo("IFNE"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModAcc);
       switch (AdrMode)
        {
         case ModAcc:
          DecodeAdr(ArgStr[2],MModDir+MModBInd+MModImm);
          switch (AdrMode)
           {
            case ModDir:
             BAsmCode[0]=DirPrefix; BAsmCode[1]=AdrVal; BAsmCode[2]=0xb9;
             CodeLen=3;
             break;
            case ModBInd:
             BAsmCode[0]=0xb9; CodeLen=1;
             break;
            case ModImm:
             BAsmCode[0]=0x99; BAsmCode[1]=AdrVal; CodeLen=2;
             break;
           }
          break;
        }
      }
     return;
    }

   if (Memo("IFBNE"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      {
       BAsmCode[0]=EvalIntExpression(ArgStr[1]+1,UInt4,&OK);
       if (OK)
        {
         BAsmCode[0]+=0x40; CodeLen=1;
        }
      }
     return;
    }

   /* Bitbefehle */

   for (z=0; z<BitOrderCnt; z++)
    if Memo(BitOrders[z].Name)
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
        HReg=EvalIntExpression(ArgStr[1],UInt3,&OK);
        if (OK)
         {
          DecodeAdr(ArgStr[2],MModDir+MModBInd);
          switch (AdrMode)
           {
            case ModDir:
             BAsmCode[0]=DirPrefix; BAsmCode[1]=AdrVal;
             BAsmCode[2]=BitOrders[z].Code+HReg; CodeLen=3;
             break;
            case ModBInd:
             BAsmCode[0]=BitOrders[z].Code+HReg; CodeLen=1;
             break;
           }
         }
       }
      return;
     }

   /* Spruenge */

   if ((Memo("JMP")) || (Memo("JSR")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrWord=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
        if (((EProgCounter()+2) >> 12)!=(AdrWord >> 12)) WrError(1910);
        else
         {
          ChkSpace(SegCode);
          BAsmCode[0]=0x20+(Memo("JSR") << 4)+((AdrWord >> 8) & 15);
          BAsmCode[1]=Lo(AdrWord);
          CodeLen=2;
         }
      }
     return;
    }

   if ((Memo("JMPL")) || (Memo("JSRL")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrWord=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
        {
         ChkSpace(SegCode);
         BAsmCode[0]=0xac+Memo("JSRL");
         BAsmCode[1]=Hi(AdrWord);
         BAsmCode[2]=Lo(AdrWord);
         CodeLen=3;
        }
      }
     return;
    }

   if (Memo("JP"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+1);
       if (OK)
        if (AdrInt==0)
         {
          BAsmCode[0]=NOPCode; CodeLen=1; WrError(60);
         }
        else if (((AdrInt>31) || (AdrInt<-32)) && (! SymbolQuestionable)) WrError(1370);
        else
         {
          BAsmCode[0]=AdrInt & 0xff; CodeLen=1;
         }
      }
     return;
    }

   if (Memo("DRSZ"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       FirstPassUnknown=false;
       DecodeAdr(ArgStr[1],MModDir);
       if (FirstPassUnknown) AdrVal|=0xf0;
       if (AdrVal<0xf0) WrError(1315);
       else
        {
         BAsmCode[0]=AdrVal-0x30; CodeLen=1;
        }
      }
     return;
    }

   WrXError(1200,OpPart);
}

        static bool ChkPC_COP8(void)
{
   switch (ActPC)
    {
     case SegCode  : return (ProgCounter()<0x2000);
     case SegData  : return (ProgCounter()< 0x100);
     default: return false;
    }
}

        static bool IsDef_COP8(void)
{
   return (Memo("SFR"));
}

        static void SwitchFrom_COP8(void)
{
   DeinitFields();
}

        static void SwitchTo_COP8(void)
{
   TurnWords=false; ConstMode=ConstModeC; SetIsOccupied=false;

   PCSymbol="."; HeaderID=0x6f; NOPCode=0xb8;
   DivideChars=","; HasAttrs=false;

   ValidSegs=(1<<SegCode)|(1<<SegData);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;

   MakeCode=MakeCode_COP8; ChkPC=ChkPC_COP8; IsDef=IsDef_COP8;
   SwitchFrom=SwitchFrom_COP8; InitFields();
}

	void codecop8_init(void)
{
   CPUCOP87L84=AddCPU("COP87L84",SwitchTo_COP8);
}


