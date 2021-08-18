// AS-Portierung
// Little/Big-Endian-Routinen
extern bool BigEndian;

extern char *Integ16Format, *Integ32Format, *Integ64Format;
extern char *IntegerFormat, *LongIntFormat, *QuadIntFormat;
extern char *LargeIntFormat;

void WSwap(void *Field, int Cnt);
void DSwap(void *Field, int Cnt);
void QSwap(void *Field, int Cnt);
void DWSwap(void *Field, int Cnt);
void QWSwap(void *Field, int Cnt);
void Double_2_TenBytes(Double inp, Byte * dest);
bool Read2(FILE * file, void *Ptr);
bool Read4(FILE * file, void *Ptr);
bool Read8(FILE * file, void *Ptr);
bool Write2(FILE * file, void *Ptr);
bool Write4(FILE * file, void *Ptr);
bool Write8(FILE * file, void *Ptr);
char *Dec32BlankString(LongInt number, int Stellen);
void endian_init(void);
