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

// el hilo de i/o va a ser quien mande a comenzar esto
// esto es del corto plazo, hay que pasarlo alla
void* blocked_a_ready(t_pcb* pcb) {
	/* VERSION 1 (LA FEA)
	if( !strcmp("FIFO", ALGORITMO_PLANIFICACION)){
		// sacar el pcb de lista_blocked y mandarlo a cola_ready
		// buscar id del pcb en la lista de blocked y mandarlo a cola_ready
		// preguntar de list_find para ver si asi se puede evitar todo el lio del pcbaux y el if
		int posicion;
		t_pcb* pcbaux = malloc(sizeof(t_pcb));
		pcbaux->lista_instrucciones = malloc( string_array_size(pcb->lista_instrucciones) );
		int size = list_size(lista_blocked);

		for(int i = 0; i < size; i++){
			posicion = i;
			*pcbaux = list_get(lista_blocked, i);

			if( (pcbaux -> id_proceso) == (pcb -> id_proceso) ){
				break;
			}
		}

		*pcbaux = list_remove(lista_blocked, posicion);
		queue_push(cola_ready, pcbaux);
	}
	else //entra aca si es SRT
	{
		//sacar el pcb de lista_blocked y mandarlo a lista_ready
		int posicion;
		t_pcb* pcbaux = malloc(sizeof(t_pcb));
		pcbaux->lista_instrucciones = malloc( string_array_size(pcb->lista_instrucciones) );
		int size = list_size(lista_blocked);

		for(int i = 0; i < size; i++){
			posicion = i;
			*pcbaux = list_get(lista_blocked, i);

			if( (pcbaux -> id_proceso) == (pcb -> id_proceso) ){
				break;
			}
		}

		*pcbaux = list_remove(lista_blocked, posicion);
		list_add(lista_ready, pcbaux);
	}
	*/

	// ##############################################

	// VERSION 2 (LA MAX POWER)
	int posicion;
	t_pcb* pcbaux = malloc(sizeof(t_pcb));
	pcbaux->lista_instrucciones = malloc( string_array_size(pcb->lista_instrucciones) );
	int size = list_size(lista_blocked);

	for(int i = 0; i < size; i++){
		posicion = i;
		*pcbaux = list_get(lista_blocked, i);

		if( (pcbaux -> id_proceso) == (pcb -> id_proceso) ){
			break;
		}
	}

	*pcbaux = list_remove(lista_blocked, posicion);
	if( !strcmp("FIFO", ALGORITMO_PLANIFICACION) )
	{
		queue_push(cola_ready, pcbaux);
	}
	else
	{
		list_add(lista_ready, pcbaux);
	}

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
