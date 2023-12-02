#include "CPU.h"

//VARIABLES GLOBALES
int serverDispatch,serverInterrupt;
int conexionMemoria;
int clienteKernelDispatch,clienteKernelInterrupt;
int reloj = 0; //Es el encargado de revisar el quantum
bool bloquear, interrupcion;
char* motivo;
//
t_log* logger;
t_config* config;
t_instruccion * instruccion;
PCB* proceso;
pthread_mutex_t mutexProceso, mutexLog,mutexInstruccion, mutexBloquear, mutexInterrupcion;

/*struct{
	t_instruccion * instruccion;
	sem_t instruccion_s;
}semaforo_instruccion;
 *
 */
//Semaforos
sem_t ciclo,instruccion_s;
int main() {


	logger = iniciar_logger("./log.log");
	t_config* config = iniciar_config("./CPU.config");

	proceso = proceso_create();
	instruccion = instruccion_create();
	char* ipMemoria = malloc(sizeof(char*)),
			*puertoEscuchaDispatch = malloc(sizeof(char*)),
			*puertoEscuchaInterrupt = malloc(sizeof(char*)),
			*puertoMemoria = malloc(sizeof(char*));
	motivo = string_new();
	//CONFIGURACION DE CPU
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria = config_get_string_value(config,"PUERTO_MEMORIA");
	puertoEscuchaDispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
	puertoEscuchaInterrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");

	//Iniciar semaforos
	sem_init(&ciclo,0,0);
	sem_init(&instruccion_s,0,0);
	pthread_mutex_init(&mutexProceso,NULL);
	pthread_mutex_init(&mutexLog,NULL);
	pthread_mutex_init(&mutexInstruccion,NULL);
	pthread_mutex_init(&mutexBloquear,NULL);
	pthread_mutex_init(&mutexInterrupcion,NULL);
	//Iniciar Cliente que conecta a memoria
	conexionMemoria = crear_conexion(ipMemoria, puertoMemoria,CPUDispatch);

	//Inicia Servidor
	serverDispatch = iniciar_servidor(puertoEscuchaDispatch);
	serverInterrupt = iniciar_servidor(puertoEscuchaInterrupt);
	// log_info(logger, "Servidor listo para recibir al cliente");
	debug("Servidor listo para recibir al cliente");
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
	pthread_mutex_lock(&mutexProceso);
	if(*registro==0)proceso->pc=pc;
	pthread_mutex_unlock(&mutexProceso);
}

void sleep_proceso(uint32_t tiempo){
	//busca la instruccion en memoria
	t_list * mensaje = list_create();
	list_add(mensaje,"sleep");
	list_add(mensaje,string_itoa((int)tiempo));
	contexto_ejecucion(mensaje);

	pthread_mutex_lock(&mutexBloquear);
	bloquear=true;
	pthread_mutex_unlock(&mutexBloquear);

	list_destroy(mensaje);
}

void wait_recurso(char* recurso){
	//busca la instruccion en memoria
	t_list * mensaje = list_create();
	list_add(mensaje,"wait");
	list_add(mensaje,recurso);
	contexto_ejecucion(mensaje);

	pthread_mutex_lock(&mutexBloquear);
	bloquear=true;
	pthread_mutex_unlock(&mutexBloquear);

	list_destroy(mensaje);
}

void signal_recurso(char* recurso){
	//busca la instruccion en memoria
	t_list * mensaje = list_create();
	list_add(mensaje,"signal");
	list_add(mensaje,recurso);
	contexto_ejecucion(mensaje);

	pthread_mutex_lock(&mutexBloquear);
	bloquear=true;
	pthread_mutex_unlock(&mutexBloquear);

	list_destroy(mensaje);
}

void exit_i(){
	//busca la instruccion en memoria
	t_list * mensaje = list_create();
	list_add(mensaje,"procesoExit");
	contexto_ejecucion(mensaje);
	pthread_mutex_lock(&mutexBloquear);
	bloquear=true;
	pthread_mutex_unlock(&mutexBloquear);
	list_destroy(mensaje);
	/*t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"proceso_exit",sizeof(char*)*11);
	serializar_proceso(paquete,proceso);
	enviar_paquete(paquete,clienteKernelDispatch);
	eliminar_paquete(paquete);*/
}


