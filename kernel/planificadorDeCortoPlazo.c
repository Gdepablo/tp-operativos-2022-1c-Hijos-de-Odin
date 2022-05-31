#include <stdio.h>
#include "planificadorDeCortoPlazo.h"

void* planificador_corto_plazo_srt(){
	//logica srt

	return "";
}


void* enviar_a_cpu_fifo(){
	while(1) {
		sem_wait(procesosEsperando);
		sem_wait(finDeEjecucion);
		sem_wait(mx_colaReady);
		executing = queue_peek(cola_ready);
		queue_pop(cola_ready);
		sem_post(mx_colaReady);
		sem_post(ejecutar);
	}
	return "";
}


void* hilo_bloqueador(){


	return "";
}
