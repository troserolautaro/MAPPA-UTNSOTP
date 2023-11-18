#ifndef Kernel_H_
#define Kernel_H_

#include<pthread.h>
#include"Consola.h"

typedef enum{
	PROCESOEXIT
}desalojo;
void paquete(int);
void terminar_programa();
void bloquear_proceso(PCB* proceso);
void wait_recurso(PCB* proceso, char* recurso);
void signal_recurso(PCB* proceso, char* recurso);
void sleep_proceso(PCB* proceso, int tiempo);

#endif /* CPU_H_ */
