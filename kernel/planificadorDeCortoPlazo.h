/*
 DESCRIPCION:

 PARAMETROS:
*/
void* planificador_corto_plazo_srt(){
	//logica srt

	return "";
}

/*
 DESCRIPCION:

 PARAMETROS:
*/

// FIFO

sem_t procesosEsperando = 0;
sem_t finDeEjecucion = 0;
sem_t mx_colaReady = 1;
sem_t ejecutar = 0;

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

void*

/*
 DESCRIPCION:

 PARAMETROS:
*/
void* hilo_bloqueador(){


	return "";
}
