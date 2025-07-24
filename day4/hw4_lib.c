#include "hw4_lib.h"

void error_handling(char *msg){
    perror(msg);
    exit(1);
}

void merge(search_d** search_data, int left, int mid, int right){
  int i, j, k, l;
  i = left;
  j = mid+1;
  k = left;
  search_d **temp = calloc(MAX,sizeof(search_d*));
  for(int t = 0; t<MAX; t++) temp[t] = calloc(1,sizeof(search_d));
  while(i<=mid && j<=right){
    if(search_data[i]->search_num>=search_data[j]->search_num)
      temp[k++] = search_data[i++];
    else
      temp[k++] = search_data[j++];
  }

  if(i>mid){
    for(l=j; l<=right; l++)
      temp[k++] = search_data[l];
  }else{
    for(l=i; l<=mid; l++)
      temp[k++] = search_data[l];
  }

  for(l=left; l<=right; l++)
    search_data[l] = temp[l];
}

void merge_sort(search_d** search_data, int left, int right){
  int mid;

  if(left<right){
    mid = (left+right)/2;
    merge_sort(search_data, left, mid); 
    merge_sort(search_data, mid+1, right); 
    merge(search_data, left, mid, right);
  }
}