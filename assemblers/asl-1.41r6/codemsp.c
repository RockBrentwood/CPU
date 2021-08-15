/* codemsp.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator MSP430                                                      */
/*                                                                           */
/* Historie:                                                                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "endian.h"
#include "stringutil.h"
#include "bpemu.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

#define TwoOpCount 12
#define OneOpCount 6
#define JmpCount 10

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          bool MayByte;
          Word Code;
         } OneOpOrder;


static CPUVar CPUMSP430;

static FixedOrder *TwoOpOrders;
static OneOpOrder *OneOpOrders;
static FixedOrder *JmpOrders;

static Word AdrMode,AdrMode2,AdrPart,AdrPart2;
static Byte AdrCnt2;
static Word AdrVal,AdrVal2;
static Byte OpSize;
static Word PCDist;

/*-------------------------------------------------------------------------*/

        static void AddTwoOp(char *NName, Word NCode)
{
   if (InstrZ>=TwoOpCount) exit(255);
   TwoOpOrders[InstrZ].Name=NName;
   TwoOpOrders[InstrZ++].Code=NCode;
}

        static void AddOneOp(char *NName, bool NMay, Word NCode)
{
   if (InstrZ>=OneOpCount) exit(255);
   OneOpOrders[InstrZ].Name=NName;
   OneOpOrders[InstrZ].MayByte=NMay;
   OneOpOrders[InstrZ++].Code=NCode;
}

        static void AddJmp(char *NName, Word NCode)
{
   if (InstrZ>=JmpCount) exit(255);
   JmpOrders[InstrZ].Name=NName;
   JmpOrders[InstrZ++].Code=NCode;
}

        static void InitFields(void)
{
   TwoOpOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*TwoOpCount); InstrZ=0;
   AddTwoOp("MOV" ,0x4000); AddTwoOp("ADD" ,0x5000);
   AddTwoOp("ADDC",0x6000); AddTwoOp("SUBC",0x7000);
   AddTwoOp("SUB" ,0x8000); AddTwoOp("CMP" ,0x9000);
   AddTwoOp("DADD",0xa000); AddTwoOp("BIT" ,0xb000);
   AddTwoOp("BIC" ,0xc000); AddTwoOp("BIS" ,0xd000);
   AddTwoOp("XOR" ,0xe000); AddTwoOp("AND" ,0xf000);

   OneOpOrders=(OneOpOrder *) malloc(sizeof(OneOpOrder)*OneOpCount); InstrZ=0;
   AddOneOp("RRC" ,true ,0x1000); AddOneOp("RRA" ,true ,0x1100);
   AddOneOp("PUSH",true ,0x1200); AddOneOp("SWPB",false,0x1080);
   AddOneOp("CALL",false,0x1280); AddOneOp("SXT" ,false,0x1180);

   JmpOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*JmpCount); InstrZ=0;
   AddJmp("JNE" ,0x2000); AddJmp("JNZ" ,0x2000);
   AddJmp("JE"  ,0x2400); AddJmp("JZ"  ,0x2400);
   AddJmp("JNC" ,0x2800); AddJmp("JC"  ,0x2c00);
   AddJmp("JN"  ,0x3000); AddJmp("JGE" ,0x3400);
   AddJmp("JL"  ,0x3800); AddJmp("JMP" ,0x3C00);
}

        static void DeinitFields(void)
{
   free(TwoOpOrders);
   free(OneOpOrders);
   free(JmpOrders);
}

