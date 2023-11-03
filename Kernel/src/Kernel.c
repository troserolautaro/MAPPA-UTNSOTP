#include "Kernel.h"


//Sockets
int conexionCPUDispatch, conexionCPUInterrupt,conexionMemoria,conexionFileSystem;

//Otros
char* AlgoritmoPlanificacion, *recursos,* instanciasRecursos;
int PIDGLOBAL = 0;
t_log* logger;
//LISTA DE TODOS LOS PROCESOS
t_list* procesos;

//COLA LARGO PLAZO
t_queue* colaLargo;

/*DECIDI HACERLO UNA COLA PORQUE CREI QUE ERA LO MEJOR */
t_queue* colaCorto;

/*TEMPORAL BOOLEANO PARA CONTROLAR LA INICIACION Y DETENCION DE PLANIFICACION*/
bool detenida=false;


int gradoMultiprogramacion, quantum;

int main(void)
{
	char* ipCPU=malloc(sizeof(char*)),*ipMemoria=malloc(sizeof(char*)),*ipFileSystem=malloc(sizeof(char*)) ;
	char* puertoCPUDispatch=malloc(sizeof(char*)),*puertoCPUInterrupt=malloc(sizeof(char*)),
		* puertoMemoria=malloc(sizeof(char*)),*puertoFileSystem=malloc(sizeof(char*));

	logger=malloc(sizeof(t_log));
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
	//MEJOR => atoi: (int)strtol(nptr, (char **)NULL, 10)
	//CONFIGURACION DE MEMORIA
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria=config_get_string_value(config,"PUERTO_MEMORIA");

	//CONFIGURACION DE FILESYSTEM
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoFileSystem=config_get_string_value(config,"PUERTO_FILESYSTEM");



	/************************************INICIALIZAR CONEXIONES************************************/
	conexionCPUDispatch = crear_conexion(ipCPU, puertoCPUDispatch,KERNEL);
	//conexionCPUInterrupt = crear_conexion(ipCPU, puertoCPUInterrupt,KERNEL);
	conexionMemoria = crear_conexion(ipMemoria, puertoMemoria,KERNEL);
	//conexionFileSystem = crear_conexion(ipFileSystem, puertoFileSystem,KERNEL);

	/************************************INICIALIZAR HILOS DE RECIBO DE MENSAJES************************************/
	//HILO DE MANEJO DE MOODULOS
	pthread_t hiloCPUDispatch;
	pthread_create(&hiloCPUDispatch,NULL,manejar_cliente,&conexionCPUDispatch);

	//pthread_t * hiloCPUInterrupt;
	//pthread_create(hiloCPUInterrupt,NULL,manejar_cliente,conexionCPUInterrupt);

	pthread_t  hiloMemoria;

	int resultado;
	if ((resultado=pthread_create(&hiloMemoria,NULL,manejar_cliente,&conexionMemoria))!=0)
		printf("Error al crear hilo. resultado %d",resultado);

	//pthread_t * hiloFilesystem;
	//pthread_create(hiloFilesystem,NULL,manejar_cliente, conexionFileSystem);

	/************************************INICIO CONSOLA INTERACTIVA*************************************************/
	pthread_t hiloConsola;
	pthread_create(&hiloConsola,NULL,manejar_consola, NULL);
	/************************************FINALIZA LOS PROGRAMAS O HILOS A FUTURO************************************/
	pthread_join(hiloCPUDispatch,NULL);
	//pthread_join(hiloCPUInterrupt,NULL);
	pthread_join(hiloMemoria,NULL);
	//pthread_join(hiloFilesystem,NULL);
	//pthread_join(hiloConsola,NULL);
	config_destroy(config);
	return EXIT_SUCCESS;
}

