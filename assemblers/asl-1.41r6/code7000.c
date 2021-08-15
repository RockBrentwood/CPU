/* code7000.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator SH7x00                                                      */
/*                                                                           */
/* Historie: 25.12.1996 Grundsteinlegung                                     */
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


#define FixedOrderCount 10
#define OneRegOrderCount 22
#define TwoRegOrderCount 18
#define MulRegOrderCount 3
#define BWOrderCount 3
#define LogOrderCount 4


#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModIReg 1
#define MModIReg (1 << ModIReg)
#define ModPreDec 2
#define MModPreDec (1 << ModPreDec)
#define ModPostInc 3
#define MModPostInc (1 << ModPostInc)
#define ModIndReg 4
#define MModIndReg (1 << ModIndReg)
#define ModR0Base 5
#define MModR0Base (1 << ModR0Base)
#define ModGBRBase 6
#define MModGBRBase (1 << ModGBRBase)
#define ModGBRR0 7
#define MModGBRR0 (1 << ModGBRR0)
#define ModPCRel 8
#define MModPCRel (1 << ModPCRel)
#define ModImm 9
#define MModImm (1 << ModImm)


#define CompLiteralsName "COMPRESSEDLITERALS"

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          CPUVar MinCPU;
          Word Code;
         } FixedMinOrder;

   typedef struct _TLiteral
            {
             struct _TLiteral *Next;
             LongInt Value,FCount;
             Boolean Is32,IsForward;
             Integer PassNo;
             LongInt DefSection;
            } *PLiteral,TLiteral;

static ShortInt OpSize;     /* Groesse=8*(2^OpSize) */
static ShortInt AdrMode;    /* Ergebnisadressmodus */
static Word AdrPart;        /* Adressierungsmodusbits im Opcode */

static PLiteral FirstLiteral;
static LongInt ForwardCount;
static SimpProc SaveInitProc;

static CPUVar CPU7000,CPU7600;

static FixedOrder *FixedOrders;
static FixedMinOrder *OneRegOrders;
static FixedOrder *TwoRegOrders;
static FixedMinOrder *MulRegOrders;
static FixedOrder *BWOrders;
static char **LogOrders;

static Boolean CurrDelayed,PrevDelayed,CompLiterals;
static LongInt DelayedAdr;

/*-------------------------------------------------------------------------*/
/* dynamische Belegung/Freigabe Codetabellen */

	static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

	static void AddOneReg(char *NName, Word NCode, CPUVar NMin)
BEGIN
   if (InstrZ>=OneRegOrderCount) exit(255);
   OneRegOrders[InstrZ].Name=NName;
   OneRegOrders[InstrZ].Code=NCode;
   OneRegOrders[InstrZ++].MinCPU=NMin;
END

	static void AddTwoReg(char *NName, Word NCode)
BEGIN
   if (InstrZ>=TwoRegOrderCount) exit(255);
   TwoRegOrders[InstrZ].Name=NName;
   TwoRegOrders[InstrZ++].Code=NCode;
END

	static void AddMulReg(char *NName, Word NCode, CPUVar NMin)
BEGIN
   if (InstrZ>=MulRegOrderCount) exit(255);
   MulRegOrders[InstrZ].Name=NName;
   MulRegOrders[InstrZ].Code=NCode;
   MulRegOrders[InstrZ++].MinCPU=NMin;
END

	static void AddBW(char *NName, Word NCode)
BEGIN
   if (InstrZ>=BWOrderCount) exit(255);
   BWOrders[InstrZ].Name=NName;
   BWOrders[InstrZ++].Code=NCode;
