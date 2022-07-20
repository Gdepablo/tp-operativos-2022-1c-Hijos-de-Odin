#include <stdio.h>
#include "planificadorDeMedianoPlazo.h"
#include "semaforos.h"

// SE EJECUTA AL RECIBIR UNA SYSCALL
//void* executing_a_blocked_o_exit() {
//	sem_post(&se_inicio_el_hilo);
//	gettimeofday(&HORA_FIN_EJECUCION, NULL);
//	t_syscall* syscall = malloc(sizeof(t_syscall));
//	//syscall->pcb.instrucciones = malloc(sizeof(t_pcb)); Esto aca no iba
//
//	while(1) {
//		recv(socket_cpu_dispatch, syscall, sizeof(syscall), MSG_WAITALL); // ver sizeof
//		sem_post(&fin_de_ejecucion);
//		if(syscall->instruccion) {
//			// EXIT
//			executing_a_exit(); // largo plazo
//		} else {
//			// IO
//			executing_a_blocked(syscall); // mediano plazo
//		}
//		sem_post(&fin_de_ejecucion);
//	}
//	return "";
//}

void executing_a_blocked(t_syscall* syscall) {
	printf("EXECUTING A BLOCKED \n");
	t_pcb* pcb_nuevo = malloc(sizeof(t_pcb));
	// ID PROCESO
	pcb_nuevo->id_proceso = syscall->pcb.id_proceso;
	// TAMANIO INSTRUCCIONES
	pcb_nuevo->tamanio_direcciones = syscall->pcb.tamanio_direcciones;
	// SIZE INSTRUCCIONES
	pcb_nuevo->size_instrucciones = syscall->pcb.size_instrucciones;
	// INSTRUCCIONES
	pcb_nuevo->instrucciones = malloc(strlen(syscall->pcb.instrucciones));
	strcpy(pcb_nuevo->instrucciones, syscall->pcb.instrucciones);
	// PROGRAM COUNTER
	pcb_nuevo->program_counter = syscall->pcb.program_counter;
	// TABLA PAGINAS
	pcb_nuevo->tabla_paginas = syscall->pcb.tabla_paginas;
	// ESTIMACION RAFAGA
	pcb_nuevo->estimacion_rafagas = syscall->pcb.estimacion_rafagas;

	printf("PCB NUEVO CREADO, INSTRUCCIONES: \n%s \n", pcb_nuevo->instrucciones);

	t_bloqueado* proceso_bloqueado = malloc(sizeof(t_bloqueado));

	proceso_bloqueado->pcb = pcb_nuevo;
	proceso_bloqueado->tiempo_de_bloqueo = syscall->tiempo_de_bloqueo;

	printf("TIEMPO DE BLOQUEO = %i \n", proceso_bloqueado->tiempo_de_bloqueo);

	if( !es_FIFO() ){
		// ACTUALIZA LA ESTIMACION? XD
		pcb_nuevo->estimacion_rafagas = calcular_rafaga(&syscall->pcb);
	}

	queue_push(cola_blocked, proceso_bloqueado);
	printf("PROCESO PASADO DE EXECUTING A BLOCKED: %i \n", pcb_nuevo->id_proceso);

	// SE AVISA QUE SE LIBERO EL CPU
	if( es_FIFO() ){
		sem_post(&fin_de_ejecucion); // para que fifo meta el proximo
	}
	else
	{
		sem_post(&procesos_en_ready);
	}

	sem_post(&proceso_en_io);
}

//void executing_a_blocked(t_syscall* syscall) {
//	t_bloqueado proceso;
//
//	proceso.pcb = syscall->pcb;
//
//	if( !es_FIFO() ){
//		// ACTUALIZA LA ESTIMACION? XD
//		proceso.pcb.estimacion_rafagas = calcular_rafaga(&syscall->pcb);
//	}
//
//	proceso.tiempo_de_bloqueo = syscall->tiempo_de_bloqueo;
//
//	pthread_create(&proceso.hilo_suspensor, NULL, blocked_a_suspended_blocked, &proceso);
//	proceso.esta_suspendido = 0;
//
//	sem_wait(&mx_cola_blocked);
//		queue_push(cola_blocked, &proceso);
//	sem_post(&mx_cola_blocked);
//
//	pthread_detach(proceso.hilo_suspensor);
//	sem_post(&proceso_en_io);
//}

