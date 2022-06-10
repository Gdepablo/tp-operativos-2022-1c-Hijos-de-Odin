#include <time.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <stdbool.h>



t_config* inicializarConfigs(void);
int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char* ip, char* puerto);

typedef struct{
	uint32_t tamanio_paginas;
	uint32_t entradas_por_tabla;
} info_traduccion_t;


typedef struct {
	uint32_t frame;
	uint32_t tamanio_frame;
	paginas_primer_nivel* paginas;
} tabla_paginas_t;

typedef struct {
	uint32_t frame;
	uint32_t tamanio_frame;
	paginas_segundo_nivel* paginas;
	char* algoritmo_swapeo_paginas;
} tabla_paginas_segundo_nivel_t;

typedef struct {
	uint32_t numero_frame;
	uint32_t pnumero_TP_segundo_nivel;
} paginas_primer_nivel;


typedef struct {
	uint32_t numero_pagina;
	uint32_t data_pagina;
	bool bit_presencia;
	bool bit_uso;
	bool bit_modificacion; // P U M
} paginas_segundo_nivel;
