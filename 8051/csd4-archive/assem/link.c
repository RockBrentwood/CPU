#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "io.h"
#include "ex.h"
#include "res.h"
#include "st.h"

typedef struct FileBuf *FileBuf;
struct FileBuf {
   char *Name;
   long Loc, Files, Segs, Gaps, Syms, Exps, Relocs;
   char **File; Segment Seg; Gap G; Symbol *Sym;
};
FileBuf FTab; int Fs;

typedef struct Block *Block;
struct Block {
   char *Src, *Obj; Segment Seg; Block Next;
};
static Block *BHead;
static int *BSize;
static Segment USeg;

typedef struct Image *Image;
struct Image {
   word Base; long Size; char *Src, *Obj; word Line; long Loc;
   Image Prev, Next;
};
static Image IHead, ITail;

Exp *ExpBuf;

static Image FormImage(word Base, long Size, char *Src, char *Obj, word Line, long Loc) {
   Image IP = Allocate(sizeof *IHead);
   IP->Base = Base, IP->Size = Size, IP->Src = Src, IP->Obj = Obj, IP->Line = Line, IP->Loc = Loc;
   return IP;
}

static void SetImage(void) {
   IHead = ITail = NULL;
   for (Block BP = BHead[0]; BP != NULL; BP = BP->Next) {
      Segment Seg = BP->Seg;
      Image IP = IHead;
      for (; IP != NULL; IP = IP->Next)
         if (IP->Base >= Seg->Base) break;
      Image P = IP == NULL? ITail: IP->Prev;
      if (P != NULL && P->Base + P->Size > Seg->Base)
         Fatal("Overlapping segments at [%d] %s and [%d] %s.", P->Line, P->Src, Seg->Line, BP->Src);
      if (IP != NULL && Seg->Base + Seg->Size > IP->Base)
         Fatal("Overlapping segments at [%d] %s and [%d] %s.", Seg->Line, BP->Src, IP->Line, IP->Src);
      Image NewI = FormImage(Seg->Base, Seg->Size, BP->Src, BP->Obj, Seg->Line, Seg->Loc);
      if (P == NULL) IHead = NewI; else P->Next = NewI;
      if (IP == NULL) ITail = NewI; else IP->Prev = NewI;
      NewI->Next = IP, NewI->Prev = P;
   }
}

static void FitGap(Gap G) {
   word Base = G->Seg->Base + G->Offset; long Size = G->Size;
   Image IP = IHead;
   for (; IP != NULL; IP = IP->Next)
      if (IP->Base > Base) break;
   Image P = IP == NULL? ITail: IP->Prev;
   if (P == NULL) Fatal("Internal error (2).");
   long End = Base + Size, EndP = P->Base + P->Size;
   if (EndP < End) Fatal("Internal error (3).");
   if (P->Base < Base) {
      P->Size = Base - P->Base;
      if (EndP > End) {
         Image NewI = FormImage((word)End, EndP - End, P->Src, P->Obj, P->Line, P->Loc + P->Size);
         NewI->Next = IP, P->Next = NewI, NewI->Prev = P;
         if (IP == NULL) ITail = NewI; else IP->Prev = NewI;
      }
   } else if (EndP > End) P->Base += Size, P->Size -= Size;
   else {
      if (P->Prev == NULL) IHead = P->Next; else P->Prev->Next = P->Next;
      if (P->Next == NULL) ITail = P->Prev; else P->Next->Prev = P->Prev;
      free(P);
   }
}

typedef struct Free *Free;
struct Free {
   word Base; long Size; Block BP;
};
static Free *FHead;
static int *FSize;

