#include "MemoriaUsuario.h"

t_list* tablapaginasGlobales;//indice=marco
t_list* tablaMarcos;//indice=marco
t_dictionary* tablaProcesos;//clave pid
typedef pagina_t* (*AlgoritmoRemplazoFuncion)();
AlgoritmoRemplazoFuncion algoritmoRemplazo;

void* espacioContiguoMemoria; //Simulador de ram => Almacenar una cantidad bytes

char* algoritmoReemplazo;
int tamPagina,tamMemoria,cantMarcos,retardoRespuesta;
int conexionFS;
int marcoFIFO;
pthread_mutex_t mutex_memoria;
sem_t sem_bloquesSwap, sem_paginaSwap, sem_escribirSwap;

//INICIAR  MEMORIA DE USUARIO
marco_t* crear_marco(int i){
	marco_t* nuevoMarco=malloc(sizeof(marco_t));
	nuevoMarco->libre=true;
	nuevoMarco->base=tamPagina * i;//0
	nuevoMarco->limite=tamPagina * (i+1);//32
	return nuevoMarco;
}

pagina_global_t* crear_pagina_global(int i){
	pagina_global_t *paginaGlobal=malloc(sizeof(pagina_global_t));
	paginaGlobal->pagina=-1;
	paginaGlobal->pid=-1;
	paginaGlobal->accesos=0;
	return paginaGlobal;
}
pagina_t* crear_pagina(){
	pagina_t* nuevaPagina=malloc(sizeof(pagina_t));
	nuevaPagina->marco=-1;
	nuevaPagina->p=0;
	nuevaPagina->m=0;
	nuevaPagina->posSWAP=0;
	return nuevaPagina;
}
pagina_t * pagina_get(uint32_t pid, uint32_t numPagina){
	t_list* tablaPaginacion =dictionary_get(tablaProcesos,string_itoa((int)pid));
	return (pagina_t*)list_get(tablaPaginacion,numPagina);
}

void cargar_tabla_marcos_y_pglobales(){

	tablaMarcos=list_create();
	tablapaginasGlobales=list_create();
	for(int i=0;i<cantMarcos;i++){
		list_add(tablaMarcos,crear_marco(i));
		list_add(tablapaginasGlobales,crear_pagina_global(i));
	}
}

//CREAR PROCESO
void crear_proceso(uint32_t pid,char* nombre, uint32_t tamaño){
	//reservar paginas
	int cantPaginasProceso=ceil(tamaño/tamPagina);
	t_list* tablaPaginacion=list_create();
	for(int i=0;i<cantPaginasProceso;i++){
		pagina_t * nuevaPagina = crear_pagina();
		list_add(tablaPaginacion,nuevaPagina);
	//	debug(string_from_format("Pagina Nro : %d",i));
	}
	//if(list_size(tablaPaginacion) != cantPaginasProceso ) debug("No hay cantidad paginas cargadas necesarias");
	dictionary_put(tablaProcesos,string_itoa(pid),tablaPaginacion);
	escritura_log(string_from_format("Creacion Tabla Paginas PID: %d - Tamaño: %d",pid,tamaño));
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
void destruir_pagina(void* pagina){
	pagina_t * pag=(pagina_t*) pagina;
	marco_t* marcoALiberar= list_get(tablaMarcos,(pag->marco));
	marcoALiberar->libre=true;
	free(pagina);
}

void destruir_tabla_paginas(void* tablaPaginacion){
	for(int i=0;i>list_size(tablaPaginacion);i++){
	list_remove_and_destroy_element(tablaPaginacion,i,destruir_pagina);
	}
	free(tablaPaginacion);
}
void finalizar_proceso(uint32_t pid){
	dictionary_remove_and_destroy(tablaProcesos,string_itoa(pid), destruir_tabla_paginas);
}


//ACCESO A TABLA DE PAGINAS PARA DEVOLVER MARCO DE LA PAGINA
uint32_t devolver_num_marco(uint32_t pid, uint32_t numPagina){
	pagina_t* pagina=pagina_get(pid,numPagina);
	if((pagina->p)==1)return (pagina->marco);
	return -1;
}

//ACCESO A ESPACIO DE USUARIO LECTURA
uint32_t get_dato(uint32_t direccion) {
	uint32_t valor;
	pthread_mutex_lock(&mutex_memoria);
	//leer direccion
	memcpy(&valor,(uint32_t*)((char*)espacioContiguoMemoria + direccion),sizeof(uint32_t));
//	memcpy(&valor,espacioContiguoMemoria[direccion], sizeof(uint32_t));
	pthread_mutex_unlock(&mutex_memoria);
	return valor;
}

//ACCESO A ESPACIO DE USUARIO ESCRITURA
void set_dato(uint32_t direccion, uint32_t valor) {
	pthread_mutex_lock(&mutex_memoria);
	memcpy((void*)((char*)espacioContiguoMemoria + direccion),(void*)&valor,sizeof(uint32_t));
	pthread_mutex_unlock(&mutex_memoria);
	//MARCO MODIFICADO
	pagina_global_t* paginaGlobal = (list_get(tablapaginasGlobales,direccion/tamPagina));
	pagina_t* pagina = pagina_get(paginaGlobal->pid,paginaGlobal->pagina);
	pagina->m = 1;
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
	if((m->libre))(*c)++;
	return (void*)c;
}
int marcos_libres(){
	int contador=0;
	return *((int*)list_fold (tablaMarcos,(void*)&contador,contar_marcosLibres));
}
bool marco_libre(marco_t * marco){return (marco->libre);}

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
	uint32_t posSWAP=(pagina->posSWAP);
	marco_t * marcoLibre;
	bool victima = false;
	char * mensaje = string_new();
	if((marcos_libres())==0){
		pagina_t* paginaVictima=algoritmoRemplazo();
		marcoLibre = list_get(tablaMarcos,paginaVictima->marco);
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
		}
		paginaVictima->p = 0;
		paginaVictima->m = 0;
		string_append_with_format(&mensaje,"REEMPLAZO - Marco: %d - ",paginaVictima->marco);
		victima = true;
	}else{
		marcoLibre = list_find(tablaMarcos,(void*)marco_libre);
	}
	marcoLibre->libre = false;
	pagina->marco = marcoLibre->base / tamPagina;
	pagina->p = 1;
	pagina_global_t* paginaGlobal = (pagina_global_t*) list_get(tablapaginasGlobales,pagina->marco);
	if (victima){
		string_append_with_format(&mensaje,"Page Out: %d-%d - Page In: %d-%d",paginaGlobal->pid,paginaGlobal->pagina,pid,numPagina);
		escritura_log(mensaje);
	}
	free(mensaje);
	paginaGlobal->pid = pid;
	paginaGlobal->pagina = numPagina;
	paginaGlobal->accesos = 0; // No sabia que hacer ?
	obtener_pagina_swap(pid,posSWAP,numPagina);
	escritura_log(string_from_format("SWAP IN -  PID: %d - Marco: %d - Page In: %d-%d",pid,pagina->marco,pid,numPagina));

}

