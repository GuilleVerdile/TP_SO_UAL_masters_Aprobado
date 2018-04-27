/*
 * CoordiandorHilos.h
 *
 *  Created on: 27 abr. 2018
 *      Author: utnso
 */

#ifndef COORDINADORHILOS_H_
#define COORDINADORHILOS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "FuncionesConexiones.h"
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>
void *conexionESI(void* listener);
t_log *logger;
#endif /* COORDINADORHILOS_H_ */
