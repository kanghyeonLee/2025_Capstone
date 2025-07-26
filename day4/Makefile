CC = gcc
CFLAGS = -Wall -g

all: hw4_server hw4_client

hw4_server: hw4_server.o hw4_lib.o
	$(CC) $(CFLAGS) -o hw4_server $^
hw4_server.o: hw4_server.c
	$(CC) $(CFLAGS) -c -o hw4_server.o $^
hw4_client: hw4_client.o hw4_lib.o
	$(CC) $(CFLAGS) -o hw4_client $^
hw4_client.o: hw4_client.c
	$(CC) $(CFLAGS) -c -o hw4_client.o $^
hw4_lib.o: hw4_lib.c
	$(CC) $(CFLAGS) -c -o hw4_lib.o $^
clean:
	rm -f hw4_server hw4_client hw4_server.o hw4_client.o hw4_lib.o