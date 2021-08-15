/* code6812.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegeneratormodul CPU12                                                  */
/*                                                                           */
/* Historie: 13.10.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "stringutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "codepseudo.h"
#include "codevars.h"


typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Word Code;
          bool MayImm,MayDir,MayExt;
          ShortInt ThisOpSize;
         } GenOrder;

typedef struct
         {
          char *Name;
          Word Code;
          bool MayDir;
         } JmpOrder;


#define ModNone (-1)
#define ModImm 0
#define MModImm (1 << ModImm)
#define ModDir 1
#define MModDir (1 << ModDir)
#define ModExt 2
#define MModExt (1 << ModExt)
#define ModIdx 3
#define MModIdx (1 << ModIdx)
#define ModIdx1 4
#define MModIdx1 (1 << ModIdx1)
#define ModIdx2 5
#define MModIdx2 (1 << ModIdx2)
#define ModDIdx 6
#define MModDIdx (1 << ModDIdx)
#define ModIIdx2 7
#define MModIIdx2 (1 << ModIIdx2)

#define MModAllIdx (MModIdx | MModIdx1 | MModIdx2 | MModDIdx | MModIIdx2)

#define FixedOrderCount 87
#define BranchOrderCount 20
#define GenOrderCount 56
#define LoopOrderCount 6
#define LEAOrderCount 3
#define JmpOrderCount 2

static ShortInt OpSize;
static ShortInt AdrMode;
static ShortInt ExPos;
static Byte AdrVals[4];
static CPUVar CPU6812;

static FixedOrder *FixedOrders;
static FixedOrder *BranchOrders;
static GenOrder *GenOrders;
static FixedOrder *LoopOrders;
static FixedOrder *LEAOrders;
static JmpOrder *JmpOrders;

/*---------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode)
{
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
}

        static void AddBranch(char *NName, Word NCode)
{
   if (InstrZ>=BranchOrderCount) exit(255);
   BranchOrders[InstrZ].Name=NName;
   BranchOrders[InstrZ++].Code=NCode;
}

        static void AddGen(char *NName, Word NCode,
                           bool NMayI, bool NMayD, bool NMayE,
                           ShortInt NSize)
{
   if (InstrZ>=GenOrderCount) exit(255);
   GenOrders[InstrZ].Name=NName;
   GenOrders[InstrZ].Code=NCode;
   GenOrders[InstrZ].MayImm=NMayI;
   GenOrders[InstrZ].MayDir=NMayD;
   GenOrders[InstrZ].MayExt=NMayE;
   GenOrders[InstrZ++].ThisOpSize=NSize;
}

        static void AddLoop(char *NName, Word NCode)
{
   if (InstrZ>=LoopOrderCount) exit(255);
   LoopOrders[InstrZ].Name=NName;
   LoopOrders[InstrZ++].Code=NCode;
}

        static void AddLEA(char *NName, Word NCode)
{
   if (InstrZ>=LEAOrderCount) exit(255);
   LEAOrders[InstrZ].Name=NName;
   LEAOrders[InstrZ++].Code=NCode;
}

        static void AddJmp(char *NName, Word NCode, bool NDir)
{
   if (InstrZ>=JmpOrderCount) exit(255);
   JmpOrders[InstrZ].Name=NName;
   JmpOrders[InstrZ].Code=NCode;
   JmpOrders[InstrZ++].MayDir=NDir;
}


        static void InitFields(void)
{
   FixedOrders=(FixedOrder *) malloc(FixedOrderCount*sizeof(FixedOrder)); InstrZ=0;
   AddFixed("ABA"  ,0x1806); AddFixed("ABX"  ,0x1ae5);
   AddFixed("ABY"  ,0x19ed); AddFixed("ASLA" ,0x0048);
   AddFixed("ASLB" ,0x0058); AddFixed("ASLD" ,0x0059);
   AddFixed("ASRA" ,0x0047); AddFixed("ASRB" ,0x0057);
   AddFixed("BGND" ,0x0000); AddFixed("CBA"  ,0x1817);
   AddFixed("CLC"  ,0x10fe); AddFixed("CLI"  ,0x10ef);
   AddFixed("CLRA" ,0x0087); AddFixed("CLRB" ,0x00c7);
   AddFixed("CLV"  ,0x10fd); AddFixed("COMA" ,0x0041);
   AddFixed("COMB" ,0x0051); AddFixed("DAA"  ,0x1807);
   AddFixed("DECA" ,0x0043); AddFixed("DECB" ,0x0053);
   AddFixed("DES"  ,0x1b9f); AddFixed("DEX"  ,0x0009);
   AddFixed("DEY"  ,0x0003); AddFixed("EDIV" ,0x0011);
   AddFixed("EDIVS",0x1814); AddFixed("EMUL" ,0x0013);
   AddFixed("EMULS",0x1813); AddFixed("FDIV" ,0x1811);
   AddFixed("IDIV" ,0x1810); AddFixed("IDIVS",0x1815);
   AddFixed("INCA" ,0x0042); AddFixed("INCB" ,0x0052);
   AddFixed("INS"  ,0x1b81); AddFixed("INX"  ,0x0008);
   AddFixed("INY"  ,0x0002); AddFixed("LSLA" ,0x0048);
   AddFixed("LSLB" ,0x0058); AddFixed("LSLD" ,0x0059);
   AddFixed("LSRA" ,0x0044); AddFixed("LSRB" ,0x0054);
   AddFixed("LSRD" ,0x0049); AddFixed("MEM"  ,0x0001);
   AddFixed("MUL"  ,0x0012); AddFixed("NEGA" ,0x0040);
   AddFixed("NEGB" ,0x0050); AddFixed("NOP"  ,0x00a7);
   AddFixed("PSHA" ,0x0036); AddFixed("PSHB" ,0x0037);
   AddFixed("PSHC" ,0x0039); AddFixed("PSHD" ,0x003b);
   AddFixed("PSHX" ,0x0034); AddFixed("PSHY" ,0x0035);
   AddFixed("PULA" ,0x0032); AddFixed("PULB" ,0x0033);
   AddFixed("PULC" ,0x0038); AddFixed("PULD" ,0x003a);
   AddFixed("PULX" ,0x0030); AddFixed("PULY" ,0x0031);
   AddFixed("REV"  ,0x183a); AddFixed("REVW" ,0x183b);
   AddFixed("ROLA" ,0x0045); AddFixed("ROLB" ,0x0055);
   AddFixed("RORA" ,0x0046); AddFixed("RORB" ,0x0056);
   AddFixed("RTC"  ,0x000a); AddFixed("RTI"  ,0x000b);
   AddFixed("RTS"  ,0x003d); AddFixed("SBA"  ,0x1816);
   AddFixed("SEC"  ,0x1401); AddFixed("SEI"  ,0x1410);
   AddFixed("SEV"  ,0x1402); AddFixed("STOP" ,0x183e);
   AddFixed("SWI"  ,0x003f); AddFixed("TAB"  ,0x180e);
   AddFixed("TAP"  ,0xb702); AddFixed("TBA"  ,0x180f);
   AddFixed("TPA"  ,0xb720); AddFixed("TSTA" ,0x0097);
   AddFixed("TSTB" ,0x00d7); AddFixed("TSX"  ,0xb775);
   AddFixed("TSY"  ,0xb776); AddFixed("TXS"  ,0xb757);
   AddFixed("TYS"  ,0xb767); AddFixed("WAI"  ,0x003e);
   AddFixed("WAV"  ,0x183c); AddFixed("XGDX" ,0xb7c5);
   AddFixed("XGDY" ,0xb7c6);

   BranchOrders=(FixedOrder *) malloc(BranchOrderCount*sizeof(FixedOrder)); InstrZ=0;
   AddBranch("BGT",0x2e);    AddBranch("BGE",0x2c);
   AddBranch("BEQ",0x27);    AddBranch("BLE",0x2f);
   AddBranch("BLT",0x2d);    AddBranch("BHI",0x22);
   AddBranch("BHS",0x24);    AddBranch("BCC",0x24);
   AddBranch("BNE",0x26);    AddBranch("BLS",0x23);
   AddBranch("BLO",0x25);    AddBranch("BCS",0x25);
   AddBranch("BMI",0x2b);    AddBranch("BVS",0x29);
   AddBranch("BRA",0x20);    AddBranch("BPL",0x2a);
   AddBranch("BGT",0x2e);    AddBranch("BRN",0x21);
   AddBranch("BVC",0x28);    AddBranch("BSR",0x07);

   GenOrders=(GenOrder *) malloc(sizeof(GenOrder)*GenOrderCount); InstrZ=0;
   AddGen("ADCA" ,0x0089,true ,true ,true , 0);
   AddGen("ADCB" ,0x00c9,true ,true ,true , 0);
   AddGen("ADDA" ,0x008b,true ,true ,true , 0);
   AddGen("ADDB" ,0x00cb,true ,true ,true , 0);
   AddGen("ADDD" ,0x00c3,true ,true ,true , 1);
   AddGen("ANDA" ,0x0084,true ,true ,true , 0);
   AddGen("ANDB" ,0x00c4,true ,true ,true , 0);
   AddGen("ASL"  ,0x0048,false,false,true ,-1);
   AddGen("ASR"  ,0x0047,false,false,true ,-1);
   AddGen("BITA" ,0x0085,true ,true ,true , 0);
   AddGen("BITB" ,0x00c5,true ,true ,true , 0);
   AddGen("CLR"  ,0x0049,false,false,true ,-1);
   AddGen("CMPA" ,0x0081,true ,true ,true , 0);
   AddGen("CMPB" ,0x00c1,true ,true ,true , 0);
   AddGen("COM"  ,0x0041,false,false,true ,-1);
   AddGen("CPD"  ,0x008c,true ,true ,true , 1);
   AddGen("CPS"  ,0x008f,true ,true ,true , 1);
   AddGen("CPX"  ,0x008e,true ,true ,true , 1);
   AddGen("CPY"  ,0x008d,true ,true ,true , 1);
   AddGen("DEC"  ,0x0043,false,false,true ,-1);
   AddGen("EMAXD",0x18fa,false,false,false,-1);
   AddGen("EMAXM",0x18fe,false,false,false,-1);
   AddGen("EMIND",0x18fb,false,false,false,-1);
   AddGen("EMINM",0x18ff,false,false,false,-1);
   AddGen("EORA" ,0x0088,true ,true ,true , 0);
   AddGen("EORB" ,0x00c8,true ,true ,true , 0);
   AddGen("INC"  ,0x0042,false,false,true ,-1);
   AddGen("LDAA" ,0x0086,true ,true ,true , 0);
   AddGen("LDAB" ,0x00c6,true ,true ,true , 0);
   AddGen("LDD"  ,0x00cc,true ,true ,true , 1);
   AddGen("LDS"  ,0x00cf,true ,true ,true , 1);
   AddGen("LDX"  ,0x00ce,true ,true ,true , 1);
   AddGen("LDY"  ,0x00cd,true ,true ,true , 1);
   AddGen("LSL"  ,0x0048,false,false,true ,-1);
   AddGen("LSR"  ,0x0044,false,false,true ,-1);
   AddGen("MAXA" ,0x18f8,false,false,false,-1);
   AddGen("MAXM" ,0x18fc,false,false,false,-1);
   AddGen("MINA" ,0x18f9,false,false,false,-1);
   AddGen("MINM" ,0x18fd,false,false,false,-1);
   AddGen("NEG"  ,0x0040,false,false,true ,-1);
   AddGen("ORAA" ,0x008a,true ,true ,true , 0);
   AddGen("ORAB" ,0x00ca,true ,true ,true , 0);
   AddGen("ROL"  ,0x0045,false,false,true ,-1);
   AddGen("ROR"  ,0x0046,false,false,true ,-1);
   AddGen("SBCA" ,0x0082,true ,true ,true , 0);
   AddGen("SBCB" ,0x00c2,true ,true ,true , 0);
   AddGen("STAA" ,0x004a,false,true ,true , 0);
   AddGen("STAB" ,0x004b,false,true ,true , 0);
   AddGen("STD"  ,0x004c,false,true ,true ,-1);
   AddGen("STS"  ,0x004f,false,true ,true ,-1);
   AddGen("STX"  ,0x004e,false,true ,true ,-1);
   AddGen("STY"  ,0x004d,false,true ,true ,-1);
   AddGen("SUBA" ,0x0080,true ,true ,true , 0);
   AddGen("SUBB" ,0x00c0,true ,true ,true , 0);
   AddGen("SUBD" ,0x0083,true ,true ,true , 1);
   AddGen("TST"  ,0x00c7,false,false,true ,-1);

   LoopOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*LoopOrderCount); InstrZ=0;
   AddLoop("DBEQ",0x00); AddLoop("DBNE",0x20);
   AddLoop("IBEQ",0x80); AddLoop("IBNE",0xa0);
   AddLoop("TBEQ",0x40); AddLoop("TBNE",0x60);

   LEAOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*LEAOrderCount); InstrZ=0;
   AddLEA("LEAS",0x1b);
   AddLEA("LEAX",0x1a);
   AddLEA("LEAY",0x19);

   JmpOrders=(JmpOrder *) malloc(sizeof(JmpOrder)*JmpOrderCount); InstrZ=0;
   AddJmp("JMP",0x06,false);
   AddJmp("JSR",0x16,true);
}

        static void DeinitFields(void)
{
   free(FixedOrders);
   free(BranchOrders);
   free(GenOrders);
   free(LoopOrders);
   free(LEAOrders);
   free(JmpOrders);
}

/*---------------------------------------------------------------------------*/

