#include"PlanificadorCorto.h"

sem_t clockT,validarQuantum,contexto;
bool ejecutandoB = false;

/*DECIDI HACERLO UNA COLA PORQUE CREI QUE ERA LO MEJOR */
void* clock_rr(void* arg) {
	int tiempoEjecucion=0;
	debug("Clock listo y esperando");
    while (1) {
    	sem_wait(&clockT); //
        tiempoEjecucion++;
        pthread_mutex_lock(&mutexColaCorto);
        bool empty = queue_is_empty(colaCorto);
        pthread_mutex_unlock(&mutexColaCorto);
        if(tiempoEjecucion>=quantum && !empty){
			enviar_interrupcion_cpu("quantum");
			sem_wait(&contexto);
			tiempoEjecucion = 0;
		}

    }
}
void* planificador_corto(){
	sem_init(&clockT,0,0);
	sem_init(&contexto,0,0);
	int idPlanificador = planificador_enum();
	debug(string_itoa(idPlanificador));
	if(idPlanificador == ROUNDROBIN){  hilo_funcion(NULL,clock_rr);}
	do{
		sem_wait(&planiCorto);
		pthread_mutex_lock(&mutexColaCorto);
		if(!queue_is_empty(colaCorto) && !detenida){
			pthread_mutex_unlock(&mutexColaCorto);

			switch(idPlanificador){
				case PRIORIDADES: prioridad(); break;
				case ROUNDROBIN:  break;
				case FIFO: break;
				default:error_show("No se reconocio el algoritmo");break;
			}

			//si no es ninguno de los anterior es fifo por que es una cola (estructura de tipo fifo)
			pthread_mutex_lock(&mutexColaCorto);
			PCB* proceso= queue_pop(colaCorto);
			pthread_mutex_unlock(&mutexColaCorto);

			cambiar_estado(proceso,EXEC);
			ejecutandoB = true;
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
}
bool buscar_proceso_ejecutando(void* proceso){
	return (((PCB*)proceso)->estado==EXEC) ? true : false;
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

	if(ejecutandoB){

		pthread_mutex_lock(&mutexProcesos);
		PCB* ejecutando = list_find(procesos,buscar_proceso_ejecutando);
		pthread_mutex_unlock(&mutexProcesos);

		pthread_mutex_lock(&mutexColaCorto);
		PCB* prioritario = (PCB*)queue_peek(colaCorto);
		pthread_mutex_unlock(&mutexColaCorto);

		if(ejecutando != NULL && ejecutando->pid != prioritario->pid && ejecutando->prioridad > prioritario->prioridad){

			enviar_interrupcion_cpu("prioridades");
			sem_wait(&contexto);
			ejecutandoB = false;

		}


	}
}



//ALGORITMO DE ROUND ROBIN
void round_robin(){

}

void enviar_interrupcion_cpu(char* motivo){
	t_paquete* paquete = crear_paquete();
	agregar_a_paquete(paquete,"interrupcion",sizeof("interrupcion"));
	agregar_a_paquete(paquete,"desalojo",sizeof("desalojo"));
	agregar_a_paquete(paquete, motivo, strlen(motivo)+1);
	enviar_paquete(paquete,conexionCPUInterrupt);
	eliminar_paquete(paquete);
}
//iniciar hilo de clock en kernel si algoritmo de planificacion es rr

int planificador_enum(){
	if(!strcasecmp(AlgoritmoPlanificacion, "prioridades"))return PRIORIDADES;
	if(!strcasecmp(AlgoritmoPlanificacion, "round_robin"))return ROUNDROBIN;
	if(!strcasecmp(AlgoritmoPlanificacion, "fifo"))return FIFO;
	return -1;
}
