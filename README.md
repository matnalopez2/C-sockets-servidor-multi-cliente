# Chat Multi-Cliente con Sockets en C

Sistema de chat multi-cliente usando sockets TCP con servidor tipo htop y comunicación full-duplex.

**Versión avanzada con threads, dashboard interactivo, y protocolo de mensajería.**

## 🚀 ¿Qué hace?

- **Servidor**: Acepta hasta 100 clientes simultáneos con dashboard de monitoreo en tiempo real
- **Cliente**: Se conecta al servidor y chatéa con otros clientes en modo asíncrono (full-duplex)
- **Dashboard**: Interfaz tipo htop que muestra clientes conectados y log de mensajes
- **Mensajería**: Privada (1:1) y broadcast (a todos)

Es un chat **asíncrono**: podés enviar y recibir mensajes simultáneamente sin esperar turnos.

## ✨ Características

- ✅ **Multi-cliente**: Hasta 100 clientes simultáneos
- ✅ **Dashboard interactivo**: Monitoreo en tiempo real tipo htop
- ✅ **Full-duplex**: Envío y recepción simultánea (usando threads)
- ✅ **Mensajes privados**: Envía mensajes a usuarios específicos
- ✅ **Broadcast**: Envía mensajes a todos los usuarios
- ✅ **Sistema de nicks**: Cada usuario tiene un identificador único
- ✅ **Log de mensajes**: Historial de los últimos 10 mensajes en el dashboard
- ✅ **Interfaz con colores**: Terminal mejorada con códigos ANSI
- ✅ **Protocolo robusto**: Comunicación estandarizada cliente-servidor
- ✅ **Cierre graceful**: Manejo correcto de señales y desconexiones

## 📦 Compilar

La forma más fácil es usar el Makefile:

```bash
make
```

Esto compila ambos programas con todas las dependencias.

También podés compilar manualmente:

```bash
# Servidor (con dashboard)
gcc Servidor/servidor.c Servidor/dashboard.c util/network.c -o Servidor/servidor -I./util -pthread

# Cliente
gcc Cliente/cliente.c util/network.c -o Cliente/cliente -I./util -pthread
```

## 🎮 Usar

### Ejecutar el Servidor

**Terminal 1 - Servidor:**
```bash
cd Servidor
./servidor 5000
```

Verás el dashboard interactivo:

```
╔═══════════════════════════════════════════════════════════════════════════╗
║                      SERVIDOR DE CHAT - DASHBOARD                        ║
╠═══════════════════════════════════════════════════════════════════════════╣
║ Puerto: 5000                                            Presiona 'q' para salir
║ Estado: ● ACTIVO                                        Actualización: 1s
╠═══════════════════════════════════════════════════════════════════════════╣
║ CLIENTES CONECTADOS (2/100)
║ ───────────────────────────────────────────────────────────────────────
║  • juan            (Conectado hace 00:05:23)
║  • maria           (Conectado hace 00:02:15)
╠═══════════════════════════════════════════════════════════════════════════╣
║ LOG DE MENSAJES (Últimos 10)
║ ───────────────────────────────────────────────────────────────────────
║  [14:23:45] juan → maria: Hola! ¿Cómo estás?
║  [14:24:01] maria → juan: Muy bien, gracias!
║  [14:24:15] juan → broadcast: Hola a todos!
╚═══════════════════════════════════════════════════════════════════════════╝
```

### Ejecutar Clientes

**Terminal 2, 3, 4... - Clientes:**
```bash
cd Cliente
./cliente 127.0.0.1 5000
```

Al conectarte verás:

```
=== CLIENTE DE CHAT ===
Ingresa tu nick: juan
Conectando a 127.0.0.1:5000...
✓ Conectado al servidor como 'juan'!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Escribe /help para ver comandos disponibles
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ℹ Bienvenido al servidor, juan! Escribe /help para ver comandos disponibles.

Tú: _
```

## 💬 Comandos Disponibles

### Comandos del Cliente

| Comando | Descripción | Ejemplo |
|---------|-------------|---------|
| `/list` | Ver clientes conectados | `/list` |
| `/msg <nick> <texto>` | Enviar mensaje privado | `/msg maria Hola!` |
| `/broadcast <texto>` | Enviar mensaje a todos | `/broadcast Buenos días` |
| `/help` | Mostrar ayuda | `/help` |
| `/quit` | Salir del chat | `/quit` |

### Ejemplo de Conversación

