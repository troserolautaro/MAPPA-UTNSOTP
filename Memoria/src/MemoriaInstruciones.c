#include "MemoriaInstrucciones.h"
char * pathInstrucciones;
t_list* cargar_instrucciones(char** file){
	FILE * fileInstrucciones;
		t_list *listaInstrucciones = list_create();
		char * direccionIns=string_new();
		//une la ruta de los archivos, con el archivo especificado
		//cambiar el archivo por variable a futuro, no prioritario de momento
		string_append(&direccionIns, pathInstrucciones);
		string_append(&direccionIns, "/");
		string_append(&direccionIns,*file);
		//abre el archivo en modo lectura
		fileInstrucciones = fopen(direccionIns, "r");
		if (fileInstrucciones == NULL){
			 error_show("no hay insctrucciones o hubo error con el archivo");
		    }else{
			char* lineaDeCodigo=NULL;
			size_t lineLength = 0;
			while (getline(&lineaDeCodigo, &lineLength, fileInstrucciones) != -1) {
				if (lineaDeCodigo[strlen(lineaDeCodigo) - 1] == '\n') {
					lineaDeCodigo[strlen(lineaDeCodigo) - 1] = '\0';
				}
				char* lineaTemporal = strdup(lineaDeCodigo);
				list_add(listaInstrucciones, lineaTemporal);
			}
				free(lineaDeCodigo);
			}
			fclose(fileInstrucciones);
			return listaInstrucciones;
}
