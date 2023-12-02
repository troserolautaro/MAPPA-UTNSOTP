#include "MemoriaUsuario.h"

pagina_t* (*algoritmoRemplazo)();
void* espacioContiguoMemoria;
char* algoritmoReemplazo;
int tamPagina,tamMemoria,cantMarcos,retardoRespuesta;
int conexionFS;
int marcoFIFO;
pthread_mutex_t mutex_memoria;

//INICIAR  MEMORIA DE USUARIO
marco_t* crear_marco(int i){
	marco_t* nuevoMarco=malloc(sizeof(marco_t*));
	nuevoMarco->libre=true;
	nuevoMarco->base=tamPagina*i;
	nuevoMarco->limite=tamPagina*(i+1);
	return nuevoMarco;
}

pagina_global_t* crear_pagina_global(int i){
	pagina_global_t *paginaGlobal=malloc(sizeof(marco_t*));
	paginaGlobal->pagina=-1;
	paginaGlobal->pid=-1;
	paginaGlobal->accesos=0;
	return paginaGlobal;
}
pagina_t* crear_pagina(){
	pagina_t* nuevaPagina=malloc(sizeof(pagina_t*));
	nuevaPagina->marco=-1;
	nuevaPagina->p=0;
	nuevaPagina->m=0;
	nuevaPagina->posSWAP=0;
	return nuevaPagina;
}
pagina_t * pagina_get(uint32_t pid, uint32_t numPagina){
	t_list* tablaPaginacion =dictionary_get(tablaProcesos,string_itoa(pid));
	return list_get(tablaPaginacion,numPagina);
}

void cargar_tabla_marcos_y_pglobales(){

	tablaMarcos=list_create();
	tablapaginasGlobales=list_create();
	for(int i=0;i<cantMarcos;i++){
		list_add(tablaMarcos,crear_marco(i));
		list_add(tablaMarcos,crear_pagina_global(i));
	}
}

void iniciar_memoria_usuario(){
	//algoritmo de remplazo
	espacioContiguoMemoria=malloc(sizeof(void*) * tamMemoria);
	cantMarcos=tamMemoria/tamPagina;
	marcoFIFO=0;
	tablaProcesos=dictionary_create();
	cargar_tabla_marcos_y_pglobales();
}

//CREAR PROCESO
void crear_proceso(uint32_t pid,char* nombre, uint32_t tamaño){
	//reservar paginas
	int cantPaginasProceso=ceil(tamaño/tamPagina);
	t_list* tablaPaginacion=list_create();
	for(int i=0;i<cantPaginasProceso;i++){
		list_add(tablaPaginacion,crear_pagina());
	}
	dictionary_put(tablaProcesos,string_itoa(pid),tablaPaginacion);
	//reservar swap con fs
	t_paquete* paquete=crear_paquete();
	agregar_a_paquete(paquete,"reservarSWAP",sizeof("reservarSWAP"));
	agregar_a_paquete(paquete,&pid,sizeof(uint32_t*));
	agregar_a_paquete(paquete,&cantPaginasProceso,sizeof(uint32_t*));
	usleep(retardoRespuesta*1000);
	enviar_paquete(paquete,conexionFS);
	eliminar_paquete(paquete);
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
uint32_t leer_espacio_usuario(uint32_t direccion) {
	uint32_t valor;
	pthread_mutex_lock(&mutex_memoria);
	//leer direccion
	//memcpy(&valor,espacioContiguoMemoria[direccion], sizeof(uint32_t));
	pthread_mutex_unlock(&mutex_memoria);
	return valor;
}

//ACCESO A ESPACIO DE USUARIO ESCRITURA
void escribir_espacio_usuario(uint32_t direccion, uint32_t valor) {
	pthread_mutex_lock(&mutex_memoria);
	//escribir direccion
	//memcpy(espacioContiguoMemoria[direccion], &valor, sizeof(uint32_t));
	pthread_mutex_unlock(&mutex_memoria);
}

//PAGE FAULT
void obtener_pagina_swap(uint32_t pid,uint32_t posSWAP){
	t_paquete* paquete=crear_paquete();
		agregar_a_paquete(paquete,"paginaSWAP",sizeof("paginaSWAP"));
		agregar_a_paquete(paquete,&pid,sizeof(int*));
		agregar_a_paquete(paquete,&posSWAP,sizeof(uint32_t*));
		usleep(retardoRespuesta*1000);
		enviar_paquete(paquete,conexionFS);
		eliminar_paquete(paquete);
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
void page_fault(uint32_t pid,uint32_t numPagina){
	pagina_t* pagina=pagina_get(pid,numPagina);
	uint32_t posSWAP=(pagina->posSWAP);
	obtener_pagina_swap(pid,posSWAP);
	if(marcos_libres()){
		pagina_t* paginaVictima=algoritmoRemplazo();
	}
}


//SELECCION DE VICTIMA
//FIFO
pagina_t* fifo(){
	pagina_global_t* paginaVictima=(pagina_global_t*)list_get(tablapaginasGlobales,marcoFIFO);
	return (pagina_t* )pagina_get((paginaVictima->pid),(paginaVictima->pagina));
}
//LRU
void* pagina_mas_reciente(void* pagina1,void* pagina2){
	pagina_global_t * p1=(pagina_global_t *) pagina1;
	pagina_global_t * p2=(pagina_global_t *) pagina2;
	if((p1->accesos)<(p2->accesos))return pagina1;
	return pagina2;
}

pagina_t* lru(){
	pagina_global_t* paginaVictima=(pagina_global_t*)list_get_minimum(tablapaginasGlobales,pagina_mas_reciente);
	return (pagina_t* )pagina_get((paginaVictima->pid),(paginaVictima->pagina));
}

