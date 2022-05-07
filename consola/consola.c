#include "consola.h"



int main(int argc, char** argv){
	if( argc != 3 ) {
		printf("cantidad de parametros incorrecta \n");
		return EXIT_FAILURE;
	}

	char c = '\0';
	char caracter[1];
	char* buffer = string_new();
	char** listaInstrucciones = string_array_new();
	FILE* archivo = fopen(argv[1], "r");


	while ( (c = fgetc(archivo)) != EOF ){
		caracter[0] = c;
		string_append(&buffer, caracter);
	}

	listaInstrucciones = string_split(buffer, "\n");

	int size = string_array_size(listaInstrucciones);
	printf("%d \n", size);
	for(int i = 0; i < size; i++) {
		printf("%s \n", listaInstrucciones[i]);
	}

	free(archivo);
	return EXIT_SUCCESS;
}















