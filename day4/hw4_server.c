#include "hw4_lib.h"
#include <pthread.h>
#define MAX_CLNT 256

typedef struct 
{   
    int num_s_word;
    char search_word[MAX][BUF_SIZE];
    int search_num;
}search_d;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];

void *handle_clnt(void *arg);

int main(int argc, char *argv[]){
    //변수 선언
    FILE *fp;
    char buf[BUF_SIZE];
    search_d **search_data;
    int s_num = 0,option = 1;
    int serv_sock,clnt_sock;
    struct sockaddr_in serv_addr,clnt_addr;
    socklen_t clnt_addr_size;
    pthread_t t_id;
    //동적메모리할당
    search_data = calloc(MAX,sizeof(search_d*));
    // command line 인자 확인 
    if(argc != 3){
        printf("Usage: %s <port> <file>\n",argv[0]);
        exit(1);
    }
    // 읽어드린 file 읽고 검색어 및 검색횟수 분리 저장
    fp = fopen(argv[2],"rb");
    if(fp == NULL){
        perror("fopen error");
        exit(1);
    }
    while (!feof(fp))
    {
        fgets(buf,BUF_SIZE,fp);
        search_data[s_num] = calloc(1,sizeof(search_d));
        int i=0;
        char *ptr = strtok(buf," ");
        while (ptr != NULL)
        {
            int tmp_n = atoi(ptr);
            if(tmp_n ==0) strcpy(search_data[s_num]->search_word[i++],ptr);
            else search_data[s_num]->search_num = tmp_n;
            ptr = strtok(NULL," ");
        }
        search_data[s_num++]->num_s_word = i;
    }
    memset(buf,0,BUF_SIZE);
    // TCP server socket start
    serv_sock = socket(PF_INET,SOCK_STREAM,0);
    if(serv_sock == -1)
        error_handling("socket() error");
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR, &option, sizeof(option));
    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    if(listen(serv_sock,5)==-1)
        error_handling("listen() error");
    while (1)
    {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_addr,&clnt_addr_size);
        if(clnt_sock == -1)
            error_handling("accept() error");
        pthread_mutex_lock(&mutex);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutex);

        pthread_create(&t_id,NULL,handle_clnt,(void *)&clnt_sock);
        pthread_detach(t_id);
        printf("Connected client %d\n",clnt_sock);
    }

    //end
    // 메모리 할당해제 및 파일 닫기
    close(serv_sock);
    fclose(fp);
    for(int i=0; i<MAX; i++)
        free(search_data[i]);
    free(search_data);
    return 0;
} 

void *handle_clnt(void *arg){
    int clnt_sock = *((int *)arg);
    char msg[BUF_SIZE];
    read(clnt_sock,msg,sizeof(msg));

    pthread_mutex_lock(&mutex);
    for(int i=0; i<clnt_cnt; i++){
        if(clnt_sock == clnt_socks[i])
        {
            while (i++<clnt_cnt-1)
                clnt_socks[i] = clnt_socks[i+1];
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutex);
    close(clnt_sock);
    return NULL;
}

