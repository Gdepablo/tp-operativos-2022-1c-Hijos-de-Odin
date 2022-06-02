#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/string.h>
#include <commons/config.h>


t_config* inicializarConfigs(void);
int crear_conexion(char *ip, char* puerto);
