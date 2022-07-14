#include <semaphore.h>
#include <pthread.h>

enum operaciones{
	solicitud_num_tabla_2 ,
	solicitud_num_frame ,
	solicitud_lectura ,
	solicitud_escritura ,
	crear_tablas ,
	suspension_tablas,
	finalizacion_proceso
	};

// cosas compartidas xd fulbo
void* memoria_real;
pthread_t hiloKernel;
sem_t hilo_iniciado; // = 0

void* hilo_kernel(void* ptr_void_socket);
