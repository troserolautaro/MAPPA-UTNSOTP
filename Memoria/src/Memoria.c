#include "Memoria.h"

t_log* logger;
t_config* config;
int conexionKernel, conexionDispatch,conexionFileSystem;
pthread_t hiloRecibirCliente;
int serverMemoria;
t_dictionary *archivosCargados;

int main() {
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
	pthread_join(hiloRecibirCliente,NULL);
	return EXIT_SUCCESS;
}

void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
	int conexion = *(int*) (list_get(mensaje,list_size(mensaje)-1));
	//Seria excelente cuanto menos aprovechar que dentro de la lista "mensaje" se encuentra al final el socket para dividir con un switch las funciones
	 if(!strcasecmp(msg,"cargar")){

		int pid=*(int*)list_get(mensaje,1);
		char* path=string_new();
		string_append(&path,(char*)list_get(mensaje,2));

		int size=*(int*)list_get(mensaje,3);
		t_list* instrucciones=list_create();
		instrucciones=cargar_instrucciones(&path);
		//list_add(instrucciones,"1");
		printf("La primera linea es : %s",(char *)list_get(instrucciones,0));
		dictionary_put(archivosCargados,string_itoa(pid),instrucciones); //Acordarse liberar diccionario
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"cargado",sizeof("cargado"));
		agregar_a_paquete(paquete,&pid,sizeof(int));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
		//free(instrucciones); puede que esto sea mejor porque el dictionary put te dice que el elemento no se libera
		free(path);
	}
	 if(!strcasecmp(msg,"instruccion")){
		uint32_t pid =*(uint32_t*)list_get(mensaje,1);
		uint32_t pc =*(uint32_t*)list_get(mensaje,2);

		t_list* listaInstrucciones =(t_list*)dictionary_get(archivosCargados,string_itoa(pid));
		char* instruccion=string_new();
		string_append(&instruccion,(char*)list_get(listaInstrucciones,pc));

		char** parametros = string_array_new();
				parametros = string_n_split(instruccion,3," ");
		t_paquete* paquete=crear_paquete();
		agregar_a_paquete(paquete,"instruccion",sizeof("instruccion"));
		//agregar_a_paquete(asdkjasd, instruccion, sizeof(instruccion));
		for(int i = 0; parametros[i]!=NULL; i++){
			agregar_a_paquete(paquete,parametros[i],sizeof(parametros[i]));
		}
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
		free(instruccion);
	}
	free(msg);
}
