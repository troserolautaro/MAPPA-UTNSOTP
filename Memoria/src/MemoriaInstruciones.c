#include "MemoriaInstrucciones.h"
char * pathInstrucciones;
t_list* cargar_instrucciones(char** file){
	FILE * fileInstrucciones;
		t_list *listaInstrucciones;
		listaInstrucciones= list_create();
		char * direccionIns=string_new();
		//une la ruta de los archivos, con el archivo especificado
		string_append(&direccionIns, pathInstrucciones);
		string_append(&direccionIns, "/");
		string_append(&direccionIns,*file);
		//abre el archivo en modo lectura
		fileInstrucciones = fopen(direccionIns, "r");
		char* lineaDeCodigo=calloc(101,sizeof(char));
		if (fileInstrucciones == NULL)
		    {
			 printf("no hay insctrucciones o hubo error con el archivo");
		    }
		 	else
		    {
		 	    printf("\nEl contenido del archivo de prueba es \n");
		 	    while (fgets(lineaDeCodigo,sizeof(lineaDeCodigo),fileInstrucciones )!=NULL)
		 	    {
		 			char* lineaTemporal=calloc(101,sizeof(char));
		 	        string_append(&lineaTemporal, lineaDeCodigo);
		 	    	list_add(listaInstrucciones,lineaTemporal);
		 	    }
		    }
	    fclose(fileInstrucciones);
		    char* primerComando=list_get(listaInstrucciones,0);
		    printf("Línea %d: %s\n", 0, primerComando);
			printf("\n tamaño de lista %d \n", list_size(listaInstrucciones));

			free(lineaDeCodigo);
			free(primerComando);
		    return listaInstrucciones;
}
