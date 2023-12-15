#ifndef PLANIFICADORLARGO_H_
#define PLANIFICADORLARGO_H_

#include"Generales.h"
#include"KernelMemoria.h"
void* planificador_largo();
void procesoFinalizado();
void deteccion_deadlock(PCB* proceso);
void proceso_terminado();
#endif /* PLANIFICADORLARGO_H_ */