**Cliente Juan:**
```
Tú: /list

╔═══════════════════════════════════════════╗
║     CLIENTES CONECTADOS AL SERVIDOR      ║
╠═══════════════════════════════════════════╣
ℹ Clientes conectados: 3/100
║ • juan (conectado hace 00:05:23)
║ • maria (conectado hace 00:02:15)
║ • pedro (conectado hace 00:01:08)
╚═══════════════════════════════════════════╝

Tú: /msg maria Hola María! ¿Cómo estás?
ℹ Mensaje enviado a maria

Tú: /broadcast Hola a todos!
ℹ Mensaje enviado a todos (2 clientes)

📢 [Broadcast] pedro: Hola juan!

📩 [Mensaje privado] maria: Hola juan! Bien, gracias
```

**Cliente María:**
```
📩 [Mensaje privado] juan: Hola María! ¿Cómo estás?

Tú: /msg juan Hola juan! Bien, gracias
ℹ Mensaje enviado a juan

📢 [Broadcast] juan: Hola a todos!
📢 [Broadcast] pedro: Hola juan!
```

## 🏗️ Arquitectura del Sistema

### Estructura del Proyecto

```
C-sockets-servidor-multi-cliente/
├── Cliente/
│   └── cliente.c              (283 líneas) - Cliente con threads
├── Servidor/
│   ├── servidor.c             (450 líneas) - Servidor multi-cliente
│   ├── dashboard.c            (300+ líneas) - Dashboard tipo htop
│   └── dashboard.h            (128 líneas) - Header del dashboard
├── util/
│   ├── network.h              (11 líneas) - Header de red (cátedra)
│   ├── network.c              (118 líneas) - Implementación de red
│   └── protocol.h             (37 líneas) - Protocolo de comunicación
├── Makefile                   (50 líneas) - Compilación automatizada
├── LICENSE                    - Licencia del proyecto
└── README.md                  - Este archivo
```

### Diagrama de Comunicación

```
                           SERVIDOR (Puerto 5000)
                                  |
                    +-------------+-------------+
                    |                           |
              [Thread Principal]        [Thread Dashboard]
                    |                           |
          ┌─────────┴─────────┐        Monitoreo en
          |                   |        tiempo real
    [Thread Cliente 1]  [Thread Cliente 2]  [Thread Cliente N]
          |                   |                   |
          |                   |                   |
    CLIENTE 1            CLIENTE 2          CLIENTE N
    (juan)               (maria)            (pedro)
       |                     |                   |
       +---------------------+-------------------+
              Comunicación Full-Duplex
           (envío y recepción simultánea)
```

### Flujo del Servidor

```
1. main()
   ├─ CreateServerSocket(puerto)
   ├─ Crear thread del dashboard
   │  └─ dashboard_thread() → Monitoreo continuo
   └─ Loop principal:
      ├─ AcceptClient()
      └─ Crear thread para cada cliente
         └─ client_handler()
            ├─ Recibir nick
            ├─ Agregar a lista de clientes
            └─ Loop de mensajes:
               ├─ recv() mensaje
               ├─ Procesar comando
               └─ Responder/Broadcast
```

### Flujo del Cliente

```
1. main()
   ├─ Solicitar nick
   ├─ ConnectToServer(ip, puerto)
   ├─ Enviar nick al servidor
   ├─ Crear thread receptor
   │  └─ receiver_thread() → Recepción continua
   └─ Loop principal:
      ├─ Leer comando del usuario
      ├─ Validar comando
      └─ send() al servidor
```

## 🔧 Componentes Técnicos

### 1. Sistema de Threads

**Servidor:**
- **Thread principal**: Acepta nuevas conexiones
- **Thread dashboard**: Actualiza la interfaz cada segundo
- **Thread por cliente**: Maneja comunicación con cada cliente

**Cliente:**
- **Thread principal**: Lee input del usuario y envía al servidor
- **Thread receptor**: Recibe y muestra mensajes del servidor

### 2. Protocolo de Comunicación

Definido en `protocol.h`:

**Comandos del cliente → servidor:**
```c
CMD_LIST       "/list"
CMD_MSG        "/msg"
CMD_BROADCAST  "/broadcast"
CMD_QUIT       "/quit"
CMD_HELP       "/help"
```

**Respuestas del servidor → cliente:**
```c
RESP_INFO           "INFO:"           // Información general
RESP_ERROR          "ERROR:"          // Mensajes de error
RESP_LIST_START     "LIST_START"      // Inicio de lista
RESP_LIST_ITEM      "LIST_ITEM:"      // Item de lista
RESP_LIST_END       "LIST_END"        // Fin de lista
RESP_MSG_FROM       "MSG_FROM:"       // Mensaje privado
RESP_BROADCAST      "BROADCAST_FROM:" // Mensaje broadcast
```

