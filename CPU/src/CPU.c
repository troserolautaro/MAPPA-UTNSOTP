#include "CPU.h"

//VARIABLES GLOBALES
int serverDispatch,serverInterrupt;
int conexionMemoria;
int clienteKernelDispatch,clienteKernelInterrupt;
int reloj = 0; //Es el encargado de revisar el quantum
//
t_log* logger ;
t_instruccion * instruccion;
PCB* proceso;

/*struct{
	t_instruccion * instruccion;
	sem_t instruccion_s;
}semaforo_instruccion;
 *
 */
//Semaforos
sem_t ciclo,instruccion_s;
int main(void) {
	logger = malloc(sizeof(t_log));
	t_config* config = malloc(sizeof(t_config));

	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	config = iniciar_config();

	proceso = proceso_create();
	instruccion = instruccion_create();
	char* ipMemoria = malloc(sizeof(char*)),
			*puertoEscuchaDispatch = malloc(sizeof(char*)),
			*puertoEscuchaInterrupt = malloc(sizeof(char*)),
			*puertoMemoria = malloc(sizeof(char*));

	//CONFIGURACION DE CPU
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria = config_get_string_value(config,"PUERTO_MEMORIA");
	puertoEscuchaDispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
	puertoEscuchaInterrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");

	//Iniciar semaforos
	sem_init(&ciclo,0,0);
	sem_init(&instruccion_s,0,0);
	//Iniciar Cliente que conecta a memoria
	conexionMemoria = crear_conexion(ipMemoria, puertoMemoria,CPUDispatch);

	//Inicia Servidor
	 serverDispatch = iniciar_servidor(puertoEscuchaDispatch);
	 serverInterrupt = iniciar_servidor(puertoEscuchaInterrupt);
	 log_info(logger, "Servidor listo para recibir al cliente");
	 clienteKernelDispatch = esperar_cliente(serverDispatch);
	 clienteKernelInterrupt = esperar_cliente(serverInterrupt);

	//HILOS
	pthread_t hiloKernelDispatch, hiloCiclo,hiloMemoria,hiloKernelInterrupt;
	//Hilos conexion
	int resultado;
	if ((resultado = pthread_create(&hiloKernelDispatch,NULL,manejar_cliente,(void*)&clienteKernelDispatch))!=0){
		printf("Error al crear hilo. resultado %d",resultado);
	}
	if ((resultado = pthread_create(&hiloKernelInterrupt,NULL,manejar_cliente,&clienteKernelInterrupt))!=0){
		printf("Error al crear hilo. resultado %d",resultado);
	}
	if ((resultado = pthread_create(&hiloMemoria,NULL,manejar_cliente,(void*)&conexionMemoria))!=0){
		printf("Error al crear hilo. resultado %d",resultado);
	}
	//Hilos proceso
	pthread_create(&hiloCiclo,NULL,(void*)ejecutar_ciclo,NULL);

	pthread_join(hiloKernelDispatch,NULL);
	pthread_join(hiloCiclo,NULL);
	pthread_join(hiloMemoria,NULL);
	pthread_join(hiloKernelInterrupt,NULL);
	return EXIT_SUCCESS;
}

//FUNCIONES DE INICIO DE MODULO
t_log* iniciar_logger(void)
{
	t_log* nuevo_logger =log_create("./tp.log","log",1,LOG_LEVEL_INFO);
	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config= config_create("./CPU.config");
	return nuevo_config;
}

//



//FUNCIONES DE INSTRUCCION
void set(uint32_t *registro, int valor){
	*registro = valor;
}

void sum(uint32_t * registroDestino,uint32_t *registroOrigen){
	*registroDestino += *registroOrigen;
}
void sub(uint32_t * registroDestino,uint32_t * registroOrigen){
	*registroDestino -= *registroOrigen;
}

void exit_i(){
	//busca la instruccion en memoria
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"proceso_exit",sizeof(char*)*11);
	serializar_proceso(paquete,proceso);
	enviar_paquete(paquete,clienteKernel);
	eliminar_paquete(paquete);
}


//FUNCION AUXILIAR PARA OBTENER LOS REGISTROS INDICADOS EN LAS INSTRUCCIONES
uint32_t * obtener_registro(char* registro){
	if(!strcasecmp(registro, "AX")){
		return &proceso->registros->AX;
	}
	if(!strcasecmp(registro, "BX")){
		return &proceso->registros->BX;
	}
	if(!strcasecmp(registro, "CX")){
		return &proceso->registros->CX;
	}
	if(!strcasecmp(registro, "DX")){
		return &proceso->registros->DX;
	}
	return (uint32_t*)NULL;
}