END

	static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("CLRT"  ,0x0008); AddFixed("CLRMAC",0x0028);
   AddFixed("NOP"   ,0x0009); AddFixed("RTE"   ,0x002b);
   AddFixed("SETT"  ,0x0018); AddFixed("SLEEP" ,0x001b);
   AddFixed("RTS"   ,0x000b); AddFixed("DIV0U" ,0x0019);
   AddFixed("BRK"   ,0x0000); AddFixed("RTB"   ,0x0001);

   OneRegOrders=(FixedMinOrder *) malloc(sizeof(FixedMinOrder)*OneRegOrderCount); InstrZ=0;
   AddOneReg("MOVT"  ,0x0029,CPU7000); AddOneReg("CMP/PZ",0x4011,CPU7000);
   AddOneReg("CMP/PL",0x4015,CPU7000); AddOneReg("ROTL"  ,0x4004,CPU7000);
   AddOneReg("ROTR"  ,0x4005,CPU7000); AddOneReg("ROTCL" ,0x4024,CPU7000);
   AddOneReg("ROTCR" ,0x4025,CPU7000); AddOneReg("SHAL"  ,0x4020,CPU7000);
   AddOneReg("SHAR"  ,0x4021,CPU7000); AddOneReg("SHLL"  ,0x4000,CPU7000);
   AddOneReg("SHLR"  ,0x4001,CPU7000); AddOneReg("SHLL2" ,0x4008,CPU7000);
   AddOneReg("SHLR2" ,0x4009,CPU7000); AddOneReg("SHLL8" ,0x4018,CPU7000);
   AddOneReg("SHLR8" ,0x4019,CPU7000); AddOneReg("SHLL16",0x4028,CPU7000);
   AddOneReg("SHLR16",0x4029,CPU7000); AddOneReg("LDBR"  ,0x0021,CPU7000);
   AddOneReg("STBR"  ,0x0020,CPU7000); AddOneReg("DT"    ,0x4010,CPU7600);
   AddOneReg("BRAF"  ,0x0032,CPU7600); AddOneReg("BSRF"  ,0x0003,CPU7600);

   TwoRegOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*TwoRegOrderCount); InstrZ=0;
   AddTwoReg("XTRCT" ,0x200d); AddTwoReg("ADDC"  ,0x300e);
   AddTwoReg("ADDV"  ,0x300f); AddTwoReg("CMP/HS",0x3002);
   AddTwoReg("CMP/GE",0x3003); AddTwoReg("CMP/HI",0x3006);
   AddTwoReg("CMP/GT",0x3007); AddTwoReg("CMP/STR",0x200c);
   AddTwoReg("DIV1"  ,0x3004); AddTwoReg("DIV0S" ,0x2007);
   AddTwoReg("MULS"  ,0x200f); AddTwoReg("MULU"  ,0x200e);
   AddTwoReg("NEG"   ,0x600b); AddTwoReg("NEGC"  ,0x600a);
   AddTwoReg("SUB"   ,0x3008); AddTwoReg("SUBC"  ,0x300a);
   AddTwoReg("SUBV"  ,0x300b); AddTwoReg("NOT"   ,0x6007);

   MulRegOrders=(FixedMinOrder *) malloc(sizeof(FixedMinOrder)*MulRegOrderCount); InstrZ=0;
   AddMulReg("MUL"   ,0x0007,CPU7600);
   AddMulReg("DMULU" ,0x3005,CPU7600);
   AddMulReg("DMULS" ,0x300d,CPU7600);

   BWOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*BWOrderCount); InstrZ=0;
   AddBW("SWAP",0x6008); AddBW("EXTS",0x600e); AddBW("EXTU",0x600c);

   LogOrders=(char **) malloc(sizeof(char *)*LogOrderCount); InstrZ=0;
   LogOrders[InstrZ++]="TST"; LogOrders[InstrZ++]="AND";
   LogOrders[InstrZ++]="XOR"; LogOrders[InstrZ++]="OR" ;
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(OneRegOrders);
   free(TwoRegOrders);
   free(MulRegOrders);
   free(BWOrders);
   free(LogOrders);
END

/*-------------------------------------------------------------------------*/
/* die PC-relative Adresse: direkt nach verzoegerten Spruengen = Sprungziel+2 */

	static LongInt PCRelAdr(void)
BEGIN
   if (PrevDelayed) return DelayedAdr+2;
   else return EProgCounter()+4;
END

	static void ChkDelayed(void)
BEGIN
   if (PrevDelayed) WrError(200);
END

/*-------------------------------------------------------------------------*/
/* Adressparsing */

	static char *LiteralName(PLiteral Lit)
BEGIN
   String Tmp;
   static String Result;

   if (Lit->IsForward) sprintf(Tmp,"F_%s",HexString(Lit->FCount,8));
   else if (Lit->Is32) sprintf(Tmp,"L_%s",HexString(Lit->Value,8));
   else sprintf(Tmp,"W_%s",HexString(Lit->Value,4));
   sprintf(Result,"LITERAL_%s_%s",Tmp,HexString(Lit->PassNo,0));
   return Result;
END
/*
	static void PrintLiterals(void)
BEGIN
   PLiteral Lauf;

   WrLstLine("LiteralList");
   Lauf=FirstLiteral;
   while (Lauf!=Nil)
    BEGIN
     WrLstLine(LiteralName(Lauf)); Lauf=Lauf->Next;
    END
END
*/
	static void SetOpSize(ShortInt Size)
BEGIN
   if (OpSize==-1) OpSize=Size;
   else if (Size!=OpSize)
    BEGIN
     WrError(1131); AdrMode=ModNone;
    END
END

	static Boolean DecodeReg(char *Asc, Byte *Erg)
BEGIN
   Boolean Err;

   if (strcasecmp(Asc,"SP")==0)
    BEGIN
     *Erg=15; return True;
    END
   else if ((strlen(Asc)<2) OR (strlen(Asc)>3) OR (toupper(*Asc)!='R')) return False;
   else
    BEGIN
     *Erg=ConstLongInt(Asc+1,&Err);
     return (Err AND (*Erg<=15));
    END
END

	static void ChkAdr(Word Mask)
BEGIN
   if ((AdrMode!=ModNone) AND ((Mask & (1 << AdrMode))==0))
    BEGIN
     WrError(1350); AdrMode=ModNone;
    END
END

   	static LongInt ExtOp(LongInt Inp, Byte Src, Boolean Signed)
