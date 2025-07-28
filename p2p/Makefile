CC = gcc
CFLAGS = -Wall -g

all: p2p

p2p: p2p.o p2p_lib.o
	$(CC) $(CFLAGS) -o p2p $^
p2p.o: p2p.c
	$(CC) -c -o p2p.o p2p.c
p2p_lib.o: p2p_lib.c
	$(CC) -c -o p2p_lib.o p2p_lib.c
clean:
	rm -f p2p p2p.o p2p_lib.o