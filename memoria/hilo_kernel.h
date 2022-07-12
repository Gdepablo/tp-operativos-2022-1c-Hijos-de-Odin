// cosas compartidas xd fulbo
#include <semaphore.h>
#include <pthread.h>


pthread_t hiloKernel;
sem_t hilo_iniciado; // = 0

void* hilo_kernel(void* ptr_void_socket);
