#ifndef FileSystem_H_
#define FileSystem_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdbool.h>
#include <commons/collections/dictionary.h>


//includes de utils globales
#include<utilsServidor.h>
#include<utilsCliente.h>

void iterator(char* value);
uint32_t abrir_archivo(char* archivo);
bool existencia_archivo(char* archivo);
void iniciar_bloques();
void iniciar_fat();

#endif /* CPU_H_ */
