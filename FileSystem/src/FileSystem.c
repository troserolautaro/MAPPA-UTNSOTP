#include "FileSystem.h"

#define valor_EOF UINT32_MAX  //Este valor representa al EOF (End of File)

t_dictionary* tablaFCB;
void* bloques;//como espacio contiguo de memoria
uint32_t* tablaFAT;
bool * tablaSWAP;//como espacio contiguo de memoria
pthread_t hiloRecibirCliente;
int serverFilesystem;
char* ipMemoria;
char* puertoEscucha,*puertoMemoria;
char* pathFAT, * pathBloques, * pathFCB;
int cantBloques,cantBloquesSWAP, cantBloquesFAT;
int tamBloque;
int retardoAccesoBloque,retardoAccesoFAT;
int tamañoFAT;  //Tamaño de los bloques FAT (lo defino en el main)
pthread_mutex_t mutexLog;
int conexionMemoria;

void conectar_a_memoria(){
	t_paquete* paquete = crear_paquete();
	agregar_a_paquete(paquete,"FS",sizeof("FS"));
	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);
}
void iniciar_swap(){
	for(int  i = 0; i<cantBloquesSWAP;i++){
		tablaSWAP[i] = false;
	}

}
int main(void) {
	tablaFCB = dictionary_create();
	logger = iniciar_logger("./log.log");
	t_config* config=iniciar_config("./FileSystem.config");

	//CONFIGURACION DE FILESYSTEM
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria = config_get_string_value(config,"PUERTO_MEMORIA");
	puertoEscucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	pathFAT = config_get_string_value(config,"PATH_FAT");
	pathBloques = config_get_string_value(config,"PATH_BLOQUES");
	pathFCB = config_get_string_value(config,"PATH_FCB");
	cantBloques = config_get_int_value(config,"CANT_BLOQUES_TOTAL");

	cantBloquesSWAP = config_get_int_value(config,"CANT_BLOQUES_SWAP");
	tablaSWAP = malloc(cantBloquesSWAP * sizeof(bool));

	tamBloque = config_get_int_value(config,"TAM_BLOQUE");
	retardoAccesoBloque = config_get_int_value(config,"RETARDO_ACCESO_BLOQUE");
	retardoAccesoFAT = config_get_int_value(config,"RETARDO_ACCESO_FAT");
	cantBloquesFAT=(cantBloques - cantBloquesSWAP);
	//tamañoFAT =  cantBloquesFAT* sizeof(uint32_t);
	conexionMemoria=crear_conexion(ipMemoria, puertoMemoria,FILESYSTEM);
	pthread_t hiloMemoria;
	pthread_create(&hiloMemoria,NULL,manejar_cliente,&conexionMemoria);
	serverFilesystem = iniciar_servidor(puertoEscucha);
	debug("Servidor listo para recibir al cliente");

	int resultado;
	if ((resultado=pthread_create(&hiloRecibirCliente,NULL,(void *)recibir_conexiones,( void *) &serverFilesystem))!=0)
		printf("Error al crear hilo. resultado %d",resultado);

	//abrir los archivos o crearlos si no existe
	conectar_a_memoria();
	iniciar_bloques();
	iniciar_fat();
	iniciar_swap();
	pthread_join(hiloRecibirCliente,NULL);
	return EXIT_SUCCESS;
}


void iniciar_bloques(){
	FILE *archivoBloques = fopen(pathBloques, "rb+");
	if (archivoBloques == NULL) {
		archivoBloques = fopen(pathBloques, "wb+");
		if (archivoBloques == NULL) {
			log_error(logger,"Error al abrir archivo");
		}
	}
	fclose(archivoBloques);
	//fread(bloques, (cantBloques*tamBloque), 1, archivoBloques);
}

