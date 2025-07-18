#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#define BUF_SIZE 1024
#define SEQ_SIZE 4096

void error_handling(char *message){
    printf("%s\n",message);
    exit(1);
}

int countseq(int filesize,int seqsize);

typedef struct{
    unsigned int mode;
    unsigned int file_size;
    char data[BUF_SIZE];
}pkt_t;

typedef struct{
    unsigned int seq;
    unsigned int data_size;
    char data[SEQ_SIZE];
}seq_t;


int main(int argc, char *argv[])
{
    int serv_sock;
    char message[BUF_SIZE],buf[BUF_SIZE];
    int str_len;
    socklen_t clnt_adr_sz;
    struct sockaddr_in serv_adr, clnt_adr;
    int opt = 1;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    double start, finish;
    struct timeval time;
    double duration;
    double throughput;
    FILE*fp;
    pkt_t *recv_pkt, *send_pkt;
    DIR *dp;
    struct dirent* d;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (serv_sock == -1)
        error_handling("UDP socket creation error");
    if(setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) < 0)
        error_handling("set socket(SO_REUSEADDR) error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    send_pkt = (pkt_t *) malloc(sizeof(pkt_t));
    recv_pkt = (pkt_t *) malloc(sizeof(pkt_t));
   
    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    
    while (1)
    {
        memset(send_pkt,0,sizeof(pkt_t));
        memset(recv_pkt,0,sizeof(pkt_t));
        clnt_adr_sz = sizeof(clnt_adr);
        recvfrom(serv_sock,recv_pkt,sizeof(pkt_t),0,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
        int mode = recv_pkt->mode;
        switch (mode)
        {
        case 0:
            dp = opendir(".");
            if(dp == NULL)
                error_handling("opendir() error");
            while ((d = readdir(dp)) != NULL)
            {
                if(!strcmp(d->d_name,".") || !strcmp(d->d_name,".."))
                    continue;
                struct stat file_stat;
                if(stat(d->d_name,&file_stat) == -1)
                    error_handling("stat() error");
                if(d->d_name[strlen(d->d_name)-1] == '\n')
                    d->d_name[strlen(d->d_name)-1] = '\0';
                send_pkt->file_size = file_stat.st_size;
                strcpy(send_pkt->data,d->d_name);
                sendto(serv_sock,send_pkt,sizeof(pkt_t),0,(struct sockaddr*)&clnt_adr,clnt_adr_sz);
            }
            strcpy(send_pkt->data,"/end");
            sendto(serv_sock,send_pkt,sizeof(pkt_t),0,(struct sockaddr*)&clnt_adr,clnt_adr_sz);
            closedir(dp);
            break;
        case 1:
            recvfrom(serv_sock,recv_pkt,sizeof(pkt_t),0,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
            if(recv_pkt->file_size == -1)
                continue;
            char filename[BUF_SIZE];
            strcpy(filename, recv_pkt->data);
            fp = fopen(filename,"rb");
            if(fp == NULL)
                error_handling("fopen() error");
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            int filesize = recv_pkt->file_size;
            int count = countseq(filesize,SEQ_SIZE);
            int i = 0,cnt = 1,flag_loss = 0;
            seq_t *seqt = malloc(sizeof(seq_t));
            if(setsockopt(serv_sock,SOL_SOCKET,SO_RCVTIMEO,(const void*)&timeout, sizeof(timeout)) < 0)
                error_handling("set socket(SO_RCVTIMEO) error");
            while (i < count)
            {
               

                memset(seqt,0,sizeof(seq_t));
                seqt->seq=i;
                seqt->data_size = (i+1 == count)? filesize%SEQ_SIZE : SEQ_SIZE;
                if(fread(seqt->data,1,seqt->data_size,fp) == -1)
                    error_handling("fread() error");
            
                if(i==0){
                    gettimeofday(&time,NULL);
                    start = time.tv_sec*1000000 + time.tv_usec;
                }
                sendto(serv_sock,seqt,sizeof(seq_t),0,(struct sockaddr*)&clnt_adr,clnt_adr_sz);
                while(recvfrom(serv_sock,recv_pkt,sizeof(pkt_t),0,(struct sockaddr*)&clnt_adr,&clnt_adr_sz)==-1){
                    if(errno == EAGAIN || errno == EWOULDBLOCK){
                        printf("%d. Packet loss and Time out! (Processing %.2f%%)\n",cnt++, (seqt->data_size*(i+1))/((double)filesize)*100);
                        sendto(serv_sock,seqt,sizeof(seq_t),0,(struct sockaddr*)&clnt_adr,clnt_adr_sz);
                    }else{
                        error_handling("recvfrom() error");
                    }
                }
                if(i+1 == count){
                    gettimeofday(&time,NULL);
                    finish = time.tv_sec*1000000 + time.tv_usec;
                }
                    
                i++;
            }
            fclose(fp);
            duration = (finish - start)/1000000;
            throughput = filesize/(double)duration;
            printf("'%s'-%dbytes throughput: %.4lfB/s (filesize:%d, duration:%.4lf)\n",filename,filesize,throughput,filesize,duration);
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            break;
        case 2:
            printf("receiver quit!\n");
            break;
        default:
            printf("Undefined mode!\n");
            break;
        }
    }
    free(send_pkt);
    free(recv_pkt);
    close(serv_sock);
    return 0;
}

int countseq(int filesize,int seqsize){
    if(filesize % seqsize !=0) return filesize/seqsize + 1;
    return filesize/seqsize;
}