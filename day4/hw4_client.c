#include "hw4_lib.h"
#define COLOR_RESET "\033[0m"

int main(int argc, char *argv[]){
    //변수 선언
    int sock;
    char buf[BUF_SIZE],tmp[BUF_SIZE],buf_temp[BUF_SIZE];
    struct sockaddr_in serv_addr;
    int len, result,num_s_word,request;
    char *pos_str;
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
    char c;
    int i = 0;
    if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) == -1)
        error_handling("connect() error");
    //end 탐색할 단어 입력받고 전송
    printf("\x1B[2J");
    while (1)
    {
       	printf("\x1B[0;0H");
        printf("Search Word: \r");
        printf("\n-----------------------------\n\r");
        printf("\x1b[%dA\x1b[%dC",2,13);
        c = get_key(0);
        if(c == 8 || c == 127){
            if(i > 0){
                memset(buf_temp,0,sizeof(buf_temp));
                strncpy(buf_temp,buf,strlen(buf)-1);
                memset(buf,0,sizeof(buf));
                strcpy(buf,buf_temp);
                i--;
            }
        }
        else if(c >= 31 && c <= 126){
            buf[i++] = c;
        }
        int empty = 0;
        if(i == 0) empty = 1;
        write(sock,&empty,sizeof(int));
        read(sock,&request,sizeof(int));
        if(!empty) write(sock,buf,strlen(buf));
        printf("\x1B[2J");
        printf("%s",buf);
        printf("\x1b[%dB\x1b[%dD\r",2,13);
        read(sock,&result,sizeof(int));
        write(sock,&request,sizeof(int));
        if(result == -1){
            printf("No search data...\n");
        }else{
            while (1)
            {
                read(sock,&num_s_word,sizeof(int));
                if(num_s_word == -1) break;
                for(int i=0; i<num_s_word; i++){
                    memset(tmp,0,sizeof(tmp));
                    read(sock,&len,sizeof(int));
                    write(sock,&verify,sizeof(char));
                    read(sock,tmp,sizeof(tmp));
                    write(sock,&verify,sizeof(char));
                    char *compare_str = capitalStrConvert(tmp);
                    char *compare_buf = capitalStrConvert(buf);
                   
                    if((pos_str = strstr(compare_str,compare_buf))){
                        int pos = pos_str - compare_str;
                        for(int p=0; p<strlen(compare_str); p++){
                            if(p >= pos && p < pos+strlen(compare_buf)){
                                printf("\033[38;2;252;127;0m%c",tmp[p]);
                            }else{
                                printf("%s",COLOR_RESET);
                                printf("%c",tmp[p]);
                            }
                        }
                        printf(" ");
                        printf("%s",COLOR_RESET);
                    }
                    else printf("%s ",tmp);
                }
                printf("\n");   
            }
        }
    }
    close(sock);
}