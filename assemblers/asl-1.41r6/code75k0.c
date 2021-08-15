/* code75k0.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator NEC 75K0                                                    */
/*                                                                           */
/* Historie: 31.12.1996 Grundsteinlegung                                     */
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


#define ModNone (-1)
#define ModReg4 0
#define MModReg4 (1 << ModReg4)
#define ModReg8 1
#define MModReg8 (1 << ModReg8)
#define ModImm 2
#define MModImm (1 << ModImm)
#define ModInd 3
#define MModInd (1 << ModInd)
#define ModAbs 4
#define MModAbs (1 << ModAbs)

#define FixedOrderCount 6
#define AriOrderCount 3
#define LogOrderCount 3

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;


static SimpProc SaveInitProc;

static FixedOrder *FixedOrders;
static char **AriOrders;
static char **LogOrders;

static LongInt MBSValue,MBEValue;
static bool MinOneIs0;
static CPUVar
   CPU75402,CPU75004,CPU75006,CPU75008,
   CPU75268,CPU75304,CPU75306,CPU75308,
   CPU75312,CPU75316,CPU75328,CPU75104,
   CPU75106,CPU75108,CPU75112,CPU75116,
   CPU75206,CPU75208,CPU75212,CPU75216,
   CPU75512,CPU75516;
static Word ROMEnd;

static ShortInt OpSize;
static Byte AdrPart;
static ShortInt AdrMode;

/*-------------------------------------------------------------------------*/
/* dynamische Codetabellenverwaltung */

   	static void AddFixed(char *NewName, Word NewCode)
{
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NewName;
   FixedOrders[InstrZ++].Code=NewCode;
}

	static void InitFields(void)
{
   bool Err;

   ROMEnd=ConstLongInt(MomCPUName+3,&Err);
   if (ROMEnd>2) ROMEnd%=10;
   ROMEnd=(ROMEnd << 10)-1;

   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("RET" ,0x00ee);
   AddFixed("RETS",0x00e0);
   AddFixed("RETI",0x00ef);
   AddFixed("HALT",0xa39d);
   AddFixed("STOP",0xb39d);
   AddFixed("NOP" ,0x0060);

   AriOrders=(char **) malloc(sizeof(char *)*AriOrderCount); InstrZ=0;
   AriOrders[InstrZ++]="ADDC";
   AriOrders[InstrZ++]="SUBS";
   AriOrders[InstrZ++]="SUBC";

   LogOrders=(char **) malloc(sizeof(char *)*LogOrderCount); InstrZ=0;
   LogOrders[InstrZ++]="AND";
   LogOrders[InstrZ++]="OR";
   LogOrders[InstrZ++]="XOR";
}

	static void DeinitFields(void)
{
   free(FixedOrders);
   free(AriOrders);
   free(LogOrders);
}

/*-------------------------------------------------------------------------*/
/* Untermengen von Befehlssatz abpruefen */

	static void CheckCPU(CPUVar MinCPU)
{
   if (MomCPU<MinCPU)
    {
     WrError(1500); CodeLen=0;
    }
}

