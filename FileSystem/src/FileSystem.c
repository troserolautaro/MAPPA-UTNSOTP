#include "FileSystem.h"

#define valor_EOF UINT32_MAX  //Este valor representa al EOF (End of File)

//t_dictionary* tablaFCB;
//void* bloques Noes necesario ya se reserva el swap, mirar el codigo ya hecho!
void* bufferMemoria;
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
sem_t validRead_s;
sem_t datosMemoria_s;


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
	//tablaFCB = dictionary_create();
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
	sem_init(&validRead_s,0,0);
	sem_init(&datosMemoria_s,0,0);
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
			uint32_t libre = 0;
			for(int i = 0; i< cantBloquesFAT;i++){ //Recordar que la posicion 0 de la FAT es la cantBloquesSwap*tamañobloque
				fseek(archivoFAT,i*sizeof(uint32_t),SEEK_SET);
				fwrite(&libre,sizeof(uint32_t),1,archivoFAT);
			}
		}
	}
	fclose(archivoFAT);

}
char* devolver_path(char* archivo){
	char* path=string_new();
	string_append(&path,pathFCB);
	string_append(&path, "/");
	string_append(&path,archivo);
	string_append(&path,".fcb");
	return path;
}

bool existencia_archivo(char* archivo){
	FILE * fcb = fopen(archivo,"r");
	if (fcb != NULL)return true;
	return false;
}

uint32_t abrir_archivo(char* archivo){
	if(existencia_archivo(archivo)){
	t_config* configArchivofcb = config_create(archivo);
	uint32_t tamaño =(uint32_t) config_get_int_value(configArchivofcb,"TAMANIO_ARCHIVO");
	return tamaño;
	}
	return -1;
}


bool crear_archivo(char* nombreArchivo){
	//abre el archivo en forma w para crearlo y lo cierra
	char* path = devolver_path(nombreArchivo);
	FILE * nuevofcb = fopen(path,"w");
	fclose(nuevofcb);
	//lo abre como config para cargarle los datos principales
	t_config * nuevoArchivo= iniciar_config(path);
	config_set_value(nuevoArchivo, "NOMBRE_ARCHIVO", nombreArchivo);
	config_set_value(nuevoArchivo, "TAMANIO_ARCHIVO", string_itoa(0));
	config_save(nuevoArchivo);
	config_destroy(nuevoArchivo);
	free(path);
	return true;
}

//si devuelve null no hay anteultimo elemento
uint32_t* get_anteultimo_bloque_archivo(uint32_t bloqueInicial){
	FILE *archivoFAT = fopen(pathFAT, "rb");
	uint32_t anteUltimoBloque=bloqueInicial;
	uint32_t ultimoBloque;
	fseek(archivoFAT,anteUltimoBloque*sizeof(uint32_t),SEEK_SET);
	fread(&ultimoBloque,sizeof(uint32_t),1,archivoFAT);
	bool esAnteultimoBLoque=(ultimoBloque == UINT32_MAX );
	if(esAnteultimoBLoque)return NULL;
    while (!esAnteultimoBLoque ) {
    	anteUltimoBloque=ultimoBloque;
    	fseek(archivoFAT,anteUltimoBloque*sizeof(uint32_t),SEEK_SET);
		fread(&ultimoBloque,sizeof(uint32_t),1,archivoFAT);
		bool esAnteultimoBLoque=(ultimoBloque == UINT32_MAX );
    }
	return anteUltimoBloque;
}

uint32_t get_ultimo_bloque_archivo(uint32_t bloqueInicial){
	FILE *archivoFAT = fopen(pathFAT, "rb");
	fseek(archivoFAT,bloqueInicial*sizeof(uint32_t),SEEK_SET);
	uint32_t ultimoBloque;
	fread(&ultimoBloque,sizeof(uint32_t),1,archivoFAT);
    while (ultimoBloque != UINT32_MAX) {
    	fseek(archivoFAT,ultimoBloque*sizeof(uint32_t),SEEK_SET);
    	fread(&ultimoBloque,sizeof(uint32_t),1,archivoFAT);
    }
	return ultimoBloque;
}

