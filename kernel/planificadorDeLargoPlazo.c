#include <stdio.h>
#include <semaphore.h>
#include "planificadorDeLargoPlazo.h"

/*

*/
void* planificador_largo_plazo_new_a_ready(int GRADO_MULTIPROGRAMACION){
	//espera activa hasta que lleguen los primeros 4 procesos

	while(1){
		wait(&semaforo_inicializador); // = 0;
		wait(&grado_multiprogramacion); // = GRADO_MULTIPROGRAMACION;

		//pasar un proceso de new a ready
		wait(mx_colaReady);
		pasarloaready();
		signal(mx_colaReady);
	}

	return "";
}

void* planificador_mediano_plazo_suspended_a_ready(){
	//espera activa hasta que lleguen los primeros 4 procesos


	while(1){
		wait(&grado_multiprogramacion); // = GRADO_MULTIPROGRAMACION;

		//pasar un proceso de new a ready
		wait(mx_colaReady);
		pasarloaready();
		signal(mx_colaReady);
	}

	return "";
}

void* planificador_largo_plazo_finalizador(){
	while(1){
		wait(finalizar);

		free(proceso_ejecutandose);

		wait(unmutex);
		procesos_en_memoria--;
		signal(unmutex);
	}

	return "";
}
