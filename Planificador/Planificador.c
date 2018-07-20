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

void terminarProceso(){

		liberarRecursos((*procesoEnEjecucion).idProceso);
		(*procesoEnEjecucion).estado = finalizado;
		list_add(terminados,procesoEnEjecucion);
		procesoEnEjecucion = NULL;
		sem_post(&sem_ESIejecutoUnaSentencia);
		sem_post(&semCambioEstado);
	/*//ESte liberar recursos lo paso al cerrar socket
	//liberarRecursos((*procesoEnEjecucion).idProceso);
	(*procesoEnEjecucion).estado = finalizado;
	procesoEnEjecucion = NULL;
	sem_post(&sem_ESIejecutoUnaSentencia);
	sem_post(&semCambioEstado);
	*/
}

void *planificadorCortoPlazo(void *miAlgoritmo){//como parametro le tengo que pasar la direccion de memoria de mi funcion algoritmo
	t_log *log_planiCorto;
	Proceso*(*algoritmo)();
	algoritmo=(Proceso*(*)()) miAlgoritmo;
	// se tiene que ejecutar todo el tiempo en un hilo aparte
	while(1){
	logTest("esperando segnal de sem planificador");
	//hay que parar
	sem_wait(&sem_replanificar);

	//
	sem_wait(&sem_pausar);
	sem_post(&sem_pausar);
	//
	logTest("se obtuvo segnal de sem planificador");
	// aca necesito sincronizar para que se ejecute solo cuando le den la segnal de replanificar
	//
	Proceso *proceso; // ese es el proceso que va a pasar de la cola de ready a ejecucion
	proceso = (*algoritmo)();

	if(proceso){
	logImportante("Se selecciono un proceso ID %d por el algoritmo",(*proceso).idProceso);
	logTest("Esperando el semaforo sem fin de ejecucion");
	sem_wait(&sem_finDeEjecucion);
	logTest("Se paso el semaforo sem fin de ejecucion");
	(*proceso).estado=ejecucion;
	procesoEnEjecucion=proceso;
	logTest("Se asigno el proceso como proceso en ejecucion");

	flag_seEnvioSignalPlanificar=0;

	sem_post(&sem_procesoEnEjecucion);
	logTest("se dio segnal de ejecutar el esi en ejecucion");
	// el while de abajo termina cuando el proceso pasa a otra lista es decir se pone en otro estado que no sea el de ejecucion
	}
	}
}