BEGIN
   switch (Src)
    BEGIN
     case 0: Inp&=0xff; break;
     case 1: Inp&=0xffff; break;
    END
   if (Signed)
    BEGIN
     if (Src<1)
      if ((Inp & 0x80)==0x80) Inp+=0xff00;
     if (Src<2)
      if ((Inp & 0x8000)==0x8000) Inp+=0xffff0000;
    END
   return Inp;
END

	static LongInt OpMask(ShortInt OpSize)
BEGIN
   switch (OpSize)
    BEGIN
     case 0: return 0xff;
     case 1: return 0xffff;
     case 2: return 0xffffffff;
     default: return 0;
    END
END

	static void DecodeAdr(char *Asc, Word Mask, Boolean Signed)
BEGIN
#define RegNone (-1)
#define RegPC (-2)
#define RegGBR (-3)

   Byte p,HReg;
   char *pos;
   ShortInt BaseReg,IndReg,DOpSize;
   LongInt DispAcc;
   String AdrStr,LStr;
   Boolean OK,FirstFlag,NIs32,Critical,Found,LDef;
   PLiteral Lauf,Last;

   AdrMode=ModNone;

   if (DecodeReg(Asc,&HReg))
    BEGIN
     AdrPart=HReg; AdrMode=ModReg; ChkAdr(Mask); return;
    END

   if (*Asc=='@')
    BEGIN
     strcpy(Asc,Asc+1);
     if (IsIndirect(Asc))
      BEGIN
       strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
       BaseReg=RegNone; IndReg=RegNone;
       DispAcc=0; FirstFlag=False; OK=True;
       while ((*Asc!='\0') AND (OK))
	BEGIN
	 pos=QuotPos(Asc,',');
         if (pos==Nil)
          BEGIN
           strmaxcpy(AdrStr,Asc,255); *Asc='\0';
          END
         else
          BEGIN
           *pos='\0'; strmaxcpy(AdrStr,Asc,255); strcpy(Asc,pos+1);
          END
	 if (strcasecmp(AdrStr,"PC")==0)
	  if (BaseReg==RegNone) BaseReg=RegPC;
	  else
	   BEGIN
	    WrError(1350); OK=False;
	   END
	 else if (strcasecmp(AdrStr,"GBR")==0)
	  if (BaseReg==RegNone) BaseReg=RegGBR;
	  else
	   BEGIN
	    WrError(1350); OK=False;
	   END
	 else if (DecodeReg(AdrStr,&HReg))
	  if (IndReg==RegNone) IndReg=HReg;
	  else if ((BaseReg==RegNone) AND (HReg==0)) BaseReg=0;
	  else if ((IndReg==0) AND (BaseReg==RegNone))
	   BEGIN
	    BaseReg=0; IndReg=HReg;
	   END
	  else
	   BEGIN
	    WrError(1350); OK=False;
	   END
	 else
	  BEGIN
	   FirstPassUnknown=False;
	   DispAcc+=EvalIntExpression(AdrStr,Int32,&OK);
	   if (FirstPassUnknown) FirstFlag=True;
	  END
	END
       if (FirstFlag) DispAcc=0;
       if ((OK) AND ((DispAcc & ((1 << OpSize)-1))!=0))
	BEGIN
	 WrError(1325); OK=False;
	END
       else if ((OK) AND (DispAcc<0))
	BEGIN
	 WrXError(1315,"Disp<0"); OK=False;
	END
       else DispAcc=DispAcc >> OpSize;
       if (OK)
	BEGIN
	 switch (BaseReg)
          BEGIN
	   case 0:
	    if ((IndReg<0) OR (DispAcc!=0)) WrError(1350);
	    else
	     BEGIN
	      AdrMode=ModR0Base; AdrPart=IndReg;
	     END
            break;
	   case RegGBR:
	    if ((IndReg==0) AND (DispAcc==0)) AdrMode=ModGBRR0;
	    else if (IndReg!=RegNone) WrError(1350);
	    else if (DispAcc>255) WrError(1320);
	    else
	     BEGIN
	      AdrMode=ModGBRBase; AdrPart=DispAcc;
	     END
            break;
	   case RegNone:
	    if (IndReg==RegNone) WrError(1350);
	    else if (DispAcc>15) WrError(1320);
	    else
	     BEGIN
	      AdrMode=ModIndReg; AdrPart=(IndReg << 4)+DispAcc;
	     END
            break;
	   case RegPC:
	    if (IndReg!=RegNone) WrError(1350);
	    else if (DispAcc>255) WrError(1320);
	    else
	     BEGIN
	      AdrMode=ModPCRel; AdrPart=DispAcc;
	     END
	    break;
          END
	END
       ChkAdr(Mask); return;
      END
     else
      BEGIN
       if (DecodeReg(Asc,&HReg))
	BEGIN
	 AdrPart=HReg; AdrMode=ModIReg;
	END
       else if ((strlen(Asc)>1) AND (*Asc=='-') AND (DecodeReg(Asc+1,&HReg)))
	BEGIN
	 AdrPart=HReg; AdrMode=ModPreDec;
	END
       else if ((strlen(Asc)>1) AND (Asc[strlen(Asc)-1]=='+'))
        BEGIN
         strmaxcpy(AdrStr,Asc,255); AdrStr[strlen(AdrStr)-1]='\0';
         if (DecodeReg(AdrStr,&HReg))
	  BEGIN
	   AdrPart=HReg; AdrMode=ModPostInc;
	  END
         else WrError(1350);
        END
       else WrError(1350);
       ChkAdr(Mask); return;
      END
    END

   if (*Asc=='#')
    BEGIN
     FirstPassUnknown=False;
     switch (OpSize)
      BEGIN
       case 0: DispAcc=EvalIntExpression(Asc+1,Int8,&OK); break;
       case 1: DispAcc=EvalIntExpression(Asc+1,Int16,&OK); break;
       case 2: DispAcc=EvalIntExpression(Asc+1,Int32,&OK); break;
       default: DispAcc=0; OK=True;
      END
     Critical=FirstPassUnknown OR UsesForwards;
     if (OK)
      BEGIN
       /* minimale Groesse optimieren */
       DOpSize=(OpSize==0) ? 0 : Ord(Critical);
       while (((ExtOp(DispAcc,DOpSize,Signed) ^ DispAcc) & OpMask(OpSize))!=0) DOpSize++;
       if (DOpSize==0)
        BEGIN
	 AdrPart=DispAcc & 0xff;
	 AdrMode=ModImm;
        END
       else if ((Mask & MModPCRel)!=0)
	BEGIN
	 /* Literalgroesse ermitteln */
         NIs32=(DOpSize==2);
	 if (NOT NIs32) DispAcc&=0xffff;
	 /* Literale sektionsspezifisch */
         strcpy(AdrStr,"[PARENT0]");
	 /* schon vorhanden ? */
	 Lauf=FirstLiteral; p=0; OK=False; Last=Nil; Found=False;
	 while ((Lauf!=Nil) AND (NOT Found))
	  BEGIN
	   Last=Lauf;
           if ((NOT Critical) AND (NOT Lauf->IsForward)
	   AND (Lauf->DefSection==MomSectionHandle))
            if (((Lauf->Is32==NIs32) AND (DispAcc==Lauf->Value))
            OR  ((Lauf->Is32) AND (NOT NIs32) AND (DispAcc==(Lauf->Value >> 16)))) Found=True;
            else if ((Lauf->Is32) AND (NOT NIs32) AND (DispAcc==(Lauf->Value & 0xffff)))
             BEGIN
              Found=True; p=2;
             END
	   if (NOT Found) Lauf=Lauf->Next;
	  END
	 /* nein - erzeugen */
	 if (NOT Found)
	  BEGIN
	   Lauf=(PLiteral) malloc(sizeof(TLiteral));
	   Lauf->Is32=NIs32; Lauf->Value=DispAcc;
           Lauf->IsForward=Critical;
           if (Critical) Lauf->FCount=ForwardCount++;
	   Lauf->Next=Nil; Lauf->PassNo=1; Lauf->DefSection=MomSectionHandle;
           do
            BEGIN
             sprintf(LStr,"%s%s",LiteralName(Lauf),AdrStr);
             LDef=IsSymbolDefined(LStr);
             if (LDef) Lauf->PassNo++;
            END
           while (LDef);
           if (Last==Nil) FirstLiteral=Lauf; else Last->Next=Lauf;
	  END
	 /* Distanz abfragen - im naechsten Pass... */
	 FirstPassUnknown=False;
         sprintf(LStr,"%s%s",LiteralName(Lauf),AdrStr);
         DispAcc=EvalIntExpression(LStr,Int32,&OK)+p;
	 if (OK)
	  BEGIN
	   if (FirstPassUnknown)
	    DispAcc=0;
           else if (NIs32)
	    DispAcc=(DispAcc-(PCRelAdr() & 0xfffffffc)) >> 2;
           else
	    DispAcc=(DispAcc-PCRelAdr()) >> 1;
	   if (DispAcc<0)
	    BEGIN
	     WrXError(1315,"Disp<0"); OK=False;
	    END
           else if ((DispAcc>255) AND (NOT SymbolQuestionable)) WrError(1330);
	   else
	    BEGIN
	     AdrMode=ModPCRel; AdrPart=DispAcc; OpSize=Ord(NIs32)+1;
	    END
	  END
	END
       else WrError(1350);
      END
     ChkAdr(Mask); return;
    END

   /* absolut ueber PC-relativ abwickeln */

   if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
   else
    BEGIN
     FirstPassUnknown=False;
     DispAcc=EvalIntExpression(Asc,Int32,&OK);
     if (FirstPassUnknown) DispAcc=0;
     else if (OpSize==2) DispAcc-=(PCRelAdr() & 0xfffffffc);
     else DispAcc-=PCRelAdr();
     if (DispAcc<0) WrXError(1315,"Disp<0");
     else if ((DispAcc & ((1 << OpSize)-1))!=0) WrError(1325);
     else
      BEGIN
       DispAcc=DispAcc >> OpSize;
       if (DispAcc>255) WrError(1320);
       else
        BEGIN
	 AdrMode=ModPCRel; AdrPart=DispAcc;
        END
      END
    END

   ChkAdr(Mask);
