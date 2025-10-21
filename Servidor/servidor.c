// ============================================================================
// servidor.c - Servidor multi-cliente con dashboard tipo htop
// ============================================================================
// Compilar: gcc servidor.c dashboard.c ../util/network.c -o servidor -I../util -pthread
// Ejecutar: ./servidor 5000
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "network.h"
#include "dashboard.h"
#include "protocol.h"

#define BUF_SIZE 1024

// ============================================================================
// Variables globales
// ============================================================================

ClientList client_list = {
    .count = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

int server_running = 1;
int server_sockfd = -1;  // Socket del servidor (global para poder cerrarlo desde cualquier thread)

// ============================================================================
// Funciones de cierre del servidor
// ============================================================================

void shutdown_server() {
    if (!server_running) return;  // Ya se llamó antes
    
    server_running = 0;
    
    // Cerrar el socket del servidor para desbloquear accept()
    if (server_sockfd >= 0) {
        shutdown(server_sockfd, SHUT_RDWR);
    }
}

// ============================================================================
// Funciones de gestión de clientes
// ============================================================================

// Agrega un cliente a la lista
int add_client(int sockfd, const char* nick) {
    pthread_mutex_lock(&client_list.mutex);
    
    if (client_list.count >= MAX_CLIENTS) {
        pthread_mutex_unlock(&client_list.mutex);
        return -1;
    }
    
    // Buscar un slot libre
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!client_list.clients[i].active) {
            client_list.clients[i].sockfd = sockfd;
            strncpy(client_list.clients[i].nick, nick, NICK_SIZE - 1);
            client_list.clients[i].nick[NICK_SIZE - 1] = '\0';
            client_list.clients[i].active = 1;
            client_list.clients[i].connected_at = time(NULL);
            client_list.count++;
            pthread_mutex_unlock(&client_list.mutex);
            return i;
        }
    }
    
    pthread_mutex_unlock(&client_list.mutex);
    return -1;
}

// Elimina un cliente de la lista
void remove_client(int sockfd) {
    pthread_mutex_lock(&client_list.mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_list.clients[i].active && 
            client_list.clients[i].sockfd == sockfd) {
            client_list.clients[i].active = 0;
            close(client_list.clients[i].sockfd);
            client_list.count--;
            break;
        }
    }
    
    pthread_mutex_unlock(&client_list.mutex);
}

// Busca un cliente por nick
// Retorna el socket del cliente o -1 si no se encuentra
int find_client_by_nick(const char* nick) {
    pthread_mutex_lock(&client_list.mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_list.clients[i].active && 
            strcmp(client_list.clients[i].nick, nick) == 0) {
            int sockfd = client_list.clients[i].sockfd;
            pthread_mutex_unlock(&client_list.mutex);
            return sockfd;
        }
    }
    
    pthread_mutex_unlock(&client_list.mutex);
    return -1;
}

// Envía la lista de clientes conectados al cliente especificado
void send_client_list(int client_sockfd) {
    char response[BUF_SIZE * 2];  // Buffer grande para toda la respuesta
    int offset = 0;
    
    pthread_mutex_lock(&client_list.mutex);
    
    // Construir toda la respuesta en un solo buffer
    offset += snprintf(response + offset, sizeof(response) - offset, "%s\n", RESP_LIST_START);
    offset += snprintf(response + offset, sizeof(response) - offset, 
                      "%s Clientes conectados: %d/%d\n", 
                      RESP_INFO, client_list.count, MAX_CLIENTS);
    
    // Agregar cada cliente
    if (client_list.count == 0) {
        offset += snprintf(response + offset, sizeof(response) - offset, 
                          "%s No hay clientes conectados\n", RESP_INFO);
    } else {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_list.clients[i].active) {
                time_t now = time(NULL);
                int elapsed = (int)difftime(now, client_list.clients[i].connected_at);
                int hours = elapsed / 3600;
                int minutes = (elapsed % 3600) / 60;
                int seconds = elapsed % 60;
                
                offset += snprintf(response + offset, sizeof(response) - offset,
                                  "%s %s (conectado hace %02d:%02d:%02d)\n", 
                                  RESP_LIST_ITEM, 
                                  client_list.clients[i].nick,
                                  hours, minutes, seconds);
            }
        }
    }
    
    // Agregar fin de lista
    offset += snprintf(response + offset, sizeof(response) - offset, "%s\n", RESP_LIST_END);
    
    // Enviar TODO de una sola vez
    send(client_sockfd, response, strlen(response), MSG_NOSIGNAL);
    
    pthread_mutex_unlock(&client_list.mutex);
}

