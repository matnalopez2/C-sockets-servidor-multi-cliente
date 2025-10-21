# Chat Simple 1:1 con Sockets en C

Ejemplo simple y didáctico de chat cliente-servidor usando sockets TCP.

**Versión simplificada usando la librería `network.h` de la cátedra.**

## ¿Qué hace?

- **Servidor**: Espera conexiones y chatéa con un cliente (chat por turnos)
- **Cliente**: Se conecta al servidor y chatéa (chat por turnos)

Es un chat **sincrónico**: el servidor recibe y luego envía, el cliente envía y luego recibe.

## Compilar

La forma más fácil es usar el Makefile:

```bash
make
```

Esto compila ambos programas usando la librería `network.h` de la cátedra.

También podés compilar manualmente:

  ```bash
# Servidor
gcc Servidor/servidor.c util/network.c -o Servidor/servidor -I./util

# Cliente
gcc Cliente/cliente.c util/network.c -o Cliente/cliente -I./util
```

## Usar

### Ejecutar el Chat

**Terminal 1 - Servidor:**
  ```bash
cd Servidor
  ./servidor 5000
  ```

Vas a ver:
```
=== SERVIDOR DE CHAT ===
Puerto: 5000
Esperando cliente...
```

**Terminal 2 - Cliente:**
  ```bash
cd Cliente
  ./cliente 127.0.0.1 5000
  ```

Vas a ver:
```
=== CLIENTE DE CHAT ===
Conectando a 127.0.0.1:5000...
Conectado al servidor!
Escribe '/quit' para salir
─────────────────────────────

Tú: _
```

### Ejemplo de Conversación

**Cliente:**
```
Tú: Hola servidor!
Servidor: Hola cliente! ¿Cómo estás?
Tú: Muy bien, gracias
Servidor: Excelente!
Tú: /quit
Cerrando conexión...
```

**Servidor:**
```
Cliente: Hola servidor!
Tú: Hola cliente! ¿Cómo estás?
Cliente: Muy bien, gracias
Tú: Excelente!
Cliente cerró la conexión.
```

### Cómo Salir

Hay 2 formas de cerrar el chat:

1. **Comando /quit** (recomendado):
   ```
   Tú: /quit
   Cerrando conexión...
   ```

2. **Ctrl+C o Ctrl+D**:
   - Presioná Ctrl+C o Ctrl+D
   - Cierra inmediatamente

## ¿Cómo funciona?

### Librería network.h (de la cátedra)

Este proyecto usa funciones simplificadas de la cátedra para manejar sockets:

**Servidor:**
```c
int OpenServer(int port);
```
- Crea socket, hace bind, listen y accept
- **¡Todo en uno!**
- Retorna el file descriptor del socket del cliente conectado

```c
int CloseServer(int sockfd);
```
- Cierra el socket

**Cliente:**
```c
int ConnectToServer(char* ip, int port);
```
- Crea socket y se conecta al servidor
- **¡Todo en uno!**
- Retorna el file descriptor del socket

```c
int DisconnectFromServer(int sockfd);
```
- Cierra el socket

### Funciones que usamos directamente

Después de establecer la conexión con las funciones de la cátedra, usamos:

```c
// Enviar datos
send(sockfd, buffer, strlen(buffer), 0);

// Recibir datos (bloqueante: espera hasta que lleguen)
ssize_t bytes = recv(sockfd, buffer, BUF_SIZE - 1, 0);
// Retorna: cantidad de bytes recibidos, o 0 si se cerró, o -1 si error
```

### Estructura del Código

**Servidor (servidor.c):**
```
1. Verificar argumentos (puerto)

2. OpenServer(puerto)          ← La librería hace todo esto:
   ├─ socket()                    • Crea socket
   ├─ bind()                      • Asocia con puerto
   ├─ listen()                    • Marca como servidor
   └─ accept()                    • Espera cliente

3. Loop del chat:
   while (1) {
       recv()  → Recibir mensaje del cliente
       printf() → Mostrar mensaje
       fgets() → Leer respuesta del servidor
       send()  → Enviar respuesta al cliente
   }

4. CloseServer(sockfd)         ← Cierra conexión
```

