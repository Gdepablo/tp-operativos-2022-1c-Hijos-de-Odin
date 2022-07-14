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

uint32_t RETARDO_MEMORIA;
char* ALGORITMO_REEMPLAZO;
uint32_t MARCOS_POR_PROCESO;
uint32_t RETARDO_SWAP;
char* PATH_SWAP;


t_list* tabla_de_paginas_global;
// Guarda todas las páginas de forma contigua


t_list* tabla_de_paginas_de_primer_nivel;
/* Hay una lista de uint32_t por cada proceso
 * Éstas guardan una lista de uint32_t que representa el número de tabla de segundo nivel a la cual dirigirse
 */

t_list* tabla_de_paginas_de_segundo_nivel;
/* Guarda las tablas de segundo nivel de todos los procesos
 * Éstas guardan una lista de páginas
 */

//uint32_t vector_de_disponibilidad[/*cantidad de frames*/];



int main(void){
	printf("MEMORIA \n");



	//mx_tabla_de_paginas_global = ;

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

	int socket_escucha = iniciar_servidor("127.0.0.1", PUERTO_ESCUCHA);

	// HANDSHAKE CON KERNEL
	int socket_kernel = accept(socket_escucha, 0, 0);
	recv(socket_kernel, &handshake, sizeof(uint32_t), MSG_WAITALL);
	printf("handshake = %i \n", handshake);
	if(handshake == 555){
		printf("HANDSHAKE RECIBIDO CORRECTAMENTE (KERNEL)\n");
		send(socket_kernel, &todo_ok, sizeof(uint32_t), 0);
	}
	else
	{
		printf("HANDSHAKE RECIBIDO DE KERNEL ERRONEO, TERMINANDO PROCESO (MEMORIA) \n");
		send(socket_kernel, &todo_mal, sizeof(uint32_t), 0);
		return 1;
	}

	// HANDSHAKE CON CPU / TODAVIA NO SE LE ENVIA INFO PARA TRADUCCION LOGICA A REAL
	int socket_cpu = accept(socket_escucha, 0, 0);
	recv(socket_cpu, &handshake, sizeof(uint32_t), MSG_WAITALL);
	if(handshake == 222){
		printf("HANDSHAKE RECIBIDO CORRECTAMENTE (CPU)\n");
		send(socket_cpu, &todo_ok, sizeof(uint32_t), 0);
	}
	else
	{
		printf("HANDSHAKE RECIBIDO ERRONEO DE CPU, TERMINANDO PROCESO (MEMORIA) \n");
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
	//FIN HANDSHAKE

	// TODO INICIAR HILO KERNEL


	// tod0 lo importante de memoria
	memoria_real = malloc(TAMANIO_MEMORIA);


	// FIN TAREAS ADMINISTRATIVAS, EMPIEZA EL LABURO
	uint32_t codigo_recibido;

	// atender peticiones del CPU y KERNEL (usan el mismo socket)
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


	close(socket_escucha);
	close(socket_cpu);
	close(socket_kernel);
	return 0;
}

t_config* inicializarConfigs(void) {
	t_config* nuevo_config;

	nuevo_config = config_create("/home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/memoria/memoria.config");

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