END

/*-------------------------------------------------------------------------*/

	static void LTORG_16(void)
BEGIN
   PLiteral Lauf;

   Lauf=FirstLiteral;
   while (Lauf!=Nil)
    BEGIN
     if ((NOT Lauf->Is32) AND (Lauf->DefSection==MomSectionHandle))
      BEGIN
       WAsmCode[CodeLen >> 1]=Lauf->Value;
       EnterIntSymbol(LiteralName(Lauf),EProgCounter()+CodeLen,SegCode,False);
       Lauf->PassNo=(-1);
       CodeLen+=2;
      END
     Lauf=Lauf->Next;
    END
END

	static void LTORG_32(void)
BEGIN
   PLiteral Lauf,EqLauf;

   Lauf=FirstLiteral;
   while (Lauf!=Nil)
    BEGIN
     if ((Lauf->Is32) AND (Lauf->DefSection==MomSectionHandle) AND (Lauf->PassNo>=0))
      BEGIN
       if (((EProgCounter()+CodeLen) & 2)!=0)
        BEGIN
         WAsmCode[CodeLen >> 1]=0; CodeLen+=2;
        END
       WAsmCode[CodeLen >> 1]=(Lauf->Value >> 16);
       WAsmCode[(CodeLen >> 1)+1]=(Lauf->Value & 0xffff);
       EnterIntSymbol(LiteralName(Lauf),EProgCounter()+CodeLen,SegCode,False);
       Lauf->PassNo=(-1);
       if (CompLiterals)
        BEGIN
         EqLauf=Lauf->Next;
         while (EqLauf!=Nil)
          BEGIN
           if ((EqLauf->Is32) AND (EqLauf->PassNo>=0) AND
               (EqLauf->DefSection==MomSectionHandle) AND
               (EqLauf->Value==Lauf->Value))
            BEGIN
             EnterIntSymbol(LiteralName(EqLauf),EProgCounter()+CodeLen,SegCode,False);
             EqLauf->PassNo=(-1);
            END
           EqLauf=EqLauf->Next;
          END
        END
       CodeLen+=4;
      END
     Lauf=Lauf->Next;
    END
