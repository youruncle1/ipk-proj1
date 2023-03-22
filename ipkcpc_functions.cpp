/*
    IPK project 1 - IPK Calculator Protocol
    author: Roman Poliacik
    login: xpolia05
*/
#include "ipkcpc_functions.h"

void sigint_handler(int sig) {
    (void) sig;

    if(strcmp(mode, "tcp") == 0 ){
        exit_tcp_gracefully(client_socket);
        exit(EXIT_SUCCESS);
    }
    /* UDP: close socket */
    close(client_socket);
    exit(EXIT_SUCCESS);
}

void exit_tcp_gracefully(int client_socket){
    char buf[BUFSIZE];
    int bytestx, bytesrx;

    /* send BYE message to server */
    bytestx = send(client_socket, "BYE\n", strlen("BYE\n"), 0);
    if (bytestx < 0){
        perror("ERROR in sendto");
    }
    
    /* wait for response BYE message from server */
    bzero(buf, BUFSIZE);
    bytesrx = recv(client_socket, buf, BUFSIZE, 0);
    if (bytesrx < 0){
        perror("ERROR in recvfrom");
    }

    /* if server responded correctly, close connection */
    if (strcmp(buf, "BYE\n") == 0){
        printf("\n%s", buf);
        close(client_socket);
    }
    else{
        fprintf(stderr, "ERROR: did not receive ACK packet from server after sending FIN packet!\n");
        exit(EXIT_FAILURE);
    }
}

