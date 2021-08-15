/* lang_DE/bind.rsc */
/****************************************************************************/
/* Makroassembler AS 							    */
/* 									    */
/* Headerdatei BIND.RSC - enth�lt Stringdefinitionen f�r BIND               */
/* 									    */
/* Historie : 28.1.1997 Grundsteinlegung                                    */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/* Fehlermeldungen */

#define ErrMsgTargetMissing   "Zieldateiangabe fehlt!"

/****************************************************************************/
/* Ansagen */

#define InfoMessHead2         " <Quelldatei(en)> <Zieldatei> [Optionen]"
#define InfoMessHelpCnt       2
static char *InfoMessHelp[InfoMessHelpCnt]=
                  {"",
		   "Optionen: -f <Headerliste>  :  auszufilternde Records"};
