#include "Memoria.h"

t_log* logger;
t_config* config;
pthread_t hiloRecibirCliente;
int serverMemoria;
t_dictionary *archivosCargados;
pthread_mutex_t mutexLog,mutexArchivos;
sem_t sem_conexion;
int main() {
//	logger = malloc(sizeof(t_log)); No es necesario
//	config = malloc(sizeof(t_config));

	logger = iniciar_logger("./log.log");
	config = iniciar_config("./Memoria.config");

	char * puertoEscucha;


	//Semaforos
	pthread_mutex_init(&mutexLog,NULL);
	pthread_mutex_init(&mutexArchivos,NULL);
	sem_init(&sem_conexion,0,0);
	sem_init(&sem_bloquesSwap,0,0);
	sem_init(&sem_paginaSwap,0,0);
	sem_init(&sem_escribirSwap,0,0);
	sem_init(&sem_escribirBloque,0,0);
	//CONFIGURACION DE MEMORIA
	puertoEscucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	tamMemoria = config_get_int_value(config,"TAM_MEMORIA");
	tamPagina = config_get_int_value(config,"TAM_PAGINA");
	pathInstrucciones = config_get_string_value(config,"PATH_INSTRUCCIONES");
	retardoRespuesta = config_get_int_value(config,"RETARDO_RESPUESTA");
	algoritmoReemplazo = config_get_string_value(config,"ALGORITMO_REEMPLAZO");
	iniciar_memoria_usuario();
	//INICIAR SERVIDOR
	serverMemoria = iniciar_servidor(puertoEscucha);
	archivosCargados=dictionary_create();
	//printf("%ld \n %ld", (long)getpid(), (long)getppid());

	debug("Servidor listo para recibir al cliente");
	//conexionFileSystem=crear_conexion(ipFyleSystem, puertoFyleSystem,MEMORIA);
	//pthread_t hiloFileSystem;
	//pthread_create(&hiloFileSystem,NULL,manejar_cliente,&conexionFileSystem);

	//LA ESPERA DE CLIENTE SE PUEDE ENCAPSULAR PERO NO ES PRIORIDAD DE MOMENTO
	int resultado;
	if ((resultado=pthread_create(&hiloRecibirCliente,NULL,(void *)recibir_conexiones,( void *) &serverMemoria))!=0)
		printf("Error al crear hilo. resultado %d",resultado);
	sem_wait(&sem_conexion);
	debug("Conexion con file system");
	pthread_join(hiloRecibirCliente,NULL);
	return EXIT_SUCCESS;
}

