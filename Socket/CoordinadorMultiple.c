/*
 * CoordinadoMultiple.c
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

int coordinadormultiple()
    {
        fd_set master;   // conjunto maestro de descriptores de fichero
        fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
        struct sockaddr_in myaddr;     // dirección del servidor
        struct sockaddr_in remoteaddr; // dirección del cliente
        int fdmax;        // número máximo de descriptores de fichero
        int listener;     // descriptor de socket a la escucha
        int newfd;        // descriptor de socket de nueva conexión aceptada
        char *buf=malloc(1024);    // buffer para datos del cliente
        int nbytes;
        int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
        int addrlen;
        int i, j;
        FD_ZERO(&master);    // borra los conjuntos maestro y temporal
        FD_ZERO(&read_fds);
        // obtener socket a la escucha
        if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }
        // obviar el mensaje "address already in use" (la dirección ya se está usando)
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
                                                            sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        // enlazar
        myaddr.sin_family = AF_INET;
        myaddr.sin_addr.s_addr = INADDR_ANY;
        myaddr.sin_port = htons(8090);
        memset(&(myaddr.sin_zero), '\0', 8);
        if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
            perror("bind");
            exit(1);
        }
        // escuchar
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
                        addrlen = sizeof(remoteaddr);
                        if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr,
                                                                 &addrlen)) == -1) {
                            perror("accept");
                        } else {
                            FD_SET(newfd, &master); // añadir al conjunto maestro
                            if (newfd > fdmax) {    // actualizar el máximo
                                fdmax = newfd;
                            }
                            printf("Nuevo cliente\n");
                            fflush(stdout);
                            send(newfd,"Hola capo soy el Coordinador\n",1024,0);
                        }
                    } else {
                        // gestionar datos de un cliente
                        if ((nbytes = recv(i, buf, 1024, 0)) <= 0) {
                            // error o conexión cerrada por el cliente
                            if (nbytes == 0) {
                                // conexión cerrada
                                printf("selectserver: socket %d hung up\n", i);
                            } else {
                                perror("recv");
                            }
                            close(i); // bye!
                            FD_CLR(i, &master); // eliminar del conjunto maestro
                        } else {
                          printf("%s\n",buf);
                          fflush(stdout);
                          send(i,"Dale capo",1024,0);
                        }
                    } // Esto es ¡TAN FEO!
                }
            }
        }
        free(buf);
        return 0;
    }