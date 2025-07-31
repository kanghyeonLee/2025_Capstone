#include "p2p_lib.h"

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT] = {0};
int send_other_socks[MAX_CLNT] = {0};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int num_recv_peer;
recv_d *recv_set;
int id = 0;
FILE *fp;
int size;
int total_file_size;
file_d file;
int total_segment_num;
int final_segment_size;

int main(int argc, char *argv[]){
    //변수 선언
    peer_d *peer = calloc(1,sizeof(peer_d));
    //명령줄 인자 다루는 과정
    optargHandler(argc,argv,peer);
    num_recv_peer = peer->max_num_rp;
    recv_set = calloc(num_recv_peer,sizeof(recv_d));
    size = peer->segment_size*KB;
    //Sendig Peer part
    if(peer->peer_flag == SENDING_PEER){
        //sending peer 변수 선언
        pthread_t t_id[num_recv_peer];
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
        for(i=0; i<num_recv_peer; i++){
            clnt_adr_sz = sizeof(clnt_adr);
            clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
            pthread_mutex_lock(&mutex);
            clnt_socks[clnt_cnt++] = clnt_sock;
            pthread_mutex_unlock(&mutex);
            pthread_create(&t_id[i],NULL,readDataFromRecv,(void *)&clnt_sock);
            printf("Connected client %d\n",clnt_sock);
        }
        for(i=0; i<num_recv_peer; i++){
            pthread_join(t_id[i],NULL);
        }
        // clnt socket에게 전달
        for(i=0; i<num_recv_peer; i++){
            pthread_create(&t_id[i],NULL,sendRecvData,(void *)&clnt_socks[i]);
        }
        for(i=0; i<num_recv_peer; i++){
            pthread_join(t_id[i],NULL);
        }

        fd_set reads,temps;
        int cnt = 0,fd_max,fd_num;
        struct timeval timeout;
        FD_ZERO(&reads);
        FD_SET(serv_sock,&reads);
        fd_max = serv_sock;
        for(i=0; i<num_recv_peer; i++){
            FD_SET(clnt_socks[i],&reads);
            fd_max = fd_max < clnt_socks[i] ? clnt_socks[i] : fd_max;
        }
        while (cnt < num_recv_peer)
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
                        read(i,&ack,sizeof(char));
                        cnt++;
                    }
                }
            }
        }
        DIR* dp = opendir(".");
        struct dirent *d;
        if(dp == NULL)
            error_handling("opendir() error");
        while ((d = readdir(dp)) != NULL)
        {
            if(strcmp(d->d_name,peer->file_name))
                continue;
            struct stat file_stat;
            if(stat(d->d_name,&file_stat) == -1)
                error_handling("stat() error");
            total_file_size = file_stat.st_size;
            break;
        }
        closedir(dp);
        strcpy(file.filename,peer->file_name);
        file.file_size = total_file_size;
        file.segment_size = peer->segment_size*KB;
        for(int i=0; i<num_recv_peer; i++){
            pthread_create(&t_id[i],NULL,sendFileData,&clnt_socks[i]);
        }
        for(int i=0; i<num_recv_peer; i++){
            pthread_join(t_id[i],NULL);
        }
        //데이터 송신과정
        fp = fopen(peer->file_name,"rb");
        if(fp == NULL)
            error_handling("fopen() error");
        total_segment_num = total_file_size / size;
        final_segment_size = total_file_size % size;
        if(final_segment_size != 0) total_segment_num ++;
        int read_len = 0,write_len;
        for(i=0; i<total_segment_num; i++){
            char segment[size];
            if(i < total_segment_num - 1) read_len = fread(segment,sizeof(char),size, fp);
            else{
                if(final_segment_size == 0) read_len = fread(segment,sizeof(char),size, fp);
                else read_len = fread(segment,sizeof(char),final_segment_size,fp);
            }
            write(clnt_socks[i%num_recv_peer],&read_len,sizeof(int));
            write_len = 0;
            while (write_len < read_len)
            {
                write_len += write(clnt_socks[i%num_recv_peer],segment,read_len*sizeof(char));
            }
            read(clnt_socks[i%num_recv_peer],&ack,sizeof(char));
        }
        
        printf("Sending Peer [");
        for(i = 0; i < 25; i++){

        }
        cnt = 0;
        while (cnt < num_recv_peer)
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
                        read(i,&ack,sizeof(char));
                        FD_CLR(i,&reads);
                        cnt++;
                    }
                }
            }
        }
        for(i=0; i<num_recv_peer; i++)
            write(clnt_socks[i],&ack,sizeof(char));
        //닫기
        for(i=0; i<num_recv_peer; i++){
            close(clnt_socks[i]);
            printf("Closed client %d\n",clnt_socks[i]);
        }
        close(serv_sock);
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
        read(sock,&num_recv_peer,sizeof(int));
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
        //Sending peer에게 다른 receiving peer정보 받기
        recv_d my_recv_set[num_recv_peer];
        write(sock,&ack,sizeof(char));
        read(sock,my_recv_set,sizeof(my_recv_set));
        pthread_create(&t_id,NULL,connectOtherRecv,&my_serv_sock);
        //다른 소켓들과 연결
        for(i=0; i<num_recv_peer; i++){
            if(i != my_recv.id){
                int send_sock = socket(PF_INET,SOCK_STREAM,0);
                send_other_socks[i] = send_sock;
                if(send_sock == -1)
                    error_handling("socket() error");
                struct sockaddr_in serv_addr;
                memset(&serv_addr,0,sizeof(serv_addr));
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_addr.s_addr = inet_addr(my_recv_set[i].ip_address);
                serv_addr.sin_port = htons(my_recv_set[i].port);
                if(connect(send_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) == -1)
                    error_handling("connect() error");
                write(send_sock,&my_recv.id,sizeof(int));
            }
        }
        pthread_join(t_id,NULL);
        write(sock,&ack,sizeof(char));
        //데이터 송신
        read(sock,&file,sizeof(file_d));
        char temp[KB]; 
        write(sock,&ack,sizeof(char));
        read(sock,temp,sizeof(char)*KB);
        strcpy(file.filename,temp);
        size = file.segment_size;
        total_file_size = file.file_size;
        char file_name[KB+10];
        total_segment_num = total_file_size / size;
        final_segment_size = total_file_size % size;
        if(final_segment_size != 0) total_segment_num ++;
        pthread_t recv_tid[num_recv_peer];
        for(i=0; i<num_recv_peer; i++){
            if(i != my_recv.id){
                int *data = calloc(2,sizeof(int));
                data[0] = i;
                data[1] = my_recv.id;
                pthread_create(&recv_tid[i],NULL, recvSegmentToMe,data);
            }
        }
        int read_len,total_read_size=0,write_len;
        int len;
        char segment[size];
        for(i=my_recv.id; i<total_segment_num; i+=num_recv_peer){   
            read(sock,&total_read_size,sizeof(int));
            read_len = 0, len = 0;
            while (read_len < total_read_size)
            {
                len = read(sock,segment+read_len,total_read_size-read_len);
                if(len < 0) error_handling("read() error");
                read_len += len;
            }
            sprintf(file_name,"/tmp/%d/%s_%d",my_recv.id,file.filename,i);
            FILE *fp = fopen(file_name,"wb");
            if(fp == NULL)
                error_handling("fopen() error");
            write_len = 0, len = 0;
            while (write_len < total_read_size)
            {
                len = fwrite(segment+write_len,sizeof(char),total_read_size-write_len,fp);
                if(len <= 0) error_handling("write() error");
                write_len += len;
            }
            fclose(fp);
            for(int j = 0; j < num_recv_peer; j++){
                if(j != my_recv.id){
                    write(send_other_socks[j],&total_read_size,sizeof(int));
                    read(send_other_socks[j],&ack,sizeof(char));
                    write_len = 0, len = 0;
                    while (write_len < total_read_size)
                    {
                       len = write(send_other_socks[j],segment,total_read_size);
                       if(len <= 0) error_handling("write() error");
                       write_len += len;
                    }
                    read(send_other_socks[j],&ack,sizeof(char));
                }
            }
            write(sock, &ack, sizeof(char));
        }
        for(i=0; i<num_recv_peer; i++){
            if(i != my_recv.id) pthread_join(recv_tid[i],NULL);
        }
        sprintf(file_name,"%s_%d",file.filename,my_recv.id);
        FILE *fp = fopen(file_name,"wb");
        if(fp == NULL) error_handling("fopen error()");
        for(i=0; i<total_segment_num; i++){
            char tmp_file[KB+10];
            char segment[size];
            read_len = 0, len = 0;
            sprintf(tmp_file,"/tmp/%d/%s_%d",my_recv.id,file.filename,i);
            FILE *tmp = fopen(tmp_file,"rb");
            if(tmp == NULL) error_handling("fopen() error");
            if(i < total_segment_num - 1){
                while (read_len < size){
                    len = fread(segment+read_len,sizeof(char),size-read_len,tmp);
                    if(len < 0)  error_handling("fwrite() error");
                    read_len += len;
                }
            }else{
                if(final_segment_size == 0){
                     while(read_len < size){
                        len = fread(segment+read_len,sizeof(char),size-read_len,tmp);
                        if(len < 0)  error_handling("fwrite() error");
                        read_len += len;
                     }
                }else{
                    while (read_len < final_segment_size){
                        len = fread(segment+read_len,sizeof(char),final_segment_size-read_len,tmp);
                        if(len < 0)  error_handling("fwrite() error");
                        read_len += len;
                    }
                }
            }
            write_len = 0, len = 0;
            while (write_len < read_len)
            {
                len = fwrite(segment+write_len,sizeof(char),read_len-write_len,fp);
                if(len <= 0)  error_handling("fwrite() error");
                write_len += len;
            }
            fclose(tmp);
            remove(tmp_file);
        }
        fclose(fp);

        for(i=0; i<num_recv_peer; i++){
            if(i != my_recv.id){
                close(clnt_socks[i]);
                close(send_other_socks[i]);
            }
            else close(my_serv_sock);
        }
        close(sock);
       
    }

    free(peer);
    free(recv_set);
} 

