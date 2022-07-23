#include "consola.h"



int main(int argc, char** argv){
	log_consola = log_create("./../logs/log_consola.log", "LOG CONSOLA", 0, LOG_LEVEL_INFO);
	if( argc != 3 ) {
		printf("cantidad de parametros incorrecta \n");
		log_error(log_consola,"cantidad de parametros incorrecta ");
		return EXIT_FAILURE;
	}

	printf("# CONSOLA # \n");
	log_info(log_consola,"# CONSOLA # ");
	printf("Datos leidos:");
	log_info(log_consola,"Datos leidos:");
	// PARSEAR INSTRUCCIONES
	t_config* config;
	int socket_kernel;
	char* ip = string_new();
	char* puerto = string_new();
	char c = '\0';
	char caracter[1];
	char* buffer = string_new();
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
	log_info(log_consola,"Ip asignada: %s", ip);
	printf("puerto: %s \n", puerto);
	log_info(log_consola,"Puerto asignada: %s", puerto);

	socket_kernel = crear_conexion(ip, puerto); // SE CONECTA AL KERNEL

	t_info_proceso infoProceso;

	infoProceso.tamanioDirecciones = (uint32_t)atoi(argv[2]);
	printf("Tamanio de Direcciones del Proceso %d \n", infoProceso.tamanioDirecciones);
	log_info(log_consola,"Tamanio de Direcciones del Proceso: %d", infoProceso.tamanioDirecciones);
	infoProceso.largoListaInstrucciones = (uint32_t)(size) + 1; //  este llevaba + 1
	printf("Largo de Lista de Instrucciones: %d \n", infoProceso.largoListaInstrucciones);
	log_info(log_consola,"Largo de Lista de Instrucciones: %d ", infoProceso.largoListaInstrucciones);
	infoProceso.listaInstrucciones = malloc(size + 1); // este llevaba + 1
	strcpy(infoProceso.listaInstrucciones, buffer);
	printf("LISTA DE INSTRUCCIONES:\n%s \nFIN LISTA \n", infoProceso.listaInstrucciones);
	log_info(log_consola,"Lista de Instrucciones:\n%s \nFIN LISTA", infoProceso.listaInstrucciones);

	t_buffer* bufferinho = malloc(sizeof(t_buffer));
	size = sizeof(uint32_t) * 2 + infoProceso.largoListaInstrucciones;
	bufferinho -> size = size;
	void* stream = malloc(size);
	int offset = 0;

	memcpy(stream+offset, &(infoProceso.tamanioDirecciones), sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(stream+offset, &(infoProceso.largoListaInstrucciones), sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(stream+offset, infoProceso.listaInstrucciones, infoProceso.largoListaInstrucciones);

	bufferinho->stream = stream;

	t_paquete* paquete = malloc( sizeof(t_paquete) );

	paquete->codOp = CREAR_PROCESO;
	paquete->buffer = bufferinho;

	void* a_enviar = malloc( sizeof(uint32_t) * 2 + paquete -> buffer -> size);
	offset = 0;

	memcpy(a_enviar+offset, &(paquete->codOp), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(a_enviar+offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(a_enviar+offset, paquete->buffer->stream, paquete->buffer->size);

	printf("Tamanio enviado del paquete: %i \n", sizeof(uint32_t) * 2 + paquete -> buffer -> size);
	log_info(log_consola,"Tamanio enviado del paquete: %i", sizeof(uint32_t) * 2 + paquete -> buffer -> size);

	send(socket_kernel, a_enviar, sizeof(uint32_t) * 2 + paquete -> buffer -> size, 0);

	printf("Datos enviados, esperando respuesta de Kernel... \n");
	log_info(log_consola,"Datos enviados, esperando respuesta de Kernel...");

	uint32_t respuesta;
	recv(socket_kernel, &respuesta, sizeof(uint32_t), MSG_WAITALL);

	if(respuesta == 10){
		printf("FINALIZACION OK ");
		log_info(log_consola,"FINALIZACION OK");
	}
	else
	{
		printf("ERROR DE COMUNICACION ENTRE KERNEL Y CONSOLA \n");
		log_error(log_consola,"Ha ocurrido un error en la comunicacion entre Kernel y Consola");
	}

	free(infoProceso.listaInstrucciones);
	close(socket_kernel);
	config_destroy(config);
	fclose(archivo);
	return EXIT_SUCCESS;
}


t_config* inicializarConfigs(void) {
			t_config* nuevo_config;

			nuevo_config = config_create("./../consola.config");

			return nuevo_config;
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
	int socket_kernel = socket( server_info -> ai_family,
								 server_info -> ai_socktype,
								 server_info -> ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo LISTO

	connect(socket_kernel, server_info -> ai_addr, server_info -> ai_addrlen );

	freeaddrinfo(server_info);

	return socket_kernel;
}









