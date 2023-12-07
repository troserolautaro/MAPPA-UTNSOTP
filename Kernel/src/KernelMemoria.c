//MANEJO DE FILE SYSTEM
#include "KernelMemoria.h"
int dfGlobal=0;
sem_t paginaCargada;
sem_t  semLectura;
sem_t  semEscritura;

t_queue * colaLocks;
t_dictionary *diccionarioDeDiccionariosLocales,* tag,*t_dictionarytap;
/*void abrir_archivo(char* archivo){
	//ver como lo mando fs
}

bool semaforo_bloqueado(sem_t *semaforo){
	int valorSemaforo;
	sem_getvalue(semaforo,&valorSemaforo);
	if(valorSemaforo<1){
		return true;
	}
	return false;
}
*/

//funciones auxiliares de tag y tap
registro_tag* crear_reg_tag(char* archivo){
	registro_tag *registroTag=malloc(sizeof(registro_tag*));
	registroTag->nombreArchivo=archivo;
	registroTag->df=dfGlobal;
	dfGlobal++;//semaforo si es necesario
	return registroTag;
}

registro_tag* get_reg_tag(char* archivo){
	//VALIDO SI ESTA EN LA TABLA DE ARCHIVOS GLOBAL
	//si esta el archivo en global lo recupera
	//sino lo crea y añade
	registro_tag *registroTag=malloc(sizeof(registro_tag*));
	if(dictionary_has_key(tag,archivo)){
		registroTag = (registro_tag*)dictionary_get(tag,archivo);
	}else{
		//sino esta en global lo crea y lo añade
		registroTag=crear_reg_tag(archivo);
		dictionary_put(tag,archivo,&registroTag);
	}
	return registroTag;
}

t_dictionary * get_tap(int pid){
	t_dictionary * tap;
	if(dictionary_has_key(diccionarioDeDiccionariosLocales,string_itoa(pid))){
		tap=(t_dictionary *) dictionary_get(diccionarioDeDiccionariosLocales,string_itoa(pid));
	}
	else{
		//si no existe lo crea  y añade el archivo
		tap=dictionary_create();
		dictionary_put(diccionarioDeDiccionariosLocales,string_itoa(pid),&tap);
	}
	return tap;
}
//no usar
registro_tap* get_reg_tap(int pid,int df){
	//VALIDO SI ESTA EN LA TABLA DE ARCHIVOS POR PROCESO
	//si esta el archivo en la tabla por proceso lo recupera
	//sino lo crea y añade
	registro_tap *registroTap=malloc(sizeof(registro_tag*));
	//SI EXISTE EL DICCIONARIO DEL PROCESO
	t_dictionary * tap=get_tap(pid);
	//si no esta abierto lo añade
	if(dictionary_has_key(tap,string_itoa(df))){
		return (registro_tap*)dictionary_get(tap,string_itoa(df));
	}
	return NULL;
}

//usar la version 2
registro_tap* get_reg_tap_v2(int pid,char* archivo){
	registro_tag* registroTag =get_reg_tag(archivo);
	registro_tap* registroTap=get_reg_tap(pid,(registroTag->df));
	return registroTap;
}

/*void f_open(PCB* proceso,char * archivo,char* modoApertura){
	//NUEVA LOGICA LUEGO DE RELEER EL ENUNCIADO DE VARIAS VECES
	registro_tag* regTag=get_reg_tag(archivo);
	registro_tap* regTap=get_reg_tap((proceso->pid),(regTag->df));
	if(modoApertura=="L"){
		if(!queue_is_empty((regTag->colaLocksEscritura))){
					cambiar_estado(proceso,BLOCKED);
					//esperar que finalice
					//posible semaforo hilo para esperar que termine la escritura
		}
		else{
			if(!queue_is_empty((regTag->colaLocksLectura))){
				agregar_proceso_como_participante(proceso);
			}
			else{
				crear_lock_lectura(archivo);
			}
		}
	}
	if(modoApertura=="R"){
		crear_lock_escritura(proceso);
		if(existe_otro_lock()){
			cambiar_estado(proceso,BLOCKED);
		}
	}
} */

