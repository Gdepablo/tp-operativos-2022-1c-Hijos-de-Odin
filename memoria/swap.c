#include "swap.h"

// 0 1 0 1
// 1 1 1 0

pagina buscar_pagina_en_swap() {
	FILE* tuvieja = fopen("swap_proceso_x.swap","a+");
	char a[500];
	int elemento = 0; //Esto esta mal pero para test
	pagina y;
	while(!eof(tuvieja)) {
			fread(a,sizeof(tuvieja)+1,1,tuvieja);
			//Mecanism de lectura para el swap y que me devuelva la pag
		return y; // mal pero para test
		}
};

uint32_t es_igual_a(pagina paginaPedida) {
	uint32_t iterador_tabla = 0;
	while (iterador_tabla <= ENTRADAS_POR_TABLA) {
		if (tabla_segundo_nivel.paginas[iterador_tabla].numero_frame == paginaPedida.numero_frame) {
			return 1;
		}
		iterador_tabla++;
	}
	return 0;
}
