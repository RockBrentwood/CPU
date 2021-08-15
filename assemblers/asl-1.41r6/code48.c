/* code48.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegeneratormodul MCS-48-Familie                                         */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "stringutil.h"
#include "stringlists.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct
         {
          char *Name;
          Byte Code;
          Byte May2X;
          Byte UPIFlag;
         } CondOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } AccOrder;

typedef struct
         {
          char *Name;
          Byte Code;
          bool Is22;
          bool IsNUPI;
         } SelOrder;

#define ModImm 0
#define ModReg 1
#define ModInd 2
#define ModAcc 3
#define ModNone (-1)

#define ClrCplCnt 4
#define CondOrderCnt 22
#define AccOrderCnt 6
#define SelOrderCnt 6

#define D_CPU8021  0
#define D_CPU8022  1
#define D_CPU8039  2
#define D_CPU8048  3
#define D_CPU80C39 4
#define D_CPU80C48 5
#define D_CPU8041  6
#define D_CPU8042  7

static ShortInt AdrMode;
static Byte AdrVal;
static CPUVar CPU8021,CPU8022,CPU8039,CPU8048,CPU80C39,CPU80C48,CPU8041,CPU8042;

static char **ClrCplVals;
static Byte *ClrCplCodes;
static CondOrder *CondOrders;
static AccOrder *AccOrders;
static SelOrder *SelOrders;

/****************************************************************************/

	static void AddAcc(char *Name, Byte Code)
{
   if (InstrZ==AccOrderCnt) exit(255);
   AccOrders[InstrZ].Name=Name;
   AccOrders[InstrZ++].Code=Code;
}

        static void AddCond(char *Name, Byte Code, Byte May2X, Byte UPIFlag)
{
   if (InstrZ==CondOrderCnt) exit(255);
   CondOrders[InstrZ].Name=Name;
   CondOrders[InstrZ].Code=Code;
   CondOrders[InstrZ].May2X=May2X;
   CondOrders[InstrZ++].UPIFlag=UPIFlag;
}

        static void AddSel(char *Name, Byte Code, Byte Is22, Byte IsNUPI)
{
   if (InstrZ==SelOrderCnt) exit(255);
   SelOrders[InstrZ].Name=Name;
   SelOrders[InstrZ].Code=Code;
   SelOrders[InstrZ].Is22=Is22;
   SelOrders[InstrZ++].IsNUPI=IsNUPI;
}

	static void InitFields(void)
{
   ClrCplVals=(char **) malloc(sizeof(char *)*ClrCplCnt);
   ClrCplCodes=(Byte *) malloc(sizeof(Byte)*ClrCplCnt);
   ClrCplVals[0]="A"; ClrCplVals[1]="C"; ClrCplVals[2]="F0"; ClrCplVals[3]="F1";
   ClrCplCodes[0]=0x27; ClrCplCodes[1]=0x97; ClrCplCodes[2]=0x85; ClrCplCodes[3]=0xa5;

   CondOrders=(CondOrder *) malloc(sizeof(CondOrder)*CondOrderCnt); InstrZ=0;
   AddCond("JTF"  ,0x16, 2, 3); AddCond("JNI"  ,0x86, 0, 2);
   AddCond("JC"   ,0xf6, 2, 3); AddCond("JNC"  ,0xe6, 2, 3);
   AddCond("JZ"   ,0xc6, 2, 3); AddCond("JNZ"  ,0x96, 2, 3);
   AddCond("JT0"  ,0x36, 1, 3); AddCond("JNT0" ,0x26, 1, 3);
   AddCond("JT1"  ,0x56, 2, 3); AddCond("JNT1" ,0x46, 2, 3);
   AddCond("JF0"  ,0xb6, 0, 3); AddCond("JF1"  ,0x76, 0, 3);
   AddCond("JNIBF",0xd6, 2, 1); AddCond("JOBF" ,0x86, 2, 1);
   AddCond("JB0"  ,0x12, 0, 3); AddCond("JB1"  ,0x32, 0, 3);
   AddCond("JB2"  ,0x52, 0, 3); AddCond("JB3"  ,0x72, 0, 3);
   AddCond("JB4"  ,0x92, 0, 3); AddCond("JB5"  ,0xb2, 0, 3);
   AddCond("JB6"  ,0xd2, 0, 3); AddCond("JB7"  ,0xf2, 0, 3);

   AccOrders=(AccOrder *) malloc(sizeof(AccOrder)*AccOrderCnt); InstrZ=0;
   AddAcc("DA"  ,0x57);
   AddAcc("RL"  ,0xe7);
   AddAcc("RLC" ,0xf7);
   AddAcc("RR"  ,0x77);
   AddAcc("RRC" ,0x67);
   AddAcc("SWAP",0x47);

   SelOrders=(SelOrder *) malloc(sizeof(SelOrder)*SelOrderCnt); InstrZ=0;
   AddSel("MB0" ,0xe5, false, true );
   AddSel("MB1" ,0xf5, false, true );
   AddSel("RB0" ,0xc5, false, false);
   AddSel("RB1" ,0xd5, false, false);
   AddSel("AN0" ,0x95, true , false);
   AddSel("AN1" ,0x85, true , false);
}

	static void DeinitFields(void)
{
   free(ClrCplVals);
   free(ClrCplCodes);
   free(CondOrders);
   free(AccOrders);
   free(SelOrders);
}

