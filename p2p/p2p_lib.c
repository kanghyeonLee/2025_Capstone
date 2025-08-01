#include "p2p_lib.h"

void optargHandler(int argc, char *argv[],peer_d *peer){
    int opt;
    while ((opt = getopt(argc,argv,"srn:f:g:a:p:")) != -1)
    {
        switch (opt)
        {
        case 's':
            peer->peer_flag = SENDING_PEER;
            break;
        case 'r':
            peer->peer_flag = RECEIVING_PEER;
            break;
        case 'n':
            if(peer->peer_flag) peer->max_num_rp = atoi(optarg);
            else{
                printf("This '%c' option is only for sending peer.\n",optopt);
                exit(1);
            }
            break;
        case 'f':
            if(peer->peer_flag) peer->file_name = optarg;
            else{
                printf("This '%c' option is only for sending peer.\n",optopt);
                exit(1);
            }
            break;
        case 'g':
            if(peer->peer_flag) peer->segment_size = (size_t)atoi(optarg);
            else{
                printf("This '%c' option is only for sending peer.\n",optopt);
                exit(1);
            }
            break;
        case 'a':
            if(!peer->peer_flag){
                peer->sending_ip_address = optarg;
                peer->sending_port = atoi(argv[optind]);
            }else{
                printf("This '%c' option is only for receiving peer.\n",optopt);
                exit(1);
            }
            break;
        case 'p':
            peer->listen_port = atoi(optarg);
            break;
        case '?':
            printf("No option: %c",optopt);
            exit(1);
            break;
        }
    }
}

void error_handling(char *msg){
    perror(msg);
    exit(1);
}

void getIpAddress(char *recv_host){
    struct ifaddrs *ifaddr, *temp;
    int family,info;
    char host[NI_MAXHOST];
    if(getifaddrs(&ifaddr) == -1)
        error_handling("getifaddrs() error");
    for(temp = ifaddr; temp != NULL; temp = temp->ifa_next){
        if(temp->ifa_addr == NULL)
            continue;
        family = temp->ifa_addr->sa_family;
        if(family == AF_INET){
            info = getnameinfo(temp->ifa_addr,sizeof(struct sockaddr_in),host,NI_MAXHOST,NULL,0,NI_NUMERICHOST);
            if(info != 0)
                error_handling("getnameinfo() error");
            if(!strcmp(temp->ifa_name,"enp94s0f0np0")){
                strcpy(recv_host,host);
            }
        }
    }
    freeifaddrs(ifaddr);
}
