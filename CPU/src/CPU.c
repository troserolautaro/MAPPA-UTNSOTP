#include "CPU.h"

//VARIABLES GLOBALES
int serverDispatch,serverInterrupt;
int conexionMemoria;
int clienteKernel,clienteKernelInterrupt;
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

	logger = iniciar_logger("./log.log");
	config = iniciar_config("./CPU.config");

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
	 clienteKernel = esperar_cliente(serverDispatch);
	 clienteKernelInterrupt = esperar_cliente(serverInterrupt);

	//HILOS
	pthread_t hiloKernel, hiloCiclo,hiloMemoria;
	//Hilos conexion
	pthread_create(&hiloKernel,NULL,(void *) manejar_cliente,&clienteKernel);
	pthread_create(&hiloMemoria,NULL,(void *)manejar_cliente,&conexionMemoria);
	//Hilos proceso
	pthread_create(&hiloCiclo,NULL,(void*)ejecutar_ciclo,NULL);
	/*PROBABLEMENTE SE PUEDA SEPARAR ESTO Y ABSTRAERLA COMO UNA FUNCION PARA UTILES*/
	pthread_join(hiloKernel,NULL);
	pthread_join(hiloCiclo,NULL);
	//manejar_cliente(NULL);
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
void fetch(int pid,int pc){
	//busca la instruccion en memoria
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"instruccion",sizeof(char*)*11);
	agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
	agregar_a_paquete(paquete,&pc,sizeof(uint32_t));
	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);
	//esta parte se podria mover a procesar mensaje
	//CONSUMIDOR esperando respuesta memoria
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
	uint32_t jnzPC;
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
		jnzPC=(uint32_t)strtol(list_get(parametros,1),NULL,10);
		jnz(registroOrigen,jnzPC);
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

	if(!strcasecmp(msg,"proceso")){
		sem_post(&ciclo);
	}
	if(!strcasecmp(msg,"instruccion")){
		char* comando = string_new();
		string_append(&comando,list_get(mensaje,1));
		string_trim_right(&comando);

		char** parametros = string_array_new();
		parametros = string_n_split(comando,3," ");

		instruccion->comando=parametros[0];

		for(int i=1; parametros[i]!=NULL;i++){
			list_add(instruccion->parametros,parametros[i]);
		}

		sem_post(&instruccion_s);
	}
	if(!strcasecmp(msg,"interrupcion")){

	}
}