/****************************************************************************/

	static void DecodeAdr(char *Asc_O)
{
   bool OK;
   String Asc;

   strmaxcpy(Asc,Asc_O,255);
   AdrMode=ModNone;

   if (*Asc=='\0') return;

   if (strcasecmp(Asc,"A")==0) AdrMode=ModAcc;

   else if (*Asc=='#')
    {
     AdrVal=EvalIntExpression(Asc+1,Int8,&OK);
     if (OK)
      {
       AdrMode=ModImm; BAsmCode[1]=AdrVal;
      }
    }

   else if ((strlen(Asc)==2) && (toupper(*Asc)=='R'))
    {
     if ((Asc[1]>='0') && (Asc[1]<='7'))
      {
       AdrMode=ModReg; AdrVal=Asc[1]-'0';
      }
    }

   else if ((strlen(Asc)==3) && (*Asc=='@') && (toupper(Asc[1])=='R'))
    {
     if ((Asc[2]>='0') && (Asc[2]<='1'))
      {
       AdrMode=ModInd; AdrVal=Asc[2]-'0';
      }
    }
}

	static void ChkN802X(void)
{
   if (CodeLen==0) return;
   if ((MomCPU==CPU8021) || (MomCPU==CPU8022))
    {
     WrError(1500); CodeLen=0;
    }
}

	static void Chk802X(void)
{
   if (CodeLen==0) return;
   if ((MomCPU!=CPU8021) && (MomCPU!=CPU8022))
    {
     WrError(1500); CodeLen=0;
    }
}

	static void ChkNUPI(void)
{
   if (CodeLen==0) return;
   if ((MomCPU==CPU8041) || (MomCPU==CPU8042))
    {
     WrError(1500); CodeLen=0;
    }
}

	static void ChkUPI(void)
{
   if (CodeLen==0) return;
   if ((MomCPU!=CPU8041) && (MomCPU!=CPU8042))
    {
     WrError(1500); CodeLen=0;
    }
}

	static void ChkExt(void)
{
   if (CodeLen==0) return;
   if ((MomCPU==CPU8039) || (MomCPU==CPU80C39))
    {
     WrError(1500); CodeLen=0;
    }
}

	static bool DecodePseudo(void)
{
   return false;
}

	void MakeCode_48(void)
{
   bool OK;
   Word AdrWord;
   Integer z;

   CodeLen=0; DontPrint=false;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(false)) return;

   if ((Memo("ADD")) || (Memo("ADDC")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1135);
     else
      {
       DecodeAdr(ArgStr[2]);
       if ((AdrMode==ModNone) || (AdrMode==ModAcc)) WrError(1350);
       else
	{
         switch (AdrMode)
          {
           case ModImm:
            CodeLen=2; BAsmCode[0]=0x03;
            break;
	   case ModReg:
	    CodeLen=1; BAsmCode[0]=0x68+AdrVal;
            break;
           case ModInd:
            CodeLen=1; BAsmCode[0]=0x60+AdrVal;
            break;
          }
         if (strlen(OpPart)==4) BAsmCode[0]+=0x10;
        }
      }
     return;
    }

   if ((Memo("ANL")) || (Memo("ORL")) || (Memo("XRL")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")==0)
      {
       DecodeAdr(ArgStr[2]);
       if ((AdrMode==-1) || (AdrMode==ModAcc)) WrError(1350);
       else
	{
	 switch (AdrMode)
          {
	   case ModImm:
            CodeLen=2; BAsmCode[0]=0x43;
            break;
	   case ModReg:
            CodeLen=1; BAsmCode[0]=0x48+AdrVal;
            break;
	   case ModInd:
	    CodeLen=1; BAsmCode[0]=0x40+AdrVal;
	    break;
	  }
	 if (Memo("ANL")) BAsmCode[0]+=0x10;
	 else if (Memo("XRL")) BAsmCode[0]+=0x90;
	}
      }
     else if ((strcasecmp(ArgStr[1],"BUS")==0) || (strcmp(ArgStr[1],"P1")==0) || (strcmp(ArgStr[1],"P2")==0))
      {
       if (Memo("XRL")) WrError(1350);
       else
	{
	 DecodeAdr(ArgStr[2]);
	 if (AdrMode!=ModImm) WrError(1350);
	 else
	  {
	   CodeLen=2; BAsmCode[0]=0x88;
	   if (toupper(*ArgStr[1])=='P') BAsmCode[0]+=ArgStr[1][1]-'0';
	   if (Memo("ANL")) BAsmCode[0]+=0x10;
	   if (strcasecmp(ArgStr[1],"BUS")==0)
	    {
	     ChkExt(); ChkNUPI();
	    }
	   ChkN802X();
	  }
	}
      }
     else WrError(1350);
     return;
    }

   if ((Memo("CALL")) || (Memo("JMP")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if ((EProgCounter()&0x7fe)==0x7fe) WrError(1900);
     else
      {
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
        if (AdrWord>0xfff) WrError(1320);
        else
         {
          if ((EProgCounter()&0x800)!=(AdrWord&0x800))
	   {
            BAsmCode[0]=0xe5+((AdrWord&0x800)>>7); CodeLen=1;
           }
          BAsmCode[CodeLen+1]=AdrWord&0xff;
          BAsmCode[CodeLen]=0x04+((AdrWord&0x700)>>3);
          if (Memo("CALL")) BAsmCode[CodeLen]+=0x10;
	  CodeLen+=2; ChkSpace(SegCode);
         }
      }
     return;
    }

   if ((Memo("CLR")) || (Memo("CPL")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       z=0; OK=false; NLS_UpString(ArgStr[1]);
       do
        {
         if (strcmp(ClrCplVals[z],ArgStr[1])==0)
 	  {
           CodeLen=1; BAsmCode[0]=ClrCplCodes[z]; OK=true;
           if (*ArgStr[1]=='F') ChkN802X();
          }
         z++;
        }
       while ((z<ClrCplCnt) && (CodeLen!=1));
       if (! OK) WrError(1135);
       else if (Memo("CPL")) BAsmCode[0]+=0x10;
      }
     return;
    }

   for (z=0; z<AccOrderCnt; z++)
    if (Memo(AccOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
      else
       {
	CodeLen=1; BAsmCode[0]=AccOrders[z].Code;
       }
      return;
     }

   if (Memo("DEC"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1]);
       switch (AdrMode)
        {
         case ModAcc:
          CodeLen=1; BAsmCode[0]=0x07;
          break;
         case ModReg:
          CodeLen=1; BAsmCode[0]=0xc8+AdrVal;
	  ChkN802X();
          break;
         default:
          WrError(1350);
        }
      }
     return;
    }

   if ((Memo("DIS")) || (Memo("EN")))
    {
     NLS_UpString(ArgStr[1]);
     if (ArgCnt!=1) WrError(1110);
     else
      {
       NLS_UpString(ArgStr[1]);
       if (strcmp(ArgStr[1],"I")==0)
        {
         CodeLen=1; BAsmCode[0]=0x05;
        }
       else if (strcmp(ArgStr[1],"TCNTI")==0)
        {
         CodeLen=1; BAsmCode[0]=0x25;
        }
       else if ((Memo("EN")) && (strcmp(ArgStr[1],"DMA")==0))
        {
         BAsmCode[0]=0xe5; CodeLen=1; ChkUPI();
        }
       else if ((Memo("EN")) && (strcmp(ArgStr[1],"FLAGS")==0))
        {
         BAsmCode[0]=0xf5; CodeLen=1; ChkUPI();
        }
       else WrError(1350);
       if (CodeLen!=0)
        {
         if (Memo("DIS")) BAsmCode[0]+=0x10;
         if (MomCPU==CPU8021)
          {
           WrError(1500); CodeLen=0;
          }
        }
      }
     return;
    }

   if (Memo("DJNZ"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1]);
       if (AdrMode!=ModReg) WrError(1350);
       else
        {
         AdrWord=EvalIntExpression(ArgStr[2],Int16,&OK);
         if (OK)
          {
           if (((EProgCounter()+1)&0xff00)!=(AdrWord&0xff00)) WrError(1910);
           else
	    {
             CodeLen=2; BAsmCode[0]=0xe8+AdrVal; BAsmCode[1]=AdrWord&0xff;
            }
          }
        }
      }
     return;
    }

   if (Memo("ENT0"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"CLK")!=0) WrError(1135);
     else
      {
       CodeLen=1; BAsmCode[0]=0x75;
       ChkN802X(); ChkNUPI();
      }
     return;
    }

   if (Memo("INC"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1]);
       switch (AdrMode)
        {
         case ModAcc:
          CodeLen=1; BAsmCode[0]=0x17;
	  break;
         case ModReg:
          CodeLen=1; BAsmCode[0]=0x18+AdrVal;
          break;
         case ModInd:
          CodeLen=1; BAsmCode[0]=0x10+AdrVal;
          break;
         default:
          WrError(1350);
        }
      }
     return;
    }

   if (Memo("IN"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
     else if (strcasecmp(ArgStr[2],"DBB")==0)
      {
       CodeLen=1; BAsmCode[0]=0x22; ChkUPI();
      }
     else if ((strlen(ArgStr[2])!=2) || (toupper(*ArgStr[2])!='P')) WrError(1350);
     else switch (ArgStr[2][1])
      {
       case '0':
       case '1':
       case '2':
        CodeLen=1; BAsmCode[0]=0x08+ArgStr[2][1]-'0';
        if (ArgStr[2][1]=='0') Chk802X();
        break;
       default:
        WrError(1350);
      }
     return;
    }

   if (Memo("INS"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
     else if (strcasecmp(ArgStr[2],"BUS")!=0) WrError(1350);
     else
      {
       CodeLen=1; BAsmCode[0]=0x08; ChkExt(); ChkNUPI();
      }
     return;
    }

   if (Memo("JMPP"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"@A")!=0) WrError(1350);
     else
      {
       CodeLen=1; BAsmCode[0]=0xb3;
      }
     return;
    }

   for (z=0; z<CondOrderCnt; z++)
    if (Memo(CondOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else
       {
        AdrWord=EvalIntExpression(ArgStr[1],UInt12,&OK);
        if (! OK);
        else if (((EProgCounter()+1)&0xff00)!=(AdrWord&0xff00)) WrError(1910);
        else
         {
          CodeLen=2; BAsmCode[0]=CondOrders[z].Code; BAsmCode[1]=AdrWord&0xff;
          ChkSpace(SegCode);
          if (CondOrders[z].May2X==0) ChkN802X();
          else if ((CondOrders[z].May2X==1) && (MomCPU==CPU8021))
           {
            WrError(1500); CodeLen=0;
           }
          if (CondOrders[z].UPIFlag==1) ChkUPI();
          else if (CondOrders[z].UPIFlag==2) ChkNUPI();
         }
       }
      return;
     }

   if (Memo("JB"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       AdrVal=EvalIntExpression(ArgStr[1],UInt3,&OK);
       if (OK)
	{
	 AdrWord=EvalIntExpression(ArgStr[2],UInt12,&OK);
	 if (! OK);
	 else if (((EProgCounter()+1)&0xff00)!=(AdrWord&0xff00)) WrError(1910);
	 else
	  {
	   CodeLen=2; BAsmCode[0]=0x12+(AdrVal<<5);
	   BAsmCode[1]=AdrWord&0xff;
	   ChkN802X();
	  }
	}
      }
     return;
    }

   if (Memo("MOV"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")==0)
      {
       if (strcasecmp(ArgStr[2],"T")==0)
	{
	 CodeLen=1; BAsmCode[0]=0x42;
	}
       else if (strcasecmp(ArgStr[2],"PSW")==0)
	{
	 CodeLen=1; BAsmCode[0]=0xc7; ChkN802X();
	}
       else
	{
	  DecodeAdr(ArgStr[2]);
	  switch (AdrMode)
           {
	    case ModReg:
	     CodeLen=1; BAsmCode[0]=0xf8+AdrVal;
	     break;
	    case ModInd:
	     CodeLen=1; BAsmCode[0]=0xf0+AdrVal;
	     break;
	    case ModImm:
	     CodeLen=2; BAsmCode[0]=0x23;
	     break;
	    default:
             WrError(1350);
	   }
	}
      }
     else if (strcasecmp(ArgStr[2],"A")==0)
      {
       if (strcasecmp(ArgStr[1],"STS")==0)
	{
	 CodeLen=1; BAsmCode[0]=0x90; ChkUPI();
	}
       else if (strcasecmp(ArgStr[1],"T")==0)
	{
	 CodeLen=1; BAsmCode[0]=0x62;
	}
       else if (strcasecmp(ArgStr[1],"PSW")==0)
	{
	 CodeLen=1; BAsmCode[0]=0xd7; ChkN802X();
	}
       else
	{
	 DecodeAdr(ArgStr[1]);
	  switch (AdrMode)
	   {
            case ModReg:
	     CodeLen=1; BAsmCode[0]=0xa8+AdrVal;
	     break;
	    case ModInd:
	     CodeLen=1; BAsmCode[0]=0xa0+AdrVal;
	     break;
	    default:
             WrError(1350);
	   }
	}
      }
     else if (*ArgStr[2]=='#')
      {
       AdrWord=EvalIntExpression(ArgStr[2]+1,Int8,&OK);
       if (OK)
        {
         DecodeAdr(ArgStr[1]);
         switch (AdrMode)
          {
           case ModReg:
            CodeLen=2; BAsmCode[0]=0xb8+AdrVal; BAsmCode[1]=AdrWord;
            break;
	   case ModInd:
            CodeLen=2; BAsmCode[0]=0xb0+AdrVal; BAsmCode[1]=AdrWord;
            break;
	   default:
            WrError(1350);
	  }
        }
      }
     else WrError(1135);
     return;
    }

   if ((Memo("ANLD")) || (Memo("ORLD")) || (Memo("MOVD")))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       OK=false;
       if ((Memo("MOVD")) && (strcasecmp(ArgStr[1],"A")==0))
        {
         strcpy(ArgStr[1],ArgStr[2]); strmaxcpy(ArgStr[2],"A",255); OK=true;
        }
       if (strcasecmp(ArgStr[2],"A")!=0) WrError(1350);
       else if ((strlen(ArgStr[1])!=2) || (toupper(*ArgStr[1])!='P')) WrError(1350);
       else if ((ArgStr[1][1]<'4') || (ArgStr[1][1]>'7')) WrError(1320);
       else
        {
         CodeLen=1; BAsmCode[0]=0x0c+ArgStr[1][1]-'4';
         if (Memo("ANLD")) BAsmCode[0]+=0x90;
         else if (Memo("ORLD")) BAsmCode[0]+=0x80;
         else if (! OK) BAsmCode[0]+=0x30;
         ChkN802X();
        }
      }
     return;
    }

   if ((Memo("MOVP")) || (Memo("MOVP3")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if ((strcasecmp(ArgStr[1],"A")!=0) || (strcasecmp(ArgStr[2],"@A")!=0)) WrError(1350);
     else
      {
       CodeLen=1; BAsmCode[0]=0xa3;
       if (Memo("MOVP3")) BAsmCode[0]+=0x40;
       ChkN802X();
      }
     return;
    }

   if (Memo("MOVX"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       OK=false;
       if (strcasecmp(ArgStr[2],"A")==0)
	{
	 strcpy(ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"A",255); OK=true;
	}
       if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
       else
	{
	 DecodeAdr(ArgStr[2]);
	 if (AdrMode!=ModInd) WrError(1350);
	 else
	  {
	   CodeLen=1; BAsmCode[0]=0x80+AdrVal;
	   if (OK) BAsmCode[0]+=0x10;
	   ChkN802X(); ChkNUPI();
	  }
        }
      }
     return;
    }

   if (Memo("NOP"))
    {
     if (ArgCnt!=0) WrError(1110);
     else
      {
       CodeLen=1; BAsmCode[0]=0x00;
      }
     return;
    }

   if (Memo("OUT"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"DBB")!=0) WrError(1350);
     else if (strcasecmp(ArgStr[2],"A")!=0) WrError(1350);
     else
      {
       BAsmCode[0]=0x02; CodeLen=1; ChkUPI();
      }
     return;
    }

   if (Memo("OUTL"))
    {
     NLS_UpString(ArgStr[1]);
     if (ArgCnt!=2) WrError(1110);
     else
      {
       NLS_UpString(ArgStr[1]);
       if (strcasecmp(ArgStr[2],"A")!=0) WrError(1350);
       else if (strcmp(ArgStr[1],"BUS")==0)
        {
         CodeLen=1; BAsmCode[0]=0x02;
         ChkN802X(); ChkExt(); ChkNUPI();
        }
       else if (strcmp(ArgStr[1],"P0")==0)
        {
         CodeLen=1; BAsmCode[0]=0x90;
        }
       else if ((strcmp(ArgStr[1],"P1")==0) || (strcmp(ArgStr[1],"P2")==0))
        {
         CodeLen=1; BAsmCode[0]=0x38+ArgStr[1][1]-'0';
        }
       else WrError(1350);
      }
     return;
    }

   if ((Memo("RET")) || (Memo("RETR")))
    {
     if (ArgCnt!=0) WrError(1110);
     else
      {
       CodeLen=1; BAsmCode[0]=0x83;
       if (strlen(OpPart)==4)
        {
	 BAsmCode[0]+=0x10; ChkN802X();
        }
      }
     return;
    }

   if (Memo("SEL"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU==CPU8021) WrError(1500);
     else
      {
       OK=false; NLS_UpString(ArgStr[1]);
       for (z=0; z<SelOrderCnt; z++)
       if (strcmp(ArgStr[1],SelOrders[z].Name)==0)
	{
	 CodeLen=1; BAsmCode[0]=SelOrders[z].Code; OK=true;
	 if ((SelOrders[z].Is22) && (MomCPU!=CPU8022))
	  {
	   CodeLen=0; WrError(1500);
	  }
	 if (SelOrders[z].IsNUPI) ChkNUPI();
        }
       if (! OK) WrError(1350);
      }
     return;
    }

   if (Memo("STOP"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"TCNT")!=0) WrError(1350);
     else
      {
       CodeLen=1; BAsmCode[0]=0x65;
      }
     return;
    }

   if (Memo("STRT"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       NLS_UpString(ArgStr[1]);
       if (strcmp(ArgStr[1],"CNT")==0)
        {
         CodeLen=1; BAsmCode[0]=0x45;
        }
       else if (strcmp(ArgStr[1],"T")==0)
        {
         CodeLen=1; BAsmCode[0]=0x55;
        }
       else WrError(1350);
      }
     return;
    }

   if (Memo("XCH"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       if (strcasecmp(ArgStr[2],"A")==0)
        {
         strcpy(ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"A",255);
        }
       if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
       else
	{
	 DecodeAdr(ArgStr[2]);
         switch (AdrMode)
          {
           case ModReg:
	    CodeLen=1; BAsmCode[0]=0x28+AdrVal;
            break;
	   case ModInd:
	    CodeLen=1; BAsmCode[0]=0x20+AdrVal;
	    break;
	   default:
            WrError(1350);
	  }
        }
      }
     return;
    }

   if (Memo("XCHD"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       if (strcasecmp(ArgStr[2],"A")==0)
        {
         strcpy(ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"A",255);
        }
       if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
       else
        {
	 DecodeAdr(ArgStr[2]);
         if (AdrMode!=ModInd) WrError(1350);
         else
          {
           CodeLen=1; BAsmCode[0]=0x30+AdrVal;
  	  }
        }
      }
     return;
    }

   if (Memo("RAD"))
    {
     if (ArgCnt!=0) WrError(1110);
     else if (MomCPU!=CPU8022) WrError(1500);
     else
      {
       CodeLen=1; BAsmCode[0]=0x80;
      }
     return;
    }

   if (Memo("RETI"))
    {
     if (ArgCnt!=0) WrError(1110);
     else if (MomCPU!=CPU8022) WrError(1500);
     else
      {
       CodeLen=1; BAsmCode[0]=0x93;
      }
     return;
    }

   if (Memo("IDL"))
    {
     if (ArgCnt!=0) WrError(1110);
     else if ((MomCPU!=CPU80C39) && (MomCPU!=CPU80C48)) WrError(1500);
     else
      {
       CodeLen=1; BAsmCode[0]=0x01;
      }
     return;
    }

   WrXError(1200,OpPart);
}

	static bool ChkPC_48(void)
{
   bool ok;

   switch (ActPC)
    {
     case SegCode:
      switch (MomCPU-CPU8021)
       {
	case D_CPU8041: ok=(ProgCounter() <  0x400); break;
	case D_CPU8042: ok=(ProgCounter() <  0x800); break;
	default       : ok=(ProgCounter() < 0x1000); break;
       }
      break;
     case SegXData:
     case SegIData:
      ok=(ProgCounter() <  0x100);
      break;
     default:
      ok=false;
    }
   return (ok);
}


	static bool IsDef_48(void)
{
   return false;
}

        static void SwitchFrom_48(void)
{
   DeinitFields();
}

	static void SwitchTo_48(void)
{
   TurnWords=false; ConstMode=ConstModeIntel; SetIsOccupied=false;

   PCSymbol="$"; HeaderID=0x21; NOPCode=0x00;
   DivideChars=","; HasAttrs=false;

   ValidSegs=(1<<SegCode)|(1<<SegIData)|(1<<SegXData);
   Grans[SegCode ]=1; ListGrans[SegCode ]=1; SegInits[SegCode ]=0;
   Grans[SegIData]=1; ListGrans[SegIData]=1; SegInits[SegIData]=0x20;
   Grans[SegXData]=1; ListGrans[SegXData]=1; SegInits[SegXData]=0;

   MakeCode=MakeCode_48; ChkPC=ChkPC_48; IsDef=IsDef_48;
   SwitchFrom=SwitchFrom_48; InitFields();
}

	void code48_init(void)
{
   CPU8021 =AddCPU("8021" ,SwitchTo_48);
   CPU8022 =AddCPU("8022" ,SwitchTo_48);
   CPU8039 =AddCPU("8039" ,SwitchTo_48);
   CPU8048 =AddCPU("8048" ,SwitchTo_48);
   CPU80C39=AddCPU("80C39",SwitchTo_48);
   CPU80C48=AddCPU("80C48",SwitchTo_48);
   CPU8041 =AddCPU("8041" ,SwitchTo_48);
   CPU8042 =AddCPU("8042" ,SwitchTo_48);
}
