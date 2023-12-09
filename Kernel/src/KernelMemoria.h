/*
 * KernelMemoria.h
 */
#ifndef KERNELMEMORIA_H_
#define KERNELMEMORIA_H_
#include"Generales.h"
extern t_dictionary * tag;
//MANEJO DE MEMORIA

extern sem_t paginaCargada,sem_archivoCreado,sem_truncado,sem_read,sem_write;
typedef enum{
	ESCRITURA,
	LECTURA,
	NOASIGNADO
}modoApertura;

typedef struct {
	uint32_t tipoDeLock;
	t_list * participantes;//son los procesos involucrados
}lock_t;

typedef struct {
	char* nombreArchivo;//descriptor de archivo
	int aperturas;
	lock_t* lockActivo;
	t_queue* colaLocks;
	uint32_t tamanio;

}registro_tag;//Tabla Archivos Global

typedef struct {
	uint32_t puntero;
	int modoApertura;//posible enum
}registro_tap;//Tabla de Archivos por Proceso
void cargar_pagina(uint32_t pid, uint32_t pagina);
void page_fault(t_list * parametros);


//MANEJO DE ARCHIVO
bool f_open(PCB* proceso,char * archivo,uint32_t modoApertura);
void f_close(PCB* proceso,char * archivo);
void f_seek(PCB* proceso, char * archivo, uint32_t puntero);
void f_truncate(t_list* parameters);
void f_read(t_list* parameters);
void f_write(t_list* parameters);
registro_tag* crear_reg_tag(char* archivo);
#endif /* KERNELMEMORIA_H_ */
