#include "utils.h"
t_log* logger;
t_instruccion *instruccion_create() {
	t_instruccion *instruccion = malloc(sizeof(t_instruccion));
	instruccion->comando = "0";
	instruccion->parametros= list_create();
	return instruccion;
}
void instruccion_destroy(t_instruccion* instruccion){
	list_destroy(instruccion->parametros);
	free(instruccion->comando);
	free(instruccion);
}
PCB* proceso_create(){
	PCB* proceso = malloc(sizeof(PCB));
	proceso->estado=0;
	proceso->pc=0;
	proceso->pid=0;
	proceso->prioridad=0;
	proceso->registros=registros_create();
	proceso->recursos = list_create();
	proceso->tablaArchivos = dictionary_create();
	proceso->mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(proceso->mutex,NULL);
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
	//dictionary_destroy_and_destroy_elements(proceso->tablaArchivos,destruir_registro_tap);
	pthread_mutex_destroy(proceso->mutex);
	dictionary_destroy(proceso->tablaArchivos);
	list_destroy(proceso->recursos);
	free(proceso->registros);
	free(proceso);
}

PCB* proceso_copy(PCB* origen){
	PCB* destino = proceso_create();
	destino->estado = origen->estado;
	destino->pc = origen->pc;
	destino->pid = origen->pid;
	destino->prioridad = origen->prioridad;
	destino->registros->AX = origen->registros->AX;
	destino->registros->BX = origen->registros->BX;
	destino->registros->CX = origen->registros->CX;
	destino->registros->DX = origen->registros->DX;
	return destino;
}
void proceso_clear(PCB* proceso){ //talvez no necesario lo dejo, por si las ddas

}

void liberar_memoria(void * elemento){
	free(elemento);
}
t_log* iniciar_logger(char* log)
{
	t_log* nuevo_logger =log_create(log,"log",1,LOG_LEVEL_DEBUG);
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
void hilo_funcion(void* parametro,funcion funcion){
	//Un hilo para cuando el procesar mensaje tenga que escribir en consola y no tenga que esperar el uso de la pantalla
	pthread_t hiloLogger;
	pthread_attr_t atributos;
	pthread_attr_init(&atributos);
	pthread_attr_setdetachstate(&atributos,PTHREAD_CREATE_DETACHED);
	int resultado;
	if((resultado=pthread_create(&hiloLogger,&atributos,(void*)funcion,parametro))!=0){
		error_show("Hilo_funcion no creado");
	}
	pthread_attr_destroy(&atributos);
}
void escritura_log(char* mensaje){
	pthread_mutex_lock(&mutexLog);
	log_info(logger,"%s",mensaje);
	pthread_mutex_unlock(&mutexLog);
}
void debug(char* mensaje){
	pthread_mutex_lock(&mutexLog);
	log_debug(logger,"%s",mensaje);
	pthread_mutex_unlock(&mutexLog);
}
