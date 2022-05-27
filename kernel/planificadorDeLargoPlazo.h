/*
 DESCRIPCION:

 PARAMETROS:
*/
void* planificador_largo_plazo_inicializador(int GRADO_MULTIPROGRAMACION){

	sem = GRADO_MULTIPROGRAMACION;

	//espera activa hasta que lleguen los primeros 4 procesos
	while(1){
		wait(sem)
		if( procesos_en_memoria <= GRADO_MULTIPROGRAMACION) {
			if( !queue_is_empty(cola_new) ){
				//pasar un proceso de new a ready
				wait(unmutex);
				procesos_en_memoria++;
				signal(unmutex);
			}
			else{
				signal(sem)
			}
		}
	}

	return "";
}

/*
 DESCRIPCION:

 PARAMETROS:
*/
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

/*
 DESCRIPCION:

 PARAMETROS:
*/
void* planificador_corto_plazo_fifo(){
	//logica fifo

	return "";
}
