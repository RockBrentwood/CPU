LINKFLAGS	=-o
CC	=qcl
CLIENT_OBJ	=client.obj com.obj
DEMO_OBJ	=demo.obj com.obj
SERIAL_OBJ	=serial.obj micronet.obj com.obj console.obj
all:	client.exe demo.exe serial.exe
client:	client.exe
demo:	demo.exe
serial:	serial.exe
client.obj:	client.c com.h port.h
demo.obj:	demo.c com.h port.h
serial.obj:	serial.c com.h port.h micronet.h console.h
micronet.obj:	micronet.c com.h port.h micronet.h
com.obj:	com.c    com.h port.h
console.obj:	console.c
client.exe:	$(CLIENT_OBJ)
	$(CC) $(LINKFLAGS) client $(CLIENT_OBJ)
demo.exe:	$(DEMO_OBJ)
	$(CC) $(LINKFLAGS) demo $(DEMO_OBJ)
serial.exe:	$(SERIAL_OBJ)
	$(CC) $(LINKFLAGS) serial $(SERIAL_OBJ)
