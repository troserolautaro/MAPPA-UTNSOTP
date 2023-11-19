#include "MemoriaUsuario.h"

RegistroTablaPagina * TablaPaginacion;
void* espacioContiguoMemoria;
char *tamMemoria,
	 *tamPagina,
	 *algoritmoReemplazo;
int cantPaginas,retardoRespuesta;

//INICIAR  MEMORIA DE USUARIO
void iniciar_memoria_usuario(){
	//algoritmo de remplazo
	espacioContiguoMemoria=malloc((tamMemoria,NULL, 10));
	cantPaginas=strtol(tamMemoria,NULL,10)/strtol(tamPagina,NULL,10);
	TablaPaginacion=malloc(cantPaginas*sizeof(RegistroTablaPagina));
}

//CREAR PROCESO

//FINALIZAR PROCESO

//ACCESO A TABLA DE PAGINAS y LLAMADO A PAGE FAULT

//ACCESO A ESPACIO DE USUARIO LECTURA

//ACCESO A ESPACIO DE USUARIO ESCRITURA

//PAGE FAULT

//FIFO

//LRU

