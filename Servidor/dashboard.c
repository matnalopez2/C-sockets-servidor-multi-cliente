// ============================================================================
// dashboard.c - Implementación del dashboard tipo htop
// ============================================================================

#include "dashboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

// ============================================================================
// Variables locales del módulo
// ============================================================================

static struct termios orig_termios;

// ============================================================================
// Implementación de funciones de terminal
// ============================================================================

void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    printf(SHOW_CURSOR);
    fflush(stdout);
}

void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    printf(HIDE_CURSOR);
    fflush(stdout);
}

int get_terminal_size(int *rows, int *cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        *rows = 24;
        *cols = 80;
        return -1;
    }
    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
}

// ============================================================================
// Implementación del log de mensajes
// ============================================================================

void log_message(MessageLog *message_log, const char *from_nick, const char *to_nick, const char *message) {
    pthread_mutex_lock(&message_log->mutex);
    
    // Calcular el índice donde insertar el nuevo mensaje (buffer circular)
    int insert_idx;
    if (message_log->count < MAX_MESSAGE_LOG) {
        // Aún hay espacio, insertar al final
        insert_idx = message_log->count;
    } else {
        // Buffer lleno, sobrescribir el más antiguo
        insert_idx = message_log->start;
        message_log->start = (message_log->start + 1) % MAX_MESSAGE_LOG;
    }
    
    // Copiar datos del mensaje
    strncpy(message_log->messages[insert_idx].from_nick, from_nick, NICK_SIZE - 1);
    message_log->messages[insert_idx].from_nick[NICK_SIZE - 1] = '\0';
    
    strncpy(message_log->messages[insert_idx].to_nick, to_nick, NICK_SIZE - 1);
    message_log->messages[insert_idx].to_nick[NICK_SIZE - 1] = '\0';
    
    strncpy(message_log->messages[insert_idx].message, message, MAX_MESSAGE_CONTENT - 1);
    message_log->messages[insert_idx].message[MAX_MESSAGE_CONTENT - 1] = '\0';
    
    message_log->messages[insert_idx].timestamp = time(NULL);
    
    if (message_log->count < MAX_MESSAGE_LOG) {
        message_log->count++;
    }
    
    pthread_mutex_unlock(&message_log->mutex);
}

// ============================================================================
// Implementación del dashboard
// ============================================================================

