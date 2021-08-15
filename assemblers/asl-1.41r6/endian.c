/* endian.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Little/Big-Endian-Routinen                                                */
/*                                                                           */
/* Historie: 30. 5.1996 Grundsteinlegung                                     */
/*            6. 7.1997 Dec32BlankString dazu                                */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>

#include "endian.h"

/*****************************************************************************/

Boolean BigEndian;

char *Integ16Format,*Integ32Format,*Integ64Format;
char *IntegerFormat,*LongIntFormat,*QuadIntFormat;
char *LargeIntFormat;

/*****************************************************************************/

	void WSwap(void *Field, int Cnt)
BEGIN
   register unsigned char *Run=(unsigned char *) Field,Swap;
   register int z;
   
   for (z=0; z<Cnt/2; z++,Run+=2)
    BEGIN
     Swap=Run[0]; Run[0]=Run[1]; Run[1]=Swap;
    END
END

	void DSwap(void *Field, int Cnt)
BEGIN
   register unsigned char *Run=(unsigned char *) Field,Swap;
   register int z;

   for (z=0; z<Cnt/4; z++,Run+=4)
    BEGIN
     Swap=Run[0]; Run[0]=Run[3]; Run[3]=Swap;
     Swap=Run[1]; Run[1]=Run[2]; Run[2]=Swap;
    END
END

	void QSwap(void *Field, int Cnt)
BEGIN
   register unsigned char *Run=(unsigned char *) Field,Swap;
   register int z;

   for (z=0; z<Cnt/8; z++,Run+=8)
    BEGIN
     Swap=Run[0]; Run[0]=Run[7]; Run[7]=Swap;
     Swap=Run[1]; Run[1]=Run[6]; Run[6]=Swap;
     Swap=Run[2]; Run[2]=Run[5]; Run[5]=Swap;
     Swap=Run[3]; Run[3]=Run[4]; Run[4]=Swap;
    END
END

	void DWSwap(void *Field, int Cnt)
BEGIN
   register unsigned char *Run=(unsigned char *) Field,Swap;
   register int z;

   for (z=0; z<Cnt/4; z++,Run+=4)
    BEGIN
     Swap=Run[0]; Run[0]=Run[2]; Run[2]=Swap;
     Swap=Run[1]; Run[1]=Run[3]; Run[3]=Swap;
    END
END

	void QWSwap(void *Field, int Cnt)
BEGIN
   register unsigned char *Run=(unsigned char *) Field,Swap;
   register int z;

   for (z=0; z<Cnt/8; z++,Run+=8)
    BEGIN
     Swap=Run[0]; Run[0]=Run[6]; Run[6]=Swap;
     Swap=Run[1]; Run[1]=Run[7]; Run[7]=Swap;
     Swap=Run[2]; Run[2]=Run[4]; Run[4]=Swap;
     Swap=Run[3]; Run[3]=Run[5]; Run[5]=Swap;
    END
END

	void Double_2_TenBytes(Double inp, Byte *dest)
BEGIN
   Byte Buffer[8];
   Byte Sign,z;
   Word Exponent;
   Boolean Denormal;

   memcpy(Buffer,&inp,8); if (BigEndian) QSwap(Buffer,8);
   Sign=(Buffer[7]&0x80);
   Exponent=(Buffer[6]>>4)+(((Word) Buffer[7]&0x7f)<<4);
   Denormal=(Exponent==0);
   if (Exponent==2047) Exponent=32767;
   else Exponent+=(16383-1023);
   dest[9]=Sign|((Exponent>>8)&0x7f);
   dest[8]=Exponent&0xff;
   Buffer[6]&=0x0f; if (NOT Denormal) Buffer[6]|=0x10;
   for (z=7; z>=2; z--)
    dest[z]=((Buffer[z-1]&0x1f)<<3)|((Buffer[z-2]&0xe0)>>5);
   dest[1]=(Buffer[0]&0x1f)<<3;
   dest[0]=0;   
END


	Boolean Read2(FILE *file, void *Ptr)
BEGIN
   if (fread(Ptr,1,2,file)!=2) return False;
   if (BigEndian) WSwap(Ptr,2);
   return True; 
