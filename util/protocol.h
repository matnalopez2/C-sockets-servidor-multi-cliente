// ============================================================================
// protocol.h - Protocolo de comandos para comunicación cliente-servidor
// ============================================================================

#ifndef PROTOCOL_H
#define PROTOCOL_H

// ============================================================================
// Comandos del protocolo
// ============================================================================

// Comandos del cliente al servidor
#define CMD_LIST "/list"           // Listar clientes conectados
#define CMD_MSG "/msg"             // Enviar mensaje privado: /msg <nick> <mensaje>
#define CMD_BROADCAST "/broadcast" // Enviar mensaje a todos: /broadcast <mensaje>
#define CMD_QUIT "/quit"           // Desconectarse
#define CMD_HELP "/help"           // Mostrar ayuda

// Prefijos de respuesta del servidor
#define RESP_LIST_START "LIST_START"
#define RESP_LIST_ITEM "LIST_ITEM:"
#define RESP_LIST_END "LIST_END"
#define RESP_ERROR "ERROR:"
#define RESP_INFO "INFO:"
#define RESP_MSG_FROM "MSG_FROM:"       // Mensaje privado de otro usuario
#define RESP_BROADCAST "BROADCAST_FROM:" // Mensaje broadcast de otro usuario

// ============================================================================
// Constantes del protocolo
// ============================================================================

#define MAX_NICK_LENGTH 32
#define MAX_MSG_LENGTH 1024

#endif // PROTOCOL_H

