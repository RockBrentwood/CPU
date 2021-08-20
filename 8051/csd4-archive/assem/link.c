#include <stdlib.h>
#include <stdio.h>
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

static Image FormImage(
   word Base, long Size, char *Src, char *Obj, word Line, long Loc
) {
   Image IP = (Image)Allocate(sizeof *IHead);
   IP->Base = Base, IP->Size = Size, IP->Src = Src, IP->Obj = Obj,
   IP->Line = Line, IP->Loc = Loc;
   return IP;
}

static void SetImage(void) {
   Block BP;
   for (IHead = ITail = 0, BP = BHead[0]; BP != 0; BP = BP->Next) {
      Segment Seg = BP->Seg; Image P, IP, NewI;
      for (IP = IHead; IP != 0; IP = IP->Next)
         if (IP->Base >= Seg->Base) break;
      P = (IP == 0)? ITail: IP->Prev;
      if (P != 0 && P->Base + P->Size > Seg->Base)
         FATAL("Overlapping segments at [%d] %s and [%d] %s.",
            P->Line, P->Src, Seg->Line, BP->Src
         );
      if (IP != 0 && Seg->Base + Seg->Size > IP->Base)
         FATAL("Overlapping segments at [%d] %s and [%d] %s.",
            Seg->Line, BP->Src, IP->Line, IP->Src
         );
      NewI = FormImage(
         Seg->Base, Seg->Size, BP->Src, BP->Obj, Seg->Line, Seg->Loc
      );
      if (P == 0) IHead = NewI; else P->Next = NewI;
      if (IP == 0) ITail = NewI; else IP->Prev = NewI;
      NewI->Next = IP, NewI->Prev = P;
   }
}

