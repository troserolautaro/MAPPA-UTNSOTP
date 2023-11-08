#include "CPU.h"

//VARIABLES GLOBALES
int serverDispatch,serverInterrupt;
int conexionMemoria;
int clienteKernelDispatch,clienteKernelInterrupt;
int reloj = 0; //Es el encargado de revisar el quantum
bool bloquear, interrupcion;
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
int main() {
	logger = malloc(sizeof(t_log));
	t_config* config = malloc(sizeof(t_config));

	logger = iniciar_logger("./log.log");
	config = iniciar_config("./CPU.config");

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
	bloquear= false;
	interrupcion=false;
	pthread_create(&hiloCiclo,NULL,(void*)ejecutar_ciclo,NULL);

	pthread_join(hiloKernelDispatch,NULL);
	pthread_join(hiloCiclo,NULL);
	pthread_join(hiloMemoria,NULL);
	pthread_join(hiloKernelInterrupt,NULL);
	return EXIT_SUCCESS;
}

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

void jnz(uint32_t * registro,uint32_t pc){
	if(*registro==0)proceso->pc=pc;
}

void sleep_proceso(uint32_t tiempo){
	//busca la instruccion en memoria
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"sleep",sizeof(char*)*5);
	agregar_a_paquete(paquete,&tiempo,sizeof(int*));
	serializar_proceso(paquete,proceso);
	enviar_paquete(paquete,clienteKernel);
	bloquear=true;
}

void wait_recurso(char* recurso){
	//busca la instruccion en memoria
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"wait",strlen("wait")+1);
	agregar_a_paquete(paquete,&recurso,strlen(recurso)+1);
	serializar_proceso(paquete,proceso);
	enviar_paquete(paquete,clienteKernel);
	bloquear=true;
}

void signal_recurso(char* recurso){
	//busca la instruccion en memoria
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"signal",strlen("signal")+1);
	agregar_a_paquete(paquete,&recurso,strlen(recurso)+1);
	serializar_proceso(paquete,proceso);
	enviar_paquete(paquete,clienteKernel);
	bloquear=true;
}

void exit_i(){
	//busca la instruccion en memoria
	contexto_ejecucion("proceso_exit");
	bloquear=true;
	/*t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"proceso_exit",sizeof(char*)*11);
	serializar_proceso(paquete,proceso);
	enviar_paquete(paquete,clienteKernel);
	eliminar_paquete(paquete);*/
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
	proceso->pc++;

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
	char*recurso;
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
		registroOrigen = obtener_registro((char*)list_get(parametros,0));
		uint32_t jnzPC=(uint32_t)strtol(list_get(parametros,1),NULL,10);
		jnz(registroOrigen,jnzPC);
	}
	if(!strcasecmp(instruccion->comando,"SLEEP")){
		uint32_t tiempo=(uint32_t)strtol(list_get(parametros,1),NULL,10);
		sleep_proceso(tiempo);
	}
	if(!strcasecmp(instruccion->comando,"WAIT")){
		recurso=list_get(parametros,1);
		wait_recurso(recurso);
	}
	if(!strcasecmp(instruccion->comando,"SIGNAL")){
		recurso=list_get(parametros,1);
		signal_recurso(recurso);
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
	if(!interrupcion && !bloquear){sem_post(&ciclo);}
	else{
	//contexto_ejecucion("contexto_ejecucion");
		bloquear=false;
		interrupcion=false;
	}
}

//FUNCION QUE EJECUTA CICLO DE INSTRUCCION
//agregar luego parametro pcb a ejecutar ciclo de instruccion;
void ejecutar_ciclo(){

	do{
		sem_wait(&ciclo);
		fetch(proceso->pid,proceso->pc);
		sem_wait(&instruccion_s);
		decode();
		execute();
		check_interrupt();
	}while(true);
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

		instruccion->comando=string_duplicate(comando);

		char * consola = string_from_format("PID: %d Ejecucion: %s",proceso->pid,comando);
		for(int i=2; i<list_size(mensaje)-1;i++){
			list_add(instruccion->parametros,list_get(mensaje,i));
			string_append(&consola,string_duplicate((char*)list_get(mensaje,i)));
		}
		// Ejecutando: %s %s %s \n",proceso->pid,comando);
		log_info(logger,"%s",consola);
		free(comando);
		free(consola);
		//free(parametros);?
		sem_post(&instruccion_s);
	}
	if(!strcasecmp(msg,"interrupcion")){
		interrupcion=true;
	}
}
void contexto_ejecucion(char * mensaje){
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,mensaje,sizeof(mensaje));
	serializar_proceso(paquete,proceso);
	enviar_paquete(paquete,clienteKernel);
}

