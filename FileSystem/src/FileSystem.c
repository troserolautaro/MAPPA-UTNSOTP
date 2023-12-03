#include "FileSystem.h"

#define valor_EOF UINT32_MAX  //Este valor representa al EOF (End of File)

t_dictionary* tablaFCB;
bloque_t *bloques;
pthread_t hiloRecibirCliente;
int serverFilesystem;
char* ipMemoria;
char* puertoEscucha,* puertoMemoria;
char* path_fat, * path_bloques, * path_fcb;
int cant_bloques_total,cant_bloques_swap, cant_bloques_fat;
int tam_bloque;
int retardo_acceso_bloque,retardo_acceso_fat;
int tamanio_fat;  //Tamaño de los bloques FAT (lo defino en el main)
pthread_mutex_t mutexLog;

int main(void) {
	tablaFCB = dictionary_create();
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
	cant_bloques_fat=(cant_bloques_total - cant_bloques_swap);
	tamanio_fat =  cant_bloques_fat* sizeof(uint32_t);

	serverFilesystem = iniciar_servidor(puertoEscucha);
	escritura_log("Servidor listo para recibir al cliente");
	int resultado;
	if ((resultado=pthread_create(&hiloRecibirCliente,NULL,(void *)recibir_conexiones,( void *) &serverFilesystem))!=0)
		printf("Error al crear hilo. resultado %d",resultado);
		pthread_join(hiloRecibirCliente,NULL);
	//abrir los archivos o crearlos si no existe
	iniciar_bloques();
	iniciar_swap();
	iniciar_fat();
	return EXIT_SUCCESS;
}


void iniciar_bloques(){
	FILE *archivoBloques = fopen(path_bloques, "rb+");
		if (archivoBloques == NULL) {
			archivoBloques = fopen(path_fat, "wb+");
			if (archivoBloques == NULL) {
				perror("Error al abrir archivo");
				//return 1;
			}
			bloques=malloc(sizeof(bloque_t)*cant_bloques_total);
			for (uint32_t i = 0; i < cant_bloques_swap; ++i) {
				bloques[i]=iniciar_bloque();
			}
		}
}
bloque_t* iniciar_bloque(){
	bloque_t * nuevoBloque=malloc(sizeof(bloque_t));
	nuevoBloque->dueño=NULL;
	nuevoBloque->valor=malloc(sizeof(bloque_t) * tam_bloque);
	return nuevoBloque;
}

void iniciar_swap(){
	for (uint32_t i = 0; i < cant_bloques_swap; ++i) {
		bloques[i]=iniciar_bloque();
	}
}

void iniciar_fat(){
	FILE *archivoFAT = fopen(path_fat, "rb+");
	if (archivoFAT == NULL) {
		archivoFAT = fopen(path_fat, "wb+");
		if (archivoFAT == NULL) {
			perror("Error al abrir archivo");
			//return 1;
		}
		for (uint32_t i = 0; i < cant_bloques_fat; ++i) {
			fwrite(0, sizeof(uint32_t), 1, archivoFAT);
			//carga los bloques en 0
			bloques[cant_bloques_swap+i]=iniciar_bloque();
		}
	}
	else{
		//carga memoria con el archivo fat
		for (uint32_t i = 0; i < cant_bloques_fat; ++i) {
		    fread((bloques[cant_bloques_swap+i]), sizeof(uint32_t), 1, archivoFAT);
		}
	}
}

bool existencia_archivo(char* archivo){
	  return dictionary_has_key(tablaFCB,archivo);
}

uint32_t abrir_archivo(char* archivo){
	if(_existencia_archivo(archivo)){
	t_config* configArchivofcb = dictionary_get(tablaFCB,archivo);
	uint32_t tamaño =(uint32_t) config_get_int_value(configArchivofcb,"TAMAÑO_ARCHIVO");
	return tamaño;
	}
	return -1;
}


bool crear_archivo(char* nombreArchivo){
	//hace la conversion de la direccion
	char * path=malloc(sizeof(char*)*100);//cambiar esto por el tamaño real con strlen
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
	dictionary_add(tablaFCB,nombreArchivo,nuevoArchivo);
	return true;
}

void asignar_bloques(char* dueño, uint32_t cantidad){
	for(int i=cant_bloques_swap; i<cant_bloques_total && cantidad>0;i++){
		if((bloques[i]->dueño)==NULL){
			bloques[i]->dueño=dueño;
			cantidad--;
		}
	}
}
void liberar_bloques(char* dueño, uint32_t cantidad){
	for(int i=cant_bloques_total; i>cant_bloques_swap && cantidad>0;i--){
			if((bloques[i]->dueño)==NULL){
				bloques[i]->dueño=dueño;
				cantidad--;
			}
		}
}

void truncar_archivo(char*nombreArchivo, uint32_t tamaño){ //situacionDeseada : ampliar o reducir tamanio
	t_config* archivo = dictionary_get(tablaFCB,nombreArchivo);
	uint32_t tamañoActual =(uint32_t) config_get_int_value(archivo,"TAMAÑO_ARCHIVO");
	config_set_value(archivo, "TAMANIO_ARCHIVO", tamaño);
	if(tamaño > tamañoActual){
		uint32_t cantidad=tamaño-tamañoActual;
		asignar_bloques(nombreArchivo,cantidad);
	}
	else if(tamaño < tamañoActual){
		uint32_t cantidad=tamañoActual-tamaño;
		liberar_bloques(nombreArchivo,cantidad);
	}
}

void leer_archivo(char*nombreArchivo, uint32_t puntero){
	//comunicacion con memoria
}

void escribir_archivo(char*nombreArchivo, uint32_t puntero){
	//comunicacion con memoria
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
		int tamaño =abrir_archivo(((char)list_get(mensaje,1)));
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
