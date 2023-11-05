#include"PlanificadorLargo.h"

void* planificador_largo(){
	do{
	pthread_mutex_lock(&mutexPlaniLargo);
	pthread_mutex_lock(&mutexColaLargo);
	while(/*gradoMultiprogramacion>queue_size(colaCorto) CAMBIAR*/ !detenida && !queue_is_empty(colaLargo)){
		PCB* proceso=(PCB*)queue_pop(colaLargo);
		pthread_mutex_unlock(&mutexColaLargo);
		proceso->estado=READY;

		pthread_mutex_lock(&mutexColaCorto);
		queue_push(colaCorto,proceso);
		pthread_mutex_unlock(&mutexColaCorto);
	}
	pthread_mutex_unlock(&mutexPlaniCorto);
	}while(true);
	return NULL;
	//semaforo a corto
	//planificador_corto();
}

void planificador_largo_salida(PCB** proceso){
	/*TALVEZ HABRIA QUE LIBERAR EL PROCESO(?) PERO COMO YO LO ENTIENDO AL REFERENCIAR EL PUNTERO CAMBIA DONDE ESTA APUNTANDO. */
	PCB* temp=*proceso;
	temp->estado=TERMINATED;
	//semaforo de gradoMultiprogramacion
	free(temp);
	free(proceso);
	pthread_mutex_unlock(&mutexPlaniLargo);
	//planificador_largo();
}
