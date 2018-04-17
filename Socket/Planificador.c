/*
 * Servidor.c
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
#include "ESI.h"//Tiene la funcion dameUnaDireccion
int planificador(char *path){
    int sockfd, new_fd;
    struct sockaddr_in my_addr=dameUnaDireccion(path,1);
    struct sockaddr_in their_addr;
    int sin_size;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0){
    	printf("Error, no se pudo crear el socket\n");
    	return 1;
    }
    printf("El socket del servidor fue creado\n");
    memset(&(my_addr.sin_zero), '\0', 8);
    int activado = 1;
    if(setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR,&activado,sizeof(activado)) == -1){
    	printf("Error en la funcion setsockopt");
    	return 1;
    };
    if((bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)))<0){
    	printf("Error, no se pudo hacer el bind al puerto\n");
    	return 1;
    }
    printf("Escuchando\n");
    listen(sockfd, 10);
    sin_size = sizeof(struct sockaddr_in);
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if(new_fd < 0){
    	printf("Error en aceptar la conexion");
    	return 1;
    }
    printf("Se acepto conexion,enviando dato\n\n\n");
    char* buffer = malloc(1024);
    recv(new_fd,buffer,1024,0);
    printf("%s",buffer);
    send(new_fd, "Oka soy planificador\n", 1024, 0);
    free(buffer);
	 return 0;
}