//FUNCION AUXILIAR PARA OBTENER LOS REGISTROS INDICADOS EN LAS INSTRUCCIONES
uint32_t * obtener_registro(char* registro){
	uint32_t* puntero = NULL;
	if(!strcasecmp(registro, "AX")){
		pthread_mutex_lock(&mutexProceso);
		puntero = &(proceso->registros->AX);
		pthread_mutex_unlock(&mutexProceso);
		return puntero;
	}
	if(!strcasecmp(registro, "BX")){
		pthread_mutex_lock(&mutexProceso);
		puntero = &(proceso->registros->BX);
		pthread_mutex_unlock(&mutexProceso);
		return puntero;
	}
	if(!strcasecmp(registro, "CX")){
		pthread_mutex_lock(&mutexProceso);
		puntero = &(proceso->registros->CX);
		pthread_mutex_unlock(&mutexProceso);
		return puntero;
	}
	if(!strcasecmp(registro, "DX")){
		pthread_mutex_lock(&mutexProceso);
		puntero = &(proceso->registros->DX);
		pthread_mutex_unlock(&mutexProceso);
		return puntero;
	}
	return puntero;
}

//FUNCIONES PARA CICLO DE EJECUCION SIMPLE
void fetch(){
	//busca la instruccion en memoria
	pthread_mutex_lock(&mutexProceso);
	uint32_t pid = proceso->pid;
	uint32_t pc = proceso->pc;
	pthread_mutex_unlock(&mutexProceso);

	char* mensaje = string_from_format("PID: %d - FETCH - Program Counter: %d",pid,pc);
	escritura_log(mensaje);
	free(mensaje);
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"instruccion",sizeof("instruccion"));
	agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
	agregar_a_paquete(paquete,&pc,sizeof(uint32_t));
	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);

	pthread_mutex_lock(&mutexProceso);
	proceso->pc++;
	pthread_mutex_unlock(&mutexProceso);
// Fetch Instrucción: “PID: <PID> - FETCH - Program Counter: <PROGRAM_COUNTER>”.
}

