#include "hilo_kernel.h"

void* hilo_kernel(void* ptr_void_socket){
	sem_post(&hilo_iniciado);

	//atender peticiones del KERNEL
	uint32_t codigo_recibido = 10;
	while(1){
		recv(socket_kernel, &codigo_recibido, sizeof(uint32_t), MSG_WAITALL);
		switch(codigo_recibido){
			case crear_tablas:
				// basicamente un list add a la lista de tablas, y crear la cantidad
				// de tablas de segundo nivel necesarias.
				uint32_t process_id;
				uint32_t espacio_de_direcciones;
				recv(socket_kernel, &process_id, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_kernel, &process_id, sizeof(uint32_t), MSG_WAITALL);

				crear_tabla_por_nivel( espacio_de_direcciones );

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
