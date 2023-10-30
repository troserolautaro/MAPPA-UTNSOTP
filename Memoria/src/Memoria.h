#ifndef Memoria_H_
#define Memoria_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>
//includes de utils globales
#include <utils.h>
#include <utilsServidor.h>
#include <utilsCliente.h>

void iterator(char* value);
t_log* iniciar_logger(void);
t_config* iniciar_config(void);
t_list* cargar_instrucciones(char* file);

#endif /* CPU_H_ */