### 3. Dashboard Interactivo

El dashboard usa:
- **Códigos ANSI**: Para colores y control de cursor
- **Modo raw**: Para capturar teclas individuales (detectar 'q')
- **ioctl TIOCGWINSZ**: Para obtener tamaño de terminal
- **Buffer circular**: Para el log de mensajes (últimos 10)

Características:
- Actualización automática cada segundo
- Muestra clientes conectados con tiempo de conexión
- Log de mensajes recientes (privados y broadcast)
- Salir con 'q' (cierre graceful)

### 4. Gestión de Clientes

```c
typedef struct {
    int sockfd;
    char nick[32];
    int active;
    time_t connected_at;
} ClientInfo;

typedef struct {
    ClientInfo clients[MAX_CLIENTS];  // Array de 100 clientes
    int count;                         // Clientes activos
    pthread_mutex_t mutex;             // Protección thread-safe
} ClientList;
```

Funciones principales:
- `add_client()` - Agrega cliente a la lista
- `remove_client()` - Elimina cliente de la lista
- `find_client_by_nick()` - Busca cliente por nick
- `broadcast_to_all()` - Envía mensaje a todos
- `send_client_list()` - Envía lista de clientes conectados

### 5. Thread Safety

Todas las estructuras compartidas usan `pthread_mutex_t`:
- `client_list.mutex` - Protege lista de clientes
- `message_log.mutex` - Protege log de mensajes

```c
pthread_mutex_lock(&client_list.mutex);
// Operación crítica
pthread_mutex_unlock(&client_list.mutex);
```

## 🎨 Interfaz y UX

### Colores del Cliente

- 🟢 **Verde**: Mensajes de confirmación e info
- 🔵 **Cyan**: Prompts y bordes
- 🟡 **Amarillo**: Advertencias y mensajes del servidor
- 🔴 **Rojo**: Errores
- 🟣 **Magenta**: Mensajes privados
- 🟡 **Amarillo brillante**: Broadcast

### Códigos ANSI Usados

```c
CLEAR_SCREEN  "\033[2J"    // Limpiar pantalla
CURSOR_HOME   "\033[H"     // Mover cursor al inicio
HIDE_CURSOR   "\033[?25l"  // Ocultar cursor
SHOW_CURSOR   "\033[?25h"  // Mostrar cursor
COLOR_GREEN   "\033[32m"   // Color verde
BOLD          "\033[1m"    // Texto en negrita
RESET_COLOR   "\033[0m"    // Resetear colores
```

## 🔍 Conceptos Clave

### 1. Sockets TCP

Los sockets TCP proporcionan:
- ✅ Conexión confiable
- ✅ Entrega ordenada de datos
- ✅ Control de flujo
- ✅ Detección de desconexiones

```c
// Librería network.h simplifica:
int CreateServerSocket(int port);  // socket + bind + listen
int AcceptClient(int server_sockfd); // accept
int ConnectToServer(char* ip, int port); // socket + connect
```

### 2. Programación Concurrente

**Threads (POSIX pthreads):**
```c
pthread_t thread;
pthread_create(&thread, NULL, thread_function, args);
pthread_join(thread, NULL);  // Esperar a que termine
pthread_detach(thread);      // No esperar, auto-limpiar
```

**Mutexes:**
```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_lock(&mutex);
// Sección crítica
pthread_mutex_unlock(&mutex);
```

### 3. Full-Duplex vs Half-Duplex

**Half-Duplex (versión anterior):**
```
Cliente: send() → espera → recv()
Servidor: recv() → espera → send()
```

**Full-Duplex (versión actual):**
```
Thread 1: send() continuo
Thread 2: recv() continuo
→ Comunicación simultánea bidireccional
```

### 4. Control de Terminal

**Modo Raw:**
```c
// Desactiva buffering de línea
// Captura teclas individuales
// Usado en el dashboard para detectar 'q'
```

**Tamaño de Terminal:**
```c
struct winsize ws;
ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
// ws.ws_row = filas, ws.ws_col = columnas
```

## 🛠️ Detalles de Implementación

### Manejo de Desconexiones

**Servidor:**
```c
int bytes = recv(sockfd, buffer, BUF_SIZE - 1, 0);
if (bytes <= 0) {
    // bytes == 0: cliente cerró conexión
    // bytes < 0: error de red
    remove_client(sockfd);
    break;
}
```

**Cliente:**
```c
// Thread receptor detecta desconexión
if (bytes <= 0) {
    printf("Servidor desconectado.\n");
    running = 0;
    exit(0);  // KISS: terminar proceso
}
```

