#include "algoritmos.h"
extern char* ALGORITMO_REEMPLAZO;
extern uint32_t ENTRADAS_POR_TABLA;
int es_clock() {
	return strcmp(ALGORITMO_REEMPLAZO, "CLOCK") != 0;
}

void* enviar_a_swap(t_list* lista_paginas, pagina pagina_buscada) {

	while(1) { // hago un boceto de la función
		if(es_clock()) {
			algoritmo_clock(lista_paginas,pagina_buscada);
		} else {
			algoritmo_clock_modificado(lista_paginas,pagina_buscada);
		}
	}
	return "";
}


void algoritmo_clock(t_list* lista_paginas, pagina pagina_buscada) {
	pagina *unaPagina;
	if(list_size(lista_paginas) == ENTRADAS_POR_TABLA) {
			for(int i=0; i <= list_size(lista_paginas); i++) {
				unaPagina = list_get(lista_paginas, i);
				if(unaPagina -> bit_uso == 0 ) {
					if(unaPagina -> bit_presencia != 1) {
						escribir_en_swap(unaPagina); //no se por que se queja
						unaPagina -> bit_presencia = 1;
					}
				} else {
					unaPagina ->bit_uso = 0;
				}
			}
		}
	 else if (list_size(lista_paginas) < ENTRADAS_POR_TABLA){
			unaPagina = buscar_pagina_en_swap(pagina_buscada); //TODO
			list_add(lista_paginas,unaPagina);
		}
		for(int j=0; j <= list_size(lista_paginas); j++) {
		unaPagina = list_get(lista_paginas, j);
		if(unaPagina -> bit_presencia ==1)
			if(unaPagina -> bit_uso == 0) {
			list_replace(lista_paginas, j, unaPagina);
			break;}
		else  {
			escribir_en_swap(unaPagina); //ni idea porque se queja
			//Escribo que me llevo una pag o algo asi (o el cambio realizado)
			unaPagina -> bit_presencia = 1;};
		}

		}

/** TODO REVISAR ALGORITMOS*/
void algoritmo_clock_modificado(t_list* lista_paginas, pagina pagina_buscada) {
pagina *unaPagina;

	if(list_size(lista_paginas) == ENTRADAS_POR_TABLA) {
	//Preguntar si la pag no esta. Marcos por proceso no se para que sirve
		for(int i=0; i <= list_size(lista_paginas); i++) {
			unaPagina = list_get(lista_paginas, i);
			if(unaPagina -> bit_uso == 0 ) {
				if(unaPagina -> bit_presencia != 1 || unaPagina -> bit_modificacion == 1) {
					escribir_en_swap(unaPagina);
					//Escribo que me llevo una pag o algo asi (o el cambio realizado)
					unaPagina -> bit_presencia = 1;
					unaPagina ->bit_modificacion =0;
				}
			} else {
				unaPagina -> bit_uso = 0;
			}
		}
		}
	 else if (list_size(lista_paginas) < ENTRADAS_POR_TABLA){
			unaPagina = buscar_pagina_en_swap(pagina_buscada);
			list_add(lista_paginas,unaPagina);
		}
		for(int j=0; j <= list_size(lista_paginas); j++) {
		unaPagina = list_get(lista_paginas, j);
		if(unaPagina -> bit_presencia == 1)
			if(unaPagina -> bit_uso == 0 && unaPagina -> bit_modificacion == 0){
			list_replace(lista_paginas, j, unaPagina);
			break;}
		else  {
			escribir_en_swap(unaPagina);
			//Escribo que me llevo una pag o algo asi (o el cambio realizado)
			unaPagina -> bit_presencia = 1;
			unaPagina -> bit_modificacion = 0;};
		}
		}

/*
 * 2 OPCIONES: 1 : Una funcion buscarPagina que si no la encuentra se fija en el algoritmo
 * y hace todoo el reemplazo y la marencoche y sino ponerlo adentro de los algoritmos
 * y que ellos pregunten si la pagina ya esta cargada de por si */

bool esta_presente(pagina pagina) {
	return pagina.bit_presencia == 1;
}

bool primer_paso_clock(pagina una_pagina) {
	return una_pagina.bit_modificacion == 0 && una_pagina.bit_uso==0;
}

bool segundo_paso_clock(pagina una_pagina) {
	return una_pagina.bit_modificacion ==1 && una_pagina.bit_uso==0;
}
