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

int prueba1(){
    int sockfd, new_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    int sin_size;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0){
    	printf("Error, no se pudo crear el socket\n");
    	return 1;
    }
    printf("El socket del servidor fue creado\n");

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(8098);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero), '\0', 8);
    int activado = 1;
    setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR,&activado,sizeof(activado));
    if((bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)))<0){
    	printf("Error, no se pudo hacer el bind al puerto\n");
    	return 1;
    }
    printf("Escuchando\n");
    listen(sockfd, 10);
    sin_size = sizeof(struct sockaddr_in);
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    printf("Se acepto conexion,enviando dato\n\n\n");

    char *msg="Hola Cliente!!\n\n";
    send(new_fd, msg, 1024, 0);

    char *buff=malloc(1024);
    recv(new_fd, buff, 1024, 0);
    printf("%s",buff);
	 return 0;
}

