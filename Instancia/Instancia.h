/*
 * Instancia.h
 *
 *  Created on: 16 abr. 2018
 *      Author: utnso
 */

#ifndef SOCKET_INSTANCIA_H_
#define SOCKET_INSTANCIA_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "FuncionesConexiones.h"
#include <parsi/parser.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
void inicializarTablaEntradas(int sockcoordinador);
#endif /* SOCKET_INSTANCIA_H_ */
