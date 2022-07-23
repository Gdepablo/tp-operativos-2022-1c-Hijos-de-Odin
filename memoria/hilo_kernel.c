#include "hilo_kernel.h"
#include "main_memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <commons/string.h>
#include <commons/collections/list.h>

t_log* log_ejecucion_kernel;

void* hilo_kernel(void* ptr_void_socket){
	sem_post(&hilo_iniciado);

	log_ejecucion_kernel = log_create("./../logs/hilo_kernel_log.log", "HILO KERNEL", 0, LOG_LEVEL_INFO);

	sem_wait(&escritura_log);
	log_info(log_ejecucion_kernel, "hilo kernel iniciado");
	sem_post(&escritura_log);

	uint32_t OK = 100;
	uint32_t codigo_recibido;
	uint32_t process_id;
	uint32_t espacio_de_direcciones;
	uint32_t numero_primer_tabla;
	int socket_kernel = *(int*)ptr_void_socket;

	while(1){
		recv(socket_kernel, &codigo_recibido, sizeof(uint32_t), MSG_WAITALL);

		sem_wait(&escritura_log);
		log_info(log_ejecucion_kernel, "\n# SOLICITUD RECIBIDA DE KERNEL #");
		sem_post(&escritura_log);

		switch(codigo_recibido){
			case crear_tablas:{
				sem_wait(&escritura_log);
				log_info(log_ejecucion_kernel, "# CREAR_TABLAS");
				sem_post(&escritura_log);

				printf("# Solicitud crear tablas (kernel) #\n");
				log_info(log_ejecucion_kernel, "# Solicitud crear tablas (kernel) #");
				printf("Datos recibidos: \n");
				log_info(log_ejecucion_kernel, "Datos recibidos:");
				recv(socket_kernel, &process_id, sizeof(uint32_t), MSG_WAITALL);
				printf("Process id: %i \n", process_id);
				log_info(log_ejecucion_kernel, "Process id: %i", process_id);
				recv(socket_kernel, &espacio_de_direcciones, sizeof(uint32_t), MSG_WAITALL);
				printf("Espacio de direcciones: %i \n", espacio_de_direcciones);
				log_info(log_ejecucion_kernel, "Espacio de direcciones: %i ", espacio_de_direcciones);

				sem_wait(&escritura_log);
				log_info(log_ejecucion_kernel, "datos recibidos: \nprocess id: %i, espacio de direcciones: %i", process_id, espacio_de_direcciones);
				sem_post(&escritura_log);

				int num_tabla_creado = crear_tablas_necesarias(espacio_de_direcciones);
				crear_archivo_swap(process_id);

				send(socket_kernel, &num_tabla_creado, sizeof(uint32_t), 0);
				printf("# FIN CREAR TABLAS \n\n");

				sem_wait(&escritura_log);
				log_info(log_ejecucion_kernel, "# FIN CREAR_TABLAS \n\n");
				sem_post(&escritura_log);
				break;
			}
			case suspension_proceso:{
				sem_wait(&escritura_log);
				log_info(log_ejecucion_kernel, "# SUSPENSION_PROCESO");
				sem_post(&escritura_log);

				printf("# Solicitud suspender proceso (kernel) #\n");
				log_info(log_ejecucion_kernel, "# Solicitud suspender proceso (kernel) #");
				printf("Datos recibidos: \n");
				log_info(log_ejecucion_kernel, "Datos recibidos:");

				recv(socket_kernel, &process_id, sizeof(uint32_t), MSG_WAITALL);
				printf("Process id: %i \n", process_id);
				log_info(log_ejecucion_kernel, "Process id: %i \n", process_id);
				recv(socket_kernel, &numero_primer_tabla, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de primer tabla: %i \n", numero_primer_tabla);
				log_info(log_ejecucion_kernel, "Numero de primer tabla: %i", numero_primer_tabla);

				sem_wait(&escritura_log);
				log_info(log_ejecucion_kernel, "Datos recibidos: process id %i, numero_primer_tabla %i", process_id, numero_primer_tabla);
				sem_post(&escritura_log);
				suspender_proceso(process_id, numero_primer_tabla);
				printf("Tablas suspendidas \n");
				log_info(log_ejecucion_kernel, "Tablas suspendidas");

				sem_wait(&escritura_log);
					log_info(log_ejecucion_kernel, "Tablas suspendidas.");
				sem_post(&escritura_log);

				send(socket_kernel, &OK, sizeof(uint32_t), 0);

				sem_wait(&escritura_log);
				log_info(log_ejecucion_kernel, "# FIN SUSPENSION_PROCESO");
				sem_post(&escritura_log);
				printf("# Fin # \n\n");
				log_info(log_ejecucion_kernel, "# Fin # \n\n");
				break;
			}

			case finalizacion_proceso:
				sem_wait(&escritura_log);
				log_info(log_ejecucion_kernel, "# FINALIZACION_PROCESO");
				sem_post(&escritura_log);

				printf("# Solicitud finalizar proceso (kernel) # \n");
				log_info(log_ejecucion_kernel, "# Solicitud finalizar proceso (kernel) #");
				printf("Datos recibidos: \n");
				log_info(log_ejecucion_kernel, "Datos recibidos:");

				recv(socket_kernel, &process_id, sizeof(uint32_t), MSG_WAITALL);
				printf("Process id: %i \n", process_id);
				log_info(log_ejecucion_kernel, "Process id: %i \n", process_id);
				recv(socket_kernel, &numero_primer_tabla, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de primer tabla: %i \n", numero_primer_tabla);
				log_info(log_ejecucion_kernel, "Numero de primer tabla: %i", numero_primer_tabla);
				sem_wait(&escritura_log);
				log_info(log_ejecucion_kernel, "Datos recibidos: process id %i, numero primer tabla %i", process_id, numero_primer_tabla);
				sem_post(&escritura_log);

				borrar_swap(process_id);
				liberar_memoria(numero_primer_tabla);


				printf("Archivo %i.swap eliminado \n", process_id);
				log_info(log_ejecucion_kernel, "Archivo %i.swap eliminado", process_id);
				printf("Memoria ocupada por el proceso %i eliminada \n", process_id);
				log_info(log_ejecucion_kernel, "Memoria ocupada por el proceso %i eliminada", process_id);

				send(socket_kernel, &OK, sizeof(uint32_t), 0);
				printf("# Fin # \n\n");
				log_info(log_ejecucion_kernel, "# Fin # \n\n");
				break;

			default:
				printf("codigo erroneo enviado por kernel \n");
				log_error(log_ejecucion_kernel, "codigo de operacion erroneo");
		}
	}

	return "";
}



uint32_t crear_tablas_necesarias( uint32_t espacio_de_direcciones ){
	int (*tabla)[ENTRADAS_POR_TABLA]= malloc(sizeof(int)*ENTRADAS_POR_TABLA);
	for(int i=0 ; i < ENTRADAS_POR_TABLA ; i++){
		(*tabla)[i] = -1;
	}
	int cantidad_paginas_necesaria = calcular_cantidad_de_paginas(espacio_de_direcciones);

	sem_wait(&escritura_log);
		log_info(log_ejecucion_kernel, "El proceso necesitara %i paginas", cantidad_paginas_necesaria);
	sem_post(&escritura_log);

	int cantidad_tablas_2do_nivel_necesarias = calcular_cantidad_de_tablas(cantidad_paginas_necesaria);

	sem_wait(&escritura_log);
		log_info(log_ejecucion_kernel, "La cantidad de tablas de 2do nivel necesaria es %i", cantidad_tablas_2do_nivel_necesarias);
	sem_post(&escritura_log);

	crear_tablas_2do_lvl(tabla, cantidad_tablas_2do_nivel_necesarias);

	sem_wait(&escritura_log);
		log_info(log_ejecucion_kernel, "Tablas de segundo nivel creadas");
	sem_post(&escritura_log);

	list_add(tabla_de_paginas_de_primer_nivel, tabla);

	sem_wait(&escritura_log);
		log_info(log_ejecucion_kernel, "tabla 1er nivel numero %i creada", list_size(tabla_de_paginas_de_primer_nivel) - 1);
	sem_post(&escritura_log);

	int (*puntero_del_clock) = malloc(sizeof(int));
	*puntero_del_clock = 0;
	list_add(punteros_clock, puntero_del_clock);

	sem_wait(&escritura_log);
		log_info(log_ejecucion_kernel, "puntero numero %i creado, inicializado en 0", list_size(punteros_clock) - 1);
	sem_post(&escritura_log);


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
		pagina_t (*tabla2)[ENTRADAS_POR_TABLA] = malloc(sizeof(pagina_t)*ENTRADAS_POR_TABLA);

		sem_wait(&operacion_en_lista_de_tablas);
		list_add(tabla_de_paginas_de_segundo_nivel, tabla2);

		(*tabla)[i] = list_size(tabla_de_paginas_de_segundo_nivel) - 1;
		sem_post(&operacion_en_lista_de_tablas);
	}

	log_info(log_ejecucion_kernel, "Se crearon %i tablas", cantidad_de_tablas);
}




void suspender_proceso(uint32_t process_id, uint32_t numero_primer_tabla){
	int (*puntero_a_tabla)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_primer_nivel, numero_primer_tabla);
	sem_wait(&escritura_log);
		log_info(log_ejecucion_kernel, "# Suspendiendo procesos tabla %i", numero_primer_tabla);
	sem_post(&escritura_log);
	for(int i = 0 ; i < ENTRADAS_POR_TABLA ; i++){
		if( (*puntero_a_tabla)[i] == -1 ) break;
		pagina_t (*puntero_a_tabla_2do_nivel)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_segundo_nivel, (*puntero_a_tabla)[i]);
		sem_wait(&escritura_log);
			log_info(log_ejecucion_kernel, "Mirando entrada %i de la tabla de primer nivel", i);
		sem_post(&escritura_log);
		for(int j = 0 ; j < ENTRADAS_POR_TABLA; j++){
			sem_wait(&escritura_log);
				log_info(log_ejecucion_kernel, "Mirando entrada %i de la tabla de segundo nivel", j);
			sem_post(&escritura_log);
			if( (*puntero_a_tabla_2do_nivel)[j].bit_presencia == 1 ){

				if( (*puntero_a_tabla_2do_nivel)[j].bit_modificacion == 1 ){
					guardar_pagina_en_swap((*puntero_a_tabla_2do_nivel)[j], process_id, i * ENTRADAS_POR_TABLA + j);
					(*puntero_a_tabla_2do_nivel)[j].bit_modificacion = 0;
				}

				(*puntero_a_tabla_2do_nivel)[j].bit_presencia = 0;
				poner_bit_en_0_bitmap( (*puntero_a_tabla_2do_nivel)[j].numero_frame );

				sem_wait(&escritura_log);
					log_info(log_ejecucion_kernel, "Bit de presencia en 1, liberando frame %i", (*puntero_a_tabla_2do_nivel)[j].numero_frame);
				sem_post(&escritura_log);
			}
		}
	}
}

