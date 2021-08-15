/* code68k.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 680x0-Familie                                               */
/*                                                                           */
/* Historie:  9. 9.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "endian.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"


typedef struct 
         {
	  char *Name;
	  Word Code;
	  CPUVar FirstCPU;
	 } CtReg;

typedef struct
         {
          char *Name;
          Byte Code;
          Boolean Dya;
         } FPUOp;

typedef struct
         {
          char *Name;
          Byte Code;
         } FPUCond;

#define CtRegCnt 8
#define CondCnt 20
#define FPUOpCnt 35
#define FPUCondCnt 26
#define PMMUCondCnt 16
#define PMMURegCnt 13

#define PMMUAvailName  "HASPMMU"     /* PMMU-Befehle erlaubt */
#define FullPMMUName   "FULLPMMU"    /* voller PMMU-Befehlssatz */

#define Mdata 1                      /* Adressierungsmasken */
#define Madr 2
#define Madri 4
#define Mpost 8
#define Mpre 16       
#define Mdadri 32
#define Maix 64
#define Mpc 128
#define Mpcidx 256
#define Mabs 512
#define Mimm 1024
#define Mfpn 2048
#define Mfpcr 4096

static Byte OpSize;
static ShortInt RelPos;
static Boolean PMMUAvail;               /* PMMU-Befehle erlaubt? */
static Boolean FullPMMU;                /* voller PMMU-Befehlssatz? */
static Byte AdrNum;                     /* Adressierungsnummer */
static Word AdrMode;                    /* Adressierungsmodus */
static Word AdrVals[10];                /* die Worte selber */

static CtReg *CtRegs;
static char **CondNams;
static Byte *CondVals;
static FPUOp *FPUOps;
static FPUCond *FPUConds;
static char **PMMUConds;
static char **PMMURegNames;
static Byte *PMMURegSizes;
static Word *PMMURegCodes;

static SimpProc SaveInitProc;
static CPUVar CPU68008,CPU68000,CPU68010,CPU68012,
              CPU68332,CPU68340,CPU68360,
              CPU68020,CPU68030;

static Word Masks[14]={0,1,2,4,8,16,32,64,128,256,512,1024,2048,4096};

/* Codetabellenverwaltung : -----------------------------------------------*/

	static void AddCtReg(char *NName, Word NCode, CPUVar NCPU)
BEGIN
   if (InstrZ>=CtRegCnt) exit(255);
   CtRegs[InstrZ].Name=NName;
   CtRegs[InstrZ].Code=NCode;
   CtRegs[InstrZ++].FirstCPU=NCPU;
END

	static void AddCond(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=CondCnt) exit(255);
   CondNams[InstrZ]=NName;
   CondVals[InstrZ++]=NCode;
END

	static void AddFPUOp(char *NName, Byte NCode, Boolean NDya)
BEGIN
   if (InstrZ>=FPUOpCnt) exit(255);
   FPUOps[InstrZ].Name=NName;
   FPUOps[InstrZ].Code=NCode;
   FPUOps[InstrZ++].Dya=NDya;
END

	static void AddFPUCond(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=FPUCondCnt) exit(255);
   FPUConds[InstrZ].Name=NName;
   FPUConds[InstrZ++].Code=NCode;
END

	static void AddPMMUCond(char *NName)
BEGIN
   if (InstrZ>=PMMUCondCnt) exit(255);
   PMMUConds[InstrZ++]=NName;
END

	static void AddPMMUReg(char *Name, Byte Size, Word Code)
BEGIN
   if (InstrZ>=PMMURegCnt) exit(255);
   PMMURegNames[InstrZ]=Name;
   PMMURegSizes[InstrZ]=Size;
   PMMURegCodes[InstrZ++]=Code;
END

	static void InitFields(void)
BEGIN
   CtRegs=(CtReg *) malloc(sizeof(CtReg)*CtRegCnt); InstrZ=0;
   AddCtReg("SFC"  ,0x000, CPU68010);
   AddCtReg("DFC"  ,0x001, CPU68010);
   AddCtReg("CACR" ,0x002, CPU68020);
   AddCtReg("USP"  ,0x800, CPU68010);
   AddCtReg("VBR"  ,0x801, CPU68010);
   AddCtReg("CAAR" ,0x802, CPU68020);
   AddCtReg("MSP"  ,0x803, CPU68020);
   AddCtReg("ISP"  ,0x804, CPU68020);

   CondNams=(char **) malloc(sizeof(char *)*CondCnt); 
   CondVals=(Byte *) malloc(sizeof(Byte)*CondCnt); InstrZ=0;
   AddCond("T" , 0);  AddCond("F" , 1);  AddCond("HI", 2);  AddCond("LS", 3);
   AddCond("CC", 4);  AddCond("CS", 5);  AddCond("NE", 6);  AddCond("EQ", 7);
   AddCond("VC", 8);  AddCond("VS", 9);  AddCond("PL",10);  AddCond("MI",11);
   AddCond("GE",12);  AddCond("LT",13);  AddCond("GT",14);  AddCond("LE",15);
   AddCond("HS", 4);  AddCond("LO", 5);  AddCond("RA", 0);  AddCond("SR", 1);

   FPUOps=(FPUOp *) malloc(sizeof(FPUOp)*FPUOpCnt); InstrZ=0;
   AddFPUOp("INT"   ,0x01, False);  AddFPUOp("SINH"  ,0x02, False);
   AddFPUOp("INTRZ" ,0x03, False);  AddFPUOp("SQRT"  ,0x04, False);
   AddFPUOp("LOGNP1",0x06, False);  AddFPUOp("ETOXM1",0x08, False);
   AddFPUOp("TANH"  ,0x09, False);  AddFPUOp("ATAN"  ,0x0a, False);
   AddFPUOp("ASIN"  ,0x0c, False);  AddFPUOp("ATANH" ,0x0d, False);
   AddFPUOp("SIN"   ,0x0e, False);  AddFPUOp("TAN"   ,0x0f, False);
   AddFPUOp("ETOX"  ,0x10, False);  AddFPUOp("TWOTOX",0x11, False);
   AddFPUOp("TENTOX",0x12, False);  AddFPUOp("LOGN"  ,0x14, False);
   AddFPUOp("LOG10" ,0x15, False);  AddFPUOp("LOG2"  ,0x16, False);
   AddFPUOp("ABS"   ,0x18, False);  AddFPUOp("COSH"  ,0x19, False);
   AddFPUOp("NEG"   ,0x1a, False);  AddFPUOp("ACOS"  ,0x1c, False);
   AddFPUOp("COS"   ,0x1d, False);  AddFPUOp("GETEXP",0x1e, False);
   AddFPUOp("GETMAN",0x1f, False);  AddFPUOp("DIV"   ,0x20, True );
   AddFPUOp("MOD"   ,0x21, True );  AddFPUOp("ADD"   ,0x22, True );
   AddFPUOp("MUL"   ,0x23, True );  AddFPUOp("SGLDIV",0x24, True );
   AddFPUOp("REM"   ,0x25, True );  AddFPUOp("SCALE" ,0x26, True );
   AddFPUOp("SGLMUL",0x27, True );  AddFPUOp("SUB"   ,0x28, True );
   AddFPUOp("CMP"   ,0x38, True );

   FPUConds=(FPUCond *) malloc(sizeof(FPUCond)*FPUCondCnt); InstrZ=0;
   AddFPUCond("EQ"  , 0x01); AddFPUCond("NE"  , 0x0e);
   AddFPUCond("GT"  , 0x12); AddFPUCond("NGT" , 0x1d);
   AddFPUCond("GE"  , 0x13); AddFPUCond("NGE" , 0x1c);
   AddFPUCond("LT"  , 0x14); AddFPUCond("NLT" , 0x1b);
   AddFPUCond("LE"  , 0x15); AddFPUCond("NLE" , 0x1a);
   AddFPUCond("GL"  , 0x16); AddFPUCond("NGL" , 0x19);
   AddFPUCond("GLE" , 0x17); AddFPUCond("NGLE", 0x18);
   AddFPUCond("OGT" , 0x02); AddFPUCond("ULE" , 0x0d);
   AddFPUCond("OGE" , 0x03); AddFPUCond("ULT" , 0x0c);
   AddFPUCond("OLT" , 0x04); AddFPUCond("UGE" , 0x0b);
   AddFPUCond("OLE" , 0x05); AddFPUCond("UGT" , 0x0a);
   AddFPUCond("OGL" , 0x06); AddFPUCond("UEQ" , 0x09);
   AddFPUCond("OR"  , 0x07); AddFPUCond("UN"  , 0x08);

   PMMUConds=(char **) malloc(sizeof(char *)*PMMUCondCnt); InstrZ=0;
   AddPMMUCond("BS"); AddPMMUCond("BC"); AddPMMUCond("LS"); AddPMMUCond("LC"); 
   AddPMMUCond("SS"); AddPMMUCond("SC"); AddPMMUCond("AS"); AddPMMUCond("AC"); 
   AddPMMUCond("WS"); AddPMMUCond("WC"); AddPMMUCond("IS"); AddPMMUCond("IC"); 
   AddPMMUCond("GS"); AddPMMUCond("GC"); AddPMMUCond("CS"); AddPMMUCond("CC"); 

   PMMURegNames=(char **) malloc(sizeof(char *)*PMMURegCnt);
   PMMURegSizes=(Byte *) malloc(sizeof(Byte)*PMMURegCnt);
   PMMURegCodes=(Word *) malloc(sizeof(Word)*PMMURegCnt); InstrZ=0;
   AddPMMUReg("TC"   ,2,16); AddPMMUReg("DRP"  ,3,17);
   AddPMMUReg("SRP"  ,3,18); AddPMMUReg("CRP"  ,3,19);
   AddPMMUReg("CAL"  ,0,20); AddPMMUReg("VAL"  ,0,21);
   AddPMMUReg("SCC"  ,0,22); AddPMMUReg("AC"   ,1,23);
   AddPMMUReg("PSR"  ,1,24); AddPMMUReg("PCSR" ,1,25);
   AddPMMUReg("TT0"  ,2, 2); AddPMMUReg("TT1"  ,2, 3);
   AddPMMUReg("MMUSR",1,24); 
END

	static void DeinitFields(void)
BEGIN
   free(CtRegs);
   free(CondNams); free(CondVals);
   free(FPUOps);
   free(FPUConds);
   free(PMMUConds);
   free(PMMURegNames); free(PMMURegSizes); free(PMMURegCodes);
END

/* Adressargument kopieren : ----------------------------------------------*/

#define CopyAdrVals(Dest) memcpy(Dest,AdrVals,AdrCnt)

	static Boolean ValReg(char Ch)
BEGIN
   return ((Ch>='0') AND (Ch<='7'));
END

	static Boolean CodeReg(char *s, Word *Erg)
BEGIN
   Boolean Result=True;

   if (strcasecmp(s,"SP")==0) *Erg=15;
   else if (ValReg(s[1])) 
    if (toupper(*s)=='D') *Erg=s[1]-'0';
    else if (toupper(*s)=='A') *Erg=s[1]-'0'+8;
    else Result=False;
   else Result=False;

   return Result;
END

        static Boolean CodeRegPair(char *Asc, Word *Erg1, Word *Erg2)
BEGIN
   if (strlen(Asc)!=5) return False;
   if (toupper(*Asc)!='D') return False;
   if (Asc[2]!=':') return False;
   if (toupper(Asc[3])!='D') return False;
   if (NOT (ValReg(Asc[1]) AND ValReg(Asc[4]))) return False;

   *Erg1=Asc[1]-'0'; *Erg2=Asc[4]-'0';

   return True;
END

        static Boolean CodeIndRegPair(char *Asc, Word *Erg1, Word *Erg2)
BEGIN
   if (strlen(Asc)!=9) return False;
   if (*Asc!='(') return False;
   if ((toupper(Asc[1])!='D') AND (toupper(Asc[1])!='A')) return False;
   if (Asc[3]!=')') return False;
   if (Asc[4]!=':') return False;
   if (Asc[5]!='(') return False;
   if ((toupper(Asc[6])!='D') AND (toupper(Asc[6])!='A')) return False;
   if (Asc[8]!=')') return False;
   if (NOT (ValReg(Asc[2]) AND ValReg(Asc[7]))) return False;

   *Erg1=Asc[2]-'0'; if (toupper(Asc[1])=='A') *Erg1+=8;
   *Erg2=Asc[7]-'0'; if (toupper(Asc[6])=='A') *Erg2+=8;

   return True;
END

/*-------------------------------------------------------------------------*/

typedef enum {PC,AReg,Index,indir,Disp,None} CompType;
typedef struct 
         {
          String Name;
          CompType Art;
          Word ANummer,INummer;
          Boolean Long;
          Word Scale;
          Word Size;
          LongInt Wert;
         } AdrComp;

	static Boolean ClassComp(AdrComp *C)
BEGIN
   char sh[10];

   if ((*C->Name=='[') AND (C->Name[strlen(C->Name)-1]==']'))
    BEGIN
     C->Art=indir; return True;
    END

   if (strcasecmp(C->Name,"PC")==0) 
    BEGIN
     C->Art=PC; return True;
    END

   sh[0]=C->Name[0]; sh[1]=C->Name[1]; sh[2]='\0';
   if (CodeReg(sh,&C->ANummer)) 
    if ((C->ANummer>7) AND (strlen(C->Name)==2)) 
     BEGIN
      C->Art=AReg; C->ANummer-=8; return True;
     END
    else
     BEGIN
      if ((strlen(C->Name)>3) AND (C->Name[2]=='.')) 
       BEGIN
	switch (toupper(C->Name[3]))
         BEGIN
	  case 'L': C->Long=True; break;
	  case 'W': C->Long=False; break;
	  default: return False;
	 END
	strcpy(C->Name+2,C->Name+4);
       END
      else C->Long=False;
      if ((strlen(C->Name)>3) AND (C->Name[2]=='*')) 
       BEGIN
	switch (C->Name[3])
         BEGIN
	  case '1': C->Scale=0; break;
	  case '2': C->Scale=1; break;
	  case '4': C->Scale=2; break;
	  case '8': C->Scale=3; break;
	  default: return False;
	 END
	strcpy(C->Name+2,C->Name+4);
       END
      else C->Scale=0;
      C->INummer=C->ANummer; C->Art=Index; return True;
     END

   C->Art=Disp;
   if (C->Name[strlen(C->Name)-2]=='.') 
    BEGIN
     switch (toupper(C->Name[strlen(C->Name)-1]))
      BEGIN
       case 'L': C->Size=2; break;
       case 'W': C->Size=1; break;
       default: return False;
      END
     C->Name[strlen(C->Name)-2]='\0';
    END
   else C->Size=1;
   C->Art=Disp;
   return True;
END

	static void ACheckCPU(CPUVar MinCPU)
BEGIN
   if (MomCPU<MinCPU) 
    BEGIN
     WrError(1505); AdrNum=0; AdrCnt=0;
    END
END

	static void ChkAdr(Word Erl)
BEGIN
   if ((Erl & Masks[AdrNum])==0) 
    BEGIN
     WrError(1350); AdrNum=0;
    END
END

	static Boolean IsShortAdr(LongInt Adr)
BEGIN
   Word WHi=(Adr>>16)&0xffff,WLo=Adr&0xffff;
 
   return ((WHi==0     ) AND (WLo<=0x7fff))
       OR ((WHi==0xffff) AND (WLo>=0x8000));
END

	static Boolean IsDisp8(LongInt Disp)
BEGIN
   return ((Disp>=-128) AND (Disp<=127));