/*-------------------------------------------------------------------------*/
/* Adressausdruck parsen */

	static bool SetOpSize(ShortInt NewSize)
{
   if (OpSize==-1) OpSize=NewSize;
   else if (NewSize!=OpSize)
    {
     WrError(1131); return false;
    }
   return true;
}

	static void ChkDataPage(Word Adr)
{
   switch (MBEValue)
    {
     case 0: if ((Adr>0x7f) && (Adr<0xf80)) WrError(110); break;
     case 1: if (Hi(Adr)!=MBSValue) WrError(110); break;
    }
}

	static void ChkAdr(Byte Mask)
{
   if ((AdrMode!=ModNone) && ((Mask & (1 << AdrMode))==0))
    {
     WrError(1350); AdrMode=ModNone;
    }
}

	static void DecodeAdr(char *Asc, Byte Mask)
{
   static char *RegNames="XAHLDEBC";

   char *p;
   int pos;
   bool OK;
   String s;

   AdrMode=ModNone;

   /* Register ? */

   memcpy(s,Asc,2); s[2]='\0'; NLS_UpString(s);
   p=strstr(RegNames,s);

   if (p!=NULL)
    {
     pos=p-RegNames;

     /* 8-Bit-Register ? */

     if (strlen(Asc)==1)
      {
       AdrPart=pos ^ 1;
       if (SetOpSize(0))
        if ((AdrPart>4) && (MomCPU<CPU75004)) WrError(1505);
	else AdrMode=ModReg4;
       ChkAdr(Mask); return;
      }

     /* 16-Bit-Register ? */

     if ((strlen(Asc)==2) && (! Odd(pos)))
      {
       AdrPart=pos;
       if (SetOpSize(1))
        if ((AdrPart>2) && (MomCPU<CPU75004)) WrError(1505);
	else AdrMode=ModReg8;
       ChkAdr(Mask); return;
      }

     /* 16-Bit-Schattenregister ? */

     if ((strlen(Asc)==3) && ((Asc[2]=='\'') || (Asc[2]=='`')) && (! Odd(pos)))
      {
       AdrPart=pos+1;
       if (SetOpSize(1))
        if (MomCPU<CPU75104) WrError(1505); else AdrMode=ModReg8;
       ChkAdr(Mask); return;
      }
    }

   /* immediate? */

   if (*Asc=='#')
    {
     if ((OpSize==-1) && (MinOneIs0)) OpSize=0;
     FirstPassUnknown=false;
     switch (OpSize)
      {
       case -1: WrError(1132); break;
       case 0: AdrPart=EvalIntExpression(Asc+1,Int4,&OK) & 15; break;
       case 1: AdrPart=EvalIntExpression(Asc+1,Int8,&OK); break;
      };
     if (OK) AdrMode=ModImm;
     ChkAdr(Mask); return;
    }

   /* indirekt ? */

   if (*Asc=='@')
    {
     strmaxcpy(s,Asc+1,255);
     if (strcasecmp(s,"HL")==0) AdrPart=1;
     else if (strcasecmp(s,"HL+")==0) AdrPart=2;
     else if (strcasecmp(s,"HL-")==0) AdrPart=3;
     else if (strcasecmp(s,"DE")==0) AdrPart=4;
     else if (strcasecmp(s,"DL")==0) AdrPart=5;
     else AdrPart=0;
     if (AdrPart!=0)
      {
       if ((MomCPU<CPU75004) && (AdrPart!=1)) WrError(1505);
       else if ((MomCPU<CPU75104) && ((AdrPart==2) || (AdrPart==3))) WrError(1505);
       else AdrMode=ModInd;
       ChkAdr(Mask); return;
      }
    }

   /* absolut */

   FirstPassUnknown=false;
   pos=EvalIntExpression(Asc,UInt12,&OK);
   if (OK)
    {
     AdrPart=Lo(pos); AdrMode=ModAbs;
     ChkSpace(SegData);
     if (! FirstPassUnknown) ChkDataPage(pos);
    }

   ChkAdr(Mask);
}

static String BName;

	static bool DecodeBitAddr(char *Asc, Word *Erg)
{
   char *p;
   int Num;
   bool OK;
   Word Adr;
   String bpart;

   p=QuotPos(Asc,'.');
   if (p==NULL)
    {
     *Erg=EvalIntExpression(Asc,Int16,&OK);
     if (Hi(*Erg)!=0) ChkDataPage(((*Erg >> 4) & 0xf00)+Lo(*Erg));
     return OK;
    }

   *p='\0';
   strmaxcpy(bpart,p+1,255);

   if (strcasecmp(bpart,"@L")==0)
    {
     FirstPassUnknown=false;
     Adr=EvalIntExpression(Asc,UInt12,&OK);
     if (FirstPassUnknown) Adr=(Adr & 0xffc) | 0xfc0;
     if (OK)
      {
       ChkSpace(SegData);
       if ((Adr & 3)!=0) WrError(1325);
       else if (Adr<0xfc0) WrError(1315);
       else if (MomCPU<CPU75004) WrError(1505);
       else
        {
         *Erg=0x40+((Adr & 0x3c) >> 2);
         sprintf(BName,"%sH.@L",HexString(Adr,3));
         return true;
        }
      }
    }
   else
    {
     Num=EvalIntExpression(bpart,UInt2,&OK);
     if (OK)
      if (strncasecmp(Asc,"@H",2)==0)
       {
        Adr=EvalIntExpression(Asc+2,UInt4,&OK);
        if (OK)
         if (MomCPU<CPU75004) WrError(1505);
         else
          {
           *Erg=(Num << 4)+Adr;
           sprintf(BName,"@H%s.%c",HexString(Adr,1),Num+'0');
           return true;
          }
       }
      else
       {
        FirstPassUnknown=false;
        Adr=EvalIntExpression(Asc,UInt12,&OK);
        if (FirstPassUnknown) Adr=(Adr | 0xff0);
        if (OK)
         {
          ChkSpace(SegData);
	  if ((Adr>=0xfb0) && (Adr<0xfc0))
           *Erg=0x80+(Num << 4)+(Adr & 15);
          else if (Adr>=0xff0)
           *Erg=0xc0+(Num << 4)+(Adr & 15);
          else
           *Erg=0x400+(((Word)Num) << 8)+Lo(Adr)+(Hi(Adr) << 12);
          sprintf(BName,"%sH.%c",HexString(Adr,3),'0'+Num);
          return true;
         }
       }
    }
   return false;
}

	static bool DecodeIntName(char *Asc, Byte *Erg)
{
   Word HErg;
   Byte LPart;
   String Asc_N;

   strmaxcpy(Asc_N,Asc,255); NLS_UpString(Asc_N); Asc=Asc_N;

   if (MomCPU<=CPU75402) LPart=0;
   else if (MomCPU<CPU75004) LPart=1;
   else if (MomCPU<CPU75104) LPart=2;
   else LPart=3;
        if (strcmp(Asc,"IEBT")==0)   HErg=0x000;
   else if (strcmp(Asc,"IEW")==0)    HErg=0x102;
   else if (strcmp(Asc,"IETPG")==0)  HErg=0x203;
   else if (strcmp(Asc,"IET0")==0)   HErg=0x104;
   else if (strcmp(Asc,"IECSI")==0)  HErg=0x005;
   else if (strcmp(Asc,"IECSIO")==0) HErg=0x205;
   else if (strcmp(Asc,"IE0")==0)    HErg=0x006;
   else if (strcmp(Asc,"IE2")==0)    HErg=0x007;
   else if (strcmp(Asc,"IE4")==0)    HErg=0x120;
   else if (strcmp(Asc,"IEKS")==0)   HErg=0x123;
   else if (strcmp(Asc,"IET1")==0)   HErg=0x224;
   else if (strcmp(Asc,"IE1")==0)    HErg=0x126;
   else if (strcmp(Asc,"IE3")==0)    HErg=0x227;
   else HErg=0xfff;
   if (HErg==0xfff) return false;
   else if (Hi(HErg)>LPart) return false;
   else
    {
     *Erg=Lo(HErg); return true;
    }
}

