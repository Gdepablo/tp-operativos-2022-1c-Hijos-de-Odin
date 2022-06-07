#include <stdio.h>
#include "planificadorDeMedianoPlazo.h"

void* blocked_a_ready_o_suspended_blocked(pcb){

	usleep(TIEMPO_MAXIMO_BLOQUEADO);
	if(sigueBloqueado(pcb)) {
		blocked_a_suspended_blocked(pcb); // <- poner signal(&grado_multiprogramacion);
	}
	else{
		blocked_a_ready();
	}
	return "";
}

void blocked_a_ready() {

	sem_wait(&mx_cola_blocked);
		if(algoritmo()) {
			queue_push(&cola_ready, queue_pop(&cola_blocked));
		} else {
			list_add(&lista_ready, queue_pop(&cola_blocked));
		}
	sem_post(&mx_cola_blocked);
	/*int posicion;
	t_pcb* pcbaux;
	int size = list_size(lista_blocked);

	for(int i = 0; i < size; i++) {
		posicion = i;
		pcbaux = list_get(lista_blocked, i);

		if( (pcbaux -> id_proceso) == (pcb -> id_proceso) ) {
			break;
		}
	}

	list_remove(lista_blocked, posicion);
	if(algoritmo()) {
		// FIFO
		queue_push(cola_ready, pcbaux);
	} else {
		// SJF
		list_add(lista_ready, pcbaux);
	}
*/
}

void blocked_a_suspended_blocked(t_pcb* pcb){
	wait(&mx_lista_blocked);
	// ver bien esto XD
	t_pcb* pcb_a_suspender = list_add(lista_blocked);
	signal(&mx_lista_blocked);

	// LOGICA PARA COMUNICARSE CON MEMORIA

	wait(&mx_lista_suspended_blocked);
	list_add(lista_suspended_blocked, pcb_a_suspender);
	signal(&mx_lista_suspended_blocked);

	signal(&grado_multiprogramacion);
}

void* suspended_blocked_a_suspended_ready() {

	return "";
}

void* suspended_ready_a_ready(){
	sem_post(&se_inicio_el_hilo);
	while(1){
		wait(&grado_multiprogramacion); // = GRADO_MULTIPROGRAMACION;

		wait(mx_lista_ready);
		pasarloaready();
		signal(mx_lista_ready);
	}

	return "";
}
// SE EJECUTA AL RECIBIR UNA SYSCALL
void* executing_a_blocked_o_exit() {
	sem_post(&se_inicio_el_hilo);
	while(1) {
		t_syscall una_syscall = malloc(executing);
		recv(*socket_cpu_pcb, una_syscall, sizeof(una_syscall), MSG_WAITALL); // ver sizeof
		if(una_syscall.instruccion) {
			// EXIT
			executing_a_exit(una_syscall.pcb); // largo plazo
		} else {
			// IO
			executing_a_blocked(una_syscall); // mediano plazo
		}
		sem_post(&fin_de_ejecucion);
	}
	return "";
}

void executing_a_blocked(t_syscall una_syscall) {
	sem_wait(&mx_cola_blocked);
		queue_push(&cola_blocked, una_syscall.pcb);
	sem_post(&mx_cola_blocked);

	t_io* io;
	io->id_proceso = una_syscall.pcb->id_proceso;
	io->tiempo_de_bloqueo = una_syscall.tiempo_de_bloqueo;

	sem_wait(&mx_cola_io);
		queue_push(&cola_io, io);
	sem_post(&mx_cola_io);

	sem_post(&proceso_bloqueado);
}

void* inputOuput() {
	t_io io;
	while(1) {
		sem_wait(&proceso_bloqueado);
		io = queue_pop(&cola_io);
		if(io.tiempo_de_bloqueo < TIEMPO_MAXIMO_BLOQUEADO) {
			usleep(io.tiempo_de_bloqueo);
			blocked_a_ready();
		}
	}
	return "";
}

















