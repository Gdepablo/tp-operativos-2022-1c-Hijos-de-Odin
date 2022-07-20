#include "kernel.h"
/*
 TIPO:

 DESCRIPCION:

 PARAMETROS:

 CUANDO SE EJECUTA:
*/
void* suspended_ready_a_ready();

/*
 TIPO:

 DESCRIPCION:

 PARAMETROS: NINGUNO

 CUANDO SE EJECUTA:
*/
void blocked_a_ready_o_suspended_blocked();

/*
 TIPO:

 DESCRIPCION:

 PARAMETROS: NINGUNO

 CUANDO SE EJECUTA:
*/
void blocked_a_ready(t_pcb* pcb);

/*
 TIPO:

 DESCRIPCION:

 PARAMETROS: PCB (del proceso a suspender)

 CUANDO SE EJECUTA:
*/
void* blocked_a_suspended_blocked(void* proceso);
void* hilo_io();

/*
 TIPO:

 DESCRIPCION:

 PARAMETROS:

 CUANDO SE EJECUTA:
*/
void suspended_blocked_a_suspended_ready(t_pcb* pcb);

/*
 TIPO:

 DESCRIPCION:

 PARAMETROS:

 CUANDO SE EJECUTA:
*/
void* suspended_ready_a_ready();

/*
 TIPO:

 DESCRIPCION:

 PARAMETROS:

 CUANDO SE EJECUTA:
*/

void executing_a_blocked(t_syscall* syscall);
uint32_t calcular_rafaga(t_pcb* pcb);
