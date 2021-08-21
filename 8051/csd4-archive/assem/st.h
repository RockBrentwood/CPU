extern bool Active; // Used for conditionals.
extern int Phase; // 0 = Parsing, 1 = Backpatching, 2 = Linking.
extern void Assemble(char *File);
extern void Generate(void);
extern FILE *OpenObj(char *Obj);