END

	static Boolean IsDisp16(LongInt Disp)
BEGIN
   if (Disp<-32768) return False;
   if (Disp>32767) return False;
   return True;
END

	static void ChkEven(LongInt Adr)
BEGIN
   if ((MomCPU<=CPU68340) AND (Odd(Adr))) WrError(180);
END

       	static void DecodeAdr(char *Asc_O, Word Erl)
BEGIN
   Byte l,i;
   char *p;
   Word rerg;
   Byte lklamm,rklamm,lastrklamm;
   Boolean doklamm;

   AdrComp AdrComps[3],OneComp;
   Byte CompCnt;
   String OutDisp;
   Byte OutDispLen;
   Boolean PreInd;

#ifdef HAS64
   QuadInt QVal;
#endif
   LongInt HVal;
   Integer HVal16;
   ShortInt HVal8;
   Single FVal;
   Double DVal;
   Boolean ValOK;
   Word SwapField[6];
   String Asc;
   char CReg[10];

   strmaxcpy(Asc,Asc_O,255); KillBlanks(Asc);
   l=strlen(Asc);
   AdrNum=0; AdrCnt=0;

   /* immediate : */

   if (*Asc=='#') 
    BEGIN
     strcpy(Asc,Asc+1);
     AdrNum=11;
     AdrMode=0x3c;
     switch (OpSize)
      BEGIN
       case 0:
	AdrCnt=2;
	HVal8=EvalIntExpression(Asc,Int8,&ValOK);
	if (ValOK) AdrVals[0]=(Word)((Byte) HVal8);
        break;
       case 1:
	AdrCnt=2;
	HVal16=EvalIntExpression(Asc,Int16,&ValOK);
	if (ValOK) AdrVals[0]=(Word) HVal16;
        break;
       case 2:
	AdrCnt=4;
	HVal=EvalIntExpression(Asc,Int32,&ValOK);
	if (ValOK) 
	 BEGIN
	  AdrVals[0]=HVal >> 16;
	  AdrVals[1]=HVal & 0xffff;
	 END
        break;
#ifdef HAS64
       case 3:
	AdrCnt=8;
	QVal=EvalIntExpression(Asc,Int64,&ValOK);
	if (ValOK) 
	 BEGIN
	  AdrVals[0]=(QVal >> 48) & 0xffff;
	  AdrVals[1]=(QVal >> 32) & 0xffff;
	  AdrVals[2]=(QVal >> 16) & 0xffff;
	  AdrVals[3]=(QVal      ) & 0xffff;
	 END
        break;
#endif
       case 4:
	AdrCnt=4;
	FVal=EvalFloatExpression(Asc,Float32,&ValOK);
	if (ValOK) 
	 BEGIN
          memcpy(SwapField,&FVal,4); 
          if (BigEndian) DWSwap((Byte *) SwapField,4);
	  AdrVals[0]=SwapField[1];
	  AdrVals[1]=SwapField[0];
	 END
        break;
       case 5:
	AdrCnt=8;
	DVal=EvalFloatExpression(Asc,Float64,&ValOK);
	if (ValOK) 
	 BEGIN
          memcpy(SwapField,&DVal,8);
          if (BigEndian) QWSwap((Byte *) SwapField,8);
	  AdrVals[0]=SwapField[3];
	  AdrVals[1]=SwapField[2];
	  AdrVals[2]=SwapField[1];
	  AdrVals[3]=SwapField[0];
	 END
        break;
       case 6:
	AdrCnt=12;
	DVal=EvalFloatExpression(Asc,Float64,&ValOK);
	if (ValOK) 
	 BEGIN
          Double_2_TenBytes(DVal,(Byte *) SwapField);
	  if (BigEndian) WSwap((Byte *) SwapField,10);
	  AdrVals[0]=SwapField[4];
	  AdrVals[1]=0;
	  AdrVals[2]=SwapField[3];
	  AdrVals[3]=SwapField[2];
	  AdrVals[4]=SwapField[1];
	  AdrVals[5]=SwapField[0];
	 END
        break;
       case 7:
	AdrCnt=12;
	DVal=EvalFloatExpression(Asc,Float64,&ValOK);
	if (ValOK) 
	 BEGIN
	  ConvertDec(DVal,SwapField);
	  AdrVals[0]=SwapField[5];
	  AdrVals[1]=SwapField[4];
	  AdrVals[2]=SwapField[3];
	  AdrVals[3]=SwapField[2];
	  AdrVals[4]=SwapField[1];
	  AdrVals[5]=SwapField[0];
	 END
        break;
      END
     ChkAdr(Erl); return;
    END

   /* CPU-Register direkt: */

   if (CodeReg(Asc,&AdrMode)) 
    BEGIN
     AdrCnt=0; AdrNum=(AdrMode >> 3)+1; ChkAdr(Erl); return;
    END

   /* Gleitkommaregister direkt: */

   if (strncasecmp(Asc,"FP",2)==0) 
    BEGIN
     if ((strlen(Asc)==3) AND (ValReg(Asc[2]))) 
      BEGIN
       AdrMode=Asc[2]-'0'; AdrCnt=0; AdrNum=12; ChkAdr(Erl); return;
      END;
     if (strcasecmp(Asc,"FPCR")==0) 
      BEGIN
       AdrMode=4; AdrNum=13; ChkAdr(Erl); return;
      END
     if (strcasecmp(Asc,"FPSR")==0) 
      BEGIN
       AdrMode=2; AdrNum=13; ChkAdr(Erl); return;
      END
     if (strcasecmp(Asc,"FPIAR")==0) 
      BEGIN
       AdrMode=1; AdrNum=13; ChkAdr(Erl); return;
      END
    END

   /* Adressregister indirekt mit Predekrement: */

   if ((l==5) AND (*Asc=='-') AND (Asc[1]=='(') AND (Asc[4]==')'))
    BEGIN
     strcpy(CReg,Asc+2); CReg[2]='\0';
     if (CodeReg(CReg,&rerg)) 
      if (rerg>7) 
       BEGIN
        AdrMode=rerg+24; AdrCnt=0; AdrNum=5; ChkAdr(Erl); return;
       END
    END

   /* Adressregister indirekt mit Postinkrement */

   if ((l==5) AND (*Asc=='(') AND (Asc[3]==')') AND (Asc[4]=='+')) 
    BEGIN
     strcpy(CReg,Asc+1); CReg[2]='\0';
     if (CodeReg(CReg,&rerg)) 
      if (rerg>7) 
       BEGIN
        AdrMode=rerg+16; AdrCnt=0; AdrNum=4; ChkAdr(Erl); return;
       END
    END

   /* Unterscheidung direkt<->indirekt: */

   lklamm=0; rklamm=0; lastrklamm=0; doklamm=True;
   for (p=Asc; *p!='\0'; p++)
    BEGIN
     if (*p=='[') doklamm=False;
     if (*p==']') doklamm=True;
     if (doklamm) 
      if (*p=='(') lklamm++;
      else if (*p==')') 
       BEGIN
	rklamm++; lastrklamm=p-Asc;
       END
    END

   if ((lklamm==1) AND (rklamm==1) AND (lastrklamm==strlen(Asc)-1)) 
    BEGIN

     /* aeusseres Displacement abspalten, Klammern loeschen: */

     p=strchr(Asc,'('); *p='\0'; strmaxcpy(OutDisp,Asc,255); strcpy(Asc,p+1);
     if ((strlen(OutDisp)>2) AND (OutDisp[strlen(OutDisp)-2]=='.')) 
      BEGIN
       switch (toupper(OutDisp[strlen(OutDisp)-1]))
        BEGIN
         case 'B': OutDispLen=0; break;
         case 'W': OutDispLen=1; break;
         case 'L': OutDispLen=2; break;
         default:
	  WrError(1130); return;
        END
       OutDisp[strlen(OutDisp)-2]='\0';
      END
     else OutDispLen=0;
     Asc[strlen(Asc)-1]='\0';

     /* in Komponenten zerteilen: */

     CompCnt=0;
     do
      BEGIN
       doklamm=True;
       p=Asc;
       do
        BEGIN
         if (*p=='[') doklamm=False;
         else if (*p==']') doklamm=True;
         p++;
        END
       while (((NOT doklamm) OR (*p!=',')) AND (*p!='\0'));
       if (*p=='\0')
        BEGIN
         strcpy(AdrComps[CompCnt].Name,Asc); *Asc='\0';
        END
       else
        BEGIN
         *p='\0'; strcpy(AdrComps[CompCnt].Name,Asc); strcpy(Asc,p+1);
        END
       if (NOT ClassComp(AdrComps+CompCnt)) 
	BEGIN
	 WrError(1350); return;
	END
       if ((CompCnt==1) AND (AdrComps[CompCnt].Art==AReg)) 
	BEGIN
	 AdrComps[CompCnt].Art=Index; 
         AdrComps[CompCnt].INummer=AdrComps[CompCnt].ANummer+8; 
         AdrComps[CompCnt].Long=False; 
         AdrComps[CompCnt].Scale=0;
	END
       if ((AdrComps[CompCnt].Art==Disp) OR ((AdrComps[CompCnt].Art!=Index) AND (CompCnt!=0))) 
	BEGIN
	 WrError(1350); return;
	END
       CompCnt++;
      END
     while (*Asc!='\0');
     if ((CompCnt>2) OR ((AdrComps[0].Art==Index) AND (CompCnt!=1))) 
      BEGIN
       WrError(1350); return;
      END

     /* 1. Variante (An....), d(An....) */

     if (AdrComps[0].Art==AReg) 
      BEGIN

       /* 1.1. Variante (An), d(An) */

       if (CompCnt==1) 
	BEGIN

         /* 1.1.1. Variante (An) */

	 if ((*OutDisp=='\0') AND ((Madri & Erl)!=0)) 
	  BEGIN
	   AdrMode=0x10+AdrComps[0].ANummer; AdrNum=3; AdrCnt=0;
	   ChkAdr(Erl); return;
	  END

         /* 1.1.2. Variante d(An) */

	 else
	  BEGIN
	   if (OutDispLen>=2) HVal=EvalIntExpression(OutDisp,Int32,&ValOK);
           else HVal=EvalIntExpression(OutDisp,SInt16,&ValOK);
	   if (NOT ValOK) 
	    BEGIN
	     WrError(1350); return;
	    END
	   if ((ValOK) AND (HVal==0) AND ((Madri & Erl)!=0) AND (OutDispLen==0)) 
	    BEGIN
             AdrMode=0x10+AdrComps[0].ANummer; AdrNum=3; AdrCnt=0;
	     ChkAdr(Erl); return;
	    END
	   if (OutDispLen==0) OutDispLen=1;
	   switch (OutDispLen)
            BEGIN
	     case 1:                   /* d16(An) */
	      AdrMode=0x28+AdrComps[0].ANummer; AdrNum=6;
	      AdrCnt=2; AdrVals[0]=HVal&0xffff;
	      ChkAdr(Erl); return;
	     case 2:                   /* d32(An) */
	      AdrMode=0x30+AdrComps[0].ANummer; AdrNum=7;
	      AdrCnt=6; AdrVals[0]=0x0170;
	      AdrVals[1]=(HVal >> 16) & 0xffff; AdrVals[2]=HVal & 0xffff;
	      ACheckCPU(CPU68332); ChkAdr(Erl); return;
	    END
	  END
	END

       /* 1.2. Variante d(An,Xi) */

       else
	BEGIN
         AdrVals[0]=(AdrComps[1].INummer << 12)+(Ord(AdrComps[1].Long) << 11)+(AdrComps[1].Scale << 9);
	 AdrMode=0x30+AdrComps[0].ANummer;
	 switch (OutDispLen)
          BEGIN
	   case 0:
	    HVal8=EvalIntExpression(OutDisp,SInt8,&ValOK);
	    if (ValOK) 
	     BEGIN
	      AdrNum=7; AdrCnt=2; AdrVals[0]+=((Byte)HVal8);
	      if (AdrComps[1].Scale!=0) ACheckCPU(CPU68332);
	     END
	    ChkAdr(Erl); return;
	   case 1:
	    HVal16=EvalIntExpression(OutDisp,SInt16,&ValOK);
	    if (ValOK) 
	     BEGIN
	      AdrNum=7; AdrCnt=4;
	      AdrVals[0]+=0x120; AdrVals[1]=HVal16;
	      ACheckCPU(CPU68332);
	     END
	    ChkAdr(Erl); return;
	   case 2:
	    HVal=EvalIntExpression(OutDisp,Int32,&ValOK);
	    if (ValOK) 
	     BEGIN
	      AdrNum=7; AdrCnt=6; AdrVals[0]+=0x130;
	      AdrVals[1]=HVal >> 16; AdrVals[2]=HVal & 0xffff;
	      ACheckCPU(CPU68332);
	     END
	    ChkAdr(Erl); return;
	  END
	END
      END

     /* 2. Variante d(PC....) */

     else if (AdrComps[0].Art==PC) 
      BEGIN

       /* 2.1. Variante d(PC) */

       if (CompCnt==1) 
	BEGIN
	 if (OutDispLen==0) OutDispLen=1;
	 HVal=EvalIntExpression(OutDisp,Int32,&ValOK)-(EProgCounter()+RelPos);
	 if (NOT ValOK) 
	  BEGIN
	   WrError(1350); return;
	  END
	 switch (OutDispLen)
          BEGIN
	   case 1:
	    AdrMode=0x3a;
	    HVal16=HVal;
	    if (NOT IsDisp16(HVal)) 
	     BEGIN
	      WrError(1330); return;
	     END
	    AdrNum=8; AdrCnt=2; AdrVals[0]=HVal16;
	    ChkAdr(Erl); return;
	   case 2:
	    AdrMode=0x3b;
	    AdrNum=9; AdrCnt=6; AdrVals[0]=0x170;
	    AdrVals[1]=HVal >> 16; AdrVals[2]=HVal & 0xffff;
	    ACheckCPU(CPU68332); ChkAdr(Erl); return;
	  END
	END

       /* 2.2. Variante d(PC,Xi) */

       else
	BEGIN
         AdrVals[0]=(AdrComps[1].INummer << 12)+(Ord(AdrComps[1].Long) << 11)+(AdrComps[1].Scale << 9);
	 HVal=EvalIntExpression(OutDisp,Int32,&ValOK)-(EProgCounter()+RelPos);
	 if (NOT ValOK) 
	  BEGIN
	   WrError(1350); return;
	  END;
	 AdrMode=0x3b;
	 switch (OutDispLen)
          BEGIN
	   case 0:
	    HVal8=HVal;
	    if (NOT IsDisp8(HVal)) 
	     BEGIN
	      WrError(1330); return;
	     END
	    AdrVals[0]+=((Byte)HVal8); AdrCnt=2; AdrNum=9;
	    if (AdrComps[1].Scale!=0) ACheckCPU(CPU68332);
	    ChkAdr(Erl); return;
	   case 1:
	    HVal16=HVal;
	    if (NOT IsDisp16(HVal)) 
	     BEGIN
	      WrError(1330); return;
	     END
	    AdrVals[0]+=0x120; AdrCnt=4; AdrNum=9;
	    AdrVals[1]=HVal16;
	    ACheckCPU(CPU68332);
	    ChkAdr(Erl); return;
	   case 2:
	    AdrVals[0]=AdrVals[0]+0x120; AdrCnt=6; AdrNum=9;
	    AdrVals[1]=HVal >> 16; AdrVals[2]=HVal & 0xffff;
	    ACheckCPU(CPU68332);
	    ChkAdr(Erl); return;
	  END
	END
      END

     /* 3. Variante (Xi), d(Xi) */

     else if (AdrComps[0].Art==Index) 
      BEGIN
       AdrVals[0]=(AdrComps[0].INummer << 12)+(Ord(AdrComps[0].Long) << 11)+(AdrComps[0].Scale << 9)+0x180;
       AdrMode=0x30;
       if (*OutDisp=='\0') 
	BEGIN
	 AdrVals[0]=AdrVals[0]+0x0010; AdrCnt=2;
	 AdrNum=7; ACheckCPU(CPU68332); ChkAdr(Erl); return;
	END
       else switch (OutDispLen)
        BEGIN
         case 0:
         case 1:
	  HVal16=EvalIntExpression(OutDisp,Int16,&ValOK);
	  if (ValOK) 
	   BEGIN
	    AdrVals[0]=AdrVals[0]+0x0020; AdrVals[1]=HVal16;
	    AdrNum=7; AdrCnt=4; ACheckCPU(CPU68332);
	   END
	  ChkAdr(Erl); return;
         case 2:
	  HVal=EvalIntExpression(OutDisp,Int32,&ValOK);
	  if (ValOK) 
	   BEGIN
	    AdrVals[0]=AdrVals[0]+0x0030; AdrNum=7; AdrCnt=6;
	    AdrVals[1]=HVal >> 16; AdrVals[2]=HVal & 0xffff;
	    ACheckCPU(CPU68332);
	   END
	  ChkAdr(Erl); return;
        END
      END

     /* 4. Variante indirekt: */

     else if (AdrComps[0].Art==indir) 
      BEGIN

       /* erst ab 68020 erlaubt */

       if (MomCPU<CPU68020) 
        BEGIN
         WrError(1505); return;
        END

       /* Unterscheidung Vor- <---> Nachindizierung: */

       if (CompCnt==2) 
	BEGIN
	 PreInd=False;
	 AdrComps[2]=AdrComps[1];
	END
       else
	BEGIN
	 PreInd=True;
	 AdrComps[2].Art=None;
	END

       /* indirektes Argument herauskopieren: */

       strcpy(Asc,AdrComps[0].Name+1);
       Asc[strlen(Asc)-1]='\0';

       /* Felder loeschen: */

       for (i=0; i<2; AdrComps[i++].Art=None);

       /* indirekten Ausdruck auseinanderfieseln: */

       do
        BEGIN

 	 /* abschneiden & klassifizieren: */

	 p=strchr(Asc,',');
	 if (p==Nil)
          BEGIN
           strcpy(OneComp.Name,Asc); *Asc='\0';
          END
         else
          BEGIN
           *p='\0'; strcpy(OneComp.Name,Asc); strcpy(Asc,p+1);
          END
	 if (NOT ClassComp(&OneComp)) 
	  BEGIN
	   WrError(1350); return;
	  END

	/* passend einsortieren: */

	 if ((AdrComps[1].Art!=None) AND (OneComp.Art==AReg)) 
	  BEGIN
	   OneComp.Art=Index; OneComp.INummer=OneComp.ANummer+8; 
           OneComp.Long=False; OneComp.Scale=0;
	  END
	 switch (OneComp.Art)
          BEGIN
	   case Disp  : i=0; break;
	   case AReg  :
           case PC    : i=1; break;
	   case Index : i=2; break;
           default    : i=(-1);
	  END
	 if (AdrComps[i].Art!=None) 
	  BEGIN
	   WrError(1350); return;
	  END
	 else AdrComps[i]=OneComp;
        END
       while (*Asc!='\0');

       /* Vor-oder Nachindizierung? */

       AdrVals[0]=0x100+(Ord(PreInd) << 2);

       /* Indexregister eintragen */

       if (AdrComps[2].Art==None) AdrVals[0]+=0x40;
       else AdrVals[0]+=(AdrComps[2].INummer << 12)+(Ord(AdrComps[2].Long) << 11)+(AdrComps[2].Scale << 9);

       /* 4.1 Variante d([...PC...]...) */

       if (AdrComps[1].Art==PC) 
	BEGIN
	 HVal=EvalIntExpression(AdrComps[0].Name,Int32,&ValOK)-(EProgCounter()+RelPos);
	 if (NOT ValOK) return;
	 AdrMode=0x3b;
	 switch (AdrComps[0].Size)
          BEGIN
	   case 1:
	    HVal16=HVal;
	    if (NOT IsDisp16(HVal)) 
	     BEGIN
	      WrError(1330); return;
	     END
	    AdrVals[1]=HVal16; AdrVals[0]+=0x20; AdrNum=7; AdrCnt=4;
	    break;
	   case 2:
	    AdrVals[1]=HVal >> 16; AdrVals[2]=HVal & 0xffff;
	    AdrVals[0]+=0x30; AdrNum=7; AdrCnt=6;
            break;
	  END
	END

       /* 4.2 Variante d([...An...]...) */

       else
	BEGIN
         if (AdrComps[1].Art==None)
	  BEGIN
	   AdrMode=0x30; AdrVals[0]+=0x80;
	  END
	 else AdrMode=0x30+AdrComps[1].ANummer;

	 if (AdrComps[0].Art==None) 
	  BEGIN
	   AdrNum=7; AdrCnt=2; AdrVals[0]+=0x10;
	  END
	 else switch (AdrComps[0].Size)
          BEGIN
	   case 1:
	    HVal16=EvalIntExpression(AdrComps[0].Name,Int16,&ValOK);
	    if (NOT ValOK) return;
	    AdrNum=7; AdrVals[1]=HVal16; AdrCnt=4; AdrVals[0]+=0x20;
	    break;
	   case 2:
	    HVal=EvalIntExpression(AdrComps[0].Name,Int32,&ValOK);
	    if (NOT ValOK) return;
	    AdrNum=7; AdrCnt=6; AdrVals[0]+=0x30;
	    AdrVals[1]=HVal >> 16; AdrVals[2]=HVal & 0xffff;
	    break;
	  END
	END

       /* aeusseres Displacement: */

       if (OutDispLen==0) OutDispLen=1;
       if (*OutDisp=='\0')
	BEGIN
	 AdrVals[0]++; ChkAdr(Erl); return;
	END
       else switch (OutDispLen)
        BEGIN
         case 1:
	  HVal16=EvalIntExpression(OutDisp,Int16,&ValOK);
	  if (NOT ValOK) 
	   BEGIN
	    AdrNum=0; AdrCnt=0; return;
	   END
	  AdrVals[AdrCnt >> 1]=HVal16; AdrCnt+=2; AdrVals[0]+=2;
	  break;
         case 2:
	  HVal=EvalIntExpression(OutDisp,Int32,&ValOK);
	  if (NOT ValOK) 
	   BEGIN
	    AdrNum=0; AdrCnt=0; return;
	   END
	  AdrVals[(AdrCnt >> 1)  ]=HVal >> 16;
	  AdrVals[(AdrCnt >> 1)+1]=HVal & 0xffff;
	  AdrCnt+=4; AdrVals[0]+=3;
	  break;
        END

       ChkAdr(Erl); return;

      END

    END

   /* absolut: */

   else
    BEGIN
     AdrCnt=0;
     if (strcasecmp(Asc+strlen(Asc)-2,".W")==0) 
      BEGIN
       AdrCnt=2; Asc[strlen(Asc)-2]='\0';
      END
     else if (strcasecmp(Asc+strlen(Asc)-2,".L")==0) 
      BEGIN
       AdrCnt=4; Asc[strlen(Asc)-2]='\0';
      END

     FirstPassUnknown=False;
     HVal=EvalIntExpression(Asc,Int32,&ValOK);
     if ((NOT FirstPassUnknown) AND (OpSize>0)) ChkEven(HVal);
     HVal16=HVal;

     if (ValOK) 
      BEGIN
       if (AdrCnt==0) AdrCnt=(IsShortAdr(HVal))?2:4; 
       AdrNum=10;

       if (AdrCnt==2) 
	BEGIN
	 if (NOT IsShortAdr(HVal)) 
	  BEGIN
	   WrError(1340); AdrNum=0;
	  END
	 else
	  BEGIN
	   AdrMode=0x38; AdrVals[0]=HVal16;
	  END
	END
       else
	BEGIN
	 AdrMode=0x39; AdrVals[0]=HVal >> 16; AdrVals[1]=HVal & 0xffff;
	END
      END
    END

   ChkAdr(Erl);
