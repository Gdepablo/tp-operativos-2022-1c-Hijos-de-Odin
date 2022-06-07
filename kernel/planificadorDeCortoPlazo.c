#include <stdio.h>
#include "planificadorDeCortoPlazo.h"

// Las versiones FIFO y SRT de las funciones pueden llegar a ser iguales salvo que usan colas o listas según corresponda
// enviando funciones como parámetro se podrían ahorrar algunos duplicados

void* ready_a_executing(){
	sem_post(&se_inicio_el_hilo);
	while(1) {
		sem_wait(&procesos_en_ready);
		sem_wait(&fin_de_ejecucion);

		sem_wait(&mx_lista_ready);
		if(algoritmo()) {
			enviar_a_CPU(queue_pop(&cola_ready)); // ENVIAR A CPU POR SOCKET
		} else {
			// algotirmo SJF
		}
		sem_post(&mx_lista_ready);
	}
	return "";
}

void enviar_a_CPU(t_pcb pcb) {

}

void* desalojar() {
	while(1) {
		if(algoritmo()) {
			// FIFO
		} else {

		}
	}
	return "";
}

// Se ejecuta en SJF al desalojar un proceso
void executing_a_ready(t_pcb pcb){ // le falta
	sem_wait(&proceso_nuevo_en_ready);

	send(socket_cpu_interrupcion, algo, sizeof(algo), 0);
	recv(*socket_cpu_pcb, syscall, sizeof(syscall), MSG_WAITALL);

	sem_wait(&mx_lista_ready);
		list_add(&lista_ready, pcb);
	sem_post(&mx_lista_ready);

}



