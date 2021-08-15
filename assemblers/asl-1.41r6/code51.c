/* code51.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator fuer MCS-51/252 Prozessoren                                 */
/*                                                                           */
/* Historie:  5. 6.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

/*-------------------------------------------------------------------------*/

typedef struct
         { 
          char *Name;
          CPUVar MinCPU;
          Word Code;
         } FixedOrder;

#define ModNone (-1)
#define ModReg 1
#define MModReg (1<<ModReg)
#define ModIReg8 2
#define MModIReg8 (1<<ModIReg8)
#define ModIReg 3
#define MModIReg (1<<ModIReg)
#define ModInd 5
#define MModInd (1<<ModInd)
#define ModImm 7
#define MModImm (1<<ModImm)
#define ModImmEx 8
#define MModImmEx (1<<ModImmEx)
#define ModDir8 9
#define MModDir8 (1<<ModDir8)
#define ModDir16 10
#define MModDir16 (1<<ModDir16)
#define ModAcc 11
#define MModAcc (1<<ModAcc)
#define ModBit51 12
#define MModBit51 (1<<ModBit51)
#define ModBit251 13
#define MModBit251 (1<<ModBit251)

#define MMod51 (MModReg+MModIReg8+MModImm+MModAcc+MModDir8)
#define MMod251 (MModIReg+MModInd+MModImmEx+MModDir16)

#define AccOrderCnt 6
#define FixedOrderCnt 5
#define CondOrderCnt 13
#define BCondOrderCnt 3

#define AccReg 11


static FixedOrder *FixedOrders;
static FixedOrder *AccOrders;
static FixedOrder *CondOrders;
static FixedOrder *BCondOrders;

static Byte AdrVals[5];
static Byte AdrPart,AdrSize;
static ShortInt AdrMode,OpSize;
static Boolean MinOneIs0;

static Boolean SrcMode,BigEndian;

static SimpProc SaveInitProc;
static CPUVar CPU87C750,CPU8051,CPU8052,CPU80C320,
       CPU80501,CPU80502,CPU80504,CPU80515,CPU80517,
       CPU80251;

/*-------------------------------------------------------------------------*/

   	static void AddFixed(char *NName, Word NCode, CPUVar NCPU)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName; 
   FixedOrders[InstrZ].Code=NCode; 
   FixedOrders[InstrZ++].MinCPU=NCPU;
END

   	static void AddAcc(char *NName, Word NCode, CPUVar NCPU)
BEGIN
   if (InstrZ>=AccOrderCnt) exit(255);
   AccOrders[InstrZ].Name=NName; 
   AccOrders[InstrZ].Code=NCode; 
   AccOrders[InstrZ++].MinCPU=NCPU;
END

   	static void AddCond(char *NName, Word NCode, CPUVar NCPU)
BEGIN
   if (InstrZ>=CondOrderCnt) exit(255);
   CondOrders[InstrZ].Name=NName;
   CondOrders[InstrZ].Code=NCode;
   CondOrders[InstrZ++].MinCPU=NCPU;
END

   	static void AddBCond(char *NName, Word NCode, CPUVar NCPU)
BEGIN
   if (InstrZ>=BCondOrderCnt) exit(255);
   BCondOrders[InstrZ].Name=NName;
   BCondOrders[InstrZ].Code=NCode;
   BCondOrders[InstrZ++].MinCPU=NCPU;
END

	static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(FixedOrderCnt*sizeof(FixedOrder)); 
   InstrZ=0;
   AddFixed("NOP" ,0x0000,CPU87C750);
   AddFixed("RET" ,0x0022,CPU87C750);
   AddFixed("RETI",0x0032,CPU87C750);
   AddFixed("ERET",0x01aa,CPU80251);
   AddFixed("TRAP",0x01b9,CPU80251);

   AccOrders=(FixedOrder *) malloc(AccOrderCnt*sizeof(FixedOrder)); 
   InstrZ=0;
   AddAcc("DA"  ,0x00d4,CPU87C750);
   AddAcc("RL"  ,0x0023,CPU87C750);
   AddAcc("RLC" ,0x0033,CPU87C750);
   AddAcc("RR"  ,0x0003,CPU87C750);
   AddAcc("RRC" ,0x0013,CPU87C750);
   AddAcc("SWAP",0x00c4,CPU87C750);


   CondOrders=(FixedOrder *) malloc(CondOrderCnt*sizeof(FixedOrder));
   InstrZ=0;
   AddCond("JC"  ,0x0040,CPU87C750);
   AddCond("JE"  ,0x01a8,CPU80251);
   AddCond("JG"  ,0x0138,CPU80251);
   AddCond("JLE" ,0x0128,CPU80251);
   AddCond("JNC" ,0x0050,CPU87C750);
   AddCond("JNE" ,0x0178,CPU80251);
   AddCond("JNZ" ,0x0070,CPU87C750);
   AddCond("JSG" ,0x0118,CPU80251);
   AddCond("JSGE",0x0158,CPU80251);
   AddCond("JSL" ,0x0148,CPU80251);
   AddCond("JSLE",0x0108,CPU80251);
   AddCond("JZ"  ,0x0060,CPU87C750);
   AddCond("SJMP",0x0080,CPU87C750);

   BCondOrders=(FixedOrder *) malloc(BCondOrderCnt*sizeof(FixedOrder));
   InstrZ=0;
   AddBCond("JB" ,0x0020,CPU87C750);
   AddBCond("JBC",0x0010,CPU87C750);
   AddBCond("JNB",0x0030,CPU87C750);
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(AccOrders);
   free(CondOrders);
   free(BCondOrders);
END

/*-------------------------------------------------------------------------*/

   	static void SetOpSize(ShortInt NewSize)
BEGIN
   if (OpSize==-1) OpSize=NewSize;
   else if (OpSize!=NewSize)
    BEGIN
     WrError(1131); AdrMode=ModNone; AdrCnt=0;
    END
END

	static Boolean DecodeReg(char *Asc, Byte *Erg,Byte *Size)
BEGIN
   static Byte Masks[3]={0,1,3};

   char *Start;
   Integer alen=strlen(Asc);
   Boolean IO;

   if (strcasecmp(Asc,"DPX")==0)
    BEGIN
     *Erg=14; *Size=2; return True;
    END

   if (strcasecmp(Asc,"SPX")==0)
    BEGIN
     *Erg=15; *Size=2; return True;
    END

   if ((alen>=2) AND (toupper(*Asc)=='R'))
    BEGIN
     Start=Asc+1; *Size=0;
    END
   else if ((MomCPU>=CPU80251) AND (alen>=3) AND (toupper(*Asc)=='W') AND (toupper(Asc[1])=='R'))
    BEGIN
     Start=Asc+2; *Size=1;
    END
   else if ((MomCPU>=CPU80251) AND (alen>=3) AND (toupper(*Asc)=='D') AND (toupper(Asc[1])=='R'))
    BEGIN
     Start=Asc+2; *Size=2;
    END
   else return False;

   *Erg=ConstLongInt(Start,&IO);
   if (NOT IO) return False;
   else if (((*Erg) & Masks[*Size])!=0) return False;
   else
    BEGIN
     (*Erg) >>= (*Size);
     switch (*Size)
      BEGIN
       case 0: return (((*Erg)<8) OR ((MomCPU>=CPU80251) AND ((*Erg)<16)));
       case 1: return ((*Erg)<16);
       case 2: return (((*Erg)<8) OR ((*Erg)==14) OR ((*Erg)==15));
       default: return False;
      END
    END
END

	static void ChkMask(Word Mask, Word ExtMask)
BEGIN
   if ((AdrMode!=ModNone) AND ((Mask & (1 << AdrMode))==0))
    BEGIN
     if ((ExtMask & (1 << AdrMode))==0) WrError(1350); else WrError(1505);
     AdrCnt=0; AdrMode=ModNone;
    END
END

   	static void DecodeAdr(char *Asc_O, Word Mask)
