/* asmmac.c  */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterroutinen des Makroprozessors                                         */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "stringlists.h"
#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"

#include "asmmac.h"


PInputTag FirstInputTag;
POutputTag FirstOutputTag;

#include "as.rsc"

/*=== Praeprozessor =======================================================*/

/*-------------------------------------------------------------------------*/
/* Verwaltung define-Symbole */

	static void FreeDefine(PDefinement P)
{
   free(P->TransFrom);
   free(P->TransTo);
   free(P);
}

	static void EnterDefine(char *Name, char *Definition)
{
   PDefinement Neu;
   Integer z,l;

   if (! ChkSymbName(Name))
    {
     WrXError(1020,Name); return;
    };

   Neu=FirstDefine;
   while (Neu!=NULL)
    {
     if (strcmp(Neu->TransFrom,Name)==0)
      {
       if (PassNo==1) WrXError(1000,Name); return;
      };
     Neu=Neu->Next;
    }

   Neu=(PDefinement) malloc(sizeof(TDefinement));
   Neu->Next=FirstDefine;
   Neu->TransFrom=strdup(Name); if (! CaseSensitive) NLS_UpString(Neu->TransFrom);
   Neu->TransTo=strdup(Definition);
   l=strlen(Name);
   for (z=0; z<256; Neu->Compiled[z++]=l);
   for (z=0; z<l-1; z++) Neu->Compiled[(unsigned int)Neu->TransFrom[z]]=l-(z+1);
   FirstDefine=Neu;
}

	static void RemoveDefine(char *Name_O)
{
   PDefinement Lauf,Del;
   String Name;

   strmaxcpy(Name,Name_O,255);
   if (! CaseSensitive) NLS_UpString(Name);

   Del=NULL;

   if (FirstDefine!=NULL)
    if (strcmp(FirstDefine->TransFrom,Name)==0)
     {
      Del=FirstDefine; FirstDefine=FirstDefine->Next;
     }
    else
     {
      Lauf=FirstDefine;
      while ((Lauf->Next!=NULL) && (strcmp(Lauf->Next->TransFrom,Name)!=0))
       Lauf=Lauf->Next;
      if (Lauf->Next!=NULL)
       {
	Del=Lauf->Next; Lauf->Next=Del->Next;
       }
     }

    if (Del==NULL) WrXError(1010,Name);
    else FreeDefine(Del);
}

	void PrintDefineList(void)
{
   PDefinement Lauf;
   String OneS;

   if (FirstDefine==NULL) return;

   NewPage(ChapDepth,true);
   WrLstLine(ListDefListHead1);
   WrLstLine(ListDefListHead2);
   WrLstLine("");

   Lauf=FirstDefine;
   while (Lauf!=NULL)
    {
     strmaxcpy(OneS,Lauf->TransFrom,255);
     strmaxcat(OneS,Blanks(10-(strlen(Lauf->TransFrom)%10)),255);
     strmaxcat(OneS," = ",255);
     strmaxcat(OneS,Lauf->TransTo,255);
     WrLstLine(OneS);
     Lauf=Lauf->Next;
    }
   WrLstLine("");
}

	void ClearDefineList(void)
{
   PDefinement Temp;

   while (FirstDefine!=NULL)
    {
     Temp=FirstDefine; FirstDefine=FirstDefine->Next;
     FreeDefine(Temp);
    }
}

/*------------------------------------------------------------------------*/
/* Interface */

	void Preprocess(void)
{
   String h,Cmd,Arg;
   char *p;

   strmaxcpy(h,OneLine+1,255);
   p=FirstBlank(h);
   if (p==NULL)
    {
     strmaxcpy(Cmd,h,255); *h='\0';
    }
   else SplitString(h,Cmd,h,p);

   KillPrefBlanks(h); KillPostBlanks(h);

   if (strcasecmp(Cmd,"DEFINE")==0)
    {
     p=FirstBlank(h);
     if (p!=NULL)
      {
       SplitString(h,Arg,h,p); KillPrefBlanks(h);
       EnterDefine(Arg,h);
      }
    }
   else if (strcasecmp(Cmd,"UNDEF")==0) RemoveDefine(h);

   CodeLen=0;
}

	static bool ExpandDefines_NErl(char inp)
{
   return (((inp>='0') && (inp<='9')) || ((inp>='A') && (inp<='Z')) || ((inp>='a') && (inp<='z')));
}

