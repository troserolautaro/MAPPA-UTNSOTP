#ifndef MEMORIAUSUARIO_H_
#define MEMORIAUSUARIO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include <commons/collections/list.h>
#include <utilsServidor.h>
#include <utilsCliente.h>

typedef struct{
	uint32_t marco,p,m,posSWAP;
	pthread_mutex_t* mutexPagina;
}pagina_t;

//para fifo y
typedef struct{
	uint32_t pagina,pid;
	pthread_mutex_t* mutexGlobal;
	t_temporal* tiempo;
}pagina_global_t;

typedef struct{
	bool libre;
	int base;
	int limite;
	pthread_mutex_t* mutexMarco;
	//ver que otros datos podriamos poner aca
}marco_t;

extern void* espacioContiguoMemoria;
extern char* algoritmoReemplazo;
extern int conexionFS,tamMemoria,tamPagina,cantMarcos,retardoRespuesta;
extern int marcoFIFO;
extern t_list* tablapaginasGlobales;
extern pagina_t* (*algoritmoRemplazo)();
extern sem_t sem_bloquesSwap, sem_paginaSwap, sem_escribirSwap,sem_escribirBloque,sem_swapLiberado;
extern pthread_mutex_t mutexInversa;
void iniciar_memoria_usuario();
void crear_proceso(uint32_t pid,char* nombre, uint32_t tamanio);
void finalizar_proceso(uint32_t pid);
uint32_t get_dato(uint32_t direccion);
void set_dato(uint32_t direccion, uint32_t valor);
void page_fault(uint32_t pid,uint32_t numPagina);
void asignar_swap(t_list* args);
pagina_t * pagina_get(uint32_t pid, uint32_t pagina);
uint32_t devolver_num_marco(uint32_t pid, uint32_t numPagina);
pagina_t* fifo();
pagina_t* lru();
void cargar_pagina_swap(uint32_t pid, uint32_t numPagina, void* datos);
void enviar_datos_bloque(uint32_t direccionFisica);
void recibir_datos_bloque(uint32_t direccionFisica,void* datos);


#endif /* MEMORIAUSUARIO_H_ */