void *ejecutarEsi(void *esi){
	t_log *log_ejecutarEsi;
	log_importante=log_create(logPlanificador,"Ejecutar ESI",1, LOG_LEVEL_INFO);
	while(1){
		logTest("Esperando al semaforo para ejecucion");
		sem_wait(&sem_procesoEnEjecucion);
		logTest("se entro a ejecutar el esi en ejecucion");
		while(procesoEnEjecucion && (*procesoEnEjecucion).estado==ejecucion){
			pthread_mutex_lock(&mutex_pausa);
			logTest("esperando semaforo de que el esi ejecuto una sentencia");
			sem_wait(&sem_ESIejecutoUnaSentencia);
			if(procesoEnEjecucion){
				logTest("pasando semaforo de esi ejecuto una sentencia");
				send((*procesoEnEjecucion).socketProceso,"1",2,0);// este send va a permitir al ESI ejecutar una sentencia
         	   tiempo_de_ejecucion++;
         	   (*procesoEnEjecucion).rafagaRealActual=(*procesoEnEjecucion).rafagaRealActual+1;
				logImportante("se envio al esi en ejecucion orden de ejecutar");
			}

			sem_wait(&semCambioEstado);
			pthread_mutex_unlock(&mutex_pausa);
		}
		//el proceso esi actual dejo de ser el que tiene que ejecutar
		sem_post(&sem_finDeEjecucion);
		logTest(log_ejecutarEsi, "se da segnal de fin de ejecucion");
	}
}
void planificadorLargoPlazo(int id,int estimacionInicial){
	Proceso *proceso=malloc(sizeof(Proceso));
	(*proceso).idProceso=idGlobal;
	(*proceso).socketProceso=id;
	(*proceso).estado=listo;
	(*proceso).estimacionAnterior=(float)estimacionInicial;
	(*proceso).rafagaRealActual=0;
	(*proceso).rafagaRealAnterior=0;
    logImportante("Ingreso un nuevo ESI con ID %d",(*proceso).idProceso);
	 list_add(procesos, proceso);
	meterEsiColaListos(proceso);
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
	if((*proc).rafagaRealActual){
		(*aux) = (*proc).estimacionAnterior - (*proc).rafagaRealActual;
		imprimirln(rojo,"remanente?");

	}
	else if((*proc).rafagaRealActual==0&&(*proc).rafagaRealAnterior==0){
		(*aux)=(*proc).estimacionAnterior;
	}
	else{
		//(*aux) = alfaPlanificador*((*proc).rafagaRealActual) +(1-alfaPlanificador)*((*proc).estimacionAnterior);
		(*aux) = alfaPlanificador*((*proc).rafagaRealAnterior) +(1-alfaPlanificador)*((*proc).estimacionAnterior);
	}
	imprimir(verde,"La estimacion del proceso con id %d es :",(*proc).idProceso);
	imprimirln(azul," %f",(*aux));
	logTest("La estimacion del proceso con id %d es : %f ",(*proc).idProceso,(*aux));
	return aux;
}
bool compararSJF(void *a,void *b){
	Proceso *primero=(Proceso *) a;
	Proceso *segundo=(Proceso *) b;
	float *af=estimarSJF(a);
	float *bf=estimarSJF(b);
	(*primero).estimacionAnterior=(*af);
	(*segundo).estimacionAnterior=(*bf);
	fflush(stdout);
	bool aux=(*af<=*bf);
	free(af);
	free(bf);
	return aux;
}
float* estimarHRRN(Proceso *proc){
	float *s;
	float *w=malloc(sizeof(float));
	s=estimarSJF(proc);
	(*w)=tiempo_de_ejecucion-(*proc).tiempo_que_entro;
	float *aux=malloc(sizeof(float));
	imprimirln(blanco,"Los valores del esi con id %d son : ",(*proc).idProceso);
	imprimir(verde,"S -> ");imprimirln(azul,"%f",*s);
	imprimir(verde,"W -> ");imprimirln(azul,"%f",*w);
	(*aux)=(float)1+(float)(*w)/(*s);
	imprimir(verde,"RR -> ");imprimirln(azul,"%f",*aux);
	logTest("La estimacion hrrn del proceso id %d es S: %f,W: %f,RR: %f",(*proc).idProceso,*s,*w,*aux);
	free(s);
	free(w);
	return aux;
}
bool compararHRRN(void *a,void *b){
	Proceso *primero=(Proceso *) a;
	Proceso *segundo=(Proceso *) b;
	float *af=estimarHRRN(a);
	float *bf=estimarHRRN(b);
	bool aux = (*af>=*bf);
	free(af);
	free(bf);
	return aux;
}
Proceso* obtenerSegunCriterio(bool (*comparar) (void*,void*)){
	t_list *aux=list_duplicate(listos);
	Proceso *proceso=NULL;
	if(list_get(aux,0)!=NULL){//REVISAR ES9TA SOLUCION PARA QUE NO MUERA EL PLANIFICADOR*******
			list_sort(aux,comparar); //CREO UNA LISTA AUXILIAR Y LO ORDENO
			proceso=list_get(aux,0);
			list_destroy(aux);
			idBuscar = (*proceso).idProceso;
			list_remove_by_condition(listos,&procesoEsIdABuscar); //ELIMINO EL PROCESO QUE COINCIDA CON TAL ID EN LA COLA DE LISTOS
	}
	return proceso;
}
Proceso *sjf(){
	return obtenerSegunCriterio(&compararSJF);
}
Proceso *hrrn(){
	return obtenerSegunCriterio(&compararHRRN);
}
//Programas de Busqueda


