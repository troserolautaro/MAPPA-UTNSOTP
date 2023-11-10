#include"Consola.h"

char* lectura_consola(){
	//Semaforo de espera mensaje
	char* linea;
//	pthread_mutex_lock(&mutexLog);
	linea = readline("\n >>");
	if (linea){ add_history(linea);}
//	pthread_mutex_unlock(&mutexLog);
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
		pthread_mutex_lock(&mutexLog);
		log_info(logger,"Inicio de Planificacion");
		pthread_mutex_unlock(&mutexLog);
	}
}
void detener_planificacion(){
	detenida=true;
	pthread_mutex_lock(&mutexLog);
	log_info(logger,"Pausa de Planificacion");
	pthread_mutex_unlock(&mutexLog);
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

		//Talvez estos mutex no son necesarios
		pthread_mutex_lock(&mutexProcesos);
		list_add(procesos,proceso);
		pthread_mutex_unlock(&mutexProcesos);
		t_paquete * paqueteArchivo=crear_paquete();
		uint32_t pid=proceso->pid;
		agregar_a_paquete(paqueteArchivo, "cargar", sizeof("cargar"));
		agregar_a_paquete(paqueteArchivo, &pid, sizeof(uint32_t));
		agregar_a_paquete(paqueteArchivo, path, (strlen(path))+1);
		agregar_a_paquete(paqueteArchivo, &size, sizeof(int));
		enviar_paquete(paqueteArchivo,conexionMemoria);
		eliminar_paquete(paqueteArchivo);

}


void finalizar_proceso(int pid){
	//POSIBLE MUTEX
	if(pid<=(list_size(procesos))){
		//porque asigna pid= -1 ?
		pid-=1;
		PCB* proceso= list_get(procesos,pid);
		if(proceso->estado!=TERMINATED){
			planificador_largo_salida(proceso);
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
		char* new=string_from_format("Estado: NEW - Procesos: "),
		*ready=string_from_format("Estado: READY - Procesos: "),
		*exec=string_from_format("Estado: EXEC - Procesos: "),
		*blocked=string_from_format("Estado: BLOCKED - Procesos: "),
		*terminated=string_from_format("Estado: TERMINATED - Procesos: ");
		for(i = 0 ; i<list_size(procesos); i++){
			proceso= list_get(procesos,i);
			switch(proceso->estado){
			case NEW:string_append_with_format(&new,"PID_%s \n",(char*)proceso->pid); break;
			case READY: string_append_with_format(&ready,"PID_%s \n",(char*)proceso->pid); break;
			case BLOCKED: string_append_with_format(&blocked,"PID_%s \n",(char*)proceso->pid); break;
			case EXEC: string_append_with_format(&exec,"PID_%s \n",(char*)proceso->pid); break;
			case TERMINATED:string_append_with_format(&terminated,"PID_%s \n",(char*)proceso->pid); break;
			}
		}
	pthread_mutex_unlock(&mutexProcesos);
	char * estados = string_new();
	string_append(&estados,new);
	string_append(&estados,ready);
	string_append(&estados,exec);
	string_append(&estados,blocked);
	string_append(&estados,terminated);
	pthread_mutex_lock(&mutexLog);
	log_info(logger,"%s",estados);
	pthread_mutex_unlock(&mutexLog);

	free(new);
	free(ready);
	free(exec);
	free(blocked);
	free(terminated);
	free(estados);
	free(proceso);

	}
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
				//	finalizar_proceso(atoi(parametros[0]));
//				enviar_mensaje("FINALIZAR PROCESO",conexionCPUDispatch);
			break;
			case INICIAR_PLANIFICACION:
					iniciar_planificacion();
					//enviar_mensaje("INICIAR PLANIFICACION",conexionCPUDispatch);
			break;
			case DETENER_PLANIFICACION:
					detener_planificacion();
					//enviar_mensaje("DETENER PLANIFICACION",conexionCPUDispatch);
			break;
			case MULTIPROGRAMACION:
					//si hay que hacer algo mas, sacar en una funcion aparte
				//	gradoMultiprogramacion=atoi(parametros[0]);
					//enviar_mensaje("MULTIPROGRAMACION",conexionCPUDispatch);
			break;

			case PROCESO_ESTADO:
					proceso_estado();
//				enviar_mensaje("PROCESO ESTADO",conexionCPUDispatch);
			break;
			case -1:
				enviar_mensaje("Me desconecte",conexionCPUDispatch);
				return NULL ;
			break;
			default:
			break;
		}
		for(size_t i = 0; parametros[i]!=NULL; i++ ){
			free(parametros[i]);
		}

	}
}