void SetFree(void) {
   FHead = Allocate(SEG_TYPES*sizeof *FHead), FSize = Allocate(SEG_TYPES*sizeof *FSize);
   for (int T = 0; T < SEG_TYPES; T++) {
      FHead[T] = Allocate((BSize[T] + 1)*sizeof *FHead[T]);
      Block P = NULL; Free F = FHead[T];
      for (Block B = BHead[T]; B != NULL; P = B, B = B->Next) {
         long Base = P == NULL? AddrTab[T].Lo: (long)P->Seg->Base + P->Seg->Size;
         if (Base < B->Seg->Base)
            F->Base = Base, F->Size = (long)B->Seg->Base - Base, F->BP = P, F++;
      }
      long Base = P == NULL? AddrTab[T].Lo: (long)P->Seg->Base + P->Seg->Size;
      if (Base <= AddrTab[T].Hi)
         F->Base = Base, F->Size = (long)AddrTab[T].Hi + 1 - Base, F->BP = P, F++;
      FSize[T] = F - FHead[T];
   }
}

void PurgeFree(void) {
   for (int T = 0; T < SEG_TYPES; T++) free(FHead[T]);
   free(FHead), free(FSize);
}

void BestFit(char *Obj, Segment Seg) {
   if (!Seg->Rel) return;
   byte T = Seg->Type;
   StartLine = Seg->Line, StartF = Seg->File;
   Free Best = NULL;
   for (Free FP = FHead[T]; FP < FHead[T] + FSize[T]; FP++)
      if (Seg->Size <= FP->Size && (Best == NULL || FP->Size < Best->Size)) Best = FP;
   if (Best == NULL) Fatal("Cannot fit segment.");
   Block B = Allocate(sizeof *B);
   B->Seg = Seg, B->Src = FileTab[StartF], B->Obj = Obj;
   if (Best->BP == NULL)
      B->Next = BHead[T], BHead[T] = B;
   else
      B->Next = Best->BP->Next, Best->BP->Next = B;
   Seg->Rel = false, Seg->Base = Best->Base,
   Best->BP = B, Best->Base += Seg->Size, Best->Size -= Seg->Size;
}

void FitSegs(FileBuf F) {
   FileTab = F->File;
   for (int S = TYPES; S < F->Segs; S++) BestFit(F->Name, F->Seg + S - TYPES);
}

void SetSeg(void) {
   USeg = Allocate(TYPES*sizeof *USeg);
   for (Segment S = USeg; S < USeg + TYPES; S++)
      S->Line = 0, S->File = 0, S->Rel = false, S->Type = S - USeg, S->Base = 0, S->Size = 0, S->Loc = 0L;
   BHead = Allocate(SEG_TYPES*sizeof *BHead), BSize = Allocate(SEG_TYPES*sizeof *BSize);
   for (int T = 0; T < SEG_TYPES; T++) BHead[T] = NULL, BSize[T] = 0;
}

void FreeBlocks(void) {
   for (int T = 0; T < SEG_TYPES; T++) for (Block B = BHead[T]; B != NULL; ) {
      Block N = B->Next; free(B), B = N;
   }
   free(BHead), free(BSize);
}

void Fit(char *Obj, Segment Seg) {
   if (Seg->Rel) return;
   byte T = Seg->Type;
   Block Prev = NULL, Cur = BHead[T];
   for (; Cur != NULL; Prev = Cur, Cur = Cur->Next)
      if (Cur->Seg->Base >= Seg->Base) break;
   if (Prev != NULL && Prev->Seg->Base + Prev->Seg->Size > Seg->Base)
      Error("Overlapping segments: %s [%d] and %s [%d].", FileTab[Seg->File], Seg->Line, Prev->Src, Prev->Seg->Line);
   if (Cur != NULL && Cur->Seg->Base < Seg->Base + Seg->Size)
      Error("Overlapping segments: %s [%d] and %s [%d].", FileTab[Seg->File], Seg->Line, Cur->Src, Cur->Seg->Line);
   Block B = Allocate(sizeof *B);
   B->Src = FileTab[Seg->File], B->Obj = Obj, B->Seg = Seg, B->Next = Cur;
   if (Prev == NULL) BHead[T] = B; else Prev->Next = B;
   BSize[T]++;
}

