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

int main(int argc, char *argv[]){
    int sock;
    struct sockaddr_in serv_addr;
    char message[BUF_SIZE];
    char file_sort[BUF_SIZE][BUF_SIZE];
    char buf[BUF_SIZE];

    int fd;
    int idx = 0, read_len = 0;
    int read_cnt;
    int menu_index;

    if(argc!=3){
        printf("Usage: %s <IP> <port>\n",argv[0]);
        exit(1);
    }


    while (1)
    {
        idx = 0;
        read_len = 0;
        sock = socket(PF_INET, SOCK_STREAM, 0);
        if(sock == -1)
            error_handling("socket() error");
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
        serv_addr.sin_port = htons(atoi(argv[2]));
        if(connect(sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
            error_handling("connect() error");
        while (read_len = read(sock, &message ,sizeof(message)))
        {
            if(read_len == -1){
                break;
            }
            strcpy(file_sort[idx++],message);
        }
        for(int i = 0; i<idx; i++){
            printf("%d. %s",i+1,file_sort[i]);
        }
        printf("Please select the file by the number of menu index(exit is 0): ");
        scanf("%d",&menu_index);

        if(menu_index < 0 || menu_index > idx)
            error_handling("Inappropriate number of menu index");

        if(menu_index == 0){
            printf("exit!\n");
            write(sock,"/end",strlen("/end")+1);
            close(sock);
            break;
        }
        char *file_name = strtok(file_sort[menu_index-1]," ");
        write(sock,file_name,strlen(file_name)+1);
        fd = open(file_name,O_CREAT | O_WRONLY | O_TRUNC);
        close(sock);
        sock = socket(PF_INET, SOCK_STREAM, 0);
        if(sock == -1)
            error_handling("socket() error");
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
        serv_addr.sin_port = htons(atoi(argv[2]));
        if(connect(sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
            error_handling("connect() error");

        while ((read_cnt = read(sock,(void *)buf,BUF_SIZE))!=0)
        {
            write(fd,buf,read_cnt);
        }
        fsync(fd);
        close(sock);
    }
    
}