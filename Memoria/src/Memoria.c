#include "Memoria.h"
int main(void)
{
	/*---------------------------------------------------PARTE 2-------------------------------------------------------------*/

	int conexion;
	char* ip;
	char* puerto;
	char* valor;

	t_log* logger;
	t_config* config;


	logger = iniciar_logger();
	config = iniciar_config();

	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/
	ip = config_get_string_value(config,"IP");
	puerto=config_get_string_value(config,"PUERTO");
	valor=config_get_string_value(config,"CLAVE");
	// ADVERTENCIA: Antes de continuar, tenemos que asegurarnos que el servidor esté corriendo para poder conectarnos a él

	// Creamos una conexión hacia el servidor
	conexion = crear_conexion(ip, puerto);

	// Enviamos al servidor el valor de CLAVE como mensaje
	enviar_mensaje(valor,conexion);

	// Armamos y enviamos el paquete
	paquete(conexion);

	terminar_programa(conexion, logger, config);

}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger =log_create("/home/utnso/tp0/client/tp0.log","log",1,LOG_LEVEL_INFO);
	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config= config_create("/home/utnso/tp0/client/cliente.config");
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


	// ¡No te olvides de liberar las líneas y el paquete antes de regresar!
	eliminar_paquete(paquete);
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}
