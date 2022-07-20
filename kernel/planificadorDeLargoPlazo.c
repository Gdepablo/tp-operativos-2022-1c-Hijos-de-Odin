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

	sem_post(&procesos_para_ready);
}

// 	THREADS

void* pasar_a_ready(){
	sem_post(&se_inicio_el_hilo);
	while(1){
		sem_wait(&procesos_para_ready);
		sem_wait(&grado_multiprogramacion);

		if( queue_size(cola_suspended_ready) == 0 ){ // => agarro la queue NEW
			sem_wait(&mx_lista_ready);
			sem_wait(&mx_cola_new);
			if(es_FIFO()) {
				// FIFO
				t_pcb* pcb = queue_pop(cola_new);
				crear_tabla_proceso(pcb);
				queue_push(cola_ready, pcb);

			} else {
				// SJF
				t_pcb* pcb = queue_pop(cola_new);
				crear_tabla_proceso(pcb);
				list_add(lista_ready, queue_pop(cola_new));

			}
			sem_post(&mx_cola_new);
			sem_post(&mx_lista_ready);
		}
		else // ==> aAgarro la queue SUSPENDED READY
		{
			if( es_FIFO() ){
				t_pcb* pcb = queue_pop(cola_suspended_ready);
				// NO HACE FALTA CREAR ME PARECE
				queue_push(cola_ready, pcb);
			} else {
				t_pcb* pcb = queue_pop(cola_suspended_ready);
				// NO HACE FALTA CREAR XD
				list_add(lista_ready, pcb);
			}
		}

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

	uint32_t codop = 6;
	uint32_t respuesta;

	send( socket_memoria, &codop, sizeof(uint32_t), 0);
	// PROCESS ID
	send( socket_memoria, &(una_syscall->pcb.id_proceso), sizeof(uint32_t), 0);
	printf("id proceso = %i \n", una_syscall->pcb.id_proceso);
	// NUMERO DE TABLA 1ER NIVEL
	send( socket_memoria, &(una_syscall->pcb.tabla_paginas), sizeof(uint32_t), 0);
	// RESPUESTA
	printf("ESPERANDO RESPUESTA \n");
	recv( socket_memoria, &respuesta, sizeof(uint32_t), MSG_WAITALL);

	if(respuesta == 100){
		printf("Se libero la memoria del proceso %i \n", una_syscall->pcb.id_proceso);
	}
	else
	{
		printf("No se pudo liberar la memoria del proceso %i \n", una_syscall->pcb.id_proceso);
	}

	// se envia el OK a la consola anashe
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