BEGIN
   Boolean OK,FirstFlag;
   Byte HSize;
   Word H16;
   Integer SegType;
   char *PPos,*MPos,Save='\0';
   LongWord H32;
   String Asc,Part;
   Word ExtMask;

   strmaxcpy(Asc,Asc_O,255);

   AdrMode=ModNone; AdrCnt=0;

   ExtMask=MMod251 & Mask;
   if (MomCPU<CPU80251) Mask &= MMod51;

   if (*Asc=='\0') return;

   if (strcasecmp(Asc,"A")==0)
    BEGIN
     if ((Mask & MModAcc)==0)
      BEGIN
       AdrMode=ModReg; AdrPart=AccReg;
      END
     else AdrMode=ModAcc;
     SetOpSize(0);
     ChkMask(Mask,ExtMask); return;
    END

   if (*Asc=='#')
    BEGIN
     if ((OpSize==-1) AND (MinOneIs0)) OpSize=0;
     switch (OpSize)
      BEGIN
       case -1:
        WrError(1132);
        break;
       case 0:
        AdrVals[0]=EvalIntExpression(Asc+1,Int8,&OK);
        if (OK)
         BEGIN
          AdrMode=ModImm; AdrCnt=1;
         END
        break;
       case 1:
        H16=EvalIntExpression(Asc+1,Int16,&OK);
        if (OK)
         BEGIN
          AdrVals[0]=Hi(H16); AdrVals[1]=Lo(H16);
          AdrMode=ModImm; AdrCnt=2;
         END
        break;
       case 2:
        FirstPassUnknown=False;
        H32=EvalIntExpression(Asc+1,Int32,&OK);
        if (FirstPassUnknown) H32 &= 0xffff;
        if (OK)
         BEGIN
          AdrVals[1] = H32 & 0xff; 
          AdrVals[0] = (H32 >> 8) & 0xff;
          H32 >>= 16; 
          if (H32==0) AdrMode=ModImm;
          else if ((H32==1) OR (H32==0xffff)) AdrMode=ModImmEx;
          else WrError(1132);
          if (AdrMode!=ModNone) AdrCnt=2;
         END
        break;
      END
     ChkMask(Mask,ExtMask); return;
    END

   if (DecodeReg(Asc,&AdrPart,&HSize))
    BEGIN
     if ((MomCPU>=CPU80251) AND ((Mask & MModReg)==0))
      if ((HSize==0) AND (AdrPart==AccReg)) AdrMode=ModAcc;
      else AdrMode=ModReg;
     else AdrMode=ModReg;
     SetOpSize(HSize);
     ChkMask(Mask,ExtMask); return;
    END

   if (*Asc=='@')
    BEGIN
     PPos=strchr(Asc,'+'); MPos=strchr(Asc,'-');
     if ((MPos!=Nil) AND ((MPos<PPos) OR (PPos==Nil))) PPos=MPos;
     if (PPos!=Nil) 
      BEGIN
       Save=(*PPos); *PPos='\0';
      END
     if (DecodeReg(Asc+1,&AdrPart,&HSize))
      BEGIN
       if (PPos==Nil)
        BEGIN
         H32=0; OK=True;
        END
       else
        BEGIN
         *PPos=Save;
         H32=EvalIntExpression(PPos,SInt16,&OK);
        END
       if (OK)
        switch (HSize)
         BEGIN
          case 0:
           if ((AdrPart>1) OR (H32!=0)) WrError(1350);
           else AdrMode=ModIReg8;
           break;
          case 1:
           if (H32==0)
	    BEGIN
	     AdrMode=ModIReg; AdrSize=0;
            END
           else
            BEGIN
             AdrMode=ModInd; AdrSize=0;
             AdrVals[1] = H32 & 0xff; 
             AdrVals[0] = (H32 >> 8) & 0xff;
             AdrCnt=2;
            END
           break;
          case 2:
           if (H32==0)
	    BEGIN
	     AdrMode=ModIReg; AdrSize=2;
            END
           else
            BEGIN
             AdrMode=ModInd; AdrSize=2;
             AdrVals[1] = H32 & 0xff; 
             AdrVals[0] = (H32 >> 8) & 0xff;
             AdrCnt=2;
            END
           break;
         END
      END
     else WrError(1350);
     if (PPos!=Nil) *PPos=Save;
     ChkMask(Mask,ExtMask); return;
    END

   FirstFlag=False;
   SegType=(-1); PPos=QuotPos(Asc,':');
   if (PPos!=Nil)
    if (MomCPU<CPU80251)
     BEGIN
      WrError(1350); return;
     END
    else
     BEGIN
      SplitString(Asc,Part,Asc,PPos);
      if (strcasecmp(Part,"S")==0) SegType=(-2);
      else
       BEGIN
        FirstPassUnknown=False;
        SegType=EvalIntExpression(Asc,UInt8,&OK);
        if (NOT OK) return;
        if (FirstPassUnknown) FirstFlag=True;
       END
     END

   FirstPassUnknown=False;
   switch (SegType)
    BEGIN
     case -2:
      H32=EvalIntExpression(Asc,UInt9,&OK);
      ChkSpace(SegIO);
      if (FirstPassUnknown) H32=(H32 & 0xff) | 0x80;
      break;
     case -1:
      H32=EvalIntExpression(Asc,UInt24,&OK);
     default:
      H32=EvalIntExpression(Asc,UInt16,&OK);
    END
   if (FirstPassUnknown) FirstFlag=True;

   if ((SegType==-2) OR ((SegType==-1) AND ((TypeFlag & (1 << SegIO))!=0)))
    BEGIN
     if (ChkRange(H32,0x80,0xff))
      BEGIN
       AdrMode=ModDir8; AdrVals[0]=H32 & 0xff; AdrCnt=1;
      END
    END

   else
    BEGIN
     if (SegType>=0) H32 += ((LongWord)SegType) << 16;
     if (FirstFlag)
      if ((MomCPU<CPU80251) OR ((Mask & ModDir16)==0)) H32 &= 0xff;
      else H32 &= 0xffff;
     if (((H32<128) OR ((H32<256) AND (MomCPU<CPU80251))) AND ((Mask & MModDir8)!=0))
      BEGIN
       if (MomCPU<CPU80251) ChkSpace(SegData);
       AdrMode=ModDir8; AdrVals[0]=H32 &0xff; AdrCnt=1;
      END
     else if ((MomCPU<CPU80251) OR (H32>0xffff)) WrError(1925);
     else
      BEGIN
       AdrMode=ModDir16; AdrCnt=2;
       AdrVals[1] = H32 & 0xff; 
       AdrVals[0] = (H32 >> 8) & 0xff;
      END
    END

   ChkMask(Mask,ExtMask);
END

	static ShortInt DecodeBitAdr(char *Asc, LongInt *Erg, Boolean MayShorten)
BEGIN
   Boolean OK;
   char *PPos,Save;

   if (MomCPU<CPU80251)
    BEGIN
     *Erg=EvalIntExpression(Asc,UInt8,&OK);
     if (OK)
      BEGIN
       ChkSpace(SegBData);
       return ModBit51;
      END
     else return ModNone;
    END
   else
    BEGIN
     PPos=RQuotPos(Asc,'.');
     if (PPos==Nil)
      BEGIN
       FirstPassUnknown=False;
       *Erg=EvalIntExpression(Asc,Int32,&OK);
       if (FirstPassUnknown) (*Erg)&=0x070000ff;
#ifdef __STDC__
       if (((*Erg)&0xf8ffff00u)!=0)
#else
       if (((*Erg)&0xf8ffff00)!=0)
#endif
        BEGIN
	 WrError(1510); OK=False;
        END
      END
     else
      BEGIN
       Save=(*PPos); *PPos='\0'; DecodeAdr(Asc,MModDir8); *PPos=Save;
       if (AdrMode==ModNone) OK=False;
       else
        BEGIN
         *Erg=EvalIntExpression(PPos+1,UInt3,&OK)<<24;
         if (OK) (*Erg)+=AdrVals[0];
        END
      END
     if (NOT OK) return ModNone;
     else if ((MayShorten) AND (NOT SrcMode))
      if (((*Erg)&0x87)==0x80)
       BEGIN
        *Erg=((*Erg)&0xf8)+((*Erg)>>24); return ModBit51;
       END
      else if (((*Erg)&0xf0)==0x20)
       BEGIN
        *Erg=(((*Erg)&0x0f)<<3)+((*Erg)>>24); return ModBit51;
       END
      else return ModBit251;
     else return ModBit251;
    END
END

        static Boolean Chk504(LongInt Adr)
BEGIN
   return ((MomCPU==CPU80504) AND ((Adr & 0x7ff)==0x7fe));
END

