#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define CREAR_PROCESO 1

int iniciar_servidor(char* ip, char* puerto);
t_config* inicializarConfigs(void);

enum operaciones { IO, EXIT, DESALOJO };

typedef struct {
	uint32_t tamanio_direcciones;
	uint32_t largo_instrucciones;
	char* instrucciones;
} t_info_proceso;

typedef struct {
	uint32_t size; // t_pcb + strlen(lista_instrucciones) + uint32_t * 2
	uint32_t size_instrucciones;
	void* stream;
} t_pcb_buffer;

typedef struct {
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct {
	uint32_t codOp;
	t_buffer* buffer;
} t_paquete;

// PCB
typedef struct {
	uint32_t id_proceso;
	uint32_t tamanio_direcciones;
	uint32_t size_instrucciones;
	char*    instrucciones;
	uint32_t program_counter;
	uint32_t tabla_paginas;
	uint32_t estimacion_rafagas; // para el SRT
} t_pcb;

typedef struct {
	t_pcb pcb;
	uint32_t instruccion; // 0==IO, 1==EXIT, 2==DESALOJO
	uint32_t tiempo_de_bloqueo; // para IO
} t_syscall;

typedef struct {
	t_pcb* pcb;
	uint32_t tiempo_de_bloqueo;
} t_bloqueado;


// VARIABLES
struct timeval HORA_INICIO_EJECUCION, HORA_FIN_EJECUCION;
t_list*  lista_ready; // SRT
t_pcb PCB_EJECUCION;

// SOCKETS

int socket_consola_proceso;
int socket_cpu_syscall;
int socket_cpu_dispatch;
int socket_cpu_interrupcion;
int socket_memoria;
// COLAS Y LISTAS
t_queue* cola_new;
t_queue* cola_ready; // FIFO

t_queue*  cola_blocked;
t_queue*  cola_suspended_blocked;
t_queue*  cola_suspended_ready;


// HILOS
pthread_t lp_new_ready_fifo, lp_new_ready_srt, lp_exec_exit; // HILOS LARGO PLAZO
pthread_t mp_suspendedready_ready; //HILOS MEDIANO PLAZO
pthread_t cp_ready_exec_fifo, cp_ready_exec_srt, cp_sacar_exec; //HILOS CORTO PLAZO
pthread_t atender_consolas;
pthread_t recibir_syscall_cpu;
pthread_t hiloIO;

// VARIABLES PARA LOS SOCKETS
char* IP_CONSOLA;
char* IP_KERNEL;
char* IP_CPU;
char* IP_MEMORIA;

char* PUERTO_CONSOLA_PROCESO;
char* PUERTO_MEMORIA;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* PUERTO_CPU_SYSCALL;

char* ALGORITMO_PLANIFICACION;
int ESTIMACION_INICIAL;
float ALFA;
int GRADO_MULTIPROGRAMACION;
int TIEMPO_MAXIMO_BLOQUEADO;


// FUNCIONES
t_info_proceso* deserializar_proceso(t_buffer* buffer);
void* atender_cliente(int* socket_cliente);
void* planificador_largo_plazo();
int crear_conexion(char *ip, char* puerto);
t_syscall* recibirSyscall();
void* recibir_procesos();
void* esperar_syscall();

int es_FIFO();

#endif
