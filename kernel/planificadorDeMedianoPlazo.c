#include <stdio.h>
#include "planificadorDeMedianoPlazo.h"

// SE EJECUTA AL RECIBIR UNA SYSCALL
void* executing_a_blocked_o_exit() {
	sem_post(&se_inicio_el_hilo);
	t_syscall syscall; // hay que hacer malloc?

	while(1) {
		recv(*socket_cpu_pcb, syscall, sizeof(syscall), MSG_WAITALL); // ver sizeof
		sem_post(&fin_de_ejecucion);
		if(syscall.instruccion) {
			// EXIT
			executing_a_exit(); // largo plazo
		} else {
			// IO
			executing_a_blocked(syscall); // mediano plazo
		}
		sem_post(&fin_de_ejecucion);
	}
	return "";
}

void executing_a_blocked(t_syscall syscall) {
	t_bloqueado proceso;
	proceso.pcb = syscall.pcb;
	proceso.tiempo_de_bloqueo = syscall.tiempo_de_bloqueo;
	pthread_create(&proceso.hilo_suspensor, NULL, (void)blocked_a_suspended_blocked, &proceso);
	proceso.esta_suspendido = 0;

	sem_wait(&mx_cola_blocked);
		queue_push(&cola_blocked, proceso);
	sem_post(&mx_cola_blocked);

	pthread_detach(proceso.hilo_suspensor);
	sem_post(&proceso_en_io);
}

// HILO QUE SUSPENDE CUANDO ESPERA MÁS DEL TIEMPO MÁXIMO
void* blocked_a_suspended_blocked(t_bloqueado* proceso){
	usleep(TIEMPO_MAXIMO_BLOQUEADO);
	proceso->esta_suspendido = 1;

	// LOGICA PARA GUARDAR PCB EN MEMORIA

	signal(&grado_multiprogramacion);

	return "";
}

void* blocked_y_suspended_a_suspended_ready_o_ready() {
	while(1) {
		sem_wait(&io_terminada);
		t_bloqueado proceso;
		proceso = queue_pop(&cola_blocked);
		if(proceso.esta_suspendido) {
			suspended_blocked_a_suspended_ready(proceso.pcb);
		} else {
			blocked_a_ready(proceso.pcb);
		}
	}
	return "";
}

void suspended_blocked_a_suspended_ready(t_pcb pcb) {
	sem_wait(&mx_cola_suspended_ready);
		queue_push(&cola_suspended_ready, pcb);
	sem_post(&mx_cola_suspended_ready);

	sem_post(&procesos_en_suspended_ready);
}

void blocked_a_ready(t_pcb pcb) {
	sem_wait(&mx_lista_ready);
		if(algoritmo()) {
			queue_push(&cola_ready, &pcb);
		} else {
			list_add(&lista_ready, &pcb);
		}
	sem_post(&mx_lista_ready);

	if(!algoritmo()) {
		sem_post(&proceso_nuevo_en_ready);
	}
}

void* inputOuput() {
	t_bloqueado proceso;

	while(1) {
		sem_wait(&proceso_en_io);

		proceso = queue_peek(&cola_blocked);
		usleep(proceso.tiempo_de_bloqueo);
		pthread_cancel(proceso.hilo_suspensor);

		sem_post(&io_terminada);
	}
	return "";
}

// hay que sincronizar que suspended ready tenga mayor prioridad que de new a ready
void* suspended_ready_a_ready() {
	while(1) {
		sem_wait(&procesos_en_suspended_ready);
		sem_wait(&grado_multiprogramacion);

		if(algoritmo()) {
			queue_push(&cola_ready, queue_pop(&cola_suspended_ready));
		} else {
			list_add(&lista_ready, queue_pop(&cola_suspended_ready));
		}

		if(!algoritmo()) {
			sem_post(&proceso_nuevo_en_ready);
		}

	}

	return "";
}

















