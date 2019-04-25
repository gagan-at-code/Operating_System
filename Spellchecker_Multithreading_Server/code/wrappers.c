#include "wrappers.h"

int open_listenfd(int port) {
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("listen");
        exit(1);
    }
        
    /* Eliminates "Address already in use" error from bind */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(1);
    }   

    /* Listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    memset(&serveraddr, 0, sizeof serveraddr);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("bind");
        exit(1);
    }

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, BACKLOG) < 0) {
        perror("listen");
        exit(1);
    }
    return listenfd;
}

int open_clientfd(int port) {
    int client_socket;
    struct hostent *server_host;
    struct sockaddr_in serveraddr;

    /* Get server host from server name. */
    server_host = gethostbyname("localhost");
    /* Initialise IPv4 server address with server host. */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server_host->h_addr_list[0], 
	  (char *)&serveraddr.sin_addr.s_addr, server_host->h_length);
    serveraddr.sin_port = htons(port);

    /* Create TCP socket. */
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* Connect to socket with server address. */
    if (connect(client_socket, (struct sockaddr *)&serveraddr, sizeof serveraddr) == -1) {
		perror("connect");
        exit(1);
	}

    return client_socket;
}


int Accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int rc;

    if ((rc = accept(s, addr, addrlen)) < 0) {
        perror("accept");
        exit(1);
    }
    return rc;
}

void Pthread_detach(pthread_t tid) {
    int rc;

    if ((rc = pthread_detach(tid)) != 0) {
        fprintf(stderr, "Accept error: %s", strerror(rc));
    }
}

void Close(int fd) {
    int rc;

    if ((rc = close(fd)) < 0) {
        fprintf(stderr, "Accept error: %s", strerror(errno));
    }
}