void iniciar_fat(){
	FILE *archivoFAT = fopen(pathFAT, "rb+");
	if (archivoFAT == NULL) {
		archivoFAT = fopen(pathFAT, "wb+");
		if (archivoFAT == NULL) {
			log_error(logger,"Error al abrir archivo");
			//return 1;
		}else{
			int valor = 0;
			for(int i = 0; i< cantBloquesFAT;i++){ //Recordar que la posicion 0 de la FAT es la cantBloquesSwap*tamañobloque
				fwrite(&valor,sizeof(uint32_t),1,archivoFAT);
			}
		}
	}
	fclose(archivoFAT);

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
	char * path=string_new();
	string_append(&path,pathFCB);
	string_append(&path, "/");
	string_append(&path,nombreArchivo);
	//abre el archivo en forma w para crearlo y lo cierra
	FILE * nuevofcb = fopen(strcat(path,".fcb"),"w");
	fclose(nuevofcb);
	//lo abre como config para cargarle los datos principales
	t_config * nuevoArchivo= iniciar_config(path);
	config_set_value(nuevoArchivo, "NOMBRE_ARCHIVO", nombreArchivo);
	config_set_value(nuevoArchivo, "TAMANIO_ARCHIVO", string_itoa(0));
	//dictionary_put(tablaFCB,nombreArchivo,nuevoArchivo);
	config_destroy(nuevoArchivo);
	return true;
}

//si devuelve null no hay anteultimo elemento
registroFAT_t* get_anteultimo_bloque_archivo(uint32_t bloqueInicial){
	registroFAT_t* anteUltimoBloque=tablaFAT[bloqueInicial];
	bool esAnteultimoBLoque=(anteUltimoBloque->proximoBloque == UINT32_MAX );
	if(esAnteultimoBLoque)return NULL;
    while (!esAnteultimoBLoque ) {
    	if(anteUltimoBloque->proximoBloque != UINT32_MAX){
    		anteUltimoBloque=tablaFAT[(anteUltimoBloque->proximoBloque)];
    	}else{
    		esAnteultimoBLoque=true;
    	}
    }
	return anteUltimoBloque;
}
registroFAT_t* get_ultimo_bloque_archivo(uint32_t bloqueInicial){
	registroFAT_t* ultimoBloque=tablaFAT[bloqueInicial];
    while (ultimoBloque->proximoBloque != UINT32_MAX) {
    	ultimoBloque=tablaFAT[(ultimoBloque->proximoBloque)];
    }
	return ultimoBloque;
}

void asignar_bloques_FAT(t_config* archivo, uint32_t cantidad){
	bool tieneBloqueInicial=config_has_property(archivo,"BLOQUE_INICIAL");
	int numBloqueInicial;
	registroFAT_t* ultimoBloque=malloc(sizeof(registroFAT_t));
	if(tieneBloqueInicial){
		numBloqueInicial=config_get_int_value(archivo,"BLOQUE_INICIAL");
		ultimoBloque =get_ultimo_bloque_archivo(numBloqueInicial);
	}
	for(int i=0; i<cantBloquesFAT && cantidad>0;i++){
		if(tablaFAT[i]==0){
			if(!tieneBloqueInicial ){
				config_set_value(archivo,"BLOQUE_INICIAL",string_itoa(i));
				numBloqueInicial=i;
				tieneBloqueInicial=true;
				ultimoBloque =tablaFAT[i];
			}
			else if(cantidad>1){
				(ultimoBloque->proximoBloque)=i;
			}
			else{//cantidad==1 asigno fin del archivo
				tablaFAT[i]=UINT32_MAX;
			}
			cantidad--;
		}
	}
}