// HACE LAS IO A MEDIDA QUE INGRESAN
void* hilo_io(){
	sem_post(&se_inicio_el_hilo);
	t_bloqueado* proceso;
	while(1){
		sem_wait(&proceso_en_io);
		proceso = queue_pop(cola_blocked);

		if( TIEMPO_MAXIMO_BLOQUEADO < proceso->tiempo_de_bloqueo ){
			printf("EL TIEMPO MAXIMO BLOQUEADO ES MENOR \n");
			usleep( TIEMPO_MAXIMO_BLOQUEADO * 1000);

			// avisar a memoria que pase a SWAP TODO POR BATATA ANASHEX

			sem_wait(&mx_cola_suspended_ready);
				queue_push(cola_suspended_ready, proceso->pcb);
			sem_post(&mx_cola_suspended_ready);
			sem_post(&grado_multiprogramacion);

			printf("PROCESO PASADO A SUSPENDED READY DESDE BLOCKED: %i  \n", proceso->pcb->id_proceso);



			usleep( ( proceso->tiempo_de_bloqueo - TIEMPO_MAXIMO_BLOQUEADO ) * 1000 );

			sem_post(&procesos_para_ready);
		}
		else
		{
			printf("EL TIEMPO DE ESPERA MAXIMO ES MAYOR \n");
			usleep( proceso->tiempo_de_bloqueo );

			if( es_FIFO() ){
				sem_wait(&mx_lista_ready);
				queue_push(cola_ready, proceso->pcb);
				sem_post(&mx_lista_ready);
				printf("PROCESO DEVUELTO A LA COLA READY ANASHE ID %i (FIFO) \n", proceso->pcb->id_proceso);
			}
			else
			{
				sem_wait(&mx_lista_ready);
				list_add(lista_ready, proceso->pcb);
				sem_post(&mx_lista_ready);
				printf("PROCESO DEVUELTO A LA LISTA READY ANASHE (SFJ) \n");
			}

			sem_post(&procesos_en_ready);
		}



		free(proceso);
	}
	return "";
}

//void* blocked_y_suspended_a_suspended_ready_o_ready() {
//	while(1) {
//		sem_wait(&io_terminada);
//		t_bloqueado* proceso = queue_pop(cola_blocked);
//		if(proceso->esta_suspendido) {

//			suspended_blocked_a_suspended_ready(&proceso->pcb);
//		} else {
//			blocked_a_ready(&proceso->pcb);
//		}
//	}
//	return "";
//}

void suspended_blocked_a_suspended_ready(t_pcb* pcb) {
	sem_wait(&mx_cola_suspended_ready);
		queue_push(cola_suspended_ready, pcb);
	sem_post(&mx_cola_suspended_ready);

	sem_post(&procesos_en_suspended_ready);
}

//void blocked_a_ready(t_pcb* pcb){
//	sem_wait(&mx_lista_ready);
//		if(es_FIFO()) {
//			queue_push(cola_ready, pcb);
//		} else {
//			list_add(lista_ready, pcb);
//		}
//	sem_post(&mx_lista_ready);
//
//	if(!es_FIFO()) {
//		sem_post(&proceso_nuevo_en_ready);
//	}
//}

// hay que sincronizar que suspended ready tenga mayor prioridad que de new a ready
//void* suspended_ready_a_ready() {
//	sem_post(&se_inicio_el_hilo);
//	while(1) {
//		sem_wait(&procesos_en_suspended_ready);
//		sem_wait(&grado_multiprogramacion);
//
//		if(es_FIFO()) {
//
//			queue_push(cola_ready, queue_pop(cola_suspended_ready));
//		} else {
//			list_add(lista_ready, queue_pop(cola_suspended_ready));
//		}
//
//		if(!es_FIFO()) {
//			sem_post(&proceso_nuevo_en_ready);
//		}
//
//	}
//
//	return "";
//}


uint32_t calcular_rafaga(t_pcb* pcb){
	uint32_t rafaga;

	long seconds = HORA_FIN_EJECUCION.tv_sec - HORA_INICIO_EJECUCION.tv_sec;
	long milisegundos = (((seconds * 1000000) + HORA_FIN_EJECUCION.tv_usec) - (HORA_INICIO_EJECUCION.tv_usec)) * 1000;

	rafaga = ALFA * (uint32_t)milisegundos + (1.0-ALFA) * pcb->estimacion_rafagas;

	return rafaga;
}














