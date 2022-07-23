#include "kernel.h"


t_pcb* algoritmo_sjf();

void* comparar_rafagas(void* pcb1, void* pcb2);
void asignar_pcb_ejecucion(t_pcb* pcb);
extern uint32_t calcular_rafaga(t_pcb* pcb);
void enviar_a_CPU(t_pcb* pcb);
bool sacar_proceso(void* pcb_void);




