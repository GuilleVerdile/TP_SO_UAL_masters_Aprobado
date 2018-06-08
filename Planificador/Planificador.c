/*
 * CoordinadoMultiple.c
 *
 *  Created on: 16 abr. 2018
 *      Author: utnso
 */
#include "Planificador.h"
#include "Consola.h"
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

void terminarProceso(){
	sem_post(&sem_ESIejecutoUnaSentencia);
	liberarRecursos((*procesoEnEjecucion).idProceso);
	(*procesoEnEjecucion).estado = finalizado;
	list_add(terminados,procesoEnEjecucion);
	procesoEnEjecucion = NULL;
	sem_post(&semCambioEstado);
}

void *planificadorCortoPlazo(void *miAlgoritmo){//como parametro le tengo que pasar la direccion de memoria de mi funcion algoritmo
	t_log *log_planiCorto;
	log_planiCorto=log_create(logPlanificador,"Planificador Corto Plazo",1, LOG_LEVEL_INFO);
	Proceso*(*algoritmo)();
	algoritmo=(Proceso*(*)()) miAlgoritmo;
	// se tiene que ejecutar todo el tiempo en un hilo aparte
	while(1){
	log_info(log_planiCorto, "esperando segnal de sem planificador");
	sem_wait(&sem_replanificar);
	log_info(log_planiCorto, "se obtuvo segnal de sem planificador");
	// aca necesito sincronizar para que se ejecute solo cuando le den la segnal de replanificar
	//no se si aca hay que hacer malloc esta bien ya que lo unico que quiero es un puntero que va a apuntar a la direccion de memoria que me va a pasar mi algoritmo
	Proceso *proceso; // ese es el proceso que va a pasar de la cola de ready a ejecucion
	proceso = (*algoritmo)();
	if(proceso){
	log_info(log_planiCorto, "Se selecciono un proceso por el algoritmo");
	log_info(log_planiCorto, "esperando el semaforo sem fin de ejecucion");
	sem_wait(&sem_finDeEjecucion);
	log_info(log_planiCorto, "se paso el semaforo sem fin de ejecucion");
	(*proceso).estado=ejecucion;
	procesoEnEjecucion=proceso;
	log_info(log_planiCorto, "se paso el semaforo sem fin de ejecucion");
	sem_post(&sem_procesoEnEjecucion);
	log_info(log_planiCorto, "se dio segnal de ejecutar el esi en ejecucion");
	// el while de abajo termina cuando el proceso pasa a otra lista es decir se pone en otro estado que no sea el de ejecucion
	}
	}
	log_destroy(log_planiCorto);
}
void *ejecutarEsi(void *esi){
	t_log *log_ejecturarEsi;
	log_ejecturarEsi=log_create(logPlanificador,"Ejecutar ESI",1, LOG_LEVEL_INFO);
	while(1){
		sem_wait(&sem_procesoEnEjecucion);
		log_info(log_ejecturarEsi, "se entro a ejecutar el esi en ejecucion");
		while(procesoEnEjecucion && (*procesoEnEjecucion).estado==ejecucion){
			log_info(log_ejecturarEsi, "esperando semaforo de que el esi ejecuto una sentencia");
			sem_wait(&sem_ESIejecutoUnaSentencia);
			log_info(log_ejecturarEsi, "pasando semaforo de esi ejecuto una sentencia");
			send((*procesoEnEjecucion).socketProceso,"1",2,0);// este send va a perimitir al ESI ejecturar uan sententencia
			log_info(log_ejecturarEsi, "se envio al es en ejecucion de ejecutar");
			sem_wait(&semCambioEstado);
		}
		if(procesoEnEjecucion){
		(*procesoEnEjecucion).rafagaRealAnterior=(*procesoEnEjecucion).rafagaRealActual;
		(*procesoEnEjecucion).rafagaRealActual=0;
		}
		sem_post(&sem_finDeEjecucion);
		log_info(log_ejecturarEsi, "se da segnal de fin de ejecucion");
	}
	log_destroy(log_ejecturarEsi);
}
void planificadorLargoPlazo(int id,int estimacionInicial){
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
	 flag_nuevoProcesoEnListo = 1;
	 idGlobal++;
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
	return (*(estimarSJF(a)))<=(*(estimarSJF(b)));
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
	list_sort(aux,comparar); //CREO UNA LISTA AUXILIAR Y LO ORDENO
	proceso=list_get(aux,0);
	list_destroy(aux);
	idBuscar = (*proceso).idProceso;
	list_remove_by_condition(listos,&procesoEsIdABuscar); //ELIMINO EL PROCESO QUE COINCIDA CON TAL ID EN LA COLA DE LISTOS
	return proceso;
}
Proceso *sjf(){
	return obtenerSegunCriterio(&compararSJF);
}
Proceso *hrrn(){
	return obtenerSegunCriterio(&compararHRRN);
}
//Programas de Busqueda


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
	idBuscar=id;
	//
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
///
///
///
//crea la cola de bloqueados con clave a id
//bloquea clave a , id tanto
//libera clave a
// REFACTORIZAR BLOQUEARPORID y BLOQUEAR!!!!!!!!!!!!!!!!!!***********
void bloquearPorID(char *clave,int id){
	char *aux=malloc(strlen(clave)+1);
	strcpy(aux,clave);
	claveABuscar=aux;
	Bloqueo *block=buscarClave();
	//aca mutex
	idBuscar=id;
	Proceso *proceso;
		//
	//REVISAR SOLUCION CON IF PARA BLOQUEAR CLAVES INICIALES POR CONFIG!!!!!!!!!!!!!!!!!!
	if(id)
	proceso=buscarProcesoPorId(id);
	if(!block){
		block=malloc(sizeof(Bloqueo));
		(*block).clave=aux;
		(*block).bloqueados=list_create();
		if(id)
		(*block).idProceso=(*proceso).idProceso;
		else
			(*block).idProceso=0;
		list_add(bloqueados,block);
	}
	else{
		if((*block).idProceso==-1){
			(*block).idProceso=(*proceso).idProceso;
		}
		else{
			list_add((*block).bloqueados,proceso);
			(*proceso).estado = bloqueado;
		}
	}
}
void bloquear(char *clave){//En el hadshake con el coordinador asignar proceso en ejecucion a proceso;
	char *aux=malloc(strlen(clave)+1);
	strcpy(aux,clave);
	claveABuscar=aux;
	Bloqueo *block=buscarClave();
	if(!block){
		block=malloc(sizeof(Bloqueo));
		(*block).clave=aux;
		(*block).bloqueados=list_create();
		(*block).idProceso=(*procesoEnEjecucion).idProceso;
		list_add(bloqueados,block);
	}
	else{
		if((*block).idProceso==-1){
			(*block).idProceso=(*procesoEnEjecucion).idProceso;
		}
		else{
			list_add((*block).bloqueados,procesoEnEjecucion);
			(*procesoEnEjecucion).estado = bloqueado;
			sem_post(&sem_replanificar); //REPLANIFICO CUANDO UN PROCESO SE VA A LA COLA DE BLOQUEADOS!
		}
	}
}
void liberarRecursos(int id){
	Bloqueo *block;
	int i=0;
	while((block=list_get(bloqueados,i))!=NULL){
		if((*block).idProceso==id){
			liberaClave((*block).clave);
		}
		else{
			Proceso *proceso;
			int j=0;
			while((proceso=list_get((*block).bloqueados,j))!=NULL){
				if((*proceso).idProceso==id){
					list_remove((*block).bloqueados,j);
					break;
				}
				j++;
			}
		}
		i++;
	}
}

