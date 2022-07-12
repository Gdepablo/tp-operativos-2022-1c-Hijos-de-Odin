#include "hilo_kernel.h"

void* hilo_kernel(void* ptr_void_socket){
	sem_post(&hilo_iniciado);

	//atender peticiones del KERNEL
	uint32_t codigo_recibido = 10;
	while(1){
		recv(socket_kernel, &codigo_recibido, sizeof(uint32_t), MSG_WAITALL);
		switch(codigo_recibido){
			case 4:
				// basicamente un list add a la lista de tablas, y crear la cantidad
				// de tablas de segundo nivel necesarias.
				printf("crear las tablas y responder el nro de 1er tabla \n");
				break;
			case 5:
				// si kernel nos envia tanto el PID como el numero de 1er tabla, es
				// facilito
				printf("sacar de memoria todas las paginas del proceso \n");
				break;
			case 6:
				// borrar el .swap y sacar las entradas en memoria si es que hay.
				printf("borrar el archivo .swap, dejar las tablas \n");
				break;
			default:
				printf("codigo erroneo enviado por kernel \n");
		}
	}

	return "";
}
