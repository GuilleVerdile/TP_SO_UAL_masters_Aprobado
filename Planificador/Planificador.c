/*
 * CoordinadoMultiple.c
 *
 *  Created on: 16 abr. 2018
 *      Author: utnso
 */
#include "Planificador.h"
int idBuscar;// esto es por ahora
int idGlobal=0;// esto es por ahora
char *claveABuscar;
t_list *procesos;
t_list *listos;
t_list *terminados;
t_list *bloqueados;
sem_t *sem_procesosListos;
sem_t *sem_procesoEnEjecucion;
sem_t *sem_finDeEjecucion;
int flag_desalojo;
int tiempo_de_ejecucion;
float alfaPlanificador;
typedef enum {bloqueado,listo,ejecucion,finalizado}Estado;
typedef struct{
	int idProceso;
	int socketProceso;
	Estado estado;
	int tiempo_que_entro;
	float estimacionAnterior;
	float rafagaRealActual;
	float rafagaRealAnterior;
}Proceso;
typedef struct{
	char *clave;
	t_list *bloqueados;
} Bloqueo;
pthread_mutex_t planiCorto;
Proceso *procesoEnEjecucion;
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
	sem_wait(sem_procesosListos);
	sem_wait(sem_finDeEjecucion);
	// aca necesito sincronizar para que se ejecute solo cuando le den la segnal de replanificar
	//no se si aca hay que hacer malloc esta bien ya que lo unico que quiero es un puntero que va a apuntar a la direccion de memoria que me va a pasar mi algoritmo
	Proceso *proceso; // ese es el proceso que va a pasar de la cola de ready a ejecucion
	proceso = (*algoritmo)();
	(*proceso).estado=ejecucion;
	procesoEnEjecucion=proceso;
	sem_post(procesoEnEjecucion);
	// el while de abajo termina cuando el proceso pasa a otra lista es decir se pone en otro estado que no sea el de ejecucion

	}
}
void *ejecutarEsi(void *esi){
	while(1){
		wait(procesoEnEjecucion);
		while((*procesoEnEjecucion).estado==ejecucion){
		send((*procesoEnEjecucion).socketProceso,"1",2,0);// este send va a perimitir al ESI ejecturar uan sententencia
		}
		(*procesoEnEjecucion).rafagaRealAnterior=(*procesoEnEjecucion).rafagaRealActual;
		(*procesoEnEjecucion).rafagaRealActual=0;
		sem_post(sem_finDeEjecucion);
	}
}
void planificadorLargoPlazo(int id,int estimacionInicial){
	if(flag_desalojo&&procesoEnEjecucion!=NULL&&(*procesoEnEjecucion).estado==ejecucion){
		(*procesoEnEjecucion).estado=listo;
		list_add(listos, procesoEnEjecucion);
	}
	Proceso *proceso=malloc(sizeof(Proceso));
	(*proceso).idProceso=idGlobal;
	(*proceso).socketProceso=id;
	(*proceso).estado=listo;
	(*proceso).estimacionAnterior=(float)estimacionInicial;
	(*proceso).rafagaRealActual=0;
	(*proceso).rafagaRealAnterior=0;
	(*proceso).tiempo_que_entro=tiempo_de_ejecucion;
	 list_add(listos, proceso);
	 list_add(procesos, proceso);
	 idGlobal++;
	 sem_post(sem_procesosListos);
	 //no hago el free porque tiene que pone la direccion de memoria del proceso en la lista!
}
//ALGORITMOS RETORNAN DE ACUERDO A SU CRITERIO EL PROCESO QUE DEBE EJECTURA DE LA COLA DE LISTO
// y elimina este proceso de la cola de listos
Proceso* fifo(){
	Proceso *proceso=list_get(listos,0);
	list_remove(listos,0);
	return proceso;
}
float *estimarSJF(Proceso *proc){
	float *aux=malloc(sizeof(float));
	if(!(*proc).rafagaRealActual){
		(*aux) = (*proc).estimacionAnterior - (*proc).rafagaRealActual;
	}
	else if((*proc).rafagaRealActual==0&&(*proc).rafagaRealAnterior==0){
		(*aux)=(*proc).estimacionAnterior;
	}
	else
		(*aux) = alfaPlanificador*(*proc).rafagaRealAnterior -(1-alfaPlanificador)*(*proc).estimacionAnterior;
	return (void*) aux;
}
bool compararSJF(void *a,void *b){
	Proceso *primero=(Proceso *) a;
	Proceso *segundo=(Proceso *) b;
	return (*(estimarSJF(a)))>(*(estimarSJF(b)));
}
float* compararHRRN(Proceso *proc){
	float *s;
	float *w=malloc(sizeof(float));
	s=estimarSJF(proc);
	(*w)=tiempo_de_ejecucion-(*proc).tiempo_que_entro;
	float *aux=malloc(sizeof(float));
	(*aux)=(*w)/(*s);
	free(s);
	free(w);
	return aux;
}

