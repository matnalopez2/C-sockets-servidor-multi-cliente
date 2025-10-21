// ============================================================================
// cliente.c - Cliente de chat simple usando sockets
// ============================================================================
// Compilar: gcc cliente.c ../util/network.c -o cliente -I../util -pthread
// Ejecutar: ./cliente 127.0.0.1 5000
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "network.h"
#include "protocol.h"

#define BUF_SIZE 1024

// Variable global para controlar el estado de ejecución
volatile int running = 1;

// Códigos ANSI para colores en el cliente
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_CYAN "\033[36m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RED "\033[31m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_WHITE "\033[37m"
#define BOLD "\033[1m"

// ============================================================================
// Funciones auxiliares
// ============================================================================

/**
 * Procesa y muestra las respuestas del servidor
 * Retorna 1 si debe continuar, 0 si debe salir
 */
int process_server_response(const char* buffer) {
    // Verificar si es inicio de lista
    if (strncmp(buffer, RESP_LIST_START, strlen(RESP_LIST_START)) == 0) {
        printf(COLOR_CYAN BOLD "\n╔═══════════════════════════════════════════╗\n" COLOR_RESET);
        printf(COLOR_CYAN BOLD "║     CLIENTES CONECTADOS AL SERVIDOR      ║\n" COLOR_RESET);
        printf(COLOR_CYAN BOLD "╠═══════════════════════════════════════════╣\n" COLOR_RESET);
        return 1;
    }
    
    // Verificar si es fin de lista
    if (strncmp(buffer, RESP_LIST_END, strlen(RESP_LIST_END)) == 0) {
        printf(COLOR_CYAN BOLD "╚═══════════════════════════════════════════╝\n" COLOR_RESET);
        return 1;
    }
    
    // Verificar si es un item de la lista
    if (strncmp(buffer, RESP_LIST_ITEM, strlen(RESP_LIST_ITEM)) == 0) {
        const char* content = buffer + strlen(RESP_LIST_ITEM);
        printf(COLOR_WHITE "║ • %s" COLOR_RESET, content);
        if (content[strlen(content)-1] != '\n') printf("\n");
        return 1;
    }
    
    // Verificar si es un mensaje de información
    if (strncmp(buffer, RESP_INFO, strlen(RESP_INFO)) == 0) {
        const char* content = buffer + strlen(RESP_INFO);
        printf(COLOR_GREEN "ℹ %s" COLOR_RESET, content);
        if (content[strlen(content)-1] != '\n') printf("\n");
        return 1;
    }
    
    // Verificar si es un mensaje de error
    if (strncmp(buffer, RESP_ERROR, strlen(RESP_ERROR)) == 0) {
        const char* content = buffer + strlen(RESP_ERROR);
        printf(COLOR_RED "✗ Error: %s" COLOR_RESET, content);
        if (content[strlen(content)-1] != '\n') printf("\n");
        return 1;
    }
    
    // Verificar si es un mensaje privado
    if (strncmp(buffer, RESP_MSG_FROM, strlen(RESP_MSG_FROM)) == 0) {
        const char* content = buffer + strlen(RESP_MSG_FROM);
        printf(COLOR_MAGENTA BOLD "📩 [Mensaje privado] %s" COLOR_RESET, content);
        if (content[strlen(content)-1] != '\n') printf("\n");
        return 1;
    }
    
    // Verificar si es un mensaje broadcast
    if (strncmp(buffer, RESP_BROADCAST, strlen(RESP_BROADCAST)) == 0) {
        const char* content = buffer + strlen(RESP_BROADCAST);
        printf(COLOR_YELLOW BOLD "📢 [Broadcast] %s" COLOR_RESET, content);
        if (content[strlen(content)-1] != '\n') printf("\n");
        return 1;
    }
    
    // Mensaje normal del servidor
    printf(COLOR_YELLOW "%s" COLOR_RESET, buffer);
    if (buffer[strlen(buffer)-1] != '\n') printf("\n");
    
    return 1;
}

/**
 * Muestra la ayuda local del cliente
 */
void show_local_help() {
    printf(COLOR_CYAN BOLD "\n╔═══════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN BOLD "║          COMANDOS DISPONIBLES            ║\n" COLOR_RESET);
    printf(COLOR_CYAN BOLD "╠═══════════════════════════════════════════╣\n" COLOR_RESET);
    printf(COLOR_WHITE "║ " COLOR_GREEN "/list" COLOR_WHITE "  - Ver clientes conectados         ║\n" COLOR_RESET);
    printf(COLOR_WHITE "║ " COLOR_GREEN "/msg <nick> <texto>" COLOR_WHITE "                 ║\n" COLOR_RESET);
    printf(COLOR_WHITE "║         Enviar mensaje privado           ║\n" COLOR_RESET);
    printf(COLOR_WHITE "║ " COLOR_GREEN "/broadcast <texto>" COLOR_WHITE "                  ║\n" COLOR_RESET);
    printf(COLOR_WHITE "║         Enviar a todos los clientes      ║\n" COLOR_RESET);
    printf(COLOR_WHITE "║ " COLOR_GREEN "/help" COLOR_WHITE "  - Mostrar esta ayuda             ║\n" COLOR_RESET);
    printf(COLOR_WHITE "║ " COLOR_GREEN "/quit" COLOR_WHITE "  - Salir del chat                 ║\n" COLOR_RESET);
    printf(COLOR_CYAN BOLD "╚═══════════════════════════════════════════╝\n" COLOR_RESET);
}

/**
 * Thread que recibe mensajes del servidor continuamente (full-duplex)
 */
void* receiver_thread(void* arg) {
    int sockfd = *((int*)arg);
    char buffer[BUF_SIZE];
    int bytes;
    
    while (running) {
        memset(buffer, 0, BUF_SIZE);
        bytes = recv(sockfd, buffer, BUF_SIZE - 1, 0);
        
        if (bytes <= 0) {
            if (running) {  // Solo mostrar mensaje si no fue un cierre intencional
                printf(COLOR_RED "\n✗ Servidor desconectado.\n" COLOR_RESET);
                printf("Cliente cerrado.\n\n");
            }
            running = 0;
            exit(0);  // KISS: Terminar el proceso inmediatamente
        }
        
        buffer[bytes] = '\0';
        
        // Borrar la línea actual del prompt para que el mensaje se vea limpio
        printf("\r\033[K");  // Retorno de carro + borrar línea
        
        // Procesar cada línea de la respuesta
        char buffer_copy[BUF_SIZE];
        strncpy(buffer_copy, buffer, BUF_SIZE);
        
        char* line = strtok(buffer_copy, "\n");
        while (line != NULL) {
            process_server_response(line);
            line = strtok(NULL, "\n");
        }
        
        // Restaurar el prompt
        printf(COLOR_CYAN BOLD "Tú: " COLOR_RESET);
        fflush(stdout);
    }
    
    return NULL;
}

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
    
    printf(COLOR_GREEN BOLD "✓ Conectado al servidor como '%s'!\n" COLOR_RESET, nick);
    
    // Dar un pequeño tiempo para recibir el mensaje de bienvenida
    usleep(100000);  // 100ms
    
    // Crear thread para recibir mensajes del servidor (FULL-DUPLEX)
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receiver_thread, &sockfd) != 0) {
        printf(COLOR_RED "Error al crear thread de recepción\n" COLOR_RESET);
        DisconnectFromServer(sockfd);
        return EXIT_FAILURE;
    }
    
    // Dar tiempo para que el thread receptor procese el mensaje de bienvenida
    usleep(200000);  // 200ms
    
    printf(COLOR_CYAN "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" COLOR_RESET);
    printf(COLOR_YELLOW "Escribe " COLOR_GREEN "/help" COLOR_YELLOW " para ver comandos disponibles\n" COLOR_RESET);
    printf(COLOR_CYAN "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n" COLOR_RESET);

    // Loop principal del chat - SOLO ENVÍO (recepción en thread separado)
    while (running) {
        printf(COLOR_CYAN BOLD "Tú: " COLOR_RESET);
        fflush(stdout);
        
        // Leer mensaje del usuario
        if (!fgets(buffer, BUF_SIZE, stdin)) {
            break;  // EOF o error
        }
        
        // Eliminar salto de línea
        buffer[strcspn(buffer, "\n")] = 0;
        
        // Verificar si la línea está vacía
        if (strlen(buffer) == 0) {
            continue;
        }
        
        // Verificar si es el comando de ayuda local
        if (strcmp(buffer, "/help") == 0) {
            show_local_help();
            continue;  // No enviar al servidor, ya lo procesamos localmente
        }
        
        // Verificar si el usuario quiere salir
        if (strcmp(buffer, "/quit") == 0) {
            printf(COLOR_YELLOW "Cerrando conexión...\n" COLOR_RESET);
            running = 0;
            send(sockfd, buffer, strlen(buffer), 0);
            break;
        }
        
        // Enviar comando/mensaje al servidor
        if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
            printf(COLOR_RED "Error al enviar mensaje\n" COLOR_RESET);
            running = 0;
            break;
        }
        
        // NO esperamos respuesta aquí - el thread receptor se encarga
    }
    
    // Esperar a que termine el thread receptor
    pthread_join(recv_thread, NULL);

    // Cerrar conexión
    DisconnectFromServer(sockfd);
    printf("\nCliente cerrado.\n\n");
    
    return EXIT_SUCCESS;
}
