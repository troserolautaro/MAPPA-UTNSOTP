#ifndef UTILSCLIENTE_H_
#define UTILSCLIENTE_H_

//include del utils general
#include"utils.h"
//El cliente es un enumerador que identifica quien es (KERNEL,CPU,MEMORIA,FILESYSTEM)
void handshake(int cliente , int socket_cliente);
int crear_conexion(char* ip, char* puerto,int cliente);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void serializar_proceso(t_paquete* paquete,PCB* proceso);
void deserializar_proceso(PCB* proceso, t_list * msg,uint32_t  posInicio);

#endif /* UTILSCLIENTE_H_ */
