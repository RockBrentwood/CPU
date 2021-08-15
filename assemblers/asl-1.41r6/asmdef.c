/* asmdef.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* global benutzte Variablen                                                 */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "stringlists.h"
#include "chunks.h"

#include "asmdef.h"

char *SrcSuffix = ".asm"; /* Standardendungen: Hauptdatei */
char *IncSuffix = ".inc"; /* Includedatei */
char *PrgSuffix = ".p"; /* Programmdatei */
char *LstSuffix = ".lst"; /* Listingdatei */
char *MacSuffix = ".mac"; /* Makroausgabe */
char *PreSuffix = ".i"; /* Ausgabe Makroprozessor */
char *LogSuffix = ".log"; /* Fehlerdatei */
char *MapSuffix = ".map"; /* Debug-Info/Map-Format */

char *Version = "1.41r5";
LongInt VerNo = 0x1415;

char *EnvName = "ASCMD"; /* Environment-Variable fuer Default-
                            Parameter */

char *SegNames[PCMax + 1] = { "NOTHING", "CODE", "DATA", "IDATA", "XDATA", "YDATA",
   "BITDATA", "IO", "REG"
};
char SegShorts[PCMax + 1] = { '-', 'C', 'D', 'I', 'X', 'Y', 'B', 'P', 'R' };

LongInt Magic = 0x1b342b4d;

char *InfoMessCopyright = "(C) 1992,1997 Alfred Arnold";

/** ValidSymChars:SET OF Char=['A'..'Z','a'..'z',#128..#165,'0'..'9','_','.']; **/

String SourceFile; /* Hauptquelldatei */

String ClrEol; /* String fuer loeschen bis Zeilenende */
String CursUp; /*   "     "  Cursor hoch */

LargeWord PCs[PCMax + 1]; /* Programmzaehler */
LargeWord StartAdr; /* Programmstartadresse */
bool StartAdrPresent; /*          "           definiert? */
LargeWord Phases[PCMax + 1]; /* Verschiebungen */
Word Grans[PCMax + 1]; /* Groesse der Adressierungselemente */
Word ListGrans[PCMax + 1]; /* Wortgroesse im Listing */
ChunkList SegChunks[PCMax + 1]; /* Belegungen */
Integer ActPC; /* gewaehlter Programmzaehler */
bool PCsUsed[PCMax + 1]; /* PCs bereits initialisiert ? */
LongInt SegInits[PCMax + 1]; /* Segmentstartwerte */
LongInt ValidSegs; /* erlaubte Segmente */
bool ENDOccured; /* END-Statement aufgetreten ? */
bool Retracted; /* Codes zurueckgenommen ? */

Word TypeFlag; /* Welche Adressraeume genutzt ? */
ShortInt SizeFlag; /* Welche Operandengroessen definiert ? */

Byte PassNo; /* Durchlaufsnummer */
Integer JmpErrors; /* Anzahl fraglicher Sprungfehler */
bool ThrowErrors; /* Fehler verwerfen bei Repass ? */
bool Repass; /* noch ein Durchlauf erforderlich */
Byte MaxSymPass; /* Pass, nach dem Symbole definiert sein muessen */
Byte ShareMode; /* 0=kein SHARED,1=Pascal-,2=C-Datei, 3=ASM-Datei */
DebugType DebugMode; /* Ausgabeformat Debug-Datei */
Byte ListMode; /* 0=kein Listing,1=Konsole,2=auf Datei */
Byte ListOn; /* Listing erzeugen ? */
bool MakeUseList; /* Belegungsliste ? */
bool MakeCrossList; /* Querverweisliste ? */
bool MakeSectionList; /* Sektionsliste ? */
bool MakeIncludeList; /* Includeliste ? */
bool RelaxedMode; /* alle Integer-Syntaxen zulassen ? */
Byte ListMask; /* Listingmaske */
bool ExtendErrors; /* erweiterte Fehlermeldungen */
bool NumericErrors; /* Fehlermeldungen mit Nummer */
bool CodeOutput; /* Code erzeugen */
bool MacProOutput; /* Makroprozessorausgabe schreiben */
bool MacroOutput; /* gelesene Makros schreiben */
bool QuietMode; /* keine Meldungen */
char *DivideChars; /* Trennzeichen fuer Parameter. Inhalt Read Only! */
bool HasAttrs; /* Opcode hat Attribut */
char *AttrChars; /* Zeichen, mit denen Attribut abgetrennt wird */
bool MsgIfRepass; /* Meldungen, falls neuer Pass erforderlich */
Integer PassNoForMessage; /* falls ja: ab welchem Pass ? */
bool CaseSensitive; /* Gross/Kleinschreibung unterscheiden ? */

FILE *PrgFile; /* Codedatei */

String ErrorPath, ErrorName; /* Ausgabedatei Fehlermeldungen */
String OutName; /* Name Code-Datei */
bool IsErrorOpen;
String CurrFileName; /* mom. bearbeitete Datei */
LongInt MomLineCounter; /* Position in mom. Datei */
LongInt CurrLine; /* virtuelle Position */
LongInt LineSum; /* Gesamtzahl Quellzeilen */
LongInt MacLineSum; /* inkl. Makroexpansion */

LongInt NOPCode; /* Maschinenbefehl NOP zum Stopfen */
bool TurnWords; /* true  = Motorola-Wortformat */
                                    /* false = Intel-Wortformat */
