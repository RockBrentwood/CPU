#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "io.h"
#include "ex.h"
#include "st.h"
#include "res.h"

#define R_EXTEND 0x10
Item RTab; int RCur;
static int RMax;

long LVal;

void ResInit(void) {
   RTab = NULL, RCur = RMax = 0;
}

void Resolve(Item IP) {
   Exp E = IP->E;
   if (Phase == 1) E = EvalExp(E);
   if (Phase > 0) StartLine = IP->Line, StartF = IP->File;
   byte Code = IP->Tag == 'Q'? 0x11: IP->Tag == 'P'? 0x01: 0;
   Segment Seg; long L;
// TypeCheck
   switch (E->Tag) {
      case AddrX:
         Seg = SegOf(E);
         if (IP->Tag != 'R'? Seg->Rel: (Seg->Rel || IP->Seg->Rel) && IP->Seg != Seg) goto DoMap;
         L = (long)(short)Seg->Base + (long)(short)OffOf(E);
         switch (IP->Tag) {
            case 'B':
               if (Seg->Type != BIT) Error("Address type mismatch.");
            break;
            case 'D':
               if (Seg->Type != DATA && Seg->Type != SFR) Error("Address type mismatch.");
            break;
            case 'R': case 'L': case 'P': case 'Q':
               if (Seg->Type != CODE) Error("Address type mismatch.");
            break;
         }
      break;
      case NumX: L = (long)(short)ValOf(E); break;
      default: goto DoMap;
   }
// BoundsCheck
   switch (IP->Tag) {
      case 'b':
         if (L < -0x80 || L >= 0x100) Error("Byte value out of range (%4lx).", L);
      break;
      case 'w':
         if (L < -0x8000L || L >= 0x10000L) Error("Word value out of range.");
      break;
      case 'B':
         if (L < 0 || L >= 0x100) Error("Bit address out of range.");
      break;
      case 'D':
         if (E->Tag != AddrX) {
            if (L < 0 || L >= 0x100) Error("Data address out of range.");
         } else if (Seg->Type == DATA) {
            if (L < 0 || L >= 0x80) Error("Register address out of range.");
         } else if (Seg->Type == SFR) {
            if (L < 0x80 || L >= 0x100) Error("SFR address out of range.");
         }
      break;
      case 'R':
         L -= (long)(short)IP->Seg->Base + (long)(short)IP->Offset + 1L;
         if (L < -0x80 || L >= 0x80) Error("Relative address out of range.");
      break;
      case 'P': case 'Q': {
         long PC = (long)(short)IP->Seg->Base + (long)(short)IP->Offset + 2L;
         if ((L&0xf800) != (PC&0xf800)) Error("Paged address out of range.");
         L = (L << 5)&0xe000 | (Code << 8)&0x1f00 | L&0xff;
      }
      break;
      case 'L':
         if (L < 0 || L >= 0x10000L) Error("Code address out of range.");
      break;
      default: Fatal("Internal error (0)");
   }
   if (Phase == 1) {
      fseek(ExF, IP->Seg->Loc + IP->Offset, SEEK_SET); IP->Map = false;
   }
Generate:
   switch (IP->Tag) {
      case 'b': case 'B': case 'D': case 'R': {
         byte B = L&0xff;
         switch (Phase) {
            case 0: PByte(B); break;
            case 1: PutB(B, ExF); break;
            case 2: IP->Tag = 'b', LVal = B; break;
         }
      }
      return;
      case 'P': case 'Q': case 'w': case 'L': {
         byte Lo = (L >> 8)&0xff, Hi = L&0xff;
         switch (Phase) {
            case 0: PByte(Lo), PByte(Hi); break;
            case 1: PutB(Lo, ExF), PutB(Hi, ExF); break;
            case 2: IP->Tag = 'w', LVal = L&0xffff; break;
         }
      }
      return;
   }
return;
DoMap:
   switch (Phase) {
      case 0: RCur++; L = 0; goto Generate;
      case 1: IP->Map = true, MarkExp(IP->E = E); break;
      case 2: Fatal("Internal error (1).");
   }
}

void Reloc(byte Code, byte Tag, Exp E) {
   if (RCur == RMax) {
      RMax += R_EXTEND, RTab = realloc(RTab, RMax*sizeof *RTab);
      if (RTab == NULL) Fatal("Out of memory.");
   }
   Item IP = &RTab[RCur];
   IP->Tag = Tag, IP->E = E, IP->Seg = SegP, IP->Offset = CurLoc, IP->Map = false, IP->Line = StartLine, IP->File = StartF;
// ajmp: Tag = 'P', acall: Tag = 'Q'.
   if (IP->Tag == 'P' && (Code&0x10)) IP->Tag = 'Q';
   Resolve(IP);
}
