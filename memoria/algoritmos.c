#include "algoritmos.h"

int es_clock() {
	return strcmp(ALGORITMO_REEMPLAZO, "CLOCK") != 0;
}

void* enviar_a_swap() {

	while(1) { // hago un boceto de la función
		if(es_clock()) {
			algoritmo_clock();
		} else {
			algoritmo_clock_modificado();
		}
	}
	return "";
}


void algoritmo_clock(uint32_t numero_tabla_pagina, uint32_t num_frame_buscado) {
	// Ver como es el tema socket
	// Algo tipo filter que filtre por el bit de presencia

	list_filter(tabla_segundo_nivel[numero_tabla_pagina] -> paginas, estaPresente());
	if(list_size(tabla_segundo_nivel) == ENTRADAS_POR_TABLA){
		//Preguntar si la pag no esta. Marcos por proceso no se para que sirve
		if(!list_find(tabla_segundo_nivel[numero_tabla_pagina].paginas,es_igual_a())) {
			uint32_t iterador_pagina_2 = 0;
			while(iterador_pagina_2 <= MARCOS_POR_PROCESO) {
				if(tabla_segundo_nivel[numero_tabla_pagina] -> paginas[iterador_pagina_2].bit_uso == 0) {
				pagina pagina_buscada = buscar_pagina_en_swap(num_frame_buscado);
				list_replace(tabla_segundo_nivel[numero_tabla_pagina].paginas,
				tabla_segundo_nivel[numero_tabla_pagina] -> paginas[iterador_pagina_2],pagina_buscada);
				} else
				{tabla_segundo_nivel[numero_tabla_pagina] -> paginas[iterador_pagina_2].bit_uso = 0;}
				iterador_pagina_2++;}}
		} else
	{pagina una_pagina =buscarPaginaEnSwap(num_frame_buscado);
	list_add(tabla_segundo_nivel[numero_tabla_pagina],una_pagina);}}

void algoritmo_clock_modificado(uint32_t numero_tabla_pagina, uint32_t num_frame_buscado) {
	// Ver como es el tema socket
	// Algo tipo filter que filtre por el bit de presencia

	list_filter(tabla_segundo_nivel[numero_tabla_pagina] -> paginas, estaPresente);
	if(list_size(tabla_segundo_nivel) == ENTRADAS_POR_TABLA){
		//Preguntar si la pag no esta. Marcos por proceso no se para que sirve
		if(!list_find(tabla_segundo_nivel[numero_tabla_pagina].paginas,es_igual_a)) {
			uint32_t iterador_pagina_2 = 0;
			uint32_t iterador_pagina = 0;
			while(iterador_pagina_2 <= MARCOS_POR_PROCESO) {
				if(tabla_segundo_nivel[numero_tabla_pagina] -> paginas[iterador_pagina_2].bit_uso == 0) {
					pagina findear_pagina = list_find(tabla_segundo_nivel[numero_tabla_pagina] -> paginas[iterador_pagina_2],primer_paso_clock);
					if( findear_pagina != NULL){
						pagina pagina_buscada = buscar_pagina_en_swap(num_frame_buscado);
						list_replace(tabla_segundo_nivel[numero_tabla_pagina].paginas,
						tabla_segundo_nivel[numero_tabla_pagina] -> paginas[iterador_pagina_2],pagina_buscada);
				}}
					pagina pagina_findeada = list_find(tabla_segundo_nivel[numero_tabla_pagina] -> paginas[iterador_pagina_2],segundo_paso_clock);
					if(pagina_findeada != NULL) {
					pagina pagina_buscada = buscar_pagina_en_swap(num_frame_buscado);
					escribir_en_swap();
					list_replace(tabla_segundo_nivel[numero_tabla_pagina].paginas,
					tabla_segundo_nivel[numero_tabla_pagina] -> paginas[iterador_pagina_2],pagina_buscada);
				}
				tabla_segundo_nivel[numero_tabla_pagina] -> paginas[iterador_pagina_2].bit_uso = 0;
				pagina_findeada.bit_uso =1; // Si no es NULL explota en 40 millones de pedazos
				iterador_pagina_2++;}}
		} else
	{pagina una_pagina =buscarPaginaEnSwap(num_frame_buscado);
	list_add(tabla_segundo_nivel[numero_tabla_pagina],una_pagina);}}

bool esta_presente(pagina pagina) {
	return pagina.bit_presencia ==1;
}

bool primer_paso_clock(pagina una_pagina) {
	return una_pagina.bit_modificacion == 0 && una_pagina.bit_uso==0;
}

/* Primero nos falta crear la funcion escribir_en_swap() */

bool segundo_paso_clock(pagina una_pagina) {
	return una_pagina.bit_modificacion ==1 && una_pagina.bit_uso==0;
}