void parameter_check(int argc, char* argv[],char **host,char **port,char **mode) {
    int option;

    /* Check if there is correct amount of parameters */
    if (argc != 7) {
        fprintf(stderr, "Usage: ipkcpc -h <host> -p <port> -m <mode>\n");
        exit(EXIT_FAILURE);
    }

    /* Get parameters */
    while ((option = getopt(argc, argv, "h:p:m:")) != -1) {
        switch (option) {
        case 'h':
            *host = optarg;
            break;
        case 'p':
            *port = optarg;
            break;
        case 'm':
            *mode = optarg;
            break;
        default:
            fprintf(stderr, "Usage: ipkcpc -h <host> -p <port> -m <mode>\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Check if required parameters are present */
    if (host == NULL || port == NULL || mode == NULL) {
        fprintf(stderr, "Usage: ipkcpc -h <host> -p <port> -m <mode>\n");
        exit(EXIT_FAILURE);
    }

    /* Check if HOST is a valid IPv4 address 
       https://man7.org/linux/man-pages/man3/inet_aton.3.html */
    struct in_addr addr;
    if (inet_aton(*host, &addr) == 0) {
        fprintf(stderr, "ERROR: Invalid IPv4 address: %s\n", *host);
        exit(EXIT_FAILURE);
    }

    /* Check if PORT is a valid number */
    long port_num = strtol(*port, nullptr, 10);
    if (port_num < 1 || port_num > 65535) {
        fprintf(stderr, "ERROR: Invalid port number: %s\n", *port);
        exit(EXIT_FAILURE);
    }

    /* Check if MODE is either tcp || ucp */
    if (strcmp(*mode, "tcp") != 0 && strcmp(*mode, "udp") != 0) {
        fprintf(stderr, "ERROR: mode must be either tcp or udp");
        exit(EXIT_FAILURE);
    }
}

void addr_soc(char *host, char *port, struct hostent **server, struct sockaddr_in *server_address) {
    struct timeval timeout={5,0}; //socket timeout value for send and receive    

    /* Obtain server address using DNS */
    if ((*server = gethostbyname(host)) == NULL) {
        fprintf(stderr,"ERROR: no such host as %s\n", host);
        exit(EXIT_FAILURE);
    }
    
    /* Find the server IP address and initialize the server_address structure */
    bzero((char *) server_address, sizeof(*server_address));
    server_address->sin_family = AF_INET;
    bcopy((char *)(*server)->h_addr, (char *)&server_address->sin_addr.s_addr, (*server)->h_length);
    server_address->sin_port = htons(atoi(port));

    if (strcmp(mode, "tcp") == 0){
        /* Create TCP socket */
        if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0){
            perror("ERROR: socket");
            exit(EXIT_FAILURE);
        }
    } else {
        /* Create UDP socket */
        if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0){
            perror("ERROR: socket");
            exit(EXIT_FAILURE);
        }
    }

    /* Set socket send and receive timeouts
       https://pubs.opengroup.org/onlinepubs/000095399/functions/setsockopt.html */
    setsockopt(client_socket,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(struct timeval));
    setsockopt(client_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

}

void connect_udp(char **host,char **port){
    socklen_t serverlen;
    struct hostent* server;
    struct sockaddr_in server_address;
    char buf[BUFSIZE];                    //stores user message to send
    char request_msg[BUFSIZE + 2];        //request packet to send
    char response_msg[MAX_RESPONSE_SIZE]; //response packet to receive
    int bytesrx, bytestx;
    unsigned int payload_length;

    addr_soc(*host, *port, &server, &server_address);

    /* Prepare signal handler */
    signal(SIGINT, sigint_handler);
	
    while(true){
        /* Read message from user */
        bzero(buf, BUFSIZE);
        if (fgets(buf, BUFSIZE, stdin) == NULL) {
            close(client_socket);
            exit(EXIT_FAILURE);
        }
    
        /* Prepare request message format */
        payload_length = strlen(buf);
        bzero(request_msg, BUFSIZE + 2);
        request_msg[0] = 0x00;
        request_msg[1] = payload_length;
        memcpy(&request_msg[2], buf, payload_length);

        /* Send request message to server */
        serverlen = sizeof(server_address);
        bytestx = sendto(client_socket, request_msg, payload_length + 2, 0, (struct sockaddr *) &server_address, serverlen);
        if (bytestx < 0){
            perror("ERROR: sendto timeout or other");
            continue;
        }

        
        /* Receive response message from server  */
        bzero(response_msg, MAX_RESPONSE_SIZE);
        bytesrx = recvfrom(client_socket, response_msg, MAX_RESPONSE_SIZE, 0, (struct sockaddr *) &server_address, &serverlen);
        if (bytesrx <= 0){
            perror("ERROR: recvfrom timeout or other");
            continue;
        }

        response_msg[MAX_RESPONSE_SIZE - 1] = '\0';
        
        /* Check opcode */
        if (response_msg[0] == 0x00){
            fprintf(stderr, "ERROR: Received invalid packet opcode from server! Try again...\n");
            continue;
        }

        /* Print message based on status code */
        if (response_msg[1] == 0x01) {
            printf("ERR:%s\n", &response_msg[3]);
        } 
        else if (response_msg[1] == 0x00) {
            printf("OK:%s\n", &response_msg[3]);
        } 
        else {
            printf("ERROR: Unknown response from server\n");
        }
    }
}

void connect_tcp(char **host,char **port){
    struct sockaddr_in server_address;
    struct hostent* server;
    int bytestx, bytesrx;
    char buf[BUFSIZE]; //stores user message to send
    
    /* obtain IP address using DNS and create socket with timeouts */
    addr_soc(*host, *port, &server, &server_address);
    
    /* Establish connection */
    if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0){
        perror("ERROR: connect");
        close(client_socket);
        exit(EXIT_FAILURE);        
    }

    /* Prepare signal handler */
    signal(SIGINT, sigint_handler);

    while(true){
        /* Read message from user */
        bzero(buf, BUFSIZE);
        if (fgets(buf, BUFSIZE, stdin) == NULL) {
            exit_tcp_gracefully(client_socket);
            exit(EXIT_FAILURE);
        }

        /* Send message to server */
        bytestx = send(client_socket, buf, strlen(buf), 0);
        if (bytestx < 0){
            perror("ERROR: sendto timeout or other error");
        }
            
        /* Receive message from server  */
        bzero(buf, BUFSIZE);
        bytesrx = recv(client_socket, buf, BUFSIZE, 0);
        if (bytesrx < 0){
            perror("ERROR: recv timeout or other error");
        }
        
        /* Print received message */
        printf("%s", buf);

        /* If received message is `BYE`, close connection */
        if (strcmp(buf, "BYE\n") == 0){
            close(client_socket);
            exit(EXIT_SUCCESS);
        }
    }   
}