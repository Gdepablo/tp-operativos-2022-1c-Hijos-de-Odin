#include <stdio.h>
#include <semaphore.h>
#include "planificadorDeLargoPlazo.h"

void* ingreso_a_new(t_pcb* pcb) {
	sem_wait(&mx_cola_new);
	queue_push(cola_new, pcb);
	sem_post(&mx_cola_new);

	sem_post(&procesos_en_ready);

	return "";
}

// 	THREADS

void* new_a_ready_fifo(){
	sem_post(&se_inicio_el_hilo);
	while(1){
		sem_wait(&procesos_en_ready);
		sem_wait(&grado_multiprogramacion);

		sem_wait(&mx_lista_ready);
		sem_wait(&mx_cola_new);
			queue_push(lista_ready, queue_pop(cola_new));
		sem_post(&mx_cola_new);
		sem_post(&mx_lista_ready);
	}
	return "";
}

void* new_a_ready(){
	sem_post(&se_inicio_el_hilo);
	while(1){
		sem_wait(&procesos_en_ready);
		sem_wait(&grado_multiprogramacion);

		sem_wait(&mx_lista_ready);
		sem_wait(&mx_cola_new);
			list_add(lista_ready, queue_pop(&cola_new));
		sem_post(&mx_cola_new);
		sem_post(&mx_lista_ready);
	}
	return "";
}

void* executing_a_exit(t_pcb* pcb){
	free(pcb->id_proceso);
	free(pcb->tamanio_direcciones);
	free(pcb->instrucciones);
	free(pcb->program_counter);
	free(pcb->tabla_paginas);
	free(pcb->estimacion_rafagas);
	free(pcb);

	sem_post(&grado_multiprogramacion);
	return "";
}
