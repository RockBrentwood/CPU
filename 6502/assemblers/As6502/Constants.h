#define LAST_CH_POS	132
#define SFIELD	23
#define STABSZ	20000
#define SBOLSZ	20

// Symbol flags
#define DEFZRO	2	// defined - page zero address
#define MDEF	3	// multiply defined
#define UNDEF	1	// undefined - may be zero page
#define DEFABS	4	// defined - two byte address
#define UNDEFAB 5	// undefined - two byte address
#define PAGESIZE 60	// number of lines on a page
#define LINESIZE 133	// number of characters on a line
#define TITLESIZE 100	// maximum characters in title

// Operation code flags.
#define PSEUDO	0x6000
#define CLASS1	0x2000
#define CLASS2	0x4000
#define IMM1	0x1000	// opval + 0x00 2 byte
#define IMM2	0x0800	// opval + 0x08 2 byte
#define ABS	0x0400	// opval + 0x0C 3 byte
#define ZER	0x0200	// opval + 0x04 2 byte
#define INDX	0x0100	// opval + 0x00 2 byte
#define ABSY2	0x0080	// opval + 0x1C 3 byte
#define INDY	0x0040	// opval + 0x10 2 byte
#define ZERX	0x0020	// opval + 0x14 2 byte
#define ABSX	0x0010	// opval + 0x1C 3 byte
#define ABSY	0x0008	// opval + 0x18 3 byte
#define ACC	0x0004	// opval + 0x08 1 byte
#define IND	0x0002	// opval + 0x2C 3 byte
#define ZERY	0x0001	// opval + 0x14 2 byte

// Pass flags.
#define FIRST_PASS	0
#define LAST_PASS	1
#define DONE		2
