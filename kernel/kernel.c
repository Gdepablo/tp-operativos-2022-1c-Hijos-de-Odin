#include "kernel.h"


int main(void){
	int socket_escucha;
	int socket_cliente;

	socket_escucha = iniciar_servidor("127.0.0.1", "8000");
	socket_cliente = accept(socket_escucha, NULL, NULL);
	printf("SE CONECTO XD \n");

	t_info_proceso* infoProceso = malloc(sizeof(t_info_proceso));
	recv(socket_cliente, &(infoProceso->tamanioDirecciones), sizeof(uint32_t), 0);
	printf("tamanio direc: %d \n", infoProceso->tamanioDirecciones);
	recv(socket_cliente, &(infoProceso->largoListaInstrucciones), sizeof(uint32_t), 0);
	printf("largoLista: %d \n", infoProceso->largoListaInstrucciones);
	infoProceso->listaInstrucciones = malloc(infoProceso->largoListaInstrucciones);
	recv(socket_cliente, infoProceso->listaInstrucciones, infoProceso->largoListaInstrucciones, 0);
	printf("lista instr: \n%s \nfin lista. \n", infoProceso->listaInstrucciones);

	return 0;
}


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

	// Creamos el socket de escucha del servidor LISTO
	socket_servidor = 	socket( servinfo -> ai_family,
								servinfo -> ai_socktype,
								servinfo -> ai_protocol);

	setsockopt(socket_servidor,SOL_SOCKET,SO_REUSEADDR,&activado,sizeof(activado));

	// Asociamos el socket a un puerto LISTO
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes LISTO
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	return socket_servidor;
}
