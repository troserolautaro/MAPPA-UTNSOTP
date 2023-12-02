//MANEJO DE FILE SYSTEM
#include "KernelMemoria.h"
int dfGlobal=0;
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

/*registro_tag* get_reg_tag(char* archivo){
	//VALIDO SI ESTA EN LA TABLA DE ARCHIVOS GLOBAL
	//si esta el archivo en global lo recupera
	//sino lo crea y añade
	registro_tag *registroTag=malloc(sizeof(registro_tag*));
	if(dictionary_has_key(tag,archivo)){
		registroTag = (registro_tag*)dictionary_get(tag,archivo);
	}else{
		//sino esta en global lo crea y lo añade
		registroTag=crear_registroTag(archivo);
		dictionary_put(tag,archivo,&registroTag);
	}
	return registroTag;
}*/

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

void f_seek(PCB* proceso, char * archivo){
/*
 * Actualiza el puntero del archivo en la tabla de archivos abiertos del proceso hacia la ubicación pasada por parámetro.
 * Se deberá devolver el contexto de ejecución a la CPU para que continúe el mismo proceso.
 */

}
void f_truncate(PCB* proceso, char * archivo){
/*
 * Esta función solicitará al módulo File System que actualice el tamaño del archivo al nuevo tamaño pasado por parámetro
 *  y bloqueará al proceso hasta que el File System informe de la finalización de la operación.
 */

}

void f_read(PCB* proceso, char * archivo){
/*
 * Para esta función se solicita al módulo File System que lea desde el puntero del archivo pasado por parámetro y
 *  lo grabe en la dirección física de memoria recibida por parámetro. El proceso que llamó a F_READ, deberá permanecer
 *  en estado bloqueado hasta que el módulo File System informe de la finalización de la operación.
 * */

}

void f_write(PCB* proceso, char * archivo){
/*
 * Esta función, en caso de que el proceso haya solicitado un lock de escritura, solicitará al módulo File System
 * que escriba en el archivo desde la dirección física de memoria recibida por parámetro. El proceso que llamó a
 * F_WRITE, deberá permanecer en estado bloqueado hasta que el módulo File System informe de la finalización de
 *  la operación. En caso de que el proceso haya solicitado un lock de lectura, se deberá cancelar la operación y
 *   enviar el proceso a EXIT con motivo de INVALID_WRITE.
 * */

}
//PAGE FAULT
void cargar_pagina(char * pagina){
	//pendiente a definir bien la paginacion bajo demanda en memoria de usuario
}
//ver si no hace falta crear un hilo para el page fault
void page_fault(PCB* proceso,char * pagina){
	cambiar_estado(proceso,BLOCKED);
	cargar_pagina(pagina);
	//sem_wait(&paginaCargada);//agregar en procesar mensaje el sem_post para este semaforo y el semaforo
	cambiar_estado(proceso,READY);
}

