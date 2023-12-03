#ifndef MEMORIAUSUARIO_H_
#define MEMORIAUSUARIO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <utilsServidor.h>
#include <utilsCliente.h>

typedef struct{
	uint32_t marco,p,m,posSWAP;
}pagina_t;

//para fifo y
typedef struct{
	uint32_t pagina,pid,accesos;
}pagina_global_t;

typedef struct{
	bool libre;
	int base;
	int limite;
	//ver que otros datos podriamos poner aca
}marco_t;

extern void* espacioContiguoMemoria;
extern char* algoritmoReemplazo;
extern int conexionFS,tamMemoria,tamPagina,cantMarcos,retardoRespuesta;
extern int marcoFIFO;
extern pagina_t* (*algoritmoRemplazo)();
void iniciar_memoria_usuario();

pagina_t* fifo();
pagina_t* lru();


#endif /* MEMORIAUSUARIO_H_ */
