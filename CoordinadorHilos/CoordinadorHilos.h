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
#include <commons/collections/list.h>
#include <pthread.h>
void *conexionESI(void* listener);
extern t_list* instancias;

enum algoritmos {EL, KE , LSU}; //PARA LOS ALGORITMOS DE DISTRIBUCION

struct Instancia{
	int estaDisponible; //ESTE VALOR DEFINE SI SE SIGUE MANTENIENDO UNA CONEXION CON EL SERVIDOR
	char* nombreInstancia; //ME VA SERVIR COMO CLAVE PARA LA RECONEXION
	int socket; //EL SOCKET VARIA CON LAS RECONEXIONES
	char** clavesBloqueadas; //LAS CLAVES QUE SE LE HICIERON GET EN ESTA INSTANCIA
	int cantEntradasDisponibles; //PARA EL LSU
}typedef instancia;

instancia* algoritmoDeDistribucion(instancia* instanciaNueva);
instancia* equitativeLoad(instancia* instancia);
void inicializarInstancia(instancia* instanciaNueva,int sockInstancia,char* nombreInstancia);
instancia* existeEnLaLista(char* id);
instancia* crearInstancia(int sockInstancia,char* nombreInstancia);
#endif /* COORDINADORHILOS_H_ */
