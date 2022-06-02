#include "memoria.h"

int main(void){
	printf("MEMORIA \n");

	//CONFIG
	t_config* config;
	config = inicializarConfigs();

	char* PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	int TAMANIO_MEMORIA = atoi(config_get_string_value(config, "TAM_MEMORIA"));
	int TAMANIO_PAGINA = atoi(config_get_string_value(config, "TAM_PAGINA"));
	int ENTRADAS_POR_TABLA = atoi(config_get_string_value(config, "ENTRADAS_POR_TABLA"));
	int RETARDO_MEMORIA = atoi(config_get_string_value(config, "RETARDO_MEMORIA"));
	char* ALGORITMO_REEMPLAZO = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	int MARCOS_POR_PROCESO = atoi(config_get_string_value(config, "MARCOS_POR_PROCESO"));
	int RETARDO_SWAP = atoi(config_get_string_value(config, "RETARDO_SWAP"));
	char* PATH_SWAP = config_get_string_value(config, "PATH_SWAP");
	//FIN CONFIG

	//SOCKETS: MEMORIA ESCUCHA TANTO EL KERNEL COMO EL CPU POR EL MISMO SOCKET
	int socket_escucha = iniciar_servidor("127.0.0.1", PUERTO_ESCUCHA);
	int socket = accept(socket_escucha, 0, 0);
	//FIN SOCKETS

	return 0;
}

t_config* inicializarConfigs(void) {
	t_config* nuevo_config;

	nuevo_config = config_create("/home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/memoria/memoria.config");

	return nuevo_config; //Creo que esto funciona así igual pero no estoy seguro
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
