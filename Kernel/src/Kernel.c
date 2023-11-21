#include "Kernel.h"

int main(void)
{
	//MEMORY ALLOCATION
	char* ipCPU=malloc(sizeof(char*)),*ipMemoria=malloc(sizeof(char*)),*ipFileSystem=malloc(sizeof(char*)) ;
	char* puertoCPUDispatch=malloc(sizeof(char*)),*puertoCPUInterrupt=malloc(sizeof(char*)),
		* puertoMemoria=malloc(sizeof(char*)),*puertoFileSystem=malloc(sizeof(char*));
	char** recursos=string_array_new();
	char**instancias=string_array_new();
	//logger=malloc(sizeof(t_log));
	//config=malloc(sizeof(config));
	logger = iniciar_logger("./log.log");
	config = iniciar_config("./Kernel.config");

	/*INICIALIZAR LISTAS */
	procesos=list_create();
	colaLargo=queue_create();
	colaCorto=queue_create();
	//Inicializar_semaforos
	sem_init(&planiLargo,0,0);
	sem_init(&planiCorto,0,0);
	pthread_mutex_init(&mutexColaCorto,NULL);
	pthread_mutex_init(&mutexColaLargo,NULL);
	pthread_mutex_init(&mutexProcesos,NULL);
	pthread_mutex_init(&mutexLog,NULL);
	pthread_mutex_init(&mutexGrado,NULL);
	/************************************RECUPERA DATOS DE ARCHIVO DE CONFIGURACION************************************/
	//TALVEZ SE PUEDE GLOBALIZAR Y PASAR A UNA FUNCION PARA QUE QUEDE MEJOR PARA LA LECTURA
	//CONFIGURACION DE CPU
	ipCPU = config_get_string_value(config,"IP_CPU");
	puertoCPUDispatch=config_get_string_value(config,"PUERTO_CPU_DISPATCH");
	puertoCPUInterrupt=config_get_string_value(config,"PUERTO_CPU_INTERRUPT");

	AlgoritmoPlanificacion=config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	quantum=config_get_int_value(config,"QUANTUM");
	gradoMultiprogramacion=config_get_int_value(config,"GRADO_MULTIPROGRAMACION_INI");
	//MEJOR => atoi: (int)strtol(nptr, (char **)NULL, 10)
	//CONFIGURACION DE MEMORIA
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria=config_get_string_value(config,"PUERTO_MEMORIA");

	//CONFIGURACION DE FILESYSTEM
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoFileSystem=config_get_string_value(config,"PUERTO_FILESYSTEM");

	//CONFIGURACION DE RECURSOS
	recursos = config_get_array_value(config,"RECURSOS");
	instancias = config_get_array_value(config,"INSTANCIAS_RECURSOS");
	diccionarioRecursos=dictionary_create();
    sem_t *semaforosDeRecursos = (sem_t *)malloc(string_array_size(recursos) * sizeof(sem_t));
    //inicializar los semaforos de los recursos y meterlos en un diccionario
	 for (int i = 0; i < string_array_size(recursos); i++) {
	    	sem_init(&(semaforosDeRecursos[i]),0,atoi((instancias[i])));//revisar si esta bien como se le pasa el puntero del semaforo
			dictionary_put(diccionarioRecursos,recursos[i],&(semaforosDeRecursos[i]));
	    }

	 diccionarioArchivosGlobal=dictionary_create();
	/************************************INICIALIZAR CONEXIONES************************************/
	conexionCPUDispatch = crear_conexion(ipCPU, puertoCPUDispatch,KERNEL);
	conexionCPUInterrupt = crear_conexion(ipCPU, puertoCPUInterrupt,KERNEL);
	conexionMemoria = crear_conexion(ipMemoria, puertoMemoria,KERNEL);
	//conexionFileSystem = crear_conexion(ipFileSystem, puertoFileSystem,KERNEL);

	/************************************INICIALIZAR HILOS DE RECIBO DE MENSAJES************************************/
	//HILO DE MANEJO DE MOODULOS
	pthread_t hiloCPUDispatch;
	pthread_create(&hiloCPUDispatch,NULL,manejar_cliente,&conexionCPUDispatch);

	pthread_t  hiloCPUInterrupt;
	pthread_create(&hiloCPUInterrupt,NULL,manejar_cliente,&conexionCPUInterrupt);

	pthread_t  hiloMemoria;
	int resultado;
	if ((resultado=pthread_create(&hiloMemoria,NULL,manejar_cliente,&conexionMemoria))!=0)
		printf("Error al crear hilo. resultado %d",resultado);

	//pthread_t * hiloFilesystem;
	//pthread_create(hiloFilesystem,NULL,manejar_cliente, conexionFileSystem);

	/************************************INICIO CONSOLA INTERACTIVA*************************************************/
	pthread_t hiloConsola, hiloCorto,hiloLargo;
	pthread_create(&hiloConsola,NULL,manejar_consola,NULL);
	pthread_create(&hiloCorto,NULL,planificador_corto,NULL);
	pthread_create(&hiloLargo,NULL,planificador_largo,NULL);
	/************************************FINALIZA LOS PROGRAMAS O HILOS A FUTURO************************************/
	pthread_join(hiloCPUDispatch,NULL);
	pthread_join(hiloCPUInterrupt,NULL);
	pthread_join(hiloMemoria,NULL);
	pthread_join(hiloCorto,NULL);
	pthread_join(hiloLargo,NULL);
	pthread_join(hiloConsola,NULL);
	//pthread_join(hiloFilesystem,NULL);
	pthread_join(hiloConsola,NULL);
	return EXIT_SUCCESS;
}
void terminar_programa()
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexionCPUDispatch);
	liberar_conexion(conexionCPUDispatch);
	liberar_conexion(conexionMemoria);
	liberar_conexion(conexionFileSystem);
	list_destroy_and_destroy_elements(procesos,(void*)proceso_destroy);
	queue_destroy(colaCorto);
	queue_destroy(colaLargo);
	//terminar_hilos();
	//int conexionCPUDispatch, conexionCPUInterrupt,conexionMemoria,conexionFileSystem;
}

