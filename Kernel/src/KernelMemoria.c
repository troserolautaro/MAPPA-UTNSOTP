//MANEJO DE FILE SYSTEM
#include "KernelMemoria.h"
sem_t paginaCargada;
sem_t  semLectura;
sem_t  semEscritura;

t_queue * colaLocks;
t_dictionary * tag,*t_dictionarytap;
lock_t* crear_lock(uint32_t tipoDeLock){
	lock_t * lock = malloc(sizeof(lock_t));
	lock->participantes=list_create();
	lock->tipoDeLock = tipoDeLock;
	return lock;
}
void agregar_participante(lock_t* lock, PCB* participante){
	list_add(lock->participantes,participante);
}
void sacar_participante(lock_t* lock, PCB* participante){
	list_remove_element(lock->participantes,participante);
}
void destructor_participantes(PCB* proceso){
	proceso_destroy(proceso);
}
void destruir_lock(lock_t* lock){
	list_destroy_and_destroy_elements(lock->participantes,(void*)destructor_participantes);
	free(lock);
}
registro_tap* crear_reg_tap(uint32_t modoApertura){
	registro_tap* registroTap = malloc(sizeof(registro_tap));
	registroTap->puntero = -1;
	registroTap->modoApertura = modoApertura;
	return registroTap;
}

void destruir_registro_tap(registro_tap* registroTap){
	free(registroTap);
}
//funciones auxiliares de tag y tap
registro_tag* crear_reg_tag(char* archivo){
	registro_tag *registroTag=malloc(sizeof(registro_tag));
	registroTag->nombreArchivo=archivo;
	registroTag->aperturas = 0;
	registroTag->lockActivo=crear_lock(NOASIGNADO);
	registroTag->colaLocks = queue_create();//semaforo si es necesario
	return registroTag;
}
void agregar_a_colaLock(lock_t* lockAEncolar,registro_tag* registroTag){
	queue_push(registroTag->colaLocks,lockAEncolar);
}
void destruir_registro_tag(registro_tag* registroTag){
	free(registroTag->nombreArchivo);
	destruir_lock(registroTag->lockActivo);
	queue_clean(registroTag->colaLocks);
	queue_destroy(registroTag->colaLocks);
	free(registroTag);
}

registro_tag* get_reg_tag(char* archivo){
	//VALIDO SI ESTA EN LA TABLA DE ARCHIVOS GLOBAL
	//si esta el archivo en global lo recupera
	//sino lo crea y añade
	registro_tag *registroTag;
	if(dictionary_has_key(tag,archivo)){
		registroTag = (registro_tag*)dictionary_get(tag,archivo);
	}else{
		//sino esta en global lo crea y lo añade
		registroTag=crear_reg_tag(archivo);
		dictionary_put(tag,archivo,registroTag);
	}
	return registroTag;
}
registro_tap* get_reg_tap(t_dictionary* tablaArchivos, char* archivo){
	//VALIDO SI ESTA EN LA TABLA DE ARCHIVOS POR PROCESO
	//si esta el archivo en la tabla por proceso lo recupera
	//sino lo crea y añade
	//SI EXISTE EL DICCIONARIO DEL PROCESO
	//si no esta abierto lo añade
	if(dictionary_has_key(tablaArchivos,archivo)){
		return (registro_tap*)dictionary_get(tablaArchivos,archivo);
	}
	return NULL;
}
void agregar_lock_activo(registro_tag* regTag,uint32_t modoApertura){
	lock_t* lockTemp = crear_lock(modoApertura);
	destruir_lock(regTag->lockActivo);
	regTag->lockActivo = lockTemp;
}
/*t_dictionary * get_tap(int pid){
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
}*/
//no usar