END

	static Boolean DecodePseudo(void)
BEGIN
#define ONOFF7000Count 2
   static ONOFFRec ONOFF7000s[ONOFF7000Count]=
  	          {{"SUPMODE",      &SupAllowed,   SupAllowedName},
	           {"COMPLITERALS", &CompLiterals, CompLiteralsName}};
   PLiteral Lauf,Tmp,Last;
   if (CodeONOFF(ONOFF7000s,ONOFF7000Count)) return True;

   /* ab hier (und weiter in der Hauptroutine) stehen die Befehle,
      die Code erzeugen, deshalb wird der Merker fuer verzoegerte
      Spruenge hier weiter geschaltet. */

   PrevDelayed=CurrDelayed; CurrDelayed=False;

   if (Memo("LTORG"))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       if ((EProgCounter() & 3)==0)
	BEGIN
	 LTORG_32(); LTORG_16();
	END
       else
	BEGIN
	 LTORG_16(); LTORG_32();
	END
       Lauf=FirstLiteral; Last=Nil;
       while (Lauf!=Nil)
	BEGIN
	 if ((Lauf->DefSection==MomSectionHandle) AND (Lauf->PassNo<0))
	  BEGIN
	   Tmp=Lauf->Next;
	   if (Last==Nil) FirstLiteral=Tmp; else Last->Next=Tmp;
	   free(Lauf); Lauf=Tmp;
	  END
	 else
	  BEGIN
	   Last=Lauf; Lauf=Lauf->Next;
	  END
	END
      END
     return True;
    END

   return False;
END

	static void SetCode(Word Code)
BEGIN
   CodeLen=2; WAsmCode[0]=Code;
END

	static void MakeCode_7000(void)
