/*
 * FuncionesConexiones.h
 *
 *  Created on: 17 abr. 2018
 *      Author: utnso
 */

#ifndef SOCKET_FUNCIONESCONEXIONES_H_
#define SOCKET_FUNCIONESCONEXIONES_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>

typedef struct{
	int a;
	char key[40];
	char *value;
}Paquete;

extern const int SET;
extern const int GET;
extern const int STORE;

//Path de los servidores
extern const char *pathCoordinador;
extern const char *pathPlanificador;

//Los pongo en escritorio para que no tengamos problemas al commitear
extern const char *logCoordinador;
extern const char *logPlanificador;
extern const char *logESI;
extern const char *logConsola;
extern const char *logInstancias;

//Funciones utilizadas en general
struct sockaddr_in dameUnaDireccion(char *path,int ipAutomatica);
int crearConexionCliente(char*path);
//Funciones utilizadas por el Coordinador
int crearConexionServidor(char*path);
int transformarNumero(char *a,int start);
Paquete deserializacion(char* texto);
Paquete recibir(int socket);
//Funciones utilizadas por el ESI
char* transformarTamagnoKey(char* key);
char* serealizarPaquete(Paquete pack);
void enviar(int socket,Paquete pack);

#endif /* SOCKET_FUNCIONESCONEXIONES_H_ */
