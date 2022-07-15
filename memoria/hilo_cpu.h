#include <netdb.h>
#include "main_memoria.h"
#include "variables_globales.h"


uint32_t buscar_tabla_2do_nivel(uint32_t numero_tabla_1er_nivel, uint32_t numero_de_entrada);
uint32_t leer_de_memoria(uint32_t numero_de_frame, uint32_t offset);
void escribir_en_memoria(uint32_t numero_de_frame,uint32_t offset,uint32_t valor_a_escribir);
pagina_t* buscar_pagina(uint32_t numero_tabla_2do_nivel_leer, uint32_t numero_de_entrada);
void cargar_a_memoria(uint32_t numero_tabla_1er_nivel_leer, pagina_t* pagina_buscada, uint32_t num_pagina);
uint32_t calcular_num_pagina(uint32_t numero_tabla_1er_nivel_leer,uint32_t numero_tabla_2do_nivel_leer, uint32_t numero_de_entrada);
