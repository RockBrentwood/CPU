/* endian.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Little/Big-Endian-Routinen                                                */
/*                                                                           */
/* Historie: 30. 5.1996 Grundsteinlegung                                     */
/*            6. 7.1997 Dec32BlankString dazu                                */
/*                                                                           */
/*****************************************************************************/

extern bool BigEndian;

extern char *Integ16Format,*Integ32Format,*Integ64Format;
extern char *IntegerFormat,*LongIntFormat,*QuadIntFormat;
extern char *LargeIntFormat;


extern void WSwap(void *Field, int Cnt);

extern void DSwap(void *Field, int Cnt);

extern void QSwap(void *Field, int Cnt);

extern void DWSwap(void *Field, int Cnt);

extern void QWSwap(void *Field, int Cnt);


extern void Double_2_TenBytes(Double inp, Byte *dest);


extern bool Read2(FILE *file, void *Ptr);

extern bool Read4(FILE *file, void *Ptr);

extern bool Read8(FILE *file, void *Ptr);


extern bool Write2(FILE *file, void *Ptr);

extern bool Write4(FILE *file, void *Ptr);

extern bool Write8(FILE *file, void *Ptr);


extern char *Dec32BlankString(LongInt number, int Stellen);


extern void endian_init(void);
