#include "FileSystem.h"

int main(void) {

	t_log *logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	t_config* config;
	char* ipMemoria;
	char* puertoEscucha,* puertoMemoria;
	char* path_fat, * path_bloques, * path_fcb;
	char* cant_bloques_total,* cant_bloques_swap;
	char* tam_bloque;
	char* retardo_acceso_bloque,* retardo_acceso_fat;

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

	int server_fd = iniciar_servidor(puertoEscucha);
	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(server_fd);
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
//					list_iterate(lista, (void*) iterator);
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


void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);

	if(!strcasecmp(msg,"conexion")){
		log_info(logger,"Hola! %d",*(int*)list_get(mensaje,1));
	}

}

//Comunicacion con kernel


bool chequear_existencia_archivo(FCB* archivo){
	return true;
}
int abrir_archivo(FCB* archivo){
	if(chequear_existencia_archivo(archivo)){
		return archivo->tamanioArchivo;
	}
	else{
		printf("El archivo no existe");
		return -1;
	}
}

void crear_archivo(char* nombreArchivo){
	FCB* archivo;

	archivo->nombreArchivo = nombreArchivo;
	archivo->tamanioArchivo = 0;
	archivo->bloqueInicial = 0;
}

void truncar_archivo(char* situacionDeseada, FCB* archivo, int tamanio){ //situacionDeseada : ampliar o reducir tamanio
	if(strcmp(situacionDeseada,"ampliarTamanio")){
		archivo->tamanioArchivo += tamanio;
		//no entendi lo de los bloques
	}
	else if(strcmp(situacionDeseada,"reducirTamanio")){
		archivo->tamanioArchivo -= tamanio;
		//no entendi lo de los bloques
	}
	else
		printf("Ingresar una situacion correcta");

}

void leer_archivo(){}

void escribir_archivo(){}

//-----------------------

//Comunicacion con Memoria

void iniciar_proceso(){}

void finalizar_proceso(){}

//------------------------

