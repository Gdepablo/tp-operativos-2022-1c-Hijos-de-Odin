#include <semaphore.h>
#include <pthread.h>
#include <netdb.h>

enum operaciones{
	solicitud_num_tabla_2 ,
	solicitud_num_frame ,
	solicitud_lectura ,
	solicitud_escritura ,
	crear_tablas ,
	suspension_tablas,
	finalizacion_proceso
	};


// cosas compartidas xd fulbo
void* memoria_real;
pthread_t hiloKernel;
sem_t hilo_iniciado; // = 0
uint32_t TAMANIO_PAGINA;
uint32_t ENTRADAS_POR_TABLA;

void* hilo_kernel(void* ptr_void_socket);
void crear_tabla_por_nivel( uint32_t espacio_de_direcciones,uint32_t process_id );
int calcular_cantidad_de_tablas(uint32_t cantidad_paginas_necesaria);
int calcular_cantidad_de_paginas(uint32_t bytes_proceso);
