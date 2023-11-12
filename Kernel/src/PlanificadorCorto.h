#ifndef PLANIFICADORCORTO_H_
#define PLANIFICADORCORTO_H_
#include"Generales.h"
typedef enum{
	FIFO,
	PRIORIDADES,
	ROUNDROBIN,

}planificacion;
void* planificador_corto();
int planificador_enum();
void fifo();
void prioridad(); //Multicola sin salto entre ellas
bool comparar_prioridad_mayor(void* proceso1,void* proceso2);
void round_robin();

#endif /* PLANIFICADORCORTO_H_ */
