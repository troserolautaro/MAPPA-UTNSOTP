#include "Memoria.h"



int main(void) {
	t_log* logger = malloc(sizeof(t_log));
	t_config* config = malloc(sizeof(t_config));
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	config = iniciar_config();

	char* ipFyleSystem = malloc(sizeof(char*)),
			*puertoEscucha = malloc(sizeof(char*)),
			*tamMemoria = malloc(sizeof(char*)),
			*tamPagina = malloc(sizeof(char*)),
			*pathInstrucciones = malloc(sizeof(char*)),
			*retardoRespuesta = malloc(sizeof(char*)),
			*algoritmoReemplazo = malloc(sizeof(char*)),
			*puertoFyleSystem = malloc(sizeof(char*));

	//CONFIGURACION DE MEMORIA
	ipFyleSystem = config_get_string_value(config,"IP_FILESYSTEM");
	puertoFyleSystem = config_get_string_value(config,"PUERTO_FYLESYSTEM");
	puertoEscucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	tamMemoria = config_get_string_value(config,"TAM_MEMORIA");
	tamPagina = config_get_string_value(config,"TAM_PAGINA");
	pathInstrucciones = config_get_string_value(config,"PATH_INSTRUCCIONES");
	retardoRespuesta = config_get_string_value(config,"RETARDO_RESPUESTA");
	algoritmoReemplazo = config_get_string_value(config,"ALGORITMO_REEMPLAZO");

	//INICIAR SERVIDOR
	int serverMemoria = iniciar_servidor(puertoEscucha);

	//printf("%ld \n %ld", (long)getpid(), (long)getppid());
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(serverMemoria);
			t_list* lista;
			while (1) {
				int cod_op = recibir_operacion(cliente_fd);
				switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(cliente_fd);
					break;
				case PAQUETE:
					lista = recibir_paquete(cliente_fd);
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

void iterator(char* value) {
	log_info(logger,"%s", value);
}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger =log_create("./tp.log","log",1,LOG_LEVEL_INFO);
	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config= config_create("./Memoria.config");
	return nuevo_config;
}
