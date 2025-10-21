# 📡 Sistema de Comandos - Chat Cliente-Servidor

## 🎯 Descripción

El servidor ahora soporta comunicación **peer-to-peer** entre clientes. Los clientes pueden:
- Ver quién está conectado
- Enviar mensajes privados directamente a otros clientes
- El servidor actúa como intermediario, no como destino de mensajes

---

## 📋 Comandos Disponibles

### Para el Cliente

| Comando | Descripción | Ejemplo |
|---------|-------------|---------|
| `/list` | Ver lista de clientes conectados | `/list` |
| `/msg <nick> <mensaje>` | Enviar mensaje privado a otro cliente | `/msg Juan Hola! |
| `/help` | Mostrar ayuda | `/help` |
| `/quit` | Desconectarse del servidor | `/quit` |

---

## 🔄 Flujo de Comunicación

### 1. Ver Clientes Conectados (`/list`)

```
Cliente → /list → Servidor
                    ↓
        Servidor consulta lista de clientes
                    ↓
Cliente ← Lista formateada ← Servidor
```

**Respuesta del servidor:**
```
LIST_START
INFO: Clientes conectados: 3/100
LIST_ITEM: Juan (conectado hace 00:05:23)
LIST_ITEM: Maria (conectado hace 00:03:12)
LIST_ITEM: Pedro (conectado hace 00:01:45)
LIST_END
```

### 2. Mensaje Privado (`/msg`)

```
Cliente A → /msg Maria Hola! → Servidor
                                   ↓
                        Servidor busca a "Maria"
                                   ↓
                       Encontró socket de Maria
                                   ↓
Cliente A ← INFO: Mensaje enviado ← Servidor
Cliente B (Maria) ← MSG_FROM: Juan: Hola! ← Servidor
```

---

## 🎨 Interfaz del Cliente

El cliente ahora tiene una interfaz con colores:

- 🟢 **Verde**: Mensajes informativos, éxito
- 🔵 **Cyan**: Headers, bordes, prompts
- 🟡 **Amarillo**: Advertencias, instrucciones
- 🔴 **Rojo**: Errores
- 🟣 **Magenta**: Mensajes privados recibidos

---

## 🛠️ Protocolo de Comunicación

### Definido en `util/protocol.h`

#### Comandos Cliente → Servidor
```c
#define CMD_LIST "/list"
#define CMD_MSG "/msg"
#define CMD_QUIT "/quit"
#define CMD_HELP "/help"
```

#### Respuestas Servidor → Cliente
```c
#define RESP_LIST_START "LIST_START"
#define RESP_LIST_ITEM "LIST_ITEM:"
#define RESP_LIST_END "LIST_END"
#define RESP_ERROR "ERROR:"
#define RESP_INFO "INFO:"
#define RESP_MSG_FROM "MSG_FROM:"
```

---

## 📝 Ejemplos de Uso

### Ejemplo 1: Ver quién está conectado

```bash
# Terminal 1 - Servidor
$ ./servidor 5000

# Terminal 2 - Cliente Juan
$ ./cliente 127.0.0.1 5000
Ingresa tu nick: Juan
Tú: /list

╔═══════════════════════════════════════════╗
║     CLIENTES CONECTADOS AL SERVIDOR      ║
╠═══════════════════════════════════════════╣
ℹ Clientes conectados: 3/100
║ • Juan (conectado hace 00:05:23)
║ • Maria (conectado hace 00:03:12)
║ • Pedro (conectado hace 00:01:45)
╚═══════════════════════════════════════════╝
```

### Ejemplo 2: Enviar mensaje privado

```bash
# Cliente Juan
Tú: /msg Maria Hola Maria, cómo estás?
ℹ Mensaje enviado a Maria

# Cliente Maria (recibe):
📩 [Mensaje privado] Juan: Hola Maria, cómo estás?
```

### Ejemplo 3: Conversación entre clientes

```bash
# Juan → Maria
Tú: /msg Maria ¿Quieres trabajar juntos en el proyecto?
ℹ Mensaje enviado a Maria

# Maria → Juan
Tú: /msg Juan Sí! ¿A qué hora nos conectamos?
ℹ Mensaje enviado a Juan

# Juan recibe:
📩 [Mensaje privado] Maria: Sí! ¿A qué hora nos conectamos?

# Juan → Maria
Tú: /msg Maria A las 3pm?
ℹ Mensaje enviado a Maria
```

---

## 🔧 Compilación

### Servidor
```bash
cd Servidor
gcc servidor.c dashboard.c ../util/network.c -o servidor -I../util -pthread
./servidor 5000
```

### Cliente
```bash
cd Cliente
gcc cliente.c ../util/network.c -o cliente -I../util
./cliente 127.0.0.1 5000
```

---

## 🚀 Arquitectura

```
┌─────────────┐
│  Cliente A  │
│   (Juan)    │
└──────┬──────┘
       │ /msg Maria Hola!
       ↓
┌─────────────┐
│  SERVIDOR   │ ← Intermediario/Router
│  (Puerto    │   - Mantiene lista de clientes
│   5000)     │   - Enruta mensajes
│             │   - Dashboard en tiempo real
└──────┬──────┘
       │ MSG_FROM: Juan: Hola!
       ↓
┌─────────────┐
│  Cliente B  │
│   (Maria)   │
└─────────────┘
```

---

## ✨ Características Implementadas

✅ Lista de clientes en tiempo real  
✅ Mensajes privados cliente-a-cliente  
✅ Protocolo extensible  
✅ Interfaz colorida y amigable  
✅ Validación de comandos  
✅ Manejo de errores robusto  
✅ Dashboard del servidor tipo htop  
✅ Cierre ordenado con 'q'  

---

## 🔜 Próximas Mejoras Posibles

- [ ] Mensajes broadcast (enviar a todos)
- [ ] Salas/canales de chat
- [ ] Historial de mensajes
- [ ] Confirmación de lectura
- [ ] Estado de usuario (disponible/ausente)
- [ ] Encriptación de mensajes

---

## 📚 Estructura de Archivos

```
C-sockets-servidor-multi-cliente/
├── util/
│   ├── network.c      # Funciones de red
│   ├── network.h      # Headers de red
│   └── protocol.h     # ✨ NUEVO: Protocolo de comandos
├── Servidor/
│   ├── servidor.c     # Lógica del servidor + comandos
│   ├── dashboard.c    # UI del servidor
│   └── dashboard.h    # Headers del dashboard
├── Cliente/
│   └── cliente.c      # ✨ MEJORADO: Cliente con comandos
└── COMANDOS.md        # Esta documentación
```

