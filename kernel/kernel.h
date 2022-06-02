#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#define CREAR_PROCESO 1

int iniciar_servidor(char* ip, char* puerto);
t_config* inicializarConfigs(void);


typedef struct {
	uint32_t tamanioDirecciones;
	uint32_t largoListaInstrucciones;
	char* listaInstrucciones;
} t_info_proceso;

typedef struct {
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct {
	uint32_t codOp;
	t_buffer* buffer;
} t_paquete;

// PCB
typedef struct {
	uint32_t id_proceso;
	uint32_t tamanio_direcciones;
	char** lista_instrucciones;
	uint32_t program_counter;
	uint32_t tabla_paginas;
	uint32_t estimacion_rafagas; // para el SRT
} t_pcb;

t_info_proceso* desserializarProceso(t_buffer* buffer);
void* atender_cliente(int* socket_cliente);
void* planificador_largo_plazo();

#endif