END

	Boolean Read4(FILE *file, void *Ptr)
BEGIN
   if (fread(Ptr,1,4,file)!=4) return False;
   if (BigEndian) DSwap(Ptr,4);
   return True; 
END

	Boolean Read8(FILE *file, void *Ptr)
BEGIN
   if (fread(Ptr,1,8,file)!=8) return False;
   if (BigEndian) QSwap(Ptr,8);
   return True; 
END



	Boolean Write2(FILE *file, void *Ptr)
BEGIN
   Boolean OK;
 
   if (BigEndian) WSwap(Ptr,2);
   OK=(fwrite(Ptr,1,2,file)==2);
   if (BigEndian) WSwap(Ptr,2);
   return OK; 
END

	Boolean Write4(FILE *file, void *Ptr)
BEGIN
   Boolean OK; 

   if (BigEndian) DSwap(Ptr,4);
   OK=(fwrite(Ptr,1,4,file)==4);
   if (BigEndian) DSwap(Ptr,4);
   return OK; 
END

	Boolean Write8(FILE *file, void *Ptr)
BEGIN
   Boolean OK; 

   if (BigEndian) QSwap(Ptr,8);
   OK=(fwrite(Ptr,1,8,file)==8);
   if (BigEndian) QSwap(Ptr,8);
   return OK; 
END


	static void CheckSingle(int Is, int Should, char *Name)
BEGIN
   if (Is!=Should)
    BEGIN
     fprintf(stderr,"Configuration error: Sizeof(%s) is %d, should be %d\n",
             Name,Is,Should);
     exit(255);
    END
END

        static void CheckDataTypes(void)
BEGIN
   CheckSingle(sizeof(Byte),    1,"Byte");
   CheckSingle(sizeof(ShortInt),1,"ShortInt");
#ifdef HAS16
   CheckSingle(sizeof(Word),    2,"Word");
   CheckSingle(sizeof(Integer), 2,"Integer");
#endif
   CheckSingle(sizeof(LongInt), 4,"LongInt");
   CheckSingle(sizeof(LongWord),4,"LongWord");
#ifdef HAS64
   CheckSingle(sizeof(QuadInt), 8,"QuadInt");
   CheckSingle(sizeof(QuadWord),8,"QuadWord");
#endif
   CheckSingle(sizeof(Single),  4,"Single");
   CheckSingle(sizeof(Double),  8,"Double");
END


	static char *AssignSingle(int size)
BEGIN
   if (size==sizeof(short)) return "%sd";
   else if (size==sizeof(int)) return "%d";
   else if (size==sizeof(long)) return "%ld";
#ifndef NOLONGLONG
   else if (size==sizeof(long long)) return "%lld";
#endif
   else
    BEGIN
     fprintf(stderr,
             "Configuration error: cannot assign format string for integer of size %d\n",size);
     exit(255);
     return "";
    END               
END

	static void AssignFormats(void)
BEGIN
#ifdef HAS16   
   IntegerFormat=Integ16Format=AssignSingle(2);
#endif
   LongIntFormat=Integ32Format=AssignSingle(4);
#ifdef HAS64
   QuadIntFormat=Integ64Format=AssignSingle(8);
#endif
   LargeIntFormat=AssignSingle(sizeof(LargeInt));
END

	char *Dec32BlankString(LongInt number, int Stellen)
BEGIN
   char Format[10];
   static char Erg[255];

   sprintf(Format,"%%%d%s",Stellen,LongIntFormat+1);
   sprintf(Erg,Format,number);

   return Erg;
END


	void endian_init(void)
BEGIN
   union
    {
     unsigned char field[sizeof(int)];
     int test;
    } TwoFace;

   CheckDataTypes(); AssignFormats();

   memset(TwoFace.field,0,sizeof(int)); 
   TwoFace.field[0]=1;
   BigEndian=((TwoFace.test)!=1);
   /*if (BigEndian)
    BEGIN
     fprintf(stderr,"Warning: Big endian machine!\n");
     fprintf(stderr,"AS is so far not adapted for big-endian-machines!\n");
    END*/
END


