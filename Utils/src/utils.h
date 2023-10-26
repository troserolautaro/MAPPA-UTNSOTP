#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<commons/string.h>
#include<string.h>
#include<assert.h>
#include<signal.h>

//USUARIOS DE LA RED
typedef enum{
	KERNEL,
	CPU,
	MEMORIA,
	FILESYSTEM
}usuarios;


//REGUISTROS DE CPU
typedef struct{
	uint32_t AX;
	uint32_t BX;
	uint32_t CX;
	uint32_t DX;
}registros_CPU;

//ESTADOS DE PROCESOS
typedef enum{
	NEW,
	READY,
	EXEC,
	TERMINATED,
	BLOCKED
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
    t_list * parametros;
}t_instruccion;
#endif /* SRC_UTILS_H_ */

