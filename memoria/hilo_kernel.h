#include <semaphore.h>
#include <pthread.h>
#include <netdb.h>
#include <commons/collections/list.h>

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
	uint32_t data_pagina;
	uint32_t bit_presencia;
	uint32_t bit_uso;
	uint32_t bit_modificacion; // P U M
} pagina_t;

t_list* tabla_de_paginas_de_primer_nivel;
/* Hay una lista de uint32_t por cada proceso
 * Éstas guardan una lista de uint32_t que representa el número de tabla de segundo nivel a la cual dirigirse
 */

t_list* tabla_de_paginas_de_segundo_nivel;
/* Guarda las tablas de segundo nivel de todos los procesos
 * Éstas guardan una lista de páginas
 */

t_list* bitmap_memoria;

// cosas compartidas xd fulbo
void* memoria_real;
pthread_t hiloKernel;
sem_t hilo_iniciado; // = 0
uint32_t TAMANIO_PAGINA;
uint32_t ENTRADAS_POR_TABLA;

void* hilo_kernel(void* ptr_void_socket);
uint32_t crear_tablas_necesarias( uint32_t espacio_de_direcciones);
int calcular_cantidad_de_tablas(uint32_t cantidad_paginas_necesaria);
int calcular_cantidad_de_paginas(uint32_t bytes_proceso);
void crear_tablas_2do_lvl(int (*tabla)[], int cantidad_de_tablas);
void crear_archivo_swap(uint32_t process_id);
void suspender_proceso(uint32_t process_id, uint32_t numero_primer_tabla);
void guardar_pagina_en_swap(pagina_t pagina, uint32_t process_id, uint32_t numero_de_pagina);
char* obtener_ruta_archivo(uint32_t process_id);
void poner_bit_en_0_bitmap(uint32_t numero_de_frame);
void* buscar_frame(uint32_t numero_de_frame);