//FUNCIONES PARA CICLO DE EJECUCION SIMPLE
void fetch(uint32_t pid,uint32_t pc){
	//busca la instruccion en memoria
	char* mensaje = string_from_format("PID: %d",pid);
	string_append_with_format(&mensaje," - FETCH - Program Counter: %d",pc);
	log_info(logger,"%s",mensaje);
	free(mensaje);
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"instruccion",sizeof("instruccion"));
	agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
	agregar_a_paquete(paquete,&pc,sizeof(uint32_t));
	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);

// Fetch Instrucción: “PID: <PID> - FETCH - Program Counter: <PROGRAM_COUNTER>”.
}

//definir como llega de memoria para definir el tipo de parametro
void decode(){
	//obtener registros necesarios para ejecutar la instruccion y pasarselos a execute
	//DECODIFICACION DE LA INSTRUCCION
	//MMU (Direcciones logicas y fisicas)
}
void execute(){
	t_list * parametros=instruccion->parametros;
	void *registroOrigen, *registroDestino;
	//obtengo parametros
	/*executo funcion, talvez es mejor que la funcion reciba un int y esto este dentro de un switch en el que adentro
	revisemos las variables que haya que utilizar*/
	if(!strcasecmp(instruccion->comando, "SET")){
		registroDestino=obtener_registro((char*)list_get(parametros,0));
		int valor = (int)strtol(list_get(parametros,1), (char **)NULL, 10);
		set(registroDestino,valor);
	}
	if(!strcasecmp(instruccion->comando, "SUM")){
		registroDestino = obtener_registro((char*)list_get(parametros,0));
		registroOrigen =  obtener_registro((char*)list_get(parametros,1));
		sum(registroDestino,registroOrigen);
	}
	if(!strcasecmp(instruccion->comando, "SUB")){
		registroDestino = obtener_registro((char*)list_get(parametros,0));
		registroOrigen =  obtener_registro((char*)list_get(parametros,1));
		sub(registroDestino,registroOrigen);
	}
	if(!strcasecmp(instruccion->comando,"JNZ")){
		//jnz();
	}
	if(!strcasecmp(instruccion->comando,"SLEEP")){
		//sleep();
	}
	if(!strcasecmp(instruccion->comando,"WAIT")){
		//WAIT();
	}
	if(!strcasecmp(instruccion->comando,"SIGNAL")){
		//SIGNAL();
	}
	if(!strcasecmp(instruccion->comando,"MOV_IN")){
		//MOV_IN();
	}
	if(!strcasecmp(instruccion->comando,"MOV_OUT")){
		//MOV_OUT();
	}
	if(!strcasecmp(instruccion->comando,"F_OPEN")){
		//F_OPEN();
	}
	if(!strcasecmp(instruccion->comando,"F_CLOSE")){
		//F_CLOSE();
	}
	if(!strcasecmp(instruccion->comando,"F_SEEK")){
		//F_SEEK();
	}
	if(!strcasecmp(instruccion->comando,"F_READ")){
		//F_READ();
	}
	if(!strcasecmp(instruccion->comando,"F_WRITE")){
		//F_WRITE();
	}
	if(!strcasecmp(instruccion->comando,"F_TRUNCATE")){
		//F_TRUNCATE();
	}
	if(!strcasecmp(instruccion->comando, "EXIT")){
		exit_i();
	}


	//	Instrucción Ejecutada: “PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.
}
void check_interrupt(){
	//pendiente a definir que hace
	//Encargarse de sacar proceso por quantum
}

//FUNCION QUE EJECUTA CICLO DE INSTRUCCION
//agregar luego parametro pcb a ejecutar ciclo de instruccion;
void ejecutar_ciclo(){
	do{
		sem_wait(&ciclo);
		fetch(proceso->pid,proceso->pc);
		sem_wait(&instruccion_s);
		decode();
		execute(instruccion);
		//check_interrupt();
	}while(true);
	//Esperando un PCB

}

void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
//Seria excelente cuanto menos aprovechar que dentro de la lista "mensaje" se encuentra al final el socket para dividir con un switch las funciones
	if(!strcasecmp(msg,"proceso")){
		deserializar_proceso(proceso,mensaje);
		sem_post(&ciclo);
	}
	if(!strcasecmp(msg,"instruccion")){
		char* comando = string_new();
		string_append(&comando,list_get(mensaje,1));
		string_trim_right(&comando);

		char** parametros = string_array_new();
		parametros = string_n_split(comando,3," ");
		instruccion->comando=string_duplicate(parametros[0]);

		char * mensaje = string_from_format("PID: %d",proceso->pid);
		string_append_with_format(&mensaje,"Ejecutando: %s",instruccion->comando);

		for(int i=1; parametros[i]!=NULL;i++){
			list_add(instruccion->parametros,string_duplicate(parametros[i]));
			string_append(&mensaje,string_duplicate(parametros[i]));
		}

		log_info(logger,"%s",mensaje);
		free(mensaje);
		//free(parametros);?
		sem_post(&instruccion_s);
	}
	if(!strcasecmp(msg,"interrupcion")){

	}
}

