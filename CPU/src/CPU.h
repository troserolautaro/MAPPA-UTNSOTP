#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>


//includes de utils globales
#include<utilsServidor.h>
#include<utilsCliente.h>
typedef enum{
	DESALOJO

}interrupcionTipo;
int serverDispatch,serverInterrupt;
int clienteKernel;
void iterator(char* value);
//MEMORIA
void cargar_tamanio_pagina();
uint32_t mmu(uint32_t* direccionLogica);
void obtener_marco(uint32_t numPagina);
//INSTRUCCIONES
void set_i(uint32_t * registroDestino, uint32_t valor);
void sum(uint32_t * registroDestino,uint32_t * registroOrigen);
void sub(uint32_t * registroDestino,uint32_t * registroOrigen);
void jnz(uint32_t * registroDestino,uint32_t pc);
void sleep_i(uint32_t tiempo); //Bloqueante
void wait(void* recurso); //Bloqueante ?¿
void signal_i(void* recurso);
void mov_in(uint32_t* registro, uint32_t* direccionFisica);
void mov_out(uint32_t* direccionFisica,uint32_t* registro);
void f_open(char* nombreArchivo, char* modoApertura);
void f_close(char* nombreArchivo);
void f_seek(char* nombreArchivo, uint32_t posicion);
void f_read(char* nombreArchivo, uint32_t direccionFisica);
void f_write(char* nombreArchivo, uint32_t direccionFisica);
void f_truncate(char* nombreArchivo, uint32_t newSize);
void exit_i();

//CICLO
void fetch();
void decode_and_execute();
void check_interrupt();
void contexto_ejecucion(t_list * mensaje);
void ejecutar_ciclo();
uint32_t * obtener_registro(char* registro);
void bloquear_proceso();




#endif /* CPU_H_ */

