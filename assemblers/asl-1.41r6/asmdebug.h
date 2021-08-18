// AS-Portierung
// Verwaltung der Debug-Informationen zur Assemblierzeit
void AddLineInfo(bool InAsm, LongInt LineNum, char *FileName, ShortInt Space, LongInt Address);
void InitLineInfo(void);
void ClearLineInfo(void);
void DumpDebugInfo(void);
void asmdebug_init(void);
