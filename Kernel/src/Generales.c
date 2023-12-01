#include"Generales.h"

uint32_t PIDGLOBAL;
bool detenida=false;
t_queue* colaLargo;
t_queue* colaCorto;
t_list* procesos;
t_log* logger;
t_config* config;
int conexionCPUDispatch, conexionCPUInterrupt,conexionMemoria,conexionFileSystem,pidGlobal;
int gradoMultiprogramacion, quantum;
char* AlgoritmoPlanificacion;
pthread_mutex_t mutexColaCorto,mutexColaLargo,mutexProcesos,mutexLog,mutexGrado;
sem_t planiLargo,planiCorto;

char* estado_enum(uint32_t estado){
	switch(estado){
		case NEW:return("NEW\n");break;
		case READY:return("READY\n");break;
		case EXEC:return("EXEC\n");break;
		case BLOCKED:return("BLOCKED\n");break;
		case TERMINATED:return("TERMINATED\n");break;
		default:return("Estado no definido\n");break;
		}
}
void cambiar_estado(PCB* proceso, int estado){
	char* mensaje =string_from_format("PID: %d",proceso->pid);
	string_append_with_format(&mensaje," - Estado Anterior: %s",estado_enum(proceso->estado)); //0
	proceso->estado=estado;
	string_append_with_format(&mensaje," - Estado Actual: %s",estado_enum(proceso->estado)); // 2

	escritura_log(mensaje);


	free(mensaje);
}


