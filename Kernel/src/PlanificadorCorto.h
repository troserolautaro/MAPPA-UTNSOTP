#ifndef PLANIFICADORCORTO_H_
#define PLANIFICADORCORTO_H_
#include"Generales.h"
typedef enum{
	FIFO,
	PRIORIDADES,
	ROUNDROBIN,

}planificacion;

char* AlgoritmoPlanificacion, *recursos,* instanciasRecursos;

int planificador_enum();
void fifo();
void prioridad(); //Multicola sin salto entre ellas
bool ordenar_prioridades();
void round_robin();

#endif /* PLANIFICADORCORTO_H_ */
