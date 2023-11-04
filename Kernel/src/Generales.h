
#ifndef GENERALES_H_
#define GENERALES_H_
#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<readline/history.h>

//includes de utils globales
#include <utilsServidor.h>
#include <utilsCliente.h>
int PIDGLOBAL = 0;
bool detenida=true;
t_queue* colaCorto;
t_list* procesos;
t_log* logger;
t_config* config;
int conexionCPUDispatch, conexionCPUInterrupt,conexionMemoria,conexionFileSystem,pidGlobal;

#endif /* GENERALES_H_ */