### Cierre Graceful del Servidor

1. **Usuario presiona 'q'** en el dashboard
2. **signal_handler()** captura SIGINT/SIGTERM
3. **shutdown_server():**
   - Pone `server_running = 0`
   - Llama `shutdown(server_sockfd)` para desbloquear `accept()`
4. **Limpieza:**
   - Cierra todos los sockets de clientes
   - Espera threads con `usleep()`
   - Restaura terminal con `SHOW_CURSOR`

### Parsing de Comandos

```c
// Comando: /msg maria Hola!
if (strncmp(buffer, CMD_MSG, strlen(CMD_MSG)) == 0) {
    char* cmd_line = buffer + strlen(CMD_MSG);  // "maria Hola!"
    
    // Saltar espacios
    while (*cmd_line == ' ') cmd_line++;
    
    // Extraer nick destino (hasta el espacio)
    char dest_nick[NICK_SIZE];
    int i = 0;
    while (*cmd_line != ' ' && *cmd_line != '\0' && i < NICK_SIZE - 1) {
        dest_nick[i++] = *cmd_line++;
    }
    dest_nick[i] = '\0';  // "maria"
    
    // Saltar espacios
    while (*cmd_line == ' ') cmd_line++;  // "Hola!"
    
    // cmd_line ahora contiene el mensaje
}
```

## 🎓 Ejercicios Propuestos

### Básico
1. ✅ Agregá validación de nicks duplicados
2. ✅ Implementá un límite máximo de longitud de mensaje
3. ✅ Agregá un comando `/users` como alias de `/list`

### Intermedio
4. ✅ Agregá timestamps a todos los mensajes
5. ✅ Implementá un comando `/whisper` como alias de `/msg`
6. ✅ Agregá un contador de mensajes enviados por usuario
7. ✅ Implementá persistencia de mensajes en archivo

### Avanzado
8. ✅ Agregá salas de chat (rooms/channels)
9. ✅ Implementá autenticación con usuarios y contraseñas
10. ✅ Agregá cifrado TLS/SSL para comunicación segura
11. ✅ Implementá un servidor web para administración
12. ✅ Agregá soporte para envío de archivos

## 🐛 Errores Comunes y Soluciones

### "Address already in use"
**Problema:** El puerto está ocupado.  
**Solución:**
```bash
# Ver qué proceso usa el puerto
lsof -i :5000

# Esperar 1-2 minutos o usar otro puerto
./servidor 5001
```

### "Connection refused"
**Problema:** El servidor no está corriendo.  
**Solución:** Asegurate de iniciar el servidor primero.

### Terminal "rota" después del crash
**Problema:** El servidor crasheó sin restaurar la terminal.  
**Solución:**
```bash
reset      # Resetear terminal
# o
stty sane  # Restaurar configuración
```

### Dashboard no se ve bien
**Problema:** Terminal muy pequeña.  
**Solución:** Aumentá el tamaño de la terminal (mínimo 80x24).

### Mutex deadlock
**Problema:** Dos threads esperan mutexes del otro.  
**Solución:** Siempre adquirir mutexes en el mismo orden.

## 📚 Comparación de Versiones

| Característica | Versión Simple (antes) | Versión Multi-Cliente (ahora) |
|----------------|------------------------|-------------------------------|
| Clientes | 1 | Hasta 100 |
| Comunicación | Por turnos (half-duplex) | Simultánea (full-duplex) |
| Threads | 0 | 1 + N clientes + 1 dashboard |
| Comandos | 1 (/quit) | 5 (/list, /msg, /broadcast, /help, /quit) |
| Protocolo | Texto plano | Protocolo estructurado |
| Interfaz | Básica | Dashboard interactivo + colores |
| Líneas de código | ~200 | ~1000+ |
| Thread-safety | N/A | Mutexes en estructuras compartidas |
| Mensajería | Echo simple | Privada + Broadcast |
| Monitoreo | No | Dashboard en tiempo real |

## 🚀 Evolución del Proyecto

### Fase 1: Chat Simple 1:1 ✅
- Servidor y cliente básicos
- Comunicación por turnos
- Un solo cliente a la vez

### Fase 2: Full-Duplex ✅
- Threads para envío y recepción simultánea
- Comunicación asíncrona
- Mejor experiencia de usuario

### Fase 3: Multi-Cliente ✅
- Soporte para múltiples clientes
- Sistema de nicks
- Lista de clientes conectados

### Fase 4: Mensajería Avanzada ✅
- Mensajes privados 1:1
- Broadcast a todos
- Protocolo estructurado

