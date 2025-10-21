// ============================================================================
// cliente.c - Cliente de chat simple usando sockets
// ============================================================================
// Compilar: gcc cliente.c ../util/network.c -o cliente -I../util
// Ejecutar: ./cliente 127.0.0.1 5000
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network.h"

#define BUF_SIZE 1024

int main(int argc, char* argv[]) {
    // Verificar argumentos
    if (argc != 3) {
        printf("Uso: %s <ip> <puerto>\n", argv[0]);
        printf("Ejemplo: %s 127.0.0.1 5000\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* ip = argv[1];
    int port = atoi(argv[2]);
    int sockfd;
    char buffer[BUF_SIZE] = {0};
    char nick[32] = {0};
    
    printf("\n=== CLIENTE DE CHAT ===\n");
    
    // Solicitar nick
    printf("Ingresa tu nick: ");
    fflush(stdout);
    if (!fgets(nick, sizeof(nick), stdin)) {
        printf("Error al leer el nick\n");
        return EXIT_FAILURE;
    }
    
    // Eliminar el salto de línea
    nick[strcspn(nick, "\n")] = 0;
    
    // Verificar que el nick no esté vacío
    if (strlen(nick) == 0) {
        printf("El nick no puede estar vacío\n");
        return EXIT_FAILURE;
    }
    
    printf("Conectando a %s:%d...\n", ip, port);

    // Conectar al servidor (crea socket y hace connect)
    sockfd = ConnectToServer(ip, port);
    if (sockfd <= 0) {
        printf("Error: No se pudo conectar al servidor\n");
        printf("¿Está el servidor corriendo?\n\n");
        return EXIT_FAILURE;
    }
    
    // Enviar el nick al servidor como primer mensaje
    if (send(sockfd, nick, strlen(nick), 0) < 0) {
        printf("Error al enviar nick al servidor\n");
        DisconnectFromServer(sockfd);
        return EXIT_FAILURE;
    }
    
    printf("Conectado al servidor como '%s'!\n", nick);
    printf("Escribe '/quit' para salir\n");
    printf("─────────────────────────────\n\n");

    // Loop principal del chat
    while (1) {
        // 1. ENVIAR mensaje al servidor
        printf("Tú: ");
        fflush(stdout);
        
        // Leer mensaje del usuario
        if (!fgets(buffer, BUF_SIZE, stdin)) {
            break;  // EOF o error
        }
        
        // Verificar si el usuario quiere salir
        if (strncmp(buffer, "/quit", 5) == 0) {
            printf("Cerrando conexión...\n");
            send(sockfd, buffer, strlen(buffer), 0);
            break;
        }
        
        // Enviar mensaje al servidor
        if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
            printf("Error al enviar mensaje\n");
            break;
        }
        
        // 2. RECIBIR respuesta del servidor
        memset(buffer, 0, BUF_SIZE);  // Limpiar buffer
        int bytes = recv(sockfd, buffer, BUF_SIZE - 1, 0);
        
        // Verificar si el servidor se desconectó
        if (bytes <= 0) {
            printf("\nServidor desconectado.\n");
            break;
        }
        
        buffer[bytes] = '\0';  // Asegurar terminación
        
        // Verificar si el servidor envió /quit
        if (strncmp(buffer, "/quit", 5) == 0) {
            printf("\nServidor cerró la conexión.\n");
            break;
        }
        
        // Mostrar mensaje recibido
        printf("Servidor: %s", buffer);
        if (buffer[bytes-1] != '\n') printf("\n");
    }

    // Cerrar conexión
    DisconnectFromServer(sockfd);
    printf("\nCliente cerrado.\n\n");
    
    return EXIT_SUCCESS;
}
