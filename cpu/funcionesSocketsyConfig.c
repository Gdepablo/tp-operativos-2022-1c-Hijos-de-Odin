#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <commons/config.h>
#include "funcionesSocketsyConfig.h"


t_config* inicializarConfigs(void) {
	t_config* nuevo_config;

	nuevo_config = config_create("/home/utnso/Documentos/borrar/configs/fulbo.config");


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
