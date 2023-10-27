#include "Memoria.h"

t_log* logger;
t_config* config;

//funcion de prueba, se puede borrar
/*
void * mostrar_instrucciones(void* elemento) {
    t_instruccion* valor = (t_instruccion*)elemento;
    printf("Elemento: %s\n", valor->comando);
    return true;
}
*/
int main(void) {
	logger = malloc(sizeof(t_log));
	config = malloc(sizeof(t_config));

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

	//CARGA INSTRUCCIONES
	t_list* listaInstrucciones =cargar_instrucciones(pathInstrucciones,"");
	//INICIAR SERVIDOR
	int serverMemoria = iniciar_servidor(puertoEscucha);

	//printf("%ld \n %ld", (long)getpid(), (long)getppid());
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	log_info(logger, "Servidor listo para recibir al cliente");

	//list_iterate(instrucciones, mostrar_instrucciones);
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
					int pc =1;
					char* instruccion=malloc(sizeof(char) * (200 + 1));
					instruccion=list_get(listaInstrucciones,pc);
	 	    		 printf("\n el comando es  %s \n", instruccion);
					enviar_mensaje(instruccion,cliente_fd);
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

//LEER INSTRUCCIONES DEL PSEUDOCODIGO  Y CARGARLAS EN MEMORIA
//se probo y funciona
t_list* cargar_instrucciones(char* path, char* file){
	FILE * fileInstrucciones = malloc(sizeof(FILE));
		t_list * instrucciones,*lineasDeCodigo;
		instrucciones= list_create();
		lineasDeCodigo= list_create();
		char direccionIns[100];
		//une la ruta de los archivos, con el archivo especificado
		//cambiar el archivo por variable a futuro, no prioritario de momento
		strcat(strcpy(direccionIns, path), "/instrucciones.txt");
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
		 	        strncpy(lineaTemporal, lineaDeCodigo, sizeof(lineaTemporal));
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
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);

	if(!strcasecmp(msg,"conexion")){
		log_info(logger,"Hola! %d",*(int*)list_get(mensaje,1));
	}

}
