#include "Memoria.h"

t_log* logger;
t_config* config;
int conexionKernel, conexionDispatch,conexionFileSystem;
pthread_t hiloRecibirCliente, hiloKernel,hiloCPU;
char * pathInstrucciones;
int serverMemoria;
t_dictionary *archivosCargados;

int main(void) {
	logger = malloc(sizeof(t_log));
	config = malloc(sizeof(t_config));

	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	config = iniciar_config();

	char* ipFyleSystem = malloc(sizeof(char*)),
		*puertoEscucha = malloc(sizeof(char*)),
		*tamMemoria = malloc(sizeof(char*)),
		*tamPagina = malloc(sizeof(char*)),
		*retardoRespuesta = malloc(sizeof(char*)),
		*algoritmoReemplazo = malloc(sizeof(char*)),
		*puertoFyleSystem = malloc(sizeof(char*));

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

	//INICIAR SERVIDOR
	serverMemoria = iniciar_servidor(puertoEscucha);
	archivosCargados=dictionary_create();
	//printf("%ld \n %ld", (long)getpid(), (long)getppid());
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	log_info(logger, "Servidor listo para recibir al cliente");
	//pthread_t hiloFileSystem
	//LA ESPERA DE CLIENTE SE PUEDE ENCAPSULAR PERO NO ES PRIORIDAD DE MOMENTO
	int resultado;
	if ((resultado=pthread_create(&hiloRecibirCliente,NULL,recibir_conexiones,( void *) &serverMemoria))!=0)
		printf("Error al crear hilo. resultado %d",resultado);
			//}
	pthread_join(hiloKernel,NULL);
	pthread_join(hiloCPU,NULL);
	pthread_join(hiloRecibirCliente,NULL);
	return EXIT_SUCCESS;
}

//LEER INSTRUCCIONES DEL PSEUDOCODIGO  Y CARGARLAS EN MEMORIA
//se probo y funciona
t_list* cargar_instrucciones(char* file){
	FILE * fileInstrucciones = malloc(sizeof(FILE));
		t_list *lineasDeCodigo;
		lineasDeCodigo= list_create();
		char * direccionIns=malloc(sizeof(char*)*100);
		//une la ruta de los archivos, con el archivo especificado
		//cambiar el archivo por variable a futuro, no prioritario de momento
		strcpy(direccionIns, pathInstrucciones);
		string_append(&direccionIns, "/");
		string_append(&direccionIns,file);
		//abre el archivo en modo lectura
		fileInstrucciones = fopen ( direccionIns, "r");
		//t_instruccion instruccion;
		//instruccion.comando=malloc(sizeof(char*));
		//instruccion.parametros= list_create();
		//char parametros[2][30];
		char* lineaDeCodigo=malloc(sizeof(char) * 100 + 1);
		if (fileInstrucciones == NULL)
		    {
			 printf("no hay insctrucciones o hubo error con el archivo");
		    }
		 	else
		    {
		 	    printf("\nEl contenido del archivo de prueba es \n");
		 	    while (fgets(lineaDeCodigo,sizeof(char) * 100,fileInstrucciones )!=NULL)
		 	    {
		 			char* lineaTemporal=malloc(sizeof(char) * 100 + 1);
		 			lineaDeCodigo[strcspn(lineaDeCodigo, "\n")] = '\0';
		 	        strncpy(lineaTemporal, lineaDeCodigo, sizeof(lineaDeCodigo));
			 	    printf("p = %s\n",lineaTemporal);
		 	    	list_add(lineasDeCodigo,lineaTemporal);
		 	    }
		    }
	    fclose(fileInstrucciones);
		    char* primerComando=malloc(sizeof(char) * 100 + 1);
		    primerComando=list_get(lineasDeCodigo,0);
		    printf("Línea %d: %s\n", 0, primerComando);
			printf("\n tamaño de lista %d \n", list_size(lineasDeCodigo));

		    return lineasDeCodigo;
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
void procesar_mensaje(t_list* mensaje){
	char* msg = malloc(sizeof(char*));
	msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
	 if(!strcasecmp(msg,"cargar")){
		printf("llego iniciar planificacion");
		char* path=malloc(sizeof(char*));
		path=list_get(mensaje,2);
		int pid=*(int*)list_get(mensaje,1);
		int size=*(int*)list_get(mensaje,3);
		t_list* instrucciones=list_create();
		instrucciones=cargar_instrucciones(path);
		dictionary_put(archivosCargados,string_itoa(pid),instrucciones);
	}
	 if(!strcasecmp(msg,"instruccion")){
		int pid =*(int*)list_get(mensaje,1);
		int pc =*(int*)list_get(mensaje,2);
		t_list* listaInstrucciones =dictionary_get(archivosCargados,string_itoa(pid));
		char* instruccion=malloc(sizeof(char) * (200 + 1));
		instruccion=list_get(listaInstrucciones,pc);
		 printf("\n el comando es  %s \n", instruccion);
		 t_paquete* paquete=crear_paquete();
		agregar_a_paquete(paquete,"instruccion",sizeof(char*)*11);
		//agregar_a_paquete(paquete,&pid,sizeof(int*));
		agregar_a_paquete(paquete,instruccion,sizeof(instruccion));
		enviar_paquete(paquete,conexionDispatch);
		eliminar_paquete(paquete);
		//enviar_mensaje(instruccion,cliente_fd);
	}
	free(msg);
}
