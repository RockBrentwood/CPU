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
	  Boolean SaveIfAsm;
	  TempResult SaveExpr;
	  enum {IfState_IFIF,IfState_IFELSE,
		   IfState_CASESWITCH,IfState_CASECASE,IfState_CASEELSE} State;
	  Boolean CaseFound;
         } TIfSave,*PIfSave;


extern Boolean IfAsm;
extern PIfSave FirstIfSave;


extern Boolean CodeIFs(void);

extern void AsmIFInit(void);

extern Integer SaveIFs(void);

extern void RestoreIFs(Integer Level);

extern Boolean IFListMask(void);

extern void asmif_init(void);