void GetHead(FileBuf F) {
   FILE *InF = fopen(F->Name, "rb+");
   if (InF == NULL) Fatal("Cannot open %s.", F->Name);
   if (GetW(InF) != 0x55aa) Fatal("Invalid object file %s.", F->Name);
   F->Loc = GetL(InF), F->Files = GetL(InF), F->Segs = GetL(InF),
   F->Gaps = GetL(InF), F->Syms = GetL(InF), F->Exps = GetL(InF), F->Relocs = GetL(InF);
   long S = F->Loc + F->Files + F->Segs + F->Gaps + F->Syms + F->Exps + F->Relocs;
   if (GetL(InF) != S&0xffffffff) Fatal("Corrupt object file %s.", F->Name);
   F->File = Allocate((word)F->Files*sizeof *F->File);
   F->Seg = F->Segs < TYPES? NULL: Allocate((word)(F->Segs - TYPES)*sizeof *F->Seg);
   F->G = Allocate((word)F->Gaps*sizeof *F->G), F->Sym = Allocate((word)F->Syms*sizeof *F->Sym);
   FileTab = F->File;
   fseek(InF, F->Loc, SEEK_SET);
   for (long S = 0; S < F->Files; S++) {
      word L = GetW(InF);
      char Buf[0x100]; fread(Buf, 1, L, InF), Buf[L] = '\0', F->File[S] = CopyS(Buf);
   }
   for (long S = TYPES; S < F->Segs; S++) {
      Segment Seg = F->Seg + S - TYPES;
      Seg->Line = GetW(InF), Seg->File = GetW(InF);
      word U = GetW(InF); Seg->Size = GetW(InF);
      Seg->Base = GetW(InF), Seg->Loc = GetL(InF);
      Seg->Rel = (U >> 8)&1, Seg->Type = U&0xff;
      Fit(F->Name, Seg);
   }
   for (long S = 0; S < F->Gaps; S++) {
      Gap G = F->G + S; word Sg = GetW(InF);
      G->Offset = GetW(InF), G->Size = GetW(InF), G->Seg = Sg < TYPES? &USeg[Sg]: &F->Seg[Sg - TYPES];
   }
   for (long S = 0; S < F->Syms; S++) {
      byte B = GetB(InF); word U = GetW(InF), Offset = GetW(InF);
      char Buf[0x100];
      word L = GetW(InF);
      if (L > 0) fread(Buf, 1, L, InF);
      Buf[L] = '\0';
      bool Global = (B&8) != 0, Defined = (B&4) != 0, Address = (B&2) != 0;
      Symbol Sym = Global? LookUp(Buf): Allocate(sizeof *Sym);
      if (!Global) {
         Sym->Variable = false, Sym->Address = Address;
         Sym->Global = Global, Sym->Defined = Defined;
         Sym->Seg = !Address? NULL: U < TYPES? &USeg[U]: &F->Seg[U - TYPES];
         Sym->Offset = Offset;
         Sym->Index = F - FTab;
         Sym->Name = CopyS(Buf);
      } else if (Sym->Defined && Defined)
         Error("Symbol %s redefined: %s %s.", Sym->Name, FTab[Sym->Index].Name, F->Name);
      else {
         if (Sym->Address) {
            Segment Seg = U < TYPES? &USeg[U]: &F->Seg[U - TYPES];
            if (!Address)
               Error("Address %s redeclared as number: %s %s", Sym->Name, FTab[Sym->Index].Name, F->Name);
            else if (Sym->Seg->Type != Seg->Type)
               Error("Type mismatch %s: %s %s", Sym->Name, FTab[Sym->Index].Name, F->Name);
         } else if (Address && Sym->Global)
            Error("Number %s redeclared as address: %s %s", Sym->Name, FTab[Sym->Index].Name, F->Name);
         if (Defined || !Sym->Global) {
            Sym->Variable = false, Sym->Address = Address;
            Sym->Global = Global, Sym->Defined = Defined;
            Sym->Seg = !Address? NULL: U < TYPES? &USeg[U]: &F->Seg[U - TYPES],
            Sym->Offset = Offset;
            Sym->Index = F - FTab;
         }
      }
      F->Sym[S] = Sym;
   }
   F->Loc = ftell(InF);
   fclose(InF);
}

