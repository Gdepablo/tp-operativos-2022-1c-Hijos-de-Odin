// CON CONTROL + SHIFT + C SACAN LOS COMENTARIOS ATTE BATATA <3
#include <commons/collections/list.h>
#include "variables_globales.h"
//#include <stdbool.h>
//#include "swap.h"
//#include "main_memoria.h"



void algoritmo_clock(int (*tabla_primer_nivel)[], pagina_t* pagina_a_ubicar, uint32_t numero_de_pagina, uint32_t process_id);
void algoritmo_clock_modificado(t_list *lista_paginas, pagina_t pagina_buscada);
void* copiar_de_swap(uint32_t pagina, uint32_t process_id);
//int es_clock();
//void* enviar_a_swap();
