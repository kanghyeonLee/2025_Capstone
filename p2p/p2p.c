#include "p2p_lib.h"

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
int send_other_socks[MAX_CLNT];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int num_recv_peer;
recv_d *recv_set;
int id = 0;

int main(int argc, char *argv[]){
    //변수 선언
    peer_d *peer = calloc(1,sizeof(peer_d));
    //명령줄 인자 다루는 과정
    optargHandler(argc,argv,peer);
    num_recv_peer = peer->max_num_rp;
    recv_set = calloc(num_recv_peer,sizeof(recv_d));
    //Sendig Peer part
    if(peer->peer_flag == SENDING_PEER){
        //sending peer 변수 선언
        pthread_t t_id;
        int serv_sock,clnt_sock;
        int i,option = 1;
        struct sockaddr_in serv_adr,clnt_adr;
        socklen_t clnt_adr_sz;
        char ack;
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
            pthread_create(&t_id,NULL,readDataFromRecv,(void *)&clnt_sock);
            pthread_join(t_id,NULL);
            printf("Connected client %d\n",clnt_sock);
        }
        // clnt socket에게 전달
        for(i=0; i<peer->max_num_rp; i++){
            printf("%d. %s %d %d\n",recv_set[i].id,recv_set[i].ip_address,recv_set[i].port,id);
            pthread_create(&t_id,NULL,sendRecvData,(void *)&clnt_socks[i]);
            pthread_join(t_id,NULL);
        }

        fd_set reads,temps;
        int cnt = 0,fd_max,fd_num;
        struct timeval timeout;
        FD_ZERO(&reads);
        FD_SET(serv_sock,&reads);
        fd_max = serv_sock;
        for(i=0; i<peer->max_num_rp; i++){
            FD_SET(clnt_socks[i],&reads);
            fd_max = fd_max < clnt_socks[i] ? clnt_socks[i] : fd_max;
        }
        while (cnt < 3)
        {
            temps = reads;
            timeout.tv_sec = 5;
            timeout.tv_usec = 0;
            if((fd_num = select(fd_max+1,&temps,0,0,&timeout)) == -1)
                break;
            if(fd_num == 0)
                continue;
            for(i=0; i<fd_max + 1; i++){
                if(FD_ISSET(i,&temps)){
                    if(i != serv_sock){
                        printf("%d\n",i);
                        read(i,&ack,sizeof(char));
                        FD_CLR(i,&reads);
                        cnt++;
                    }
                }
            }
        }
        
        for(int i=0; i<peer->max_num_rp; i++){
            write(clnt_socks[i],&ack,sizeof(char));
        }
        close(serv_sock);
        for(int i=0; i<num_recv_peer; i++)
        close(clnt_socks[i]);
    }else{//Receiving Peer part
        //변수선언
        int sock;
        struct sockaddr_in serv_addr;
        recv_d my_recv;
        int i;
        char ack;
        // my_recv 설정
        //getIpAddress(my_recv.ip_address);
        strcpy(my_recv.ip_address,"127.0.0.1");
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
        //Sending peer에게 데이터 전달
        write(sock,&my_recv,sizeof(recv_d));
        read(sock,&my_recv.id,sizeof(int));
        //서버 열기
        int my_serv_sock;
        struct sockaddr_in my_serv_adr;
        int option = 1;
        my_serv_sock = socket(PF_INET,SOCK_STREAM,0);
        if(my_serv_sock == -1)
            error_handling("socket() error");
        memset(&my_serv_adr,0,sizeof(my_serv_adr));
        my_serv_adr.sin_family = AF_INET;
        my_serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
        my_serv_adr.sin_port = htons(peer->listen_port);
        setsockopt(my_serv_sock,SOL_SOCKET,SO_REUSEADDR, &option, sizeof(option));
        if(bind(my_serv_sock, (struct sockaddr*) &my_serv_adr, sizeof(my_serv_adr)) == -1)
            error_handling("bind() error");
        if(listen(my_serv_sock,5)==-1)
            error_handling("listen() error");
        pthread_t t_id;
        pthread_create(&t_id,NULL,connectOtherRecv,&my_serv_sock);
        pthread_detach(t_id);
        //Sending peer에게 다른 receiving peer정보 받기
        read(sock,&num_recv_peer,sizeof(int));
        recv_d my_recv_set[num_recv_peer];
        int other_clnt[num_recv_peer];
        write(sock,&ack,sizeof(char));
        read(sock,my_recv_set,sizeof(my_recv_set));
        //다른 소켓들과 연결
        for(i=0; i<num_recv_peer; i++){
            if(i != my_recv.id){
                int sock = socket(PF_INET,SOCK_STREAM,0);
                send_other_socks[i] = sock;
                if(sock == -1)
                    error_handling("socket() error");
                struct sockaddr_in serv_addr;
                memset(&serv_addr,0,sizeof(serv_addr));
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_addr.s_addr = inet_addr(my_recv_set[i].ip_address);
                serv_addr.sin_port = htons(my_recv_set[i].port);
                if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) == -1)
                    error_handling("connect() error");
            }
        }
        write(sock,&ack,sizeof(char));
        read(sock,&ack,sizeof(char));
        
    }

    free(peer);
    free(recv_set);
}   

void *readDataFromRecv(void *arg){
    int clnt_sock = *((int *)arg);
    recv_d recv_temp;
    read(clnt_sock,&recv_temp,sizeof(recv_d));
    write(clnt_sock,&id,sizeof(int));
    write(clnt_sock,&num_recv_peer,sizeof(int));
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

void *sendRecvData(void *arg){
    int clnt_sock = *((int*)arg);
    char ack;
    read(clnt_sock,&ack,sizeof(char));
    write(clnt_sock,recv_set,sizeof(recv_d)*num_recv_peer);
    return NULL;
}

void *connectOtherRecv(void *arg){
    int my_serv_sock = *((int *)arg);
    for(int i=0; i<num_recv_peer-1; i++){
        int clnt_sock;
        struct sockaddr_in clnt_adr;
        socklen_t clnt_adr_sz;
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(my_serv_sock,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
        pthread_mutex_lock(&mutex);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}
