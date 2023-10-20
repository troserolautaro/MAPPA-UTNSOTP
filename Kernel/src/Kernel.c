#include "Kernel.h"


//VARIABLES GLOBALES
int conexionCPUDispatch, conexionCPUInterrupt,conexionMemoria,conexionFileSystem;
char* AlgoritmoPlanificacion,* recursos,* instanciasRecursos;
int PIDGLOBAL = 0;

//LISTA DE TODOS LOS PROCESOS
t_list* procesos;

//COLA LARGO PLAZO
t_queue* colaLargo;

/*DECIDI HACERLO UNA COLA PORQUE CREI QUE ERA LO MEJOR */
t_queue* colaCorto;

/*TEMPORAL BOOLEANO PARA CONTROLAR LA INICIACION Y DETENCION DE PLANIFICACION*/
bool detenida=false;
bool prioridades = false;


int gradoMultiprogramacion, quantum;

int main(void)
{
	char* ipCPU,* ipMemoria,* ipFileSystem ;
	char* puertoCPUDispatch,*puertoCPUInterrupt,* puertoMemoria,* puertoFileSystem ;
	char* valor;

	t_log* logger=malloc(sizeof(logger));
	t_config* config=malloc(sizeof(config));
	logger = iniciar_logger();
	config = iniciar_config();

	/*INICIALIZAR LISTAS */
	procesos=list_create();
	colaLargo=queue_create();
	colaCorto=queue_create();

	/************************************RECUPERA DATOS DE ARCHIVO DE CONFIGURACION************************************/
	//TALVEZ SE PUEDE GLOBALIZAR Y PASAR A UNA FUNCION PARA QUE QUEDE MEJOR PARA LA LECTURA
	//CONFIGURACION DE CPU
	ipCPU = config_get_string_value(config,"IP_CPU");
	puertoCPUDispatch=config_get_string_value(config,"PUERTO_CPU_DISPATCH");
	puertoCPUInterrupt=config_get_string_value(config,"PUERTO_CPU_INTERRUPT");
	AlgoritmoPlanificacion=config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	quantum=atoi(config_get_string_value(config,"QUANTUM"));
	gradoMultiprogramacion=atoi(config_get_string_value(config,"GRADO_MULTIPROGRAMACION_INI"));

	//CONFIGURACION DE MEMORIA
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria=config_get_string_value(config,"PUERTO_MEMORIA");

	//CONFIGURACION DE FILESYSTEM
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoFileSystem=config_get_string_value(config,"PUERTO_FILESYSTEM");




	/************************************INICIALIZAR CONEXIONES************************************/

	conexionCPUDispatch = crear_conexion(ipCPU, puertoCPUDispatch);
	//conexionCPUInterrupt = crear_conexion(ipCPU, puertoCPUInterrupt);
	//int conexiones[4];
	//conexiones[0]=conexionCPUDispatch;
	//conexiones[1]=conexionCPUInterrupt;

//	conexionMemoria = crear_conexion(ipMemoria, puertoMemoria);
//	conexionFileSystem = crear_conexion(ipFileSystem, puertoFileSystem);

	/************************************INICIALIZAR HILOS DE ENVIO Y RECIBO DE MENSAJES************************************/
	//HILO DE MANEJO DE CONSOLA
	pthread_t hiloConsola;
	pthread_create(&hiloConsola,NULL,manejar_consola,NULL );

	//Habria que hacer un producto consumidor
	//manejar_consola(NULL);
	/************************************INICIO CONSOLA INTERACTIVA*************************************************/

	/************************************FINALIZA LOS PROGRAMAS O HILOS A FUTURO************************************/
	pthread_join(hiloConsola,NULL);
	config_destroy(config);
	return EXIT_SUCCESS;
}

