#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUF_SIZE 1024
#define MAX 30

typedef struct 
{   
    unsigned int num_s_word;
    char search_word[MAX][BUF_SIZE];
    unsigned int search_num;
}search_d;

void error_handling(char *msg);
void merge(search_d** search_data, int left, int mid, int right);
void merge_sort(search_d** search_data, int left, int right);