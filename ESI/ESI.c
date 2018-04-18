/*
 * Cliente.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "FuncionesConexiones.h"


int esi(char* pathCoordinador,char*pathPlanificador){
		int sockplanificador=crearConexionCliente(pathPlanificador);
		int sockcoordinador=crearConexionCliente(pathCoordinador);
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
int main(){
	 char *pathCoordinador="/home/utnso/git/tp-2018-1c-UAL-masters/Config/Coordinador.cfg";
			 char *pathPlanificador="/home/utnso/git/tp-2018-1c-UAL-masters/Config/Planificador.cfg";
			 esi(pathCoordinador,pathPlanificador);
			 while(1){}
	return 0;
}

