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
#include <commons/collections/queue.h>
#include <commons/collections/list.h>


/*
typedef struct {
	uint32_t num_tabla;
	pagina* paginas = list_create(); // máximo según config
} tabla_paginas_segundo_nivel_t;
*/
//typedef struct {
//	char* direccion_fisica; // 0x000...etc
//	uint32_t data; //Lo que lleva adentro, es decir el dato
//} memoria;


t_config* inicializarConfigs(void);
int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char* ip, char* puerto);
void inicializar_bitmap();

void* hilo_cpu(void* socket_cpu_void);

typedef struct{
	uint32_t tamanio_paginas;
	uint32_t entradas_por_tabla;
} info_traduccion_t;

pthread_t hiloCPU;
uint32_t RETARDO_MEMORIA;
uint32_t ENTRADAS_POR_TABLA;
t_list* tabla_de_paginas_de_primer_nivel;
t_list* tabla_de_paginas_de_segundo_nivel;
t_list* bitmap_memoria;
char* ALGORITMO_REEMPLAZO;
uint32_t MARCOS_POR_PROCESO;
/* Hay una lista de uint32_t por cada proceso
 * Éstas guardan una lista de uint32_t que representa el número de tabla de segundo nivel a la cual dirigirse
 */


/* Guarda las tablas de segundo nivel de todos los procesos
 * Éstas guardan una lista de páginas
 */

/** LEER ACLARACIONES mientras se mira el dibujo pasado al grupo*/

/* Esto simboliza lo que dijo Leandro hoy de que la CPU puede pedir acceder a
 * la tabla de paginas 1, la 2, o directamente que yo escriba en memoria.
 * Mi 'Escribir' en memoria sería (o podría ser) un array de structs que va de 0 a tam-1,
 * y le pongo direccion_fisica para endulzar un poco, no sé, me parecio que tenia que ir.
 * Despues La CPU puede pasar directamente el numero de frame para facilitar las operaciones
 * y tambien podria (aunque no es necesario porque en el tp siempre son 4 bytes) pasar
 * cuantos bytes quiere que memoria lea. Faltaría crear el array de tabla de páginas
 * (porque es una tabla de páginas por proceso). EL array de tabla de páginas va a contener
 * en cada índice una tabla de páginas que tiene un puntero a otra tabla (pq son tablas
 * jerarquizadas)
 * La 'memoria' es un malloc de void asterisco, o sea, que por ejemplo, nuestro struct
 * memoria debería ser malloc(sizeof(void*)); y tienen de x posicion a y posicion
 * la CPU puede pedirme acceder a TP1, TP2 o memoria directamente (pero creo que ya lo
 * escribi)
 * Clock reemplaza por bit de uso (no se fija en el modificado) mientras que Clock-M
 * se fija en el bit de uso pero además prioriza reemplazar las que no estén modificadas
 * (para ahorrarse la escritura en swap).
 * Falta definir los array de tabla de páginas, básicamente, a lo que entendí, sería
 * un array de subarrays; porque tengo el array 'general' de tablas de páginas pero cada
 * TP (Tabla de páginas) apunta a otra tabla con muchas páginas entonces...
 */


/* SWAP ES DISCO, LA PÁGINA MODIF LA ESCRIBO EN SWAP, ABRIR EL SWAP CON FOPEN Y BORRARLO
 * NPI COMO. despues array de tabla de paginas por proceso, Puedo tener un bitmap
 * Ordenado por frame y de ahí buscar o cosas asi (para ver que lugares de mem estan
 * ocupados) o puede ser un array. El indice es innecesario pq si es un array
 * ya viene indexado. El algoritmo de SWAPEO no sé si vale la pena que esté en la
 * Tabla de Paginas*/

#endif
