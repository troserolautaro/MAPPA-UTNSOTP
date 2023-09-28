#include "Kernel.h"

/*Esta funcion lo que hace es utilizar la libreria READLINE para poder leer en consola*/

char* lectura_consola(void){
	char* linea=malloc(sizeof(char*));
	linea = readline(">>");
	if (linea) {
		add_history(linea);
	}
	return linea;
}

/*Esta funcion valida que es lo que se escribio por consola y lo asigna a un valor del enumerador de funciones de
la consola (PROBABLEMENTE SE PUEDA MEJORAR)*/
int validacion_contenido_consola(char* comando){
	if(!strcasecmp(comando, "iniciar_proceso")){

		return INICIAR_PROCESO;
	}
	if(!strcasecmp(comando, "finalizar_proceso")){

		return FINALIZAR_PROCESO;
	}
	if(!strcasecmp(comando, "iniciar_planificacion")){

		return INICIAR_PLANIFICACION;
	}
	if(!strcasecmp(comando, "detener_planificacion")){

		return DETENER_PLANIFICACION;
	}
	if(!strcasecmp(comando, "multiprogramacion")){

		return MULTIPROGRAMACION;
	}
	if(!strcasecmp(comando, "proceso_estado")){

		return PROCESO_ESTADO;
	}

	return -2;
}

pidGlobal = 0;

int main(void)
{
	char* ipCPU,* ipMemoria,* ipFileSystem ;
	char* puertoCPUDispatch,*puertoCPUInterrupt,* puertoMemoria,* puertoFileSystem ;
	char* valor;

	char* AlgoritmoPlanificacion,* quantum,* recursos,* instanciasRecursos,* gradoMultiprogramacion;
	t_log* logger=malloc(sizeof(logger));
	t_config* config=malloc(sizeof(config));
	int conexionCPUDispatch, conexionCPUInterrupt,conexionMemoria,conexionFileSystem;
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
//	conexionMemoria = crear_conexion(ipMemoria, puertoMemoria);
//	conexionFileSystem = crear_conexion(ipFileSystem, puertoFileSystem);

	/************************************INICIALIZAR HILOS DE ENVIO Y RECIBO DE MENSAJES************************************/

	// Enviamos al servidor el valor de CLAVE como mensaje
//	enviar_mensaje(valor,conexionCPUDispatch);
//	enviar_mensaje(valor,conexionCPUInterrupt);
//	enviar_mensaje(valor,conexionMemoria);
//	enviar_mensaje(valor,conexionFileSystem);

	// Armamos y enviamos el paquete
//	paquete(conexionCPUDispatch);
//	paquete(conexionCPUInterrupt);
//	paquete(conexionMemoria);
//	paquete(conexionFileSystem);


	while(1){
		char* comando = lectura_consola();
		int idComando = validacion_contenido_consola(comando);
		/*PROBABLEMENTE HAY QUE MEJORAR ESTO, SI BIEN FUNCIONA NO TOMA LOS PARAMETROS QEU SE INGRESAN EN
		 * CONSOLA */
		switch(idComando){
			case INICIAR_PROCESO:
				printf("INICIAR PROCESO \n");
				enviar_mensaje("INICIAR PROCESO",conexionCPUDispatch);
			break;

			case FINALIZAR_PROCESO:
				printf("FINALIZAR PROCESO \n");
				enviar_mensaje("FINALIZAR PROCESO",conexionCPUDispatch);
			break;

			case INICIAR_PLANIFICACION:
				printf("INICIAR PLANIFICACION \n");
				enviar_mensaje("INICIAR PLANIFICACION",conexionCPUDispatch);
			break;

			case DETENER_PLANIFICACION:
				printf("DETENER PLANIFICACION \n");
				enviar_mensaje("DETENER PLANIFICACION",conexionCPUDispatch);
			break;

			case MULTIPROGRAMACION:
				printf("MULTIPROGRAMACION \n");
				enviar_mensaje("MULTIPROGRAMACION",conexionCPUDispatch);
			break;

			case PROCESO_ESTADO:
				printf("PROCESO ESTADO \n");
				enviar_mensaje("PROCESO ESTADO",conexionCPUDispatch);
			break;

			case -1:
				printf("Saliendo! \n");
				enviar_mensaje("Me desconecte",conexionCPUDispatch);
			break;
			default:
				printf("No se reconocio el comando \n");
			break;
		}
		free(comando);

	}


	/************************************INICIO CONSOLA INTERACTIVA*************************************************/

	/************************************FINALIZA LOS PROGRAMAS O HILOS A FUTURO************************************/

//	terminar_programa(conexionCPUDispatch, logger, config);
//	terminar_programa(conexionCPUInterrupt, logger, config);
	terminar_programa(conexionMemoria, logger, config);
//	terminar_programa(conexionFileSystem, logger, config);
}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger =log_create("./tp.log","log",1,LOG_LEVEL_INFO);
	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config = config_create("./Kernel.config");
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
	        free(leido);
	    }
}

void paquete(int conexion)
{
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

PCB* iniciar_proceso(char* path, int size, int prioridad){

	pidGlobal++;

	PCB* proceso = malloc(sizeof(PCB));

	proceso->estado = NEW;
	proceso->prioridad = prioridad;

	proceso->registros->AX = 0;
	proceso->registros->BX = 0;
	proceso->registros->CX = 0;
	proceso->registros->DX = 0;

	proceso->pid = pidGlobal; //Modificar en caso de que sea necesario

}


void finalizar_proceso(PCB* proceso, int pid){
	proceso->estado = EXIT;

}
