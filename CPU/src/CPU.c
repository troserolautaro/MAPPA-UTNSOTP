#include "CPU.h"

//VARIABLES GLOBALES
int serverDispatch,serverInterrupt;
int conexionMemoria;
int clienteKernelDispatch,clienteKernelInterrupt;
int reloj = 0; //Es el encargado de revisar el quantum
uint32_t tamPagina, marco,valor;
bool bloquear, interrupcion, pageFault, error;
char* motivo;
//
t_log* logger;
t_config* config;
t_instruccion * instruccion;
PCB* proceso;
pthread_mutex_t mutexProceso, mutexLog,mutexInstruccion, mutexBloquear, mutexInterrupcion,mutexMotivo;
sem_t tamPagina_s, respuesta;
sem_t memoria_s;
uint32_t pcInicial = 0;
/*struct{
	t_instruccion * instruccion;
	sem_t instruccion_s;
}semaforo_instruccion;
 *
 */
//Semaforos
sem_t ciclo,instruccion_s;
int main() {


	logger = iniciar_logger("./log.log");
	t_config* config = iniciar_config("./CPU.config");

	proceso = proceso_create();
	list_destroy(proceso->recursos);
	dictionary_destroy(proceso->tablaArchivos);
	pthread_mutex_destroy(proceso->mutex);
	free(proceso->mutex);
	instruccion = instruccion_create();
	char* ipMemoria = malloc(sizeof(char*)),
			*puertoEscuchaDispatch = malloc(sizeof(char*)),
			*puertoEscuchaInterrupt = malloc(sizeof(char*)),
			*puertoMemoria = malloc(sizeof(char*));
	motivo = string_new();
	//CONFIGURACION DE CPU
	ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	puertoMemoria = config_get_string_value(config,"PUERTO_MEMORIA");
	puertoEscuchaDispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
	puertoEscuchaInterrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");

	//Iniciar semaforos
	sem_init(&ciclo,0,0);
	sem_init(&instruccion_s,0,0);
	sem_init(&tamPagina_s,0,0);
	sem_init(&memoria_s,0,0);
	sem_init(&respuesta,0,0);
	pthread_mutex_init(&mutexProceso,NULL);
	pthread_mutex_init(&mutexLog,NULL);
	pthread_mutex_init(&mutexInstruccion,NULL);
	pthread_mutex_init(&mutexBloquear,NULL);
	pthread_mutex_init(&mutexInterrupcion,NULL);
	pthread_mutex_init(&mutexMotivo,NULL);
	//Iniciar Cliente que conecta a memoria
	conexionMemoria = crear_conexion(ipMemoria, puertoMemoria,CPUDispatch);

	//Inicia Servidor
	serverDispatch = iniciar_servidor(puertoEscuchaDispatch);
	serverInterrupt = iniciar_servidor(puertoEscuchaInterrupt);
	// log_info(logger, "Servidor listo para recibir al cliente");
	debug("Servidor listo para recibir al cliente");
	clienteKernelDispatch = esperar_cliente(serverDispatch);
	clienteKernelInterrupt = esperar_cliente(serverInterrupt);

	//HILOS
	pthread_t hiloKernelDispatch, hiloCiclo,hiloMemoria,hiloKernelInterrupt;
	//Hilos conexion
	int resultado;
	if ((resultado = pthread_create(&hiloKernelDispatch,NULL,manejar_cliente,(void*)&clienteKernelDispatch))!=0){
		printf("Error al crear hilo. resultado %d",resultado);
	}
	if ((resultado = pthread_create(&hiloKernelInterrupt,NULL,manejar_cliente,&clienteKernelInterrupt))!=0){
		printf("Error al crear hilo. resultado %d",resultado);
	}
	if ((resultado = pthread_create(&hiloMemoria,NULL,manejar_cliente,(void*)&conexionMemoria))!=0){
		printf("Error al crear hilo. resultado %d",resultado);
	}
	cargar_tamanio_pagina();
	sem_wait(&tamPagina_s);
	//Hilos proceso
	bloquear= false;
	interrupcion=false;
	pthread_create(&hiloCiclo,NULL,(void*)ejecutar_ciclo,NULL);

	pthread_join(hiloKernelDispatch,NULL);
	pthread_join(hiloCiclo,NULL);
	pthread_join(hiloMemoria,NULL);
	pthread_join(hiloKernelInterrupt,NULL);
	return EXIT_SUCCESS;
}
void cargar_tamanio_pagina(){
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"tamanioPagina",sizeof("tamanioPagina"));
	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);

}
//TRADUCCIR DIRECCION LOGICA A FISICA
uint32_t mmu(uint32_t* direccionLogica){
	uint32_t numPagina,desplazamiento;
	//debug(string_itoa(*(int*)direccionLogica));
	numPagina = floor((*direccionLogica) / tamPagina);
	desplazamiento = (*direccionLogica) - numPagina * tamPagina;
	obtener_marco(numPagina);
	if(!pageFault){
		escritura_log(string_from_format("PID %d - OBTENER MARCO - Pagina: %d - Marco: %d",proceso->pid,numPagina,marco));
		return marco*tamPagina+desplazamiento ;
	}else{
		escritura_log(string_from_format("Page Fault PID: %d - Página: %d",proceso->pid,numPagina));
	}
	return -1;
}
//PARA FUNCIONES DE F_*
void obtener_marco(uint32_t numPagina){
	uint32_t pid = proceso->pid;
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"marco",sizeof("marco"));
	agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
	agregar_a_paquete(paquete,&numPagina,sizeof(uint32_t));
	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);
	sem_wait(&memoria_s);
}
void page_fault(uint32_t* direccionLogica){
	pthread_mutex_lock(&mutexProceso);
	proceso->pc--;
	pthread_mutex_unlock(&mutexProceso);
	bloquear_proceso();
	uint32_t numPagina = floor((*direccionLogica) / tamPagina);
	t_list * mensaje = list_create();
	list_add(mensaje,"page_fault");
	list_add(mensaje,string_itoa((int)numPagina));
	contexto_ejecucion(mensaje);
}
void bloquear_proceso(){
	pthread_mutex_lock(&mutexBloquear);
	bloquear=true;
	pthread_mutex_unlock(&mutexBloquear);
}
//FUNCIONES DE INSTRUCCION
void set(uint32_t *registro, int valor){
	*registro = valor;
}

