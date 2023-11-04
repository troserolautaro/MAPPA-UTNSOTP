#include"PlanificadorLargo.h"

void planificador_largo(){
	while(gradoMultiprogramacion>queue_size(colaCorto) /*CAMBIAR*/ && !detenida && !queue_is_empty(colaLargo)){
		PCB* proceso=queue_pop(colaLargo);
		proceso->estado=READY;
		queue_push(colaCorto,proceso);
	}
	//semaforo a corto
	//planificador_corto();
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