void liberar_memoria(uint32_t numero_primer_tabla){
	int (*puntero_a_tabla)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_primer_nivel, numero_primer_tabla);

	for(int i = 0 ; i < ENTRADAS_POR_TABLA ; i++){
		if( (*puntero_a_tabla)[i] == -1 ) break;
		pagina_t (*puntero_a_tabla_2do_nivel)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_segundo_nivel, (*puntero_a_tabla)[i]);

		for(int j = 0 ; j < ENTRADAS_POR_TABLA; j++){
			if( (*puntero_a_tabla_2do_nivel)[j].bit_presencia == 1 ){
				(*puntero_a_tabla_2do_nivel)[j].bit_presencia = 0;
				poner_bit_en_0_bitmap( (*puntero_a_tabla_2do_nivel)[j].numero_frame );
			}
		}

		free(puntero_a_tabla_2do_nivel);
	}

	free(puntero_a_tabla);

	sem_wait(&escritura_log);
		log_info(log_ejecucion_kernel, "memoria ocupada por la tabla %i liberada", numero_primer_tabla);
	sem_post(&escritura_log);
}

void poner_bit_en_0_bitmap(uint32_t numero_de_frame){
	int* puntero_bitmap = list_get(bitmap_memoria, numero_de_frame);
	sem_wait(&operacion_en_bitmap);
	*puntero_bitmap = 0;
	sem_post(&operacion_en_bitmap);

	sem_wait(&escritura_log);
		log_info(log_ejecucion_kernel, "frame %i liberado", numero_de_frame);
	sem_post(&escritura_log);
}

