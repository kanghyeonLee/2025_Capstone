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

int main(int argc, char *argv[]){
    int serv_sock;
    int clnt_sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    int fd;
    int read_cnt;

    char message[BUF_SIZE];
    char buf[BUF_SIZE];

    if(argc!=2){
        printf("Usage: %s <port>\n",argv[0]);
        exit(1);
    }

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
    
    while (1)
    {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if(clnt_sock == -1)
            error_handling("accept() error");
        DIR *dp = opendir(".");
        if(dp == NULL)
            error_handling("opendir() error");
        struct dirent *d;
        while ((d = readdir(dp)) != NULL)
        {
            if(!strcmp(d->d_name,".") || !strcmp(d->d_name,".."))
                continue;
            struct stat file_stat;
            if(stat(d->d_name,&file_stat) == -1)
                error_handling("stat() error");
            sprintf(message,"%s %lldbytes\n",d->d_name,file_stat.st_size);
            write(clnt_sock,message,sizeof(message));
        }
        closedir(dp);
        shutdown(clnt_sock, SHUT_WR);
        read(clnt_sock,message,sizeof(message));
        if(!strcmp(message,"/end")){
            printf("end of connection\n");
            close(clnt_sock);
            break;
        }
        fd = open(message,O_RDONLY);
        close(clnt_sock);
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        while (1)
        {
            read_cnt = read(fd,buf,BUF_SIZE);
            if(read_cnt < BUF_SIZE){
                write(clnt_sock,buf,read_cnt);
                break;
            }
            write(clnt_sock,buf,BUF_SIZE);
        }
        close(clnt_sock);
    }
    close(serv_sock);
}