#include"PlanificadorCorto.h"

/*DECIDI HACERLO UNA COLA PORQUE CREI QUE ERA LO MEJOR */
void* planificador_corto(){
	do{
		sem_wait(&planiCorto);
		pthread_mutex_lock(&mutexColaCorto);
		if(!queue_is_empty(colaCorto)){
			int idPlanificador = planificador_enum();
			switch(idPlanificador){
				case PRIORIDADES: prioridad(); break;
				case ROUNDROBIN: round_robin(); break;
				case FIFO: break;
				default:printf("No se reconocio el algoritmo");break;
			}
			//si no es ninguno de los anterior es fifo por que es una cola (estructura de tipo fifo)
			PCB* proceso= queue_pop(colaCorto);
			pthread_mutex_unlock(&mutexColaCorto);
			cambiar_estado(proceso,EXEC);
			//ENVIAR PROCESO
			t_paquete* paquete = crear_paquete();
			agregar_a_paquete(paquete, "proceso", sizeof("proceso"));
			serializar_proceso(paquete,proceso);
			enviar_paquete(paquete,conexionCPUDispatch);
			eliminar_paquete(paquete);
		}else {
			pthread_mutex_unlock(&mutexColaCorto);
		}

	}while(true);
	return NULL;
		/*PRUEBA DE COMO HACER EL PLANIFICADOR */


}

void prioridad(){
	list_sort((colaCorto->elements),comparar_prioridad_mayor);
}

bool comparar_prioridad_mayor(void* proceso1,void* proceso2 ){
		PCB* PCB1 =  (PCB*) proceso1;
		PCB* PCB2 =  (PCB*) proceso2;
		if(PCB1->prioridad < PCB2->prioridad)return true;
		return false;
}

void round_robin(){
//si el proceso (proceso con estado en EXEC) en ejecucion completo el quantum, cambia el estado, envia interrupccion a cpu y lo manda al final de la cola
//si termino no hace nada.
//si se bloqueo por io lo manda al final de la cola
}

int planificador_enum(){
	if(!strcasecmp(AlgoritmoPlanificacion, "prioridades\0"))return PRIORIDADES;
	if(!strcasecmp(AlgoritmoPlanificacion, "round robin\0"))return ROUNDROBIN;
	if(!strcasecmp(AlgoritmoPlanificacion, "fifo\0"))return FIFO;
	return -1;
}
