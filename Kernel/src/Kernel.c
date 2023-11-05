#include "Kernel.h"

int main(void)
{
	//MEMORY ALLOCATION
	char* ipCPU=malloc(sizeof(char*)),*ipMemoria=malloc(sizeof(char*)),*ipFileSystem=malloc(sizeof(char*)) ;
	char* puertoCPUDispatch=malloc(sizeof(char*)),*puertoCPUInterrupt=malloc(sizeof(char*)),
		* puertoMemoria=malloc(sizeof(char*)),*puertoFileSystem=malloc(sizeof(char*));

	logger=malloc(sizeof(t_log));
	config=malloc(sizeof(config));
	logger = iniciar_logger(/*nombre config por estar en utils*/);
	config = iniciar_config(/*nombre config por estar en utils*/);

	/*INICIALIZAR LISTAS */
	procesos=list_create();
	colaLargo=queue_create();
	colaCorto=queue_create();
	//Inicializar_semaforos
	pthread_mutex_init(&mutexPlaniLargo,NULL);
	pthread_mutex_init(&mutexPlaniCorto,NULL);
	pthread_mutex_init(&mutexColaCorto,NULL);
	pthread_mutex_init(&mutexColaLargo,NULL);
	/************************************RECUPERA DATOS DE ARCHIVO DE CONFIGURACION************************************/
	//TALVEZ SE PUEDE GLOBALIZAR Y PASAR A UNA FUNCION PARA QUE QUEDE MEJOR PARA LA LECTURA
	//CONFIGURACION DE CPU
	ipCPU = config_get_string_value(config,"IP_CPU");
	puertoCPUDispatch=config_get_string_value(config,"PUERTO_CPU_DISPATCH");
	puertoCPUInterrupt=config_get_string_value(config,"PUERTO_CPU_INTERRUPT");

	AlgoritmoPlanificacion=config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	quantum=atoi(config_get_string_value(config,"QUANTUM"));
	gradoMultiprogramacion=atoi(config_get_string_value(config,"GRADO_MULTIPROGRAMACION_INI"));
	//MEJOR => atoi: (int)strtol(nptr, (char **)NULL, 10)
	//CONFIGURACION DE MEMORIA
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria=config_get_string_value(config,"PUERTO_MEMORIA");

	//CONFIGURACION DE FILESYSTEM
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoFileSystem=config_get_string_value(config,"PUERTO_FILESYSTEM");

	/************************************INICIALIZAR CONEXIONES************************************/
	conexionCPUDispatch = crear_conexion(ipCPU, puertoCPUDispatch,KERNEL);
	conexionCPUInterrupt = crear_conexion(ipCPU, puertoCPUInterrupt,KERNEL);
	conexionMemoria = crear_conexion(ipMemoria, puertoMemoria,KERNEL);
	//conexionFileSystem = crear_conexion(ipFileSystem, puertoFileSystem,KERNEL);

	/************************************INICIALIZAR HILOS DE RECIBO DE MENSAJES************************************/
	//HILO DE MANEJO DE MOODULOS
	pthread_t hiloCPUDispatch;
	pthread_create(&hiloCPUDispatch,NULL,manejar_cliente,&conexionCPUDispatch);

	//pthread_t  hiloCPUInterrupt;
	//pthread_create(hiloCPUInterrupt,NULL,manejar_cliente,conexionCPUInterrupt);

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
	//pthread_join(hiloCPUInterrupt,NULL);
	pthread_join(hiloMemoria,NULL);
	pthread_join(hiloCorto,NULL);
	pthread_join(hiloLargo,NULL);
	pthread_join(hiloConsola,NULL);
	//pthread_join(hiloFilesystem,NULL);
	//pthread_join(hiloConsola,NULL);
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
void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
	if(!strcasecmp(msg,"CrearProceso")){
		int pid=*(int*)list_get(mensaje,1);
		int resultado=*(int*)list_get(mensaje,2);
		if(resultado>0)printf("se cargo archivo en memoria, proceso %d",pid);
	}
	if(!strcasecmp(msg,"proceso_exit")){
		planificador_largo_salida(list_get(mensaje,1));
	}
	if(!strcasecmp(msg,"cargado")){
		int pid = *(int*)list_get(mensaje,1);
		queue_push(colaLargo,list_get(procesos,pid));
	}
	free(msg);
}
