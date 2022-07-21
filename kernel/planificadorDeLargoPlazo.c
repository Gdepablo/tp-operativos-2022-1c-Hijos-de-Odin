#include <stdio.h>
#include <semaphore.h>
#include "planificadorDeLargoPlazo.h"
#include "semaforos.h"

void ingreso_a_new(t_pcb* pcb) {
	sem_wait(&mx_cola_new);
		queue_push(cola_new, pcb);
	sem_post(&mx_cola_new);
	printf("Proceso %i # Ingreso en new \n", pcb->id_proceso);

	sem_post(&procesos_para_ready);
}

void* pasar_a_ready() {
//	sem_post(&se_inicio_el_hilo);
	while(1) {
		sem_wait(&procesos_para_ready);
		sem_wait(&grado_multiprogramacion);

		if( queue_size(cola_suspended_ready) == 0 ) {
			sem_wait(&mx_cola_new);
				t_pcb* pcb = queue_pop(cola_new);
			sem_post(&mx_cola_new);

			crear_tabla_proceso(pcb);

			if(es_FIFO()) {
				// FIFO
				sem_wait(&mx_lista_ready);
					queue_push(cola_ready, pcb);
				sem_post(&mx_lista_ready);
			} else {
				// SJF
				sem_wait(&mx_lista_ready);
					list_add(lista_ready, pcb);
				sem_post(&mx_lista_ready);
			}
			printf("Proceso %i # Ingreso en ready desde new \n", pcb->id_proceso);
		}
		else {
			sem_wait(&mx_cola_suspended_ready);
				t_pcb* pcb = queue_pop(cola_suspended_ready);
			sem_post(&mx_cola_suspended_ready);

			if(es_FIFO()) {
				sem_wait(&mx_lista_ready);
					queue_push(cola_ready, pcb);
				sem_post(&mx_lista_ready);
//				printf("PROCESO PASADO A READY DESDE SUSPENDED: %i \n", pcb->id_proceso);
			} else {
				sem_wait(&mx_lista_ready);
					list_add(lista_ready, pcb);
				sem_post(&mx_lista_ready);
//				printf("PROCESO PASADO A READY DESDE SUSPENDED: %i \n", pcb->id_proceso);
			}
			printf("Proceso %i # Ingreso en ready desde suspendido \n", pcb->id_proceso);
		}
		sem_post(&procesos_en_ready);
	}
	return "";
}

void crear_tabla_proceso(t_pcb* pcb){
	int codop = 4;
	sem_wait(&esperando_respuesta_memoria);
	send(socket_memoria, &codop, sizeof(uint32_t), 0);

	send(socket_memoria, &(pcb->id_proceso), sizeof(uint32_t), 0);
	send(socket_memoria, &(pcb->tamanio_direcciones), sizeof(uint32_t), 0);

	recv(socket_memoria, &(pcb->tabla_paginas), sizeof(uint32_t), MSG_WAITALL);
	
	printf("Proceso %i # Tabla creada \n ", pcb->id_proceso);

	sem_post(&esperando_respuesta_memoria);
}

void executing_a_exit(t_syscall* syscall){
	sem_post(&grado_multiprogramacion);

	uint32_t codop = 6;
	uint32_t respuesta;

	sem_wait(&esperando_respuesta_memoria);

	send( socket_memoria, &codop, sizeof(uint32_t), 0);
	send( socket_memoria, &(syscall->pcb.id_proceso), sizeof(uint32_t), 0);
//	printf("id proceso = %i \n", syscall->pcb.id_proceso);
	send( socket_memoria, &(syscall->pcb.tabla_paginas), sizeof(uint32_t), 0);
//	printf("ESPERANDO RESPUESTA \n");
	recv( socket_memoria, &respuesta, sizeof(uint32_t), MSG_WAITALL);
	sem_post(&esperando_respuesta_memoria);

	if(respuesta == 100){
		printf("Proceso %i # Memoria liberada \n", syscall->pcb.id_proceso);
	}
	else
	{
		printf("Proceso %i # No se pudo liberar la memoria \n", syscall->pcb.id_proceso);
	}

	uint32_t OK = 10;
	send(syscall->pcb.id_proceso, &OK, sizeof(uint32_t), 0);

	if(es_FIFO()) {
		// FIFO
		sem_post(&fin_de_ejecucion);
	}
	else {
		// SJF
		PCB_EJECUCION.id_proceso = -1;
		sem_post(&procesos_en_ready);
	}
}
