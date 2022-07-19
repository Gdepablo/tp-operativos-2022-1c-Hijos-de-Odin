#include "kernel.h"
#include "semaforos.h"
#include "planificadorDeCortoPlazo.h"
#include "planificadorDeMedianoPlazo.h"
#include "planificadorDeLargoPlazo.h"

// VARIABLES GLOBALES
char* PUERTO_ESCUCHA;





int main(void){
	printf("# KERNEL # \n");

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
	// TOdo, se recibe por config
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
	sem_init(&pcb_recibido, 0, 0);

	// INICIO DE HILOS, SE ESPERA A QUE TERMINEN ANTES DE CONTINUAR
	pthread_create(&lp_new_ready_fifo, NULL, new_a_ready, NULL);
	pthread_create(&mp_suspendedready_ready, NULL, suspended_ready_a_ready, NULL);
	pthread_create(&cp_ready_exec_fifo, NULL, ready_a_executing, NULL);
	pthread_create(&atender_consolas, NULL, recibir_procesos, NULL);
	pthread_create(&recibir_syscall_cpu, NULL, esperar_syscall, NULL);

	pthread_detach(lp_new_ready_fifo);
	pthread_detach(mp_suspendedready_ready);
	pthread_detach(cp_ready_exec_fifo);
	pthread_detach(atender_consolas);

	for(int i = 0; i < 5; i++){
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
	sem_post(&se_inicio_el_hilo);
	while(1) {
		t_syscall* una_syscall = recibirSyscall();
		switch(una_syscall->instruccion) {
		case IO:
			executing_a_blocked(una_syscall);
			break;
		case EXIT:
			printf("ENTRO EN EXIT \n");
			executing_a_exit(una_syscall);
			free(una_syscall->pcb.instrucciones);
			free(una_syscall);
			break;
		case DESALOJO:{
			// VER
			// recibir pcb
			// meter pcb en ready
			t_pcb* pcb_nuevo = malloc(sizeof(t_pcb));
			// ID PROCESO
			pcb_nuevo->id_proceso = una_syscall->pcb.id_proceso;
			// TAMANIO INSTRUCCIONES
			pcb_nuevo->tamanio_direcciones = una_syscall->pcb.tamanio_direcciones;
			// SIZE INSTRUCCIONES
			pcb_nuevo->size_instrucciones = una_syscall->pcb.size_instrucciones;
			// INSTRUCCIONES
			pcb_nuevo->instrucciones = malloc(strlen(una_syscall->pcb.instrucciones));
			strcpy(pcb_nuevo->instrucciones, una_syscall->pcb.instrucciones);
			// PROGRAM COUNTER
			pcb_nuevo->program_counter = una_syscall->pcb.program_counter;
			// TABLA PAGINAS
			pcb_nuevo->tabla_paginas = una_syscall->pcb.tabla_paginas;
			// ESTIMACION RAFAGA
			pcb_nuevo->estimacion_rafagas = una_syscall->pcb.estimacion_rafagas;
			// LISTO

			free(una_syscall->pcb.instrucciones);
			free(una_syscall);

			list_add(lista_ready, pcb_nuevo);
			// TODO CREAR PCB NUEVO, VER EN CPU EL CODIGO 2, VER STRUCTS t_pcb
			sem_post(&pcb_recibido);
			break;}
		default:
			printf("Error de código de instrucción \n");
		}
	}
}

t_syscall* recibirSyscall(){
    t_pcb_buffer* buffer = malloc(sizeof(t_pcb_buffer));


    recv(socket_cpu_dispatch, &(buffer->size), sizeof(uint32_t), MSG_WAITALL);
    printf("#### RECIBIENDO SYSCALL #### \n");
    printf("tamanio recibido: %i \n", buffer->size);
    recv(socket_cpu_dispatch, &(buffer->size_instrucciones), sizeof(uint32_t), 0);
    printf("size de instrucciones es: %i \n", buffer->size_instrucciones);

    buffer->stream = malloc(buffer->size);

    recv(socket_cpu_dispatch, buffer->stream, buffer->size, 0);

    t_syscall* syscall_recibida = malloc(sizeof(t_syscall));

    int offset = 0;
    // INSTRUCCION
    memcpy(&(syscall_recibida->instruccion), buffer->stream+offset, sizeof(uint32_t));
    printf("syscall instruccion es: %i \n", syscall_recibida->instruccion);
    offset+=sizeof(uint32_t);
    // TIEMPO DE BLOQUEO
    memcpy(&(syscall_recibida->tiempo_de_bloqueo), buffer->stream+offset, sizeof(uint32_t));
    printf("syscall tiempo de bloqueo es: %i \n", syscall_recibida->tiempo_de_bloqueo);
    offset+=sizeof(uint32_t);
    // PCB
    // ID PROCESO
    memcpy(&(syscall_recibida->pcb.id_proceso), buffer->stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    printf("syscall pcb id proceso es: %i \n", syscall_recibida->pcb.id_proceso);
    // TAMANIO DIRECCIONES
    memcpy(&(syscall_recibida->pcb.tamanio_direcciones), buffer->stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    printf("syscall pcb tamanio direcciones es: %i \n", syscall_recibida->pcb.tamanio_direcciones);
    // LISTA DE INSTRUCCIONES (PELIGRO)
    syscall_recibida->pcb.instrucciones = malloc(buffer->size_instrucciones + 1);
    memcpy(syscall_recibida->pcb.instrucciones, buffer->stream+offset, buffer->size_instrucciones);
    offset+=buffer->size_instrucciones;
    printf("la lista de instrucciones es: %s \n", syscall_recibida->pcb.instrucciones);
    // PROGRAM COUNTER
    memcpy(&(syscall_recibida->pcb.program_counter), buffer->stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    printf("el program counter es: %i \n", syscall_recibida->pcb.program_counter);
    // TABLA DE PAGINAS
    memcpy(&(syscall_recibida->pcb.tabla_paginas), buffer->stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    printf("la tabla de paginas es: %i \n", syscall_recibida->pcb.tabla_paginas);
    // ESTIMACION DE RAFAGAS
    memcpy(&(syscall_recibida->pcb.estimacion_rafagas), buffer->stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    printf("la estimacion de rafagas es: %i \n", syscall_recibida->pcb.estimacion_rafagas);

    // TODO liberar memoria, ver struct

	return syscall_recibida;
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
	t_log* log_fallas = log_create("./../logs/log de fallas.log", "log fulbo", true, LOG_LEVEL_INFO);

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

	int resultado_conexion = connect(socket_cliente, server_info -> ai_addr, server_info -> ai_addrlen );

	while(resultado_conexion == -1){
		log_info(log_fallas, "Hubo un fallo al conectarse a la direccion %s:%s, reintentando...", ip, puerto);
		sleep(1);
		resultado_conexion = connect(socket_cliente, server_info -> ai_addr, server_info -> ai_addrlen );
	}

	freeaddrinfo(server_info);
	log_destroy(log_fallas);

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




