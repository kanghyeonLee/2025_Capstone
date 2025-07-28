#include "p2p_lib.h"

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int num_recv_peer;
recv_d *recv_set;
int id = 0;

int main(int argc, char *argv[]){
    //변수 선언
    peer_d *peer;
    //명령줄 인자 다루는 과정
    optargHandler(argc,argv,peer);
    recv_set = calloc(num_recv_peer,sizeof(recv_d));
    //Sendig Peer part
    if(peer->peer_flag == SENDING_PEER){
        //sending peer 변수 선언
        pthread_t t_id;
        int serv_sock,clnt_sock;
        int i,option = 1;
        num_recv_peer = peer->max_num_rp;
        struct sockaddr_in serv_adr,clnt_adr;
        socklen_t clnt_adr_sz;
        // socket setup
        serv_sock = socket(PF_INET,SOCK_STREAM,0);
        if(serv_sock == -1)
            error_handling("socket() error");
        memset(&serv_adr,0,sizeof(serv_adr));
        serv_adr.sin_family = AF_INET;
        serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_adr.sin_port = htons(peer->listen_port);
        setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR, &option, sizeof(option));
        if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
            error_handling("bind() error");
        if(listen(serv_sock,5)==-1)
            error_handling("listen() error");
        // clnt socket 연결          
        for(i=0; i<peer->max_num_rp; i++){
            clnt_adr_sz = sizeof(clnt_adr);
            clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
            pthread_mutex_lock(&mutex);
            clnt_socks[clnt_cnt++] = clnt_sock;
            pthread_mutex_unlock(&mutex);
            pthread_create(&t_id,NULL,connectRecv,(void *)&clnt_sock);
            if(i < peer->max_num_rp -1 )pthread_detach(t_id);
            else pthread_join(t_id,NULL);
            printf("Connected client %d\n",clnt_sock);
        }
        // clnt socket에게 전달
        for(int i=0; i<peer->max_num_rp; i++){
            printf("%d. %s %d %d\n",recv_set[i].id,recv_set[i].ip_address,recv_set[i].port,id);
        }

    }else{//Receiving Peer part
        //변수선언
        int sock;
        struct sockaddr_in serv_addr;
        recv_d my_recv;
        // my_recv 설정
        getIpAddress(my_recv.ip_address);
        my_recv.port = peer->listen_port;
        //연결 과정
        sock = socket(PF_INET,SOCK_STREAM,0);
        if(sock == -1)
            error_handling("socket() error");
        memset(&serv_addr,0,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(peer->sending_ip_address);
        serv_addr.sin_port = htons(peer->sending_port);
        if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) == -1)
            error_handling("connect() error");
        write(sock,&my_recv,sizeof(recv_d));
    }
}   

void *connectRecv(void *arg){
    int clnt_sock = *((int *)arg);
    recv_d recv_temp;
    read(clnt_sock,&recv_temp,sizeof(recv_d));
    pthread_mutex_lock(&mutex);
    recv_temp.id = id;
    recv_set[id++] = recv_temp;
    pthread_mutex_unlock(&mutex);
    /*
    pthread_mutex_lock(&mutex);
    for(int i=0; i<clnt_cnt; i++){
        if(clnt_sock == clnt_socks[i])
        {
            while (i<clnt_cnt-1){
                clnt_socks[i] = clnt_socks[i+1];
                i++;
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutex);
    close(clnt_sock);
    printf("Closed client %d\n",clnt_sock);*/
    return NULL;
}