//usar la version 2
/*registro_tap* get_reg_tap_v2(int pid,char* archivo){
	registro_tag* registroTag =get_reg_tag(archivo);
	registro_tap* registroTap=get_reg_tap(pid,);
	return registroTap;
}*/
void agregar_reg_tap(PCB* proceso, char* archivo, uint32_t modoApertura){
	registro_tap* regTap = crear_reg_tap(modoApertura);
	dictionary_put(proceso->tablaArchivos,archivo,regTap);
}
void f_open(PCB* proceso,char * archivo,uint32_t modoApertura){
	//parte fisica del fopen
	if(!dictionary_has_key(tag,archivo)){
	t_paquete * paquete = crear_paquete();
	agregar_a_paquete(paquete,"f_open",sizeof("f_open"));
	//agregar_a_paquete(paquete,&(proceso->pid),sizeof(uint32_t));
	agregar_a_paquete(paquete,archivo,strlen(archivo)+1);
	enviar_paquete(paquete,conexionFileSystem);
	eliminar_paquete(paquete);
	}
	//posible semaforo aca esperando respuesta de fs
	//parte logica del fopen
	registro_tag* regTag=get_reg_tag(archivo);
//	registro_tap* regTap=get_reg_tap((proceso.),archivo);

	switch(modoApertura){
		case ESCRITURA:
			lock_t * lockEscritura = crear_lock(modoApertura);
			agregar_participante(lockEscritura,proceso);
			//Crear LOCK exclusivo por lectura, es decir nuevo lock con un solo participante
			if(regTag->lockActivo->tipoDeLock == NOASIGNADO){
				//No bloquear y darle el archivo
				destruir_lock(regTag->lockActivo);
				regTag->lockActivo = lockEscritura;
				agregar_reg_tap(proceso,archivo,modoApertura);
			}else{
				//Bloquear y agregar a la COLA LOCK
				bloquear_proceso(proceso,archivo);
				agregar_a_colaLock(lockEscritura,regTag);
			}
			break;

		case LECTURA:
			if(regTag->lockActivo->tipoDeLock == ESCRITURA){
				lock_t* lockLectura = crear_lock(modoApertura);
				agregar_participante(lockLectura,proceso);
				bloquear_proceso(proceso,archivo);
				agregar_a_colaLock(lockLectura,regTag);
				//Bloquear proceso y agregar a cola LOCK
				//Al hacer que la cola solo reciba lock* tengo que crear uno nuevo por cada nuevo de lectura,
				//por ende despues solo habria que usar el primero
			}
			if(regTag->lockActivo->tipoDeLock == LECTURA){
				//Agregar como participante al LOCK
				agregar_participante(regTag->lockActivo,proceso);
				agregar_reg_tap(proceso,archivo,modoApertura);
			}
			if(regTag->lockActivo->tipoDeLock == NOASIGNADO){
				//Crear lock con un solo participante
				agregar_lock_activo(regTag,modoApertura);
				agregar_participante(regTag->lockActivo,proceso);
				agregar_reg_tap(proceso,archivo,modoApertura);

			}
			break;
	}

	}

//F CLOSE
void borrar_reg_tap(t_dictionary* tablaArchivos, char* regABorrar){
	dictionary_remove_and_destroy(tablaArchivos,regABorrar,(void*)destruir_registro_tap);
}


void f_close(PCB* proceso,char * archivo){
	registro_tag* regTag=get_reg_tag(archivo);
	borrar_reg_tap(proceso->tablaArchivos,archivo);
	lock_t* lockActivo = regTag->lockActivo;
	sacar_participante(lockActivo,proceso);
	if(list_is_empty(lockActivo->participantes)){
		//Si la cola de locks no esta vacia agarro el lock que espero mas tiempo y lo pongo como principal
		if(!queue_is_empty(regTag->colaLocks)){
			bool escritura = false;
			destruir_lock(lockActivo);
			lockActivo = queue_pop(regTag->colaLocks);;
		//Si el nuevo lock no es de escritura saco todos los posibles
		if(lockActivo->tipoDeLock != ESCRITURA){
			while(!queue_is_empty(regTag->colaLocks) && !escritura){
				lock_t* lockTemp = queue_peek(regTag->colaLocks);
				if(lockTemp->tipoDeLock == ESCRITURA){
					escritura = true;
				}else{
					agregar_participante(lockActivo,list_get(lockTemp->participantes,0));
					queue_pop(regTag->colaLocks);
					destruir_lock(lockTemp);
				}
			}
		}

		}else{
			//Si la cola de locks esta vacio creo un lock temporal para asignarlo a la TAG
			agregar_lock_activo(regTag,NOASIGNADO);
			//HACER LUEGO METODO PARA DESTRUIR ARCHIVO
			//dictionary_remove_and_destroy(t_dictionary *, char *, void(*element_destroyer)(void*));
			//dictionary_remove(diccionarioArchivos,archivo);//VER SI ENREALIDAD SE SACA DE LA GLOBAL
		}
	}

}

void f_seek(PCB* proceso, char * archivo,uint32_t puntero){
	registro_tap *registroTap = get_reg_tap(proceso->tablaArchivos,archivo);
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

