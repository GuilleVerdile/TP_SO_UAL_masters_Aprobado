/*
 * ClienteChat.c
 *
 *  Created on: 16 abr. 2018
 *      Author: utnso
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ESI.h"//Tiene la funcion dameUnaDireccion

int instancia(char*path){
		//decleraciones
	   int sockfd;
       struct sockaddr_in dest_addr=dameUnaDireccion(path,0);;   // Guardará la dirección de destino
       sockfd = socket(AF_INET, SOCK_STREAM, 0); // ¡Comprueba errores!
       memset(&(dest_addr.sin_zero), '\0', 8);  // Poner a cero el resto de la estructura
       // no olvides comprobar los errores de connect()!
       connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
       char *buff=malloc(1024);
       recv(sockfd,buff,1024,0);
       printf("\n%s",buff);
	   while(1){
		   printf("\nCliente --> ");
		   scanf("%*[^\n]");
		   scanf("\n");
		   scanf("%[^\n]", buff);
		   send(sockfd,buff,1024,0);
		   printf("\nServidor --> ");
		   recv(sockfd,buff,1024,0);
		   printf("%s",buff);
	   }
	 return 0;
}
