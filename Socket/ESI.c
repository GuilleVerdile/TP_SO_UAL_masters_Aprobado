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

void setearDireccion (struct sockaddr_in *midireccion,int puerto,char *ip){
	(*midireccion).sin_family = AF_INET;
	(*midireccion).sin_port = htons(puerto);
	(*midireccion).sin_addr.s_addr = inet_addr(ip);
	memset(&(*midireccion).sin_zero, '\0', 8);
}
int prueba0999999(){
		//decleraciones
	   int sockplanificador=socket(AF_INET, SOCK_STREAM, 0);;
	   int sockcoordinador=socket(AF_INET, SOCK_STREAM, 0);;
	   struct sockaddr_in planificador;
	   struct sockaddr_in coordinador;
	   if(sockplanificador<0||sockcoordinador<0){
	       	printf("Error, no se pudo crear el socket\n");
	       	return 1;
	       }
	   printf("Se crearon socketS cliente!");
	   setearDireccion(&planificador,8080,"127.0.0.1");
	   setearDireccion(&coordinador,8081,"127.0.0.1");

	   //conecto
	   connect(sockplanificador, (struct sockaddr *)&planificador, sizeof(struct sockaddr));
	   connect(sockcoordinador, (struct sockaddr *)&coordinador, sizeof(struct sockaddr));

	   printf("Se conecto a los 2 servidores\n");
	   //me avisan todo bien
	   char *buff=malloc(1024);
	   recv(sockplanificador, buff, 1024, 0);
	   printf("%s",buff);
	   recv(sockcoordinador, buff, 1024, 0);
	   printf("%s",buff);
	   free(buff);
	 return 0;
}