**Cliente (cliente.c):**
```
1. Verificar argumentos (ip, puerto)

2. ConnectToServer(ip, puerto) ← La librería hace todo esto:
   ├─ socket()                    • Crea socket
   ├─ getaddrinfo()               • Resuelve dirección
   └─ connect()                   • Conecta al servidor

3. Loop del chat:
   while (1) {
       fgets() → Leer mensaje del usuario
       send()  → Enviar mensaje al servidor
       recv()  → Recibir respuesta del servidor
       printf() → Mostrar respuesta
   }

4. DisconnectFromServer(sockfd) ← Cierra conexión
```

## Diagrama de Flujo

```
SERVIDOR                             CLIENTE
   |                                    |
OpenServer(puerto)              ConnectToServer(ip, puerto)
   |                                    |
   ├─ socket()                     ├─ socket()
   ├─ bind()                       ├─ getaddrinfo()
   ├─ listen()                     └─ connect() ───────────┐
   └─ accept() <──────────────────────────────────────────┘
   |                                    |
   |                                    |
   ├─ recv() <──────[mensaje]────────── send()
   |                                    |
   ├─ send() ───────[respuesta]──────> recv()
   |                                    |
   └─ ...                               └─ ...
   |                                    |
CloseServer()                    DisconnectFromServer()
```

## Conceptos Clave

### 1. Sockets

Los **sockets** permiten comunicación entre programas por red.

```c
// Crear socket TCP
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//                   IPv4     TCP
```

La librería `network.h` **encapsula** todas estas funciones:
- `socket()` - Crea el endpoint
- `bind()` - Asocia con una dirección/puerto
- `listen()` - Marca como servidor
- `accept()` - Acepta conexiones
- `connect()` - Conecta a un servidor

Nosotros solo llamamos a `OpenServer()` o `ConnectToServer()` y la librería hace todo el trabajo.

### 2. TCP (Transmission Control Protocol)

Es un protocolo **confiable** que:
- ✅ Garantiza que los datos lleguen en orden
- ✅ Garantiza que los datos lleguen sin errores
- ✅ Detecta desconexiones

### 3. Funciones Bloqueantes

Tanto `recv()` como `fgets()` son **bloqueantes**:
- Se quedan esperando hasta que lleguen datos
- El programa "se pausa" en esa línea

```c
recv(sockfd, buffer, BUF_SIZE, 0);  // Espera hasta recibir algo
fgets(buffer, BUF_SIZE, stdin);      // Espera hasta que escribas algo
```

Por eso este chat es **por turnos**: cada lado espera su turno para hablar.

## Flujo de Datos

### Cliente → Servidor

```c
// CLIENTE
fgets(buffer, BUF_SIZE, stdin);        // 1. Leo del teclado
send(sockfd, buffer, strlen(buffer), 0); // 2. Envío por la red
                                        ↓
// SERVIDOR                             ↓
recv(sockfd, buffer, BUF_SIZE, 0);     // 3. Recibo del cliente
printf("Cliente: %s", buffer);         // 4. Muestro en pantalla
```

### Servidor → Cliente

```c
// SERVIDOR
fgets(buffer, BUF_SIZE, stdin);        // 1. Leo del teclado
send(sockfd, buffer, strlen(buffer), 0); // 2. Envío por la red
                                        ↓
// CLIENTE                              ↓
recv(sockfd, buffer, BUF_SIZE, 0);     // 3. Recibo del servidor
printf("Servidor: %s", buffer);        // 4. Muestro en pantalla
```

## Detalles de Implementación

### Manejo de Buffer

```c
char buffer[BUF_SIZE] = {0};

// Limpiar buffer antes de recibir
memset(buffer, 0, BUF_SIZE);

// Asegurar terminación de string
buffer[bytes] = '\0';
```

### Detección de Desconexión

```c
int bytes = recv(sockfd, buffer, BUF_SIZE - 1, 0);

if (bytes <= 0) {
    // bytes == 0: conexión cerrada
    // bytes < 0: error
    printf("Desconectado.\n");
    break;
}
```

### Comando /quit

```c
if (strncmp(buffer, "/quit", 5) == 0) {
    printf("Cerrando conexión...\n");
    break;
}
```

