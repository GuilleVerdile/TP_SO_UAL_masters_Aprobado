/*
 * ESI.h
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#ifndef SOCKET_ESI_H_
#define SOCKET_ESI_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <FuncionesConexiones.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <parsi/parser.h>

 #include <unistd.h>

int esi(char* path,int sockcoordinador,int sockplanificador);


#endif /* SOCKET_ESI_H_ */
