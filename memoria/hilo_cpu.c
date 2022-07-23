#include "main_memoria.h"
#include "hilo_kernel.h"
#include "hilo_cpu.h"
#include <pthread.h>

int PAGINAS_SACADAS = 0;


void* hilo_cpu(void* socket_cpu_void){
	sem_wait(&escritura_log);
	log_info(log_ejecucion_main, "hilo cpu iniciado");
	sem_post(&escritura_log);

	int socket_cpu = *(int*)socket_cpu_void;

	uint32_t process_id;
	uint32_t numero_pagina;
	uint32_t codigo_recibido;
	uint32_t numero_de_entrada;
	uint32_t numero_de_entrada_2;
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
		sem_wait(&escritura_log);
		log_info(log_ejecucion_main, "\n# SOLICITUD RECIBIDA DE CPU #");
		sem_post(&escritura_log);


		switch(codigo_recibido){
			case solicitud_num_tabla_2:
				sem_wait(&escritura_log);
				log_info(log_ejecucion_main, "# SOLICITUD NUMERO DE TABLA 2");
				sem_post(&escritura_log);
				printf("# Solicitud numero de tabla 2 (cpu) #\n");
				printf("Datos recibidos: \n");
				recv(socket_cpu, &numero_tabla_1er_nivel_leer, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de primer tabla: %i \n", numero_tabla_1er_nivel_leer);
				log_info(log_ejecucion_main,"Numero de primer tabla: %i", numero_tabla_1er_nivel_leer);
				recv(socket_cpu, &numero_de_entrada, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de entrada: %i \n", numero_de_entrada);
				log_info(log_ejecucion_main,"Numero de entrada: %i \n", numero_de_entrada);

				sem_wait(&escritura_log);
				log_info(log_ejecucion_main,
						"datos recibidos: numero_tabla_1er_nivel_leer %i numero_de_entrada %i",
						numero_tabla_1er_nivel_leer,
						numero_de_entrada);
				sem_post(&escritura_log);

				numero_2da_tabla = buscar_tabla_2do_nivel(numero_tabla_1er_nivel_leer, numero_de_entrada);

				usleep(RETARDO_MEMORIA * 1000);

				printf("# Valor devuelto: %i \n", numero_2da_tabla);
				log_info(log_ejecucion_main,"# Valor devuelto: %i \n", numero_2da_tabla);

				send(socket_cpu, &numero_2da_tabla, sizeof(uint32_t), 0);
				printf("# Fin #\n\n");

				sem_wait(&escritura_log);
					log_info(log_ejecucion_main, "# FIN SOLICITUD NUMERO DE TABLA 2 \n\n");
				sem_post(&escritura_log);
				break;
			case solicitud_num_frame:
				printf("# Solicitud numero de frame (cpu) #\n");
				printf("Datos recibidos: \n");
				log_info(log_ejecucion_main,"# Solicitud numero de frame (cpu) #");
				log_info(log_ejecucion_main,"Datos recibidos: ");
				recv(socket_cpu, &process_id, sizeof(uint32_t), MSG_WAITALL);
				printf("Process id: %i \n", process_id);
				log_info(log_ejecucion_main,"Process id: %i", process_id);
				recv(socket_cpu, &numero_tabla_2do_nivel_leer, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de tabla 2: %i \n", numero_tabla_2do_nivel_leer);
				log_info(log_ejecucion_main,"Numero de tabla 2: %i", numero_tabla_2do_nivel_leer);
				recv(socket_cpu, &numero_de_entrada_2, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de entrada 2: %i \n", numero_de_entrada_2);
				log_info(log_ejecucion_main,"Numero de entrada 2: %i", numero_de_entrada_2);
				recv(socket_cpu, &numero_tabla_1er_nivel_leer, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de 1er tabla: %i \n", numero_tabla_1er_nivel_leer);
				log_info(log_ejecucion_main,"Numero de 1er tabla: %i", numero_tabla_1er_nivel_leer);

				pagina_t* pagina_buscada = buscar_pagina(numero_tabla_2do_nivel_leer, numero_de_entrada_2);

				if( pagina_buscada->bit_presencia == 1 ){
					a_enviar = pagina_buscada->numero_frame;
				}
				else
				{

					numero_pagina = calcular_num_pagina(numero_tabla_1er_nivel_leer, numero_tabla_2do_nivel_leer, numero_de_entrada_2);
					cargar_a_memoria(numero_tabla_1er_nivel_leer,pagina_buscada,numero_pagina, process_id);
					a_enviar = pagina_buscada->numero_frame;
				}

				usleep(RETARDO_MEMORIA * 1000);
				printf("# Valor devuelto: %i \n", a_enviar);
				log_info(log_ejecucion_main,"# Valor devuelto: %i \n", a_enviar);
				send(socket_cpu, &a_enviar, sizeof(uint32_t), 0);
				printf("# Fin # \n\n");
				log_info(log_ejecucion_main,"# Fin # %i \n\n", a_enviar);
				break;
			case solicitud_lectura:
				printf("# Solicitud de lectura (cpu) #\n");
				log_info(log_ejecucion_main,"# Solicitud de lectura (cpu) #");
				printf("Datos recibidos: \n");
				log_info(log_ejecucion_main,"Datos recibidos:");

				recv(socket_cpu, &numero_de_frame, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de frame: %i \n", numero_de_frame);
				log_info(log_ejecucion_main,"Numero de frame: %i", numero_de_frame);
				recv(socket_cpu, &offset, sizeof(uint32_t), MSG_WAITALL);
				printf("Offset: %i \n", offset);
				log_info(log_ejecucion_main,"Offset: %i ", offset);
				recv(socket_cpu, &numero_tabla_1er_nivel_leer, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de tabla primer nivel: %i \n", numero_tabla_1er_nivel_leer);
				log_info(log_ejecucion_main,"Numero de tabla primer nivel: %i", numero_tabla_1er_nivel_leer);
				recv(socket_cpu, &numero_de_entrada, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de entrada tabla nivel 1: %i \n", numero_de_entrada);
				log_info(log_ejecucion_main,"Numero de entrada tabla nivel 1: %i", numero_de_entrada);

				a_enviar = leer_de_memoria(numero_de_frame, offset);

				bit_de_uso_en_1(numero_tabla_1er_nivel_leer, numero_de_entrada, numero_de_frame);

				usleep(RETARDO_MEMORIA * 1000);
				printf("# Valor devuelto: %i \n", a_enviar);
				log_info(log_ejecucion_main,"# Valor devuelto: %i", a_enviar);
				send(socket_cpu, &a_enviar, sizeof(uint32_t), 0);

				printf("# Fin # \n\n");
				break;
			case solicitud_escritura:
				printf("# Solicitud escritura (cpu) #\n");
				log_info(log_ejecucion_main,"# Valor devuelto: %i", a_enviar);
				printf("Datos recibidos: \n");
				log_info(log_ejecucion_main,"Datos recibidos:");

				recv(socket_cpu, &numero_de_frame, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de frame: %i \n", numero_de_frame);
				log_info(log_ejecucion_main,"Numero de frame: %i", numero_de_frame);
				recv(socket_cpu, &offset, sizeof(uint32_t), MSG_WAITALL);
				printf("Offset: %i \n", offset);
				log_info(log_ejecucion_main,"Offset: %i", offset);
				recv(socket_cpu, &valor_a_escribir, sizeof(uint32_t), MSG_WAITALL);
				printf("Valor a escribir: %i \n", valor_a_escribir);
				log_info(log_ejecucion_main,"Valor a escribir: %i", valor_a_escribir);
				recv(socket_cpu, &numero_tabla_1er_nivel_leer, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de tabla de primer nivel: %i \n", numero_tabla_1er_nivel_leer);
				log_info(log_ejecucion_main,"Numero de tabla de primer nivel: %i", numero_tabla_1er_nivel_leer);
				recv(socket_cpu, &numero_de_entrada, sizeof(uint32_t), MSG_WAITALL);
				printf("Numero de entrada tabla nivel 1: %i \n", numero_de_entrada);
				log_info(log_ejecucion_main,"Numero de entrada tabla nivel 1: %i", numero_de_entrada);

				escribir_en_memoria(numero_de_frame, offset, valor_a_escribir);

				bit_de_uso_en_1(numero_tabla_1er_nivel_leer, numero_de_entrada, numero_de_frame);
				bit_de_modificado_en_1(numero_tabla_1er_nivel_leer, numero_de_entrada, numero_de_frame);

				usleep(RETARDO_MEMORIA * 1000);
				send(socket_cpu, &OK, sizeof(uint32_t), 0);

				printf("# Fin # \n\n");
				log_info(log_ejecucion_main, "# Fin # \n\n");
				break;
			default:
				printf("codigo erroneo enviado por kernel \n");
				log_error(log_ejecucion_main, "Código erróneo enviado por Kernel");

		}
	}
	return "";
}

t_list* buscar_paginas_presentes(int (*tabla_primer_nivel)[]){
	t_list* lista = list_create();

	for(int i = 0 ; i < ENTRADAS_POR_TABLA ; i++){
		if( (*tabla_primer_nivel)[i] == -1 ) break;
		pagina_t (*puntero_a_tabla_2do_nivel)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_segundo_nivel, (*tabla_primer_nivel)[i]);
		for(int j = 0 ; j < ENTRADAS_POR_TABLA; j++){
			if( (*puntero_a_tabla_2do_nivel)[j].bit_presencia == 1 ){
				list_add(lista, &(*puntero_a_tabla_2do_nivel)[j]);
			}
		}
	}

	return lista;
}

uint32_t buscar_tabla_2do_nivel(uint32_t numero_tabla_1er_nivel, uint32_t numero_de_entrada){
	int (*ptr_tabla_1er_nivel)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_primer_nivel, numero_tabla_1er_nivel);

	sem_wait(&escritura_log);
		log_info(log_ejecucion_main, "Numero de tabla 2 encontrado: %i", (*ptr_tabla_1er_nivel)[numero_de_entrada]);
	sem_post(&escritura_log);

	return (*ptr_tabla_1er_nivel)[numero_de_entrada];
}

uint32_t leer_de_memoria(uint32_t numero_de_frame, uint32_t offset){
	uint32_t numero_leido;

	sem_wait(&operacion_en_memoria);
		memcpy(&numero_leido, memoria_real + numero_de_frame * TAMANIO_PAGINA + offset, sizeof(uint32_t));
	sem_post(&operacion_en_memoria);

	return numero_leido;
}


void escribir_en_memoria(uint32_t numero_de_frame,uint32_t offset,uint32_t valor_a_escribir){
	sem_wait(&operacion_en_memoria);
		memcpy( memoria_real + numero_de_frame * TAMANIO_PAGINA + offset, &valor_a_escribir, sizeof(uint32_t));
	sem_post(&operacion_en_memoria);
}

pagina_t* buscar_pagina(uint32_t numero_tabla_2do_nivel, uint32_t numero_de_entrada){
	pagina_t (*puntero_a_tabla)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_segundo_nivel, numero_tabla_2do_nivel);



	return &(*puntero_a_tabla)[numero_de_entrada];
}


void cargar_a_memoria(uint32_t numero_tabla_1er_nivel, pagina_t* pagina_a_cargar_a_memoria, uint32_t num_pagina, uint32_t process_id){
	t_list* paginas_presentes = buscar_paginas_presentes(list_get(tabla_de_paginas_de_primer_nivel, numero_tabla_1er_nivel));

	if( list_size(paginas_presentes) < MARCOS_POR_PROCESO && memoria_no_llena() ){
		int frame = buscar_frame_libre();
		void* a_copiar = copiar_de_swap(num_pagina, process_id);
		sem_wait(&operacion_en_memoria);
			memcpy(memoria_real + frame * TAMANIO_PAGINA , a_copiar, TAMANIO_PAGINA);
		sem_post(&operacion_en_memoria);
		poner_bit_en_1_bitmap(frame);
		pagina_a_cargar_a_memoria->bit_presencia = 1;
		pagina_a_cargar_a_memoria->bit_uso = 1;
		pagina_a_cargar_a_memoria->numero_frame = frame;
		free(a_copiar);
	}
	else
	{
		PAGINAS_SACADAS++;
		printf("SACO UNA PAGINA, TOTAL HASTA AHORA: %i \n", PAGINAS_SACADAS);
		log_info(log_ejecucion_main, "PAGINA SACADA, TOTAL: %i", PAGINAS_SACADAS);
		pagina_t* pagina_a_reemplazar = elegir_pagina_a_sacar(numero_tabla_1er_nivel);

		if( pagina_a_reemplazar->bit_modificacion == 1 ){
			int num_pagina = calcular_numero_de_pagina_a_reemplazar(pagina_a_reemplazar, numero_tabla_1er_nivel);
			guardar_pagina_en_swap( *pagina_a_reemplazar, process_id, num_pagina);
			pagina_a_reemplazar->bit_modificacion=0;
		}
		pagina_a_reemplazar->bit_presencia=0;

		cargar_pagina_a_memoria( num_pagina, pagina_a_reemplazar->numero_frame, process_id );
		pagina_a_cargar_a_memoria->bit_presencia=1;
		pagina_a_cargar_a_memoria->bit_uso=1;
		pagina_a_cargar_a_memoria->numero_frame = pagina_a_reemplazar->numero_frame;
	}
	list_destroy(paginas_presentes);
}

pagina_t* elegir_pagina_a_sacar(uint32_t numero_primer_tabla){
	int (*puntero_a_tabla)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_primer_nivel, numero_primer_tabla);

	t_list (*paginas_en_memoria) = list_create();

	for(int i = 0 ; i < ENTRADAS_POR_TABLA ; i++){
		if( (*puntero_a_tabla)[i] == -1 ) break;
		pagina_t (*puntero_a_tabla_2do_nivel)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_segundo_nivel, (*puntero_a_tabla)[i]);

		for(int j = 0 ; j < ENTRADAS_POR_TABLA; j++){
			if( (*puntero_a_tabla_2do_nivel)[j].bit_presencia == 1 ){
				list_add_sorted(paginas_en_memoria, &(*puntero_a_tabla_2do_nivel)[j], comparador);
			}
		}
	}

	printf("CANTIDAD DE PAGINAS EN MEMORIA: %i \n", list_size(paginas_en_memoria));
	log_info(log_ejecucion_main,"CANTIDAD DE PAGINAS EN MEMORIA: %i", list_size(paginas_en_memoria));

	pagina_t* puntero_pagina;

	if( !strcmp(ALGORITMO_REEMPLAZO, "CLOCK") ){
		puntero_pagina = clock_comun(numero_primer_tabla, paginas_en_memoria);
	}
	else
	{
		printf("es clock modificado \n");
		puntero_pagina = clock_mejorado(numero_primer_tabla, paginas_en_memoria);
	}

	list_destroy(paginas_en_memoria);

	return puntero_pagina;
}

void cargar_pagina_a_memoria(uint32_t numero_de_pagina, uint32_t numero_de_frame, uint32_t process_id){
	char* ruta_archivo = obtener_ruta_archivo(process_id);
	FILE* archivo_swap = fopen( ruta_archivo, "rb" );
	fseek(archivo_swap, numero_de_pagina * TAMANIO_PAGINA, SEEK_SET);
	void* a_copiar = malloc(TAMANIO_PAGINA);
	// MOMENTO SWAP
	sem_wait(&operacion_swap);

	ACCESOS_SWAP++;
	printf("#################### TOTAL ACCESOS A SWAP: %i \n", ACCESOS_SWAP);
	log_info(log_ejecucion_main,"#################### TOTAL ACCESOS A SWAP: %i", ACCESOS_SWAP);

	usleep(RETARDO_SWAP * 1000);
	fread(a_copiar, TAMANIO_PAGINA, 1 , archivo_swap);

	sem_post(&operacion_swap);

	sem_wait(&operacion_en_memoria);
		memcpy( memoria_real + numero_de_frame * TAMANIO_PAGINA, a_copiar, TAMANIO_PAGINA );
	sem_post(&operacion_en_memoria);

	free(a_copiar);
	free(ruta_archivo);
	fclose(archivo_swap);
}

void* copiar_de_swap(uint32_t pagina, uint32_t process_id){
	char* ruta = obtener_ruta_archivo(process_id);
	FILE* archivo = fopen(ruta, "rb");
	fseek( archivo, pagina * TAMANIO_PAGINA, SEEK_SET);
	void* a_copiar = malloc(TAMANIO_PAGINA);

	ACCESOS_SWAP++;
	printf("TOTAL ACCESOS A SWAP: %i \n", ACCESOS_SWAP);
	log_info(log_ejecucion_main,"TOTAL ACCESOS A SWAP: %i", ACCESOS_SWAP);

	sem_wait(&operacion_swap);
	usleep(RETARDO_SWAP * 1000);
	fread( a_copiar, TAMANIO_PAGINA, 1, archivo );

	sem_post(&operacion_swap);

	fclose(archivo);

	return a_copiar;
}

int buscar_frame_libre(){
	int posicion = 0;
	int *ptr = list_get(bitmap_memoria, posicion);

	while( (*ptr) != 0 ){
		posicion++;
		ptr = list_get(bitmap_memoria, posicion);
	}

	return posicion;
}

bool esta_libre(void* void_bitmap){
	int bit = *(int*)void_bitmap;

	return bit == 0;
}

bool memoria_no_llena(){
	return list_any_satisfy(bitmap_memoria, esta_libre);
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


bool comparador(void* puntero_void_1, void* puntero_void_2){
    pagina_t pagina = *(pagina_t*)puntero_void_1;
    pagina_t pagina_2 = *(pagina_t*)puntero_void_2;

    return pagina.numero_frame < pagina_2.numero_frame;
}

pagina_t* clock_mejorado(uint32_t numero_de_tabla, t_list* paginas_en_memoria){
	int* puntero_clock = list_get(punteros_clock, numero_de_tabla);
	pagina_t* puntero_pagina;
	bool pagina_encontrada = false;
	while(pagina_encontrada == false){

		for(int i = 0 ; i < list_size(paginas_en_memoria) ; i++){
			puntero_pagina = list_get(paginas_en_memoria, *puntero_clock);
			log_info(log_ejecucion_main,"BUSCANDO 0,0");
			log_info(log_ejecucion_main,"Bit de uso: %i", puntero_pagina->bit_uso);
			log_info(log_ejecucion_main,"Bit de modificacion: %i", puntero_pagina->bit_modificacion);
			log_info(log_ejecucion_main,"Numero de frame: %i", puntero_pagina->numero_frame);

			(*puntero_clock)++;
            if( (*puntero_clock) == list_size(paginas_en_memoria) ){
                (*puntero_clock) = 0;
            }
			if( (puntero_pagina->bit_uso == 0) && (puntero_pagina->bit_modificacion == 0) ){
				pagina_encontrada = true;
				break;
			}
		}

		if( pagina_encontrada ) break;

		printf("NO HAY NINGUNO EN 0,0 \n");
		log_info(log_ejecucion_main,"NO HAY NINGUNO EN 0,0");


		for(int i = 0 ; i < list_size(paginas_en_memoria) ; i++){
			puntero_pagina = list_get(paginas_en_memoria, *puntero_clock);
			log_info(log_ejecucion_main,"BUSCANDO BIT DE USO EN 0");
			log_info(log_ejecucion_main,"Bit de uso: %i", puntero_pagina->bit_uso);
			log_info(log_ejecucion_main,"Bit de modificacion: %i", puntero_pagina->bit_modificacion);
			log_info(log_ejecucion_main,"Numero de frame: %i", puntero_pagina->numero_frame);
			(*puntero_clock)++;
            if( (*puntero_clock) == list_size(paginas_en_memoria) ){
                (*puntero_clock) = 0;
            }
			if( (puntero_pagina->bit_uso) == 0 ){
				pagina_encontrada = true;
				break;
			}
			else
			{
				puntero_pagina->bit_uso = 0;
			}
		}
	}

	return puntero_pagina;
}


pagina_t* clock_comun(uint32_t numero_de_tabla, t_list* paginas_en_memoria){
    int* puntero_clock = list_get(punteros_clock, numero_de_tabla);
    pagina_t* puntero_pagina;
    while(1){
        puntero_pagina = list_get(paginas_en_memoria, *puntero_clock);
        log_info(log_ejecucion_main,"########## pagina en el frame %i ", puntero_pagina->numero_frame);
        log_info(log_ejecucion_main,"puntero apuntado bit de uso en %i ", puntero_pagina->bit_uso);
        log_info(log_ejecucion_main,"puntero apuntado bit de presencia en %i", puntero_pagina->bit_presencia);
        log_info(log_ejecucion_main,"puntero apuntado bit de modificacion en %i", puntero_pagina->bit_modificacion);
        if(puntero_pagina->bit_uso == 0){
            (*puntero_clock)++;
            break;
        }
        else
        {
           (*puntero_clock)++;
           puntero_pagina->bit_uso = 0;
        }

        if( *puntero_clock == list_size(paginas_en_memoria) ){
            *puntero_clock = 0;
        }
    }

    return puntero_pagina;
}


int calcular_numero_de_pagina_a_reemplazar(pagina_t* pagina_a_reemplazar, uint32_t numero_primer_tabla){
	int (*puntero_a_tabla)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_primer_nivel, numero_primer_tabla);
	int contador = 0;


	for(int i = 0 ; i < ENTRADAS_POR_TABLA ; i++){
		if( (*puntero_a_tabla)[i] == -1 ) break;
		pagina_t (*puntero_a_tabla_2do_nivel)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_segundo_nivel, (*puntero_a_tabla)[i]);

		for(int j = 0 ; j < ENTRADAS_POR_TABLA; j++){
			if( (*puntero_a_tabla_2do_nivel)[j].bit_presencia == 1 && (*puntero_a_tabla_2do_nivel)[j].numero_frame == 1){
				contador = i * ENTRADAS_POR_TABLA + j;
			}
		}
	}

	return contador;
}


void bit_de_uso_en_1(uint32_t numero_tabla_1er_nivel_leer, uint32_t numero_de_entrada, uint32_t num_frame){
	int (*numero_tabla)[ENTRADAS_POR_TABLA] = list_get( tabla_de_paginas_de_primer_nivel, numero_tabla_1er_nivel_leer);
	pagina_t (*puntero_pagina)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_segundo_nivel, (*numero_tabla)[numero_de_entrada]);

	for( int i = 0; i < ENTRADAS_POR_TABLA ; i++ ){
		if( ((*puntero_pagina)[i].bit_presencia == 1) && ((*puntero_pagina)[i].numero_frame == num_frame)){
			(*puntero_pagina)[i].bit_uso = 1;
		}
	}
}


void bit_de_modificado_en_1(uint32_t numero_tabla_1er_nivel_leer, uint32_t numero_de_entrada, uint32_t num_frame){
	int (*numero_tabla)[ENTRADAS_POR_TABLA] = list_get( tabla_de_paginas_de_primer_nivel, numero_tabla_1er_nivel_leer);
	pagina_t (*puntero_pagina)[ENTRADAS_POR_TABLA] = list_get(tabla_de_paginas_de_segundo_nivel, (*numero_tabla)[numero_de_entrada]);

	for( int i = 0; i < ENTRADAS_POR_TABLA ; i++ ){
		if( ((*puntero_pagina)[i].bit_presencia == 1) && ((*puntero_pagina)[i].numero_frame == num_frame)){
			(*puntero_pagina)[i].bit_modificacion = 1;
		}
	}
}