void sum(uint32_t * registroDestino,uint32_t *registroOrigen){
	*registroDestino += *registroOrigen;
}
void sub(uint32_t * registroDestino,uint32_t * registroOrigen){
	*registroDestino -= *registroOrigen;
}

void jnz(uint32_t * registro,uint32_t pc){
	pthread_mutex_lock(&mutexProceso);
	if(*registro!=0)proceso->pc=pc;
	pthread_mutex_unlock(&mutexProceso);
}

void sleep_proceso(uint32_t tiempo){
	t_list * mensaje = list_create();
	list_add(mensaje,"sleep");
	list_add(mensaje,string_itoa((int)tiempo));
	contexto_ejecucion(mensaje);

	bloquear_proceso();

	list_destroy(mensaje);
}

void wait_recurso(char* recurso){
	t_list * mensaje = list_create();
	list_add(mensaje,"wait");
	list_add(mensaje,recurso);
	contexto_ejecucion(mensaje);

	bloquear_proceso();

	list_destroy(mensaje);
}

void signal_recurso(char* recurso){
	t_list * mensaje = list_create();
	list_add(mensaje,"signal");
	list_add(mensaje,recurso);
	contexto_ejecucion(mensaje);

	bloquear_proceso();

	list_destroy(mensaje);
}

