CC = gcc
CFLAGS = -Wall

all: hw3_client hw3_server

hw3_client: hw3_client.o hw3_clntlib.o 
	$(CC) $(CFLAGS) -o hw3_client $^
hw3_client.o: hw3_lib.h hw3_client.c
	$(CC) $(CFLAGS) -c -o hw3_client.o hw3_client.c
hw3_clntlib.o: hw3_lib.h hw3_clntlib.c
	$(CC) $(CFLAGS) -c -o hw3_clntlib.o hw3_clntlib.c
hw3_server: hw3_server.o hw3_servlib.o
	$(CC) $(CFLAGS) -o hw3_server $^
hw3_server.o: hw3_lib.h hw3_server.c
	$(CC) $(CFLAGS) -c -o hw3_server.o hw3_server.c
hw3_servlib.o: hw3_lib.h hw3_servlib.c 
	$(CC) $(CFLAGS) -c -o hw3_servlib.o hw3_servlib.c
clean:
	rm -f hw3_client hw3_server hw3_client.o hw3_clntlib.o hw3_server.o hw3_servlib.o