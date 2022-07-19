#include "cpu.h"
#include <commons/collections/list.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

int seleccionarOperacion(char* nombre_instruccion){
	if(!strcmp(nombre_instruccion, "NO_OP")) {
		return NO_OP;
	}
	if(!strcmp(nombre_instruccion, "IO")) {
		return IO;
	}
	if(!strcmp(nombre_instruccion, "READ")){
		return READ;
	}
	if(!strcmp(nombre_instruccion, "WRITE")){
		return WRITE;
	}
	if(!strcmp(nombre_instruccion, "COPY")){
		return COPY;
	}
	if(!strcmp(nombre_instruccion, "EXIT")){
		return EXIT;
	}

	return -1;
}


// Descripcion: recibe la cantidad de veces que se debe hacer el sleep(retardo_noop).
// Ejemplo: instr_no_op(5) ejecuta 5 veces un usleep con el tiempo dado en el config pasado a milisegundos

void instr_no_op(int cant_de_no_op){
	int i = 0;
	while( i < cant_de_no_op ){
		usleep(retardo_noop * 1000); // pasa de milisegundos (dado por config) a microsegundos (lo que usa usleep)
		i++;
	}
}



// Descripcion: simula ser una operacion bloqueante de i/o, entonces se envia a kernel con el tiempo que diga la instruccion
// Ejemplo: instr_io(1000) le envia a kernel el PCB sumado al codigo de instruccion + el numero 1000 (enviado por parametro)

void instr_io(int tiempo_en_milisegundos){
	extern t_pcb pcb_ejecutando;
	t_syscall* syscall_a_enviar = malloc(sizeof(t_syscall));
	//instruccion = IO
	syscall_a_enviar->instruccion = 0;
	//tiempo de bloqueo
	syscall_a_enviar->tiempo_de_bloqueo = tiempo_en_milisegundos;
	//pcb
	syscall_a_enviar->pcb.estimacion_rafagas = pcb_ejecutando.estimacion_rafagas;
	syscall_a_enviar->pcb.id_proceso = pcb_ejecutando.estimacion_rafagas;
	syscall_a_enviar->pcb.program_counter = pcb_ejecutando.program_counter;
	syscall_a_enviar->pcb.tabla_paginas = pcb_ejecutando.tabla_paginas;
	syscall_a_enviar->pcb.tamanio_direcciones = pcb_ejecutando.tamanio_direcciones;
	syscall_a_enviar->pcb.lista_instrucciones = malloc(string_length(pcb_ejecutando.lista_instrucciones));
	strcpy(syscall_a_enviar->pcb.lista_instrucciones, pcb_ejecutando.lista_instrucciones);

	enviar_syscall(syscall_a_enviar);
	syscall_bloqueante=1;
}


// Descripcion: leer una direccion logica. Se envia una solicitud a memoria, esta retorna el contenido en el frame con
// el offset pedido y el valor es mostrado por pantalla
// Ejemplo: instr_read(11) lee lo que haya en la direccion logica 11 del proceso y la muestra por pantalla

void instr_read(uint32_t dir_logica){
	uint32_t frame_a_utilizar = buscar_frame(dir_logica);
										// formula de numero de pagina
	uint32_t offset = dir_logica - ( floor(dir_logica/info_traduccion.tamanio_paginas) * info_traduccion.tamanio_paginas );
	uint32_t entrada_1er_tabla = floor(numero_pagina / info_traduccion.entradas_por_tabla );
	uint32_t contenido_frame = pedir_contenido(frame_a_utilizar, offset, entrada_1er_tabla);
	printf("Dato leido en la direccion logica %i: %i \n", dir_logica, contenido_frame);

}



// Descripcion: escribir en una direccion logica. Se envia la solicitud a memoria para que esta escriba. Esta contesta con un
// OK si se pudo escribir.
// Ejemplo: instr_write(11, 69) escribe en la direccion 11 el valor 69

