#include "Kernel.h"

int main(void)
{
	//MEMORY ALLOCATION
	char* ipCPU,*ipMemoria,*ipFileSystem;
	char* puertoCPUDispatch,*puertoCPUInterrupt,
		* puertoMemoria,*puertoFileSystem;
	char** recursos=string_array_new();
	char**temp=string_array_new();
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
	sem_init(&desalojo_signal,0,0);
	sem_init(&paginaCargada,0,0);
	sem_init(&sem_archivoCreado,0,0);
	sem_init(&sem_truncado,0,0);
	sem_init(&sem_write,0,0);
	sem_init(&sem_read,0,0);
	pthread_mutex_init(&mutexColaCorto,NULL);
	pthread_mutex_init(&mutexColaLargo,NULL);
	pthread_mutex_init(&mutexProcesos,NULL);
	pthread_mutex_init(&mutexLog,NULL);
	pthread_mutex_init(&mutexGrado,NULL);
	pthread_mutex_init(&mutexRecursos,NULL);
	pthread_mutex_init(&mutexGlobal,NULL);
	iniciar_KernelMemoria();
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
	ipFileSystem = config_get_string_value(config,"IP_MEMORIA");
	puertoFileSystem=config_get_string_value(config,"PUERTO_FILESYSTEM");

	//CONFIGURACION DE RECURSOS
	recursos = config_get_array_value(config,"RECURSOS");
	temp = config_get_array_value(config,"INSTANCIAS_RECURSOS");
	diccionarioRecursos = dictionary_create();
    //inicializar los semaforos de los recursos y meterlos en un diccionario
	 for (int i = 0; i < string_array_size(recursos); i++) {
		 t_list* elements = list_create();
		 t_queue * colaEspera= queue_create();
		 int* instancia = malloc(sizeof(int));
		 *instancia =  (int)strtol(temp[i], (char **)NULL, 10);
		 list_add(elements,instancia);
		 list_add(elements,colaEspera);
		 pthread_mutex_lock(&mutexRecursos);
		 dictionary_put(diccionarioRecursos,recursos[i],elements);
		 pthread_mutex_unlock(&mutexRecursos);
	}
	/************************************INICIALIZAR CONEXIONES************************************/
	conexionCPUDispatch = crear_conexion(ipCPU, puertoCPUDispatch,KERNEL);
	conexionCPUInterrupt = crear_conexion(ipCPU, puertoCPUInterrupt,KERNEL);
	conexionMemoria = crear_conexion(ipMemoria, puertoMemoria,KERNEL);
	conexionFileSystem = crear_conexion(ipFileSystem, puertoFileSystem,KERNEL);

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

	pthread_t hiloFilesystem;
	pthread_create(&hiloFilesystem,NULL,manejar_cliente, &conexionFileSystem);

	/************************************INICIO CONSOLA INTERACTIVA*************************************************/
	pthread_t hiloConsola, hiloCorto,hiloLargo;
	pthread_create(&hiloConsola,NULL,manejar_consola,NULL);
	pthread_create(&hiloCorto,NULL,planificador_corto,NULL);
	pthread_create(&hiloLargo,NULL,planificador_largo,NULL);
	/************************************FINALIZA LOS PROGRAMAS O HILOS A FUTURO************************************/
	pthread_join(hiloConsola,NULL);
	pthread_join(hiloCPUDispatch,NULL);
	pthread_join(hiloCPUInterrupt,NULL);
	pthread_join(hiloMemoria,NULL);
	pthread_join(hiloCorto,NULL);
	pthread_join(hiloLargo,NULL);
	pthread_join(hiloFilesystem,NULL);
	terminar_programa();
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


void bloquear_proceso(PCB* proceso,char* motivo){

	cambiar_estado(proceso,BLOCKED);

	//“PID: <PID> - Bloqueado por: <SLEEP / NOMBRE_RECURSO / NOMBRE_ARCHIVO>”
	char* mensaje = string_from_format("PID: %d - Bloqueado por: %s",proceso->pid,motivo);
	escritura_log(mensaje);
	free(mensaje);
	if(dictionary_has_key(diccionarioRecursos,motivo)) {
		pthread_mutex_lock(&mutexRecursos);
		deteccion_deadlock(proceso);
		pthread_mutex_unlock(&mutexRecursos);
	}


}
//MANEJO DE PROCESO
void sleep_proceso(void* parametros){
	PCB* proceso = (PCB*)list_get((t_list*)parametros,1);
	int tiempo = atoi((char*)list_get((t_list*) parametros,0));
	bloquear_proceso(proceso,"SLEEP");
	sleep(tiempo);
	push_colaCorto(proceso);
	sem_post(&planiCorto);

}

//MANEJO DE RECRUSOS
void wait_recurso(PCB* proceso,char* recurso){
	pthread_mutex_lock(&mutexRecursos);
	if(dictionary_has_key(diccionarioRecursos,recurso)){
		t_list* elements = (t_list*)dictionary_get(diccionarioRecursos,recurso);
		int* instancias = (int*)list_get(elements,0);
		if(*instancias > 0 ){
			*instancias-= 1;

			list_add(proceso->recursos,recurso);
			char* mensaje = string_from_format("PID: %d - Wait: %s - Instancias: %d",proceso->pid,recurso,*instancias);
			pthread_mutex_unlock(&mutexRecursos);

			escritura_log(mensaje);
			free(mensaje);
			char* mensajeCola = string_from_format("Cola Ready %s: ",AlgoritmoPlanificacion);

			pthread_mutex_lock(&mutexColaCorto);
			queue_push(colaCorto,proceso);
			iterar_lista(&mensajeCola);
			pthread_mutex_unlock(&mutexColaCorto);
			sem_post(&planiCorto);

			escritura_log(mensajeCola);
			free(mensajeCola);
		}else{
			t_queue* colaEspera = (t_queue*)list_get(elements,1);
			queue_push(colaEspera,proceso);
			pthread_mutex_unlock(&mutexRecursos);
			bloquear_proceso(proceso,recurso);
		}

	}else{
		pthread_mutex_unlock(&mutexRecursos);
		planificador_largo_salida(proceso,"INVALID_RESOURCE");
	}
	sem_post(&planiCorto);

}
void ordenar_adelante(PCB* proceso){
	char* mensajeCola = string_from_format("Cola Ready %s: ",AlgoritmoPlanificacion);
	pthread_mutex_lock(&mutexColaCorto);
	if(!queue_is_empty(colaCorto)){
		t_list * listaNueva=list_create();
		list_add(listaNueva,proceso);
		list_add_all(listaNueva,colaCorto->elements);
		list_destroy(colaCorto->elements);
		colaCorto->elements=listaNueva;
	}else{
		queue_push(colaCorto,proceso);
	}
	iterar_lista(&mensajeCola);
	pthread_mutex_unlock(&mutexColaCorto);
	escritura_log(mensajeCola);
	free(mensajeCola);
}
void signal_recurso(PCB* proceso,char* recurso){
	pthread_mutex_lock(&mutexRecursos);
	if(dictionary_has_key(diccionarioRecursos,recurso) && !list_is_empty(proceso->recursos)){
		bool buscar_recurso(void* element){
			return (!strcasecmp((char*)element,recurso)) ? true : false;
		}
		char* recurso = (char*)list_remove_by_condition(proceso->recursos,buscar_recurso);
		//debug(string_from_format("Signal: %s",recurso));
		if(recurso!=NULL){

			t_list* elements = (t_list*)dictionary_get(diccionarioRecursos,recurso);
			int* instancias = (int*)list_get(elements,0);
			*instancias += 1;
			ordenar_adelante(proceso); //Pone el proceso adelante de la colaCOrto

 /*---------------------------------*/
			t_queue* colaEspera = (t_queue*)list_get(elements,1);
			if(!queue_is_empty(colaEspera)){

				PCB* temp = (PCB*) queue_pop(colaEspera);
				*instancias -=1;
				list_add(temp->recursos,recurso);

				push_colaCorto(temp);
			}
			char* mensaje = string_from_format("PID: %d - Signal: %s - Instancias: %d",proceso->pid,recurso,*instancias);
			pthread_mutex_unlock(&mutexRecursos);
			escritura_log(mensaje);
			free(mensaje);

			pthread_mutex_lock(&mutexEjecutando);
			if(ejecutandoB){
				pthread_mutex_unlock(&mutexEjecutando);
				enviar_interrupcion_cpu_sin_pid("desalojo_signal");
			}else{
				pthread_mutex_unlock(&mutexEjecutando);
				sem_post(&planiCorto);
			}

		}else{
			pthread_mutex_unlock(&mutexRecursos);
			planificador_largo_salida(proceso,"INVALID_RESOURCE");
		}
	}else{
		pthread_mutex_unlock(&mutexRecursos);
		planificador_largo_salida(proceso,"INVALID_RESOURCE");
	}


}
int motivo_desalojo(char * desalojo){
	int motivo = EXIT;
	if(!strcasecmp(desalojo,"procesoExit")) return PROCESOEXIT;
	if(!strcasecmp(desalojo,"prioridades")) return PRIORIDADES;
	if(!strcasecmp(desalojo,"quantum")) return ROUNDROBIN;
	if(!strcasecmp(desalojo,"wait")) return WAIT;
	if(!strcasecmp(desalojo,"signal")) return SIGNAL;
	if(!strcasecmp(desalojo,"sleep")) return SLEEP;
	if(!strcasecmp(desalojo,"desalojo_signal"))return DESALOJO_SIGNAL;
	if(!strcasecmp(desalojo,"page_fault")) return PAGE_FAULT;
	if(!strcasecmp(desalojo,"f_open")) return F_OPEN;
	if(!strcasecmp(desalojo,"f_close")) return F_CLOSE;
	if(!strcasecmp(desalojo,"f_write")) return F_WRITE;
	if(!strcasecmp(desalojo,"f_read")) return F_READ;
	if(!strcasecmp(desalojo,"f_truncate")) return F_TRUNCATE;
	if(!strcasecmp(desalojo,"f_seek")) return F_SEEK;
	if(!strcasecmp(desalojo,"TERMINATED")) return SIGK;


	return motivo;
}

void respuesta(){
	t_paquete* paquete = crear_paquete();
	agregar_a_paquete(paquete,"respuesta",sizeof("respuesta"));
	enviar_paquete(paquete,conexionCPUDispatch);
	eliminar_paquete(paquete);
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


		if(!strcasecmp((char*)list_get(mensaje,2),"yes")){
			pthread_mutex_lock(&mutexColaLargo);
			queue_push(colaLargo,proceso);
			pthread_mutex_unlock(&mutexColaLargo);

			escritura_log(string_from_format("Se crea el proceso %d en NEW",proceso->pid));

			pthread_mutex_lock(&mutexDetenida);
			if(!detenida){
					sem_post(&planiLargo);
			}
			pthread_mutex_unlock(&mutexDetenida);
		}else{
			pthread_mutex_lock(&mutexProcesos);
			PCB* proceso = list_remove(procesos,pid-1);
			pthread_mutex_unlock(&mutexProcesos);
			proceso_destroy(proceso);
			pthread_mutex_lock(&mutexGlobal);
			PIDGLOBAL --;
			pthread_mutex_unlock(&mutexGlobal);

		}
		sem_post(&procesoCargado);

	}


	if(!strcasecmp(msg,"contexto")){
		int motivo = motivo_desalojo((char*)list_get(mensaje,1));
		int posInicio = (*(int*)(list_get(mensaje,list_size(mensaje)-2)));
		uint32_t pid = (*(uint32_t*)list_get(mensaje,posInicio))-1;
		uint32_t direccionFisica,puntero;
		char* archivo;
		t_list* params;

		pthread_mutex_lock(&mutexEjecutando);
		ejecutandoB = false;
		pthread_mutex_unlock(&mutexEjecutando);

		pthread_mutex_lock(&mutexProcesos);
		PCB* proceso = (PCB*)list_get(procesos,pid);
		pthread_mutex_unlock(&mutexProcesos);
		if(proceso->estado!=TERMINATED){
		pthread_mutex_lock(proceso->mutex);
		deserializar_proceso(proceso,mensaje,posInicio);
		pthread_mutex_unlock(proceso->mutex);
		switch(motivo){
		case PROCESOEXIT:
				planificador_largo_salida(proceso,"SUCCESS");
				respuesta();
			break;

		case PRIORIDADES:
				if(proceso->estado==EXEC){
					push_colaCorto(proceso);
				}
				sem_post(&contexto);
				break;

		case ROUNDROBIN:
				if(proceso->estado==EXEC){
					char* mensaje = string_from_format("Fin de Quantum: PID: %d - Desalojado por fin de Quantum",proceso->pid);
					escritura_log(mensaje);
					free(mensaje);
					push_colaCorto(proceso);
				}
				sem_post(&planiCorto);
			break;

		case WAIT:{
				cambiar_estado(proceso,READY);
				char* recurso = (char*)list_get(mensaje,2);
				wait_recurso(proceso,recurso);
				respuesta();
				//free(recurso);
			break;
		}
		case SLEEP:
				cambiar_estado(proceso,READY);
				sem_post(&planiCorto);
				t_list* parametros = list_create();
				list_add(parametros,list_get(mensaje,2));
				list_add(parametros, proceso);
				hilo_funcion(parametros,(void*)sleep_proceso);
				respuesta();
			break;

		case SIGNAL:{
				cambiar_estado(proceso,READY);
				sem_post(&planiCorto);
				char* recurso = (char*)list_get(mensaje,2);
				signal_recurso(proceso,recurso);
				respuesta();
				//free(recurso);
		}
			break;
		case DESALOJO_SIGNAL:
				sem_post(&planiCorto);
			break;
		case PAGE_FAULT:
				escritura_log(string_from_format("Page Fault PID: %d - Pagina: %s", proceso->pid,(char*)list_get(mensaje,2)));
				t_list* parameters = list_create();
				list_add(parameters, proceso);
				list_add(parameters,list_get(mensaje,2));
				hilo_funcion(parameters,(void*)page_fault);
				respuesta();
				sem_post(&planiCorto);

			break;
		case F_OPEN:
			cambiar_estado(proceso,READY);
			respuesta();
			int modoApertura;
			char* archivo = (char*)list_get(mensaje,2);
			if(!strcasecmp((char*)list_get(mensaje,3),"W"))modoApertura=ESCRITURA;
			else modoApertura=LECTURA;
			bool bloqueado = f_open(proceso,archivo,modoApertura);
			if(!bloqueado){
				escritura_log(string_from_format("PID: %d - Abrir Archivo: %s", proceso->pid,(char*)list_get(mensaje,2)));
				ordenar_adelante(proceso); //Pone el proceso adelante de la colaCorto
			}
			sem_post(&planiCorto);

			/*
			t_list* parameters = list_create();
			list_add(parameters, proceso);
			list_add(parameters,list_get(mensaje,2));//nombre archivo
			list_add(parameters,list_get(mensaje,3));//modo apertura
			hilo_funcion(parameters,(void*)f_open);
			//sem_post(&planiCorto);
			 */
			break;
		case F_CLOSE:
			cambiar_estado(proceso,READY);
			respuesta();
			escritura_log(string_from_format("PID: %d - Cerrar Archivo: %s", proceso->pid,(char*)list_get(mensaje,2)));
			f_close(proceso,(char*)list_get(mensaje,2));
			ordenar_adelante(proceso); //Pone el proceso adelante de la colaCorto
			sem_post(&planiCorto);

			break;
		case F_SEEK:
			cambiar_estado(proceso,READY);
			respuesta();
			puntero=strtol((char*)list_get(mensaje,3),NULL,10);
			escritura_log(string_from_format("PID: %d -  Actualizar puntero Archivo: %s - Puntero: %d" , proceso->pid,(char*)list_get(mensaje,2),puntero));
			f_seek(proceso,(char*)list_get(mensaje,2),puntero);
			ordenar_adelante(proceso); //Pone el proceso adelante de la colaCorto
			sem_post(&planiCorto);
			break;
		case F_WRITE:
			archivo = (char*)list_get(mensaje,2);
			params = list_create();
			list_add(params,proceso);
			list_add(params,archivo);
			list_add(params,list_get(mensaje,3));
			hilo_funcion(params,(void*)f_write);
			respuesta();
			break;
		case F_READ:
			archivo = (char*)list_get(mensaje,2);
			params = list_create();
			list_add(params,proceso);
			list_add(params,archivo);
			list_add(params,list_get(mensaje,3));
			hilo_funcion(params,(void*)f_read);
			respuesta();
			break;
		case F_TRUNCATE:
			archivo = (char*)list_get(mensaje,2);
			uint32_t size = (uint32_t)strtol((char*)list_get(mensaje,3),NULL,10);
			escritura_log(string_from_format("PID: %d - Archivo: %s - Tamaño %d", proceso->pid,(char*)list_get(mensaje,2),size));
			params = list_create();
			list_add(params,proceso);
			list_add(params,archivo);
			list_add(params,list_get(mensaje,3));
			hilo_funcion(params,(void*)f_truncate);
			respuesta();//Solucion lista semaforos por proceso
			break;
		case SIGK:
			cambiar_estado(proceso,READY);
			proceso_terminado();
			break;
		case EXIT:
			error_show("Motivo desconocido");

		}
		}
	}

	if(!strcasecmp(msg,"paginaCargada")){
		sem_post(&paginaCargada);
	}
	if(!strcasecmp(msg,"tamanio")){
		char* archivo = (char*)list_get(mensaje,1);
		uint32_t tamanio = *(uint32_t*)list_get(mensaje,2);
		tamanio_func(archivo,tamanio);
		free(archivo);
		sem_post(&sem_archivoCreado);
		}
	if(!strcasecmp(msg,"f_truncate")){
			sem_post(&sem_truncado);
		}
	if(!strcasecmp(msg,"f_read")){
		sem_post(&sem_read);
	}
	if(!strcasecmp(msg,"valid_read")){
		sem_post(&sem_read);
		}
	if(!strcasecmp(msg,"valid_write")){
		//debug("se escribio el archivo");
		sem_post(&sem_write);
	}
	if(!strcasecmp(msg,"procesoFinalizado")){
		procesoFinalizado();
	}
	free(msg);
}



