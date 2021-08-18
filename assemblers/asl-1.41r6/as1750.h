// Exports of as1750.c.
void init_as1750();
unsigned short as1750(char *operation, int n_operands, char *operand[]);

#ifdef AS1750
void add_word(ushort word);
void add_reloc(symbol_t sym);
char *get_num(char *s, int *outnum);
char *get_sym_num(char *s, int *outnum);
status parse_addr(char *s);
status error(char *layout, ...);
#else
#   define OKAY      0
#   define ERROR     0xFFFD
#   define NO_OPCODE 0xFFFE
#endif
