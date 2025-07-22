#include "hw3_lib.h"


int main(int argc, char *argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr,clnt_addr;
    struct timeval timeout;
    fd_set reads, cpy_reads;
    pkt_t** file_list,**clnt_file;
    pkt_data *data;
    cmd_d *cmd_data;
    char cwd[BUF_SIZE];
    char current_d[BUF_SIZE][BUF_SIZE];
    int *length_of_list = malloc(sizeof(int));
    int *length = malloc(sizeof(int));

    *length_of_list = FILE_LENGTH;
    *length = FILE_LENGTH;

    socklen_t adr_sz;
    int fd_max, fd_num, i;
    if(argc != 2){
        printf("Usage: %s <port>\n",argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    if(listen(serv_sock,5) == -1)
        error_handling("listen() error");
    
    FD_ZERO(&reads);
    FD_SET(serv_sock,&reads);
    fd_max = serv_sock;
    file_list = malloc(sizeof(pkt_t*) * FILE_LENGTH);
    clnt_file = malloc(sizeof(pkt_t*) * FILE_LENGTH);
    for(i=0; i<FILE_LENGTH; i++){
        file_list[i] = malloc(sizeof(pkt_t));
        clnt_file[i] = malloc(sizeof(pkt_t));
        memset(file_list[i],0,sizeof(pkt_t));
        memset(clnt_file[i],0,sizeof(pkt_t));
    }
    data = malloc(sizeof(pkt_data));
    memset(data,0,sizeof(pkt_data));
    cmd_data = malloc(sizeof(cmd_d));
    memset(cmd_data,0,sizeof(cmd_d));

    while (1)
    {
        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        if((fd_num=select(fd_max+1,&cpy_reads,0,0,&timeout)) == -1)
            break;
        if(fd_num == 0)
            continue;
        
        for(i=0; i<fd_max+1; i++)
        {
            if(FD_ISSET(i, &cpy_reads))
            {
                if(i == serv_sock)
                {
                    adr_sz = sizeof(clnt_addr);
                    clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_addr,&adr_sz);
                    FD_SET(clnt_sock,&reads);
                    fd_max = fd_max < clnt_sock ? clnt_sock : fd_max;
                    getcwd(current_d[clnt_sock],sizeof(current_d[clnt_sock]));
                    printf("connected client: %d\n",clnt_sock);
                }
                else
                {
                    read(i,cmd_data,sizeof(cmd_d));
                    int c;
                    write(i,&c,sizeof(int));
                    read(i,cmd_data->cmd2,sizeof(cmd_data->cmd2));
                    handlingCmd_server(cmd_data,i,file_list,clnt_file,current_d,length_of_list,length);
                   
                    if(!strcmp(cmd_data->cmd1,"exit")){
                        FD_CLR(i,&reads);
                        close(i);
                        printf("closed client: %d\n",i);
                    }
                    
                }
            }
        }
    }
    close(serv_sock);
    for(i=0; i<*length_of_list; i++){
        free(file_list[i]);
    }
    for(i=0; i<*length; i++)
        free(clnt_file[i]);
    free(file_list);
    free(clnt_file);
    free(data);
    free(length_of_list);
    free(length);
    return 0;
}