void *sendFileData(void *arg){
    char ack;
    int clnt_sock = *((int *)arg);
    write(clnt_sock,&file,sizeof(file_d));
    read(clnt_sock,&ack,sizeof(char));
    write(clnt_sock,&file.filename,strlen(file.filename)+1);
    return NULL;
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
    int id;
    for(int i=0; i<num_recv_peer-1; i++){
        int clnt_sock;
        struct sockaddr_in clnt_adr;
        socklen_t clnt_adr_sz;
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(my_serv_sock,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
        read(clnt_sock,&id,sizeof(int));
        pthread_mutex_lock(&mutex);
        clnt_socks[id] = clnt_sock;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}


//Receiving peer 한 개가 특정 sock한테 수신받는 함수 
void *recvSegmentToMe(void *arg){
    int index = ((int *) arg)[0];
    int clnt_sock = clnt_socks[index];
    int read_len,total_size,write_len,len;
    char segment[size],ack;
    int i;
    int my_id = ((int*)arg)[1];
    char file_name[KB+10];
    for(i = index; i < total_segment_num; i+=num_recv_peer){
        read(clnt_sock,&total_size,sizeof(int));
        write(clnt_sock,&ack,sizeof(char));
        read_len = 0;
        memset(segment,0,size);
        while (read_len < total_size)
        {
           len = read(clnt_sock,&segment[read_len],total_size-read_len);
           if(len < 0) error_handling("read() error");
           read_len += len;
        }
        sprintf(file_name,"/tmp/%d/%s_%d",my_id,file.filename,i);
        FILE *fp = fopen(file_name,"wb");
        if(fp == NULL){
            printf("%s\n",file_name);
            error_handling("fopen() error");
        }
        write_len = 0;
        while (write_len < total_size)
        {
            len = fwrite(segment+write_len,sizeof(char),total_size-write_len,fp);
            if(len <= 0) error_handling("read() error");
            write_len += len;
        }
        fclose(fp);
        write(clnt_sock,&ack,sizeof(char));
    }
    free(arg);
    return NULL;
}
