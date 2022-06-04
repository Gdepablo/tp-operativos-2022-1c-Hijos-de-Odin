#include "cpu.h"


sem_t hiloCreado, ejecutar;
t_pcb pcb_ejecutando;
char** lista_de_instrucciones_actual;

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
	uint32_t retardo_noop = atoi( config_get_string_value(config, "RETARDO_NOOP") );
	uint32_t entradas_tlb = atoi( config_get_string_value(config, "ENTRADAS_TLB"));
	//FIN CONFIG

	uint32_t handshake = 0;

	//SOCKETS
	int socket_escucha_dispatch = iniciar_servidor(ip, puerto_dispatch); // escucha de kernel
	int socket_dispatch = accept(socket_escucha_dispatch, NULL, NULL); //bloqueante
	printf("se conecto el kernel al dispatch \n");
	recv(socket_dispatch, &handshake, sizeof(uint32_t), MSG_WAITALL);
	printf("handshake recibido: %i \n", handshake);
	if(handshake == 333){
		printf("HANDSHAKE DEL KERNEL CORRECTO (DISPATCH) \n");
	}
	else
	{
		printf("HANDSHAKE DEL KERNEL ERRONEO, TERMINANDO PROCESO (DISPATCH) \n");
		return 1;
	}

	// METER UN SLEEP(1) EN KERNEL SINO NO LLEGA A CONECTAR XD;

	int socket_escucha_interrupt = iniciar_servidor(ip, puerto_interrupt); // escucha de kernel
	int socket_interrupt = accept(socket_escucha_interrupt, NULL, NULL); //bloqueante
	printf("se conecto el kernel al interrupt \n");
	recv(socket_interrupt, &handshake, sizeof(uint32_t), MSG_WAITALL);
	if(handshake == 111){
		printf("HANDSHAKE DEL KERNEL CORRECTO (INTERRUPT) \n");
	}
	else
	{
		printf("HANDSHAKE DEL KERNEL ERRONEO, TERMINANDO PROCESO (INTERRUPT) \n");
		return 1;
	}

	uint32_t handshake_memoria = 222;
	uint32_t respuesta_memoria = 0;

	int socket_memoria = crear_conexion(ip, puerto_memoria); // se conecta a memoria, el que acepta es memoria
	send(socket_memoria, &handshake_memoria, sizeof(uint32_t), 0);
	recv(socket_memoria, &respuesta_memoria, sizeof(uint32_t), MSG_WAITALL);
	if(respuesta_memoria == 1)
	{
		printf("HANDSHAKE INICIAL CON MEMORIA CORRECTO \n");
	}
	else
	{
		printf("HANDSHAKE INICIAL CON MEMORIA ERRONEO, TERMINANDO PROCESO \n");
		return 1;
	}

	// BORRAR
	sleep(5);
	// BORRAR

	printf("Conexiones con memoria y Kernel realizadas correctamente. \n");

	//FIN SOCKETS, A ESTA ALTURA DEBERIAN ESTAR MEMORIA, CPU Y KERNEL CONECTADOS (si todo esta correcto)


	//RECIBO INFORMACION DE LA MEMORIA PARA LA TRADUCCION DE MEMORIA
	info_traduccion_t info_traduccion;
	recv(socket_memoria, &(info_traduccion), sizeof(info_traduccion_t), MSG_WAITALL);
	//YA RECIBI LA INFORMACION DE LA MEMORIA PARA LA TRADUCCION DE MEMORIA


	// CREAR POOL DE HILOS
	pthread_t executerThread, interruptsThread;
	sem_init(&hiloCreado, 0, 0);

	pthread_create(&executerThread, NULL, (void*)executer, NULL);
	pthread_create(&interruptsThread, NULL, (void*)interrupt, NULL);

	pthread_detach(executerThread);
	pthread_detach(interruptsThread);

	for(int i = 0; i < 2; i++){
		sem_wait(&hiloCreado);
	}

	sem_destroy(&hiloCreado);
	// FIN DE CREACION DE HILO

	printf("hilos creados, esperando PCB del kernel... \n");

	// COMIENZO A RECIBIR COSAS POR DISPATCH
	sem_init(&ejecutar, 0, 0);


	while(1){
		pcb_ejecutando = recibir_pcb(socket_dispatch); // debe recibir la lista de instrucciones como char*
		lista_de_instrucciones_actual = string_array_new();
		lista_de_instrucciones_actual = string_split( pcb_ejecutando.lista_instrucciones, "\n" );

	}
	// FIN DE RECIBIR COSAS POR DISPATCH

	//momento liberacion de memoria
	close(socket_escucha_interrupt);
	close(socket_interrupt);
	close(socket_escucha_dispatch);
	close(socket_dispatch);
	close(socket_memoria);
	config_destroy(config);
	return 0;
}

// ejecuta las instrucciones del pcb
void* executer(){
	sem_post(&hiloCreado);
	sem_wait(&ejecutar);

	while(1);

	return 0;
}

// recibe el aviso del kernel de que hay que desalojar
void* interrupt(){
	sem_post(&hiloCreado);

	while(1);

	return 0;
}

t_pcb recibir_pcb(int socket_dispatch){
	t_pcb_buffer* pcb_buffer = malloc(sizeof(t_pcb_buffer));
	t_pcb nuevo_pcb;
    int offset = 0;

	recv(socket_dispatch, &(pcb_buffer->size), sizeof(uint32_t), MSG_WAITALL);
    recv(socket_dispatch, &(pcb_buffer->size_instrucciones), sizeof(uint32_t), 0);

    printf("tamanio: %i \n", pcb_buffer->size);
    printf("tamanio instrucciones: %i \n", pcb_buffer->size_instrucciones);

    pcb_buffer->stream = malloc(pcb_buffer->size);
    recv(socket_dispatch, pcb_buffer->stream, sizeof(uint32_t) * 5 + pcb_buffer->size_instrucciones, 0);


    memcpy(&(nuevo_pcb.id_proceso), pcb_buffer->stream+offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(nuevo_pcb.tamanio_direcciones), pcb_buffer->stream+offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    nuevo_pcb.lista_instrucciones = malloc( pcb_buffer->size_instrucciones );
    memcpy(nuevo_pcb.lista_instrucciones, pcb_buffer->stream+offset, pcb_buffer->size_instrucciones);
    offset += pcb_buffer->size_instrucciones;
    memcpy(&(nuevo_pcb.program_counter), pcb_buffer->stream+offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(nuevo_pcb.tabla_paginas), pcb_buffer->stream+offset, sizeof(uint32_t) );
    offset += sizeof(uint32_t);
    memcpy(&(nuevo_pcb.estimacion_rafagas), pcb_buffer->stream+offset, sizeof(uint32_t));

    printf("PCB ARMADO: \n");
    printf("id proceso: %i \n", nuevo_pcb.id_proceso);
    printf("tamanio direcciones: %i \n", nuevo_pcb.tamanio_direcciones);
    printf("lista instrucciones:  %s \n", nuevo_pcb.lista_instrucciones);
    printf("program counter: %i \n", nuevo_pcb.program_counter);
    printf("tabla de paginas: %i \n", nuevo_pcb.tabla_paginas);
    printf("estimacion rafagas: %i \n", nuevo_pcb.estimacion_rafagas);


	free (pcb_buffer);
	return nuevo_pcb;
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
