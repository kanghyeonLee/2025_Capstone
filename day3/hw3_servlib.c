#include "hw3_lib.h"

void error_handling(char *message){
    printf("%s\n",message);
    exit(1);
}

int viewFileList_server(int clnt_sock, pkt_t **send_pkt, char current_d[BUF_SIZE][BUF_SIZE]){
    int numFile = 0;
    DIR* dp = opendir(current_d[clnt_sock]);
    struct dirent *d;
    if(dp == NULL)
        error_handling("opendir() error");
    while ((d = readdir(dp)) != NULL)
    {
        char name[BUF_SIZE];
        sprintf(name,"%s/%s",current_d[clnt_sock],d->d_name);
        if(!strcmp(d->d_name,".") || !strcmp(d->d_name,".."))
            continue;
        memset(send_pkt[numFile],0,sizeof(pkt_t));
        struct stat file_stat;
        if(stat(name,&file_stat) == -1)
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

void handlingCmd_server(cmd_d* data, int i,pkt_t **file_list,pkt_t **clnt_file,char current_d[BUF_SIZE][BUF_SIZE], int *length_of_list, int *length){
    char result[BUF_SIZE];
    if(data->mode == 0){
        strcpy(result,"\n\tcd <directory>: change current directory to <directory>\n\tls : print file list from current directory\n\tdfile <file index num>: download file for index\n\tufile: upload file from my current directory\n\n");
        write(i,result,BUF_SIZE);
    }else if(data->mode == 1){
        strcpy(result,"");
        write(i,result,BUF_SIZE);
    }
    else if(data->mode == 2){
        *length_of_list = viewFileList_server(i,file_list,current_d);
        write(i,length_of_list,sizeof(int));
        write(i,current_d[i],strlen(current_d[i]));
        int c;
        read(i,&c,sizeof(c));
        for(int j=0; j<*length_of_list; j++){
            write(i,file_list[j],sizeof(pkt_t));
        }

    }
    else{
        if(data->mode == 3){
            if(!strcmp(data->cmd2,"")){
                strcpy(result,"Change Directory Fail..\n");
            }else{
                if(strlen(data->cmd2) >=2){
                    if(current_d[i][strlen(current_d[i])-1] == '/') current_d[i][strlen(current_d[i])-1] = '\0';
                    if(strstr(data->cmd2,"../") != NULL){
                        sprintf(current_d[i],"%s/%s",current_d[i],data->cmd2);
                    }else if(data->cmd2[0] == '.' && data->cmd2[1] == '/'){
                        char tmp[BUF_SIZE];
                        for(int j=1; j<strlen(data->cmd2); j++)
                            tmp[j-1] = data->cmd2[j];
                        strcat(current_d[i],tmp);
                    }else if(data->cmd2[0] == '/'){
                        strcpy(current_d[i],data->cmd2);
                    }else{
                        sprintf(current_d[i],"%s/%s",current_d[i],data->cmd2);
                    }
                }else{
                    if(data->cmd2[0] == '/'){
                        strcpy(current_d[i],data->cmd2);
                    }else{
                        strcpy(result,"Change Directory Fail..\n");
                    }
                }
                if(!chdir(current_d[i])) strcpy(result,"Change Directory success\n");
                else{
                    getcwd(current_d[i],sizeof(current_d[i]));
                    strcpy(result,"Change Directory Fail..\n");
                }
            }
            write(i,result,BUF_SIZE);
        }else if(data->mode == 4){
            int index = atoi(data->cmd2);
            if(index < 1 || index > *length_of_list){
                strcpy(result,"Wrong file index!");
                write(i,result,BUF_SIZE);
                unsigned int tmp = -1;
                write(i,&tmp,sizeof(unsigned int));
            }else{
                strcpy(result,"Start file download!");
                write(i,result,BUF_SIZE);
                write(i,&file_list[index-1]->file_size,sizeof(unsigned int));
                FILE *fp = fopen(file_list[index-1]->file_name,"rb");
                if(fp == NULL)
                    error_handling("fopen error()");
                int temp_len;
                char buf[BUF_SIZE];
                while (1)
                {
                    temp_len = fread(buf,1,BUF_SIZE,fp);
                    if(temp_len < BUF_SIZE){
                        write(i,buf,temp_len);
                        break;
                    }
                    write(i,buf,BUF_SIZE);
                }
                fclose(fp);
                char end;
                read(i,&end,sizeof(char));
            }
        }else if(data->mode == 5){
            read(i,length,sizeof(int));
            for(int j=0; j<*length; j++){
                read(i,clnt_file[j],sizeof(pkt_t));
            }
            char tmp[BUF_SIZE];
            int index;
            read(i,&index,sizeof(int));
            strcat(tmp,clnt_file[index-1]->file_name);
            sprintf(tmp,"%s/temp_%s",current_d[i],clnt_file[index-1]->file_name);
            FILE *fp = fopen(tmp,"wb");
            if(fp == NULL)
                error_handling("fopen error()");
            int read_len = 0;
            int temp_len;
            char buf[BUF_SIZE];
            while (read_len < clnt_file[index-1]->file_size)
            {
                temp_len = read(i,buf,BUF_SIZE);
                fwrite(buf,1,temp_len,fp);
                read_len += temp_len;
            }
            fclose(fp);
            strcpy(result,"Upload Success!\n");
            write(i,result,sizeof(result));
        }else{
            strcpy(result,"Undefined command!\n");
            write(i,result,BUF_SIZE);
        }
    }
}

