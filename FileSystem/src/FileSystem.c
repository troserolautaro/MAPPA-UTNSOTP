#include "FileSystem.h"

#define valor_EOF UINT32_MAX  //Este valor representa al EOF (End of File)

t_dictionary* tablaFCB;
void* bloques;//como espacio contiguo de memoria
registroFAT_t* tablaFAT;//como espacio contiguo de memoria
pthread_t hiloRecibirCliente;
int serverFilesystem;
char* ipMemoria;
char* puertoEscucha,* puertoMemoria;
char* pathFAT, * pathBloques, * pathFCB;
int cantBloques,cantBloquesSWAP, cantBloquesFAT;
int tamBloque;
int retardoAccesoBloque,retardoAccesoFAT;
int tamañoFAT;  //Tamaño de los bloques FAT (lo defino en el main)
pthread_mutex_t mutexLog;

int main(void) {
	tablaFCB = dictionary_create();
	t_log *logger = iniciar_logger("./log.log");
	t_config* config=iniciar_config("./FileSystem.config");

	//CONFIGURACION DE FILESYSTEM
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria = config_get_string_value(config,"PUERTO_MEMORIA");
	puertoEscucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	pathFAT = config_get_string_value(config,"PATH_FATH");
	pathBloques = config_get_string_value(config,"PATH_BLOQUES");
	pathFCB = config_get_string_value(config,"PATH_FCB");
	cantBloques = config_get_int_value(config,"CANT_BLOQUES_TOTAL");
	cantBloquesSWAP = config_get_int_value(config,"CANT_BLOQUES_SWAP");
	tamBloque = config_get_int_value(config,"TAM_BLOQUE");
	retardoAccesoBloque = config_get_int_value(config,"RETARDO_ACCESO_BLOQUE");
	retardoAccesoFAT = config_get_int_value(config,"RETARDO_ACCESO_FAT");
	cantBloquesFAT=(cantBloques - cantBloquesSWAP);
	//tamañoFAT =  cantBloquesFAT* sizeof(uint32_t);

	serverFilesystem = iniciar_servidor(puertoEscucha);
	escritura_log("Servidor listo para recibir al cliente");
	int resultado;
	if ((resultado=pthread_create(&hiloRecibirCliente,NULL,(void *)recibir_conexiones,( void *) &serverFilesystem))!=0)
		printf("Error al crear hilo. resultado %d",resultado);
		pthread_join(hiloRecibirCliente,NULL);
	//abrir los archivos o crearlos si no existe
	iniciar_bloques();
	iniciar_fat();
	return EXIT_SUCCESS;
}


void iniciar_bloques(){
	bloques=malloc(cantBloques * tamBloque);
	FILE *archivoBloques = fopen(pathBloques, "rb+");
	if (archivoBloques == NULL) {
		archivoBloques = fopen(pathBloques, "wb+");
		if (archivoBloques == NULL) {
			perror("Error al abrir archivo");
		}
	}
	fread(bloques, (cantBloques*tamBloque), 1, archivoBloques);
}

void iniciar_fat(){
	tablaFAT=malloc(cantBloquesFAT* sizeof(uint32_t));
	FILE *archivoFAT = fopen(pathFAT, "rb+");
	if (archivoFAT == NULL) {
		archivoFAT = fopen(pathFAT, "wb+");
		if (archivoFAT == NULL) {
			perror("Error al abrir archivo");
			//return 1;
		}
		for (int i = 0; i < cantBloquesFAT; ++i) {
			(tablaFAT[i]).proximoBloque=0;
		}
	}
	else{
		//carga memoria con el archivo fat
		for (int i = 0; i < cantBloquesFAT; ++i) {
		    fread(&((tablaFAT[i]).proximoBloque), sizeof(uint32_t), 1, archivoFAT);
		}
	}
}

bool existencia_archivo(char* archivo){
	  return dictionary_has_key(tablaFCB,archivo);
}

uint32_t abrir_archivo(char* archivo){
	if(existencia_archivo(archivo)){
	t_config* configArchivofcb = dictionary_get(tablaFCB,archivo);
	uint32_t tamaño =(uint32_t) config_get_int_value(configArchivofcb,"TAMAÑO_ARCHIVO");
	return tamaño;
	}
	return -1;
}


