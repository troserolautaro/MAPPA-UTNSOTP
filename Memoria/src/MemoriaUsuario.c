#include "MemoriaUsuario.h"

t_list* tablapaginasGlobales;//indice=marco, mutexInversa
t_list* tablaMarcos;//indice=marco,mutexTablaMarcos

t_dictionary* tablaProcesos;//clave pid,mutexTablasPagina

typedef pagina_t* (*AlgoritmoRemplazoFuncion)(char** mensaje);
AlgoritmoRemplazoFuncion algoritmoRemplazo;

void* espacioContiguoMemoria; //mutexRAM

char* algoritmoReemplazo;
int tamPagina,tamMemoria,cantMarcos,retardoRespuesta;
int conexionFS;
int marcoFIFO;
pthread_mutex_t mutexMemoria,mutexTablasPagina, mutexTablaMarcos, mutexInversa;
sem_t sem_bloquesSwap, sem_paginaSwap, sem_escribirSwap,sem_escribirBloque,sem_swapLiberado;

//INICIAR  MEMORIA DE USUARIO
pthread_mutex_t* crear_mutex(){
	pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex,NULL);
	return mutex;
}

marco_t* crear_marco(int i){
	marco_t* nuevoMarco=malloc(sizeof(marco_t));
	nuevoMarco->libre=true;
	nuevoMarco->base=tamPagina * i;//0
	nuevoMarco->limite=tamPagina * (i+1);//32
	nuevoMarco->mutexMarco = crear_mutex();
	return nuevoMarco;
}

pagina_global_t* crear_pagina_global(int i){
	pagina_global_t *paginaGlobal=malloc(sizeof(pagina_global_t));
	paginaGlobal->pagina=-1;
	paginaGlobal->pid=-1;
	paginaGlobal->tiempo = temporal_create();
	paginaGlobal->mutexGlobal = crear_mutex();
	return paginaGlobal;
}

pagina_t* crear_pagina(){
	pagina_t* nuevaPagina=malloc(sizeof(pagina_t));
	nuevaPagina->marco=-1;
	nuevaPagina->p=0;
	nuevaPagina->m=0;
	nuevaPagina->posSWAP=0;
	nuevaPagina->mutexPagina = crear_mutex();
	return nuevaPagina;
}
pagina_t * pagina_get(uint32_t pid, uint32_t numPagina){
	pthread_mutex_lock(&mutexTablasPagina);
	t_list* tablaPaginacion =dictionary_get(tablaProcesos,string_itoa((int)pid));
	pagina_t* paginaObtenida = (pagina_t*)list_get(tablaPaginacion,numPagina);
	pthread_mutex_unlock(&mutexTablasPagina);
	return paginaObtenida ;
}

void cargar_tabla_marcos_y_pglobales(){
	pthread_mutex_lock(&mutexTablaMarcos);
	tablaMarcos=list_create();
	pthread_mutex_unlock(&mutexTablaMarcos);

	pthread_mutex_lock(&mutexInversa);
	tablapaginasGlobales=list_create();
	pthread_mutex_unlock(&mutexInversa);


	for(int i=0;i<cantMarcos;i++){

		list_add(tablaMarcos,crear_marco(i));
		list_add(tablapaginasGlobales,crear_pagina_global(i));

	}
}

//CREAR PROCESO
void crear_proceso(uint32_t pid,char* nombre, uint32_t tamanio){
	//reservar paginas
	int cantPaginasProceso=ceil(tamanio/tamPagina);
	t_list* tablaPaginacion=list_create();
	for(int i=0;i<cantPaginasProceso;i++){
		pagina_t * nuevaPagina = crear_pagina();
		list_add(tablaPaginacion,nuevaPagina);
	//	debug(string_from_format("Pagina Nro : %d",i));
	}
	//if(list_size(tablaPaginacion) != cantPaginasProceso ) debug("No hay cantidad paginas cargadas necesarias");
	pthread_mutex_lock(&mutexTablasPagina);
	dictionary_put(tablaProcesos,string_itoa(pid),tablaPaginacion);
	pthread_mutex_unlock(&mutexTablasPagina);
	char* mensaje= string_from_format("Creacion Tabla Paginas PID: %d - Tamaño: %d",pid,cantPaginasProceso);
	escritura_log(mensaje);
	free(mensaje);
	//reservar swap con fs
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"reservarSWAP",sizeof("reservarSWAP"));
	agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
	agregar_a_paquete(paquete,&cantPaginasProceso,sizeof(int));
	enviar_paquete(paquete,conexionFS);
	eliminar_paquete(paquete);
	sem_wait(&sem_bloquesSwap);
	//asignar la posSWAP en cada pagina segun lo que responda en procesar mensaje con el pid
}