//pasar conexiones en el paramotro como array o struct
void * manejar_consola( void* args ){
	while(1){
		char* comando = lectura_consola();
		char* contexto;
		char* token;


		token = strtok_r(comando, " ", &contexto);
		int idComando = validacion_contenido_consola(comando);
		/*PROBABLEMENTE HAY QUE MEJORAR ESTO, SI BIEN FUNCIONA NO TOMA LOS PARAMETROS QEU SE INGRESAN EN
		 * CONSOLA */
		switch(idComando){
			case INICIAR_PROCESO:
					char* path=malloc(sizeof(char*));
					int size;
					int prioridad;
					/* ESTA FUNCION SE ENCARGA DE ASIGNAR LA DIFERENTES PARTES DEL COMANDO A SU RESPECTIVA VARIABLE */
					int i=0;
					while (token != NULL && !strcmp(token," ")) {
						if(i==0){
							path=token;
						}else{
							char* end=malloc(sizeof(char*));
							long var = strtol(token,&end,10);
							if(var!=0){
								if(i==1){
									size=atoi(token);
								}else{
									prioridad=atoi(token);
								}
							}else{
								printf("Interrumpido el inicio del proceso, mal ingresado un valor");
							}

						}
						i++;
						token = strtok_r(NULL, " ", &contexto);
					}

					iniciar_proceso(path,size,prioridad);
					planificador_largo();
					printf("INICIAR PROCESO \n");
					free(path);
			break;

			case FINALIZAR_PROCESO:
					printf("FINALIZAR PROCESO \n");
					int pid;
					while (token != NULL) {
						pid=atoi(token);
						token = strtok_r(NULL, " ", &contexto);
					}
					finalizar_proceso(pid);
//				enviar_mensaje("FINALIZAR PROCESO",conexionCPUDispatch);
			break;

			case INICIAR_PLANIFICACION:
					printf("INICIAR PLANIFICACION \n");
					enviar_mensaje("INICIAR PLANIFICACION",conexionCPUDispatch);
			break;

			case DETENER_PLANIFICACION:
					printf("DETENER PLANIFICACION \n");
					enviar_mensaje("DETENER PLANIFICACION",conexionCPUDispatch);
			break;

			case MULTIPROGRAMACION:
					printf("MULTIPROGRAMACION \n");
					enviar_mensaje("MULTIPROGRAMACION",conexionCPUDispatch);
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
			free(comando);

	}


	/************************************INICIO CONSOLA INTERACTIVA*************************************************/

	/************************************FINALIZA LOS PROGRAMAS O HILOS A FUTURO************************************/

//	terminar_programa(conexionCPUDispatch, logger, config);
//	terminar_programa(conexionCPUInterrupt, logger, config);
	terminar_programa(conexionMemoria, logger);

//	terminar_programa(conexionFileSystem, logger, config);
}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger =log_create("./tp.log","log",1,LOG_LEVEL_INFO);
	if(nuevo_logger == NULL){
			perror("No se ha encontrado el logger\n");
		}
	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config = config_create("./Kernel.config");
	if(nuevo_config == NULL){
		perror("No se ha encontrado el config\n");
	}
	return nuevo_config;
}

//void leer_consola(t_log* logger)
//{
//	char* leido;
//	   while (1) {
//	        leido = readline(">");
//	        if (!strcmp(leido, "")) {
//	        	free(leido);
//	        	break;
//	        }
//	        free(leido);
//	    }
//}

void paquete(int conexion)
{
	char* leido;
	t_paquete* paquete=crear_paquete();
	while (1) {
			leido = readline(">");
			if (!strcmp(leido, "")) {
				free(leido);
				break;
			}
	}
}
void iniciar_planificacion(){
	if(detenida == true){
		detenida=false;
	}

}
void detener_planificacion(){
	detenida=true;
}

void terminar_programa(int conexion, t_log* logger)
{
	log_destroy(logger);
	liberar_conexion(conexion);

}

void iniciar_proceso(char* path, int size, int prioridad){
		//Mutex
		PIDGLOBAL++;
		PCB* proceso = malloc(sizeof(PCB));

		proceso->estado = NEW;
		proceso->prioridad = prioridad;

		proceso->registros.AX= 0;
		proceso->registros.BX = 0;
		proceso->registros.CX = 0;
		proceso->registros.DX = 0;
		proceso->pid = PIDGLOBAL; //Modificar en caso de que sea necesario
		list_add(procesos,proceso);
		queue_push(colaLargo,proceso);

}


