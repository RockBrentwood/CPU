/* codest6.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator ST6-Familie                                                 */
/*                                                                           */
/* Historie: 14.11.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

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
          Byte Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Word Code;
         } AccOrder;


#define FixedOrderCnt 5

#define RelOrderCnt 4

#define ALUOrderCnt 4

#define AccOrderCnt 3


#define ModNone (-1)
#define ModAcc 0
#define MModAcc (1 << ModAcc)
#define ModDir 1
#define MModDir (1 << ModDir)
#define ModInd 2
#define MModInd (1 << ModInd)


static Byte AdrMode;
static ShortInt AdrType;
static Byte AdrVal;

static LongInt WinAssume;

static SimpProc SaveInitProc;

static CPUVar CPUST6210,CPUST6215,CPUST6220,CPUST6225;

static FixedOrder *FixedOrders;
static FixedOrder *RelOrders;
static FixedOrder *ALUOrders;
static AccOrder *AccOrders;

/*---------------------------------------------------------------------------------*/

	static void AddFixed(char *NName, Byte NCode)
{
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
}

        static void AddRel(char *NName, Byte NCode)
{
   if (InstrZ>=RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ++].Code=NCode;
}

        static void AddALU(char *NName, Byte NCode)
{
   if (InstrZ>=ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name=NName;
   ALUOrders[InstrZ++].Code=NCode;
}

        static void AddAcc(char *NName, Word NCode)
{
   if (InstrZ>=AccOrderCnt) exit(255);
   AccOrders[InstrZ].Name=NName;
   AccOrders[InstrZ++].Code=NCode;
}

	static void InitFields(void)
{
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("NOP" , 0x04);
   AddFixed("RET" , 0xcd);
   AddFixed("RETI", 0x4d);
   AddFixed("STOP", 0x6d);
   AddFixed("WAIT", 0xed);

   RelOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RelOrderCnt); InstrZ=0;
   AddRel("JRZ" , 0x04);
   AddRel("JRNZ", 0x00);
   AddRel("JRC" , 0x06);
   AddRel("JRNC", 0x02);

   ALUOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*ALUOrderCnt); InstrZ=0;
   AddALU("ADD" , 0x47);
   AddALU("AND" , 0xa7);
   AddALU("CP"  , 0x27);
   AddALU("SUB" , 0xc7);

   AccOrders=(AccOrder *) malloc(sizeof(AccOrder)*AccOrderCnt); InstrZ=0;
   AddAcc("COM", 0x002d);
   AddAcc("RLC", 0x00ad);
   AddAcc("SLA", 0xff5f);
}

	static void DeinitFields(void)
{
   free(FixedOrders);
   free(RelOrders);
   free(ALUOrders);
   free(AccOrders);
}

