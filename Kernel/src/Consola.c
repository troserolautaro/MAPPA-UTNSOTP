#include"Consola.h"
sem_t sem_readline,procesoCargado;
pthread_mutex_t mutexGlobal;
char* lectura_consola(){
	//Semaforo de espera mensaje
	char* linea;
//	pthread_mutex_lock(&mutexLog);
	sem_wait(&sem_readline);
	linea = readline("\n");
	if (linea){ add_history(linea);}
//	pthread_mutex_unlock(&mutexLog);
	return linea;
}

int validacion_contenido_consola(char* comando){
	if(!strcasecmp(comando, "iniciar_proceso\0") || !strcasecmp(comando, "create\0") )return INICIAR_PROCESO;
	if(!strcasecmp(comando, "finalizar_proceso\0") || !strcasecmp(comando, "kill\0") )return FINALIZAR_PROCESO;
	if(!strcasecmp(comando, "iniciar_planificacion\0") ||!strcasecmp(comando, "start\0") )return INICIAR_PLANIFICACION;
	if(!strcasecmp(comando, "detener_planificacion\0") || !strcasecmp(comando, "stop\0"))return DETENER_PLANIFICACION;
	if(!strcasecmp(comando, "multiprogramacion\0"))return MULTIPROGRAMACION;
	if(!strcasecmp(comando, "proceso_estado\0"))return PROCESO_ESTADO;
	if(!strcasecmp(comando,"Exit\0"))return EXIT;
	return -2;
}

void iniciar_planificacion(){
	pthread_mutex_lock(&mutexDetenida);
	if(detenida){
		escritura_log("Inicio de planificacion");
		detenida=false;
		sem_post(&planiLargo);

	}
	pthread_mutex_unlock(&mutexDetenida);
}
void detener_planificacion(){
	pthread_mutex_lock(&mutexDetenida);
	if(!detenida){
		detenida=true;
		escritura_log("Pausa de planificacion");
	}
	pthread_mutex_unlock(&mutexDetenida);
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
		proceso->pid = PIDGLOBAL;
		proceso->tablaArchivos = dictionary_create();
		//añade el proceso a la lista de procesos y a la cola del planificador a largo plazo

		//envia archivo a cargar en memoria para este proceso a el modulo de memoria
		//SE PUEDE EXPORTAR SI ES NECESARIO EL ENVIO DE PAQUETE SI ES NECESARIO

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
		sem_wait(&procesoCargado);

}

//FALTA LIBERAR TABLA DE ARCHIVOS
void finalizar_proceso(int pid){
	//POSIBLE MUTEX;
	pthread_mutex_lock(&mutexProcesos);
	int size = (list_size(procesos));
	pthread_mutex_unlock(&mutexProcesos);
	if(pid<=size){
		//porque asigna pid= -1 ?
		pid-=1;
		pthread_mutex_lock(&mutexProcesos);
		PCB* proceso= list_get(procesos,pid);
		pthread_mutex_unlock(&mutexProcesos);

		pthread_mutex_lock(proceso->mutex);
		if(proceso->estado!=TERMINATED){
			pthread_mutex_unlock(proceso->mutex);
			planificador_largo_salida(proceso,"SIGKILL");
		}else{
			pthread_mutex_unlock(proceso->mutex);
		}

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
			proceso = list_get(procesos,i);
			pthread_mutex_lock(proceso->mutex);
			switch(proceso->estado){
				case NEW:string_append_with_format(&new,"PID_%s ",string_itoa(proceso->pid)); break;
				case READY: string_append_with_format(&ready,"PID_%s ",string_itoa(proceso->pid)); break;
				case BLOCKED: string_append_with_format(&blocked,"PID_%s ",string_itoa(proceso->pid)); break;
				case EXEC: string_append_with_format(&exec,"PID_%s ",string_itoa(proceso->pid)); break;
				case TERMINATED:string_append_with_format(&terminated,"PID_%s ",string_itoa(proceso->pid)); break;
			}
			pthread_mutex_unlock(proceso->mutex);
	}
	pthread_mutex_unlock(&mutexProcesos);
	char * estados = string_new();
	string_append_with_format(&estados,"%s \n",new);
	string_append_with_format(&estados,"%s \n",ready);
	string_append_with_format(&estados,"%s \n",blocked);
	string_append_with_format(&estados,"%s \n",exec);
	string_append_with_format(&estados,"%s \n",terminated);

	escritura_log(estados);

	free(new);
	free(ready);
	free(exec);
	free(blocked);
	free(terminated);
	free(estados);

	}
}

//pasar conexiones en el paramotro como array o struct
void * manejar_consola( void* args ){
	int idComando;
	sem_init(&sem_readline,0,0);
	sem_init(&procesoCargado,0,0);
	sem_post(&sem_readline);
	while(1){
		char * lectura = string_new();
		string_append(&lectura,lectura_consola());
		string_trim(&lectura);
		char** parametros = string_array_new();
		parametros = string_n_split(lectura,4," ");
		idComando = validacion_contenido_consola(parametros[0]);
		free(lectura);
		switch(idComando){
			case INICIAR_PROCESO:
					if(parametros[1]==NULL){
						break;
					}
					char* path =string_new();
					path = string_duplicate(parametros[1]);
					if(parametros[2]==NULL){
						free(path);
						break;
					}
					char* error;
					int size = strtol(parametros[2],&error,10);
					if(parametros[3]==NULL || error == parametros[2]){
						free(path);
						break;
					}
					char* error2;
					int prioridad = strtol(parametros[3],&error2,10);
					if(error2 == parametros[3]){
						free(path);
						break;
					}
					iniciar_proceso(path,size,prioridad);
					free(path);
			break;
			case FINALIZAR_PROCESO:
				if(parametros[1]!=NULL){
					char* error;
					int pid = (int)strtol((parametros[1]), &error, 10);
					if(error == parametros[1]) break;
					if(pid>0){
						finalizar_proceso(pid);
					}

				}
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
				if(parametros[1]!=NULL){
					int nuevoGrado = (int)strtol((parametros[1]), (char **)NULL, 10);
					if(nuevoGrado>0 ){
						char * mensaje = string_from_format("Grado Anterior: %d",gradoMultiprogramacion);
						gradoMultiprogramacion=nuevoGrado;
						string_append_with_format(&mensaje," - Grado Actual: %d",gradoMultiprogramacion);
						escritura_log(mensaje);
						free(mensaje);
						if(!detenida){sem_post(&planiLargo);}
					}
				}
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
		string_array_destroy(parametros);
		sem_post(&sem_readline);
	}
}
