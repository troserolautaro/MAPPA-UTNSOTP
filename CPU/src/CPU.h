#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils.h>

//includes de utils globales
#include <utils.h>
#include <utilsServidor.h>
#include <utilsCliente.h>
#include <pthread.h>

int serverDispatch,serverInterrupt;
int clienteKernel;
void * manejar_cliente(void*);
t_config* iniciar_config(void);
t_log* iniciar_logger(void);
void iterator(char* value);

void set(uint32_t registro, int valor);
void sum(uint32_t registroDestino,registroOrigen);
void sub(uint32_t registroDestino,uint32_t registroOrigen);
void exit();
uint32_t* obtener_registro(char* componente,registros_CPU* registro);


#endif /* CPU_H_ */