/*---------------------------------------------------------------------------------*/

	static void ResetAdr(void)
{
   AdrType=ModNone; AdrCnt=0;
}

	static void ChkAdr(Byte Mask)
{
   if ((AdrType!=ModNone) && ((Mask && (1 << AdrType))==0))
    {
     ResetAdr(); WrError(1350);
    }
}

	static void DecodeAdr(char *Asc, Byte Mask)
{
#define RegCnt 5
   static char *RegNames[RegCnt+1]={"A","V","W","X","Y"};
   static Byte RegCodes[RegCnt+1]={0xff,0x82,0x83,0x80,0x81};

   bool OK;
   Integer z,AdrInt;

   ResetAdr();

   if ((strcasecmp(Asc,"A")==0) && ((Mask & MModAcc)!=0))
    {
     AdrType=ModAcc; ChkAdr(Mask); return;
    }

   for (z=0; z<RegCnt; z++)
    if (strcasecmp(Asc,RegNames[z])==0)
     {
      AdrType=ModDir; AdrCnt=1; AdrVal=RegCodes[z];
      ChkAdr(Mask); return;
     }

   if (strcasecmp(Asc,"(X)")==0)
    {
     AdrType=ModInd; AdrMode=0; ChkAdr(Mask); return;
    }

   if (strcasecmp(Asc,"(Y)")==0)
    {
     AdrType=ModInd; AdrMode=1; ChkAdr(Mask); return;
    }

   AdrInt=EvalIntExpression(Asc,UInt16,&OK);
   if (OK)
    if ((TypeFlag & (1 << SegCode))!=0)
     {
      AdrType=ModDir; AdrVal=(AdrInt & 0x3f)+0x40; AdrCnt=1;
      if (! FirstPassUnknown)
       if (WinAssume!=(AdrInt >> 6)) WrError(110);
     }
    else
     {
      if (FirstPassUnknown) AdrInt=Lo(AdrInt);
      if (AdrInt>0xff) WrError(1320);
      else
       {
	AdrType=ModDir; AdrVal=AdrInt; ChkAdr(Mask); return;
       }
     }

   ChkAdr(Mask);
}

	static bool DecodePseudo(void)
{
#define ASSUME62Count 1
   static ASSUMERec ASSUME62s[ASSUME62Count]=
   	            {{"ROMBASE", &WinAssume, 0, 0x3f, 0x40}};

   bool OK,Flag;
   Integer z;
   String s;

   if (Memo("SFR"))
    {
     CodeEquate(SegData,0,0xff);
     return true;
    }

   if ((Memo("ASCII")) || (Memo("ASCIZ")))
    {
     if (ArgCnt==0) WrError(1110);
     else
      {
       z=1; Flag=Memo("ASCIZ");
       do
        {
	 EvalStringExpression(ArgStr[z],&OK,s);
	 if (OK)
	  {
	   TranslateString(s);
	   if (CodeLen+strlen(s)+Flag>MaxCodeLen)
	    {
	     WrError(1920); OK=false;
	    }
	   else
	    {
	     memcpy(BAsmCode+CodeLen,s,strlen(s)); CodeLen+=strlen(s);
	     if (Flag) BAsmCode[CodeLen++]=0;
	    }
	  }
	 z++;
        }
       while ((OK) && (z<=ArgCnt));
       if (! OK) CodeLen=0;
      }
     return true;
    }

   if (Memo("BYTE"))
    {
     strmaxcpy(OpPart,"BYT",255); DecodeMotoPseudo(false);
     return true;
    }

   if (Memo("WORD"))
    {
     strmaxcpy(OpPart,"ADR",255); DecodeMotoPseudo(false);
     return true;
    }

   if (Memo("BLOCK"))
    {
     strmaxcpy(OpPart,"DFS",255); DecodeMotoPseudo(false);
     return true;
    }

   if (Memo("ASSUME"))
    {
     CodeASSUME(ASSUME62s,ASSUME62Count);
     return true;
    }

   return false;
}

	static bool IsReg(Byte Adr)
{
   return ((Adr & 0xfc)==0x80);
}

        static Byte MirrBit(Byte inp)
{
   return (((inp & 1) << 2)+(inp & 2)+((inp & 4) >> 2));
}

	static void MakeCode_ST62(void)
{
   Integer z,AdrInt;
   bool OK;

   CodeLen=0; DontPrint=false;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     {
      if (ArgCnt!=0) WrError(1110);
      else
       {
        CodeLen=1; BAsmCode[0]=FixedOrders[z].Code;
       }
      return;
     }

   /* Datentransfer */

   if (Memo("LD"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModAcc+MModDir+MModInd);
       switch (AdrType)
        {
         case ModAcc:
	  DecodeAdr(ArgStr[2],MModDir+MModInd);
	  switch (AdrType)
           {
	    case ModDir:
	     if (IsReg(AdrVal))
    	      {
	       CodeLen=1; BAsmCode[0]=0x35+((AdrVal & 3) << 6);
	      }
	     else
	      {
	       CodeLen=2; BAsmCode[0]=0x1f; BAsmCode[1]=AdrVal;
	      }
             break;
	    case ModInd:
	     CodeLen=1; BAsmCode[0]=0x07+(AdrMode << 3);
	     break;
	   }
	  break;
         case ModDir:
	  DecodeAdr(ArgStr[2],MModAcc);
	  if (AdrType!=ModNone)
	   if (IsReg(AdrVal))
	    {
	     CodeLen=1; BAsmCode[0]=0x3d+((AdrVal & 3) << 6);
	    }
	   else
	    {
	     CodeLen=2; BAsmCode[0]=0x9f; BAsmCode[1]=AdrVal;
	    }
	   break;
         case ModInd:
	  DecodeAdr(ArgStr[2],MModAcc);
	  if (AdrType!=ModNone)
	   {
	    CodeLen=1; BAsmCode[0]=0x87+(AdrMode << 3);
	   }
	  break;
        }
      }
     return;
    }

   if (Memo("LDI"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       AdrInt=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
	{
	 DecodeAdr(ArgStr[1],MModAcc+MModDir);
	 switch (AdrType)
          {
	   case ModAcc:
	    CodeLen=2; BAsmCode[0]=0x17; BAsmCode[1]=Lo(AdrInt);
	    break;
	   case ModDir:
	    CodeLen=3; BAsmCode[0]=0x0d; BAsmCode[1]=AdrVal;
	    BAsmCode[2]=Lo(AdrInt);
	    break;
	  }
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
        AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+1);
        if (OK)
         if ((! SymbolQuestionable) && ((AdrInt<-16) || (AdrInt>15))) WrError(1370);
         else
          {
           CodeLen=1;
           BAsmCode[0]=RelOrders[z].Code+((AdrInt << 3) & 0xf8);
          }
       }
      return;
     }

   if ((Memo("JP")) || (Memo("CALL")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
        if ((AdrInt<0) || (AdrInt>0xfff)) WrError(1925);
        else
         {
          CodeLen=2;
          BAsmCode[0]=0x01+(Memo("JP") << 3)+((AdrInt & 0x00f) << 4);
          BAsmCode[1]=AdrInt >> 4;
         }
      }
     return;
    }

   /* Arithmetik */

   for (z=0; z<ALUOrderCnt; z++)
    if (strncmp(ALUOrders[z].Name,OpPart,strlen(ALUOrders[z].Name))==0)
     switch (OpPart[strlen(ALUOrders[z].Name)])
      {
       case '\0':
        if (ArgCnt!=2) WrError(1110);
        else
         {
          DecodeAdr(ArgStr[1],MModAcc);
          if (AdrType!=ModNone)
           {
            DecodeAdr(ArgStr[2],MModDir+MModInd);
            switch (AdrType)
             {
              case ModDir:
               CodeLen=2; BAsmCode[0]=ALUOrders[z].Code+0x18;
               BAsmCode[1]=AdrVal;
               break;
              case ModInd:
               CodeLen=1; BAsmCode[0]=ALUOrders[z].Code+(AdrMode << 3);
               break;
             }
           }
         }
        return;
       case 'I':
        if (ArgCnt!=2) WrError(1110);
        else
         {
          DecodeAdr(ArgStr[1],MModAcc);
          if (AdrType!=ModNone)
           {
            BAsmCode[1]=EvalIntExpression(ArgStr[2],Int8,&OK);
            if (OK)
             {
              CodeLen=2; BAsmCode[0]=ALUOrders[z].Code+0x10;
             }
           }
         }
        return;
      }

   if (Memo("CLR"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModAcc+MModDir);
       switch (AdrType)
        {
         case ModAcc:
	  CodeLen=2; BAsmCode[0]=0xdf; BAsmCode[1]=0xff;
	  break;
         case ModDir:
	  CodeLen=3; BAsmCode[0]=0x0d; BAsmCode[1]=AdrVal; BAsmCode[2]=0;
	  break;
        }
      }
     return;
    }

   for (z=0; z<AccOrderCnt; z++)
    if (Memo(AccOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else
       {
        DecodeAdr(ArgStr[1],MModAcc);
        if (AdrType!=ModNone)
         {
          OK=(Hi(AccOrders[z].Code)!=0);
          CodeLen=1+OK;
          BAsmCode[0]=Lo(AccOrders[z].Code);
          if (OK) BAsmCode[1]=Hi(AccOrders[z].Code);
         }
       }
      return;
     }

   if ((Memo("INC")) || (Memo("DEC")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModDir+MModInd);
       switch (AdrType)
        {
         case ModDir:
	  if (IsReg(AdrVal))
	   {
	    CodeLen=1; BAsmCode[0]=0x15+((AdrVal & 3) << 6);
	    if (Memo("DEC")) BAsmCode[0]+=8;
	   }
	  else
	   {
	    CodeLen=2; BAsmCode[0]=0x7f+(Memo("DEC") << 7);
	    BAsmCode[1]=AdrVal;
	   }
          break;
         case ModInd:
	  CodeLen=1;
	  BAsmCode[0]=0x67+(AdrMode << 3)+(Memo("DEC") << 7);
	  break;
        }
      }
     return;
    }

   /* Bitbefehle */

   if ((Memo("SET")) || (Memo("RES")))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       BAsmCode[0]=MirrBit(EvalIntExpression(ArgStr[1],UInt3,&OK));
       if (OK)
	{
	 DecodeAdr(ArgStr[2],MModDir);
	 if (AdrType!=ModNone)
	  {
	   CodeLen=2;
	   BAsmCode[0]=(BAsmCode[0] << 5)+(Memo("SET") << 4)+0x0b;
	   BAsmCode[1]=AdrVal;
	  }
	}
      }
     return;
    }

   if ((Memo("JRR")) || (Memo("JRS")))
    {
     if (ArgCnt!=3) WrError(1110);
     else
      {
       BAsmCode[0]=MirrBit(EvalIntExpression(ArgStr[1],UInt3,&OK));
       if (OK)
	{
	 BAsmCode[0]=(BAsmCode[0] << 5)+3+(Memo("JRS") << 4);
	 DecodeAdr(ArgStr[2],MModDir);
	 if (AdrType!=ModNone)
	  {
	   BAsmCode[1]=AdrVal;
           AdrInt=EvalIntExpression(ArgStr[3],UInt16,&OK)-(EProgCounter()+3);
	   if (OK)
	    if ((! SymbolQuestionable) && ((AdrInt>127) || (AdrInt<-128))) WrError(1370);
	    else
	     {
	      CodeLen=3; BAsmCode[2]=Lo(AdrInt);
	     }
	  }
	}
      }
     return;
    }

   WrXError(1200,OpPart);
}

	static void InitCode_ST62(void)
{
   SaveInitProc();
   WinAssume=0x40;
}

	static bool ChkPC_ST62(void)
{
   switch (ActPC)
    {
     case SegCode:
      return (ProgCounter()<((MomCPU<CPUST6220)?0x1000:0x800));
     case SegData:
      return (ProgCounter()<0x100);
     default: return false;
    }
}

	static bool IsDef_ST62(void)
{
   return (Memo("SFR"));
}

        static void SwitchFrom_ST62(void)
{
   DeinitFields();
}

	static void SwitchTo_ST62(void)
{
   TurnWords=false; ConstMode=ConstModeIntel; SetIsOccupied=true;

   PCSymbol="PC"; HeaderID=0x78; NOPCode=0x04;
   DivideChars=","; HasAttrs=false;

   ValidSegs=(1<<SegCode)+(1<<SegData);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;

   MakeCode=MakeCode_ST62; ChkPC=ChkPC_ST62; IsDef=IsDef_ST62;
   SwitchFrom=SwitchFrom_ST62; InitFields();
}

	void codest6_init(void)
{
   CPUST6210=AddCPU("ST6210",SwitchTo_ST62);
   CPUST6215=AddCPU("ST6215",SwitchTo_ST62);
   CPUST6220=AddCPU("ST6220",SwitchTo_ST62);
   CPUST6225=AddCPU("ST6225",SwitchTo_ST62);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_ST62;
}

