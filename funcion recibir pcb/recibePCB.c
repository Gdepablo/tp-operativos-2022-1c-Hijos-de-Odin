#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <commons/config.h>

typedef struct {
	uint32_t id_proceso;
	uint32_t tamanio_direcciones;
	char* lista_instrucciones;
	uint32_t program_counter;
	uint32_t tabla_paginas;
	uint32_t estimacion_rafagas; // para el SRT
} t_pcb;

typedef struct {
	uint32_t size;
	uint32_t size_instrucciones;
	void* stream;
} t_pcb_buffer;


t_config* inicializarConfigs(void) {
	t_config* nuevo_config;

	nuevo_config = config_create("/home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/cpu/cpu.config");

	return nuevo_config; //Creo que esto funciona así igual pero no estoy seguro
}


int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket. LISTO
	int socket_cliente = socket( server_info -> ai_family,
								 server_info -> ai_socktype,
								 server_info -> ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo LISTO

	connect(socket_cliente, server_info -> ai_addr, server_info -> ai_addrlen );

	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciar_servidor(char* ip, char* puerto) {
	int socket_servidor;

	struct addrinfo hints;
	struct addrinfo *servinfo;
	int activado =1;


	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
	socket_servidor = 	socket( servinfo -> ai_family,
								servinfo -> ai_socktype,
								servinfo -> ai_protocol);

	setsockopt(socket_servidor,SOL_SOCKET,SO_REUSEADDR,&activado,sizeof(activado));

	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	return socket_servidor;
}

t_pcb recibir_pcb(int socket_dispatch){
	t_pcb_buffer* pcb_buffer = malloc(sizeof(t_pcb_buffer));
	t_pcb nuevo_pcb;
    int offset = 0;

	recv(socket_dispatch, &(pcb_buffer->size), sizeof(uint32_t), MSG_WAITALL);
    recv(socket_dispatch, &(pcb_buffer->size_instrucciones), sizeof(uint32_t), 0);

    printf("tamanio: %i \n", pcb_buffer->size);
    printf("tamanio instrucciones: %i \n", pcb_buffer->size_instrucciones);

    pcb_buffer->stream = malloc(pcb_buffer->size);
    recv(socket_dispatch, pcb_buffer->stream, sizeof(uint32_t) * 5 + pcb_buffer->size_instrucciones, 0);
    

    memcpy(&(nuevo_pcb.id_proceso), pcb_buffer->stream+offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(nuevo_pcb.tamanio_direcciones), pcb_buffer->stream+offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    nuevo_pcb.lista_instrucciones = malloc( pcb_buffer->size_instrucciones );
    memcpy(nuevo_pcb.lista_instrucciones, pcb_buffer->stream+offset, pcb_buffer->size_instrucciones);
    offset += pcb_buffer->size_instrucciones;
    memcpy(&(nuevo_pcb.program_counter), pcb_buffer->stream+offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(nuevo_pcb.tabla_paginas), pcb_buffer->stream+offset, sizeof(uint32_t) );
    offset += sizeof(uint32_t);
    memcpy(&(nuevo_pcb.estimacion_rafagas), pcb_buffer->stream+offset, sizeof(uint32_t));

    printf("PCB ARMADO: \n");
    printf("id proceso: %i \n", nuevo_pcb.id_proceso);
    printf("tamanio direcciones: %i \n", nuevo_pcb.tamanio_direcciones);
    printf("lista instrucciones:  %s \n", nuevo_pcb.lista_instrucciones);
    printf("program counter: %i \n", nuevo_pcb.program_counter);
    printf("tabla de paginas: %i \n", nuevo_pcb.tabla_paginas);
    printf("estimacion rafagas: %i \n", nuevo_pcb.estimacion_rafagas);
    

	free (pcb_buffer);
	return nuevo_pcb;
}

int main(void){
    int socket_servidor = iniciar_servidor("127.0.0.1", "5050");
    int socket = accept(socket_servidor, NULL, NULL);

    printf("se conecto \n");

    t_pcb pcb = recibir_pcb(socket);

    printf("ya recibi el PCB \n");

    getchar();

    return 0;
}