void refresh_dashboard(ClientList *client_list, MessageLog *message_log, int server_running) {
    int rows, cols;
    get_terminal_size(&rows, &cols);
    
    pthread_mutex_lock(&client_list->mutex);
    
    // Limpiar pantalla y mover cursor al inicio
    printf(CLEAR_SCREEN CURSOR_HOME);
    
    // Borde superior
    for (int i = 0; i < cols; i++) putchar('=');
    putchar('\n');
    
    // Título
    printf(COLOR_GREEN BOLD);
    printf("%-*s\n", cols, "    SERVIDOR MULTI-CLIENTE - DASHBOARD");
    printf(RESET_COLOR);
    
    for (int i = 0; i < cols; i++) putchar('-');
    putchar('\n');
    
    // Información del servidor
    printf(COLOR_YELLOW);
    printf("  Clientes conectados: %d / %d\n", client_list->count, MAX_CLIENTS);
    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    printf("  Hora actual: %s\n", time_str);
    printf(RESET_COLOR);
    
    for (int i = 0; i < cols; i++) putchar('=');
    putchar('\n');
    
    // Headers de la tabla
    printf(COLOR_CYAN BOLD);
    printf("  %-4s  %-20s  %-10s  %-15s\n", 
           "ID", "NICK", "SOCKET", "TIEMPO CONECTADO");
    printf(RESET_COLOR);
    
    for (int i = 0; i < cols; i++) putchar('-');
    putchar('\n');
    
    // Lista de clientes
    if (client_list->count == 0) {
        printf(COLOR_YELLOW);
        printf("  No hay clientes conectados\n");
        printf(RESET_COLOR);
    } else {
        int client_num = 0;
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_list->clients[i].active) {
                client_num++;
                
                // Calcular tiempo conectado
                int elapsed = (int)difftime(now, client_list->clients[i].connected_at);
                int hours = elapsed / 3600;
                int minutes = (elapsed % 3600) / 60;
                int seconds = elapsed % 60;
                
                char conn_time[20];
                snprintf(conn_time, sizeof(conn_time), "%02d:%02d:%02d", 
                         hours, minutes, seconds);
                
                printf(COLOR_WHITE);
                printf("  %-4d  %-20s  %-10d  %-15s\n", 
                       client_num,
                       client_list->clients[i].nick,
                       client_list->clients[i].sockfd,
                       conn_time);
                printf(RESET_COLOR);
            }
        }
    }
    
    // Separador
    for (int i = 0; i < cols; i++) putchar('=');
    putchar('\n');
    
    // Sección de mensajes
    printf(COLOR_CYAN BOLD);
    printf("  ÚLTIMOS MENSAJES INTERCAMBIADOS\n");
    printf(RESET_COLOR);
    
    for (int i = 0; i < cols; i++) putchar('-');
    putchar('\n');
    
    pthread_mutex_unlock(&client_list->mutex);
    
    // Mostrar mensajes del log
    pthread_mutex_lock(&message_log->mutex);
    
    if (message_log->count == 0) {
        printf(COLOR_YELLOW);
        printf("  No hay mensajes registrados\n");
        printf(RESET_COLOR);
    } else {
        // Mostrar los mensajes más recientes primero
        int num_messages = message_log->count < MAX_MESSAGE_LOG ? message_log->count : MAX_MESSAGE_LOG;
        
        // Recorrer desde el más reciente al más antiguo
        for (int i = num_messages - 1; i >= 0; i--) {
            int idx;
            if (message_log->count < MAX_MESSAGE_LOG) {
                // Buffer no lleno aún
                idx = i;
            } else {
                // Buffer circular lleno
                idx = (message_log->start + i) % MAX_MESSAGE_LOG;
            }
            
            MessageLogEntry *msg = &message_log->messages[idx];
            
            // Formatear timestamp
            char time_str[32];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", localtime(&msg->timestamp));
            
            // Truncar mensaje si es muy largo
            char msg_preview[80];
            if (strlen(msg->message) > 70) {
                strncpy(msg_preview, msg->message, 67);
                msg_preview[67] = '.';
                msg_preview[68] = '.';
                msg_preview[69] = '.';
                msg_preview[70] = '\0';
            } else {
                strncpy(msg_preview, msg->message, sizeof(msg_preview) - 1);
                msg_preview[sizeof(msg_preview) - 1] = '\0';
            }
            
            printf(COLOR_WHITE);
            printf("  [%s] %s > %s: %s\n", 
                   time_str,
                   msg->from_nick, 
                   msg->to_nick, 
                   msg_preview);
            printf(RESET_COLOR);
        }
    }
    
    pthread_mutex_unlock(&message_log->mutex);
    
    // Separador inferior
    for (int i = 0; i < cols; i++) putchar('=');
    putchar('\n');
    
    // Mensaje de ayuda
    if (server_running) {
        printf(COLOR_YELLOW);
        printf("  Presiona 'q' para salir | Actualización automática cada segundo\n");
        printf(RESET_COLOR);
    } else {
        printf(COLOR_RED BOLD);
        printf("  Cerrando servidor...\n");
        printf(RESET_COLOR);
    }
    
    for (int i = 0; i < cols; i++) putchar('=');
    putchar('\n');
    
    fflush(stdout);
}

// ============================================================================
// Thread del dashboard
// ============================================================================

void* dashboard_thread(void* arg) {
    DashboardThreadArgs *args = (DashboardThreadArgs*)arg;
    
    enable_raw_mode();
    
    while (*args->server_running) {
        refresh_dashboard(args->client_list, args->message_log, *args->server_running);
        
        // Verificar si se presionó 'q'
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == 'q' || c == 'Q') {
                args->shutdown_callback();
                break;
            }
        }
        
        sleep(1);
    }
    
    // Mostrar una última actualización indicando que está cerrando
    refresh_dashboard(args->client_list, args->message_log, *args->server_running);
    sleep(1);
    
    return NULL;
}

