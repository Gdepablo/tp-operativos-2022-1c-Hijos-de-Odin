#include "main_memoria.h"
//#include "algoritmos.h"
//#include "swap.h"
//#include "com_kernel.h"
//#include "com_cpu.h"
#include "hilo_kernel.h"
#include <pthread.h>


// SEMÁFOROS

//mutex
sem_t mx_tabla_de_paginas_global;

char* PUERTO_ESCUCHA;
uint32_t TAMANIO_MEMORIA;






t_list* tabla_de_paginas_global;
// Guarda todas las páginas de forma contigua

int main(void){
	printf("# MEMORIA #\n");

	log_ejecucion_main = log_create("./../logs/log_memoria_main.log", "MEMORIA - MAIN", 0, LOG_LEVEL_INFO);

	sem_init(&escritura_log, 0, 1);
	sem_init(&hilo_iniciado, 0, 0);
	sem_init(&operacion_swap, 0, 1);
	sem_init(&operacion_en_memoria, 0, 1);
	sem_init(&operacion_en_bitmap, 0, 1);
	sem_init(&operacion_en_lista_de_tablas, 0, 1);

	//CONFIG
	t_config* config;
	config = inicializarConfigs();

	PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	TAMANIO_MEMORIA = atoi(config_get_string_value(config, "TAM_MEMORIA"));
	TAMANIO_PAGINA = atoi(config_get_string_value(config, "TAM_PAGINA"));
	ENTRADAS_POR_TABLA = atoi(config_get_string_value(config, "ENTRADAS_POR_TABLA"));
	RETARDO_MEMORIA = atoi(config_get_string_value(config, "RETARDO_MEMORIA"));
	ALGORITMO_REEMPLAZO = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	MARCOS_POR_PROCESO = atoi(config_get_string_value(config, "MARCOS_POR_PROCESO"));
	RETARDO_SWAP = atoi(config_get_string_value(config, "RETARDO_SWAP"));
	PATH_SWAP = config_get_string_value(config, "PATH_SWAP");
	//FIN CONFIG


	//SOCKETS: MEMORIA ESCUCHA TANTO EL KERNEL COMO EL CPU POR EL MISMO SOCKET
	uint32_t handshake = 0;
	uint32_t todo_ok = 1;
	uint32_t todo_mal = 0;

	int socket_escucha = iniciar_servidor("fulbo", PUERTO_ESCUCHA);

	// HANDSHAKE CON KERNEL
	int socket_kernel = accept(socket_escucha, 0, 0);
	recv(socket_kernel, &handshake, sizeof(uint32_t), MSG_WAITALL);
	if(handshake == 555){
		printf("HANDSHAKE RECIBIDO CORRECTAMENTE (KERNEL)\n");
		log_info(log_ejecucion_main, "se conecto con el kernel");
		send(socket_kernel, &todo_ok, sizeof(uint32_t), 0);
	}
	else
	{
		printf("HANDSHAKE RECIBIDO DE KERNEL ERRONEO, TERMINANDO PROCESO (MEMORIA) \n");
		send(socket_kernel, &todo_mal, sizeof(uint32_t), 0);
		log_error(log_ejecucion_main, "error al conectarse con el kernel");
		return 1;
	}

	// HANDSHAKE CON CPU / TODAVIA NO SE LE ENVIA INFO PARA TRADUCCION LOGICA A REAL
	int socket_cpu = accept(socket_escucha, 0, 0);
	recv(socket_cpu, &handshake, sizeof(uint32_t), MSG_WAITALL);
	if(handshake == 222){
		printf("HANDSHAKE RECIBIDO CORRECTAMENTE (CPU)\n");
		log_info(log_ejecucion_main, "se conecto con el CPU");
		send(socket_cpu, &todo_ok, sizeof(uint32_t), 0);
	}
	else
	{
		printf("HANDSHAKE RECIBIDO ERRONEO DE CPU, TERMINANDO PROCESO (MEMORIA) \n");
		log_error(log_ejecucion_main, "error al conectarse con el kernel");
		send(socket_cpu, &todo_mal, sizeof(uint32_t), 0);
		return 1;
	}

	printf("Conexiones con Kernel y CPU establecidas correctamente. \n");
	//FIN SOCKETS

	//COMIENZO HANDSHAKE CON CPU (para que cpu traduzca las direcciones)
	info_traduccion_t info_traduccion;
	info_traduccion.entradas_por_tabla = ENTRADAS_POR_TABLA;
	info_traduccion.tamanio_paginas = TAMANIO_PAGINA;

	send(socket_cpu, &info_traduccion, sizeof(info_traduccion_t), 0);
	log_info(log_ejecucion_main, "datos de traduccion enviados al CPU");
	//FIN HANDSHAKE

	// tod0 lo importante de memoria
	memoria_real = malloc(TAMANIO_MEMORIA);
	log_info(log_ejecucion_main, "memoria inicializada con un tamaño de %i", TAMANIO_MEMORIA);
	log_info(log_ejecucion_main, "la cantidad de frames en esta ejecucion es de %i", TAMANIO_MEMORIA / TAMANIO_PAGINA);
	tabla_de_paginas_de_primer_nivel = list_create();
	tabla_de_paginas_de_segundo_nivel = list_create();
	punteros_clock = list_create();
	bitmap_memoria = list_create();
	inicializar_bitmap();


	// inicia el hilo de kernel
	void* puntero_al_socket = &socket_kernel;
	pthread_create(&hiloKernel, NULL, hilo_kernel, puntero_al_socket);
	pthread_detach(hiloKernel);

	sem_wait(&hilo_iniciado);

	puntero_al_socket = &socket_cpu;
	pthread_create(&hiloCPU, NULL, hilo_cpu, puntero_al_socket);
	pthread_join(hiloCPU, NULL);
	// FIN TAREAS ADMINISTRATIVAS, EMPIEZA EL LABURO
	// atender peticiones del CPU y KERNEL con hilo_cpu.c e hilo_kernel.c




	close(socket_escucha);
	close(socket_cpu);
	close(socket_kernel);
	return 0;
}

void inicializar_bitmap(){
	for(int i = 0 ; i < TAMANIO_MEMORIA / TAMANIO_PAGINA ; i++){
		uint32_t* bit = malloc(sizeof(uint32_t));
		*bit = 0;
		list_add(bitmap_memoria, bit);
	}
	log_info(log_ejecucion_main, "bitmap creado con un tamaño de %i", TAMANIO_MEMORIA / TAMANIO_PAGINA);
}

t_config* inicializarConfigs(void) {
	t_config* nuevo_config;

	nuevo_config = config_create("./../memoria.config");

	return nuevo_config; //Creo que esto funciona así igual pero no estoy seguro
}

int crear_conexion(char *ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket. LISTO
	int socket_cliente = socket( server_info -> ai_family,
								 server_info -> ai_socktype,
								 server_info -> ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo LISTO

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

	getaddrinfo(NULL, puerto, &hints, &servinfo);

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





