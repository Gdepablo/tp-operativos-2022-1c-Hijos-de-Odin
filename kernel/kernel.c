#include "kernel.h"

#include "planificadorDeCortoPlazo.h"
#include "planificadorDeMedianoPlazo.h"
#include "planificadorDeLargoPlazo.h"

// VARIABLES GLOBALES
t_pcb PCB_EJECUCION;
struct timeval HORA_INICIO_EJECUCION, HORA_FIN_EJECUCION;

// MUTEX
sem_t mx_cola_new; // = 1
sem_t mx_lista_ready; // = 1
sem_t mx_cola_blocked; // = 1
sem_t mx_cola_suspended_blocked; // = 1
sem_t mx_cola_suspended_ready; // = 1

// CONTADORES
sem_t procesos_en_new; // = 0
sem_t procesos_en_ready; // = 0
sem_t grado_multiprogramacion; // = GRADO_MULTIPROGRAMACION DEL .CONFIG
sem_t io_terminada;
sem_t procesos_en_suspended_ready;
sem_t proceso_nuevo_en_ready;
// SINCRONIZADORES
sem_t proceso_finalizado; // = 0
sem_t fin_de_ejecucion; // = 0
sem_t se_inicio_el_hilo; // = 0
sem_t proceso_en_io; // = 0

// COLAS Y LISTAS
t_queue* cola_new;
t_queue* cola_ready; // FIFO
t_list*  lista_ready; // SRT
t_queue*  cola_blocked;
t_queue*  cola_suspended_blocked;
t_queue*  cola_suspended_ready;

// SOCKETS

int socket_consola_proceso;
int socket_cpu_syscall;
int socket_cpu_pcb;
int socket_cpu_interrupcion;

// HILOS
pthread_t lp_new_ready_fifo, lp_new_ready_srt, lp_exec_exit; // HILOS LARGO PLAZO
pthread_t mp_suspendedready_ready; //HILOS MEDIANO PLAZO
pthread_t cp_ready_exec_fifo, cp_ready_exec_srt, cp_sacar_exec; //HILOS CORTO PLAZO

// VARIABLES PARA LOS SOCKETS
char* IP_CONSOLA;
char* IP_KERNEL;
char* IP_CPU;
char* IP_MEMORIA;

char* PUERTO_CONSOLA_PROCESO;
char* PUERTO_MEMORIA;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* PUERTO_CPU_SYSCALL;

char* ALGORITMO_PLANIFICACION;
int ESTIMACION_INICIAL;
float ALFA;
int GRADO_MULTIPROGRAMACION;
int TIEMPO_MAXIMO_BLOQUEADO;

