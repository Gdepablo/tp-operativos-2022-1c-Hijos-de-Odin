#include "com_kernel.h"
extern t_list* tabla_de_paginas_de_primer_nivel;
extern int socket_kernel;
void* esperar_kernel() {
	uint32_t codigo;
	while(1) {
		/** Se supone que espera init, suspender o finalizar*/
		recv(socket_kernel,&codigo,sizeof(uint32_t),MSG_WAITALL); // esperar por socket llamada de kernel
		// llamar init, suspender o finalizar según corresponda
	}
	return "";
}

void init() { // Ni idea qué pinta esto
	t_list tabla_de_primer_nivel = list_create();
	list_add(tabla_de_paginas_de_primer_nivel, tabla_de_primer_nivel);
	//seguir
}

void suspender() {

}

void finalizar() {

}
