#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>

//includes de utils globales
#include<utilsServidor.h>
#include<utilsCliente.h>

int serverDispatch,serverInterrupt;
int clienteKernel;
t_config* iniciar_config(void);
t_log* iniciar_logger(void);
void iterator(char* value);
void ejecutar_ciclo();

//INSTRUCCIONES
void set_i(uint32_t * registroDestino, uint32_t valor);
void sum(uint32_t * registroDestino,uint32_t * registroOrigen);
void sub(uint32_t * registroDestino,uint32_t * registroOrigen);
void jnz(uint32_t * registroDestino,uint32_t pc);
void sleep_i(uint32_t tiempo); //Bloqueante
void wait(void* recurso); //Bloqueante ?¿
void signal_i(void* recurso);
void mov_in(uint32_t *registroDestino,void *direccionLogica);
void mov_out(void *direccionLogica, uint32_t * regisdtroDestino);
void f_open(void* nombreArchivo, void* apertura);
void f_close(void* nombreArchivo);
void f_seek(void*nombreArchivo,void* posicion);
void f_read(void *nombreArchivo, void *direccionLogica);
void f_write(void *nombreArchivo, void *direccionLogica);
void f_truncate(void *nombreArchivo,void *tamaño);
void exit_i();

//CICLO
void fetch(int pid,int pc);
void decode();
void execute();
void check_interrupt();
uint32_t * obtener_registro(char* registro);




#endif /* CPU_H_ */

