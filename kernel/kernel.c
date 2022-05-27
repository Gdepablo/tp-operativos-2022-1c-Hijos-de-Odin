#include "kernel.h"

// VARIABLES GLOBALES
sem_t mutex_cola_new;
t_queue* cola_new;

int main(void){
	sem_init(&mutex_cola_new, 0, 1);
	int socket_escucha;
	cola_new = queue_create();

	t_config* config = inicializarConfigs();
	char* IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
	char* PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	char* IP_CPU = config_get_string_value(config, "IP_CPU");
	char* PUERTO_CPU_DISPATCH = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
	char* PUERTO_CPU_INTERRUPT = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
	char* ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	int ESTIMACION_INICIAL = atoi( config_get_string_value(config, "ESTIMACION_INICIAL") );
	int ALFA = atoi( config_get_string_value(config, "ALFA") );
	int GRADO_MULTIPROGRAMACION = atoi(config_get_string_value(config, "GRADO_MULTIPROGRAMACION"));

	socket_escucha = iniciar_servidor("127.0.0.1", "8000");

	// ARRANCA EL WHILE
	while(1){
		uint32_t codOp;

		int* socket_cliente = malloc(sizeof(int));
		*socket_cliente = accept(socket_escucha, NULL, NULL);

		recv(*socket_cliente, &(codOp), sizeof(uint32_t), 0);

		// SE FIJA EL CODIGO DE OPERACION
		switch(codOp){
			case CREAR_PROCESO:{
				pthread_t thread;

				// crea un hilo y le envia el socket del cliente que entro, en este caso de un proceso
				pthread_create(&thread, NULL, (void*)atender_cliente, socket_cliente );
				pthread_join(thread, NULL);
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
int iniciar_servidor(char* ip, char* puerto)
{
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

	nuevo_config = config_create("/home/utnso/tp/tp/kernel/kernel.config");

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

	sem_wait(mutex_cola_new);
	queue_push(cola_new, proceso);
	sem_post(mutex_cola_new);

	return 0;
}

/*
 DESCRIPCION:

 PARAMETROS:
*/
void* planificador_largo_plazo_inicializador(int GRADO_MULTIPROGRAMACION){

	sem = GRADO_MULTIPROGRAMACION;

	//espera activa hasta que lleguen los primeros 4 procesos
	while(1){
		wait(sem)
		if( procesos_en_memoria <= GRADO_MULTIPROGRAMACION) {
			if( !queue_is_empty(cola_new) ){
				//pasar un proceso de new a ready
				wait(unmutex);
				procesos_en_memoria++;
				signal(unmutex);
			}
			else{
				signal(sem)
			}
		}
	}

	return "";
}

/*
 DESCRIPCION:

 PARAMETROS:
*/
void* planificador_largo_plazo_finalizador(){
	while(1){
		wait(finalizar);

		free(proceso_ejecutandose);

		wait(unmutex);
		procesos_en_memoria--;
		signal(unmutex);
	}

	return "";
}

/*
 DESCRIPCION:

 PARAMETROS:
*/
void* planificador_corto_plazo_fifo(){
	//logica fifo

	return "";
}

/*
 DESCRIPCION:

 PARAMETROS:
*/
void* planificador_corto_plazo_srt(){
	//logica srt

	return "";
}

/*
 DESCRIPCION:

 PARAMETROS:
*/
void* hilo_bloqueador(){


	return "";
}


void* planificador_mediano_plazo(){


	return "";
}