/*-------------------------------------------------------------------------*/

        static void ResetAdr(void)
{
   AdrMode=0xff; AdrCnt=0;
}

	static void ChkAdr(Byte Mask)
{
    if ((AdrMode!=0xff) && ((Mask & (1 << AdrMode))==0))
    {
     ResetAdr(); WrError(1350);
    }
}

        static bool DecodeReg(char *Asc, Word *Erg)
{
   bool OK;

   if (strcasecmp(Asc,"PC")==0)
    {
     *Erg=0; return true;
    }
   else if (strcasecmp(Asc,"SP")==0)
    {
     *Erg=1; return true;
    }
   else if (strcasecmp(Asc,"SR")==0)
    {
     *Erg=2; return true;
    }
   if ((toupper(*Asc)=='R') && (strlen(Asc)>=2) && (strlen(Asc)<=3))
    {
     *Erg=ConstLongInt(Asc+1,&OK);
     return ((OK) && (*Erg<16));
    }
   return false;
}

        static void DecodeAdr(char *Asc, Byte Mask, bool MayImm)
{
   Word AdrWord;
   bool OK;
   char *p;

   ResetAdr();

   /* immediate */

   if (*Asc=='#')
    {
     if (! MayImm) WrError(1350);
     else
      {
       AdrWord=EvalIntExpression(Asc+1,(OpSize==1)?Int8:Int16,&OK);
       if (OK)
        {
         switch (AdrWord)
          {
           case 0:
            AdrPart=3; AdrMode=0;
            break;
           case 1:
            AdrPart=3; AdrMode=1;
            break;
           case 2:
            AdrPart=3; AdrMode=2;
            break;
           case 4:
            AdrPart=2; AdrMode=2;
            break;
           case 8:
            AdrPart=2; AdrMode=3;
            break;
           case 0xffff:
            AdrPart=3; AdrMode=3;
            break;
           default:
            AdrVal=AdrWord; AdrCnt=1;
            AdrPart=0; AdrMode=3;
            break;
          }
        }
      }
     ChkAdr(Mask); return;
    }

   /* absolut */

   if (*Asc=='&')
    {
     AdrVal=EvalIntExpression(Asc+1,UInt16,&OK);
     if (OK)
      {
       AdrMode=1; AdrPart=2; AdrCnt=1;
      }
     ChkAdr(Mask); return;
    }

   /* Register */

   if (DecodeReg(Asc,&AdrPart))
    {
     if (AdrPart==3) WrXError(1445,Asc);
     else AdrMode=0;
     ChkAdr(Mask); return;
    }

   /* Displacement */

   if (Asc[strlen(Asc)-1]==')')
    {
     Asc[strlen(Asc)-1]='\0';
     p=RQuotPos(Asc,'(');
     if (p!=NULL)
      {
       if (DecodeReg(p+1,&AdrPart))
        {
         *p='\0';
         AdrVal=EvalIntExpression(Asc,Int16,&OK);
         if (OK)
          if ((AdrPart==2) || (AdrPart==3)) WrXError(1445,Asc);
          else if ((AdrVal==0) && ((Mask & 4)!=0)) AdrMode=2;
          else
           {
            AdrCnt=1; AdrMode=1;
           }
         *p='(';
         ChkAdr(Mask); return;
        }
      }
     Asc[strlen(Asc)]=')';
    }

    /* indirekt mit/ohne Autoinkrement */

    if ((*Asc=='@') || (*Asc=='*'))
     {
      if (Asc[strlen(Asc)-1]=='+')
       {
        AdrWord=1; Asc[strlen(Asc)-1]='\0';
       }
      else AdrWord=0;
      if (! DecodeReg(Asc+1,&AdrPart)) WrXError(1445,Asc);
      else if ((AdrPart==2) || (AdrPart==3)) WrXError(1445,Asc);
      else if ((AdrWord==0) && ((Mask & 4)==0))
       {
        AdrVal=0; AdrCnt=1; AdrMode=1;
       }
      else AdrMode=2+AdrWord;
      ChkAdr(Mask); return;
     }

    /* bleibt PC-relativ */

    AdrWord=EvalIntExpression(Asc,UInt16,&OK)-EProgCounter()-PCDist;
    if (OK)
     {
      AdrPart=0; AdrMode=1; AdrCnt=1; AdrVal=AdrWord;
     }

   ChkAdr(Mask);
}

