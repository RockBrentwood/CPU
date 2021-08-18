// AS-Portierung
// Verwaltung von Datei-Nummern
void InitFileList(void);
void ClearFileList(void);
void AddFile(char *FName);
Integer GetFileNum(char *Name);
char *GetFileName(Byte Num);
Integer GetFileCount(void);
void asmfnums_init(void);
