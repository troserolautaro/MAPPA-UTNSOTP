#include "utilsCliente.h"


void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto, int cliente)
{
	struct addrinfo hints;
	struct addrinfo *server_info = malloc(sizeof(struct addrinfo));

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.

	int socket_cliente = socket(server_info->ai_family,
	                    server_info->ai_socktype,
	                    server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	//continuando sobre cuando creamos el socket del cliente
	int reuse = 1;
	if (setsockopt(socket_cliente, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
	    perror("setsockopt");
	    // Handle error
	}

	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);


	freeaddrinfo(server_info);

	handshake(cliente,socket_cliente);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
void handshake(int cliente, int socket_cliente){
	t_paquete * temp = crear_paquete();
	agregar_a_paquete(temp,"conexion",sizeof("conexion"));
	agregar_a_paquete(temp,&cliente,sizeof(cliente));
	agregar_a_paquete(temp,&socket_cliente,sizeof(socket_cliente));
	enviar_paquete(temp,socket_cliente);
	eliminar_paquete(temp);
}
void serializar_proceso(t_paquete* paquete,PCB* proceso){
	//PCB* temp = *proceso;
	agregar_a_paquete(paquete,&proceso->pid,sizeof(uint32_t));
	agregar_a_paquete(paquete,&proceso->pc,sizeof(uint32_t));
	agregar_a_paquete(paquete,&(proceso->registros->AX),sizeof(uint32_t));
	agregar_a_paquete(paquete,&(proceso->registros->BX),sizeof(uint32_t));
	agregar_a_paquete(paquete,&(proceso->registros->CX),sizeof(uint32_t));
	agregar_a_paquete(paquete,&(proceso->registros->DX),sizeof(uint32_t));
	//free(temp);
}
void deserializar_proceso(PCB* proceso, t_list * msg,uint32_t  posInicio){
	proceso->pid = *(uint32_t*)(list_get(msg,posInicio));
	proceso->pc = *(uint32_t*)list_get(msg,posInicio+1);
	proceso->registros->AX = *(uint32_t*)list_get(msg,posInicio+2);
	proceso->registros->BX = *(uint32_t*)list_get(msg,posInicio+3);
	proceso->registros->CX = *(uint32_t*)list_get(msg,posInicio+4);
	proceso->registros->DX = *(uint32_t*)list_get(msg,posInicio+5);
}
