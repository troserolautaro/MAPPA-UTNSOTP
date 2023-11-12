#include "Kernel.h"

int main(void)
{
	//MEMORY ALLOCATION
	char* ipCPU=malloc(sizeof(char*)),*ipMemoria=malloc(sizeof(char*)),*ipFileSystem=malloc(sizeof(char*)) ;
	char* puertoCPUDispatch=malloc(sizeof(char*)),*puertoCPUInterrupt=malloc(sizeof(char*)),
		* puertoMemoria=malloc(sizeof(char*)),*puertoFileSystem=malloc(sizeof(char*));
	char** recursos=string_array_new();
	char**instancias=string_array_new();
	logger=malloc(sizeof(t_log));
	config=malloc(sizeof(config));
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
	        printf("recurso %d: %s, instancias: %s \n", i, recursos[i],instancias[i]);
	    	sem_init(&(semaforosDeRecursos[i]),0,atoi((instancias[i])));//revisar si esta bien como se le pasa el puntero del semaforo
			dictionary_put(diccionarioRecursos,recursos[i],&(semaforosDeRecursos[i]));
	    }

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

void sleep_proceso(PCB* proceso, int tiempo){
	cambiar_estado(proceso,BLOCKED);
	sleep(tiempo);
	cambiar_estado(proceso,READY);

}

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

void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
	//Seria excelente cuanto menos aprovechar que dentro de la lista "mensaje" se encuentra al final el socket para dividir con un switch las funciones
	if(!strcasecmp(msg,"procesoExit")){
		uint32_t pid = *(uint32_t*)list_get(mensaje,1);

		pthread_mutex_lock(&mutexProcesos);
		PCB* temp = (PCB*)list_get(procesos,pid-1);
		pthread_mutex_unlock(&mutexProcesos);

		deserializar_proceso(temp,mensaje,1);
		planificador_largo_salida(temp);

	}
	if(!strcasecmp(msg,"cargado")){
		int pid = *(int*)list_get(mensaje,1);

		pthread_mutex_lock(&mutexProcesos);
		PCB * proceso = list_get(procesos,(pid-1));
		pthread_mutex_unlock(&mutexProcesos);

		pthread_mutex_lock(&mutexColaLargo);
		queue_push(colaLargo,proceso);
		pthread_mutex_unlock(&mutexColaLargo);

		char * mensaje = string_from_format("Se crea el proceso %d en NEW",proceso->pid);
		hilo_funcion(mensaje,escritura_log);

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

	}
	free(msg);
}
