/*
 * FuncionesConexiones.c
 *
 *  Created on: 17 abr. 2018
 *      Author: utnso
 */
#include <commons/config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>


struct sockaddr_in dameUnaDireccion(char *path,int ipAutomatica){
	t_config *config=config_create(path);
	struct sockaddr_in midireccion;
	midireccion.sin_family = AF_INET;
	midireccion.sin_port = htons(config_get_int_value(config, "Puerto"));
	if(ipAutomatica){
		midireccion.sin_addr.s_addr = INADDR_ANY;
	}
	else{
		midireccion.sin_addr.s_addr = inet_addr(config_get_string_value(config,"Ip"));
	}
	return midireccion;
}
// estas Funciones retornan -1 en caso de error hay que ver despues como manejarlas
int crearConexionCliente(char*path){//retorna el descriptor de fichero
	int sock=socket(AF_INET, SOCK_STREAM, 0);
	if(sock<0){
	return -1;// CASO DE ERROR
	}
	struct sockaddr_in midireccion=dameUnaDireccion(path,1);
	memset(&midireccion.sin_zero, '\0', 8);
	if(connect(sock, (struct sockaddr *)&midireccion, sizeof(struct sockaddr))<0){
		return -1;
	}
	return sock;
}
int crearConexionServidor(char*path){//Retorna el sock del Servidor
	int sockYoServidor=socket(AF_INET, SOCK_STREAM, 0);// no lo retorno me sierve para la creacion del servidor
	if(sockYoServidor<0){
		printf("Error, no se pudo crear el socket\n");
		return -1;// CASO DE ERROR
	}
	struct sockaddr_in miDireccion=dameUnaDireccion(path,0);
	memset(&miDireccion.sin_zero, '\0', 8);
	struct sockaddr_in their_addr;//Aca queda almacenada la informacion del cliente
	int activado = 1;
    if(setsockopt(sockYoServidor, SOL_SOCKET,SO_REUSEADDR,&activado,sizeof(activado)) == -1){
    	printf("Error en la funcion setsockopt");
    	return -1;
    };
    if((bind(sockYoServidor, (struct sockaddr *)&miDireccion, sizeof(struct sockaddr)))<0){
    	printf("Error, no se pudo hacer el bind al puerto\n");
    	return -1;
    }
    return sockYoServidor;
}
