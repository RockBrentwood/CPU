typedef unsigned char byte;
typedef unsigned int word;

extern int PortStatus(void);
#define BUFFER_FULL 0x01
#define TIMED_OUT   0x10
#define PACKET_OVER 0x20
#define BAD_HEADER  0x40
#define BAD_SUM     0x80

extern int GetStats(int Counter, byte *Buf, int Bytes);
extern int GetDump(int Counter, byte *Buf, int Bytes);
extern int SendStatus(int Unit);
extern void SendAbort(int Unit);
extern void SendTest(int Unit);
