#include "kernel.h"
#include "semaforos.h"
#include "planificadorDeCortoPlazo.h"
#include "planificadorDeMedianoPlazo.h"
#include "planificadorDeLargoPlazo.h"

// VARIABLES GLOBALES
char* PUERTO_ESCUCHA;





int main(void){
	PCB_EJECUCION.id_proceso = -1;

	// VARIABLES DEL CONFIG
	t_config* config = inicializarConfigs();
	IP_CPU     = config_get_string_value(config, "IP_CPU");
	IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");

	PUERTO_MEMORIA          = config_get_string_value(config, "PUERTO_MEMORIA");
	PUERTO_CPU_DISPATCH     = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
	PUERTO_CPU_INTERRUPT    = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
	PUERTO_ESCUCHA 			= config_get_string_value(config, "PUERTO_ESCUCHA");

	ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	ESTIMACION_INICIAL      = atoi( config_get_string_value(config, "ESTIMACION_INICIAL") );
	ALFA                    = atof( config_get_string_value(config, "ALFA") );
	GRADO_MULTIPROGRAMACION = atoi(config_get_string_value(config, "GRADO_MULTIPROGRAMACION"));
	TIEMPO_MAXIMO_BLOQUEADO = atoi(config_get_string_value(config, "TIEMPO_MAXIMO_BLOQUEADO"));

	// COLAS Y LISTAS
	cola_new                = queue_create();
	cola_ready              = queue_create();
	lista_ready             = list_create();
	cola_blocked            = queue_create();
	cola_suspended_blocked  = queue_create();
	cola_suspended_ready    = queue_create();

	printf("%s \n", PUERTO_MEMORIA);
	printf("%s \n", PUERTO_CPU_DISPATCH);
	printf("%s \n", PUERTO_CPU_INTERRUPT);

	// CAMBIAR ESTA IP
	char* IP_ESCUCHA = "127.0.0.1";

	// SOCKETS
	uint32_t handshake;
	uint32_t respuesta;

	socket_consola_proceso  = iniciar_servidor(IP_ESCUCHA, PUERTO_ESCUCHA); // por aca se comunican las consolas XD

	socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
	handshake = 555;
	send(socket_memoria, &handshake, sizeof(uint32_t), 0);

	recv(socket_memoria, &respuesta, sizeof(uint32_t), MSG_WAITALL);
	if(respuesta == 1) {
		printf("Se conectó a memoria \n");
	} else {
		printf("Error en la conexión con la memoria \n");
		return 1;
	}

	socket_cpu_dispatch = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH); // se conecta a cpu
	handshake = 333;
	send(socket_cpu_dispatch, &handshake, sizeof(uint32_t), 0);
	respuesta = 0;
	recv(socket_cpu_dispatch, &respuesta, sizeof(uint32_t), MSG_WAITALL);
	if(respuesta == 1) {
		printf("Se conectó al dispatch \n");
	} else {
		printf("Error en la conexión con el dispatch \n");
		return 1;
	}

	int socket_cpu_interrupcion = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT); // se conecta a cpu
	handshake = 111;
		send(socket_cpu_interrupcion, &handshake, sizeof(uint32_t), 0);
		respuesta = 0;
		recv(socket_cpu_interrupcion, &respuesta, sizeof(uint32_t), MSG_WAITALL);
		if(respuesta == 1) {
			printf("Se conectó a interrupciones \n");
		} else {
			printf("Error en la conexión con las interrupciones \n");
			return 1;
		}

	printf("FULBO \n");

	// INICIALIZACION DE SEMÁFOROS
	sem_init(&mx_cola_new, 0, 1);
	sem_init(&mx_lista_ready , 0, 1);
	sem_init(&fin_de_ejecucion, 0, 1);
	sem_init(&se_inicio_el_hilo, 0, 0);
	sem_init(&procesos_en_ready, 0, 0);
	sem_init(&grado_multiprogramacion, 0, GRADO_MULTIPROGRAMACION);

	// INICIO DE HILOS, SE ESPERA A QUE TERMINEN ANTES DE CONTINUAR
	pthread_create(&lp_new_ready_fifo, NULL, new_a_ready, NULL);
	pthread_create(&mp_suspendedready_ready, NULL, suspended_ready_a_ready, NULL);
	pthread_create(&cp_ready_exec_fifo, NULL, ready_a_executing, NULL);
	pthread_create(&atender_consolas, NULL, recibir_procesos, NULL);

	pthread_detach(lp_new_ready_fifo);
	pthread_detach(mp_suspendedready_ready);
	pthread_detach(cp_ready_exec_fifo);
	pthread_detach(atender_consolas);

	for(int i = 0; i < 4; i++){
		sem_wait(&se_inicio_el_hilo);
	}
	printf("se iniciaron todos los hilos \n");

	while(1); // ver esto xd

	return 0;
}

