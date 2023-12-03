#include "Memoria.h"

t_log* logger;
t_config* config;
int conexionKernel, conexionDispatch,conexionFileSystem;
pthread_t hiloRecibirCliente;
int serverMemoria;
t_dictionary *archivosCargados;
pthread_mutex_t mutexLog,mutexArchivos;

int main() {
//	logger = malloc(sizeof(t_log)); No es necesario
//	config = malloc(sizeof(t_config));

	logger = iniciar_logger("./log.log");
	config = iniciar_config("./Memoria.config");

	char* ipFyleSystem = malloc(sizeof(char*)),
		* puertoEscucha = malloc(sizeof(char*)),
		* puertoFyleSystem = malloc(sizeof(char*));
	algoritmoReemplazo = malloc(sizeof(char*));
	pathInstrucciones = malloc(sizeof(char*));


	//Semaforos
	pthread_mutex_init(&mutexLog,NULL);
	pthread_mutex_init(&mutexArchivos,NULL);
	//CONFIGURACION DE MEMORIA
	ipFyleSystem = config_get_string_value(config,"IP_FILESYSTEM");
	puertoFyleSystem = config_get_string_value(config,"PUERTO_FYLESYSTEM");
	puertoEscucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	tamMemoria = config_get_int_value(config,"TAM_MEMORIA");
	tamPagina = config_get_int_value(config,"TAM_PAGINA");
	pathInstrucciones = config_get_string_value(config,"PATH_INSTRUCCIONES");
	retardoRespuesta = config_get_int_value(config,"RETARDO_RESPUESTA");
	algoritmoReemplazo = config_get_string_value(config,"ALGORITMO_REEMPLAZO");
	iniciar_memoria_usuario();
	//INICIAR SERVIDOR
	serverMemoria = iniciar_servidor(puertoEscucha);
	archivosCargados=dictionary_create();
	//printf("%ld \n %ld", (long)getpid(), (long)getppid());

	debug("Servidor listo para recibir al cliente");
	//conexionFileSystem=crear_conexion(ipFyleSystem, puertoFyleSystem,MEMORIA);
	//pthread_t hiloFileSystem;
	//pthread_create(&hiloFileSystem,NULL,manejar_cliente,&conexionFileSystem);

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
	if(!strcasecmp(msg,"tamañoPagina")){
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"tamañoPagina",sizeof("tamañoPagina"));
		agregar_a_paquete(paquete,&tamPagina,sizeof(int));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	if(!strcasecmp(msg,"mov_in")){
		uint32_t direccionFisica=*(uint32_t*)list_get(mensaje,1);
		bool error=true;
		uint32_t dato=-1;
		//valida si es una direccion fisica valida
		if(direccionFisica<tamMemoria){
		error=false;
		dato=get_dato(*(uint32_t*)list_get(mensaje,1));
		}
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"mov_in",sizeof("mov_in"));
		agregar_a_paquete(paquete,&error,sizeof(bool));
		agregar_a_paquete(paquete,&dato,sizeof(int));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	if(!strcasecmp(msg,"mov_out")){
		uint32_t direccionFisica=*(uint32_t*)list_get(mensaje,1);
		bool error=true;
		if(direccionFisica<tamMemoria){
			error=false;
			set_dato(*(uint32_t*)list_get(mensaje,1),*(uint32_t*)list_get(mensaje,2));
		}
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"mov_out",sizeof("mov_out"));
		agregar_a_paquete(paquete,&error,sizeof(bool));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	if(!strcasecmp(msg,"page_fault")){
		uint32_t pid, numPagina;
		pid=*(uint32_t*)list_get(mensaje,1);
		numPagina=*(uint32_t*)list_get(mensaje,2);
		page_fault(pid,numPagina);
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"page_fault",sizeof("page_fault"));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	if(!strcasecmp(msg,"cargar")){//cambiar cargar por crear proceso, y ver si se puede encapsular la parte de instrucciones

		uint32_t pid=*(uint32_t*)list_get(mensaje,1);
		char* path=string_new();
		string_append(&path,(char*)list_get(mensaje,2));

		uint32_t size=*(uint32_t*)list_get(mensaje,3);
		t_list* instrucciones=list_create();
		instrucciones=cargar_instrucciones(&path);
		//list_add(instrucciones,"1");
		printf("La primera linea es : %s",(char *)list_get(instrucciones,0));

		pthread_mutex_lock(&mutexArchivos);
		dictionary_put(archivosCargados,string_itoa(pid),instrucciones);
		pthread_mutex_unlock(&mutexArchivos);
		//crea las estructuras para la memoria de usuario del proceso
		crear_proceso(pid,path,size);
		//Acordarse liberar diccionario
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"cargado",sizeof("cargado"));
		agregar_a_paquete(paquete,&pid,sizeof(int*));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
		//free(instrucciones); puede que esto sea mejor porque el dictionary put te dice que el elemento no se libera
		free(path);
	}
	if(!strcasecmp(msg,"finalizar_proceso")){
		uint32_t pid=*(uint32_t*)list_get(mensaje,1);
		//liberar diccionario de instrucciones
		void finalizar_proceso(pid);
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"proceso finalizado",sizeof("proceso finalizado"));
		agregar_a_paquete(paquete,&pid,sizeof(int*));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	 if(!strcasecmp(msg,"instruccion")){
		uint32_t pid =*(uint32_t*)list_get(mensaje,1);
		uint32_t pc =*(uint32_t*)list_get(mensaje,2);

		pthread_mutex_lock(&mutexArchivos);
		t_list* listaInstrucciones =(t_list*)dictionary_get(archivosCargados,string_itoa(pid));
		pthread_mutex_unlock(&mutexArchivos);

		char* instruccion=string_new();
		string_append(&instruccion,(char*)list_get(listaInstrucciones,pc));
		t_paquete* paquete=crear_paquete();
		agregar_a_paquete(paquete,"instruccion",sizeof("instruccion"));
		if(!strcasecmp(instruccion,"EXIT")){
			agregar_a_paquete(paquete,instruccion,strlen(instruccion)+1);
		}else{
			char** parametros = string_array_new();
			parametros = string_n_split(instruccion,3," ");
			for(int i = 0; parametros[i]!=NULL; i++){
				agregar_a_paquete(paquete,parametros[i],strlen(parametros[i])+1);
			}
		}
		usleep(retardoRespuesta*1000);
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
		free(instruccion);
	}
	free(msg);
}