#define t_toupper(ch) ((CaseSensitive) ? (ch) : (toupper(ch)))

	void ExpandDefines(char *Line)
{

   PDefinement Lauf;
   Integer LPos,Diff,p,p2,p3,z,z2,FromLen,ToLen,LineLen;

   Lauf=FirstDefine;
   while (Lauf!=NULL)
    {
     LPos=0; FromLen=strlen(Lauf->TransFrom); ToLen=strlen(Lauf->TransTo);
     Diff=ToLen-FromLen;
     do
      {
       /* Stelle, ab der verbatim, suchen -->p */
       p=LPos;
       while ((p<strlen(Line)) && (Line[p]!='\'') && (Line[p]!='"')) p++;
       /* nach Quellstring suchen, ersetzen, bis keine Treffer mehr */
       p2=LPos;
       do
        {
         z2=0;
         while ((z2>=0) && (p2<=p-FromLen))
          {
           z2=FromLen-1; z=p2+z2;
           while ((z2>=0) && (t_toupper(Line[z])==Lauf->TransFrom[z2]))
            {
             z2--; z--;
            }
           if (z2>=0) p2+=Lauf->Compiled[(unsigned int)t_toupper(Line[z])];
          }
         if (z2==-1)
          {
           if (((p2==0) || (! ExpandDefines_NErl(Line[p2-1])))
           && ((p2+FromLen==p) || (! ExpandDefines_NErl(Line[p2+FromLen]))))
            {
             if (Diff!=0)
              memmove(Line+p2+ToLen,Line+p2+FromLen,strlen(Line)-p2-FromLen+1);
             memcpy(Line+p2,Lauf->TransTo,ToLen);
             p+=Diff; /* !!! */
             p2+=ToLen;
            }
           else p2+=FromLen;
          }
        }
       while (z2==-1);
       /* Endposition verbatim suchen */
       p3=p+1; LineLen=strlen(Line);
       while ((p3<LineLen) && (Line[p3]!=Line[p])) p3++;
       /* Zaehler entsprechend herauf */
       LPos=p3+1;
      }
     while (LPos<=LineLen-FromLen);
     Lauf=Lauf->Next;
    }
}

/*=== Makrolistenverwaltung ===============================================*/

typedef struct _TMacroNode
         {
	  struct _TMacroNode *Left,*Right; /* Soehne im Baum */
          ShortInt Balance;
	  LongInt DefSection;              /* Gueltigkeitssektion */
	  bool Defined;
	  PMacroRec Contents;
         } TMacroNode,*PMacroNode;

