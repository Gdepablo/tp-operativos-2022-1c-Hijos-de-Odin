#include "cpu.h"

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


void instr_read(){
	//todo
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
	// envia por el socket_dispatch
	// trabajo para batata aka el gordo printf
}

void romperTodo(){
	//todo
}
