#include "CPU.h"



int main(void) {

	t_log* logger = malloc(sizeof(t_log));
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
	char* valor= malloc(sizeof(char*));
	valor=ipMemoria;
	enviar_mensaje(valor,conexionMemoria);

	//Inicia Servidor
	int serverDispatch = iniciar_servidor(puertoEscuchaDispatch);
	int serverInterrupt = iniciar_servidor(puertoEscuchaInterrupt);
	log_info(logger, "Servidor listo para recibir al cliente");
	int clienteFd = esperar_cliente(serverDispatch);
	/*PROBABLEMENTE SE PUEDA SEPARAR ESTO Y ABSTRAERLA COMO UNA FUNCION PARA UTILES*/
			t_list* lista = malloc(sizeof(t_list));
			while (1) {
				int cod_op = recibir_operacion(clienteFd);
				switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(clienteFd);
					break;
				case PAQUETE:
					lista = recibir_paquete(clienteFd);
					log_info(logger, "Me llegaron los siguientes valores:\n");
					list_iterate(lista, (void*) iterator);
					break;
				case -1:
					log_error(logger, "Un cliente se desconecto.");
					log_destroy(logger);
					return EXIT_SUCCESS;
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
				}
			}
			free(lista);


	return EXIT_SUCCESS;
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
