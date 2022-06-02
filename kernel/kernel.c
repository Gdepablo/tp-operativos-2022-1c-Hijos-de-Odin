#include "kernel.h"

#include "planificadorDeCortoPlazo.h"
#include "planificadorDeMedianoPlazo.h"
#include "planificadorDeLargoPlazo.h"

// VARIABLES GLOBALES

// MUTEX
sem_t mx_cola_new; // = 1
sem_t mx_lista_ready; // = 1
sem_t mx_lista_blocked; // = 1
sem_t mx_lista_suspended_blocked; // = 1
// CONTADORES
sem_t procesos_en_ready; // = 0
sem_t grado_multiprogramacion; // = GRADO_MULTIPROGRAMACION DEL .CONFIG
// SINCRONIZADORES
sem_t proceso_finalizado; // = 0
sem_t fin_de_ejecucion; // = 0
sem_t se_inicio_el_hilo; // = 0

// COLAS Y LISTAS
t_queue* cola_new;
t_queue* cola_ready; // FIFO
t_list*  lista_ready; // SRT
t_list*  lista_blocked;
t_list*  lista_suspended_blocked;
t_list*  lista_suspended_ready;

t_pcb executing;

// HILOS
pthread_t lp_new_ready_fifo, lp_new_ready_srt, lp_exec_exit; // HILOS LARGO PLAZO
pthread_t mp_suspendedready_ready; //HILOS MEDIANO PLAZO
pthread_t cp_ready_exec_fifo, cp_ready_exec_srt, cp_sacar_exec; //HILOS CORTO PLAZO

int ESTIMACION_INICIAL;

int main(void){
	int socket_escucha;

	// INICIALIZACION DE SEMÁFOROS
	sem_init(&mx_cola_new, 0, 1);
	sem_init(&se_inicio_el_hilo, 0, 0);

	// INICIO DE HILOS, SE ESPERA A QUE TERMINEN ANTES DE CONTINUAR
	pthread_create(&lp_new_ready_fifo, NULL, (void*)new_a_ready_fifo, NULL);
	pthread_create(&lp_new_ready_srt, NULL, (void*)new_a_ready_srt, NULL);
	pthread_create(&lp_exec_exit, NULL, (void*)executing_a_exit, NULL);
	pthread_create(&mp_suspendedready_ready, NULL, (void*)suspended_ready_a_ready, NULL);
	pthread_create(&cp_ready_exec_fifo, NULL, (void*)ready_a_executing_fifo, NULL);
	pthread_create(&cp_ready_exec_srt, NULL, (void*)ready_a_executing_srt, NULL);
	pthread_create(&cp_sacar_exec, NULL, (void*)executing_a_ready_o_blocked, NULL);
	pthread_detach(lp_new_ready_fifo);
	pthread_detach(lp_new_ready_srt);
	pthread_detach(lp_exec_exit);
	pthread_detach(mp_suspendedready_ready);
	pthread_detach(cp_ready_exec_fifo);
	pthread_detach(cp_ready_exec_srt);
	pthread_detach(cp_sacar_exec);

	for(int i = 0; i < 7; i++){
		sem_wait(&se_inicio_el_hilo);
	}

	usleep(10000000);

	// VARIABLES DEL CONFIG
	t_config* config = inicializarConfigs();
	char* IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
	char* PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	char* IP_CPU = config_get_string_value(config, "IP_CPU");
	char* PUERTO_CPU_DISPATCH = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
	char* PUERTO_CPU_INTERRUPT = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
	char* ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	ESTIMACION_INICIAL = atoi( config_get_string_value(config, "ESTIMACION_INICIAL") );
	int ALFA = atoi( config_get_string_value(config, "ALFA") );
	int GRADO_MULTIPROGRAMACION = atoi(config_get_string_value(config, "GRADO_MULTIPROGRAMACION"));



	// COLAS
	cola_new     = queue_create();
	cola_ready   = queue_create();
	lista_ready   = list_create();
	lista_blocked = list_create();
	lista_suspended_blocked = list_create();
	lista_suspended_ready   = list_create();



	socket_escucha = iniciar_servidor("127.0.0.1", "8000");


	// ARRANCA EL WHILE
	while(1){
		uint32_t codOp;

		// liberar? no afecta al avisar el exit a la consola?
		int* socket_cliente = malloc(sizeof(int));
		*socket_cliente = accept(socket_escucha, NULL, NULL);

		recv(*socket_cliente, &(codOp), sizeof(uint32_t), 0);

		// SE FIJA EL CODIGO DE OPERACION
		switch(codOp){
			case CREAR_PROCESO:{
				// HACE FALTA UN HILO O PUEDO LLAMAR A LA FUNCION atender_cliente() Y LISTO?
				pthread_t thread;

				// crea un hilo y le envia el socket del cliente que entro, en este caso de un proceso
				pthread_create(&thread, NULL, (void*)atender_cliente, socket_cliente );
				pthread_join(thread, NULL);
				free(socket_cliente); // ya copie el numero con el que identifica al socket en atender_cliente()
				break;
			}
			default:
				printf("codigo erroneo \n");
				break;
		}
	}


	return 0;
}

