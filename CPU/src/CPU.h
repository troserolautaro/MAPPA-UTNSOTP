#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>

//includes de utils globales
#include <utils.h>
#include <utilsServidor.h>
#include <utilsCliente.h>

t_config* iniciar_config(void);
t_log* iniciar_logger(void);
void iterator(char* value);



#endif /* CPU_H_ */

