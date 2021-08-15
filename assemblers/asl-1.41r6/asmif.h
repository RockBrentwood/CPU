/* asmif.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Befehle zur bedingten Assemblierung                                       */
/*                                                                           */
/* Historie: 15. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

typedef struct _TIfSave
         {
	  struct _TIfSave *Next;
	  Integer NestLevel;
	  bool SaveIfAsm;
	  TempResult SaveExpr;
	  enum {IfState_IFIF,IfState_IFELSE,
		   IfState_CASESWITCH,IfState_CASECASE,IfState_CASEELSE} State;
	  bool CaseFound;
         } TIfSave,*PIfSave;


extern bool IfAsm;
extern PIfSave FirstIfSave;


extern bool CodeIFs(void);

extern void AsmIFInit(void);

extern Integer SaveIFs(void);

extern void RestoreIFs(Integer Level);

extern bool IFListMask(void);

extern void asmif_init(void);
