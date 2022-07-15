#include "hilo_cpu.h"
#include "main_memoria.h"
#include "swap.h"
#include "algoritmos.h"
#include <commons/collections/list.h>


int es_clock() {
	return strcmp(ALGORITMO_REEMPLAZO, "CLOCK") != 0;
}



void algoritmo_clock(int (*tabla_primer_nivel)[], pagina_t* pagina_a_ubicar, uint32_t numero_de_pagina, uint32_t process_id){
	t_list* paginas_presentes = buscar_paginas_presentes(tabla_primer_nivel);

	if( list_size(paginas_presentes) < MARCOS_POR_PROCESO && memoria_no_llena() ){
		int frame = buscar_frame_libre();
		void* a_copiar = copiar_de_swap(numero_de_pagina, process_id);
		memcpy(memoria_real + frame * TAMANIO_PAGINA , a_copiar, TAMANIO_PAGINA);
		pagina_a_ubicar->bit_presencia = 1;
		pagina_a_ubicar->bit_uso = 1;
		// fijarse si falta algo TODO
		free(a_copiar);
	}
	else // debe buscar cual reemplazar
	{

	}

	list_destroy(paginas_presentes);
}

//void algoritmo_clock(t_list* lista_paginas, pagina_t* pagina_buscada) {
//	pagina_t *unaPagina;
//	if( list_size(lista_paginas) == ENTRADAS_POR_TABLA ) {
//			for(int i=0; i <= list_size(lista_paginas); i++) {
//				unaPagina = list_get(lista_paginas, i);
//				if(unaPagina -> bit_uso == 0 ) {
//					if(unaPagina -> bit_presencia != 1) {
//						escribir_en_swap(unaPagina); //no se por que se queja
//						unaPagina -> bit_presencia = 1;
//					}
//				} else {
//					unaPagina ->bit_uso = 0;
//				}
//			}
//		}
//	 else if (list_size(lista_paginas) < ENTRADAS_POR_TABLA){
//			unaPagina = buscar_pagina_en_swap(pagina_buscada); //TODO
//			list_add(lista_paginas,unaPagina);
//		}
//		for(int j=0; j <= list_size(lista_paginas); j++) {
//		unaPagina = list_get(lista_paginas, j);
//		if(unaPagina -> bit_presencia ==1)
//			if(unaPagina -> bit_uso == 0) {
//			list_replace(lista_paginas, j, unaPagina);
//			break;
//			}
//		else {
//			escribir_en_swap(unaPagina); //ni idea porque se queja
//			//Escribo que me llevo una pag o algo asi (o el cambio realizado)
//			unaPagina -> bit_presencia = 1;};
//		}
//}

/** TODO REVISAR ALGORITMOS*/
//void algoritmo_clock_modificado(t_list* lista_paginas, pagina_t pagina_buscada) {
//	pagina_t *unaPagina;
//
//	if(list_size(lista_paginas) == ENTRADAS_POR_TABLA) {
//	//Preguntar si la pag no esta. Marcos por proceso no se para que sirve
//		for(int i=0; i <= list_size(lista_paginas); i++) {
//			unaPagina = list_get(lista_paginas, i);
//			if(unaPagina -> bit_uso == 0 ) {
//				if(unaPagina -> bit_presencia != 1 || unaPagina -> bit_modificacion == 1) {
//					escribir_en_swap(unaPagina);
//					//Escribo que me llevo una pag o algo asi (o el cambio realizado)
//					unaPagina -> bit_presencia = 1;
//					unaPagina ->bit_modificacion =0;
//				}
//			} else {
//				unaPagina -> bit_uso = 0;
//			}
//		}
//		}
//	 else if (list_size(lista_paginas) < ENTRADAS_POR_TABLA){
//			unaPagina = buscar_pagina_en_swap(pagina_buscada);
//			list_add(lista_paginas,unaPagina);
//		}
//		for(int j=0; j <= list_size(lista_paginas); j++) {
//		unaPagina = list_get(lista_paginas, j);
//		if(unaPagina -> bit_presencia == 1)
//			if(unaPagina -> bit_uso == 0 && unaPagina -> bit_modificacion == 0){
//			list_replace(lista_paginas, j, unaPagina);
//			break;}
//		else  {
//			escribir_en_swap(unaPagina);
//			//Escribo que me llevo una pag o algo asi (o el cambio realizado)
//			unaPagina -> bit_presencia = 1;
//			unaPagina -> bit_modificacion = 0;};
//		}
//		}
//
///*
// * 2 OPCIONES: 1 : Una funcion buscarPagina que si no la encuentra se fija en el algoritmo
// * y hace todoo el reemplazo y la marencoche y sino ponerlo adentro de los algoritmos
// * y que ellos pregunten si la pagina ya esta cargada de por si */
//
//bool esta_presente(pagina_t pagina) {
//	return pagina.bit_presencia == 1;
//}
//
//bool primer_paso_clock(pagina_t una_pagina) {
//	return una_pagina.bit_modificacion == 0 && una_pagina.bit_uso==0;
//}
//
//bool segundo_paso_clock(pagina_t una_pagina) {
//	return una_pagina.bit_modificacion ==1 && una_pagina.bit_uso==0;
//}


