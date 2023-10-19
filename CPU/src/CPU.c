#include "CPU.h"

//VARIABLES GLOBALES
int serverDispatch,serverInterrupt;
int clienteKernel;
t_log* logger ;
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
	int conexionMemoria = crear_conexion(ipMemoria, puertoMemoria);
	//char* valor= malloc(sizeof(char*));
	//valor=ipMemoria;
	//enviar_mensaje(puertoMemoria,conexionMemoria);

	//Inicia Servidor
	 serverDispatch = iniciar_servidor(puertoEscuchaDispatch);
	 serverInterrupt = iniciar_servidor(puertoEscuchaInterrupt);
	log_info(logger, "Servidor listo para recibir al cliente");
	clienteKernel = esperar_cliente(serverDispatch);

	//HILO DE MANEJO CLIENTE KERNEL
	pthread_t hiloKernel;
	pthread_create(&hiloKernel,NULL,(void *) manejar_cliente,NULL );
	/*PROBABLEMENTE SE PUEDA SEPARAR ESTO Y ABSTRAERLA COMO UNA FUNCION PARA UTILES*/
	pthread_join(hiloKernel,NULL);
	//manejar_cliente(NULL);
	return EXIT_SUCCESS;
}
void * manejar_cliente(void*){
	printf("Se conecto Kernel \n");
	t_list* lista = malloc(sizeof(t_list));
				while (1) {
					int cod_op = recibir_operacion(clienteKernel);
					switch (cod_op) {
					case MENSAJE:
						recibir_mensaje(clienteKernel);
						break;
					case PAQUETE:
						lista = recibir_paquete(clienteKernel);
						log_info(logger, "Me llegaron los siguientes valores:\n");
						list_iterate(lista, (void*) iterator);
						break;
					case -1:
						log_error(logger, "Un cliente se desconecto.");
						log_destroy(logger);
						return NULL;
						break;
					default:
						log_warning(logger,"Operacion desconocida. No quieras meter la pata");
						break;
					}
				}
				free(lista);
}
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


void iterator(char* value) {
	log_info(logger,"%s", value);
}

void set(uint32_t registro, int valor){
	registro = valor;
}

void sum(uint32_t registroDestino,uint32_t registroOrigen){
	registroDestino += registroOrigen;
}
void sub(uint32_t registroDestino,uint32_t registroOrigen){
	registroDestino -= registroOrigen;
}

void exit();

uint32_t obtener_registro(char* componente,registros_cpu* registro){

	uint32_t registroAux;

	switch(componente){
	case "AX":
		registroAux = registro->AX;
		break;
	case "BX":
		registroAux = registro->BX;
			break;
	case "CAX":
		registroAux = registro->CX;
			break;
	case "DX":
		registroAux = registro->DX;
			break;
	default:
		printf("Registro no válido");
	}

	return registroAux;


}




