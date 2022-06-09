#include <stdio.h>
#include <semaphore.h>
#include "planificadorDeLargoPlazo.h"

void ingreso_a_new(t_pcb* pcb) {
	// guardar en memoria XD
	sem_wait(&mx_cola_new);
		queue_push(cola_new, pcb);
	sem_post(&mx_cola_new);

	sem_post(&procesos_en_ready);
}

// 	THREADS

void* new_a_ready(){
	sem_post(&se_inicio_el_hilo);
	while(1){
		sem_wait(&procesos_en_new);
		sem_wait(&grado_multiprogramacion);

		sem_wait(&mx_lista_ready);
		sem_wait(&mx_cola_new);
		if(es_FIFO()) {
			// FIFO
				queue_push(&cola_ready, queue_pop(&cola_new));
		} else {
			// SJF
				list_add(&lista_ready, queue_pop(&cola_new));
		}
		sem_post(&mx_cola_new);
		sem_post(&mx_lista_ready);

		sem_post(&procesos_en_ready);
	}
	return "";
}

void executing_a_exit(t_pcb* pcb){
	sem_post(&grado_multiprogramacion);
	// liberar memoria XD
	if(!es_FIFO()) {
		// SJF
		PCB_EJECUCION.id_proceso = -1;
		sem_post(&procesos_en_ready);
	}
}