void instr_write(uint32_t dir_logica, uint32_t valor){
	uint32_t frame_a_utilizar = buscar_frame(dir_logica);
	uint32_t offset = dir_logica - ( floor(dir_logica/info_traduccion.tamanio_paginas) * info_traduccion.tamanio_paginas );
	uint32_t entrada_1er_nivel = floor(numero_pagina / info_traduccion.entradas_por_tabla);
	escribir_frame(frame_a_utilizar, offset, valor, entrada_1er_nivel);
}


// Descripcion: copiar un valor en una direccion logica. muy parecida a instr_write, por la salvedad de que el valor viene del
// contenido de otra direccion logica del proceso.
// Ejemplo: instr_write(11, 69) escribe en la direccion 11 el valor 69, siendo este el valor que se recibio antes

void instr_copy(uint32_t dir_logica_destino, uint32_t valor){
	uint32_t frame_destino= buscar_frame(dir_logica_destino);
	uint32_t offset = dir_logica_destino - ( floor(dir_logica_destino/info_traduccion.tamanio_paginas) * info_traduccion.tamanio_paginas );
	uint32_t entrada_1er_nivel = floor(numero_pagina / info_traduccion.entradas_por_tabla);
	escribir_frame(frame_destino, offset, valor, entrada_1er_nivel);
}

// Descripcion: manda a la puta al proceso. Devuelve la PCB al kernel y este termina de realizar las tareas administrativas.
// Ejemlpo: se esta ejecutando el proceso A, al ejecutarse instr_exit, se manda el pcb del proceso A al kernel de vuelta.

void instr_exit(){
	t_syscall* exit=malloc(sizeof(t_syscall));
	exit->pcb.lista_instrucciones = malloc(string_length(pcb_ejecutando.lista_instrucciones));
	exit->pcb = pcb_ejecutando;
	exit->instruccion = 1;
	exit->tiempo_de_bloqueo = 0;
	enviar_syscall(exit);
	syscall_bloqueante=1;
}

