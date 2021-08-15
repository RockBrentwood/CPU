/* codescmp.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator National SC/MP                                              */
/*                                                                           */
/* Historie: 17.2.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"


#define FixedOrderCnt 21
#define ImmOrderCnt 8
#define RegOrderCnt 3
#define MemOrderCnt 8
#define JmpOrderCnt 4

typedef struct
         {
          char *Name;
          Byte Code;
         } FixedOrder;


static CPUVar CPUSCMP;

static FixedOrder *FixedOrders;
static FixedOrder *ImmOrders;
static FixedOrder *RegOrders;
static FixedOrder *MemOrders;
static FixedOrder *JmpOrders;

/*---------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Byte NCode)
{
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
}

        static void AddImm(char *NName, Byte NCode)
{
   if (InstrZ>=ImmOrderCnt) exit(255);
   ImmOrders[InstrZ].Name=NName;
   ImmOrders[InstrZ++].Code=NCode;
}

        static void AddReg(char *NName, Byte NCode)
{
   if (InstrZ>=RegOrderCnt) exit(255);
   RegOrders[InstrZ].Name=NName;
   RegOrders[InstrZ++].Code=NCode;
}

        static void AddMem(char *NName, Byte NCode)
{
   if (InstrZ>=MemOrderCnt) exit(255);
   MemOrders[InstrZ].Name=NName;
   MemOrders[InstrZ++].Code=NCode;
}

        static void AddJmp(char *NName, Byte NCode)
{
   if (InstrZ>=JmpOrderCnt) exit(255);
   JmpOrders[InstrZ].Name=NName;
   JmpOrders[InstrZ++].Code=NCode;
}

        static void InitFields(void)
{
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("LDE" ,0x40); AddFixed("XAE" ,0x01); AddFixed("ANE" ,0x50);
   AddFixed("ORE" ,0x58); AddFixed("XRE" ,0x60); AddFixed("DAE" ,0x68);
   AddFixed("ADE" ,0x70); AddFixed("CAE" ,0x78); AddFixed("SIO" ,0x19);
   AddFixed("SR"  ,0x1c); AddFixed("SRL" ,0x1d); AddFixed("RR"  ,0x1e);
   AddFixed("RRL" ,0x1f); AddFixed("HALT",0x00); AddFixed("CCL" ,0x02);
   AddFixed("SCL" ,0x03); AddFixed("DINT",0x04); AddFixed("IEN" ,0x05);
   AddFixed("CSA" ,0x06); AddFixed("CAS" ,0x07); AddFixed("NOP" ,0x08);

   ImmOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*ImmOrderCnt); InstrZ=0;
   AddImm("LDI" ,0xc4); AddImm("ANI" ,0xd4); AddImm("ORI" ,0xdc);
   AddImm("XRI" ,0xe4); AddImm("DAI" ,0xec); AddImm("ADI" ,0xf4);
   AddImm("CAI" ,0xfc); AddImm("DLY" ,0x8f);

   RegOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RegOrderCnt); InstrZ=0;
   AddReg("XPAL",0x30); AddReg("XPAH",0x34); AddReg("XPPC",0x3c);

   MemOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*MemOrderCnt); InstrZ=0;
   AddMem("LD"  ,0xc0); AddMem("ST"  ,0xc8); AddMem("AND" ,0xd0);
   AddMem("OR"  ,0xd8); AddMem("XOR" ,0xe0); AddMem("DAD" ,0xe8);
   AddMem("ADD" ,0xf0); AddMem("CAD" ,0xf8);

   JmpOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*JmpOrderCnt); InstrZ=0;
   AddJmp("JMP" ,0x90); AddJmp("JP"  ,0x94); AddJmp("JZ"  ,0x98);
   AddJmp("JNZ" ,0x9c);
}

        static void DeinitFields(void)
{
   free(FixedOrders);
   free(ImmOrders);
   free(RegOrders);
   free(MemOrders);
   free(JmpOrders);
}

/*---------------------------------------------------------------------------*/

	static bool DecodeReg(char *Asc, Byte *Erg)
{
   if ((strlen(Asc)!=2) || (toupper(*Asc)!='P')) return false;

   switch (toupper(Asc[1]))
    {
     case 'C': *Erg=0; break;
     case '0': case '1': case '2':
     case '3': *Erg=Asc[1]-'0'; break;
     default: return false;
    }

   return true;
}

	static bool DecodeAdr(char *Asc, bool MayInc, Byte PCDisp, Byte *Arg)
{
   Integer Disp;
   Word PCVal;
   bool OK;
   int l=strlen(Asc);

   if ((l>=4) && (Asc[l-1]==')') && (Asc[l-4]=='('))
    {
     Asc[l-1]='\0';
     if (DecodeReg(Asc+l-3,Arg))
      {
       Asc[l-4]='\0';
       if (*Asc=='@')
        {
         if (! MayInc)
          {
           WrError(1350); return false;
          }
         strcpy(Asc,Asc+1); *Arg+=4;
        }
       if (strcasecmp(Asc,"E")==0) BAsmCode[1]=0x80;
       else if (*Arg==0)
        {
         WrXError(1445,Asc+l-3); return false;
        }
       else
        {
         BAsmCode[1]=EvalIntExpression(Asc,SInt8,&OK);
         if (! OK) return false;
        }
       return true;
      }
     else Asc[l-1]=')';
    }

   PCVal=(EProgCounter() & 0xf000)+((EProgCounter()+1) & 0xfff);
   Disp=EvalIntExpression(Asc,UInt16,&OK)-PCDisp-PCVal;
   if (OK)
    if ((! SymbolQuestionable) && ((Disp<-128) || (Disp>127))) WrError(1370);
    else
     {
      BAsmCode[1]=Disp & 0xff; *Arg=0; return true;
     }
   return false;
}

