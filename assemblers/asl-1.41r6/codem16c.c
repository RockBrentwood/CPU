/* codem16c.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator M16C                                                        */
/*                                                                           */
/* Historie: 17.12.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include "string.h"
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

#define ModNone (-1)
#define ModGen 0
#define MModGen (1 << ModGen)
#define ModAbs20 1
#define MModAbs20 (1 << ModAbs20)
#define ModAReg32 2
#define MModAReg32 (1 << ModAReg32)
#define ModDisp20 3
#define MModDisp20 (1 << ModDisp20)
#define ModReg32 4
#define MModReg32 (1 << ModReg32)
#define ModIReg32 5
#define MModIReg32 (1 << ModIReg32)
#define ModImm 6
#define MModImm (1 << ModImm)
#define ModSPRel 7
#define MModSPRel (1 << ModSPRel)

#define FixedOrderCnt 8
#define StringOrderCnt 4
#define Gen1OrderCnt 5
#define Gen2OrderCnt 6
#define DivOrderCnt 3
#define ConditionCnt 18
#define BCDOrderCnt 4
#define DirOrderCnt 4
#define BitOrderCnt 13

typedef struct
         {
          char *Name;
          Word Code;
	 } FixedOrder;

typedef struct
         {
          char *Name;
          Byte Code1,Code2,Code3;
         } Gen2Order;

typedef struct
         {
          char *Name;
          Byte Code;
         } Condition;


static char *Flags="CDZSBOIU";

static CPUVar CPUM16C,CPUM30600M8;

static String Format;
static Byte FormatCode;
static ShortInt OpSize;
static Byte AdrMode,AdrMode2;
static ShortInt AdrType,AdrType2;
static Byte AdrCnt2;
static Byte AdrVals[3],AdrVals2[3];

static FixedOrder *FixedOrders;
static FixedOrder *StringOrders;
static FixedOrder *Gen1Orders;
static Gen2Order *Gen2Orders;
static Gen2Order *DivOrders;
static Condition *Conditions;
static char **BCDOrders;
static char **DirOrders;
static FixedOrder *BitOrders;

/*------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode)
{
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
}

        static void AddString(char *NName, Word NCode)
{
   if (InstrZ>=StringOrderCnt) exit(255);
   StringOrders[InstrZ].Name=NName;
   StringOrders[InstrZ++].Code=NCode;
}

        static void AddGen1(char *NName, Word NCode)
{
   if (InstrZ>=Gen1OrderCnt) exit(255);
   Gen1Orders[InstrZ].Name=NName;
   Gen1Orders[InstrZ++].Code=NCode;
}

        static void AddGen2(char *NName, Byte NCode1, Byte NCode2, Byte NCode3)
{
   if (InstrZ>=Gen2OrderCnt) exit(255);
   Gen2Orders[InstrZ].Name=NName;
   Gen2Orders[InstrZ].Code1=NCode1;
   Gen2Orders[InstrZ].Code2=NCode2;
   Gen2Orders[InstrZ++].Code3=NCode3;
}

        static void AddDiv(char *NName, Byte NCode1, Byte NCode2, Byte NCode3)
{
   if (InstrZ>=DivOrderCnt) exit(255);
   DivOrders[InstrZ].Name=NName;
   DivOrders[InstrZ].Code1=NCode1;
   DivOrders[InstrZ].Code2=NCode2;
   DivOrders[InstrZ++].Code3=NCode3;
}

        static void AddCondition(char *NName, Word NCode)
{
   if (InstrZ>=ConditionCnt) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
}

	static void AddBCD(char *NName)
{
   if (InstrZ>=BCDOrderCnt) exit(255);
   BCDOrders[InstrZ++]=NName;
}

	static void AddDir(char *NName)
{
   if (InstrZ>=DirOrderCnt) exit(255);
   DirOrders[InstrZ++]=NName;
}

        static void AddBit(char *NName, Word NCode)
{
   if (InstrZ>=BitOrderCnt) exit(255);
   BitOrders[InstrZ].Name=NName;
   BitOrders[InstrZ++].Code=NCode;
}

	static void InitFields(void)
{
   InstrZ=0; FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt);
   AddFixed("BRK"   ,0x0000);
   AddFixed("EXITD" ,0x7df2);
   AddFixed("INTO"  ,0x00f6);
   AddFixed("NOP"   ,0x0004);
   AddFixed("REIT"  ,0x00fb);
   AddFixed("RTS"   ,0x00f3);
   AddFixed("UND"   ,0x00ff);
   AddFixed("WAIT"  ,0x7df3);

   InstrZ=0; StringOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*StringOrderCnt);
   AddString("RMPA" ,0x7cf1);
   AddString("SMOVB",0x7ce9);
   AddString("SMOVF",0x7ce8);
   AddString("SSTR" ,0x7cea);

   InstrZ=0; Gen1Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*Gen1OrderCnt);
   AddGen1("ABS" ,0x76f0);
   AddGen1("ADCF",0x76e0);
   AddGen1("NEG" ,0x7450);
   AddGen1("ROLC",0x76a0);
   AddGen1("RORC",0x76b0);

   InstrZ=0; Gen2Orders=(Gen2Order *) malloc(sizeof(Gen2Order)*Gen2OrderCnt);
   AddGen2("ADC" ,0xb0,0x76,0x60);
   AddGen2("SBB" ,0xb8,0x76,0x70);
   AddGen2("TST" ,0x80,0x76,0x00);
   AddGen2("XOR" ,0x88,0x76,0x10);
   AddGen2("MUL" ,0x78,0x7c,0x50);
   AddGen2("MULU",0x70,0x7c,0x40);

   InstrZ=0; DivOrders=(Gen2Order *) malloc(sizeof(Gen2Order)*DivOrderCnt);
   AddDiv("DIV" ,0xe1,0x76,0xd0);
   AddDiv("DIVU",0xe0,0x76,0xc0);
   AddDiv("DIVX",0xe3,0x76,0x90);

   InstrZ=0; Conditions=(Condition *) malloc(sizeof(Condition)*ConditionCnt);
   AddCondition("GEU", 0); AddCondition("C"  , 0);
   AddCondition("GTU", 1); AddCondition("EQ" , 2);
   AddCondition("Z"  , 2); AddCondition("N"  , 3);
   AddCondition("LTU", 4); AddCondition("NC" , 4);
   AddCondition("LEU", 5); AddCondition("NE" , 6);
   AddCondition("NZ" , 6); AddCondition("PZ" , 7);
   AddCondition("LE" , 8); AddCondition("O"  , 9);
   AddCondition("GE" ,10); AddCondition("GT" ,12);
   AddCondition("NO" ,13); AddCondition("LT" ,14);

   InstrZ=0; BCDOrders=(char **) malloc(sizeof(char *)*BCDOrderCnt);
   AddBCD("DADD"); AddBCD("DSUB"); AddBCD("DADC"); AddBCD("DSBB");

   InstrZ=0; DirOrders=(char **) malloc(sizeof(char *)*DirOrderCnt);
   AddDir("MOVLL"); AddDir("MOVHL"); AddDir("MOVLH"); AddDir("MOVHH");

   InstrZ=0; BitOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*BitOrderCnt);
   AddBit("BAND"  , 4); AddBit("BNAND" , 5);
   AddBit("BNOR"  , 7); AddBit("BNTST" , 3);
   AddBit("BNXOR" ,13); AddBit("BOR"   , 6);
   AddBit("BTSTC" , 0); AddBit("BTSTS" , 1);
   AddBit("BXOR"  ,12); AddBit("BCLR"  , 8);
   AddBit("BNOT"  ,10); AddBit("BSET"  , 9);
   AddBit("BTST"  ,11);
}

	static void DeinitFields(void)
{
   free(FixedOrders);
   free(StringOrders);
   free(Gen1Orders);
   free(Gen2Orders);
   free(DivOrders);
   free(Conditions);
   free(BCDOrders);
   free(DirOrders);
   free(BitOrders);
}

/*------------------------------------------------------------------------*/
/* Adressparser */

        static void SetOpSize(ShortInt NSize)
{
   if (OpSize==-1) OpSize=NSize;
   else if (NSize!=OpSize)
    {
     WrError(1131);
     AdrCnt=0; AdrType=ModNone;
    }
}

	static void ChkAdr(Word Mask)
{
   if ((AdrType!=ModNone) && ((Mask & (1 << AdrType))==0))
    {
     AdrCnt=0; AdrType=ModNone; WrError(1350);
    }
}

        static void DecodeAdr(char *Asc, Word Mask)
{
   LongInt DispAcc;
   String RegPart;
   char *p;
   bool OK;

   AdrCnt=0; AdrType=ModNone;

   /* Datenregister 8 Bit */

   if ((strlen(Asc)==3) && (toupper(*Asc)=='R') && (Asc[1]>='0') && (Asc[1]<='1') &&
      ((toupper(Asc[2])=='L') || (toupper(Asc[2])=='H')))
    {
     AdrType=ModGen;
     AdrMode=((Asc[1]-'0') << 1)+(toupper(Asc[2])=='H');
     SetOpSize(0);
     ChkAdr(Mask); return;
    };

   /* Datenregister 16 Bit */

   if ((strlen(Asc)==2) && (toupper(*Asc)=='R') && (Asc[1]>='0') && (Asc[1]<='3'))
    {
     AdrType=ModGen;
     AdrMode=Asc[1]-'0';
     SetOpSize(1);
     ChkAdr(Mask); return;
    }

   /* Datenregister 32 Bit */

   if (strcasecmp(Asc,"R2R0")==0)
    {
     AdrType=ModReg32; AdrMode=0;
     SetOpSize(2);
     ChkAdr(Mask); return;
    };

   if (strcasecmp(Asc,"R3R1")==0)
    {
     AdrType=ModReg32; AdrMode=1;
     SetOpSize(2);
     ChkAdr(Mask); return;
    }

   /* Adressregister */

   if ((strlen(Asc)==2) && (toupper(*Asc)=='A') && (Asc[1]>='0') && (Asc[1]<='1'))
    {
     AdrType=ModGen;
     AdrMode=Asc[1]-'0'+4;
     ChkAdr(Mask); return;
    }

   /* Adressregister 32 Bit */

   if (strcasecmp(Asc,"A1A0")==0)
    {
     AdrType=ModAReg32;
     SetOpSize(2);
     ChkAdr(Mask); return;
    }

   /* indirekt */

   p=strchr(Asc,'[');
   if ((p!=NULL) && (Asc[strlen(Asc)-1]==']'))
    {
     strmaxcpy(RegPart,p+1,255); RegPart[strlen(RegPart)-1]='\0';
     if ((strcasecmp(RegPart,"A0")==0) || (strcasecmp(RegPart,"A1")==0))
      {
       *p='\0';
       DispAcc=EvalIntExpression(Asc,((Mask & MModDisp20)==0)?Int16:Int20,&OK);
       if (OK)
        if ((DispAcc==0) && ((Mask & MModGen)!=0))
         {
          AdrType=ModGen;
          AdrMode=RegPart[1]-'0'+6;
         }
        else if ((DispAcc>=0) && (DispAcc<=255) && ((Mask & MModGen)!=0))
         {
          AdrType=ModGen;
          AdrVals[0]=DispAcc & 0xff;
          AdrCnt=1;
          AdrMode=RegPart[1]-'0'+8;
         }
        else if ((DispAcc>=-32768) && (DispAcc<=65535) && ((Mask & MModGen)!=0))
         {
          AdrType=ModGen;
          AdrVals[0]=DispAcc & 0xff; AdrVals[1]=(DispAcc >> 8) & 0xff;
          AdrCnt=2;
          AdrMode=RegPart[1]-'0'+12;
         }
        else if (strcasecmp(RegPart,"A0")!=0) WrError(1350);
        else
         {
          AdrType=ModDisp20;
          AdrVals[0]=DispAcc & 0xff;
          AdrVals[1]=(DispAcc >> 8) & 0xff;
          AdrVals[2]=(DispAcc >> 16) & 0x0f;
          AdrCnt=3;
          AdrMode=RegPart[1]-'0';
         }
      }
     else if (strcasecmp(RegPart,"SB")==0)
      {
       *p='\0';
       DispAcc=EvalIntExpression(Asc,Int16,&OK);
       if (OK)
        if ((DispAcc>=0) && (DispAcc<=255))
         {
          AdrType=ModGen;
          AdrVals[0]=DispAcc & 0xff;
          AdrCnt=1;
          AdrMode=10;
         }
        else
         {
          AdrType=ModGen;
          AdrVals[0]=DispAcc & 0xff; AdrVals[1]=(DispAcc >> 8) & 0xff;
          AdrCnt=2;
          AdrMode=14;
         }
      }
     else if (strcasecmp(RegPart,"FB")==0)
      {
       *p='\0';
       DispAcc=EvalIntExpression(Asc,SInt8,&OK);
       if (OK)
        {
         AdrType=ModGen;
         AdrVals[0]=DispAcc & 0xff;
         AdrCnt=1;
         AdrMode=11;
        }
      }
     else if (strcasecmp(RegPart,"SP")==0)
      {
       *p='\0';
       DispAcc=EvalIntExpression(Asc,SInt8,&OK);
       if (OK)
        {
         AdrType=ModSPRel;
         AdrVals[0]=DispAcc & 0xff;
         AdrCnt=1;
        }
      }
     else if (strcasecmp(RegPart,"A1A0")==0)
      {
       *p='\0';
       DispAcc=EvalIntExpression(Asc,SInt8,&OK);
       if (OK)
        if (DispAcc!=0) WrError(1320);
        else AdrType=ModIReg32;
      }
     ChkAdr(Mask); return;
    }

   /* immediate */

   if (*Asc=='#')
    {
     switch (OpSize)
      {
       case -1:
        WrError(1132);
        break;
       case 0:
        AdrVals[0]=EvalIntExpression(Asc+1,Int8,&OK);
        if (OK)
         {
          AdrType=ModImm; AdrCnt=1;
         }
        break;
       case 1:
        DispAcc=EvalIntExpression(Asc+1,Int16,&OK);
        if (OK)
         {
          AdrType=ModImm;
          AdrVals[0]=DispAcc & 0xff;
          AdrVals[1]=(DispAcc >> 8) & 0xff;
          AdrCnt=2;
         }
        break;
      }
     ChkAdr(Mask); return;
    }

   /* dann absolut */

   DispAcc=EvalIntExpression(Asc,((Mask & MModAbs20)==0)?UInt16:UInt20,&OK);
   if ((DispAcc<=0xffff) && ((Mask & MModGen)!=0))
    {
     AdrType=ModGen;
     AdrMode=15;
     AdrVals[0]=DispAcc & 0xff;
     AdrVals[1]=(DispAcc >> 8) & 0xff;
     AdrCnt=2;
    }
   else
    {
     AdrType=ModAbs20;
     AdrVals[0]=DispAcc & 0xff;
     AdrVals[1]=(DispAcc >> 8) & 0xff;
     AdrVals[2]=(DispAcc >> 16) & 0x0f;
     AdrCnt=3;
    }

   ChkAdr(Mask);
}

	static bool DecodeReg(char *Asc, Byte *Erg)
{
   if (strcasecmp(Asc,"FB")==0) *Erg=7;
   else if (strcasecmp(Asc,"SB")==0) *Erg=6;
   else if ((strlen(Asc)==2) && (toupper(*Asc)=='A') &&
            (Asc[1]>='0') && (Asc[1]<='1')) *Erg=Asc[1]-'0'+4;
   else if ((strlen(Asc)==2) && (toupper(*Asc)=='R') &&
            (Asc[1]>='0') && (Asc[1]<='3')) *Erg=Asc[1]-'0';
   else return false;
   return true;
}

	static bool DecodeCReg(char *Asc, Byte *Erg)
{
   if (strcasecmp(Asc,"INTBL")==0) *Erg=1;
   else if (strcasecmp(Asc,"INTBH")==0) *Erg=2;
   else if (strcasecmp(Asc,"FLG")==0) *Erg=3;
   else if (strcasecmp(Asc,"ISP")==0) *Erg=4;
   else if (strcasecmp(Asc,"SP")==0) *Erg=5;
   else if (strcasecmp(Asc,"SB")==0) *Erg=6;
   else if (strcasecmp(Asc,"FB")==0) *Erg=7;
   else
    {
     WrXError(1440,Asc); return false;
    }
   return true;
}

   	static void DecodeDisp(char *Asc, IntType Type1, IntType Type2, LongInt *DispAcc, bool *OK)
{
   if (ArgCnt==2) *DispAcc+=EvalIntExpression(Asc,Type2,OK)*8;
   else *DispAcc=EvalIntExpression(Asc,Type1,OK);
}

	static bool DecodeBitAdr(bool MayShort)
{
   LongInt DispAcc;
   bool OK;
   char *Pos1;
   String Asc,Reg;

   AdrCnt=0;

   /* Nur 1 oder 2 Argumente zugelassen */

   if ((ArgCnt<1) || (ArgCnt>2))
    {
     WrError(1110); return false;
    }

   /* Ist Teil 1 ein Register ? */

   if ((DecodeReg(ArgStr[ArgCnt],&AdrMode)))
    if (AdrMode<6)
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
        AdrVals[0]=EvalIntExpression(ArgStr[1],UInt4,&OK);
        if (OK)
         {
          AdrCnt=1; return true;
         }
       }
      return false;
     }

   /* Bitnummer ? */

   if (ArgCnt==2)
    {
     DispAcc=EvalIntExpression(ArgStr[1],UInt3,&OK);
     if (! OK) return false;
    }
   else DispAcc=0;

   /* Registerangabe ? */

   strmaxcpy(Asc,ArgStr[ArgCnt],255);
   Pos1=QuotPos(Asc,'[');

   /* nein->absolut */

   if (Pos1==NULL)
    {
     DecodeDisp(Asc,UInt16,UInt13,&DispAcc,&OK);
     if (OK)
      {
       AdrMode=15;
       AdrVals[0]=DispAcc & 0xff;
       AdrVals[1]=(DispAcc >> 8) & 0xff;
       AdrCnt=2;
       return true;
      }
     return false;
    }

   /* Register abspalten */

   if (Asc[strlen(Asc)-1]!=']')
    {
     WrError(1350); return false;
    }
   *Pos1='\0'; strmaxcpy(Reg,Pos1+1,255); Reg[strlen(Reg)-1]='\0';

   if ((strlen(Reg)==2) && (toupper(*Reg)=='A') && (Reg[1]>='0') && (Reg[1]<='1'))
    {
     AdrMode=Reg[1]-'0';
     DecodeDisp(Asc,UInt13,UInt16,&DispAcc,&OK);
     if (OK)
      {
       if (DispAcc==0) AdrMode+=6;
       else if ((DispAcc>0) && (DispAcc<256))
        {
         AdrMode+=8; AdrVals[0]=DispAcc & 0xff; AdrCnt=1;
        }
       else
        {
         AdrMode+=12;
         AdrVals[0]=DispAcc & 0xff;
         AdrVals[1]=(DispAcc >> 8) & 0xff;
         AdrCnt=2;
        }
       return true;
      }
     return false;
    }
   else if (strcasecmp(Reg,"SB")==0)
    {
     DecodeDisp(Asc,UInt13,UInt16,&DispAcc,&OK);
     if (OK)
      {
       if ((MayShort) && (DispAcc<0x7ff))
        {
         AdrMode=16+(DispAcc & 7);
	 AdrVals[0]=DispAcc >> 3; AdrCnt=1;
        }
       else if ((DispAcc>0) && (DispAcc<256))
        {
         AdrMode=10; AdrVals[0]=DispAcc & 0xff; AdrCnt=1;
        }
       else
        {
         AdrMode=14;
         AdrVals[0]=DispAcc & 0xff;
         AdrVals[1]=(DispAcc >> 8) & 0xff;
         AdrCnt=2;
        }
       return true;
      }
     return false;
    }
   else if (strcasecmp(Reg,"FB")==0)
    {
     DecodeDisp(Asc,SInt5,SInt8,&DispAcc,&OK);
     if (OK)
      {
       AdrMode=11; AdrVals[0]=DispAcc & 0xff; AdrCnt=1;
       return true;
      }
     return false;
    }
   else
    {
     WrXError(1445,Reg);
     return false;
    }
}

