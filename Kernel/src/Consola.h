#ifndef CONSOLA_H_
#define CONSOLA_H_

//includes de utils globales

#include"PlanificadorLargo.h"
#include"PlanificadorCorto.h"


typedef enum{
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
	INICIAR_PLANIFICACION,
	DETENER_PLANIFICACION,
	MULTIPROGRAMACION,
	PROCESO_ESTADO,
	EXIT=-1
}consola;

extern sem_t procesoCargado;
extern pthread_mutex_t mutexGlobal;
char* lectura_consola();
void * manejar_consola( void* args);
int validacion_contenido_consola(char* comando);

//Comandos de consola
void iniciar_planificacion();
void detener_planificacion();
void terminar_programa();
void iniciar_proceso(char* path, int size, int prioridad);
void finalizar_proceso(int pid);
void proceso_estado();

#endif /* CONSOLA_H_ */
