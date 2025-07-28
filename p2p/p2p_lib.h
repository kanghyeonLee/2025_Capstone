#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SIZE 1024

typedef struct 
{
    int peer_flag; // 0은 receiving peer, 1은 sending peer
    int max_num_rp;
    char *file_name;
    int segment_size;
    int listen_port;
    char *sending_ip_address;
    int sending_port;
}peer_d;

void optargHandler(int argc, char *argv[], peer_d* peer);