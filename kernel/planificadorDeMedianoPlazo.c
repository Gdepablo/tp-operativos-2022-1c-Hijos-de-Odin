#include <stdio.h>
#include "planificadorDeMedianoPlazo.h"

void blocked_a_ready_o_suspended_blocked(){

	inputoutput(pcb, x);
	usleep(tiempoMaximo);
	if(sigueBloqueado(pcb)) {
		blocked_a_suspended_blocked(pcb); // <- poner signal(&grado_multiprogramacion);
	}
	else{
		blocked_a_ready();
	}
}

void* blocked_a_ready() {

	return "";
}

void blocked_a_suspended_blocked(t_pcb* pcb){
	wait(&mx_lista_blocked);
	// ver bien esto XD
	t_pcb* pcb_a_suspender = list_add(lista_blocked);
	signal(&mx_lista_blocked);

	// LOGICA PARA COMUNICARSE CON MEMORIA

	wait(&mx_lista_suspended_blocked);
	list_add(lista_suspended_blocked, pcb_a_suspender);
	signal(&mx_lista_suspended_blocked);

	signal(&grado_multiprogramacion);
}

void* suspended_blocked_a_suspended_ready() {

	return "";
}

void* suspended_ready_a_ready(){
	sem_post(&se_inicio_el_hilo);
	while(1){
		wait(&grado_multiprogramacion); // = GRADO_MULTIPROGRAMACION;

		wait(mx_lista_ready);
		pasarloaready();
		signal(mx_lista_ready);
	}

	return "";
}
