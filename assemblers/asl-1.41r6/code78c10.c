/* code78c10.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator NEC uPD78(C)1x                                              */
/*                                                                           */
/* Historie: 29.12.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

/*---------------------------------------------------------------------------*/

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } SReg;

#define FixedOrderCnt 23
#define ALUOrderCnt 15
#define AbsOrderCnt 10
#define Reg2OrderCnt 10
#define WorkOrderCnt 4
#define EAOrderCnt 4
#define SRegCnt 28


static LongInt WorkArea;

static SimpProc SaveInitProc;

static CPUVar CPU7810,CPU78C10;

static FixedOrder *FixedOrders;
static Byte *ALUOrderCodes;
static char **ALUOrderImmOps,**ALUOrderRegOps,**ALUOrderEAOps;
static FixedOrder *AbsOrders;
static FixedOrder *Reg2Orders;
static FixedOrder *WorkOrders;
static FixedOrder *EAOrders;
static SReg *SRegs;

/*--------------------------------------------------------------------------------*/

	static void AddFixed(char *NName, Word NCode)
{
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
}

	static void AddSReg(char *NName, Word NCode)
{
   if (InstrZ>=SRegCnt) exit(255);
   SRegs[InstrZ].Name=NName;
   SRegs[InstrZ++].Code=NCode;
}

	static void AddALU(Byte NCode, char *NName1, char *NName2, char *NName3)
{
   if (InstrZ>=ALUOrderCnt) exit(255);
   ALUOrderCodes[InstrZ]=NCode;
   ALUOrderImmOps[InstrZ]=NName1;
   ALUOrderRegOps[InstrZ]=NName2;
   ALUOrderEAOps[InstrZ++]=NName3;
}

        static void AddAbs(char *NName, Word NCode)
{
   if (InstrZ>=AbsOrderCnt) exit(255);
   AbsOrders[InstrZ].Name=NName;
   AbsOrders[InstrZ++].Code=NCode;
}

        static void AddReg2(char *NName, Word NCode)
{
   if (InstrZ>=Reg2OrderCnt) exit(255);
   Reg2Orders[InstrZ].Name=NName;
   Reg2Orders[InstrZ++].Code=NCode;
}

        static void AddWork(char *NName, Word NCode)
{
   if (InstrZ>=WorkOrderCnt) exit(255);
   WorkOrders[InstrZ].Name=NName;
   WorkOrders[InstrZ++].Code=NCode;
}

        static void AddEA(char *NName, Word NCode)
{
   if (InstrZ>=EAOrderCnt) exit(255);
   EAOrders[InstrZ].Name=NName;
   EAOrders[InstrZ++].Code=NCode;
}

	static void InitFields(void)
{
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("EXX"  , 0x0011); AddFixed("EXA"  , 0x0010);
   AddFixed("EXH"  , 0x0050); AddFixed("BLOCK", 0x0031);
   AddFixed("TABLE", 0x48a8); AddFixed("DAA"  , 0x0061);
   AddFixed("STC"  , 0x482b); AddFixed("CLC"  , 0x482a);
   AddFixed("NEGA" , 0x483a); AddFixed("RLD"  , 0x4838);
   AddFixed("RRD"  , 0x4839); AddFixed("JB"   , 0x0021);
   AddFixed("JEA"  , 0x4828); AddFixed("CALB" , 0x4829);
   AddFixed("SOFTI", 0x0072); AddFixed("RET"  , 0x00b8);
   AddFixed("RETS" , 0x00b9); AddFixed("RETI" , 0x0062);
   AddFixed("NOP"  , 0x0000); AddFixed("EI"   , 0x00aa);
   AddFixed("DI"   , 0x00ba); AddFixed("HLT"  , 0x483b);
   AddFixed("STOP" , 0x48bb);

   SRegs=(SReg *) malloc(sizeof(SReg)*SRegCnt); InstrZ=0;
   AddSReg("PA"  , 0x00); AddSReg("PB"  , 0x01);
   AddSReg("PC"  , 0x02); AddSReg("PD"  , 0x03);
   AddSReg("PF"  , 0x05); AddSReg("MKH" , 0x06);
   AddSReg("MKL" , 0x07); AddSReg("ANM" , 0x08);
   AddSReg("SMH" , 0x09); AddSReg("SML" , 0x0a);
   AddSReg("EOM" , 0x0b); AddSReg("ETNM", 0x0c);
   AddSReg("TMM" , 0x0d); AddSReg("MM"  , 0x10);
   AddSReg("MCC" , 0x11); AddSReg("MA"  , 0x12);
   AddSReg("MB"  , 0x13); AddSReg("MC"  , 0x14);
   AddSReg("MF"  , 0x17); AddSReg("TXB" , 0x18);
   AddSReg("RXB" , 0x19); AddSReg("TM0" , 0x1a);
   AddSReg("TM1" , 0x1b); AddSReg("CR0" , 0x20);
   AddSReg("CR1" , 0x21); AddSReg("CR2" , 0x22);
   AddSReg("CR3" , 0x23); AddSReg("ZCM" , 0x28);

   ALUOrderCodes=(Byte *) malloc(sizeof(Byte)*ALUOrderCnt);
   ALUOrderImmOps=(char **) malloc(sizeof(char *)*ALUOrderCnt);
   ALUOrderRegOps=(char **) malloc(sizeof(char *)*ALUOrderCnt);
   ALUOrderEAOps=(char **) malloc(sizeof(char *)*ALUOrderCnt); InstrZ=0;
   AddALU(10,"ACI"  ,"ADC"  ,"DADC"  );
   AddALU( 4,"ADINC","ADDNC","DADDNC");
   AddALU( 8,"ADI"  ,"ADD"  ,"DADD"  );
   AddALU( 1,"ANI"  ,"ANA"  ,"DAN"   );
   AddALU(15,"EQI"  ,"EQA"  ,"DEQ"   );
   AddALU( 5,"GTI"  ,"GTA"  ,"DGT"   );
   AddALU( 7,"LTI"  ,"LTA"  ,"DLT"   );
   AddALU(13,"NEI"  ,"NEA"  ,"DNE"   );
   AddALU(11,"OFFI" ,"OFFA" ,"DOFF"  );
   AddALU( 9,"ONI"  ,"ONA"  ,"DON"   );
   AddALU( 3,"ORI"  ,"ORA"  ,"DOR"   );
   AddALU(14,"SBI"  ,"SBB"  ,"DSBB"  );
   AddALU( 6,"SUINB","SUBNB","DSUBNB");
   AddALU(12,"SUI"  ,"SUB"  ,"DSUB"  );
   AddALU( 2,"XRI"  ,"XRA"  ,"DXR"   );

   AbsOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*AbsOrderCnt); InstrZ=0;
   AddAbs("CALL", 0x0040); AddAbs("JMP" , 0x0054);
   AddAbs("LBCD", 0x701f); AddAbs("LDED", 0x702f);
   AddAbs("LHLD", 0x703f); AddAbs("LSPD", 0x700f);
   AddAbs("SBCD", 0x701e); AddAbs("SDED", 0x702e);
   AddAbs("SHLD", 0x703e); AddAbs("SSPD", 0x700e);

   Reg2Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*Reg2OrderCnt); InstrZ=0;
   AddReg2("DCR" , 0x0050); AddReg2("DIV" , 0x483c);
   AddReg2("INR" , 0x0040); AddReg2("MUL" , 0x482c);
   AddReg2("RLL" , 0x4834); AddReg2("RLR" , 0x4830);
   AddReg2("SLL" , 0x4824); AddReg2("SLR" , 0x4820);
   AddReg2("SLLC", 0x4804); AddReg2("SLRC", 0x4800);

   WorkOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*WorkOrderCnt); InstrZ=0;
   AddWork("DCRW", 0x33); AddWork("INRW", 0x20);
   AddWork("LDAW", 0x01); AddWork("STAW", 0x63);

   EAOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*EAOrderCnt); InstrZ=0;
   AddEA("DRLL", 0x48b4); AddEA("DRLR", 0x48b0);
   AddEA("DSLL", 0x48a4); AddEA("DSLR", 0x48a0);
}

	static void DeinitFields(void)
{
   free(FixedOrders);
   free(ALUOrderCodes); free(ALUOrderImmOps); free(ALUOrderRegOps); free(ALUOrderEAOps);
   free(AbsOrders);
   free(Reg2Orders);
   free(WorkOrders);
   free(EAOrders);
   free(SRegs);
}

