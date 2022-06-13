#include "memoria.h"
paginas_segundo_nivel_t tabla_segundo_nivel = list_create();
uint32_t tabla_primer_nivel; // Array de uint32_t;
char* PUERTO_ESCUCHA;
int TAMANIO_MEMORIA;
int TAMANIO_PAGINA;
int ENTRADAS_POR_TABLA;
int RETARDO_MEMORIA;
char* ALGORITMO_REEMPLAZO;
int MARCOS_POR_PROCESO;
int RETARDO_SWAP;
char* PATH_SWAP;

int main(void){
	printf("MEMORIA \n");

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

	sleep(5);

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

uint32_t clock(uint32_t numero_pagina, uint32_t num_frame_buscado) {
	if(list_size(tabla_segundo_nivel) <= ENTRADAS_POR_TABLA){
		uint32_t aux2;
		//Preguntar si la pag no esta
		if(list_find(tabla_segundo_nivel[numero_pagina].paginas,es_igual_a())) {

		}

	while(tabla_segundo_nivel[numero_pagina] -> paginas[aux2].numero_frame != num_frame_buscado
			 && aux2 <= MARCOS_POR_PROCESO) {
		if (tabla_segundo_nivel[numero_pagina] -> paginas[aux2].numero_frame == num_frame_buscado)
		{return num_frame_buscado;}
		else if (aux2 >= MARCOS_POR_PROCESO)
		{return -1;}
		aux2++;

	}
		pagina pagina =buscarPaginaEnSwap(num_frame_buscado);
		list_add(tabla_segundo_nivel[numero_pagina],pagina);}
		else{
			uint32_t aux;
			while(tabla_segundo_nivel[numero_pagina] -> paginas[aux].bit_uso != 0
					&& aux <= MARCOS_POR_PROCESO) {
				tabla_segundo_nivel[numero_pagina] -> paginas[aux].bit_uso = 0;
				aux++;}
		if(tabla_segundo_nivel[numero_pagina] -> paginas[aux].bit_uso == 0) {
			pagina y = buscar_pagina_en_swap();
				list_replace(tabla_segundo_nivel[numero_pagina].paginas,
				tabla_segundo_nivel[numero_pagina] -> paginas[aux],y);
			}

		}

}

pagina buscar_pagina_en_swap() {
	FILE* tuvieja = fopen("swap_proceso_x.swap","a+");
	char a[500];
	pagina y;
	while(!eof(tuvieja)) {
			fread(a,sizeof(tuvieja)+1,1,tuvieja);
			//Mecanism de lectura para el swap y que me devuelva la pag
		return elemento;
		}
}

bool es_igual_a(pagina paginaPedida) {
	return tabla_segundo_nivel.paginas == paginaPedida;

}
