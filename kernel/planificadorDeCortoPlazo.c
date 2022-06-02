#include <stdio.h>
#include "planificadorDeCortoPlazo.h"

// Las versiones FIFO y SRT de las funciones pueden llegar a ser iguales salvo que usan colas o listas según corresponda
// enviando funciones como parámetro se podrían ahorrar algunos duplicados

void* ready_a_executing_fifo(){
	sem_post(&se_inicio_el_hilo);
	while(1) {
		sem_wait(&procesos_en_ready);
		sem_wait(&fin_de_ejecucion);
		sem_wait(&mx_lista_ready);
		enviar_a_CPU(queue_pop(&cola_ready)); // ENVIAR A CPU POR SOCKET
		sem_post(&mx_lista_ready);
	}
	return "";
}

void* ready_a_executing_srt(){
	sem_post(&se_inicio_el_hilo);
	while(1) {
		sem_wait(&procesos_en_ready);
		// lógica SRT

		}
	return "";
}
void* executing_a_ready_o_blocked() {
	// este le tiene que avisar al de ready a executing
	sem_post(&se_inicio_el_hilo);
	while(1) {
		// espera el fin de ejecución
		if(/*se fue a I/O*/) {
			executing_a_blocked();
		} else {
			executing_a_ready();
		}
	}

	return "";
}

void* executing_a_ready_fifo(){
	// HACE FALTA ESTA FUNCION?

	sem_wait(&mx_lista_ready);
	queue_push(cola_ready, /*proceso ejecutando*/);
	sem_post(&mx_lista_ready);

	return "";
}

void* executing_a_ready_srt(){
	sem_wait(&mx_lista_ready);
	list_add(lista_ready, /*proceso ejecutando*/);
	sem_post(&mx_lista_ready);

	return "";
}

void * executing_a_blocked_srt() {
	// no podemos usar el mismo para srt y fifo?

	sem_wait(&mx_lista_blocked);
	list_add(&lista_blocked, /*proceso ejecutando*/);
	sem_post(&mx_lista_blocked);

	return "";
}