/* void f_close(PCB* proceso,char * archivo){
	if(validar_lock_lectura(archivo)){
		sem_post(&(get_lock_escritura(archivo)));//reducir participantes
		//if participantes 0  cierra lock de lectura
	}
	else{
		if(validar_lock_escritura(archivo)){
			agregar_proceso_como_participante(proceso);
			//sem_wait(&(get_lock_lectura()));//revisar si seria un wait el agregar participante
		}
		else{
			crear_lock_lectura(archivo);
		}
	}
	//HACER LUEGO METODO PARA DESTRUIR ARCHIVO
	//dictionary_remove_and_destroy(t_dictionary *, char *, void(*element_destroyer)(void*));
	//dictionary_remove(diccionarioArchivos,archivo);//VER SI ENREALIDAD SE SACA DE LA GLOBAL
} */

void f_seek(PCB* proceso, char * archivo,uint32_t puntero){
	registro_tap *registroTap=get_reg_tap_v2((proceso->pid),archivo);
	if(registroTap!=NULL){
	registroTap->puntero=puntero;
	}
}

void f_truncate(PCB* proceso, char * archivo,uint32_t tamaño){
/*
 * Esta función solicitará al módulo File System que actualice el tamaño del archivo al nuevo tamaño pasado por parámetro
 *  y bloqueará al proceso hasta que el File System informe de la finalización de la operación.
 */
	t_paquete * paquete = crear_paquete();
	agregar_a_paquete(paquete,"f_truncate",sizeof("f_truncate"));
	agregar_a_paquete(paquete,&(proceso->pid),sizeof(uint32_t));
	agregar_a_paquete(paquete,archivo,sizeof(char));
	agregar_a_paquete(paquete,&tamaño,sizeof(uint32_t));
	enviar_paquete(paquete,conexionFileSystem);
	eliminar_paquete(paquete);
	//semaforo de filesystem
}

void f_read(PCB* proceso, char * archivo,uint32_t direccionFisica){
/*
 * Para esta función se solicita al módulo File System que lea desde el puntero del archivo pasado por parámetro y
 *  lo grabe en la dirección física de memoria recibida por parámetro. El proceso que llamó a F_READ, deberá permanecer
 *  en estado bloqueado hasta que el módulo File System informe de la finalización de la operación.
 * */
	t_paquete * paquete = crear_paquete();
	agregar_a_paquete(paquete,"f_read",sizeof("f_read"));
	agregar_a_paquete(paquete,&(proceso->pid),sizeof(uint32_t));
	agregar_a_paquete(paquete,archivo,sizeof(char));
	agregar_a_paquete(paquete,&direccionFisica,sizeof(uint32_t));
	enviar_paquete(paquete,conexionFileSystem);
	eliminar_paquete(paquete);
	//semaforo de filesystem
}

void f_write(PCB* proceso, char * archivo,uint32_t direccionFisica){
/*
 * Esta función, en caso de que el proceso haya solicitado un lock de escritura, solicitará al módulo File System
 * que escriba en el archivo desde la dirección física de memoria recibida por parámetro. El proceso que llamó a
 * F_WRITE, deberá permanecer en estado bloqueado hasta que el módulo File System informe de la finalización de
 *  la operación. En caso de que el proceso haya solicitado un lock de lectura, se deberá cancelar la operación y
 *   enviar el proceso a EXIT con motivo de INVALID_WRITE.
 * */
	t_paquete * paquete = crear_paquete();
	agregar_a_paquete(paquete,"f_write",sizeof("f_write"));
	agregar_a_paquete(paquete,&(proceso->pid),sizeof(uint32_t));
	agregar_a_paquete(paquete,archivo,sizeof(char));
	agregar_a_paquete(paquete,&direccionFisica,sizeof(uint32_t));
	enviar_paquete(paquete,conexionFileSystem);
	eliminar_paquete(paquete);
	//semaforo de filesystem

}
//PAGE FAULT
void cargar_pagina(uint32_t pid, uint32_t pagina){
	t_paquete * paquete = crear_paquete();
	agregar_a_paquete(paquete,"pageFault",sizeof("pageFault"));
	agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
	agregar_a_paquete(paquete,&pagina,sizeof(uint32_t));
	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);
	//pendiente a definir bien la paginacion bajo demanda en memoria de usuario
}
//ver si no hace falta crear un hilo para el page fault
void page_fault(t_list* parametros){
	uint32_t pagina = (uint32_t) strtol(list_get(parametros,1), (char **)NULL, 10);
	PCB* proceso = list_get(parametros,0);
	cambiar_estado(proceso,PAGBLOCK);
	cargar_pagina((uint32_t)proceso->pid,pagina);
	sem_wait(&paginaCargada);//agregar en procesar mensaje el sem_post para este semaforo y el semaforo
	push_colaCorto(proceso);
	sem_post(&planiCorto);
}