//definir como llega de memoria para definir el tipo de parametro
void decode(){
	//obtener registros necesarios para ejecutar la instruccion y pasarselos a execute
	//DECODIFICACION DE LA INSTRUCCION
	//MMU (Direcciones logicas y fisicas)
}
void execute(){
	pthread_mutex_lock(&mutexInstruccion);
	t_list * parametros=instruccion->parametros;
	char* comando = instruccion->comando;
	pthread_mutex_unlock(&mutexInstruccion);
	void *registroOrigen, *registroDestino;
	char*recurso;

	//obtengo parametros
	/*executo funcion, talvez es mejor que la funcion reciba un int y esto este dentro de un switch en el que adentro
	revisemos las variables que haya que utilizar*/
	if(!strcasecmp(comando, "SET")){
		registroDestino=obtener_registro((char*)list_get(parametros,0));
		int valor = (int)strtol(list_get(parametros,1), (char **)NULL, 10);
		set(registroDestino,valor);
	}
	if(!strcasecmp(comando, "SUM")){
		registroDestino = obtener_registro((char*)list_get(parametros,0));
		registroOrigen =  obtener_registro((char*)list_get(parametros,1));
		sum(registroDestino,registroOrigen);
	}
	if(!strcasecmp(comando, "SUB")){
		registroDestino = obtener_registro((char*)list_get(parametros,0));
		registroOrigen =  obtener_registro((char*)list_get(parametros,1));
		sub(registroDestino,registroOrigen);
	}
	if(!strcasecmp(comando,"JNZ")){
		registroOrigen = obtener_registro((char*)list_get(parametros,0));
		uint32_t jnzPC=(uint32_t)strtol(list_get(parametros,0),NULL,10);
		jnz(registroOrigen,jnzPC);
	}
	if(!strcasecmp(comando,"SLEEP")){
		uint32_t tiempo=(uint32_t)strtol(list_get(parametros,0),NULL,10);
		sleep_proceso(tiempo);
	}
	if(!strcasecmp(comando,"WAIT")){
		recurso=list_get(parametros,0);
		wait_recurso(recurso);
	}
	if(!strcasecmp(comando,"SIGNAL")){
		recurso=list_get(parametros,0);
		signal_recurso(recurso);
	}
	if(!strcasecmp(comando,"MOV_IN")){
		//MOV_IN();
	}
	if(!strcasecmp(comando,"MOV_OUT")){
		//MOV_OUT();
	}
	if(!strcasecmp(comando,"F_OPEN")){
		//F_OPEN();
	}
	if(!strcasecmp(comando,"F_CLOSE")){
		//F_CLOSE();
	}
	if(!strcasecmp(comando,"F_SEEK")){
		//F_SEEK();
	}
	if(!strcasecmp(comando,"F_READ")){
		//F_READ();
	}
	if(!strcasecmp(comando,"F_WRITE")){
		//F_WRITE();
	}
	if(!strcasecmp(comando,"F_TRUNCATE")){
		//F_TRUNCATE();
	}
	if(!strcasecmp(comando, "EXIT")){
		exit_i();
	}
	pthread_mutex_lock(&mutexInstruccion);
	list_clean_and_destroy_elements(instruccion->parametros,(void*)liberar_memoria);
	pthread_mutex_unlock(&mutexInstruccion);
	//	Instrucción Ejecutada: “PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.
}
void check_interrupt(){

	pthread_mutex_lock(&mutexBloquear);
	pthread_mutex_lock(&mutexInterrupcion);
	if(!interrupcion && !bloquear){sem_post(&ciclo);}
	pthread_mutex_unlock(&mutexBloquear);
	pthread_mutex_unlock(&mutexInterrupcion);

	pthread_mutex_lock(&mutexBloquear);
	if(bloquear){

			bloquear=false;

	}
	pthread_mutex_unlock(&mutexBloquear);

	pthread_mutex_lock(&mutexInterrupcion);
	if(interrupcion){
			t_list * mensaje = list_create();
			list_add(mensaje,motivo);
			contexto_ejecucion(mensaje);
			list_destroy(mensaje);
			interrupcion=false;
	}
	pthread_mutex_unlock(&mutexInterrupcion);
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
int razon_interrupcion(char * razon){
	if(!strcasecmp(razon,"desalojo")) return DESALOJO;
	return -1;
}
void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
//Seria excelente cuanto menos aprovechar que dentro de la lista "mensaje" se encuentra al final el socket para dividir con un switch las funciones
	if(!strcasecmp(msg,"proceso")){

		pthread_mutex_lock(&mutexProceso);
		deserializar_proceso(proceso,mensaje,1);
		pthread_mutex_unlock(&mutexProceso);

		sem_post(&ciclo);
	}
	if(!strcasecmp(msg,"instruccion")){
		char* comando = string_new();
		string_append(&comando,list_get(mensaje,1));

		pthread_mutex_lock(&mutexInstruccion);
		instruccion->comando=string_duplicate(comando);
		pthread_mutex_unlock(&mutexInstruccion);

		pthread_mutex_lock(&mutexProceso);
		int pid = proceso->pid;
		pthread_mutex_unlock(&mutexProceso);

		char * consola = string_from_format("PID: %d Ejecucion: %s",pid,comando);
		for(int i=2; i<list_size(mensaje)-1;i++){

			pthread_mutex_lock(&mutexInstruccion);
			list_add(instruccion->parametros,list_get(mensaje,i));
			pthread_mutex_unlock(&mutexInstruccion);

			string_append_with_format(&consola,"%s ",(char*)list_get(mensaje,i));
		}
		escritura_log(consola);
		free(consola);
		sem_post(&instruccion_s);
	}
	if(!strcasecmp(msg,"interrupcion")){
		int razon = razon_interrupcion((char*)list_get(mensaje,1));
		bool banderaInterrumpir = false;
		switch(razon){
			case DESALOJO:
					uint32_t pid = 0;
					motivo = ((char*) list_get(mensaje,2));
					pid= *(uint32_t*)list_get(mensaje,3);
					pthread_mutex_lock(&mutexProceso);
					if (pid == (proceso->pid)) banderaInterrumpir = true;

					pthread_mutex_unlock(&mutexProceso);
				break;
		}
		if(banderaInterrumpir){
			pthread_mutex_lock(&mutexInterrupcion);
			interrupcion=true;
			pthread_mutex_unlock(&mutexInterrupcion);

		}
	}
	free(msg);
}
void contexto_ejecucion(t_list * mensaje){
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"contexto",sizeof("contexto"));
	for(int i=0;i<list_size(mensaje);i++){
		agregar_a_paquete(paquete,list_get(mensaje,i),(strlen((char*)list_get(mensaje,i)))+1);
	}
	int size = (list_size(mensaje))+1;

	pthread_mutex_lock(&mutexProceso);
	PCB* temp = proceso_copy(proceso);
	pthread_mutex_unlock(&mutexProceso);


	serializar_proceso(paquete,temp);
	agregar_a_paquete(paquete,&size,sizeof(size));
	enviar_paquete(paquete,clienteKernelDispatch);
	proceso_destroy(temp);
	eliminar_paquete(paquete);

}