bool crear_archivo(char* nombreArchivo){
	//hace la conversion de la direccion
	char * path=malloc(sizeof(char*)*100);//cambiar esto por el tamaño real con strlen
	strcpy(path, pathFCB);
	string_append(&path, "/");
	string_append(&path,nombreArchivo);
	//abre el archivo en forma w para crearlo y lo cierra
	FILE * nuevofcb = fopen(path,"w");
	fclose(nuevofcb);
	//lo abre como config para cargarle los datos principales
	t_config * nuevoArchivo= iniciar_config(path);
	config_set_value(nuevoArchivo, "NOMBRE_ARCHIVO", nombreArchivo);
	config_set_value(nuevoArchivo, "TAMANIO_ARCHIVO", 0);
	dictionary_put(tablaFCB,nombreArchivo,nuevoArchivo);
	return true;
}

//la idea seria que vaya enlazando los bloques y cargando en la lista del primer bloque la secuencia de bloques
//por ahi abria que cambiar la estructura de dicha lista para saber el bloque y el bloque al que apunta
//y asignar la constante al ultimo bloque
void asignar_bloques_FAT(t_config* archivo, uint32_t cantidad){
	bool tieneBloqueInicial=config_has_property(archivo,"BLOQUE_INICIAL");
	int numBloqueInicial;
	if(tieneBloqueInicial){
		numBloqueInicial=config_get_int_value(archivo,"BLOQUE_INICIAL");
	}
	for(int i=0; i<cantBloquesFAT && cantidad>0;i++){
		if((tablaFAT[i]).proximoBloque==0){
			if(!tieneBloqueInicial ){
				config_set_value(archivo,"BLOQUE_INICIAL",string_itoa(i));
				numBloqueInicial=i;
				tieneBloqueInicial=true;
			}
			list_add(((tablaFAT[numBloqueInicial]).bloques), i);//revisar que no vaya cambiando
			cantidad--;
		}
	}
}

//la idea seria que vaya desenlazando los bloques y removiendolos en la lista del bloque inicial
//por ahi abria que cambiar la estructura de dicha lista para saber el bloque y el bloque al que apunta
//y asignar la constante al ultimo bloque cuando apunta al bloque siguiete
void liberar_bloques_FAT(t_config* archivo, uint32_t cantidad){
	int numBloqueInicial=config_get_int_value(archivo,"BLOQUE_INICIAL");
	registroFAT_t BloqueInicial=tablaFAT[numBloqueInicial];
	int indiceBloque;
	for(int i=list_size(BloqueInicial.bloques) ; i>0 && cantidad>0;i--){
		//bloqueFAT[]->bloqueSiguiente=0;
		cantidad--;
	}
}

void truncar_archivo(char*nombreArchivo, uint32_t tamaño){ //situacionDeseada : ampliar o reducir tamanio
	t_config* archivo = dictionary_get(tablaFCB,nombreArchivo);
	uint32_t tamañoActual =(uint32_t) config_get_int_value(archivo,"TAMAÑO_ARCHIVO");
	config_set_value(archivo, "TAMANIO_ARCHIVO", tamaño);
	if(tamaño > tamañoActual){
		uint32_t cantidad=tamaño-tamañoActual;
		asignar_bloques_FAT(archivo,cantidad);
	}
	else if(tamaño < tamañoActual){
		uint32_t cantidad=tamañoActual-tamaño;
		liberar_bloques_FAT(archivo,cantidad);
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
	for (int i = 0; i < cantBloques; ++i) {
	memcpy((void*)((char*)bloques + i),"\0",sizeof(uint32_t));
	}

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
		//leer_archivo();
	}
	if(!strcasecmp(msg,"escribir archivo")){
		//escribir_archivo();
	}
	//peticiones de memoria
	if(!strcasecmp(msg,"iniciar proceso")){
		iniciar_proceso();
	}
	if(!strcasecmp(msg,"finalizar proceso")){
		finalizar_proceso();
	}
}