/*------------------------------------------------------------------------*/

	static bool CheckFormat(char *FSet)
{
   char *p;

   if (strcmp(Format," ")==0)
    {
     FormatCode=0; return true;
    }
   else
    {
     p=strchr(FSet,*Format);
     if (p==NULL) WrError(1090);
     else FormatCode=p-FSet+1;
     return (p!=0);
    }
}

        static Integer ImmVal(void)
{
   if (OpSize==0) return (ShortInt)AdrVals[0];
   else return (((Integer)AdrVals[1]) << 8)+AdrVals[0];
}

        static bool IsShort(Byte GenMode, Byte *SMode)
{
   switch (GenMode)
    {
     case 0:  *SMode=4; break;
     case 1:  *SMode=3; break;
     case 10: *SMode=5; break;
     case 11: *SMode=6; break;
     case 15: *SMode=7; break;
     default: return false;
    }
   return true;
}

/*------------------------------------------------------------------------*/

	static bool DecodePseudo(void)
{
   return false;
}

        static void CopyAdr(void)
{
   AdrType2=AdrType;
   AdrMode2=AdrMode;
   AdrCnt2=AdrCnt;
   memcpy(AdrVals2,AdrVals,AdrCnt2);
}

        static void CodeGen(Byte GenCode,Byte Imm1Code,Byte Imm2Code)
{
   if (AdrType==ModImm)
    {
     BAsmCode[0]=Imm1Code+OpSize;
     BAsmCode[1]=Imm2Code+AdrMode2;
     memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
     memcpy(BAsmCode+2+AdrCnt2,AdrVals,AdrCnt);
    }
   else
    {
     BAsmCode[0]=GenCode+OpSize;
     BAsmCode[1]=(AdrMode << 4)+AdrMode2;
     memcpy(BAsmCode+2,AdrVals,AdrCnt);
     memcpy(BAsmCode+2+AdrCnt,AdrVals2,AdrCnt2);
    }
   CodeLen=2+AdrCnt+AdrCnt2;
}

	static bool CodeData(void)
{
   Integer z,Num1;
   bool OK;
   Byte SMode;

   if (Memo("MOV"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("GSQZ"))
      {
       DecodeAdr(ArgStr[2],MModGen+MModSPRel);
       if (AdrType!=ModNone)
        {
         CopyAdr(); DecodeAdr(ArgStr[1],MModGen+MModSPRel+MModImm);
         if (AdrType!=ModNone)
          if (OpSize==-1) WrError(1132);
          else if ((OpSize!=0) && (OpSize!=1)) WrError(1130);
          else
           {
            if (FormatCode==0)
             if ((AdrType2==ModSPRel) || (AdrType==ModSPRel)) FormatCode=1;
             else if ((OpSize==0) && (AdrType==ModImm) && (IsShort(AdrMode2,&SMode)))
              FormatCode=(ImmVal()==0) ? 4 : 2;
             else if ((AdrType==ModImm) && (ImmVal()>=-8) && (ImmVal()<=7)) FormatCode=3;
             else if ((AdrType==ModImm) && ((AdrMode2 & 14)==4)) FormatCode=2;
             else if ((OpSize==0) && (AdrType==ModGen) && (IsShort(AdrMode,&SMode)) && ((AdrMode2 & 14)==4)
                  && ((AdrMode>=2) || (Odd(AdrMode ^ AdrMode2)))) FormatCode=2;
             else if ((OpSize==0) && (AdrType==ModGen) && (AdrMode<=1) && (IsShort(AdrMode2,&SMode))
                 && ((AdrMode2>=2) || (Odd(AdrMode ^ AdrMode2)))) FormatCode=2;
             else if ((OpSize==0) && (AdrMode2<=1) && (AdrType==ModGen) && (IsShort(AdrMode,&SMode))
                 && ((AdrMode>=2) || (Odd(AdrMode ^ AdrMode2)))) FormatCode=2;
             else FormatCode=1;
            switch (FormatCode)
             {
              case 1:
               if (AdrType==ModSPRel)
                {
                 BAsmCode[0]=0x74+OpSize;
                 BAsmCode[1]=0xb0+AdrMode2;
                 memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
                 memcpy(BAsmCode+2+AdrCnt2,AdrVals,AdrCnt);
                 CodeLen=2+AdrCnt+AdrCnt2;
                }
	       else if (AdrType2==ModSPRel)
                {
                 BAsmCode[0]=0x74+OpSize;
                 BAsmCode[1]=0x30+AdrMode;
                 memcpy(BAsmCode+2,AdrVals,AdrCnt);
                 memcpy(BAsmCode+2+AdrCnt,AdrVals2,AdrCnt2);
                 CodeLen=2+AdrCnt2+AdrCnt;
                }
	       else CodeGen(0x72,0x74,0xc0);
               break;
              case 2:
               if (AdrType==ModImm)
                if (AdrType2!=ModGen) WrError(1350);
                else if ((AdrMode2 & 14)==4)
                 {
                  BAsmCode[0]=0xa2+(OpSize << 6)+((AdrMode2 & 1) << 3);
                  memcpy(BAsmCode+1,AdrVals,AdrCnt);
                  CodeLen=1+AdrCnt;
                 }
                else if (IsShort(AdrMode2,&SMode))
                 if (OpSize!=0) WrError(1130);
                 else
	          {
                   BAsmCode[0]=0xc0+SMode;
                   memcpy(BAsmCode+1,AdrVals,AdrCnt);
                   CodeLen=1+AdrCnt;
	          }
	        else WrError(1350);
               else if ((AdrType==ModGen) && (IsShort(AdrMode,&SMode)))
                if (AdrType2!=ModGen) WrError(1350);
                else if ((AdrMode2 & 14)==4)
                 if ((AdrMode<=1) && (! Odd(AdrMode ^ AdrMode2))) WrError(1350);
                 else
                  {
                   if (SMode==3) SMode++;
                   BAsmCode[0]=0x30+((AdrMode2 & 1) << 2)+(SMode & 3);
                   memcpy(BAsmCode+1,AdrVals,AdrCnt);
                   CodeLen=1+AdrCnt;
                  }
                else if ((AdrMode2 & 14)==0)
                 if ((AdrMode<=1) && (! Odd(AdrMode ^ AdrMode2))) WrError(1350);
                 else
                  {
                   if (SMode==3) SMode++;
                   BAsmCode[0]=0x08+((AdrMode2 & 1) << 2)+(SMode & 3);
                   memcpy(BAsmCode+1,AdrVals,AdrCnt);
                   CodeLen=1+AdrCnt;
                  }
                else if (((AdrMode & 14)!=0) || (! IsShort(AdrMode2,&SMode))) WrError(1350);
                else if ((AdrMode2<=1) && (! Odd(AdrMode ^ AdrMode2))) WrError(1350);
                else
                 {
                  if (SMode==3) SMode++;
                  BAsmCode[0]=0x00+((AdrMode & 1) << 2)+(SMode & 3);
                  memcpy(BAsmCode+1,AdrVals,AdrCnt2);
                  CodeLen=1+AdrCnt2;
                 }
               else WrError(1350);
               break;
              case 3:
               if (AdrType!=ModImm) WrError(1350);
               else
                {
                 Num1=ImmVal();
                 if (ChkRange(Num1,-8,7))
                  {
                   BAsmCode[0]=0xd8+OpSize;
                   BAsmCode[1]=(Num1 << 4)+AdrMode2;
                   memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
                   CodeLen=2+AdrCnt2;
                  }
                }
               break;
              case 4:
               if (OpSize!=0) WrError(1130);
	       else if (AdrType!=ModImm) WrError(1350);
               else if (! IsShort(AdrMode2,&SMode)) WrError(1350);
               else
                {
                 Num1=ImmVal();
                 if (ChkRange(Num1,0,0))
                  {
                   BAsmCode[0]=0xb0+SMode;
                   memcpy(BAsmCode+1,AdrVals2,AdrCnt2);
                   CodeLen=1+AdrCnt2;
                  }
                }
               break;
             }
           };
        };
      }
     return true;
    }

   if ((Memo("LDC")) || (Memo("STC")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("G"))
      {
       if (Memo("STC"))
        {
         strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
         z=1;
        }
       else z=0;
       if (strcasecmp(ArgStr[2],"PC")==0)
        if (Memo("LDC")) WrError(1350);
        else
         {
          DecodeAdr(ArgStr[1],MModGen+MModReg32+MModAReg32);
          if (AdrType==ModAReg32) AdrMode=4;
          if ((AdrType==ModGen) && (AdrMode<6)) WrError(1350);
          else
           {
            BAsmCode[0]=0x7c; BAsmCode[1]=0xc0+AdrMode;
            memcpy(BAsmCode+2,AdrVals,AdrCnt);
            CodeLen=2+AdrCnt;
           }
         }
       else if (DecodeCReg(ArgStr[2],&SMode))
        {
         SetOpSize(1);
         DecodeAdr(ArgStr[1],MModGen+(Memo("LDC")?MModImm:0));
         if (AdrType==ModImm)
          {
           BAsmCode[0]=0xeb; BAsmCode[1]=SMode << 4;
           memcpy(BAsmCode+2,AdrVals,AdrCnt);
           CodeLen=2+AdrCnt;
          }
         else if (AdrType==ModGen)
          {
           BAsmCode[0]=0x7a+z; BAsmCode[1]=0x80+(SMode << 4)+AdrMode;
           memcpy(BAsmCode+2,AdrVals,AdrCnt);
           CodeLen=2+AdrCnt;
          }
        }
      }
     return true;
    }

   if ((Memo("LDCTX")) || (Memo("STCTX")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      {
       DecodeAdr(ArgStr[1],MModGen);
       if (AdrType==ModGen)
        if (AdrMode!=15) WrError(1350);
        else
         {
          memcpy(BAsmCode+2,AdrVals,AdrCnt);
          DecodeAdr(ArgStr[2],MModAbs20);
          if (AdrType==ModAbs20)
           {
            memcpy(BAsmCode+4,AdrVals,AdrCnt);
            BAsmCode[0]=0x7c+Memo("STCTX");
            BAsmCode[1]=0xf0;
            CodeLen=7;
           }
         }
      }
     return true;
    }

   if ((Memo("LDE")) || (Memo("STE")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("G"))
      {
       if (Memo("LDE"))
        {
         strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
         z=1;
        }
       else z=0;
       DecodeAdr(ArgStr[1],MModGen);
       if (AdrType!=ModNone)
        if (OpSize==-1) WrError(1132);
        else if (OpSize>1) WrError(1130);
        else
         {
          CopyAdr(); DecodeAdr(ArgStr[2],MModAbs20+MModDisp20+MModIReg32);
          if (AdrType!=ModNone)
           {
            BAsmCode[0]=0x74+OpSize;
            BAsmCode[1]=(z << 7)+AdrMode2;
            switch (AdrType)
             {
              case ModDisp20: BAsmCode[1]+=0x10; break;
              case ModIReg32: BAsmCode[1]+=0x20; break;
             }
            memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
            memcpy(BAsmCode+2+AdrCnt2,AdrVals,AdrCnt);
            CodeLen=2+AdrCnt2+AdrCnt;
           }
         }
      }
     return true;
    }

   if (Memo("MOVA"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("G"))
      {
       DecodeAdr(ArgStr[1],MModGen);
       if (AdrType!=ModNone)
        if (AdrMode<8) WrError(1350);
        else
         {
          CopyAdr(); DecodeAdr(ArgStr[2],MModGen);
          if (AdrType!=ModNone)
           if (AdrMode>5) WrError(1350);
           else
            {
             BAsmCode[0]=0xeb;
             BAsmCode[1]=(AdrMode << 4)+AdrMode2;
             memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
             CodeLen=2+AdrCnt2;
            }
         }
      }
     return true;
    }

   for (z=0; z<DirOrderCnt; z++)
    if (Memo(DirOrders[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else if (OpSize>0) WrError(1130);
      else if (CheckFormat("G"))
       {
        OK=true; Num1=0;
        if (strcasecmp(ArgStr[2],"R0L")==0);
        else if (strcasecmp(ArgStr[1],"R0L")==0) Num1=1;
        else OK=false;
        if (! OK) WrError(1350);
        else
         {
          DecodeAdr(ArgStr[Num1+1],MModGen);
          if (AdrType!=ModNone)
           if (((AdrMode & 14)==4) || ((AdrMode==0) && (Num1==1))) WrError(1350);
           else
            {
             BAsmCode[0]=0x7c; BAsmCode[1]=(Num1 << 7)+(z << 4)+AdrMode;
             memcpy(BAsmCode+2,AdrVals,AdrCnt);
             CodeLen=2+AdrCnt;
            }
         }
       }
      return true;
     }

   if ((Memo("PUSH")) || (Memo("POP")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (CheckFormat("GS"))
      {
       z=Memo("POP");
       DecodeAdr(ArgStr[1],MModGen+((Memo("PUSH"))?MModImm:0));
       if (AdrType!=ModNone)
        if (OpSize==-1) WrError(1132);
        else if (OpSize>1) WrError(1130);
        else
         {
          if (FormatCode==0)
           if ((AdrType!=ModGen)) FormatCode=1;
           else if ((OpSize==0) && (AdrMode<2)) FormatCode=2;
           else if ((OpSize==1) && ((AdrMode & 14)==4)) FormatCode=2;
           else FormatCode=1;
          switch (FormatCode)
           {
            case 1:
	     if (AdrType==ModImm)
              {
               BAsmCode[0]=0x7c+OpSize;
               BAsmCode[1]=0xe2;
              }
             else
              {
               BAsmCode[0]=0x74+OpSize;
               BAsmCode[1]=0x40+(z*0x90)+AdrMode;
              }
             memcpy(BAsmCode+2,AdrVals,AdrCnt);
             CodeLen=2+AdrCnt;
             break;
            case 2:
             if (AdrType!=ModGen) WrError(1350);
             else if ((OpSize==0) && (AdrMode<2))
              {
               BAsmCode[0]=0x82+(AdrMode << 3)+(z << 4);
               CodeLen=1;
              }
             else if ((OpSize==1) && ((AdrMode & 14)==4))
              {
               BAsmCode[0]=0xc2+((AdrMode & 1) << 3)+(z << 4);
               CodeLen=1;
              }
             else WrError(1350);
             break;
           }
         }
      }
     return true;
    }

   if ((Memo("PUSHC")) || (Memo("POPC")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (CheckFormat("G"))
      if (DecodeCReg(ArgStr[1],&SMode))
       {
        BAsmCode[0]=0xeb;
        BAsmCode[1]=0x02+Memo("POPC")+(SMode << 4);
        CodeLen=2;
       }
     return true;
    }

   if ((Memo("PUSHM")) || (Memo("POPM")))
    {
     if (ArgCnt<1) WrError(1110);
     else
      {
       BAsmCode[1]=0; OK=true; z=1;
       while ((OK) && (z<=ArgCnt))
        {
         OK=DecodeReg(ArgStr[z],&SMode);
         if (OK)
          {
           BAsmCode[1]|=(1<<((Memo("POPM"))?SMode:7-SMode));
           z++;
          }
        }
       if (! OK) WrXError(1440,ArgStr[z]);
       else
        {
         BAsmCode[0]=0xec+Memo("POPM");
         CodeLen=2;
        }
      }
     return true;
    }

   if (Memo("PUSHA"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (CheckFormat("G"))
      {
       DecodeAdr(ArgStr[1],MModGen);
       if (AdrType!=ModNone)
        if (AdrMode<8) WrError(1350);
        else
         {
          BAsmCode[0]=0x7d;
          BAsmCode[1]=0x90+AdrMode;
          memcpy(BAsmCode+2,AdrVals,AdrCnt);
          CodeLen=2+AdrCnt;
         }
      }
     return true;
    }

   if (Memo("XCHG"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("G"))
      {
       DecodeAdr(ArgStr[1],MModGen);
       if (AdrType!=ModNone)
        {
         CopyAdr(); DecodeAdr(ArgStr[2],MModGen);
         if (AdrType!=ModNone)
          if (OpSize==-1) WrError(1132);
          else if (OpSize>1) WrError(1130);
          else if (AdrMode<4)
           {
            BAsmCode[0]=0x7a+OpSize;
            BAsmCode[1]=(AdrMode << 4)+AdrMode2;
            memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
            CodeLen=2+AdrCnt2;
           }
          else if (AdrMode2<4)
           {
            BAsmCode[0]=0x7a+OpSize;
            BAsmCode[1]=(AdrMode2 << 4)+AdrMode;
            memcpy(BAsmCode+2,AdrVals,AdrCnt);
            CodeLen=2+AdrCnt;
           }
          else WrError(1350);
        }
      }
     return true;
    }

   if ((Memo("STZ")) || (Memo("STNZ")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("G"))
      {
       if (OpSize==-1) OpSize++;
       DecodeAdr(ArgStr[2],MModGen);
       if (AdrType!=ModNone)
        if (! IsShort(AdrMode,&SMode)) WrError(1350);
        else
         {
          CopyAdr(); DecodeAdr(ArgStr[1],MModImm);
          if (AdrType!=ModNone)
           {
            BAsmCode[0]=0xc8+(Memo("STNZ") << 3)+SMode;
            BAsmCode[1]=AdrVals[0];
            memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
            CodeLen=2+AdrCnt2;
           }
         }
      }
     return true;
    }

   if ((Memo("STZX")))
    {
     if (ArgCnt!=3) WrError(1110);
     else if (CheckFormat("G"))
      {
       if (OpSize==-1) OpSize++;
       DecodeAdr(ArgStr[3],MModGen);
       if (AdrType!=ModNone)
        if (! IsShort(AdrMode,&SMode)) WrError(1350);
        else
         {
          CopyAdr(); DecodeAdr(ArgStr[1],MModImm);
          if (AdrType!=ModNone)
           {
            Num1=AdrVals[0]; DecodeAdr(ArgStr[2],MModImm);
            if (AdrType!=ModNone)
             {
              BAsmCode[0]=0xd8+SMode;
              BAsmCode[1]=Num1;
              memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
              BAsmCode[2+AdrCnt2]=AdrVals[0];
              CodeLen=3+AdrCnt2;
             }
           }
         }
      }
     return true;
    }

   return false;
}

        static void MakeCode_M16C(void)
{
   Integer z,Num1;
   char *p;
   LongInt AdrLong,Diff;
   bool OK,MayShort;
   Byte SMode;
   ShortInt OpSize2;

   OpSize=(-1);

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Formatangabe abspalten */

   switch (AttrSplit)
    {
     case '.':
      p=strchr(AttrPart,':');
      if (p!=NULL)
       {
        if (p<AttrPart+strlen(AttrPart)-1) strmaxcpy(Format,p+1,255);
        *p='\0';
       }
      else strcpy(Format," ");
      break;
     case ':':
      p=strchr(AttrPart,'.');
      if (p==NULL)
       {
        strmaxcpy(Format,AttrPart,255); *AttrPart='\0';
       }
      else
       {
        *p='\0';
        strmaxcpy(Format,(p==AttrPart)?" ":AttrPart,255);
       }
      break;
     default:
      strcpy(Format," ");
    }

   /* Attribut abarbeiten */

   switch (toupper(*AttrPart))
    {
     case '\0': OpSize=(-1); break;
     case 'B': OpSize=0; break;
     case 'W': OpSize=1; break;
     case 'L': OpSize=2; break;
     case 'Q': OpSize=3; break;
     case 'S': OpSize=4; break;
     case 'D': OpSize=5; break;
     case 'X': OpSize=6; break;
     case 'A': OpSize=7; break;
     default:
      WrError(1107); return;
    }
   NLS_UpString(Format);

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(false)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     {
      if (ArgCnt!=0) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else if (strcmp(Format," ")!=0) WrError(1090);
      else if (Hi(FixedOrders[z].Code)==0)
       {
        BAsmCode[0]=Lo(FixedOrders[z].Code); CodeLen=1;
       }
      else
       {
        BAsmCode[0]=Hi(FixedOrders[z].Code);
        BAsmCode[1]=Lo(FixedOrders[z].Code); CodeLen=2;
       };
      return;
     }

   for (z=0; z<StringOrderCnt; z++)
    if (Memo(StringOrders[z].Name))
     {
      if (OpSize==-1) OpSize=1;
      if (ArgCnt!=0) WrError(1110);
      else if ((OpSize!=0) && (OpSize!=1)) WrError(1130);
      else if (strcmp(Format," ")!=0) WrError(1090);
      else if (Hi(StringOrders[z].Code)==0)
       {
        BAsmCode[0]=Lo(StringOrders[z].Code)+OpSize; CodeLen=1;
       }
      else
       {
        BAsmCode[0]=Hi(StringOrders[z].Code)+OpSize;
        BAsmCode[1]=Lo(StringOrders[z].Code); CodeLen=2;
       }
      return;
     }

   /* Datentransfer */

   if (CodeData()) return;

   /* Arithmetik */

   if (Memo("ADD"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[2],"SP")==0)
      {
       if (OpSize==-1) OpSize=1;
       if (CheckFormat("GQ"))
        {
         DecodeAdr(ArgStr[1],MModImm);
         if (AdrType!=ModNone)
          if (OpSize==-1) WrError(1132);
          else if (OpSize>1) WrError(1130);
          else
           {
            AdrLong=ImmVal();
            if (FormatCode==0)
             if ((AdrLong>=-8) && (AdrLong<=7)) FormatCode=2;
             else FormatCode=1;
            switch (FormatCode)
             {
              case 1:
               BAsmCode[0]=0x7c+OpSize;
               BAsmCode[1]=0xeb;
               memcpy(BAsmCode+2,AdrVals,AdrCnt);
               CodeLen=2+AdrCnt;
               break;
              case 2:
               if (ChkRange(AdrLong,-8,7))
                {
                 BAsmCode[0]=0x7d;
                 BAsmCode[1]=0xb0+(AdrLong & 15);
                 CodeLen=2;
                }
               break;
             }
           }
        }
      }
     else if (CheckFormat("GQS"))
      {
       DecodeAdr(ArgStr[2],MModGen);
       if (AdrType!=ModNone)
        {
         CopyAdr();
         DecodeAdr(ArgStr[1],MModImm+MModGen);
         if (AdrType!=ModNone)
          if (OpSize==-1) WrError(1132);
          else if ((OpSize!=0) && (OpSize!=1)) WrError(1130);
          else
           {
            if (FormatCode==0)
             if (AdrType==ModImm)
              if ((ImmVal()>=-8) && (ImmVal()<=7)) FormatCode=2;
              else if ((IsShort(AdrMode2,&SMode)) && (OpSize==0)) FormatCode=3;
	      else FormatCode=1;
             else
              if ((OpSize==0) && (IsShort(AdrMode,&SMode)) && (AdrMode2<=1) &&
	          ((AdrMode>1) || (Odd(AdrMode ^ AdrMode2)))) FormatCode=3;
	      else FormatCode=1;
	    switch (FormatCode)
             {
              case 1:
               CodeGen(0xa0,0x76,0x40);
               break;
              case 2:
               if (AdrType!=ModImm) WrError(1350);
               else
                {
                 Num1=ImmVal();
                 if (ChkRange(Num1,-8,7))
                  {
                   BAsmCode[0]=0xc8+OpSize;
                   BAsmCode[1]=(Num1 << 4)+AdrMode2;
                   memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
                   CodeLen=2+AdrCnt2;
                  }
                }
               break;
              case 3:
               if (OpSize!=0) WrError(1130);
               else if (! IsShort(AdrMode2,&SMode)) WrError(1350);
               else if (AdrType==ModImm)
                {
                 BAsmCode[0]=0x80+SMode; BAsmCode[1]=AdrVals[0];
                 memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
                 CodeLen=2+AdrCnt2;
                }
               else if ((AdrMode2>=2) || (! IsShort(AdrMode,&SMode))) WrError(1350);
               else if ((AdrMode<2) && (! Odd(AdrMode ^ AdrMode2))) WrError(1350);
               else
                {
                 if (SMode==3) SMode++;
                 BAsmCode[0]=0x20+((AdrMode2 & 1) << 3)+(SMode & 3);
                 memcpy(BAsmCode+1,AdrVals,AdrCnt);
                 CodeLen=1+AdrCnt;
                }
               break;
	     }
           }
        }
      }
     return;
    }

   if (Memo("CMP"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("GQS"))
      {
       DecodeAdr(ArgStr[2],MModGen);
       if (AdrType!=ModNone)
        {
         CopyAdr(); DecodeAdr(ArgStr[1],MModImm+MModGen);
         if (AdrType!=ModNone)
          if (OpSize==-1) WrError(1132);
          else if ((OpSize!=0) && (OpSize!=1)) WrError(1130);
          else
           {
            if (FormatCode==0)
             if (AdrType==ModImm)
              if ((ImmVal()>=-8) && (ImmVal()<=7)) FormatCode=2;
              else if ((IsShort(AdrMode2,&SMode)) && (OpSize==0)) FormatCode=3;
	      else FormatCode=1;
             else
              if ((OpSize==0) && (IsShort(AdrMode,&SMode)) && (AdrMode2<=1) &&
	          ((AdrMode>1) || (Odd(AdrMode ^ AdrMode2)))) FormatCode=3;
	      else FormatCode=1;
	    switch (FormatCode)
             {
              case 1:
               CodeGen(0xc0,0x76,0x80);
               break;
              case 2:
               if (AdrType!=ModImm) WrError(1350);
               else
                {
                 Num1=ImmVal();
                 if (ChkRange(Num1,-8,7))
                  {
                   BAsmCode[0]=0xd0+OpSize;
                   BAsmCode[1]=(Num1 << 4)+AdrMode2;
                   memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
                   CodeLen=2+AdrCnt2;
                  }
                }
               break;
              case 3:
               if (OpSize!=0) WrError(1130);
               else if (! IsShort(AdrMode2,&SMode)) WrError(1350);
               else if (AdrType==ModImm)
                {
                 BAsmCode[0]=0xe0+SMode; BAsmCode[1]=AdrVals[0];
                 memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
                 CodeLen=2+AdrCnt2;
                }
               else if ((AdrMode2>=2) || (! IsShort(AdrMode,&SMode))) WrError(1350);
               else if ((AdrMode<2) && (! Odd(AdrMode ^ AdrMode2))) WrError(1350);
               else
                {
                 if (SMode==3) SMode++;
                 BAsmCode[0]=0x38+((AdrMode2 & 1) << 3)+(SMode & 3);
                 memcpy(BAsmCode+1,AdrVals,AdrCnt);
                 CodeLen=1+AdrCnt;
                }
               break;
	     }
           }
        }
      }
     return;
    }

   if (Memo("SUB"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("GQS"))
      {
       DecodeAdr(ArgStr[2],MModGen);
       if (AdrType!=ModNone)
        {
         CopyAdr(); DecodeAdr(ArgStr[1],MModImm+MModGen);
         if (AdrType!=ModNone)
          if (OpSize==-1) WrError(1132);
          else if ((OpSize!=0) && (OpSize!=1)) WrError(1130);
          else
           {
            if (FormatCode==0)
             if (AdrType==ModImm)
              if ((ImmVal()>=-7) && (ImmVal()<=8)) FormatCode=2;
              else if ((IsShort(AdrMode2,&SMode)) && (OpSize==0)) FormatCode=3;
	      else FormatCode=1;
             else
              if ((OpSize==0) && (IsShort(AdrMode,&SMode)) && (AdrMode2<=1) &&
	          ((AdrMode>1) || (Odd(AdrMode ^ AdrMode2)))) FormatCode=3;
	      else FormatCode=1;
	    switch (FormatCode)
             {
              case 1:
               CodeGen(0xa8,0x76,0x50);
               break;
              case 2:
               if (AdrType!=ModImm) WrError(1350);
               else
                {
                 Num1=ImmVal();
                 if (ChkRange(Num1,-7,8))
                  {
                   BAsmCode[0]=0xd0+OpSize;
                   BAsmCode[1]=((-Num1) << 4)+AdrMode2;
                   memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
                   CodeLen=2+AdrCnt2;
                  }
                }
               break;
              case 3:
               if (OpSize!=0) WrError(1130);
               else if (! IsShort(AdrMode2,&SMode)) WrError(1350);
               else if (AdrType==ModImm)
                {
                 BAsmCode[0]=0x88+SMode; BAsmCode[1]=AdrVals[0];
                 memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
                 CodeLen=2+AdrCnt2;
                }
               else if ((AdrMode2>=2) || (! IsShort(AdrMode,&SMode))) WrError(1350);
               else if ((AdrMode<2) && (! Odd(AdrMode ^ AdrMode2))) WrError(1350);
               else
                {
                 if (SMode==3) SMode++;
                 BAsmCode[0]=0x28+((AdrMode2 & 1) << 3)+(SMode & 3);
                 memcpy(BAsmCode+1,AdrVals,AdrCnt);
                 CodeLen=1+AdrCnt;
                }
               break;
	     }
           }
        }
      }
     return;
    }

   for (z=0; z<Gen1OrderCnt; z++)
    if (Memo(Gen1Orders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (CheckFormat("G"))
       {
        DecodeAdr(ArgStr[1],MModGen);
        if (AdrType!=ModNone)
         if (OpSize==-1) WrError(1132);
         else if ((OpSize!=0) && (OpSize!=1)) WrError(1130);
         else
          {
           BAsmCode[0]=Hi(Gen1Orders[z].Code)+OpSize;
           BAsmCode[1]=Lo(Gen1Orders[z].Code)+AdrMode;
           memcpy(BAsmCode+2,AdrVals,AdrCnt);
           CodeLen=2+AdrCnt;
          }
       }
      return;
     }

   for (z=0; z<Gen2OrderCnt; z++)
    if (Memo(Gen2Orders[z].Name))
     {
      if (ArgCnt!=2) WrError(1110);
      else if (CheckFormat("G"))
       {
        DecodeAdr(ArgStr[2],MModGen);
        if (AdrType!=ModNone)
         {
          CopyAdr(); DecodeAdr(ArgStr[1],MModGen+MModImm);
          if (AdrType!=ModNone)
           if (OpSize==-1) WrError(1132);
           else if ((OpSize!=0) && (OpSize!=1)) WrError(1130);
           else if ((*OpPart=='M') && ((AdrMode2==3) || (AdrMode2==5) || (AdrMode2-OpSize==1))) WrError(1350);
           else CodeGen(Gen2Orders[z].Code1,Gen2Orders[z].Code2,Gen2Orders[z].Code3);
         }
       }
      return;
     }

   if ((Memo("INC")) || (Memo("DEC")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (CheckFormat("G"))
      {
       DecodeAdr(ArgStr[1],MModGen);
       if (AdrType!=ModNone)
        if (OpSize==-1) WrError(1132);
        else if ((OpSize==1) && ((AdrMode & 14)==4))
         {
          BAsmCode[0]=0xb2+(Memo("DEC") << 6)+((AdrMode & 1) << 3);
	  CodeLen=1;
         }
        else if (! IsShort(AdrMode,&SMode)) WrError(1350);
        else if (OpSize!=0) WrError(1130);
        else
         {
          BAsmCode[0]=0xa0+(Memo("DEC") << 3)+SMode;
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          CodeLen=1+AdrCnt;
         }
      }
     return;
    }

   for (z=0; z<DivOrderCnt; z++)
    if (Memo(DivOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (CheckFormat("G"))
       {
        DecodeAdr(ArgStr[1],MModImm+MModGen);
        if (AdrType!=ModNone)
         if (OpSize==-1) WrError(1132);
         else if ((OpSize!=0) && (OpSize!=1)) WrError(1130);
         else if (AdrType==ModImm)
          {
           BAsmCode[0]=0x7c+OpSize;
           BAsmCode[1]=DivOrders[z].Code1;
           memcpy(BAsmCode+2,AdrVals,AdrCnt);
           CodeLen=2+AdrCnt;
          }
         else
          {
           BAsmCode[0]=DivOrders[z].Code2+OpSize;
           BAsmCode[1]=DivOrders[z].Code3+AdrMode;
           memcpy(BAsmCode+2,AdrVals,AdrCnt);
           CodeLen=2+AdrCnt;
          }
       }
      return;
     }

   for (z=0; z<BCDOrderCnt; z++)
    if (Memo(BCDOrders[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
        DecodeAdr(ArgStr[2],MModGen);
        if (AdrType!=ModNone)
         if (AdrMode!=0) WrError(1350);
         else
          {
           DecodeAdr(ArgStr[1],MModGen+MModImm);
           if (AdrType!=ModNone)
            if (AdrType==ModImm)
             {
              BAsmCode[0]=0x7c+OpSize;
              BAsmCode[1]=0xec+z;
              memcpy(BAsmCode+2,AdrVals,AdrCnt);
              CodeLen=2+AdrCnt;
             }
            else if (AdrMode!=1) WrError(1350);
            else
             {
              BAsmCode[0]=0x7c+OpSize;
              BAsmCode[1]=0xe4+z;
              CodeLen=2;
             }
          }
       }
      return;
     }

   if (Memo("EXTS"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (CheckFormat("G"))
      {
       DecodeAdr(ArgStr[1],MModGen);
       if (OpSize==-1) OpSize=0;
       if (AdrType!=ModNone)
        if (OpSize==0)
         if ((AdrMode==1) || ((AdrMode>=3) && (AdrMode<=5))) WrError(1350);
         else
          {
           BAsmCode[0]=0x7c;
           BAsmCode[1]=0x60+AdrMode;
           memcpy(BAsmCode+2,AdrVals,AdrCnt);
           CodeLen=2+AdrCnt;
          }
	else if (OpSize==1)
         if (AdrMode!=0) WrError(1350);
         else
          {
           BAsmCode[0]=0x7c; BAsmCode[1]=0xf3;
           CodeLen=2;
          }
	else WrError(1130);
      }
     return;
    }

   if (Memo("NOT"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (CheckFormat("GS"))
      {
       DecodeAdr(ArgStr[1],MModGen);
       if (AdrType!=ModNone)
        if (OpSize==-1) WrError(1132);
        else if (OpSize>1) WrError(1130);
        else
         {
          if (FormatCode==0)
           if ((OpSize==0) && (IsShort(AdrMode,&SMode))) FormatCode=2;
           else FormatCode=1;
          switch (FormatCode)
           {
            case 1:
             BAsmCode[0]=0x74+OpSize;
             BAsmCode[1]=0x70+AdrMode;
             memcpy(BAsmCode+2,AdrVals,AdrCnt);
             CodeLen=2+AdrCnt;
             break;
            case 2:
             if (OpSize!=0) WrError(1130);
             else if (! IsShort(AdrMode,&SMode)) WrError(1350);
             else
              {
               BAsmCode[0]=0xb8+SMode;
               memcpy(BAsmCode+1,AdrVals,AdrCnt);
               CodeLen=1+AdrCnt;
              }
             break;
           }
         }
      }
     return;
    }

   /* Logik*/

   if ((Memo("AND")) || (Memo("OR")))
    {
     z=Memo("OR");
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("GQ"))
      {
       DecodeAdr(ArgStr[2],MModGen);
       if (AdrType!=ModNone)
        {
         CopyAdr(); DecodeAdr(ArgStr[1],MModGen+MModImm);
         if (AdrType!=ModNone)
          if (OpSize==-1) WrError(1132);
          else if (OpSize>1) WrError(1130);
          else
           {
            if (FormatCode==0)
             if (AdrType==ModImm)
              if ((OpSize==0) && (IsShort(AdrMode2,&SMode))) FormatCode=2;
              else FormatCode=1;
             else
              if ((AdrMode2<=1) && (IsShort(AdrMode,&SMode)) && ((AdrMode>1) || Odd(AdrMode ^ AdrMode2))) FormatCode=2;
              else FormatCode=1;
            switch (FormatCode)
             {
              case 1:
               CodeGen(0x90+(z << 3),0x76,0x20+(z << 4));
               break;
              case 2:
               if (OpSize!=0) WrError(1130);
               else if (AdrType==ModImm)
               if (! IsShort(AdrMode2,&SMode)) WrError(1350);
               else
                {
                 BAsmCode[0]=0x90+(z << 3)+SMode;
                 BAsmCode[1]=ImmVal();
                 memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
                 CodeLen=2+AdrCnt2;
                }
              else if ((! IsShort(AdrMode,&SMode)) || (AdrMode2>1)) WrError(1350);
              else if ((AdrMode<=1) && (! Odd(AdrMode ^ AdrMode2))) WrError(1350);
              else
               {
                if (SMode==3) SMode++;
                BAsmCode[0]=0x10+(z << 3)+((AdrMode2 & 1) << 2)+(SMode & 3);
                memcpy(BAsmCode+1,AdrVals,AdrCnt);
                CodeLen=1+AdrCnt;
               }
             }
           }
        }
      }
     return;
    }

   if (Memo("ROT"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("G"))
      {
       DecodeAdr(ArgStr[2],MModGen);
       if (AdrType!=ModNone)
        if (OpSize==-1) WrError(1132);
        else if (OpSize>1) WrError(1130);
        else
         {
          OpSize2=OpSize; OpSize=0; CopyAdr();
          DecodeAdr(ArgStr[1],MModGen+MModImm);
          if (AdrType==ModGen)
           if (AdrMode!=3) WrError(1350);
           else if (AdrMode2+2*OpSize2==3) WrError(1350);
           else
            {
             BAsmCode[0]=0x74+OpSize2;
             BAsmCode[1]=0x60+AdrMode2;
             memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
             CodeLen=2+AdrCnt2;
            }
          else if (AdrType==ModImm)
           {
            Num1=ImmVal();
            if (Num1==0) WrError(1315);
            else if (ChkRange(Num1,-8,8))
             {
              if (Num1>0) Num1--; else Num1=(-9)-Num1;
              BAsmCode[0]=0xe0+OpSize2;
              BAsmCode[1]=(Num1 << 4)+AdrMode2;
              memcpy(BAsmCode+2,AdrVals2,AdrCnt);
              CodeLen=2+AdrCnt2;
             }
           }
         }
      }
     return;
    }

   if ((Memo("SHA")) || (Memo("SHL")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("G"))
      {
       z=Memo("SHA");
       DecodeAdr(ArgStr[2],MModGen+MModReg32);
       if (AdrType!=ModNone)
        if (OpSize==-1) WrError(1132);
        else if ((OpSize>2) || ((OpSize==2) && (AdrType==ModGen))) WrError(1130);
        else
         {
          CopyAdr(); OpSize2=OpSize; OpSize=0;
          DecodeAdr(ArgStr[1],MModImm+MModGen);
          if (AdrType==ModGen)
           if (AdrMode!=3) WrError(1350);
           else if (AdrMode2*2+OpSize2==3) WrError(1350);
           else
            {
             if (OpSize2==2)
              {
               BAsmCode[0]=0xeb;
               BAsmCode[1]=0x01+(AdrMode2 << 4)+(z << 5);
              }
             else
              {
               BAsmCode[0]=0x74+OpSize2;
               BAsmCode[1]=0xe0+(z << 4)+AdrMode2;
              }
             memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
             CodeLen=2+AdrCnt2;
            }
          else if (AdrType==ModImm)
           {
            Num1=ImmVal();
            if (Num1==0) WrError(1315);
            else if (ChkRange(Num1,-8,8))
             {
              if (Num1>0) Num1--; else Num1=(-9)-Num1;
              if (OpSize2==2)
               {
                BAsmCode[0]=0xeb;
                BAsmCode[1]=0x80+(AdrMode2 << 4)+(z << 5)+(Num1 & 15);
               }
              else
               {
                BAsmCode[0]=0xe8+(z << 3)+OpSize2;
                BAsmCode[1]=(Num1 << 4)+AdrMode2;
               }
              memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
              CodeLen=2+AdrCnt2;
             }
           }
         }
      }
     return;
    }

   /* Bitoperationen */

   for (z=0; z<BitOrderCnt; z++)
    if (Memo(BitOrders[z].Name))
     {
      MayShort=(BitOrders[z].Code & 12)==8;
      if (CheckFormat(MayShort?"GS":"G"))
       if (DecodeBitAdr((FormatCode!=1) && (MayShort)))
        if (AdrMode>=16)
         {
          BAsmCode[0]=0x40+((BitOrders[z].Code-8) << 3)+(AdrMode & 7);
          BAsmCode[1]=AdrVals[0];
          CodeLen=2;
         }
        else
         {
          BAsmCode[0]=0x7e;
          BAsmCode[1]=(BitOrders[z].Code << 4)+AdrMode;
          memcpy(BAsmCode+2,AdrVals,AdrCnt);
          CodeLen=2+AdrCnt;
         }
      return;
     }

    if (strncmp(OpPart,"BM",2)==0)
     for (z=0; z<ConditionCnt; z++)
      if (strcmp(OpPart+2,Conditions[z].Name)==0)
       {
        if ((ArgCnt==1) && (strcasecmp(ArgStr[1],"C")==0))
         {
          BAsmCode[0]=0x7d; BAsmCode[1]=0xd0+Conditions[z].Code;
	  CodeLen=2;
         }
        else if (DecodeBitAdr(false))
         {
          BAsmCode[0]=0x7e;
          BAsmCode[1]=0x20+AdrMode;
          memcpy(BAsmCode+2,AdrVals,AdrCnt); z=Conditions[z].Code;
          if ((z>=4) && (z<12)) z^=12;
	  if (z>=8) z+=0xf0;
          BAsmCode[2+AdrCnt]=z;
          CodeLen=3+AdrCnt;
         }
        return;
       }

   if ((Memo("FCLR")) || (Memo("FSET")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (strlen(ArgStr[1])!=1) WrError(1350);
     else
      {
       p=strchr(Flags,toupper(*ArgStr[1]));
       if (p==NULL) WrXError(1440,ArgStr[1]);
       else
        {
         BAsmCode[0]=0xeb;
         BAsmCode[1]=0x04+Memo("FCLR")+((p-Flags) << 4);
         CodeLen=2;
        }
      }
     return;
    }

   /* Spruenge */

   if (Memo("JMP"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrLong=EvalIntExpression(ArgStr[1],UInt20,&OK);
       Diff=AdrLong-EProgCounter();
       if (OpSize==-1)
        {
         if ((Diff>=2) && (Diff<=9)) OpSize=4;
         else if ((Diff>=-127) && (Diff<=128)) OpSize=0;
         else if ((Diff>=-32767) && (Diff<=32768)) OpSize=1;
         else OpSize=7;
        }
       switch (OpSize)
        {
         case 4:
          if (((Diff<2) || (Diff>9)) && (! SymbolQuestionable)) WrError(1370);
          else
           {
            BAsmCode[0]=0x60+((Diff-2) & 7); CodeLen=1;
           }
          break;
         case 0:
          if (((Diff<-127) || (Diff>128)) && (! SymbolQuestionable)) WrError(1370);
          else
           {
            BAsmCode[0]=0xfe;
	    BAsmCode[1]=(Diff-1) & 0xff;
	    CodeLen=2;
           }
          break;
         case 1:
          if (((Diff<-32767) || (Diff>32768)) && (! SymbolQuestionable)) WrError(1370);
          else
           {
            BAsmCode[0]=0xf4; Diff--;
	    BAsmCode[1]=Diff & 0xff;
            BAsmCode[2]=(Diff >> 8) & 0xff;
	    CodeLen=3;
           }
          break;
         case 7:
          BAsmCode[0]=0xfc;
          BAsmCode[1]=AdrLong & 0xff;
          BAsmCode[2]=(AdrLong >> 8) & 0xff;
          BAsmCode[3]=(AdrLong >> 16) & 0xff;
          CodeLen=4;
          break;
         default:
          WrError(1130);
        }
      }
     return;
    }

   if (Memo("JSR"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrLong=EvalIntExpression(ArgStr[1],UInt20,&OK);
       Diff=AdrLong-EProgCounter();
       if (OpSize==-1)
        {
         if ((Diff>=-32767) && (Diff<=32768)) OpSize=1;
         else OpSize=7;
        }
       switch (OpSize)
        {
         case 1:
          if (((Diff<-32767) || (Diff>32768)) && (! SymbolQuestionable)) WrError(1370);
          else
           {
            BAsmCode[0]=0xf5; Diff--;
	    BAsmCode[1]=Diff & 0xff;
            BAsmCode[2]=(Diff >> 8) & 0xff;
	    CodeLen=3;
           }
          break;
         case 7:
          BAsmCode[0]=0xfd;
          BAsmCode[1]=AdrLong & 0xff;
          BAsmCode[2]=(AdrLong >> 8) & 0xff;
          BAsmCode[3]=(AdrLong >> 16) & 0xff;
          CodeLen=4;
          break;
         default:
          WrError(1130);
        }
      }
     return;
    }

   if ((Memo("JMPI")) || (Memo("JSRI")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (CheckFormat("G"))
      {
       if (OpSize==7) OpSize=2;
       DecodeAdr(ArgStr[1],MModGen+MModDisp20+MModReg32+MModAReg32);
       if ((AdrType==ModGen) && ((AdrMode & 14)==12))
        AdrVals[AdrCnt++]=0;
       if ((AdrType==ModGen) && ((AdrMode & 14)==4))
        if (OpSize==-1) OpSize=1;
        else if (OpSize!=1)
	 {
	  AdrType=ModNone; WrError(1131);
	 }
       if (AdrType==ModAReg32) AdrMode=4;
       if (AdrType!=ModNone)
        if (OpSize==-1) WrError(1132);
        else if ((OpSize!=1) && (OpSize!=2)) WrError(1130);
        else
         {
          BAsmCode[0]=0x7d;
          BAsmCode[1]=(Memo("JSRI") << 4)+((OpSize==1) << 5)+AdrMode;
          memcpy(BAsmCode+2,AdrVals,AdrCnt);
          CodeLen=2+AdrCnt;
         }
      }
     return;
    }

   if ((Memo("JMPS")) || (Memo("JSRS")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       OpSize=0;
       FirstPassUnknown=false;
       DecodeAdr(ArgStr[1],MModImm);
       if ((FirstPassUnknown) && (AdrVals[0]<18)) AdrVals[0]=18;
       if (AdrType!=ModNone)
        if (AdrVals[0]<18) WrError(1315);
        else
         {
          BAsmCode[0]=0xee + Memo("JSRS"); /* ANSI :-O */
          BAsmCode[1]=AdrVals[0];
          CodeLen=2;
         }
      }
     return;
    }

   if (*OpPart=='J')
    for (z=0; z<ConditionCnt; z++)
     if (strcmp(Conditions[z].Name,OpPart+1)==0)
      {
       Num1=1+(Conditions[z].Code>=8);
       if (ArgCnt!=1) WrError(1110);
       else if (*AttrPart!='\0') WrError(1100);
       else if (strcmp(Format," ")!=0) WrError(1090);
       else
        {
         AdrLong=EvalIntExpression(ArgStr[1],UInt20,&OK)-(EProgCounter()+Num1);
         if (OK)
          if ((! SymbolQuestionable) && ((AdrLong>127) || (AdrLong<-128))) WrError(1370);
          else if (Conditions[z].Code>=8)
           {
            BAsmCode[0]=0x7d; BAsmCode[1]=0xc0+Conditions[z].Code;
            BAsmCode[2]=AdrLong & 0xff;
            CodeLen=3;
           }
          else
           {
            BAsmCode[0]=0x68+Conditions[z].Code; BAsmCode[1]=AdrLong & 0xff;
            CodeLen=2;
           }
        }
       return;
      }

   if ((Memo("ADJNZ")) || (Memo("SBJNZ")))
    {
     if (ArgCnt!=3) WrError(1110);
     else if (CheckFormat("G"))
      {
       DecodeAdr(ArgStr[2],MModGen);
       if (AdrType!=ModNone)
        if (OpSize==-1) WrError(1132);
        else if (OpSize>1) WrError(1130);
        else
         {
          CopyAdr(); OpSize2=OpSize; OpSize=0;
          FirstPassUnknown=false;
	  DecodeAdr(ArgStr[1],MModImm); Num1=ImmVal();
          if (FirstPassUnknown) Num1=0;
          if (Memo("SBJNZ")) Num1=(-Num1);
          if (ChkRange(Num1,-8,7))
           {
            AdrLong=EvalIntExpression(ArgStr[3],UInt20,&OK)-(EProgCounter()+2);
            if (OK)
             if (((AdrLong<-128) || (AdrLong>127)) && (! SymbolQuestionable)) WrError(1370);
             else
              {
               BAsmCode[0]=0xf8+OpSize2;
               BAsmCode[1]=(Num1 << 4)+AdrMode2;
               memcpy(BAsmCode+2,AdrVals2,AdrCnt2);
               BAsmCode[2+AdrCnt2]=AdrLong & 0xff;
               CodeLen=3+AdrCnt2;
              }
           }
         }
      }
     return;
    }

   if (Memo("INT"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      {
       BAsmCode[1]=0xc0+EvalIntExpression(ArgStr[1]+1,UInt6,&OK);
       if (OK)
        {
         BAsmCode[0]=0xeb; CodeLen=2;
        }
      }
     return;
    }

   /* Miszellaneen */

   if (Memo("ENTER"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      {
       BAsmCode[2]=EvalIntExpression(ArgStr[1]+1,UInt8,&OK);
       if (OK)
        {
         BAsmCode[0]=0x7c; BAsmCode[1]=0xf2; CodeLen=3;
        }
      }
     return;
    }

   if (Memo("LDINTB"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      {
       AdrLong=EvalIntExpression(ArgStr[1]+1,UInt20,&OK);
       if (OK)
        {
         BAsmCode[0]=0xeb; BAsmCode[1]=0x20;
         BAsmCode[2]=(AdrLong >> 16) & 0xff; BAsmCode[3]=0;
         BAsmCode[4]=0xeb; BAsmCode[5]=0x10;
         BAsmCode[6]=(AdrLong >> 8) & 0xff; BAsmCode[7]=AdrLong & 0xff;
         CodeLen=8;
        }
      }
     return;
    }

   if (Memo("LDIPL"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      {
       BAsmCode[1]=0xa0+EvalIntExpression(ArgStr[1]+1,UInt3,&OK);
       if (OK)
        {
         BAsmCode[0]=0x7d; CodeLen=2;
        }
      }
     return;
    }

   WrXError(1200,OpPart);
}

        static bool ChkPC_M16C(void)
{
   switch (ActPC)
    {
     case SegCode: return (EProgCounter()<=0xffff);
     default: return false;
    }
}

        static bool IsDef_M16C(void)
{
   return false;
}

        static void SwitchFrom_M16C(void)
{
   DeinitFields();
}

        static void SwitchTo_M16C(void)
{
   TurnWords=true; ConstMode=ConstModeIntel; SetIsOccupied=false;

   PCSymbol="$"; HeaderID=0x14; NOPCode=0x04;
   DivideChars=","; HasAttrs=true; AttrChars=".:";

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_M16C; ChkPC=ChkPC_M16C; IsDef=IsDef_M16C;
   SwitchFrom=SwitchFrom_M16C; InitFields();
}

	void codem16c_init(void)
{
   CPUM16C=AddCPU("M16C",SwitchTo_M16C);
   CPUM30600M8=AddCPU("M30600M8",SwitchTo_M16C);
}


