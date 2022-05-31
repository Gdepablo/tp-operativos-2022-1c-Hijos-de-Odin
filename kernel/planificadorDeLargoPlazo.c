#include <stdio.h>
#include <semaphore.h>
#include "planificadorDeLargoPlazo.h"


void* ingreso_a_new(pcb) {
	sem_wait(&mx_cola_new);
	queue_push(cola_new, pcb);
	sem_post(&mx_cola_new);

	sem_post(&procesos_en_ready);

	return "";
}

void* new_a_ready_fifo(){
	while(1){
		sem_wait(&procesos_en_ready);
		sem_wait(&grado_multiprogramacion);

		sem_wait(&mx_lista_ready);
		sem_wait(&mx_cola_new);
		queue_push(lista_ready, queue_pop(&cola_new));
		sem_post(&mx_cola_new);
		sem_signal(&mx_lista_ready);
	}
	return "";
}

void* new_a_ready_srt(){
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
	while(1){
		sem_wait(&proceso_finalizado);

		free(executing);

		sem_post(&grado_multiprogramacion);
	}
	return "";
}