### Fase 5: Dashboard y Monitoreo ✅
- Interfaz tipo htop
- Log de mensajes
- Estadísticas en tiempo real

### Fase 6: Propuestas Futuras 🎯
- Salas de chat
- Autenticación de usuarios
- Cifrado TLS/SSL
- Persistencia de mensajes
- Transferencia de archivos

## 💡 Lecciones Aprendidas

### Sobre Sockets
- TCP garantiza entrega ordenada y confiable
- `recv()` puede retornar menos bytes de los esperados
- Siempre verificar retorno de `recv()` para detectar desconexiones
- `MSG_NOSIGNAL` evita SIGPIPE en conexiones rotas

### Sobre Threads
- Un thread por cliente escala bien hasta ~1000 clientes
- Mutexes son esenciales para estructuras compartidas
- `pthread_detach()` para threads que no necesitan `join()`
- Siempre liberar recursos en caso de error

### Sobre Diseño
- KISS (Keep It Simple, Stupid) es clave
- Protocolo bien definido facilita extensiones
- Separar lógica de presentación (servidor/dashboard)
- Código modular es más fácil de mantener

### Sobre UX
- Colores mejoran mucho la experiencia
- Feedback inmediato es importante
- Full-duplex es más natural que por turnos
- Dashboard de monitoreo es invaluable

## 📖 Recursos para Profundizar

### Documentación
```bash
man 2 socket    # Sistema de sockets
man 2 recv      # Recepción de datos
man 2 send      # Envío de datos
man 3 pthread   # POSIX threads
man 4 tcp       # Protocolo TCP
man termios     # Control de terminal
```

### Libros Recomendados
- **"Unix Network Programming" - W. Richard Stevens** (la biblia)
- **"The Linux Programming Interface" - Michael Kerrisk**
- **"Beej's Guide to Network Programming"** (gratis, excelente)

### Temas para Continuar
1. **epoll/select/poll** - Alternativa a threads para muchos clientes
2. **Async I/O** - Programación asíncrona
3. **Load balancing** - Distribuir carga entre servidores
4. **WebSockets** - Sockets para aplicaciones web
5. **gRPC** - Framework moderno de RPC
6. **Redis/RabbitMQ** - Message brokers para escalado

## 🎯 Casos de Uso Reales

Este proyecto enseña conceptos usados en:
- **WhatsApp/Telegram**: Servidores de chat
- **Zoom/Discord**: Comunicación en tiempo real
- **Multiplayer games**: Sincronización de jugadores
- **IoT**: Comunicación entre dispositivos
- **Microservicios**: Comunicación inter-servicios
- **Streaming**: Netflix, Spotify, YouTube Live

## 🏆 Conclusión

Has construido un sistema completo de chat multi-cliente que demuestra:
- ✅ Programación de sockets TCP
- ✅ Concurrencia con threads
- ✅ Sincronización con mutexes
- ✅ Control avanzado de terminal
- ✅ Diseño de protocolos
- ✅ Arquitectura cliente-servidor
- ✅ Manejo de errores y desconexiones

Este proyecto es una base sólida para entender sistemas de red modernos. Los conceptos aquí aprendidos son fundamentales en:
- Backend development
- Sistemas distribuidos
- Cloud computing
- DevOps
- Cybersecurity

---

## 📝 Notas Finales

- ✅ **Código didáctico**: Prioriza claridad sobre optimización
- ✅ **Thread-safe**: Usa mutexes correctamente
- ✅ **Manejo de errores**: Cubre casos comunes
- ✅ **Multiplataforma**: Linux, macOS, WSL
- ✅ **Extensible**: Fácil agregar nuevas funcionalidades

**¡Felicitaciones por llegar hasta acá!** 🎉

Has pasado de un simple echo server a un sistema completo de chat multi-cliente con monitoreo en tiempo real. Esto demuestra cómo los sistemas complejos se construyen iterativamente, agregando funcionalidades de a poco.

## 🤝 Contribuir

Este es un proyecto educativo. Si encontrás bugs o tenés ideas de mejoras:
1. Abrí un issue describiendo el problema/mejora
2. Hacé un fork y creá una feature branch
3. Enviá un pull request con tus cambios

## 📄 Licencia

Código educativo para aprendizaje. Usalo, modificalo, y aprendé.

---

**Última actualización:** Octubre 2025  
**Versión:** 2.0 - Multi-Cliente con Dashboard  
**Estado:** ✅ Producción (con fines educativos)

---

*"La mejor forma de aprender es construir, romper, y volver a construir."*

**¡A seguir experimentando!** 🚀