// ============================================================================
// Manejo de clientes
// ============================================================================

void* client_handler(void* arg) {
    int client_sockfd = *((int*)arg);
    free(arg);
    
    char buffer[BUF_SIZE] = {0};
    char nick[NICK_SIZE] = {0};
    
    // Recibir el nick del cliente
    int bytes = recv(client_sockfd, nick, NICK_SIZE - 1, 0);
    if (bytes <= 0) {
        close(client_sockfd);
        return NULL;
    }
    
    nick[bytes] = '\0';
    
    // Agregar cliente a la lista
    int client_idx = add_client(client_sockfd, nick);
    if (client_idx < 0) {
        // Servidor lleno
        const char* msg = "Servidor lleno\n";
        send(client_sockfd, msg, strlen(msg), 0);
        close(client_sockfd);
        return NULL;
    }
    
    // Mensaje de bienvenida
    snprintf(buffer, BUF_SIZE, 
             "%s Bienvenido al servidor, %s! Escribe /help para ver comandos disponibles.\n", 
             RESP_INFO, nick);
    send(client_sockfd, buffer, strlen(buffer), MSG_NOSIGNAL);
    
    // Loop de recepción de mensajes
    while (server_running) {
        memset(buffer, 0, BUF_SIZE);
        bytes = recv(client_sockfd, buffer, BUF_SIZE - 1, 0);
        
        if (bytes <= 0 || !server_running) {
            break;  // Cliente desconectado o servidor cerrando
        }
        
        buffer[bytes] = '\0';
        
        // Eliminar salto de línea al final
        if (buffer[bytes-1] == '\n') {
            buffer[bytes-1] = '\0';
        }
        
        // Procesar comandos
        if (strncmp(buffer, CMD_QUIT, strlen(CMD_QUIT)) == 0) {
            // Comando /quit
            break;
            
        } else if (strncmp(buffer, CMD_LIST, strlen(CMD_LIST)) == 0) {
            // Comando /list - enviar lista de clientes
            send_client_list(client_sockfd);
            
        } else if (strncmp(buffer, CMD_HELP, strlen(CMD_HELP)) == 0) {
            // Comando /help - mostrar ayuda
            snprintf(buffer, BUF_SIZE, 
                     "%s === COMANDOS DISPONIBLES ===\n"
                     "%s /list      - Ver clientes conectados\n"
                     "%s /msg <nick> <mensaje> - Enviar mensaje privado a un cliente\n"
                     "%s /help      - Mostrar esta ayuda\n"
                     "%s /quit      - Desconectarse del servidor\n",
                     RESP_INFO, RESP_INFO, RESP_INFO, RESP_INFO, RESP_INFO);
            send(client_sockfd, buffer, strlen(buffer), MSG_NOSIGNAL);
            
        } else if (strncmp(buffer, CMD_MSG, strlen(CMD_MSG)) == 0) {
            // Comando /msg <nick> <mensaje> - enviar mensaje privado
            char* cmd_line = buffer + strlen(CMD_MSG);
            
            // Saltar espacios
            while (*cmd_line == ' ') cmd_line++;
            
            // Extraer nick destino
            char dest_nick[NICK_SIZE];
            int i = 0;
            while (*cmd_line != ' ' && *cmd_line != '\0' && i < NICK_SIZE - 1) {
                dest_nick[i++] = *cmd_line++;
            }
            dest_nick[i] = '\0';
            
            // Saltar espacios
            while (*cmd_line == ' ') cmd_line++;
            
            if (strlen(dest_nick) == 0 || strlen(cmd_line) == 0) {
                snprintf(buffer, BUF_SIZE, "%s Uso: /msg <nick> <mensaje>\n", RESP_ERROR);
                send(client_sockfd, buffer, strlen(buffer), MSG_NOSIGNAL);
            } else {
                // Buscar cliente destino
                int dest_sockfd = find_client_by_nick(dest_nick);
                if (dest_sockfd < 0) {
                    snprintf(buffer, BUF_SIZE, "%s Cliente '%s' no encontrado\n", 
                             RESP_ERROR, dest_nick);
                    send(client_sockfd, buffer, strlen(buffer), MSG_NOSIGNAL);
                } else {
                    // Enviar mensaje al destinatario
                    char msg_to_dest[BUF_SIZE];
                    snprintf(msg_to_dest, BUF_SIZE, "%s %s: %s\n", 
                             RESP_MSG_FROM, nick, cmd_line);
                    send(dest_sockfd, msg_to_dest, strlen(msg_to_dest), MSG_NOSIGNAL);
                    
                    // Confirmar al remitente
                    snprintf(buffer, BUF_SIZE, "%s Mensaje enviado a %s\n", 
                             RESP_INFO, dest_nick);
                    send(client_sockfd, buffer, strlen(buffer), MSG_NOSIGNAL);
                }
            }
            
        } else {
            // Comando desconocido o mensaje normal - hacer eco
            snprintf(buffer, BUF_SIZE, "%s Comando no reconocido. Usa /help para ver comandos.\n", 
                     RESP_ERROR);
            send(client_sockfd, buffer, strlen(buffer), MSG_NOSIGNAL);
        }
    }
    
    // Remover cliente de la lista
    remove_client(client_sockfd);
    
    return NULL;
}

