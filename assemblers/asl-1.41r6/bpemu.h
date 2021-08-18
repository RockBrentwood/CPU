// AS-Portierung
// Emulation einiger Borland-Pascal-Funktionen
char *FExpand(char *Src);
char *FSearch(char *File, char *Path);
long FileSize(FILE * file);
Byte Lo(Word inp);
Byte Hi(Word inp);
bool Odd(int inp);
void bpemu_init(void);