static PMacroNode MacroRoot;

        static bool AddMacro_AddNode(PMacroNode *Node, PMacroRec Neu,
                                        LongInt DefSect, bool Protest)
{
   bool Grown;
   PMacroNode p1,p2;
   bool Result;

   ChkStack();


   if (*Node==NULL)
    {
     *Node=(PMacroNode) malloc(sizeof(TMacroNode));
     (*Node)->Left=NULL; (*Node)->Right=NULL;
     (*Node)->Balance=0; (*Node)->Defined=true;
     (*Node)->DefSection=DefSect; (*Node)->Contents=Neu;
     return true;
    }
   else Result=false;

   switch (StrCmp(Neu->Name,(*Node)->Contents->Name,DefSect,(*Node)->DefSection))
    {
     case 1:
      Grown=AddMacro_AddNode(&((*Node)->Right),Neu,DefSect,Protest);
      if ((BalanceTree) && (Grown))
       switch ((*Node)->Balance)
        {
         case -1:
          (*Node)->Balance=0;
          break;
         case 0:
          (*Node)->Balance=1; Result=true;
          break;
         case 1:
          p1=(*Node)->Right;
          if (p1->Balance==1)
           {
            (*Node)->Right=p1->Left; p1->Left=(*Node);
            (*Node)->Balance=0; *Node=p1;
           }
          else
           {
            p2=p1->Left;
            p1->Left=p2->Right; p2->Right=p1;
            (*Node)->Right=p2->Left; p2->Left=(*Node);
            if (p2->Balance== 1) (*Node)->Balance=(-1); else (*Node)->Balance=0;
            if (p2->Balance==-1) p1     ->Balance=   1; else p1     ->Balance=0;
            *Node=p2;
           }
          (*Node)->Balance=0;
          break;
        }
      break;
     case -1:
      Grown=AddMacro_AddNode(&((*Node)->Left),Neu,DefSect,Protest);
      if ((BalanceTree) && (Grown))
       switch ((*Node)->Balance)
        {
         case 1:
          (*Node)->Balance=0;
          break;
         case 0:
          (*Node)->Balance=(-1); Result=true;
          break;
         case -1:
          p1=(*Node)->Left;
          if (p1->Balance==-1)
           {
            (*Node)->Left=p1->Right; p1->Right=(*Node);
            (*Node)->Balance=0; *Node=p1;
           }
          else
           {
            p2=p1->Right;
            p1->Right=p2->Left; p2->Left=p1;
            (*Node)->Left=p2->Right; p2->Right=(*Node);
            if (p2->Balance==-1) (*Node)->Balance=   1; else (*Node)->Balance=0;
            if (p2->Balance== 1) p1     ->Balance=(-1); else p1     ->Balance=0;
            *Node=p2;
           }
          (*Node)->Balance=0;
          break;
        }
      break;
     case 0:
      if ((*Node)->Defined)
       if (Protest) WrXError(1815,Neu->Name);
       else
        {
	 ClearMacroRec(&((*Node)->Contents)); (*Node)->Contents=Neu;
	 (*Node)->DefSection=DefSect;
        }
      else
       {
        ClearMacroRec(&((*Node)->Contents)); (*Node)->Contents=Neu;
        (*Node)->DefSection=DefSect; (*Node)->Defined=true;
       }
    }

   return Result;
}

	void AddMacro(PMacroRec Neu, LongInt DefSect, bool Protest)
{
   if (! CaseSensitive) NLS_UpString(Neu->Name);
   AddMacro_AddNode(&MacroRoot,Neu,DefSect,Protest);
}

	static bool FoundMacro_FNode(LongInt Handle,PMacroRec *Erg, char *Part)
{
   PMacroNode Lauf;
   ShortInt CErg;

   Lauf=MacroRoot; CErg=2;
   while ((Lauf!=NULL) && (CErg!=0))
    {
     switch (CErg=StrCmp(Part,Lauf->Contents->Name,Handle,Lauf->DefSection))
      {
       case -1: Lauf=Lauf->Left; break;
       case  1: Lauf=Lauf->Right; break;
      }
    }
   if (Lauf!=NULL) *Erg=Lauf->Contents;
   return (Lauf!=NULL);
}

	bool FoundMacro(PMacroRec *Erg)
{
   PSaveSection Lauf;
   String Part;

   strmaxcpy(Part,LOpPart,255); if (! CaseSensitive) NLS_UpString(Part);

   if (FoundMacro_FNode(MomSectionHandle,Erg,Part)) return true;
   Lauf=SectionStack;
   while (Lauf!=NULL)
    {
     if (FoundMacro_FNode(Lauf->Handle,Erg,Part)) return true;
     Lauf=Lauf->Next;
    }
   return false;
}

	static void ClearMacroList_ClearNode(PMacroNode *Node)
{
   ChkStack();

   if (*Node==NULL) return;

   ClearMacroList_ClearNode(&((*Node)->Left));
   ClearMacroList_ClearNode(&((*Node)->Right));

   ClearMacroRec(&((*Node)->Contents)); free(*Node); *Node=NULL;
}

	void ClearMacroList(void)
{
   ClearMacroList_ClearNode(&MacroRoot);
}

	static void ResetMacroDefines_ResetNode(PMacroNode Node)
{
   ChkStack();

   if (Node==NULL) return;

   ResetMacroDefines_ResetNode(Node->Left);
   ResetMacroDefines_ResetNode(Node->Right);
   Node->Defined=false;
}

	void ResetMacroDefines(void)
{
   ResetMacroDefines_ResetNode(MacroRoot);
}

	void ClearMacroRec(PMacroRec *Alt)
{
   free((*Alt)->Name);
   ClearStringList(&((*Alt)->FirstLine));
   free(*Alt); *Alt=NULL;
}

        static void PrintMacroList_PNode(PMacroNode Node, LongInt *Sum, bool *cnt, char *OneS)
{
   String h;

   strmaxcpy(h,Node->Contents->Name,255);
   if (Node->DefSection!=-1)
    {
     strmaxcat(h,"[",255);
     strmaxcat(h,GetSectionName(Node->DefSection),255);
     strmaxcat(h,"]",255);
    }
   strmaxcat(OneS,h,255);
   if (strlen(h)<37) strmaxcat(OneS,Blanks(37-strlen(h)),255);
   if (! (*cnt)) strmaxcat(OneS," | ",255);
   else
    {
     WrLstLine(OneS); OneS[0]='\0';
    }
   *cnt=! (*cnt); (*Sum)++;
}

	static void PrintMacroList_PrintNode(PMacroNode Node, LongInt *Sum, bool *cnt, char *OneS)
{
   if (Node==NULL) return;
   ChkStack();

   PrintMacroList_PrintNode(Node->Left,Sum,cnt,OneS);

   PrintMacroList_PNode(Node,Sum,cnt,OneS);

   PrintMacroList_PrintNode(Node->Right,Sum,cnt,OneS);
}

	void PrintMacroList(void)
{
   String OneS;
   bool cnt;
   LongInt Sum;

   if (MacroRoot==NULL) return;

   NewPage(ChapDepth,true);
   WrLstLine(ListMacListHead1);
   WrLstLine(ListMacListHead2);
   WrLstLine("");

   OneS[0]='\0'; cnt=false; Sum=0;
   PrintMacroList_PrintNode(MacroRoot,&Sum,&cnt,OneS);
   if (cnt)
    {
     OneS[strlen(OneS)-1]='\0';
     WrLstLine(OneS);
    }
   WrLstLine("");
   sprintf(OneS,"%7d%s",Sum,(Sum==1)?ListMacSumMsg:ListMacSumsMsg);
   WrLstLine(OneS);
   WrLstLine("");
}

/*=== Eingabefilter Makroprozessor ========================================*/


	void asmmac_init(void)
{
   MacroRoot=NULL;
}
