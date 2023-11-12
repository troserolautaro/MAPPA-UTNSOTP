#include"PlanificadorLargo.h"
void* planificador_largo(){
	int multiprogramacion = 0;
	do{
	sem_wait(&planiLargo);
	pthread_mutex_lock(&mutexColaLargo);
	bool vacio = queue_is_empty(colaLargo);
	int size = queue_size(colaLargo);
	pthread_mutex_unlock(&mutexColaLargo);
	int i = 0;
	pthread_mutex_lock(&mutexGrado);
	while(!vacio && i<size && !detenida && multiprogramacion<gradoMultiprogramacion){

		pthread_mutex_lock(&mutexColaLargo);
		PCB* proceso=(PCB*)queue_pop(colaLargo);
		pthread_mutex_unlock(&mutexColaLargo);

		cambiar_estado(proceso, READY);

		pthread_mutex_lock(&mutexColaCorto);
		queue_push(colaCorto,proceso);
		pthread_mutex_unlock(&mutexColaCorto);
		multiprogramacion++;
		i++;
	}
	pthread_mutex_unlock(&mutexGrado);
	sem_post(&planiCorto);
	}while(true);
	return NULL;
}

void planificador_largo_salida(PCB* proceso){
	cambiar_estado(proceso,TERMINATED);
	// <SUCCESS / INVALID_RESOURCE / INVALID_WRITE>”
	char *mensaje = string_from_format("Finaliza el proceso %d",proceso->pid);
	escritura_log(mensaje);
	free(mensaje);
	sem_post(&planiLargo);

}
