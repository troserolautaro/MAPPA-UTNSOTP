#include"PlanificadorCorto.h"

sem_t clockT,validarQuantum,contexto;
pthread_mutex_t  mutexEjecutando;
bool ejecutandoB = false;
void* clock_rr(PCB* proceso) {
	bool bandera=true;
    while (bandera) { // clockT = 2;
    	usleep(quantum*1000);
        pthread_mutex_lock(&mutexColaCorto);
        bool empty = queue_is_empty(colaCorto);
        pthread_mutex_unlock(&mutexColaCorto);
        if(!empty){
        	pthread_mutex_lock(proceso->mutex);
        	enviar_interrupcion_cpu("quantum",&(proceso->pid));
        	pthread_mutex_unlock(proceso->mutex);
        	bandera=false;
        }

    }
    return NULL;
}

void* planificador_corto(){
	sem_init(&contexto,0,0);
	pthread_mutex_init(&mutexEjecutando,NULL);
	int idPlanificador = planificador_enum();
	do{
		sem_wait(&planiCorto);
		pthread_mutex_lock(&mutexDetenida);
		pthread_mutex_lock(&mutexColaCorto);
		if(!queue_is_empty(colaCorto) && !detenida){
			pthread_mutex_unlock(&mutexDetenida);
			pthread_mutex_unlock(&mutexColaCorto);
			//Se podria mejorar haciendo lo de algoritmoRemplazo();
			switch(idPlanificador){
				case PRIORIDADES: prioridad(); break;
				case ROUNDROBIN:  break;
				case FIFO: break;
				default:error_show("No se reconocio el algoritmo");break;
			}

			//si no es ninguno de los anterior es fifo por que es una cola (estructura de tipo fifo)
			pthread_mutex_lock(&mutexEjecutando);
			if(!ejecutandoB){
				pthread_mutex_unlock(&mutexEjecutando);

				pthread_mutex_lock(&mutexColaCorto);
				PCB* proceso= queue_pop(colaCorto);
				pthread_mutex_unlock(&mutexColaCorto);

				if(proceso->estado!=EXEC){
					cambiar_estado(proceso,EXEC);
				}

				pthread_mutex_lock(&mutexEjecutando);
				ejecutandoB = true;
				pthread_mutex_unlock(&mutexEjecutando);
				//ENVIAR PROCESO
				t_paquete* paquete = crear_paquete();
				agregar_a_paquete(paquete, "proceso", sizeof("proceso"));
				pthread_mutex_lock(proceso->mutex);
				serializar_proceso(paquete,proceso);
				pthread_mutex_unlock(proceso->mutex);

				enviar_paquete(paquete,conexionCPUDispatch);
				eliminar_paquete(paquete);

				if(idPlanificador == ROUNDROBIN){
					hilo_funcion(proceso,(void*)clock_rr);
				}
			}else{
				pthread_mutex_unlock(&mutexEjecutando);
			}

		}else {
			pthread_mutex_unlock(&mutexDetenida);
			pthread_mutex_unlock(&mutexColaCorto);
		}

	}while(true);
	return NULL;
}
bool buscar_proceso_ejecutando(PCB* proceso){
	pthread_mutex_lock(proceso->mutex);
	uint32_t estado = proceso->estado;

	pthread_mutex_unlock(proceso->mutex);
	return (estado == EXEC) ? true : false;
}

bool comparar_prioridad_mayor(void* proceso1,void* proceso2 ){
	//Lo cambie porque sino estabamos leakeando memoria que no podiamos liberar
		return (((PCB*) proceso1)->prioridad < ((PCB*) proceso2)->prioridad) ? true : false;
}
//ALGORITMO DE PRIORIDADES
void prioridad(){
	pthread_mutex_lock(&mutexColaCorto);
	list_sort((colaCorto->elements),comparar_prioridad_mayor);
	pthread_mutex_unlock(&mutexColaCorto);

	pthread_mutex_lock(&mutexEjecutando);
	if(ejecutandoB){
		pthread_mutex_unlock(&mutexEjecutando);

		pthread_mutex_lock(&mutexProcesos);
		PCB* ejecutando = (PCB*)list_find(procesos,(void*)buscar_proceso_ejecutando);
		pthread_mutex_unlock(&mutexProcesos);

		pthread_mutex_lock(&mutexColaCorto);
		PCB* prioritario = (PCB*)queue_peek(colaCorto);
		pthread_mutex_unlock(&mutexColaCorto);
		if(ejecutando != NULL){
			pthread_mutex_lock(prioritario->mutex);
			int pidPrio = prioritario->pid;
			int prioridadPrio = prioritario->prioridad;
			pthread_mutex_unlock(prioritario->mutex);
			pthread_mutex_lock(ejecutando->mutex);
			int pidEjec = ejecutando->pid;
			int prioridadEjec = ejecutando->prioridad;
			pthread_mutex_unlock(ejecutando->mutex);
			if(pidEjec != pidPrio && prioridadEjec > prioridadPrio){
				enviar_interrupcion_cpu("prioridades",&(pidEjec));
				sem_wait(&contexto);
			}

		}

	}else{
		pthread_mutex_unlock(&mutexEjecutando);
	}

}



//ALGORITMO DE ROUND ROBIN
void round_robin(){

}

void enviar_interrupcion_cpu(char* motivo, void* pid){
	t_paquete* paquete = crear_paquete();
	agregar_a_paquete(paquete,"interrupcion",sizeof("interrupcion"));
	agregar_a_paquete(paquete,"desalojo",sizeof("desalojo"));
	agregar_a_paquete(paquete,motivo,strlen(motivo)+1);
	agregar_a_paquete(paquete,pid,sizeof(pid));
	enviar_paquete(paquete,conexionCPUInterrupt);
	eliminar_paquete(paquete);
}
void enviar_interrupcion_cpu_sin_pid(char* motivo){
	t_paquete* paquete = crear_paquete();
	agregar_a_paquete(paquete,"interrupcion",sizeof("interrupcion"));
	agregar_a_paquete(paquete,"desalojo",sizeof("desalojo"));
	agregar_a_paquete(paquete,motivo,strlen(motivo)+1);
	enviar_paquete(paquete,conexionCPUInterrupt);
	eliminar_paquete(paquete);
}


int planificador_enum(){
	if(!strcasecmp(AlgoritmoPlanificacion, "prioridades"))return PRIORIDADES;
	if(!strcasecmp(AlgoritmoPlanificacion, "round_robin"))return ROUNDROBIN;
	if(!strcasecmp(AlgoritmoPlanificacion, "fifo"))return FIFO;
	return -1;
}
