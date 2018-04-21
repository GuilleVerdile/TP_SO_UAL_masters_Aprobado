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
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>

typedef struct{
	int a;
	char key[40];
	char *value;
}Paquete;
//1024
int transformarNumero(char *a,int start){
	int tam=string_length(a);
	int resultado=0;
	for(int i=0;i<tam;i++){
		resultado=(a[i+start]-48)+resultado*10;
	}
	return resultado;
}
Paquete deserializacion(char* texto){
	Paquete pack;
	pack.a = texto[0] -48;
	if(!pack.a){
		int tam = (texto[1]-48)*10 + texto[2]-48;
		strcpy(pack.key,string_substring(texto,3,tam));
		pack.value = string_substring_from(texto,tam+3);
	}
	else{
		strcpy(pack.key, string_substring_from(texto,1));
	}
	return pack;
}
Paquete recibir(int socket){
	char *total=string_new();
	char *buff=malloc(5);
	while(1){
		recv(socket, buff, 5, 0);
			if(string_contains(buff, "z")){
				int i=0;
				char *aux=malloc(5);
				strcpy(aux,buff);
				aux[string_length(buff)-1]='\0';
				printf("%s",aux);
				string_append(&total,aux);
				printf("%s",total);
				free(aux);
				break;
			}
		string_append(&total, buff);
		printf("%s",total);
	}
	free(buff);
	int tot=transformarNumero(total,0);
	printf("%d",tot);
	char *buf=malloc(tot);
	recv(socket,buf,tot,0);
	printf("%s",buf);
	Paquete pack=deserializacion(buf);
	free(buf);
	return pack;
}
void crearSelect(int soyCoordinador,char *pathYoServidor,char *pathYoCliente){// en el caso del coordinador el pathYoCliente lo pasa como NULL
	 int j=0;
	 int packRecibido=0;
	char* path;
	 int listener;
	 if(soyCoordinador)
		path="/home/utnso/git/tp-2018-1c-UAL-masters/Logs/Coordinador.log";
	 else
		path="/home/utnso/git/tp-2018-1c-UAL-masters/Logs/Planificador.log";
	 t_log *logger=log_create(path,"crearSelect",1, LOG_LEVEL_INFO);
	 fd_set master;   // conjunto maestro de descriptores de fichero
	 fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	 struct sockaddr_in their_addr; // datos cliente
	 int fdmax;        // el descriptor mas grande
	 if((listener=crearConexionServidor(pathYoServidor))==-1){
		 log_error(logger, "No se pudo crear el socket servidor");
		 log_destroy(logger);
		 exit(-1);
	 }
	 else
		 log_info(logger, "Se creao el socket de Servidor");
     int nuevoCliente;        // descriptor de socket de nueva conexión aceptada
     char *buf=malloc(1024);    // buffer para datos del cliente
     int nbytes;
     int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
     int addrlen;
     int i;
     int casoDiscriminador;
     FD_ZERO(&master);    // borra los conjuntos maestro
     FD_ZERO(&read_fds);	// borra los conjuntos maestro

     if(soyCoordinador==0){
    	if((casoDiscriminador=crearConexionCliente(pathYoCliente))==-1){
    		 log_error(logger, "No se pudo crear socket de cliente");
    		 log_destroy(logger);
    		 exit(-1);
    	}
    	else
    		log_info(logger, "Se creo el socket de cliente");
     }
     if (listen(listener, 10) == -1){
    	 log_error(logger, "No se pudo escuchar");
		 log_destroy(logger);
		 exit(-1);
     }
     else
    	 log_info(logger, "Se esta escuchando");
     if(soyCoordinador==1){
    	 addrlen = sizeof(their_addr);
    	 if((casoDiscriminador = accept(listener, (struct sockaddr *)&their_addr,&addrlen))==-1){
    		 log_error(logger, "No se aceptar la conexion");
    		 log_destroy(logger);
    		 exit(-1);
    	 }
    	 else
    		 log_info(logger, "Se acepto la conexion");
    	 printf("Nuevo cliente, se conecto el Planificador\n");
    	 fflush(stdout);
    	 if(send(casoDiscriminador,"Hola capo soy el Coordinador\n",1024,0)==-1){
    		 log_error(logger, "No se pudo enviar el mensaje");
    	 }
    	 else
    		 log_info(logger, "Mensaje enviado correctamente");
     }

     FD_SET(listener, &master);
     FD_SET(casoDiscriminador, &master);
     if(casoDiscriminador>listener)
    	 	fdmax=casoDiscriminador;
     else
         	fdmax = listener;
     for(;;) {
                 read_fds = master; // cópialo
                 if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
                	 log_error(logger, "Error Al seleccionar");
                	 exit(-1);
                 }
                 else
                	 log_info(logger, "Se selecciono correctamente");
                 // explorar conexiones existentes en busca de datos que leer
                 for(i = 0; i <= fdmax; i++) {
                     if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
                         if (i == listener) {
                        	 log_info(logger, "Conexion entrante del listener");
                             // gestionar nuevas conexiones
                             addrlen = sizeof(their_addr);
                             if ((nuevoCliente = accept(listener, (struct sockaddr *)&their_addr,
                                                                      &addrlen)) == -1) {
                            	 log_error(logger, "Al Aceptar al nuevo cliente");
                             } else {
                                 FD_SET(nuevoCliente, &master); // añadir al conjunto maestro
                                 if (nuevoCliente > fdmax) {    // actualizar el máximo
                                     fdmax = nuevoCliente;
                                 }
                                 printf("Nuevo cliente\n");
                                 log_info(logger, "Ingreso un nuevo cliente");
                                 fflush(stdout);
                                 //send(nuevoCliente,"Hola capo soy tu Servidor Select\n",1024,0);
                             }

                         }
                         else if(i==casoDiscriminador){//aca trato al coordinador//Caso discriminador
                        	log_info(logger, "Conexion entrante del discriminador");
                         	recv(casoDiscriminador,buf,1024,0);//funcion relacionada
                         	printf("%s\n",buf);
                         	fflush(stdout);
                         }
                         else {
                             /*// gestionar datos de un cliente
                             if ((nbytes = recv(i, buf, 1024, 0)) <= 0) {
                                 // error o conexión cerrada por el cliente
                                 if (nbytes == 0) {
                                     // conexión cerrada
                                	 log_info(logger, "El cliente se fue");
                                     printf("selectserver: socket %d hung up\n", i);
                                 } else {
                                	 log_info(logger, "Problema de conexion con el cliente");
                                     perror("recv");
                                 }
                                 close(i); // cierra socket
                                 FD_CLR(i, &master); // eliminar del conjunto maestro
                             } else {
                            	log_info(logger, "Conexion entrante del cliente");
                               printf("%s\n",buf);
                               fflush(stdout);
                               send(i,"Dale Esi",1024,0);
                             }*/
                        	 /*if(j==2){
                        		 packRecibido=0;
                        		 j=0;
                        	 }
                        	 if(!packRecibido){
                        		 Paquete pack= recibir(i);
                        		 packRecibido = 1;
                        	 }
                        	 j++;*/
                        	 Paquete pack= recibir(i);
                         }
                     }
             }
             }
             free(buf);
}

int coordinador(char *pathCoordinador)
    {
		crearSelect(1,pathCoordinador,NULL);
        return 0;
    }
int main(){
	coordinador("/home/utnso/git/tp-2018-1c-UAL-masters/Config/Coordinador.cfg");
	return 0;
}
