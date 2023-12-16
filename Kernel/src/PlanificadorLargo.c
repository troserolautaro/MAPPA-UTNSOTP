#include"PlanificadorLargo.h"
	pthread_mutex_t mutexMulti;
	int multiprogramacion = 0;
	sem_t procesoTerminado, sem_paginasLiberadas;
void* planificador_largo(){
	pthread_mutex_init(&mutexMulti,NULL);
	sem_init(&procesoTerminado,0,0);
	sem_init(&sem_paginasLiberadas,0,0);
	pthread_mutex_init(&mutexDetenida,NULL);
	do{
	sem_wait(&planiLargo);
	pthread_mutex_lock(&mutexColaLargo);
	bool vacio = queue_is_empty(colaLargo);
	int size = queue_size(colaLargo);
	pthread_mutex_unlock(&mutexColaLargo);
	int i = 0;
	pthread_mutex_lock(&mutexGrado);
	pthread_mutex_lock(&mutexDetenida);
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
	pthread_mutex_unlock(&mutexDetenida);
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
			pthread_mutex_lock(temp->mutex);
			list_add(temp->recursos,recurso);
			pthread_mutex_unlock(temp->mutex);
			sem_post(&planiCorto);
		}else{
			*instancias +=1;
		}
		i++;
	}

}
void deteccion_deadlock(PCB* proceso){
	pthread_mutex_lock(proceso->mutex);
	escritura_log("Analisis de deteccion de Deadlocks");
	char* mensajeDeadlock = string_new();
	void iterar_recursos(char* temp) {string_append_with_format(&mensajeDeadlock,"%s ",temp);}
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
						string_append_with_format(&mensajeDeadlock,"Deadlock detectado: %d - Recursos en posesion: ", temp->pid);
						list_iterate(temp->recursos,(void*)iterar_recursos);
						t_list* keys = dictionary_keys(temp->tablaArchivos);
						if(!list_is_empty(keys)){
							list_iterate(keys,(void*) iterar_recursos);
						}
						list_clean(keys);
						list_destroy(keys);
						string_append_with_format(&mensajeDeadlock,"Recurso Solicitado: %s \n",recurso);
						bandera = true;
					}else if(!list_is_empty(temp->recursos)){
						pthread_mutex_lock(temp->mutex);
						PCB* encontrado = list_find(temp->recursos,(void*)verificacion_DL);
						if(encontrado!=NULL){
							string_append_with_format(&mensajeDeadlock,"Deadlock detectado: %d - Recursos en posesion: ", temp->pid);
							list_iterate(temp->recursos,(void*)iterar_recursos);
							t_list* keys = dictionary_keys(temp->tablaArchivos);
							if(!list_is_empty(keys)){
								list_iterate(keys,(void*) iterar_recursos);
							}
							list_clean(keys);
							list_destroy(keys);
							string_append_with_format(&mensajeDeadlock,"Recurso Solicitado: %s \n",recurso);
							bandera=true;
						}
						pthread_mutex_unlock(temp->mutex);
					}

					i++;
				}
			}
			return bandera;
		}
		PCB* encontrado = list_find(proceso->recursos,(void*)verificacion_DL);
		if(encontrado!=NULL){
			escritura_log(mensajeDeadlock);

		}

	}
	free(mensajeDeadlock);
	pthread_mutex_unlock(proceso->mutex);

}


void liberar_archivos(PCB* proceso){
	//Liberar lock FCLOSE() ya lo hace, y liberar el dictionary
	if(!dictionary_is_empty(proceso->tablaArchivos)){
		t_list* lista = dictionary_keys(proceso->tablaArchivos);
		for(int i = 0; i<list_size(lista);i++){
			char* tempChar = string_new();
			tempChar = string_duplicate((char*)list_get(lista,i));
			f_close(proceso,tempChar);
			free(tempChar);
		}
		list_destroy(lista);
		dictionary_clean(proceso->tablaArchivos);
		dictionary_destroy(proceso->tablaArchivos);
	}

}
void procesoFinalizado(){
	sem_post(&sem_paginasLiberadas);
}
void liberar_paginas(PCB* proceso){
	t_paquete* paquete = crear_paquete();
	agregar_a_paquete(paquete,"finalizar_proceso",sizeof("finalizar_proceso"));
	agregar_a_paquete(paquete,&(proceso->pid),sizeof(uint32_t));
	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);
	sem_wait(&sem_paginasLiberadas);
}

void planificador_largo_salida(PCB* proceso,char* razon){
	bool bloqueado = false;
	bool estadoAntiguo = proceso->estado;
	if(proceso->estado == BLOCKED) bloqueado = true;
	if(proceso->estado == EXEC && !strcasecmp(razon,"SIGKILL")) {
		t_paquete* paquete = crear_paquete();
		agregar_a_paquete(paquete,"interrupcion",sizeof("interrupcion"));
		agregar_a_paquete(paquete,"desalojo",sizeof("desalojo"));
		agregar_a_paquete(paquete,"TERMINATED",sizeof("TERMINATED"));
		pthread_mutex_lock(proceso->mutex);
		agregar_a_paquete(paquete,&(proceso->pid),sizeof(uint32_t));
		pthread_mutex_unlock(proceso->mutex);

		enviar_paquete(paquete,conexionCPUInterrupt);
		eliminar_paquete(paquete);
		sem_wait(&procesoTerminado);
	}
	pthread_mutex_lock(&mutexColaCorto);
	list_remove_element(colaCorto->elements,proceso);
	pthread_mutex_unlock(&mutexColaCorto);

	pthread_mutex_lock(&mutexColaLargo);
	list_remove_element(colaLargo->elements,proceso);
	pthread_mutex_unlock(&mutexColaLargo);

	pthread_mutex_lock(&mutexRecursos);
	liberar_recursos(proceso);
	pthread_mutex_unlock(&mutexRecursos);
	liberar_archivos(proceso);
	liberar_paginas(proceso);


	char *mensaje = string_from_format("Finaliza el proceso %d - Motivo %s",proceso->pid,razon);
	escritura_log(mensaje);
	free(mensaje);


	cambiar_estado(proceso,TERMINATED);


	// <SUCCESS / INVALID_RESOURCE / INVALID_WRITE>”


	if(bloqueado){
		deteccion_deadlock(proceso);
	}

	if(estadoAntiguo!=NEW){
		pthread_mutex_lock(&mutexMulti);
		multiprogramacion--;
		pthread_mutex_unlock(&mutexMulti);

		sem_post(&planiLargo);
	}

}
void proceso_terminado(){
	sem_post(&procesoTerminado);
}
