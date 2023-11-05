#ifndef MEMORIAINSTRUCCIONES_H_
#define MEMORIAINSTRUCCIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>

#include <pthread.h>

extern char * pathInstrucciones;
t_list* cargar_instrucciones(char** file);

#endif /* MEMORIAINSTRUCCIONES_H_ */
