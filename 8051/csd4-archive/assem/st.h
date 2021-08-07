extern byte Active; /* Used for conditionals */
/* 0 = Parsing, 1 = Backpatching, 2 = Linking */
extern int Phase;
extern void Assemble(char *File);
extern void Generate(void);
extern FILE *OpenObj(char *Obj);
