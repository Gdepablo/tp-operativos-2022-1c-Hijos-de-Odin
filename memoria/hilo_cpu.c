#include "main_memoria.h"
#include "hilo_kernel.h"
#include <pthread.h>

void* hilo_cpu(void* socket_cpu_void){
	int socket_cpu = *(int*)socket_cpu_void;

	sem_post(&hilo_iniciado);

	uint32_t codigo_recibido;

	while(1){
		recv(socket_cpu, &codigo_recibido, sizeof(uint32_t), MSG_WAITALL);

		switch(codigo_recibido){
			case solicitud_num_tabla_2:
				// con el numero de entrada + el numero de tabla que envia cpu, hay
				// que devolver el numero de la segunda tabla
				printf("responder con numero de tabla 2 \n");
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
