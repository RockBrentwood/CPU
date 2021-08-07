extern int Status;
#define BUFFER_FULL 0x01
#define TIMED_OUT   0x10
#define PACKET_OVER 0x20
#define BAD_HEADER  0x40
#define BAD_SUM     0x80

extern int Recv(void);
extern void Send(int Tx);
extern int OpenPort(int Net, unsigned Baud, char Line, unsigned Size);
extern void ClosePort(void);
extern void FlushPort(void);
extern void SetTx(void);
extern void SetRx(void);
