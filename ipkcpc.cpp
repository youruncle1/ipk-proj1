/*
    IPK project 1 - IPK Calculator Protocol
    author: Roman Poliacik
    login: xpolia05
*/
#include "ipkcpc_functions.h"

int client_socket; //client socket #
char *mode; //connection mode

int main(int argc, char* argv[]) {
    char *host; //host address (IPv4)
    char *port; //port #

    parameter_check(argc, argv, &host, &port, &mode);
    
    if(strcmp(mode, "tcp") == 0)
        connect_tcp(&host, &port);
    else
        connect_udp(&host, &port);
    
    return 0;
}