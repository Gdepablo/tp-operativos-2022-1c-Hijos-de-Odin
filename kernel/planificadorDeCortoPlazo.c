#include <stdio.h>
#include "planificadorDeCortoPlazo.h"

// Las versiones FIFO y SRT de las funciones pueden llegar a ser iguales salvo que usan colas o listas según corresponda
// enviando funciones como parámetro se podrían ahorrar algunos duplicados

t_list* lista_exec;

void* ready_a_executing(){
	sem_post(&se_inicio_el_hilo);
	while(1) {
		sem_wait(&procesos_en_ready);
		sem_wait(&fin_de_ejecucion);

		sem_wait(&mx_lista_ready);
		if(algoritmo()) {
			enviar_a_CPU(queue_pop(&cola_ready)); // ENVIAR A CPU POR SOCKET
		} else {
			// algotirmo SJF
		}
		sem_post(&mx_lista_ready);
	}
	return "";
}

void enviar_a_CPU(t_pcb pcb) {
	/*send(socket_cpu_pcb,0,sizeof(pcb),0); //No estoy seguro si es el socket correcto
	recv(*socket_cpu_pcb,syscall,sizeof(syscall),MSG_WAITALL);*/
}

void* desalojar() {
	while(1) {
		if(algoritmo()) {
			// FIFO
		} else {

		}
	}
	return "";
}

// Se ejecuta en SJF al desalojar un proceso
void executing_a_ready(t_pcb pcb){ // le falta
	sem_wait(&proceso_nuevo_en_ready);

	send(socket_cpu_interrupcion, 0, sizeof(pcb), 0); //aca toqueteamos tambien
	recv(*socket_cpu_pcb, syscall, sizeof(syscall), MSG_WAITALL);

	sem_wait(&mx_lista_ready);
		list_add(&lista_ready, pcb);
	sem_post(&mx_lista_ready);

}

void* algoritmo_SJF(t_pcb pcb) {
		t_pcb pcb_en_ejecucion;
		uint32_t rafagas_ejecutadas;
		while(1) {
		sem_wait(&proceso_nuevo_en_ready);
		pcb_en_ejecucion = pcb;
		//Habria que sacar de la lista al pcb en exec de la lista de ready
		if(list_get_minimum(lista_ready,pcb.estimacion_rafagas()) != pcb_en_ejecucion)
		{ t_pcb prox_pcb_a_ejecutar;
		prox_pcb_a_ejecutar = list_get_minimum(lista_ready,pcb.estimacion_rafagas());
		esperar_syscall(); // espera el interrupt del SO
		pcb_en_ejecucion.estimacion_rafagas() -= rafagas_ejecutadas;
		list_add(lista_ready,pcb_en_ejecucion); //esto no es add, es remove.
		list_add(lista_exec,pcb_en_ejecucion);
		//Add to lista_exec
		elemento = prox_pcb_a_ejecutar;
		//Tenemos que sacar un elemento pero no sabemos cómo hacer.
		list_remove_by_condition(&lista_ready,elemento.id_proceso() == prox_pcb_a_ejecutar.id_proceso());
		gestionarTransicion(); //Esto creo q lo hace el PMP
		//No sé si va, pero es el tema de si está bloqueado x tiempo etc.
		} else
		mandar_a_CPU();}} // ni idea como implementarlo