void mov_in(uint32_t* registro, uint32_t* direccionLogica) {
	uint32_t direccionFisica;
	uint32_t pid = proceso->pid;
	direccionFisica=mmu(direccionLogica);
	if(!pageFault){
		t_paquete* paquete=crear_paquete();
		agregar_a_paquete(paquete,"mov_in",sizeof("mov_in"));
		agregar_a_paquete(paquete,&direccionFisica,sizeof(uint32_t));
		agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
		enviar_paquete(paquete,conexionMemoria);
		eliminar_paquete(paquete);
		escritura_log(string_from_format("PID: %d - Accion: Leer - Direccion Fisica: %d - Valor: %d", proceso->pid,direccionFisica, *registro));
		sem_wait(&memoria_s);
		*registro = valor;
		if(error){
		//mostrar error
		}
	}
	else{
		page_fault(direccionLogica);
	}
}
void mov_out(uint32_t* direccionLogica,uint32_t* registro) {
	uint32_t direccionFisica;
	uint32_t pid = proceso->pid;
	direccionFisica=mmu(direccionLogica);
	if(!pageFault){
		t_paquete* paquete=crear_paquete();
		agregar_a_paquete(paquete,"mov_out",sizeof("mov_out"));
		agregar_a_paquete(paquete,&direccionFisica,sizeof(uint32_t));
		agregar_a_paquete(paquete,registro,sizeof(uint32_t));
		agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
		enviar_paquete(paquete,conexionMemoria);
		eliminar_paquete(paquete);
		escritura_log(string_from_format("PID: %d - Accion: Escribir - Direccion Fisica: %d - Valor %d", proceso->pid,direccionFisica, *registro));
		sem_wait(&memoria_s);
		if(error){
			//mostrar error
		}
	}else{
		page_fault(direccionLogica);
	}
}

void f_open(char* nombreArchivo, char* modoApertura) {
	bloquear_proceso();
	t_list * mensaje = list_create();
	list_add(mensaje,"f_open");
	list_add(mensaje,nombreArchivo);
	list_add(mensaje,modoApertura);
	contexto_ejecucion(mensaje);



	list_destroy(mensaje);
}

void f_close(char* nombreArchivo) {
	bloquear_proceso();
	t_list * mensaje = list_create();
	list_add(mensaje,"f_close");
	list_add(mensaje,nombreArchivo);
	contexto_ejecucion(mensaje);



	list_destroy(mensaje);
}

void f_seek(char* nombreArchivo, uint32_t posicion) {
	bloquear_proceso();
	t_list * mensaje = list_create();
	list_add(mensaje,"f_seek");
	list_add(mensaje,nombreArchivo);
	list_add(mensaje,string_itoa(posicion));
	contexto_ejecucion(mensaje);



	list_destroy(mensaje);
}

void f_read(char* nombreArchivo, uint32_t direccionLogica) {

	uint32_t direccionFisica;
	direccionFisica=mmu(&direccionLogica);
	if(!pageFault){
		bloquear_proceso();
		t_list * mensaje = list_create();
		list_add(mensaje,"f_read");
		list_add(mensaje,nombreArchivo);
		list_add(mensaje,string_itoa(direccionFisica));

		contexto_ejecucion(mensaje);



		list_destroy(mensaje);
	}
	else{
		page_fault(&direccionLogica);
	}
}

void f_write(char* nombreArchivo, uint32_t direccionLogica) {
	uint32_t direccionFisica;
	direccionFisica=mmu(&direccionLogica);
	if(!pageFault){
		bloquear_proceso();
		t_list * mensaje = list_create();
		list_add(mensaje,"f_write");
		list_add(mensaje,nombreArchivo);
		list_add(mensaje,string_itoa(direccionFisica));
		contexto_ejecucion(mensaje);




		list_destroy(mensaje);
	}
	else{
		page_fault(&direccionLogica);
	}
}

void f_truncate(char* nombreArchivo, uint32_t newSize) {
	bloquear_proceso();
	t_list * mensaje = list_create();
	list_add(mensaje,"f_truncate");
	list_add(mensaje,nombreArchivo);
	list_add(mensaje,string_itoa(newSize));
	contexto_ejecucion(mensaje);


	list_destroy(mensaje);
}

