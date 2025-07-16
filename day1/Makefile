CC = gcc
CFLAGS = -Wall

all: hw1_client hw1_server

hw1_client: hw1_client.c
	$(CC) $(CFLAGS) -o hw1_client hw1_client.c

hw1_server: hw1_server.c
	$(CC) $(CFLAGS) -o hw1_server hw1_server.c

clean:
	rm -f hw1_client hw1_server
