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


/*int coordinador(char *pathCoordinador,){
	int sockNuevoCliente;
	struct sockaddr_in their_addr; //datos del cliente
    int sockYoServidor=crearConexionServidor(pathCoordinador)idor, el 10 es el numero maximos de conexiones en cola
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
}*/

int coordinador(char *pathCoordinador)
    {
        fd_set master;   // conjunto maestro de descriptores de fichero
        fd_set read_fds; // conjunto temporal de descriptores de fichero para select()

        struct sockaddr_in their_addr; // datos cliente
        int fdmax;        // número máximo de descriptores de fichero
        int listener=crearConexionServidor(pathCoordinador);     //se usa la funcion que me devuelve el sock del servidor
        int nuevoCliente;        // descriptor de socket de nueva conexión aceptada
        char *buf=malloc(1024);    // buffer para datos del cliente
        int nbytes;
        int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
        int addrlen;
        int i, j;
        FD_ZERO(&master);    // borra los conjuntos maestro y temporal
        FD_ZERO(&read_fds);

        if (listen(listener, 10) == -1) {
            perror("listen");
            exit(1);
        }
        printf("Escuchando\n");
        // añadir listener al conjunto maestro
        FD_SET(listener, &master);
        // seguir la pista del descriptor de fichero mayor
        fdmax = listener; // por ahora es éste
        // bucle principal
        int seConectoPlanificador=0;

        for(;;) {
            read_fds = master; // cópialo
            if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
                perror("select");
                exit(1);
            }
            // explorar conexiones existentes en busca de datos que leer
            for(i = 0; i <= fdmax; i++) {
                if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
                    if (i == listener) {
                        // gestionar nuevas conexiones
                        addrlen = sizeof(their_addr);
                        if ((nuevoCliente = accept(listener, (struct sockaddr *)&their_addr,
                                                                 &addrlen)) == -1) {
                            perror("accept");
                        } else {
                        	if(seConectoPlanificador==0){
                        		int sockPlanificador = nuevoCliente; // ACA SE GUARDA EL SOCK QUE SE REFIERE AL PLANIFICADOR
                        		seConectoPlanificador=1;
                        	}
                            FD_SET(nuevoCliente, &master); // añadir al conjunto maestro
                            if (nuevoCliente > fdmax) {    // actualizar el máximo
                                fdmax = nuevoCliente;
                            }
                            printf("Nuevo cliente\n");
                            fflush(stdout);
                            send(nuevoCliente,"Hola capo soy el Coordinador\n",1024,0);
                        }

                    }
                    else {
                        // gestionar datos de un cliente
                        if ((nbytes = recv(i, buf, 1024, 0)) <= 0) {
                            // error o conexión cerrada por el cliente
                            if (nbytes == 0) {
                                // conexión cerrada
                                printf("selectserver: socket %d hung up\n", i);
                            } else {
                                perror("recv");
                            }
                            close(i); // cierra socket
                            FD_CLR(i, &master); // eliminar del conjunto maestro
                        } else {
                          printf("%s\n",buf);
                          fflush(stdout);
                          send(i,"Dale Esi",1024,0);
                        }
                    }
                }
        }
        }
        free(buf);
        return 0;
    }


