/*
    IPK project 1 - IPK Calculator Protocol
    author: Roman Poliacik
    login: xpolia05
*/
#ifndef IPKCPC_FUNCTIONS_H
#define IPKCPC_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <netdb.h>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

#define BUFSIZE 255
#define MAX_RESPONSE_SIZE 259

extern int client_socket;
extern char *mode;

void sigint_handler(int sig);
void parameter_check(int argc, char* argv[],char **host,char **port,char **mode);
void addr_soc(char *host, char *port, struct hostent **server, struct sockaddr_in *server_address);
void connect_udp(char **host,char **port);
void connect_tcp(char **host,char **port);
void exit_tcp_gracefully(int client_socket);

#endif /* IPKCPC_FUNCTIONS_H */