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

// Descripcion: recibe la cantidad de veces que se debe hacer el sleep(retardo_noop)
void instr_no_op(int cant_de_no_op){
	int i = 0;
	while( i < cant_de_no_op ){
		sleep(retardo_noop);
		i++;
	}
}



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


void instr_read(uint32_t dir_logica){
	uint32_t frame_a_utilizar = buscar_frame(dir_logica);


	uint32_t contenido_frame = pedir_contenido_frame(frame_a_utilizar);
	printf("%d \n",contenido_frame);

}



void instr_write(uint32_t dir_logica, uint32_t valor){
	uint32_t frame_a_utilizar = buscar_frame(dir_logica);
	escribir_frame(frame_a_utilizar, valor);
}



void instr_copy(uint32_t dir_logica_destino, uint32_t valor){
	uint32_t frame_destino= buscar_frame(dir_logica_destino);
	escribir_frame(frame_destino,valor);

}



void instr_exit(){
	t_syscall* exit=malloc(sizeof(t_syscall));
	exit->pcb.lista_instrucciones= malloc(string_length(pcb_ejecutando.lista_instrucciones));
	exit->pcb= pcb_ejecutando;
	exit->instruccion=1;
	exit->tiempo_de_bloqueo=0;
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


	send(socket_dispatch, a_enviar, buffer->size + sizeof(uint32_t) * 2, 0);

	free(buffer->stream);
	free(buffer);
	free(syscall_a_enviar->pcb.lista_instrucciones);
	free(syscall_a_enviar);
}

uint32_t buscar_frame(uint32_t dir_logica){ // @suppress("No return")
	numero_pagina= floor(dir_logica/info_traduccion.tamanio_paginas);
	if(list_any_satisfy(tlbs,encontrar_pagina)){
		t_tlb* tlb_a_retornar=list_find(tlbs, encontrar_pagina);
		return tlb_a_retornar->marco;
	}
	else{
		uint32_t numero_frame=pedir_todo_memoria();
		guardar_en_TLB(numero_pagina, numero_frame);
		return numero_frame;
	}

}

bool encontrar_pagina(void* tlb){
	t_tlb* tlb_en_uso = tlb;
	return (numero_pagina == tlb_en_uso->pagina);

}

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
uint32_t pedir_num_frame(uint32_t entrada_2da_tabla, uint32_t num_tabla_2){
	//EL CODIGO DE OPERACION ES 1
	uint32_t codigo_de_operacion =1;
	send(socket_memoria, &codigo_de_operacion,sizeof(uint32_t),0);
    //ENVIAR NUMERO TABLA DE PAGINAS 2
    send(socket_memoria, &(num_tabla_2), sizeof(uint32_t), 0);
    //ENVIAR ENTRADA DE TABLA 2
    send(socket_memoria, &entrada_2da_tabla, sizeof(uint32_t), 0);

    uint32_t num_frame;
    recv(socket_memoria, &num_frame, sizeof(uint32_t), MSG_WAITALL);
    return num_frame;
}
uint32_t pedir_contenido_frame(uint32_t numero_de_frame){
	//EL CODIGO DE OPERACION ES 2
	uint32_t codigo_de_operacion =2;
	send(socket_memoria, &codigo_de_operacion,sizeof(uint32_t),0);
    //ENVIAR NUMERO DE FRAME
    send(socket_memoria, &numero_de_frame, sizeof(uint32_t), 0);

    uint32_t contenido;
    recv(socket_memoria, &contenido, sizeof(uint32_t), MSG_WAITALL);
    return contenido;
}
uint32_t pedir_todo_memoria(){
	uint32_t entrada_1er_tabla = floor(numero_pagina/info_traduccion.entradas_por_tabla);
	uint32_t num_tabla_2 =pedir_num_tabla_2(entrada_1er_tabla);
	uint32_t entrada_2da_tabla = numero_pagina % info_traduccion.entradas_por_tabla;
	uint32_t numero_frame = pedir_num_frame(entrada_2da_tabla, num_tabla_2);
	return numero_frame;
}

void escribir_frame(uint32_t numero_de_frame, uint32_t valor){
	//EL CODIGO DE OPERACION ES 3
	uint32_t codigo_de_operacion =3;
	send(socket_memoria, &codigo_de_operacion,sizeof(uint32_t),0);
    //ENVIAR NUMERO DE FRAME
    send(socket_memoria, &numero_de_frame, sizeof(uint32_t), 0);
    //ENVIAR VALOR
    send(socket_memoria, &valor, sizeof(uint32_t), 0);
    uint32_t respuesta;
    recv(socket_memoria, &respuesta, sizeof(uint32_t), MSG_WAITALL);
    if(respuesta == 1){
    	printf("Se escribio en el frame.");
    }
    else printf("La mula esta de huelga");
}
