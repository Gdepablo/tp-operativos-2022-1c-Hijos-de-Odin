/*
 * cpu.c
 *
 *  Created on: 7 may. 2022
 *      Author: utnso
*/
#include <stdio.h>
#include <commons/string.h>


int main(void){
	printf("CPU");
	/*
	while( c != EOF ) {
		while ( ((c = fgetc(archivo)) != '\n') ){ //se carga una linea en el buffer
			if(c != EOF)
			{
				char caracter[1];
				caracter[0] = c;
				string_append(&buffer, caracter);
			}
			else
			{
				break;
			}
		}

		pushearInstruccion(buffer, &instrucciones);
		printf("buffer: %s \n", buffer);
		printf("%p \n", buffer);
		free(buffer);
		buffer = string_new();
	} */

	return 0;
}

/*
void pushearInstruccion(char* instruccion, char*** listaInstrucciones){
	char* aux = malloc(1024);
	aux = string_new();
	aux = string_duplicate(instruccion);

	char* identificador = string_new();
	identificador = strtok(aux, " ");

	if( !strcmp("NO_OP", identificador) )
	{
		int parametro = atoi((strtok(NULL, " ")));
		for( int i = 0; i < parametro; i++) {
			string_array_push(listaInstrucciones, "NO_OP");
		}
	}
	else
	{
		string_array_push(listaInstrucciones, instruccion);
	}

	free(aux);

	return;
} */

