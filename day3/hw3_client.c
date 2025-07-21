#include "hw3_lib.h"

int clnt_file_length = FILE_LENGTH;
int length = FILE_LENGTH;

int main(int argc, char *argv[]){
    int sock;
    struct sockaddr_in serv_addr;
    int i;
    pkt_t** file_list,**clnt_file;
    pkt_data *data;
    cmd_d *cmd_data;
    char cwd[BUF_SIZE];
    char result[BUF_SIZE];

    if(argc != 3){
        printf("Usage: %s IP <port>\n",argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    if(connect(sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    data = malloc(sizeof(pkt_data));
    memset(data,0,sizeof(pkt_data));
    cmd_data = malloc(sizeof(cmd_d));
    memset(cmd_data,0,sizeof(cmd_d));

    file_list = malloc(sizeof(pkt_t*) * FILE_LENGTH);
    clnt_file = malloc(sizeof(pkt_t*) * FILE_LENGTH);
    for(i=0; i<FILE_LENGTH; i++){
        file_list[i] = malloc(sizeof(pkt_t));
        clnt_file[i] = malloc(sizeof(pkt_t));
        memset(file_list[i],0,sizeof(pkt_t));
        memset(clnt_file[i],0,sizeof(pkt_t));
    }

    data->mode=0;
    write(sock,data,sizeof(pkt_data));
    read(sock,&length,sizeof(int));
    read(sock,cwd,sizeof(cwd));
    printf("\nCurrent Directory: %s\n",cwd);
    printFileList(file_list,length,sock);
    printf("Execute command ('exit' command is to quit the process)\n");
    data->mode=1;
    while (strcmp(data->cmd,"exit"))
    {
        write(sock,data,sizeof(pkt_data));
        printf("cmd> ");
        fgets(data->cmd,sizeof(data->cmd),stdin);
        data->cmd[strlen(data->cmd)-1] = '\0';
        int cmd_mode = handlingCmd_client(data->cmd,cmd_data);
        write(sock,cmd_data,sizeof(cmd_data));
        int c;
        read(sock,&c,sizeof(int));
        write(sock,cmd_data->cmd2,sizeof(cmd_data->cmd2));
        if(cmd_mode == 0 || cmd_mode == 1){
            read(sock,result,sizeof(result));
            printf("%s",result);
        }else if(cmd_mode == 2){
            read(sock,&length,sizeof(int));
            read(sock,cwd,BUF_SIZE);
            int c;
            write(sock,&c,sizeof(int));
            printf("\nCurrent Directory: %s\n",cwd);
            printFileList(file_list,length,sock);
        }else if(cmd_mode == 3){
            read(sock,result,sizeof(result));
            printf("%s",result);
        }else if(cmd_mode == 4){
            read(sock,result,sizeof(result));
            printf("%s\n",result);
            int flag = dfile_func(sock,cmd_data,file_list);
            if(flag == 0) continue;
            char end;
            write(sock,&end,sizeof(char));
        }else if(cmd_mode == 5){
            ufile_func(clnt_file_length,clnt_file,sock);
            read(sock,result,sizeof(result));
            printf("%s",result);
        }else{
            read(sock,result,sizeof(result));
            printf("%s",result);
        }
    }
    data->mode=2;
    write(sock,data,sizeof(pkt_data));
    close(sock);
    for(i=0; i<length; i++){
        free(file_list[i]);
    }
    for(i=0; i<clnt_file_length; i++)
        free(clnt_file[i]);
    free(file_list);
    free(clnt_file);
    free(data);
    return 0;
}