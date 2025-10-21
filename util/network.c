#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "network.h"

#define BACKLOG 10


int OpenServer(int portnr)
{
    int sockfd, new_fd, dummy=1;    // listen on sock_fd
    struct sockaddr_in my_addr;     // my address information
    struct sockaddr_storage their_addr;	// connector's information
    int porttype = SOCK_STREAM;
    socklen_t sin_size = sizeof(struct sockaddr);

    char errorst[100];  // Aumentado de 50 a 100 para evitar overflow

    if ( (sockfd = socket(AF_INET, porttype, 0) ) == -1)
        perror("Error try ing to set up socket...\n");

    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&dummy,sizeof(int)) == -1)
        perror("Error trying to set REUSEADDR to new socket...\n");

    my_addr.sin_family = AF_INET;           // host byte order
    my_addr.sin_port = htons(portnr);       // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY;   // automatically fill with my IP
    memset(&(my_addr.sin_zero), 0, 8);      // zero the rest of the struct

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))== -1)
    {       sprintf(errorst,"Error trying to bind new socket to address %s:%d",inet_ntoa(my_addr.sin_addr), portnr);
            perror(errorst);
    }


    if (listen(sockfd, BACKLOG) == -1)
    {   sprintf(errorst, "Error trying to start listening from new socket (bound to port nr %d)", portnr) ;
        perror(errorst);
    }

    // Queda a la espera de nuevas conexiones
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        
    close(sockfd);

    return new_fd;

}

int CloseServer(int sockfd)
{
    return (close(sockfd));
}


int ConnectToServer(char * Server, int Port)
{
    int sockfd;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char sPort[10];


    sprintf(sPort,"%d",Port);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;



    if ((rv = getaddrinfo(Server, sPort, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 0;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
	{
            perror("client: socket");
            continue;
        }


        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
	{
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 0;
    }

   return sockfd;
   
}

int DisconnectFromServer(int socketfd)
{
    return (close(socketfd));
}

// ============================================================================
// Nuevas funciones para servidor multi-cliente
// ============================================================================

// Crea el socket del servidor, hace bind y listen, pero NO acepta
int CreateServerSocket(int portnr)
{
    int sockfd, dummy=1;
    struct sockaddr_in my_addr;
    int porttype = SOCK_STREAM;
    char errorst[100];

    if ((sockfd = socket(AF_INET, porttype, 0)) == -1) {
        perror("Error trying to set up socket...\n");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(int)) == -1) {
        perror("Error trying to set REUSEADDR to new socket...\n");
        return -1;
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(portnr);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero), 0, 8);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        sprintf(errorst, "Error trying to bind new socket to address %s:%d", 
                inet_ntoa(my_addr.sin_addr), portnr);
        perror(errorst);
        return -1;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        sprintf(errorst, "Error trying to start listening from new socket (bound to port nr %d)", portnr);
        perror(errorst);
        return -1;
    }

    return sockfd;
}

// Acepta un cliente en un socket de servidor
int AcceptClient(int server_sockfd)
{
    struct sockaddr_storage their_addr;
    socklen_t sin_size = sizeof(struct sockaddr);
    int new_fd;

    new_fd = accept(server_sockfd, (struct sockaddr *)&their_addr, &sin_size);
    
    return new_fd;
}
