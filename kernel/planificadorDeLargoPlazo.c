#include <stdio.h>
#include <semaphore.h>
#include "planificadorDeLargoPlazo.h"

// funcion
void* ingreso_a_new(t_pcb* pcb) {
	sem_wait(&mx_cola_new);
	queue_push(cola_new, pcb);
	sem_post(&mx_cola_new);

	sem_post(&procesos_en_ready);

	return "";
}

//threads
void* new_a_ready_fifo(){
	sem_post(&se_inicio_el_hilo);
	while(1){
		sem_wait(&procesos_en_ready);
		sem_wait(&grado_multiprogramacion);
		int a = 2;
		sem_wait(&mx_lista_ready);
		sem_wait(&mx_cola_new);
		queue_push(lista_ready, queue_pop(cola_new));
		sem_post(&mx_cola_new);
		sem_signal(&mx_lista_ready);
	}
	return "";
}

void* new_a_ready_srt(){
	sem_post(&se_inicio_el_hilo);
	while(1){
		sem_wait(&procesos_en_ready);
		sem_wait(&grado_multiprogramacion);

		sem_wait(&mx_lista_ready);
		sem_wait(&mx_cola_new);
		list_add(lista_ready, queue_pop(&cola_new));
		sem_post(&mx_cola_new);
		sem_signal(&mx_lista_ready);
	}
	return "";
}

void* executing_a_exit(){
	sem_post(&se_inicio_el_hilo);
	while(1){
		sem_wait(&proceso_finalizado);
		//tenemos que liberar los punteros que hayan dentro del pcb antes de liberar el puntero al pcb
		free(executing);

		sem_post(&grado_multiprogramacion);
	}
	return "";
}