/*--------------------------------------------------------------------------------*/

	static bool Decode_r(char *Asc, ShortInt *Erg)
{
   static char *Names="VABCDEHL";
   char *p;

   if (strlen(Asc)!=1) return false;
   p=strchr(Names,toupper(*Asc));
   if (p==NULL) return false;
   *Erg=p-Names; return true;;
}

	static bool Decode_r1(char *Asc, ShortInt *Erg)
{
   if (strcasecmp(Asc,"EAL")==0) *Erg=1;
   else if (strcasecmp(Asc,"EAH")==0) *Erg=0;
   else
    {
     if (! Decode_r(Asc,Erg)) return false;
     return (*Erg>1);
    }
   return true;
}

	static bool Decode_r2(char *Asc, ShortInt *Erg)
{
   if (! Decode_r(Asc,Erg)) return false;
   return ((*Erg>0) && (*Erg<4));
}

	static bool Decode_rp2(char *Asc, ShortInt *Erg)
{
#define RegCnt 5
   static char *Regs[RegCnt]={"SP","B","D","H","EA"};

   for (*Erg=0; *Erg<RegCnt; (*Erg)++)
    if (strcasecmp(Asc,Regs[*Erg])==0) break;
   return (*Erg<RegCnt);
}

	static bool Decode_rp(char *Asc, ShortInt *Erg)
{
   if (! Decode_rp2(Asc,Erg)) return false;
   return (*Erg<4);
}

	static bool Decode_rp1(char *Asc, ShortInt *Erg)
{
   if (strcasecmp(Asc,"VA")==0) *Erg=0;
   else
    {
     if (! Decode_rp2(Asc,Erg)) return false;
     return (*Erg!=0);
    }
   return true;
}

	static bool Decode_rp3(char *Asc, ShortInt *Erg)
{
   if (! Decode_rp2(Asc,Erg)) return false;
   return ((*Erg<4) && (*Erg>0));
}

	static bool Decode_rpa2(char *Asc, ShortInt *Erg, ShortInt *Disp)
{
#define OpCnt 13
   static char *OpNames[OpCnt]={"B","D","H","D+","H+","D-","H-",
				   "H+A","A+H","H+B","B+H","H+EA","EA+H"};
   static Byte OpCodes[OpCnt]={1,2,3,4,5,6,7,12,12,13,13,14,14};

   Integer z;
   char *p,*pm;
   bool OK;

   for (z=0; z<OpCnt; z++)
    if (strcasecmp(Asc,OpNames[z])==0)
     {
      *Erg=OpCodes[z]; return true;
     }

   p=QuotPos(Asc,'+'); pm=QuotPos(Asc,'-');
   if ((p==NULL) || ((pm!=NULL) && (pm<p))) p=pm;
   if (p==NULL) return false;

   if (p==Asc+1)
    switch (toupper(*Asc))
     {
      case 'H': *Erg=15; break;
      case 'D': *Erg=11; break;
      default: return false;
     }
   else return false;
   *Disp=EvalIntExpression(p,SInt8,&OK);
   return OK;
}

	static bool Decode_rpa(char *Asc, ShortInt *Erg)
{
   ShortInt Dummy;

   if (! Decode_rpa2(Asc,Erg,&Dummy)) return false;
   return (*Erg<=7);
}

	static bool Decode_rpa1(char *Asc, ShortInt *Erg)
{
   ShortInt Dummy;

   if (! Decode_rpa2(Asc,Erg,&Dummy)) return false;
   return (*Erg<=3);
}

	static bool Decode_rpa3(char *Asc, ShortInt *Erg, ShortInt *Disp)
{
   if (strcasecmp(Asc,"D++")==0) *Erg=4;
   else if (strcasecmp(Asc,"H++")==0) *Erg=5;
   else
    {
     if (! Decode_rpa2(Asc,Erg,Disp)) return false;
     return ((*Erg==2) || (*Erg==3) || (*Erg>=8));
    }
   return true;
}

	static bool Decode_f(char *Asc, ShortInt *Erg)
{
#define FlagCnt 3
   static char *Flags[FlagCnt]={"CY","HC","Z"};

   for (*Erg=0; *Erg<FlagCnt; (*Erg)++)
    if (strcasecmp(Flags[*Erg],Asc)==0) break;
   *Erg+=2; return (*Erg<=4);
}

	static bool Decode_sr0(char *Asc, ShortInt *Erg)
{
   Integer z;

   for (z=0; z<SRegCnt; z++)
    if (strcasecmp(Asc,SRegs[z].Name)==0) break;
   if ((z==SRegCnt-1) && (MomCPU==CPU7810))
    {
     WrError(1440); return false;
    }
   if (z<SRegCnt)
    {
     *Erg=SRegs[z].Code; return true;
    }
   else return false;
}

	static bool Decode_sr1(char *Asc, ShortInt *Erg)
{
   if (! Decode_sr0(Asc,Erg)) return false;
   return (((*Erg>=0) && (*Erg<=9)) || (*Erg==11) || (*Erg==13) || (*Erg==25) || ((*Erg>=32) && (*Erg<=35)));
}

	static bool Decode_sr(char *Asc, ShortInt *Erg)
{
   if (! Decode_sr0(Asc,Erg)) return false;
   return (((*Erg>=0) && (*Erg<=24)) || (*Erg==26) || (*Erg==27) || (*Erg==40));
}

	static bool Decode_sr2(char *Asc, ShortInt *Erg)
{
   if (! Decode_sr0(Asc,Erg)) return false;
   return (((*Erg>=0) && (*Erg<=9)) || (*Erg==11) || (*Erg==13));
}

        static bool Decode_sr3(char *Asc, ShortInt *Erg)
{
   if (strcasecmp(Asc,"ETM0")==0) *Erg=0;
   else if (strcasecmp(Asc,"ETM1")==0) *Erg=1;
   else return false;
   return true;
}

        static bool Decode_sr4(char *Asc, ShortInt *Erg)
{
   if (strcasecmp(Asc,"ECNT")==0) *Erg=0;
   else if (strcasecmp(Asc,"ECPT")==0) *Erg=1;
   else return false;
   return true;
}

	static bool Decode_irf(char *Asc, ShortInt *Erg)
{
#undef FlagCnt
#define FlagCnt 18
   static char *FlagNames[FlagCnt]=
	     {"NMI" ,"FT0" ,"FT1" ,"F1"  ,"F2"  ,"FE0" ,
	      "FE1" ,"FEIN","FAD" ,"FSR" ,"FST" ,"ER"  ,
	      "OV"  ,"AN4" ,"AN5" ,"AN6" ,"AN7" ,"SB"   };
   static ShortInt FlagCodes[FlagCnt]=
	     {0,1,2,3,4,5,6,7,8,9,10,11,12,16,17,18,19,20};

   for (*Erg=0; *Erg<FlagCnt; (*Erg)++)
    if (strcasecmp(FlagNames[*Erg],Asc)==0) break;
   if (*Erg>=FlagCnt) return false;
   *Erg=FlagCodes[*Erg];
   return true;
}

	static bool Decode_wa(char *Asc, Byte *Erg)
{
   Word Adr;
   bool OK;

   FirstPassUnknown=false;
   Adr=EvalIntExpression(Asc,Int16,&OK); if (! OK) return false;
   if ((FirstPassUnknown) && (Hi(Adr)!=WorkArea)) WrError(110);
   *Erg=Lo(Adr);
   return true;
}

	static bool HasDisp(ShortInt Mode)
{
   return ((Mode & 11)==11);
}

