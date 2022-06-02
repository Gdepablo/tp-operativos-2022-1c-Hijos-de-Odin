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
void* blocked_a_ready();

/*
 TIPO:

 DESCRIPCION:

 PARAMETROS: PCB (del proceso a suspender)

 CUANDO SE EJECUTA:
*/
void blocked_a_suspended_blocked(t_pcb* pcb);

/*
 TIPO:

 DESCRIPCION:

 PARAMETROS:

 CUANDO SE EJECUTA:
*/
void* suspended_blocked_a_suspended_ready();

/*
 TIPO:

 DESCRIPCION:

 PARAMETROS:

 CUANDO SE EJECUTA:
*/
void* suspended_ready_a_ready();
