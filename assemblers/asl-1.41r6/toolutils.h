// AS-Portierung
// Unterroutinen fuer die AS-Tools
#include "fileformat.h"

extern LongWord Magic;

extern Word FileID;

extern char *OutName;

void WrCopyRight(char *Msg);
void DelSuffix(char *Name);
void AddSuffix(char *Name, char *Suff);
void FormatError(char *Name, char *Detail);
void ChkIO(char *Name);
Word Granularity(Byte Header);
void ReadRecordHeader(Byte * Header, Byte * Segment, Byte * Gran, char *Name, FILE * f);
void WriteRecordHeader(Byte * Header, Byte * Segment, Byte * Gran, char *Name, FILE * f);
CMDResult CMD_FilterList(bool Negate, char *Arg);
bool FilterOK(Byte Header);
bool RemoveOffset(char *Name, LongWord * Offset);
void toolutils_init(void);
