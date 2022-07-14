#include "hilo_kernel.h"
#include "main_memoria.h"
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
			case crear_tablas:
				// basicamente un list add a la lista de tablas, y crear la cantidad
				// de tablas de segundo nivel necesarias.

				recv(socket_kernel, &process_id, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_kernel, &espacio_de_direcciones, sizeof(uint32_t), MSG_WAITALL);

				crear_tabla_por_nivel( espacio_de_direcciones, process_id );

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

void crear_tabla_por_nivel( uint32_t espacio_de_direcciones,uint32_t process_id ){
	int (*tabla) [ENTRADAS_POR_TABLA]= malloc(sizeof(int)*ENTRADAS_POR_TABLA);


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

void crear_tabla_2do_lvl(int cantidad_de_tablas){
	for(int i=0; i < cantidad_de_tablas; i++ ){
		pagina_t (*tabla2) [ENTRADAS_POR_TABLA]= malloc(sizeof(pagina_t)*ENTRADAS_POR_TABLA);
	}

}
