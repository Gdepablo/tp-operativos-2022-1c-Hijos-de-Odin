#ifndef CPU_H_
#define CPU_H_

// LIBRARIES
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <commons/config.h>
#include "funcionesSocketsyConfig.h"
#include <commons/collections/list.h>

// ENUM
enum operaciones{ NO_OP, IO, READ, WRITE, COPY, EXIT };

// STRUCTS
typedef struct {
	uint32_t id_proceso;
	uint32_t tamanio_direcciones;
	char* lista_instrucciones;
	uint32_t program_counter;
	uint32_t tabla_paginas;
	uint32_t estimacion_rafagas; // para el SRT
} t_pcb;

typedef struct {
	t_pcb pcb;
	uint32_t instruccion; // 0==IO, 1==EXIT
	uint32_t tiempo_de_bloqueo; // para IO
} t_syscall;

typedef struct{
	uint32_t tamanio_paginas;
	uint32_t entradas_por_tabla;
} info_traduccion_t; 	// Necesario para hacer la traduccion de logica a real

typedef struct{
	uint32_t pagina;
	uint32_t marco;
}t_tlb;

typedef struct {
	uint32_t size;
	uint32_t size_instrucciones;
	void* stream;
} t_pcb_buffer;


// VAR GLOBALES
uint32_t retardo_noop;
uint32_t tamanio_de_pagina;
t_list* tlbs;
uint32_t numero_pagina;
int socket_memoria;
t_pcb pcb_ejecutando;
info_traduccion_t info_traduccion;


// FUNCIONES (mantener el orden o hay tabla)
// PROPIAS DEL CPU
void* executer();
void* interrupt();
void crear_TLB();
void limpiar_TLB();
void reemplazar_TLB();
void guardar_en_TLB(uint32_t numero_de_pagina, uint32_t numero_de_frame);
int hay_interrupcion();
t_config* inicializarConfigs(void);
int se_hizo_una_syscall_bloqueante();
uint32_t fetchOperand(uint32_t dir_logica);
int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char* ip, char* puerto);
int seleccionarOperacion(char* nombre_instruccion);
void crear_TLB();
void limpiar_TLB();
t_tlb elegir_pagina_a_reemplazar_TLB();
void guardar_en_TLB(uint32_t numero_de_pagina, uint32_t numero_de_frame);

// INSTRUCCIONES
void instr_exit();
void instr_no_op(int cant_de_no_op);
void instr_read(uint32_t dir_logica);
void instr_io(int tiempo_en_milisegundos);
void instr_write(uint32_t dir_logica, uint32_t valor);
void instr_copy(uint32_t dir_logica_destino, uint32_t valor);

// COMUNICACION CON MEMORIA
uint32_t pedir_todo_memoria();
uint32_t buscar_frame(uint32_t dir_logica);
uint32_t pedir_num_tabla_2(uint32_t entrada_1er_tabla);
uint32_t pedir_contenido_frame(uint32_t numero_de_frame);
void escribir_frame(uint32_t numero_de_frame, uint32_t valor);
uint32_t pedir_num_frame(uint32_t entrada_2da_tabla, uint32_t num_tabla_2);

// COMUNICACION CON KERNEL
void enviar_PCB();
t_pcb recibir_pcb(int socket_dispatch);
void enviar_syscall(t_syscall* syscall_a_enviar);

// MISCELLANEOUS
bool encontrar_pagina(void* tlb);

#endif








