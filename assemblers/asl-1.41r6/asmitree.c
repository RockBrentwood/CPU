// AS-Portierung
// Opcode-Abfrage als Binaerbaum
#include "stdinc.h"
#include <string.h>

#include "chunks.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"

#include "asmitree.h"

/*---------------------------------------------------------------------------*/

static bool AddSingle(PInstTreeNode * Node, char *NName, InstProc NProc, Word NIndex) {
   PInstTreeNode p1, p2;
   bool Result = false;

   ChkStack();

   if (*Node == NULL) {
      *Node = (PInstTreeNode) malloc(sizeof(TInstTreeNode));
      (*Node)->Left = NULL;
      (*Node)->Right = NULL;
      (*Node)->Proc = NProc;
      (*Node)->Index = NIndex;
      (*Node)->Balance = 0;
      (*Node)->Name = strdup(NName);
      Result = true;
   } else if (strcmp(NName, (*Node)->Name) < 0) {
      if (AddSingle(&((*Node)->Left), NName, NProc, NIndex))
         switch ((*Node)->Balance) {
            case 1:
               (*Node)->Balance = 0;
               break;
            case 0:
               (*Node)->Balance = (-1);
               Result = true;
               break;
            case -1:
               p1 = (*Node)->Left;
               if (p1->Balance == -1) {
                  (*Node)->Left = p1->Right;
                  p1->Right = (*Node);
                  (*Node)->Balance = 0;
                  *Node = p1;
               } else {
                  p2 = p1->Right;
                  p1->Right = p2->Left;
                  p2->Left = p1;
                  (*Node)->Left = p2->Right;
                  p2->Right = (*Node);
                  if (p2->Balance == -1) (*Node)->Balance = 1;
                  else (*Node)->Balance = 0;
                  if (p2->Balance == 1) p1->Balance = (-1);
                  else p1->Balance = 0;
                  *Node = p2;
               }
               (*Node)->Balance = 0;
               break;
         }
   } else {
      if (AddSingle(&((*Node)->Right), NName, NProc, NIndex))
         switch ((*Node)->Balance) {
            case -1:
               (*Node)->Balance = 0;
               break;
            case 0:
               (*Node)->Balance = 1;
               Result = true;
               break;
            case 1:
               p1 = (*Node)->Right;
               if (p1->Balance == 1) {
                  (*Node)->Right = p1->Left;
                  p1->Left = (*Node);
                  (*Node)->Balance = 0;
                  *Node = p1;
               } else {
                  p2 = p1->Left;
                  p1->Left = p2->Right;
                  p2->Right = p1;
                  (*Node)->Right = p2->Left;
                  p2->Left = (*Node);
                  if (p2->Balance == 1) (*Node)->Balance = (-1);
                  else (*Node)->Balance = 0;
                  if (p2->Balance == -1) p1->Balance = 1;
                  else p1->Balance = 0;
                  *Node = p2;
               }
               (*Node)->Balance = 0;
               break;
         }
   }
   return Result;
}

void AddInstTree(PInstTreeNode * Root, char *NName, InstProc NProc, Word NIndex) {
   AddSingle(Root, NName, NProc, NIndex);
}

static void ClearSingle(PInstTreeNode * Node) {
   ChkStack();

   if (*Node != NULL) {
      free((*Node)->Name);
      ClearSingle(&((*Node)->Left));
      ClearSingle(&((*Node)->Right));
      free(*Node);
      *Node = NULL;
   }
}

void ClearInstTree(PInstTreeNode * Root) {
   ClearSingle(Root);
}

bool SearchInstTree(PInstTreeNode Root) {
   Integer z;

   z = 0;
   while ((Root != NULL) && (!Memo(Root->Name))) {
      Root = (strcmp(OpPart, Root->Name) < 0) ? Root->Left : Root->Right;
      z++;
   }

   if (Root == NULL) return false;
   else {
      Root->Proc(Root->Index);
      return true;
   }
}

static char Format[20];

static void PNode(PInstTreeNode Node, Word Lev) {
   ChkStack();
   if (Node != NULL) {
      PNode(Node->Left, Lev + 1);
      sprintf(Format, "%%%ds %%s %%p %%p %%d\n", 5 * Lev);
      printf(Format, "", Node->Name, Node->Left, Node->Right, Node->Balance);
      PNode(Node->Right, Lev + 1);
   }
}

void PrintInstTree(PInstTreeNode Root) {
   PNode(Root, 0);
}

void asmitree_init(void) {
}