static void FitGap(Gap G) {
   Image P, IP, NewI; long End, EndP;
   word Base = G->Seg->Base + G->Offset; long Size = G->Size;
   for (IP = IHead; IP != 0; IP = IP->Next)
      if (IP->Base > Base) break;
   P = (IP == 0)? ITail: IP->Prev;
   if (P == 0) FATAL("Internal error (2).");
   End = Base + Size, EndP = P->Base + P->Size;
   if (EndP < End) FATAL("Internal error (3).");
   if (P->Base < Base) {
      P->Size = Base - P->Base;
      if (EndP > End) {
         NewI = FormImage(
            (word)End, EndP - End, P->Src, P->Obj, P->Line, P->Loc + P->Size
         );
         NewI->Next = IP, P->Next = NewI;
         NewI->Prev = P;
         if (IP == 0) ITail = NewI; else IP->Prev = NewI;
      }
   } else if (EndP > End) P->Base += Size, P->Size -= Size;
   else {
      if (P->Prev == 0) IHead = P->Next; else P->Prev->Next = P->Next;
      if (P->Next == 0) ITail = P->Prev; else P->Next->Prev = P->Prev;
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
   int T;
   FHead = (Free *)Allocate(SEG_TYPES * sizeof *FHead);
   FSize = (int *)Allocate(SEG_TYPES * sizeof *FSize);
   for (T = 0; T < SEG_TYPES; T++) {
      Block P, B; Free F; long Base;
      FHead[T] = (Free)Allocate((BSize[T] + 1) * sizeof *FHead[T]);
      for (P = 0, B = BHead[T], F = FHead[T]; B != 0; P = B, B = B->Next) {
         Base = (P == 0)? AddrTab[T].Lo: (long) P->Seg->Base + P->Seg->Size;
         if (Base >= B->Seg->Base) continue;
         F->Base = Base, F->Size = (long)B->Seg->Base - Base, F->BP = P, F++;
      }
      Base = (P == 0)? AddrTab[T].Lo: (long) P->Seg->Base + P->Seg->Size;
      if (Base <= AddrTab[T].Hi)
         F->Base = Base, F->Size = (long)AddrTab[T].Hi + 1 - Base,
         F->BP = P, F++;
      FSize[T] = F - FHead[T];
   }
}

void PurgeFree(void) {
   int T;
   for (T = 0; T < SEG_TYPES; T++) free(FHead[T]);
   free(FHead), free(FSize);
}

void BestFit(char *Obj, Segment Seg) {
   byte T = Seg->Type; Free Best, FP; Block B;
   if (!Seg->Rel) return;
   StartLine = Seg->Line, StartF = Seg->File;
   for (Best = 0, FP = FHead[T]; FP < FHead[T] + FSize[T]; FP++) {
      if (
         Seg->Size <= FP->Size && (Best == 0 || FP->Size < Best->Size)
      ) Best = FP;
   }
   if (Best == 0) FATAL("Cannot fit segment.");
   B = (Block)Allocate(sizeof *B);
   B->Seg = Seg, B->Src = FileTab[StartF], B->Obj = Obj;
   if (Best->BP == 0)
      B->Next = BHead[T], BHead[T] = B;
   else
      B->Next = Best->BP->Next, Best->BP->Next = B;
   Seg->Rel = 0, Seg->Base = Best->Base,
   Best->BP = B, Best->Base += Seg->Size, Best->Size -= Seg->Size;
}

void FitSegs(FileBuf F) {
   int S; Segment Seg;
   FileTab = F->File;
   for (S = TYPES; S < F->Segs; S++) {
      Seg = F->Seg + S - TYPES, BestFit(F->Name, Seg);
   }
}

void SetSeg(void) {
   Segment S; int I;
   USeg = (Segment)Allocate(TYPES * sizeof *USeg);
   for (S = USeg; S < USeg + TYPES; S++)
      S->Line = S->File = 0, S->Rel = 0, S->Type = S - USeg,
      S->Base = 0, S->Size = 0, S->Loc = 0L;
   BHead = (Block *)Allocate(SEG_TYPES * sizeof *BHead);
   BSize = (int *)Allocate(SEG_TYPES * sizeof *BSize);
   for (I = 0; I < SEG_TYPES; I++) BHead[I] = 0, BSize[I] = 0;
}

void FreeBlocks(void) {
   int I;
   for (I = 0; I < SEG_TYPES; I++) {
      Block B, N;
      for (B = BHead[I]; B != 0; B = N) N = B->Next, free(B);
   }
   free(BHead), free(BSize);
}

void Fit(char *Obj, Segment Seg) {
   byte T = Seg->Type; Block Cur, Prev, B;
   if (Seg->Rel) return;
   for (Prev = 0, Cur = BHead[T]; Cur != 0; Prev = Cur, Cur = Cur->Next)
      if (Cur->Seg->Base >= Seg->Base) break;
   if (Prev != 0 && Prev->Seg->Base + Prev->Seg->Size > Seg->Base)
      ERROR("Overlapping segments: %s [%d] and %s [%d].",
         FileTab[Seg->File], Seg->Line, Prev->Src, Prev->Seg->Line
      );
   if (Cur != 0 && Cur->Seg->Base < Seg->Base + Seg->Size)
      ERROR("Overlapping segments: %s [%d] and %s [%d].",
         FileTab[Seg->File], Seg->Line, Cur->Src, Cur->Seg->Line
      );
   B = (Block)Allocate(sizeof *B);
   B->Src = FileTab[Seg->File], B->Obj = Obj, B->Seg = Seg, B->Next = Cur;
   if (Prev == 0) BHead[T] = B; else Prev->Next = B;
   BSize[T]++;
}

void GetHead(FileBuf F) {
   word MAGIC; long S; byte Buf[0x100];
   FILE *FP = fopen(F->Name, "rb+");
   if (FP == 0) FATAL("Cannot open %s.", F->Name);
   MAGIC = GetW(FP);
   if (MAGIC != 0x55aa) FATAL("Invalid object file %s.", F->Name);
   F->Loc = GetL(FP),
   F->Files = GetL(FP), F->Segs = GetL(FP),
   F->Gaps = GetL(FP), F->Syms = GetL(FP),
   F->Exps = GetL(FP), F->Relocs = GetL(FP);
   S = F->Loc + F->Files + F->Segs + F->Gaps + F->Syms + F->Exps + F->Relocs;
   if (GetL(FP) != S&0xffffffff) FATAL("Corrupt object file %s.", F->Name);
   F->File = (char **)Allocate((word)F->Files * sizeof *F->File);
   F->Seg = (F->Segs < TYPES)? 0: (Segment)Allocate((word)(F->Segs - TYPES) * sizeof *F->Seg);
   F->G = (Gap)Allocate((word)F->Gaps * sizeof *F->G);
   F->Sym = (Symbol *)Allocate((word)F->Syms * sizeof *F->Sym);
   FileTab = F->File;
   fseek(FP, F->Loc, SEEK_SET);
   for (S = 0; S < F->Files; S++) {
      word L;
      L = GetW(FP), fread(Buf, 1, L, FP), Buf[L] = '\0';
      F->File[S] = CopyS(Buf);
   }
   for (S = TYPES; S < F->Segs; S++) {
      Segment Seg = F->Seg + S - TYPES; word U;
      Seg->Line = GetW(FP), Seg->File = GetW(FP),
      U = GetW(FP), Seg->Size = GetW(FP),
      Seg->Base = GetW(FP), Seg->Loc = GetL(FP);
      Seg->Rel = (U >> 8)&1, Seg->Type = U&0xff;
      Fit(F->Name, Seg);
   }
   for (S = 0; S < F->Gaps; S++) {
      Gap G = F->G + S; word Sg;
      Sg = GetW(FP), G->Offset = GetW(FP), G->Size = GetW(FP);
      G->Seg = Sg < TYPES? &USeg[Sg]: &F->Seg[Sg - TYPES];
   }
   for (S = 0; S < F->Syms; S++) {
      Symbol Sym; byte B, Defined, Global, Address; word U, L, Offset;
      B = GetB(FP), U = GetW(FP), Offset = GetW(FP),
      L = GetW(FP);
      if (L > 0) fread(Buf, 1, L, FP);
      Buf[L] = '\0';
      Global = (B&8)? 1: 0, Defined = (B&4)? 1: 0, Address = (B&2)? 1: 0;
      if (!Global) {
         Sym = (Symbol)Allocate(sizeof *Sym);
         Sym->Variable = 0, Sym->Address = Address;
         Sym->Global = Global, Sym->Defined = Defined;
         Sym->Seg = !Address? 0: U < TYPES? &USeg[U]: &F->Seg[U - TYPES];
         Sym->Offset = Offset;
         Sym->Index = F - FTab;
         Sym->Name = CopyS(Buf);
      } else {
         Sym = LookUp(Buf);
         if (Sym->Defined && Defined)
            ERROR("Symbol %s redefined: %s %s.",
               Sym->Name, FTab[Sym->Index].Name, F->Name
            );
         else {
            if (Sym->Address) {
               Segment Seg = U < TYPES? &USeg[U]: &F->Seg[U - TYPES];
               if (!Address)
                  ERROR("Address %s redeclared as number: %s %s",
                     Sym->Name, FTab[Sym->Index].Name, F->Name
                  );
               else if (Sym->Seg->Type != Seg->Type)
                  ERROR("Type mismatch %s: %s %s",
                     Sym->Name, FTab[Sym->Index].Name, F->Name
                  );
            } else if (Address && Sym->Global)
               ERROR("Number %s redeclared as address: %s %s",
                  Sym->Name, FTab[Sym->Index].Name, F->Name
               );
            if (Defined || !Sym->Global) {
               Sym->Variable = 0, Sym->Address = Address;
               Sym->Global = Global, Sym->Defined = Defined;
               Sym->Seg = !Address? 0: U < TYPES? &USeg[U]: &F->Seg[U - TYPES],
               Sym->Offset = Offset;
               Sym->Index = F - FTab;
            }
         }
      }
      F->Sym[S] = Sym;
   }
   F->Loc = ftell(FP);
   fclose(FP);
}

typedef struct RList *RList;
struct RList {
   byte Size; long PC, Val; RList Next;
};
RList RHead;

void AddReloc(byte Size, long PC, long Val) {
   RList R = Allocate(sizeof *R), P, S;
   for (P = 0, S = RHead; S != 0; P = S, S = S->Next)
      if (S->PC >= PC) break;
   if (S != 0 && S->PC == PC) FATAL("Internal error (4)."); 
   if (P == 0) RHead = R; else P->Next = R;
   R->Next = S,
   R->Size = Size, R->PC = PC, R->Val = Val;
}

void SetRelocs(FileBuf F) {
   long S; struct Item IBuf; Exp E, NextE;
   FILE *FP = fopen(F->Name, "rb+");
   if (FP == 0) FATAL("Cannot open %s.", F->Name);
   FileTab = F->File; fseek(FP, F->Loc, SEEK_SET);
   ExpBuf = (Exp *)Allocate((unsigned)F->Exps * sizeof *ExpBuf);
   ExpInit();
   for (S = 0; S < F->Exps; S++) {
      Exp *EP = ExpBuf + S; byte Tag; Segment Seg; Exp A, B, C;
      word Line, File, U, Offset; byte Op;
      StartLine = GetW(FP), StartF = GetW(FP), Tag = GetB(FP);
      switch (Tag) {
         case NumX:
            Value = GetW(FP); *EP = MakeExp(NumX, Value);
         break;
         case AddrX:
            U = GetW(FP), Offset = GetW(FP);
            Seg = U < TYPES? &USeg[U]: &F->Seg[U - TYPES];
            *EP = MakeExp(AddrX, Seg, Offset);
         break;
         case SymX:
            U = GetW(FP); *EP = MakeExp(SymX, F->Sym[U]);
         break;
         case UnX:
            Op = GetB(FP),
            U = GetW(FP), A = ExpBuf[U],
            *EP = MakeExp(UnX, Op, A);
         break;
         case BinX:
            Op = GetB(FP),
            U = GetW(FP), A = ExpBuf[U],
            U = GetW(FP), B = ExpBuf[U],
            *EP = MakeExp(BinX, Op, A, B);
         break;
         case CondX:
            U = GetW(FP), A = ExpBuf[U],
            U = GetW(FP), B = ExpBuf[U],
            U = GetW(FP), C = ExpBuf[U],
            *EP = MakeExp(CondX, A, B, C);
         break;
      }
   }
   for (S = 0; S < F->Relocs; S++) {
      word U, PC;
      IBuf.Line = GetW(FP), IBuf.File = GetW(FP),
      IBuf.Tag = GetB(FP),
      U = GetW(FP), IBuf.E = ExpBuf[U],
      U = GetW(FP), IBuf.Seg = U < TYPES? &USeg[U]: &F->Seg[U - TYPES];
      IBuf.Offset = GetW(FP);
      Resolve(&IBuf);
      PC = (long)IBuf.Seg->Base + (long)IBuf.Offset;
      switch (IBuf.Tag) {
         case 'b': AddReloc(1, PC, LVal); break;
         case 'w': AddReloc(2, PC, LVal); break;
         default: FATAL("Internal error (5).");
      }
   }
   fclose(FP);
   for (E = ExpHead; E != 0; E = NextE) NextE = E->Next, free(E);
   free(ExpBuf);
}

/* HEX OUTPUT ROUTINES */
static FILE *HexF = 0;
byte HexBuf[0x10]; int HexX;
long HexAddr;
void OpenHex(char *Hex) {
   HexF = fopen(Hex, "w");
   if (HexF == 0) FATAL("Cannot open output file.");
   HexX = 0;
}
void EndHex(void) {
   if (HexX > 0) {
      int H, Sum;
      Sum = (HexAddr&0xff) + ((HexAddr >> 8)&0xff) + HexX;
      fprintf(HexF, ":%02X%04X00", HexX, HexAddr);
      for (H = 0; H < HexX; H++)
         fprintf(HexF, "%02X", HexBuf[H]), Sum += HexBuf[H];
      fprintf(HexF, "%02X\n", (-Sum)&0xff);
      HexX = 0;
   }
}
void PutHex(long Addr, byte Hex) {
   if (HexX == 0) HexAddr = Addr;
   HexBuf[HexX++] = Hex;
   if (HexX == 0x10) {
      int H, Sum;
      Sum = (HexAddr&0xff) + ((HexAddr >> 8)&0xff) + HexX;
      fprintf(HexF, ":10%04X00", HexAddr);
      for (H = 0; H < HexX; H++)
         fprintf(HexF, "%02X", HexBuf[H]), Sum += HexBuf[H];
      fprintf(HexF, "%02X\n", (-Sum)&0xff);
      HexX = 0;
   }
}
void CloseHex(void) {
   EndHex();
   fprintf(HexF, ":00000001FF\n");
   fclose(HexF);
}

void GenImage(char *Hex) {
   Image IP; RList R; FILE *FP; char *File;
   FP = 0, File = 0;
   OpenHex(Hex);
   for (IP = IHead, R = RHead; IP != 0; IP = IP->Next) {
      long Addr;
      if (IP->Obj != File) {
         if (FP != 0) fclose(FP), FP = 0;
         FP = fopen(IP->Obj, "rb+");
         if (FP == 0) FATAL("Cannot open %s.", IP->Obj);
      }
      fseek(FP, IP->Loc, SEEK_SET);
      for (Addr = IP->Base; Addr < IP->Base + IP->Size; ) {
         if (R != 0 && R->PC < Addr) FATAL("Internal error (6).");
         if (R != 0 && R->PC == Addr) {
            switch (R->Size) {
               case 1: fgetc(FP); PutHex(Addr++, (byte)(R->Val&0xff)); break;
               case 2:
                  fgetc(FP), fgetc(FP);
                  PutHex(Addr++, (byte)((R->Val >> 8)&0xff)),
                  PutHex(Addr++, (byte)(R->Val&0xff));
               break;
               default: FATAL("Internal error (7).");
            }
            R = R->Next;
         } else {
            int Ch = fgetc(FP);
            if (Ch == EOF) FATAL("Internal error (8).");
            PutHex(Addr++, (byte)(Ch&0xff));
         }
      }
      if (IP->Next != 0 && IP->Base + IP->Size < IP->Next->Base) EndHex();
   }
   CloseHex();
}

void Link(char *Hex) {
   int A;
   Active = 1, Phase = 2;
   SymInit(); SetSeg();
   for (A = 0; A < Fs; A++) GetHead(FTab + A);
   for (Sym = NIL->Next[0]; Sym != NIL; Sym = Sym->Next[0])
      if (!Sym->Defined) ERROR("Unresolved external: %s.", Sym->Name);
   CHECK();
   InSeg = 1;
   SetFree();
   for (A = 0; A < Fs; A++) FitSegs(FTab + A);
   PurgeFree();
   InSeg = 0;
   CHECK();
   SetImage();
   for (A = 0; A < Fs; A++) {
      Gap G; FileBuf F = FTab + A;
      for (G = F->G; G < F->G + F->Gaps; G++) FitGap(G);
   }
   CHECK();
   FreeBlocks();
   InSeg = 1; RHead = 0;
   for (A = 0; A < Fs; A++) SetRelocs(FTab + A);
   InSeg = 0;
   CHECK();
   GenImage(Hex);
}
