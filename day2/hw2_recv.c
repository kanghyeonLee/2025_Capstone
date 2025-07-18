#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#define BUF_SIZE 1024
#define SEQ_SIZE 4000

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

int main(int argc, char *argv[]){
    int sock;
    char message[BUF_SIZE];
    int str_len,read_len;
    socklen_t adr_sz;
    int opt = 1;
    FILE* fp;
    pkt_t *recv_pkt,*send_pkt;
    int mode,idx, total_num = 0,flag = 1;
    char filename[BUF_SIZE][BUF_SIZE];
    int filesize[BUF_SIZE];
    int flag_update = 0;

    struct sockaddr_in serv_adr,from_adr;
    if(argc != 3){
        printf("Usage: %s <IP> <port>\n",argv[0]);
        exit(1);
    }

    sock = socket(PF_INET,SOCK_DGRAM,0);
    if(sock == -1)
        error_handling("socket() error");
    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) < 0)
        error_handling("set socket(SO_REUSEADDR) error");
    
    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    send_pkt = (pkt_t *) malloc(sizeof(pkt_t));
    recv_pkt = (pkt_t *) malloc(sizeof(pkt_t));

    while (flag)
    {
        memset(send_pkt,0,sizeof(pkt_t));
        memset(recv_pkt,0,sizeof(pkt_t));
        printf("Insert mode(0: view of file list 1: select specific file and store it 2: quit): ");
        scanf("%d",&mode);
        send_pkt->mode = mode;
        adr_sz = sizeof(from_adr);
        sendto(sock,send_pkt,sizeof(pkt_t),0,(struct sockaddr*)&serv_adr,sizeof(serv_adr));
        switch (mode)
        {
        case 0:
            idx = 0;
            while (recvfrom(sock,recv_pkt,sizeof(pkt_t),0,(struct sockaddr*)&from_adr,&adr_sz))
            {
                if(!strcmp(recv_pkt->data,"/end"))
                    break;
                strcpy(filename[idx],recv_pkt->data);
                filesize[idx++] = recv_pkt->file_size;
            }
            total_num = idx;
            for(int i=0; i<total_num; i++)
                printf("%d. %s %dbytes\n",i+1,filename[i],filesize[i]);
            flag_update = 1;
            break;
        case 1:
            if(!flag_update){
                printf("If you did not see the updated file list, you should see that first.\n");
                send_pkt->file_size = -1;
                sendto(sock,send_pkt,sizeof(pkt_t),0,(struct sockaddr*)&serv_adr,sizeof(serv_adr));
                continue;
            }
            int menu_index;
            printf("Please enter the file number (1~%d): ",total_num);
            scanf("%d",&menu_index);
            if(menu_index < 1 || menu_index > total_num){
                printf("Inappropriate file number\n");
                continue;
            }
            strcpy(send_pkt->data,filename[menu_index-1]);
            send_pkt->file_size = filesize[menu_index-1];
            sendto(sock,send_pkt,sizeof(pkt_t),0,(struct sockaddr*)&serv_adr,sizeof(serv_adr));
            char temp[BUF_SIZE] = "tmp_";
            strcat(temp,filename[menu_index-1]);
            fp = fopen(temp,"wb");
            if(fp == NULL)
                error_handling("fopen() error");
            int count = countseq(filesize[menu_index-1],SEQ_SIZE);
            int i = 0, cnt = 1;
            seq_t *seqt = malloc(sizeof(seq_t));
            while (i < count)
            {
                
                memset(seqt,0,sizeof(seq_t));
                recvfrom(sock,seqt,sizeof(seq_t),0,(struct sockaddr*)&from_adr,&adr_sz);
                if(seqt->seq == i){
                    fwrite(seqt->data,1,seqt->data_size,fp);
                    i++;
                }
                sendto(sock,send_pkt,sizeof(pkt_t),0,(struct sockaddr*)&serv_adr,sizeof(serv_adr));
            }
            fclose(fp);
            flag_update = 0;
            break;
        case 2:
            printf("quit!\n");
            flag = 0;
            break;
        default:
            printf("Undefined mode!\n");
            break;
        }
    }
    
    free(send_pkt);
    free(recv_pkt);
    close(sock);
    return 0;
}

int countseq(int filesize,int seqsize){
    if(filesize % seqsize !=0) return filesize/seqsize + 1;
    return filesize/seqsize;
}