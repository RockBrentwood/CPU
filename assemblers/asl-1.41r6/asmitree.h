// AS-Portierung
// Opcode-Abfrage als Binaerbaum
typedef void (*InstProc)(Word Index);
typedef struct _TInstTreeNode {
   struct _TInstTreeNode *Left, *Right;
   InstProc Proc;
   char *Name;
   Word Index;
   ShortInt Balance;
} TInstTreeNode, *PInstTreeNode;

void AddInstTree(PInstTreeNode * Root, char *NName, InstProc NProc, Word NIndex);
void ClearInstTree(PInstTreeNode * Root);
bool SearchInstTree(PInstTreeNode Root);
void PrintInstTree(PInstTreeNode Root);
void asmitree_init(void);
