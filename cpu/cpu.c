#include "cpu.h"
#include <time.h>
#include <math.h>

//	DOCUMENTACION COSMICA REALIZADA POR BATATA
sem_t 	hiloCreado, // = 0
		ejecutar,	// = 0
		sem_interrupcion; // = 1

char** lista_de_instrucciones_actual; 	// lista de instrucciones del proceso en ejecucion
int socket_dispatch;					// socket por el que recibo los PCB
int socket_interrupt;					// socket por el cual el kernel envia interrupciones
uint32_t interrupcion = 0;				// si == 1 hay interrupcion, si == 0 no hay interrupcion
uint32_t entradas_tlb;					// recibido por config
char* reemplazo_tlb;					// modo de reemplazo TLB, puede ser FIFO o LRU

int main(void){
	lista_tlb = list_create(); 			// La lista representa la TLB
	sem_init(&ejecutar, 0, 0);

	//CONFIG
	t_config* config = inicializarConfigs();;					// abro el config, se encuentra en la carpeta padre y se llama cpu.config
	//config = inicializarConfigs();

	char* ip = config_get_string_value(config, "IP_MEMORIA"); // ip de la memoria
	char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA"); // puerto al cual el cpu se va a conectar con la memoria
	char* puerto_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"); // aca se comunica el kernel para mensajes de dispatch
	char* puerto_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT"); // aca se comunica con el kernel para enviar interrupciones
	reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB"); // FIFO o LRU
	retardo_noop = atoi( config_get_string_value(config, "RETARDO_NOOP") ); // Tiempo que toma el NOOP
	entradas_tlb = atoi( config_get_string_value(config, "ENTRADAS_TLB"));	// cantidad de entradas de la TLB


	//FIN CONFIG

	uint32_t handshake = 0;
	uint32_t todo_ok = 1;
	uint32_t todo_mal = 0;

	//SOCKETS
	int socket_escucha_dispatch = iniciar_servidor(ip, puerto_dispatch); // escucha de kernel
	socket_dispatch = accept(socket_escucha_dispatch, NULL, NULL); //bloqueante
	recv(socket_dispatch, &handshake, sizeof(uint32_t), MSG_WAITALL);
	if(handshake == 333){
		printf("Conexion con kernel realizada con exito... (DISPATCH) \n");
		send(socket_dispatch, &todo_ok, sizeof(uint32_t), 0);
	}
	else
	{
		printf("HANDSHAKE DEL KERNEL ERRONEO, TERMINANDO PROCESO (DISPATCH) \n");
		send(socket_dispatch, &todo_mal, sizeof(uint32_t), 0);
		return 1;
	}

	int socket_escucha_interrupt = iniciar_servidor(ip, puerto_interrupt); // escucha de kernel
	socket_interrupt = accept(socket_escucha_interrupt, NULL, NULL); //bloqueante
	recv(socket_interrupt, &handshake, sizeof(uint32_t), MSG_WAITALL);
	if(handshake == 111){
		printf("Conexion con kernel realizada con exito... (INTERRUPT) \n");
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
		printf("Conexion con memoria realizada con exito... \n");
	}
	else
	{
		printf("ERROR: HANDSHAKE INICIAL CON MEMORIA ERRONEO, TERMINANDO PROCESO \n");
		return 1;
	}
	//FIN SOCKETS, A ESTA ALTURA DEBERIAN ESTAR MEMORIA, CPU Y KERNEL CONECTADOS (si toodo esta correcto)


	//RECIBO INFORMACION DE LA MEMORIA PARA LA TRADUCCION DE MEMORIA
	recv(socket_memoria, &(info_traduccion), sizeof(info_traduccion_t), MSG_WAITALL);
	printf("Informacion para la traduccion de memoria recibida... \n");


	// CREAR POOL DE HILOS
	pthread_t executerThread, interruptsThread;
	sem_init(&hiloCreado, 0, 0);
	sem_init(&sem_interrupcion, 0, 0);

	pthread_create(&executerThread, NULL, executer, NULL);
	pthread_detach(executerThread);
	pthread_create(&interruptsThread, NULL, interrupt, NULL);
	pthread_detach(interruptsThread);


	for(int i = 0; i < 2; i++){
		sem_wait(&hiloCreado);
	}

	crear_TLB(); // crea tantas entradas como el config lo defina

	sem_destroy(&hiloCreado); 						//ya no lo necesito
	// FIN DE CREACION DE HILO

	printf("hilos creados, esperando PCB del kernel... \n"); // avisa que se iniciaron los hilos



	// COMIENZO A RECIBIR COSAS POR DISPATCH
	while(1){
		pcb_ejecutando = recibir_PCB(); // debe recibir la lista de instrucciones como char*
		lista_de_instrucciones_actual = string_array_new();
		lista_de_instrucciones_actual = string_split( pcb_ejecutando.lista_instrucciones, "\n" );
		printf("\nCOMIENZA LA EJECUCION DEL PROCESO %i \n", pcb_ejecutando.id_proceso);
		printf("LA ESTIMACION INICIAL ES %i \n", pcb_ejecutando.estimacion_rafagas);
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
	list_destroy(lista_tlb);
	config_destroy(config);
	return 0;
}

// HILOS
// ejecuta las instrucciones del pcb - DONE
void* executer(){
	sem_post(&hiloCreado); // simplemente avisa que se creo el hilo

	char* siguiente_instruccion;		// char para la siguiente instruccion
	char** instruccion_spliteada;		// agarra siguiente instruccion y la separa en 2 o 3 lineas, dependiendo de cual es
	uint32_t operando = 0;				// para la instruccion copy

	while(1){
		sem_wait(&ejecutar);

		//FETCH - DONE
		siguiente_instruccion = malloc( string_length(lista_de_instrucciones_actual[pcb_ejecutando.program_counter]) );
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
			// Cada funcion de cada instruccion tiene su documentacion hecha en instrucciones.c
			case NO_OP:				//CANTIDAD DE NO_OP, SEGUNDO PARAMETRO DE LA INSTRUCCION
				instr_no_op(atoi(instruccion_spliteada[1]));
				pcb_ejecutando.program_counter++;
				printf("NO OP \n");
				break;
			case IO: // DESPUES DE ESTA INSTRUCCION HAY QUE CORTAR LA EJECUCION
				instr_io(  atoi(instruccion_spliteada[1]) );
				pcb_ejecutando.program_counter++;
				printf("IO \n");
				break;
			case READ:				//DIR LOGICA
				instr_read(atoi(instruccion_spliteada[1]));
				pcb_ejecutando.program_counter++;
				printf("READ \n");
				break;
			case WRITE:				//DIR LOGICA					VALOR
				instr_write( atoi(instruccion_spliteada[1]), atoi(instruccion_spliteada[2]) );
				pcb_ejecutando.program_counter++;
				printf("WRITE \n");
				break;
			case COPY:				//DIR LOGICA DESTINO			VALOR
				instr_copy( atoi(instruccion_spliteada[1]), operando );
				pcb_ejecutando.program_counter++;
				printf("COPY \n");
				break;
			case EXIT: // DESPUES DE ESTA INSTRUCCION HAY QUE CORTAR LA EJECUCION DONE
				instr_exit();
				printf("EXIT \n");
				break;
			default:
				printf("LA INSTRUCCION %s NO ES VALIDA \n", instruccion_spliteada[0]);
		}



		// CHECK INTERRUPT - DONE

		if( hay_interrupcion() ){
			// LIMPIAR TLB, DEVOLVER PCB.
			limpiar_TLB();
			enviar_PCB();
//			free(pcb_ejecutando.lista_instrucciones);
			sem_wait(&sem_interrupcion);
			interrupcion = 0;
			sem_post(&sem_interrupcion);
		}
		else if (se_hizo_una_syscall_bloqueante()){
			// el pcb se envia en la instruccion IO o EXIT
			limpiar_TLB();
//			free(pcb_ejecutando.lista_instrucciones);
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
/*
 * 	Este hilo solamente atiende lo recibido de KERNEL para detener la ejecucion
 *
 */
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
			printf("EL HANDSHAKE RECIBIDO EN INTERRUPCION NO FUE 55 \n NO CAMBIO EL VALOR DE INTERRUPCION\n");
		}
	}

	return 0;
}

