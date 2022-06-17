#include "cpu.h"
#include <commons/collections/list.h>
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
}


void instr_read(uint32_t dir_logica){
	extern tlbs;
	//if(){}
	printf();
}



void instr_write(){
	//todo
}



void instr_copy(){
	//todo
}



void instr_exit(){
	//todo
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

void romper_todo(){
	//todo
}

void decidir_fulbo(uint32_t dir_logica){
	uint32_t num_pagina = floor(dir_logica/tamanio_de_pagina);
	if(list_any_satisfy(tlbs,encontrar_pagina(num_pagina))){
		list_find(tlbs, encontrar_pagina(num_pagina));
	}
}

void encontrar_pagina(uint32_t num){
	//todo
}

