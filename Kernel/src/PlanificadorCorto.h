#ifndef PLANIFICADORCORTO_H_
#define PLANIFICADORCORTO_H_
#include"Generales.h"

extern sem_t contexto,clockT;
typedef enum{
	FIFO,
	PRIORIDADES,
	ROUNDROBIN,

}planificacion;
void* planificador_corto();
int planificador_enum();
void fifo();
void prioridad(); //Multicola sin salto entre ellas
void round_robin();
void enviar_interrupcion_cpu(char* motivo);
bool buscar_proceso_ejecutando(void* proceso);
#endif /* PLANIFICADORCORTO_H_ */
