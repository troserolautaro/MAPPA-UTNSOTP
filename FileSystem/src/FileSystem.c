#include "FileSystem.h"

#define valor_EOF UINT32_MAX  //Este valor representa al EOF (End of File)

t_dictionary* tablaFat;

char* ipMemoria;
char* puertoEscucha,* puertoMemoria;
char* path_fat, * path_bloques, * path_fcb;
char* cant_bloques_total,* cant_bloques_swap;
char* tam_bloque;
char* retardo_acceso_bloque,* retardo_acceso_fat;
int tamanio_fat;  //Tamaño de los bloques FAT (lo defino en el main)
pthread_mutex_t mutexLog;

int main(void) {
	tablaFat = dictionary_create();
	t_log *logger = iniciar_logger("./log.log");
	t_config* config=iniciar_config("./FileSystem.config");

	//CONFIGURACION DE FILESYSTEM
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria = config_get_string_value(config,"PUERTO_MEMORIA");
	puertoEscucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	path_fat = config_get_string_value(config,"PATH_FATH");
	path_bloques = config_get_string_value(config,"PATH_BLOQUES");
	path_fcb = config_get_string_value(config,"PATH_FCB");
	cant_bloques_total = config_get_string_value(config,"CANT_BLOQUES_TOTAL");
	cant_bloques_swap = config_get_string_value(config,"CANT_BLOQUES_SWAP");
	tam_bloque = config_get_string_value(config,"TAM_BLOQUE");
	retardo_acceso_bloque = config_get_string_value(config,"RETARDO_ACCESO_BLOQUE");
	retardo_acceso_fat = config_get_string_value(config,"RETARDO_ACCESO_FAT");
	tamanio_fat = (cant_bloques_total - cant_bloques_swap) * sizeof(uint32_t);

	int server_fd = iniciar_servidor(puertoEscucha);
	escritura_log("Servidor listo para recibir al cliente");

	int cliente_fd = esperar_cliente(server_fd);
	FILE *archivobloques = fopen("archivobloques.bin", "w");
	if (archivobloques == NULL) {
		perror("Error al abrir archivo");
		return 1;
	}
	fclose(archivobloques);
	return EXIT_SUCCESS;
}


void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);

	if(!strcasecmp(msg,"conexion")){
		char* mensaje = string_from_format("Hola! %d",*(int*)list_get(mensaje,1));
		escritura_log(mensaje);
		free(mensaje);
	}

}

//Comunicacion con kernel


int chequear_existencia_archivo(FCB *archivo){
	  return dictionary_has_key(tablaFat,string_itoa( archivo->bloqueInicial));
}


t_config* abrir_archivo(char* archivo){
	t_config* archivoReturn = config_create(archivoReturn) ;
	if(archivoReturn != NULL)
		return archivoReturn;

	printf ("No existe el archivo");

	return NULL;
}


bool crear_archivo(char* nombreArchivo){
	char * path=malloc(sizeof(char*)*100);
	strcpy(path, path_fcb);
	string_append(&path, "/");
	string_append(&path,nombreArchivo);
	FILE * nuevofcb = fopen(path,"w");

	fclose(nuevofcb);
	return true;
}

void crear_fcb(char* nombreArchivo){
	FCB archivo;

	/*archivo->nombreArchivo = nombreArchivo;
	archivo->tamanioArchivo = 0;
	archivo->bloqueInicial = NULL;
	*/
}
/*
void truncar_archivo(int tamanioNuevo, FCB archivo, int tamanio){ //situacionDeseada : ampliar o reducir tamanio
	archivo->tamanioArchivo = tamanioNuevo;
	if(tamanioNuevo > archivo->tamanioArchivo){
		//no entendi lo de los bloques
	}
	else if(tamanioNuevo < archivo->tamanioArchivo){
		//no entendi lo de los bloques
	}

}
*/

void leer_archivo(FCB archivo){
	//comunicacion con memoria
}

void escribir_archivo(){

}

/*-----------------------*/

//Comunicacion con Memoria

void iniciar_proceso(){

}

void finalizar_proceso(){

}

/*------------------------*/
