
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

extern uint32_t PIDGLOBAL ;
extern bool detenida,ejecutandoB;
extern t_queue* colaLargo;
extern t_queue* colaCorto;
extern t_list* procesos;
extern int conexionCPUDispatch, conexionCPUInterrupt,conexionMemoria,conexionFileSystem,pidGlobal;
extern int gradoMultiprogramacion, quantum;
extern char* AlgoritmoPlanificacion;
extern pthread_mutex_t  mutexColaCorto,mutexColaLargo,mutexProcesos,mutexGrado, mutexEjecutando, mutexRecursos,mutexDetenida;
extern sem_t planiLargo,planiCorto;
extern t_dictionary *diccionarioRecursos;
char* estado_enum(uint32_t estado);
void  cambiar_estado(PCB* proceso, int estado);
void push_colaCorto(PCB* proceso);
void planificador_largo_salida(PCB* proceso, char* razon);
void iterar_lista(char** mensaje);
void bloquear_proceso(PCB* proceso,char* motivo);

#endif /* GENERALES_H_ */