bool contieneAlProceso(void *a){//me dice si un bloque contiene al proceso
	Bloqueo *b=(Bloqueo*) a;
	//EL PROBLEMA ESTA ACA
	return list_any_satisfy((*b).bloqueados,&procesoEsIdABuscar);
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
		if(id){
			(*block).idProceso=(*proceso).idProceso;
			logImportante("El ID %d,tomo la clave %s",(*proceso).idProceso,(*block).clave);
		}
		else
			(*block).idProceso=0;
		list_add(bloqueados,block);
	}
	else{
		if((*block).idProceso==-1){
			(*block).idProceso=(*proceso).idProceso;
			logImportante("El ID %d,tomo la clave %s",(*proceso).idProceso,(*block).clave);
		}
		else{
			logImportante("El ID %d se metio a la cola de la clave %s",(*proceso).idProceso,(*block).clave);
			list_add((*block).bloqueados,proceso);
			(*proceso).estado = bloqueado;
		}
	}
}
//Necesito mutex al usar esta funcion
void bloquearPorConsola(char *clave,int id){
	char *aux=malloc(strlen(clave)+1);
	strcpy(aux,clave);
	claveABuscar=aux;
	Bloqueo *block=buscarClave();
	idBuscar=id;
	Proceso *proceso=buscarProcesoPorId(id);
	if((*proceso).estado==ejecucion||(*proceso).estado==listo){
	logTest("proceso listo o en ejecucion",clave);

	if(!block){
		logTest(logger,"El bloque con clave %s NO EXISTE",clave);
			block=malloc(sizeof(Bloqueo));
			(*block).clave=aux;
			(*block).bloqueados=list_create();
			(*block).idProceso=-1;
			list_add(bloqueados,block);
		logTest(logger,"El bloque con clave %s se CREO",clave);
		}
	//Esto no se si va, me fijo si el proceso ya esta bloqueado por esta clave asi
	//no lo vuelvo a agregar a la cola de bloqueados
	else{ //<---- Este else me dice que el block no es null que existe entonces voe si el proceso ya esta bloqueado
		logTest(logger,"El bloque con clave %s EXISTE",clave);
		//Si encuentra un proceso que coincide con el id a buscar quiere decir que el proceso esta en la lista de bloqueados
		if(list_find((*block).bloqueados,&procesoEsIdABuscar)){
			logImportante("Se intento bloquear el proceso que YA ESTABA bloqueado por eso clave");
			return; // osea no lo agrego
		}
	}
	(*proceso).estado=bloqueado;
	list_add((*block).bloqueados,proceso);
	logImportante("El Proceso esta en la cola de bloqueados de la clave %s",clave);
	}
	else
		logImportantec("El proceso no se encontraba en listo o ejecucion");
}
//jiava
void bloquear(char *clave){//En el hadshake con el coordinador asignar proceso en ejecucion a proceso;
	char *aux=malloc(strlen(clave)+1);
	strcpy(aux,clave);
	claveABuscar=aux;
	Bloqueo *block=buscarClave();
	if(!block){
		logTest("La clave no existe se va a crear la cola de bloqueados de la clave %s",clave);
		block=malloc(sizeof(Bloqueo));
		(*block).clave=aux;
		logTest("la clave que se bloqueo es %s",(*block).clave);
		(*block).bloqueados=list_create();
		(*block).idProceso=(*procesoEnEjecucion).idProceso;
		logTest("el id que bloqueo la clave %s es %d",(*block).clave,(*block).idProceso);
		list_add(bloqueados,block);
	}
	else{
		logTest("La clave %s existe",clave);

		if((*block).idProceso==-1){
			logTest("Pero se puede usar");
			(*block).idProceso=(*procesoEnEjecucion).idProceso;
		}
		else{

			logTest("No se puede usar, se agrega a la cola de bloqueados");
			if(procesoEnEjecucion){
				(*procesoEnEjecucion).rafagaRealAnterior=(*procesoEnEjecucion).rafagaRealActual;
				(*procesoEnEjecucion).rafagaRealActual=0;
			}
			(*procesoEnEjecucion).estado = bloqueado;
			list_add((*block).bloqueados,procesoEnEjecucion);
			procesoEnEjecucion = NULL;
			sem_post(&sem_ESIejecutoUnaSentencia);
			sem_post(&semCambioEstado);
			//revisar este semaforo
			if(list_get(listos,0)!=NULL)//SI NO ESTA VACIA LA LISTA DE DE LISTOS
				enviarSegnalPlanificar(); //REPLANIFICO CUANDO UN PROCESO SE VA A LA COLA DE BLOQUEADOS!
		}
		free(aux);
	}
}
void aplicacion(void *a){
	Bloqueo *block=(Bloqueo *) a;
	if((*block).idProceso==idBuscar){
				imprimir(azul,",%s",(*block).clave);
				send(socketCoordinador,"l",2,0);
				enviarCantBytes(socketCoordinador,(*block).clave);
				send(socketCoordinador,(*block).clave,strlen((*block).clave)+1,0);
				liberaClave((*block).clave);
	}
	else{
				list_remove_by_condition((*block).bloqueados,&procesoEsIdABuscar);
			}
}
void liberarRecursos(int id){
	Bloqueo *block;
	idBuscar=id;
	imprimir(verde,"Liberando claves recurso id %d :",id);
	list_iterate(bloqueados,aplicacion);
	printf("\n");
}

void liberaClave(char *clave){
	logTest("Se entro a liberar clave");
	claveABuscar=clave;
	Bloqueo *block=buscarClave();
	if(block){
		logTest("Se encontro la clave %s",clave);
		if(list_get((*block).bloqueados,0)){
			logTest("La clave %s tiene procesos bloqueados",clave);
				Proceso *proceso=list_remove((*block).bloqueados,0);
				logTest("Se removio el primer elemento de la lista de bloqueados");
				if(list_get((*block).bloqueados,0)==NULL){
					logTest("la clave %s NO POSEE elementos bloqueados",clave);
					claveABuscar=clave;
					list_remove_by_condition(bloqueados,&esIgualAClaveABuscar);
					destruirUnBloqueado(block);
				}
				else
				(*block).idProceso=-1;
				//(*proceso).estado=listo;
				meterEsiColaListos(proceso);
			}
			else{
				logTest("La clave %s NO tiene procesos bloqueados",clave);
				claveABuscar=clave;
				list_remove_by_condition(bloqueados,&esIgualAClaveABuscar);
				destruirUnBloqueado(block);
			}
	}
	else
		logTest("NO se encontro la clave %s",clave);


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
	logTest("la clave es %s con id %d",(*block).clave,(*block).idProceso);
	if(block && (*block).idProceso==(*proceso).idProceso)
		return "1";
	else
		return "0";
}


