#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/select.h>
#include <dirent.h>

#define KB 1024
#define SENDING_PEER 1
#define RECEIVING_PEER 0
#define MAX_CLNT 256

typedef struct 
{
    int peer_flag;
    int max_num_rp;
    char *file_name;
    int segment_size;
    int listen_port;
    char *sending_ip_address;
    int sending_port;
}peer_d;

typedef struct 
{
    int id;
    char ip_address[16];
    int port;
}recv_d;

typedef struct 
{
    int segment_size;
    char *segment;
}seg_d;

typedef struct 
{
    int file_size;
    int segment_size;
    char filename[KB];
}file_d;



void optargHandler(int argc, char *argv[], peer_d* peer);
void error_handling(char *msg);
void getIpAddress(char *recv_host);
void *connectOtherRecv(void *arg);
void *sendRecvData(void *arg);
void *readDataFromRecv(void *arg);
void *recvSegmentToMe(void *arg);
void *sendFileData(void *arg);