END


/*---------------------------------------------------------------------------*/

        static void PutByte(Byte b)
BEGIN
   if (((CodeLen&1)==1) AND (NOT BigEndian))
    BEGIN
     BAsmCode[CodeLen]=BAsmCode[CodeLen-1];
     BAsmCode[CodeLen-1]=b;
    END
   else
    BEGIN
     BAsmCode[CodeLen]=b;
    END
   CodeLen++;
END

        static Boolean DecodePseudo(void)
BEGIN
#define ONOFF68KCount 4
static ONOFFRec ONOFF68Ks[ONOFF68KCount]=
             {{"PMMU"    , &PMMUAvail , PMMUAvailName },
              {"FULLPMMU", &FullPMMU  , FullPMMUName  },
              {"FPU"     , &FPUAvail  , FPUAvailName  },
              {"SUPMODE" , &SupAllowed, SupAllowedName}};
   int z,l;

   if (Memo("STR"))
    BEGIN
     l=strlen(ArgStr[1]);
     if (ArgCnt!=1) WrError(1110);
     else if (l<2) WrError(1135);
     else if (*ArgStr[1]!='\'') WrError(1135);
     else if (ArgStr[1][l-1]!='\'') WrError(1135);
     else
      BEGIN
       PutByte(l-2);
       for (z=1; z<l-1; z++)
        PutByte(CharTransTable[(unsigned int) ArgStr[1][z]]);
      END
     if ((Odd(CodeLen)) AND (DoPadding)) PutByte(0);
     return True;
    END

   if (CodeONOFF(ONOFF68Ks,ONOFF68KCount)) return True;

   return False;
END


        static Word ShiftCodes(char *s)
BEGIN
   size_t l=strlen(s);
   char Ch=s[l-1];
   Word Erg;

   s[l-1]='\0';
   if (strcmp(s,"AS")==0) Erg=0;
   else if (strcmp(s,"LS")==0) Erg=1;
   else if (strcmp(s,"RO")==0) Erg=3;
   else if (strcmp(s,"ROX")==0) Erg=2;
   else Erg=0xffff;

   s[l-1]=Ch; return Erg;
END

        static Byte OneReg(char *Asc)
BEGIN
   if (strlen(Asc)!=2) return 16;
   if ((toupper(*Asc)!='A') AND (toupper(*Asc)!='D')) return 16;
   if (NOT ValReg(Asc[1])) return 16;
   return Asc[1]-'0'+((toupper(*Asc)=='D')?0:8);
END

        static Boolean DecodeRegList(char *Asc_o, Word *Erg)
BEGIN
   static Word Masks[16]={1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};
   Byte h,h2,z;
   char *p;
   String Asc,s;

   *Erg=0; strmaxcpy(Asc,Asc_o,255);
   do
    BEGIN
     p=strchr(Asc,'/');
     if (p==Nil)
      BEGIN
       strcpy(s,Asc); *Asc='\0';
      END
     else
      BEGIN
       *p='\0'; strcpy(s,Asc); strcpy(Asc,p+1);
      END
     if (*Asc=='/') strcpy(Asc,Asc+1);
     p=strchr(s,'-');
     if (p==Nil) 
      BEGIN
       if ((h=OneReg(s))==16) return False;
       *Erg|=Masks[h];
      END
     else
      BEGIN
       *p='\0';
       if ((h=OneReg(s))==16) return False;
       if ((h2=OneReg(p+1))==16) return False;
       for (z=h; z<=h2; *Erg|=Masks[z++]);
      END
    END    
   while (*Asc!='\0');
   return True;
END

        static Boolean DecodeCtrlReg(char *Asc, Word *Erg)
BEGIN
   Byte z;
   String Asc_N;

   strmaxcpy(Asc_N,Asc,255); NLS_UpString(Asc_N); Asc=Asc_N;

   z=0;
   while ((z<CtRegCnt) AND ((strcmp(CtRegs[z].Name,Asc)!=0) OR (CtRegs[z].FirstCPU>MomCPU)))
    z++;
   if (z!=CtRegCnt) *Erg=CtRegs[z].Code;
   return (z!=CtRegCnt);
END

	static Boolean OneField(char *Asc, Word *Erg, Boolean Ab1)
BEGIN
   Boolean ValOK;

   if ((strlen(Asc)==2) AND (toupper(*Asc)=='D') AND (ValReg(Asc[1])))
    BEGIN
     *Erg=0x20+(Asc[1]-'0'); return True;
    END
   else
    BEGIN
     *Erg=EvalIntExpression(Asc,Int8,&ValOK);
     if ((Ab1) AND (*Erg==32)) *Erg=0;
     return ((ValOK) AND (*Erg<32));
    END
END

	static Boolean SplitBitField(char *Arg_o, Word *Erg)
BEGIN
   char *p;
   Word OfsVal;
   String Desc,Arg;

   strmaxcpy(Arg,Arg_o,255);
   p=strchr(Arg,'{');
   if (p==Nil) return False;
   *p='\0'; strcpy(Desc,p+1);
   if (Desc[strlen(Desc)-1]!='}') return False;
   Desc[strlen(Desc)-1]='\0';

   p=strchr(Desc,':');
   if (p==Nil) return False;
   *p='\0';
   if (NOT OneField(Desc,&OfsVal,False)) return False;
   if (NOT OneField(p+1,Erg,True)) return False;
   Erg+=OfsVal << 6;
   return True;
END

        static void CheckCPU(CPUVar Level)
BEGIN
   if (MomCPU<Level) 
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

	static void Check020(void)
BEGIN
   if (MomCPU!=CPU68020)
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

	static void Check32(void)
BEGIN
   if ((MomCPU<CPU68332) OR (MomCPU>CPU68360))
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

        static void CheckSup(void)
BEGIN
   if (NOT SupAllowed) WrError(50);
END

/*-------------------------------------------------------------------------*/


        static Boolean DecodeOneFPReg(char *Asc, Byte * h)
BEGIN
   if ((strlen(Asc)==3) AND (strncasecmp(Asc,"FP",2)==0) AND ValReg(Asc[2]))
    BEGIN
     *h=Asc[2]-'0'; return True;
    END
   else return False;
END

        static void DecodeFRegList(char *Asc_o, Byte *Typ, Byte *Erg)
BEGIN
   String s,Asc;
   Word hw;
   Byte h2,h3,z;
   char *h1;

   strmaxcpy(Asc,Asc_o,255);
   *Typ=0; if (*Asc=='\0') return;

   if ((strlen(Asc)==2) AND (*Asc=='D') AND ValReg(Asc[1]))
    BEGIN
     *Typ=1; *Erg=(Asc[1]-'0') << 4; return;
    END;

   hw=0;
   do
    BEGIN
     h1=strchr(Asc,'/');
     if (h1==Nil)
      BEGIN
       strcpy(s,Asc); *Asc='\0';
      END
     else
      BEGIN
       *h1='\0'; strcpy(s,Asc); strcpy(Asc,h1+1);
      END
     if (strcasecmp(s,"FPCR")==0) hw|=0x400;
     else if (strcasecmp(s,"FPSR")==0) hw|=0x200;
     else if (strcasecmp(s,"FPIAR")==0) hw|=0x100;
     else
      BEGIN
       h1=strchr(s,'-');
       if (h1==Nil)
        BEGIN
         if (NOT DecodeOneFPReg(s,&h2)) return;
         hw|=(1 << (7-h2));
        END
       else
        BEGIN
         *h1='\0';
         if (NOT DecodeOneFPReg(s,&h2)) return;
         if (NOT DecodeOneFPReg(h1+1,&h3)) return;
         for (z=h2; z<=h3; z++) hw|=(1 << (7-z));
        END
      END
    END
   while (*Asc!='\0');
   if (Hi(hw)==0)
    BEGIN
     *Typ=2; *Erg=Lo(hw);
    END
   else if (Lo(hw)==0)
    BEGIN
     *Typ=3; *Erg=Hi(hw);
    END
