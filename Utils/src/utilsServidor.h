#ifndef UTILSSERVIDOR_H_
#define UTILSSERVIDOR_H_

//include del utils general
#include"utils.h"




void* recibir_buffer(int*, int);

int iniciar_servidor(char*);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
void * procesar_tipo(int);
void procesar_mensaje(t_list *);
void * manejar_cliente(void *);
void * recibir_conexiones(void *);
void terminar_hilos();
#endif /* UTILSSERVIDOR_H_ */
