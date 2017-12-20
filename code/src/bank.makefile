CC = gcc
PC = proc
RM = rm
CFLAGS = -c -Wall
LINK = -lpthread #-lclntsh
all: client server open query save withdraw transfer close
server: server.o
	$(CC) $^ -o ../bin/$@
server.o: server.c
	$(CC) $(CFLAGS) $^
client: client.o network.o
	$(CC) $^ -o ../bin/$@
client.o: client.c
	$(CC) $(CFLAGS) $^
open: open.o dao.o network.o
	$(CC) $^ $(LINK) -o ../bin/$@
open.o: open.c 
	$(CC) $(CFLAGS) $^
query: query.o dao.o network.o
	$(CC) $^ $(LINK) -o ../bin/$@
query.o: query.c
	$(CC) $(CFLAGS) $^
withdraw: withdraw.o dao.o network.o
	$(CC) $^ $(LINK) -o ../bin/$@
withdraw.o: withdraw.c
	$(CC) $(CFLAGS) $^
save: save.o dao.o network.o
	$(CC) $^ $(LINK) -o ../bin/$@
save.o: save.c
	$(CC) $(CFLAGS) $^
transfer: transfer.o network.o dao.o
	$(CC) $^ $(LINK) -o ../bin/$@
transfer.o: transfer.c
	$(CC) $(CFLAGS) $^
close: close.o dao.o network.o
	$(CC) $^ $(LINK) -o ../bin/$@
close.o: close.c
	$(CC) $(CFLAGS) $^
network.o: network.c
	$(CC) $(CFLAGS) $^
dao.o: dao.c
	$(CC) -c $^
#dao.c: dao.pc
#	$(PC) char_map=string $^
clean:
	$(RM) *.o     \
	../bin/query  \
	../bin/client \
	../bin/server \
	../bin/open	  \
	../bin/close  \
	../bin/save	  \
	../bin/transfer	  \
	../bin/withdraw