END

        static void GenerateMovem(Byte z1, Byte z2)
BEGIN
   Byte hz2,z;

   if (AdrNum==0) return;
   CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
   WAsmCode[0]=0xf200 | AdrMode;
   switch (z1)
    BEGIN
     case 1:
     case 2:
      WAsmCode[1]|=0xc000;
      if (z1==1) WAsmCode[1]|=0x800;
      if (AdrNum!=5) WAsmCode[1]|=0x1000;
      if ((AdrNum==5) AND (z1==2))
       BEGIN
        hz2=z2; z2=0;
        for (z=0; z<8; z++)
         BEGIN
          z2=z2 << 1; if ((hz2&1)==1) z2|=1;
          hz2=hz2 >> 1;
         END
       END
      WAsmCode[1]|=z2;
      break;
     case 3:
      WAsmCode[1]|=0x8000 | (((Word)z2) << 10);
    END
END

        static void DecodeFPUOrders(void)
BEGIN
   static Byte SizeCodes[87]={6,4,0,7,1,5,2,3};
   Byte z,z1,z2;
   char *p;
   String sk;
   LongInt HVal;
   Integer HVal16;
   Boolean ValOK;

   for (z=0; z<FPUOpCnt; z++)
    if (Memo(FPUOps[z].Name)) break;
   if (z<FPUOpCnt)
    BEGIN
     if ((ArgCnt==1) AND (NOT FPUOps[z].Dya))
      BEGIN
       strcpy(ArgStr[2],ArgStr[1]); ArgCnt=2;
      END
     if (*AttrPart=='\0') OpSize=6;
     if (OpSize==3) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Mfpn);
       if (AdrNum==12)
        BEGIN
         WAsmCode[0]=0xf200;
         WAsmCode[1]=FPUOps[z].Code | (AdrMode << 7);
         RelPos=4;
         DecodeAdr(ArgStr[1],((OpSize<=2) OR (OpSize==4))?
                             Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn:
                             Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn);
         if (AdrNum==12)
          BEGIN
           WAsmCode[1]|=AdrMode << 10;
           if (OpSize==6) CodeLen=4; else WrError(1130);
          END
         else if (AdrNum!=0)
          BEGIN
           CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
           WAsmCode[0]|=AdrMode;
           WAsmCode[1]|=0x4000 | (((Word)SizeCodes[OpSize]) << 10);
          END
        END
      END
     return;
    END

   if (Memo("SAVE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madri+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
	BEGIN
	 CodeLen=2+AdrCnt; WAsmCode[0]=0xf300 | AdrMode;
	 CopyAdrVals(WAsmCode+1); CheckSup();
	END
      END
     return;
    END

   if (Memo("RESTORE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madri+Mpost+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
	BEGIN
	 CodeLen=2+AdrCnt; WAsmCode[0]=0xf340 | AdrMode;
	 CopyAdrVals(WAsmCode+1); CheckSup();
	END
      END
     return;
    END

   if (Memo("NOP"))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else
      BEGIN
       CodeLen=4; WAsmCode[0]=0xf280; WAsmCode[1]=0;
      END
     return;
    END

   if (Memo("MOVE"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (OpSize==3) WrError(1130);
     else
      BEGIN
       p=strchr(AttrPart,'{');
       if (p!=0)                               /* k-Faktor abspalten */
        BEGIN
         strcpy(sk,p); *p='\0';
        END
       else *sk='\0';
       DecodeAdr(ArgStr[2],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs+Mfpn+Mfpcr);
       if (AdrNum==12)                         /* FMOVE.x <ea>/FPm,FPn ? */
        BEGIN
         WAsmCode[0]=0xf200; WAsmCode[1]=AdrMode << 7;
         RelPos=4;
         if (*AttrPart=='\0') OpSize=6;
         DecodeAdr(ArgStr[1],((OpSize<=2) OR (OpSize==4))?
                             Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn:
                             Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn);
         if (AdrNum==12)                       /* FMOVE.X FPm,FPn ? */
          BEGIN
           WAsmCode[1]|=AdrMode << 10;
           if (OpSize==6) CodeLen=4; else WrError(1130);
          END
         else if (AdrNum!=0)                   /* FMOVE.x <ea>,FPn ? */
          BEGIN
           CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
           WAsmCode[0]|=AdrMode;
           WAsmCode[1]|=0x4000 | (((Word)SizeCodes[OpSize]) << 10);
          END
        END
       else if (AdrNum==13)                    /* FMOVE.L <ea>,FPcr ? */
        BEGIN
         if ((OpSize!=2) AND (*AttrPart!='\0')) WrError(1130);
         else
          BEGIN
           RelPos=4;
           WAsmCode[0]=0xf200; WAsmCode[1]=0x8000 | (AdrMode << 10);
           DecodeAdr(ArgStr[1],(AdrMode==1)?
                     Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Mpc+Mpcidx+Mabs+Mimm:
                     Mdata+Madri+Mpost+Mpre+Mdadri+Mpc+Mpcidx+Mabs+Mimm);
           if (AdrNum!=0)
            BEGIN
             WAsmCode[0]|=AdrMode; CodeLen=4+AdrCnt;
             CopyAdrVals(WAsmCode+2);
            END
          END
        END
       else if (AdrNum!=0)                     /* FMOVE.x ????,<ea> ? */
        BEGIN
         WAsmCode[0]=0xf200 | AdrMode;
         CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
         DecodeAdr(ArgStr[1],(AdrMode==1)?Mfpcr:Mfpn+Mfpcr);
         if (AdrNum==12)                       /* FMOVE.x FPn,<ea> ? */
          BEGIN
           if (*AttrPart=='\0') OpSize=6;
           WAsmCode[1]=0x6000 | (((Word)SizeCodes[OpSize]) << 10) | (AdrMode << 7);
           if (OpSize==7)
            if (strlen(sk)>2)
             BEGIN
              OpSize=0; strcpy(sk,sk+1); sk[strlen(sk)-1]='\0';
              DecodeAdr(sk,Mdata+Mimm);
              if (AdrNum==1) WAsmCode[1]|=(AdrMode << 4) | 0x1000;
              else if (AdrNum==11) WAsmCode[1]|=(AdrVals[0] & 127);
              else CodeLen=0;
             END
            else WAsmCode[1]|=17;
          END
         else if (AdrNum==13)                  /* FMOVE.L FPcr,<ea> ? */
          BEGIN
           if ((*AttrPart!='\0') AND (OpSize!=2))
            BEGIN
             WrError(1130); CodeLen=0;
            END
           else
            BEGIN
             WAsmCode[1]=0xa000 | (AdrMode << 10);
             if ((AdrMode!=1) AND ((WAsmCode[0] & 0x38)==8))
              BEGIN
               WrError(1350); CodeLen=0;
              END
            END
          END
         else CodeLen=0;
        END
      END
     return;
    END

   if (Memo("MOVECR"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((*AttrPart!='\0') AND (OpSize!=6)) WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Mfpn);
       if (AdrNum==12)
        BEGIN
         WAsmCode[0]=0xf200; WAsmCode[1]=0x5c00 | (AdrMode << 7);
         OpSize=0;
         DecodeAdr(ArgStr[1],Mimm);
         if (AdrNum==11)
          if (AdrVals[0]>63) WrError(1700);
          else
           BEGIN
            CodeLen=4;
            WAsmCode[1]|=AdrVals[0];
           END
        END
      END
     return;
    END

   if (Memo("TST"))
    BEGIN
     if (*AttrPart=='\0') OpSize=6;
     else if (OpSize==3) WrError(1130);
     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       RelPos=4;
       DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn);
       if (AdrNum==12)
        BEGIN
         WAsmCode[0]=0xf200; WAsmCode[1]=0x3a | (AdrMode << 10);
         CodeLen=4;
        END
       else if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0xf200 | AdrMode;
         WAsmCode[1]=0x403a | (((Word)SizeCodes[OpSize]) << 10);
         CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
        END
      END
     return;
    END

   if (Memo("SINCOS"))
    BEGIN
     if (*AttrPart=='\0') OpSize=6;
     if (OpSize==3) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       p=strrchr(ArgStr[2],':');
       if (p!=Nil)
        BEGIN
         *p='\0'; strcpy(sk,ArgStr[2]); strcpy(ArgStr[2],p+1);
        END
       else *sk='\0';
       DecodeAdr(sk,Mfpn);
       if (AdrNum==12)
        BEGIN
         WAsmCode[1]=AdrMode | 0x30;
         DecodeAdr(ArgStr[2],Mfpn);
         if (AdrNum==12)
          BEGIN
           WAsmCode[1]|=(AdrMode << 7);
           RelPos=4;
           DecodeAdr(ArgStr[1],((OpSize<=2) OR (OpSize==4))?
                               Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn:
                               Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn);
           if (AdrNum==12)
            BEGIN
             WAsmCode[0]=0xf200; WAsmCode[1]|=(AdrMode << 10);
             CodeLen=4;
            END
           else if (AdrNum!=0)
            BEGIN
             WAsmCode[0]=0xf200 | AdrMode;
             WAsmCode[1]|=0x4000 | (((Word)SizeCodes[OpSize]) << 10);
             CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
            END
          END
        END
      END
     return;
    END

   if (*OpPart=='B')
    BEGIN
     for (z=0; z<FPUCondCnt; z++)
      if (strcmp(OpPart+1,FPUConds[z].Name)==0) break;
     if (z>=FPUCondCnt) WrError(1360);
     else
      BEGIN
       if ((OpSize!=1) AND (OpSize!=2) AND (OpSize!=6)) WrError(1130);
       else if (ArgCnt!=1) WrError(1110);
       else
	BEGIN
	 HVal=EvalIntExpression(ArgStr[1],Int32,&ValOK)-(EProgCounter()+2);
	 HVal16=HVal;

	 if (OpSize==1)
	  BEGIN
	   OpSize=(IsDisp16(HVal))?2:6;
	  END

         if (OpSize==2)
          BEGIN
           if ((NOT IsDisp16(HVal)) AND (NOT SymbolQuestionable)) WrError(1370);
           else
            BEGIN
             CodeLen=4; WAsmCode[0]=0xf280 | FPUConds[z].Code;
             WAsmCode[1]=HVal16;
            END
          END
         else
          BEGIN
           CodeLen=6; WAsmCode[0]=0xf2c0 | FPUConds[z].Code;
           WAsmCode[2]=HVal & 0xffff; WAsmCode[1]=HVal >> 16;
	   if ((IsDisp16(HVal)) AND (PassNo>1) AND (*AttrPart=='\0'))
	    BEGIN
	     WrError(20); WAsmCode[0]^=0x40;
             CodeLen-=2; WAsmCode[1]=WAsmCode[2]; StopfZahl++;
            END
          END
        END
      END
     return;
    END

   if (strncmp(OpPart,"DB",2)==0)
    BEGIN
     for (z=0; z<FPUCondCnt; z++)
      if (strcmp(OpPart+2,FPUConds[z].Name)==0) break;
     if (z>=FPUCondCnt) WrError(1360);
     else
      BEGIN
       if ((OpSize!=1) AND (*AttrPart!='\0')) WrError(1130);
       else if (ArgCnt!=2) WrError(1110);
       else
        BEGIN
         DecodeAdr(ArgStr[1],Mdata);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]=0xf248 | AdrMode; WAsmCode[1]=FPUConds[z].Code;
           HVal=EvalIntExpression(ArgStr[2],Int32,&ValOK)-(EProgCounter()+4);
           if (ValOK)
            BEGIN
             HVal16=HVal; WAsmCode[2]=HVal16;
             if ((NOT IsDisp16(HVal)) AND (NOT SymbolQuestionable)) WrError(1370); else CodeLen=6;
            END
          END
        END
      END
     return;
    END

   if (*OpPart=='S')
    BEGIN
     for (z=0; z<FPUCondCnt; z++)
      if (strcmp(OpPart+1,FPUConds[z].Name)==0) break;
     if (z>=FPUCondCnt) WrError(1360);
     else
      BEGIN
       if ((OpSize!=0) AND (*AttrPart!='\0')) WrError(1130);
       else if (ArgCnt!=1) WrError(1110);
       else
        BEGIN
         DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
         if (AdrNum!=0)
          BEGIN
           CodeLen=4+AdrCnt; WAsmCode[0]=0xf240 | AdrMode;
           WAsmCode[1]=FPUConds[z].Code; CopyAdrVals(WAsmCode+2);
          END
        END
      END
     return;
    END

   if (strncmp(OpPart,"TRAP",4)==0)
    BEGIN
     for (z=0; z<FPUCondCnt; z++)
      if (strcmp(OpPart+4,FPUConds[z].Name)==0) break;
     if (z>=FPUCondCnt) WrError(1360);
     else
      BEGIN
       if (*AttrPart=='\0') OpSize=0;
       if (OpSize>2) WrError(1130);
       else if (((OpSize==0) AND (ArgCnt!=0)) OR ((OpSize!=0) AND (ArgCnt!=1))) WrError(1110);
       else
        BEGIN
         WAsmCode[0]=0xf278; WAsmCode[1]=FPUConds[z].Code;
         if (OpSize==0)
          BEGIN
           WAsmCode[0]|=4; CodeLen=4;
          END
         else
          BEGIN
           DecodeAdr(ArgStr[1],Mimm);
           if (AdrNum!=0)
            BEGIN
             WAsmCode[0]|=(OpSize+1);
             CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
            END
          END
        END
      END
     return;
    END

   if (Memo("MOVEM"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeFRegList(ArgStr[2],&z1,&z2);
       if (z1!=0)
        BEGIN
         if ((*AttrPart!='\0') AND (((z1<3) AND (OpSize!=6)) OR ((z1==3) AND (OpSize!=2))))
          WrError(1130);
         else
          BEGIN
           RelPos=4;
           DecodeAdr(ArgStr[1],Madri+Mpost+(z1==3?0:Mpre)+Mdadri+Maix+Mpc+Mpcidx+Mabs);
           WAsmCode[1]=0; GenerateMovem(z1,z2);
          END
        END
       else
        BEGIN
         DecodeFRegList(ArgStr[1],&z1,&z2);
         if (z1!=0)
          BEGIN
           if ((*AttrPart!='\0') AND (((z1<3) AND (OpSize!=6)) OR ((z1==3) AND (OpSize!=2))))
            WrError(1130);
           else
            BEGIN
             DecodeAdr(ArgStr[2],Madri+Mpost+(z1==3?0:Mpre)+Mdadri+Maix+Mabs);
             WAsmCode[1]=0x2000; GenerateMovem(z1,z2);
            END
          END
         else WrError(1410);
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

/*-------------------------------------------------------------------------*/


        static Boolean DecodeFC(char *Asc, Word *erg)
BEGIN
   Boolean OK;
   Word Val;
   String Asc_N;

   strmaxcpy(Asc_N,Asc,255); NLS_UpString(Asc_N); Asc=Asc_N;

   if (strcmp(Asc,"SFC")==0)
    BEGIN
     *erg=0; return True;
    END

   if (strcmp(Asc,"DFC")==0)
    BEGIN
     *erg=1; return True;
    END

   if ((strlen(Asc)==2) AND (*Asc=='D') AND ValReg(Asc[1]))
    BEGIN
     *erg=Asc[2]-'0'+8; return True;
    END

   if (*Asc=='#')
    BEGIN
     Val=EvalIntExpression(Asc+1,Int4,&OK);
     if (OK) *erg=Val+16; return OK;
    END

   return False;
END

        static Boolean DecodePMMUReg(char *Asc, Word *erg, Byte *Size)
BEGIN
   Byte z;

   if ((strlen(Asc)==4) AND (strncasecmp(Asc,"BAD",3)==0) AND ValReg(Asc[3]))
    BEGIN
     *Size=1;
     *erg=0x7000+((Asc[3]-'0') << 2); return True;
    END
   if ((strlen(Asc)==4) AND (strncasecmp(Asc,"BAC",3)==0) AND ValReg(Asc[3]))
    BEGIN
     *Size=1;
     *erg=0x7400+((Asc[3]-'0') << 2); return True;
    END

   for (z=0; z<PMMURegCnt; z++)
    if (strcasecmp(Asc,PMMURegNames[z])==0) break;
   if (z<PMMURegCnt)
    BEGIN
     *Size=PMMURegSizes[z];
     *erg=PMMURegCodes[z] << 10;
    END
   return (z<PMMURegCnt);
END

        static void DecodePMMUOrders(void)
BEGIN
   Byte z;
   Word Mask;
   LongInt HVal;
   Integer HVal16;
   Boolean ValOK;

   if (Memo("SAVE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else if (NOT FullPMMU) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madri+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
	BEGIN
	 CodeLen=2+AdrCnt; WAsmCode[0]=0xf100 | AdrMode;
	 CopyAdrVals(WAsmCode+1); CheckSup();
	END
      END
     return;
    END

   if (Memo("RESTORE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else if (NOT FullPMMU) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madri+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
	BEGIN
	 CodeLen=2+AdrCnt; WAsmCode[0]=0xf140 | AdrMode;
	 CopyAdrVals(WAsmCode+1); CheckSup();
	END
      END
     return;
    END

   if (Memo("FLUSHA"))
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=4; WAsmCode[0]=0xf000; WAsmCode[1]=0x2400; CheckSup();
      END
     return;
    END

   if ((Memo("FLUSH")) OR (Memo("FLUSHS")))
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if ((ArgCnt!=2) AND (ArgCnt!=3)) WrError(1110);
     else if ((Memo("FLUSHS")) AND (NOT FullPMMU)) WrError(1500);
     else if (NOT DecodeFC(ArgStr[1],WAsmCode+1)) WrError(1710);
     else
      BEGIN
       OpSize=0;
       DecodeAdr(ArgStr[2],Mimm);
       if (AdrNum!=0)
        BEGIN
         if (AdrVals[0]>15) WrError(1720);
         else
          BEGIN
           WAsmCode[1]|=(AdrVals[0] << 5) | 0x3000;
           if (Memo("FLUSHS")) WAsmCode[1]|=0x400;
           WAsmCode[0]=0xf000; CodeLen=4; CheckSup();
           if (ArgCnt==3)
            BEGIN
             WAsmCode[1]|=0x800;
             DecodeAdr(ArgStr[3],Madri+Mdadri+Maix+Mabs);
             if (AdrNum==0) CodeLen=0;
             else
              BEGIN
               WAsmCode[0]|=AdrMode; CodeLen+=AdrCnt;
               CopyAdrVals(WAsmCode+2);
              END
            END
          END
        END
      END
     return;
    END

   if (Memo("FLUSHR"))
    BEGIN
     if (*AttrPart=='\0') OpSize=3;
     if (OpSize!=3) WrError(1130);
     else if (ArgCnt!=1) WrError(1110);
     else if (NOT FullPMMU) WrError(1500);
     else
      BEGIN
       RelPos=4;
       DecodeAdr(ArgStr[1],Madri+Mpre+Mpost+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0xf000 | AdrMode; WAsmCode[1]=0xa000;
         CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt; CheckSup();
        END
      END
     return;
    END

   if ((Memo("LOADR")) OR (Memo("LOADW")))
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeFC(ArgStr[1],WAsmCode+1)) WrError(1710);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Madri+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0xf000 | AdrMode; WAsmCode[1]|=0x2000;
         if (Memo("LOADR")) WAsmCode[1]|=0x200;
         CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2); CheckSup();
        END
      END
     return;
    END

   if ((Memo("MOVE")) OR (Memo("MOVEFD")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (DecodePMMUReg(ArgStr[1],WAsmCode+1,&z))
	BEGIN
	 WAsmCode[1]|=0x200;
	 if (*AttrPart=='\0') OpSize=z;
	 if (OpSize!=z) WrError(1130);
	 else
	  BEGIN
           Mask=Madri+Mdadri+Maix+Mabs;
           if (FullPMMU)
	    BEGIN
	     Mask*=Mpost+Mpre;
             if (z!=3) Mask+=Mdata+Madr;
            END
           DecodeAdr(ArgStr[2],Mask);
	   if (AdrNum!=0)
	    BEGIN
	     WAsmCode[0]=0xf000 | AdrMode;
	     CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
             CheckSup();
	    END
	  END
	END
       else if (DecodePMMUReg(ArgStr[2],WAsmCode+1,&z))
	BEGIN
	 if (*AttrPart=='\0') OpSize=z;
	 if (OpSize!=z) WrError(1130);
	 else
	  BEGIN
           RelPos=4;
           Mask=Madri+Mdadri+Maix+Mabs;
           if (FullPMMU)
	    BEGIN
	     Mask+=Mpost+Mpre+Mpc+Mpcidx+Mimm;
             if (z!=3) Mask+=Mdata+Madr;
            END
           DecodeAdr(ArgStr[1],Mask);
	   if (AdrNum!=0)
	    BEGIN
	     WAsmCode[0]=0xf000 | AdrMode;
	     CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
             if (Memo("MOVEFD")) WAsmCode[1]+=0x100;
             CheckSup();
	    END
	  END
	END
       else WrError(1730);
      END
     return;
    END

   if ((Memo("TESTR")) OR (Memo("TESTW")))
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if ((ArgCnt>4) OR (ArgCnt<3)) WrError(1110);
     else
      BEGIN
       if (NOT DecodeFC(ArgStr[1],WAsmCode+1)) WrError(1710);
       else
        BEGIN
         DecodeAdr(ArgStr[2],Madri+Mdadri+Maix+Mabs);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]=0xf000 | AdrMode; CodeLen=4+AdrCnt;
           WAsmCode[1]|=0x8000;
           CopyAdrVals(WAsmCode+2);
           if (Memo("TESTR")) WAsmCode[1]|=0x200;
           DecodeAdr(ArgStr[3],Mimm);
           if (AdrNum!=0)
            if (AdrVals[0]>7)
             BEGIN
              WrError(1740); CodeLen=0;
             END
            else
             BEGIN
              WAsmCode[1]|=AdrVals[0] << 10;
              if (ArgCnt==4)
               BEGIN
                DecodeAdr(ArgStr[4],Madr);
                if (AdrNum==0) CodeLen=0; else WAsmCode[1]|=AdrMode << 5; 
                CheckSup();
               END
             END
           else CodeLen=0;
          END
        END
      END
     return;
    END

   if (Memo("VALID"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT FullPMMU) WrError(1500);
     else if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Madri+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0xf000 | AdrMode;
         WAsmCode[1]=0x2800; CodeLen=4+AdrCnt;
         CopyAdrVals(WAsmCode+1);
         if (strcasecmp(ArgStr[1],"VAL")==0);
         else
          BEGIN
           DecodeAdr(ArgStr[1],Madr);
           if (AdrNum!=0)
            BEGIN
             WAsmCode[1]|=0x400 | (AdrMode & 7);
            END
           else CodeLen=0;
          END
        END
      END
     return;
    END

   if (*OpPart=='B')
    BEGIN
     for (z=0; z<PMMUCondCnt; z++)
      if (strcmp(OpPart+1,PMMUConds[z])==0) break;
     if (z==PMMUCondCnt) WrError(1360);
     else
      BEGIN
       if ((OpSize!=1) AND (OpSize!=2) AND (OpSize!=6)) WrError(1130);
       else if (ArgCnt!=1) WrError(1110);
       else if (NOT FullPMMU) WrError(1500);
       else
	BEGIN

	 HVal=EvalIntExpression(ArgStr[1],Int32,&ValOK)-(EProgCounter()+2);
	 HVal16=HVal;

	 if (OpSize==1) OpSize=(IsDisp16(HVal))?2:6;

	 if (OpSize==2)
	  BEGIN
           if ((NOT IsDisp16(HVal)) AND (NOT SymbolQuestionable)) WrError(1370);
	   else
	    BEGIN
	     CodeLen=4; WAsmCode[0]=0xf080 | z;
	     WAsmCode[1]=HVal16; CheckSup();
	    END
	  END
	 else
	  BEGIN
	   CodeLen=6; WAsmCode[0]=0xf0c0 | z;
	   WAsmCode[2]=HVal & 0xffff; WAsmCode[1]=HVal >> 16;
	   CheckSup();
	  END
	END
      END
     return;
    END;

   if (strncmp(OpPart,"DB",2)==0)
    BEGIN
     for (z=0; z<PMMUCondCnt; z++)
      if (strcmp(OpPart+2,PMMUConds[z])==0) break;
     if (z==PMMUCondCnt) WrError(1360);  
     else
      BEGIN
       if ((OpSize!=1) AND (*AttrPart!='\0')) WrError(1130);
       else if (ArgCnt!=2) WrError(1110);
       else if (NOT FullPMMU) WrError(1500);
       else
        BEGIN
         DecodeAdr(ArgStr[1],Mdata);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]=0xf048 | AdrMode; WAsmCode[1]=z;
           HVal=EvalIntExpression(ArgStr[2],Int32,&ValOK)-(EProgCounter()+4);
           if (ValOK)
            BEGIN
             HVal16=HVal; WAsmCode[2]=HVal16;
             if ((NOT IsDisp16(HVal)) AND (NOT SymbolQuestionable)) WrError(1370); 
             else CodeLen=6; 
             CheckSup();
            END
          END
        END
      END
     return;
    END

   if (*OpPart=='S')
    BEGIN
     for (z=0; z<PMMUCondCnt; z++)
      if (strcmp(OpPart+1,PMMUConds[z])==0) break;
     if (z==PMMUCondCnt) WrError(1360);  
     else
      BEGIN
       if ((OpSize!=0) AND (*AttrPart!='\0')) WrError(1130);
       else if (ArgCnt!=1) WrError(1110);
       else if (NOT FullPMMU) WrError(1500);
       else
        BEGIN
         DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
         if (AdrNum!=0)
          BEGIN
           CodeLen=4+AdrCnt; WAsmCode[0]=0xf040 | AdrMode;
           WAsmCode[1]=z; CopyAdrVals(WAsmCode+2); CheckSup();
          END
        END
      END
     return;
    END

   if (strncmp(OpPart,"TRAP",4)==0)
    BEGIN
     for (z=0; z<PMMUCondCnt; z++)
      if (strcmp(OpPart+4,PMMUConds[z])==0) break;
     if (z==PMMUCondCnt) WrError(1360);  
     else
      BEGIN
       if (*AttrPart=='\0') OpSize=0;
       if (OpSize>2) WrError(1130);
       else if (((OpSize==0) AND (ArgCnt!=0)) OR ((OpSize!=0) AND (ArgCnt!=1))) WrError(1110);
       else if (NOT FullPMMU) WrError(1500);
       else
        BEGIN
         WAsmCode[0]=0xf078; WAsmCode[1]=z;
         if (OpSize==0)
          BEGIN
           WAsmCode[0]|=4; CodeLen=4; CheckSup();
          END
         else
          BEGIN
           DecodeAdr(ArgStr[1],Mimm);
           if (AdrNum!=0)
            BEGIN
             WAsmCode[0]|=(OpSize+1);
             CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt; CheckSup();
            END
          END
        END
      END
     return;
    END

   WrError(1200);
END

/*-------------------------------------------------------------------------*/

	static Boolean CodeSingle(void)
BEGIN
   if (Memo("PEA")) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1100);
     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       OpSize=0;
       DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
       if (AdrNum!=0) 
	BEGIN
	 CodeLen=2+AdrCnt;
	 WAsmCode[0]=0x4840 | AdrMode;
	 CopyAdrVals(WAsmCode+1);
	END
      END
     return True;
    END

   if ((Memo("CLR")) OR (Memo("TST")))
    BEGIN
     if (OpSize>2) WrError(1130);
     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0) 
	BEGIN
	 CodeLen=2+AdrCnt;
	 WAsmCode[0]=(Memo("TST")) ? 0x4a00 : 0x4200;
	 WAsmCode[0] |= ((OpSize << 6) | AdrMode);
	 CopyAdrVals(WAsmCode+1);
	END
      END
     return True;
    END

   if ((Memo("JMP")) OR (Memo("JSR"))) 
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
       if (AdrNum!=0) 
	BEGIN
	 CodeLen=2+AdrCnt;
	 WAsmCode[0]=(Memo("JMP"))?0x4ec0:0x4e80;
	 WAsmCode[0]|=AdrMode;
	 CopyAdrVals(WAsmCode+1);
	END
      END
     return True;
    END

   if ((Memo("NBCD")) OR (Memo("TAS"))) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize!=0)) WrError(1130);
     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       OpSize=0;
       DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0) 
	BEGIN
	 CodeLen=2+AdrCnt;
	 WAsmCode[0]=(Memo("NBCD"))?0x4800:0x4ac0;
	 WAsmCode[0]|=AdrMode;
	 CopyAdrVals(WAsmCode+1);
	END
      END
     return True;
    END

   if ((Memo("NEG")) OR (Memo("NOT")) OR (Memo("NEGX"))) 
    BEGIN
     if (OpSize>2) WrError(1130);
     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
	BEGIN
	 CodeLen=2+AdrCnt;
	 if (Memo("NOT")) WAsmCode[0]=0x4600;
	 else WAsmCode[0]=(Memo("NEG"))?0x4400:0x4000;
	 WAsmCode[0]|=((OpSize << 6) | AdrMode);
	 CopyAdrVals(WAsmCode+1);
	END
      END
     return True;
    END

   if (Memo("SWAP")) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata);
       if (AdrNum!=0) 
	BEGIN
	 CodeLen=2; WAsmCode[0]=0x4840 | AdrMode;
	END
      END
     return True;
    END

   if (Memo("UNLK")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madr);
       if (AdrNum!=0) 
	BEGIN
	 CodeLen=2; WAsmCode[0]=0x4e58 | AdrMode;
	END
      END
     return True;
    END

   if (Memo("EXT")) 
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if ((OpSize==0) OR (OpSize>2)) WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata);
       if (AdrNum==1) 
	BEGIN
	 WAsmCode[0]=0x4880 | AdrMode | (((Word)OpSize-1) << 6);
	 CodeLen=2;
	END
      END
     return True;
    END

   return False;
END

	static Boolean CodeDual(void)
BEGIN
   Boolean ValOK;
   ShortInt HVal8;

   if ((Memo("MOVE")) OR (Memo("MOVEA"))) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"USP")==0) 
      BEGIN
       if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
       else
	BEGIN
	 DecodeAdr(ArgStr[2],Madr);
	 if (AdrNum!=0) 
	  BEGIN
	   CodeLen=2; WAsmCode[0]=0x4e68 | (AdrMode & 7); CheckSup();
	  END
	END
      END
     else if (strcasecmp(ArgStr[2],"USP")==0) 
      BEGIN
       if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
       else
	BEGIN
	 DecodeAdr(ArgStr[1],Madr);
	 if (AdrNum!=0) 
	  BEGIN
	   CodeLen=2; WAsmCode[0]=0x4e60 | (AdrMode & 7); CheckSup();
	  END
	END
      END
     else if (strcasecmp(ArgStr[1],"SR")==0) 
      BEGIN
       if (OpSize!=1) WrError(1130);
       else
	BEGIN
	 DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	 if (AdrNum!=0) 
	  BEGIN
	   CodeLen=2+AdrCnt; WAsmCode[0]=0x40c0 | AdrMode;
	   CopyAdrVals(WAsmCode+1);
	   if (MomCPU>=CPU68010) CheckSup();
	  END
	END
      END
     else if (strcasecmp(ArgStr[1],"CCR")==0) 
      BEGIN
       if ((*AttrPart!='\0') AND (OpSize!=0)) WrError(1130);
       else
	BEGIN
         OpSize=0;
	 DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	 if (AdrNum!=0) 
	  BEGIN
	   CodeLen=2+AdrCnt; WAsmCode[0]=0x42c0 | AdrMode;
	   CopyAdrVals(WAsmCode+1); CheckCPU(CPU68010);
	  END
	END
      END
     else if (strcasecmp(ArgStr[2],"SR")==0) 
      BEGIN
       if (OpSize!=1) WrError(1130);
       else
	BEGIN
	 DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 if (AdrNum!=0) 
	  BEGIN
	   CodeLen=2+AdrCnt; WAsmCode[0]=0x46c0 | AdrMode;
	   CopyAdrVals(WAsmCode+1); CheckSup();
	  END
	END
      END
     else if (strcasecmp(ArgStr[2],"CCR")==0) 
      BEGIN
       if ((*AttrPart!='\0') AND (OpSize!=0)) WrError(1130);
       else
	BEGIN
         OpSize=0;
	 DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 if (AdrNum!=0) 
	  BEGIN
	   CodeLen=2+AdrCnt; WAsmCode[0]=0x44c0 | AdrMode;
	   CopyAdrVals(WAsmCode+1);
	  END
	END
      END
     else
      BEGIN
       if (OpSize>2) WrError(1130);
       else
	BEGIN
	 DecodeAdr(ArgStr[1],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 if (AdrNum!=0) 
	  BEGIN
	   CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
	   if (OpSize==0) WAsmCode[0]=0x1000;
	   else if (OpSize==1) WAsmCode[0]=0x3000;
	   else WAsmCode[0]=0x2000;
	   WAsmCode[0]|=AdrMode;
	   DecodeAdr(ArgStr[2],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	   if (AdrMode!=0) 
	    BEGIN
	     AdrMode=((AdrMode & 7) << 3) | (AdrMode >> 3);
	     WAsmCode[0]|=AdrMode << 6;
	     CopyAdrVals(WAsmCode+(CodeLen >> 1));
	     CodeLen+=AdrCnt;
	    END
	  END
	END
      END
     return True;
    END

   if (Memo("LEA")) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Madr);
       if (AdrNum!=0) 
	BEGIN
         OpSize=0;
	 WAsmCode[0]=0x41c0 | ((AdrMode & 7) << 9);
	 DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
	 if (AdrNum!=0) 
	  BEGIN
	   WAsmCode[0]|=AdrMode; CodeLen=2+AdrCnt;
	   CopyAdrVals(WAsmCode+1);
	  END
	END
      END
     return True;
    END

   if ((Memo("ASL")) OR (Memo("ASR")) OR (Memo("LSL" )) OR (Memo("LSR" ))
    OR (Memo("ROL")) OR (Memo("ROR")) OR (Memo("ROXL")) OR (Memo("ROXR"))) 
    BEGIN
     if (ArgCnt==1) 
      BEGIN
       strcpy(ArgStr[2],ArgStr[1]); strcpy(ArgStr[1],"#1");
       ArgCnt=2;
      END
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum==1) 
	BEGIN
	 if (OpSize>2) WrError(1130);
	 else
	  BEGIN
	   WAsmCode[0]=0xe000 | AdrMode | (ShiftCodes(OpPart) << 3) | (OpSize << 6);
	   if (OpPart[strlen(OpPart)-1]=='L') WAsmCode[0]|=0x100;
	   OpSize=1;
	   DecodeAdr(ArgStr[1],Mdata+Mimm);
	   if ((AdrNum==1) OR ((AdrNum==11) AND (Lo(AdrVals[0])>=1) AND (Lo(AdrVals[0])<=8)))
	    BEGIN
	     CodeLen=2;
             WAsmCode[0] |= (AdrNum==1) ? 0x20|(AdrMode<<9) : ((AdrVals[0] & 7) << 9);
	    END
           else WrError(1380);
	  END
	END
       else if (AdrNum!=0) 
	BEGIN
	 if (OpSize!=1) WrError(1130);
	 else
	  BEGIN
	   WAsmCode[0]=0xe0c0 | AdrMode | (ShiftCodes(OpPart) << 9);
	   if (OpPart[strlen(OpPart)-1]=='L') WAsmCode[0]|=0x100;
	   CopyAdrVals(WAsmCode+1);
	   if (*ArgStr[1]=='#') strcpy(ArgStr[1],ArgStr[1]+1);
	   HVal8=EvalIntExpression(ArgStr[1],Int8,&ValOK);
	   if ((ValOK) AND (HVal8==1)) CodeLen=2+AdrCnt;
           else WrError(1390);
	  END
	END
      END
     return True;
    END

   if ((Memo("ADDQ")) OR (Memo("SUBQ"))) 
    BEGIN
     if (OpSize>2) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0) 
	BEGIN
	 WAsmCode[0]=0x5000 | AdrMode | (OpSize << 6);
	 if (Memo("SUBQ")) WAsmCode[0]|=0x100;
	 CopyAdrVals(WAsmCode+1);
	 if (*ArgStr[1]=='#') strcpy(ArgStr[1],ArgStr[1]+1);
	 HVal8=EvalIntExpression(ArgStr[1],Int8,&ValOK);
	 if ((ValOK) AND (HVal8>=1) AND (HVal8<=8)) 
	  BEGIN
	   CodeLen=2+AdrCnt;
	   WAsmCode[0]|=(((Word) HVal8 & 7) << 9);
	  END
         else WrError(1390);
	END
      END
     return True;
    END

   if ((Memo("ADDX")) OR (Memo("SUBX"))) 
    BEGIN
     if (OpSize>2) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Mpre);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=0x9100 | (OpSize << 6) OR (AdrMode & 7);
	 if (AdrNum==5) WAsmCode[0]|=8;
	 if (strcmp(OpPart,"ADDX")==0) WAsmCode[0]|=0x4000;
	 DecodeAdr(ArgStr[2],Masks[AdrNum]);
	 if (AdrNum!=0) 
	  BEGIN
	   CodeLen=2;
	   WAsmCode[0]|= (AdrMode & 7) << 9;
	  END
	END
      END
     return True;
    END

   if (Memo("CMPM")) 
    BEGIN
     if (OpSize>2) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mpost);
       if (AdrNum==4) 
	BEGIN
	 WAsmCode[0]=0xb108 | (OpSize << 6) | (AdrMode & 7);
	 DecodeAdr(ArgStr[2],Mpost);
	 if (AdrNum==4) 
	  BEGIN
	   WAsmCode[0]|=(AdrMode & 7) << 9;
	   CodeLen=2;
	  END
	END
      END
     return True;
    END

   if ((strncmp(OpPart,"ADD",3)==0) OR (strncmp(OpPart,"SUB",3)==0) OR ((strncmp(OpPart,"CMP",3)==0) AND (OpPart[3]!='2')))
    BEGIN
     OpPart[3]='\0';
     if (OpSize>2) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum==2)        /* ADDA ? */
	if (OpSize==0) WrError(1130);
	else
	 BEGIN
	  WAsmCode[0]=0x90c0 | ((AdrMode & 7) << 9);
	  if (strcmp(OpPart,"ADD")==0) WAsmCode[0]|=0x4000;
	  else if (strcmp(OpPart,"CMP")==0) WAsmCode[0]|=0x2000;
	  if (OpSize==2) WAsmCode[0]|=0x100;
	  DecodeAdr(ArgStr[1],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	  if (AdrNum!=0) 
	   BEGIN
	    WAsmCode[0]|=AdrMode; CodeLen=2+AdrCnt;
	    CopyAdrVals(WAsmCode+1);
	   END
	 END
       else if (AdrNum==1)      /* ADD <EA>,Dn ? */
	BEGIN
	 WAsmCode[0]=0x9000 | (OpSize << 6) | (AdrMode << 9);
	 if (strcmp(OpPart,"ADD")==0) WAsmCode[0]|=0x4000;
	 else if (strcmp(OpPart,"CMP")==0) WAsmCode[0]|=0x2000;
	 DecodeAdr(ArgStr[1],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 if (AdrNum!=0) 
	  BEGIN
	   CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
	   WAsmCode[0]|=AdrMode;
	  END
	END
       else
	BEGIN
	 DecodeAdr(ArgStr[1],Mdata+Mimm);
	 if (AdrNum==11)        /* ADDI ? */
	  BEGIN
	   WAsmCode[0]=0x400 | (OpSize << 6);
	   if (strcmp(OpPart,"ADD")==0) WAsmCode[0]|=0x200;
	   else if (strcmp(OpPart,"CMP")==0) WAsmCode[0]|=0x800;
	   CodeLen=2+AdrCnt;
	   CopyAdrVals(WAsmCode+1);
	   DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	   if (AdrNum!=0) 
	    BEGIN
	     WAsmCode[0]|=AdrMode;
	     CopyAdrVals(WAsmCode+(CodeLen >> 1));
	     CodeLen+=AdrCnt;
	    END
	   else CodeLen=0;
	  END
	 else if (AdrNum!=0)    /* ADD Dn,<EA> ? */
	  BEGIN
	   if (strcmp(OpPart,"CMP")==0) WrError(1420);
	   else
	    BEGIN
	     WAsmCode[0]=0x9100 | (OpSize << 6) | (AdrMode << 9);
	     if (strcmp(OpPart,"ADD")==0) WAsmCode[0]|=0x4000;
	     DecodeAdr(ArgStr[2],Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	     if (AdrNum!=0) 
	      BEGIN
	       CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
	       WAsmCode[0]|=AdrMode;
	      END
	    END
	  END
	END
      END
     return True;
    END

   if ((strncmp(OpPart,"AND",3)==0) OR (strncmp(OpPart,"OR",2)==0)) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (OpSize>2) WrError(1130);
     else
      BEGIN
       if ((strcasecmp(ArgStr[2],"CCR")!=0) AND (strcasecmp(ArgStr[2],"SR")!=0)) 
        DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (strcasecmp(ArgStr[2],"CCR")==0)     /* AND #...,CCR */
	BEGIN
	 if ((*AttrPart!='\0') AND (OpSize!=0)) WrError(1130);
	 else
	  BEGIN
	   WAsmCode[0] = (strncmp(OpPart,"AND",3)==0) ? 0x023c : 0x003c;
	   OpSize=0; DecodeAdr(ArgStr[1],Mimm);
	   if (AdrNum!=0) 
	    BEGIN
	     CodeLen=4; WAsmCode[1]=AdrVals[0];
	     CheckCPU(CPU68000);
	    END
	  END
	END
       else if (strcasecmp(ArgStr[2],"SR")==0) /* AND #...,SR */
	BEGIN
	 if (OpSize!=1) WrError(1130);
	 else
	  BEGIN
	   WAsmCode[0] = (strncmp(OpPart,"AND",3)==0) ? 0x027c : 0x007c;
	   OpSize=1; DecodeAdr(ArgStr[1],Mimm);
	   if (AdrNum!=0) 
	    BEGIN
	     CodeLen=4; WAsmCode[1]=AdrVals[0]; CheckSup();
	     CheckCPU(CPU68000);
	    END
	  END
	END
       else if (AdrNum==1)                 /* AND <EA>,Dn */
	BEGIN
	 WAsmCode[0]=0x8000 | (OpSize << 6) | (AdrMode << 9);
	 if (strncmp(OpPart,"AND",3)==0) WAsmCode[0]|=0x4000;
	 DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 if (AdrNum!=0) 
	  BEGIN
	   WAsmCode[0]|=AdrMode;
	   CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
	  END
	END
       else if (AdrNum!=0)                 /* AND ...,<EA> */
	BEGIN
	 DecodeAdr(ArgStr[1],Mdata+Mimm);
	 if (AdrNum==11)                   /* AND #..,<EA> */
	  BEGIN
	   WAsmCode[0]=OpSize << 6;
	   if (strncmp(OpPart,"AND",3)==0) WAsmCode[0]|=0x200;
	   CodeLen=2+AdrCnt;
	   CopyAdrVals(WAsmCode+1);
	   DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	   if (AdrNum!=0)
	    BEGIN
	     WAsmCode[0]|=AdrMode;
	     CopyAdrVals(WAsmCode+(CodeLen >> 1));
	     CodeLen+=AdrCnt;
	    END
	   else CodeLen=0;
	  END
	 else if (AdrNum!=0)               /* AND Dn,<EA> ? */
	  BEGIN
	   WAsmCode[0]=0x8100 | (OpSize << 6) | (AdrMode << 9);
	   if (strncmp(OpPart,"AND",3)==0) WAsmCode[0]|=0x4000;
	   DecodeAdr(ArgStr[2],Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	   if (AdrNum!=0)
	    BEGIN
	     CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
	     WAsmCode[0]|=AdrMode;
	    END
	  END
	END
      END
     return True;
    END

   if (strncmp(OpPart,"EOR",3)==0) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[2],"CCR")==0) 
      BEGIN
       if ((*AttrPart!='\0') AND (OpSize!=0)) WrError(1130);
       else
	BEGIN
	 WAsmCode[0]=0xa3c; OpSize=0;
	 DecodeAdr(ArgStr[1],Mimm);
	 if (AdrNum!=0)
	  BEGIN
	   CodeLen=4; WAsmCode[1]=AdrVals[0];
	  END
	END
      END
     else if (strcasecmp(ArgStr[2],"SR")==0) 
      BEGIN
       if (OpSize!=1) WrError(1130);
       else
	BEGIN
	 WAsmCode[0]=0xa7c;
	 DecodeAdr(ArgStr[1],Mimm);
	 if (AdrNum!=0) 
	  BEGIN
	   CodeLen=4; WAsmCode[1]=AdrVals[0]; CheckSup();
	   CheckCPU(CPU68000);
	  END
	END
      END
     else if (OpSize>2) WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Mimm);
       if (AdrNum==1) 
	BEGIN
	 WAsmCode[0]=0xb100 | (AdrMode << 9) | (OpSize << 6);
	 DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	 if (AdrNum!=0)
	  BEGIN
	   CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
	   WAsmCode[0]|=AdrMode;
	  END
	END
       else if (AdrNum==11) 
	BEGIN
	 WAsmCode[0]=0x0a00 | (OpSize << 6);
	 CopyAdrVals(WAsmCode+1); CodeLen=2+AdrCnt;
	 DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	 if (AdrNum!=0)
	  BEGIN
	   CopyAdrVals(WAsmCode+(CodeLen >> 1));
	   CodeLen+=AdrCnt;
	   WAsmCode[0]|=AdrMode;
	  END
	 else CodeLen=0;
	END
      END
     return True;
    END

   return False;
END

	static Boolean CodeBits(void)
BEGIN
   if ((Memo("BSET")) OR (Memo("BCLR")) OR (Memo("BCHG")) OR (Memo("BTST"))) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (*AttrPart=='\0') OpSize=0;
       if (strcmp(OpPart,"BTST")!=0) DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       else DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs);
       if (*AttrPart=='\0') OpSize=(AdrNum==1) ? 2 : 0;
       if (AdrNum!=0)
	BEGIN
         if (((AdrNum==1) AND (OpSize!=2)) OR ((AdrNum!=1) AND (OpSize!=0))) WrError(1130);
         else
          BEGIN
           WAsmCode[0]=AdrMode;
           if (Memo("BSET")) WAsmCode[0]|=0xc0;
           else if (Memo("BCLR")) WAsmCode[0]|=0x80;
           else if (Memo("BCHG")) WAsmCode[0]|=0x40;
           CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
           OpSize=0;
           DecodeAdr(ArgStr[1],Mdata+Mimm);
           if (AdrNum==1) 
            BEGIN
             WAsmCode[0]|=0x100 | (AdrMode << 9);
            END
           else if (AdrNum==11) 
            BEGIN
             memmove(WAsmCode+2,WAsmCode+1,CodeLen-2); WAsmCode[1]=AdrVals[0];
             WAsmCode[0]|=0x800;
             CodeLen+=2;
             if ((AdrVals[0]>31)
             OR  (((WAsmCode[0] & 0x38)!=0) AND (AdrVals[0]>7))) 
              BEGIN
               CodeLen=0; WrError(1510);
              END
            END
           else CodeLen=0;
          END
	END
      END
     return True;
    END

   if ((Memo("BFSET")) OR (Memo("BFCLR"))
    OR (Memo("BFCHG")) OR (Memo("BFTST"))) 
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else if (NOT SplitBitField(ArgStr[1],WAsmCode+1)) WrError(1750);
     else
      BEGIN
       RelPos=4;
       OpSize=0;
       if (Memo("BFTST")) DecodeAdr(ArgStr[1],Mdata+Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
       else DecodeAdr(ArgStr[1],Mdata+Madri+Mdadri+Maix+Mabs);
       if (AdrNum!=0) 
	BEGIN
	 WAsmCode[0]=0xe8c0 | AdrMode;
	 CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
	 if (Memo("BFSET")) WAsmCode[0]|=0x600;
	 else if (Memo("BFCLR")) WAsmCode[0]|=0x400;
	 else if (Memo("BFCHG")) WAsmCode[0]|=0x200;
	 CheckCPU(CPU68020);
	END
      END
     return True;
    END

   if ((Memo("BFEXTS")) OR (Memo("BFEXTU")) OR (Memo("BFFFO"))) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else if (NOT SplitBitField(ArgStr[1],WAsmCode+1)) WrError(1750);
     else
      BEGIN
       RelPos=4;
       OpSize=0;
       DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=0xe9c0+AdrMode; CopyAdrVals(WAsmCode+2);
	 if (Memo("BFEXTS")) WAsmCode[0]+=0x200;
	 else if (Memo("BFFFO")) WAsmCode[0]+=0x400;
	 CodeLen=4+AdrCnt;
	 DecodeAdr(ArgStr[2],Mdata);
	 if (AdrNum!=0)
	  BEGIN
	   WAsmCode[1]|=AdrMode << 12;
	   CheckCPU(CPU68020);
	  END
	 else CodeLen=0;
	END
      END
     return True;
    END

   if (Memo("BFINS")) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else if (NOT SplitBitField(ArgStr[2],WAsmCode+1)) WrError(1750);
     else
      BEGIN
       OpSize=0;
       DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=0xefc0+AdrMode; CopyAdrVals(WAsmCode+2);
	 CodeLen=4+AdrCnt;
	 DecodeAdr(ArgStr[1],Mdata);
	 if (AdrNum!=0)
	  BEGIN
	   WAsmCode[1]|=AdrMode << 12;
	   CheckCPU(CPU68020);
	  END
	 else CodeLen=0;
	END
      END
     return True;
    END

   return False;
END

       	static void MakeCode_68K(void)
BEGIN
   Boolean ValOK;
   char *p;
   LongInt HVal;
   Integer i,HVal16;
   ShortInt HVal8;
   Byte z;
   Word w1,w2;

   CodeLen=0; OpSize=1; DontPrint=False; RelPos=2;

   if (*AttrPart!='\0')
    switch (toupper(*AttrPart))
     BEGIN
      case 'B': OpSize=0; break;
      case 'W': OpSize=1; break;
      case 'L': OpSize=2; break;
      case 'Q': OpSize=3; break;
      case 'S': OpSize=4; break;
      case 'D': OpSize=5; break;
      case 'X': OpSize=6; break;
      case 'P': OpSize=7; break;
      default: WrError(1107); return;
     END

   /* Nullanweisung */

   if ((Memo("")) AND (*AttrPart=='\0') AND (ArgCnt==0)) return;

   /* Pseudoanweisungen */

   if (DecodeMoto16Pseudo(OpSize,True)) return;

   if (DecodePseudo()) return;

   /* Befehlserweiterungen */

   if ((*OpPart=='F') AND (FPUAvail))
    BEGIN
     strcpy(OpPart,OpPart+1);
     DecodeFPUOrders();
     return;
    END

   if ((*OpPart=='P') AND (NOT Memo("PEA")) AND (PMMUAvail))
    BEGIN
     strcpy(OpPart,OpPart+1);
     DecodePMMUOrders();
     return;
    END

   /* Anweisungen ohne Argument */

   if (Memo("NOP"))
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=2; WAsmCode[0]=0x4e71;
      END
     return;
    END

   if (Memo("RESET")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=2; WAsmCode[0]=0x4e70; CheckSup();
      END
     return;
    END

   if (Memo("ILLEGAL")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=2; WAsmCode[0]=0x4afc;
      END
     return;
    END

   if (Memo("TRAPV")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=2; WAsmCode[0]=0x4e76;
      END
     return;
    END

   if (Memo("RTE")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=2; WAsmCode[0]=0x4e73; CheckSup();
      END
     return;
    END

   if (Memo("RTR")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=2; WAsmCode[0]=0x4e77;
      END
     return;
    END

   if (Memo("RTS")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=2; WAsmCode[0]=0x4e75;
      END
     return;
    END

   if (Memo("BGND")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=2; WAsmCode[0]=0x4afa;
       Check32();
      END
     return;
    END

   /* Anweisungen mit konstantem Argument */

   if (Memo("STOP")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]!='#') WrError(1120);
     else
      BEGIN
       HVal=EvalIntExpression(ArgStr[1]+1,Int16,&ValOK);
       if (ValOK) 
	BEGIN
	 CodeLen=4; WAsmCode[0]=0x4e72; WAsmCode[1]=HVal; CheckSup();
	END
      END
     return;
    END

   if (Memo("LPSTOP")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]!='#') WrError(1120);
     else
      BEGIN
       HVal=EvalIntExpression(ArgStr[1]+1,Int16,&ValOK);
       if (ValOK) 
	BEGIN
	 CodeLen=6;
	 WAsmCode[0]=0xf800;
	 WAsmCode[1]=0x01c0;
	 WAsmCode[2]=HVal;
	 CheckSup(); Check32();
	END
      END
     return;
    END

   if (Memo("TRAP")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]!='#') WrError(1120);
     else
      BEGIN
       HVal=EvalIntExpression(ArgStr[1]+1,Int4,&ValOK);
       if (ValOK) 
	BEGIN
	 HVal8=HVal;
	 CodeLen=2; WAsmCode[0]=0x4e40+(HVal8 & 15);
	END
      END
     return;
    END

   if (Memo("BKPT")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]!='#') WrError(1120);
     else
      BEGIN
       HVal=EvalIntExpression(ArgStr[1]+1,UInt3,&ValOK);
       if (ValOK) 
	BEGIN
	 HVal8=HVal;
	 CodeLen=2; WAsmCode[0]=0x4848+(HVal8 & 7);
	 CheckCPU(CPU68010);
	END;
      END
     return;
    END

   if (Memo("RTD")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]!='#') WrError(1120);
     else
      BEGIN
       HVal=EvalIntExpression(ArgStr[1]+1,Int16,&ValOK);
       if (ValOK) 
	BEGIN
	 CodeLen=4; WAsmCode[0]=0x4e74; WAsmCode[1]=HVal;
	 CheckCPU(CPU68010);
	END
      END
     return;
    END

   /* Anweisungen mit einem Speicheroperanden */

   if (CodeSingle()) return;

   /* zwei Operanden */


   if (CodeDual()) return;

   if (CodeBits()) return;

   if (Memo("MOVEM")) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
     else
      BEGIN
       if (DecodeRegList(ArgStr[2],WAsmCode+1))
	BEGIN
	 DecodeAdr(ArgStr[1],Madri+Mpost+Mdadri+Maix+Mpc+Mpcidx+Mabs);
	 if (AdrNum!=0)
	  BEGIN
	   WAsmCode[0]=0x4c80 | AdrMode | ((OpSize-1) << 6);
	   CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
	  END
	END
       else if (DecodeRegList(ArgStr[1],WAsmCode+1)) 
	BEGIN
	 DecodeAdr(ArgStr[2],Madri+Mpre+Mdadri+Maix+Mabs);
	 if (AdrNum!=0) 
	  BEGIN
	   WAsmCode[0]=0x4880 | AdrMode | ((OpSize-1) << 6);
	   CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
	   if (AdrNum==5) 
	    BEGIN
	     WAsmCode[9]=WAsmCode[1]; WAsmCode[1]=0;
	     for (z=0; z<16; z++)
	      BEGIN
	       WAsmCode[1]=WAsmCode[1] << 1;
	       if ((WAsmCode[9]&1)==1) WAsmCode[1]++;
	       WAsmCode[9]=WAsmCode[9] >> 1;
	      END
	    END
	  END
	END
       else WrError(1410);
      END
     return;
    END

   if (Memo("MOVEQ")) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Mdata);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=0x7000 | (AdrMode << 9);
	 OpSize=0;
	 DecodeAdr(ArgStr[1],Mimm);
	 if (AdrNum!=0) 
	  BEGIN
	   CodeLen=2; WAsmCode[0]|=AdrVals[0];
	  END
	END
      END
     return;
    END

   if (Memo("EXG")) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Madr);
       if (AdrNum==1)
	BEGIN
	 WAsmCode[0]=0xc100 | (AdrMode << 9);
	 DecodeAdr(ArgStr[2],Mdata+Madr);
	 if (AdrNum==1)
	  BEGIN
	   WAsmCode[0]|=0x40 | AdrMode; CodeLen=2;
	  END
	 else if (AdrNum==2)
	  BEGIN
	   WAsmCode[0]|=0x88 | (AdrMode & 7); CodeLen=2;
	  END
	END
       else if (AdrNum==2)
	BEGIN
	 WAsmCode[0]=0xc100 | (AdrMode & 7);
	 DecodeAdr(ArgStr[2],Mdata+Madr);
	 if (AdrNum==1)
	  BEGIN
	   WAsmCode[0]|=0x88 OR (AdrMode << 9); CodeLen=2;
	  END
	 else
	  BEGIN
	   WAsmCode[0]|=0x48 | ((AdrMode & 7) << 9); CodeLen=2;
	  END
	END
      END
     return;
    END

   if ((Memo("DIVSL")) OR (Memo("DIVUL"))) 
    BEGIN
     if (*AttrPart=='\0') OpSize=2;
     if (ArgCnt!=2) WrError(1110);
     else if (OpSize!=2) WrError(1130);
     else if (NOT CodeRegPair(ArgStr[2],&w1,&w2)) WrError(1760);
     else
      BEGIN
       RelPos=4;
       WAsmCode[1]=w1+(w2 << 12);
       if (OpPart[3]=='S') WAsmCode[1]|=0x800;
       DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=0x4c40+AdrMode;
	 CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
	 CheckCPU(CPU68332);
	END
      END
     return;
    END

   if ((strncmp(OpPart,"MUL",3)==0) OR (strncmp(OpPart,"DIV",3)==0)) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (OpSize==1) 
      BEGIN
       DecodeAdr(ArgStr[2],Mdata);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=0x80c0 | (AdrMode << 9);
	 if (strncmp(OpPart,"MUL",3)==0) WAsmCode[0]|=0x4000;
	 if (OpPart[3]=='S') WAsmCode[0]|=0x100;
	 DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 if (AdrNum!=0)
	  BEGIN
	   WAsmCode[0]|=AdrMode;
	   CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
	  END
	END
      END
     else if (OpSize==2)
      BEGIN
       if (strchr(ArgStr[2],':')==Nil)
        BEGIN
         strcat(ArgStr[2],":");
         strcat(ArgStr[2],ArgStr[2]);
         ArgStr[2][strlen(ArgStr[2])-1]='\0';
        END
       if (NOT CodeRegPair(ArgStr[2],&w1,&w2)) WrError(1760);
       else
	BEGIN
	 WAsmCode[1]=w1+(w2 << 12); RelPos=4;
	 if (w1!=w2) WAsmCode[1]|=0x400;
	 if (OpPart[3]=='S') WAsmCode[1]|=0x800;
	 DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 if (AdrNum!=0)
	  BEGIN
	   WAsmCode[0]=0x4c00+AdrMode;
	   if (strncmp(OpPart,"DIV",3)==0) WAsmCode[0]|=0x40;
	   CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
	   CheckCPU(CPU68332);
	  END
	END
      END
     else WrError(1130);
     return;
    END

   if ((Memo("ABCD")) OR (Memo("SBCD"))) 
    BEGIN
     if ((OpSize!=0) AND (*AttrPart!='\0')) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       OpSize=0;
       DecodeAdr(ArgStr[1],Mdata+Mpre);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=0x8100 | (AdrMode & 7);
	 if (AdrNum==5) WAsmCode[0]|=8;
	 if (Memo("ABCD")) WAsmCode[0]|=0x4000;
	 DecodeAdr(ArgStr[2],Masks[AdrNum]);
	 if (AdrNum!=0)
	  BEGIN
	   CodeLen=2;
	   WAsmCode[0]|=(AdrMode & 7) << 9;
	  END
	END
      END
     return;
    END

   if (Memo("CHK")) 
    BEGIN
     if (OpSize!=1) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=0x4180 | AdrMode; CodeLen=2+AdrCnt;
	 CopyAdrVals(WAsmCode+1);
	 DecodeAdr(ArgStr[2],Mdata);
	 if (AdrNum==1) WAsmCode[0]|=WAsmCode[0] | (AdrMode << 9);
	 else CodeLen=0;
	END
      END
     return;
    END

   if (Memo("LINK")) 
    BEGIN
     if ((OpSize<1) OR (OpSize>2)) WrError(1130);
     else if ((OpSize==2) AND (MomCPU<CPU68332)) WrError(1500);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madr);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=(OpSize==1) ? 0x4e50 : 0x4808;
	 WAsmCode[0]+=AdrMode & 7;
	 DecodeAdr(ArgStr[2],Mimm);
	 if (AdrNum==11)
	  BEGIN
	   CodeLen=2+AdrCnt; memcpy(WAsmCode+1,AdrVals,AdrCnt);
	  END
	END
      END
     return;
    END

   if (Memo("MOVEP")) 
    BEGIN
     if ((OpSize==0) OR (OpSize>2)) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Mdadri);
       if (AdrNum==1)
	BEGIN
	 WAsmCode[0]=0x188 | ((OpSize-1) << 6) | (AdrMode << 9);
	 DecodeAdr(ArgStr[2],Mdadri);
	 if (AdrNum==6)
	  BEGIN
	   WAsmCode[0]|=AdrMode & 7;
	   CodeLen=4; WAsmCode[1]=AdrVals[0];
	  END
	END
       else if (AdrNum==6)
	BEGIN
	 WAsmCode[0]=0x108 | ((OpSize-1) << 6) | (AdrMode & 7);
	 WAsmCode[1]=AdrVals[0];
	 DecodeAdr(ArgStr[2],Mdata);
	 if (AdrNum==1)
	  BEGIN
	   WAsmCode[0]|=(AdrMode & 7) << 9;
	   CodeLen=4;
	  END
	END
      END
     return;
    END

   if (Memo("MOVEC")) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
      BEGIN
       if (DecodeCtrlReg(ArgStr[1],WAsmCode+1)) 
	BEGIN
	 DecodeAdr(ArgStr[2],Mdata+Madr);
	 if (AdrNum!=0)
	  BEGIN
	   CodeLen=4; WAsmCode[0]=0x4e7a;
	   WAsmCode[1]|=AdrMode << 12; CheckSup();
	  END
	END
       else if (DecodeCtrlReg(ArgStr[2],WAsmCode+1)) 
	BEGIN
	 DecodeAdr(ArgStr[1],Mdata+Madr);
	 if (AdrNum!=0) 
	  BEGIN
	   CodeLen=4; WAsmCode[0]=0x4e7b;
	   WAsmCode[1]|=AdrMode << 12; CheckSup();
	  END
	END
       else WrError(1440);
      END
     return;
    END

   if (Memo("MOVES")) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (OpSize>2) WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if ((AdrNum==1) OR (AdrNum==2)) 
	BEGIN
	 WAsmCode[1]=0x800 | (AdrMode << 12);
	 DecodeAdr(ArgStr[2],Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	 if (AdrNum!=0)
	  BEGIN
	   WAsmCode[0]=0xe00 | AdrMode | (OpSize << 6); CodeLen=4+AdrCnt;
	   CopyAdrVals(WAsmCode+2); CheckSup();
	   CheckCPU(CPU68010);
	  END
	END
       else if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=0xe00 | AdrMode | (OpSize << 6);
	 CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
	 DecodeAdr(ArgStr[2],Mdata+Madr);
	 if (AdrNum!=0)
	  BEGIN
	   WAsmCode[1]=AdrMode << 12;
	   CheckSup();
	   CheckCPU(CPU68010);
	  END
	 else CodeLen=0;
	END
      END
     return;
    END

   if (Memo("CALLM")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       OpSize=0;
       DecodeAdr(ArgStr[1],Mimm);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[1]=AdrVals[0]; RelPos=4;
	 DecodeAdr(ArgStr[2],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
	 if (AdrNum!=0)
	  BEGIN
	   WAsmCode[0]=0x06c0+AdrMode;
	   CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
	   CheckCPU(CPU68020); Check020();
	  END
	END
      END
     return;
    END

   if (Memo("CAS")) 
    BEGIN
     if (OpSize>2) WrError(1130);
     else if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[1]=AdrMode;
	 DecodeAdr(ArgStr[2],Mdata);
	 if (AdrNum!=0)
	  BEGIN
	   RelPos=4;
	   WAsmCode[1]+=(((Word)AdrMode) << 6);
	   DecodeAdr(ArgStr[3],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	   if (AdrNum!=0)
	    BEGIN
	     WAsmCode[0]=0x08c0+AdrMode+(((Word)OpSize+1) << 9);
	     CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
	     CheckCPU(CPU68020);
	    END
	  END
	END
      END
     return;
    END

   if (Memo("CAS2")) 
    BEGIN
     if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
     else if (ArgCnt!=3) WrError(1110);
     else if (NOT CodeRegPair(ArgStr[1],WAsmCode+1,WAsmCode+2)) WrError(1760);
     else if (NOT CodeRegPair(ArgStr[2],&w1,&w2)) WrError(1760);
     else
      BEGIN
       WAsmCode[1]+=(w1 << 6);
       WAsmCode[2]+=(w2 << 6);
       if (NOT CodeIndRegPair(ArgStr[3],&w1,&w2)) WrError(1760);
       else
	BEGIN
	 WAsmCode[1]+=(w1 << 12);
	 WAsmCode[2]+=(w2 << 12);
	 WAsmCode[0]=0x0cfc+(((Word)OpSize-1) << 9);
	 CodeLen=6;
	 CheckCPU(CPU68020);
	END
      END
     return;
    END

   if ((Memo("CHK2")) OR (Memo("CMP2")))
    BEGIN
     if (OpSize>2) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Mdata+Madr);
       if (AdrNum!=0)
	BEGIN
	 RelPos=4;
	 WAsmCode[1]=((Word)AdrMode) << 12;
	 DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
	 if (AdrNum!=0)
	  BEGIN
	   WAsmCode[0]=0x00c0+(((Word)OpSize) << 9)+AdrMode;
	   if (Memo("CHK2")) WAsmCode[1]|=0x0800;
	   CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
	   CheckCPU(CPU68332);
	  END
	END
      END
     return;
    END

   if (Memo("EXTB")) 
    BEGIN
     if ((OpSize!=2) AND (*AttrPart!='\0')) WrError(1130);
     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=0x49c0+AdrMode; CodeLen=2;
	 CheckCPU(CPU68332);
	END
      END
     return;
    END

   if ((Memo("PACK")) OR (Memo("UNPK"))) 
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Mpre);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=(Memo("PACK")?0x8140:0x8180) | (AdrMode & 7);
	 if (AdrNum==5) WAsmCode[0]+=8;
	 DecodeAdr(ArgStr[2],Masks[AdrNum]);
	 if (AdrNum!=0)
	  BEGIN
	   WAsmCode[0]|=((AdrMode & 7) << 9);
	   DecodeAdr(ArgStr[3],Mimm);
	   if (AdrNum!=0)
	    BEGIN
	     WAsmCode[1]=AdrVals[0]; CodeLen=4;
	     CheckCPU(CPU68020);
	    END
	  END
	END
      END
     return;
    END

   if (Memo("RTM")) 
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Madr);
       if (AdrNum!=0)
	BEGIN
	 WAsmCode[0]=0x06c0+AdrMode; CodeLen=2;
	 CheckCPU(CPU68020); Check020();
	END
      END
     return;
    END

   if ((Memo("TBLS")) OR (Memo("TBLSN")) OR (Memo("TBLU")) OR (Memo("TBLUN"))) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (OpSize>2) WrError(1130);
     else if (MomCPU<CPU68332) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Mdata);
       if (AdrNum!=0)
        BEGIN
         HVal=AdrMode;
         p=strchr(ArgStr[1],':');
         if (p==0)
          BEGIN
           DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mabs+Mpc+Mpcidx);
           if (AdrNum!=0) 
            BEGIN
             WAsmCode[0]=0xf800+AdrMode;
             WAsmCode[1]=0x0100+(OpSize << 6)+(HVal << 12);
             if (OpPart[3]=='S') WAsmCode[1]+=0x0800;
             if (OpPart[strlen(OpPart)-1]=='N') WAsmCode[1]+=0x0400;
             memcpy(WAsmCode+2,AdrVals,AdrCnt);
             CodeLen=4+AdrCnt; Check32();
            END
          END
         else
          BEGIN
           strcpy(ArgStr[3],p+1); *p='\0';
           DecodeAdr(ArgStr[1],Mdata);
           if (AdrNum!=0)
            BEGIN
             w2=AdrMode;
             DecodeAdr(ArgStr[3],Mdata);
             if (AdrNum!=0)
              BEGIN
               WAsmCode[0]=0xf800+w2;
               WAsmCode[1]=0x0000+(OpSize << 6)+(HVal << 12)+AdrMode;
               if (OpPart[3]=='S') WAsmCode[1]+=0x0800;
               if (OpPart[strlen(OpPart)-1]=='N') WAsmCode[1]+=0x0400;
               CodeLen=4; Check32();
              END
            END
          END
        END
      END
     return;
    END

   /* bedingte Befehle */

   if ((strlen(OpPart)<=3) AND (*OpPart=='B')) 
    BEGIN
     /* .W, .S, .L, .X erlaubt */

     if ((OpSize!=1) AND (OpSize!=2) AND (OpSize!=4) AND (OpSize!=6)) 
      WrError(1130);

     /* nur ein Operand erlaubt */

     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN

       /* Bedingung finden, evtl. meckern */

       for (i=0; i<CondCnt; i++)
        if (strcmp(OpPart+1,CondNams[i])==0) break;
       if (i==CondCnt) WrError(1360);
       else
	BEGIN

	 /* Zieladresse ermitteln, zum Programmzaehler relativieren */

	 HVal=EvalIntExpression(ArgStr[1],Int32,&ValOK);
	 HVal=HVal-(EProgCounter()+2);

	 /* Bei Automatik Groesse festlegen */

	 if (OpSize==1) 
	  BEGIN
	   if (IsDisp8(HVal)) OpSize=4;
	   else if (IsDisp16(HVal)) OpSize=2;
	   else OpSize=6;
	  END

	 if (ValOK)
	  BEGIN

	   /* 16 Bit ? */

	   if (OpSize==2)
	    BEGIN

	     /* zu weit ? */

	     HVal16=HVal;
             if ((NOT IsDisp16(HVal)) AND (NOT SymbolQuestionable)) WrError(1370);
	     else
	      BEGIN

	       /* Code erzeugen */

	       CodeLen=4; WAsmCode[0]=0x6000 | (CondVals[i] << 8);
	       WAsmCode[1]=HVal16;
	      END
	    END

	   /* 8 Bit ? */

	   else if (OpSize==4)
	    BEGIN

	     /* zu weit ? */

	     HVal8=HVal;
             if ((NOT IsDisp8(HVal)) AND (NOT SymbolQuestionable)) WrError(1370);

	     /* Code erzeugen */
	     else
	      BEGIN
	       CodeLen=2;
	       if (HVal8!=0)
		BEGIN
		 WAsmCode[0]=0x6000 | (CondVals[i] << 8) | ((Byte)HVal8);
		END
	       else
		BEGIN
		 WAsmCode[0]=NOPCode;
                 if ((NOT Repass) AND (*AttrPart!='\0')) WrError(60);
		END
	      END
	    END

	   /* 32 Bit ? */

	   else
	    BEGIN
	     CodeLen=6; WAsmCode[0]=0x60ff | (CondVals[i] << 8);
	     WAsmCode[1]=HVal >> 16; WAsmCode[2]=HVal & 0xffff;
	     CheckCPU(CPU68332);
	    END
	  END
	END
       return;
      END
    END

   if ((strlen(OpPart)<=3) AND (*OpPart=='S')) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize!=0)) WrError(1130);
     else if (ArgCnt!=1) WrError(1130);
     else
      BEGIN
       for (i=0; i<CondCnt-2; i++)
        if (strcmp(OpPart+1,CondNams[i])==0) break;
       if (i==CondCnt-2) WrError(1360);
       else
	BEGIN
         OpSize=0;
	 DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	 if (AdrNum!=0)
	  BEGIN
	   WAsmCode[0]=0x50c0 | (CondVals[i] << 8) | AdrMode;
	   CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
	  END
	END
      END
     return;
    END

   if ((strlen(OpPart)<=4) AND (strncmp(OpPart,"DB",2)==0)) 
    BEGIN
     if (OpSize!=1) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       for (i=0; i<CondCnt-1; i++)
        if (strcmp(OpPart+2,CondNams[i])==0) break;
       if (i==18) i=1;
       if (i==CondCnt-1) WrError(1360);
       else
	BEGIN
	 HVal=EvalIntExpression(ArgStr[2],Int32,&ValOK);
	 if (ValOK)
	  BEGIN
	   HVal-=(EProgCounter()+2); HVal16=HVal;
           if ((NOT IsDisp16(HVal)) AND (NOT SymbolQuestionable)) WrError(1370);
	   else
	    BEGIN
	     CodeLen=4; WAsmCode[0]=0x50c8 | (CondVals[i] << 8);
	     WAsmCode[1]=HVal16;
	     DecodeAdr(ArgStr[1],Mdata);
	     if (AdrNum==1) WAsmCode[0]|=AdrMode;
	     else CodeLen=0;
	    END
	  END
	END
       return;
      END
    END

   if ((strlen(OpPart)<=6) AND (strncmp(OpPart,"TRAP",4)==0)) 
    BEGIN
     if (*AttrPart=='\0') OpSize=0;
     i=(OpSize==0) ? 0 : 1;
     if (OpSize>2) WrError(1130);
     else if (ArgCnt!=i) WrError(1110);
     else
      BEGIN
       for (i=0; i<CondCnt-2; i++)
        if (strcmp(OpPart+4,CondNams[i])==0) break;
       if (i==18) WrError(1360);
       else
	BEGIN
	 WAsmCode[0]=0x50f8+(i << 8);
	 if (OpSize==0)
	  BEGIN
	   WAsmCode[0]+=4; CodeLen=2;
	  END
	 else
	  BEGIN
	   DecodeAdr(ArgStr[1],Mimm);
	   if (AdrNum!=0)
	    BEGIN
	     WAsmCode[0]+=OpSize+1;
	     CopyAdrVals(WAsmCode+1); CodeLen=2+AdrCnt;
	    END
	  END
	 CheckCPU(CPU68332);
	END
      END
     return;
    END

   /* unbekannter Befehl */

   WrXError(1200,OpPart);
