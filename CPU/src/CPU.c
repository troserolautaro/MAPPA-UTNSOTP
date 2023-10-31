#include "CPU.h"

//VARIABLES GLOBALES
int serverDispatch,serverInterrupt;
int conexionMemoria;
int clienteKernel;
int reloj = 0; //Es el encargado de revisar el quantum
t_log* logger ;
PCB pcbPrueba;
int main(void) {
	logger = malloc(sizeof(t_log));
	t_config* config = malloc(sizeof(t_config));

	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	config = iniciar_config();

	char* ipMemoria = malloc(sizeof(char*)),
			*puertoEscuchaDispatch = malloc(sizeof(char*)),
			*puertoEscuchaInterrupt = malloc(sizeof(char*)),
			*puertoMemoria = malloc(sizeof(char*));

	//CONFIGURACION DE CPU
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria = config_get_string_value(config,"PUERTO_MEMORIA");
	puertoEscuchaDispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
	puertoEscuchaInterrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");

	//Iniciar Cliente que conecta a memoria
	 conexionMemoria = crear_conexion(ipMemoria, puertoMemoria);
	pcbPrueba.pid=1;
	pcbPrueba.pc=1;
	ejecutar_ciclo();

	//Inicia Servidor
	 serverDispatch = iniciar_servidor(puertoEscuchaDispatch);
	 serverInterrupt = iniciar_servidor(puertoEscuchaInterrupt);
	 log_info(logger, "Servidor listo para recibir al cliente");
	 clienteKernel = esperar_cliente(serverDispatch);



	//HILO DE MANEJO CLIENTE KERNEL
	pthread_t hiloKernel;
	pthread_create(&hiloKernel,NULL,(void *) manejar_cliente,clienteKernel);
	/*PROBABLEMENTE SE PUEDA SEPARAR ESTO Y ABSTRAERLA COMO UNA FUNCION PARA UTILES*/
	pthread_join(hiloKernel,NULL);
	//manejar_cliente(NULL);
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

void exit_p(){
	pcbPrueba.estado=TERMINATED;
}


//FUNCION AUXILIAR PARA OBTENER LOS REGISTROS INDICADOS EN LAS INSTRUCCIONES
uint32_t * obtener_registro(char registro[],registros_CPU* registros){
	if(!strcasecmp(registro, "AX")){
		return &registros->AX;
	}
	if(!strcasecmp(registro, "BX")){
		return &registros->BX;
	}
	if(!strcasecmp(registro, "CX")){
		return &registros->CX;
	}
	if(!strcasecmp(registro, "DX")){
		return  &registros->DX;
	}
	return -2;
}

//FUNCIONES PARA CICLO DE EJECUCION SIMPLE
char* fetch(int,pid,int pc){
	//busca la instruccion en memoria
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"instruccion",sizeof(char*)*11);
	agregar_a_paquete(paquete,&pid,sizeof(int*));
	agregar_a_paquete(paquete,&pc,sizeof(int*));
	//semaforo de conexion a memoria
	enviar_paquete(paquete,conexionMemoria);
	//signal
	eliminar_paquete(paquete);
	//esta parte se podria mover a procesar mensaje
	t_list* mensaje=list_create();
	mensaje= procesar_tipo(conexionMemoria);
	if(){

	}
	int size;
	char * lineaInstruccion=malloc(sizeof(char)*100+1);
	lineaInstruccion=recibir_buffer(&size,conexionMemoria);
	return lineaInstruccion;
}

//definir como llega de memoria para definir el tipo de parametro
t_instruccion decode( char* lineaDeCodigo){
	//obtener registros necesarios para ejecutar la instruccion y pasarselos a execute
	//DECODIFICACION DE LA INSTRUCCION
	t_instruccion instruccion;
	instruccion.comando=malloc(sizeof(char*));
	instruccion.parametros= list_create();
	char * parametros[2];
	parametros[0]=malloc(sizeof(char) * 30 + 1);
	parametros[1]=malloc(sizeof(char) * 30 + 1);
	if (sscanf(lineaDeCodigo, "%s %s %s", instruccion.comando,parametros[0], parametros[1]) >= 1) {
		 //printf("\n el comando es  %s \n", instruccion.comando);

		for (int i = 0; i < 2; i++) {
			if (strlen(parametros[i]) > 0) {
				parametros[i][strcspn(parametros[i], "\n")] = '\0';
				list_add(instruccion.parametros,parametros[i]);
				printf("parametro %d tiene : %s \n", i,parametros[i]);
			}
		}
	}
	return instruccion;
}
void execute(t_instruccion instruccion){
	t_list * parametros=instruccion.parametros;
	uint32_t *registroOrigen, *registroDestino;
	//obtengo parametros
	char* parametro1=malloc(sizeof(char)*30+1);
	parametro1=list_get(parametros,0);
	if(parametro1!=NULL){
		registroOrigen=obtener_registro(parametro1,&pcbPrueba.registros);
	}
	char* parametro2=malloc(sizeof(char)*30+1);
	parametro2=list_get(parametros,1);
	if(parametro2!=NULL && strcasecmp(instruccion.comando, "SET")){
		registroDestino=obtener_registro(parametro2,&pcbPrueba.registros);
	}
	//executo funcion
	if(!strcasecmp(instruccion.comando, "SET")){
		set(registroOrigen,(int)*parametro2);
	}
	if(!strcasecmp(instruccion.comando, "SUM")){
		sum(registroOrigen,registroDestino);
	}
	if(!strcasecmp(instruccion.comando, "SUB")){
		sub(registroOrigen,registroDestino);
	}
	if(!strcasecmp(instruccion.comando, "EXIT")){
		exit_p();
	}
}
void check_interrupt(){
	//pendiente a definir que hace
}

//FUNCION QUE EJECUTA CICLO DE INSTRUCCION
//agregar luego parametro pcb a ejecutar ciclo de instruccion;
void ejecutar_ciclo(){
	t_instruccion instruccion;
	char* operacion, *lineaDeCodigo;
	lineaDeCodigo=fetch(pcbPrueba.pid,pcbPrueba.pc);
	instruccion= decode(lineaDeCodigo);
	execute(instruccion);
	//check_interrupt();
}

void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);

	if(!strcasecmp(msg,"conexion")){
		log_info(logger,"Hola! %d",*(int*)list_get(mensaje,1));
	}

}