/*
 DESCRIPCION:

 PARAMETROS:
*/
int iniciar_servidor(char* ip, char* puerto) {
	int socket_servidor;

	struct addrinfo hints;
	struct addrinfo *servinfo;
	int activado =1;


	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
	socket_servidor = 	socket( servinfo -> ai_family,
								servinfo -> ai_socktype,
								servinfo -> ai_protocol);

	setsockopt(socket_servidor,SOL_SOCKET,SO_REUSEADDR,&activado,sizeof(activado));

	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	return socket_servidor;
}

/*
 DESCRIPCION:

 PARAMETROS:
*/
t_config* inicializarConfigs(void) {
	t_config* nuevo_config;

	nuevo_config = config_create("/home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/kernel/kernel.config");

	return nuevo_config;
}

/*
 DESCRIPCION:

 PARAMETROS:
*/
t_info_proceso* desserializarProceso(t_buffer* buffer){
	t_info_proceso* procesoNuevo = malloc(sizeof(t_info_proceso));
	int offset = 0;
	void* stream = buffer -> stream;

	memcpy(&(procesoNuevo->tamanioDirecciones), stream, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(procesoNuevo->largoListaInstrucciones), stream+offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	procesoNuevo -> listaInstrucciones = malloc(procesoNuevo->largoListaInstrucciones);
	memcpy(procesoNuevo->listaInstrucciones, stream+offset, procesoNuevo->largoListaInstrucciones);


	return procesoNuevo;
}


/*
 DESCRIPCION: CREA LA PCB Y LA PUSHEA A cola_new

 PARAMETROS: socket del cliente
*/
void* atender_cliente(int* socket_cliente){
	t_buffer* buffer = malloc(sizeof(buffer));

	recv(*socket_cliente, &(buffer->size), sizeof(uint32_t), 0);
	printf("size recibido en el hilo: %d \n", buffer->size);
	buffer -> stream = malloc(buffer->size);
	recv(*socket_cliente, buffer->stream, buffer->size, 0);

	t_info_proceso* proceso = desserializarProceso(buffer);
	printf("mi lista de instrucciones es: \n%s \nfin de la lista \n", proceso -> listaInstrucciones);
	printf("\n");

	t_pcb* pcb = malloc( sizeof(uint32_t) * 5 + proceso->largoListaInstrucciones + sizeof(char**) );

	pcb->id_proceso = *socket_cliente;
	pcb->tamanio_direcciones = proceso->tamanioDirecciones;
	pcb->lista_instrucciones = string_split( proceso->listaInstrucciones, "\n");
	pcb->program_counter = 0;
	pcb->tabla_paginas = 0;
	pcb->estimacion_rafagas = ESTIMACION_INICIAL;

	free(buffer->stream);
	free(buffer);
	free(proceso->listaInstrucciones);
	free(proceso);

	ingreso_a_new(pcb); // planificador largo plazo

	return 0;
}




