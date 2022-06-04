#include <time.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <commons/config.h>



t_config* inicializarConfigs(void);
int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char* ip, char* puerto);

typedef struct{
	uint32_t tamanio_paginas;
	uint32_t entradas_por_tabla;
} info_traduccion_t;
