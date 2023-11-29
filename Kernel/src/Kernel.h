#ifndef Kernel_H_
#define Kernel_H_

#include<pthread.h>
#include"Consola.h"

typedef enum{
	PROCESOEXIT
}desalojo;
void paquete(int);
void terminar_programa();
void sleep_proceso(PCB* proceso, int tiempo);
//MANEJO DE RECURSOS
void wait_recurso(PCB* proceso, char* recurso);
void signal_recurso(PCB* proceso, char* recurso);
//MANEJO DE MEMORIA
void cargar_pagina(char * pagina);
void page_fault(PCB* proceso,char * pagina);

//MANEJO DE ARCHIVO
void f_open(PCB* proceso,char * archivo);
void f_close(PCB* proceso,char * archivo);
void f_seek(PCB* proceso, char * archivo);
void f_truncate(PCB* proceso, char * archivo);
void f_read(PCB* proceso, char * archivo);
void f_write(PCB* proceso, char * archivo);

t_dictionary *tag;//clave archivo, valor archivo_t
t_dictionary *diccionarioDeDiccionariosLocales;//clave proceso, valor diccionario de archivos abiertos por el proceso

typedef struct {
	int df;//descriptor de archivo
	char* nombreArchivo;
	t_queue * colaLectura;
	t_queue * colaEscritura;
}registro_tag;//Tabla Archivos Global

typedef struct {
	char* nombreArchivo;
	int puntero;
	char* modoApertura;//posible enum
	sem_t * semaforoLectura;
	sem_t * semaforoEscritura;
}registro_tap;//Tabla de Archivos por Proceso


#endif /* Kernel_H_ */