#define PCReg 3

	static bool DecodeReg16(char *Asc, Byte *Erg)
{
   *Erg=0xff;
   if (strcasecmp(Asc,"X")==0) *Erg=0;
   else if (strcasecmp(Asc,"Y")==0) *Erg=1;
   else if (strcasecmp(Asc,"SP")==0) *Erg=2;
   else if (strcasecmp(Asc,"PC")==0) *Erg=PCReg;
   return (*Erg!=0xff);
}

	static bool ValidReg(char *Asc_o)
{
   Byte Dummy;
   String Asc;
   Integer l=strlen(Asc_o);

   strmaxcpy(Asc,Asc_o,255);

   if ((*Asc=='-') || (*Asc=='+')) strcpy(Asc,Asc+1);
   else if ((Asc[l-1]=='-') || (Asc[l-1]=='+')) Asc[l-1]='\0';
   return DecodeReg16(Asc,&Dummy);
}

	static bool DecodeReg8(char *Asc, Byte *Erg)
{
   *Erg=0xff;
   if (strcasecmp(Asc,"A")==0) *Erg=0;
   else if (strcasecmp(Asc,"B")==0) *Erg=1;
   else if (strcasecmp(Asc,"D")==0) *Erg=2;
   return (*Erg!=0xff);
}

	static bool DecodeReg(char *Asc, Byte *Erg)
{
   if (DecodeReg8(Asc,Erg))
    {
     if (*Erg==2) *Erg=4; return true;
    }
   else if (DecodeReg16(Asc,Erg))
    {
     *Erg+=5; return (*Erg!=PCReg);
    }
   else if (strcasecmp(Asc,"CCR")==0)
    {
     *Erg=2; return true;
    }
   else return false;
}

	static void CutShort(char *Asc, Integer *ShortMode)
{
   if (*Asc=='>')
    {
     *ShortMode=1;
     strcpy(Asc,Asc+1);
    }
   else if (*Asc=='<')
    {
     *ShortMode=2;
     strcpy(Asc,Asc+1);
     if (*Asc=='<')
      {
       *ShortMode=3;
       strcpy(Asc,Asc+1);
      }
    }
   else *ShortMode=0;
}

	static bool DistFits(Byte Reg, Integer Dist, Integer Offs, LongInt Min, LongInt Max)
{
   if (Reg==PCReg) Dist-=Offs;
   return (((Dist>=Min) && (Dist<=Max)) || ((Reg==PCReg) && SymbolQuestionable));
}

	static void ChkAdr(Word Mask)
{
  if ((AdrMode!=ModNone) && (((1 << AdrMode) & Mask)==0))
   {
    AdrMode=ModNone; AdrCnt=0; WrError(1350);
   }
}

        static void DecodeAdr(Integer Start, Integer Stop, Word Mask)
{
   Integer AdrWord,ShortMode,l;
   char *p;
   bool OK;
   bool DecFlag,AutoFlag,PostFlag;

   AdrMode=ModNone; AdrCnt=0;

   if (Stop-Start==0)
    {

     /* immediate */

     if (*ArgStr[Start]=='#')
      {
       switch (OpSize)
        {
         case -1:WrError(1132); break;
         case 0:
          AdrVals[0]=EvalIntExpression(ArgStr[Start]+1,Int8,&OK);
          if (OK)
           {
            AdrCnt=1; AdrMode=ModImm;
           }
          break;
         case 1:
          AdrWord=EvalIntExpression(ArgStr[Start]+1,Int16,&OK);
          if (OK)
           {
            AdrVals[0]=AdrWord >> 8; AdrVals[1]=AdrWord & 0xff;
            AdrCnt=2; AdrMode=ModImm;
           }
          break;
        }
       ChkAdr(Mask); return;
      }

     /* indirekt */

     if ((*ArgStr[Start]=='[') && (ArgStr[Start][strlen(ArgStr[Start])-1]==']'))
      {
       strcpy(ArgStr[Start],ArgStr[Start]+1);
       ArgStr[Start][strlen(ArgStr[Start])-1]='\0';
       p=QuotPos(ArgStr[Start],','); if (p!=NULL) *p='\0';
       if (p==NULL) WrError(1350);
       else if (! DecodeReg16(p+1,AdrVals))
        WrXError(1445,p+1);
       else if (strcasecmp(ArgStr[Start],"D")==0)
        {
         AdrVals[0]=(AdrVals[0] << 3) | 0xe7;
         AdrCnt=1; AdrMode=ModDIdx;
        }
       else
        {
         AdrWord=EvalIntExpression(ArgStr[Start],Int16,&OK);
         if (OK)
          {
           if (AdrVals[0]==PCReg) AdrWord-=EProgCounter()+ExPos+3;
           AdrVals[0]=(AdrVals[0] << 3) | 0xe3;
           AdrVals[1]=AdrWord >> 8;
           AdrVals[2]=AdrWord & 0xff;
           AdrCnt=3; AdrMode=ModIIdx2;
          }
        }
       ChkAdr(Mask); return;
      }

     /* dann absolut */

     CutShort(ArgStr[Start],&ShortMode);

     if ((ShortMode==2) || ((ShortMode==0) && ((Mask & MModExt)==0)))
      AdrWord=EvalIntExpression(ArgStr[Start],UInt8,&OK);
     else
      AdrWord=EvalIntExpression(ArgStr[Start],UInt16,&OK);

     if (OK)
      if ((ShortMode!=1) && ((AdrWord & 0xff00)==0) && ((Mask & MModDir)!=0))
       {
        AdrMode=ModDir; AdrVals[0]=AdrWord & 0xff; AdrCnt=1;
       }
      else
       {
        AdrMode=ModExt;
	AdrVals[0]=(AdrWord >> 8) & 0xff; AdrVals[1]=AdrWord & 0xff;
	AdrCnt=2;
       }
     ChkAdr(Mask); return;
    }

   else if (Stop-Start==1)
    {

     /* Autoin/-dekrement abspalten */

     l=strlen(ArgStr[Stop]);
     if ((*ArgStr[Stop]=='-') || (*ArgStr[Stop]=='+'))
      {
       DecFlag=(*ArgStr[Stop]=='-');
       AutoFlag=true; PostFlag=false; strcpy(ArgStr[Stop],ArgStr[Stop]+1);
      }
     else if ((ArgStr[Stop][l-1]=='-') || (ArgStr[Stop][l-1]=='+'))
      {
       DecFlag=(ArgStr[Stop][l-1]=='-');
       AutoFlag=true; PostFlag=true; ArgStr[Stop][l-1]='\0';
      }
     else AutoFlag=DecFlag=PostFlag=false;

     if (AutoFlag)
      {
       if (! DecodeReg16(ArgStr[Stop],AdrVals)) WrXError(1445,ArgStr[Stop]);
       else if (AdrVals[0]==PCReg) WrXError(1445,ArgStr[Stop]);
       else
        {
         FirstPassUnknown=false;
         AdrWord=EvalIntExpression(ArgStr[Start],SInt8,&OK);
         if (FirstPassUnknown) AdrWord=1;
         if (AdrWord==0)
          {
           AdrVals[0]=0; AdrCnt=1; AdrMode=ModIdx;
          }
         else if (AdrWord>8) WrError(1320);
         else if (AdrWord<-8) WrError(1315);
         else
          {
           if (AdrWord<0)
            {
             DecFlag=! DecFlag; AdrWord=(-AdrWord);
            }
           if (DecFlag) AdrWord=8-AdrWord; else AdrWord--;
           AdrVals[0]=(AdrVals[0] << 6)+0x20+(PostFlag << 4)+(DecFlag << 3)+(AdrWord & 7);
           AdrCnt=1; AdrMode=ModIdx;
          }
        }
       ChkAdr(Mask); return;
      }

     else
      {
       if (! DecodeReg16(ArgStr[Stop],AdrVals)) WrXError(1445,ArgStr[Stop]);
       else if (DecodeReg8(ArgStr[Start],AdrVals+1))
        {
         AdrVals[0]=(AdrVals[0] << 3)+AdrVals[1]+0xe4;
         AdrCnt=1; AdrMode=ModIdx;
        }
       else
        {
         CutShort(ArgStr[Start],&ShortMode);
         AdrWord=EvalIntExpression(ArgStr[Start],Int16,&OK);
         if (AdrVals[0]==PCReg) AdrWord-=EProgCounter()+ExPos;
         if (OK)
          if ((ShortMode!=1) && (ShortMode!=2) && ((Mask & MModIdx)!=0) && (DistFits(AdrVals[0],AdrWord,1,-16,15)))
           {
            if (AdrVals[0]==PCReg) AdrWord--;
            AdrVals[0]=(AdrVals[0] << 6)+(AdrWord & 0x1f);
            AdrCnt=1; AdrMode=ModIdx;
           }
          else if ((ShortMode!=1) && (ShortMode!=3) && ((Mask & MModIdx1)!=0) && DistFits(AdrVals[0],AdrWord,2,-256,255))
           {
            if (AdrVals[0]==PCReg) AdrWord-=2;
            AdrVals[0]=0xe0+(AdrVals[0] << 3)+((AdrWord >> 8) & 1);
            AdrVals[1]=AdrWord & 0xff;
            AdrCnt=2; AdrMode=ModIdx1;
           }
          else
           {
            if (AdrVals[0]==PCReg) AdrWord-=3;
            AdrVals[0]=0xe3+(AdrVals[0] << 3);
            AdrVals[1]=(AdrWord >> 8) & 0xff;
            AdrVals[2]=AdrWord & 0xff;
            AdrCnt=3; AdrMode=ModIdx2;
           }
        }
       ChkAdr(Mask); return;
      }
    }

   else WrError(1350);

   ChkAdr(Mask);
}