void liberaClave(char *clave){
	claveABuscar=clave;
	Bloqueo *block=buscarClave();
	if(!list_is_empty((*block).bloqueados)){
		Proceso *proceso=list_remove((*block).bloqueados,0);
		if(list_is_empty((*block).bloqueados)){
			list_destroy((*block).bloqueados);
			claveABuscar=clave;
			free(list_remove_by_condition(bloqueados,&esIgualAClaveABuscar));
		}
		else
		(*block).idProceso=-1;
		(*proceso).estado=listo;
		list_add(listos,proceso);
		sem_post(&sem_replanificar);
	}
	else{
		claveABuscar=clave;
		free(list_remove_by_condition(bloqueados,&esIgualAClaveABuscar));
	}
}

char *sePuedeBloquear(char*clave){
	claveABuscar=clave;
	Bloqueo *block=buscarClave();
	if(!block || (*block).idProceso==-1)
		return "1";
	else
		return "0";
}
char *verificarClave(Proceso *proceso,char *clave){
	claveABuscar=clave;
	Bloqueo *block=buscarClave();
	if(block && (*block).idProceso==(*proceso).idProceso)
		return "1";
	else
		return "0";
}

//este lo tengo que usar cuando el esi me dice que hace un store;
void desbloquear(int id){
	Proceso *proceso =buscarProcesoPorId(id);
}

