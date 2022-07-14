#include "hilo_kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/list.h>
//#include "main_memoria.h"

void* hilo_kernel(void* ptr_void_socket){
	sem_post(&hilo_iniciado);

	//atender peticiones del KERNEL
	uint32_t OK = 100;
	uint32_t codigo_recibido;
	uint32_t process_id;
	uint32_t espacio_de_direcciones;
	uint32_t numero_primer_tabla;
	int socket_kernel = *(int*)ptr_void_socket;

	while(1){
		recv(socket_kernel, &codigo_recibido, sizeof(uint32_t), MSG_WAITALL);

		// no sacar las llaves de los casem atte batata
		switch(codigo_recibido){
			// TESTEADO - FUNCIONA BIEN (o eso parece)
			case crear_tablas:{
				// recibe un PROCESS ID y TAMAÑO DEL ESPACIO DE DIRECCIONES
				// basicamente un list add a la lista de tablas, y crear la cantidad de
				// tablas de segundo nivel necesarias. Tambien se crea el archivo .swap

				// recibo las cosas necesarias
				recv(socket_kernel, &process_id, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_kernel, &espacio_de_direcciones, sizeof(uint32_t), MSG_WAITALL);

				// hago la tarea necesaria
				int num_tabla_creado = crear_tablas_necesarias(espacio_de_direcciones);
				crear_archivo_swap(process_id);

				//respondo con el numero de tabla creado
				send(socket_kernel, &num_tabla_creado, sizeof(uint32_t), 0);

				break;
			}
			case suspension_proceso:{
				// sacar todos los frames del proceso que esten en memoria y actualizarlos en el
				// swap si es necesario

				// recibo las cosas necesarias para el laburo

				recv(socket_kernel, &process_id, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_kernel, &numero_primer_tabla, sizeof(uint32_t), MSG_WAITALL);

				// hago el laburo
				suspender_proceso(process_id, numero_primer_tabla);

				// se avisa que esta todo bien
				// fulbo
				send(socket_kernel, &OK, sizeof(uint32_t), 0);

				break;
			}

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
uint32_t crear_tablas_necesarias( uint32_t espacio_de_direcciones){
	int (*tabla)[ENTRADAS_POR_TABLA]= malloc(sizeof(int)*ENTRADAS_POR_TABLA);
	for(int i=0 ; i < ENTRADAS_POR_TABLA ; i++){
		(*tabla)[i] = -1; // facilita el momento de recorrer la tabla si tiene menos de ENTRADAS_POR_TABLA entradas
	}
	int cantidad_paginas_necesaria = calcular_cantidad_de_paginas(espacio_de_direcciones);
	int cantidad_tablas_2do_nivel_necesarias = calcular_cantidad_de_tablas(cantidad_paginas_necesaria);
	crear_tablas_2do_lvl(tabla, cantidad_tablas_2do_nivel_necesarias);
	list_add(tabla_de_paginas_de_primer_nivel, tabla);

	return list_size(tabla_de_paginas_de_primer_nivel) - 1;
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
	char* ruta_archivo = obtener_ruta_archivo(process_id);

    FILE* archivo = fopen(ruta_archivo, "wb+");

    free(ruta_archivo);
    fclose(archivo);
}

void suspender_proceso(uint32_t process_id, uint32_t numero_primer_tabla){
	int (*puntero_a_tabla)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_primer_nivel, numero_primer_tabla);

	// empieza a ver cada posicion de la primer tabla hasta que haya un -1 o hasta que i == ENTRADAS_POR_TABLA
	for(int i = 0 ; i < ENTRADAS_POR_TABLA ; i++){
		if( (*puntero_a_tabla)[i] == -1 ) break; // si hay un -1 entonces no hay mas tablas de 2do nivel
		pagina_t (*puntero_a_tabla_2do_nivel)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_segundo_nivel, (*puntero_a_tabla)[i]);

		// empieza a ver cada posicion de la tabla de segundo nivel
		for(int j = 0 ; j < ENTRADAS_POR_TABLA; j++){
			if( (*puntero_a_tabla_2do_nivel)[j].bit_presencia == 1 ){

				if( (*puntero_a_tabla_2do_nivel)[j].bit_modificacion == 1 ){
					// guardar pagina en swap, batata tiene una funcion
					// guardar_pagina_en_swap((*puntero_a_tabla_2do_nivel)[j], process_id);
				}

				// poner_bit_en_0( (*puntero_a_tabla_2do_nivel)[j].numero_frame );
			}
		}
	}
}

void guardar_pagina_en_swap(pagina_t pagina, uint32_t process_id){
	// fulbo
}

void poner_bit_en_0(uint32_t numero_de_frame){
	// fulbo
}

char* obtener_ruta_archivo(uint32_t process_id){
	char* nombre_archivo = string_itoa(process_id);
	string_append(&nombre_archivo,".swap");

	char* ruta_archivo = string_new();
	string_append(&ruta_archivo, "./swap/");
	string_append(&ruta_archivo, nombre_archivo);

	free(nombre_archivo);

	return ruta_archivo;
}












