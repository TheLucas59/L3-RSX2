CC = gcc
CFLAGS = -Wall -Wextra

all: ReceiveUDP SendUDP DNS DNSsimple

ReceiveUDP: ReceiveUDP.o
	$(CC) $(CFLAGS) -o $@ $^

ReceiveUDP.o : ReceiveUDP.c
	$(CC) $(CFLAGS) -c $^

SendUDP: SendUDP.o
	$(CC) $(CFLAGS) -o $@ $^

SendUDP.o : SendUDP.c
	$(CC) $(CFLAGS) -c $^

DNS: DNS.o
	$(CC) $(CFLAGS) -o $@ $^

DNS.o: DNS.c
	$(CC) $(CFLAGS) -c $^

DNSsimple : DNSsimple.c
	$(CC) $? -o $@

clean :
	rm -f *.o DNS ReceiveUDP SendUDP DNSsimple