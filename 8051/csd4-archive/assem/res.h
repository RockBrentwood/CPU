typedef struct Item *Item;
extern struct Item {
   char Tag; Exp E; byte Map; word Line, File;
   Segment Seg; word Offset;
} *RTab;
extern int RCur;

extern long LVal;

extern void ResInit(void);
extern void Resolve(Item IP);
extern void Reloc(byte Code, byte Tag, Exp E);
