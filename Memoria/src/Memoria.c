#include "Memoria.h"

t_log* logger;
t_config* config;
int conexionKernel, conexionDispatch,conexionFileSystem;
pthread_t hiloRecibirCliente, hiloKernel,hiloCPU;
int serverMemoria;
t_dictionary *archivosCargados;

int main(void) {
	logger = malloc(sizeof(t_log));
	config = malloc(sizeof(t_config));

	logger = iniciar_logger("./log.log");
	config = iniciar_config("./Memoria.config");

	char* ipFyleSystem = malloc(sizeof(char*)),
		*puertoEscucha = malloc(sizeof(char*)),
		*puertoFyleSystem = malloc(sizeof(char*));
	tamMemoria = malloc(sizeof(char*));
	tamPagina = malloc(sizeof(char*));
	retardoRespuesta = malloc(sizeof(char*));
	algoritmoReemplazo = malloc(sizeof(char*));
	pathInstrucciones = malloc(sizeof(char*));
	//CONFIGURACION DE MEMORIA
	ipFyleSystem = config_get_string_value(config,"IP_FILESYSTEM");
	puertoFyleSystem = config_get_string_value(config,"PUERTO_FYLESYSTEM");
	puertoEscucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	tamMemoria = config_get_string_value(config,"TAM_MEMORIA");
	tamPagina = config_get_string_value(config,"TAM_PAGINA");
	pathInstrucciones = config_get_string_value(config,"PATH_INSTRUCCIONES");
	retardoRespuesta = config_get_string_value(config,"RETARDO_RESPUESTA");
	algoritmoReemplazo = config_get_string_value(config,"ALGORITMO_REEMPLAZO");
	iniciar_memoria_usuario();
	//INICIAR SERVIDOR
	serverMemoria = iniciar_servidor(puertoEscucha);
	archivosCargados=dictionary_create();
	//printf("%ld \n %ld", (long)getpid(), (long)getppid());
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	log_info(logger, "Servidor listo para recibir al cliente");
	//pthread_t hiloFileSystem
	//LA ESPERA DE CLIENTE SE PUEDE ENCAPSULAR PERO NO ES PRIORIDAD DE MOMENTO
	int resultado;
	if ((resultado=pthread_create(&hiloRecibirCliente,NULL,(void *)recibir_conexiones,( void *) &serverMemoria))!=0)
		printf("Error al crear hilo. resultado %d",resultado);
			//}
	pthread_join(hiloKernel,NULL);
	pthread_join(hiloCPU,NULL);
	pthread_join(hiloRecibirCliente,NULL);
	return EXIT_SUCCESS;
}


void procesar_mensaje(t_list* mensaje){
	char* msg = malloc(sizeof(char*));
	msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
	 if(!strcasecmp(msg,"cargar")){

		int pid=*(int*)list_get(mensaje,1);

		char* path=string_new();
		string_append(&path,(char*)list_get(mensaje,2));

		int size=*(int*)list_get(mensaje,3);
		t_list* instrucciones=list_create();

		instrucciones=cargar_instrucciones(&path);
		dictionary_put(archivosCargados,string_itoa(pid),instrucciones); //Acordarse liberar diccionario

		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"cargado",sizeof(char *)*8);
		agregar_a_paquete(paquete,&pid,sizeof(int));
		enviar_paquete(paquete,conexionKernel);
		eliminar_paquete(paquete);

		free(path);
	}
	 if(!strcasecmp(msg,"instruccion")){
		int pid =*(int*)list_get(mensaje,1);
		int pc =*(int*)list_get(mensaje,2);

		t_list* listaInstrucciones =dictionary_get(archivosCargados,string_itoa(pid));
		char* instruccion=string_new();
		string_append(&instruccion,(char *)list_get(listaInstrucciones,pc));
		 printf("\n el comando es  %s \n", instruccion);

		 t_paquete* paquete=crear_paquete();
		agregar_a_paquete(paquete,"instruccion",sizeof(char*)*11);
		agregar_a_paquete(paquete,instruccion,sizeof(instruccion));
		enviar_paquete(paquete,conexionDispatch);
		eliminar_paquete(paquete);
		//enviar_mensaje(instruccion,cliente_fd);
	}
	free(msg);
}
