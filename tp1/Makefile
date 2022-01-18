CC = gcc
CFLAGS = -Wall -Wextra

ReceiveUDP: ReceiveUDP.o
	$(CC) $(CFLAGS) -o $@ $^

ReceiveUDP.o : ReceiveUDP.c
	$(CC) $(CFLAGS) -c $^

SendUDP: SendUDP.o
	$(CC) $(CFLAGS) -o $@ $^

SendUDP.o : SendUDP.c
	$(CC) $(CFLAGS) -c $^

DNSsimple : DNSsimple.c
	$(CC) $? -o $@

clean :
	rm -f *.o ReceiveUDP SendUDP DNSsimple