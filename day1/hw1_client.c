#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
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
    int sock;
    struct sockaddr_in serv_addr;
    char message[BUF_SIZE];
    char file_sort[BUF_SIZE][BUF_SIZE];
    char buf[BUF_SIZE];
    pkt_t *send_pkt, *recv_pkt;

    FILE *fp;
    int idx = 0, read_len = 0, file_len = -1;
    int read_cnt = 0;
    int menu_index;
    int mode;
    int flag = 1;

    if(argc!=3){
        printf("Usage: %s <IP> <port>\n",argv[0]);
        exit(1);
    }

    send_pkt = (pkt_t *) malloc(sizeof(pkt_t));
    recv_pkt = (pkt_t *) malloc(sizeof(pkt_t));

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        error_handling("socket() error");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    if(connect(sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    while (flag)
    {
        memset(send_pkt,0,sizeof(pkt_t));
        memset(recv_pkt,0,sizeof(pkt_t));
        idx = 0;
        read_len = 0;

        printf("Select a mode (0~1, 2 to quit):");
        scanf("%d",&mode);
        send_pkt->mode = mode;
        switch (mode)
        {
        case 0:
            write(sock,send_pkt,sizeof(pkt_t));
            read(sock,recv_pkt ,sizeof(pkt_t));
            char *tmp = strtok(recv_pkt->msg,"\n");
            while (tmp != NULL)
            {
                strcpy(file_sort[idx++],tmp);
                tmp = strtok(NULL,"\n");
            }
            
            for(int i=0; i<idx; i++){
                printf("%d. %sbytes\n",i+1,file_sort[i]);
            }
            file_len = idx;
            break;
        case 1:
            if(file_len == -1){
                printf("If you did not see the file list, you should see that first.\n");
                continue;
            }
            printf("Please enter the file number (1~%d): ",file_len);
            scanf("%d",&menu_index);
            if(menu_index < 1 || menu_index > file_len){
                printf("Inappropriate file number\n");
                continue;
            }
            char temp[BUF_SIZE];
            strcpy(temp,file_sort[menu_index-1]);
            char *file_name = strtok(temp," ");
            strcpy(send_pkt->msg,file_name);
            file_name = strtok(NULL," ");
            int file_size = atoi(file_name);
            write(sock,send_pkt,sizeof(pkt_t));
            fp = fopen(send_pkt->msg,"wb");
            while (read_len < file_size)
            {
                read_cnt = read(sock,(void *)buf,BUF_SIZE);
                if(read_cnt == -1)
                    error_handling("read() error");
                read_len += read_cnt;
                fwrite(buf,1,read_cnt,fp);
            }
            fclose(fp);
            break;
        case 2:
            printf("quit!\n");
            write(sock,send_pkt,sizeof(pkt_t));
            flag = 0;
            break;
        default:
            printf("Undefined mode!\n");
            continue;
        }
    }
    close(sock);
}