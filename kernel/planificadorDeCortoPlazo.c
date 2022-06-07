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


void* executing_a_ready_fifo(){
	// HACE FALTA ESTA FUNCION?
	//luca: respondo abajo

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

void executing_a_blocked(t_syscall syscall) {
	while(1) {
		queue_push(&cola_blocked, syscall.pcb);
		pthread_t hilo;
		pthread_create(&hilo, NULL, contador_tiempo_bloqueado(), );
	}
}

void* contador_tiempo_bloqueado(uint32_t id) {
	usleep(TIEMPO_MAXIMO_BLOQUEADO);

	return "";
}



