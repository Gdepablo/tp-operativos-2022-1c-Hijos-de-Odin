#include <netdb.h>
#include "main_memoria.h"
#include "variables_globales.h"


uint32_t buscar_tabla_2do_nivel(uint32_t numero_tabla_1er_nivel, uint32_t numero_de_entrada);
uint32_t leer_de_memoria(uint32_t numero_de_frame, uint32_t offset);
void escribir_en_memoria(uint32_t numero_de_frame,uint32_t offset,uint32_t valor_a_escribir);
pagina_t* buscar_pagina(uint32_t numero_tabla_2do_nivel_leer, uint32_t numero_de_entrada);
void cargar_a_memoria(uint32_t numero_tabla_1er_nivel, pagina_t* pagina_a_cargar_a_memoria, uint32_t num_pagina, uint32_t process_id);
uint32_t calcular_num_pagina(uint32_t numero_tabla_1er_nivel_leer,uint32_t numero_tabla_2do_nivel_leer, uint32_t numero_de_entrada);
bool esta_libre(void* void_bitmap);
bool memoria_no_llena();
int buscar_frame_libre();
t_list* buscar_paginas_presentes(int (*tabla_primer_nivel)[]);
void* copiar_de_swap(uint32_t pagina, uint32_t process_id);
bool comparador(void* puntero_void_1, void* puntero_void_2);
pagina_t* clock_comun(uint32_t numero_de_tabla, t_list* paginas_en_memoria);
int calcular_numero_de_pagina_a_reemplazar(pagina_t* pagina_a_reemplazar, uint32_t numero_primer_tabla);
void cargar_pagina_a_memoria(uint32_t numero_de_pagina, uint32_t numero_de_frame, uint32_t process_id);
void bit_de_uso_en_1(uint32_t numero_tabla_1er_nivel_leer, uint32_t numero_de_entrada, uint32_t num_frame);
void bit_de_modificado_en_1(uint32_t numero_tabla_1er_nivel_leer, uint32_t numero_de_entrada, uint32_t num_frame);
pagina_t* clock_mejorado(uint32_t numero_de_tabla, t_list* paginas_en_memoria);
pagina_t* elegir_pagina_a_sacar(uint32_t numero_primer_tabla);

