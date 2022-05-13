#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int iniciar_servidor(char* ip, char* puerto);

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
	t_buffer* buffer;
} t_paquete;

#endif
