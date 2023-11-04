#ifndef PLANIFICADORLARGO_H_
#define PLANIFICADORLARGO_H_

#include"Generales.h"
int gradoMultiprogramacion, quantum;

t_queue* colaLargo;
void planificador_largo();
void planificador_largo_salida();

#endif /* PLANIFICADORLARGO_H_ */