void poner_bit_en_1_bitmap(uint32_t numero_de_frame){
	int* puntero_bitmap = list_get(bitmap_memoria, numero_de_frame);
	sem_wait(&operacion_en_bitmap);
	*puntero_bitmap = 1;
	sem_post(&operacion_en_bitmap);
}

void* buscar_frame(uint32_t numero_de_frame){
	void* frame = memoria_real + numero_de_frame * TAMANIO_PAGINA;

	return frame;
}

void crear_archivo_swap(uint32_t process_id){
	char* ruta_archivo = obtener_ruta_archivo(process_id);
	printf("ruta archivo %s \n", ruta_archivo);

	sem_wait(&operacion_swap);
	usleep(RETARDO_SWAP * 1000);

    FILE* archivo = fopen(ruta_archivo, "wb");
	sem_post(&operacion_swap);

	sem_wait(&escritura_log);
		log_info(log_ejecucion_kernel, "archivo %s creado", ruta_archivo);
	sem_post(&escritura_log);

    free(ruta_archivo);
    fclose(archivo);
}

void guardar_pagina_en_swap(pagina_t pagina, uint32_t process_id, uint32_t numero_de_pagina){
	char* ruta_archivo = obtener_ruta_archivo(process_id);
	FILE* swap = fopen(ruta_archivo, "rb+");
	void* frame_a_copiar = buscar_frame(pagina.numero_frame);

	fseek(swap, numero_de_pagina * TAMANIO_PAGINA, SEEK_SET);

	sem_wait(&operacion_swap);

	ACCESOS_SWAP++;
	printf("################ TOTAL ACCESOS A SWAP: %i \n", ACCESOS_SWAP);
	log_info(log_ejecucion_kernel,"################ TOTAL ACCESOS A SWAP: %i", ACCESOS_SWAP);

	usleep(RETARDO_SWAP * 1000);

	sem_wait(&operacion_en_memoria);
		fwrite(frame_a_copiar, TAMANIO_PAGINA, 1, swap);
	sem_post(&operacion_en_memoria);

	sem_post(&operacion_swap);

	sem_wait(&escritura_log);
		log_info(log_ejecucion_kernel, "pagina %i guardada en %s", numero_de_pagina, ruta_archivo);
	sem_post(&escritura_log);

	free(ruta_archivo);
	fclose(swap);
}

void borrar_swap(uint32_t process_id){
	sem_wait(&operacion_swap);
	usleep(RETARDO_SWAP * 1000);
	remove(obtener_ruta_archivo(process_id));
	sem_post(&operacion_swap);

	sem_wait(&escritura_log);
		log_info(log_ejecucion_kernel, "archivo %i.swap borradp", process_id);
	sem_post(&escritura_log);
}

char* obtener_ruta_archivo(uint32_t process_id){
	char* nombre_archivo = string_itoa(process_id);
	string_append(&nombre_archivo,".swap");

	char* ruta_archivo = string_new();
	string_append(&ruta_archivo, PATH_SWAP);
	string_append(&ruta_archivo, "/");
	string_append(&ruta_archivo, nombre_archivo);

	free(nombre_archivo);


	return ruta_archivo;
}













