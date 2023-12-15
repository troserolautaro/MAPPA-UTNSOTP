#include"Generales.h"

uint32_t PIDGLOBAL;
bool detenida=false;
t_queue* colaLargo;
t_queue* colaCorto;
t_list* procesos,*mutexProceso;
t_log* logger;
t_config* config;
int conexionCPUDispatch, conexionCPUInterrupt,conexionMemoria,conexionFileSystem,pidGlobal;
int gradoMultiprogramacion, quantum;
char* AlgoritmoPlanificacion;
pthread_mutex_t mutexColaCorto,mutexColaLargo,mutexProcesos,mutexLog,mutexGrado,mutexRecursos,mutexDetenida;
sem_t planiLargo,planiCorto;
t_dictionary *diccionarioRecursos;

char* estado_enum(uint32_t estado){
	switch(estado){
		case NEW:return("NEW\n");break;
		case READY:return("READY\n");break;
		case EXEC:return("EXEC\n");break;
		case BLOCKED:return("BLOCKED\n");break;
		case TERMINATED:return("TERMINATED\n");break;
		case PAGBLOCK: return("PAGBLOCK"); break;
		default:return("Estado no definido\n");break;
		}
}
void cambiar_estado(PCB* proceso, int estado){
	char* mensaje =string_from_format("PID: %d",proceso->pid);
	pthread_mutex_lock(proceso->mutex);
	string_append_with_format(&mensaje," - Estado Anterior: %s",estado_enum(proceso->estado)); //0
	proceso->estado=estado;
	string_append_with_format(&mensaje," - Estado Actual: %s",estado_enum(proceso->estado)); // 2
	pthread_mutex_unlock(proceso->mutex);
	escritura_log(mensaje);


	free(mensaje);
}
void iterar_lista(char** mensaje) {
    void iterator(PCB* proceso) {
    	string_append_with_format(mensaje,"PID_%s ",string_itoa((int)proceso->pid));
    }
    list_iterate(colaCorto->elements, (void*) iterator);
}
void push_colaCorto(PCB* proceso){
	bool buscar_proceso_en_cola(PCB* temp){
		return (temp->pid == proceso->pid);
	}
	cambiar_estado(proceso,READY);
	char* mensaje = string_from_format("Cola Ready %s: ",AlgoritmoPlanificacion);
	pthread_mutex_lock(&mutexColaCorto);
	if(list_find(colaCorto->elements,(void*)buscar_proceso_en_cola) == NULL){
		queue_push(colaCorto,proceso);
	}
	iterar_lista(&mensaje);
	pthread_mutex_unlock(&mutexColaCorto);
	escritura_log(mensaje);
	free(mensaje);
}


