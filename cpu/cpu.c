#include "cpu.h"


/*
 * TODO
 * COPIAR EL CONFIG - LISTO
 * HACER LOS SOCKETS: - LISTOS
 * 	- ESCUCHAR la conexion del kernel DISPATCH (ip 127.0.0.1, puerto 15001)
 * 	- ESCUCHAR la conexion del kernel INTERRUPT (ip 127.0.0.1, puerto 15005)
 * 	- CONECTARSE a memoria (ip 127.0.0.1, puerto 15002)
 *
 */



int main(void){
	printf("HOLA SOY EL CPU \n");

	//CONFIG
	t_config* config;
	config = inicializarConfigs();

	char* ip = config_get_string_value(config, "IP_MEMORIA"); // ip de la memoria
	char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA"); // puerto al cual el cpu se va a conectar con la memoria
	char* puerto_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"); // aca se comunica el kernel para mensajes de dispatch
	char* puerto_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT"); // aca se comunica con el kernel para enviar interrupciones
	char* reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB");
	int retardo_noop = atoi(config_get_string_value(config, "RETARDO_NOOP"));
	int entradas_tlb = atoi(config_get_string_value(config, "ENTRADAS_TLB"));
	//FIN CONFIG

	//SOCKETS
	int socket_escucha_dispatch = iniciar_servidor(ip, puerto_dispatch); // escucha de kernel
	int socket_dispatch = accept(socket_escucha_dispatch, NULL, NULL); //bloqueante
	printf("se conecto el kernel al dispatch \n");
	int socket_escucha_interrupt = iniciar_servidor(ip, puerto_interrupt); // escucha de kernel
	int socket_interrupt = accept(socket_escucha_dispatch, NULL, NULL); //bloqueante
	printf("se conecto el kernel al interrupt \n");
	int socket_memoria = crear_conexion(ip, puerto_memoria); // se conecta a memoria, el que acepta es memoria
	printf("conexion con memoria establecida \n");
	//FIN SOCKETS, A ESTA ALTURA DEBERIAN ESTAR MEMORIA, CPU Y KERNEL CONECTADOS



	config_destroy(config);
	return 0;
}

t_config* inicializarConfigs(void) {
	t_config* nuevo_config;

	nuevo_config = config_create("/home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/cpu/cpu.config");

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
/*
void pushearInstruccion(char* instruccion, char*** listaInstrucciones){
	char* aux = malloc(1024);
	aux = string_new();
	aux = string_duplicate(instruccion);

	char* identificador = string_new();
	identificador = strtok(aux, " ");

	if( !strcmp("NO_OP", identificador) )
	{
		int parametro = atoi((strtok(NULL, " ")));
		for( int i = 0; i < parametro; i++) {
			string_array_push(listaInstrucciones, "NO_OP");
		}
	}
	else
	{
		string_array_push(listaInstrucciones, instruccion);
	}

	free(aux);

	return;
} */

/*
	while( c != EOF ) {
		while ( ((c = fgetc(archivo)) != '\n') ){ //se carga una linea en el buffer
			if(c != EOF)
			{
				char caracter[1];
				caracter[0] = c;
				string_append(&buffer, caracter);
			}
			else
			{
				break;
			}
		}

		pushearInstruccion(buffer, &instrucciones);
		printf("buffer: %s \n", buffer);
		printf("%p \n", buffer);
		free(buffer);
		buffer = string_new();
	} */
