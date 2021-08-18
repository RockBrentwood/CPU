// AS-Portierung
// Befehle zur bedingten Assemblierung
typedef struct _TIfSave {
   struct _TIfSave *Next;
   Integer NestLevel;
   bool SaveIfAsm;
   TempResult SaveExpr;
   enum { IfState_IFIF, IfState_IFELSE,
      IfState_CASESWITCH, IfState_CASECASE, IfState_CASEELSE
   } State;
   bool CaseFound;
} TIfSave, *PIfSave;

extern bool IfAsm;
extern PIfSave FirstIfSave;

bool CodeIFs(void);
void AsmIFInit(void);
Integer SaveIFs(void);
void RestoreIFs(Integer Level);
bool IFListMask(void);
void asmif_init(void);