void exit_i(){
	//busca la instruccion en memoria
	t_list * mensaje = list_create();
	list_add(mensaje,"procesoExit");
	contexto_ejecucion(mensaje);
	bloquear_proceso();
	list_destroy(mensaje);
	/*t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"proceso_exit",sizeof(char*)*11);
	serializar_proceso(paquete,proceso);
	enviar_paquete(paquete,clienteKernelDispatch);
	eliminar_paquete(paquete);*/
}


//FUNCION AUXILIAR PARA OBTENER LOS REGISTROS INDICADOS EN LAS INSTRUCCIONES
uint32_t * obtener_registro(char* registro){
	uint32_t* puntero = NULL;
	if(!strcasecmp(registro, "AX")){
		pthread_mutex_lock(&mutexProceso);
		puntero = &(proceso->registros->AX);
		pthread_mutex_unlock(&mutexProceso);
		return puntero;
	}
	if(!strcasecmp(registro, "BX")){
		pthread_mutex_lock(&mutexProceso);
		puntero = &(proceso->registros->BX);
		pthread_mutex_unlock(&mutexProceso);
		return puntero;
	}
	if(!strcasecmp(registro, "CX")){
		pthread_mutex_lock(&mutexProceso);
		puntero = &(proceso->registros->CX);
		pthread_mutex_unlock(&mutexProceso);
		return puntero;
	}
	if(!strcasecmp(registro, "DX")){
		pthread_mutex_lock(&mutexProceso);
		puntero = &(proceso->registros->DX);
		pthread_mutex_unlock(&mutexProceso);
		return puntero;
	}
	return puntero;
}

//FUNCIONES PARA CICLO DE EJECUCION SIMPLE
void fetch(){
	//busca la instruccion en memoria
	pthread_mutex_lock(&mutexProceso);
	uint32_t pid = proceso->pid;
	uint32_t pc = proceso->pc;
	pthread_mutex_unlock(&mutexProceso);

	char* mensaje = string_from_format("PID: %d - FETCH - Program Counter: %d",pid,pc);
	escritura_log(mensaje);
	free(mensaje);
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"instruccion",sizeof("instruccion"));
	agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
	agregar_a_paquete(paquete,&pc,sizeof(uint32_t));
	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);

	pthread_mutex_lock(&mutexProceso);
	proceso->pc++;
	pthread_mutex_unlock(&mutexProceso);
// Fetch Instrucción: “PID: <PID> - FETCH - Program Counter: <PROGRAM_COUNTER>”.
}

