#include "main_memoria.h"
#include "hilo_kernel.h"
#include "hilo_cpu.h"
#include <pthread.h>

void* hilo_cpu(void* socket_cpu_void){
	int socket_cpu = *(int*)socket_cpu_void;

	uint32_t codigo_recibido;
	uint32_t numero_de_entrada;
	uint32_t numero_tabla_1er_nivel;

	while(1){
		recv(socket_cpu, &codigo_recibido, sizeof(uint32_t), MSG_WAITALL);

		switch(codigo_recibido){
			case solicitud_num_tabla_2:
				// con el numero de entrada + el numero de tabla que envia cpu, hay
				// que devolver el numero de la segunda tabla
				recv(socket_cpu, &numero_tabla_1er_nivel, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_cpu, &numero_de_entrada, sizeof(uint32_t), MSG_WAITALL);

				uint32_t numero_2da_tabla = buscar_tabla_2do_nivel(numero_tabla_1er_nivel, numero_de_entrada);

				usleep(RETARDO_MEMORIA * 1000);

				send(socket_cpu, &numero_2da_tabla, sizeof(uint32_t), 0);

				break;
			case solicitud_num_frame:
				// con el numero de segunda tabla + entrada de la segunda tabla,
				// hay que devolver el numero de frame en el que se encuentra. Si no
				// esta cargada en memoria, hay que cargarla y responder con el
				// numero de frame.
				printf("responder con numero de frame \n");
				break;
			case solicitud_lectura:
				// hay que fijarse que hay en el frame dado.
				printf("responde contenido en numero de frame con el offset dado \n");
				break;
			case solicitud_escritura:
				// hay que escribir en el frame dado
				printf("escribir en el frame con offset otorgado \n");
				break;
			default:
				printf("codigo erroneo enviado por kernel \n");
		}
	}
	return "";
}



uint32_t buscar_tabla_2do_nivel(uint32_t numero_tabla_1er_nivel, uint32_t numero_de_entrada){
	int (*ptr_tabla_1er_nivel)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_primer_nivel, numero_tabla_1er_nivel);

	return (*ptr_tabla_1er_nivel)[numero_de_entrada];
}












