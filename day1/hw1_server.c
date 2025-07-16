#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 1024

void error_handling(char *message){
    fprintf(stderr,"%s\n",message);
    exit(1);
}

typedef struct 
{
    unsigned int mode;
    unsigned char msg[BUF_SIZE];
}pkt_t;


int main(int argc, char *argv[]){
    int serv_sock;
    int clnt_sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;
    pkt_t * recv_pkt;
    pkt_t * send_pkt;
    FILE* fp;
    DIR * dp;
    struct dirent *d;
    int read_cnt;

    char message[BUF_SIZE];
    char buf[BUF_SIZE];

    if(argc!=2){
        printf("Usage: %s <port>\n",argv[0]);
        exit(1);
    }
    // socket 생성 과정 
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error");
    
    memset(&serv_addr, 0, sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serv_addr.sin_port = htons(atoi(argv[1])); 

    if(bind(serv_sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if(clnt_sock == -1)
            error_handling("accept() error");
    // 메뉴에 따른 요청 처리 과정 시작 
    recv_pkt = (pkt_t *) malloc(sizeof(pkt_t));
    int read_len;
    while((read_len = read(clnt_sock,recv_pkt,sizeof(pkt_t)))!=0)
    {
        send_pkt = (pkt_t *) malloc(sizeof(pkt_t));
        send_pkt->mode = recv_pkt->mode;
        printf("Mode %d\n",recv_pkt->mode);
        switch (recv_pkt->mode)
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
                sprintf(message,"%s %lld\n",d->d_name,file_stat.st_size);
                strcat(send_pkt->msg,message);
            }
            closedir(dp);
            write(clnt_sock,send_pkt,sizeof(pkt_t));
            break;
        case 1:
            fp = fopen(recv_pkt->msg,"rb");
            while (1)
            {
                read_cnt = fread(buf,1,BUF_SIZE,fp);
                if(read_cnt < BUF_SIZE){
                    write(clnt_sock,buf,read_cnt);
                    break;
                }
                write(clnt_sock,buf,BUF_SIZE);
            }
            fclose(fp);
            break;
        case 2:
            close(clnt_sock);
            clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
            if(clnt_sock == -1)
                    error_handling("accept() error");
            break;
        default:
            printf("Undefined mode!\n");
            break;
        }
    }
    
    close(serv_sock);
}