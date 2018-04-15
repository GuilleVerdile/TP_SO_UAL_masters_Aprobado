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

int prueba(){
	   int sockfd;
	   struct sockaddr_in dest_addr;
	   sockfd = socket(AF_INET, SOCK_STREAM, 0);
	   if(sockfd<0){
	       	printf("Error, no se pudo crear el socket\n");
	       	exit(0);
	       }
	   printf("Se creo socket cliente!");
	   dest_addr.sin_family = AF_INET;
	   dest_addr.sin_port = htons(8097);
	   dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	   memset(&(dest_addr.sin_zero), '\0', 8);
	   connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
	   printf("Se conecto al servidor\n");
	   char *buff=malloc(1024);
	   recv(sockfd, buff, 1024, 0);
	   printf("%s",buff);

	   char *msg="Hola servidor!!\n\n";
	   send(sockfd, msg, 1024, 0);

	 return 0;
}
