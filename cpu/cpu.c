#include "cpu.h"
#include <time.h>
#include <math.h>

sem_t 	hiloCreado, // = 0
		ejecutar,	// = 0
		sem_interrupcion; // = 1

char** lista_de_instrucciones_actual; 	// lista de instrucciones del proceso en ejecucion
int socket_dispatch;					// socket por el que recibo los PCB
int socket_interrupt;					// socket por el cual el kernel envia interrupciones
uint32_t interrupcion = 0;				// si == 1 hay interrupcion, si == 0 no hay interrupcion
uint32_t entradas_tlb;					// recibido por config
char* reemplazo_tlb;					// modo de reemplazo TLB, puede ser FIFO o LRU
sem_t nada_ejecutando;

int main(void){
	printf("# CPU # \n");

	log_ejecucion_cpu = log_create("./../logs/log_cpu.log", "CPU", 0, LOG_LEVEL_INFO);


	lista_tlb = list_create(); 			// La lista representa la TLB
	sem_init(&ejecutar, 0, 0);
	sem_init(&nada_ejecutando, 0, 1);

	t_config* config = inicializarConfigs();

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
	int socket_escucha_dispatch = iniciar_servidor(ip, puerto_dispatch);
	socket_dispatch = accept(socket_escucha_dispatch, NULL, NULL);
	recv(socket_dispatch, &handshake, sizeof(uint32_t), MSG_WAITALL);
	if(handshake == 333){
		printf("Conexion con kernel realizada con exito... (DISPATCH) \n");
		log_info(log_ejecucion_cpu, "se realizo la conexion al kernel (dispatch)");
		send(socket_dispatch, &todo_ok, sizeof(uint32_t), 0);
	}
	else
	{
		printf("HANDSHAKE DEL KERNEL ERRONEO, TERMINANDO PROCESO (DISPATCH) \n");
		log_error(log_ejecucion_cpu, "no se realizo la conexion al kernel (dispatch)");
		send(socket_dispatch, &todo_mal, sizeof(uint32_t), 0);
		return 1;
	}

	int socket_escucha_interrupt = iniciar_servidor(ip, puerto_interrupt);
	socket_interrupt = accept(socket_escucha_interrupt, NULL, NULL);
	recv(socket_interrupt, &handshake, sizeof(uint32_t), MSG_WAITALL);
	if(handshake == 111){
		printf("Conexion con kernel realizada con exito... (INTERRUPT) \n");
		log_info(log_ejecucion_cpu, "se realizo la conexion al kernel (interrupt)");
		send(socket_interrupt, &todo_ok, sizeof(uint32_t), 0);
	}
	else
	{
		printf("HANDSHAKE DEL KERNEL ERRONEO, TERMINANDO PROCESO (INTERRUPT) \n");
		log_error(log_ejecucion_cpu, "no se realizo la conexion al kernel (interrupt)");
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
		log_info(log_ejecucion_cpu, "se realizo la conexion a memoria");
	}
	else
	{
		printf("ERROR: HANDSHAKE INICIAL CON MEMORIA ERRONEO, TERMINANDO PROCESO \n");
		log_error(log_ejecucion_cpu, "no se realizo la conexion a memoria (interrupt)");
		return 1;
	}
	//FIN SOCKETS, A ESTA ALTURA DEBERIAN ESTAR MEMORIA, CPU Y KERNEL CONECTADOS (si toodo esta correcto)


	//RECIBO INFORMACION DE LA MEMORIA PARA LA TRADUCCION DE MEMORIA
	recv(socket_memoria, &(info_traduccion), sizeof(info_traduccion_t), MSG_WAITALL);
	log_info(log_ejecucion_cpu, "se recibio la informacion para traduccion de direccion logica");
	printf("Informacion para la traduccion de direcciones logicas recibida... \n");


	// CREAR POOL DE HILOS
	pthread_t executerThread, interruptsThread;
	sem_init(&hiloCreado, 0, 0);
	sem_init(&sem_interrupcion, 0, 1);


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
		sem_wait(&nada_ejecutando);
		pcb_ejecutando = recibir_PCB(); // debe recibir la lista de instrucciones como char*
		lista_de_instrucciones_actual = string_array_new();
		lista_de_instrucciones_actual = string_split( pcb_ejecutando.lista_instrucciones, "\n" );
		printf("\nCOMIENZA LA EJECUCION DEL PROCESO %i \n", pcb_ejecutando.id_proceso);
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
// ejecuta las instrucciones del pcb
void* executer(){
	sem_post(&hiloCreado); // simplemente avisa que se creo el hilo
	log_info(log_ejecucion_cpu, "hilo executer iniciado \n");

	char* siguiente_instruccion;		// char para la siguiente instruccion
	char** instruccion_spliteada;		// agarra siguiente instruccion y la separa en 2 o 3 lineas, dependiendo de cual es
	uint32_t operando = 0;				// para la instruccion copy

	while(1){
		sem_wait(&ejecutar);
		log_info(log_ejecucion_cpu, "### COMIENZO CICLO DE INSTRUCCION ###");

		//FETCH - DONE
		log_info(log_ejecucion_cpu, "FETCH NEXT INSTRUCTION");
		siguiente_instruccion = malloc( string_length(lista_de_instrucciones_actual[pcb_ejecutando.program_counter]) );
		siguiente_instruccion = string_duplicate(lista_de_instrucciones_actual[pcb_ejecutando.program_counter]);
		log_info(log_ejecucion_cpu, "siguiente instruccion: %s \n", siguiente_instruccion);
		instruccion_spliteada = string_array_new();
		instruccion_spliteada = string_split(siguiente_instruccion, " ");

		//DECODE - ETAPA OPCIONAL - DONE
		if(!strcmp(instruccion_spliteada[0], "COPY")){
			log_info(log_ejecucion_cpu, "FETCH OPERAND");
			operando = fetchOperand(atoi(instruccion_spliteada[2]));
		}


		//EXECUTE - DONE
		log_info(log_ejecucion_cpu, "EXECUTE");
		int numOperacion = seleccionarOperacion(instruccion_spliteada[0]);
		log_info(log_ejecucion_cpu, "PROXIMA OPERACION A EJECUTAR: %s", instruccion_spliteada[0]);

		switch(numOperacion){
			// Cada funcion de cada instruccion tiene su documentacion hecha en instrucciones.c
			case NO_OP:				//CANTIDAD DE NO_OP, SEGUNDO PARAMETRO DE LA INSTRUCCION
				printf("# NO OP \n");
				log_info(log_ejecucion_cpu, "Se comienza a ejecutar la instruccion NO_OP");
				instr_no_op(atoi(instruccion_spliteada[1]));
				pcb_ejecutando.program_counter++;
				printf("# FIN NO OP \n\n");
				log_info(log_ejecucion_cpu, "Se termino de ejecutar la instruccion NO_OP \n");
				break;
			case IO: // DESPUES DE ESTA INSTRUCCION HAY QUE CORTAR LA EJECUCION
				printf("# IO \n");
				log_info(log_ejecucion_cpu, "Se comienza a ejecutar la instruccion IO");
				pcb_ejecutando.program_counter++;
				instr_io(  atoi(instruccion_spliteada[1]) );
				printf("# FIN IO \n\n");
				log_info(log_ejecucion_cpu, "Se termino de ejecutar la instruccion IO \n");
				break;
			case READ:				//DIR LOGICA
				printf("# READ \n");
				log_info(log_ejecucion_cpu, "Se comienza a ejecutar la instruccion READ");
				instr_read( atoi(instruccion_spliteada[1]));
				pcb_ejecutando.program_counter++;
				printf("# FIN READ \n\n");
				log_info(log_ejecucion_cpu, "Se termino de ejecutar la instruccion READ \n");
				break;
			case WRITE:				//DIR LOGICA					VALOR
				printf("# WRITE \n");
				log_info(log_ejecucion_cpu, "se comienza a ejecutar la instruccion WRITE");
				instr_write( atoi(instruccion_spliteada[1]), atoi(instruccion_spliteada[2]) );
				pcb_ejecutando.program_counter++;
				printf("# FIN WRITE \n\n");
				log_info(log_ejecucion_cpu, "se termino de ejecutar la instruccion WRITE \n");
				break;
			case COPY:				//DIR LOGICA DESTINO			VALOR
				printf("# COPY \n");
				log_info(log_ejecucion_cpu, "se comienza a ejecutar la instruccion COPY");
				instr_copy( atoi(instruccion_spliteada[1]), operando );
				pcb_ejecutando.program_counter++;
				printf("# FIN COPY \n\n");
				log_info(log_ejecucion_cpu, "se termino de ejecutar la instruccion COPY \n");
				break;
			case EXIT: // DESPUES DE ESTA INSTRUCCION HAY QUE CORTAR LA EJECUCION DONE
				printf("# EXIT \n");
				log_info(log_ejecucion_cpu, "se comienza a ejecutar la instruccion EXIT");
				instr_exit();
				printf("# FIN EXIT \n\n");
				log_info(log_ejecucion_cpu, "se termino de ejecutar la instruccion EXIT \n");
				break;
			default:
				printf("LA INSTRUCCION %s NO ES VALIDA \n", instruccion_spliteada[0]);
				log_error(log_ejecucion_cpu, "instruccion no valida");
				getchar();
		}



		// CHECK INTERRUPT
		log_info(log_ejecucion_cpu, "CHECK INTERRUPT");
		log_info(log_ejecucion_cpu, "chequeando interrupciones y/o syscall bloqueante");
		if( hay_interrupcion() ){
			log_info(log_ejecucion_cpu, "se detecto una interrupcion \n\n");
			// LIMPIAR TLB, DEVOLVER PCB.
			limpiar_TLB();

			t_syscall* pcb_desalojada = malloc(sizeof(t_syscall));
			pcb_desalojada->pcb.lista_instrucciones = malloc(string_length(pcb_ejecutando.lista_instrucciones));
			pcb_desalojada->pcb = pcb_ejecutando;
			pcb_desalojada->instruccion = 2;
			pcb_desalojada->tiempo_de_bloqueo = 0;
			enviar_syscall(pcb_desalojada);
			sem_post(&nada_ejecutando);
			sem_wait(&sem_interrupcion);
				interrupcion = 0;
			sem_post(&sem_interrupcion);
		}
		else if (se_hizo_una_syscall_bloqueante()){
			log_info(log_ejecucion_cpu, "se detecto una syscall bloqueante \n\n");
			// el pcb se envia en la instruccion IO o EXIT
			limpiar_TLB();
			sem_post(&nada_ejecutando);
			syscall_bloqueante=0;
		} else {
			log_info(log_ejecucion_cpu, "no hay interrupcion ni syscall bloqueante \n\n");
			sem_post(&ejecutar);
		}

		free(siguiente_instruccion);
		free(instruccion_spliteada);
	}

	return 0;
}

// recibe el aviso del kernel de que hay que desalojar
/**
 * 	Este hilo solamente atiende lo recibido de KERNEL para detener la ejecucion
 *
 */
void* interrupt(){
	sem_post(&hiloCreado);

	t_log* log_ejecucion_cpu_interrupt = log_create("./../logs/log_interrupt.log", "CPU - INTERRUPT", 0, LOG_LEVEL_INFO);


	log_info(log_ejecucion_cpu_interrupt, "hilo interrupt iniciado");
	uint32_t valor_recibido = -1;
	while(1){
		recv(socket_interrupt, &valor_recibido, sizeof(uint32_t), MSG_WAITALL);

		if(valor_recibido == 55){
			sem_wait(&sem_interrupcion);
				interrupcion = 1;
			sem_post(&sem_interrupcion);
			printf("#### SE RECIBIO UNA INTERRUPCION #### \n");
			log_info(log_ejecucion_cpu_interrupt, "Se recibio una interrupcion de kernel");
		}
		else
		{
			printf("EL HANDSHAKE RECIBIDO EN INTERRUPCION NO FUE 55 \n NO CAMBIO EL VALOR DE INTERRUPCION\n");
			log_error(log_ejecucion_cpu_interrupt, "se recibio una interrupcion pero con valor erroneo");
		}
	}

	return 0;
}

// FUNCIONES
// va a buscar el contenido de operando a memoria
uint32_t fetchOperand(uint32_t dir_logica){
	printf("# FETCH OPERAND (PASO PREVIO INSTRUCCION COPY) \n");
	uint32_t frame_a_buscar = buscar_frame(dir_logica);
	uint32_t offset = dir_logica - ( floor(dir_logica/info_traduccion.tamanio_paginas) * info_traduccion.tamanio_paginas );
	uint32_t numero_entrada = floor(numero_pagina / info_traduccion.entradas_por_tabla);
	uint32_t contenido_del_frame = pedir_contenido(frame_a_buscar, offset, numero_entrada);
	log_info(log_ejecucion_cpu, "datos fetcheados: frame_a_buscar: %i, offset: %i, numero_entrada: %i, contenido_del_frame: %i", frame_a_buscar, offset, numero_entrada, contenido_del_frame);
	log_info(log_ejecucion_cpu, "se termina de ejecutar fetchOperand");

	return contenido_del_frame;
}

void crear_TLB(){
	lista_tlb = list_create();
	for(int i=0;entradas_tlb > i;i++){
		t_tlb* tlb = malloc (sizeof(t_tlb));
		tlb->marco=-1;
		tlb->pagina=-1;
		tlb->primera_referencia = clock();
		tlb->ultima_referencia=clock();
		list_add(lista_tlb, tlb);
		log_info(log_ejecucion_cpu, "se creo la tlb");
	}
}

void limpiar_TLB(){
	list_iterate(lista_tlb,cambiar_puntero_tlb);
	log_info(log_ejecucion_cpu, "se limpio la tlb");
}

t_tlb* elegir_pagina_a_reemplazar_TLB(){
	t_tlb* pagina_a_retornar;

	//Algoritmo LRU
	if(!strcmp(reemplazo_tlb,"LRU")){
		log_info(log_ejecucion_cpu, "se elegira una pagina a reemplazar con algoritmo LRU");
		pagina_a_retornar=list_get_maximum(lista_tlb,tlb_menos_referenciado);
		log_info(log_ejecucion_cpu, "la entrada reemplazada contenia la pagina numero %i ", pagina_a_retornar->pagina);
	}

	//Como el unico algoritmo alternativo es fifo no se aclaran nuevas condiciones para esta entrada.
	//Algoritmo FIFO
	else{
		log_info(log_ejecucion_cpu, "se elegira una pagina a reemplazar con algoritmo FIFO");
		pagina_a_retornar= list_get_maximum(lista_tlb,tlb_mas_viejo);
		log_info(log_ejecucion_cpu, "la entrada reemplazada contenia la pagina numero %i ", pagina_a_retornar->pagina);
	}
	return pagina_a_retornar;
}

void guardar_en_TLB(uint32_t numero_de_pagina, uint32_t numero_de_frame){
	t_tlb* pagina_a_reemplazar = elegir_pagina_a_reemplazar_TLB();
	pagina_a_reemplazar->pagina = numero_de_pagina;
	pagina_a_reemplazar->marco = numero_de_frame;
	log_info(log_ejecucion_cpu, "la pagina a reemplazar es %i, frame %i", pagina_a_reemplazar->pagina, pagina_a_reemplazar->marco);
	log_info(log_ejecucion_cpu, "la pagina fue reemplazada por la numero %i, frame %i", numero_de_pagina, numero_de_frame);
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

// debe fijarse si la var global 'interrupcion' == 1
bool hay_interrupcion(){
	return interrupcion==1;
}

// debe fijarse si se hizo una syscall bloqueante
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

t_pcb recibir_PCB(){
    t_pcb_buffer buffer;

    recv(socket_dispatch, &(buffer.size), sizeof(uint32_t), MSG_WAITALL);

    recv(socket_dispatch, &(buffer.size_instrucciones), sizeof(uint32_t), MSG_WAITALL);

    buffer.stream = malloc(buffer.size);
    recv(socket_dispatch, buffer.stream, buffer.size, MSG_WAITALL);

    t_pcb pcb;
    int offset=0;
    memcpy(&(pcb.id_proceso), buffer.stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);

    memcpy(&(pcb.tamanio_direcciones), buffer.stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);


    pcb.lista_instrucciones = malloc(buffer.size_instrucciones + 1);
    memcpy(pcb.lista_instrucciones, buffer.stream+offset, buffer.size_instrucciones);
    offset+=buffer.size_instrucciones;

    log_info(log_ejecucion_cpu, "PCB RECIBIDO \n");
    log_info(log_ejecucion_cpu, "INSTRUCCIONES RECIBIDAS: %s \n", pcb.lista_instrucciones);


    memcpy(&(pcb.program_counter), buffer.stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    memcpy(&(pcb.tabla_paginas), buffer.stream+offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    memcpy(&(pcb.estimacion_rafagas), buffer.stream+offset, sizeof(uint32_t));

    return pcb;
}