//pasar conexiones en el paramotro como array o struct
void * manejar_consola( void* args ){
	int idComando;
	while(1){
		char * lectura = string_new();
		string_append(&lectura,lectura_consola());
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
					planificador_largo();

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

	/************************************INICIO CONSOLA INTERACTIVA*************************************************/

	/************************************FINALIZA LOS PROGRAMAS O HILOS A FUTURO************************************/

	terminar_programa(conexionCPUDispatch, logger);
	terminar_programa(conexionCPUInterrupt, logger);
	terminar_programa(conexionMemoria, logger);
	terminar_programa(conexionFileSystem, logger);
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
		//crea el proceso
		PCB* proceso = malloc(sizeof(PCB));
		proceso->estado = NEW;
		proceso->prioridad = prioridad;
		proceso->registros.AX= 0;
		proceso->registros.BX = 0;
		proceso->registros.CX = 0;
		proceso->registros.DX = 0;
		proceso->pc=0;
		proceso->pid = PIDGLOBAL; //Modificar en caso de que sea necesario
		//añade el proceso a la lista de procesos y a la cola del planificador a largo plazo
		list_add(procesos,proceso);
		queue_push(colaLargo,proceso);
		//envia archivo a cargar en memoria para este proceso a el modulo de memoria
		//SE PUEDE EXPORTAR SI ES NECESARIO EL ENVIO DE PAQUETE SI ES NECESARIO
		t_paquete * paqueteArchivo=crear_paquete();
		int pid=proceso->pid;
		agregar_a_paquete(paqueteArchivo, "cargar", sizeof(char*)*6);
		agregar_a_paquete(paqueteArchivo, &pid, sizeof(int*));
		agregar_a_paquete(paqueteArchivo, path, (sizeof(path)-1));
		agregar_a_paquete(paqueteArchivo, &size, sizeof(int*));
		//WAIT SEMAFORO DE CONEXION A MEORIA
		enviar_paquete(paqueteArchivo,conexionMemoria);
		//SIGNAL
		eliminar_paquete(paqueteArchivo);
}


void finalizar_proceso(int pid){
	//POSIBLE MUTEX
	if(pid<=(list_size(procesos))){
		//porque asigna pid= -1 ?
		pid-=1;
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
		PCB* proceso;
		for(i = 0 ; i<list_size(procesos); i++){
			proceso= list_get(procesos,i);
			printf("[PID]: %d  ",proceso->pid);
			printf("[ESTADO]: ");
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
		free(proceso);
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

	if(!strcasecmp(comando, "iniciar_proceso\0")){

		return INICIAR_PROCESO;
	}
	if(!strcasecmp(comando, "finalizar_proceso\0")){

		return FINALIZAR_PROCESO;
	}
	if(!strcasecmp(comando, "iniciar_planificacion\0")){

		return INICIAR_PLANIFICACION;
	}
	if(!strcasecmp(comando, "detener_planificacion\0")){

		return DETENER_PLANIFICACION;
	}
	if(!strcasecmp(comando, "multiprogramacion\0")){

		return MULTIPROGRAMACION;
	}
	if(!strcasecmp(comando, "proceso_estado\0")){

		return PROCESO_ESTADO;
	}
	if(!strcasecmp(comando,"Exit\0")){
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
	//semaforo de gradoMultiprogramacion
	free(temp);
	planificador_largo();
}


void planificador_corto(){
		/*PRUEBA DE COMO HACER EL PLANIFICADOR */
		int idPlanificador = planificador_enum();
		switch(idPlanificador){
			case PRIORIDADES:
				prioridad();
				break;

			case ROUNDROBIN:
				round_robin();
				break;
			case FIFO:

				break;
			default:
				printf("No se reconocio el algoritmo");
				break;
		}
		//si no es ninguno de los anterior es fifo por que es una cola (estructura de tipo fifo)

		PCB* proceso= queue_pop(colaCorto);
		proceso->estado=EXEC;
		//ENVIAR PROCESO
		enviar_mensaje("INICIAR PROCESO",conexionCPUDispatch);
}

void prioridad(){
	ordenar_prioridades(&colaCorto->elements);
	//Horrible lo que hay que hacer aca
}

void round_robin(){
//si el proceso (proceso con estado en EXEC) en ejecucion completo el quantum, cambia el estado, envia interrupccion a cpu y lo manda al final de la cola
//si termino no hace nada.
//si se bloqueo por io lo manda al final de la cola
}

bool ordenar_prioridades(t_list** lista ){
	t_list_iterator* i = list_iterator_create(*lista);
	while (i->index	< list_size(*lista)){
		PCB* actual =  (PCB*) i->actual;
		PCB* siguiente = (!(i->next != NULL)) ?   (PCB*)i->next : NULL;

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

int planificador_enum(){
	if(!strcasecmp(AlgoritmoPlanificacion, "prioridades\0")){

			return PRIORIDADES;
	}
	if(!strcasecmp(AlgoritmoPlanificacion, "round robin\0")){

			return ROUNDROBIN;
	}
	if(!strcasecmp(AlgoritmoPlanificacion, "fifo\0")){

			return FIFO;
	}
	return -1;
}
void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);
	if(!strcasecmp(msg,"CrearProceso")){
		int pid=*(int*)list_get(mensaje,1);
		int resultado=*(int*)list_get(mensaje,2);
		//1 valido, 0 no valido
		if(resultado>0){
			printf("se cargo archivo en memoria, proceso %d",pid);
		}
	}
	free(msg);
}