void finalizar_proceso(int pid){
	//POSIBLE MUTEX
	if(pid<=(list_size(procesos))){
		pid=-1;
		/*POR AHORA PONGO COMENTARIOS DONDE TALVEZ HABRIA QUE HACER UN FREE! */
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
	//POSIBLE MUTEX PARA LA LISTA DE PROCESOS
	if(!list_is_empty(procesos)){
		int i;
		for(i = 0 ; i<list_size(procesos); i++){

			PCB* proceso= list_get(procesos,i);
			printf("[PID] %d \n",proceso->pid);
			printf("[ESTADO] ");

			switch(proceso->estado){
				case NEW:
					printf("NEW\n");
				break;
				case READY:
					printf("READY\n");
				break;
				case EXEC:
					printf("EXEC\n");
				break;
				case BLOCKED:
					printf("BLOCKED\n");
				break;
				case TERMINATED:
					printf("TERMINATED\n");
				break;
				default:
					printf("Estado no definido\n");
				break;
				}
			}

	}else{
		printf("No hay procesos \n");
	}
}

char* lectura_consola(){
	char* linea;
	linea = readline(">>");
	if (linea) {
		add_history(linea);
	}
	return linea;
}
int validacion_contenido_consola(char* comando){
	if(!strcasecmp(comando, "iniciar_proceso")){

		return INICIAR_PROCESO;
	}
	if(!strcasecmp(comando, "finalizar_proceso")){

		return FINALIZAR_PROCESO;
	}
	if(!strcasecmp(comando, "iniciar_planificacion")){

		return INICIAR_PLANIFICACION;
	}
	if(!strcasecmp(comando, "detener_planificacion")){

		return DETENER_PLANIFICACION;
	}
	if(!strcasecmp(comando, "multiprogramacion")){

		return MULTIPROGRAMACION;
	}
	if(!strcasecmp(comando, "proceso_estado")){

		return PROCESO_ESTADO;
	}
	if(!strcasecmp(comando,"Exit")){
		return EXIT;
	}

	return -2;
}
/*PROBABLEMENTE AL SER LLAMADOS EN MUCHOS LADOS LOS PLANIFICADORES DEBERIAMOS O HACERLOS THREAD SAFE O CADA VEZ QUE LOS USEMOS
 * USAR UN MUTEX */
void planificador_largo(){
	while(gradoMultiprogramacion>=queue_size(colaCorto) && !detenida && !queue_is_empty(colaLargo)){
		PCB* proceso=queue_pop(colaLargo);
		proceso->estado=READY;
		queue_push(colaCorto,proceso);
	}
	planificador_corto();
}

void planificador_largo_salida(PCB* proceso){
	/*TALVEZ HABRIA QUE LIBERAR EL PROCESO(?) PERO COMO YO LO ENTIENDO AL REFERENCIAR EL PUNTERO CAMBIA DONDE ESTA APUNTANDO. */
	PCB* temp=(list_get(procesos,proceso->pid));
	proceso=temp;
	proceso->estado=TERMINATED;
	planificador_largo();
	free(temp);
}


void planificador_corto(){
		/*PRUEBA DE COMO HACER EL PLANIFICADOR */
		if(prioridades){

			prioridad();
			/*Mandar mensaje con el quantum del kernel y el proceso */
		}else{
			if(quantum < 1 ){

				fifo();
				/*Manda mensaje al cpu */

			}else{

				round_robin();
				/*Mandar mensaje con el quantum del kernel y el proceso */

			}

		}
		PCB* proceso= queue_pop(colaCorto);
		proceso->estado=EXEC;

}
void fifo(){


}

void prioridad(){
	ordenar_prioridades(&colaCorto->elements);
	//Horrible lo que hay que hacer aca
}

void round_robin(){

}

bool ordenar_prioridades(t_list** lista ){
	t_list_iterator* i = list_iterator_create(*lista);
	while (i->index	< list_size(*lista)){
		PCB* actual = *i->actual;
		PCB* siguiente = (!(i->next != NULL)) ?   *i->next : NULL;

		if(siguiente == NULL){
			break;
		}

		if(actual->prioridad > siguiente->prioridad){
			PCB* temp = actual;
			actual = siguiente;
			siguiente = temp;
		}

		list_iterator_next(i);
	}
	return false;
}