typedef struct RList *RList;
struct RList {
   byte Size; long PC, Val; RList Next;
};
RList RHead;

void AddReloc(byte Size, long PC, long Val) {
   RList P = NULL, S = RHead;
   for (; S != NULL; P = S, S = S->Next)
      if (S->PC >= PC) break;
   if (S != NULL && S->PC == PC) Fatal("Internal error (4).");
   RList R = Allocate(sizeof *R);
   if (P == NULL) RHead = R; else P->Next = R;
   R->Next = S, R->Size = Size, R->PC = PC, R->Val = Val;
}

void SetRelocs(FileBuf F) {
   FILE *InF = fopen(F->Name, "rb+");
   if (InF == NULL) Fatal("Cannot open %s.", F->Name);
   FileTab = F->File; fseek(InF, F->Loc, SEEK_SET);
   ExpBuf = Allocate((unsigned)F->Exps*sizeof *ExpBuf);
   ExpInit();
   for (long S = 0; S < F->Exps; S++) {
      Exp *EP = ExpBuf + S;
      StartLine = GetW(InF), StartF = GetW(InF);
      byte Tag = GetB(InF);
      switch (Tag) {
         case NumX:
            Value = GetW(InF); *EP = MakeExp(NumX, Value);
         break;
         case AddrX: {
            word U = GetW(InF), Offset = GetW(InF);
            *EP = MakeExp(AddrX, U < TYPES? &USeg[U]: &F->Seg[U - TYPES], Offset);
         }
         break;
         case SymX: *EP = MakeExp(SymX, F->Sym[GetW(InF)]); break;
         case UnX: {
            byte Op = GetB(InF);
            Exp A = ExpBuf[GetW(InF)];
            *EP = MakeExp(UnX, Op, A);
         }
         break;
         case BinX: {
            byte Op = GetB(InF);
            Exp A = ExpBuf[GetW(InF)], B = ExpBuf[GetW(InF)];
            *EP = MakeExp(BinX, Op, A, B);
         }
         break;
         case CondX: {
            Exp A = ExpBuf[GetW(InF)], B = ExpBuf[GetW(InF)], C = ExpBuf[GetW(InF)];
            *EP = MakeExp(CondX, A, B, C);
         }
         break;
      }
   }
   for (long S = 0; S < F->Relocs; S++) {
      struct Item IBuf;
      IBuf.Line = GetW(InF), IBuf.File = GetW(InF), IBuf.Tag = GetB(InF), IBuf.E = ExpBuf[GetW(InF)];
      word U = GetW(InF);
      IBuf.Seg = U < TYPES? &USeg[U]: &F->Seg[U - TYPES];
      IBuf.Offset = GetW(InF);
      Resolve(&IBuf);
      word PC = (long)IBuf.Seg->Base + (long)IBuf.Offset;
      switch (IBuf.Tag) {
         case 'b': AddReloc(1, PC, LVal); break;
         case 'w': AddReloc(2, PC, LVal); break;
         default: Fatal("Internal error (5).");
      }
   }
   fclose(InF);
   for (Exp E = ExpHead; E != NULL; ) {
      Exp NextE = E->Next; free(E), E = NextE;
   }
   free(ExpBuf);
}

