#include"PlanificadorLargo.h"

void* planificador_largo(){
	do{
	sem_wait(&planiLargo);
	while(/*gradoMultiprogramacion>queue_size(colaCorto) CAMBIAR*/ !detenida && !queue_is_empty(colaLargo)){
		pthread_mutex_lock(&mutexColaLargo);
		PCB* proceso=(PCB*)queue_pop(colaLargo);
		pthread_mutex_unlock(&mutexColaLargo);
		cambiar_estado(proceso, READY);

		pthread_mutex_lock(&mutexColaCorto);
		queue_push(colaCorto,proceso);
		pthread_mutex_unlock(&mutexColaCorto);
	}
	sem_post(&planiCorto);
	}while(true);
	return NULL;
	//semaforo a corto
	//planificador_corto();
}

void planificador_largo_salida(PCB* proceso){
	cambiar_estado(proceso,TERMINATED);
	// <SUCCESS / INVALID_RESOURCE / INVALID_WRITE>”
	char *mensaje = string_from_format("Finaliza el proceso %d",proceso->pid);
	log_info(logger,"%s",mensaje);
	sem_post(&planiLargo);
	//planificador_largo();
}
