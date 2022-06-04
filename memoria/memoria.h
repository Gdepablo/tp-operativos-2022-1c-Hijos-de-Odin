#include <time.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <commons/config.h>



t_config* inicializarConfigs(void);
int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char* ip, char* puerto);
