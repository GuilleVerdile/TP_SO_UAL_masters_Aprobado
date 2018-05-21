/*
 * CoordinadoMultiple.c
 *
 *  Created on: 16 abr. 2018
 *      Author: utnso
 */
#include "Planificador.h"
int idBuscar;// esto es por ahora
int idGlobal=0;// esto es por ahora
t_list *procesos;
t_list *listos;
t_list *terminados;
typedef enum {bloqueado,listo,ejecucion,finalizado}Estado;
typedef struct{
	int idProceso;
	int socketProceso;
	Estado estado;
}Proceso;
// tengo 2 funciones bastantes parecidas ver como poder refactorizar
bool procesoEsIdABuscar(void * proceso){
	Proceso *proc=(Proceso*) proceso;
	if((*proc).idProceso==idBuscar)
		return true;
	else
		return false;
}
bool procesoEsIdABuscarSocket(void * proceso){
	Proceso *proc=(Proceso*) proceso;
	if((*proc).socketProceso==idBuscar)
		return true;
	else
		return false;
}
void actualizarEstado(int id,Estado estado,int porSocket){// 0 si es busqueda normal, otra cosa si es por socket
	//si voy a usar esta variable global falta mutex
		idBuscar= id;
		bool(*criterio)(void*);

	if(porSocket)
		criterio=&procesoEsIdABuscarSocket;
	else
		criterio=&procesoEsIdABuscar;
	Proceso *proceso =(Proceso *) list_find(procesos, criterio);
	(*proceso).estado=estado;
}
void *planificadorCortoPlazo(void *miAlgoritmo){//como parametro le tengo que pasar la direccion de memoria de mi funcion algoritmo
	Proceso*(*algoritmo)();
	algoritmo=(Proceso*(*)()) miAlgoritmo;
	// se tiene que ejecutar todo el tiempo en un hilo aparte
	while(1){
	// aca necesito sincronizar para que se ejecute solo cuando le den la segnal de replanificar
	//no se si aca hay que hacer malloc esta bien ya que lo unico que quiero es un puntero que va a apuntar a la direccion de memoria que me va a pasar mi algoritmo
	Proceso *proceso; // ese es el proceso que va a pasar de la cola de ready a ejecucion
	proceso = (*algoritmo)();
	(*proceso).estado=ejecucion;
	// el while de abajo termina cuando el proceso pasa a otra lista es decir se pone en otro estado que no sea el de ejecucion
		while((*proceso).estado==ejecucion){
			send((*proceso).socketProceso,"1",2,0);// este send va a perimitir al hilo ejecturar uan sententencia
		}
	}
}
void *planificadorLargoPlazo(void *id){
	Proceso *proceso=malloc(sizeof(Proceso));
	(*proceso).idProceso=idGlobal;
	(*proceso).socketProceso=(int) id;
	(*proceso).estado=listo;
	 list_add(listos, proceso);
	 list_add(procesos, proceso);
	 idGlobal++;
	 //no hago el free porque tiene que pone la direccion de memoria del proceso en la lista!
}
//ALGORITMOS RETORNAN DE ACUERDO A SU CRITERIO EL PROCESO QUE DEBE EJECTURA DE LA COLA DE LISTO
// y elimina este proceso de la cola de listos
Proceso* fifo(){
	Proceso *proceso=list_get(listos,0);
	list_remove(listos,0);
	return proceso;
}
/*void fifo(int i,char *buf){
    int *primerElemento=list_get(listos,0);
    if(*primerElemento==i){

    int *aux = malloc(sizeof(int));
    *aux=i;
    send(i,"1",2,0);
    log_info(logger, "Se le permitio al esi parsear");
    list_remove(listos,0);
    log_info(logger, "Se saco el esi de la cola de listos");
    list_add(ejecucion,aux);
    log_info(logger, "Se metio el esi a la cola de ejecucion");
    if(recv(i,buf,2,0)-48)
         list_remove(ejecucion,0);
    log_info(logger, "Se saco el esi a la cola de ejecucion");
     list_add(terminados,aux);
     log_info(logger, "se termino el esi");
                           	   // no se si usar este JIJOOOO

      }
                              else{
                            	  send(i,"0",2,0);
                            	  log_info(logger, "Se le nego al esi parsear");
                              }
}*/
void crearSelect(int soyCoordinador,char *pathYoServidor,char *pathYoCliente,void(*miAlgoritmo)(int,char*)){// en el caso del coordinador el pathYoCliente lo pasa como NULL
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
     listos=list_create();
     ejecucion=list_create();
     terminados=list_create();
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
                                 list_add(listos, aux);
                                 printf("Nuevo cliente\n");
                                 log_info(logger, "Ingreso un nuevo cliente");
                                 fflush(stdout);
                             }

                         }
                         else if(i==casoDiscriminador){//aca trato al coordinador//Caso discriminador
                        	 buf = malloc(1024);
                        	 if ((nbytes = recv(i, buf, 1024, 0)) <= 0) {

                        	                                 // error o conexión cerrada por el cliente
                        		 if (nbytes == 0) {
                        	                                     // conexión cerrada
                        	                                	 log_info(logger, "El coordinator se fue");
                        	                                     printf("selectserver: socket %d hung up\n", i);
                        	                                 } else {
                        	                                	 log_info(logger, "Problema de conexion con el coordinador");
                        	                                     perror("recv");
                        	                                 }
                        	                                 close(i); // cierra socket
                        	                                 FD_CLR(i, &master); // eliminar del conjunto maestro
                        	                             }
                        	 else{

                        		                         	log_info(logger, "Conexion entrante del discriminador");
                        		                          	printf("%s\n",buf);
                        		                          	fflush(stdout);
                        		                          	free(buf);
                        	 }
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
                               //aca va el algoritmo
                               (*miAlgoritmo)(i,buf);
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
	void(*miAlgoritmo)(int,char*);
	t_config *config=config_create(pathPlanificador);
	switch (config_get_int_value(config, "AlgoritmoDePlanificador")){
	config_destroy(config);
	case 0: // este es el fifo
		miAlgoritmo=&fifo;
		break;
	}
		crearSelect(0,pathPlanificador,pathCoordinador,miAlgoritmo);
        return 0;
    }
int main(){
	 planificador();
	return 0;
}
