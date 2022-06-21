#include "swap.h"

/** Lee una pagina desde su archivo de SWAP
 Lee hasta encontrar el final del archivo
 * TODO: No hardcodear ese 1?*/
pagina leer_de_swap(int posicion) {
    char *extension = ".dat";
    char numero[50];
    int j=0;
    int array_pags[500];
    snprintf(numero, sizeof(int), "%d", posicion);
    char* nombre_archivo = strcat(numero,extension);
    pagina paginaAux;
    if(fopen(nombre_archivo, "rb") != NULL) {
    lista_de_swap[posicion] = fopen(nombre_archivo, "rb");
    rewind(lista_de_swap[posicion]);
    while(!feof(lista_de_swap[posicion])) {
    fread(&array_pags[j],sizeof(int),1,lista_de_swap[posicion]);
    j++;
    }
    return paginaAux;
    fclose(lista_de_swap[posicion]);}
    else return perror("El archivo no existe o no se pudo abrir");
}


/** Escribe una pagina en SWAP*/
void escribir_en_swap(pagina *una_pagina) {
	 for (int i = 0; i < ENTRADAS_POR_TABLA; i++)
	 {
	     char nombre_archivo[40];
	     sprintf(nombre_archivo, "%d.dat", i);
	     lista_de_swap[i] = fopen(nombre_archivo, "a+");
	     fwrite(una_pagina,sizeof(pagina),1,lista_de_swap[i]);
         rewind(lista_de_swap[i]);
         fclose(lista_de_swap[i]);
	 }
}


uint32_t es_igual_a(t_list* lista_paginas,pagina paginaPedida) {
	uint32_t iterador_tabla = 0;
	while (iterador_tabla <= ENTRADAS_POR_TABLA) {
		pagina aux = list_get(lista_paginas,iterador_tabla);
		if (aux -> numero_frame == paginaPedida.numero_frame) {
			return 1;
		}
		iterador_tabla++;
	}
	return 0;
}

buscar_pagina_en_swap(pagina_buscada) {

}
