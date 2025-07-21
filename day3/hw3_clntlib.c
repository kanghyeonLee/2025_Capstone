#include "hw3_lib.h"

void error_handling(char *message){
    printf("%s\n",message);
    exit(1);
}

void printFileList(pkt_t **file_list, int length, int sock){
    printf("\nThe File List of %d Server\n",sock);
   
    for(int i=0; i<length; i++){
        read(sock,file_list[i],sizeof(pkt_t));
    }
    for(int i=0; i<length; i++){
        if(!file_list[i]->directory)
            printf("%d. %s %dbytes type: file\n",i+1,file_list[i]->file_name,file_list[i]->file_size);
        else
            printf("%d. %s %dbytes type: directory\n",i+1,file_list[i]->file_name,file_list[i]->file_size);
    }
    printf("\n");

}

int handlingCmd_client(char *cmd, cmd_d* cmd_data){
    if(!strcmp(cmd,"help")){
        cmd_data->mode = 0;
        strcpy(cmd_data->cmd1,cmd);
    }else if(!strcmp(cmd,"exit")){
        cmd_data->mode = 1;
        strcpy(cmd_data->cmd1,cmd);
    }
    else if(!strcmp(cmd,"ls")){
        cmd_data->mode = 2;
        strcpy(cmd_data->cmd1,cmd);
    }
    else if(!strcmp(cmd,"ufile")){
        cmd_data->mode = 5;
        strcpy(cmd_data->cmd1,cmd);
    }
    else{
        char tmp_cmd[BUF_SIZE];
        strcpy(tmp_cmd,cmd);
        char *tmp = strtok(tmp_cmd," ");
        strcpy(cmd_data->cmd1,tmp);
        tmp = strtok(NULL," ");
        strcpy(cmd_data->cmd2,tmp);
        if(!strcmp(cmd_data->cmd1,"cd")){
            cmd_data->mode = 3;
        }else if(!strcmp(cmd_data->cmd1,"dfile")){
            cmd_data->mode = 4;
        }else cmd_data->mode = -1;
    }
    return cmd_data->mode;
}

int viewFileList_client(pkt_t **send_pkt){
    int numFile = 0;
    DIR* dp = opendir(".");
    struct dirent *d;
    if(dp == NULL)
        error_handling("opendir() error");
    while ((d = readdir(dp)) != NULL)
    {
        if(!strcmp(d->d_name,".") || !strcmp(d->d_name,".."))
            continue;
        memset(send_pkt[numFile],0,sizeof(pkt_t));
        struct stat file_stat;
        if(stat(d->d_name,&file_stat) == -1)
            error_handling("stat() error");
        if(d->d_name[strlen(d->d_name)-1] == '\n')
            d->d_name[strlen(d->d_name)-1] = '\0';
        send_pkt[numFile]->file_size = file_stat.st_size;
        send_pkt[numFile]->directory = S_ISDIR(file_stat.st_mode)? 1 : 0;
        strcpy(send_pkt[numFile]->file_name,d->d_name);
        numFile++;
    }
    closedir(dp);
    return numFile;
}

int dfile_func(int sock, cmd_d *cmd_data, pkt_t**file_list){
    int file_size;
    read(sock,&file_size,sizeof(unsigned int));
    if(file_size == -1) return 0;
    int index = atoi(cmd_data->cmd2);
    char name[BUF_SIZE] = "tmp_";
    strcat(name,file_list[index-1]->file_name);
    FILE *fp = fopen(name,"wb");
    if(fp == NULL)
        error_handling("fopen error()");
    int read_len = 0;
    int temp_len;
    char buf[BUF_SIZE];
    while (read_len < file_list[index-1]->file_size)
    {
        temp_len = read(sock,buf,BUF_SIZE);
        fwrite(buf,1,temp_len,fp);
        read_len += temp_len;
    }
    fclose(fp);
    return 1;
}

void ufile_func(int clnt_file_length,pkt_t** clnt_file,int sock){
    clnt_file_length = viewFileList_client(clnt_file);
    printf("\nThe File List of Local\n");
    for(int i=0; i<clnt_file_length; i++){
        printf("%d. %s %dbytes\n",i+1,clnt_file[i]->file_name,clnt_file[i]->file_size);
    }
    write(sock,&clnt_file_length,sizeof(int));
    for(int j=0; j<clnt_file_length; j++)
        write(sock,clnt_file[j],sizeof(pkt_t));
    int clnt_index;
    do{
    printf("\nPlease enter the appropriate index : ");
    scanf("%d",&clnt_index);
    getchar();
    }while (clnt_index < 1 || clnt_index > clnt_file_length);
    
    write(sock,&clnt_index,sizeof(int));
    FILE *fp = fopen(clnt_file[clnt_index-1]->file_name,"rb");
    if(fp == NULL)
        error_handling("fopen error()");
    int temp_len;
    char buf[BUF_SIZE];
    while (1)
    {
        temp_len = fread(buf,1,BUF_SIZE,fp);
        if(temp_len < BUF_SIZE){
            write(sock,buf,temp_len);
            break;
        }
        write(sock,buf,BUF_SIZE);
    }
    fclose(fp);
}

