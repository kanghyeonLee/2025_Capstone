#include "hw4_lib.h"

void error_handling(char *msg){
    perror(msg);
    exit(1);
}