void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
	int conexion = *(int*) (list_get(mensaje,list_size(mensaje)-1));
	if(!strcasecmp(msg,"FS")){
		conexionFS = conexion;
		sem_post(&sem_conexion);
	}
	//Seria excelente cuanto menos aprovechar que dentro de la lista "mensaje" se encuentra al final el socket para dividir con un switch las funciones
	if(!strcasecmp(msg,"tamanioPagina")){
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"tamanioPagina",sizeof("tamanioPagina"));
		agregar_a_paquete(paquete,&tamPagina,sizeof(int));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	if(!strcasecmp(msg,"mov_in")){
		uint32_t direccionFisica=*(uint32_t*)list_get(mensaje,1);
		bool error=true;
		uint32_t dato=-1;
		//valida si es una direccion fisica valida
		if(direccionFisica<tamMemoria){
		error=false;
		escritura_log(string_from_format("PID: %d - Accion: LEER - Direccion fisica: %d",*(uint32_t*)list_get(mensaje,2),direccionFisica));
		dato=get_dato(*(uint32_t*)list_get(mensaje,1));
		}
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"mov_in",sizeof("mov_in"));
		agregar_a_paquete(paquete,&error,sizeof(bool));
		agregar_a_paquete(paquete,&dato,sizeof(uint32_t));
		usleep(retardoRespuesta*1000);
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	if(!strcasecmp(msg,"mov_out")){
		uint32_t direccionFisica=*(uint32_t*)list_get(mensaje,1);
		bool error=true;
		if(direccionFisica<tamMemoria){
			error=false;
			escritura_log(string_from_format("PID: %d - Accion: ESCRIBIR - Direccion fisica: %d",*(uint32_t*)list_get(mensaje,3),direccionFisica));
			set_dato(*(uint32_t*)list_get(mensaje,1),*(uint32_t*)list_get(mensaje,2));
		}
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"mov_out",sizeof("mov_out"));
		agregar_a_paquete(paquete,&error,sizeof(bool));
		usleep(retardoRespuesta*1000);
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	if(!strcasecmp(msg,"pageFault")){
		uint32_t pid, numPagina;
		pid = *(uint32_t*)list_get(mensaje,1);
		numPagina = *(uint32_t*)list_get(mensaje,2);
	//	debug(string_from_format("PAGE FAULT -> PID: %d, Numero de pagina: %d", pid,numPagina));
		page_fault(pid,numPagina);
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"paginaCargada",sizeof("paginaCargada"));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	if(!strcasecmp(msg,"cargar")){//cambiar cargar por crear proceso, y ver si se puede encapsular la parte de instrucciones

		uint32_t pid=*(uint32_t*)list_get(mensaje,1);
		char* path=string_new();
		string_append(&path,(char*)list_get(mensaje,2));

		uint32_t size=*(uint32_t*)list_get(mensaje,3);
		t_list* instrucciones=list_create();
		char* temp = string_new();
		string_append(&temp, pathInstrucciones);
		string_append(&temp, "/");
		string_append(&temp,path);
		FILE* prueba = fopen(temp,"r");
		bool yes = false;
		if(prueba!=NULL){
			yes = true;
			fclose(prueba);
		}
		free(temp);
		if(yes){
			instrucciones=cargar_instrucciones(&path);
			//list_add(instrucciones,"1");
			pthread_mutex_lock(&mutexArchivos);
			dictionary_put(archivosCargados,string_itoa(pid),instrucciones);
			pthread_mutex_unlock(&mutexArchivos);
			//crea las estructuras para la memoria de usuario del proceso
			crear_proceso(pid,path,size);
			//Acordarse liberar diccionario
		}

		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"cargado",sizeof("cargado"));
		agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
		if(yes){
			agregar_a_paquete(paquete,"yes",sizeof("yes"));

		}else{
			agregar_a_paquete(paquete,"not",sizeof("not"));

		}

		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
		//free(instrucciones); puede que esto sea mejor porque el dictionary put te dice que el elemento no se libera
		free(path);
	}
	if(!strcasecmp(msg,"finalizar_proceso")){
		uint32_t pid=*(uint32_t*)list_get(mensaje,1);
		//liberar diccionario de instrucciones
		finalizar_proceso(pid);
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"procesoFinalizado",sizeof("procesoFinalizado"));
		agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	 if(!strcasecmp(msg,"instruccion")){
		uint32_t pid =*(uint32_t*)list_get(mensaje,1);
		uint32_t pc =*(uint32_t*)list_get(mensaje,2);

		pthread_mutex_lock(&mutexArchivos);
		t_list* listaInstrucciones =(t_list*)dictionary_get(archivosCargados,string_itoa(pid));
		pthread_mutex_unlock(&mutexArchivos);

		char* instruccion=string_new();
		string_append(&instruccion,(char*)list_get(listaInstrucciones,pc));
		t_paquete* paquete=crear_paquete();
		agregar_a_paquete(paquete,"instruccion",sizeof("instruccion"));
		if(!strcasecmp(instruccion,"EXIT")){
			agregar_a_paquete(paquete,instruccion,strlen(instruccion)+1);
		}else{
			char** parametros = string_array_new();
			parametros = string_n_split(instruccion,3," ");
			for(int i = 0; parametros[i]!=NULL; i++){
				agregar_a_paquete(paquete,parametros[i],strlen(parametros[i])+1);
			}
		}
		usleep(retardoRespuesta*1000);
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
		free(instruccion);
	}
	if(!strcasecmp(msg,"marco")){
		uint32_t pid = *(uint32_t*)list_get(mensaje,1);
		uint32_t numeroPagina = *(uint32_t*)list_get(mensaje,2);
		bool pageFault = true;
		uint32_t marco = -1;

	//	debug(string_from_format("PID: %s, Numero de pagina: %s",string_itoa(pid),string_itoa(numeroPagina)));

		pagina_t * pagina = pagina_get(pid,numeroPagina);
		if(pagina!=NULL){
			//Talvez habria que liberarlos
			pthread_mutex_lock(pagina->mutexPagina);
			escritura_log(string_from_format("PID: %d - Pagina: %d - Marco: %d",pid,numeroPagina,pagina->marco));
			if(pagina->p == 1){
				marco = pagina->marco;
				pthread_mutex_unlock(pagina->mutexPagina);

				pthread_mutex_lock(&mutexInversa);
				pagina_global_t* paginaGlobal = (pagina_global_t*) list_get(tablapaginasGlobales,marco);
				pthread_mutex_unlock(&mutexInversa);

				pthread_mutex_lock(paginaGlobal->mutexGlobal);
				temporal_destroy(paginaGlobal->tiempo);
				paginaGlobal->tiempo = temporal_create();
				pthread_mutex_unlock(paginaGlobal->mutexGlobal);
				pageFault = false;
			}else{
				pthread_mutex_unlock(pagina->mutexPagina);
			}

			t_paquete* paquete=crear_paquete();
			agregar_a_paquete(paquete,"marco",sizeof("marco"));
			agregar_a_paquete(paquete,&pageFault,sizeof(bool));
			agregar_a_paquete(paquete,&marco,sizeof(uint32_t));
			usleep(retardoRespuesta*1000);
			enviar_paquete(paquete,conexion);
			eliminar_paquete(paquete);

		}

	}
	if(!strcasecmp(msg,"bloquesSwap")){
		asignar_swap(mensaje);
	}
	if(!strcasecmp(msg,"f_write")){
		usleep(retardoRespuesta*1000);
		enviar_datos_bloque(*(uint32_t*)list_get(mensaje,1));
	}
	if(!strcasecmp(msg,"f_read")){
		usleep(retardoRespuesta*1000);
		recibir_datos_bloque(*(uint32_t*)list_get(mensaje,1), (void*)list_get(mensaje,2));
		t_paquete* paquete = crear_paquete();
		agregar_a_paquete(paquete,"valid_read",sizeof("valid_read"));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	if(!strcasecmp(msg,"paginaSwap")){
		uint32_t pid = *(uint32_t*)list_get(mensaje,1);
		uint32_t numPagina =  *(uint32_t*)list_get(mensaje,2);
		void * datos = list_get(mensaje,3);
		cargar_pagina_swap(pid,numPagina,datos);
	}
	if(!strcasecmp(msg,"escribirSwap")){

		sem_post(&sem_escribirSwap);
	}
	if(!strcasecmp(msg,"liberar_swap")){
		sem_post(&sem_swapLiberado);
	}

}
