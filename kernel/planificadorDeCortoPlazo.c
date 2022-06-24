#include <stdio.h>
#include <semaphore.h>
#include "planificadorDeCortoPlazo.h"
#include "semaforos.h"
#include "kernel.h"

void* ready_a_executing(){
	sem_post(&se_inicio_el_hilo);
	while(1) {
		sem_wait(&procesos_en_ready);
		if( es_FIFO()) {
			// FIFO
			sem_wait(&fin_de_ejecucion);

			sem_wait(&mx_lista_ready);
			//enviar_a_CPU( queue_pop(&cola_ready) ); // ENVIAR A CPU POR SOCKET
			sem_post(&mx_lista_ready);
		} else {
			// SJF
			t_pcb* pcb_elegido = algoritmo_sjf();
			if(pcb_elegido->id_proceso != PCB_EJECUCION.id_proceso) {
				uint32_t interrupcion = 1000;
				send(socket_cpu_interrupcion, &interrupcion, sizeof(uint32_t), 0);
				// recibir pcb
				// meter pcb en ready
				// mandar nuevo pcb
				asignar_pcb_ejecucion(pcb_elegido);

				gettimeofday(&HORA_INICIO_EJECUCION, NULL);
			}
		}

	}
	return "";
}

/*void enviar_a_CPU(t_pcb pcb) { // TODO POR BATATA
	send(socket_cpu_pcb,0,sizeof(pcb),0); //No estoy seguro si es el socket correcto
	recv(*socket_cpu_pcb,syscall,sizeof(syscall),MSG_WAITALL);
	//Envia fulbo;
}*/

// Se ejecuta en SJF al desalojar un proceso
void executing_a_ready(t_pcb* pcb){ // le falta
	sem_wait(&proceso_nuevo_en_ready);

	send(socket_cpu_interrupcion, 0, sizeof(pcb), 0); //aca toqueteamos tambien
	recv(socket_cpu_pcb, syscall, sizeof(syscall), MSG_WAITALL);
	pcb->estimacion_rafagas = calcular_rafaga(pcb);

	sem_wait(&mx_lista_ready);
		list_add(lista_ready, pcb);
	sem_post(&mx_lista_ready);

}

t_pcb* algoritmo_sjf() {
		t_pcb* pcb_minimo = list_get_minimum(lista_ready, comparar_rafagas );

		struct timeval hora_actual;
		gettimeofday(&hora_actual, NULL);

		long seconds = hora_actual.tv_sec - HORA_INICIO_EJECUCION.tv_sec;
		long milisegundos = (((seconds * 1000000) + hora_actual.tv_usec) - (HORA_INICIO_EJECUCION.tv_usec)) * 1000;
		uint32_t rafaga_restante_pcb_en_ejecucion = PCB_EJECUCION.estimacion_rafagas - milisegundos;

		if(pcb_minimo->estimacion_rafagas < rafaga_restante_pcb_en_ejecucion || PCB_EJECUCION.id_proceso == -1) {
			return pcb_minimo;
		} else {
			return &PCB_EJECUCION;

		}
}

void* comparar_rafagas(void* pcb1, void* pcb2){
    t_pcb *pcbUno = pcb1;
    t_pcb *pcbDos = pcb2;

    if( pcbUno->estimacion_rafagas <= pcbDos->estimacion_rafagas ){
        return pcbUno;
    }
    else
    {
        return pcbDos;
    }
}



void asignar_pcb_ejecucion(t_pcb* pcb){
	PCB_EJECUCION.estimacion_rafagas = pcb->estimacion_rafagas;
	PCB_EJECUCION.id_proceso = pcb->id_proceso;
	PCB_EJECUCION.instrucciones = pcb->instrucciones;
	PCB_EJECUCION.program_counter = pcb->program_counter;
	PCB_EJECUCION.size_instrucciones = pcb->size_instrucciones;
	PCB_EJECUCION.tabla_paginas = pcb->tabla_paginas;
	PCB_EJECUCION.tamanio_direcciones = pcb->tamanio_direcciones;
}
