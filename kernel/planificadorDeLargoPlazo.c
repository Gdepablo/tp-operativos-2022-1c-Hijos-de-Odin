#include <stdio.h>
#include <semaphore.h>
#include "planificadorDeLargoPlazo.h"
#include "semaforos.h"


void ingreso_a_new(t_pcb* pcb) {
	// guardar en memoria XD
	sem_wait(&mx_cola_new);
		queue_push(cola_new, pcb);
		printf("se metio en queue al pcb \n");
	sem_post(&mx_cola_new);

	sem_post(&procesos_en_new);
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
				//
				t_pcb* pcb = queue_pop(cola_new);
				crear_tabla_proceso(pcb);
				//
				queue_push(cola_ready, pcb);

		} else {
			// SJF
				list_add(lista_ready, queue_pop(cola_new));

		}
		sem_post(&mx_cola_new);
		sem_post(&mx_lista_ready);

		sem_post(&procesos_en_ready);
	}
	return "";
}

void crear_tabla_proceso(t_pcb* pcb){
	// CODIGO DE OPERACION = 4
	int codop = 4;
	send(socket_memoria, &codop, sizeof(uint32_t), 0);

	// FULBO
	send(socket_memoria, &(pcb->id_proceso), sizeof(uint32_t), 0);
	send(socket_memoria, &(pcb->tamanio_direcciones), sizeof(uint32_t), 0);

	// RECIBO NUMERO DE TABLA
	recv(socket_memoria, &(pcb->tabla_paginas), sizeof(uint32_t), MSG_WAITALL);
}

void executing_a_exit(t_syscall* una_syscall){
	sem_post(&grado_multiprogramacion);
	// avisar a consola
	uint32_t OK = 10;
	send(una_syscall->pcb.id_proceso, &OK, sizeof(uint32_t), 0);

	if(!es_FIFO()) {
		// SJF
		PCB_EJECUCION.id_proceso = -1;
		sem_post(&procesos_en_ready);
	}
	else
	{
		sem_post(&fin_de_ejecucion);
	}
}
