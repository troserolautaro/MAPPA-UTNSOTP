#include "FileSystem.h"

#define valor_EOF UINT32_MAX  //Este valor representa al EOF (End of File)

t_dictionary* tablaFat;
pthread_t hiloRecibirCliente;
int serverFilesystem;
char* ipMemoria;
char* puertoEscucha,* puertoMemoria;
char* path_fat, * path_bloques, * path_fcb;
int cant_bloques_total,cant_bloques_swap;
int tam_bloque;
int retardo_acceso_bloque,retardo_acceso_fat;
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
	cant_bloques_total = config_get_int_value(config,"CANT_BLOQUES_TOTAL");
	cant_bloques_swap = config_get_int_value(config,"CANT_BLOQUES_SWAP");
	tam_bloque = config_get_int_value(config,"TAM_BLOQUE");
	retardo_acceso_bloque = config_get_int_value(config,"RETARDO_ACCESO_BLOQUE");
	retardo_acceso_fat = config_get_int_value(config,"RETARDO_ACCESO_FAT");
	tamanio_fat = (cant_bloques_total - cant_bloques_swap) * sizeof(uint32_t);

	serverFilesystem = iniciar_servidor(puertoEscucha);
	escritura_log("Servidor listo para recibir al cliente");
	int resultado;
	if ((resultado=pthread_create(&hiloRecibirCliente,NULL,(void *)recibir_conexiones,( void *) &serverFilesystem))!=0)
		printf("Error al crear hilo. resultado %d",resultado);
		pthread_join(hiloRecibirCliente,NULL);
	FILE *archivobloques = fopen("archivobloques.bin", "w");
	if (archivobloques == NULL) {
		perror("Error al abrir archivo");
		return 1;
	}
	fclose(archivobloques);
	return EXIT_SUCCESS;
}

int chequear_existencia_archivo(FCB *archivo){
	  return dictionary_has_key(tablaFat,string_itoa( archivo->bloqueInicial));
}


int abrir_archivo(char* archivo){
	t_config* configArchivofcb = iniciar_config(archivo);
	int tamaño = config_get_int_value(configArchivofcb,"TAMAÑO_ARCHIVO");
	return tamaño;
}


bool crear_archivo(char* nombreArchivo){
	//hace la conversion de la direccion
	char * path=malloc(sizeof(char*)*100);
	strcpy(path, path_fcb);
	string_append(&path, "/");
	string_append(&path,nombreArchivo);
	//abre el archivo en forma w para crearlo y lo cierra
	FILE * nuevofcb = fopen(path,"w");
	fclose(nuevofcb);
	//lo abre como config para cargarle los datos principales
	t_config * nuevoArchivo= iniciar_config(path);
	config_set_value(nuevoArchivo, "NOMBRE_ARCHIVO", nombreArchivo);
	config_set_value(nuevoArchivo, "TAMANIO_ARCHIVO", 0);
	//config_set_value(nuevoArchivo, "BLOQUE_INICIAL", nombreArchivo);
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

void leer_archivo( ){
	//comunicacion con memoria
}

void escribir_archivo(){

}

/*-----------------------*/

//Comunicacion con Memoria

void iniciar_proceso(){
	//RESERVAR BLOQUES DE SWAP
}

void finalizar_proceso(){
	//LIBERAR BLOQUES DE SWAP
}

void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
	int conexion = *(int*) (list_get(mensaje,list_size(mensaje)-1));
	//peticiones del kernel
	if(!strcasecmp(msg,"abrir archivo")){
		int tamaño =abrir_archivo(((char*)list_get(mensaje,1)));
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"tamaño",sizeof("tamaño"));
		agregar_a_paquete(paquete,tamaño,sizeof(int));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}

	if(!strcasecmp(msg,"crear archivo")){
		t_paquete * paquete = crear_paquete();

		if(crear_archivo(((char*)list_get(mensaje,1)))){
			agregar_a_paquete(paquete,"archivo creado",sizeof("archivo creado"));
			enviar_paquete(paquete,conexion);
			eliminar_paquete(paquete);
		}
		else{
			agregar_a_paquete(paquete,"archivo no creado",sizeof("archivo no creado"));
			enviar_paquete(paquete,conexion);
			eliminar_paquete(paquete);
		}
	}
	if(!strcasecmp(msg,"leer archivo")){
		leer_archivo();
	}
	if(!strcasecmp(msg,"escribir archivo")){
		escribir_archivo();
	}
	//peticiones de memoria
	if(!strcasecmp(msg,"iniciar proceso")){
		iniciar_proceso();
	}
	if(!strcasecmp(msg,"finalizar proceso")){
		finalizar_proceso();
	}
}
