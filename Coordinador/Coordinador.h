/*
 * CoordiandorHilos.h
 *
 *  Created on: 27 abr. 2018
 *      Author: utnso
 */

#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <parsi/parser.h>
#include <pthread.h>
#include <semaphore.h>
#include <FuncionesConexiones.h>

extern t_list* instancias;


struct Instancia{
	int estaDisponible; //ESTE VALOR DEFINE SI SE SIGUE MANTENIENDO UNA CONEXION CON EL SERVIDOR
	char* nombreInstancia; //ME VA SERVIR COMO CLAVE PARA LA RECONEXION
	t_list* clavesBloqueadas; //LAS CLAVES QUE SE LE HICIERON GET EN ESTA INSTANCIA
	int* cantEntradasDisponibles; //PARA EL LSU
	int nroSemaforo;
}typedef instancia;
void *conexionESI(void* nuevoCliente);
void *conexionInstancia(void* cliente);
void* conexionPlanificador();
void enviarDatosInstancia(int sockInstancia, char* tipo);
typedef instancia*(*algoritmo)(instancia* instancia,t_list* lista);
instancia* equitativeLoad(instancia* instancia,t_list* lista);
instancia* lsu(instancia* instanciaAUsar,t_list* lista);
void inicializarInstancia(instancia* instanciaNueva,char* nombreInstancia);
instancia* existeEnLaLista(char* id);
instancia* crearInstancia(int sockInstancia,char* nombreInstancia,int* cantidadDeEntradas);
algoritmo obtenerAlgoritmoDistribucion();
instancia* buscarInstancia(char* clave);
instancia* keyExplicit(instancia* instancia,t_list* lista);
void liberarClave(instancia* instancia,char* clave);
#endif /* COORDINADOR_H_ */
