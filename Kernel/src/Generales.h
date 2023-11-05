
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

extern int PIDGLOBAL ;
extern bool detenida;
extern t_queue* colaLargo;
extern t_queue* colaCorto;
extern t_list* procesos;
extern t_log* logger;
extern t_config* config;
extern int conexionCPUDispatch, conexionCPUInterrupt,conexionMemoria,conexionFileSystem,pidGlobal;
extern int gradoMultiprogramacion, quantum;
extern char* AlgoritmoPlanificacion;
extern pthread_mutex_t  mutexPlaniLargo,mutexPlaniCorto,mutexColaCorto,mutexColaLargo;
#endif /* GENERALES_H_ */
