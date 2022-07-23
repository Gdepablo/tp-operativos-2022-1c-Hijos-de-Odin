#include <stdio.h>
#include <semaphore.h>
#include "planificadorDeCortoPlazo.h"
#include "semaforos.h"
#include "kernel.h"

t_log* log_corto_plazo = log_create("./../logs/log_corto_plazo.log", "log kernel", 0, LOG_LEVEL_INFO);


void* ready_a_executing(){
	while(1) {
		sem_wait(&procesos_en_ready);
		if(es_FIFO()) {
			// FIFO
			sem_wait(&fin_de_ejecucion);
			
			sem_wait(&mx_lista_ready);
				t_pcb* pcb_a_enviar = queue_pop(cola_ready);
				enviar_a_CPU(pcb_a_enviar);

				printf("Proceso %i # Enviado a CPU \n", pcb_a_enviar->id_proceso);
				log_info(log_corto_plazo, "Proceso %i # Enviado a CPU \n", pcb_a_enviar->id_proceso);

				free(pcb_a_enviar->instrucciones);
				free(pcb_a_enviar);
			sem_post(&mx_lista_ready);
		} else {
			// SJF
			if( list_size(lista_ready) != 0 ){

				t_pcb* pcb_a_enviar = algoritmo_sjf();
				if( pcb_a_enviar->id_proceso != PCB_EJECUCION.id_proceso) {
					pcb_a_enviar = list_remove_by_condition(lista_ready, sacar_proceso);

					if( (int)PCB_EJECUCION.id_proceso != -1 ){
						printf("### INTERRUPCIÓN ENVIADA ### \n");
						log_info(log_corto_plazo, "### INTERRUPCIÓN ENVIADA ###");

						uint32_t interrupcion = 55;
						send(socket_cpu_interrupcion, &interrupcion, sizeof(uint32_t), 0);
						sem_wait(&procesos_en_ready);
					}

					enviar_a_CPU(pcb_a_enviar);
					printf("Proceso %i # Enviado a CPU \n", pcb_a_enviar->id_proceso);
					log_info(log_corto_plazo, "Proceso %i # Enviado a CPU \n", pcb_a_enviar->id_proceso);

					asignar_pcb_ejecucion(pcb_a_enviar);
					gettimeofday(&HORA_INICIO_EJECUCION, NULL);

					free(pcb_a_enviar->instrucciones);
					free(pcb_a_enviar);
				} else {
					printf("Proceso %i # Permanece en CPU \n", pcb_a_enviar->id_proceso);
					log_info(log_corto_plazo, "Proceso %i # Permanece en CPU \n", pcb_a_enviar->id_proceso);
				}
			}
		}
	}
	return "";
}

void enviar_a_CPU(t_pcb* pcb_a_enviar) {
	t_pcb_buffer* buffer = malloc(sizeof(t_pcb_buffer));

	buffer->size = sizeof(uint32_t) * 5 + strlen(pcb_a_enviar->instrucciones) + 1;
	buffer->size_instrucciones = strlen(pcb_a_enviar->instrucciones) + 1;
	buffer->stream = malloc(buffer->size);

	int offset = 0;
	memcpy(buffer->stream+offset, &(pcb_a_enviar->id_proceso), sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	
	memcpy(buffer->stream+offset, &(pcb_a_enviar->tamanio_direcciones), sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	
	memcpy(buffer->stream+offset, pcb_a_enviar->instrucciones, buffer->size_instrucciones);
	offset+=buffer->size_instrucciones;
	
	memcpy(buffer->stream+offset, &(pcb_a_enviar->program_counter), sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	
	memcpy(buffer->stream+offset, &(pcb_a_enviar->tabla_paginas), sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	
	memcpy(buffer->stream+offset, &(pcb_a_enviar->estimacion_rafagas), sizeof(uint32_t));
	offset=0;
	
	int tamanio_a_enviar = sizeof(uint32_t) * 2 + buffer->size;
	void* a_enviar = malloc( tamanio_a_enviar );

	memcpy(a_enviar+offset, &(buffer->size), sizeof(uint32_t));
	offset+=sizeof(uint32_t);

	memcpy(a_enviar+offset, &(buffer->size_instrucciones), sizeof(uint32_t));
	offset+=sizeof(uint32_t);

	memcpy(a_enviar+offset, buffer->stream, buffer->size);

	send(socket_cpu_dispatch, a_enviar, sizeof(uint32_t) * 7 + strlen(pcb_a_enviar->instrucciones) + 1, 0);
}

bool sacar_proceso(void* pcb_void){
	t_pcb* pcb = pcb_void;

	t_pcb* pcb_minimo = list_get_minimum(lista_ready, comparar_rafagas );

	if( (pcb->id_proceso) == (pcb_minimo->id_proceso) ) {
		return true;
	} else {
		return false;
	}
}

t_pcb* algoritmo_sjf() {
	printf("Ejecutando algoritmo SJF \n");
	log_info(log_corto_plazo, "Ejecutando algoritmo SJF");
	
	t_pcb* pcb_minimo = list_get_minimum(lista_ready, comparar_rafagas );

	struct timeval HORA_ACTUAL;
	gettimeofday(&HORA_ACTUAL, NULL);

    int tiempo_actual_de_ejecucion_microsegundos = (HORA_ACTUAL.tv_sec - HORA_INICIO_EJECUCION.tv_sec) * 1000000 + HORA_ACTUAL.tv_usec - HORA_INICIO_EJECUCION.tv_usec;
    int rafaga_restante_pcb_en_ejecucion =  PCB_EJECUCION.estimacion_rafagas - ( tiempo_actual_de_ejecucion_microsegundos  / 1000 );

    if( (rafaga_restante_pcb_en_ejecucion) > (int)(pcb_minimo->estimacion_rafagas) || (int)(PCB_EJECUCION.id_proceso) == -1 ){
        return pcb_minimo;
    }
    else
    {
        return &PCB_EJECUCION;
    }
}

void* comparar_rafagas(void* pcb1, void* pcb2){
    t_pcb *pcbUno = pcb1;
    t_pcb *pcbDos = pcb2;

    if( pcbUno->estimacion_rafagas <= pcbDos->estimacion_rafagas ){
        return pcbUno;
    } else {
        return pcbDos;
    }
}

void asignar_pcb_ejecucion(t_pcb* pcb){
	PCB_EJECUCION.estimacion_rafagas = pcb->estimacion_rafagas;
	PCB_EJECUCION.id_proceso = pcb->id_proceso;
	PCB_EJECUCION.instrucciones = malloc(strlen(pcb->instrucciones) + 1);
	PCB_EJECUCION.instrucciones = pcb->instrucciones;
	PCB_EJECUCION.program_counter = pcb->program_counter;
	PCB_EJECUCION.tabla_paginas = pcb->tabla_paginas;
	PCB_EJECUCION.tamanio_direcciones = pcb->tamanio_direcciones;
}