// Hex Output Routines.
static FILE *HexF = NULL;
byte HexBuf[0x10]; unsigned HexX;
unsigned HexAddr;
void OpenHex(char *Hex) {
   HexF = fopen(Hex, "w");
   if (HexF == NULL) Fatal("Cannot open output file.");
   HexX = 0;
}
void EndHex(void) {
   if (HexX > 0) {
      int Sum = (HexAddr&0xff) + ((HexAddr >> 8)&0xff) + HexX;
      fprintf(HexF, ":%02X%04X00", HexX, HexAddr);
      for (unsigned H = 0; H < HexX; H++)
         fprintf(HexF, "%02X", HexBuf[H]), Sum += HexBuf[H];
      fprintf(HexF, "%02X\n", (unsigned)((-Sum)&0xff));
      HexX = 0;
   }
}
void PutHex(long Addr, byte Hex) {
   if (HexX == 0) HexAddr = Addr;
   HexBuf[HexX++] = Hex;
   if (HexX == 0x10) {
      int Sum = (HexAddr&0xff) + ((HexAddr >> 8)&0xff) + HexX;
      fprintf(HexF, ":10%04X00", HexAddr);
      for (unsigned H = 0; H < HexX; H++)
         fprintf(HexF, "%02X", HexBuf[H]), Sum += HexBuf[H];
      fprintf(HexF, "%02X\n", (unsigned)((-Sum)&0xff));
      HexX = 0;
   }
}
void CloseHex(void) {
   EndHex();
   fprintf(HexF, ":00000001FF\n");
   fclose(HexF);
}

void GenImage(char *Hex) {
   FILE *InF = NULL; char *File = NULL;
   OpenHex(Hex);
   RList R = RHead;
   for (Image IP = IHead; IP != NULL; IP = IP->Next) {
      if (IP->Obj != File) {
         if (InF != NULL) fclose(InF), InF = NULL;
         InF = fopen(IP->Obj, "rb+");
         if (InF == NULL) Fatal("Cannot open %s.", IP->Obj);
      }
      fseek(InF, IP->Loc, SEEK_SET);
      for (long Addr = IP->Base; Addr < IP->Base + IP->Size; ) {
         if (R != NULL && R->PC < Addr) Fatal("Internal error (6).");
         if (R != NULL && R->PC == Addr) {
            switch (R->Size) {
               case 1: fgetc(InF); PutHex(Addr++, (byte)(R->Val&0xff)); break;
               case 2:
                  fgetc(InF), fgetc(InF);
                  PutHex(Addr++, (byte)((R->Val >> 8)&0xff)),
                  PutHex(Addr++, (byte)(R->Val&0xff));
               break;
               default: Fatal("Internal error (7).");
            }
            R = R->Next;
         } else {
            int Ch = fgetc(InF);
            if (Ch == EOF) Fatal("Internal error (8).");
            PutHex(Addr++, (byte)(Ch&0xff));
         }
      }
      if (IP->Next != NULL && IP->Base + IP->Size < IP->Next->Base) EndHex();
   }
   CloseHex();
}

void Link(char *Hex) {
   Active = true, Phase = 2;
   SymInit(); SetSeg();
   for (int F = 0; F < Fs; F++) GetHead(FTab + F);
   for (Sym = NIL->Next[0]; Sym != NIL; Sym = Sym->Next[0])
      if (!Sym->Defined) Error("Unresolved external: %s.", Sym->Name);
   Check();
   InSeg = 1;
   SetFree();
   for (int F = 0; F < Fs; F++) FitSegs(FTab + F);
   PurgeFree();
   InSeg = 0;
   Check();
   SetImage();
   for (int F = 0; F < Fs; F++) {
      FileBuf FP = FTab + F;
      for (Gap G = FP->G; G < FP->G + FP->Gaps; G++) FitGap(G);
   }
   Check();
   FreeBlocks();
   InSeg = 1; RHead = NULL;
   for (int F = 0; F < Fs; F++) SetRelocs(FTab + F);
   InSeg = 0;
   Check();
   GenImage(Hex);
}
