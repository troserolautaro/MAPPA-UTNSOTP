#include"PlanificadorCorto.h"

/*DECIDI HACERLO UNA COLA PORQUE CREI QUE ERA LO MEJOR */
void* planificador_corto(){
	do{
		pthread_mutex_lock(&mutexPlaniCorto);
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
			proceso->estado=EXEC;
			//ENVIAR PROCESO
			t_paquete* paquete = crear_paquete();
			agregar_a_paquete(paquete, "proceso", sizeof(char*)*8);
			agregar_a_paquete(paquete,proceso,sizeof(PCB*));
		}
		return NULL;
	}while(true);
		/*PRUEBA DE COMO HACER EL PLANIFICADOR */


}

void prioridad(){
	ordenar_prioridades(&colaCorto->elements);
	//Horrible lo que hay que hacer aca
}

void round_robin(){
//si el proceso (proceso con estado en EXEC) en ejecucion completo el quantum, cambia el estado, envia interrupccion a cpu y lo manda al final de la cola
//si termino no hace nada.
//si se bloqueo por io lo manda al final de la cola
}

bool ordenar_prioridades(t_list** lista ){
	t_list_iterator* i = list_iterator_create(*lista);
	while (i->index	< list_size(*lista)){
		PCB* actual =  (PCB*) i->actual;
		PCB* siguiente = (!(i->next != NULL)) ?   (PCB*)i->next : NULL;
		if(siguiente == NULL)break;
		if(actual->prioridad > siguiente->prioridad){
			PCB* temp = actual;
			actual = siguiente;
			siguiente = temp;
		}
		list_iterator_next(i);
	}
	return false;
}

int planificador_enum(){
	if(!strcasecmp(AlgoritmoPlanificacion, "prioridades\0"))return PRIORIDADES;
	if(!strcasecmp(AlgoritmoPlanificacion, "round robin\0"))return ROUNDROBIN;
	if(!strcasecmp(AlgoritmoPlanificacion, "fifo\0"))return FIFO;
	return -1;
}