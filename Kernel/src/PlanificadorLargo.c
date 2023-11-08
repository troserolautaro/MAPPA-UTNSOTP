#include"PlanificadorLargo.h"

void* planificador_largo(){
	do{
	sem_wait(&planiLargo);
	while(/*gradoMultiprogramacion>queue_size(colaCorto) CAMBIAR*/ !detenida && !queue_is_empty(colaLargo)){
		pthread_mutex_lock(&mutexColaLargo);
		PCB* proceso=(PCB*)queue_pop(colaLargo);
		pthread_mutex_unlock(&mutexColaLargo);
		proceso->estado=READY;

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

void planificador_largo_salida(PCB** proceso){
	/*TALVEZ HABRIA QUE LIBERAR EL PROCESO(?) PERO COMO YO LO ENTIENDO AL REFERENCIAR EL PUNTERO CAMBIA DONDE ESTA APUNTANDO. */
	PCB* temp=*proceso;
	temp->estado=TERMINATED;
	// <SUCCESS / INVALID_RESOURCE / INVALID_WRITE>”
	char *mensaje = string_from_format("Finaliza el proceso %d",temp->pid);
	log_info(logger,"%s",mensaje);
	free(temp);
	sem_post(&planiLargo);
	//planificador_largo();
}
