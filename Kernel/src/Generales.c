#include"Generales.h"

int PIDGLOBAL;
bool detenida;
t_queue* colaLargo;
t_queue* colaCorto;
t_list* procesos;
t_log* logger;
t_config* config;
int conexionCPUDispatch, conexionCPUInterrupt,conexionMemoria,conexionFileSystem,pidGlobal;
int gradoMultiprogramacion, quantum;
char* AlgoritmoPlanificacion;
pthread_mutex_t mutexPlaniLargo,mutexPlaniCorto,mutexColaCorto,mutexColaLargo;