Byte HeaderID; /* Kennbyte des Codeheaders */
char *PCSymbol; /* Symbol, womit Programmzaehler erreicht wird. Inhalt Read Only! */
TConstMode ConstMode;
bool SetIsOccupied; /* true: SET ist Prozessorbefehl */
void (*MakeCode)(void); /* Codeerzeugungsprozedur */
bool (*ChkPC)(void); /* ueberprueft Codelaengenueberschreitungen */
bool (*IsDef)(void); /* ist Label nicht als solches zu werten ? */
void (*SwitchFrom)(void); /* bevor von einer CPU weggeschaltet wird */
void (*InternSymbol)(char *Asc, TempResult * Erg); /* vordefinierte Symbole ? */
void (*InitPassProc)(void); /* Funktion zur Vorinitialisierung vor einem Pass */
void (*ClearUpProc)(void); /* Aufraeumen nach Assemblierung */

String IncludeList; /* Suchpfade fuer Includedateien */
Integer IncDepth, NextIncDepth; /* Verschachtelungstiefe INCLUDEs */
FILE *ErrorFile; /* Fehlerausgabe */
FILE *LstFile; /* Listdatei */
FILE *ShareFile; /* Sharefile */
FILE *MacProFile; /* Makroprozessorausgabe */
FILE *MacroFile; /* Ausgabedatei Makroliste */
String LstName; /* Name der Listdatei */
String MacroName, MacProName;
bool DoLst, NextDoLst; /* Listing an */
String ShareName; /* Name des Sharefiles */
/**   PrgName:String;                  { Name der Codedatei }**/

CPUVar MomCPU, MomVirtCPU; /* definierter/vorgegaukelter Prozessortyp */
char MomCPUIdent[10]; /* dessen Name in ASCII */
PCPUDef FirstCPUDef; /* Liste mit Prozessordefinitionen */
CPUVar CPUCnt; /* Gesamtzahl Prozessoren */

bool FPUAvail; /* Koprozessor erlaubt ? */
bool DoPadding; /* auf gerade Byte-Zahl ausrichten ? */
bool SupAllowed; /* Supervisormode freigegeben */
bool Maximum; /* CPU nicht kastriert */

String LabPart, OpPart, AttrPart, /* Komponenten der Zeile */
 ArgPart, CommPart, LOpPart;
char AttrSplit;
ArgStrField ArgStr; /* Komponenten des Arguments */
Byte ArgCnt; /* Argumentzahl */
String OneLine; /* eingelesene Zeile */

Byte LstCounter; /* Zeilenzaehler fuer automatischen Umbruch */
Word PageCounter[ChapMax + 1]; /* hierarchische Seitenzaehler */
Byte ChapDepth; /* momentane Kapitelverschachtelung */
String ListLine; /* alternative Ausgabe vor Listing fuer EQU */
String ErrorPos; /* zus. Positionsinformation in Makros */
Byte PageLength, PageWidth; /* Seitenlaenge/breite in Zeilen/Spalten */
bool LstMacroEx; /* Makroexpansionen auflisten */
String PrtInitString; /* Druckerinitialisierungsstring */
String PrtExitString; /* Druckerdeinitialisierungsstring */
String PrtTitleString; /* Titelzeile */
String ExtendError; /* erweiterte Fehlermeldung */

LongInt MomSectionHandle; /* mom. Namensraum */
PSaveSection SectionStack; /* gespeicherte Sektionshandles */

LongInt CodeLen; /* Laenge des erzeugten Befehls */
LongWord *DAsmCode; /* Zwischenspeicher erzeugter Code */
Word *WAsmCode;
Byte *BAsmCode;

bool DontPrint; /* Flag:PC veraendert, aber keinen Code erzeugt */
/**   MultiFace:RECORD Case Byte OF
		    0:(Feld:WordField);
		    1:(Val32:Single);
		    2:(Val64:Double);
		    3:(Val80:Extended);
		    4:(ValCo:Comp);
		    };**/

Byte StopfZahl; /* Anzahl der im 2.Pass festgestellten
                   ueberfluessigen Worte, die mit NOP ge-
                   fuellt werden muessen */

bool SuppWarns;

unsigned char CharTransTable[256]; /* Zeichenuebersetzungstabelle */

PFunction FirstFunction; /* Liste definierter Funktionen */

PDefinement FirstDefine; /* Liste von Praeprozessor-Defines */

PSaveState FirstSaveState; /* gesicherte Zustaende */

bool MakeDebug; /* Debugginghilfe */
FILE *Debug;

void AsmDefInit(void) {
   LongInt z;

   DoLst = true;
   PassNo = 1;
   MaxSymPass = 1;

   LineSum = 0;

   for (z = 0; z <= ChapMax; PageCounter[z++] = 0);
   LstCounter = 0;
   ChapDepth = 0;

   PrtInitString[0] = '\0';
   PrtExitString[0] = '\0';
   PrtTitleString[0] = '\0';

   ExtendError[0] = '\0';

   CurrFileName[0] = '\0';
   MomLineCounter = 0;

   FirstFunction = NULL;
   FirstDefine = NULL;
   FirstSaveState = NULL;
}

void NullProc(void) {
}

void Default_InternSymbol(char *Asc, TempResult * Erg) {
   if (Asc); /* satisfy compiler */
   Erg->Typ = TempNone;
}

void asmdef_init(void) {
   Integer z;

   InitPassProc = NullProc;
   ClearUpProc = NullProc;
   FirstCPUDef = NULL;
   CPUCnt = 0;
   SwitchFrom = NullProc;
   for (z = 0; z < 256; z++) CharTransTable[z] = z;
   InternSymbol = Default_InternSymbol;

   DAsmCode = (LongWord *) malloc(MaxCodeLen / 4);
   WAsmCode = (Word *) DAsmCode;
   BAsmCode = (Byte *) DAsmCode;

   RelaxedMode = true;
   ConstMode = ConstModeC;
}
