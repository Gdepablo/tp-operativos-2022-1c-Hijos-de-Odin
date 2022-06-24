#include "cpu.h"
#include <time.h>


sem_t hiloCreado, ejecutar, sem_interrupcion;

char** lista_de_instrucciones_actual;
int socket_dispatch;
int socket_interrupt;
uint32_t interrupcion = 0;
uint32_t entradas_tlb;
char* reemplazo_tlb;


int main(void){
	printf("HOLA SOY EL CPU \n");
	lista_tlb = list_create();
	//CONFIG
	t_config* config;
	config = inicializarConfigs();

	char* ip = config_get_string_value(config, "IP_MEMORIA"); // ip de la memoria
	char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA"); // puerto al cual el cpu se va a conectar con la memoria
	char* puerto_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"); // aca se comunica el kernel para mensajes de dispatch
	char* puerto_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT"); // aca se comunica con el kernel para enviar interrupciones
	reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB");
	retardo_noop = atoi( config_get_string_value(config, "RETARDO_NOOP") );
	entradas_tlb = atoi( config_get_string_value(config, "ENTRADAS_TLB"));

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
	free(ip);
	free(puerto_memoria);
	free(puerto_dispatch);
	free(puerto_interrupt);
	close(socket_escucha_interrupt);
	close(socket_interrupt);
	close(socket_escucha_dispatch);
	close(socket_dispatch);
	close(socket_memoria);
	config_destroy(config);
	list_destroy(lista_tlb);
	return 0;
}

// HILOS
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
			case IO: // DESPUES DE ESTA INSTRUCCION HAY QUE CORTAR LA EJECUCION DONE
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
			case EXIT: // DESPUES DE ESTA INSTRUCCION HAY QUE CORTAR LA EJECUCION DONE
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
			free(pcb_ejecutando.lista_instrucciones);
			sem_wait(&sem_interrupcion);
			interrupcion = 0;
			sem_post(&sem_interrupcion);
		}
		else if (se_hizo_una_syscall_bloqueante()){
			// el pcb se envia en la instruccion IO o EXIT
			limpiar_TLB();
			free(pcb_ejecutando.lista_instrucciones);
			syscall_bloqueante=0;
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
		else
		{
			printf("EL HANDSHAKE RECIBIDO EN INTERRUPCION NO FUE 55 \n NO CAMBIO EL VALOR DE INTERRUPCION XD \n");
		}
	}

	return 0;
}

// FUNCIONES
// va a buscar el contenido de operando a memoria - DONE
uint32_t fetchOperand(uint32_t dir_logica){
	uint32_t frame_a_buscar = buscar_frame(dir_logica);
	uint32_t contenido_del_frame = pedir_contenido_frame(frame_a_buscar);

	return contenido_del_frame;
}

void crear_TLB(){ // DONE
	lista_tlb = list_create();
	for(int i=0;entradas_tlb > i;i++){
		t_tlb* tlb = malloc (sizeof(t_tlb));
		tlb->marco=0;
		tlb->pagina=0;
		tlb->primera_referencia = clock();
		tlb->ultima_referencia=clock();
		list_add(lista_tlb, tlb);
	}
}

void limpiar_TLB(){ // DONE
	list_iterate(lista_tlb,cambiar_puntero_tlb);
}

t_tlb* elegir_pagina_a_reemplazar_TLB(){ // DONE
	t_tlb* pagina_a_retornar;

	//Algoritmo LRU
	if(!strcmp(reemplazo_tlb,"LRU")){
		pagina_a_retornar=list_get_maximum(lista_tlb,tlb_menos_referenciado);
	}

	//Como el unico algoritmo alternativo es fifo no se aclaran nuevas condiciones para esta entrada.
	//Algoritmo FIFO
	else{
		pagina_a_retornar= list_get_maximum(lista_tlb,tlb_mas_viejo);
	}
	return pagina_a_retornar;
}