void asignar_bloques_FAT(t_config* archivo, uint32_t cantidad){
	bool tieneBloqueInicial=config_has_property(archivo,"BLOQUE_INICIAL");
	uint32_t ultimoBloque,bloqueActual, bloqueInicial, finArch = valor_EOF;
	if(tieneBloqueInicial){
		bloqueInicial=(uint32_t)config_get_int_value(archivo,"BLOQUE_INICIAL");
		ultimoBloque =get_ultimo_bloque_archivo(bloqueInicial);
	}
	FILE *archivoFAT = fopen(pathFAT, "rb+");
	for(uint32_t i=1; i<cantBloquesFAT && cantidad>0;i++){
		fseek(archivoFAT,(i*sizeof(uint32_t)),SEEK_SET);
    	fread(&bloqueActual,sizeof(uint32_t),1,archivoFAT);
		if(bloqueActual==0){
			if(!tieneBloqueInicial){
				config_set_value(archivo,"BLOQUE_INICIAL",string_itoa(i));
				fseek(archivoFAT,(i*sizeof(uint32_t)),SEEK_SET);
				fwrite(&finArch,sizeof(uint32_t),1,archivoFAT);
				tieneBloqueInicial = true;
			}else{
				fseek(archivoFAT,(i*sizeof(uint32_t)),SEEK_SET);
				fwrite(&finArch,sizeof(uint32_t),1,archivoFAT);
				fseek(archivoFAT, ultimoBloque*sizeof(uint32_t),SEEK_SET);
				fwrite(&i,sizeof(uint32_t),1,archivoFAT);
			}
			ultimoBloque = i;
			cantidad--;
		}


	}
	fclose(archivoFAT);

}

//la idea seria que vaya desenlazando los bloques y removiendolos en la lista del bloque inicial
//por ahi abria que cambiar la estructura de dicha lista para saber el bloque y el bloque al que apunta
//y asignar la constante al ultimo bloque cuando apunta al bloque siguiete
void liberar_bloques_FAT(t_config* archivo, uint32_t cantidad){
	bool tieneBloqueInicial=config_has_property(archivo,"BLOQUE_INICIAL");
	bool tieneAnteUltimoBloque=false;
	bool error=false;
	uint32_t numBloqueInicial,ultimoBloque, anteultimoBloque,finArch,libre;
	finArch=UINT32_MAX;
	libre=0;
	FILE *archivoFAT = fopen(pathFAT, "rb+");
	if(tieneBloqueInicial){
		numBloqueInicial=(uint32_t)config_get_int_value(archivo,"BLOQUE_INICIAL");
		ultimoBloque =get_ultimo_bloque_archivo(numBloqueInicial);
		anteultimoBloque =get_anteultimo_bloque_archivo(numBloqueInicial);
	}
	while(cantidad>0 || !error){
		if(!tieneBloqueInicial ){
			error=true;//si no tengo bloque inicial errror
		}
		else{
			anteultimoBloque =get_anteultimo_bloque_archivo(numBloqueInicial);
			ultimoBloque =get_ultimo_bloque_archivo(numBloqueInicial);
			fseek(archivoFAT,ultimoBloque*sizeof(uint32_t),SEEK_SET);
			fwrite(&libre,sizeof(uint32_t),1,archivoFAT);
			fseek(archivoFAT,anteultimoBloque*sizeof(uint32_t),SEEK_SET);
			fwrite(&finArch,sizeof(uint32_t),1,archivoFAT);
		}
		cantidad--;
	}
}