// FUNCIONES
// va a buscar el contenido de operando a memoria - DONE
uint32_t fetchOperand(uint32_t dir_logica){
	uint32_t frame_a_buscar = buscar_frame(dir_logica);
	uint32_t offset = dir_logica - ( floor(dir_logica/info_traduccion.tamanio_paginas) * info_traduccion.tamanio_paginas );
	uint32_t numero_entrada = floor(numero_pagina / info_traduccion.entradas_por_tabla);
	uint32_t contenido_del_frame = pedir_contenido(frame_a_buscar, offset, numero_entrada);

	return contenido_del_frame;
}

void crear_TLB(){ // DONE
	lista_tlb = list_create();
	for(int i=0;entradas_tlb > i;i++){
		t_tlb* tlb = malloc (sizeof(t_tlb));
		tlb->marco=-1;
		tlb->pagina=-1;
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
	tlb_puntero->pagina=-1;
	tlb_puntero->marco=-1;
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

bool marco_obsoleto(uint32_t numero_de_marco){
    if(marco_duplicado(numero_de_marco)){
        numero_de_marco_glob = numero_de_marco;
        t_tlb* puntero_tlb_mas_viejo = list_get_maximum( list_filter(lista_tlb, filtrar_por_marco), tlb_mas_viejo );

        if( (puntero_tlb_mas_viejo -> pagina) == numero_pagina ){
            return true;
        }

    }
    return false;
}

bool marco_duplicado(uint32_t numero_de_marco){
    t_tlb* puntero_a_tlb;
    int cantidad_de_marcos = 0;

    for(int i = 0; i < list_size(lista_tlb); i++){
        puntero_a_tlb = list_get(lista_tlb, i);

        if(puntero_a_tlb->marco == numero_de_marco){
            cantidad_de_marcos++;
        }
    }

    if(cantidad_de_marcos>=2){
        return true;
    }
    else
    {
        return false;
    }
}

bool filtrar_por_marco(void* tlb_puntero_void){
    t_tlb* tlb_puntero = tlb_puntero_void;

    return (tlb_puntero->marco == numero_de_marco_glob);
}

// puede ser una funcion void que directamente modifique la var global?
//t_pcb recibir_PCB(){
//	t_pcb_buffer* pcb_buffer = malloc(sizeof(t_pcb_buffer));
//	t_pcb nuevo_pcb;
//    int offset = 0;
//
//	recv(socket_dispatch, &(pcb_buffer->size), sizeof(uint32_t), MSG_WAITALL);
//    recv(socket_dispatch, &(pcb_buffer->size_instrucciones), sizeof(uint32_t), 0);
//
//    printf("tamanio: %i \n", pcb_buffer->size);
//    printf("tamanio instrucciones: %i \n", pcb_buffer->size_instrucciones);
//
//    pcb_buffer->stream = malloc(pcb_buffer->size);
//    recv(socket_dispatch, pcb_buffer->stream, sizeof(uint32_t) * 5 + pcb_buffer->size_instrucciones, 0);
//
//
//    memcpy(&(nuevo_pcb.id_proceso), pcb_buffer->stream+offset, sizeof(uint32_t));
//    offset += sizeof(uint32_t);
//
//    memcpy(&(nuevo_pcb.tamanio_direcciones), pcb_buffer->stream+offset, sizeof(uint32_t));
//    offset += sizeof(uint32_t);
//
//    nuevo_pcb.lista_instrucciones = malloc( pcb_buffer->size_instrucciones );
//    memcpy(nuevo_pcb.lista_instrucciones, (pcb_buffer->stream)+offset, pcb_buffer->size_instrucciones);
//    offset += pcb_buffer->size_instrucciones;
//
//    memcpy(&(nuevo_pcb.program_counter), pcb_buffer->stream+offset, sizeof(uint32_t));
//    offset += sizeof(uint32_t);
//
//    memcpy(&(nuevo_pcb.tabla_paginas), pcb_buffer->stream+offset, sizeof(uint32_t) );
//    offset += sizeof(uint32_t);
//
//    memcpy(&(nuevo_pcb.estimacion_rafagas), pcb_buffer->stream+offset, sizeof(uint32_t));
//
//    printf("PCB ARMADO: \n");
//    printf("id proceso: %i \n", nuevo_pcb.id_proceso);
//    printf("tamanio direcciones: %i \n", nuevo_pcb.tamanio_direcciones);
//    printf("lista instrucciones:  %s \n", nuevo_pcb.lista_instrucciones);
//    printf("strlen: %i \n", (int)strlen(nuevo_pcb.lista_instrucciones));
//    printf("program counter: %i \n", nuevo_pcb.program_counter);
//    printf("tabla de paginas: %i \n", nuevo_pcb.tabla_paginas);
//    printf("estimacion rafagas: %i \n", nuevo_pcb.estimacion_rafagas);
//
//    free(pcb_buffer->stream);
//	free(pcb_buffer);
//	return nuevo_pcb;
//}

t_pcb recibir_PCB(){
    t_pcb_buffer buffer;

    recv(socket_dispatch, &(buffer.size), sizeof(uint32_t), MSG_WAITALL);
//    printf("buffer->size: %i \n", buffer.size);
    recv(socket_dispatch, &(buffer.size_instrucciones), sizeof(uint32_t), MSG_WAITALL);
//    printf("buffer->size_instrucciones: %i \n", buffer.size_instrucciones);
    buffer.stream = malloc(buffer.size);
    recv(socket_dispatch, buffer.stream, buffer.size, MSG_WAITALL);

    t_pcb pcb;
    int offset=0;
    memcpy(&(pcb.id_proceso), buffer.stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
//    printf("pcb.id_proceso = %i \n", pcb.id_proceso);
    memcpy(&(pcb.tamanio_direcciones), buffer.stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
//    printf("pcb.tamanio_direcciones = %i \n", pcb.tamanio_direcciones);

    pcb.lista_instrucciones = malloc(buffer.size_instrucciones);
    memcpy(pcb.lista_instrucciones, buffer.stream+offset, buffer.size_instrucciones);
    offset+=buffer.size_instrucciones;
//    printf("pcb.lista_instrucciones = %s \n", pcb.lista_instrucciones);
//    printf("strlen: %i \n", (int)strlen(pcb.lista_instrucciones));

    memcpy(&(pcb.program_counter), buffer.stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
//    printf("pcb.program_counter = %i \n", pcb.program_counter);
    memcpy(&(pcb.tabla_paginas), buffer.stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
//    printf("pcb.tabla_paginas = %i \n", pcb.tabla_paginas);
    memcpy(&(pcb.estimacion_rafagas), buffer.stream+offset, sizeof(uint32_t));
//    printf("pcb.estimacion_rafagas = %i \n", pcb.estimacion_rafagas);

    return pcb;
}

