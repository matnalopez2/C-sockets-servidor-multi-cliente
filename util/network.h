#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern int OpenServer(int portnr);
extern int CloseServer(int socketfd);

// Nuevas funciones para servidor multi-cliente
extern int CreateServerSocket(int portnr);  // Crea socket, bind y listen
extern int AcceptClient(int server_sockfd); // Acepta un cliente

extern int ConnectToServer(char * Server, int Port);
extern int DisconnectFromServer(int socketfd);

