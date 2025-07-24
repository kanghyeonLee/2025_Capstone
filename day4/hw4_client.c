#include "hw4_lib.h"

int main(int argc, char *argv[]){
    //변수 선언
    int sock;
    char buf[BUF_SIZE],tmp[BUF_SIZE];
    struct sockaddr_in serv_addr;
    int len, result,num_s_word;
    char verify;
    //인자 확인
    if(argc != 3){
        printf("Usage: %s <IP> <port>\n",argv[0]);
        exit(1);
    }
    // TCP client socket start
    sock = socket(PF_INET,SOCK_STREAM,0);
    if(sock == -1)
        error_handling("socket() error");
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) == -1)
        error_handling("connect() error");
    //end 탐색할 단어 입력받고 전송
    printf("Input Search Word: ");
    scanf("%s",buf);
    write(sock,buf,strlen(buf));
    read(sock,&result,sizeof(int));
    if(result == -1){
        printf("No search data...\n");
    }else{
        printf("-----------------------------\n");
        while(1)
        {
            read(sock,&num_s_word,sizeof(int));
            if(num_s_word == -1) break;
            for(int i=0; i<num_s_word; i++){
                memset(tmp,0,sizeof(tmp));
                read(sock,&len,sizeof(int));
                write(sock,&verify,sizeof(char));
                read(sock,tmp,sizeof(tmp));
                write(sock,&verify,sizeof(char));
                printf("%s ",tmp);
            }
            printf("\n");
        }
    }

    close(sock);
}