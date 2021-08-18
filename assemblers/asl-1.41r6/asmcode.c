// AS-Portierung
// Verwaltung der Code-Datei
#include "stdinc.h"
#include <string.h>

#include "endian.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"

static Word LenSoFar;
static LongInt RecPos, LenPos;

#define CodeBufferSize 512

static Word CodeBufferFill;
static Byte CodeBuffer[CodeBufferSize + 1];

/**
	static bool Write2(void *Buffer)
{
   bool OK;

   if (BigEndian) WSwap(Buffer,2);
   OK=(fwrite(Buffer,1,2,PrgFile)==2);
   if (BigEndian) WSwap(Buffer,2);
   return OK;
}

	static bool Write4(void *Buffer)
{
   bool OK;

   if (BigEndian) DSwap(Buffer,4);
   OK=(fwrite(Buffer,1,4,PrgFile)==4);
   if (BigEndian) DSwap(Buffer,4);
   return OK;
}
**/
static void FlushBuffer(void) {
   if (CodeBufferFill > 0) {
      if (fwrite(CodeBuffer, 1, CodeBufferFill, PrgFile) != CodeBufferFill) ChkIO(10004);
      CodeBufferFill = 0;
   }
}

void DreheCodes(void) {
   Integer z;
   LongInt l = CodeLen * Granularity();

   switch (ListGran()) {
      case 2:
         for (z = 0; z < (l >> 1); z++)
            WAsmCode[z] = ((WAsmCode[z] & 0xff) << 8) + ((WAsmCode[z] & 0xff00) >> 8);
         break;
      case 4:
         for (z = 0; z < (l >> 2); z++) {
            DAsmCode[z] = ((DAsmCode[z] & 0xff000000u) >> 24) + ((DAsmCode[z] & 0x00ff0000u) >> 8) + ((DAsmCode[z] & 0x0000ff00u) << 8) + ((DAsmCode[z] & 0x000000ffu) << 24);
         }
         break;
   }
}

/*--- neuen Record in Codedatei anlegen.  War der bisherige leer, so wird ---
 ---- dieser ueberschrieben. ------------------------------------------------*/

static void WrRecHeader(void) {
   Byte b;

   if (ActPC != SegCode) {
      b = FileHeaderDataRec;
      if (fwrite(&b, 1, 1, PrgFile) != 1) ChkIO(10004);
   }
   if (fwrite(&HeaderID, 1, 1, PrgFile) != 1) ChkIO(10004);
   if (ActPC != SegCode) {
      b = ActPC;
      if (fwrite(&b, 1, 1, PrgFile) != 1) ChkIO(10004);
      b = Grans[ActPC];
      if (fwrite(&b, 1, 1, PrgFile) != 1) ChkIO(10004);
   }
}

void NewRecord(void) {
   LongInt h;
   LongWord PC;

   FlushBuffer();
   if (LenSoFar == 0) {
      if (fseek(PrgFile, RecPos, SEEK_SET) != 0) ChkIO(10003);
      WrRecHeader();
      h = PCs[ActPC];
      if (!Write4(PrgFile, &h)) ChkIO(10004);
      LenPos = ftell(PrgFile);
      if (!Write2(PrgFile, &LenSoFar)) ChkIO(10004);
   } else {
      h = ftell(PrgFile);
      if (fseek(PrgFile, LenPos, SEEK_SET) != 0) ChkIO(10003);
      if (!Write2(PrgFile, &LenSoFar)) ChkIO(10004);
      if (fseek(PrgFile, h, SEEK_SET) != 0) ChkIO(10003);

      RecPos = h;
      LenSoFar = 0;
      WrRecHeader();
      PC = PCs[ActPC];
      if (!Write4(PrgFile, &PC)) ChkIO(10004);
      LenPos = ftell(PrgFile);
      if (!Write2(PrgFile, &LenSoFar)) ChkIO(10004);
   }
}

/*--- Codedatei eroeffnen --------------------------------------------------*/

void OpenFile(void) {
   Word h;

   errno = 0;
   PrgFile = fopen(OutName, OPENWRMODE);
   if (PrgFile == NULL) ChkIO(10001);

   errno = 0;
   h = FileMagic;
   if (!Write2(PrgFile, &h)) ChkIO(10004);

   CodeBufferFill = 0;
   RecPos = ftell(PrgFile);
   LenSoFar = 0;
   NewRecord();
}

/*---- Codedatei schliessen -------------------------------------------------*/

void CloseFile(void) {
   Byte Head;
   char h[20];
   LongWord Adr;
   strcpy(h, "AS ");

   NewRecord();
   fseek(PrgFile, RecPos, SEEK_SET);

   if (StartAdrPresent) {
      Head = FileHeaderStartAdr;
      if (fwrite(&Head, sizeof(Head), 1, PrgFile) != 1) ChkIO(10004);
      Adr = StartAdr;
      if (!Write4(PrgFile, &Adr)) ChkIO(10004);
   }

   strcat(h, Version);
   Head = FileHeaderEnd;
   if (fwrite(&Head, sizeof(Head), 1, PrgFile) != 1) ChkIO(10004);
   if (fwrite(h, 1, strlen(h), PrgFile) != strlen(h)) ChkIO(10004);
   fclose(PrgFile);
   if (Magic != 0) unlink(OutName);
}

/*--- erzeugten Code einer Zeile in Datei ablegen ---------------------------*/

void WriteBytes(void) {
   Word ErgLen;

   if (CodeLen == 0) return;
   ErgLen = CodeLen * Granularity();
   if ((TurnWords != 0) != (BigEndian != 0)) DreheCodes();
   if (((LongInt) LenSoFar) + ((LongInt) ErgLen) > 0xffff) NewRecord();
   if (CodeBufferFill + ErgLen < CodeBufferSize) {
      memcpy(CodeBuffer + CodeBufferFill, BAsmCode, ErgLen);
      CodeBufferFill += ErgLen;
   } else {
      FlushBuffer();
      if (ErgLen < CodeBufferSize) {
         memcpy(CodeBuffer, BAsmCode, ErgLen);
         CodeBufferFill = ErgLen;
      } else if (fwrite(BAsmCode, 1, ErgLen, PrgFile) != ErgLen) ChkIO(10004);
   }
   LenSoFar += ErgLen;
   if ((TurnWords != 0) != (BigEndian != 0)) DreheCodes();
}

void RetractWords(Word Cnt) {
   Word ErgLen;

   ErgLen = Cnt * Granularity();
   if (LenSoFar < ErgLen) {
      WrError(1950);
      return;
   }

   if (MakeUseList) DeleteChunk(SegChunks + ActPC, ProgCounter() - Cnt, Cnt);

   PCs[ActPC] -= Cnt;

   if (CodeBufferFill >= ErgLen) CodeBufferFill -= ErgLen;
   else {
      if (fseek(PrgFile, SEEK_CUR, -(ErgLen - CodeBufferFill)) == -1)
         ChkIO(10004);
      CodeBufferFill = 0;
   }

   LenSoFar -= ErgLen;

   Retracted = true;
}

void asmcode_init(void) {
}
