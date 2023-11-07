#ifndef Memoria_H_
#define Memoria_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>
//includes de utils globales
#include <utilsServidor.h>
#include <utilsCliente.h>

void iterator(char* value);
t_list* cargar_instrucciones(char** file);

#endif /* CPU_H_ */