/*-------------------------------------------------------------------------*/

	static void PutByte(Byte Value)
{
   if (((CodeLen&1)==1) && (! BigEndian))
    {
     BAsmCode[CodeLen]=BAsmCode[CodeLen-1];
     BAsmCode[CodeLen-1]=Value;
    }
   else
    {
     BAsmCode[CodeLen]=Value;
    }
   CodeLen++;
}

	static bool DecodePseudo(void)
{
#define ONOFF430Count 1
static ONOFFRec ONOFF430s[ONOFF430Count]=
             {{"PADDING", &DoPadding, DoPaddingName}};

   TempResult t;
   Word HVal16;
   Integer z;
   char *p;
   bool OK;

   if (CodeONOFF(ONOFF430s,ONOFF430Count)) return true;

   if (Memo("BYTE"))
    {
     if (ArgCnt==0) WrError(1110);
     else
      {
       z=1; OK=true;
       do
        {
         KillBlanks(ArgStr[z]);
         FirstPassUnknown=false;
         EvalExpression(ArgStr[z],&t);
         switch (t.Typ)
          {
           case TempInt:
            if (FirstPassUnknown) t.Contents.Int&=0xff;
            if (! RangeCheck(t.Contents.Int,Int8)) WrError(1320);
            else if (CodeLen==MaxCodeLen)
             {
              WrError(1920); OK=false;
             }
            else PutByte(t.Contents.Int);
            break;
           case TempFloat:
            WrError(1135); OK=false;
            break;
           case TempString:
            if (strlen(t.Contents.Ascii)+CodeLen>=MaxCodeLen)
             {
              WrError(1920); OK=false;
             }
            else
             {
              TranslateString(t.Contents.Ascii);
              for (p=t.Contents.Ascii; *p!='\0'; PutByte(*(p++)));
             }
            break;
           case TempNone:
            OK=false; break;
          }
         z++;
        }
       while ((z<=ArgCnt) && (OK));
       if (! OK) CodeLen=0;
       else if ((Odd(CodeLen)) && (DoPadding)) PutByte(0);
      }
     return true;
    }

   if (Memo("WORD"))
    {
     if (ArgCnt==0) WrError(1110);
     else
      {
       z=1; OK=true;
       do
        {
         HVal16=EvalIntExpression(ArgStr[z],Int16,&OK);
         if (OK)
	  {
	   WAsmCode[CodeLen >> 1]=HVal16;
	   CodeLen+=2;
          }
         z++;
        }
       while ((z<=ArgCnt) && (OK));
       if (! OK) CodeLen=0;
      }
     return true;
    }

   if (Memo("BSS"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       FirstPassUnknown=false;
       HVal16=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (FirstPassUnknown) WrError(1820);
       else if (OK)
        {
         if ((Odd(HVal16)) && (DoPadding)) HVal16++;
         DontPrint=true; CodeLen=HVal16;
         if (MakeUseList)
          if (AddChunk(SegChunks+ActPC,ProgCounter(),HVal16,ActPC==SegCode)) WrError(90);
         }
      }
     return true;
    }

/*  float exp (8bit bias 128) sign mant (impl. norm.)
   double exp (8bit bias 128) sign mant (impl. norm.) */

   return false;
}

        static void MakeCode_MSP(void)
{
   Integer z,AdrInt;
   bool OK;

   CodeLen=0; DontPrint=false;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Attribut bearbeiten */

   if (*AttrPart=='\0') OpSize=0;
   else if (strlen(AttrPart)>1) WrError(1107);
   else
    switch (toupper(*AttrPart))
     {
      case 'B': OpSize=1; break;
      case 'W': OpSize=0; break;
      default:  WrError(1107); return;
     }

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* zwei Operanden */

   for (z=0; z<TwoOpCount; z++)
    if (Memo(TwoOpOrders[z].Name))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
        PCDist=2; DecodeAdr(ArgStr[1],15,true);
        if (AdrMode!=0xff)
         {
          AdrMode2=AdrMode; AdrPart2=AdrPart; AdrCnt2=AdrCnt; AdrVal2=AdrVal;
          PCDist+=AdrCnt2 << 1; DecodeAdr(ArgStr[2],3,false);
          if (AdrMode!=0xff)
           {
            WAsmCode[0]=TwoOpOrders[z].Code+(AdrPart2 << 8)+(AdrMode << 7)
                       +(OpSize << 6)+(AdrMode2 << 4)+AdrPart;
            memcpy(WAsmCode+1,&AdrVal2,AdrCnt2 << 1);
            memcpy(WAsmCode+1+AdrCnt2,&AdrVal,AdrCnt << 1);
            CodeLen=(1+AdrCnt+AdrCnt2) << 1;
           }
         }
       }
      return;
     }

   /* ein Operand */

   for (z=0; z<OneOpCount; z++)
    if (Memo(OneOpOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if ((OpSize==1) && (! OneOpOrders[z].MayByte)) WrError(1130);
      else
       {
        PCDist=2; DecodeAdr(ArgStr[1],15,false);
        if (AdrMode!=0xff)
         {
          WAsmCode[0]=OneOpOrders[z].Code+(OpSize << 6)+(AdrMode << 4)+AdrPart;
          memcpy(WAsmCode+1,&AdrVal,AdrCnt << 1);
          CodeLen=(1+AdrCnt) << 1;
         }
       }
      return;
     }

   /* kein Operand */

   if (Memo("RETI"))
    {
     if (ArgCnt!=0) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (OpSize!=0) WrError(1130);
     else
      {
       WAsmCode[0]=0x1300; CodeLen=2;
      }
     return;
    }

   /* Spruenge */

   for (z=0; z<JmpCount; z++)
    if (Memo(JmpOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (OpSize!=0) WrError(1130);
      else
       {
        AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+2);
        if (OK)
         if (Odd(AdrInt)) WrError(1375);
         else if ((! SymbolQuestionable) && ((AdrInt<-1024) || (AdrInt>1022))) WrError(1370);
         else
          {
           WAsmCode[0]=JmpOrders[z].Code+((AdrInt >> 1) & 0x3ff);
           CodeLen=2;
          }
       }
      return;
     }

   WrXError(1200,OpPart);
}

        static bool ChkPC_MSP(void)
{
   switch (ActPC)
    {
     case SegCode  : return ProgCounter()<0x10000;
     default: return false;
    }
}

        static bool IsDef_MSP(void)
{
   return false;
}

        static void SwitchFrom_MSP(void)
{
   DeinitFields();
}

        static void SwitchTo_MSP(void)
{
   TurnWords=true; ConstMode=ConstModeIntel; SetIsOccupied=false;

   PCSymbol="$"; HeaderID=0x4a; NOPCode=0x4303; /* = MOV #0,#0 */
   DivideChars=","; HasAttrs=true; AttrChars=".";

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=2; SegInits[SegCode]=0;

   MakeCode=MakeCode_MSP; ChkPC=ChkPC_MSP; IsDef=IsDef_MSP;
   SwitchFrom=SwitchFrom_MSP; InitFields();
}

	void codemsp_init(void)
{
   CPUMSP430=AddCPU("MSP430",SwitchTo_MSP);
}
