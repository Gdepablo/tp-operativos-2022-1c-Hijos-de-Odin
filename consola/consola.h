#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <commons/log.h>
#define CREAR_PROCESO 1

void pushearInstruccion(char* instruccion, char*** listaInstrucciones);
t_config* inicializarConfigs(void);
int crear_conexion(char* ip_kernel, char* puerto_kernel);

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

t_log* log_consola;

#endif
