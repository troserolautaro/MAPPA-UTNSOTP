#include"Consola.h"

char* lectura_consola(){
	char* linea;
	linea = readline("\n >>");
	if (linea) add_history(linea);
	return linea;
}

int validacion_contenido_consola(char* comando){
	if(!strcasecmp(comando, "iniciar_proceso\0"))return INICIAR_PROCESO;
	if(!strcasecmp(comando, "finalizar_proceso\0"))return FINALIZAR_PROCESO;
	if(!strcasecmp(comando, "iniciar_planificacion\0"))return INICIAR_PLANIFICACION;
	if(!strcasecmp(comando, "detener_planificacion\0"))return DETENER_PLANIFICACION;
	if(!strcasecmp(comando, "multiprogramacion\0"))return MULTIPROGRAMACION;
	if(!strcasecmp(comando, "proceso_estado\0"))return PROCESO_ESTADO;
	if(!strcasecmp(comando,"Exit\0"))return EXIT;
	return -2;
}

void iniciar_planificacion(){
	if(detenida == true){
		detenida=false;
		sem_post(&planiLargo);
		log_info(logger,"Inicio de Planificacion");
	}
}
void detener_planificacion(){
	detenida=true;
	log_info(logger,"Pausa de Planificacion");
}

void iniciar_proceso(char* path, int size, int prioridad){
		//Mutex
		PIDGLOBAL++;
		//crea el proceso
		PCB* proceso = proceso_create();
		proceso->estado = NEW;
		proceso->prioridad = prioridad;
		proceso->registros->AX= 0;
		proceso->registros->BX = 0;
		proceso->registros->CX = 0;
		proceso->registros->DX = 0;
		proceso->pc=0;
		proceso->pid = PIDGLOBAL; //Modificar en caso de que sea necesario
		//añade el proceso a la lista de procesos y a la cola del planificador a largo plazo

		//envia archivo a cargar en memoria para este proceso a el modulo de memoria
		//SE PUEDE EXPORTAR SI ES NECESARIO EL ENVIO DE PAQUETE SI ES NECESARIO
		t_paquete * paqueteArchivo=crear_paquete();
		uint32_t pid=proceso->pid;
		agregar_a_paquete(paqueteArchivo, "cargar", sizeof("cargar"));
		agregar_a_paquete(paqueteArchivo, &pid, sizeof(uint32_t));
		agregar_a_paquete(paqueteArchivo, path, (sizeof(path)));
		agregar_a_paquete(paqueteArchivo, &size, sizeof(int*));
		enviar_paquete(paqueteArchivo,conexionMemoria);
		eliminar_paquete(paqueteArchivo);
		//semaforos procesos mutex
		list_add(procesos,proceso);
}


void finalizar_proceso(int pid){
	//POSIBLE MUTEX
	if(pid<=(list_size(procesos))){
		//porque asigna pid= -1 ?
		pid-=1;
		PCB* proceso= list_get(procesos,pid);
		if(proceso->estado!=TERMINATED){
			planificador_largo_salida(&proceso);
		}else{
			printf("El proceso ya fue finalizado");
		}
	}else{
		printf("Intentando eliminar un proceso que no existe\n");
	}
}

void proceso_estado(){
	//Modificar
	pthread_mutex_lock(&mutexProcesos);
	if(!list_is_empty(procesos)){
		int i;
		PCB* proceso;
		for(i = 0 ; i<list_size(procesos); i++){
			proceso= list_get(procesos,i);
			printf("[PID]: %d  ",proceso->pid);
			printf("[ESTADO]: ");
			estado_enum(proceso->estado);
		}
	pthread_mutex_unlock(&mutexProcesos);
	free(proceso);
	}else{printf("No hay procesos \n");}
}

//pasar conexiones en el paramotro como array o struct
void * manejar_consola( void* args ){
	int idComando;
	while(1){
		char * lectura = string_new();
		string_append(&lectura,lectura_consola());
		string_trim(&lectura);
		char** parametros = string_array_new();
		parametros = string_n_split(lectura,4," ");
		idComando = validacion_contenido_consola(parametros[0]);
		free(lectura);
		/*PROBABLEMENTE HAY QUE MEJORAR ESTO, SI BIEN FUNCIONA NO TOMA LOS PARAMETROS QEU SE INGRESAN EN
		 * CONSOLA */
		switch(idComando){
			case INICIAR_PROCESO:
					if(parametros[1]==NULL){
						break;
					}
					char* path = malloc(sizeof(parametros[1]));
					path = string_duplicate(parametros[1]);
					printf("%s", path);

					if(parametros[2]==NULL){
						free(path);
						break;
					}
					int size = (int) *parametros[2];

					if(parametros[3]==NULL){
						free(path);
						break;
					}
					int prioridad = (int) *parametros[3];
					iniciar_proceso(path,size,prioridad);

			break;
			case FINALIZAR_PROCESO:
					printf("FINALIZAR PROCESO \n");
				//	finalizar_proceso(atoi(parametros[0]));
//				enviar_mensaje("FINALIZAR PROCESO",conexionCPUDispatch);
			break;
			case INICIAR_PLANIFICACION:
					printf("INICIAR PLANIFICACION \n");
					iniciar_planificacion();
					//enviar_mensaje("INICIAR PLANIFICACION",conexionCPUDispatch);
			break;
			case DETENER_PLANIFICACION:
					printf("DETENER PLANIFICACION \n");
					detener_planificacion();
					//enviar_mensaje("DETENER PLANIFICACION",conexionCPUDispatch);
			break;
			case MULTIPROGRAMACION:
					printf("MULTIPROGRAMACION \n");
					//si hay que hacer algo mas, sacar en una funcion aparte
				//	gradoMultiprogramacion=atoi(parametros[0]);
					//enviar_mensaje("MULTIPROGRAMACION",conexionCPUDispatch);
			break;

			case PROCESO_ESTADO:
					printf("PROCESO ESTADO \n");
					proceso_estado();
//				enviar_mensaje("PROCESO ESTADO",conexionCPUDispatch);
			break;
			case -1:
				printf("Saliendo! \n");
				enviar_mensaje("Me desconecte",conexionCPUDispatch);
				return NULL ;
			break;
			default:
				printf("No se reconocio el comando \n");
			break;
		}
		for(size_t i = 0; parametros[i]!=NULL; i++ ){
			free(parametros[i]);
		}

	}
}
