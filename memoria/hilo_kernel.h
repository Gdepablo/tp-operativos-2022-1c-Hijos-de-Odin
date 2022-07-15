#include <semaphore.h>
#include <pthread.h>
#include <netdb.h>
#include <commons/collections/list.h>
#include "variables_globales.h"


// cosas compartidas xd fulbo
void* memoria_real;
pthread_t hiloKernel;
sem_t hilo_iniciado; // = 0
sem_t operacion_swap; // = 0
uint32_t TAMANIO_PAGINA;

uint32_t RETARDO_SWAP;

void* hilo_kernel(void* ptr_void_socket);
uint32_t crear_tablas_necesarias( uint32_t espacio_de_direcciones);
int calcular_cantidad_de_tablas(uint32_t cantidad_paginas_necesaria);
int calcular_cantidad_de_paginas(uint32_t bytes_proceso);
void crear_tablas_2do_lvl(int (*tabla)[], int cantidad_de_tablas);
void suspender_proceso(uint32_t process_id, uint32_t numero_primer_tabla);
void poner_bit_en_0_bitmap(uint32_t numero_de_frame);
void* buscar_frame(uint32_t numero_de_frame);
void liberar_memoria(uint32_t numero_tabla);

void crear_archivo_swap(uint32_t process_id);
void borrar_swap(uint32_t process_id);
void guardar_pagina_en_swap(pagina_t pagina, uint32_t process_id, uint32_t numero_de_pagina);
char* obtener_ruta_archivo(uint32_t process_id);
