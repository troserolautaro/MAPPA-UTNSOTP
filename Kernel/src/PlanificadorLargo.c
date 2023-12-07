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

		push_colaCorto(proceso);

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

void liberar_recursos(PCB* proceso){

	void remover_espera(char* key, t_list* elements){
		t_queue* colaEspera = (t_queue*)list_get(elements,1);
		if(!queue_is_empty(colaEspera)){
			list_remove_element(colaEspera->elements,proceso);
		}
	}
	dictionary_iterator(diccionarioRecursos,(void*)remover_espera);

	int i =0;
	while(!list_is_empty(proceso->recursos)){
		char* recurso = (char*)list_remove(proceso->recursos,i);

		t_list* elements = (t_list*)dictionary_get(diccionarioRecursos,recurso);
		int* instancias = (int*)list_get(elements,0);
		t_queue* colaEspera = (t_queue*)list_get(elements,1);

		if(!queue_is_empty(colaEspera)){
			PCB* temp = (PCB*) queue_pop(colaEspera);
			push_colaCorto(temp);
			list_add(temp->recursos,recurso);
			sem_post(&planiCorto);
		}else{
			*instancias +=1;
		}
		i++;
	}

}
void deteccion_deadlock(PCB* proceso){
	escritura_log("Analisis de deteccion de Deadlocks");
	if(!list_is_empty(proceso->recursos)){
		bool verificacion_DL(char* recurso){
		t_list* elements = (t_list*)dictionary_get(diccionarioRecursos,recurso);
		t_queue* colaEspera = (t_queue*)list_get(elements,1);
		bool bandera = false;
		if(!queue_is_empty(colaEspera)){
			int i = 0;
			while(!bandera && i<queue_size(colaEspera)){
				PCB* temp = list_get(colaEspera->elements,i);
				if(temp->pid == proceso->pid){
					bandera = true;
				}else if(!list_is_empty(temp->recursos)){
					PCB* encontrado = list_find(temp->recursos,(void*)verificacion_DL);
					if(encontrado!=NULL){
						bandera=true;
					}
				}

				i++;
			}
		}
			return bandera;
		}
		PCB* encontrado = list_find(proceso->recursos,(void*)verificacion_DL);
		if(encontrado!=NULL){
			debug("Hay Deadlock");
		}

	}
}
void planificador_largo_salida(PCB* proceso,char* razon){
	if(proceso->estado == BLOCKED) deteccion_deadlock(proceso);
	cambiar_estado(proceso,TERMINATED);
	liberar_recursos(proceso);
	// <SUCCESS / INVALID_RESOURCE / INVALID_WRITE>”
	char *mensaje = string_from_format("Finaliza el proceso %d - Motivo %s",proceso->pid,razon);
	escritura_log(mensaje);
	free(mensaje);

	pthread_mutex_lock(&mutexMulti);
	multiprogramacion--;
	pthread_mutex_unlock(&mutexMulti);

	sem_post(&planiLargo);

}
