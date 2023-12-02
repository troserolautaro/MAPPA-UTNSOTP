/*
 * KernelMemoria.h
 */
#ifndef KERNELMEMORIA_H_
#define KERNELMEMORIA_H_
#include"Generales.h"
//MANEJO DE MEMORIA
void cargar_pagina(char * pagina);
void page_fault(PCB* proceso,char * pagina);

//MANEJO DE ARCHIVO
void f_open(PCB* proceso,char * archivo,char*);
void f_close(PCB* proceso,char * archivo);
void f_seek(PCB* proceso, char * archivo);
void f_truncate(PCB* proceso, char * archivo);
void f_read(PCB* proceso, char * archivo);
void f_write(PCB* proceso, char * archivo);

sem_t  semLectura;
sem_t  semEscritura;
t_queue * colaLocks;
t_dictionary *diccionarioDeDiccionariosLocales;
typedef struct {
	char* archivo;
	int tipoDeLock;
	t_list * participantes;//son los procesos involucrados
}lock;

typedef struct {
	int df;//descriptor de archivo
	char* nombreArchivo;
	int aperturas;
	lock * colaLocksEscritura;
	lock   * colaLocksLectura;
}registro_tag;//Tabla Archivos Global

typedef struct {
	char* nombreArchivo;
	int puntero;
	char* modoApertura;//posible enum
}registro_tap;//Tabla de Archivos por Proceso

#endif /* KERNELMEMORIA_H_ */