/*--------------------------------------------------------------------------*/

	static bool DecodePseudo(void)
{
#define ASSUME78C10Count 1
static ASSUMERec ASSUME78C10s[ASSUME78C10Count]=
		 {{"V" , &WorkArea, 0, 0xff, 0x100}};

   if (Memo("ASSUME"))
    {
     CodeASSUME(ASSUME78C10s,ASSUME78C10Count);
     return true;
    }

   return false;
}

	static void MakeCode_78C10(void)
{
   Integer z,AdrInt;
   ShortInt HVal8,HReg;
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
      else if ((Memo("STOP")) && (MomCPU==CPU7810)) WrError(1500);
      else
       {
        CodeLen=0;
        if (Hi(FixedOrders[z].Code)!=0) BAsmCode[CodeLen++]=Hi(FixedOrders[z].Code);
        BAsmCode[CodeLen++]=Lo(FixedOrders[z].Code);
       }
      return;
     }

   if (Memo("MOV"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")==0)
      if (Decode_sr1(ArgStr[2],&HReg))
       {
	CodeLen=2; BAsmCode[0]=0x4c;
	BAsmCode[1]=0xc0+HReg;
       }
      else if (Decode_r1(ArgStr[2],&HReg))
       {
	CodeLen=1; BAsmCode[0]=0x08+HReg;
       }
      else WrError(1350);
     else if (strcasecmp(ArgStr[2],"A")==0)
      if (Decode_sr(ArgStr[1],&HReg))
       {
	CodeLen=2; BAsmCode[0]=0x4d;
	BAsmCode[1]=0xc0+HReg;
       }
      else if (Decode_r1(ArgStr[1],&HReg))
       {
	CodeLen=1; BAsmCode[0]=0x18+HReg;
       }
      else WrError(1350);
     else if (Decode_r(ArgStr[1],&HReg))
      {
       AdrInt=EvalIntExpression(ArgStr[2],Int16,&OK);
       if (OK)
	{
	 CodeLen=4; BAsmCode[0]=0x70; BAsmCode[1]=0x68+HReg;
	 BAsmCode[2]=Lo(AdrInt); BAsmCode[3]=Hi(AdrInt);
	}
      }
     else if (Decode_r(ArgStr[2],&HReg))
      {
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
	{
	 CodeLen=4; BAsmCode[0]=0x70; BAsmCode[1]=0x78+HReg;
	 BAsmCode[2]=Lo(AdrInt); BAsmCode[3]=Hi(AdrInt);
	}
      }
     else WrError(1350);
     return;
    }

   if (Memo("MVI"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       BAsmCode[1]=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
	if (Decode_r(ArgStr[1],&HReg))
	 {
	  CodeLen=2; BAsmCode[0]=0x68+HReg;
	 }
	else if (Decode_sr2(ArgStr[1],&HReg))
	 {
	  CodeLen=3; BAsmCode[2]=BAsmCode[1]; BAsmCode[0]=0x64;
	  BAsmCode[1]=(HReg & 7)+((HReg & 8) << 4);
	 }
	else WrError(1350);
      }
     return;
    }

   if (Memo("MVIW"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (Decode_wa(ArgStr[1],BAsmCode+1))
      {
       BAsmCode[2]=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
	{
	 CodeLen=3; BAsmCode[0]=0x71;
	}
      }
     return;
    }

   if (Memo("MVIX"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (! Decode_rpa1(ArgStr[1],&HReg)) WrError(1350);
     else
      {
       BAsmCode[1]=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
	{
	 BAsmCode[0]=0x48+HReg; CodeLen=2;
	}
      }
     return;
    }

   if ((Memo("LDAX")) || (Memo("STAX")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (! Decode_rpa2(ArgStr[1],&HReg,(ShortInt *) BAsmCode+1)) WrError(1350);
     else
      {
       CodeLen=1+HasDisp(HReg);
       BAsmCode[0]=0x28+(Memo("STAX") << 4)+((HReg & 8) << 4)+(HReg & 7);
      }
     return;
    }

   if ((Memo("LDEAX")) || (Memo("STEAX")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (! Decode_rpa3(ArgStr[1],&HReg,(ShortInt *) BAsmCode+2)) WrError(1350);
     else
      {
       CodeLen=2+HasDisp(HReg); BAsmCode[0]=0x48;
       BAsmCode[1]=0x80+(Memo("STEAX") << 4)+HReg;
      }
     return;
    }

   if (Memo("LXI"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (! Decode_rp2(ArgStr[1],&HReg)) WrError(1350);
     else
      {
       AdrInt=EvalIntExpression(ArgStr[2],Int16,&OK);
       if (OK)
	{
	 CodeLen=3; BAsmCode[0]=0x04+(HReg << 4);
	 BAsmCode[1]=Lo(AdrInt); BAsmCode[2]=Hi(AdrInt);
	}
      }
     return;
    }

   if ((Memo("PUSH")) || (Memo("POP")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (! Decode_rp1(ArgStr[1],&HReg)) WrError(1350);
     else
      {
       CodeLen=1;
       BAsmCode[0]=0xa0+(Memo("PUSH") << 4)+HReg;
      }
     return;
    }

   if (Memo("DMOV"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       if (strcasecmp(ArgStr[1],"EA")!=0)
        {
         strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
         OK=true;
	}
       else OK=false;
       if (strcasecmp(ArgStr[1],"EA")!=0) WrError(1350);
       else if (Decode_rp3(ArgStr[2],&HReg))
        {
         CodeLen=1; BAsmCode[0]=0xa4+HReg;
         if (OK) BAsmCode[0]+=0x10;
        }
       else if (((OK) && (Decode_sr3(ArgStr[2],&HReg)))
             || ((! OK) && (Decode_sr4(ArgStr[2],&HReg))))
        {
         CodeLen=2; BAsmCode[0]=0x48; BAsmCode[1]=0xc0+HReg;
         if (OK) BAsmCode[1]+=0x12;
        }
       else WrError(1350);
      }
     return;
    }

   for (z=0; z<ALUOrderCnt; z++)
    if (Memo(ALUOrderImmOps[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
	HVal8=EvalIntExpression(ArgStr[2],Int8,&OK);
	if (OK)
	 if (strcasecmp(ArgStr[1],"A")==0)
	  {
	   CodeLen=2;
	   BAsmCode[0]=0x06+((ALUOrderCodes[z] & 14) << 3)+(ALUOrderCodes[z] & 1);
	   BAsmCode[1]=HVal8;
	  }
	 else if (Decode_r(ArgStr[1],&HReg))
	  {
	   CodeLen=3; BAsmCode[0]=0x74; BAsmCode[2]=HVal8;
	   BAsmCode[1]=HReg+(ALUOrderCodes[z] << 3);
	  }
	 else if (Decode_sr2(ArgStr[1],&HReg))
	  {
	   CodeLen=3; BAsmCode[0]=0x64; BAsmCode[2]=HVal8;
	   BAsmCode[1]=(HReg & 7)+(ALUOrderCodes[z] << 3)+((HReg & 8) << 4);
	  }
	 else WrError(1350);
       }
      return;
     }
    else if (Memo(ALUOrderRegOps[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else
       {
	if (strcasecmp(ArgStr[1],"A")!=0)
	 {
	  strcpy(ArgStr[3],ArgStr[1]);
          strcpy(ArgStr[1],ArgStr[2]);
          strcpy(ArgStr[2],ArgStr[3]);
	  OK=false;
	 }
	else OK=true;
	if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
	else if (! Decode_r(ArgStr[2],&HReg)) WrError(1350);
	else
	 {
	  CodeLen=2; BAsmCode[0]=0x60;
	  BAsmCode[1]=(ALUOrderCodes[z] << 3)+HReg;
	  if ((OK) || (Memo("ONA")) || (Memo("OFFA")))
	   BAsmCode[1]+=0x80;
	 }
       }
      return;
     }
    else if ((OpPart[strlen(OpPart)-1]=='W') && (strncmp(ALUOrderRegOps[z],OpPart,strlen(ALUOrderRegOps[z]))==0))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (Decode_wa(ArgStr[1],BAsmCode+2))
       {
	CodeLen=3; BAsmCode[0]=0x74;
	BAsmCode[1]=0x80+(ALUOrderCodes[z] << 3);
       }
      return;
     }
    else if ((OpPart[strlen(OpPart)-1]=='X') && (strncmp(ALUOrderRegOps[z],OpPart,strlen(ALUOrderRegOps[z]))==0))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (! Decode_rpa(ArgStr[1],&HReg)) WrError(1350);
      else
       {
	CodeLen=2; BAsmCode[0]=0x70;
	BAsmCode[1]=0x80+(ALUOrderCodes[z] << 3)+HReg;
       }
      return;
     }
    else if (Memo(ALUOrderEAOps[z]))
     {
      if (ArgCnt!=2) WrError(1110);
      else if (strcasecmp(ArgStr[1],"EA")!=0) WrError(1350);
      else if (! Decode_rp3(ArgStr[2],&HReg)) WrError(1350);
      else
       {
	CodeLen=2; BAsmCode[0]=0x74;
	BAsmCode[1]=0x84+(ALUOrderCodes[z] << 3)+HReg;
       }
      return;
     }
    else if (((OpPart[strlen(OpPart)-1]=='W') && (strncmp(ALUOrderImmOps[z],OpPart,strlen(ALUOrderImmOps[z]))==0)) && (Odd(ALUOrderCodes[z])))
     {
      if (ArgCnt!=2) WrError(1110);
      else if (Decode_wa(ArgStr[1],BAsmCode+1))
       {
	BAsmCode[2]=EvalIntExpression(ArgStr[2],Int8,&OK);
	if (OK)
	 {
	  CodeLen=3;
	  BAsmCode[0]=0x05+((ALUOrderCodes[z] >> 1) << 4);
	 }
       }
      return;
     }

   for (z=0; z<AbsOrderCnt; z++)
    if (Memo(AbsOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else
       {
        AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
        if (OK)
         {
          CodeLen=0;
          if (Hi(AbsOrders[z].Code)!=0) BAsmCode[CodeLen++]=Hi(AbsOrders[z].Code);
          BAsmCode[CodeLen++]=Lo(AbsOrders[z].Code);
          BAsmCode[CodeLen++]=Lo(AdrInt);
          BAsmCode[CodeLen++]=Hi(AdrInt);
         }
       }
      return;
     }

   for (z=0; z<Reg2OrderCnt; z++)
    if (Memo(Reg2Orders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (! Decode_r2(ArgStr[1],&HReg)) WrError(1350);
      else
       {
        CodeLen=0;
        if (Hi(Reg2Orders[z].Code)!=0) BAsmCode[CodeLen++]=Hi(Reg2Orders[z].Code);
        BAsmCode[CodeLen++]=Lo(Reg2Orders[z].Code)+HReg;
       }
      return;
     }

   for (z=0; z<WorkOrderCnt; z++)
    if (Memo(WorkOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (Decode_wa(ArgStr[1],BAsmCode+1))
       {
        CodeLen=2; BAsmCode[0]=WorkOrders[z].Code;
       }
      return;
     }

   if ((Memo("DCX")) || (Memo("INX")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"EA")==0)
      {
       CodeLen=1; BAsmCode[0]=0xa8+Memo("DCX");
      }
     else if (Decode_rp(ArgStr[1],&HReg))
      {
       CodeLen=1; BAsmCode[0]=0x02+Memo("DCX")+(HReg << 4);
      }
     else WrError(1350);
     return;
    }

   if ((Memo("EADD")) || (Memo("ESUB")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"EA")!=0) WrError(1350);
     else if (! Decode_r2(ArgStr[2],&HReg)) WrError(1350);
     else
      {
       CodeLen=2; BAsmCode[0]=0x70;
       BAsmCode[1]=0x40+(Memo("ESUB") << 5)+HReg;
      }
     return;
    }

   if ((Memo("JR")) || (Memo("JRE")))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK)-(EProgCounter()+1+Memo("JRE"));
       if (OK)
	if (Memo("JR"))
	 if ((! SymbolQuestionable) && ((AdrInt<-32) || (AdrInt>31))) WrError(1370);
	 else
	  {
	   CodeLen=1; BAsmCode[0]=0xc0+(AdrInt & 0x3f);
	  }
	else
	 if ((! SymbolQuestionable) && ((AdrInt<-256) || (AdrInt>255))) WrError(1370);
	 else
	  {
	   if ((AdrInt>=-32) && (AdrInt<=31)) WrError(20);
	   CodeLen=2; BAsmCode[0]=0x4e + (Hi(AdrInt) & 1); /* ANSI :-O */
 	   BAsmCode[1]=Lo(AdrInt);
	  }
      }
     return;
    }

   if (Memo("CALF"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       FirstPassUnknown=false;
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
	if ((! FirstPassUnknown) && ((AdrInt >> 11)!=1)) WrError(1905);
	else
	 {
	  CodeLen=2;
	  BAsmCode[0]=Hi(AdrInt)+0x70; BAsmCode[1]=Lo(AdrInt);
	 }
      }
     return;
    }

   if (Memo("CALT"))
    {
     if (ArgCnt!=1) WrError(1110);
     else
      {
       FirstPassUnknown=false;
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
	if ((! FirstPassUnknown) && ((AdrInt & 0xffc1)!=0x80)) WrError(1905);
	else
	 {
	  CodeLen=1;
	  BAsmCode[0]=0x80+((AdrInt & 0x3f) >> 1);
	 }
      }
     return;
    }

   if (Memo("BIT"))
    {
     if (ArgCnt!=2) WrError(1110);
     else
      {
       HReg=EvalIntExpression(ArgStr[1],UInt3,&OK);
       if (OK)
	if (Decode_wa(ArgStr[2],BAsmCode+1))
	 {
	  CodeLen=2; BAsmCode[0]=0x58+HReg;
	 }
      }
     return;
    }

   for (z=0; z<EAOrderCnt; z++)
    if (Memo(EAOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (strcasecmp(ArgStr[1],"EA")!=0) WrError(1350);
      else
       {
        CodeLen=2; BAsmCode[0]=Hi(EAOrders[z].Code); BAsmCode[1]=Lo(EAOrders[z].Code);
       }
      return;
     }

   if ((Memo("SK")) || (Memo("SKN")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (! Decode_f(ArgStr[1],&HReg)) WrError(1350);
     else
      {
       CodeLen=2; BAsmCode[0]=0x48;
       BAsmCode[1]=0x08+(Memo("SKN") << 4)+HReg;
      }
     return;
    }

   if ((Memo("SKIT")) || (Memo("SKINT")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (! Decode_irf(ArgStr[1],&HReg)) WrError(1350);
     else
      {
       CodeLen=2; BAsmCode[0]=0x48;
       BAsmCode[1]=0x40+(Memo("SKINT") << 5)+HReg;
      }
     return;
    }

   WrXError(1200,OpPart);
}

	static void InitCode_78C10(void)
{
   SaveInitProc();
   WorkArea=0x100;
}

	static bool ChkPC_78C10(void)
{
   switch (ActPC)
    {
     case SegCode: return ProgCounter()<0x10000;
     default: return false;
    }
}

	static bool IsDef_78C10(void)
{
   return false;
}

        static void SwitchFrom_78C10(void)
{
   DeinitFields();
}

	static void SwitchTo_78C10(void)
{
   TurnWords=false; ConstMode=ConstModeIntel; SetIsOccupied=false;

   PCSymbol="$"; HeaderID=0x7a; NOPCode=0x00;
   DivideChars=","; HasAttrs=false;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_78C10; ChkPC=ChkPC_78C10; IsDef=IsDef_78C10;
   SwitchFrom=SwitchFrom_78C10; InitFields();
}

	void code78c10_init(void)
{
   CPU7810 =AddCPU("7810" ,SwitchTo_78C10);
   CPU78C10=AddCPU("78C10",SwitchTo_78C10);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_78C10;
}