//FINALIZAR PROCESO
bool marco_libre(marco_t * marco){
	//if(marco->libre) debug("Estoy libre");
	return (marco->libre);
}
void destruir_pagina(pagina_t* pagina){
	if(pagina->p == 1){
		pthread_mutex_lock(&mutexTablaMarcos);
		marco_t* marcoALiberar = list_get(tablaMarcos,(pagina->marco));
		pthread_mutex_unlock(&mutexTablaMarcos);

		pthread_mutex_lock(marcoALiberar->mutexMarco);
		marcoALiberar->libre = true;
		pthread_mutex_unlock(marcoALiberar->mutexMarco);

		//list_iterate(tablaMarcos,(void*)marco_libre);

	}
	//pthread_mutex_destroy(pagina->mutexPagina);
	free(pagina->mutexPagina);
	free(pagina);
}

void destruir_tabla_paginas(t_list* tablaPaginacion){
	t_paquete* paquete = crear_paquete();
	agregar_a_paquete(paquete,"liberar_swap",sizeof("liberar_swap"));
	void agregar_swap(pagina_t* pagina){
		agregar_a_paquete(paquete,&(pagina->posSWAP),sizeof(uint32_t));
	}
	list_iterate(tablaPaginacion,(void*)agregar_swap);
	enviar_paquete(paquete,conexionFS);
	eliminar_paquete(paquete);
	list_destroy_and_destroy_elements(tablaPaginacion,(void*)destruir_pagina);
}
void finalizar_proceso(uint32_t pid){
	pthread_mutex_lock(&mutexTablasPagina);
	if(dictionary_has_key(tablaProcesos,string_itoa(pid))){
		pthread_mutex_unlock(&mutexTablasPagina);

		pthread_mutex_lock(&mutexTablasPagina);
		t_list* lista = dictionary_remove(tablaProcesos,string_itoa(pid));
		pthread_mutex_unlock(&mutexTablasPagina);
		//talvez mutex
		char* mensaje= string_from_format("Destruccion Tabla Paginas PID: %d - Tamaño: %d",pid,list_size(lista));
		escritura_log(mensaje);
		free(mensaje);
		destruir_tabla_paginas(lista);
		sem_wait(&sem_swapLiberado);
	}else{
		pthread_mutex_unlock(&mutexTablasPagina);
	}

}


//ACCESO A TABLA DE PAGINAS PARA DEVOLVER MARCO DE LA PAGINA
uint32_t devolver_num_marco(uint32_t pid, uint32_t numPagina){
	pagina_t* pagina=pagina_get(pid,numPagina);
	pthread_mutex_lock(pagina->mutexPagina);
	uint32_t presencia = pagina->p;
	uint32_t marco = pagina->marco;
	pthread_mutex_unlock(pagina->mutexPagina);
	if(presencia==1)return marco;
	return -1;
}

//ACCESO A ESPACIO DE USUARIO LECTURA
uint32_t get_dato(uint32_t direccion) {
	uint32_t valor;
	pthread_mutex_lock(&mutexMemoria);
	//leer direccion
	memcpy(&valor,(uint32_t*)((char*)espacioContiguoMemoria + direccion),sizeof(uint32_t));
	pthread_mutex_unlock(&mutexMemoria);
	return valor;
}

