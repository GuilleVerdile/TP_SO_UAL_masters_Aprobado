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


int clientechat(){
		//decleraciones
	   int sockfd;
       struct sockaddr_in dest_addr;   // Guardará la dirección de destino
       sockfd = socket(AF_INET, SOCK_STREAM, 0); // ¡Comprueba errores!
       dest_addr.sin_family = AF_INET;          // Ordenación de máquina
       dest_addr.sin_port = htons(8090);   // short, Ordenación de la red
       dest_addr.sin_addr.s_addr = inet_addr("127.0.0.3");
       memset(&(dest_addr.sin_zero), '\0', 8);  // Poner a cero el resto de la estructura
       // no olvides comprobar los errores de connect()!
       connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
       char *buff=malloc(1024);
       recv(sockfd,buff,1024,0);
       printf("\n%s",buff);
	   while(1){
		   printf("\nCliente --> ");
		   fgets(buff,1024, stdin);
		   send(sockfd,buff,1024,0);
		   printf("\nServidor --> ");
		   recv(sockfd,buff,1024,0);
		   printf("%s",buff);
	   }
	 return 0;
}