/*---------------------------------------------------------------------------*/

	static bool DecodePseudo(void)
{
   return false;
}

	static void Try2Split(Integer Src)
{
   char *p;
   Integer z;

   KillPrefBlanks(ArgStr[Src]); KillPostBlanks(ArgStr[Src]);
   p=ArgStr[Src]+strlen(ArgStr[Src])-1;
   while ((p>=ArgStr[Src]) && (! isspace(*p))) p--;
   if (p>=ArgStr[Src])
    {
     for (z=ArgCnt; z>=Src; z--) strcpy(ArgStr[z+1],ArgStr[z]); ArgCnt++;
     *p='\0'; strcpy(ArgStr[Src+1],p+1);
     KillPostBlanks(ArgStr[Src]); KillPrefBlanks(ArgStr[Src+1]);
    }
}

        static void MakeCode_6812(void)
{
   Integer z;
   LongInt Address;
   Byte HReg,HCnt;
   bool OK;
   Word Mask;

   CodeLen=0; DontPrint=false; OpSize=(-1);

   /* Operandengroesse festlegen */

   if (*AttrPart!='\0')
    switch (toupper(*AttrPart))
     {
      case 'B':OpSize=0; break;
      case 'W':OpSize=1; break;
      case 'L':OpSize=2; break;
      case 'Q':OpSize=3; break;
      case 'S':OpSize=4; break;
      case 'D':OpSize=5; break;
      case 'X':OpSize=6; break;
      case 'P':OpSize=7; break;
      default:
       WrError(1107); return;
     }

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(true)) return;
   if (DecodeMoto16Pseudo(OpSize,true)) return;

   /* Anweisungen ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if Memo(FixedOrders[z].Name)
     {
      if (ArgCnt!=0) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       {
        CodeLen=1+(Hi(FixedOrders[z].Code)!=0);
        if (CodeLen==2) BAsmCode[0]=Hi(FixedOrders[z].Code);
        BAsmCode[CodeLen-1]=Lo(FixedOrders[z].Code);
       }
      return;
     }

   /* einfacher Adressoperand */

   for (z=0; z<GenOrderCount; z++)
    if Memo(GenOrders[z].Name)
     {
      if ((ArgCnt<1) || (ArgCnt>2)) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       {
        ExPos=1+(Hi(GenOrders[z].Code)!=0); OpSize=GenOrders[z].ThisOpSize;
        Mask=MModAllIdx;
        if (GenOrders[z].MayImm) Mask+=MModImm;
        if (GenOrders[z].MayDir) Mask+=MModDir;
        if (GenOrders[z].MayExt) Mask+=MModExt;
        DecodeAdr(1,ArgCnt,Mask);
        if (AdrMode!=ModNone)
         if (Hi(GenOrders[z].Code)==0)
          {
           BAsmCode[0]=GenOrders[z].Code; CodeLen=1;
          }
         else
          {
           BAsmCode[0]=Hi(GenOrders[z].Code);
           BAsmCode[1]=Lo(GenOrders[z].Code); CodeLen=2;
          };
        switch (AdrMode)
         {
          case ModImm: break;
          case ModDir: BAsmCode[CodeLen-1]+=0x10; break;
          case ModIdx:
          case ModIdx1:
          case ModIdx2:
          case ModDIdx:
          case ModIIdx2: BAsmCode[CodeLen-1]+=0x20; break;
          case ModExt: BAsmCode[CodeLen-1]+=0x30; break;
         }
        if (AdrMode!=ModNone)
         {
          memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
          CodeLen+=AdrCnt;
         }
       }
      return;
     }

   /* Arithmetik */

   for (z=0; z<LEAOrderCount; z++)
    if (Memo(LEAOrders[z].Name))
     {
      if ((ArgCnt<1) || (ArgCnt>2)) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       {
        ExPos=1;
        DecodeAdr(1,ArgCnt,MModIdx+MModIdx1+MModIdx2);
        if (AdrMode!=ModNone)
         {
          BAsmCode[0]=LEAOrders[z].Code;
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          CodeLen=1+AdrCnt;
         }
       }
      return;
     }

   if ((Memo("TBL")) || (Memo("ETBL")))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      {
       ExPos=2;
       DecodeAdr(1,ArgCnt,MModIdx);
       if (AdrMode==ModIdx)
        {
         BAsmCode[0]=0x18;
         BAsmCode[1]=0x3d+(Memo("ETBL") << 1);
         memcpy(BAsmCode+2,AdrVals,AdrCnt);
         CodeLen=2+AdrCnt;
        }
      }
     return;
    }

   if (Memo("EMACS"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      {
       Address=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
        {
         BAsmCode[0]=0x18;
         BAsmCode[1]=0x12;
         BAsmCode[2]=(Address >> 8) & 0xff;
         BAsmCode[3]=Address & 0xff;
         CodeLen=4;
        }
      }
     return;
    }

   /* Transfer */

   if ((Memo("TFR")) || (Memo("EXG")))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (! DecodeReg(ArgStr[2],BAsmCode+1)) WrXError(1445,ArgStr[2]);
     else if (! DecodeReg(ArgStr[1],&HReg)) WrXError(1445,ArgStr[1]);
     else
      {
       BAsmCode[0]=0xb7;
       BAsmCode[1]+=(Memo("EXG") << 7)+(HReg << 4);
       CodeLen=2;
      }
     return;
    }

   if (Memo("SEX"))
    {
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (! DecodeReg(ArgStr[2],BAsmCode+1)) WrXError(1445,ArgStr[2]);
     else if (BAsmCode[1]<4) WrXError(1445,ArgStr[2]);
     else if (! DecodeReg(ArgStr[1],&HReg)) WrXError(1445,ArgStr[1]);
     else if (HReg>3) WrXError(1445,ArgStr[1]);
     else
      {
       BAsmCode[0]=0xb7;
       BAsmCode[1]+=(Memo("EXG") << 7)+(HReg << 4);
       CodeLen=2;
      }
     return;
    }

   if ((Memo("MOVB")) || (Memo("MOVW")))
    {
     switch (ArgCnt)
      {
       case 1:Try2Split(1); break;
       case 2:Try2Split(1); if (ArgCnt==2) Try2Split(2); break;
       case 3:Try2Split(2); break;
      }
     if ((ArgCnt<2) || (ArgCnt>4)) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      {
       if (ArgCnt==2) HReg=2;
       else if (ArgCnt==4) HReg=3;
       else if (ValidReg(ArgStr[2])) HReg=3;
       else HReg=2;
       OpSize=Memo("MOVW"); ExPos=2;
       BAsmCode[0]=0x18; BAsmCode[1]=Memo("MOVB") << 3;
       DecodeAdr(1,HReg-1,MModImm+MModExt+MModIdx);
       switch (AdrMode)
        {
         case ModImm:
          memmove(AdrVals+2,AdrVals,AdrCnt); HCnt=AdrCnt;
          ExPos=4+2*OpSize; DecodeAdr(HReg,ArgCnt,MModExt+MModIdx);
          switch (AdrMode)
           {
            case ModExt:
             BAsmCode[1]+=3;
             memcpy(BAsmCode+2,AdrVals+2,HCnt);
             memcpy(BAsmCode+2+HCnt,AdrVals,AdrCnt);
             CodeLen=2+HCnt+AdrCnt;
             break;
            case ModIdx:
             BAsmCode[2]=AdrVals[0];
             memcpy(BAsmCode+3,AdrVals+2,HCnt);
             CodeLen=3+HCnt;
             break;
           }
          break;
         case ModExt:
          memmove(AdrVals+2,AdrVals,AdrCnt); HCnt=AdrCnt;
          ExPos=6; DecodeAdr(HReg,ArgCnt,MModExt+MModIdx);
          switch (AdrMode)
           {
            case ModExt:
             BAsmCode[1]+=4;
             memcpy(BAsmCode+2,AdrVals+2,HCnt);
             memcpy(BAsmCode+2+HCnt,AdrVals,AdrCnt);
             CodeLen=2+HCnt+AdrCnt;
             break;
            case ModIdx:
             BAsmCode[1]+=1;
             BAsmCode[2]=AdrVals[0];
             memcpy(BAsmCode+3,AdrVals+2,HCnt);
             CodeLen=3+HCnt;
             break;
           };
          break;
         case ModIdx:
          HCnt=AdrVals[0];
          ExPos=4; DecodeAdr(HReg,ArgCnt,MModExt+MModIdx);
          switch (AdrMode)
           {
            case ModExt:
             BAsmCode[1]+=5;
             BAsmCode[2]=HCnt;
             memcpy(BAsmCode+3,AdrVals,AdrCnt);
             CodeLen=3+AdrCnt;
             break;
            case ModIdx:
             BAsmCode[1]+=2;
             BAsmCode[2]=HCnt;
             BAsmCode[3]=AdrVals[0];
             CodeLen=4;
             break;
           }
          break;
        }
      }
     return;
    }

   /* Logik */

   if ((Memo("ANDCC")) || (Memo("ORCC")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      {
       OpSize=0; DecodeAdr(1,1,MModImm);
       if (AdrMode==ModImm)
        {
         BAsmCode[0]=0x10+(Memo("ORCC") << 2);
         BAsmCode[1]=AdrVals[0];
         CodeLen=2;
        }
      }
     return;
    }

    if ((Memo("BSET")) || (Memo("BCLR")))
     {
      if ((ArgCnt==1) || (ArgCnt==2)) Try2Split(ArgCnt);
      if ((ArgCnt<2) || (ArgCnt>3)) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       {
        if (*ArgStr[ArgCnt]=='#') strcpy(ArgStr[ArgCnt],ArgStr[ArgCnt]+1);
        HReg=EvalIntExpression(ArgStr[ArgCnt],UInt8,&OK);
        if (OK)
         {
          ExPos=2; /* wg. Masken-Postbyte */
          DecodeAdr(1,ArgCnt-1,MModDir+MModExt+MModIdx+MModIdx1+MModIdx2);
          if (AdrMode!=ModNone)
           {
            BAsmCode[0]=0x0c+Memo("BCLR");
            switch (AdrMode)
             {
              case ModDir: BAsmCode[0]+=0x40; break;
              case ModExt: BAsmCode[0]+=0x10; break;
             }
            memcpy(BAsmCode+1,AdrVals,AdrCnt);
            BAsmCode[1+AdrCnt]=HReg;
            CodeLen=2+AdrCnt;
           }
         }
       }
      return;
     }

   /* Spruenge */

   for (z=0; z<BranchOrderCount; z++)
    if Memo(BranchOrders[z].Name)
     {
      if (ArgCnt!=1) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       {
        Address=EvalIntExpression(ArgStr[1],UInt16,&OK)-EProgCounter()-2;
        if (OK)
         if (((Address<-128) || (Address>127)) && (! SymbolQuestionable)) WrError(1370);
         else
          {
           BAsmCode[0]=BranchOrders[z].Code; BAsmCode[1]=Address & 0xff; CodeLen=2;
          }
       }
      return;
     }
    else if ((*OpPart=='L') && (strcmp(OpPart+1,BranchOrders[z].Name)==0))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       {
        Address=EvalIntExpression(ArgStr[1],UInt16,&OK)-EProgCounter()-4;
        if (OK)
         {
          BAsmCode[0]=0x18;
          BAsmCode[1]=BranchOrders[z].Code;
          BAsmCode[2]=(Address >> 8) & 0xff;
          BAsmCode[3]=Address & 0xff;
          CodeLen=4;
         }
       }
      return;
     }

   for (z=0; z<LoopOrderCount; z++)
    if Memo(LoopOrders[z].Name)
     {
      if (ArgCnt!=2) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       {
        if (DecodeReg8(ArgStr[1],&HReg))
         {
          OK=true; if (HReg==2) HReg=4;
         }
        else if (DecodeReg16(ArgStr[1],&HReg))
         {
          OK=(HReg!=PCReg); HReg+=5;
         }
        else OK=false;
        if (! OK) WrXError(1445,ArgStr[1]);
        else
         {
          Address=EvalIntExpression(ArgStr[2],UInt16,&OK)-(EProgCounter()+3);
          if (OK)
           if (((Address<-256) || (Address>255)) && (! SymbolQuestionable)) WrError(1370);
           else
            {
             BAsmCode[0]=0x04;
             BAsmCode[1]=LoopOrders[z].Code+HReg+((Address >> 4) & 0x10);
             BAsmCode[2]=Address & 0xff;
             CodeLen=3;
            }
         }
       }
      return;
     }

   for (z=0; z<JmpOrderCount; z++)
    if (Memo(JmpOrders[z].Name))
     {
      if ((ArgCnt<1) || (ArgCnt>2)) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       {
        Mask=MModAllIdx+MModExt; if (JmpOrders[z].MayDir) Mask+=MModDir;
        ExPos=1; DecodeAdr(1,ArgCnt,Mask);
        if (AdrMode!=ModNone)
         {
          switch (AdrMode)
           {
            case ModExt: BAsmCode[0]=JmpOrders[z].Code; break;
            case ModDir: BAsmCode[0]=JmpOrders[z].Code+1; break;
            case ModIdx:
            case ModIdx1:
            case ModIdx2:
            case ModDIdx:
            case ModIIdx2: BAsmCode[0]=JmpOrders[z].Code-1; break;
           }
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          CodeLen=1+AdrCnt;
         }
       }
      return;
     }

   if (Memo("CALL"))
    {
     if (ArgCnt<1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (*ArgStr[1]=='[')
      {
       if (ArgCnt!=1) WrError(1110);
       else
        {
         ExPos=1; DecodeAdr(1,1,MModDIdx+MModIIdx2);
         if (AdrMode!=ModNone)
          {
           BAsmCode[0]=0x4b;
           memcpy(BAsmCode+1,AdrVals,AdrCnt);
           CodeLen=1+AdrCnt;
          }
        }
      }
     else
      {
       if ((ArgCnt<2) || (ArgCnt>3)) WrError(1110);
       else
        {
         HReg=EvalIntExpression(ArgStr[ArgCnt],UInt8,&OK);
         if (OK)
          {
           ExPos=2; /* wg. Seiten-Byte eins mehr */
           DecodeAdr(1,ArgCnt-1,MModExt+MModIdx+MModIdx1+MModIdx2);
           if (AdrMode!=ModNone)
            {
             BAsmCode[0]=0x4a+(AdrMode!=ModExt);
             memcpy(BAsmCode+1,AdrVals,AdrCnt);
             BAsmCode[1+AdrCnt]=HReg;
             CodeLen=2+AdrCnt;
            }
          }
        }
      }
     return;
    }

   if ((Memo("BRSET")) || (Memo("BRCLR")))
    {
     if (ArgCnt==1)
      {
       Try2Split(1); Try2Split(1);
      }
     else if (ArgCnt==2)
      {
       Try2Split(ArgCnt); Try2Split(2);
      }
     if ((ArgCnt<3) || (ArgCnt>4)) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      {
       if (*ArgStr[ArgCnt-1]=='#') strcpy(ArgStr[ArgCnt-1],ArgStr[ArgCnt-1]+1);
       HReg=EvalIntExpression(ArgStr[ArgCnt-1],UInt8,&OK);
       if (OK)
        {
         Address=EvalIntExpression(ArgStr[ArgCnt],UInt16,&OK)-EProgCounter();
         if (OK)
          {
           ExPos=3; /* Opcode, Maske+Distanz */
           DecodeAdr(1,ArgCnt-2,MModDir+MModExt+MModIdx+MModIdx1+MModIdx2);
           if (AdrMode!=ModNone)
            {
             BAsmCode[0]=0x0e + Memo("BRCLR"); /* ANSI :-O */
             memcpy(BAsmCode+1,AdrVals,AdrCnt);
             switch (AdrMode)
              {
               case ModDir: BAsmCode[0]+=0x40; break;
               case ModExt: BAsmCode[0]+=0x10; break;
              }
             BAsmCode[1+AdrCnt]=HReg;
             Address-=3+AdrCnt;
             if (((Address<-128) || (Address>127)) && (! SymbolQuestionable)) WrError(1370);
             else
              {
               BAsmCode[2+AdrCnt]=Address & 0xff;
               CodeLen=3+AdrCnt;
              }
            }
          }
        }
      }
     return;
    }

   if (Memo("TRAP"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      {
       FirstPassUnknown=false;
       if (*ArgStr[1]=='#') strcpy(ArgStr[1],ArgStr[1]+1);
       BAsmCode[1]=EvalIntExpression(ArgStr[1],UInt8,&OK);
       if (FirstPassUnknown) BAsmCode[1]=0x30;
       if (OK)
        if ((BAsmCode[1]<0x30) || ((BAsmCode[1]>0x39) && (BAsmCode[1]<0x40))) WrError(1320);
        else
         {
          BAsmCode[0]=0x18; CodeLen=2;
         }
      }
     return;
    }

   WrXError(1200,OpPart);
}

        static bool ChkPC_6812(void)
{
   if (ActPC==SegCode) return (ProgCounter()<0x10000);
   else return false;
}

        static bool IsDef_6812(void)
{
   return false;
}

        static void SwitchFrom_6812(void)
{
   DeinitFields();
}

        static void SwitchTo_6812(void)
{
   TurnWords=false; ConstMode=ConstModeMoto; SetIsOccupied=false;

   PCSymbol="*"; HeaderID=0x66; NOPCode=0xa7;
   DivideChars=","; HasAttrs=true; AttrChars=".";

   ValidSegs=(1<<SegCode);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_6812; ChkPC=ChkPC_6812; IsDef=IsDef_6812;
   SwitchFrom=SwitchFrom_6812; InitFields();
}

	void code6812_init(void)
{
   CPU6812=AddCPU("68HC12",SwitchTo_6812);
}