void tirarErrorYexit(char* mensajeError) {
	log_error(logger, mensajeError);
	log_destroy(logger);
	cerrarPlanificador();
	exit(-1);
}
void matarESI(int id){
	idBuscar=id;
	if(!list_find(listos,&procesoEsIdABuscar)){
		list_remove_by_condition(listos,&procesoEsIdABuscar);
	}
	if((*procesoEnEjecucion).idProceso==id){
		sem_post(&sem_replanificar);
	}
	liberarRecursos(id);
	idBuscar = id;
	Proceso* procesoAEliminar = list_remove_by_condition(procesos,&procesoEsIdABuscar);
	free(procesoAEliminar);
}
void crearSelect(int estimacionInicial){// en el caso del coordinador el pathYoCliente lo pasa como NULL
     procesos=list_create();
	 listos=list_create();
     terminados=list_create();
     bloqueados=list_create();
     procesoEnEjecucion = NULL;
	 sem_init(&sem_replanificar,0,0);
	 sem_init(&sem_procesoEnEjecucion,0,0);
	 sem_init(&sem_ESIejecutoUnaSentencia,0,1);
	 sem_init(&sem_finDeEjecucion,0,1);
	 sem_init(&semCambioEstado,0,0);
	 int listener;
	 char* buf;
	 t_config *config=config_create("/home/utnso/git/tp-2018-1c-UAL-masters/Config/Planificador.cfg");
	 bloquearClavesIniciales(config);
	//HOLA
	 logger=log_create(logPlanificador,"crearSelect",1, LOG_LEVEL_INFO);
	 fd_set master;   // conjunto maestro de descriptores de fichero
	 fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	 struct sockaddr_in their_addr; // datos cliente
	 int fdmax;        // el descriptor mas grande
	 if((listener=crearConexionServidor(config_get_int_value(config, "Puerto de Escucha de conexiones"),"127.0.0.2"))==-1){
		 config_destroy(config);
		tirarErrorYexit("No se pudo crear el socket servidor");
	 }
	 else
		 log_info(logger, "Se creo el socket de Servidor");
     int nuevoCliente;        // descriptor de socket de nueva conexión aceptada

     int nbytes;
     int addrlen;
     int i;
     int casoDiscriminador;
     FD_ZERO(&master);    // borra los conjuntos maestro
     FD_ZERO(&read_fds);	// borra los conjuntos maestro
    	if(((casoDiscriminador=crearConexionCliente(8001,"127.0.0.1")))==-1){
    		config_destroy(config);
    		tirarErrorYexit("No se pudo crear socket de cliente");
    	}
    	/*if((casoDiscriminador=crearConexionCliente(config_get_int_value(config, "Puerto de Conexión al Coordinador"),config_get_string_value(config, "IP de Conexion al Coordinador")))==-1){
    		config_destroy(config);
    		tirarErrorYexit("No se pudo crear socket de cliente");
    	}*/
    	else
    		log_info(logger, "Se creo el socket de cliente");
     config_destroy(config); // SI NO HAY ERROR SE DESTRUYE FINALMENTE EL CONFIG
     if (listen(listener, 10) == -1){
    	 tirarErrorYexit("No se pudo escuchar");
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
                	 tirarErrorYexit("Error al seleccionar");
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
                            	 log_error(logger, "Al aceptar al nuevo cliente");
                             } else {
                                 FD_SET(nuevoCliente, &master); // añadir al conjunto maestro
                                 if (nuevoCliente > fdmax) {    // actualizar el máximo
                                     fdmax = nuevoCliente;
                                 }
                                 planificadorLargoPlazo(nuevoCliente,estimacionInicial);
                                 if(procesoEnEjecucion==NULL){ //SI ES NULL SIGNIFICA QUE NO HAY NADIE EN EJECUCION.
                                	 sem_post(&sem_replanificar);
                                	 flag_nuevoProcesoEnListo = 0; //COMO YA METI UN NUEVO PROCESO A EJECUCION NO HACE FALTA QUE REPLANIFIQUE EN CASO DE DESALOJO
                                 }
                                 log_info(logger, "Ingreso un nuevo cliente");
                             }
                         }
                         else if(i==casoDiscriminador){ //aca trato al coordinador//Caso discriminador
                        	 buf = malloc(2);
                        	 if ((nbytes = recv(i, buf, 2, 0)) <= 0) {

                        	                                 // error o conexión cerrada por el cliente
                        		 if (nbytes == 0) {
                        	                                     // conexión cerrada
                        	                             	 log_info(logger, "El coordinator se fue");
                        	                                     printf("selectserver: socket %d hung up\n", i);
                        	                                     cerrarPlanificador;
                        	                                 } else {
                        	                                	 log_info(logger, "Problema de conexion con el coordinador");
                        	                                     perror("recv");
                        	                                 }
                        	                                 close(i); // cierra socket
                        	                                 FD_CLR(i, &master); // eliminar del conjunto maestro
                        	                             }
                        	 else{

                        		                         	log_info(logger, "Conexion entrante del discriminador");
                        		                         	char *aux= malloc(2);
                        		                         	strcpy(aux,buf);
                        		                         	free(buf);
                        		                         	int tam=obtenerTamDelSigBuffer(i);
                        		                         	buf=malloc(tam);
                        		                         	recv(i, buf, tam, 0);
                        		                         	log_info(logger,"Se realizo el recv %s",buf);
                        		                         	switch(aux[0]){
                        		                         	case 'n':
                        		                         		send(i,sePuedeBloquear(buf),2,0);
                        		                         		break;
                        		                         	case 'v':
                        		                         		send(i,verificarClave(procesoEnEjecucion,buf),2,0);
                        		                         		break;
                        		                         	case 'b':
                        		                         		bloquear(buf);
                        		                         		break;
                        		                         	case 'l':
                        		                         		liberaClave(buf);
                        		                         		break;
                        		                         	}
                        		                         	log_info(logger,"Se realizo correctamente la comunicacion con el coordinador");
                        		                         	free(buf);
                        		                         	free(aux);
                        	 }
                         }
                         else { // ACA COMIENZA EL HANDSHAKE CON LOS ESIS

                        	 buf = malloc(2);
                             // gestionar datos de ESI
                             if ((nbytes = recv(i, buf, 2, 0)) <= 0) {
                                 // error o conexión cerrada por el cliente
                                 if (nbytes == 0) {
                                     // conexión cerrada
                                	 log_warning(logger, "El ESI se fue");
                                 } else {
                                	 tirarErrorYexit("Problema de conexion con el ESI");
                                 }
                                 close(i); // cierra socket
                                 FD_CLR(i, &master); // eliminar del conjunto maestro
                             } else {
                            	 log_info(logger, "Conexion entrante del ESI");
                               //aca hago un case de los posibles send de un esi, que son
                               //1.- termino ejecucion el esi y nos esta informando
                               Estado estado;
                               int tam;
                               switch(buf[0]){
                               case 'f':
                            	   terminarProceso();
                            	   sem_post(&sem_replanificar);
                            	   break;
                               case 'e':
                            	   tiempo_de_ejecucion++;
                            	   (*procesoEnEjecucion).rafagaRealActual++;
                            	   if(flag_desalojo && flag_nuevoProcesoEnListo){
                            		  sem_post(&sem_replanificar); flag_nuevoProcesoEnListo = 0;}
                            	   else sem_post(&sem_ESIejecutoUnaSentencia);
                            	   sem_post(&semCambioEstado);
                                   break;
                               case 'a':
                            	   tam = obtenerTamDelSigBuffer(i);
                            	   buf = realloc(buf,tam);
                            	   recv(i,buf,tam,0);
                            	   matarESI(transformarNumero(buf,0));
                            	   break;
                               }
                             }
                             free(buf);
                         }
                     }
             }
}

}
void main()
    {
	idGlobal=1;
	void(*miAlgoritmo)();
	t_config *config=config_create("/home/utnso/git/tp-2018-1c-UAL-masters/Config/Planificador.cfg");
	int estimacionInicial=config_get_int_value(config,"Estimacion inicial");
	char*algoritmo= config_get_string_value(config, "Algoritmo de planificacion");
	pthread_t hilo_planificadrCortoPlazo;
	pthread_t hilo_ejecutarEsi;
	pthread_t hilo_consola;
	if(!strcmp(algoritmo,"fifo")){
			miAlgoritmo=&fifo;
		flag_desalojo=0;
	}
	if(!strcmp(algoritmo,"SJF-CD")){
			miAlgoritmo=&sjf;
			flag_desalojo=1;
		}
	if(!strcmp(algoritmo,"SJF-SD")){
			miAlgoritmo=&sjf;
			flag_desalojo=0;
		}
	config_destroy(config);
	pthread_create(&hilo_planificadrCortoPlazo,NULL,planificadorCortoPlazo,miAlgoritmo);
	pthread_create(&hilo_ejecutarEsi,NULL,ejecutarEsi,NULL);
	pthread_create(&hilo_consola,NULL,(void *)consola,NULL);
	crearSelect(estimacionInicial);
	cerrarPlanificador();
    }