//ACCESO A ESPACIO DE USUARIO ESCRITURA
void set_dato(uint32_t direccion, uint32_t valor) {
	//debug(string_itoa(direccion));
	pthread_mutex_lock(&mutexMemoria);
	memcpy((void*)(((char*)espacioContiguoMemoria) + direccion),(void*)&valor,sizeof(uint32_t));
	pthread_mutex_unlock(&mutexMemoria);
	//MARCO MODIFICADO
	pthread_mutex_lock(&mutexInversa);
	pagina_global_t* paginaGlobal = (list_get(tablapaginasGlobales,direccion/tamPagina));
	pthread_mutex_unlock(&mutexInversa);
	pthread_mutex_lock(paginaGlobal->mutexGlobal);
	pagina_t* pagina = pagina_get(paginaGlobal->pid,paginaGlobal->pagina);
	pthread_mutex_unlock(paginaGlobal->mutexGlobal);
	pthread_mutex_lock(pagina->mutexPagina);
	pagina->m = 1;
	pthread_mutex_unlock(pagina->mutexPagina);
}

//PAGE FAULT
void obtener_pagina_swap(uint32_t pid,uint32_t posSWAP,uint32_t pagina){
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"paginaSWAP",sizeof("paginaSWAP"));
	agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
	agregar_a_paquete(paquete,&posSWAP,sizeof(uint32_t));
	agregar_a_paquete(paquete,&pagina,sizeof(uint32_t));
	//	usleep(retardoRespuesta*1000);
	enviar_paquete(paquete,conexionFS);
	eliminar_paquete(paquete);
	sem_wait(&sem_paginaSwap);
}
void* contar_marcosLibres(void* contador,void* marco ){

	marco_t* m=(marco_t*)marco;

	int* c=(int *)contador;
	pthread_mutex_lock(m->mutexMarco);
	if((m->libre))(*c)++;
	pthread_mutex_unlock(m->mutexMarco);
	return (void*)c;
}

int marcos_libres(){
	int contador=0;
	pthread_mutex_lock(&mutexTablaMarcos);
	int retorno = *((int*)list_fold (tablaMarcos,(void*)&contador,contar_marcosLibres));
	pthread_mutex_unlock(&mutexTablaMarcos);
	return retorno;
}


void* descargar_pagina_swap(int base){
	void * datos = malloc(tamPagina);
	for(int i = 0; i < tamPagina/sizeof(uint32_t);i++){
		*((uint32_t*)((char*)datos + i * sizeof(uint32_t))) = get_dato((base)+(i*sizeof(uint32_t)));
		//escritura_log(string_from_format("PID: %d - Accion: Leer - Direccion fisica: %d",pid,(int)(marco->base+i*sizeof(uint32_t))));
	}
	return datos;
}
void page_fault(uint32_t pid,uint32_t numPagina){
	pagina_t* pagina=pagina_get(pid,numPagina);
	pthread_mutex_lock(pagina->mutexPagina);
	uint32_t posSWAP=(pagina->posSWAP);
	pthread_mutex_unlock(pagina->mutexPagina);
	marco_t * marcoLibre;
	bool victima = false;
	char * mensaje = string_new();
	if((marcos_libres())==0){
		char* mensaje2 = string_new();
		pagina_t* paginaVictima=algoritmoRemplazo(&mensaje2);

		pthread_mutex_lock(&mutexTablaMarcos);
		marcoLibre = list_get(tablaMarcos,paginaVictima->marco);
		pthread_mutex_unlock(&mutexTablaMarcos);

		pthread_mutex_lock(paginaVictima->mutexPagina);
		if(paginaVictima->m!=0){
			void* datos = descargar_pagina_swap(marcoLibre->base);
			//Guardar en bloques SWAP y luego continuar
			t_paquete* paquete = crear_paquete();
			agregar_a_paquete(paquete,"escribirSwap",sizeof("escribirSwap"));
			agregar_a_paquete(paquete,&paginaVictima->posSWAP,sizeof(uint32_t));
			agregar_a_paquete(paquete,datos,tamPagina);
			enviar_paquete(paquete,conexionFS);
			eliminar_paquete(paquete);
			sem_wait(&sem_escribirSwap);
			escritura_log(mensaje2);
			free(mensaje2);
		}
		paginaVictima->p = 0;
		paginaVictima->m = 0;
		string_append_with_format(&mensaje,"REEMPLAZO - Marco: %d - ",paginaVictima->marco);
		pthread_mutex_unlock(paginaVictima->mutexPagina);

		victima = true;
	}else{
		pthread_mutex_lock(&mutexTablaMarcos);
		marcoLibre = list_find(tablaMarcos,(void*)marco_libre);
		pthread_mutex_unlock(&mutexTablaMarcos);
	}

	pthread_mutex_lock(marcoLibre->mutexMarco);
	marcoLibre->libre = false;
	uint32_t base = marcoLibre->base;
	pthread_mutex_unlock(marcoLibre->mutexMarco);

	pthread_mutex_lock(pagina->mutexPagina);
	pagina->marco = base / tamPagina;
	pagina->p = 1;
	pthread_mutex_unlock(pagina->mutexPagina);

	pthread_mutex_lock(&mutexInversa);
	pagina_global_t* paginaGlobal = (pagina_global_t*) list_get(tablapaginasGlobales,pagina->marco);
	pthread_mutex_unlock(&mutexInversa);
	if (victima){
		string_append_with_format(&mensaje,"Page Out: %d-%d - Page In: %d-%d",paginaGlobal->pid,paginaGlobal->pagina,pid,numPagina);
		escritura_log(mensaje);
	}
	free(mensaje);

	pthread_mutex_lock(paginaGlobal->mutexGlobal);
	paginaGlobal->pid = pid;
	paginaGlobal->pagina = numPagina;
	temporal_destroy(paginaGlobal->tiempo);
	paginaGlobal->tiempo = temporal_create(); // No sabia que hacer ?
	pthread_mutex_unlock(paginaGlobal->mutexGlobal);

	obtener_pagina_swap(pid,posSWAP,numPagina);
	char* mensaje2 = string_from_format("SWAP IN -  PID: %d - Marco: %d - Page In: %d-%d",pid,pagina->marco,pid,numPagina);
	escritura_log(mensaje2);
	free(mensaje2);

}

