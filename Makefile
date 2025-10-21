# Makefile para Chat con Sockets (usando librería network.h de la cátedra)

CC = gcc
CFLAGS = -Wall -Wextra -pthread -I./util
SERVIDOR = Servidor/servidor
CLIENTE = Cliente/cliente
NETWORK_LIB = util/network.c

all: servidor cliente
	@echo ""
	@echo "✓ Compilado con librería network.h de la cátedra"
	@echo ""
	@echo "Para ejecutar:"
	@echo "  Terminal 1: cd Servidor && ./servidor 5000"
	@echo "  Terminal 2: cd Cliente && ./cliente 127.0.0.1 5000"
	@echo ""

servidor: $(SERVIDOR)

$(SERVIDOR): Servidor/servidor.c $(NETWORK_LIB)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "✓ Servidor compilado"

cliente: $(CLIENTE)

$(CLIENTE): Cliente/cliente.c $(NETWORK_LIB)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "✓ Cliente compilado"

clean:
	rm -f $(SERVIDOR) $(CLIENTE)
	@echo "✓ Limpieza completada"

help:
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "  Makefile - Chat con Sockets"
	@echo "═══════════════════════════════════════════════════════════"
	@echo ""
	@echo "Comandos disponibles:"
	@echo "  make          Compila servidor y cliente"
	@echo "  make servidor Solo compila el servidor"
	@echo "  make cliente  Solo compila el cliente"
	@echo "  make clean    Elimina archivos compilados"
	@echo "  make help     Muestra esta ayuda"
	@echo ""

.PHONY: all servidor cliente clean help