void listar(char* clave){
	claveABuscar=clave;
	Bloqueo *block=buscarClave();
	if(!list_is_empty((*block).bloqueados) || !(*block).idProceso){
		if(!(*block).idProceso)
					printf("Esta Clave fue bloqueada por config\n");
		else
					printf("Proceso con id %d posee a la clave\n",(*block).idProceso);

		int j=0;
		Proceso *proceso;
		while((proceso=list_get((*block).bloqueados,j))!=NULL){
			printf("Proceso con id %d esta en la lista de clave\n",(*proceso).idProceso);
			j++;
		}
		}
	else
		printf("No existe la clave\n");
}
void bloquearClavesIniciales(t_config *config){
	char ** claves;
	claves=config_get_array_value(config,"Claves inicialmente bloqueadas");
	char *aux=*(claves);
	int i=0;
	while(aux){
		bloquearPorID(aux,0);
		i++;
		aux=*(claves+i);
	}
}
void destruirUnBloqueado(void *elemento){
	Bloqueo *b=(Bloqueo *) elemento;
	list_destroy((*b).bloqueados);
	free((*b).clave);
	free(b);

}
void destruirUnProceso(void *elemento){
	Proceso *b=(Proceso *) elemento;
	free(b);
}
void cerrarPlanificador(){
	list_destroy_and_destroy_elements(bloqueados,&destruirUnBloqueado);
	list_destroy(listos);
	list_destroy(terminados);
	list_destroy_and_destroy_elements(procesos,&destruirUnProceso);
}
