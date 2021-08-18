// AS-Portierung
// Verwaltung der Code-Datei
extern Word LenSoFar;
extern LongInt RecPos;

void DreheCodes(void);
void NewRecord(void);
void OpenFile(void);
void CloseFile(void);
void WriteBytes(void);
void RetractWords(Word Cnt);
void asmcode_init(void);
