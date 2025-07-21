#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define FILE_LENGTH 30

typedef struct 
{
    unsigned int file_size;
    char file_name[BUF_SIZE];
    unsigned int directory;
}pkt_t;

typedef struct 
{
    unsigned int mode;
    char cmd[BUF_SIZE];
}pkt_data;

typedef struct 
{
    unsigned int mode;
    char cmd1[BUF_SIZE];
    char cmd2[BUF_SIZE];
}cmd_d;

void error_handling(char *message);
int viewFileList_server(int clnt_sock,pkt_t **send_pkt, char current_d[BUF_SIZE][BUF_SIZE]);
void handlingCmd_server(cmd_d* data,int i,pkt_t **file_list,pkt_t **clnt_file, char current_d[BUF_SIZE][BUF_SIZE], int *length_of_list, int *length);
void printFileList(pkt_t **file_list, int length, int sock);
int handlingCmd_client(char *cmd, cmd_d* cmd_data);
int viewFileList_client(pkt_t **send_pkt);
int dfile_func(int sock, cmd_d *cmd_data, pkt_t**file_list);
void ufile_func(int clnt_file_length,pkt_t** clnt_file,int sock);