void decode_and_execute(){
	pthread_mutex_lock(&mutexInstruccion);
	t_list * parametros=instruccion->parametros;
	char* comando = instruccion->comando;
	pthread_mutex_unlock(&mutexInstruccion);
	uint32_t *registroOrigen, *registroDestino;
	char*recurso;
	//obtengo parametros
	/*executo funcion, talvez es mejor que la funcion reciba un int y esto este dentro de un switch en el que adentro
	revisemos las variables que haya que utilizar*/
	if(!strcasecmp(comando, "SET")){
		registroDestino=obtener_registro((char*)list_get(parametros,0));
		int valor = (int)strtol(list_get(parametros,1), (char **)NULL, 10);
		set(registroDestino,valor);
	}
	if(!strcasecmp(comando, "SUM")){
		registroDestino = obtener_registro((char*)list_get(parametros,0));
		registroOrigen =  obtener_registro((char*)list_get(parametros,1));
		sum(registroDestino,registroOrigen);
	}
	if(!strcasecmp(comando, "SUB")){
		registroDestino = obtener_registro((char*)list_get(parametros,0));
		registroOrigen =  obtener_registro((char*)list_get(parametros,1));
		sub(registroDestino,registroOrigen);
	//	debug(string_itoa(*registroDestino));
	}
	if(!strcasecmp(comando,"JNZ")){
		registroOrigen = obtener_registro((char*)list_get(parametros,0));
		uint32_t jnzPC=(uint32_t)strtol(list_get(parametros,1),NULL,10);
		jnz(registroOrigen,jnzPC);
	}
	if(!strcasecmp(comando,"SLEEP")){
		uint32_t tiempo=(uint32_t)strtol(list_get(parametros,0),NULL,10);
		sleep_proceso(tiempo);
	}
	if(!strcasecmp(comando,"WAIT")){
		recurso=list_get(parametros,0);
		wait_recurso(recurso);
	}
	if(!strcasecmp(comando,"SIGNAL")){
		recurso=list_get(parametros,0);
		signal_recurso(recurso);
	}
	if(!strcasecmp(comando,"MOV_IN")){
		registroOrigen =  obtener_registro((char*)list_get(parametros,0));
		uint32_t direccionLogica =(uint32_t)strtol(list_get(parametros,1),NULL,10);
		mov_in(registroOrigen,&direccionLogica);
	}
	if (!strcasecmp(comando, "MOV_OUT")) {
		registroDestino =  obtener_registro((char*)list_get(parametros,1));
		uint32_t direccionLogica =(uint32_t)strtol(list_get(parametros,0),NULL,10);
	    mov_out(&direccionLogica,registroDestino);
	}
	if (!strcasecmp(comando, "F_OPEN")) {
	    f_open((char*)list_get(parametros,0), (char*)list_get(parametros,1));
	}
	if (!strcasecmp(comando, "F_CLOSE")) {
	    f_close((char*)list_get(parametros,0));
	}
	if (!strcasecmp(comando, "F_SEEK")) {
	    f_seek((char*)list_get(parametros,0),((uint32_t)strtol(list_get(parametros,1),NULL,10)));
	}
	if (!strcasecmp(comando, "F_READ")) {
	    f_read((char*)list_get(parametros,0), ((uint32_t)strtol(list_get(parametros,1),NULL,10)));
	}
	if (!strcasecmp(comando, "F_WRITE")) {
	    f_write((char*)list_get(parametros,0), ((uint32_t)strtol(list_get(parametros,1),NULL,10)));
	}
	if (!strcasecmp(comando, "F_TRUNCATE")) {
	    f_truncate((char*)list_get(parametros,0), ((uint32_t)strtol(list_get(parametros,1),NULL,10)));
	}
	if(!strcasecmp(comando, "EXIT")){
		exit_i();
	}
	pthread_mutex_lock(&mutexInstruccion);
	list_clean_and_destroy_elements(instruccion->parametros,(void*)liberar_memoria);
	pthread_mutex_unlock(&mutexInstruccion);
	//	Instrucción Ejecutada: “PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.
}
void check_interrupt(){
	pthread_mutex_lock(&mutexBloquear);
	pthread_mutex_lock(&mutexInterrupcion);
	if(!interrupcion && !bloquear){sem_post(&ciclo);}
	pthread_mutex_unlock(&mutexBloquear);
	pthread_mutex_unlock(&mutexInterrupcion);
	pthread_mutex_lock(&mutexBloquear);
	if(bloquear){

			bloquear=false;
			sem_wait(&respuesta);
	}
	pthread_mutex_unlock(&mutexBloquear);

	pthread_mutex_lock(&mutexInterrupcion);
	if(interrupcion){
		t_list * mensaje = list_create();
		pthread_mutex_lock(&mutexMotivo);
		list_add(mensaje,motivo);
		pthread_mutex_unlock(&mutexMotivo);
		contexto_ejecucion(mensaje);
		list_destroy(mensaje);
		interrupcion=false;
	}
	pthread_mutex_unlock(&mutexInterrupcion);
}

