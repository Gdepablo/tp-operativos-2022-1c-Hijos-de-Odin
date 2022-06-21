#ifndef SWAP_H

#define SWAP_H

#include "main_memoria.h"

uint32_t es_igual_a(t_list* lista_paginas,pagina paginaPedida);
void escribir_en_swap(pagina *una_pagina);
pagina leer_de_swap(int posicion);
buscar_pagina_en_swap(pagina_buscada);

#endif
