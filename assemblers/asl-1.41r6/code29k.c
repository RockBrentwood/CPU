/* code29k.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator AM29xxx-Familie                                             */
/*                                                                           */
/* Historie: 18.11.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "stringutil.h"
#include "bpemu.h"
#include "stringlists.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct
         {
          char *Name;
          bool MustSup;
          CPUVar MinCPU;
          LongWord Code;
         } StdOrder;

typedef struct
         {
          char *Name;
          bool HasReg,HasInd;
          CPUVar MinCPU;
          LongWord Code;
         } JmpOrder;

typedef struct
         {
          char *Name;
          LongWord Code;
         } SPReg;

#define StdOrderCount 51
#define NoImmOrderCount 22
#define VecOrderCount 10
#define JmpOrderCount 5
#define FixedOrderCount 2
#define MemOrderCount 7
#define SPRegCount 28

static StdOrder *StdOrders;
static StdOrder *NoImmOrders;
static StdOrder *VecOrders;
static JmpOrder *JmpOrders;
static StdOrder *FixedOrders;
static StdOrder *MemOrders;
static SPReg *SPRegs;


static CPUVar CPU29000,CPU29240,CPU29243,CPU29245;
static LongInt Reg_RBP;
static StringList Emulations;
static SimpProc SaveInitProc;

/*-------------------------------------------------------------------------*/

        static void AddStd(char *NName, CPUVar NMin, bool NSup, LongWord NCode)
{
   if (InstrZ>=StdOrderCount) exit(255);
   StdOrders[InstrZ].Name=NName;
   StdOrders[InstrZ].Code=NCode;
   StdOrders[InstrZ].MustSup=NSup;
   StdOrders[InstrZ++].MinCPU=NMin;
}

        static void AddNoImm(char *NName, CPUVar NMin, bool NSup, LongWord NCode)
{
   if (InstrZ>=NoImmOrderCount) exit(255);
   NoImmOrders[InstrZ].Name=NName;
   NoImmOrders[InstrZ].Code=NCode;
   NoImmOrders[InstrZ].MustSup=NSup;
   NoImmOrders[InstrZ++].MinCPU=NMin;
}

        static void AddVec(char *NName, CPUVar NMin, bool NSup, LongWord NCode)
{
   if (InstrZ>=VecOrderCount) exit(255);
   VecOrders[InstrZ].Name=NName;
   VecOrders[InstrZ].Code=NCode;
   VecOrders[InstrZ].MustSup=NSup;
   VecOrders[InstrZ++].MinCPU=NMin;
}

        static void AddJmp(char *NName, CPUVar NMin, bool NHas, bool NInd, LongWord NCode)
{
   if (InstrZ>=JmpOrderCount) exit(255);
   JmpOrders[InstrZ].Name=NName;
   JmpOrders[InstrZ].HasReg=NHas;
   JmpOrders[InstrZ].HasInd=NInd;
   JmpOrders[InstrZ].Code=NCode;
   JmpOrders[InstrZ++].MinCPU=NMin;
}

        static void AddFixed(char *NName, CPUVar NMin, bool NSup, LongWord NCode)
{
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].Code=NCode;
   FixedOrders[InstrZ].MustSup=NSup;
   FixedOrders[InstrZ++].MinCPU=NMin;
}

        static void AddMem(char *NName, CPUVar NMin, bool NSup, LongWord NCode)
{
   if (InstrZ>=MemOrderCount) exit(255);
   MemOrders[InstrZ].Name=NName;
   MemOrders[InstrZ].Code=NCode;
   MemOrders[InstrZ].MustSup=NSup;
   MemOrders[InstrZ++].MinCPU=NMin;
}

	static void AddSP(char *NName, LongWord NCode)
{
   if (InstrZ>=SPRegCount) exit(255);
   SPRegs[InstrZ].Name=NName;
   SPRegs[InstrZ++].Code=NCode;
}

	static void InitFields(void)
{
   StdOrders=(StdOrder *) malloc(sizeof(StdOrder)*StdOrderCount); InstrZ=0;
   AddStd("ADD"    ,CPU29245,false,0x14); AddStd("ADDC"   ,CPU29245,false,0x1c);
   AddStd("ADDCS"  ,CPU29245,false,0x18); AddStd("ADDCU"  ,CPU29245,false,0x1a);
   AddStd("ADDS"   ,CPU29245,false,0x10); AddStd("ADDU"   ,CPU29245,false,0x12);
   AddStd("AND"    ,CPU29245,false,0x90); AddStd("ANDN"   ,CPU29245,false,0x9c);
   AddStd("CPBYTE" ,CPU29245,false,0x2e); AddStd("CPEQ"   ,CPU29245,false,0x60);
   AddStd("CPGE"   ,CPU29245,false,0x4c); AddStd("CPGEU"  ,CPU29245,false,0x4e);
   AddStd("CPGT"   ,CPU29245,false,0x48); AddStd("CPGTU"  ,CPU29245,false,0x4a);
   AddStd("CPLE"   ,CPU29245,false,0x44); AddStd("CPLEU"  ,CPU29245,false,0x46);
   AddStd("CPLT"   ,CPU29245,false,0x40); AddStd("CPLTU"  ,CPU29245,false,0x42);
   AddStd("CPNEQ"  ,CPU29245,false,0x62); AddStd("DIV"    ,CPU29245,false,0x6a);
   AddStd("DIV0"   ,CPU29245,false,0x68); AddStd("DIVL"   ,CPU29245,false,0x6c);
   AddStd("DIVREM" ,CPU29245,false,0x6e); AddStd("EXBYTE" ,CPU29245,false,0x0a);
   AddStd("EXHW"   ,CPU29245,false,0x7c); AddStd("EXTRACT",CPU29245,false,0x7a);
   AddStd("INBYTE" ,CPU29245,false,0x0c); AddStd("INHW"   ,CPU29245,false,0x78);
   AddStd("MUL"    ,CPU29245,false,0x64); AddStd("MULL"   ,CPU29245,false,0x66);
   AddStd("MULU"   ,CPU29245,false,0x74); AddStd("NAND"   ,CPU29245,false,0x9a);
   AddStd("NOR"    ,CPU29245,false,0x98); AddStd("OR"     ,CPU29245,false,0x92);
   AddStd("SLL"    ,CPU29245,false,0x80); AddStd("SRA"    ,CPU29245,false,0x86);
   AddStd("SRL"    ,CPU29245,false,0x82); AddStd("SUB"    ,CPU29245,false,0x24);
   AddStd("SUBC"   ,CPU29245,false,0x2c); AddStd("SUBCS"  ,CPU29245,false,0x28);
   AddStd("SUBCU"  ,CPU29245,false,0x2a); AddStd("SUBR"   ,CPU29245,false,0x34);
   AddStd("SUBRC"  ,CPU29245,false,0x3c); AddStd("SUBRCS" ,CPU29245,false,0x38);
   AddStd("SUBRCU" ,CPU29245,false,0x3a); AddStd("SUBRS"  ,CPU29245,false,0x30);
   AddStd("SUBRU"  ,CPU29245,false,0x32); AddStd("SUBS"   ,CPU29245,false,0x20);
   AddStd("SUBU"   ,CPU29245,false,0x22); AddStd("XNOR"   ,CPU29245,false,0x96);
   AddStd("XOR"    ,CPU29245,false,0x94);

   NoImmOrders=(StdOrder *) malloc(sizeof(StdOrder)*NoImmOrderCount); InstrZ=0;
   AddNoImm("DADD"    ,CPU29000,false,0xf1); AddNoImm("DDIV"    ,CPU29000,false,0xf7);
   AddNoImm("DEQ"     ,CPU29000,false,0xeb); AddNoImm("DGE"     ,CPU29000,false,0xef);
   AddNoImm("DGT"     ,CPU29000,false,0xed); AddNoImm("DIVIDE"  ,CPU29000,false,0xe1);
   AddNoImm("DIVIDU"  ,CPU29000,false,0xe3); AddNoImm("DMUL"    ,CPU29000,false,0xf5);
   AddNoImm("DSUB"    ,CPU29000,false,0xf3); AddNoImm("FADD"    ,CPU29000,false,0xf0);
   AddNoImm("FDIV"    ,CPU29000,false,0xf6); AddNoImm("FDMUL"   ,CPU29000,false,0xf9);
   AddNoImm("FEQ"     ,CPU29000,false,0xea); AddNoImm("FGE"     ,CPU29000,false,0xee);
   AddNoImm("FGT"     ,CPU29000,false,0xec); AddNoImm("FMUL"    ,CPU29000,false,0xf4);
   AddNoImm("FSUB"    ,CPU29000,false,0xf2); AddNoImm("MULTIPLU",CPU29243,false,0xe2);
   AddNoImm("MULTIPLY",CPU29243,false,0xe0); AddNoImm("MULTM"   ,CPU29243,false,0xde);
   AddNoImm("MULTMU"  ,CPU29243,false,0xdf); AddNoImm("SETIP"   ,CPU29245,false,0x9e);

   VecOrders=(StdOrder *) malloc(sizeof(StdOrder)*VecOrderCount); InstrZ=0;
   AddVec("ASEQ"   ,CPU29245,false,0x70); AddVec("ASGE"   ,CPU29245,false,0x5c);
   AddVec("ASGEU"  ,CPU29245,false,0x5e); AddVec("ASGT"   ,CPU29245,false,0x58);
   AddVec("ASGTU"  ,CPU29245,false,0x5a); AddVec("ASLE"   ,CPU29245,false,0x54);
   AddVec("ASLEU"  ,CPU29245,false,0x56); AddVec("ASLT"   ,CPU29245,false,0x50);
   AddVec("ASLTU"  ,CPU29245,false,0x52); AddVec("ASNEQ"  ,CPU29245,false,0x72);

   JmpOrders=(JmpOrder *) malloc(sizeof(JmpOrder)*JmpOrderCount); InstrZ=0;
   AddJmp("CALL"   ,CPU29245,true ,true ,0xa8); AddJmp("JMP"    ,CPU29245,false,true ,0xa0);
   AddJmp("JMPF"   ,CPU29245,true ,true ,0xa4); AddJmp("JMPFDEC",CPU29245,true ,false,0xb4);
   AddJmp("JMPT"   ,CPU29245,true ,true ,0xac);

   FixedOrders=(StdOrder *) malloc(sizeof(StdOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("HALT"   ,CPU29245,true,0x89); AddFixed("IRET"   ,CPU29245,true,0x88);

   MemOrders=(StdOrder *) malloc(sizeof(StdOrder)*MemOrderCount); InstrZ=0;
   AddMem("LOAD"   ,CPU29245,false,0x16); AddMem("LOADL"  ,CPU29245,false,0x06);
   AddMem("LOADM"  ,CPU29245,false,0x36); AddMem("LOADSET",CPU29245,false,0x26);
   AddMem("STORE"  ,CPU29245,false,0x1e); AddMem("STOREL" ,CPU29245,false,0x0e);
   AddMem("STOREM" ,CPU29245,false,0x3e);

   SPRegs=(SPReg *) malloc(sizeof(SPReg)*SPRegCount); InstrZ=0;
   AddSP("VAB",   0);
   AddSP("OPS",   1);
   AddSP("CPS",   2);
   AddSP("CFG",   3);
   AddSP("CHA",   4);
   AddSP("CHD",   5);
   AddSP("CHC",   6);
   AddSP("RBP",   7);
   AddSP("TMC",   8);
   AddSP("TMR",   9);
   AddSP("PC0",  10);
   AddSP("PC1",  11);
   AddSP("PC2",  12);
   AddSP("MMU",  13);
   AddSP("LRU",  14);
   AddSP("CIR",  29);
   AddSP("CDR",  30);
   AddSP("IPC", 128);
   AddSP("IPA", 129);
   AddSP("IPB", 130);
   AddSP("Q",   131);
   AddSP("ALU", 132);
   AddSP("BP",  133);
   AddSP("FC",  134);
   AddSP("CR",  135);
   AddSP("FPE", 160);
   AddSP("INTE",161);
   AddSP("FPS", 162);
}

	static void DeinitFields(void)
{
   free(StdOrders);
   free(NoImmOrders);
   free(VecOrders);
   free(JmpOrders);
   free(FixedOrders);
   free(MemOrders);
   free(SPRegs);
}

/*-------------------------------------------------------------------------*/

	static void ChkSup(void)
{
   if (! SupAllowed) WrError(50);
}

	static bool IsSup(LongWord RegNo)
{
   return ((RegNo<0x80) || (RegNo>=0xa0));
}

	static bool ChkCPU(CPUVar Min)
{
   if (MomCPU>=Min) return true;
   else return (StringListPresent(Emulations,OpPart));
}

/*-------------------------------------------------------------------------*/

	static bool DecodeReg(char *Asc, LongWord *Erg)
{
   bool io,OK;

   if ((strlen(Asc)>=2) && (toupper(*Asc)=='R'))
    {
     *Erg=ConstLongInt(Asc+1,&io);
     OK=((io) && (*Erg<=255));
    }
   else if ((strlen(Asc)>=3) && (toupper(*Asc)=='G') && (toupper(Asc[1])=='R'))
    {
     *Erg=ConstLongInt(Asc+2,&io);
     OK=((io) && (*Erg<=127));
    }
   else if ((strlen(Asc)>=3) && (toupper(*Asc)=='L') && (toupper(Asc[1])=='R'))
    {
     *Erg=ConstLongInt(Asc+2,&io);
     OK=((io) && (*Erg<=127));
     *Erg+=128;
    }
   else OK=false;
   if (OK)
    if ((*Erg<127) && (Odd(Reg_RBP >> ((*Erg) >> 4)))) ChkSup();
   return OK;
}

	static bool DecodeSpReg(char *Asc_O, LongWord *Erg)
{
   int z;
   String Asc;

   strmaxcpy(Asc,Asc_O,255); NLS_UpString(Asc);
   for (z=0; z<SPRegCount; z++)
    if (strcmp(Asc,SPRegs[z].Name)==0)
     {
      *Erg=SPRegs[z].Code;
      break;
     }
   return (z<SPRegCount);
}

/*-------------------------------------------------------------------------*/

	static bool DecodePseudo(void)
{
#define ONOFF29KCount 1
   static ONOFFRec ONOFF29Ks[ONOFF29KCount]=
             {{"SUPMODE", &SupAllowed, SupAllowedName}};
#define ASSUME29KCount 1
   static ASSUMERec ASSUME29Ks[ASSUME29KCount]=
             {{"RBP", &Reg_RBP, 0, 0xff, 0x00000000}};

   Integer z;

   if (CodeONOFF(ONOFF29Ks,ONOFF29KCount)) return true;

   if (Memo("ASSUME"))
    {
     CodeASSUME(ASSUME29Ks,ASSUME29KCount);
     return true;
    }

   if (Memo("EMULATED"))
    {
     if (ArgCnt<1) WrError(1110);
     else
      for (z=1; z<=ArgCnt; z++)
       {
        NLS_UpString(ArgStr[z]);
        if (! StringListPresent(Emulations,ArgStr[z]))
         AddStringListLast(&Emulations,ArgStr[z]);
       }
     return true;
    }

   return false;
}

	static void MakeCode_29K(void)
{
   Integer z;
   int l;
   LongWord Dest,Src1,Src2,Src3,AdrLong;
   LongInt AdrInt;
   bool OK;

   CodeLen=0; DontPrint=false;

   /* Nullanweisung */

   if (Memo("") && (*AttrPart=='\0') && (ArgCnt==0)) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(true)) return;

   /* Variante 1: Register <-- Register op Register/uimm8 */

   for (z=0; z<StdOrderCount; z++)
    if (Memo(StdOrders[z].Name))
     {
      if ((ArgCnt>3) || (ArgCnt<2)) WrError(1110);
      else if (! DecodeReg(ArgStr[1],&Dest)) WrXError(1445,ArgStr[1]);
      else
       {
        OK=true;
        if (ArgCnt==2) Src1=Dest;
        else OK=DecodeReg(ArgStr[2],&Src1);
        if (! OK) WrXError(1445,ArgStr[2]);
        else
         {
          if (DecodeReg(ArgStr[ArgCnt],&Src2))
           {
            OK=true; Src3=0;
           }
          else
           {
            Src2=EvalIntExpression(ArgStr[ArgCnt],UInt8,&OK);
            Src3=0x1000000;
           }
          if (OK)
           {
            CodeLen=4;
            DAsmCode[0]=(StdOrders[z].Code << 24)+Src3+(Dest << 16)+(Src1 << 8)+Src2;
            if (StdOrders[z].MustSup) ChkSup();
           }
         }
       }
      return;
     }

   /* Variante 2: Register <-- Register op Register */

   for (z=0; z<NoImmOrderCount; z++)
    if (Memo(NoImmOrders[z].Name))
     {
      if ((ArgCnt>3) || (ArgCnt<2)) WrError(1110);
      else if (! DecodeReg(ArgStr[1],&Dest)) WrXError(1445,ArgStr[1]);
      else
       {
        OK=true;
        if (ArgCnt==2) Src1=Dest;
        else OK=DecodeReg(ArgStr[2],&Src1);
        if (! OK) WrError(1445);
        else if (! DecodeReg(ArgStr[ArgCnt],&Src2)) WrError(1445);
        else
         {
          CodeLen=4;
          DAsmCode[0]=(NoImmOrders[z].Code << 24)+(Dest << 16)+(Src1 << 8)+Src2;
          if (NoImmOrders[z].MustSup) ChkSup();
         }
       }
      return;
     }

   /* Variante 3: Vektor <-- Register op Register/uimm8 */

   for (z=0; z<VecOrderCount; z++)
    if (Memo(VecOrders[z].Name))
     {
      if (ArgCnt!=3) WrError(1110);
      else
       {
        FirstPassUnknown=false;
        Dest=EvalIntExpression(ArgStr[1],UInt8,&OK);
        if (FirstPassUnknown) Dest=64;
        if (OK)
         if (! DecodeReg(ArgStr[2],&Src1)) WrError(1445);
         else
          {
           if (DecodeReg(ArgStr[ArgCnt],&Src2))
            {
             OK=true; Src3=0;
            }
           else
            {
             Src2=EvalIntExpression(ArgStr[ArgCnt],UInt8,&OK);
             Src3=0x1000000;
            }
           if (OK)
            {
             CodeLen=4;
             DAsmCode[0]=(VecOrders[z].Code << 24)+Src3+(Dest << 16)+(Src1 << 8)+Src2;
             if ((VecOrders[z].MustSup) || (Dest<=63)) ChkSup();
            }
          }
       }
      return;
     }

   /* Variante 4: ohne Operanden */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     {
      if (ArgCnt!=0) WrError(1110);
      else
       {
        CodeLen=4; DAsmCode[0]=FixedOrders[z].Code << 24;
        if (FixedOrders[z].MustSup) ChkSup();
       }
      return;
     }

   /* Variante 5 : [0], Speichersteuerwort, Register, Register/uimm8 */

   for (z=0; z<MemOrderCount; z++)
    if (Memo(MemOrders[z].Name))
     {
      if ((ArgCnt!=3) && (ArgCnt!=4)) WrError(1110);
      else
       {
        if (ArgCnt==3)
         {
          OK=true; AdrLong=0;
         }
        else
         {
          AdrLong=EvalIntExpression(ArgStr[1],Int32,&OK);
          if (OK) OK=ChkRange(AdrLong,0,0);
         }
        if (OK)
         {
          Dest=EvalIntExpression(ArgStr[ArgCnt-2],UInt7,&OK);
          if (OK)
           if (DecodeReg(ArgStr[ArgCnt-1],&Src1))
            {
             if (DecodeReg(ArgStr[ArgCnt],&Src2))
              {
               OK=true; Src3=0;
              }
             else
              {
               Src2=EvalIntExpression(ArgStr[ArgCnt],UInt8,&OK);
               Src3=0x1000000;
              }
             if (OK)
              {
               CodeLen=4;
               DAsmCode[0]=(MemOrders[z].Code << 24)+Src3+(Dest << 16)+(Src1 << 8)+Src2;
               if (MemOrders[z].MustSup) ChkSup();
              }
            }
         }
       }
      return;
     }

   /* Sprungbefehle */

   for (z=0; z<JmpOrderCount; z++)
    {
     l=strlen(JmpOrders[z].Name);
     if ((strncmp(OpPart,JmpOrders[z].Name,l)==0) && ((OpPart[l]=='\0') || (OpPart[l]=='I')))
      {
       if (ArgCnt!=1+JmpOrders[z].HasReg) WrError(1110);
       else if (DecodeReg(ArgStr[ArgCnt],&Src1))
        {
         if (! JmpOrders[z].HasReg)
          {
           Dest=0; OK=true;
          }
         else OK=DecodeReg(ArgStr[1],&Dest);
         if (OK)
          {
           CodeLen=4;
           DAsmCode[0]=((JmpOrders[z].Code+0x20) << 24)+(Dest << 8)+Src1;
          }
        }
       else if (OpPart[l]=='I') WrError(1445);
       else
        {
         if (! JmpOrders[z].HasReg)
          {
           Dest=0; OK=true;
          }
         else OK=DecodeReg(ArgStr[1],&Dest);
         if (OK)
          {
           AdrLong=EvalIntExpression(ArgStr[ArgCnt],Int32,&OK);
           AdrInt=AdrLong-EProgCounter();
           if (OK)
            if ((AdrLong & 3)!=0) WrError(1325);
             else if ((AdrInt<=0x1ffff) && (AdrInt>=-0x20000))
             {
              CodeLen=4;
              AdrLong-=EProgCounter();
              DAsmCode[0]=(JmpOrders[z].Code << 24)
                         +((AdrLong & 0x3fc00) << 6)
                         +(Dest << 8)+((AdrLong & 0x3fc) >> 2);
             }
            else if ((! SymbolQuestionable) && (AdrLong>0x3fffff)) WrError(1370);
            else
             {
              CodeLen=4;
              DAsmCode[0]=((JmpOrders[z].Code+1) << 24)
                         +((AdrLong & 0x3fc00) << 6)
                         +(Dest << 8)+((AdrLong & 0x3fc) >> 2);
             }
          }
        }
       return;
      }
    }

   /* Sonderfaelle */

   if (Memo("CLASS"))
    {
     if (ArgCnt!=3) WrError(1110);
     else if (! ChkCPU(CPU29000)) WrError(1500);
     else if (! DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else if (! DecodeReg(ArgStr[2],&Src1)) WrError(1445);
     else
      {
       Src2=EvalIntExpression(ArgStr[3],UInt2,&OK);
       if (OK)
        {
         CodeLen=4;
         DAsmCode[0]=0xe6000000+(Dest << 16)+(Src1 << 8)+Src2;
        }
      }
     return;
    }

   if (Memo("EMULATE"))
    {
     if (ArgCnt!=3) WrError(1110);
     else
      {
       FirstPassUnknown=false;
       Dest=EvalIntExpression(ArgStr[1],UInt8,&OK);
       if (FirstPassUnknown) Dest=64;
       if (OK)
        if (! DecodeReg(ArgStr[2],&Src1)) WrError(1445);
        else if (! DecodeReg(ArgStr[ArgCnt],&Src2)) WrError(1445);
        else
         {
          CodeLen=4;
	  DAsmCode[0]=0xd7000000+(Dest << 16)+(Src1 << 8)+Src2;
          if (Dest<=63) ChkSup();
         }
      }
     return;
    }

   if (Memo("SQRT"))
    {
     if ((ArgCnt!=3) && (ArgCnt!=2)) WrError(1110);
     else if (! ChkCPU(CPU29000)) WrError(1500);
     else if (! DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else
      {
       if (ArgCnt==2)
        {
         OK=true; Src1=Dest;
        }
       else OK=DecodeReg(ArgStr[2],&Src1);
       if (! OK) WrError(1445);
       else
        {
         Src2=EvalIntExpression(ArgStr[ArgCnt],UInt2,&OK);
         if (OK)
          {
           CodeLen=4;
           DAsmCode[0]=0xe5000000+(Dest << 16)+(Src1 << 8)+Src2;
          }
        }
      }
     return;
    }

   if (Memo("CLZ"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (! DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else
      {
       if (DecodeReg(ArgStr[2],&Src1))
        {
         OK=true; Src3=0;
	}
       else
	{
	 Src1=EvalIntExpression(ArgStr[2],UInt8,&OK);
         Src3=0x1000000;
        }
       if (OK)
        {
         CodeLen=4;
	 DAsmCode[0]=0x08000000+Src3+(Dest << 16)+Src1;
        }
      }
     return;
    }

   if (Memo("CONST"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (! DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else
      {
       AdrLong=EvalIntExpression(ArgStr[2],Int32,&OK);
       if (OK)
        {
         CodeLen=4;
	 DAsmCode[0]=((AdrLong & 0xff00) << 8)+(Dest << 8)+(AdrLong & 0xff);
         AdrLong=AdrLong >> 16;
         if (AdrLong==0xffff) DAsmCode[0]+=0x01000000;
         else
          {
           DAsmCode[0]+=0x03000000;
           if (AdrLong!=0)
            {
             CodeLen=8;
             DAsmCode[1]=0x02000000+((AdrLong & 0xff00) << 16)+(Dest << 8)+(AdrLong & 0xff);
            }
          }
        }
      }
     return;
    }

   if ((Memo("CONSTH")) || (Memo("CONSTN")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (! DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else
      {
       FirstPassUnknown=false;
       AdrLong=EvalIntExpression(ArgStr[2],Int32,&OK);
       if (FirstPassUnknown) AdrLong&=0xffff;
       if ((Memo("CONSTN")) && ((AdrLong >> 16)==0xffff)) AdrLong&=0xffff;
       if (ChkRange(AdrLong,0,0xffff))
        {
         CodeLen=4;
         DAsmCode[0]=0x1000000+((AdrLong & 0xff00) << 8)+(Dest << 8)+(AdrLong & 0xff);
         if (Memo("CONSTH")) DAsmCode[0]+=0x1000000;
	}
      }
     return;
    }

   if (Memo("CONVERT"))
    {
     if (ArgCnt!=6) WrError(1110);
     else if (! ChkCPU(CPU29000)) WrError(1500);
     else if (! DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else if (! DecodeReg(ArgStr[2],&Src1)) WrError(1445);
     else
      {
       Src2=0;
       Src2+=EvalIntExpression(ArgStr[3],UInt1,&OK) << 7;
       if (OK)
        {
         Src2+=EvalIntExpression(ArgStr[4],UInt3,&OK) << 4;
         if (OK)
          {
           Src2+=EvalIntExpression(ArgStr[5],UInt2,&OK) << 2;
           if (OK)
            {
             Src2+=EvalIntExpression(ArgStr[6],UInt2,&OK);
             if (OK)
              {
               CodeLen=4;
               DAsmCode[0]=0xe4000000+(Dest << 16)+(Src1 << 8)+Src2;
              }
            }
          }
        }
      }
     return;
    }

   if (Memo("EXHWS"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (! DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else if (! DecodeReg(ArgStr[2],&Src1)) WrError(1445);
     else
      {
       CodeLen=4;
       DAsmCode[0]=0x7e000000+(Dest << 16)+(Src1 << 8);
      }
     return;
    }

   if ((Memo("INV")) || (Memo("IRETINV")))
    {
     if (ArgCnt>1) WrError(1110);
     else
      {
       if (ArgCnt==0)
        {
         Src1=0; OK=true;
        }
       else Src1=EvalIntExpression(ArgStr[1],UInt2,&OK);
       if (OK)
        {
         CodeLen=4;
         DAsmCode[0]=Src1 << 16;
         if (Memo("INV")) DAsmCode[0]+=0x9f000000;
         else DAsmCode[0]+=0x8c000000;
         ChkSup();
        }
      }
     return;
    }

   if (Memo("MFSR"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (! DecodeReg(ArgStr[1],&Dest)) WrXError(1445,ArgStr[1]);
     else if (! DecodeSpReg(ArgStr[2],&Src1)) WrXError(1440,ArgStr[2]);
     else
      {
       DAsmCode[0]=0xc6000000+(Dest << 16)+(Src1 << 8);
       CodeLen=4; if (IsSup(Src1)) ChkSup();
      }
     return;
    }

   if (Memo("MTSR"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (! DecodeSpReg(ArgStr[1],&Dest)) WrXError(1440,ArgStr[1]);
     else if (! DecodeReg(ArgStr[2],&Src1)) WrXError(1445,ArgStr[2]);
     else
      {
       DAsmCode[0]=0xce000000+(Dest << 8)+Src1;
       CodeLen=4; if (IsSup(Dest)) ChkSup();
      }
     return;
    }

   if (Memo("MTSRIM"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (! DecodeSpReg(ArgStr[1],&Dest)) WrXError(1440,ArgStr[1]);
     else
      {
       Src1=EvalIntExpression(ArgStr[2],UInt16,&OK);
       if (OK)
        {
         DAsmCode[0]=0x04000000+((Src1 & 0xff00) << 8)+(Dest << 8)+Lo(Src1);
         CodeLen=4; if (IsSup(Dest)) ChkSup();
        }
      }
     return;
    }

   if (Memo("MFTLB"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (! DecodeReg(ArgStr[1],&Dest)) WrXError(1445,ArgStr[1]);
     else if (! DecodeReg(ArgStr[2],&Src1)) WrXError(1445,ArgStr[2]);
     else
      {
       DAsmCode[0]=0xb6000000+(Dest << 16)+(Src1 << 8);
       CodeLen=4; ChkSup();
      }
     return;
    }

   if (Memo("MTTLB"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (! DecodeReg(ArgStr[1],&Dest)) WrXError(1445,ArgStr[1]);
     else if (! DecodeReg(ArgStr[2],&Src1)) WrXError(1445,ArgStr[2]);
     else
      {
       DAsmCode[0]=0xbe000000+(Dest << 8)+Src1;
       CodeLen=4; ChkSup();
      }
     return;
    }

   /* unbekannter Befehl */

   WrXError(1200,OpPart);
}

	static void InitCode_29K(void)
{
   SaveInitProc();
   Reg_RBP=0; ClearStringList(&Emulations);
}

        static bool ChkPC_29K(void)
{
#ifdef HAS64
   return ((ActPC==SegCode) && (ProgCounter()<=0xffffffffll));
#else
   return (ActPC==SegCode);
#endif
}

        static bool IsDef_29K(void)
{
   return false;
}

        static void SwitchFrom_29K(void)
{
   DeinitFields();
}

        static void SwitchTo_29K(void)
{
   TurnWords=true; ConstMode=ConstModeC; SetIsOccupied=false;

   PCSymbol="$"; HeaderID=0x29; NOPCode=0x000000000;
   DivideChars=","; HasAttrs=false;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=4; SegInits[SegCode]=0;

   MakeCode=MakeCode_29K; ChkPC=ChkPC_29K; IsDef=IsDef_29K;

   SwitchFrom=SwitchFrom_29K; InitFields();
}

	void code29k_init(void)
{
   CPU29245=AddCPU("AM29245",SwitchTo_29K);
   CPU29243=AddCPU("AM29243",SwitchTo_29K);
   CPU29240=AddCPU("AM29240",SwitchTo_29K);
   CPU29000=AddCPU("AM29000",SwitchTo_29K);

   Emulations=NULL;

   SaveInitProc=InitPassProc; InitPassProc=InitCode_29K;
}