//FUNCION QUE EJECUTA CICLO DE INSTRUCCION
//agregar luego parametro pcb a ejecutar ciclo de instruccion;
void ejecutar_ciclo(){

	do{
		sem_wait(&ciclo);
		fetch();
		sem_wait(&instruccion_s);
		decode_and_execute();
		check_interrupt();
	}while(true);
}
int razon_interrupcion(char * razon){
	if(!strcasecmp(razon,"desalojo")) return DESALOJO;
	return -1;
}
void procesar_mensaje(t_list* mensaje){
	char* msg = string_new();
	string_append(&msg,list_get(mensaje,0));
	string_trim(&msg);
	string_to_lower(msg);

//Seria excelente cuanto menos aprovechar que dentro de la lista "mensaje" se encuentra al final el socket para dividir con un switch las funciones
	if(!strcasecmp(msg,"tamanioPagina")){
		tamPagina=*(int*)list_get(mensaje,1);
		sem_post(&tamPagina_s);
	}
	if(!strcasecmp(msg,"marco")){
		pageFault = *(bool*)list_get(mensaje,1);
		marco = *(uint32_t*)list_get(mensaje,2);

		sem_post(&memoria_s);
	}
	if(!strcasecmp(msg,"mov_in")){
			error=*(bool*)list_get(mensaje,1);
			valor= *(uint32_t*)list_get(mensaje,2);
			sem_post(&memoria_s);
		}
	if(!strcasecmp(msg,"mov_out")){
			error=*(bool*)list_get(mensaje,1);
			sem_post(&memoria_s);
		}
	if(!strcasecmp(msg,"proceso")){

		pthread_mutex_lock(&mutexProceso);
		deserializar_proceso(proceso,mensaje,1);
		pthread_mutex_unlock(&mutexProceso);
		sem_post(&ciclo);
	}
	if(!strcasecmp(msg,"instruccion")){
		char* comando = string_new();
		string_append(&comando,list_get(mensaje,1));

		pthread_mutex_lock(&mutexInstruccion);
		instruccion->comando=string_duplicate(comando);
		pthread_mutex_unlock(&mutexInstruccion);

		pthread_mutex_lock(&mutexProceso);
		int pid = proceso->pid;
		pthread_mutex_unlock(&mutexProceso);

		char * consola = string_from_format("PID: %d Ejecucion: %s",pid,comando);
		for(int i=2; i<list_size(mensaje)-1;i++){

			pthread_mutex_lock(&mutexInstruccion);
			list_add(instruccion->parametros,list_get(mensaje,i));
			pthread_mutex_unlock(&mutexInstruccion);

			string_append_with_format(&consola,"%s ",(char*)list_get(mensaje,i));
		}
		escritura_log(consola);
		free(consola);
		sem_post(&instruccion_s);
	}
	if(!strcasecmp(msg,"interrupcion")){
		int razon = razon_interrupcion((char*)list_get(mensaje,1));
		bool banderaInterrumpir = false;
		switch(razon){
			case DESALOJO:
					uint32_t pid = 0;
					pthread_mutex_lock(&mutexMotivo);
					motivo = ((char*) list_get(mensaje,2));
					pthread_mutex_unlock(&mutexMotivo);
					pid= *(uint32_t*)list_get(mensaje,3);
					pthread_mutex_lock(&mutexProceso);
					if (pid == (proceso->pid)) banderaInterrumpir = true;
					pthread_mutex_unlock(&mutexProceso);
				break;
		}
		if(banderaInterrumpir){
			pthread_mutex_lock(&mutexInterrupcion);
			interrupcion=true;
			pthread_mutex_unlock(&mutexInterrupcion);

		}
	}
	if(!strcasecmp(msg,"respuesta")){
		sem_post(&respuesta);
	}
	free(msg);
}
void contexto_ejecucion(t_list * mensaje){
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"contexto",sizeof("contexto"));
	for(int i=0;i<list_size(mensaje);i++){
		agregar_a_paquete(paquete,list_get(mensaje,i),(strlen((char*)list_get(mensaje,i)))+1);
	}
	int size = (list_size(mensaje))+1;

	pthread_mutex_lock(&mutexProceso);
	PCB* temp = proceso_copy(proceso);
	pthread_mutex_unlock(&mutexProceso);
	serializar_proceso(paquete,temp);
	agregar_a_paquete(paquete,&size,sizeof(size));
	enviar_paquete(paquete,clienteKernelDispatch);
	proceso_destroy(temp);
	eliminar_paquete(paquete);

}

