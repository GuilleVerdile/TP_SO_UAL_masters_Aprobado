/*
 * Coordinador.h
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#ifndef SOCKET_COORDINADOR_H_
#define SOCKET_COORDINADOR_H_

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

int coordinador();

#endif /* SOCKET_COORDINADOR_H_ */
