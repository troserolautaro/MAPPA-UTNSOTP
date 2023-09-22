#ifndef Kernel_H_
#define Kernel_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<readline/history.h>

//includes de utils globales
#include <utils.h>
#include <utilsServidor.h>
#include <utilsCliente.h>




typedef struct{
	int pid;
	int pc;
	//struct *registros;
	//char* tablaArchivos[];

}PCB;
typedef enum consola{
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
	INICIAR_PLANIFICACION,
	DETENER_PLANIFICACION,
	MULTIPROGRAMACION,
	PROCESO_ESTADO
};



t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);

#endif /* CPU_H_ */