//MANEJO DE PROCESO
void sleep_proceso(PCB* proceso, int tiempo){
	cambiar_estado(proceso,BLOCKED);
	sleep(tiempo);
	cambiar_estado(proceso,READY);

}

//MANEJO DE RECRUSOS
void wait_recurso(PCB* proceso,char* recurso){
	cambiar_estado(proceso,BLOCKED);
	sem_t *semRecurso=(sem_t*)dictionary_get(diccionarioRecursos,recurso);
	sem_wait(semRecurso);
	cambiar_estado(proceso,READY);
}

void signal_recurso(PCB* proceso,char* recurso){
	cambiar_estado(proceso,BLOCKED);
	sem_t *semRecurso=(sem_t*)dictionary_get(diccionarioRecursos,recurso);
	sem_post(semRecurso);
	cambiar_estado(proceso,READY);
}
int motivo_desalojo(char * desalojo){
	int motivo = EXIT;
	if(!strcasecmp(desalojo,"procesoExit")) return PROCESOEXIT;
	if(!strcasecmp(desalojo,"prioridades")) return PRIORIDADES;
	if(!strcasecmp(desalojo,"quantum")) return ROUNDROBIN;
	return motivo;
}
//MANEJO DE FILE SYSTEM
void abrir_archivo(char* archivo){
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

void actualizar_tablas(PCB* proceso,char* archivo){
	//VALIDO SI ESTA EN LA TABLA DE ARCHIVOS GLOBAL
	//si esta el archivo en global lo recupera
	archivo_t *nuevoArchivo=malloc(sizeof(archivo_t*));
	if(dictionary_has_key(diccionarioArchivosGlobal,archivo)){
		nuevoArchivo=dictionary_get(diccionarioArchivosGlobal,archivo);
		//ver si va aca esta validacion del semaforo o no
		if(!semaforo_bloquedo((nuevoArchivo->semaforoLectura))){
			cambiar_estado(proceso,BLOCKED);
			sem_wait(&(nuevoArchivo->semaforoLectura));
		}
	}else{
		//sino esta en global lo crea y lo añade
		nuevoArchivo->nombreArchivo=archivo;
		nuevoArchivo->puntero=0;
		pthread_mutex_init(&(nuevoArchivo->semaforoLectura),NULL);
		pthread_mutex_init(&(nuevoArchivo->semaforoEscritura),NULL);
		dictionary_put(diccionarioArchivosGlobal,archivo,&nuevoArchivo);
		abrir_archivo(archivo);
	}
	//CARGAR ARCHIVO EN TABLA GLOBAL Y EN TABLA DE PROCESO
	//SI EXISTE EL DICCIONARIO DEL PROCESO
	t_dictionary * diccionarioProceso;
	if(dictionary_has_key(diccionarioDeDiccionariosLocales,string_itoa(proceso->pid))){
		diccionarioProceso=(t_dictionary *)dictionary_get(diccionarioDeDiccionariosLocales,string_itoa(proceso->pid));
		//si no esta abierto lo añade
		if(!dictionary_has_key(diccionarioProceso,archivo)){
				dictionary_put(diccionarioProceso,archivo,&nuevoArchivo);
		}
	}
	else{
		//si no existe lo crea  y añade el archivo
		diccionarioProceso=dictionary_create();
		dictionary_put(diccionarioProceso,archivo,&nuevoArchivo);
		dictionary_put(diccionarioDeDiccionariosLocales,archivo,&diccionarioProceso);
	}
}

void f_open(PCB* proceso,char * archivo,char* modoApertura){
	//NUEVA LOGICA LUEGO DE RELEER EL ENUNCIADO DE VARIAS VECES
	if(modoApertura=="L"){
		if(validar_lock_escritura(archivo)){
			cambiar_estado(proceso,BLOCKED);
			sem_wait(&(get_lock_escritura(archivo)));
		}
		else{
			if(validar_lock_lectura(archivo)){
				agregar_proceso_como_participante(proceso);
				//sem_wait(&(get_lock_lectura()));//revisar si seria un wait el agregar participante
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
}

void f_close(PCB* proceso,char * archivo){
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
}

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


void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
	//Seria excelente cuanto menos aprovechar que dentro de la lista "mensaje" se encuentra al final el socket para dividir con un switch las funciones

	if(!strcasecmp(msg,"cargado")){
		int pid = *(int*)list_get(mensaje,1);

		pthread_mutex_lock(&mutexProcesos);
		PCB * proceso = list_get(procesos,(pid-1));
		pthread_mutex_unlock(&mutexProcesos);

		pthread_mutex_lock(&mutexColaLargo);
		queue_push(colaLargo,proceso);
		pthread_mutex_unlock(&mutexColaLargo);

		char * mensaje = string_from_format("Se crea el proceso %d en NEW",proceso->pid);
		hilo_funcion(mensaje,(void*)escritura_log);
		if(!detenida){
				sem_post(&planiLargo);
		}
	}
	if(!strcasecmp(msg,"sleep")){
		uint32_t tiempo = *(uint32_t*)list_get(mensaje,1);
		uint32_t pid = *(uint32_t*)list_get(mensaje,2);
		pthread_mutex_lock(&mutexProcesos);
		PCB* temp = (PCB*)list_get(procesos,pid-1);
		pthread_mutex_unlock(&mutexProcesos);
		deserializar_proceso(temp,mensaje,2);
		sleep_proceso(temp,tiempo);
	}
	if(!strcasecmp(msg,"wait")){
		char* recurso = *(char*)list_get(mensaje,1);
		uint32_t pid = *(uint32_t*)list_get(mensaje,2);
		pthread_mutex_lock(&mutexProcesos);
		PCB* temp = (PCB*)list_get(procesos,pid-1);
		pthread_mutex_unlock(&mutexProcesos);
		deserializar_proceso(temp,mensaje,2);
		wait_recurso(temp,recurso);
	}
	if(!strcasecmp(msg,"signal")){
		char* recurso = *(char*)list_get(mensaje,1);
		uint32_t pid = *(uint32_t*)list_get(mensaje,2);
		pthread_mutex_lock(&mutexProcesos);
		PCB* temp = (PCB*)list_get(procesos,pid-1);
		pthread_mutex_unlock(&mutexProcesos);
		deserializar_proceso(temp,mensaje,2);
		signal_recurso(temp,recurso);
	}

	if(!strcasecmp(msg,"contexto")){
		int motivo = motivo_desalojo((char*)list_get(mensaje,1));
		uint32_t pid = *(uint32_t*)list_get(mensaje,2);

		pthread_mutex_lock(&mutexProcesos);
		PCB* temp = (PCB*)list_get(procesos,pid-1);
		pthread_mutex_unlock(&mutexProcesos);

		deserializar_proceso(temp,mensaje,2);
		switch(motivo){
		case PROCESOEXIT:
			hilo_funcion(temp,(void*)planificador_largo_salida);

			break;
		case PRIORIDADES:
			cambiar_estado(temp,READY);
			sem_post(&contexto);
			break;
		case ROUNDROBIN:
			cambiar_estado(temp,READY);
			pthread_mutex_lock(&mutexColaCorto);
			queue_push(colaCorto,temp);
			pthread_mutex_unlock(&mutexColaCorto);

			pthread_mutex_lock(&mutexEjecutando);
			ejecutandoB = false;
			pthread_mutex_unlock(&mutexEjecutando);
			sem_post(&planiCorto);
			break;
		case EXIT:
			error_show("Motivo desconocido");
		}
	}
//	if(!strcasecmp(msg,"instruccion") && !strcasecmp(AlgoritmoPlanificacion, "round_robin\0")){
//		sem_post(&clockT);
//	}
	free(msg);
}


