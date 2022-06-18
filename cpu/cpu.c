#include "cpu.h"


sem_t hiloCreado, ejecutar, sem_interrupcion;

char** lista_de_instrucciones_actual;
//uint32_t retardo_noop;
int socket_dispatch;
int socket_interrupt;
int interrupcion = 0;


int main(void){
	printf("HOLA SOY EL CPU \n");
	tlbs = list_create();
	//CONFIG
	t_config* config;
	config = inicializarConfigs();

	char* ip = config_get_string_value(config, "IP_MEMORIA"); // ip de la memoria
	char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA"); // puerto al cual el cpu se va a conectar con la memoria
	char* puerto_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"); // aca se comunica el kernel para mensajes de dispatch
	char* puerto_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT"); // aca se comunica con el kernel para enviar interrupciones
	char* reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB");
	retardo_noop = atoi( config_get_string_value(config, "RETARDO_NOOP") );
	uint32_t entradas_tlb = atoi( config_get_string_value(config, "ENTRADAS_TLB"));

	//FIN CONFIG

	uint32_t handshake = 0;
	uint32_t todo_ok = 1;
	uint32_t todo_mal = 0;

	//SOCKETS
	int socket_escucha_dispatch = iniciar_servidor(ip, puerto_dispatch); // escucha de kernel
	socket_dispatch = accept(socket_escucha_dispatch, NULL, NULL); //bloqueante
	printf("se conecto el kernel al dispatch \n");
	recv(socket_dispatch, &handshake, sizeof(uint32_t), MSG_WAITALL);
	printf("handshake recibido: %i \n", handshake);
	if(handshake == 333){
		printf("HANDSHAKE DEL KERNEL CORRECTO (DISPATCH) \n");
		send(socket_dispatch, &todo_ok, sizeof(uint32_t), 0);
	}
	else
	{
		printf("HANDSHAKE DEL KERNEL ERRONEO, TERMINANDO PROCESO (DISPATCH) \n");
		send(socket_dispatch, &todo_mal, sizeof(uint32_t), 0);
		return 1;
	}

	// METER UN SLEEP(1) EN KERNEL SINO NO LLEGA A CONECTAR XD;

	int socket_escucha_interrupt = iniciar_servidor(ip, puerto_interrupt); // escucha de kernel
	socket_interrupt = accept(socket_escucha_interrupt, NULL, NULL); //bloqueante
	printf("se conecto el kernel al interrupt \n");
	recv(socket_interrupt, &handshake, sizeof(uint32_t), MSG_WAITALL);
	if(handshake == 111){
		printf("HANDSHAKE DEL KERNEL CORRECTO (INTERRUPT) \n");
		send(socket_interrupt, &todo_ok, sizeof(uint32_t), 0);
	}
	else
	{
		printf("HANDSHAKE DEL KERNEL ERRONEO, TERMINANDO PROCESO (INTERRUPT) \n");
		send(socket_interrupt, &todo_mal, sizeof(uint32_t), 0);
		return 1;
	}

	uint32_t handshake_memoria = 222;
	uint32_t respuesta_memoria = 0;

	socket_memoria = crear_conexion(ip, puerto_memoria); // se conecta a memoria, el que acepta es memoria
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

	printf("Conexiones con memoria y Kernel realizadas correctamente. \n");

	//FIN SOCKETS, A ESTA ALTURA DEBERIAN ESTAR MEMORIA, CPU Y KERNEL CONECTADOS (si toodo esta correcto)


	//RECIBO INFORMACION DE LA MEMORIA PARA LA TRADUCCION DE MEMORIA

	recv(socket_memoria, &(info_traduccion), sizeof(info_traduccion_t), MSG_WAITALL);
	//YA RECIBI LA INFORMACION DE LA MEMORIA PARA LA TRADUCCION DE MEMORIA


	// CREAR POOL DE HILOS
	pthread_t executerThread, interruptsThread;
	sem_init(&hiloCreado, 0, 0);
	sem_init(&sem_interrupcion, 0, 0);

	pthread_create(&executerThread, NULL, executer, NULL);
	pthread_create(&interruptsThread, NULL, interrupt, NULL);

	pthread_detach(executerThread);
	pthread_detach(interruptsThread);

	crear_TLB();

	for(int i = 0; i < 3; i++){
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
		sem_post(&ejecutar);
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

// ejecuta las instrucciones del pcb - DONE
void* executer(){
	sem_post(&hiloCreado);

	char* siguiente_instruccion;
	char** instruccion_spliteada;
	uint32_t operando = 0;

	while(1){
		sem_wait(&ejecutar);
		//FETCH - DONE
		siguiente_instruccion = malloc(string_length(lista_de_instrucciones_actual[pcb_ejecutando.program_counter]));
		siguiente_instruccion = string_duplicate(lista_de_instrucciones_actual[pcb_ejecutando.program_counter]);
		instruccion_spliteada = string_array_new();
		instruccion_spliteada = string_split(siguiente_instruccion, " ");

		//DECODE - ETAPA OPCIONAL - DONE
		if(!strcmp(instruccion_spliteada[0], "COPY")){
			operando = fetchOperand(atoi(instruccion_spliteada[2])); //instruccion_spliteada[2] = dir_logica_origen
		}


		//EXECUTE - DONE
		int numOperacion = seleccionarOperacion(instruccion_spliteada[0]); // retorna 0,1,2,3,4,5

		switch(numOperacion){
			case NO_OP:				//CANTIDAD DE NO_OP
				instr_no_op(atoi(instruccion_spliteada[1]));
				pcb_ejecutando.program_counter++;
				break;
			case IO: // DESPUES DE ESTA INSTRUCCION HAY QUE CORTAR LA EJECUCION todo
				instr_io(  atoi(instruccion_spliteada[1]) );
				pcb_ejecutando.program_counter++;
				break;
			case READ:				//DIR LOGICA
				instr_read(atoi(instruccion_spliteada[1]));
				pcb_ejecutando.program_counter++;
				break;
			case WRITE:				//DIR LOGICA					VALOR
				instr_write( atoi(instruccion_spliteada[1]), atoi(instruccion_spliteada[2]) );
				pcb_ejecutando.program_counter++;
				break;
			case COPY:				//DIR LOGICA DESTINO			VALOR
				instr_copy( atoi(instruccion_spliteada[1]), operando );
				pcb_ejecutando.program_counter++;
				break;
			case EXIT: // DESPUES DE ESTA INSTRUCCION HAY QUE CORTAR LA EJECUCION todo
				instr_exit();
				break;
			default:
				printf("LA INSTRUCCION %s NO ES VALIDA \n", instruccion_spliteada[0]);
		}



		// CHECK INTERRUPT - DONE

		if( hay_interrupcion() ){
			// LIMPIAR TLB, DEVOLVER PCB.
			limpiar_TLB();
			enviar_PCB();
			sem_wait(&sem_interrupcion);
			interrupcion = 0;
			sem_post(&sem_interrupcion);
		}
		else if (se_hizo_una_syscall_bloqueante()){
			limpiar_TLB();
		} else {
			sem_post(&ejecutar);
		}

		free(siguiente_instruccion);
		free(instruccion_spliteada);
	}

	return 0;
}

// recibe el aviso del kernel de que hay que desalojar - DONE
void* interrupt(){
	sem_post(&hiloCreado);

	uint32_t valor_recibido = -1;
	while(1){
		recv(socket_interrupt, &valor_recibido, sizeof(uint32_t), MSG_WAITALL);

		if(valor_recibido == 55){
			sem_wait(&sem_interrupcion);
			interrupcion = 1;
			sem_post(&sem_interrupcion);
		}
	}

	return 0;
}

// va a buscar el contenido de operando a memoria - DONE
uint32_t fetchOperand(uint32_t dir_logica){
	uint32_t frame_a_buscar = buscar_frame(dir_logica);
	uint32_t contenido_del_frame = pedir_contenido_frame(frame_a_buscar);

	return contenido_del_frame;
}

// debe fijarse si la var global 'interrupcion' == 1 // TODO
int hay_interrupcion(){
	printf("HAY INTERRUPCION \n");

	return 0;
}

// debe fijarse si se hizo una syscall bloqueante // TODO
int se_hizo_una_syscall_bloqueante(){
	printf("SE HIZO UNA SYSCALL BLOQUEANTE \n");

	return 0;
}

void crear_TLB(){ // TODO

}

void limpiar_TLB(){ // TODO
	printf("limpiar TLB \n");
}

t_tlb elegir_pagina_a_reemplazar_TLB(){ // TODO
	t_tlb pagina_a_retornar;

	return pagina_a_retornar;
}

void guardar_en_TLB(uint32_t numero_de_pagina, uint32_t numero_de_frame){

}

void enviar_PCB(){ // TODO
	printf("enviar PCB \n");
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
