/* codeallg.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* von allen Codegeneratoren benutzte Pseudobefehle                          */
/*                                                                           */
/* Historie:  10. 5.1996 Grundsteinlegung                                    */
/*                                                                           */
/*****************************************************************************/

extern void SetCPU(CPUVar NewCPU, bool NotPrev);

extern bool CodeGlobalPseudo(void);

extern void codeallg_init(void);
