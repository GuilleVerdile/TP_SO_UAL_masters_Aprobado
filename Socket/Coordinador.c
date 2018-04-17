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
#include "FuncionesConexiones.h"


int coordinador(char *pathCoordinador,char *pathPlanificador){
	int sockNuevoCliente;
	struct sockaddr_in their_addr; //datos del cliente
    int sockYoServidor=crearConexionServidor(pathCoordinador);//Me inicio como servidor, el 10 es el numero maximos de conexiones en cola
    int sockYoCliente=crearConexionCliente(pathPlanificador);//Me inicio como cliente de planificador
    listen(sockYoServidor, 10);
    int sin_size = sizeof(struct sockaddr_in);
    sockNuevoCliente = accept(sockYoServidor, (struct sockaddr *)&their_addr, &sin_size);
    if(sockNuevoCliente < 0){
        	printf("Error en aceptar la conexion");
        	return 1;//error
    }
    printf("Se acepto conexion,enviando dato\n\n\n");
    char* buffer = malloc(1024);
    //verificacion conexion con ESI
    recv(sockNuevoCliente,buffer,1024,0);
    printf("%s",buffer);
    fflush(stdout);
    send(sockNuevoCliente, "Oka soy coordinador\n", 1024, 0);
    //verificacion conexion con Planificador
    send(sockYoCliente,"Hola soy el coordinador\n",1024,0);
    recv(sockYoCliente,buffer,1024,0);
    printf("%s",buffer);
    fflush(stdout);
    //libero al buffer
    free(buffer);
	return 0;
}


