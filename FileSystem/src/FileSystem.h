#ifndef FileSystem_H_
#define FileSystem_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdbool.h>
#include <commons/collections/dictionary.h>


//includes de utils globales
#include<utilsServidor.h>
#include<utilsCliente.h>


typedef struct {
	char* nombreArchivo;
	int tamanioArchivo;
	int bloqueInicial;
}FCB;

void iterator(char* value);
int abrir_archivo(char* archivo);
int chequear_existencia_archivo(FCB* archivo);



#endif /* CPU_H_ */
