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
// Implementación del dashboard
// ============================================================================

void refresh_dashboard(ClientList *client_list, int server_running) {
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
    
    pthread_mutex_unlock(&client_list->mutex);
    
    fflush(stdout);
}

// ============================================================================
// Thread del dashboard
// ============================================================================

void* dashboard_thread(void* arg) {
    DashboardThreadArgs *args = (DashboardThreadArgs*)arg;
    
    enable_raw_mode();
    
    while (*args->server_running) {
        refresh_dashboard(args->client_list, *args->server_running);
        
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
    refresh_dashboard(args->client_list, *args->server_running);
    sleep(1);
    
    return NULL;
}

