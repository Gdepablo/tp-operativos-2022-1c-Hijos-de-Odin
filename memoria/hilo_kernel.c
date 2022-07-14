#include "hilo_kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/list.h>
//#include "main_memoria.h"

void* hilo_kernel(void* ptr_void_socket){
	sem_post(&hilo_iniciado);

	//atender peticiones del KERNEL
	uint32_t codigo_recibido = 10;
	uint32_t process_id;
	uint32_t espacio_de_direcciones;
	int socket_kernel = *(int*)ptr_void_socket;

	while(1){
		recv(socket_kernel, &codigo_recibido, sizeof(uint32_t), MSG_WAITALL);

		switch(codigo_recibido){
			// TESTEADO - FUNCIONA BIEN (o eso parece)
			case crear_tablas:
				// basicamente un list add a la lista de tablas, y crear la cantidad de
				// tablas de segundo nivel necesarias. Tambien se crea el archivo .swap

				recv(socket_kernel, &process_id, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_kernel, &espacio_de_direcciones, sizeof(uint32_t), MSG_WAITALL);

				crear_tablas_necesarias(espacio_de_direcciones);
				crear_archivo_swap(process_id);

				printf("crear las tablas y responder el nro de 1er tabla \n");
				break;
			case suspension_tablas:
				// si kernel nos envia tanto el PID como el numero de 1er tabla, es
				// facilito
				printf("sacar de memoria todas las paginas del proceso \n");
				break;
			case finalizacion_proceso:
				// borrar el .swap y sacar las entradas en memoria si es que hay.
				printf("borrar el archivo .swap, dejar las tablas \n");
				break;
			default:
				printf("codigo erroneo enviado por kernel \n");
		}
	}

	return "";
}


/*
 * crea tanto la tabla de 1er nivel como las de segundo nivel que sean necesarias.
 */
void crear_tablas_necesarias( uint32_t espacio_de_direcciones){
	int (*tabla)[ENTRADAS_POR_TABLA]= malloc(sizeof(int)*ENTRADAS_POR_TABLA);
	int cantidad_paginas_necesaria = calcular_cantidad_de_paginas(espacio_de_direcciones);
	int cantidad_tablas_2do_nivel_necesarias = calcular_cantidad_de_tablas(cantidad_paginas_necesaria);
	crear_tablas_2do_lvl(tabla, cantidad_tablas_2do_nivel_necesarias);
	list_add(tabla_de_paginas_de_primer_nivel, tabla);
}

int calcular_cantidad_de_paginas(uint32_t bytes_proceso){
    int contador = 1;
    while ( bytes_proceso > TAMANIO_PAGINA * contador) {
        contador++;
    }
    return contador;
}

int calcular_cantidad_de_tablas(uint32_t cantidad_paginas_necesaria){
    int contador = 1;
    while(contador * ENTRADAS_POR_TABLA < cantidad_paginas_necesaria){
        contador++;
    }
    return contador;
}

void crear_tablas_2do_lvl(int (*tabla)[], int cantidad_de_tablas){
	for(int i=0; i < cantidad_de_tablas; i++ ){
		pagina_t (*tabla2)[ENTRADAS_POR_TABLA]= malloc(sizeof(pagina_t)*ENTRADAS_POR_TABLA);
		list_add(tabla_de_paginas_de_segundo_nivel, tabla2);
		(*tabla)[i] = list_size(tabla_de_paginas_de_segundo_nivel) - 1;
	}
}

// ATENCION: ESTO SOLAMENTE LO CREA. SI EXISTE UNO, ENTONCES LO VA A REEMPLAZAR
// el archivo se crea en la carpeta swap
void crear_archivo_swap(uint32_t process_id){
	char* nombre_archivo = string_itoa(process_id);
	string_append(&nombre_archivo,".swap");

    printf("nombre_archivo = %s \n", nombre_archivo);

	char* ruta_archivo = string_new();
    string_append(&ruta_archivo, "./swap/");
    string_append(&ruta_archivo, nombre_archivo);

    FILE* archivo = fopen(ruta_archivo, "wb+");

    free(nombre_archivo);
    free(ruta_archivo);
    fclose(archivo);
}