END

	static void InitCode_68K(void)
BEGIN
   SaveInitProc();
   SetFlag(&PMMUAvail,PMMUAvailName,False);
   SetFlag(&FullPMMU,FullPMMUName,True);
END

	static Boolean ChkPC_68K(void)
BEGIN
#ifdef HAS64
   return ((ActPC==SegCode) AND (ProgCounter()<=0xffffffffll)); 
#else  
   return (ActPC==SegCode);
#endif
END

	static Boolean IsDef_68K(void)
BEGIN
   return False;
END

        static void SwitchFrom_68K(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_68K(void)
BEGIN
   TurnWords=True; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x01; NOPCode=0x4e71;
   DivideChars=","; HasAttrs=True; AttrChars=".";

   ValidSegs=(1<<SegCode);
   Grans[SegCode]=1; ListGrans[SegCode]=2; SegInits[SegCode]=0;

   MakeCode=MakeCode_68K; ChkPC=ChkPC_68K; IsDef=IsDef_68K;
   SwitchFrom=SwitchFrom_68K; InitFields();

   SetFlag(&FullPMMU,FullPMMUName,MomCPU<=CPU68020);
END

	void code68k_init(void)
BEGIN
   CPU68008=AddCPU("68008",SwitchTo_68K);
   CPU68000=AddCPU("68000",SwitchTo_68K);
   CPU68010=AddCPU("68010",SwitchTo_68K);
   CPU68012=AddCPU("68012",SwitchTo_68K);
   CPU68332=AddCPU("68332",SwitchTo_68K);
   CPU68340=AddCPU("68340",SwitchTo_68K);
   CPU68360=AddCPU("68360",SwitchTo_68K);
   CPU68020=AddCPU("68020",SwitchTo_68K);
   CPU68030=AddCPU("68030",SwitchTo_68K);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_68K;
END
