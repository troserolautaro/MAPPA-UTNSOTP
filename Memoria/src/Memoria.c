#include "Memoria.h"

t_log* logger;
t_config* config;
int conexionKernel, conexionDispatch,conexionFileSystem;
pthread_t hiloRecibirCliente;
char * pathInstrucciones;
int serverMemoria;
t_dictionary *archivosCargados;

int main() {
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
	if ((resultado=pthread_create(&hiloRecibirCliente,NULL,(void *)recibir_conexiones,( void *) &serverMemoria))!=0)
		printf("Error al crear hilo. resultado %d",resultado);
			//}
	pthread_join(hiloRecibirCliente,NULL);
	return EXIT_SUCCESS;
}

//LEER INSTRUCCIONES DEL PSEUDOCODIGO  Y CARGARLAS EN MEMORIA
//se probo y funciona
t_list* cargar_instrucciones(char** file){
	FILE * fileInstrucciones;
		t_list *listaInstrucciones = list_create();
		char * direccionIns=string_new();
		//une la ruta de los archivos, con el archivo especificado
		//cambiar el archivo por variable a futuro, no prioritario de momento
		string_append(&direccionIns, pathInstrucciones);
		string_append(&direccionIns, "/");
		string_append(&direccionIns,*file);
		//abre el archivo en modo lectura
		fileInstrucciones = fopen(direccionIns, "r");

		if (fileInstrucciones == NULL)
		    {
			 printf("no hay insctrucciones o hubo error con el archivo");
		    }
		 	else
		    {
		 	    printf("\nEl contenido del archivo de prueba es \n");

			char* lineaDeCodigo=NULL;
			size_t lineLength = 0;
			while (getline(&lineaDeCodigo, &lineLength, fileInstrucciones) != -1) {
				if (lineaDeCodigo[strlen(lineaDeCodigo) - 1] == '\n') {
					lineaDeCodigo[strlen(lineaDeCodigo) - 1] = '\0';
				}
				char* lineaTemporal = strdup(lineaDeCodigo);
				list_add(listaInstrucciones, lineaTemporal);
			}
				free(lineaDeCodigo);
			}
			fclose(fileInstrucciones);
			printf("\n tamaño de lista %d \n", list_size(listaInstrucciones));
			return listaInstrucciones;
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
		t_list* instrucciones=cargar_instrucciones(&path);
		list_add(instrucciones,"1");
		printf("La primera linea es : %s",(char *)list_get(instrucciones,0));
		dictionary_put(archivosCargados,string_itoa(pid),instrucciones); //Acordarse liberar diccionario
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"cargado",sizeof("cargado"));
		agregar_a_paquete(paquete,&pid,sizeof(int));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
		list_destroy(instrucciones);
		free(path);
	}
	 if(!strcasecmp(msg,"instruccion")){
		uint32_t pid =*(uint32_t*)list_get(mensaje,1);
		uint32_t pc =*(uint32_t*)list_get(mensaje,2);

		t_list* listaInstrucciones =(t_list*)dictionary_get(archivosCargados,string_itoa(pid));
		char* instruccion=(char*)list_get(listaInstrucciones,pc);

		t_paquete* paquete=crear_paquete();
		agregar_a_paquete(paquete,"instruccion",sizeof("instruccion"));
		agregar_a_paquete(paquete,instruccion,sizeof(instruccion));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
		list_destroy(listaInstrucciones);
		free(instruccion);
	}
	free(msg);
}
