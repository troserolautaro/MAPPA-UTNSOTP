#ifndef MEMORIAUSUARIO_H_
#define MEMORIAUSUARIO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>

typedef struct{
	int Marco,P,M,PosSWAP;
}RegistroTablaPagina;

extern RegistroTablaPagina * TablaPaginacion;
extern void* espacioContiguoMemoria;
extern char *tamMemoria,
			*tamPagina,
			*algoritmoReemplazo;
extern int cantPaginas,retardoRespuesta;

void iniciar_memoria_usuario();
#endif /* MEMORIAUSUARIO_H_ */