void tirarErrorYexit(char* mensajeError) {
	log_error(logger, mensajeError);
	log_destroy(logger);
	exit(-1);
}
void sendFinaliza(int id){
	idBuscar=id;
	Proceso* procesoAEliminar = list_find(procesos,&procesoEsIdABuscar);
	if(procesoAEliminar==NULL||(*procesoAEliminar).estado==finalizado)
		imprimir(rojo,"Error ingreso un id invalido");
	else
		send((*procesoAEliminar).socketProceso,"f",2,0);
}
void matarESI(int id){
	idBuscar=id;
	list_remove_by_condition(listos,&procesoEsIdABuscar);
	liberarRecursos(id);
	Proceso* procesoAEliminar = list_find(procesos,&procesoEsIdABuscar);
	(*procesoAEliminar).estado=finalizado;
	list_add(terminados,procesoAEliminar);
	/*
	if((*procesoEnEjecucion).idProceso==id){
		if(list_get(listos,0)!=NULL){
			enviarSegnalPlanificar();
		}
	}
	liberarRecursos(id);
	idBuscar = id;
	Proceso* procesoAEliminar = list_remove_by_condition(procesos,&procesoEsIdABuscar);
	free(procesoAEliminar);
	*/
}
void realizarStatus(char* nombreInstancia){
	imprimir(verde,"Instancia: ");
	imprimirln(azul,"%s",nombreInstancia);
	int tam = obtenerTamDelSigBuffer(socketCoordinador);
	char* buf=malloc(tam);
	recv(socketCoordinador,buf,tam,0);
	imprimir(verde,"Valor: ");
	imprimirln(azul,"%s",buf);
	free(buf);
}
void crearSelect(int estimacionInicial){// en el caso del coordinador el pathYoCliente lo pasa como NULL
	log_importante=log_create(logPlanificador,"Conexion",1, LOG_LEVEL_INFO);
	procesoEnEjecucion = NULL;
	 int listener;
	 t_config *config=config_create(pathPlanificador);

	 char* buf;
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
		 logImportante("Se creo socket de servidor");
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
    	socketCoordinador=casoDiscriminador;
    	send(casoDiscriminador,"p",2,0);
    	logImportante("Se establecio comunicacion con el coordinador como cliente");
     config_destroy(config); // SI NO HAY ERROR SE DESTRUYE FINALMENTE EL CONFIG
     if (listen(listener, 10) == -1){
    	 tirarErrorYexit("No se pudo escuchar");
     }
     else
    	 logTest("En espera de conexion");

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
                	 logTest("Hay informacion en el select");
                 // explorar conexiones existentes en busca de datos que leer
                 for(i = 0; i <= fdmax; i++) {
                     if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
                         if (i == listener) {
                        	 logImportante("Conexion entrante de un nuevo ESI");
                             // gestionar nuevas conexiones
                             addrlen = sizeof(their_addr);
                             if ((nuevoCliente = accept(listener, (struct sockaddr *)&their_addr,
                                                                      &addrlen)) == -1) {

                             } else {
                                 FD_SET(nuevoCliente, &master); // añadir al conjunto maestro
                                 if (nuevoCliente > fdmax) {    // actualizar el máximo
                                     fdmax = nuevoCliente;
                                 }
                                 planificadorLargoPlazo(nuevoCliente,estimacionInicial);
                                 /*if(procesoEnEjecucion==NULL){ //SI ES NULL SIGNIFICA QUE NO HAY NADIE EN EJECUCION.
                                	 log_info(logger,"Se decidio replanificar :D");
                                	 sem_post(&sem_replanificar);
                                	 flag_nuevoProcesoEnListo = 0; //COMO YA METI UN NUEVO PROCESO A EJECUCION NO HACE FALTA QUE REPLANIFIQUE EN CASO DE DESALOJO
                                 }*/

                             }
                         }
                         else if(i==casoDiscriminador){ //aca trato al coordinador//Caso discriminador
                        	 buf = malloc(2);
                        	 if ((nbytes = recv(i, buf, 2, 0)) <= 0) {

                        	                                 // error o conexión cerrada por el cliente
                        		 if (nbytes == 0) {
                        	                                     // conexión cerrada
                        	                             	 logTest("El coordinador se fue");
                        	                                     cerrarPlanificador();
                        	                                 } else {
                        	                                	 logTest("Error conexion con el coordinador");
                        	                                     perror("recv");
                        	                                 }
                        	                                 close(i); // cierra socket
                        	                                 FD_CLR(i, &master); // eliminar del conjunto maestro
                        	                             }
                        	 else{

                            	 	 	 	 	 	 	 	logTest("Conexion entrante del coordinador");
                        		                         	char *aux= malloc(2);
                        		                         	strcpy(aux,buf);
                        		                         	free(buf);
                        		                         	int tam=obtenerTamDelSigBuffer(i);
                        		                         	buf=malloc(tam);
                        		                         	recv(i, buf, tam, 0);
                        		                         	logTest("Se realizo el recv %s",buf);

                        		                         	switch(aux[0]){
                        		                         	case 'n':
                        		                         		logTest("Se decidio verificar un GET de la clave %s",buf);
                        		                         		send(i,sePuedeBloquear(buf),2,0);
                        		                         		break;
                        		                         	case 'v':
                        		                         		logTest("Se decidio verificar un SET o STORE de la clave %s",buf);
                        		                         		send(i,verificarClave(procesoEnEjecucion,buf),2,0);
                        		                         		break;
                        		                         	case 'b':
                        		                         		logTest("Se decidio bloquear la clave %s",buf);
                        		                         		bloquear(buf);
                        		                         		break;
                        		                         	case 'l':
                        		                         		logTest("Se decidio liberar la clave %s",buf);
                        		                         		liberaClave(buf);
                        		                         		break;
                        		                         	//CASE PARA ESTATUS
                        		                         	case 's':
                        		                         		logTest("Se decidio obtener el status %s",buf);
                        		                         		realizarStatus(buf);

                        		                         		break;

                        		                         	}
                        		                         	logTest("Se realizo correctamente la comunicacion con el coordinador");
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
                                     // conexión cerrada y liberacion de recursos
                                	 idBuscar=i;
                                	 Proceso *proc_finalizado=list_find(procesos,&procesoEsIdABuscarSocket);
                                	 liberarRecursos((*proc_finalizado).idProceso);
                                	 (*proc_finalizado).estado=finalizado;
                                	 list_add(terminados,proc_finalizado);
                                	 //log_warning(logger, "El ESI se fue y se liberaron sus recursos");*/
                                	 logImportante("El ESI se FUE con ID %d",(*proc_finalizado).idProceso);
                                 } else {
                                	 printf("%d",nbytes);
                                	 tirarErrorYexit("Problema de conexion con el ESI");
                                 }
                                 close(i); // cierra socket
                                 FD_CLR(i, &master); // eliminar del conjunto maestro
                             } else {
                            	 logTest("Conexion entrante del ESI");
                               //aca hago un case de los posibles send de un esi, que son
                               //1.- termino ejecucion el esi y nos esta informando
                               Estado estado;
                               int tam;
                               switch(buf[0]){
                               case 'f':
                            	   if(procesoEnEjecucion!=NULL&&(*procesoEnEjecucion).socketProceso==i){
                                	   terminarProceso();
                                	  // sem_post(&sem_replanificar);
                                	   if(list_get(listos,0)!=NULL)
                                		   //envia solo la signal si no hay mas procesos para planificar
                                		   enviarSegnalPlanificar();
                            	   }
                            	   else{
                            		   idBuscar=i;
                            		   Proceso *proc_finalizado=list_find(procesos,&procesoEsIdABuscarSocket);
                            		   matarESI((*proc_finalizado).idProceso);
                            	   }
                            	   close(i); // cierra socket
                            	   FD_CLR(i, &master); // eliminar del conjunto maestro
                            	   break;
                               case 'e':
                            	   if(flag_quierenDesalojar&&flag_desalojo){
                            		   //ver la linea de abajo SOLUCION VAGA NO ME PARECE QUE DEBA ESTAR
                            		   flag_quierenDesalojar=0;
                            		   meterEsiColaListos(procesoEnEjecucion);
                            		   //ESTO ES PARTE DE LA SOLUCION VAGA
                            		   //enviarSegnalPlanificar();
                            	   }
                            	   sem_post(&sem_ESIejecutoUnaSentencia);
                            	   sem_post(&semCambioEstado);
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
	flag_seEnvioSignalPlanificar=0;
	flag_quierenDesalojar=0;
	//
    procesos=list_create();
	listos=list_create();
    terminados=list_create();
    bloqueados=list_create();
	//
	log_test=log_create(logPlanificador,"Plani_test",1, LOG_LEVEL_INFO);
	log_importante=log_create(logPlanificador,"Planificador",1, LOG_LEVEL_INFO);
	//
	sem_init(&sem_replanificar,0,0);
	sem_init(&sem_procesoEnEjecucion,0,0);
	sem_init(&sem_ESIejecutoUnaSentencia,0,1);
	sem_init(&sem_finDeEjecucion,0,1);
	sem_init(&semCambioEstado,0,0);
	sem_init(&sem_pausar,0,1);
	//
	pthread_mutex_init(&mutex_pausa,NULL);
	idGlobal=1;
	tiempo_de_ejecucion=0;
	//
	Proceso*(*miAlgoritmo)();
	t_config *config=config_create(pathPlanificador);
	bloquearClavesIniciales(config);
	//
	logTest("Se creo archivo config");
	int estimacionInicial=config_get_int_value(config,"Estimacion inicial");
	alfaPlanificador=(float)config_get_int_value(config,"Alfa planificacion")/100;
	logImportante("El alfa de planificacion es %d",alfaPlanificador);
	logImportante("La estimacion inicial es : %d",estimacionInicial);
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
	if(!strcmp(algoritmo,"HRRN")){
				miAlgoritmo=&hrrn;
				flag_desalojo=0;
			}
	logImportante("Se asigno el algoritmo %s",algoritmo);
	config_destroy(config);
	pthread_create(&hilo_planificadrCortoPlazo,NULL,planificadorCortoPlazo,(void *)miAlgoritmo);
	logTest("Se creo el hilo planificador CORTO PLAZO");
	pthread_create(&hilo_ejecutarEsi,NULL,ejecutarEsi,NULL);
	logTest("Se creo hilo para ejecutar ESIS");
	pthread_create(&hilo_consola,NULL,(void *)consola,NULL);
	logTest("Se creo hilo para la CONSOLA");
	crearSelect(estimacionInicial);
	//cerrarPlanificador();
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
		logImportantec("Bloqueando Clave inicial %s",aux);
		free(aux);
		i++;
		aux=*(claves+i);
	}
	free(claves);
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
//*****************Algoritmo de banquero
//AlgunoCumple
bool esIgualAlIndixeABuscar(void *i){
	int *index=(int*) i;
	return (*index)==idBanquero;
}
bool estaElProceso(t_list *a,int index){
	if(list_get(a,0)==NULL)
		return false;
	else
	{
		idBanquero=index;
		return list_any_satisfy(a,&esIgualAlIndixeABuscar);
	}
}
//
//Funciones de matriz
bool esBloqueCero(void *a){
	Bloqueo *block=(Bloqueo *) a;
	if((*block).idProceso){
				return false;
	}
	else{
				return true;
			}
}
bool estaElProcesoZero(){
	return list_any_satisfy(bloqueados,&esBloqueCero);
}
//algoritmoBanquerro 2*******************
int cantidadDeFilasProcesos(){
	int filas = list_size(procesos);
	if(estaElProcesoZero()){
		filas++;
	}
	return filas;
}
int cantidadColumasClaves(){
	int columnas= list_size(bloqueados);
	return columnas;
}
//DISCRIMINANTES MATRIZES si el index es negativo se trata del caso 0
bool loPosee(int indexProceso,int indexClave){
	Bloqueo *block=list_get(bloqueados,indexClave);
	if(indexProceso<0){
		return 0==(*block).idProceso;
	}
	else{

		Proceso *proceso=list_get(procesos,indexProceso);
		return ((*proceso).idProceso)==((*block).idProceso);
	}
}
bool noLoPosee(int indexProceso,int indexClave){
	Bloqueo *block=list_get(bloqueados,indexClave);
	if(indexProceso<0){
		return (0!=(*block).idProceso);
	}
	else{
		Proceso *proceso=list_get(procesos,indexProceso);
		return (*proceso).idProceso!=(*block).idProceso;
	}
}
bool estaBloqueado(int indexProceso,int indexClave){
	if(indexProceso<0){
			return false;
	}
	else{
		Bloqueo *block=list_get(bloqueados,indexClave);
		Proceso *proceso=list_get(procesos,indexProceso);
		idBuscar = (*proceso).idProceso;
		return contieneAlProceso(block);
	}
}
//
int **dameMatriz(bool(*discriminante)(int,int)){//el discriminante es el encargado de rellenar los valores de la matriz
	//Ver como refactorizar estos 2 enteros
	int filas=cantidadDeFilasProcesos();
	int columnas=cantidadColumasClaves();
	int cantidadProcesos=list_size(procesos);
	//
	//ASigno espacio
	int **matriz = (int **)malloc(filas * sizeof(int *));
	    for (int i=0; i<filas; i++){
	    	matriz[i] = (int *)malloc(columnas * sizeof(int));
	    }

	 //meto valores
	 for (int i = 0; i <  cantidadProcesos ; i++){
		  for (int j = 0; j < columnas; j++){
			  matriz[i][j] = discriminante(i,j) ;
		  }
	 }
	 if(estaElProcesoZero()){
		 for (int j = 0; j < columnas; j++)
			  matriz[filas-1][j] = discriminante(-1,j) ;
	 }
	 return matriz;
}
void imprimirMatriz(int **a,int filas,int columnas){
	 for (int i = 0; i < filas; i++){
			  for (int j = 0; j < columnas; j++){
				  if(a[i]==NULL){
				  }

				  printf("%d " ,a[i][j]);
			  }
			  printf("\n");
}

}
//DISCRIMINANTES VECTORES
bool total(int index){
	return true;
}
bool actual(int index){
	Bloqueo *block = list_get(bloqueados,index);
	return ((*block).idProceso==-1);//Esta libre
}
//
int *dameVector(bool (*discriminante) (int),int elementos){
	int *vector = (int *)malloc(elementos * sizeof(int));
	  for (int i = 0; i < elementos; i++)
		            vector[i] = discriminante(i);
	 return vector;
}
//Comparadores
bool elementoMenorOIgual(int a,int b){
	return a<=b;
}
bool elementoIgual(int a,int b){
	return a==b;
}
bool compararElementosVectores(int *a,int *b,bool (*comparador) (int,int),int cantidadElementosAComparar){
	for(int i=0;i<cantidadElementosAComparar;i++){
		if(!comparador(a[i],b[i]))//Si no se cumple la condicion de comparacion para algun elemento retorno falso
			return false;
	}
	return true;

}
//Funciones de manejo de indices
void sumarVectores(int *a,int *b,int cantidadElementosAComparar){
	for(int i=0;i<cantidadElementosAComparar;i++){
			a[i]=a[i]+b[i];

		}
}
//Para obtener el mejor caso
int dameLaNormaInfinita(int *a,int cantElementos){
	int aux=0;
	for(int i=0;i<cantElementos;i++){
				aux=aux+a[i];
			}
	return aux;
}
int *dameElMejor(t_list *indicesQueCumplen,int **matrizDeAsignados,int cantidadColumnas){
	int aux=-1;
	int *auxIndice;
	for(int i=0;i<list_size(indicesQueCumplen);i++){
		int *indice=list_get(indicesQueCumplen,i);
		imprimir(blanco,"la norma es %d",dameLaNormaInfinita(matrizDeAsignados[(*indice)],cantidadColumnas));
		int aux2 = dameLaNormaInfinita(matrizDeAsignados[(*indice)],cantidadColumnas);
		if(aux2>aux){
			aux=aux2;
			auxIndice=indice;
		}
		else
			free(indice);
	}
	return auxIndice;
}
//
bool retieneAlgo(int index,int**matrizDeAsignados,int columnas){
	int *vec = matrizDeAsignados[index];
	for(int i=0;i<columnas;i++){
		if(vec[i]!=0)
			return true;
	}
	return false;
}
void imprimirVector(int *a,int columnas){
	for(int i=0;i<columnas;i++){
		printf("%d ",a[i]);
	}
	printf("\n ");
}
t_list *algoritmoBanquero(){//devuelve lista de indices de procesos en deadlock
	int filas=cantidadDeFilasProcesos();
	int columnas=cantidadColumasClaves();
	int **matrizDeAsignados=dameMatriz(&loPosee);//retencion
	//int **matrizDeNecesidad=dameMatriz(&noLoPosee);
	int **matrizDeNecesidad=dameMatriz(&estaBloqueado);
	imprimirMatriz(matrizDeAsignados,filas,columnas);
	imprimirMatriz(matrizDeNecesidad,filas,columnas);
	int *vectorRecursosTotales=dameVector(&total,columnas);
	int *vectorRecursosActuales=dameVector(&actual,columnas);
	imprimirVector(vectorRecursosTotales,columnas);
	imprimirVector(vectorRecursosActuales,columnas);
	//INDICES
	t_list *indicesQueCumplen=list_create();
	t_list *indicesDescartados=list_create();
	t_list *indicesProcesosQueEstanEnDeadlock=list_create();
	for (int j=0;j<filas;j++){
		for(int i=0;i<filas;i++){
			int *aux=malloc(sizeof(int));
			(*aux)=i;
			if(!estaElProceso(indicesDescartados,i)&&
					compararElementosVectores(matrizDeNecesidad[i],vectorRecursosActuales,&elementoMenorOIgual,columnas)){
					list_add(indicesQueCumplen,aux);
			}
		}
		//
		if(list_get(indicesQueCumplen,0)==NULL){
			//No se encontraron filas que cumplan con la condicion
			//REFACTORIZAR ESTA PARTE
			imprimir(azul,"yupi me fui");
			for(int k=0;k<filas;k++){
				int *aux=malloc(sizeof(int));
				(*aux)=k;
				if(!estaElProceso(indicesDescartados,k)&&
						retieneAlgo(k,matrizDeAsignados,columnas)){
					list_add(indicesProcesosQueEstanEnDeadlock,aux);
				}
				else
					free(aux);
			}
			//Libero memoria ocupada por el algoritmo
			eliminarMatriz(matrizDeNecesidad,filas);
			eliminarMatriz(matrizDeAsignados,filas);
			free(vectorRecursosTotales);
			free(vectorRecursosActuales);
			list_destroy_and_destroy_elements(indicesQueCumplen,&destruirEntero);
			list_destroy_and_destroy_elements(indicesDescartados,&destruirEntero);
			return indicesProcesosQueEstanEnDeadlock;
			//
		}
		else{
			imprimir(azul,"encontre >OOOO");
				int *elMejor=dameElMejor(indicesQueCumplen,matrizDeAsignados,columnas);
				//
				imprimir(azul,"xxxxxxxxxx");
				list_add(indicesDescartados,elMejor);
				imprimir(azul,"zzzzzzzzzzzzz");
				list_clean(indicesQueCumplen);
				imprimir(azul,"ttttttttttttttt");
				imprimirVector(matrizDeAsignados[(*elMejor)],columnas);
				imprimir(azul,"ssssssssssssss");
				sumarVectores(vectorRecursosActuales,matrizDeAsignados[(*elMejor)],columnas);
				imprimir(azul,"aaaaaaaaaaaaa");
			}
	}
	//REVISAR ESTA PARTE
	if(!compararElementosVectores(vectorRecursosActuales,vectorRecursosTotales,&elementoMenorOIgual,columnas)){
		imprimir(azul,"asdasdqseqwewqeqw");
		for(int k=0;k<filas;k++){
					int *aux=malloc(sizeof(int));
					(*aux)=k;
					if(!estaElProceso(indicesDescartados,k)&&retieneAlgo(k,matrizDeAsignados,columnas)){
						list_add(indicesProcesosQueEstanEnDeadlock,aux);
					}
					else
						free(aux);
				}
	}
	//Libero memoria ocupada por el algoritmo
	eliminarMatriz(matrizDeNecesidad,filas);
	eliminarMatriz(matrizDeAsignados,filas);
	free(vectorRecursosTotales);
	free(vectorRecursosActuales);
	list_destroy_and_destroy_elements(indicesQueCumplen,&destruirEntero);
	list_destroy_and_destroy_elements(indicesDescartados,&destruirEntero);
	return indicesProcesosQueEstanEnDeadlock;
}
//Planificacion
void enviarSegnalPlanificar(){
	//si todavia no se envio la signal
	if(!flag_seEnvioSignalPlanificar){
		flag_seEnvioSignalPlanificar=1;
		sem_post(&sem_replanificar);
	}
}
void meterEsiColaListos(Proceso *proceso){
	list_add(listos,proceso);

if((*proceso).estado==bloqueado&&flag_desalojo==1){
	if(procesoEnEjecucion!=NULL){
		//tengo que esperar a que finalice su sentencia
		flag_quierenDesalojar=1;
	}
	//si estaba bloqueado y lo quiero agregar fue porque se desbloqueo
	enviarSegnalPlanificar();
}
else if((*proceso).estado==ejecucion){
	//el proceso en ejecucion fue desalojado por algun otro proceso
	//no tengo que replanificar
}
//aca quiere decir que es proceso nuevo
else{
	//llego proceso nuevo y esta habilitado el desalojo entonces replanifico
	if(flag_desalojo==1){
		if(procesoEnEjecucion!=NULL){
			//tengo que esperar a que finalice su sentencia
			flag_quierenDesalojar=1;
		}
		enviarSegnalPlanificar();
	}
}
(*proceso).tiempo_que_entro=tiempo_de_ejecucion;
(*proceso).estado=listo;
if(procesoEnEjecucion==NULL){
	enviarSegnalPlanificar();
}
}

//
void deadlock(){
	t_list *elementos=algoritmoBanquero();
	if(list_get(elementos,0)==NULL){
		imprimir(rojo,"NO HAY DEADLOCK");
	}
	else{
		for(int i=0;i<list_size(elementos);i++){
			int *index=list_get(elementos,i);
			Proceso *proceso=list_get(procesos,(*index));
			imprimir(rojo,"El proceso de id : %d esta en deadlock",(*proceso).idProceso);
		}
	}
	list_destroy_and_destroy_elements(elementos,&destruirEntero);
}
//
void mostrarProcesos(void *a){
	Proceso *proceso=(Proceso*) a;
	imprimir(azul,"El proceso de id : %d",(*proceso).idProceso);
}
void status(char *clave){
	claveABuscar=clave;
	Bloqueo *block=buscarClave();
	if(block){
		imprimir(azul,"Proceso bloqueados por esta clave : ");
		if(list_get((*block).bloqueados,0)==NULL)
			imprimir(rojo,"ninguno");
		else{
			list_iterate((*block).bloqueados,&mostrarProcesos);
		}
	}
	//COORDINADOR
	send(socketCoordinador,"s",2,0);
	enviarCantBytes(socketCoordinador,clave);
	send(socketCoordinador,clave,strlen(clave)+1,0);

}
//liberar memoria

void eliminarMatriz(int **a,int fila){
	for(int i=0;i<fila;i++){
		free(a[i]);
	}
	free(a);
}
void destruirEntero(void *a){
	int *b = (int*) a;
	free(b);

}