void enviar_syscall(t_syscall* syscall_a_enviar){
	extern int socket_dispatch;

	t_pcb_buffer* buffer = malloc(sizeof(t_pcb_buffer));
	buffer->size = sizeof(uint32_t) * 7 + strlen(syscall_a_enviar->pcb.lista_instrucciones);
	buffer->size_instrucciones = strlen(syscall_a_enviar->pcb.lista_instrucciones);
	buffer->stream = malloc(buffer->size);

	int offset = 0;
	// COPY DEL NUMERO DE INSTRUCCION
	memcpy(buffer->stream+offset, &(syscall_a_enviar->instruccion), sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	// COPY DEL TIEMPO DE BLOQUEO
	memcpy(buffer->stream+offset, &(syscall_a_enviar->tiempo_de_bloqueo), sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	// COPY DEL PCB
	memcpy(buffer->stream + offset, &(syscall_a_enviar->pcb.id_proceso), sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(buffer->stream + offset, &(syscall_a_enviar->pcb.tamanio_direcciones), sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	// ATENCION ZONA DE PELIGRO ATENCION ZONA DE PELIGRO ATENCION ZONA DE PELIGRO ATENCION ZONA DE PELIGRO
	memcpy(buffer->stream + offset, syscall_a_enviar->pcb.lista_instrucciones, buffer->size_instrucciones);
	offset+=buffer->size_instrucciones;
	// FIN ZONA DE PELIGRO FIN ZONA DE PELIGRO FIN ZONA DE PELIGRO FIN ZONA DE PELIGRO FIN ZONA DE PELIGRO
	memcpy(buffer->stream+offset, &(syscall_a_enviar->pcb.program_counter), sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(buffer->stream+offset, &(syscall_a_enviar->pcb.tabla_paginas), sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(buffer->stream+offset, &(syscall_a_enviar->pcb.estimacion_rafagas), sizeof(uint32_t));


	offset = 0;
	int tamanio_stream = sizeof(uint32_t) * 2 + buffer->size;
	void* a_enviar = malloc(tamanio_stream);
	memcpy(a_enviar + offset, &(buffer->size), sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(a_enviar + offset, &(buffer->size_instrucciones), sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(a_enviar+offset, buffer->stream, buffer->size);


	send(socket_dispatch, a_enviar, buffer->size + sizeof(uint32_t) * 2 + 1, 0);

	free((syscall_a_enviar->pcb).lista_instrucciones);
	free(syscall_a_enviar);
	free(buffer->stream);
	free(buffer);
}

// Descripcion:
uint32_t buscar_frame(uint32_t dir_logica){ // @suppress("No return")
	numero_pagina = floor(dir_logica/info_traduccion.tamanio_paginas);
	if(list_any_satisfy(lista_tlb,encontrar_pagina)){
		printf("LA DIRECCION LOGICA %i ESTA EN TLB, ", dir_logica);
		t_tlb* tlb_a_retornar=list_find(lista_tlb, encontrar_pagina);
		if(marco_obsoleto(tlb_a_retornar->marco)){
			printf("PERO EL DATO ESTA OBSOLETO \n");
			//esta la pagina pero esta duplicada y obsoleta
			uint32_t numero_frame=pedir_todo_memoria();
			guardar_en_TLB(numero_pagina, numero_frame);
			return numero_frame;
		}
		else
		{
			printf("Y EL DATO NO ESTA OBSOLETO \n");
			//esta la pagina y no esta obsoleta XD
			tlb_a_retornar->ultima_referencia=clock();
			return tlb_a_retornar->marco;
		}
	}
	else{
		printf("no esta en tlb \n");
		// la pagina no esta en ninguna entrada
		uint32_t numero_frame=pedir_todo_memoria();
		guardar_en_TLB(numero_pagina, numero_frame);
		return numero_frame;
	}
}

bool encontrar_pagina(void* tlb){
	t_tlb* tlb_en_uso = tlb;
	return (numero_pagina == tlb_en_uso->pagina);

}



// Descripcion: Pide a memoria el numero de la segunda tabla ubicada en la entrada especificada de la tabla de primer nivel
// especificada. Retorna el valor que le retorna memoria
// Ejemplo: pedir_num_tabla_2(3) le pide a memoria que: de la tabla de paginas de primer nivel del proceso, le de el numero
// de segunda tabla que esta en la entrada numero 3.
uint32_t pedir_num_tabla_2(uint32_t entrada_1er_tabla){
	//EL CODIGO DE OPERACION ES 0
	uint32_t codigo_de_operacion =0;
	send(socket_memoria, &codigo_de_operacion,sizeof(uint32_t),0);
    //ENVIAR NUMERO TABLA DE PAGINAS 1
    send(socket_memoria, &(pcb_ejecutando.tabla_paginas), sizeof(uint32_t), 0);
    //ENVIAR ENTRADA DE TABLA 1
    send(socket_memoria, &entrada_1er_tabla, sizeof(uint32_t), 0);


    uint32_t num_tabla_2;
    recv(socket_memoria, &num_tabla_2, sizeof(uint32_t), MSG_WAITALL);
    return num_tabla_2;
}



// Descripcion: Pide a memoria que retorne el numero de frame del numero de pagina que se encuentra en la entrada de la tabla
// de segundo nivel dada.
// Ejemplo: pedir_num_frame(1, 2) pide a memoria que se retorne el numero de frame asignado a la pagina que esta en la
// tabla 2, entrada 1

uint32_t pedir_num_frame(uint32_t entrada_2da_tabla, uint32_t num_tabla_2, uint32_t entrada_1er_tabla){
	//EL CODIGO DE OPERACION ES 1
	uint32_t codigo_de_operacion =1;
	send(socket_memoria, &codigo_de_operacion,sizeof(uint32_t),0);
	// PROCESS ID
	send(socket_memoria, &(pcb_ejecutando.id_proceso), sizeof(uint32_t), 0);
    //ENVIAR NUMERO TABLA DE PAGINAS 2
    send(socket_memoria, &(num_tabla_2), sizeof(uint32_t), 0);
    //ENVIAR ENTRADA DE TABLA 2
    send(socket_memoria, &entrada_2da_tabla, sizeof(uint32_t), 0);
    // NUMERO DE TABLA 1
    send(socket_memoria, &(pcb_ejecutando.tabla_paginas), sizeof(uint32_t), 0);

    uint32_t num_frame;
    recv(socket_memoria, &num_frame, sizeof(uint32_t), MSG_WAITALL);
    return num_frame;
}



// Descripcion: pide a memoria el contenido que se encuentra en el numero de frame con el offset dado
// Ejemplo: pedir_contenido(5, 13) pide a memoria que del frame 5 + 13 de offset me retorne su contenido.
uint32_t pedir_contenido(uint32_t numero_de_frame, uint32_t offset, uint32_t entrada_1er_tabla){
	//EL CODIGO DE OPERACION ES 2
	uint32_t codigo_de_operacion =2;
	send(socket_memoria, &codigo_de_operacion,sizeof(uint32_t),0);
    //ENVIAR NUMERO DE FRAME
    send(socket_memoria, &numero_de_frame, sizeof(uint32_t), 0);
    //ENVIAR OFFSET
    send(socket_memoria, &offset, sizeof(uint32_t), 0);
    //ENVIAR NUMERO 1ER TABLA
    send(socket_memoria, &(pcb_ejecutando.tabla_paginas), sizeof(uint32_t), 0);
    //ENVIAR ENTRADA 1ER TABLA
    send(socket_memoria, &entrada_1er_tabla, sizeof(uint32_t), 0);

    uint32_t contenido;
    recv(socket_memoria, &contenido, sizeof(uint32_t), MSG_WAITALL);

    return contenido;
}

// Descripcion: pide todos los datos a memoria menos el contenido del frame.
uint32_t pedir_todo_memoria(){
	uint32_t entrada_1er_tabla = floor(numero_pagina/info_traduccion.entradas_por_tabla);
	uint32_t num_tabla_2 =pedir_num_tabla_2(entrada_1er_tabla);
	uint32_t entrada_2da_tabla = numero_pagina % info_traduccion.entradas_por_tabla;
	uint32_t numero_frame = pedir_num_frame(entrada_2da_tabla, num_tabla_2, entrada_1er_tabla);
	return numero_frame;
}


// Descripcion: escribe en el numero de frame con el offset dado, el valor del tercero parametro.
// ejemplo: escribir_frame(1,5,8) escribe en el frame 1 con un offset de 5, el valor 8

/* TODO
 * agregar el offset po aweonao XD
 * eso afecta instr_copy e instr_write, compilar antes de pushear anashe
 */
void escribir_frame(uint32_t numero_de_frame, uint32_t offset, uint32_t valor, uint32_t entrada_1er_tabla){
	//EL CODIGO DE OPERACION ES 3
	uint32_t codigo_de_operacion =3;
	send(socket_memoria, &codigo_de_operacion,sizeof(uint32_t),0);
    //ENVIAR NUMERO DE FRAME
    send(socket_memoria, &numero_de_frame, sizeof(uint32_t), 0);
    //ENVIAR OFFSET
    send(socket_memoria, &offset, sizeof(uint32_t), 0);
    //ENVIAR VALOR
    send(socket_memoria, &valor, sizeof(uint32_t), 0);
    //ENVIAR 1RA TABLA
    send(socket_memoria, &(pcb_ejecutando.tabla_paginas), sizeof(uint32_t), 0);
    //ENVIAR ENTRADA 1RA TABLA
    send(socket_memoria, &entrada_1er_tabla, sizeof(uint32_t), 0);


    uint32_t respuesta;
    recv(socket_memoria, &respuesta, sizeof(uint32_t), MSG_WAITALL);
    if(respuesta == 1){
    	printf("Se escribio en el frame %i, offset %i, el valor %i \n", numero_de_frame, offset, valor);
    }
    else printf("La mula esta de huelga.");
}
