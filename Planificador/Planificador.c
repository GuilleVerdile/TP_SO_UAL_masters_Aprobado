/*
 * CoordinadoMultiple.c
 *
 *  Created on: 16 abr. 2018
 *      Author: utnso
 */
#include "Planificador.h"



void crearSelect(int soyCoordinador,char *pathYoServidor,char *pathYoCliente){// en el caso del coordinador el pathYoCliente lo pasa como NULL
	 char* path;
	 int listener;
	 char* buf;
	 if(soyCoordinador)
		path=logCoordinador;
	 else
		path=logPlanificador;
	 logger=log_create(path,"crearSelect",1, LOG_LEVEL_INFO);
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

     FD_SET(listener, &master);
     FD_SET(casoDiscriminador, &master);
     if(casoDiscriminador>listener)
    	 	fdmax=casoDiscriminador;
     else
         	fdmax = listener;
     t_queue *listos;
     listos=queue_create();
     t_queue *ejecucion;
     ejecucion=queue_create();
     t_queue *terminados;
     terminados=queue_create();
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
                                 int *aux = malloc(sizeof(int));
                                 *aux=nuevoCliente;
                                 queue_push(listos, aux);
                                 printf("Nuevo cliente\n");
                                 log_info(logger, "Ingreso un nuevo cliente");
                                 fflush(stdout);
                             }

                         }
                         else if(i==casoDiscriminador){//aca trato al coordinador//Caso discriminador
                        	buf = malloc(1024);
                        	log_info(logger, "Conexion entrante del discriminador");
                         	recv(casoDiscriminador,buf,1024,0);//funcion relacionada
                         	printf("%s\n",buf);
                         	fflush(stdout);
                         	free(buf);
                         }
                         else {

                        	 buf = malloc(2);
                             // gestionar datos de un cliente
                             if ((nbytes = recv(i, buf, 2, 0)) <= 0) {
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
                               int *primerElemento=queue_peek(listos);
                               if(*primerElemento==i){
                            	   int *aux = malloc(sizeof(int));
                            	   *aux=i;
                            	   send(i,"1",2,0);
                            	   log_info(logger, "Se le permitio al esi parsear");
                            	   queue_pop(listos);
                            	   log_info(logger, "Se saco el esi de la cola de listos");
                            	   queue_push(ejecucion,aux);
                            	   log_info(logger, "Se metio el esi a la cola de ejecucion");
                            	   if(recv(i,buf,2,0)-48)
                            		   queue_pop(ejecucion);
                            	   log_info(logger, "Se saco el esi a la cola de ejecucion");
                            	   queue_push(terminados,aux);
                            	   log_info(logger, "se termino el esi");
                            	   // no se si usar este JIJOOOO
                               }
                               else
                            	   send(i,"0",2,0);

                             }
                             free(buf);
                         }
                     }
             }
             }
             free(buf);
}

int planificador()
    {

		crearSelect(0,pathPlanificador,pathCoordinador);
        return 0;
    }
int main(){
	 planificador();
	return 0;
}
