/*
 * Cliente.c
 *
 *  Created on: 15 abr. 2018
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

int crearConexionEsi(char*path){//retorna el descriptor de fichero
	int sock=socket(AF_INET, SOCK_STREAM, 0);
	if(sock<0){
	return -1;// CASO DE ERROR
	}
	struct sockaddr_in midireccion=dameUnaDireccion(path,0);
	memset(&midireccion.sin_zero, '\0', 8);
	if(connect(sock, (struct sockaddr *)&midireccion, sizeof(struct sockaddr))<0){
		return -1;
	}
	return sock;
}

int esi(char* pathCoordinador,char*pathPlanificador){
		int sockplanificador=crearConexionEsi(pathPlanificador);
		int sockcoordinador=crearConexionEsi(pathCoordinador);
	   printf("Se crearon sockets cliente!");
	   if(sockplanificador<0 || sockcoordinador<0){
		   printf("Error de conexion a los servidores\n");
		   return 1;
	   }
	   printf("Se conecto a los 2 servidores\n");
	   //me avisan todo bien
	   send(sockplanificador,"Hola soy el cliente :D",1024,0);
	   send(sockcoordinador,"Hola soy el cliente :D",1024,0);
	   char* buffer = malloc(1024);
	   recv(sockcoordinador,buffer,1024,0);
	   printf("%s",buffer);
	   recv(sockplanificador,buffer,1024,0);
	   printf("%s",buffer);
	   free(buffer);
	 return 0;
}