// ============================================================================
// Manejador de señales
// ============================================================================

void signal_handler(int signum) {
    (void)signum;
    shutdown_server();
}

// ============================================================================
// Función principal
// ============================================================================

int main(int argc, char* argv[]) {
    // Verificar argumentos
    if (argc != 2) {
        printf("Uso: %s <puerto>\n", argv[0]);
        printf("Ejemplo: %s 5000\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    
    // Configurar manejador de señales
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Crear socket del servidor
    server_sockfd = CreateServerSocket(port);
    if (server_sockfd < 0) {
        printf("Error: No se pudo iniciar el servidor en el puerto %d\n", port);
        return EXIT_FAILURE;
    }
    
    // Configurar argumentos para el thread del dashboard
    DashboardThreadArgs dash_args = {
        .client_list = &client_list,
        .server_running = &server_running,
        .shutdown_callback = shutdown_server
    };
    
    // Crear thread para el dashboard
    pthread_t dash_thread;
    pthread_create(&dash_thread, NULL, dashboard_thread, &dash_args);
    
    // Loop principal: aceptar clientes
    while (server_running) {
        int* client_sockfd = malloc(sizeof(int));
        *client_sockfd = AcceptClient(server_sockfd);
        
        if (*client_sockfd < 0) {
            free(client_sockfd);
            // Si server_running es 0, significa que estamos cerrando
            if (!server_running) {
                break;
            }
            // Si todavía estamos corriendo pero accept falló, reintentar
            usleep(100000);  // Esperar 100ms antes de reintentar
            continue;
        }
        
        // Si estamos cerrando, no aceptar más clientes
        if (!server_running) {
            close(*client_sockfd);
            free(client_sockfd);
            break;
        }
        
        // Crear thread para manejar el cliente
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, client_handler, client_sockfd);
        pthread_detach(client_thread);
    }
    
    // Esperar a que termine el thread del dashboard
    pthread_join(dash_thread, NULL);
    
    // Notificar y cerrar todas las conexiones de clientes
    pthread_mutex_lock(&client_list.mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_list.clients[i].active) {
            // Enviar mensaje de despedida al cliente
            const char* goodbye_msg = "\nServidor cerrando. Desconectando...\n";
            send(client_list.clients[i].sockfd, goodbye_msg, strlen(goodbye_msg), MSG_NOSIGNAL);
            
            // Cerrar la conexión
            shutdown(client_list.clients[i].sockfd, SHUT_RDWR);
            close(client_list.clients[i].sockfd);
            client_list.clients[i].active = 0;
        }
    }
    client_list.count = 0;
    pthread_mutex_unlock(&client_list.mutex);
    
    // Dar tiempo para que los threads de cliente terminen
    usleep(500000);  // 500ms
    
    // Cerrar servidor
    if (server_sockfd >= 0) {
        close(server_sockfd);
        server_sockfd = -1;
    }
    
    // Limpiar terminal
    printf(CLEAR_SCREEN CURSOR_HOME SHOW_CURSOR);
    printf("\n");
    printf(COLOR_GREEN "═══════════════════════════════════════════════════\n" RESET_COLOR);
    printf(COLOR_GREEN "  Servidor cerrado correctamente.\n" RESET_COLOR);
    printf(COLOR_GREEN "═══════════════════════════════════════════════════\n" RESET_COLOR);
    printf("\n");
    
    return EXIT_SUCCESS;
}
