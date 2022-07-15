#include "main_memoria.h"
#include "hilo_kernel.h"
#include "hilo_cpu.h"
#include <pthread.h>

void* hilo_cpu(void* socket_cpu_void){
	int socket_cpu = *(int*)socket_cpu_void;

	uint32_t process_id;
	uint32_t numero_pagina;
	pagina_t pagina_a_buscar;
	uint32_t codigo_recibido;
	uint32_t numero_de_entrada;
	uint32_t numero_tabla_1er_nivel_leer;
	uint32_t numero_tabla_2do_nivel_leer;
	uint32_t numero_de_frame;
	uint32_t offset;
	uint32_t a_enviar;
	uint32_t numero_2da_tabla;
	uint32_t valor_a_escribir;
	uint32_t OK = 1;

	while(1){
		recv(socket_cpu, &codigo_recibido, sizeof(uint32_t), MSG_WAITALL);

		switch(codigo_recibido){
			case solicitud_num_tabla_2:
				// con el numero de entrada + el numero de tabla que envia cpu, hay
				// que devolver el numero de la segunda tabla
				recv(socket_cpu, &numero_tabla_1er_nivel_leer, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_cpu, &numero_de_entrada, sizeof(uint32_t), MSG_WAITALL);

				numero_2da_tabla = buscar_tabla_2do_nivel(numero_tabla_1er_nivel_leer, numero_de_entrada);

				usleep(RETARDO_MEMORIA * 1000);

				send(socket_cpu, &numero_2da_tabla, sizeof(uint32_t), 0);

				break;
			case solicitud_num_frame:

				recv(socket_cpu, &process_id, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_cpu, &numero_tabla_1er_nivel_leer, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_cpu, &numero_tabla_2do_nivel_leer, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_cpu, &numero_de_entrada, sizeof(uint32_t), MSG_WAITALL);

				pagina_t* pagina_buscada = buscar_pagina(numero_tabla_2do_nivel_leer, numero_de_entrada);

				if( pagina_buscada->bit_presencia == 1 ){
					a_enviar = pagina_buscada->numero_frame;
				}
				else
				{
					numero_pagina = calcular_num_pagina(numero_tabla_1er_nivel_leer, numero_tabla_2do_nivel_leer, numero_de_entrada);
					cargar_a_memoria(numero_tabla_1er_nivel_leer, numero_tabla_2do_nivel_leer,pagina_buscada,numero_pagina);
				}


				send(socket_cpu, &a_enviar, sizeof(uint32_t), 0);

				printf("responder con numero de frame \n");
				break;
			case solicitud_lectura:

				recv(socket_cpu, &numero_de_frame, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_cpu, &offset, sizeof(uint32_t), MSG_WAITALL);

				a_enviar = leer_de_memoria(numero_de_frame, offset);

				usleep(RETARDO_MEMORIA * 1000);
				send(socket_cpu, &a_enviar, sizeof(uint32_t), 0);

				break;
			case solicitud_escritura:

				recv(socket_cpu, &numero_de_frame, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_cpu, &offset, sizeof(uint32_t), MSG_WAITALL);
				recv(socket_cpu, &valor_a_escribir, sizeof(uint32_t), MSG_WAITALL);

				escribir_en_memoria(numero_de_frame, offset, valor_a_escribir);

				usleep(RETARDO_MEMORIA * 1000);
				send(socket_cpu, &OK, sizeof(uint32_t), 0);
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

uint32_t leer_de_memoria(uint32_t numero_de_frame, uint32_t offset){
	uint32_t numero_leido;

	memcpy(&numero_leido, memoria_real + numero_de_frame * TAMANIO_PAGINA + offset, sizeof(uint32_t));

	return numero_leido;
}


void escribir_en_memoria(uint32_t numero_de_frame,uint32_t offset,uint32_t valor_a_escribir){
	memcpy( memoria_real + numero_de_frame * TAMANIO_PAGINA + offset, &valor_a_escribir, sizeof(uint32_t));
}

pagina_t* buscar_pagina(uint32_t numero_tabla_2do_nivel_leer, uint32_t numero_de_entrada){
	pagina_t (*puntero_a_tabla)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_segundo_nivel, numero_tabla_2do_nivel_leer);

	return &(*puntero_a_tabla)[numero_de_entrada];
}


void cargar_a_memoria(uint32_t numero_tabla_1er_nivel_leer, uint32_t numero_tabla_2do_nivel_leer, pagina_t* pagina_buscada, uint32_t num_pagina){
	uint32_t numero_de_frame;
	// asignacion fija = 5;
	// marcos por proceso < cant maxima => busca un lugar libre y ya esta
	// marcos por proceso == cant maxima => usa clock o clock-m


	if( es_clock() ){
		algoritmo_clock(numero_tabla_1er_nivel_leer, pagina_buscada);
	}
	else
	{

	}
}


uint32_t calcular_num_pagina(uint32_t numero_tabla_1er_nivel_leer,uint32_t numero_tabla_2do_nivel_leer, uint32_t numero_de_entrada){
	uint32_t fulbo;
	int (*ptr)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_primer_nivel, numero_tabla_1er_nivel_leer);

	int i = 0;

	for(i = 0; i < ENTRADAS_POR_TABLA ; i++){
		if( (*ptr)[i] == numero_tabla_2do_nivel_leer){
			break;
		}
	}

	fulbo = i * ENTRADAS_POR_TABLA + numero_de_entrada;

	return fulbo;
}


