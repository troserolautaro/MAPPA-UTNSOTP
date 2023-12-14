#ifndef Kernel_H_
#define Kernel_H_

#include<pthread.h>
#include"Consola.h"

typedef enum{
	PROCESOEXIT,
	WAIT=4,
	SIGNAL=5,
	SLEEP=6,
	DESALOJO_SIGNAL=7,
	PAGE_FAULT = 8,
	F_OPEN = 9,
	F_CLOSE = 10,
	F_WRITE = 11,
	F_READ = 12,
	F_TRUNCATE = 13,
	F_SEEK = 14,
	SIGK = 15
}desalojo;
void paquete(int);
void terminar_programa();
void sleep_proceso(void*);
//MANEJO DE RECURSOS
void wait_recurso(PCB* proceso, char* recurso);
void signal_recurso(PCB* proceso, char* recurso);


sem_t desalojo_signal;//clave archivo, valor archivo_t
//clave proceso, valor diccionario de archivos abiertos por el proceso




#endif /* Kernel_H_ */
