#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>


//includes de utils globales
#include<utilsServidor.h>
#include<utilsCliente.h>

int serverDispatch,serverInterrupt;
int clienteKernel;
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
void fetch();
void decode();
void execute();
void check_interrupt();
void contexto_ejecucion(t_list * mensaje);
uint32_t * obtener_registro(char* registro);



//OVERLOAD
void contexto_ejecucion_lista(t_list* mensaje);
void contexto_ejecucion_char(char* mensaje);
#define OVERLOAD(func, ...) CHOOSE_OVERLOAD(func, __VA_ARGS__)

#define CHOOSE_OVERLOAD(func, ...) GET_OVERLOAD(func, COUNT_ARGS(__VA_ARGS__))

#define GET_OVERLOAD(func, count) func ## _ ## count

#define COUNT_ARGS(...) COUNT_ARGS_(__VA_ARGS__, 5, 4, 3, 2, 1)
#define COUNT_ARGS_(_1, _2, _3, _4, _5, N, ...) N
#endif /* CPU_H_ */

