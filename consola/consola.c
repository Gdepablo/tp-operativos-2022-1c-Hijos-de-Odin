#include "consola.h"



int main(int argc, char** argv){
	if( argc != 3 ) {
		printf("cantidad de parametros incorrecta \n");
		return EXIT_FAILURE;
	}

	// PARSEAR INSTRUCCIONES
	t_config* config;
	int socket_cliente;
	char* ip = string_new();
	char* puerto = string_new();
	char c = '\0';
	char caracter[1];
	char* buffer = string_new();
	uint32_t exit_code = 1;
	FILE* archivo = fopen(argv[1], "r");


	while ( (c = fgetc(archivo)) != EOF ){
		caracter[0] = c;
		string_append(&buffer, caracter);
	}

	int size = strlen(buffer);

	// SOCKETS
	// ASEGURARSE DE QUE EL PROCESO KERNEL ESTE ABIERTO

	config = inicializarConfigs();

	ip = config_get_string_value(config, "SERVER");
	puerto = config_get_string_value(config, "PUERTO");

	printf("ip: %s \n", ip);
	printf("puerto: %s \n", puerto);

	socket_cliente = crear_conexion(ip, puerto); // SE CONECTA AL KERNEL

	t_info_proceso infoProceso;

	infoProceso.tamanioDirecciones = (uint32_t)atoi(argv[2]);
	printf("TAMANIO: %d \n", infoProceso.tamanioDirecciones);
	infoProceso.largoListaInstrucciones = (uint32_t)(size+1);
	printf("LARGO LISTA: %d \n", infoProceso.largoListaInstrucciones);
	infoProceso.listaInstrucciones = malloc(size+1);
	strcpy(infoProceso.listaInstrucciones, buffer);
	printf("LISTA DE INSTRUCCIONES:\n%s \nFIN LISTA \n", infoProceso.listaInstrucciones);
	//t_info_proceso listo


	size = sizeof(uint32_t) * 2 + infoProceso.largoListaInstrucciones;
	void* stream = malloc(size);
	int offset = 0;

	memcpy(stream+offset, &(infoProceso.tamanioDirecciones), sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(stream+offset, &(infoProceso.largoListaInstrucciones), sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(stream+offset, infoProceso.listaInstrucciones, infoProceso.largoListaInstrucciones);
	//se pasan bien

	send(socket_cliente, stream, size, 0);

	free(infoProceso.listaInstrucciones);
	close(socket_cliente);
	config_destroy(config);
	free(archivo);
	return EXIT_SUCCESS;
}


t_config* inicializarConfigs(void) {
			t_config* nuevo_config;

			nuevo_config = config_create("/home/utnso/tp/tp/consola/consola.config");

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