/*-------------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
#define ONOFF51Count 2
  static ONOFFRec ONOFF51s[ONOFF51Count]=
             {{"SRCMODE",   &SrcMode,   SrcModeName},
  	      {"BIGENDIAN", &BigEndian, BigEndianName}};

   Integer z,DSeg;
   Boolean OK;
   String s;
   Word AdrByte;
   LongInt AdrLong;

   if ((Memo("SFR")) OR (Memo("SFRB")))
    BEGIN
     FirstPassUnknown=False;
     if (ArgCnt!=1) WrError(1110);
     else if ((Memo("SFRB")) AND (MomCPU>=CPU80251)) WrError(1500);
     else
      BEGIN
       if (MomCPU>=CPU80251) AdrByte=EvalIntExpression(ArgStr[1],UInt9,&OK);
       else AdrByte=EvalIntExpression(ArgStr[1],UInt8,&OK);
       if ((OK) AND (NOT FirstPassUnknown))
        BEGIN
         PushLocHandle(-1);
         DSeg=(MomCPU>=CPU80251)?(SegIO):(SegData);
	 EnterIntSymbol(LabPart,AdrByte,DSeg,False);
	 if (MakeUseList)
	  if (AddChunk(SegChunks+DSeg,AdrByte,1,False)) WrError(90);
	 if (Memo("SFRB"))
	  BEGIN
           if (AdrByte>0x7f)
            BEGIN
             if ((AdrByte & 7)!=0) WrError(220);
            END
           else
            BEGIN
             if ((AdrByte & 0xe0)!=0x20) WrError(220);
             AdrByte=(AdrByte-0x20) << 3;
            END
	   for (z=0; z<8; z++)
	    BEGIN
             sprintf(s,"%s.%c",LabPart,z+'0');
	     EnterIntSymbol(s,AdrByte+z,SegBData,False);
	    END
	   if (MakeUseList)
	    if (AddChunk(SegChunks+SegBData,AdrByte,8,False)) WrError(90);
           sprintf(ListLine,"=%sH-",HexString(AdrByte,2));
           strmaxcat(ListLine,HexString(AdrByte+7,2),255);
           strmaxcat(ListLine,"H",255);
          END
	 else sprintf(ListLine,"=%sH",HexString(AdrByte,2));
         PopLocHandle();
        END
      END
     return True;
    END

   if (Memo("BIT"))
    BEGIN
     if (MomCPU>=CPU80251)
      BEGIN
       if (ArgCnt!=1) WrError(1110);
       else if (DecodeBitAdr(ArgStr[1],&AdrLong,False)==ModBit251)
        BEGIN
         EnterIntSymbol(LabPart,AdrLong,SegNone,False);
         sprintf(ListLine,"=%sH.%s",HexString(AdrLong&0xff,2),HexString(AdrLong>>24,1));
        END
      END
     else CodeEquate(SegBData,0,0xff);
     return True;
    END

   if (Memo("PORT"))
    BEGIN
     if (MomCPU<CPU80251) WrError(1500);
     else CodeEquate(SegIO,0,0x1ff);
     return True;
    END

   if (CodeONOFF(ONOFF51s,ONOFF51Count))
    BEGIN
     if ((MomCPU<CPU80251) AND (SrcMode))
      BEGIN
       WrError(1500);
       SrcMode=False;
      END
     return True;
    END

   return False;
END

	static Boolean NeedsPrefix(Word Opcode)
BEGIN
   return (((Opcode&0x0f)>=6) AND ((SrcMode!=0)!=((Hi(Opcode)!=0)!=0)));
END

	static void PutCode(Word Opcode)
BEGIN
   if (((Opcode&0x0f)<6) OR ((SrcMode!=0)!=((Hi(Opcode)==0)!=0)))
    BEGIN
     BAsmCode[0]=Lo(Opcode); CodeLen=1;
    END
   else
    BEGIN
     BAsmCode[0]=0xa5; BAsmCode[1]=Lo(Opcode); CodeLen=2;
    END
END

        static void DecodeMOV(void)
BEGIN
   LongInt AdrLong;
   Byte HSize,HReg;
   Integer AdrInt;

   if (ArgCnt!=2) WrError(1110);
   else if ((strcasecmp(ArgStr[1],"C")==0) OR (strcasecmp(ArgStr[1],"CY")==0))
    BEGIN
     switch (DecodeBitAdr(ArgStr[2],&AdrLong,True))
      BEGIN
       case ModBit51:
        PutCode(0xa2);
        BAsmCode[CodeLen] = AdrLong & 0xff;
        CodeLen++;
        break;
       case ModBit251:
        PutCode(0x1a9);
        BAsmCode[CodeLen  ] = 0xa0 +(AdrLong >> 24);
        BAsmCode[CodeLen+1] = AdrLong & 0xff;
        CodeLen+=2;
        break;
      END
    END
   else if ((strcasecmp(ArgStr[2],"C")==0) OR (strcasecmp(ArgStr[2],"CY")==0))
    BEGIN
     switch (DecodeBitAdr(ArgStr[1],&AdrLong,True))
      BEGIN
       case ModBit51:
        PutCode(0x92);
        BAsmCode[CodeLen] = AdrLong & 0xff;
        CodeLen++;
        break;
       case ModBit251:
        PutCode(0x1a9);
        BAsmCode[CodeLen  ] = 0x90 + (AdrLong >> 24);
        BAsmCode[CodeLen+1] = AdrLong & 0xff;
        CodeLen+=2;
        break;
      END
    END
   else if (strcasecmp(ArgStr[1],"DPTR")==0)
    BEGIN
     SetOpSize(1); DecodeAdr(ArgStr[2],MModImm);
     switch (AdrMode)
      BEGIN
       case ModImm:
        PutCode(0x90);
        memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
        CodeLen+=AdrCnt;
        break;
      END
    END
   else
    BEGIN
     DecodeAdr(ArgStr[1],MModAcc+MModReg+MModIReg8+MModIReg+MModInd+MModDir8+MModDir16);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModIReg+MModInd+MModDir8+MModDir16+MModImm);
        switch (AdrMode)
         BEGIN
          case ModReg:
           if ((AdrPart<8) AND (NOT SrcMode)) PutCode(0xe8+AdrPart);
           else if (MomCPU<CPU80251) WrError(1505);
           else
            BEGIN
             PutCode(0x17c);
             BAsmCode[CodeLen++] = (AccReg << 4) + AdrPart;
            END
           break;
          case ModIReg8:
           PutCode(0xe6+AdrPart);
           break;
          case ModIReg:
           PutCode(0x17e);
           BAsmCode[CodeLen++] = (AdrPart << 4) + 0x09 + AdrSize;
           BAsmCode[CodeLen++] = (AccReg << 4);
           break;
          case ModInd:
           PutCode(0x109+(AdrSize << 4));
           BAsmCode[CodeLen++] = (AccReg << 4) + AdrPart;
           memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
           CodeLen+=AdrCnt;
           break;
          case ModDir8:
           PutCode(0xe5);
           BAsmCode[CodeLen++] = AdrVals[0];
           break;
          case ModDir16:
           PutCode(0x17e);
           BAsmCode[CodeLen++] = (AccReg << 4) + 0x03;
           memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
           CodeLen+=AdrCnt;
           break;
          case ModImm:
           PutCode(0x74);
           BAsmCode[CodeLen++]=AdrVals[0];
           break;
         END
        break;
       case ModReg:
        HReg=AdrPart;
        DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModIReg+MModInd+MModDir8+MModDir16+MModImm+MModImmEx);
        switch (AdrMode)
         BEGIN
          case ModReg:
           if ((OpSize==0) AND (AdrPart==AccReg) AND (HReg<8)) PutCode(0xf8+HReg);
           else if ((OpSize==0) AND (HReg==AccReg) AND (AdrPart<8)) PutCode(0xe8+AdrPart);
           else if (MomCPU<CPU80251) WrError(1505);
           else             
            BEGIN
             PutCode(0x17c+OpSize); 
             if (OpSize==2) BAsmCode[CodeLen-1]++;
             BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
            END
           break;
          case ModIReg8:
           if ((OpSize!=0) OR (HReg!=AccReg)) WrError(1350);
           else PutCode(0xe6+AdrPart);
           break;
          case ModIReg:
           if (OpSize==0)
            BEGIN
             PutCode(0x17e);
             BAsmCode[CodeLen++] = (AdrPart << 4) + 0x09 + AdrSize;
             BAsmCode[CodeLen++] = HReg << 4;
            END
           else if (OpSize==1)
            BEGIN
             PutCode(0x10b);
             BAsmCode[CodeLen++] = (AdrPart << 4) + 0x08 + AdrSize;
             BAsmCode[CodeLen++] = HReg << 4;
            END
           else WrError(1350);
           break;
          case ModInd:
           if (OpSize==2) WrError(1350);
           else
            BEGIN
             PutCode(0x109+(AdrSize << 4)+(OpSize << 6));
             BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
             memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
             CodeLen+=AdrCnt;
            END
           break;
          case ModDir8:
           if ((OpSize==0) AND (HReg==AccReg))
            BEGIN
             PutCode(0xe5);
             BAsmCode[CodeLen++] = AdrVals[0];
            END
           else if ((OpSize==0) AND (HReg<8) AND (NOT SrcMode))
            BEGIN
             PutCode(0xa8+HReg);
             BAsmCode[CodeLen++] = AdrVals[0];
            END
           else if (MomCPU<CPU80251) WrError(1505);
           else
            BEGIN
             PutCode(0x17e);
             BAsmCode[CodeLen++] = 0x01 + (HReg << 4) + (OpSize << 2);
             if (OpSize==2) BAsmCode[CodeLen-1]+=4;
             BAsmCode[CodeLen++] = AdrVals[0];
            END
           break;
          case ModDir16: 
           PutCode(0x17e);
           BAsmCode[CodeLen++] = 0x03 + (HReg << 4) + (OpSize << 2);
           if (OpSize==2) BAsmCode[CodeLen-1]+=4;
           memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
           CodeLen+=AdrCnt;
           break;
          case ModImm:
           if ((OpSize==0) AND (HReg==AccReg))
            BEGIN
             PutCode(0x74);
             BAsmCode[CodeLen++] = AdrVals[0];
            END
           else if ((OpSize==0) AND (HReg<8) AND (NOT SrcMode))
            BEGIN
             PutCode(0x78+HReg);
             BAsmCode[CodeLen++] = AdrVals[0];
            END
           else if (MomCPU<CPU80251) WrError(1505);
           else
            BEGIN
             PutCode(0x17e);
             BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2);
             memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
             CodeLen+=AdrCnt;
            END
           break;
          case ModImmEx:
           PutCode(0x17e);
           BAsmCode[CodeLen++]=0x0c + (HReg << 4);
           memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
           CodeLen+=AdrCnt;
           break;
         END
        break;
       case ModIReg8:
        SetOpSize(0); HReg=AdrPart;
        DecodeAdr(ArgStr[2],MModAcc+MModDir8+MModImm);
        switch (AdrMode)
         BEGIN
          case ModAcc:
           PutCode(0xf6+HReg);
           break;
          case ModDir8:
           PutCode(0xa6+HReg);
           BAsmCode[CodeLen++]=AdrVals[0];
           break;
          case ModImm:
           PutCode(0x76+HReg);
           BAsmCode[CodeLen++]=AdrVals[0];
           break;
         END
        break;
       case ModIReg:
        HReg=AdrPart; HSize=AdrSize;
        DecodeAdr(ArgStr[2],MModReg);
        switch (AdrMode)
         BEGIN
          case ModReg:
           if (OpSize==0)
            BEGIN
             PutCode(0x17a);
             BAsmCode[CodeLen++] = (HReg << 4) + 0x09 + HSize;
             BAsmCode[CodeLen++] = AdrPart << 4;
            END
           else if (OpSize==1)
            BEGIN
             PutCode(0x11b);
             BAsmCode[CodeLen++] = (HReg << 4) + 0x08 + HSize;
             BAsmCode[CodeLen++] = AdrPart << 4;
            END
           else WrError(1350);
         END
        break;
       case ModInd:
        HReg=AdrPart; HSize=AdrSize;
        AdrInt=(((Word)AdrVals[0])<<8)+AdrVals[1];
        DecodeAdr(ArgStr[2],MModReg);
        switch (AdrMode)
         BEGIN
          case ModReg:
           if (OpSize==2) WrError(1350);
           else
            BEGIN
             PutCode(0x119 + (HSize << 4) + (OpSize << 6));
             BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
             BAsmCode[CodeLen++] = Hi(AdrInt);
             BAsmCode[CodeLen++] = Lo(AdrInt);
            END
         END
        break;
       case ModDir8:
        MinOneIs0=True; HReg=AdrVals[0];
        DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModDir8+MModImm);
        switch (AdrMode)
         BEGIN
          case ModReg:
           if ((OpSize==0) AND (AdrPart==AccReg))
            BEGIN
             PutCode(0xf5);
             BAsmCode[CodeLen++] = HReg;
            END
           else if ((OpSize==0) AND (AdrPart<8) AND (NOT SrcMode))
            BEGIN
             PutCode(0x88+AdrPart);
             BAsmCode[CodeLen++] = HReg;
            END
           else if (MomCPU<CPU80251) WrError(1505);
           else
            BEGIN
             PutCode(0x17a);
             BAsmCode[CodeLen++] = 0x03 + (AdrPart << 4) + (OpSize << 1);
             if (OpSize==2) BAsmCode[CodeLen-1]+=6;
             BAsmCode[CodeLen++] = HReg;
            END
           break;
          case ModIReg8:
           PutCode(0x86+AdrPart);
           BAsmCode[CodeLen++] = HReg;
           break;
          case ModDir8:
           PutCode(0x85);
           BAsmCode[CodeLen++] = AdrVals[0];
           BAsmCode[CodeLen++] = HReg;
           break;
          case ModImm:
           PutCode(0x75);
           BAsmCode[CodeLen++] = HReg;
           BAsmCode[CodeLen++] = AdrVals[0];
           break;
         END
        break;
       case ModDir16:
        AdrInt=(((Word)AdrVals[0]) << 8) + AdrVals[1];
        DecodeAdr(ArgStr[2],MModReg);
        switch (AdrMode)
         BEGIN
          case ModReg:
           PutCode(0x17a);
           BAsmCode[CodeLen++]=0x03 + (AdrPart << 4) + (OpSize << 2);
           if (OpSize==2) BAsmCode[CodeLen-1]+=4;
           BAsmCode[CodeLen++] = Hi(AdrInt);
           BAsmCode[CodeLen++] = Lo(AdrInt);
           break;
         END
        break;
      END
    END
END

	static void DecodeLogic(void)
BEGIN
   Boolean InvFlag;
   Byte HReg;
   LongInt AdrLong;
   Integer z;

   if (ArgCnt!=2) WrError(1110);
   else if ((strcasecmp(ArgStr[1],"C")==0) OR (strcasecmp(ArgStr[1],"CY")==0))
    BEGIN
     if (Memo("XRL")) WrError(1350);
     else
      BEGIN
       HReg=Ord(Memo("ANL")) << 4;
       InvFlag=(*ArgStr[2]=='/');
       if (InvFlag) strcpy(ArgStr[2],ArgStr[2]+1);
       switch (DecodeBitAdr(ArgStr[2],&AdrLong,True))
        BEGIN
         case ModBit51:
          PutCode((InvFlag) ? 0xa0+HReg : 0x72+HReg);
          BAsmCode[CodeLen++] = AdrLong & 0xff;
          break;
         case ModBit251:
          PutCode(0x1a9);
          BAsmCode[CodeLen++] = ((InvFlag) ? 0xe0 : 0x70) + HReg + (AdrLong >>24);
          BAsmCode[CodeLen++] = AdrLong & 0xff;
          break;
        END
      END
    END
   else
    BEGIN
     if (Memo("ANL")) z=0x50;
     else if (Memo("ORL")) z=0x40;
     else z=0x60;
     DecodeAdr(ArgStr[1],MModAcc+MModReg+MModDir8);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModIReg+MModDir8+MModDir16+MModImm);
        switch (AdrMode)
         BEGIN
          case ModReg:
           if ((AdrPart<8) AND (NOT SrcMode)) PutCode(z+8+AdrPart);
           else
            BEGIN
             PutCode(z+0x10c);
             BAsmCode[CodeLen++] = AdrPart + (AccReg << 4);
            END
           break;
          case ModIReg8:
           PutCode(z+6+AdrPart);
           break;
          case ModIReg:
           PutCode(z+0x10e);
           BAsmCode[CodeLen++] = 0x09 + AdrSize + (AdrPart << 4);
           BAsmCode[CodeLen++] = AccReg << 4;
           break;
          case ModDir8:
           PutCode(z+0x05);
           BAsmCode[CodeLen++] = AdrVals[0];
           break;
          case ModDir16:
           PutCode(0x10e + z);
           BAsmCode[CodeLen++] = 0x03 + (AccReg << 4);
           memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
           CodeLen+=AdrCnt;
           break;
          case ModImm:
           PutCode(z+0x04);
           BAsmCode[CodeLen++] = AdrVals[0];
           break;
         END
        break;
       case ModReg:
        if (MomCPU<CPU80251) WrError(1350);
        else
         BEGIN
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModIReg+MModDir8+MModDir16+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg:
             if (OpSize==2) WrError(1350);
             else
              BEGIN
               PutCode(z+0x10c+OpSize);
               BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
              END
             break;
            case ModIReg8:
             if ((OpSize!=0) OR (HReg!=AccReg)) WrError(1350);
             else PutCode(z+0x06+AdrPart);
             break;
            case ModIReg:
             if (OpSize!=0) WrError(1350);
             else
              BEGIN
               PutCode(0x10e + z);
               BAsmCode[CodeLen++] = 0x09 + AdrSize + (AdrPart << 4);
               BAsmCode[CodeLen++] = HReg << 4;
              END
             break;
            case ModDir8:
             if ((OpSize==0) AND (HReg==AccReg))
              BEGIN
               PutCode(0x05+z);
               BAsmCode[CodeLen++] = AdrVals[0];
              END
             else if (OpSize==2) WrError(1350);
             else
              BEGIN
               PutCode(0x10e + z);
               BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2) + 1;
               BAsmCode[CodeLen++] = AdrVals[0];
              END
             break;
            case ModDir16:
             if (OpSize==2) WrError(1350);
             else
              BEGIN
               PutCode(0x10e + z);
               BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2) + 3;
               memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
               CodeLen+=AdrCnt;
              END
             break;
            case ModImm:
             if ((OpSize==0) AND (HReg==AccReg))
              BEGIN
               PutCode(0x04+z);
               BAsmCode[CodeLen++] = AdrVals[0];
              END
             else if (OpSize==2) WrError(1350);
             else
              BEGIN
               PutCode(0x10e + z);
               BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2);
               memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
               CodeLen+=AdrCnt;
              END
             break;
           END
         END
        break;
       case ModDir8:
        HReg=AdrVals[0]; SetOpSize(0);
        DecodeAdr(ArgStr[2],MModAcc+MModImm);
        switch (AdrMode)
         BEGIN
          case ModAcc:
           PutCode(z+0x02);
           BAsmCode[CodeLen++] = HReg;
           break;
          case ModImm:
           PutCode(z+0x03);
           BAsmCode[CodeLen++] = HReg;
           BAsmCode[CodeLen++] = AdrVals[0];
           break;
         END
        break;
      END
    END
END

	static void MakeCode_51(void)
BEGIN
   Boolean OK;
   LongInt AdrLong,BitLong;
   Integer z;
   FixedOrder *FixedZ;
   int CmpErg;
   LongInt Dist;
   Byte HReg;
   Boolean Questionable;

   CodeLen=0; DontPrint=False; OpSize=(-1); MinOneIs0=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(BigEndian)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (MomCPU<FixedOrders[z].MinCPU) WrError(1500);
      else PutCode(FixedOrders[z].Code);
      return;
     END

   /* Datentransfer */

   if (Memo("MOV"))
    BEGIN
     DecodeMOV();
     return;
    END

   if (Memo("MOVC"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc);
       switch (AdrMode)
        BEGIN
         case ModAcc:
          if (strcasecmp(ArgStr[2],"@A+DPTR")==0) PutCode(0x93);
          else if (strcasecmp(ArgStr[2],"@A+PC")==0) PutCode(0x83);
          else WrError(1350);
          break;
        END
      END
     return;
    END

   if (Memo("MOVH"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80251) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       switch (AdrMode)
        BEGIN
         case ModReg:
          if (OpSize!=2) WrError(1350);
          else
           BEGIN
            HReg=AdrPart; OpSize--;
            DecodeAdr(ArgStr[2],MModImm);
            switch (AdrMode)
             BEGIN
              case ModImm:
               PutCode(0x17a);
               BAsmCode[CodeLen++] = 0x0c + (HReg << 4);
               memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
               CodeLen+=AdrCnt;
               break;
             END
           END
          break;
        END
      END
     return;
    END

   if ((Memo("MOVS")) OR (Memo("MOVZ")))
    BEGIN
     z=Ord(Memo("MOVS")) << 4;
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80251) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       switch (AdrMode)
        BEGIN
         case ModReg:
          if (OpSize!=1) WrError(1350);
          else
           BEGIN
            HReg=AdrPart; OpSize--;
            DecodeAdr(ArgStr[2],MModReg);
            switch (AdrMode)
             BEGIN
              case ModReg:
               PutCode(0x10a+z);
               BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
               break;
             END
           END
          break;
        END
      END
     return;
    END

   if (Memo("MOVX"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       z=0;
       if ((strcasecmp(ArgStr[2],"A")==0) OR ((MomCPU>=CPU80251) AND (strcasecmp(ArgStr[2],"R11")==0)))
        BEGIN
         z=0x10; strcpy(ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"A",255);
        END
       if ((strcasecmp(ArgStr[1],"A")!=0) AND ((MomCPU<CPU80251) OR (strcasecmp(ArgStr[2],"R11")==0))) WrError(1350);
       else if (strcasecmp(ArgStr[2],"@DPTR")==0) PutCode(0xe0+z);
       else
        BEGIN
         DecodeAdr(ArgStr[2],MModIReg8);
         switch (AdrMode)
          BEGIN
           case ModIReg8:
	    PutCode(0xe2+AdrPart+z);
            break;
          END
        END
      END
     return;
    END

   if ((Memo("POP")) OR (Memo("PUSH")) OR (Memo("PUSHW")))
    BEGIN
     z=Ord(Memo("POP")) << 4;
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       if (*ArgStr[1]=='#')
        SetOpSize(Ord(Memo("PUSHW")));
       if (Memo("POP")) DecodeAdr(ArgStr[1],MModDir8+MModReg);
       else DecodeAdr(ArgStr[1],MModDir8+MModReg+MModImm);
       switch (AdrMode)
        BEGIN
         case ModDir8:
          PutCode(0xc0+z);
          BAsmCode[CodeLen++] = AdrVals[0];
          break;
         case ModReg:
          if (MomCPU<CPU80251) WrError(1505);
          else
           BEGIN
            PutCode(0x1ca+z);
            BAsmCode[CodeLen++] = 0x08 + (AdrPart << 4) + OpSize + (3 * Ord(OpSize == 2));
           END
          break;
         case ModImm:
          if (MomCPU<CPU80251) WrError(1505);
          else
           BEGIN
            PutCode(0x1ca);
            BAsmCode[CodeLen++] = 0x02 + (OpSize << 2);
            memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
            CodeLen+=AdrCnt;
           END
          break;
        END
      END
     return;
    END

   if (Memo("XCH"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModReg+MModIReg8+MModDir8);
       switch (AdrMode)
        BEGIN
         case ModAcc:
          DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModDir8);
          switch (AdrMode)
           BEGIN
            case ModReg:
             if (AdrPart>7) WrError(1350);
             else PutCode(0xc8+AdrPart);
             break;
            case ModIReg8:
	     PutCode(0xc6+AdrPart);
             break;
            case ModDir8:
             PutCode(0xc5);
             BAsmCode[CodeLen++] = AdrVals[0];
             break;
           END
          break;
         case ModReg:
          if ((OpSize!=0) OR (AdrPart>7)) WrError(1350);
          else
           BEGIN
            HReg=AdrPart;
            DecodeAdr(ArgStr[2],MModAcc);
            switch (AdrMode)
             BEGIN
              case ModAcc:
               PutCode(0xc8+HReg);
               break;
             END
           END
          break;
         case ModIReg8:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModAcc);
          switch (AdrMode)
           BEGIN
            case ModAcc:
             PutCode(0xc6+HReg);
             break;
           END
          break;
         case ModDir8:
          HReg=AdrVals[0];
          DecodeAdr(ArgStr[2],MModAcc);
          switch (AdrMode)
           BEGIN
            case ModAcc:
             PutCode(0xc5);
             BAsmCode[CodeLen++] = HReg;
             break;
           END
          break;
        END
      END
     return;
    END

   if (Memo("XCHD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModIReg8);
       switch (AdrMode)
        BEGIN
         case ModAcc:
          DecodeAdr(ArgStr[2],MModIReg8);
          switch (AdrMode)
           BEGIN
            case ModIReg8:
	     PutCode(0xd6+AdrPart);
             break;
           END
          break;
         case ModIReg8:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModAcc);
          switch (AdrMode)
           BEGIN
            case ModAcc:
             PutCode(0xd6+HReg);
             break;
           END
          break;
        END
      END
     return;
    END

   /* ein Operand */

   for (z=0; z<AccOrderCnt; z++)
    if (Memo(AccOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (MomCPU<AccOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModAcc);
        switch (AdrMode)
         BEGIN
          case ModAcc: 
           PutCode(AccOrders[z].Code);
           break;
         END
       END;
      return;
     END

   /* Spruenge */

   if ((Memo("ACALL")) OR (Memo("AJMP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[1],Int24,&OK);
       if (OK)
	if ((NOT SymbolQuestionable) AND (((EProgCounter()+2) >> 11)!=(AdrLong >> 11))) WrError(1910);
        else if (Chk504(EProgCounter())) WrError(1900);
	else
         BEGIN
          ChkSpace(SegCode);
          PutCode(0x01 + (Ord(Memo("ACALL")) << 4) + ((Hi(AdrLong) & 7) << 5));
	  BAsmCode[CodeLen++]=Lo(AdrLong);
         END
      END
     return;
    END

   if ((Memo("LCALL")) OR (Memo("LJMP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU8051) WrError(1500);
     else if (*ArgStr[1]=='@')
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg);
       switch (AdrMode)
        BEGIN
         case ModIReg:
          if (AdrSize!=0) WrError(1350);
          else
           BEGIN
            PutCode(0x189 + (Ord(Memo("LCALL")) << 4));
            BAsmCode[CodeLen++] = 0x04 + (AdrPart << 4);
           END
          break;
        END
      END
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[1],Int24,&OK);
       if (OK)
        if ((MomCPU>=CPU80251) AND (((EProgCounter()+3) >> 16)!=(AdrLong >> 16))) WrError(1910);
        else
	 BEGIN
	  ChkSpace(SegCode);
          PutCode(0x02 + (Ord(Memo("LCALL")) << 4));
	  BAsmCode[CodeLen++] = (AdrLong >> 8) & 0xff;
	  BAsmCode[CodeLen++] = AdrLong & 0xff;
         END
      END
     return;
    END

   if ((Memo("ECALL")) OR (Memo("EJMP")))
    BEGIN
     z=Ord(Memo("ECALL")); 
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU80251) WrError(1500);
     else if (*ArgStr[1]=='@')
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg);
       switch (AdrMode)
        BEGIN
         case ModIReg:
          if (AdrSize!=2) WrError(1350);
          else
           BEGIN
            PutCode(0x189 + (z << 4));
            BAsmCode[CodeLen++] = 0x08 + (AdrPart << 4);
           END
          break;
        END
      END
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK);
       if (OK)
	BEGIN
	 ChkSpace(SegCode);
         PutCode(0x18a + (z << 4));
	 BAsmCode[CodeLen++] = (AdrLong >> 16) & 0xff;
	 BAsmCode[CodeLen++] = (AdrLong >>  8) & 0xff;
	 BAsmCode[CodeLen++] =  AdrLong        & 0xff;
        END
      END
     return;
    END

   for (z=0,FixedZ=CondOrders; z<CondOrderCnt; z++,FixedZ++)
    if ((CmpErg=strcmp(OpPart,FixedZ->Name))==0)
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (MomCPU<FixedZ->MinCPU) WrError(1500);
      else
       BEGIN
        AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK);
        if (OK)
         BEGIN
          AdrLong-=EProgCounter()+2+Ord(NeedsPrefix(FixedZ->Code));
          if (((AdrLong<-128) OR (AdrLong>127)) AND (NOT SymbolQuestionable)) WrError(1370);
          else
           BEGIN
            ChkSpace(SegCode);
            PutCode(FixedZ->Code);
	    BAsmCode[CodeLen++] = AdrLong & 0xff;
           END
         END
       END
      return;
     END
    else if (CmpErg<0) break;

   if (Memo("JMP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"@A+DPTR")==0) PutCode(0x73);
     else if (*ArgStr[1]=='@')
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg);
       switch (AdrMode)
        BEGIN
         case ModIReg:
          PutCode(0x189);
          BAsmCode[CodeLen++] = 0x04 + (AdrSize << 1) + (AdrPart << 4);
          break;
        END
      END
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK);
       if (OK)
	BEGIN
	 Dist=AdrLong-(EProgCounter()+2);
	 if ((Dist<=127) AND (Dist>=-128))
	  BEGIN
           PutCode(0x80);
	   BAsmCode[CodeLen++] = Dist & 0xff;
	  END
	 else if ((NOT Chk504(EProgCounter())) AND ((AdrLong >> 11)==((EProgCounter()+2) >> 11)))
	  BEGIN
	   PutCode(0x01 + ((Hi(AdrLong) & 7) << 5));
	   BAsmCode[CodeLen++] = Lo(AdrLong);
	  END
         else if (MomCPU<CPU8051) WrError(1910);
	 else if (((EProgCounter()+3) >> 16)==(AdrLong >> 16))
	  BEGIN
	   PutCode(0x02);
	   BAsmCode[CodeLen++] = Hi(AdrLong); 
           BAsmCode[CodeLen++] = Lo(AdrLong);
	  END
         else if (MomCPU<CPU80251) WrError(1910);
         else
          BEGIN
           PutCode(0x18a);
           BAsmCode[CodeLen++] =(AdrLong >> 16) & 0xff;
           BAsmCode[CodeLen++] =(AdrLong >>  8) & 0xff;
           BAsmCode[CodeLen++] = AdrLong        & 0xff;
          END
	END
      END
     return;
    END

   if (Memo("CALL"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]=='@')
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg);
       switch (AdrMode)
        BEGIN
         case ModIReg:
          PutCode(0x199);
          BAsmCode[CodeLen++] = 0x04 + (AdrSize << 1) + (AdrPart << 4);
          break;
        END
      END
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK);
       if (OK)
	BEGIN
	 if ((NOT Chk504(EProgCounter())) AND ((AdrLong >> 11)==((EProgCounter()+2) >> 11)))
	  BEGIN
	   PutCode(0x11 + ((Hi(AdrLong) & 7) << 5));
	   BAsmCode[CodeLen++] = Lo(AdrLong);
	  END
         else if (MomCPU<CPU8051) WrError(1910);
         else if ((AdrLong >> 16)!=((EProgCounter()+3) >> 16)) WrError(1910);
         else
	  BEGIN
	   PutCode(0x12);
	   BAsmCode[CodeLen++] = Hi(AdrLong);
	   BAsmCode[CodeLen++] = Lo(AdrLong);
	  END
	END
      END
     return;
    END

   for (z=0; z<BCondOrderCnt; z++)
    if (Memo(BCondOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        AdrLong=EvalIntExpression(ArgStr[2],UInt24,&OK);
        Questionable=SymbolQuestionable;
        if (OK)
         BEGIN
          ChkSpace(SegCode);
          switch (DecodeBitAdr(ArgStr[1],&BitLong,True))
           BEGIN
            case ModBit51:
             AdrLong-=EProgCounter()+3+Ord(NeedsPrefix(BCondOrders[z].Code));
             if (((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable)) WrError(1370);
             else
              BEGIN
               PutCode(BCondOrders[z].Code);
               BAsmCode[CodeLen++] = BitLong & 0xff;
               BAsmCode[CodeLen++] = AdrLong & 0xff;
              END
             break;
            case ModBit251:
             AdrLong-=EProgCounter()+4+Ord(NeedsPrefix(0x1a9));
             if (((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable)) WrError(1370);
             else
              BEGIN
               PutCode(0x1a9);
               BAsmCode[CodeLen++] = BCondOrders[z].Code+(BitLong >> 24);
               BAsmCode[CodeLen++] = BitLong & 0xff;
               BAsmCode[CodeLen++] = AdrLong & 0xff;
              END
             break;
           END
         END
       END
      return;
     END

   if (Memo("DJNZ"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[2],UInt24,&OK); 
       Questionable=SymbolQuestionable;
       if (OK)
        BEGIN
         DecodeAdr(ArgStr[1],MModReg+MModDir8);
         switch (AdrMode)
          BEGIN
           case ModReg:
	    if ((OpSize!=0) OR (AdrPart>7)) WrError(1350);
	    else
	     BEGIN
	      AdrLong-=EProgCounter()+2+Ord(NeedsPrefix(0xd8+AdrPart));
              if (((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable)) WrError(1370);
              else
               BEGIN
	        PutCode(0xd8+AdrPart);
                BAsmCode[CodeLen++] = AdrLong & 0xff;
               END
             END
            break;
           case ModDir8:
            AdrLong-=EProgCounter()+3+Ord(NeedsPrefix(0xd5));
            if (((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable)) WrError(1370);
            else
             BEGIN
              PutCode(0xd5);
              BAsmCode[CodeLen++] = AdrVals[0];
              BAsmCode[CodeLen++] = Lo(AdrLong);
             END
            break;
          END
        END
      END
     return;
    END

   if (Memo("CJNE"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[3],UInt24,&OK);
       Questionable=SymbolQuestionable;
       if (OK)
        BEGIN
	 DecodeAdr(ArgStr[1],MModAcc+MModIReg8+MModReg);
         switch (AdrMode)
          BEGIN
           case ModAcc:
            DecodeAdr(ArgStr[2],MModDir8+MModImm);
            switch (AdrMode)
             BEGIN
              case ModDir8:
               AdrLong-=EProgCounter()+3+Ord(NeedsPrefix(0xb5));
               if (((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable)) WrError(1370);
               else
                BEGIN
                 PutCode(0xb5);
	         BAsmCode[CodeLen++] = AdrVals[0];
                 BAsmCode[CodeLen++] = AdrLong & 0xff;
                END
               break;
 	      case ModImm:
               AdrLong-=EProgCounter()+3+Ord(NeedsPrefix(0xb5));
               if (((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable)) WrError(1370);
               else
                BEGIN
	         PutCode(0xb4);
	         BAsmCode[CodeLen++] = AdrVals[0];
                 BAsmCode[CodeLen++] = AdrLong & 0xff;
	        END
               break;
             END
            break;
           case ModReg:
	    if ((OpSize!=0) OR (AdrPart>7)) WrError(1350);
	    else
	     BEGIN
              HReg=AdrPart;
              DecodeAdr(ArgStr[2],MModImm);
              switch (AdrMode)
               BEGIN
                case ModImm:
                 AdrLong-=EProgCounter()+3+Ord(NeedsPrefix(0xb8+HReg));
                 if (((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable)) WrError(1370);
                 else
                  BEGIN
                   PutCode(0xb8+HReg);
                   BAsmCode[CodeLen++] = AdrVals[0];
                   BAsmCode[CodeLen++] = AdrLong & 0xff;
                  END
                 break;
               END
             END
            break;
           case ModIReg8:
            HReg=AdrPart; SetOpSize(0);
            DecodeAdr(ArgStr[2],MModImm);
            switch (AdrMode)
             BEGIN
	      case ModImm:
               AdrLong-=EProgCounter()+3+Ord(NeedsPrefix(0xb6+HReg));
               if (((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable)) WrError(1370);
               else
                BEGIN
                 PutCode(0xb6+HReg);
                 BAsmCode[CodeLen++] = AdrVals[0];
                 BAsmCode[CodeLen++] = AdrLong & 0xff;
                END
               break;
             END
            break;
          END
        END
      END
     return;
    END

   /* Arithmetik */

   if (Memo("ADD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModReg);
       switch (AdrMode)
        BEGIN
         case ModAcc:
          DecodeAdr(ArgStr[2],MModImm+MModDir8+MModDir16+MModIReg8+MModIReg+MModReg);
          switch (AdrMode)
           BEGIN
            case ModImm:
             PutCode(0x24); BAsmCode[CodeLen++] = AdrVals[0];
             break;
            case ModDir8:
             PutCode(0x25); BAsmCode[CodeLen++] = AdrVals[0];
             break;
            case ModDir16:
             PutCode(0x12e);
	     BAsmCode[CodeLen++] = (AccReg << 4) + 3;
             memcpy(BAsmCode+CodeLen,AdrVals,2);
             CodeLen+=2;
             break;
            case ModIReg8:
             PutCode(0x26+AdrPart);
             break;
            case ModIReg:
             PutCode(0x12e);
             BAsmCode[CodeLen++] = 0x09 + AdrSize + (AdrPart << 4);
             BAsmCode[CodeLen++] = AccReg << 4;
             break;
            case ModReg:
             if ((AdrPart<8) AND (NOT SrcMode)) PutCode(0x28+AdrPart);
             else if (MomCPU<CPU80251) WrError(1505);
             else
              BEGIN
               PutCode(0x12c);
               BAsmCode[CodeLen++] = AdrPart + (AccReg << 4);
              END;
             break;
           END
          break;
         case ModReg:
          if (MomCPU<CPU80251) WrError(1505);
          else
           BEGIN
            HReg=AdrPart;
            DecodeAdr(ArgStr[2],MModImm+MModReg+MModDir8+MModDir16+MModIReg8+MModIReg);
            switch (AdrMode)
             BEGIN
              case ModImm:
               if ((OpSize==0) AND (HReg==AccReg))
                BEGIN
                 PutCode(0x24);
	         BAsmCode[CodeLen++] = AdrVals[0];
                END
               else
                BEGIN
                 PutCode(0x12e);
                 BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2);
                 memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
                 CodeLen+=AdrCnt;
                END
               break;
              case ModReg:
               PutCode(0x12c+OpSize); 
               if (OpSize==2) BAsmCode[CodeLen-1]++;
	       BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
               break;
              case ModDir8:
               if (OpSize==2) WrError(1350);
               else if ((OpSize==0) AND (HReg==AccReg))
                BEGIN
                 PutCode(0x25);
                 BAsmCode[CodeLen++] = AdrVals[0];
                END
               else
                BEGIN
                 PutCode(0x12e);
                 BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2) + 1;
                 BAsmCode[CodeLen++] = AdrVals[0];
                END
               break;
              case ModDir16:
               if (OpSize==2) WrError(1350);
               else
                BEGIN
                 PutCode(0x12e);
                 BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2) + 3;
                 memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
                 CodeLen+=AdrCnt;
                END
               break;
              case ModIReg8:
               if ((OpSize!=0) OR (HReg!=AccReg)) WrError(1350);
	       else PutCode(0x26+AdrPart);
               break;
              case ModIReg:
               if (OpSize!=0) WrError(1350);
               else
                BEGIN
                 PutCode(0x12e);
                 BAsmCode[CodeLen++] = 0x09 + AdrSize + (AdrPart << 4);
                 BAsmCode[CodeLen++] = HReg << 4;
                END
               break;
             END
           END
          break;
        END
      END
     return;
    END

   if ((Memo("SUB")) OR (Memo("CMP")))
    BEGIN
     z=(Memo("SUB")) ? 0x90 : 0xb0;
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80251) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       switch (AdrMode)
        BEGIN
         case ModReg:
          HReg=AdrPart;
          if (Memo("CMP"))
	   DecodeAdr(ArgStr[2],MModImm+MModImmEx+MModReg+MModDir8+MModDir16+MModIReg);
          else
           DecodeAdr(ArgStr[2],MModImm+MModReg+MModDir8+MModDir16+MModIReg);
          switch (AdrMode)
           BEGIN
            case ModImm:
             PutCode(0x10e + z);
             BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2);
             memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
             CodeLen+=AdrCnt;
             break;
            case ModImmEx:
             PutCode(0x10e + z);
             BAsmCode[CodeLen++] = (HReg << 4) + 0x0c;
             memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
             CodeLen+=AdrCnt;
             break;
            case ModReg:
             PutCode(0x10c + z + OpSize);
	     if (OpSize==2) BAsmCode[CodeLen-1]++;
	     BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
             break;
            case ModDir8:
             if (OpSize==2) WrError(1350);
             else
              BEGIN
               PutCode(0x10e + z);
               BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2) + 1;
               BAsmCode[CodeLen++] = AdrVals[0];
              END
             break;
            case ModDir16:
             if (OpSize==2) WrError(1350);
             else
              BEGIN
               PutCode(0x10e + z);
               BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2) + 3;
               memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
               CodeLen+=AdrCnt;
              END
             break;
            case ModIReg:
             if (OpSize!=0) WrError(1350);
             else
              BEGIN
               PutCode(0x10e + z);
               BAsmCode[CodeLen++] = 0x09 + AdrSize + (AdrPart << 4);
               BAsmCode[CodeLen++] = HReg << 4;
              END
             break;
           END
          break;
        END
      END
     return;
    END

   if ((Memo("ADDC")) OR (Memo("SUBB")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc);
       switch (AdrMode)
        BEGIN
         case ModAcc:
          HReg=(Memo("ADDC")) ? 0x30 : 0x90;
          DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModDir8+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg:
	     if (AdrPart>7) WrError(1350);
	     else PutCode(HReg+0x08+AdrPart);
             break;
            case ModIReg8:
             PutCode(HReg+0x06+AdrPart);
             break;
            case ModDir8:
             PutCode(HReg+0x05);
	     BAsmCode[CodeLen++] = AdrVals[0];
             break;
            case ModImm:
             PutCode(HReg+0x04);
	     BAsmCode[CodeLen++] = AdrVals[0];
             break;
           END
          break;
        END
      END
     return;
    END

   if ((Memo("DEC")) OR (Memo("INC")))
    BEGIN
     if (ArgCnt==1) strmaxcpy(ArgStr[++ArgCnt],"#1",255);
     z=Ord(Memo("DEC")) << 4;
     if (ArgCnt!=2) WrError(1110);
     else if (*ArgStr[2]!='#') WrError(1350);
     else
      BEGIN
       FirstPassUnknown=False;
       HReg=EvalIntExpression(ArgStr[2]+1,UInt3,&OK);
       if (FirstPassUnknown) HReg=1;
       if (OK)
        BEGIN
         OK=True;
         if (HReg==1) HReg=0;
         else if (HReg==2) HReg=1;
         else if (HReg==4) HReg=2;
         else OK=False;
         if (NOT OK) WrError(1320);
         else if (strcasecmp(ArgStr[1],"DPTR")==0)
          BEGIN
           if (Memo("DEC")) WrError(1350);
           else if (HReg!=0) WrError(1320);
           else PutCode(0xa3);
          END
         else
          BEGIN
           DecodeAdr(ArgStr[1],MModAcc+MModReg+MModDir8+MModIReg8);
           switch (AdrMode)
            BEGIN
             case ModAcc:
              if (HReg==0) PutCode(0x04+z);
              else if (MomCPU<CPU80251) WrError(1320);
              else
               BEGIN
                PutCode(0x10b+z);
                BAsmCode[CodeLen++] = (AccReg << 4) + HReg;
               END
              break;
             case ModReg:
              if ((OpSize==0) AND (AdrPart==AccReg) AND (HReg==0)) PutCode(0x04+z);
              else if ((AdrPart<8) AND (OpSize==0) AND (NOT SrcMode)) PutCode(0x08+z+AdrPart);
              else if (MomCPU<CPU80251) WrError(1505);
              else
               BEGIN
                PutCode(0x10b+z);
                BAsmCode[CodeLen++] = (AdrPart << 4) + (OpSize << 2) + HReg;
                if (OpSize==2) BAsmCode[CodeLen-1]+=4;
               END
              break;
             case ModDir8:
              if (HReg!=0) WrError(1320);
              else
               BEGIN
                PutCode(0x05+z);
                BAsmCode[CodeLen++] = AdrVals[0];
               END
              break;
             case ModIReg8:
              if (HReg!=0) WrError(1320);
              else PutCode(0x06+z+AdrPart);
              break;
            END
          END
        END
      END
     return;
    END

   if ((Memo("DIV")) OR (Memo("MUL")))
    BEGIN
     z=Ord(Memo("MUL")) << 5;
     if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
     else if (ArgCnt==1) 
      BEGIN
       if (strcasecmp(ArgStr[1],"AB")!=0) WrError(1350);
       else PutCode(0x84+z);
      END
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       switch (AdrMode)
        BEGIN
         case ModReg:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg);
          switch (AdrMode)
           BEGIN
            case ModReg:
             if (MomCPU<CPU80251) WrError(1505);
             else if (OpSize==2) WrError(1350);
             else
              BEGIN
               PutCode(0x18c+z+OpSize);
               BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
              END
             break;
           END
          break;
        END
      END
     return;
    END

   /* Logik */

   if ((Memo("ANL")) OR (Memo("ORL")) OR (Memo("XRL")))
    BEGIN
     DecodeLogic();
     return;
    END

   if ((Memo("CLR")) OR (Memo("CPL")) OR (Memo("SETB")))
    BEGIN
     if (Memo("CLR")) z=0x10;
     else if (Memo("CPL")) z=0;
     else z=0x20;
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")==0)
      BEGIN
       if (Memo("SETB")) WrError(1350);
       else PutCode(0xf4-z);
      END
     else if ((strcasecmp(ArgStr[1],"C")==0) OR (strcasecmp(ArgStr[1],"CY")==0))
      PutCode(0xb3+z);
     else 
      switch (DecodeBitAdr(ArgStr[1],&AdrLong,True))
       BEGIN
        case ModBit51:
         PutCode(0xb2+z);
         BAsmCode[CodeLen++] = AdrLong & 0xff;
         break;
        case ModBit251:
         PutCode(0x1a9);
         BAsmCode[CodeLen++] = 0xb0 + z + (AdrLong >> 24);
         BAsmCode[CodeLen++] = AdrLong & 0xff;
         break;
       END
     return;
    END

   if ((Memo("SRA")) OR (Memo("SRL")) OR (Memo("SLL")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU80251) WrError(1500);
     else
      BEGIN
       if (Memo("SLL")) z=0x30;
       else if (Memo("SRA")) z=0;
       else z=0x10;
       DecodeAdr(ArgStr[1],MModReg);
       switch (AdrMode)
        BEGIN
         case ModReg:
          if (OpSize==2) WrError(1350);
          else
           BEGIN
            PutCode(0x10e + z);
            BAsmCode[CodeLen++] = (AdrPart << 4) + (OpSize << 2);
           END
          break;
        END
      END
     return;
    END

/*=========================================================================*/

   WrXError(1200,OpPart);
END

	static Boolean ChkPC_51(void)
BEGIN
   Boolean ok;

   if (MomCPU<CPU80251)
    switch (ActPC)
     BEGIN
      case SegCode:
       if (MomCPU==CPU87C750) ok=(ProgCounter()<0x800);
       else ok=(ProgCounter()<0x10000);
       break;
      case SegXData:
       ok=(ProgCounter()<0x10000);
       break;
      case SegData:
      case SegBData:
       ok=(ProgCounter()<0x100);
       break;
      case SegIData:
       ok=(ProgCounter()<0x100);
       break;
      default:
       ok=False;
     END
   else
    switch (ActPC)
     BEGIN
      case SegCode:
       ok=(ProgCounter()<0x1000000);
       break;
      case SegIO:
       ok=(ProgCounter()<0x200);
       break;
      default: 
       ok=False;
     END

   return (ok);
END

	static Boolean IsDef_51(void)
BEGIN
   if (MomCPU<CPU80251)
    return ((Memo("BIT")) OR (Memo("SFR")) OR (Memo("SFRB")));
   else
    return ((Memo("BIT")) OR (Memo("SFR")) OR (Memo("PORT")));
END

        static void InitPass_51(void)
BEGIN
   SaveInitProc();
   SetFlag(&SrcMode,SrcModeName,False);
   SetFlag(&BigEndian,BigEndianName,False);
END

        static void SwitchFrom_51(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_51(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x31; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   if (MomCPU>=CPU80251)
    BEGIN
     ValidSegs=(1<<SegCode)|(1<<SegIO);
     Grans[SegCode ]=1; ListGrans[SegCode ]=1; SegInits[SegCode ]=0;
     Grans[SegIO   ]=1; ListGrans[SegIO   ]=1; SegInits[SegIO   ]=0;
    END
   else
    BEGIN
     ValidSegs=(1<<SegCode)|(1<<SegData)|(1<<SegIData)|(1<<SegXData)|(1<<SegBData);
     Grans[SegCode ]=1; ListGrans[SegCode ]=1; SegInits[SegCode ]=0;
     Grans[SegData ]=1; ListGrans[SegData ]=1; SegInits[SegData ]=0x30;
     Grans[SegIData]=1; ListGrans[SegIData]=1; SegInits[SegIData]=0x80;
     Grans[SegXData]=1; ListGrans[SegXData]=1; SegInits[SegXData]=0;
     Grans[SegBData]=1; ListGrans[SegBData]=1; SegInits[SegBData]=0;
    END

   MakeCode=MakeCode_51; ChkPC=ChkPC_51; IsDef=IsDef_51;

   InitFields(); SwitchFrom=SwitchFrom_51;
END


	void code51_init(void)
BEGIN
   CPU87C750 =AddCPU("87C750",SwitchTo_51);
   CPU8051   =AddCPU("8051"  ,SwitchTo_51);
   CPU8052   =AddCPU("8052"  ,SwitchTo_51);
   CPU80C320 =AddCPU("80C320",SwitchTo_51);
   CPU80501  =AddCPU("80C501",SwitchTo_51);
   CPU80502  =AddCPU("80C502",SwitchTo_51);
   CPU80504  =AddCPU("80C504",SwitchTo_51);
   CPU80515  =AddCPU("80515" ,SwitchTo_51);
   CPU80517  =AddCPU("80517" ,SwitchTo_51);
   CPU80251  =AddCPU("80C251",SwitchTo_51);

   SaveInitProc=InitPassProc;
   InitPassProc=InitPass_51;
END
