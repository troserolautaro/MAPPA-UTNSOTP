#include"PlanificadorCorto.h"

sem_t mutexClock;
sem_t clock,validarQuantum;
int tiempoEjecucion;
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
			if(idPlanificador==ROUNDROBIN)sem_post();
		}else {
			pthread_mutex_unlock(&mutexColaCorto);
		}

	}while(true);
	return NULL;
	/*PRUEBA DE COMO HACER EL PLANIFICADOR */
}
//ALGORITMO DE PRIORIDADES
void prioridad(){
	list_sort((colaCorto->elements),comparar_prioridad_mayor);
}

bool comparar_prioridad_mayor(void* proceso1,void* proceso2 ){
		PCB* PCB1 =  (PCB*) proceso1;
		PCB* PCB2 =  (PCB*) proceso2;
		if(PCB1->prioridad < PCB2->prioridad)return true;
		return false;
}

//ALGORITMO DE ROUND ROBIN
void round_robin(){
	while(1){
		sem_post(&validarQuantum);
	}
}
void* validar_quantum(){
	while(1){
		sem_wait(&validarQuantum);
		pthread_mutex_lock(&mutexClock);
		if(tiempoEjecucion<quantum)sem_post(&clock);
		//else interrupcion a cpu
		pthread_mutex_unlock(&mutexClock);
	}
}
void enviar_interrupcion_cpu(){
	t_paquete* paquete = crear_paquete();
	agregar_a_paquete(paquete, "quantum", sizeof("quantum"));
	enviar_paquete(paquete,conexionCPUInterrupt);
	eliminar_paquete(paquete);
}
//iniciar hilo de clock en kernel si algoritmo de planificacion es rr
void* clock_rr(void* arg) {
	tiempoEjecucion=0;
    while (1) {
    	sem_wait(&clock);
        sleep(1000);  // Dormir 1 segundo (puedes ajustar esto según tus necesidades)
        pthread_mutex_lock(&mutexClock);
        tiempoEjecucion++;
        pthread_mutex_unlock(&mutexClock);
        sem_post(&validarQuantum);
    }
}
int planificador_enum(){
	if(!strcasecmp(AlgoritmoPlanificacion, "prioridades\0"))return PRIORIDADES;
	if(!strcasecmp(AlgoritmoPlanificacion, "round robin\0"))return ROUNDROBIN;
	if(!strcasecmp(AlgoritmoPlanificacion, "fifo\0"))return FIFO;
	return -1;
}
