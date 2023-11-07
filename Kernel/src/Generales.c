#include"Generales.h"

uint32_t PIDGLOBAL;
bool detenida;
t_queue* colaLargo;
t_queue* colaCorto;
t_list* procesos;
t_log* logger;
t_config* config;
int conexionCPUDispatch, conexionCPUInterrupt,conexionMemoria,conexionFileSystem,pidGlobal;
int gradoMultiprogramacion, quantum;
char* AlgoritmoPlanificacion;
pthread_mutex_t mutexColaCorto,mutexColaLargo,mutexProcesos;
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
