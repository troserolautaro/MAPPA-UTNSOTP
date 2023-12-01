#include"PlanificadorLargo.h"
	pthread_mutex_t mutexMulti;
	int multiprogramacion = 0;
void* planificador_largo(){
	pthread_mutex_init(&mutexMulti,NULL);
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

		pthread_mutex_lock(&mutexMulti);
		multiprogramacion++;
		pthread_mutex_unlock(&mutexMulti);
		i++;
	}
	pthread_mutex_unlock(&mutexGrado);
	sem_post(&planiCorto);
	}while(true);
	return NULL;
}

void planificador_largo_salida(PCB* proceso,char* razon){
	cambiar_estado(proceso,TERMINATED);
	// <SUCCESS / INVALID_RESOURCE / INVALID_WRITE>”
	char *mensaje = string_from_format("Finaliza el proceso %d - Motivo %s",proceso->pid,razon);
	escritura_log(mensaje);
	free(mensaje);

	pthread_mutex_lock(&mutexMulti);
	multiprogramacion--;
	pthread_mutex_unlock(&mutexMulti);

	pthread_mutex_lock(&mutexEjecutando);
	ejecutandoB = false;
	pthread_mutex_unlock(&mutexEjecutando);
	sem_post(&planiLargo);

}