void asignar_swap(t_list* args){
	uint32_t pid = *(uint32_t*)list_get(args,1);
	uint32_t cantBloques = *(uint32_t*)list_get(args,2);
	//t_list* paginas =diccionario_get(tablaProcesos,string_itoa((int) pid));
	pagina_t* temp;
	for(int i=3; i < cantBloques+3 ; i++){
		temp = pagina_get(pid,i-3);
		pthread_mutex_t* mutex = temp->mutexPagina;
		pthread_mutex_lock(mutex);
		temp->posSWAP = *(uint32_t*) list_get(args,i);
		pthread_mutex_unlock(mutex);

	}
	sem_post(&sem_bloquesSwap);
}

//SELECCION DE VICTIMA
//FIFO
pagina_t* FIFO(char** mensaje){
	pthread_mutex_lock(&mutexInversa);
	pagina_global_t* paginaVictima=(pagina_global_t*)list_get(tablapaginasGlobales,marcoFIFO);
	pthread_mutex_unlock(&mutexInversa);
	if(marcoFIFO == cantMarcos-1){
		marcoFIFO = 0;
	}else{
		marcoFIFO++;
	}
	pthread_mutex_lock(paginaVictima->mutexGlobal);
	pagina_t* paginaVict=pagina_get((paginaVictima->pid),(paginaVictima->pagina));
	pthread_mutex_unlock(paginaVictima->mutexGlobal);
	string_append_with_format(mensaje,"SWAP OUT -  PID: %d - Marco: %d - Page Out: %d-%d",paginaVictima->pid,paginaVict->marco,paginaVictima->pid,paginaVictima->pagina);
	return paginaVict;
}
//LRU
void* pagina_mas_reciente(void* pagina1,void* pagina2){
	pagina_global_t * p1=(pagina_global_t *) pagina1;
	pagina_global_t * p2=(pagina_global_t *) pagina2;
	int64_t diferencia = temporal_diff(p1->tiempo,p2->tiempo);
	if(diferencia > -1)return pagina1;
	return pagina2;
}
void frenar_tiempo(pagina_global_t* paginaGlobal){
	temporal_stop(paginaGlobal->tiempo);
}
void reanudar_tiempo(pagina_global_t* paginaGlobal){
	temporal_resume(paginaGlobal->tiempo);
}
pagina_t* LRU(char** mensaje){
	pthread_mutex_lock(&mutexInversa);
	list_iterate(tablapaginasGlobales,(void*)frenar_tiempo);
	pagina_global_t* paginaVictima=(pagina_global_t*)list_get_minimum(tablapaginasGlobales,pagina_mas_reciente);
	list_iterate(tablapaginasGlobales,(void*)reanudar_tiempo);
	pthread_mutex_unlock(&mutexInversa);
	pthread_mutex_lock(paginaVictima->mutexGlobal);
	pagina_t* paginaVict = pagina_get((paginaVictima->pid),(paginaVictima->pagina));
	pthread_mutex_unlock(paginaVictima->mutexGlobal);
	string_append_with_format(mensaje,"SWAP OUT -  PID: %d - Marco: %d - Page In: %d-%d",paginaVictima->pid,paginaVict->marco,paginaVictima->pid,paginaVictima->pagina);
	return paginaVict;
}
void iniciar_memoria_usuario(){
	//algoritmo de remplazo pthread_mutex_t mutexMemoria,mutexTablasPagina, mutexTablaMarcos, mutexInversa,mutexRAM;
	pthread_mutex_init(&mutexTablasPagina,NULL);
	pthread_mutex_init(&mutexTablaMarcos,NULL);
	pthread_mutex_init(&mutexInversa,NULL);
	pthread_mutex_init(&mutexMemoria,NULL);
	sem_init(&sem_swapLiberado,0,0);
	if(!strcasecmp(algoritmoReemplazo,"fifo")){
		algoritmoRemplazo = FIFO;
	}else{
		algoritmoRemplazo = LRU;
	}
	espacioContiguoMemoria=malloc(tamMemoria);
	cantMarcos=tamMemoria/tamPagina;
	marcoFIFO=0;

	pthread_mutex_lock(&mutexTablasPagina);
	tablaProcesos=dictionary_create();
	pthread_mutex_unlock(&mutexTablasPagina);
	cargar_tabla_marcos_y_pglobales();
}
void cargar_pagina_swap(uint32_t pid, uint32_t numPagina, void* datos){
	//Carga un bloque entero en base a uint32_t
	pthread_mutex_lock(&mutexTablaMarcos);
	marco_t* marco = (marco_t*) list_get(tablaMarcos,devolver_num_marco(pid,numPagina));
	pthread_mutex_unlock(&mutexTablaMarcos);

	for(int i = 0; i < tamPagina/sizeof(uint32_t);i++){
		pthread_mutex_lock(marco->mutexMarco);
		set_dato((marco->base)+(i*sizeof(uint32_t)),*((uint32_t*)((char*)datos + i * sizeof(uint32_t))));
		escritura_log(string_from_format("PID: %d - Accion: ESCRIBIR - Direccion fisica: %d",pid,(int)(marco->base+i*sizeof(uint32_t))));
		pthread_mutex_unlock(marco->mutexMarco);
	}
	free(datos);
	sem_post(&sem_paginaSwap);
}


void enviar_datos_bloque(uint32_t direccionFisica){
	void* datos =descargar_pagina_swap(direccionFisica);//me sirve para fat
	t_paquete* paquete = crear_paquete();
	agregar_a_paquete(paquete,"datos_memoria",sizeof("datos_memoria"));
	//agregar_a_paquete(paquete,&paginaVictima->posSWAP,sizeof(uint32_t));
	agregar_a_paquete(paquete,datos,tamPagina);
	enviar_paquete(paquete,conexionFS);
	eliminar_paquete(paquete);
}
void recibir_datos_bloque(uint32_t direccionFisica, void* datos){
	//Carga un bloque entero en base a uint32_t
	for(int i = 0; i < tamPagina/sizeof(uint32_t);i++){
		set_dato(direccionFisica+(i*sizeof(uint32_t)),*((uint32_t*)((char*)datos + i * sizeof(uint32_t))));
		escritura_log(string_from_format("PID: %d - Accion: ESCRIBIR - Direccion fisica: %d",0,(int)(direccionFisica+i*sizeof(uint32_t))));
	}
	free(datos);

}
