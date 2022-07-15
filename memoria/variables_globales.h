#ifndef VAR_GLOB_
#define VAR_GLOB_
pthread_t hiloCPU;
uint32_t RETARDO_MEMORIA;
uint32_t ENTRADAS_POR_TABLA;
t_list* tabla_de_paginas_de_primer_nivel;
t_list* tabla_de_paginas_de_segundo_nivel;
t_list* punteros_clock;
t_list* bitmap_memoria;
char* ALGORITMO_REEMPLAZO;
uint32_t MARCOS_POR_PROCESO;
void* memoria_real;
pthread_t hiloKernel;
sem_t hilo_iniciado; // = 0
sem_t operacion_swap; // = 0
uint32_t TAMANIO_PAGINA;
uint32_t RETARDO_SWAP;


// ENUM
enum operaciones{
	solicitud_num_tabla_2 ,
	solicitud_num_frame ,
	solicitud_lectura ,
	solicitud_escritura ,
	crear_tablas ,
	suspension_proceso,
	finalizacion_proceso
};


typedef struct {
	uint32_t numero_frame;
	uint32_t bit_presencia;
	uint32_t bit_uso;
	uint32_t bit_modificacion; // P U M
} pagina_t;

#endif