/*---------------------------------------------------------------------------*/

	static bool DecodePseudo(void)
{
   return false;
}

        static void ChkPage(void)
{
   if (((EProgCounter()) & 0xf000)!=((EProgCounter()+CodeLen) & 0xf000)) WrError(250);
}

        static void MakeCode_SCMP(void)
{
   Integer z;
   bool OK;

   CodeLen=0; DontPrint=false;

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
      else
       {
        BAsmCode[0]=FixedOrders[z].Code; CodeLen=1;
       }
      return;
     }

   /* immediate */

   for (z=0; z<ImmOrderCnt; z++)
    if (Memo(ImmOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else
       {
        BAsmCode[1]=EvalIntExpression(ArgStr[1],Int8,&OK);
        if (OK)
         {
          BAsmCode[0]=ImmOrders[z].Code; CodeLen=2; ChkPage();
         }
       }
      return;
     }

   /* ein Register */

   for (z=0; z<RegOrderCnt; z++)
    if (Memo(RegOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (! DecodeReg(ArgStr[1],BAsmCode+0)) WrXError(1445,ArgStr[1]);
      else
       {
        BAsmCode[0]+=RegOrders[z].Code; CodeLen=1;
       }
      return;
     }

   /* ein Speicheroperand */

   for (z=0; z<MemOrderCnt; z++)
    if (Memo(MemOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (DecodeAdr(ArgStr[1],true,0,BAsmCode+0))
       {
        BAsmCode[0]+=MemOrders[z].Code; CodeLen=2; ChkPage();
       }
      return;
     }

   for (z=0; z<JmpOrderCnt; z++)
    if (Memo(JmpOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (DecodeAdr(ArgStr[1],false,1,BAsmCode+0))
       {
        BAsmCode[0]+=JmpOrders[z].Code; CodeLen=2; ChkPage();
       }
      return;
     }

   if ((Memo("ILD")) || (Memo("DLD")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (DecodeAdr(ArgStr[1],false,0,BAsmCode+0))
      {
       BAsmCode[0]+=0xa8+(Memo("DLD") << 4); CodeLen=2; ChkPage();
      }
     return;
    }

   WrXError(1200,OpPart);
}

        static bool ChkPC_SCMP(void)
{
   switch (ActPC)
    {
     case SegCode: return (ProgCounter()<0x10000);
     default: return false;
    }
}

        static bool IsDef_SCMP(void)
{
   return false;
}

        static void SwitchFrom_SCMP(void)
{
   DeinitFields();
}

        static void SwitchTo_SCMP(void)
{
   TurnWords=false; ConstMode=ConstModeC; SetIsOccupied=false;

   PCSymbol="$"; HeaderID=0x6e; NOPCode=0x08;
   DivideChars=","; HasAttrs=false;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_SCMP; ChkPC=ChkPC_SCMP; IsDef=IsDef_SCMP;
   SwitchFrom=SwitchFrom_SCMP; InitFields();
}

	void codescmp_init(void)
{
   CPUSCMP=AddCPU("SC/MP",SwitchTo_SCMP);
}


