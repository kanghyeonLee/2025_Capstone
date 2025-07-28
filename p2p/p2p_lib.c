#include "p2p_lib.h"

void optargHandler(int argc, char *argv[],peer_d *peer){
    int opt;
    while ((opt = getopt(argc,argv,"srn:f:g:a:p:")) != -1)
    {
        switch (opt)
        {
        case 's':
            peer->peer_flag = 1;
            break;
        case 'r':
            peer->peer_flag = 0;
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
            if(peer->peer_flag) peer->segment_size = atoi(optarg);
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