BEGIN
   Integer z;
   LongInt AdrLong;
   Boolean OK;
   Byte HReg;

   CodeLen=0; DontPrint=False; OpSize=(-1);

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* Attribut verwursten */

   if (*AttrPart!='\0')
    BEGIN
     if (strlen(AttrPart)!=1)
      BEGIN
       WrError(1105); return;
      END
     switch (toupper(*AttrPart))
      BEGIN
       case 'B': SetOpSize(0); break;
       case 'W': SetOpSize(1); break;
       case 'L': SetOpSize(2); break;
       case 'Q': SetOpSize(3); break;
       case 'S': SetOpSize(4); break;
       case 'D': SetOpSize(5); break;
       case 'X': SetOpSize(6); break;
       case 'P': SetOpSize(7); break;
       default:
        WrError(1107); return;
      END
    END

   if (DecodeMoto16Pseudo(OpSize,True)) return;

   /* Anweisungen ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else SetCode(FixedOrders[z].Code);
      if ((NOT SupAllowed) AND ((Memo("RTB")) OR (Memo("BRK")))) WrError(50);
      return;
     END

   /* Datentransfer */

   if (Memo("MOV"))
    BEGIN
     if (OpSize==-1) SetOpSize(2);
     if (ArgCnt!=2) WrError(1110);
     else if (OpSize>2) WrError(1130);
     else if (DecodeReg(ArgStr[1],&HReg))
      BEGIN
       DecodeAdr(ArgStr[2],MModReg+MModIReg+MModPreDec+MModIndReg+MModR0Base+MModGBRBase,True);
       switch (AdrMode)
        BEGIN
         case ModReg:
	  if (OpSize!=2) WrError(1130);
	  else SetCode(0x6003+(HReg << 4)+(AdrPart << 8));
          break;
         case ModIReg:
	  SetCode(0x2000+(HReg << 4)+(AdrPart << 8)+OpSize);
          break;
         case ModPreDec:
	  SetCode(0x2004+(HReg << 4)+(AdrPart << 8)+OpSize);
          break;
         case ModIndReg:
	  if (OpSize==2)
	   SetCode(0x1000+(HReg << 4)+(AdrPart & 15)+((AdrPart & 0xf0) << 4));
	  else if (HReg!=0) WrError(1350);
	  else SetCode(0x8000+AdrPart+(((Word)OpSize) << 8));
          break;
         case ModR0Base:
	  SetCode(0x0004+(AdrPart << 8)+(HReg << 4)+OpSize);
          break;
         case ModGBRBase:
	  if (HReg!=0) WrError(1350);
	  else SetCode(0xc000+AdrPart+(((Word)OpSize) << 8));
          break;
        END
      END
     else if (DecodeReg(ArgStr[2],&HReg))
      BEGIN
       DecodeAdr(ArgStr[1],MModImm+MModPCRel+MModIReg+MModPostInc+MModIndReg+MModR0Base+MModGBRBase,True);
       switch (AdrMode)
        BEGIN
         case ModIReg:
	  SetCode(0x6000+(AdrPart << 4)+(((Word)HReg) << 8)+OpSize);
          break;
         case ModPostInc:
	  SetCode(0x6004+(AdrPart << 4)+(((Word)HReg) << 8)+OpSize);
          break;
         case ModIndReg:
	  if (OpSize==2)
	   SetCode(0x5000+(((Word)HReg) << 8)+AdrPart);
	  else if (HReg!=0) WrError(1350);
	  else SetCode(0x8400+AdrPart+(((Word)OpSize) << 8));
          break;
         case ModR0Base:
	  SetCode(0x000c+(AdrPart << 4)+(((Word)HReg) << 8)+OpSize);
          break;
         case ModGBRBase:
	  if (HReg!=0) WrError(1350);
	  else SetCode(0xc400+AdrPart+(((Word)OpSize) << 8));
          break;
         case ModPCRel:
	  if (OpSize==0) WrError(1350);
	  else SetCode(0x9000+(((Word)OpSize-1) << 14)+(((Word)HReg) << 8)+AdrPart);
          break;
         case ModImm:
	  SetCode(0xe000+(((Word)HReg) << 8)+AdrPart);
          break;
        END
      END
     else WrError(1350);
     return;
    END

   if (Memo("MOVA"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeReg(ArgStr[2],&HReg)) WrError(1350);
     else if (HReg!=0) WrError(1350);
     else
      BEGIN
       SetOpSize(2);
       DecodeAdr(ArgStr[1],MModPCRel,False);
       if (AdrMode!=ModNone)
        BEGIN
         CodeLen=2; WAsmCode[0]=0xc700+AdrPart;
        END
      END
     return;
    END

   if ((Memo("LDC")) OR (Memo("STC")))
    BEGIN
     if (OpSize==-1) SetOpSize(2);
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (Memo("LDC"))
	BEGIN
	 strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
	END
       if (strcasecmp(ArgStr[1],"SR")==0) HReg=0;
       else if (strcasecmp(ArgStr[1],"GBR")==0) HReg=1;
       else if (strcasecmp(ArgStr[1],"VBR")==0) HReg=2;
       else
	BEGIN
	 WrError(1440); HReg=0xff;
	END
       if (HReg<0xff)
	BEGIN
         DecodeAdr(ArgStr[2],MModReg+((Memo("LDC"))?MModPostInc:MModPreDec),False);
	 switch (AdrMode)
          BEGIN
	   case ModReg:
	    if (Memo("LDC")) SetCode(0x400e + (AdrPart << 8)+(HReg << 4)); /* ANSI :-0 */
	    else SetCode(0x0002+(AdrPart << 8)+(HReg << 4));
            break;
	   case ModPostInc:
	    SetCode(0x4007+(AdrPart << 8)+(HReg << 4));
            break;
	   case ModPreDec:
	    SetCode(0x4003+(AdrPart << 8)+(HReg << 4));
            break;
	  END
	END
      END
     return;
    END

   if ((Memo("LDS")) OR (Memo("STS")))
    BEGIN
     if (OpSize==-1) SetOpSize(2);
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (Memo("LDS"))
	BEGIN
	 strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
	END
       if (strcasecmp(ArgStr[1],"MACH")==0) HReg=0;
       else if (strcasecmp(ArgStr[1],"MACL")==0) HReg=1;
       else if (strcasecmp(ArgStr[1],"PR")==0) HReg=2;
       else
	BEGIN
	 WrError(1440); HReg=0xff;
	END
       if (HReg<0xff)
	BEGIN
         DecodeAdr(ArgStr[2],MModReg+((Memo("LDS"))?MModPostInc:MModPreDec),False);
	 switch (AdrMode)
          BEGIN
	   case ModReg:
	    if (Memo("LDS")) SetCode(0x400a+(AdrPart << 8)+(HReg << 4));
	    else SetCode(0x000a+(AdrPart << 8)+(HReg << 4));
            break;
	   case ModPostInc:
	    SetCode(0x4006+(AdrPart << 8)+(HReg << 4));
            break;
	   case ModPreDec:
	    SetCode(0x4002+(AdrPart << 8)+(HReg << 4));
            break;
	  END
	END
      END
     return;
    END

   /* nur ein Register als Argument */

   for (z=0; z<OneRegOrderCount; z++)
    if (Memo(OneRegOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else if (MomCPU<OneRegOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
	DecodeAdr(ArgStr[1],MModReg,False);
	if (AdrMode!=ModNone)
	 SetCode(OneRegOrders[z].Code+(AdrPart << 8));
	if ((NOT SupAllowed) AND ((Memo("STBR")) OR (Memo("LDBR")))) WrError(50);
        if (*OpPart=='B')
         BEGIN
          CurrDelayed=True; DelayedAdr=0x7fffffff;
          ChkDelayed();
         END
       END
      return;
     END

   if (Memo("TAS"))
    BEGIN
     if (OpSize==-1) SetOpSize(0);
     if (ArgCnt!=1) WrError(1110);
     else if (OpSize!=0) WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg,False);
       if (AdrMode!=ModNone) SetCode(0x401b+(AdrPart << 8));
      END
     return;
    END

   /* zwei Register */

   for (z=0; z<TwoRegOrderCount; z++)
    if (Memo(TwoRegOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg,False);
        if (AdrMode!=ModNone)
         BEGIN
          WAsmCode[0]=TwoRegOrders[z].Code+(AdrPart << 4);
          DecodeAdr(ArgStr[2],MModReg,False);
          if (AdrMode!=ModNone) SetCode(WAsmCode[0]+(((Word)AdrPart) << 8));
         END
       END
      return;
     END

   for (z=0; z<MulRegOrderCount; z++)
    if (Memo(MulRegOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (MomCPU<MulRegOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
        if (*AttrPart=='\0') OpSize=2;
        if (OpSize!=2) WrError(1130);
        else
         BEGIN
          DecodeAdr(ArgStr[1],MModReg,False);
          if (AdrMode!=ModNone)
           BEGIN
            WAsmCode[0]=MulRegOrders[z].Code+(AdrPart << 4);
            DecodeAdr(ArgStr[2],MModReg,False);
            if (AdrMode!=ModNone) SetCode(WAsmCode[0]+(((Word)AdrPart) << 8));
           END
         END
       END
      return;
     END

   for (z=0; z<BWOrderCount; z++)
    if (Memo(BWOrders[z].Name))
     BEGIN
      if (OpSize==-1) SetOpSize(1);
      if (ArgCnt!=2) WrError(1110);
      else if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg,False);
        if (AdrMode!=ModNone)
         BEGIN
          WAsmCode[0]=BWOrders[z].Code+OpSize+(AdrPart << 4);
          DecodeAdr(ArgStr[2],MModReg,False);
          if (AdrMode!=ModNone) SetCode(WAsmCode[0]+(((Word)AdrPart) << 8));
         END
       END
      return;
     END

   if (Memo("MAC"))
    BEGIN
     if (OpSize==-1) SetOpSize(1);
     if (ArgCnt!=2) WrError(1110);
     else if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
     else if ((OpSize==2) AND (MomCPU<CPU7600)) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModPostInc,False);
       if (AdrMode!=ModNone)
	BEGIN
	 WAsmCode[0]=0x000f+(AdrPart << 4)+(((Word)2-OpSize) << 14);
	 DecodeAdr(ArgStr[2],MModPostInc,False);
	 if (AdrMode!=ModNone) SetCode(WAsmCode[0]+(((Word)AdrPart) << 8));
	END
      END
     return;
    END

   if (Memo("ADD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       DecodeAdr(ArgStr[2],MModReg,False);
       if (AdrMode!=ModNone)
	BEGIN
         HReg=AdrPart; OpSize=2;
	 DecodeAdr(ArgStr[1],MModReg+MModImm,True);
	 switch (AdrMode)
          BEGIN
	   case ModReg:
	    SetCode(0x300c+(((Word)HReg) << 8)+(AdrPart << 4));
            break;
	   case ModImm:
	    SetCode(0x7000+AdrPart+(((Word)HReg) << 8));
            break;
	  END
	END
      END
     return;
    END

   if (Memo("CMP/EQ"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       DecodeAdr(ArgStr[2],MModReg,False);
       if (AdrMode!=ModNone)
	BEGIN
         HReg=AdrPart; OpSize=2; DecodeAdr(ArgStr[1],MModReg+MModImm,True);
	 switch (AdrMode)
          BEGIN
	   case ModReg:
	    SetCode(0x3000+(((Word)HReg) << 8)+(AdrPart << 4));
            break;
	   case ModImm:
	    if (HReg!=0) WrError(1350);
	    else SetCode(0x8800+AdrPart);
            break;
	  END
	END
      END
     return;
    END

   for (z=0; z<LogOrderCount; z++)
    if (Memo(LogOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
	DecodeAdr(ArgStr[2],MModReg+MModGBRR0,False);
	switch (AdrMode)
         BEGIN
	  case ModReg:
	   if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
	   else
	    BEGIN
             OpSize=2;
	     HReg=AdrPart; DecodeAdr(ArgStr[1],MModReg+MModImm,False);
	     switch (AdrMode)
              BEGIN
	       case ModReg:
	        SetCode(0x2008+z+(((Word)HReg) << 8)+(AdrPart << 4));
                break;
	       case ModImm:
	        if (HReg!=0) WrError(1350);
	        else SetCode(0xc800+(z << 8)+AdrPart);
                break;
	      END
	    END
           break;
	  case ModGBRR0:
	   DecodeAdr(ArgStr[1],MModImm,False);
	   if (AdrMode!=ModNone)
	    SetCode(0xcc00+(z << 8)+AdrPart);
	   break;
	 END
       END
      return;
     END

   /* Miszellaneen.. */

   if (Memo("TRAPA"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       OpSize=0;
       DecodeAdr(ArgStr[1],MModImm,False);
       if (AdrMode==ModImm) SetCode(0xc300+AdrPart);
       ChkDelayed();
      END
     return;
    END

   /* Spruenge */

   if ((Memo("BF")) OR (Memo("BT"))
   OR  (Memo("BF/S")) OR (Memo("BT/S")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1110);
     else if ((strlen(OpPart)==4) AND (MomCPU<CPU7600)) WrError(1500);
     else
      BEGIN
       DelayedAdr=EvalIntExpression(ArgStr[1],Int32,&OK);
       AdrLong=DelayedAdr-(EProgCounter()+4);
       if (OK)
	if (Odd(AdrLong)) WrError(1375);
        else if (((AdrLong<-256) OR (AdrLong>254)) AND (NOT SymbolQuestionable)) WrError(1370);
	else
	 BEGIN
	  WAsmCode[0]=0x8900+((AdrLong >> 1) & 0xff);
	  if (OpPart[1]=='F') WAsmCode[0]+=0x200;
          if (strlen(OpPart)==4)
	   BEGIN
	    WAsmCode[0]+=0x400; CurrDelayed=True;
           END
	  CodeLen=2;
          ChkDelayed();
	 END
      END
     return;
    END

   if ((Memo("BRA")) OR (Memo("BSR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1110);
     else
      BEGIN
       DelayedAdr=EvalIntExpression(ArgStr[1],Int32,&OK);
       AdrLong=DelayedAdr-(EProgCounter()+4);
       if (OK)
	if (Odd(AdrLong)) WrError(1375);
        else if (((AdrLong<-4096) OR (AdrLong>4094)) AND (NOT SymbolQuestionable)) WrError(1370);
	else
	 BEGIN
	  WAsmCode[0]=0xa000+((AdrLong >> 1) & 0xfff);
	  if (Memo("BSR")) WAsmCode[0]+=0x1000;
	  CodeLen=2;
          CurrDelayed=True; ChkDelayed();
	 END
      END
     return;
    END

   if ((Memo("JSR")) OR (Memo("JMP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg,False);
       if (AdrMode!=ModNone)
        BEGIN
         SetCode(0x400b+(AdrPart << 8)+(Ord(Memo("JMP")) << 5));
         CurrDelayed=True; DelayedAdr=0x7fffffff;
         ChkDelayed();
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

	static void InitCode_7000(void)
BEGIN
   SaveInitProc();
   FirstLiteral=Nil; ForwardCount=0;
END

	static Boolean ChkPC_7000(void)
BEGIN
#ifdef HAS64
   return ((ActPC==SegCode) AND (ProgCounter()<=0xffffffffll));
#else
   return (ActPC==SegCode);
#endif
END

	static Boolean IsDef_7000(void)
BEGIN
   return False;
END

	static void SwitchFrom_7000(void)
BEGIN
   DeinitFields();
   if (FirstLiteral!=Nil) WrError(1495);
END

	static void SwitchTo_7000(void)
BEGIN
   TurnWords=True; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x6c; NOPCode=0x0009;
   DivideChars=","; HasAttrs=True; AttrChars=".";

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=2; SegInits[SegCode]=0;

   MakeCode=MakeCode_7000; ChkPC=ChkPC_7000; IsDef=IsDef_7000;
   SwitchFrom=SwitchFrom_7000;

   CurrDelayed=False; PrevDelayed=False;

   InitFields();
END

	void code7000_init(void)
BEGIN
   CPU7000=AddCPU("SH7000",SwitchTo_7000);
   CPU7600=AddCPU("SH7600",SwitchTo_7000);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_7000;
   FirstLiteral=Nil;
END
