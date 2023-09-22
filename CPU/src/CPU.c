#include "CPU.h"



int main(void) {

	t_log* logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	t_config* config = iniciar_config();

	char* ipMemoria;
	char* puertoEscuchaDispatch,* puertoEscuchaInterrupt,* puertoMemoria;

	//CONFIGURACION DE CPU
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria = config_get_string_value(config,"PUERTO_MEMORIA");
	puertoEscuchaDispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
	puertoEscuchaInterrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");


	int serverDispatch = iniciar_servidor(puertoEscuchaDispatch);
	int serverInterrupt = iniciar_servidor(puertoEscuchaInterrupt);
	log_info(logger, "Servidor listo para recibir al cliente");
	int clienteFd = esperar_cliente(serverDispatch);
			t_list* lista;
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


	return EXIT_SUCCESS;
}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger =log_create("../tp.log","log",1,LOG_LEVEL_INFO);
	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config= config_create("../CPU.config");
	return nuevo_config;
}


void iterator(char* value) {
	log_info(logger,"%s", value);
}