Proceso* obtenerSegunCriterio(bool (*comparar) (void*,void*)){
	t_list *aux=list_duplicate(listos);
	Proceso *proceso;
	list_sort(aux,&comparar);
	proceso=list_get(aux,0);
	list_destroy(aux);
	return proceso;
}
Proceso *sjf(){
	return obtenerSegunCriterio(&compararSJF);
}
Proceso *hrrn(){
	return obtenerSegunCriterio(&compararHRRN);
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
//Programas de Busqueda

//
bool contieneAlProceso(void *a){
	Bloqueo *b=(Bloqueo*) a;
	Proceso *proceso=list_find((*b).bloqueados,&procesoEsIdABuscar);
	if(!proceso)
		return true;
	else
		return false;
}
bool esIgualAClaveABuscar(void *a){
	Bloqueo *b=(Bloqueo*) a;
	if(!strcmp((*b).clave,claveABuscar)){
		return true;
	}
	else
		return false;
}
Bloqueo *buscarClave(){
	return list_find(bloqueados,&esIgualAClaveABuscar);
}
//antes de esta funcion un mutex
Bloqueo *buscarBloqueoPorProceso(int id){
	idBuscar=id;
	return list_find(bloqueados,&contieneAlProceso);
}
Proceso *buscarProcesoPorId(int id){
	idBuscar=id;
	return list_find(procesos,&procesoEsIdABuscar);
}

void eliminarDeLista(int id){
	//aca mutex
	Proceso *proceso =buscarProcesoPorId(id);
	t_list *t;
	Bloqueo *a;
	switch((*proceso).estado){
	case listo:
			t=listos;
			list_remove_by_condition(t,&procesoEsIdABuscar);
			break;
	case bloqueado:
			a=buscarBloqueoPorProceso(id);
			t=(*a).bloqueados;
			list_remove_by_condition(t,&procesoEsIdABuscar);
			break;
	case ejecucion:
			procesoEnEjecucion=NULL;
			break;
	}
}
// este lo uso cuando el coordinador me dice que esi bloquear

void bloquear(int id,char *valor){
	Proceso* proceso=buscarProcesoPorId(id);
	claveABuscar=valor;
	Bloqueo *block=buscarClave();
	(*proceso).estado=bloqueado;
	if(block!=NULL){
		list_add((*block).bloqueados,proceso);
	}
	else{
		block=malloc(sizeof(Bloqueo));
		(*block).clave=valor;
		(*block).bloqueados=list_create();
		list_add((*block).bloqueados,proceso);
	}
	//Con este send le aviso al proceso que fue bloqueado
	send((*proceso).socketProceso,"2",2,0);

}
//este lo tengo que usar cuando el esi me dice que hace un store;
void desbloquear(int id){
	Proceso *proceso =buscarProcesoPorId(id);
}
void crearSelect(int soyCoordinador,char *pathYoServidor,char *pathYoCliente,Proceso(*algoritmo)(),int estimacionInicial){// en el caso del coordinador el pathYoCliente lo pasa como NULL
     procesos=list_create();
	 listos=list_create();
     terminados=list_create();
     bloqueados=list_create();
	 pthread_t planificadrCortoPlazo;
	 pthread_t ejecutarEsi;
	 pthread_create(&planificadorCortoPlazo,NULL,planificadorCortoPlazo,(void *) algoritmo);
	 pthread_create(&ejecutarEsi,NULL,ejecutarEsi,NULL);
	 sem_init(sem_procesosListos,0,0);
	 sem_init(sem_procesoEnEjecucion,0,0);
	 sem_init(sem_finDeEjecucion,0,1);
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
                                 planificadorLargoPlazo(nuevoCliente,estimacionInicial);
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
                               //aca hago un case de los posibles send de un esi, que son
                               //1.- termino ejecucion el esi y nos esta informando
                               int caso = buf[0]-48;
                               Estado estado;
                               switch(caso){
                               case 1:
                            	   estado=finalizado;
                            	   actualizarEstado(i,finalizado,1);// puse 1 en el ultimo parametro por que la actualizacion la tengo que hacer por socket
                            	   break;
                               case 2:
                            	   tiempo_de_ejecucion++;
                            	   (*procesoEnEjecucion).rafagaRealActual++;
                                   break;
                               }
                               fflush(stdout);
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
	int estimacionInicial=config_get_int_value(config,"EstimacionInicial");
	char*algoritmo= (config_get_string_value(config, "AlgoritmoDePlanificador"));
	config_destroy(config);
	if(!strcmp(algoritmo,"fifo")){
		miAlgoritmo=&fifo;
		flag_desalojo=0;
	}
	if(!strcmp(algoritmo,"SJF-CD")){
			miAlgoritmo=&fifo;
			flag_desalojo=1;
		}
	if(!strcmp(algoritmo,"SJF-SD")){
			miAlgoritmo=&fifo;
			flag_desalojo=0;
		}
		crearSelect(0,pathPlanificador,pathCoordinador,miAlgoritmo,estimacionInicial);
        return 0;
    }
int main(){
	 planificador();
	return 0;
}