void truncar_archivo(char*path, uint32_t tamaño){ //situacionDeseada : ampliar o reducir tamanio
	t_config* archivo =config_create(path);
	uint32_t tamañoActual =(uint32_t) config_get_int_value(archivo,"TAMANIO_ARCHIVO");
	if(tamaño*tamBloque > tamañoActual){
		uint32_t cantidad=tamaño-tamañoActual;
		asignar_bloques_FAT(archivo,cantidad);
	}
	else if(tamaño*tamBloque  < tamañoActual){
		uint32_t cantidad=tamañoActual-tamaño;
		liberar_bloques_FAT(archivo,cantidad);
	}
	config_set_value(archivo,"TAMANIO_ARCHIVO",string_itoa(tamaño*tamBloque));
	config_save(archivo);
	config_destroy(archivo);
}
uint32_t obtener_bloque(t_config *config, uint32_t puntero){
	uint32_t bloque =(uint32_t) config_get_int_value(config,"BLOQUE_INICIAL");
	FILE *archivoFAT = fopen(pathFAT, "rb+");
	while(puntero>0){
		fseek(archivoFAT,bloque*sizeof(uint32_t),SEEK_SET);
		fread(&bloque,sizeof(uint32_t),1,archivoFAT);
		puntero--;
	}
	return bloque;
}
void leer_archivo(char*path,uint32_t direccionFisica, uint32_t puntero,int conexion){
	//comunicacion con memoria
	uint32_t bloque;
	t_config *configArchivo=config_create(path);
	t_paquete * paquete = crear_paquete();
	void* valorBloque=(void *)malloc(tamBloque);
	uint32_t punteroFisico;
	int tamaño=config_get_int_value(configArchivo,"TAMANIO_ARCHIVO");
	if( tamaño/tamBloque>=puntero){
		bloque=obtener_bloque(configArchivo,puntero);
		debug(string_from_format("bloque %d",bloque));
		uint32_t punteroFisico=(cantBloquesSWAP+bloque)*tamBloque;
		FILE *archivoBloques = fopen(pathBloques, "rb");
		fseek(archivoBloques,punteroFisico,SEEK_SET);
		fread(valorBloque,tamBloque,1,archivoBloques);
		fclose(archivoBloques);
		//debug(string_from_format("bloque %d",valorBloque));
		agregar_a_paquete(paquete,"f_read",sizeof("f_read"));
		agregar_a_paquete(paquete,&direccionFisica,sizeof(uint32_t));
		agregar_a_paquete(paquete,valorBloque,tamBloque);
		enviar_paquete(paquete,conexionMemoria);
		eliminar_paquete(paquete);
		sem_wait(&validRead_s);
		paquete = crear_paquete();
		agregar_a_paquete(paquete,"valid_read",sizeof("valid_read"));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	else{
		agregar_a_paquete(paquete,"invalid_read",sizeof("invalid_read"));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	config_destroy(configArchivo);


}

void solicitar_datos_memoria(char*path,uint32_t direccionFisica, uint32_t puntero){
	t_config *configArchivo=config_create(path);
	int tamaño=config_get_int_value(configArchivo,"TAMANIO_ARCHIVO");
	if( tamaño>=puntero){
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"f_write",sizeof("f_write"));
		agregar_a_paquete(paquete,&direccionFisica,sizeof(uint32_t));
		enviar_paquete(paquete,conexionMemoria);
		eliminar_paquete(paquete);
	}
}
void escribir_archivo(char*path, uint32_t puntero,int conexion){
	t_config *configArchivo=config_create(path);
	uint32_t bloque=obtener_bloque(configArchivo,puntero);
	uint32_t punteroFisico=cantBloquesSWAP+bloque;
	FILE *archivoBloques = fopen(pathBloques, "rb+");
	fseek(archivoBloques,punteroFisico,SEEK_SET);
	fwrite(bufferMemoria,tamBloque,1,archivoBloques);
	debug("envio paquete");
	t_paquete * paquete = crear_paquete();
	agregar_a_paquete(paquete,"valid_write",sizeof("valid_write"));
	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	config_destroy(configArchivo);
}
/*-----------------------*/

//Comunicacion con Memoria

void iniciar_proceso(){
	//RESERVAR BLOQUES DE SWAP

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
void* obtener_pagina_swap(uint32_t posSWAP){
	FILE* archivoBloques = fopen(pathBloques,"rb");
	escritura_log(string_from_format("Acceso SWAP: %d", (posSWAP/tamBloque)));
	void * datos = malloc(tamBloque);
	fseek(archivoBloques, posSWAP,SEEK_SET);
	fread(datos,tamBloque,1,archivoBloques);
	fclose(archivoBloques);
	return datos;
}
void escribir_pagina_swap(uint32_t posSWAP, void* datos){
	FILE* archivoBloques = fopen(pathBloques,"rb+");
	escritura_log(string_from_format("Acceso SWAP: %d", (posSWAP/tamBloque)));
	fseek(archivoBloques,posSWAP,SEEK_SET);
	fwrite(datos,tamBloque,1,archivoBloques);
	fclose(archivoBloques);
}
void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
	int conexion = *(int*) (list_get(mensaje,list_size(mensaje)-1));
	//peticiones del kernel
	if(!strcasecmp(msg,"f_open")){
		char* archivo = (char*)list_get(mensaje,1);
		char* path = devolver_path(archivo);
		escritura_log(string_from_format("Abrir Archivo: %s",archivo));
		uint32_t tamaño = abrir_archivo(path);
		if(tamaño==-1){
			escritura_log(string_from_format("Crear Archivo: %s",archivo));
			if(crear_archivo(archivo)){
				tamaño = abrir_archivo(path);
			}
		}
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"tamaño",sizeof("tamaño"));
		agregar_a_paquete(paquete,archivo,strlen(archivo)+1);
		agregar_a_paquete(paquete,&tamaño,sizeof(uint32_t));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
		//free(path);
		//free(archivo);
	}
	if(!strcasecmp(msg,"f_truncate")){
		char* archivo = (char*)list_get(mensaje,1);
		char* path=devolver_path(archivo);
		uint32_t tamaño = *(uint32_t*)list_get(mensaje,2);
		escritura_log(string_from_format("Truncar Archivo: %s - Tamaño %d",archivo,tamaño));
		truncar_archivo(path,tamaño/tamBloque);
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"f_truncate",sizeof("f_truncate"));
		agregar_a_paquete(paquete,(char*)list_get(mensaje,1),strlen((char*)list_get(mensaje,1))+1);
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	if(!strcasecmp(msg,"f_read")){
		char* path=devolver_path((char*)list_get(mensaje,1));
		leer_archivo(path,*(uint32_t*)list_get(mensaje,2),*(uint32_t*)list_get(mensaje,3),conexion);
	}
	if(!strcasecmp(msg,"valid_read")){
		sem_post(&validRead_s);
	}
	if(!strcasecmp(msg,"f_write")){
		debug(string_from_format(" Archivo: %s",(char*)list_get(mensaje,1)));
		char* path=devolver_path((char*)list_get(mensaje,1));
		solicitar_datos_memoria(path,*(uint32_t*)list_get(mensaje,2), *(uint32_t*)list_get(mensaje,2));
		sem_wait(&datosMemoria_s);
		escribir_archivo(path,*(uint32_t*)list_get(mensaje,3),conexion);
	}
	if(!strcasecmp(msg,"datos_memoria")){
		bufferMemoria=(void*)list_get(mensaje,1);
		sem_post(&datosMemoria_s);
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
		void * datos = obtener_pagina_swap(posSwap);
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete,"paginaSWAP",sizeof("paginaSWAP"));
		agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
		agregar_a_paquete(paquete,&pagina,sizeof(uint32_t));
		agregar_a_paquete(paquete,datos,tamBloque);
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
		free(datos);
	}
	if(!strcasecmp(msg,"escribirSwap")){
		escribir_pagina_swap(*(uint32_t*)list_get(mensaje,1), list_get(mensaje,2));
		t_paquete* paquete = crear_paquete();
		agregar_a_paquete(paquete,"escribirSwap",sizeof("escribirSwap"));
		enviar_paquete(paquete,conexion);
		eliminar_paquete(paquete);
	}
	free(msg);
}
