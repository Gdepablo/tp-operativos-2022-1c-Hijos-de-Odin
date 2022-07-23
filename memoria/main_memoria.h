#ifndef MEMORIA_H_
#define MEMORIA_H_
#include <time.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

t_config* inicializarConfigs(void);
int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char* ip, char* puerto);
void inicializar_bitmap();

void* hilo_cpu(void* socket_cpu_void);

typedef struct{
	uint32_t tamanio_paginas;
	uint32_t entradas_por_tabla;
} info_traduccion_t;

sem_t escritura_log;
t_log* log_ejecucion_main;

#endif
