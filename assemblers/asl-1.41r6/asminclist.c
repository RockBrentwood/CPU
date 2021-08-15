/* asminclist.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung der Include-Verschachtelungsliste                              */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "stringutil.h"
#include "chunks.h"
#include "asmfnums.h"
#include "asmdef.h"
#include "asmsub.h"

#include "asminclist.h"

#include "as.rsc"


typedef void **PFileArray;
typedef struct _TFileNode
         {
          Integer Name;
          Integer Len;
          struct _TFileNode *Parent;
          PFileArray Subs;
         } TFileNode,*PFileNode;

static PFileNode Root,Curr;


  	void PushInclude(char *S)
{
   PFileNode Neu;

   Neu=(PFileNode) malloc(sizeof(TFileNode));
   Neu->Name=GetFileNum(S);
   Neu->Len=0; Neu->Subs=NULL;
   Neu->Parent=Curr;
   if (Root==NULL) Root=Neu;
   if (Curr==NULL) Curr=Neu;
   else
    {
     if (Curr->Len==0)
      Curr->Subs=(PFileArray) malloc(sizeof(void *));
     else
      Curr->Subs=(PFileArray) realloc(Curr->Subs,sizeof(void *)*(Curr->Len+1));
     Curr->Subs[Curr->Len++]=Neu;
     Curr=Neu;
    }
}


	void PopInclude(void)
{
   if (Curr!=NULL) Curr=Curr->Parent;
}


        static void PrintIncludeList_PrintNode(PFileNode Node, Integer Indent)
{
   Integer z;
   String h;

   ChkStack();

   if (Node!=NULL)
    {
     strmaxcpy(h,Blanks(Indent),255);
     strmaxcat(h,GetFileName(Node->Name),255);
     WrLstLine(h);
     for (z=0; z<Node->Len; z++) PrintIncludeList_PrintNode(Node->Subs[z],Indent+5);
    }
}

	void PrintIncludeList(void)
{
   NewPage(ChapDepth,true);
   WrLstLine(ListIncludeListHead1);
   WrLstLine(ListIncludeListHead2);
   WrLstLine("");
   PrintIncludeList_PrintNode(Root,0);
}


        static void ClearIncludeList_ClearNode(PFileNode Node)
{
   Integer z;

   ChkStack();

   if (Node!=NULL)
    {
     for (z=0; z<Node->Len; ClearIncludeList_ClearNode(Node->Subs[z++]));
     if (Node->Len>0) free(Node->Subs);
     free(Node);
    }
}

	void ClearIncludeList(void)
{
   ClearIncludeList_ClearNode(Root);
   Curr=NULL; Root=NULL;
}


	void asminclist_init(void)
{
  Root=NULL; Curr=NULL;
}
