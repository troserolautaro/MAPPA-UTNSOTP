#include "utilsServidor.h"


int iniciar_servidor(char* puerto)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	socket_servidor = socket(servinfo->ai_family,
	                         servinfo->ai_socktype,
	                         servinfo->ai_protocol);

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	listen(socket_servidor, SOMAXCONN);


	freeaddrinfo(servinfo);
	//log_trace(logger, "Listo para escuchar a mi cliente");
	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{

	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
//	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	printf("Me llego el mensaje %s \n", buffer);
//	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}
void * procesar_tipo(int socket){
	//log_info(logger,"socket %d",socket);
		t_list* lista = malloc(sizeof(t_list));
		while (1) {
			int cod_op = recibir_operacion(socket);
			switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(socket);
				break;
			case PAQUETE:
				lista = recibir_paquete(socket);
			//	log_info(logger, "Me llegaron los siguientes valores:\n");
			//	list_iterate(lista, (void*) iterator);
				return lista;
				break;
			case -1:
				log_error(logger, "Un cliente se desconecto.");
				log_destroy(logger);
				return NULL;
				break;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}
}
void iterator(char* value) {
	log_info(logger,"%s", value);
}

void * manejar_cliente(void* socket){
	int socketint=*((int *) socket);
	do{
		t_list* mensaje = procesar_tipo(socketint);
		if(mensaje!=NULL && !list_is_empty(mensaje)){
			//printf("%d",socketint);
			list_add(mensaje, socket);
			procesar_mensaje(mensaje);
			//list_clean_and_destroy_elements(mensaje,free);
			//liberar mensaje
		}
		if(mensaje==NULL){
			//free(socket);
			break;
		}
		//list_destroy(mensaje);
	}while(true);
	return NULL;
}
void * recibir_conexiones(void * server){
	int serverInt=*((int* )server);
	do{
		int cliente_fd = esperar_cliente(serverInt);
	//	printf("%d",cliente_fd);
		t_list* mensaje = procesar_tipo(cliente_fd);
		if(mensaje!=NULL && !list_is_empty(mensaje)){
			char* msg = malloc(sizeof(char*));
			msg = string_new();
			string_append(&msg,list_get(mensaje,0));
			string_trim(&msg);
			string_to_lower(msg);
			int modulo = *(int*)list_get(mensaje,1);
			if(!strcasecmp(msg,"conexion")){
				char* mensaje = string_from_format("Hola! %d",modulo);
				debug(mensaje);
				free(mensaje);
				int resultado;
				int* cliente_fd_copy = (int*)malloc(sizeof(int));
				*cliente_fd_copy = cliente_fd;
				switch(modulo){
					case KERNEL:;
						pthread_t hiloKernel;
						 if ((resultado=pthread_create(&hiloKernel,NULL,manejar_cliente,( void *)cliente_fd_copy))!=0)
							printf("Error al crear hilo. resultado %d",resultado);

					break;
					case CPUDispatch:;
						pthread_t hiloCPUDispatch;
						if ((resultado=pthread_create(&hiloCPUDispatch,NULL,manejar_cliente,( void *) cliente_fd_copy))!=0)
							printf("Error al crear hilo. resultado %d",resultado);

					break;
					case CPUInterrupt:;
						pthread_t hiloCPUInterrupt;
						if ((resultado=pthread_create(&hiloCPUInterrupt,NULL,manejar_cliente,( void *) cliente_fd_copy))!=0)
												printf("Error al crear hilo. resultado %d",resultado);

					break;
					case MEMORIA:;
						pthread_t hiloMemoria;
						if ((resultado=pthread_create(&hiloMemoria,NULL,manejar_cliente,( void *) cliente_fd_copy))!=0)
												printf("Error al crear hilo. resultado %d",resultado);

					break;
					case FILESYSTEM:;
						pthread_t hiloFileSystem;
						if ((resultado=pthread_create(&hiloFileSystem,NULL,manejar_cliente,( void *) cliente_fd_copy))!=0)
												printf("Error al crear hilo. resultado %d",resultado);

					break;

					default:
						printf("TIPO NO DEFINIDO\n");
					break;
				}
			}
			free(msg);
		}

	}while(true);
}
