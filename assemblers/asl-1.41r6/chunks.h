#ifndef _CHUNKS_H
#define _CHUNKS_H

// AS-Portierung
// Verwaltung von Adressbereichslisten
typedef struct {
   LargeWord Start, Length;
} OneChunk;

typedef struct {
   Word RealLen, AllocLen;
   OneChunk *Chunks;
} ChunkList;

bool AddChunk(ChunkList * NChunk, LargeWord NewStart, LargeWord NewLen, bool Warn);
void DeleteChunk(ChunkList * NChunk, LargeWord DelStart, LargeWord DelLen);
void InitChunk(ChunkList * NChunk);
void ClearChunk(ChunkList * NChunk);
void chunks_init(void);

#endif /* _CHUNKS_H */
