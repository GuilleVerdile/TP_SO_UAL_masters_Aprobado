/*
 * FuncionesConexiones.h
 *
 *  Created on: 17 abr. 2018
 *      Author: utnso
 */

#ifndef SOCKET_FUNCIONESCONEXIONES_H_
#define SOCKET_FUNCIONESCONEXIONES_H_

struct sockaddr_in dameUnaDireccion(char *path,int ipAutomatica);
int crearConexionCliente(char*path);

#endif /* SOCKET_FUNCIONESCONEXIONES_H_ */
