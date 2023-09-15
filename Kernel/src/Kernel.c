#include "Kernel.h"

int main(void)
{
	char* ipCPU,* ipMemoria,* ipFileSystem ;
	char* puertoCPUDispatch,*puertoCPUInterrupt,* puertoMemoria,* puertoFileSystem ;
	char* valor;
	char* AlgoritmoPlanificacion,* quantum,* recursos,* instanciasRecursos,* gradoMultiprogramacion;
	t_log* logger;
	t_config* config;
	int conexionCPUDispatch, conexionCPUInterrupt,conexionMemoria,conexionFileSistem;
	logger = iniciar_logger();
	config = iniciar_config();

	/************************************RECUPERA DATOS DE ARCHIVO DE CONFIGURACION************************************/
	//CONFIGURACION DE CPU
	ipCPU = config_get_string_value(config,"IP_CPU");
	puertoCPUDispatch=config_get_string_value(config,"PUERTO_CPU_DISPATCH");
	puertoCPUInterrupt=config_get_string_value(config,"PUERTO_CPU_INTERRUPT");
	AlgoritmoPlanificacion=config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	quantum=config_get_string_value(config,"QUANTUM");
	//CONFIGURACION DE MEMORIA
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria=config_get_string_value(config,"PUERTO_MEMORIA");
	//CONFIGURACION DE FILESYSTEM
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoFileSystem=config_get_string_value(config,"PUERTO_FILESYSTEM");

	//BORRAR LUEVO ESTE CAMPO
	valor=config_get_string_value(config,"CLAVE");

	/************************************INICIALIZAR CONEXIONES************************************/
	conexionCPUDispatch = crear_conexion(ipCPU, puertoCPUDispatch);
	conexionCPUInterrupt = crear_conexion(ipCPU, puertoCPUInterrupt);
	conexionMemoria = crear_conexion(ipMemoria, puertoMemoria);
	conexionFileSistem = crear_conexion(ipFileSystem, puertoFileSystem);
	/************************************INICIALIZAR HILOS DE ENVIO Y RECIVO DE MENSAJES************************************/
	// Enviamos al servidor el valor de CLAVE como mensaje
	enviar_mensaje(valor,conexionCPUDispatch);
	enviar_mensaje(valor,conexionCPUInterrupt);
	enviar_mensaje(valor,conexionMemoria);
	enviar_mensaje(valor,conexionFileSistem);
	// Armamos y enviamos el paquete
	paquete(conexionCPUDispatch);
	paquete(conexionCPUInterrupt);
	paquete(conexionMemoria);
	paquete(conexionFileSistem);
	/************************************FINALIZA LOS PROGRAMAS O HILOS A FUTURO************************************/
	terminar_programa(conexionCPUDispatch, logger, config);
	terminar_programa(conexionCPUInterrupt, logger, config);
	terminar_programa(conexionMemoria, logger, config);
	terminar_programa(conexionFileSistem, logger, config);
}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger =log_create("../tp.log","log",1,LOG_LEVEL_INFO);
	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config= config_create("./Kernel.config");
	return nuevo_config;
}

void leer_consola(t_log* logger)
{
	char* leido;
	   while (1) {
	        leido = readline(">");
	               if (!strcmp(leido, "")) {
	                   free(leido);
	                   break;
	               }
	        log_info(logger,leido);
	        free(leido);
	    }
}

void paquete(int conexion)
{
	// Ahora toca lo divertido!
	char* leido;
	t_paquete* paquete=crear_paquete();
	while (1) {
			leido = readline(">");
				   if (!strcmp(leido, "")) {
					   free(leido);
					   break;
				   }
			agregar_a_paquete(paquete,leido,strlen(leido)+1);
			free(leido);
		}
   	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}

