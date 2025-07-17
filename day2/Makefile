CC = gcc
CFLAGS = -Wall

all: hw2_recv hw2_send

hw2_recv: hw2_recv.c
	$(CC) $(CFLAGS) -o hw2_recv hw2_recv.c

hw2_send: hw2_send.c
	$(CC) $(CFLAGS) -o hw2_send hw2_send.c

clean:
	rm -f hw2_recv hw2_send