// RECEPCIÓN DE PROCESOS DESDE CONSOLA Y CARGA EN NEW

void* recibir_procesos() {
	sem_post(&se_inicio_el_hilo);
	int id = 0;
	uint32_t codop;
	while(1) {
		int* socket_nuevo = malloc(sizeof(int));
		*socket_nuevo = accept(socket_consola_proceso, NULL, NULL);

		t_buffer* buffer = malloc(sizeof(buffer));

		recv(*socket_nuevo, &codop, sizeof(uint32_t), MSG_WAITALL);
		printf("codop %i \n", codop);
		recv(*socket_nuevo, &(buffer->size), sizeof(uint32_t), MSG_WAITALL);
		buffer -> stream = malloc(buffer->size);
		recv(*socket_nuevo, buffer->stream, buffer->size, MSG_WAITALL);

		t_info_proceso* proceso = deserializar_proceso(buffer);

		printf("mi lista de instrucciones es: \n%s \nfin de la lista \n", proceso->instrucciones);
		printf("\n");

		t_pcb* pcb = malloc( sizeof(uint32_t) * 5 + proceso->largo_instrucciones + sizeof(char*) );

		pcb->id_proceso = *socket_nuevo;
		pcb->tamanio_direcciones = proceso->tamanio_direcciones;
		pcb->instrucciones = proceso->instrucciones;
		pcb->program_counter = 0;
		pcb->tabla_paginas = 0;
		pcb->estimacion_rafagas = ESTIMACION_INICIAL;

		printf("%i \n", pcb->id_proceso);
		printf("%i \n", pcb->tamanio_direcciones);
		printf("%s \n", pcb->instrucciones);
		printf("%i \n", pcb->program_counter);
		printf("%i \n", pcb->tabla_paginas);
		printf("%i \n", pcb->estimacion_rafagas);

		free(buffer->stream);
		free(buffer);
		free(proceso);

		ingreso_a_new(pcb); // planificador largo plazo
		id++;
	}
	return "";
}

// ESPERA DE SYSCALLS PROVENIENTES DEL CPU (IO Y EXIT)

void* esperar_syscall() {
	while(1) {
		t_syscall* una_syscall = recibirSyscall(); 	// espera por una sycall, la deserializa y la devuelve
													// TODO por batata
		switch(una_syscall->instruccion) {
		case 0: // IO
			executing_a_blocked(una_syscall);
			break;
		case 1: // EXIT
			executing_a_exit();
			break;
		default:
			printf("Error de código de instrucción \n");
		}
	}
}

t_syscall* recibirSyscall(){ //ToDO POR BATATA
	t_syscall* CAMBIAR_NOMBRE;

	return CAMBIAR_NOMBRE;
}

t_info_proceso* deserializar_proceso(t_buffer* buffer) {
	t_info_proceso* procesoNuevo = malloc(sizeof(t_info_proceso));
	int offset = 0;
	void* stream = buffer -> stream;

	memcpy(&(procesoNuevo->tamanio_direcciones), stream, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	printf("procesonuevo->tamanio_direcciones = %i \n", procesoNuevo->tamanio_direcciones);

	memcpy(&(procesoNuevo->largo_instrucciones), stream+offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	printf("procesonuevo->largo_instrucciones = %i \n", procesoNuevo->largo_instrucciones);

	procesoNuevo->instrucciones = malloc(procesoNuevo->largo_instrucciones);
	memcpy(procesoNuevo->instrucciones, stream+offset, procesoNuevo->largo_instrucciones);

	return procesoNuevo;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket( server_info -> ai_family,
								 server_info -> ai_socktype,
								 server_info -> ai_protocol);

	connect(socket_cliente, server_info -> ai_addr, server_info -> ai_addrlen );

	freeaddrinfo(server_info);

	return socket_cliente;
}

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

t_config* inicializarConfigs(void) {
	t_config* nuevo_config;

	nuevo_config = config_create("./../kernel.config");

	return nuevo_config;
}

int es_FIFO(){
	return !strcmp(ALGORITMO_PLANIFICACION, "FIFO");
}




