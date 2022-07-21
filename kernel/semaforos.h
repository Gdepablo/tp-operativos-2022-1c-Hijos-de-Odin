#include <semaphore.h>

// MUTEX
sem_t mx_cola_new; // = 1
sem_t mx_lista_ready; // = 1
sem_t mx_cola_blocked; // = 1
sem_t mx_cola_suspended_blocked; // = 1
sem_t mx_cola_suspended_ready; // = 1
sem_t mx_suspension; // = 1

// CONTADORES
sem_t procesos_para_ready; // = 0
sem_t procesos_en_ready; // = 0
sem_t grado_multiprogramacion; // = GRADO_MULTIPROGRAMACION DEL .CONFIG
sem_t io_terminada;
sem_t procesos_en_suspended_ready;
sem_t proceso_nuevo_en_ready;
sem_t pcb_recibido;
// SINCRONIZADORES
sem_t proceso_finalizado; // = 0
sem_t fin_de_ejecucion; // = 1
sem_t se_inicio_el_hilo; // = 0
sem_t proceso_en_io; // = 0
sem_t se_inicio_suspensor;
sem_t suspendiendo;
sem_t esperando_respuesta_memoria; // = 1