//la idea seria que vaya desenlazando los bloques y removiendolos en la lista del bloque inicial
//por ahi abria que cambiar la estructura de dicha lista para saber el bloque y el bloque al que apunta
//y asignar la constante al ultimo bloque cuando apunta al bloque siguiete
void liberar_bloques_FAT(t_config* archivo, uint32_t cantidad){
	bool tieneBloqueInicial=config_has_property(archivo,"BLOQUE_INICIAL");
	bool tieneAnteUltimoBloque=false;
	bool error=false;
	int numBloqueInicial;
	registroFAT_t* ultimoBloque=malloc(sizeof(registroFAT_t));
	registroFAT_t* anteultimoBloque=malloc(sizeof(registroFAT_t));
	if(tieneBloqueInicial){
		numBloqueInicial=config_get_int_value(archivo,"BLOQUE_INICIAL");
		ultimoBloque =get_ultimo_bloque_archivo(numBloqueInicial);
		anteultimoBloque =get_anteultimo_bloque_archivo(numBloqueInicial);
		if(anteultimoBloque==NULL){
			tieneAnteUltimoBloque=false;
		}
	}
	while(cantidad>0 || !error){
		if(!tieneBloqueInicial ){
			error=true;//si no tengo bloque inicial errror
		}
		else{
			anteultimoBloque =get_anteultimo_bloque_archivo(numBloqueInicial);
			(ultimoBloque->proximoBloque)=0;//LIBERO EL ULTIMO BLOQUE
			(anteultimoBloque->proximoBloque)=UINT32_MAX;// AL ANTE ULTIMO BLOQUE LI DIGO QUE ES EL ULTIMO
		}
		cantidad--;
	}
}

void truncar_archivo(char*nombreArchivo, uint32_t tamaño){ //situacionDeseada : ampliar o reducir tamanio
	t_config* archivo = dictionary_get(tablaFCB,nombreArchivo);
	uint32_t tamañoActual =(uint32_t) config_get_int_value(archivo,"TAMAÑO_ARCHIVO");
	config_set_value(archivo, "TAMANIO_ARCHIVO", (void*)&tamaño);
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
void reservar_SWAP(uint32_t pid, uint32_t cantBloques){
	FILE* archivoBloques = fopen(pathBloques,"rb+");
	int i = 0;
	t_paquete * paquete = crear_paquete();
	agregar_a_paquete(paquete,"bloquesSwap",sizeof("bloquesSwap"));
	agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
	agregar_a_paquete(paquete,&cantBloques,sizeof(uint32_t));
	while(i<cantBloquesSWAP && cantBloques>0){
		if(!tablaSWAP[i]){
			uint32_t posSWAP = i*tamBloque;
			agregar_a_paquete(paquete,&posSWAP,sizeof(uint32_t));
			cantBloques--;
			fseek(archivoBloques,i*tamBloque,SEEK_SET);
			fwrite(&("\0"),tamBloque,1,archivoBloques);
			tablaSWAP[i] = true;
		}
		i++;
	}
	fclose(archivoBloques);
	if (cantBloques>0){
		error_show("Cantidad maxima de SWAP ocupada");
		agregar_a_paquete(paquete,"error",sizeof("error"));
	}
	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);

}
void* obtener_pagina_swap(uint32_t pid, uint32_t posSWAP){
	FILE* archivoBloques = fopen(pathBloques,"rb");
	escritura_log(string_from_format("Acceso SWAP: %d", (posSWAP/tamBloque)));
	void * datos = malloc(tamBloque);
	fseek(archivoBloques, posSWAP,SEEK_SET);
	fread(datos,tamBloque,1,archivoBloques);
	fclose(archivoBloques);
	return datos;
}
void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
	int conexion = *(int*) (list_get(mensaje,list_size(mensaje)-1));
	//peticiones del kernel
	if(!strcasecmp(msg,"abrir archivo")){
		int tamaño =abrir_archivo((char*)list_get(mensaje,1));
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"tamaño",sizeof("tamaño"));
		agregar_a_paquete(paquete,&tamaño,sizeof(int));
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
	if(!strcasecmp(msg,"reservarSWAP")){
		reservar_SWAP(*(uint32_t*)list_get(mensaje,1),*(uint32_t*)list_get(mensaje,2));
	}
	if(!strcasecmp(msg,"paginaSWAP")){
		uint32_t pid = *(uint32_t*) list_get(mensaje,1);
		uint32_t posSwap = *(uint32_t*) list_get(mensaje,2);
		uint32_t pagina = *(uint32_t*) list_get(mensaje,3);
		void * datos = obtener_pagina_swap(pid,posSwap);
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"paginaSWAP",sizeof("paginaSWAP"));
		agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
		agregar_a_paquete(paquete,&pagina,sizeof(uint32_t));
		agregar_a_paquete(paquete,datos,tamBloque);
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
		free(datos);
	}

}