/*-------------------------------------------------------------------------*/

	static bool DecodePseudo(void)
{
#define ASSUME75Count 2
   static ASSUMERec ASSUME75s[ASSUME75Count]=
	     {{"MBS", &MBSValue, 0, 0x0f, 0x10},
	      {"MBE", &MBEValue, 0, 0x01, 0x01}};

   Word BErg;

   if (Memo("ASSUME"))
    {
     CodeASSUME(ASSUME75s,ASSUME75Count);
     if ((MomCPU==CPU75402) && (MBEValue!=0))
      {
       MBEValue=0; WrError(1440);
      }
     return true;
    }

   if (Memo("SFR"))
    {
     CodeEquate(SegData,0,0xfff);
     return true;
    }

   if (Memo("BIT"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       FirstPassUnknown=false;
       if (DecodeBitAddr(ArgStr[1],&BErg))
        if (! FirstPassUnknown)
	 {
          PushLocHandle(-1);
	  EnterIntSymbol(LabPart,BErg,SegNone,false);
          sprintf(ListLine,"=%s",BName);
          PopLocHandle();
         }
      }
     return true;
    }

   return false;
}

	static void PutCode(Word Code)
{
   BAsmCode[0]=Lo(Code);
   if (Hi(Code)==0) CodeLen=1;
   else
    {
     BAsmCode[1]=Hi(Code); CodeLen=2;
    }
}

        static void MakeCode_75K0(void)
{
   Integer z,AdrInt,Dist;
   Byte HReg;
   Word BVal;
   bool OK,BrRel,BrLong;

   CodeLen=0; DontPrint=false; OpSize=(-1); MinOneIs0=false;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(true)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     {
      if (ArgCnt!=0) WrError(1110);
      else PutCode(FixedOrders[z].Code);
      return;
     }

   /* Datentransfer */

   if (Memo("MOV"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg4+MModReg8+MModInd+MModAbs);
       switch (AdrMode)
        {
         case ModReg4:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg4+MModInd+MModAbs+MModImm);
          switch (AdrMode)
           {
            case ModReg4:
             if (HReg==0)
	      {
               PutCode(0x7899+(((Word)AdrPart) << 8)); CheckCPU(CPU75004);
              }
             else if (AdrPart==0)
	      {
               PutCode(0x7099+(((Word)HReg) << 8)); CheckCPU(CPU75004);
              }
             else WrError(1350);
             break;
            case ModInd:
             if (HReg!=0) WrError(1350);
             else PutCode(0xe0+AdrPart);
             break;
            case ModAbs:
             if (HReg!=0) WrError(1350);
             else
              {
               BAsmCode[0]=0xa3; BAsmCode[1]=AdrPart; CodeLen=2;
              }
             break;
            case ModImm:
             if (HReg==0) PutCode(0x70+AdrPart);
             else
	      {
	       PutCode(0x089a+(((Word)AdrPart) << 12)+(((Word)HReg) << 8));
               CheckCPU(CPU75004);
              }
             break;
           }
          break;
         case ModReg8:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModInd+MModImm);
          switch (AdrMode)
           {
            case ModReg8:
             if (HReg==0)
	      {
               PutCode(0x58aa+(((Word)AdrPart) << 8)); CheckCPU(CPU75004);
              }
	     else if (AdrPart==0)
	      {
               PutCode(0x50aa+(((Word)HReg) << 8)); CheckCPU(CPU75004);
              }
	     else WrError(1350);
             break;
            case ModAbs:
             if (HReg!=0) WrError(1350);
             else
              {
               BAsmCode[0]=0xa2; BAsmCode[1]=AdrPart; CodeLen=2;
               if ((! FirstPassUnknown) && (Odd(AdrPart))) WrError(180);
              }
             break;
            case ModInd:
             if ((HReg!=0) || (AdrPart!=1)) WrError(1350);
             else
  	      {
               PutCode(0x18aa); CheckCPU(CPU75004);
              }
             break;
            case ModImm:
             if (Odd(HReg)) WrError(1350);
             else
              {
               BAsmCode[0]=0x89+HReg; BAsmCode[1]=AdrPart; CodeLen=2;
              }
             break;
           }
          break;
         case ModInd:
          if (AdrPart!=1) WrError(1350);
          else
           {
            DecodeAdr(ArgStr[2],MModReg4+MModReg8);
            switch (AdrMode)
             {
              case ModReg4:
               if (AdrPart!=0) WrError(1350);
	       else
	        {
                 PutCode(0xe8); CheckCPU(CPU75004);
                }
               break;
              case ModReg8:
               if (AdrPart!=0) WrError(1350);
	       else
	        {
                 PutCode(0x10aa); CheckCPU(CPU75004);
                }
               break;
             }
           }
          break;
         case ModAbs:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg4+MModReg8);
          switch (AdrMode)
           {
            case ModReg4:
             if (AdrPart!=0) WrError(1350);
	     else
	      {
	       BAsmCode[0]=0x93; BAsmCode[1]=HReg; CodeLen=2;
              }
             break;
            case ModReg8:
             if (AdrPart!=0) WrError(1350);
	     else
	      {
	       BAsmCode[0]=0x92; BAsmCode[1]=HReg; CodeLen=2;
               if ((! FirstPassUnknown) && (Odd(HReg))) WrError(180);
	      }
             break;
           }
          break;
        }
      }
     return;
    }

   if (Memo("XCH"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg4+MModReg8+MModAbs+MModInd);
       switch (AdrMode)
        {
         case ModReg4:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg4+MModAbs+MModInd);
          switch (AdrMode)
           {
            case ModReg4:
             if (HReg==0) PutCode(0xd8+AdrPart);
             else if (AdrPart==0) PutCode(0xd8+HReg);
             else WrError(1350);
             break;
            case ModAbs:
             if (HReg!=0) WrError(1350);
             else
              {
               BAsmCode[0]=0xb3; BAsmCode[1]=AdrPart; CodeLen=2;
              }
             break;
            case ModInd:
             if (HReg!=0) WrError(1350);
             else PutCode(0xe8+AdrPart);
             break;
           }
          break;
         case ModReg8:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModInd);
          switch (AdrMode)
           {
            case ModReg8:
             if (HReg==0)
	      {
               PutCode(0x40aa+(((Word)AdrPart) << 8)); CheckCPU(CPU75004);
              }
             else if (AdrPart==0)
	      {
               PutCode(0x40aa+(((Word)HReg) << 8)); CheckCPU(CPU75004);
              }
             else WrError(1350);
             break;
            case ModAbs:
             if (HReg!=0) WrError(1350);
             else
              {
               BAsmCode[0]=0xb2; BAsmCode[1]=AdrPart; CodeLen=2;
               if ((FirstPassUnknown) && (Odd(AdrPart))) WrError(180);
              }
             break;
            case ModInd:
             if ((AdrPart!=1) || (HReg!=0)) WrError(1350);
             else
	      {
               PutCode(0x11aa); CheckCPU(CPU75004);
              }
             break;
           }
          break;
         case ModAbs:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg4+MModReg8);
          switch (AdrMode)
           {
            case ModReg4:
             if (AdrPart!=0) WrError(1350);
             else
              {
               BAsmCode[0]=0xb3; BAsmCode[1]=HReg; CodeLen=2;
              }
             break;
            case ModReg8:
             if (AdrPart!=0) WrError(1350);
             else
              {
               BAsmCode[0]=0xb2; BAsmCode[1]=HReg; CodeLen=2;
               if ((FirstPassUnknown) && (Odd(HReg))) WrError(180);
              }
             break;
           }
          break;
         case ModInd:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg4+MModReg8);
          switch (AdrMode)
           {
            case ModReg4:
             if (AdrPart!=0) WrError(1350);
             else PutCode(0xe8+HReg);
             break;
            case ModReg8:
             if ((AdrPart!=0) || (HReg!=1)) WrError(1350);
             else
	      {
               PutCode(0x11aa); CheckCPU(CPU75004);
              };
             break;
           }
          break;
        }
      }
     return;
    }

   if (Memo("MOVT"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"XA")!=0) WrError(1350);
     else if (strcasecmp(ArgStr[2],"@PCDE")==0)
      {
       PutCode(0xd4); CheckCPU(CPU75004);
      }
     else if (strcasecmp(ArgStr[2],"@PCXA")==0) PutCode(0xd0);
     else WrError(1350);
     return;
    }

   if ((Memo("PUSH")) || (Memo("POP")))
    {
     OK=Memo("PUSH");
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"BS")==0)
      {
       PutCode(0x0699+(OK << 8)); CheckCPU(CPU75004);
      }
     else
      {
       DecodeAdr(ArgStr[1],MModReg8);
       switch (AdrMode)
        {
         case ModReg8:
          if (Odd(AdrPart)) WrError(1350);
          else PutCode(0x48+OK+AdrPart);
          break;
        }
      }
     return;
    }

   if ((Memo("IN")) || (Memo("OUT")))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       z=Memo("IN");
       if (z>0)
        {
         strcpy(ArgStr[3],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[3]);
        }
       if (strncasecmp(ArgStr[1],"PORT",4)!=0) WrError(1350);
       else
        {
         BAsmCode[1]=0xf0+EvalIntExpression(ArgStr[1]+4,UInt4,&OK);
         if (OK)
          {
           DecodeAdr(ArgStr[2],MModReg8+MModReg4);
           switch (AdrMode)
            {
             case ModReg4:
              if (AdrPart!=0) WrError(1350);
              else
               {
                BAsmCode[0]=0x93+(z << 4); CodeLen=2;
               }
              break;
             case ModReg8:
              if (AdrPart!=0) WrError(1350);
              else
               {
                BAsmCode[0]=0x92+(z << 4); CodeLen=2;
                CheckCPU(CPU75004);
               }
              break;
            }
          }
        }
      }
     return;
    }

   /* Arithmetik */

   if (Memo("ADDS"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg4+MModReg8);
       switch (AdrMode)
        {
         case ModReg4:
          if (AdrPart!=0) WrError(1350);
          else
           {
            DecodeAdr(ArgStr[2],MModImm+MModInd);
            switch (AdrMode)
             {
              case ModImm:
               PutCode(0x60+AdrPart); break;
              case ModInd:
               if (AdrPart==1) PutCode(0xd2); else WrError(1350);
               break;
             }
           }
          break;
         case ModReg8:
          if (AdrPart==0)
           {
            DecodeAdr(ArgStr[2],MModReg8+MModImm);
            switch (AdrMode)
             {
              case ModReg8:
	       PutCode(0xc8aa+(((Word)AdrPart) << 8));
               CheckCPU(CPU75104);
               break;
              case ModImm:
               BAsmCode[0]=0xb9; BAsmCode[1]=AdrPart;
               CodeLen=2;
               CheckCPU(CPU75104);
               break;
             }
           }
          else if (strcasecmp(ArgStr[2],"XA")!=0) WrError(1350);
          else
  	   {
	    PutCode(0xc0aa+(((Word)AdrPart) << 8));
            CheckCPU(CPU75104);
           }
          break;
        }
      }
     return;
    }

   for (z=0; z<AriOrderCount; z++)
    if (Memo(AriOrders[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
        DecodeAdr(ArgStr[1],MModReg4+MModReg8);
        switch (AdrMode)
         {
          case ModReg4:
           if (AdrPart!=0) WrError(1350);
           else
            {
             DecodeAdr(ArgStr[2],MModInd);
             switch (AdrMode)
              {
               case ModInd:
	        if (AdrPart==1)
                 {
                  BAsmCode[0]=0xa8;
                  if (z==0) BAsmCode[0]++;
                  if (z==2) BAsmCode[0]+=0x10;
                  CodeLen=1;
                  if (! Memo("ADDC")) CheckCPU(CPU75004);
                 }
	        else WrError(1350);
               break;
              }
            }
           break;
          case ModReg8:
           if (AdrPart==0)
            {
             DecodeAdr(ArgStr[2],MModReg8);
             switch (AdrMode)
              {
               case ModReg8:
	        PutCode(0xc8aa+((z+1) << 12)+(((Word)AdrPart) << 8));
                CheckCPU(CPU75104);
                break;
              }
            }
           else if (strcasecmp(ArgStr[2],"XA")!=0) WrError(1350);
           else
	    {
	     PutCode(0xc0aa+((z+1) << 12)+(((Word)AdrPart) << 8));
             CheckCPU(CPU75104);
            }
           break;
         }
       }
      return;
     }

   for (z=0; z<LogOrderCount; z++)
    if (Memo(LogOrders[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
        DecodeAdr(ArgStr[1],MModReg4+MModReg8);
        switch (AdrMode)
         {
          case ModReg4:
           if (AdrPart!=0) WrError(1350);
           else
            {
             DecodeAdr(ArgStr[2],MModImm+MModInd);
             switch (AdrMode)
              {
               case ModImm:
	        PutCode(0x2099+(((Word)AdrPart & 15) << 8)+((z+1) << 12));
                CheckCPU(CPU75004);
                break;
               case ModInd:
	        if (AdrPart==1) PutCode(0x80+((z+1) << 4)); else WrError(1350);
                break;
              }
            }
           break;
          case ModReg8:
           if (AdrPart==0)
            {
             DecodeAdr(ArgStr[2],MModReg8);
             switch (AdrMode)
              {
               case ModReg8:
	        PutCode(0x88aa+(((Word)AdrPart) << 8)+((z+1) << 12));
                CheckCPU(CPU75104);
                break;
              }
            }
           else if (strcasecmp(ArgStr[2],"XA")!=0) WrError(1350);
           else
	    {
	     PutCode(0x80aa+(((Word)AdrPart) << 8)+((z+1) << 12));
             CheckCPU(CPU75104);
            }
           break;
         }
       }
      return;
     }

   if (Memo("INCS"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg4+MModReg8+MModInd+MModAbs);
       switch (AdrMode)
        {
         case ModReg4:
          PutCode(0xc0+AdrPart);
          break;
         case ModReg8:
          if ((AdrPart<1) || (Odd(AdrPart))) WrError(1350);
          else
           {
            PutCode(0x88+AdrPart); CheckCPU(CPU75104);
           }
          break;
         case ModInd:
          if (AdrPart==1)
	   {
            PutCode(0x0299); CheckCPU(CPU75004);
	   }
	  else WrError(1350);
          break;
         case ModAbs:
          BAsmCode[0]=0x82; BAsmCode[1]=AdrPart; CodeLen=2;
          break;
        }
      }
     return;
    }

   if (Memo("DECS"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg4+MModReg8);
       switch (AdrMode)
        {
         case ModReg4:
          PutCode(0xc8+AdrPart);
          break;
         case ModReg8:
          PutCode(0x68aa+(((Word)AdrPart) << 8));
          CheckCPU(CPU75104);
          break;
        }
      }
     return;
    }

   if (Memo("SKE"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       DecodeAdr(ArgStr[1],MModReg4+MModReg8+MModInd);
       switch (AdrMode)
        {
         case ModReg4:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModImm+MModInd+MModReg4);
          switch (AdrMode)
           {
            case ModReg4:
             if (HReg==0)
	      {
               PutCode(0x0899+(((Word)AdrPart) << 8)); CheckCPU(CPU75004);
              }
             else if (AdrPart==0)
	      {
               PutCode(0x0899+(((Word)HReg) << 8)); CheckCPU(CPU75004);
              }
             else WrError(1350);
             break;
            case ModImm:
	     BAsmCode[0]=0x9a; BAsmCode[1]=(AdrPart << 4)+HReg;
             CodeLen=2;
             break;
            case ModInd:
	     if ((AdrPart==1) && (HReg==0)) PutCode(0x80);
	     else WrError(1350);
             break;
           }
          break;
         case ModReg8:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModInd+MModReg8);
          switch (AdrMode)
           {
            case ModReg8:
	     if (HReg==0)
	      {
               PutCode(0x48aa+(((Word)AdrPart) << 8)); CheckCPU(CPU75104);
              }
	     else if (AdrPart==0)
	      {
               PutCode(0x48aa+(((Word)HReg) << 8)); CheckCPU(CPU75104);
              }
             else WrError(1350);
             break;
            case ModInd:
             if (AdrPart==1)
	      {
               PutCode(0x19aa); CheckCPU(CPU75104);
	      }
	     else WrError(1350);
             break;
           }
          break;
         case ModInd:
          if (AdrPart!=1) WrError(1350);
          else
           {
            MinOneIs0=true;
            DecodeAdr(ArgStr[2],MModImm+MModReg4+MModReg8);
            switch (AdrMode)
             {
              case ModImm:
               PutCode(0x6099+(((Word)AdrPart) << 8)); CheckCPU(CPU75004);
               break;
              case ModReg4:
               if (AdrPart==0) PutCode(0x80); else WrError(1350);
               break;
              case ModReg8:
               if (AdrPart==0)
	        {
                 PutCode(0x19aa); CheckCPU(CPU75004);
	        }
	       else WrError(1350);
               break;
             }
           }
          break;
        }
      }
     return;
    }

   if ((Memo("RORC")) || (Memo("NOT")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
     else if (Memo("RORC")) PutCode(0x98);
     else PutCode(0x5f99);
     return;
    }

   /* Bitoperationen */

   if (Memo("MOV1"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       OK=true;
       if (strcasecmp(ArgStr[1],"CY")==0) z=0xbd;
       else if (strcasecmp(ArgStr[2],"CY")==0) z=0x9b;
       else OK=false;
       if (! OK) WrError(1350);
       else if (DecodeBitAddr(ArgStr[((z >> 2) & 3)-1],&BVal))
        if (Hi(BVal)!=0) WrError(1350);
        else
         {
          BAsmCode[0]=z; BAsmCode[1]=BVal; CodeLen=2;
          CheckCPU(CPU75104);
         }
      }
     return;
    }

   if ((Memo("SET1")) || (Memo("CLR1")))
    {
     OK=Memo("SET1");
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"CY")==0) PutCode(0xe6+OK);
     else if (DecodeBitAddr(ArgStr[1],&BVal))
      if (Hi(BVal)!=0)
       {
        BAsmCode[0]=0x84+OK+(Hi(BVal & 0x300) << 4);
	BAsmCode[1]=Lo(BVal); CodeLen=2;
       }
      else
       {
        BAsmCode[0]=0x9c+OK; BAsmCode[1]=BVal; CodeLen=2;
       }
     return;
    }

   if ((Memo("SKT")) || (Memo("SKF")))
    {
     OK=Memo("SKT");
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"CY")==0)
      if (Memo("SKT")) PutCode(0xd7);
      else WrError(1350);
     else if (DecodeBitAddr(ArgStr[1],&BVal))
      if (Hi(BVal)!=0)
       {
        BAsmCode[0]=0x86+OK+(Hi(BVal & 0x300) << 4);
	BAsmCode[1]=Lo(BVal); CodeLen=2;
       }
      else
       {
        BAsmCode[0]=0xbe + OK; /* ANSI :-0 */
        BAsmCode[1]=BVal; CodeLen=2;
       }
     return;
    }

   if (Memo("NOT1"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"CY")!=0) WrError(1350);
     else PutCode(0xd6);
     return;
    }

   if (Memo("SKTCLR"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (DecodeBitAddr(ArgStr[1],&BVal))
      if (Hi(BVal)!=0) WrError(1350);
      else
       {
        BAsmCode[0]=0x9f; BAsmCode[1]=BVal; CodeLen=2;
       }
     return;
    }

   if (OpPart[strlen(OpPart)-1]=='1')
   for (z=0; z<LogOrderCount; z++)
    if (strncmp(LogOrders[z],OpPart,strlen(LogOrders[z]))==0)
     {
      if (ArgCnt!=2) WrError(1110);
      else if (strcasecmp(ArgStr[1],"CY")!=0) WrError(1350);
      else if (DecodeBitAddr(ArgStr[2],&BVal))
       if (Hi(BVal)!=0) WrError(1350);
       else
        {
         BAsmCode[0]=0xac+((z & 1) << 1)+((z & 2) << 3);
         BAsmCode[1]=BVal; CodeLen=2;
        }
      return;
     }

   /* Spruenge */

   if (Memo("BR"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"PCDE")==0)
      {
       PutCode(0x0499); CheckCPU(CPU75004);
      }
     else if (strcasecmp(ArgStr[1],"PCXA")==0)
      {
       BAsmCode[0]=0x99; BAsmCode[1]=0x00; CodeLen=2;
       CheckCPU(CPU75104);
      }
     else
      {
       BrRel=false; BrLong=false;
       if (*ArgStr[1]=='$')
        {
         BrRel=true; strcpy(ArgStr[1],ArgStr[1]+1);
        }
       else if (*ArgStr[1]=='!')
        {
         BrLong=true; strcpy(ArgStr[1],ArgStr[1]+1);
        }
       AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
        {
         Dist=AdrInt-EProgCounter();
         if ((BrRel) || ((Dist<=16) && (Dist>=-15) && (Dist!=0)))
          {
           if (Dist>0)
            {
             Dist--;
             if ((Dist>15) && (! SymbolQuestionable)) WrError(1370);
             else PutCode(0x00+Dist);
            }
           else
            {
             if ((Dist<-15) && (! SymbolQuestionable)) WrError(1370);
             else PutCode(0xf0+15+Dist);
            }
          }
         else if ((! BrLong) && ((AdrInt >> 12)==(EProgCounter() >> 12)) && ((EProgCounter() & 0xfff)<0xffe))
          {
	   BAsmCode[0]=0x50+((AdrInt >> 8) & 15);
           BAsmCode[1]=Lo(AdrInt);
           CodeLen=2;
          }
         else
          {
           BAsmCode[0]=0xab;
           BAsmCode[1]=Hi(AdrInt & 0x3fff);
	   BAsmCode[2]=Lo(AdrInt);
           CodeLen=3;
           CheckCPU(CPU75004);
          }
         ChkSpace(SegCode);
        }
      }
     return;
    }

   if (Memo("BRCB"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
        if ((AdrInt >> 12)!=(EProgCounter() >> 12)) WrError(1910);
	else if ((EProgCounter() & 0xfff)>=0xffe) WrError(1905);
        else
         {
	  BAsmCode[0]=0x50+((AdrInt >> 8) & 15);
          BAsmCode[1]=Lo(AdrInt);
          CodeLen=2;
          ChkSpace(SegCode);
         }
      }
     return;
    }

   if (Memo("CALL"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       if (*ArgStr[1]=='!')
        {
         strcpy(ArgStr[1],ArgStr[1]+1); BrLong=true;
        }
       else BrLong=false;
       FirstPassUnknown=false;
       AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (FirstPassUnknown) AdrInt&=0x7ff;
       if (OK)
        {
         if ((BrLong) || (AdrInt>0x7ff))
          {
           BAsmCode[0]=0xab;
           BAsmCode[1]=0x40+Hi(AdrInt & 0x3fff);
           BAsmCode[2]=Lo(AdrInt);
           CodeLen=3;
           CheckCPU(CPU75004);
          }
	 else
	  {
           BAsmCode[0]=0x40+Hi(AdrInt & 0x7ff);
           BAsmCode[1]=Lo(AdrInt);
           CodeLen=2;
	  }
         ChkSpace(SegCode);
        }
      }
     return;
    }

   if (Memo("CALLF"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       if (*ArgStr[1]=='!') strcpy(ArgStr[1],ArgStr[1]+1);
       AdrInt=EvalIntExpression(ArgStr[1],UInt11,&OK);
       if (OK)
        {
         BAsmCode[0]=0x40+Hi(AdrInt);
         BAsmCode[1]=Lo(AdrInt);
         CodeLen=2;
         ChkSpace(SegCode);
        }
      }
     return;
    }

   if (Memo("GETI"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       BAsmCode[0]=EvalIntExpression(ArgStr[1],UInt6,&OK);
       CodeLen=OK;
       CheckCPU(CPU75004);
      }
     return;
    }

   /* Steueranweisungen */

   if ((Memo("EI")) || (Memo("DI")))
    {
     OK=Memo("EI");
     if (ArgCnt==0) PutCode(0xb29c+OK);
     else if (ArgCnt!=1) WrError(1110);
     else if (DecodeIntName(ArgStr[1],&HReg)) PutCode(0x989c+OK+(((Word)HReg) << 8));
     else WrError(1440);
     return;
    }

   if (Memo("SEL"))
    {
     BAsmCode[0]=0x99;
     if (ArgCnt!=1) WrError(1110);
     else if (strncasecmp(ArgStr[1],"RB",2)==0)
      {
       BAsmCode[1]=0x20+EvalIntExpression(ArgStr[1]+2,UInt2,&OK);
       if (OK)
        {
         CodeLen=2; CheckCPU(CPU75104);
        }
      }
     else if (strncasecmp(ArgStr[1],"MB",2)==0)
      {
       BAsmCode[1]=0x10+EvalIntExpression(ArgStr[1]+2,UInt4,&OK);
       if (OK)
        {
         CodeLen=2; CheckCPU(CPU75004);
        }
      }
     else WrError(1350);
     return;
    }

   WrXError(1200,OpPart);
}

        static void InitCode_75K0(void)
{
   SaveInitProc();
   MBSValue=0; MBEValue=0;
}

        static bool ChkPC_75K0(void)
{
   switch (ActPC)
    {
     case SegCode : return (ProgCounter()<=ROMEnd);
     case SegData : return (ProgCounter()<0x1000);
     default      : return false;
    }
}

        static bool IsDef_75K0(void)
{
   return ((Memo("SFR")) || (Memo("BIT")));
}

        static void SwitchFrom_75K0(void)
{
   DeinitFields();
}

        static void SwitchTo_75K0(void)
{
   TurnWords=false; ConstMode=ConstModeIntel; SetIsOccupied=false;

   PCSymbol="PC"; HeaderID=0x7b; NOPCode=0x60;
   DivideChars=","; HasAttrs=false;

   ValidSegs=(1<<SegCode)|(1<<SegData);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;

   MakeCode=MakeCode_75K0; ChkPC=ChkPC_75K0; IsDef=IsDef_75K0;
   SwitchFrom=SwitchFrom_75K0; InitFields();
}

	void code75k0_init(void)
{
   CPU75402=AddCPU("75402",SwitchTo_75K0);
   CPU75004=AddCPU("75004",SwitchTo_75K0);
   CPU75006=AddCPU("75006",SwitchTo_75K0);
   CPU75008=AddCPU("75008",SwitchTo_75K0);
   CPU75268=AddCPU("75268",SwitchTo_75K0);
   CPU75304=AddCPU("75304",SwitchTo_75K0);
   CPU75306=AddCPU("75306",SwitchTo_75K0);
   CPU75308=AddCPU("75308",SwitchTo_75K0);
   CPU75312=AddCPU("75312",SwitchTo_75K0);
   CPU75316=AddCPU("75316",SwitchTo_75K0);
   CPU75328=AddCPU("75328",SwitchTo_75K0);
   CPU75104=AddCPU("75104",SwitchTo_75K0);
   CPU75106=AddCPU("75106",SwitchTo_75K0);
   CPU75108=AddCPU("75108",SwitchTo_75K0);
   CPU75112=AddCPU("75112",SwitchTo_75K0);
   CPU75116=AddCPU("75116",SwitchTo_75K0);
   CPU75206=AddCPU("75206",SwitchTo_75K0);
   CPU75208=AddCPU("75208",SwitchTo_75K0);
   CPU75212=AddCPU("75212",SwitchTo_75K0);
   CPU75216=AddCPU("75216",SwitchTo_75K0);
   CPU75512=AddCPU("75512",SwitchTo_75K0);
   CPU75516=AddCPU("75516",SwitchTo_75K0);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_75K0;
}

