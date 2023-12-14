#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<commons/string.h>
#include<commons/error.h>
#include<commons/memory.h>
#include<string.h>
#include<assert.h>
#include<signal.h>
#include<pthread.h>
#include <semaphore.h>

//Acordarse de inicializarlos
extern t_log* logger;
extern pthread_mutex_t mutexLog,mutexConfig;
extern t_config* config;
typedef void* funcion(void*);
//funciones

//USUARIOS DE LA RED
typedef enum{
	KERNEL,
	CPUDispatch,
	CPUInterrupt,
	MEMORIA,
	FILESYSTEM
}usuarios;

typedef struct{
	uint32_t AX;
	uint32_t BX;
	uint32_t CX;
	uint32_t DX;
}registros_CPU;

typedef struct{
	uint32_t pid;
	uint32_t pc;
	uint32_t prioridad;
	uint32_t estado;
	registros_CPU* registros;
	t_dictionary* tablaArchivos;
	t_list * recursos;
	pthread_mutex_t * mutex;
}PCB;

//REGUISTROS DE CPU


//ESTADOS DE PROCESOS
typedef enum{
	NEW,
	READY,
	EXEC,
	TERMINATED,
	BLOCKED,
	PAGBLOCK
}estados;

//CODIGOS DE OPERACION
typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

//BUFFER
typedef struct
{
	int size;
	void* stream;
} t_buffer;

//PAQUETE
typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

//INSTRUCCION
typedef struct {
    char * comando;
    t_list * parametros; //Maximo dos parametros
}t_instruccion;


PCB* proceso_create();
void proceso_destroy(PCB* proceso);
PCB* proceso_copy(PCB* origen);
void proceso_clear(PCB* proceso);
registros_CPU* registros_create();
void registros_destroy();



t_instruccion *instruccion_create();
void instruccion_destroy(t_instruccion*);
void liberar_memoria(void * elemento);
t_log* iniciar_logger(char* log);
t_config* iniciar_config(char* config);
void hilo_funcion(void* parametro,funcion funcion);
void escritura_log(char* mensaje);
void debug(char* mensaje);
#endif /* SRC_UTILS_H_ */

