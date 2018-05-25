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
void manejarPaquete(t_esi_operacion paquete,int sockcoordinador);
void meterClaveALaTabla(char* clave);
void meterValorParTalClave(char clave[40], char*valor);
void* hacerDump();
void almacenarInformacion(t_config* config);
void algoritmoCircular(char clave[40], char*valor,int posicionTablaE,int posicionEntradaDeTabla,int posicionEnEntradas, int cuantoFaltaGuardar) ;
#endif /* SOCKET_INSTANCIA_H_ */