int main(void){
	PCB_EJECUCION.id_proceso = -1;

	// VARIABLES DEL CONFIG
	t_config* config = inicializarConfigs();
	IP_CONSOLA = config_get_string_value(config, "IP_CONSOLA");
	IP_KERNEL  = config_get_string_value(config, "IP_KERNEL");
	IP_CPU     = config_get_string_value(config, "IP_CPU");
	IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");

	PUERTO_MEMORIA          = config_get_string_value(config, "PUERTO_MEMORIA");
	PUERTO_CPU_SYSCALL          = config_get_string_value(config, "PUERTO_ESCUCHA");
	PUERTO_CPU_DISPATCH     = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
	PUERTO_CPU_INTERRUPT    = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");

	ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	ESTIMACION_INICIAL      = atoi( config_get_string_value(config, "ESTIMACION_INICIAL") );
	ALFA                    = atof( config_get_string_value(config, "ALFA") );
	GRADO_MULTIPROGRAMACION = atoi(config_get_string_value(config, "GRADO_MULTIPROGRAMACION"));
	TIEMPO_MAXIMO_BLOQUEADO = atoi(config_get_string_value(config, "TIEMPO_MAXIMO_BLOQUEADO"));

	// COLAS Y LISTAS
	cola_new                = queue_create();
	cola_ready              = queue_create();
	lista_ready             = list_create();
	cola_blocked            = list_create();
	cola_suspended_blocked  = queue_create();
	cola_suspended_ready    = queue_create();


	// SOCKETS
	uint32_t handshake;
	uint32_t respuesta;

	int socket_consola_proceso  = iniciar_servidor(IP_KERNEL, PUERTO_CONSOLA_PROCESO); // por aca se comunican las consolas XD

	int socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
	handshake = 555;
	send(socket_memoria, &handshake, sizeof(uint32_t), 0);

	recv(socket_memoria, &respuesta, sizeof(uint32_t), MSG_WAITALL);
		if(respuesta == 1) {
			printf("Se conectó a memoria \n");
		} else {
			printf("Error en la conexión con la memoria \n");
			return 1;
		}

	int socket_cpu_pcb = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH); // se conecta a cpu
	handshake = 333;
	send(socket_cpu_pcb, &handshake, sizeof(uint32_t), 0);
	respuesta = 0;
	recv(socket_cpu_pcb, &respuesta, sizeof(uint32_t), MSG_WAITALL);
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

	// INICIALIZACION DE SEMÁFOROS
	sem_init(&mx_cola_new, 0, 1);
	sem_init(&se_inicio_el_hilo, 0, 0);
	sem_init(&procesos_en_ready, 0, 0);

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

	for(int i = 0; i < 6; i++){
		sem_wait(&se_inicio_el_hilo);
	}
	printf("se iniciaron todos los hilos \n");

	return 0;
}

// RECEPCIÓN DE PROCESOS DESDE CONSOLA Y CARGA EN NEW

void* recibir_procesos() {
	int id = 0;
	while(1) {
		t_buffer* buffer = malloc(sizeof(buffer));

		recv(*socket_consola_proceso, &(buffer->size), sizeof(uint32_t), 0);
		buffer -> stream = malloc(buffer->size);
		recv(*socket_consola_proceso, buffer->stream, buffer->size, 0);

		t_info_proceso* proceso = deserializar_proceso(buffer);
		printf("mi lista de instrucciones es: \n%s \nfin de la lista \n", proceso->instrucciones);
		printf("\n");

		t_pcb* pcb = malloc( sizeof(uint32_t) * 5 + proceso->largo_instrucciones + sizeof(char*) );

		pcb->id_proceso = *socket_consola_proceso;
		pcb->tamanio_direcciones = proceso->tamanio_direcciones;
		pcb->instrucciones = proceso->instrucciones;
		pcb->program_counter = 0;
		pcb->tabla_paginas = 0;
		pcb->estimacion_rafagas = ESTIMACION_INICIAL;

		free(buffer->stream);
		free(buffer);
		free(proceso->instrucciones);
		free(proceso);

		ingreso_a_new(pcb); // planificador largo plazo
		id++;
	}
	return "";
}

// ESPERA DE SYSCALLS PROVENIENTES DEL CPU (IO Y EXIT)

void* esperar_syscall() {
	while(1) {
		t_syscall una_syscall = recibirSyscall(); // espera por una sycall, la deserializa y la devuelve
		switch(una_syscall.instruccion) {
		case 0: // IO
			executing_a_blocked(una_syscall.pcb, una_syscall.tiempo_de_bloqueo);
			break;
		case 1: // EXIT
			executing_a_exit(una_syscall.pcb);
			break;
		default:
			printf("Error de código de instrucción \n");
		}
	}
}

t_info_proceso* deserializar_proceso(t_buffer* buffer) {
	t_info_proceso* procesoNuevo = malloc(sizeof(t_info_proceso));
	int offset = 0;
	void* stream = buffer -> stream;

	memcpy(&(procesoNuevo->tamanio_direcciones), stream, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(procesoNuevo->instrucciones), stream+offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
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

	nuevo_config = config_create("/home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/kernel/kernel.config");

	return nuevo_config;
}

int es_FIFO() {
	return strcmp(ALGORITMO_PLANIFICACION, "FIFO") != 0;
}




