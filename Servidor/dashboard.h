// ============================================================================
// dashboard.h - Dashboard tipo htop para servidor multi-cliente
// ============================================================================

#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <time.h>
#include <pthread.h>

// ============================================================================
// Constantes
// ============================================================================

#define MAX_CLIENTS 100
#define NICK_SIZE 32
#define MAX_MESSAGE_LOG 10
#define MAX_MESSAGE_CONTENT 256

// ============================================================================
// Códigos ANSI para control de terminal
// ============================================================================

#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#define RESET_COLOR "\033[0m"
#define BOLD "\033[1m"
#define COLOR_GREEN "\033[32m"
#define COLOR_CYAN "\033[36m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_WHITE "\033[37m"
#define COLOR_RED "\033[31m"

// ============================================================================
// Estructuras
// ============================================================================

typedef struct {
    int sockfd;
    char nick[NICK_SIZE];
    int active;
    time_t connected_at;
} ClientInfo;

typedef struct {
    ClientInfo clients[MAX_CLIENTS];
    int count;
    pthread_mutex_t mutex;
} ClientList;

typedef struct {
    char from_nick[NICK_SIZE];
    char to_nick[NICK_SIZE];
    char message[MAX_MESSAGE_CONTENT];
    time_t timestamp;
} MessageLogEntry;

typedef struct {
    MessageLogEntry messages[MAX_MESSAGE_LOG];
    int count;  // Número total de mensajes registrados (puede ser > MAX_MESSAGE_LOG)
    int start;  // Índice del mensaje más antiguo en el buffer circular
    pthread_mutex_t mutex;
} MessageLog;

// ============================================================================
// Funciones públicas
// ============================================================================

/**
 * Habilita el modo raw en la terminal
 * Desactiva el modo canónico y el echo para capturar teclas individualmente
 */
void enable_raw_mode(void);

/**
 * Deshabilita el modo raw en la terminal
 * Restaura la configuración original de la terminal
 */
void disable_raw_mode(void);

/**
 * Obtiene el tamaño de la terminal
 * @param rows Puntero donde se guardará el número de filas
 * @param cols Puntero donde se guardará el número de columnas
 * @return 0 si tiene éxito, -1 en caso de error
 */
int get_terminal_size(int *rows, int *cols);

/**
 * Refresca y muestra el dashboard con información del servidor
 * @param client_list Puntero a la lista de clientes
 * @param message_log Puntero al log de mensajes
 * @param server_running Flag que indica si el servidor está corriendo
 */
void refresh_dashboard(ClientList *client_list, MessageLog *message_log, int server_running);

/**
 * Registra un mensaje en el log del dashboard
 * @param message_log Puntero al log de mensajes
 * @param from_nick Nick del remitente
 * @param to_nick Nick del destinatario
 * @param message Contenido del mensaje
 */
void log_message(MessageLog *message_log, const char *from_nick, const char *to_nick, const char *message);

/**
 * Thread principal del dashboard
 * Actualiza el dashboard cada segundo y detecta cuando se presiona 'q'
 * @param arg Puntero a una estructura DashboardThreadArgs
 * @return NULL
 */
void* dashboard_thread(void* arg);

// ============================================================================
// Estructura para argumentos del thread del dashboard
// ============================================================================

typedef struct {
    ClientList *client_list;
    MessageLog *message_log;
    int *server_running;
    void (*shutdown_callback)(void);
} DashboardThreadArgs;

#endif // DASHBOARD_H

