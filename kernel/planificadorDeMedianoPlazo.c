#include <stdio.h>
#include "planificadorDeMedianoPlazo.h"
#include "semaforos.h"

t_log* log_mediano_plazo;
t_log* log_io;

void* hilo_suspensor(void* bloqueado_void){
    sem_post(&se_inicio_suspensor);
    t_bloqueado* proceso_bloqueado = bloqueado_void;
    uint32_t confirmacion = 0;
    uint32_t codop = 5;

    usleep(TIEMPO_MAXIMO_BLOQUEADO * 1000);

    sem_wait(&suspendiendo);
    sem_wait(&esperando_respuesta_memoria);
    	proceso_bloqueado->esta_suspendido = 1;

		printf("Proceso %i # Se suspende bloqueado \n", proceso_bloqueado->pcb->id_proceso);
		log_info(log_mediano_plazo, "Proceso %i # Se suspende bloqueado", proceso_bloqueado->pcb->id_proceso);

    	send(socket_memoria, &codop, sizeof(uint32_t), 0);
    	send(socket_memoria, &(proceso_bloqueado->pcb->id_proceso), sizeof(uint32_t), 0);
    	send(socket_memoria, &(proceso_bloqueado->pcb->tabla_paginas), sizeof(uint32_t), 0);

    	recv(socket_memoria, &(confirmacion), sizeof(uint32_t), MSG_WAITALL);
    sem_post(&esperando_respuesta_memoria);
    sem_post(&suspendiendo);

    return "";
}

void executing_a_blocked(t_syscall* syscall) {
	t_pcb* pcb_nuevo = malloc(sizeof(t_pcb));

	pcb_nuevo->id_proceso = syscall->pcb.id_proceso;
	pcb_nuevo->tamanio_direcciones = syscall->pcb.tamanio_direcciones;
	pcb_nuevo->instrucciones = malloc(strlen(syscall->pcb.instrucciones) + 1);
	strcpy(pcb_nuevo->instrucciones, syscall->pcb.instrucciones);
	pcb_nuevo->program_counter = syscall->pcb.program_counter;
	pcb_nuevo->tabla_paginas = syscall->pcb.tabla_paginas;
	pcb_nuevo->estimacion_rafagas = syscall->pcb.estimacion_rafagas;

	t_bloqueado* proceso_bloqueado = malloc(sizeof(t_bloqueado));

	proceso_bloqueado->pcb = pcb_nuevo;
	proceso_bloqueado->tiempo_de_bloqueo = syscall->tiempo_de_bloqueo;
	proceso_bloqueado->esta_suspendido = 0;

	queue_push(cola_blocked, proceso_bloqueado);

	printf("Proceso %i # Se bloquea \n", proceso_bloqueado->pcb->id_proceso);
	log_info(log_mediano_plazo, "Proceso %i # Se bloquea", proceso_bloqueado->pcb->id_proceso);

	pthread_create(&(proceso_bloqueado->suspensor), NULL, hilo_suspensor, (void*)proceso_bloqueado);
	pthread_detach(proceso_bloqueado->suspensor);
	sem_wait(&se_inicio_suspensor);

	sem_post(&proceso_en_io);

	if( es_FIFO() ){
		sem_post(&fin_de_ejecucion);
	}
	else
	{
		PCB_EJECUCION.id_proceso = -1;
		sem_post(&procesos_en_ready);
	}
}

void* hilo_io(){
    sem_post(&se_inicio_el_hilo);
	log_mediano_plazo = log_create("./../logs/log_mediano_plazo.log", "log kernel", 0, LOG_LEVEL_INFO);
    log_io = log_create("./../logs/log_io.log", "log kernel", 0, LOG_LEVEL_INFO);
    t_bloqueado* bloqueado;
    while(1){
        sem_wait(&proceso_en_io);

        bloqueado = queue_pop(cola_blocked);
		printf("Proceso %i # Ejecutando I/O \n", bloqueado->pcb->id_proceso);
		log_info(log_io, "Proceso %i # Ejecutando I/O", bloqueado->pcb->id_proceso);
        usleep(bloqueado->tiempo_de_bloqueo * 1000);
        printf("Proceso %i # Finaliza I/O \n", bloqueado->pcb->id_proceso);
		log_info(log_io, "Proceso %i # Finaliza I/O", bloqueado->pcb->id_proceso);
        sem_wait(&suspendiendo);
            if(bloqueado->esta_suspendido == 1) {
                sem_wait(&mx_cola_suspended_ready);
                	queue_push(cola_suspended_ready, bloqueado->pcb);
                sem_post(&mx_cola_suspended_ready);

				printf("Proceso %i # Ingreso a suspendido ready \n", bloqueado->pcb->id_proceso);
				log_info(log_mediano_plazo, "Proceso %i # Ingreso a suspendido ready", bloqueado->pcb->id_proceso);

                sem_post(&grado_multiprogramacion);
                sem_post(&procesos_para_ready);
            }
            else {
                pthread_cancel(bloqueado->suspensor);
                if(es_FIFO()){
    				sem_wait(&mx_lista_ready);
    					queue_push(cola_ready, bloqueado->pcb);
    				sem_post(&mx_lista_ready);
                }
                else {
    				sem_wait(&mx_lista_ready);
    					list_add(lista_ready, bloqueado->pcb);
    				sem_post(&mx_lista_ready);
                }
				
				printf("Proceso %i # Ingreso a ready \n", bloqueado->pcb->id_proceso);
				log_info(log_mediano_plazo, "Proceso %i # Ingreso a ready", bloqueado->pcb->id_proceso);


                sem_post(&procesos_en_ready);
            }
        sem_post(&suspendiendo);
    }
}

uint32_t calcular_rafaga(t_pcb* pcb){
	uint32_t rafaga;

	int tiempo_actual_de_ejecucion_microsegundos = (HORA_FIN_EJECUCION.tv_sec - HORA_INICIO_EJECUCION.tv_sec) * 1000000 + HORA_FIN_EJECUCION.tv_usec - HORA_INICIO_EJECUCION.tv_usec;

	rafaga = ALFA * (uint32_t)(tiempo_actual_de_ejecucion_microsegundos / 1000) + (1.0-ALFA) * pcb->estimacion_rafagas;

	return rafaga;
}