void guardar_en_TLB(uint32_t numero_de_pagina, uint32_t numero_de_frame){ // DONE
	t_tlb* pagina_a_reemplazar = elegir_pagina_a_reemplazar_TLB();
	pagina_a_reemplazar->pagina = numero_de_pagina;
	pagina_a_reemplazar->marco = numero_de_frame;
	pagina_a_reemplazar->primera_referencia=clock();
	pagina_a_reemplazar->ultima_referencia=clock();
}

void cambiar_puntero_tlb(void* tlb){
	t_tlb* tlb_puntero= tlb;
	tlb_puntero->pagina=0;
	tlb_puntero->marco=0;
	tlb_puntero->primera_referencia=clock();
	tlb_puntero->ultima_referencia=clock();
}

void* tlb_mas_viejo(void* tlbA,void* tlbB){
	t_tlb* tlb1 = tlbA;
	t_tlb* tlb2 = tlbB;
	clock_t tiempo_de_cambio=(float)clock();
	float tiempo_tlb1= tiempo_de_cambio - (float)tlb1->primera_referencia;
	float tiempo_tlb2= tiempo_de_cambio - (float)tlb2->primera_referencia;
	if(tiempo_tlb1 > tiempo_tlb2 ){
		return tlb1;
	}
	else{
		return tlb2;
	}
}

void* tlb_menos_referenciado(void* tlbA, void* tlbB){
	t_tlb* tlb1 = tlbA;
	t_tlb* tlb2 = tlbB;
	clock_t tiempo_de_cambio=(float)clock();
	float tiempo_tlb1= tiempo_de_cambio - (float)tlb1->ultima_referencia;
	float tiempo_tlb2= tiempo_de_cambio - (float)tlb2->ultima_referencia;
	if(tiempo_tlb1 > tiempo_tlb2 ){
		return tlb1;
	}
	else{
		return tlb2;
	}

}

// debe fijarse si la var global 'interrupcion' == 1 // DONE
bool hay_interrupcion(){
	return interrupcion==1;
}

// debe fijarse si se hizo una syscall bloqueante // DONE
bool se_hizo_una_syscall_bloqueante(){

	return syscall_bloqueante ==1;

}

void enviar_PCB(){
    t_pcb* pcb_a_enviar = &pcb_ejecutando;
    t_pcb_buffer* buffer = malloc(sizeof(t_pcb_buffer));
    buffer->size = sizeof(uint32_t) * 5 + strlen(pcb_a_enviar->lista_instrucciones);
    buffer->size_instrucciones = strlen(pcb_a_enviar->lista_instrucciones);
    buffer->stream = malloc(buffer->size);

    int offset = 0;
    // COPY DEL ID DE PROCESO
    memcpy(buffer->stream+offset, &(pcb_a_enviar->id_proceso), sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    // COPY DE TAMANIO DE DIRECCIONES
    memcpy(buffer->stream+offset, &(pcb_a_enviar->tamanio_direcciones), sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    // COPY LISTA DE INSTRUCCIONES
    memcpy(buffer->stream+offset, pcb_a_enviar->lista_instrucciones, buffer->size_instrucciones);
    offset+=buffer->size_instrucciones;
    // COPY PROGRAM COUNTER
    memcpy(buffer->stream+offset, &(pcb_a_enviar->program_counter), sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    // COPY TABLA DE PAGINAS
    memcpy(buffer->stream+offset, &(pcb_a_enviar->tabla_paginas), sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    // COPY ESTIMACION DE RAFAGA
    memcpy(buffer->stream+offset, &(pcb_a_enviar->estimacion_rafagas), sizeof(uint32_t));

    offset=0;
    int tamanio_a_enviar = sizeof(uint32_t) * 2 + buffer->size;
    void* a_enviar = malloc( tamanio_a_enviar );

    memcpy(a_enviar+offset, &(buffer->size), sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    memcpy(a_enviar+offset, &(buffer->size_instrucciones), sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    memcpy(a_enviar+offset, buffer->stream, tamanio_a_enviar);

    send(socket_dispatch, a_enviar, tamanio_a_enviar, 0);

    free(buffer->stream);
    free(buffer);
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
