#include "utils.h"

t_instruccion *instruccion_create() {
	t_instruccion *instruccion = malloc(sizeof(t_instruccion));
	instruccion->comando = "0";
	instruccion->parametros= NULL;
	return instruccion;
}

PCB* proceso_create(){
	PCB* proceso = malloc(sizeof(PCB));
	proceso->estado=0;
	proceso->pc=0;
	proceso->pid=0;
	proceso->prioridad=0;
	proceso->registros=registros_create();
	return proceso;
}

registros_CPU* registros_create(){
	registros_CPU* registros=malloc(sizeof(registros_CPU));
	registros->AX=0;
	registros->BX=0;
	registros->CX=0;
	registros->DX=0;
	return registros;
}
void proceso_destroy(PCB* proceso){
	free(proceso->registros);
	free(proceso);
}
void instruccion_destroy(t_instruccion* instruccion){
	list_destroy(instruccion->parametros);
	free(instruccion->comando);
	free(instruccion);
}
t_log* iniciar_logger(char* log)
{
	t_log* nuevo_logger =log_create(log,"log",1,LOG_LEVEL_INFO);
	if(nuevo_logger == NULL){
			perror("No se ha encontrado el logger\n");
		}
	return nuevo_logger;
}

t_config* iniciar_config(char* config)
{
	t_config* nuevo_config = config_create(config);
	if(nuevo_config == NULL){
		perror("No se ha encontrado el config\n");
	}
	return nuevo_config;
}