## Estructura del Proyecto

```
chat-sockets-C/
├── Cliente/
│   └── cliente.c          (96 líneas)
├── Servidor/
│   └── servidor.c         (95 líneas)
├── util/
│   ├── network.h          (11 líneas) - Header de la librería
│   └── network.c          (118 líneas) - Implementación
├── Makefile               (49 líneas)
└── README.md              (este archivo)
```

## Ventajas de usar network.h

| Antes (manual) | Ahora (con network.h) |
|----------------|------------------------|
| `socket()` + `bind()` + `listen()` + `accept()` | `OpenServer(puerto)` |
| `socket()` + `getaddrinfo()` + `connect()` | `ConnectToServer(ip, puerto)` |
| ~150 líneas de código | ~95 líneas de código |
| Manejo complejo de errores | Errores simplificados |

✅ **Menos código**  
✅ **Más simple**  
✅ **Más claro**  
✅ **Mejor para aprender**  

## Ejercicios Propuestos

### Básico
1. Agregá un contador de mensajes enviados/recibidos
2. Agregá timestamp a cada mensaje (usar `time()`)
3. Agregá validación de puerto (1024-65535)

### Intermedio
4. Implementá un comando `/help` que muestre comandos disponibles
5. Agregá colores a los mensajes (usar ANSI escape codes)
6. Implementá un comando `/clear` para limpiar la pantalla

### Avanzado
7. Modificá para que sea asincrónico usando threads
8. Agregá soporte para múltiples clientes (uno a la vez)
9. Implementá un protocolo simple con headers

## Errores Comunes y Soluciones

### "Address already in use"
**Problema:** El puerto todavía está ocupado.  
**Solución:** Esperá 1-2 minutos o usá otro puerto.

```bash
./servidor 5001  # Usar otro puerto
```

### "Connection refused"
**Problema:** El servidor no está corriendo.  
**Solución:** Asegurate de iniciar el servidor primero.

### El cliente no puede conectar
**Problema:** IP o puerto incorrectos.  
**Solución:** Verificá que coincidan:
```bash
# Servidor
./servidor 5000

# Cliente (debe usar el mismo puerto)
./cliente 127.0.0.1 5000
```

## Preguntas Frecuentes

**¿Por qué es por turnos?**  
Porque no usamos threads. Solo hay un flujo de ejecución que alterna entre enviar y recibir.

**¿Puedo hablar con computadoras remotas?**  
Sí! Reemplazá `127.0.0.1` con la IP de la otra computadora. Asegurate de que el firewall permita la conexión.

**¿Qué pasa si envío mensajes muy largos?**  
Los mensajes están limitados a `BUF_SIZE` (1024 bytes). Para enviar mensajes más largos necesitarías fragmentarlos.

**¿Es seguro?**  
No. Los mensajes van en texto plano por la red. Para un chat real necesitarías cifrado (TLS/SSL).

## Para Seguir Aprendiendo

### Próximos Pasos
1. **Agregar threads** → Chat simultáneo (no por turnos)
2. **Múltiples clientes** → Servidor que acepta varios clientes
3. **Broadcast** → Mensajes a todos los clientes
4. **Protocolo** → Definir formato de mensajes (JSON, etc)

### Recursos
- `man socket` - Documentación del sistema
- `man recv` - Sobre recepción de datos
- `man send` - Sobre envío de datos
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - Tutorial excelente en inglés

## Notas Finales

- ✅ **Código educativo**: Simple y directo para aprender
- ✅ **Librería de la cátedra**: Simplifica el manejo de sockets
- ✅ **Manejo básico de errores**: Suficiente para entender
- ✅ **Multiplataforma**: Funciona en Linux, macOS y WSL

Este proyecto es perfecto como **primer contacto con sockets**. Una vez que lo entiendas, podés avanzar a versiones más complejas con threads, múltiples clientes, y protocolos más sofisticados.

---

**¡A experimentar!** 🚀

La mejor forma de aprender es modificar el código y ver qué pasa. Probá romper cosas, arreglarlas, y agregar funcionalidades.

## Licencia

Código educativo para aprendizaje. Usalo como quieras.
