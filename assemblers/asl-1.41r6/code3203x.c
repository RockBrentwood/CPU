/* code3203x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TMS320C3x-Familie                                           */
/*                                                                           */
/* Historie: 12.12.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "endian.h"
#include "bpemu.h"
#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmcode.h"
#include "codepseudo.h"
#include "codevars.h"


#define ConditionCount 28
#define FixedOrderCount 3
#define RotOrderCount 4
#define StkOrderCount 4
#define GenOrderCount 41
#define ParOrderCount 8
#define SingOrderCount 3

typedef struct
         {
          char *Name;
          Byte Code;
         } Condition;

typedef struct
         {
          char *Name;
          LongWord Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          int NameLen;
          bool May1,May3;
          Byte Code,Code3;
          bool OnlyMem;
          bool SwapOps;
          bool ImmFloat;
          Byte ParMask,Par3Mask;
          Byte PCodes[8],P3Codes[8];
         } GenOrder;

typedef struct
         {
          char *Name;
          LongWord Code;
          Byte Mask;
         } SingOrder;


static CPUVar CPU32030,CPU32031;
static SimpProc SaveInitProc;

static bool NextPar,ThisPar;
static Byte PrevARs,ARs;
static char PrevOp[7];
static Integer z2;
static ShortInt PrevSrc1Mode,PrevSrc2Mode,PrevDestMode;
static ShortInt CurrSrc1Mode,CurrSrc2Mode,CurrDestMode;
static Word PrevSrc1Part,PrevSrc2Part,PrevDestPart;
static Word CurrSrc1Part,CurrSrc2Part,CurrDestPart;

static Condition *Conditions;
static FixedOrder *FixedOrders;
static char **RotOrders;
static char **StkOrders;
static GenOrder *GenOrders;
static char **ParOrders;
static SingOrder *SingOrders;

static LongInt DPValue;

/*-------------------------------------------------------------------------*/
/* Befehlstabellenverwaltung */

	static void AddCondition(char *NName, Byte NCode)
{
   if (InstrZ>=ConditionCount) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
}

	static void AddFixed(char *NName, LongWord NCode)
{
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
}

	static void AddSing(char *NName, LongWord NCode, Byte NMask)
{
   if (InstrZ>=SingOrderCount) exit(255);
   SingOrders[InstrZ].Name=NName;
   SingOrders[InstrZ].Code=NCode;
   SingOrders[InstrZ++].Mask=NMask;
}

	static void AddGen(char *NName, bool NMay1, bool NMay3,
                           Byte NCode, Byte NCode3,
			   bool NOnly, bool NSwap, bool NImm,
                           Byte NMask1, Byte NMask3,
			   Byte C20, Byte C21, Byte C22, Byte C23, Byte C24,
                           Byte C25, Byte C26, Byte C27, Byte C30, Byte C31,
                           Byte C32, Byte C33, Byte C34, Byte C35, Byte C36,
                           Byte C37)
{
   if (InstrZ>=GenOrderCount) exit(255);
   GenOrders[InstrZ].Name=NName;
   GenOrders[InstrZ].NameLen=strlen(NName);
   GenOrders[InstrZ].May1=NMay1; GenOrders[InstrZ].May3=NMay3;
   GenOrders[InstrZ].Code=NCode; GenOrders[InstrZ].Code3=NCode3;
   GenOrders[InstrZ].OnlyMem=NOnly; GenOrders[InstrZ].SwapOps=NSwap;
   GenOrders[InstrZ].ImmFloat=NImm;
   GenOrders[InstrZ].ParMask=NMask1; GenOrders[InstrZ].Par3Mask=NMask3;
   GenOrders[InstrZ].PCodes[0]=C20;  GenOrders[InstrZ].PCodes[1]=C21;
   GenOrders[InstrZ].PCodes[2]=C22;  GenOrders[InstrZ].PCodes[3]=C23;
   GenOrders[InstrZ].PCodes[4]=C24;  GenOrders[InstrZ].PCodes[5]=C25;
   GenOrders[InstrZ].PCodes[6]=C26;  GenOrders[InstrZ].PCodes[7]=C27;
   GenOrders[InstrZ].P3Codes[0]=C30; GenOrders[InstrZ].P3Codes[1]=C31;
   GenOrders[InstrZ].P3Codes[2]=C32; GenOrders[InstrZ].P3Codes[3]=C33;
   GenOrders[InstrZ].P3Codes[4]=C34; GenOrders[InstrZ].P3Codes[5]=C35;
   GenOrders[InstrZ].P3Codes[6]=C36; GenOrders[InstrZ++].P3Codes[7]=C37;
}

	static void InitFields(void)
{
   Conditions=(Condition *) malloc(sizeof(Condition)*ConditionCount); InstrZ=0;
   AddCondition("U"  ,0x00); AddCondition("LO" ,0x01);
   AddCondition("LS" ,0x02); AddCondition("HI" ,0x03);
   AddCondition("HS" ,0x04); AddCondition("EQ" ,0x05);
   AddCondition("NE" ,0x06); AddCondition("LT" ,0x07);
   AddCondition("LE" ,0x08); AddCondition("GT" ,0x09);
   AddCondition("GE" ,0x0a); AddCondition("Z"  ,0x05);
   AddCondition("NZ" ,0x06); AddCondition("P"  ,0x09);
   AddCondition("N"  ,0x07); AddCondition("NN" ,0x0a);
   AddCondition("NV" ,0x0c); AddCondition("V"  ,0x0d);
   AddCondition("NUF",0x0e); AddCondition("UF" ,0x0f);
   AddCondition("NC" ,0x04); AddCondition("C"  ,0x01);
   AddCondition("NLV",0x10); AddCondition("LV" ,0x11);
   AddCondition("NLUF",0x12);AddCondition("LUF",0x13);
   AddCondition("ZUF",0x14); AddCondition(""   ,0x00);

   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("IDLE",0x06000000); AddFixed("SIGI",0x16000000);
   AddFixed("SWI" ,0x66000000);

   RotOrders=(char **) malloc(sizeof(char *)*RotOrderCount); InstrZ=0;
   RotOrders[InstrZ++]="ROL"; RotOrders[InstrZ++]="ROLC";
   RotOrders[InstrZ++]="ROR"; RotOrders[InstrZ++]="RORC";

   StkOrders=(char **) malloc(sizeof(char *)*StkOrderCount); InstrZ=0;
   StkOrders[InstrZ++]="POP";  StkOrders[InstrZ++]="POPF";
   StkOrders[InstrZ++]="PUSH"; StkOrders[InstrZ++]="PUSHF";

   GenOrders=(GenOrder *) malloc(sizeof(GenOrder)*GenOrderCount); InstrZ=0;
/*         Name         May3      Cd3       Swap       PM1                                              PCodes3     */
/*                May1        Cd1     OMem        ImmF     PM3           PCodes1                                    */
   AddGen("ABSF" ,true ,false,0x00,0xff,false,false,true , 4, 0,
	  0xff,0xff,0x04,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("ABSI" ,true ,false,0x01,0xff,false,false,false, 8, 0,
	  0xff,0xff,0xff,0x05,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("ADDC" ,false,true ,0x02,0x00,false,false,false, 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("ADDF" ,false,true ,0x03,0x01,false,false,true , 0, 4,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0x06,0xff,0xff,0xff,0xff,0xff);
   AddGen("ADDI" ,false,true ,0x04,0x02,false,false,false, 0, 8,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x07,0xff,0xff,0xff,0xff);
   AddGen("AND"  ,false,true ,0x05,0x03,false,false,false, 0, 8,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x08,0xff,0xff,0xff,0xff);
   AddGen("ANDN" ,false,true ,0x06,0x04,false,false,false, 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("ASH"  ,false,true ,0x07,0x05,false,false,false, 0, 8,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x09,0xff,0xff,0xff,0xff);
   AddGen("CMPF" ,false,true ,0x08,0x06,false,false,true , 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("CMPI" ,false,true ,0x09,0x07,false,false,false, 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("FIX"  ,true ,false,0x0a,0xff,false,false,true , 8, 0,
	  0xff,0xff,0xff,0x0a,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("FLOAT",true ,false,0x0b,0xff,false,false,false, 4, 0,
	  0xff,0xff,0x0b,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LDE"  ,false,false,0x0d,0xff,false,false,true , 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LDF"  ,false,false,0x0e,0xff,false,false,true , 5, 0,
	  0x02,0xff,0x0c,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LDFI" ,false,false,0x0f,0xff,true ,false,true , 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LDI"  ,false,false,0x10,0xff,false,false,false,10, 0,
	  0xff,0x03,0xff,0x0d,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LDII" ,false,false,0x11,0xff,true ,false,false, 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LDM"  ,false,false,0x12,0xff,false,false,true , 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LSH"  ,false,true ,0x13,0x08,false,false,false, 0, 8,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x0e,0xff,0xff,0xff,0xff);
   AddGen("MPYF" ,false,true ,0x14,0x09,false,false,true , 0,52,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0x0f,0xff,0x00,0x01,0xff,0xff);
   AddGen("MPYI" ,false,true ,0x15,0x0a,false,false,false, 0,200,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x10,0xff,0xff,0x02,0x03);
   AddGen("NEGB" ,true ,false,0x16,0xff,false,false,false, 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("NEGF" ,true ,false,0x17,0xff,false,false,true , 4, 0,
	  0xff,0xff,0x11,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("NEGI" ,true ,false,0x18,0xff,false,false,false, 8, 0,
	  0xff,0xff,0xff,0x12,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("NORM" ,true ,false,0x1a,0xff,false,false,true , 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("NOT"  ,true ,false,0x1b,0xff,false,false,false, 8, 0,
	  0xff,0xff,0xff,0x13,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("OR"   ,false,true ,0x20,0x0b,false,false,false, 0, 8,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x14,0xff,0xff,0xff,0xff);
   AddGen("RND"  ,true ,false,0x22,0xff,false,false,true , 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("STF"  ,false,false,0x28,0xff,true ,true ,true , 4, 0,
	  0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("STFI" ,false,false,0x29,0xff,true ,true ,true , 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("STI"  ,false,false,0x2a,0xff,true ,true ,false, 8, 0,
	  0xff,0xff,0xff,0x01,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("STII" ,false,false,0x2b,0xff,true ,true ,false, 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("SUBB" ,false,true ,0x2d,0x0c,false,false,false, 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("SUBC" ,false,false,0x2e,0xff,false,false,false, 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("SUBF" ,false,true ,0x2f,0x0d,false,false,true , 0, 4,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0x15,0xff,0xff,0xff,0xff,0xff);
   AddGen("SUBI" ,false,true ,0x30,0x0e,false,false,false, 0, 8,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x16,0xff,0xff,0xff,0xff);
   AddGen("SUBRB",false,false,0x31,0xff,false,false,false, 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("SUBRF",false,false,0x32,0xff,false,false,true , 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("SUBRI",false,false,0x33,0xff,false,false,false, 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("TSTB" ,false,true ,0x34,0x0f,false,false,false, 0, 0,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("XOR"  ,false,true ,0x35,0x10,false,false,false, 0, 8,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x17,0xff,0xff,0xff,0xff);

   ParOrders=(char **) malloc(sizeof(char *)*ParOrderCount); InstrZ=0;
   ParOrders[InstrZ++]="LDF";   ParOrders[InstrZ++]="LDI";
   ParOrders[InstrZ++]="STF";   ParOrders[InstrZ++]="STI";
   ParOrders[InstrZ++]="ADDF3"; ParOrders[InstrZ++]="SUBF3";
   ParOrders[InstrZ++]="ADDI3"; ParOrders[InstrZ++]="SUBI3";

   SingOrders=(SingOrder *) malloc(sizeof(SingOrder)*SingOrderCount); InstrZ=0;
   AddSing("IACK",0x1b000000,6);
   AddSing("NOP" ,0x0c800000,5);
   AddSing("RPTS",0x139b0000,15);
}

	static void DeinitFields(void)
{
   free(Conditions);
   free(FixedOrders);
   free(RotOrders);
   free(StkOrders);
   free(GenOrders);
   free(ParOrders);
   free(SingOrders);
}

/*-------------------------------------------------------------------------*/
/* Gleitkommawandler */

	static void SplitExt(Double Inp, LongInt *Expo, LongWord *Mant)
{
   Byte Field[8];
   bool Sign;
   int z;

   memcpy(Field,&Inp,8); if (BigEndian) QSwap(Field,8);
   Sign=(Field[7]>0x7f);
   *Expo=(((LongWord) Field[7]&0x7f)<<4)+(Field[6]>>4);
   *Mant=Field[6]&0x0f; if (*Expo!=0) *Mant|=0x10;
   for (z=5; z>2; z--) *Mant=((*Mant)<<8)|Field[z];
   *Mant=((*Mant)<<3)+(Field[2]>>5);
   *Expo-=0x3ff;
   if (Sign) *Mant=0xffffffff-(*Mant);
   *Mant=(*Mant)^0x80000000;
}

	static bool ExtToShort(Double Inp, Word *Erg)
{
   LongInt Expo;
   LongWord Mant;

   if (Inp==0) *Erg=0x8000;
   else
    {
     SplitExt(Inp,&Expo,&Mant);
     if (abs(Expo)>7)
      {
       WrError((Expo>0)?1320:1315);
       return false;
      }
     *Erg=((Expo << 12) & 0xf000) | ((Mant >> 20) & 0xfff);
    }
   return true;
}

	static bool ExtToSingle(Double Inp, LongWord *Erg)
{
   LongInt Expo;
   LongWord Mant;

   if (Inp==0) *Erg=0x80000000;
   else
    {
     SplitExt(Inp,&Expo,&Mant);
     if (abs(Expo)>127)
      {
       WrError((Expo>0)?1320:1315);
       return false;
      }
     *Erg=((Expo << 24) & 0xff000000)+(Mant >> 8);
    }
   return true;
}

	static bool ExtToExt(Double Inp, LongWord *ErgL, LongWord *ErgH)
{
   LongInt Exp;

   if (Inp==0)
    {
     *ErgH=0x80; *ErgL=0x00000000;
    }
   else
    {
     SplitExt(Inp,&Exp,ErgL);
     if (abs(Exp)>127)
      {
       WrError((Exp>0)?1320:1315);
       return false;
      }
     *ErgH=Exp&0xff;
    }
   return true;
}

/*-------------------------------------------------------------------------*/
/* Adressparser */

#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModDir 1
#define MModDir (1 << ModDir)
#define ModInd 2
#define MModInd (1 << ModInd)
#define ModImm 3
#define MModImm (1 << ModImm)

static ShortInt AdrMode;
static LongInt AdrPart;

	static bool DecodeReg(char *Asc, Byte *Erg)
{
#define RegCnt 12
#define RegStart 0x10
    static char *Regs[RegCnt]=
	{"DP","IR0","IR1","BK","SP","ST","IE","IF","IOF","RS","RE","RC"};
    bool Err;

   if ((toupper(*Asc)=='R') && (strlen(Asc)<=3) && (strlen(Asc)>=2))
    {
     *Erg=ConstLongInt(Asc+1,&Err);
     if ((Err) && (*Erg<=0x1b)) return true;
    }

   if ((strlen(Asc)==3) && (toupper(*Asc)=='A') && (toupper(Asc[1])=='R') && (Asc[2]>='0') && (Asc[2]<='7'))
    {
     *Erg=Asc[2]-'0'+8; return true;
    }

   *Erg=0;
   while ((*Erg<RegCnt) && (strcasecmp(Regs[*Erg],Asc)!=0)) (*Erg)++;
   if (*Erg<RegCnt)
    {
     *Erg+=RegStart; return true;
    }

   return false;
}

	static void ChkAdr(Byte Erl)
{
   if ((AdrMode!=ModNone) && ((Erl & (1 << AdrMode))==0))
    {
     AdrMode=ModNone; WrError(1350);
    }
}

	static void DecodeAdr(char *Asc, Byte Erl, bool ImmFloat)
{
   Byte HReg;
   Integer Disp;
   char *p;
   int l;
   Double f;
   Word fi;
   LongInt AdrLong;
   bool BitRev,Circ;
   String NDisp;
   bool OK;
   enum {ModBase,ModAdd,ModSub,ModPreInc,ModPreDec,ModPostInc,ModPostDec} Mode;

   KillBlanks(Asc);

   AdrMode=ModNone;

   /* I. Register? */

   if (DecodeReg(Asc,&HReg))
    {
     AdrMode=ModReg; AdrPart=HReg; ChkAdr(Erl); return;
    }

   /* II. indirekt ? */

   if (*Asc=='*')
    {
     /* II.1. Erkennungszeichen entfernen */

     strcpy(Asc,Asc+1);

     /* II.2. Extrawuerste erledigen */

     BitRev=false; Circ=false;
     if (toupper(Asc[strlen(Asc)-1])=='B')
      {
       BitRev=true; Asc[strlen(Asc)-1]='\0';
      }
     else if (Asc[strlen(Asc)-1]=='%')
      {
       Circ=true; Asc[strlen(Asc)-1]='\0';
      }

     /* II.3. Displacement entfernen und auswerten:
	     0..255-->Displacement
	     -1,-2 -->IR0,IR1
	     -3    -->Default */

     p=QuotPos(Asc,'(');
     if (p!=NULL)
      {
       if (Asc[strlen(Asc)-1]!=')')
	{
	 WrError(1350); return;
	}
       *p='\0'; strmaxcpy(NDisp,p+1,255); NDisp[strlen(NDisp)-1]='\0';
       if (strcasecmp(NDisp,"IR0")==0) Disp=(-1);
       else if (strcasecmp(NDisp,"IR1")==0) Disp=(-2);
       else
	{
	 Disp=EvalIntExpression(NDisp,UInt8,&OK);
	 if (! OK) return;
	}
      }
     else Disp=(-3);

     /* II.4. Addieren/Subtrahieren mit/ohne Update? */

     l=strlen(Asc);
     if (*Asc=='-')
      {
       if (Asc[1]=='-')
	{
	 Mode=ModPreDec; strcpy(Asc,Asc+2);
	}
       else
	{
	 Mode=ModSub; strcpy(Asc,Asc+1);
	}
      }
     else if (*Asc=='+')
      {
       if (Asc[1]=='+')
	{
	 Mode=ModPreInc; strcpy(Asc,Asc+2);
	}
       else
	{
	 Mode=ModAdd; strcpy(Asc,Asc+1);
	}
      }
     else if (Asc[l-1]=='-')
      {
       if (Asc[l-2]=='-')
	{
	 Mode=ModPostDec; Asc[l-2]='\0';
	}
       else
	{
	 WrError(1350); return;
	}
      }
     else if (Asc[l-1]=='+')
      {
       if (Asc[l-2]=='+')
	{
	 Mode=ModPostInc; Asc[l-2]='\0';
	}
       else
	{
	 WrError(1350); return;
	}
      }
     else Mode=ModBase;

     /* II.5. Rest muss Basisregister sein */

     if ((! DecodeReg(Asc,&HReg)) || (HReg<8) || (HReg>15))
      {
       WrError(1350); return;
      }
     HReg-=8;
     if ((ARs & (1l << HReg))==0) ARs+=1l << HReg;
     else WrXError(210,Asc);

     /* II.6. Default-Displacement explizit machen */

     if (Disp==-3)
      Disp=(Mode==ModBase) ? 0 : 1;

     /* II.7. Entscheidungsbaum */

     switch (Mode)
      {
       case ModBase:
       case ModAdd:
        if ((Circ) || (BitRev)) WrError(1350);
        else
         {
	  switch (Disp)
           {
	    case -2: AdrPart=0x8000; break;
	    case -1: AdrPart=0x4000; break;
	    case  0: AdrPart=0xc000; break;
	    default: AdrPart=Disp;
	   }
	  AdrPart+=((Word)HReg) << 8; AdrMode=ModInd;
         }
        break;
       case ModSub:
        if ((Circ) || (BitRev)) WrError(1350);
        else
         {
	  switch (Disp)
           {
	    case -2: AdrPart=0x8800; break;
	    case -1: AdrPart=0x4800; break;
	    case  0: AdrPart=0xc000; break;
	    default: AdrPart=0x0800+Disp;
	   }
	  AdrPart+=((Word)HReg) << 8; AdrMode=ModInd;
         }
        break;
       case ModPreInc:
        if ((Circ) || (BitRev)) WrError(1350);
        else
         {
	  switch (Disp)
           {
	    case -2: AdrPart=0x9000; break;
	    case -1: AdrPart=0x5000; break;
	    default: AdrPart=0x1000+Disp;
	   }
	  AdrPart+=((Word)HReg) << 8; AdrMode=ModInd;
         }
        break;
       case ModPreDec:
        if ((Circ) || (BitRev)) WrError(1350);
        else
         {
	  switch (Disp)
           {
	    case -2: AdrPart=0x9800; break;
	    case -1: AdrPart=0x5800; break;
	    default: AdrPart=0x1800+Disp;
	   }
	  AdrPart+=((Word)HReg) << 8; AdrMode=ModInd;
         }
        break;
       case ModPostInc:
        if (BitRev)
         {
  	  if (Disp!=-1) WrError(1350);
 	  else
 	   {
 	    AdrPart=0xc800+(((Word)HReg) << 8); AdrMode=ModInd;
 	   }
         }
        else
         {
 	  switch (Disp)
           {
 	    case -2: AdrPart=0xa000; break;
 	    case -1: AdrPart=0x6000; break;
 	    default: AdrPart=0x2000+Disp;
 	   }
 	  if (Circ) AdrPart+=0x1000;
 	  AdrPart+=((Word)HReg) << 8; AdrMode=ModInd;
         }
        break;
       case ModPostDec:
        if (BitRev) WrError(1350);
        else
         {
  	  switch (Disp)
           {
 	    case -2: AdrPart=0xa800; break;
 	    case -1: AdrPart=0x6800; break;
 	    default: AdrPart=0x2800+Disp; break;
 	   }
 	  if (Circ) AdrPart+=0x1000;
 	  AdrPart+=((Word)HReg) << 8; AdrMode=ModInd;
         }
        break;
      }

     ChkAdr(Erl); return;
    }

   /* III. absolut */

   if (*Asc=='@')
    {
     AdrLong=EvalIntExpression(Asc+1,UInt24,&OK);
     if (OK)
      {
       if ((DPValue!=-1) && ((AdrLong >> 16)!=DPValue)) WrError(110);
       AdrMode=ModDir; AdrPart=AdrLong & 0xffff;
      }
     ChkAdr(Erl); return;
    }

   /* IV. immediate */

   if (ImmFloat)
    {
     f=EvalFloatExpression(Asc,Float64,&OK);
     if (OK)
      if (ExtToShort(f,&fi))
       {
	AdrPart=fi; AdrMode=ModImm;
       }
    }
   else
    {
     AdrPart=EvalIntExpression(Asc,Int16,&OK);
     if (OK)
      {
       AdrPart&=0xffff; AdrMode=ModImm;
      }
    }

   ChkAdr(Erl);
}

	static Word EffPart(Byte Mode, Word Part)
{
   switch (Mode)
    {
     case ModReg: return Lo(Part);
     case ModInd: return Hi(Part);
     default: WrError(10000); return 0;
    }
}

/*-------------------------------------------------------------------------*/
/* Code-Erzeugung */

	static bool DecodePseudo(void)
{
#define ASSUME3203Count 1
   static ASSUMERec ASSUME3203s[ASSUME3203Count]=
                 {{"DP", &DPValue, -1, 0xff, 0x100}};

   bool OK;
   Integer z,z2;
   LongInt Size;
   Double f;
   TempResult t;

   if (Memo("ASSUME"))
    {
     CodeASSUME(ASSUME3203s,ASSUME3203Count);
     return true;
    }

   if (Memo("SINGLE"))
    {
     if (ArgCnt==0) WrError(1110);
     else
      {
       OK=true;
       for (z=1; z<=ArgCnt; z++)
	if (OK)
	 {
	  f=EvalFloatExpression(ArgStr[z],Float64,&OK);
	  if (OK)
	   OK=OK && ExtToSingle(f,DAsmCode+(CodeLen++));
	 }
       if (! OK) CodeLen=0;
      }
     return true;
    }

   if (Memo("EXTENDED"))
    {
     if (ArgCnt==0) WrError(1110);
     else
      {
       OK=true;
       for (z=1; z<=ArgCnt; z++)
	if (OK)
	 {
	  f=EvalFloatExpression(ArgStr[z],Float64,&OK);
	  if (OK)
	   OK=OK && ExtToExt(f,DAsmCode+CodeLen+1,DAsmCode+CodeLen);
	  CodeLen+=2;
	 }
       if (! OK) CodeLen=0;
      }
     return true;
    }

   if (Memo("WORD"))
    {
     if (ArgCnt==0) WrError(1110);
     else
      {
       OK=true;
       for (z=1; z<=ArgCnt; z++)
	if (OK) DAsmCode[CodeLen++]=EvalIntExpression(ArgStr[z],Int32,&OK);
       if (! OK) CodeLen=0;
      }
     return true;
    }

   if (Memo("DATA"))
    {
     if (ArgCnt==0) WrError(1110);
     else
      {
       OK=true;
       for (z=1; z<=ArgCnt; z++)
	if (OK)
	 {
	  EvalExpression(ArgStr[z],&t);
	  switch (t.Typ)
           {
	    case TempInt:
#ifdef HAS64
             if (! RangeCheck(t.Contents.Int,Int32))
              {
               OK=false; WrError(1320);
              }
             else
#endif
	      DAsmCode[CodeLen++]=t.Contents.Int;
	     break;
	    case TempFloat:
	     if (! ExtToSingle(t.Contents.Float,DAsmCode+(CodeLen++))) OK=false;
             break;
	    case TempString:
	     for (z2=0; z2<strlen(t.Contents.Ascii); z2++)
	      {
	       if ((z2 & 3)==0) DAsmCode[CodeLen++]=0;
	       DAsmCode[CodeLen-1]+=
		  (((LongWord)CharTransTable[(int)t.Contents.Ascii[z2]])) << (8*(3-(z2 & 3)));
	      }
	     break;
	    case TempNone:
             OK=false;
	   }
	 }
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
       Size=EvalIntExpression(ArgStr[1],UInt24,&OK);
       if (FirstPassUnknown) WrError(1820);
       if ((OK) && (! FirstPassUnknown))
	{
	 DontPrint=true;
	 CodeLen=Size;
	 if (MakeUseList)
	  if (AddChunk(SegChunks+ActPC,ProgCounter(),CodeLen,ActPC==SegCode)) WrError(90);
	}
      }
     return true;
    }

   return false;
}

	static void JudgePar(GenOrder *Prim, Integer Sec, Byte *ErgMode, Byte *ErgCode)
{
   if (Sec>3) *ErgMode=3;
   else if (Prim->May3) *ErgMode=1;
   else *ErgMode=2;
   if (*ErgMode==2) *ErgCode=Prim->PCodes[Sec];
	       else *ErgCode=Prim->P3Codes[Sec];
}

	static LongWord EvalAdrExpression(char *Asc, bool *OK)
{
   if (*Asc=='@') strcpy(Asc,Asc+1);
   return EvalIntExpression(Asc,UInt24,OK);
}

	static void SwapMode(ShortInt *M1, ShortInt *M2)
{
   AdrMode=(*M1); *M1=(*M2); *M2=AdrMode;
}

	static void SwapPart(Word *P1, Word *P2)
{
   AdrPart=(*P1); *P1=(*P2); *P2=AdrPart;
}

	static void MakeCode_3203X(void)
{
   bool OK,Is3;
   Byte HReg,HReg2,Sum;
   Integer z,z3;
   LongInt AdrLong,DFlag,Disp;
   String HOp,Form;

   CodeLen=0; DontPrint=false;

   ThisPar=(strcmp(LabPart,"||")==0);
   if ((strlen(OpPart)>2) && (strncmp(OpPart,"||",2)==0))
    {
     ThisPar=true; strcpy(OpPart,OpPart+2);
    }
   if ((! NextPar) && (ThisPar))
    {
     WrError(1950); return;
    }
   ARs=0;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     {
      if (ArgCnt!=0) WrError(1110);
      else if (ThisPar) WrError(1950);
      else
       {
        DAsmCode[0]=FixedOrders[z].Code; CodeLen=1;
       }
      NextPar=false; return;
     }

   /* Arithmetik/Logik */

   for (z=0; z<GenOrderCount; z++)
    if ((strncmp(OpPart,GenOrders[z].Name,GenOrders[z].NameLen)==0)
    && ((OpPart[GenOrders[z].NameLen]=='\0') || (OpPart[GenOrders[z].NameLen]=='3')))
     {
      NextPar=false;
      /* Argumentzahl abgleichen */
      if (ArgCnt==1)
       if (GenOrders[z].May1)
        {
         ArgCnt=2; strcpy(ArgStr[2],ArgStr[1]);
        }
       else
        {
         WrError(1110); return;
        }
      if ((ArgCnt==3) && (OpPart[strlen(OpPart)-1]!='3')) strcat(OpPart,"3");
      Is3=(OpPart[strlen(OpPart)-1]=='3');
      if ((GenOrders[z].SwapOps) && (! Is3))
       {
        strcpy(ArgStr[3],ArgStr[1]);
        strcpy(ArgStr[1],ArgStr[2]);
        strcpy(ArgStr[2],ArgStr[3]);
       }
      if ((Is3) && (ArgCnt==2))
       {
        ArgCnt=3; strcpy(ArgStr[3],ArgStr[2]);
       }
      if ((ArgCnt<2) || (ArgCnt>3) || ((Is3) && (! GenOrders[z].May3)))
       {
        WrError(1110); return;
       }
      /* Argumente parsen */
      if (Is3)
       {
        if (Memo("TSTB3"))
         {
          CurrDestMode=ModReg; CurrDestPart=0;
         }
        else
         {
          DecodeAdr(ArgStr[3],MModReg,GenOrders[z].ImmFloat);
          if (AdrMode==ModNone) return;
          CurrDestMode=AdrMode; CurrDestPart=AdrPart;
         }
        DecodeAdr(ArgStr[2],MModReg+MModInd,GenOrders[z].ImmFloat);
        if (AdrMode==ModNone) return;
        if ((AdrMode==ModInd) && ((AdrPart & 0xe000)==0) && (Lo(AdrPart)!=1))
         {
          WrError(1350); return;
         }
        CurrSrc2Mode=AdrMode; CurrSrc2Part=AdrPart;
        DecodeAdr(ArgStr[1],MModReg+MModInd,GenOrders[z].ImmFloat);
        if (AdrMode==ModNone) return;
        if ((AdrMode==ModInd) && ((AdrPart & 0xe000)==0) && (Lo(AdrPart)!=1))
         {
          WrError(1350); return;
         }
        CurrSrc1Mode=AdrMode; CurrSrc1Part=AdrPart;
       }
      else /* ! Is3 */
       {
        DecodeAdr(ArgStr[1],MModDir+MModInd+((GenOrders[z].OnlyMem)?0:MModReg+MModImm),GenOrders[z].ImmFloat);
        if (AdrMode==ModNone) return;
        CurrSrc1Mode=AdrMode; CurrSrc1Part=AdrPart;
        DecodeAdr(ArgStr[2],MModReg+MModInd,GenOrders[z].ImmFloat);
        switch (AdrMode)
         {
          case ModReg:
           CurrDestMode=AdrMode; CurrDestPart=AdrPart;
           CurrSrc2Mode=CurrSrc1Mode; CurrSrc2Part=CurrSrc1Part;
           break;
          case ModInd:
           if (((strcmp(OpPart,"TSTB")!=0) && (strcmp(OpPart,"CMPI")!=0) && (strcmp(OpPart,"CMPF")!=0))
           ||  ((CurrSrc1Mode==ModDir) || (CurrSrc1Mode==ModImm))
           ||  ((CurrSrc1Mode==ModInd) && ((CurrSrc1Part & 0xe000)==0) && (Lo(CurrSrc1Part)!=1))
           ||  (((AdrPart & 0xe000)==0) && (Lo(AdrPart)!=1)))
            {
             WrError(1350); return;
            }
           else
            {
             Is3=true; CurrDestMode=ModReg; CurrDestPart=0;
             CurrSrc2Mode=AdrMode; CurrSrc2Part=AdrPart;
            }
           break;
          case ModNone:
           return;
         }
       }
      /* auswerten: parallel... */
      if (ThisPar)
       {
        /* in Standardreihenfolge suchen */
        if (PrevOp[strlen(PrevOp)-1]=='3') HReg=GenOrders[z2].Par3Mask;
        else HReg=GenOrders[z2].ParMask;
        z3=0;
        while ((z3<ParOrderCount) && ((! Odd(HReg)) || (strcmp(ParOrders[z3],OpPart)!=0)))
         {
          z3++; HReg>>=1;
         }
        if (z3<ParOrderCount) JudgePar(GenOrders+z2,z3,&HReg,&HReg2);
        /* in gedrehter Reihenfolge suchen */
        else
         {
          if (OpPart[strlen(OpPart)-1]=='3') HReg=GenOrders[z].Par3Mask;
          else HReg=GenOrders[z].ParMask;
          z3=0;
          while ((z3<ParOrderCount) && ((! Odd(HReg)) || (strcmp(ParOrders[z3],PrevOp)!=0)))
           {
            z3++; HReg>>=1;
           }
          if (z3<ParOrderCount)
           {
            JudgePar(GenOrders+z,z3,&HReg,&HReg2);
            SwapMode(&CurrDestMode,&PrevDestMode);
            SwapMode(&CurrSrc1Mode,&PrevSrc1Mode);
            SwapMode(&CurrSrc2Mode,&PrevSrc2Mode);
            SwapPart(&CurrDestPart,&PrevDestPart);
            SwapPart(&CurrSrc1Part,&PrevSrc1Part);
            SwapPart(&CurrSrc2Part,&PrevSrc2Part);
           }
          else
           {
            WrError(1950); return;
           }
         }
        /* mehrfache Registernutzung ? */
        for (z3=0; z3<8; z3++)
         if ((ARs & PrevARs & (1l << z3))!=0)
          {
           sprintf(Form,"AR%d",z3); WrXError(210,Form);
          }
        /* 3 Basisfaelle */
        switch (HReg)
         {
          case 1:
           if ((strcmp(PrevOp,"LSH3")==0) || (strcmp(PrevOp,"ASH3")==0) || (strcmp(PrevOp,"SUBF3")==0) || (strcmp(PrevOp,"SUBI3")==0))
            {
             SwapMode(&PrevSrc1Mode,&PrevSrc2Mode);
             SwapPart(&PrevSrc1Part,&PrevSrc2Part);
            }
           if ((PrevDestPart>7) || (CurrDestPart>7))
            {
             WrError(1445); return;
            }
           /* Bei Addition und Multiplikation Kommutativitaet nutzen */
           if  ((PrevSrc2Mode==ModInd) && (PrevSrc1Mode==ModReg)
            && ((strncmp(PrevOp,"ADD",3)==0) || (strncmp(PrevOp,"MPY",3)==0)
              || (strncmp(PrevOp,"AND",3)==0) || (strncmp(PrevOp,"XOR",3)==0)
              || (strncmp(PrevOp,"OR",2)==0)))
            {
             SwapMode(&PrevSrc1Mode,&PrevSrc2Mode);
             SwapPart(&PrevSrc1Part,&PrevSrc2Part);
            }
           if ((PrevSrc2Mode!=ModReg) || (PrevSrc2Part>7)
            || (PrevSrc1Mode!=ModInd) || (CurrSrc1Mode!=ModInd))
            {
             WrError(1355); return;
            }
           RetractWords(1);
           DAsmCode[0]=0xc0000000+(((LongWord)HReg2) << 25)
		      +(((LongWord)PrevDestPart) << 22)
		      +(((LongWord)PrevSrc2Part) << 19)
		      +(((LongWord)CurrDestPart) << 16)
		      +(CurrSrc1Part & 0xff00)+Hi(PrevSrc1Part);
           CodeLen=1; NextPar=false;
           break;
          case 2:
           if ((PrevDestPart>7) || (CurrDestPart>7))
            {
             WrError(1445); return;
            }
           if ((PrevSrc1Mode!=ModInd) || (CurrSrc1Mode!=ModInd))
            {
             WrError(1355); return;
            }
           RetractWords(1);
           DAsmCode[0]=0xc0000000+(((LongWord)HReg2) << 25)
		      +(((LongWord)PrevDestPart) << 22)
		      +(CurrSrc1Part & 0xff00)+Hi(PrevSrc1Part);
           if ((strcmp(PrevOp,OpPart)==0) && (*OpPart=='L'))
            {
             DAsmCode[0]+=((LongWord)CurrDestPart) << 19;
             if (PrevDestPart==CurrDestPart) WrError(140);
            }
           else
            DAsmCode[0]+=((LongWord)CurrDestPart) << 16;
           CodeLen=1; NextPar=false;
           break;
          case 3:
           if ((PrevDestPart>1) || (CurrDestPart<2) || (CurrDestPart>3))
            {
             WrError(1445); return;
            }
           Sum=0;
           if (PrevSrc1Mode==ModInd) Sum++;
           if (PrevSrc2Mode==ModInd) Sum++;
           if (CurrSrc1Mode==ModInd) Sum++;
           if (CurrSrc2Mode==ModInd) Sum++;
           if (Sum!=2)
            {
             WrError(1355); return;
            }
           RetractWords(1);
           DAsmCode[0]=0x80000000+(((LongWord)HReg2) << 26)
	     	      +(((LongWord)PrevDestPart & 1) << 23)
		      +(((LongWord)CurrDestPart & 1) << 22);
           CodeLen=1;
           if (CurrSrc2Mode==ModReg)
            if (CurrSrc1Mode==ModReg)
             {
              DAsmCode[0]+=((LongWord)0x00000000)
                          +(((LongWord)CurrSrc2Part) << 19)
                          +(((LongWord)CurrSrc1Part) << 16)
                          +(PrevSrc2Part & 0xff00)+Hi(PrevSrc1Part);
             }
            else
             {
              DAsmCode[0]+=((LongWord)0x03000000)
                          +(((LongWord)CurrSrc2Part) << 16)
                          +Hi(CurrSrc1Part);
              if (PrevSrc1Mode==ModReg)
               DAsmCode[0]+=(((LongWord)PrevSrc1Part) << 19)+(PrevSrc2Part & 0xff00);
              else
               DAsmCode[0]+=(((LongWord)PrevSrc2Part) << 19)+(PrevSrc1Part & 0xff00);
             }
           else
            if (CurrSrc1Mode==ModReg)
             {
              DAsmCode[0]+=((LongWord)0x01000000)
                          +(((LongWord)CurrSrc1Part) << 16)
                          +Hi(CurrSrc2Part);
              if (PrevSrc1Mode==ModReg)
               DAsmCode[0]+=(((LongWord)PrevSrc1Part) << 19)+(PrevSrc2Part & 0xff00);
              else
               DAsmCode[0]+=(((LongWord)PrevSrc2Part) << 19)+(PrevSrc1Part & 0xff00);
             }
            else
             {
              DAsmCode[0]+=((LongWord)0x02000000)
                          +(((LongWord)PrevSrc2Part) << 19)
                          +(((LongWord)PrevSrc1Part) << 16)
                          +(CurrSrc2Part & 0xff00)+Hi(CurrSrc1Part);
             }
           break;
         }
       }
      /* ...sequentiell */
      else
       {
        PrevSrc1Mode=CurrSrc1Mode; PrevSrc1Part=CurrSrc1Part;
        PrevSrc2Mode=CurrSrc2Mode; PrevSrc2Part=CurrSrc2Part;
        PrevDestMode=CurrDestMode; PrevDestPart=CurrDestPart;
        strcpy(PrevOp,OpPart); PrevARs=ARs; z2=z;
        if (Is3)
         DAsmCode[0]=0x20000000+(((LongWord)GenOrders[z].Code3) << 23)
                    +(((LongWord)CurrDestPart) << 16)
       	            +(((LongWord)CurrSrc2Mode) << 20)+(EffPart(CurrSrc2Mode,CurrSrc2Part) << 8)
       	            +(((LongWord)CurrSrc1Mode) << 21)+EffPart(CurrSrc1Mode,CurrSrc1Part);
        else
         DAsmCode[0]=0x00000000+(((LongWord)GenOrders[z].Code) << 23)
       	            +(((LongWord)CurrSrc1Mode) << 21)+CurrSrc1Part
       	            +(((LongWord)CurrDestPart) << 16);
        CodeLen=1; NextPar=true;
       }
      return;
     }

   for (z=0; z<RotOrderCount; z++)
    if (Memo(RotOrders[z]))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (ThisPar) WrError(1950);
      else if (! DecodeReg(ArgStr[1],&HReg)) WrError(1350);
      else
       {
	DAsmCode[0]=0x11e00000+(((LongWord)z) << 23)+(((LongWord)HReg) << 16);
	CodeLen=1;
       }
      NextPar=false; return;
     }

   for (z=0; z<StkOrderCount; z++)
    if (Memo(StkOrders[z]))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (ThisPar) WrError(1950);
      else if (! DecodeReg(ArgStr[1],&HReg)) WrError(1350);
      else
       {
	DAsmCode[0]=0x0e200000+(((LongWord)z) << 23)+(((LongWord)HReg) << 16);
	CodeLen=1;
       }
      NextPar=false; return;
     }

   /* Datentransfer */

   if ((strncmp(OpPart,"LDI",3)==0) || (strncmp(OpPart,"LDF",3)==0))
    {
     strcpy(HOp,OpPart); strcpy(OpPart,OpPart+3);
     for (z=0; z<ConditionCount; z++)
      if (Memo(Conditions[z].Name))
       {
	if (ArgCnt!=2) WrError(1110);
	else if (ThisPar) WrError(1950);
	else
	 {
	  DecodeAdr(ArgStr[2],MModReg,false);
	  if (AdrMode!=ModNone)
	   {
	    HReg=AdrPart;
	    DecodeAdr(ArgStr[1],MModReg+MModDir+MModInd+MModImm,HOp[2]=='F');
	    if (AdrMode!=ModNone)
	     {
	      DAsmCode[0]=0x40000000+(((LongWord)HReg) << 16)
			 +(((LongWord)Conditions[z].Code) << 23)
			 +(((LongWord)AdrMode) << 21)+AdrPart;
	      if (HOp[2]=='I') DAsmCode[0]+=0x10000000;
	      CodeLen=1;
	     }
	   }
	 }
	NextPar=false; return;
       }
     WrXError(1200,HOp); NextPar=false; return;
    }

   /* Sonderfall NOP auch ohne Argumente */

   if ((Memo("NOP")) && (ArgCnt==0))
    {
     CodeLen=1; DAsmCode[0]=NOPCode; return;
    }

   /* Sonderfaelle */

   for (z=0; z<SingOrderCount; z++)
    if (Memo(SingOrders[z].Name))
     {
      if (ArgCnt!=1) WrError(1110);
      else if (ThisPar) WrError(1950);
      else
       {
	DecodeAdr(ArgStr[1],SingOrders[z].Mask,false);
	if (AdrMode!=ModNone)
	 {
	  DAsmCode[0]=SingOrders[z].Code+(((LongWord)AdrMode) << 21)+AdrPart;
	  CodeLen=1;
	 }
       };
      NextPar=false; return;
     }

   if (Memo("LDP"))
    {
     if ((ArgCnt!=1) && (ArgCnt!=2)) WrError(1110);
     else if (ThisPar) WrError(1950);
     else if ((ArgCnt==2) && (strcasecmp(ArgStr[2],"DP")!=0)) WrError(1350);
     else
      {
       AdrLong=EvalAdrExpression(ArgStr[1],&OK);
       if (OK)
	{
	 DAsmCode[0]=0x08700000+(AdrLong >> 16);
	 CodeLen=1;
	}
      }
     NextPar=false; return;
    }

   /* Schleifen */

   if (Memo("RPTB"))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (ThisPar) WrError(1950);
     else
      {
       AdrLong=EvalAdrExpression(ArgStr[1],&OK);
       if (OK)
	{
	 DAsmCode[0]=0x64000000+AdrLong;
	 CodeLen=1;
	}
      }
     NextPar=false; return;
    }

   /* Spruenge */

   if ((Memo("BR")) || (Memo("BRD")) || (Memo("CALL")))
    {
     if (ArgCnt!=1) WrError(1110);
     else if (ThisPar) WrError(1950);
     else
      {
       AdrLong=EvalAdrExpression(ArgStr[1],&OK);
       if (OK)
	{
	 DAsmCode[0]=0x60000000+AdrLong;
	 if (Memo("BRD")) DAsmCode[0]+=0x01000000;
	 else if (Memo("CALL")) DAsmCode[0]+=0x02000000;
	 CodeLen=1;
	}
      }
     NextPar=false; return;
    }

   if (*OpPart=='B')
    {
     strcpy(HOp,OpPart);
     strcpy(OpPart,OpPart+1);
     if (OpPart[strlen(OpPart)-1]=='D')
      {
       OpPart[strlen(OpPart)-1]='\0'; DFlag=1l << 21;
       Disp=3;
      }
     else
      {
       DFlag=0; Disp=1;
      }
     for (z=0; z<ConditionCount; z++)
      if (Memo(Conditions[z].Name))
       {
	if (ArgCnt!=1) WrError(1110);
        else if (ThisPar) WrError(1950);
	else if (DecodeReg(ArgStr[1],&HReg))
	 {
	  DAsmCode[0]=0x68000000+(((LongWord)Conditions[z].Code) << 16)+DFlag+HReg;
	  CodeLen=1;
	 }
	else
	 {
          AdrLong=EvalAdrExpression(ArgStr[1],&OK)-(EProgCounter()+Disp);
	  if (OK)
	   if ((! SymbolQuestionable) && ((AdrLong>0x7fffl) || (AdrLong<-0x8000l))) WrError(1370);
	   else
	    {
	     DAsmCode[0]=0x6a000000+(((LongWord)Conditions[z].Code) << 16)+DFlag+(AdrLong & 0xffff);
	     CodeLen=1;
	    }
	 }
	NextPar=false; return;
       }
     WrXError(1200,HOp); NextPar=false; return;
    }

   if (strncmp(OpPart,"CALL",4)==0)
    {
     strcpy(HOp,OpPart); strcpy(OpPart,OpPart+4);
     for (z=0; z<ConditionCount; z++)
      if (Memo(Conditions[z].Name))
       {
	if (ArgCnt!=1) WrError(1110);
	else if (ThisPar) WrError(1950);
	else if (DecodeReg(ArgStr[1],&HReg))
	 {
	  DAsmCode[0]=0x70000000+(((LongWord)Conditions[z].Code) << 16)+HReg;
	  CodeLen=1;
	 }
	else
	 {
	  AdrLong=EvalAdrExpression(ArgStr[1],&OK)-(EProgCounter()+1);
	  if (OK)
	   if ((! SymbolQuestionable) && ((AdrLong>0x7fffl) || (AdrLong<-0x8000l))) WrError(1370);
	   else
	    {
	     DAsmCode[0]=0x72000000+(((LongWord)Conditions[z].Code) << 16)+(AdrLong & 0xffff);
	     CodeLen=1;
	    }
	 }
	NextPar=false; return;
       }
     WrXError(1200,HOp); NextPar=false; return;
    }

   if (strncmp(OpPart,"DB",2)==0)
    {
     strcpy(HOp,OpPart);
     strcpy(OpPart,OpPart+2);
     if (OpPart[strlen(OpPart)-1]=='D')
      {
       OpPart[strlen(OpPart)-1]='\0'; DFlag=1l << 21;
       Disp=3;
      }
     else
      {
       DFlag=0; Disp=1;
      }
     for (z=0; z<ConditionCount; z++)
      if (Memo(Conditions[z].Name))
       {
	if (ArgCnt!=2) WrError(1110);
	else if (ThisPar) WrError(1950);
	else if (! DecodeReg(ArgStr[1],&HReg2)) WrError(1350);
	else if ((HReg2<8) || (HReg2>15)) WrError(1350);
	else
	 {
	  HReg2-=8;
	  if (DecodeReg(ArgStr[2],&HReg))
	   {
	    DAsmCode[0]=0x6c000000
		       +(((LongWord)Conditions[z].Code) << 16)
		       +DFlag
		       +(((LongWord)HReg2) << 22)
		       +HReg;
	    CodeLen=1;
	   }
	  else
	   {
            AdrLong=EvalAdrExpression(ArgStr[2],&OK)-(EProgCounter()+Disp);
	    if (OK)
	     if ((! SymbolQuestionable) && ((AdrLong>0x7fffl) || (AdrLong<-0x8000l))) WrError(1370);
	     else
	      {
	       DAsmCode[0]=0x6e000000
			  +(((LongWord)Conditions[z].Code) << 16)
			  +DFlag
			  +(((LongWord)HReg2) << 22)
			  +(AdrLong & 0xffff);
	       CodeLen=1;
	      }
	   }
	 }
	NextPar=false; return;
       }
     WrXError(1200,HOp); NextPar=false; return;
    }

   if ((strncmp(OpPart,"RETI",4)==0) || (strncmp(OpPart,"RETS",4)==0))
    {
     DFlag=(OpPart[3]=='S')?(1l << 23):(0);
     strcpy(HOp,OpPart); strcpy(OpPart,OpPart+4);
     for (z=0; z<ConditionCount; z++)
      if (Memo(Conditions[z].Name))
       {
	if (ArgCnt!=0) WrError(1110);
	else if (ThisPar) WrError(1950);
	else
	 {
	  DAsmCode[0]=0x78000000+DFlag+(((LongWord)Conditions[z].Code) << 16);
	  CodeLen=1;
	 }
	NextPar=false; return;
       }
     WrXError(1200,HOp); NextPar=false; return;
    }

   if (strncmp(OpPart,"TRAP",4)==0)
    {
     strcpy(HOp,OpPart); strcpy(OpPart,OpPart+4);
     for (z=0; z<ConditionCount; z++)
      if (Memo(Conditions[z].Name))
       {
	if (ArgCnt!=1) WrError(1110);
	else if (ThisPar) WrError(1950);
	else
	 {
	  HReg=EvalIntExpression(ArgStr[1],UInt4,&OK);
	  if (OK)
	   {
	    DAsmCode[0]=0x74000000+HReg+(((LongWord)Conditions[z].Code) << 16);
	    CodeLen=1;
	   }
	 }
	NextPar=false; return;
       }
     WrXError(1200,HOp); NextPar=false; return;
    }

   WrXError(1200,OpPart); NextPar=false;
}

	static void InitCode_3203x(void)
{
   SaveInitProc();
   DPValue=0;
}

	static bool ChkPC_3203X(void)
{
    switch (ActPC)
     {
      case SegCode: return (ProgCounter()<=0xffffff);
      default: return false;
    }
}

	static bool IsDef_3203X(void)
{
   return (strcmp(LabPart,"||")==0);
}

	static void SwitchFrom_3203X(void)
{
   DeinitFields();
}

	static void SwitchTo_3203X(void)
{
   TurnWords=false; ConstMode=ConstModeIntel; SetIsOccupied=false;

   PCSymbol="$"; HeaderID=0x76; NOPCode=0x0c800000;
   DivideChars=","; HasAttrs=false;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=4; ListGrans[SegCode]=4; SegInits[SegCode]=0;

   MakeCode=MakeCode_3203X; ChkPC=ChkPC_3203X; IsDef=IsDef_3203X;
   SwitchFrom=SwitchFrom_3203X; InitFields(); NextPar=false;
}

	void code3203x_init(void)
{
   CPU32030=AddCPU("320C30",SwitchTo_3203X);
   CPU32031=AddCPU("320C31",SwitchTo_3203X);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_3203x;
}