void asignar_swap(t_list* args){
	uint32_t pid = *(uint32_t*)list_get(args,1);
	uint32_t cantBloques = *(uint32_t*)list_get(args,2);
	//t_list* paginas =diccionario_get(tablaProcesos,string_itoa((int) pid));
	pagina_t* temp;
	for(int i=3; i < cantBloques+3 ; i++){
		temp = pagina_get(pid,i-3);
		temp->posSWAP = *(uint32_t*) list_get(args,i);

	}
	sem_post(&sem_bloquesSwap);
}

//SELECCION DE VICTIMA
//FIFO
pagina_t* FIFO(){
	pagina_global_t* paginaVictima=(pagina_global_t*)list_get(tablapaginasGlobales,marcoFIFO);
	if(marcoFIFO == cantMarcos-1){
		marcoFIFO = 0;
	}else{
		marcoFIFO++;
	}

	return (pagina_t* )pagina_get((paginaVictima->pid),(paginaVictima->pagina));
}
//LRU
void* pagina_mas_reciente(void* pagina1,void* pagina2){
	pagina_global_t * p1=(pagina_global_t *) pagina1;
	pagina_global_t * p2=(pagina_global_t *) pagina2;
	if((p1->accesos)<(p2->accesos))return pagina1;
	return pagina2;
}

pagina_t* LRU(){
	pagina_global_t* paginaVictima=(pagina_global_t*)list_get_minimum(tablapaginasGlobales,pagina_mas_reciente);
	return (pagina_t* )pagina_get((paginaVictima->pid),(paginaVictima->pagina));
}
void iniciar_memoria_usuario(){
	//algoritmo de remplazo
	if(!strcasecmp(algoritmoReemplazo,"fifo")){
		algoritmoRemplazo = FIFO;
	}else{
		algoritmoRemplazo = LRU;
	}
	espacioContiguoMemoria=malloc(tamMemoria);
	cantMarcos=tamMemoria/tamPagina;
	marcoFIFO=0;
	tablaProcesos=dictionary_create();
	cargar_tabla_marcos_y_pglobales();
}
void cargar_pagina_swap(uint32_t pid, uint32_t numPagina, void* datos){
	//Carga un bloque entero en base a uint32_t
	marco_t* marco = (marco_t*) list_get(tablaMarcos,devolver_num_marco(pid,numPagina));
	for(int i = 0; i < tamPagina/sizeof(uint32_t);i++){
		set_dato((marco->base)+(i*sizeof(uint32_t)),*((uint32_t*)((char*)datos + i * sizeof(uint32_t))));
		escritura_log(string_from_format("PID: %d - Accion: ESCRIBIR - Direccion fisica: %d",pid,(int)(marco->base+i*sizeof(uint32_t))));
	}
	free(datos);
	sem_post(&sem_paginaSwap);
}

