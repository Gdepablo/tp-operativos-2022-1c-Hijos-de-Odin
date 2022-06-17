#include "com_kernel.h"

void* esperar_kernel() {
	while(1) {
		recv() // esperar por socket llamada de kernel
		// llamar init, suspender o finalizar según corresponda
	}
	return "";
}

void init() {
	t_list tabla_de_primer_nivel = list_create();
	list_add(tabla_de_tablas_de_primer_nivel, tabla_de_primer_nivel);
	//seguir
}

void suspender() {

}

void finalizar() {

}
