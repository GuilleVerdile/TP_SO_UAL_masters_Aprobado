#include "Coordinador.h"
t_list* instancias;
t_list* semaforosInstancias;
int cantidadDeInstancias;
sem_t esperaInicializacion;
sem_t semaforoEsi;
t_esi_operacion paqueteAEnviar;
int operacionValida;
sem_t semaforoLiberar;
instancia* (*algoritmoDeDistribucion)();
char* operacion;
char* claveAComunicar;
char* errorMensajeInstancia;
int socketPlanificador;
t_log* loggerReal;
int main(int argc, char**argv){
	mkdir("../Logs/", 0777); // creo carpeta Logs
	sem_init(&esperaInicializacion,0,0);
	sem_init(&semaforoEsi,0,0);
	sem_init(&semaforoLiberar,0,0);
	cantidadDeInstancias =0;
	semaforosInstancias = list_create();
	int listener;
	struct sockaddr_in their_addr; // datos cliente
    if(argc > 1)
    	pathCoordinador = argv[1];
	loggerReal = log_create(logCoordinador,"Coordinador",1,LOG_LEVEL_INFO);
	logger =log_create(logCoordinador,"CoordinadorTest",0, LOG_LEVEL_INFO);
	t_config *config = config_create(pathCoordinador);
	if((listener=crearConexionServidor(config_get_int_value(config, "Puerto"),config_get_string_value(config, "Ip")))==-1){
		log_error(logger, "No se pudo crear el socket servidor");
		config_destroy(config);
		log_destroy(logger);
		return -1;
	}
		config_destroy(config);
	log_info(logger, "Se creo el socket de Servidor");
	int nuevoCliente;        // descriptor de socket de nueva conexión aceptada
	int addrlen;

	if (listen(listener, 10) == -1){ //ESCUCHA!
	    log_error(logger, "No se pudo escuchar");
		log_destroy(logger);
		return -1;
	}
	log_info(logger, "Se esta escuchando");
	addrlen = sizeof(their_addr);
	pthread_t hiloPlanificador;
	char* buff= malloc(2);
	log_info(loggerReal,"Esperando la conexion con el planificador");
	while(1){
	if((socketPlanificador = accept(listener, (struct sockaddr *)&their_addr,&addrlen))==-1){ //NOS CONECTAMOS CON EL PLANIFICADOR
	    log_error(logger, "No se aceptar la conexion");
	    log_destroy(logger);
	    return -1;
	}
		recv(socketPlanificador,buff,2,0);
		if(buff[0]=='p'){
			log_info(loggerReal, "Se acepto la conexion");
			free(buff);
			break;
		}
		log_warning(loggerReal, "No es el planificador esperando devuelta la conexion");
		close(socketPlanificador);
	}
	instancia* instanciaABuscar;
	pthread_create(&hiloPlanificador,NULL,conexionPlanificador,NULL);
	algoritmoDeDistribucion = obtenerAlgoritmoDistribucion(); //OBTENGO EL ALGORITMO DE DISTRUBUCION SEGUN EL ARCHIVO CONFIG
	pthread_t* hilosInstancias = NULL;
	pthread_t hiloEsi = 0;
	instancias = list_create();
	//ACA COMIENZA LO DIVERTIDO =)
	sem_wait(&esperaInicializacion);
	while((nuevoCliente = accept(listener, (struct sockaddr *)&their_addr,&addrlen))) //Esperamos a que llegue la primera conexion
	{
		log_info(logger,"Se acepto una nueva conexion");
		buff = malloc(2);
		int recValor = recv(nuevoCliente, buff, 2, 0);
		int tipoCliente = 2;
		(recValor == -1)? exit(-1):((recValor == 0 )? close(nuevoCliente):(tipoCliente = transformarNumero(buff,0))); // borre funcion esEsi y uso operador ternario
		free(buff);
		switch(tipoCliente){
		case 1:
			log_info(logger,"El cliente es ESI");
			if((pthread_create(&hiloEsi , NULL , conexionESI, (void*)&nuevoCliente)) < 0) //HAY UN HILO QUE VA ATENDER LA CONEXION CON EL ESI
	    	{
				log_error(logger,"No se pudo crear un hilo");
				return -1;
	    	}
			sem_wait(&esperaInicializacion);
			log_info(logger,"Se asigno una conexion con hilos al ESI");
			pthread_join(hiloEsi,NULL);
			break;
		case 0:
			log_info(logger,"El cliente es INSTANCIA");
			hilosInstancias = realloc(hilosInstancias,sizeof(pthread_t)*(cantidadDeInstancias+1));
			if(pthread_create(&(hilosInstancias[cantidadDeInstancias]) , NULL , conexionInstancia, (void*)&nuevoCliente) < 0) //VA HABER UN HILO POR CADA INSTANCIA
	    	{
				log_error(logger,"No se pudo crear un hilo");
				return -1;
	    	}
			sem_wait(&esperaInicializacion); //ESPERA A QUE PRIMERO SE TERMINE LA INICIALIZACIONDE LA INSTANCIA QUE SE CONECTO.
			cantidadDeInstancias++; //EL VALOR DE LA CANTIDAD DE INSTANCIAS ME SIRVE TAMBIEN PARA CONOCER EL NUMERO DE SEMAFORO QUE LE TOCO A CADA INSTANCIA
			log_info(logger,"Se asigno una conexion con hilos a la instancia");
			break;
		}
	}
	return 0;
}

instancia* realizarSimulacion(){
	t_list* instanciaAux = list_duplicate(instancias);
	instancia* instancia = algoritmoDeDistribucion(NULL,instanciaAux);
	list_destroy(instanciaAux);
	return instancia;
}

void* conexionPlanificador(){
	sem_post(&esperaInicializacion);
	char* buff = malloc(2);
	int tam;
	char* clave;
	instancia* instanciaAEnviar;
	while(recv(socketPlanificador,buff,2,0)>0){
		log_info(logger,"Recibi un paquete");
		if(buff[0]=='1' || buff[0]=='0'){
			operacionValida = buff[0]-48;
			sem_post(&semaforoEsi);
		}
		switch(buff[0]){
			case'l':
				tam = obtenerTamDelSigBuffer(socketPlanificador);
				clave = malloc(tam);
				recv(socketPlanificador,clave,tam,0);
				log_info(logger,"La clave a liberar es: %s",clave);
				instanciaAEnviar = buscarInstancia(clave);
				if(instanciaAEnviar){
	    		liberarClave(instanciaAEnviar,clave);
	    		operacion = "l";
	    		claveAComunicar=clave;
	    		log_info(logger,"Vamos a liberar la instancia con el semaforo %d",(*(*instanciaAEnviar).nroSemaforo));
	    		sem_post(list_get(semaforosInstancias,(*(*instanciaAEnviar).nroSemaforo)));
	    		sem_wait(&semaforoLiberar);
				}
				free(clave);
				break;
			case 's':
				tam = obtenerTamDelSigBuffer(socketPlanificador);
				log_info(logger,"tam buff: %d", tam);
				clave = malloc(tam);
				recv(socketPlanificador,clave,tam,0);
				send(socketPlanificador,"s",2,0);
				log_info(loggerReal,"El planificador requiere el estado de la clave: %s", clave);
				if((instanciaAEnviar=buscarInstancia(clave))!=NULL){
					enviarCantBytes(socketPlanificador,(*instanciaAEnviar).nombreInstancia);
					send(socketPlanificador,(*instanciaAEnviar).nombreInstancia,strlen((*instanciaAEnviar).nombreInstancia)+1,0);
					operacion = "o";
					claveAComunicar = malloc(strlen(clave)+1);
					strcpy(claveAComunicar,clave);
					log_info(loggerReal,"Se va a preguntar a la instancia: %s", (*instanciaAEnviar).nombreInstancia);
					sem_post(list_get(semaforosInstancias,(*(*instanciaAEnviar).nroSemaforo)));
				}else{
					paqueteAEnviar.argumentos.GET.clave= clave;
					instanciaAEnviar = realizarSimulacion();
					log_info(loggerReal,"Se va realizar una simulacion!");
					log_info(logger,"La simulacion fue un exito nombre instancia: %s",(*instanciaAEnviar).nombreInstancia);
					enviarCantBytes(socketPlanificador,(*instanciaAEnviar).nombreInstancia);
					send(socketPlanificador,(*instanciaAEnviar).nombreInstancia,strlen((*instanciaAEnviar).nombreInstancia)+1,0);
					enviarCantBytes(socketPlanificador,"Fue un simulacro");
					send(socketPlanificador,"Fue un simulacro",17,0);
				}
				free(clave);
				break;
		}
	}
	free(buff);
}

void enviarDatosInstancia(int sockInstancia, char* tipo){
	t_config *config=config_create(pathCoordinador);
	char* buff=config_get_string_value(config, tipo);
	enviarCantBytes(sockInstancia,buff);
	send(sockInstancia,buff,string_length(buff)+1,0);
	config_destroy(config); //EL CONFIG DESTROY HACE FREE DEL BUFF!
}

instancia* buscarInstancia(char* clave){
	int i =0;
	instancia* instancia;
	while((instancia = list_get(instancias,i))!= NULL){ //ME FIJO HASTA LA ULTIMA LISTA
		if((*instancia).clavesBloqueadas != NULL){ //PRIMERO ME FIJO QUE EXISTA AL MENOS UNA CLAVE
			int j =0;
			char*aux;
			while((aux =list_get((*instancia).clavesBloqueadas,j))!=NULL){ //BUSCO HASTA ENCONTRAR LA ULTIMA CLAVE
				if(strcmp(clave,aux) == 0){ //SI ENCUENTRO UNA CLAVE QUE COINCIDA ENTONCES RETORNO LA INSTANCIA
					log_info(logger,"Se encontro la instancia");
					return instancia; //RETORNO LA INSTANCIA ASOCIADA A ESA CLAVE
				}
				j++;
			}
		}
		i++;
	}
	log_warning(logger,"No se encontro la instancia :c");
	return NULL; //ES NULL SI NO ENCUENTRO NINGUNA INSTANCIA QUE LA USE
}


void liberarClave(instancia* instancia,char* clave){
	int j = 0;
	while(strcmp(clave,list_get((*instancia).clavesBloqueadas,j)) != 0){ //BUSCO HASTA ENCONTRAR LA CLAVE ASOCIADA
		j++;
	}
	list_remove_and_destroy_element((*instancia).clavesBloqueadas,j,free);//ASIGNO EL PUNTERO A UN AUXILIAR PARA HACER EL FREE
	if(!list_get((*instancia).clavesBloqueadas,0)){
		list_destroy((*instancia).clavesBloqueadas);
		(*instancia).clavesBloqueadas = NULL;
	}
}

void agregarClave(instancia* instancia,char* clave){
	if(!(*instancia).clavesBloqueadas){
		(*instancia).clavesBloqueadas = list_create();
	}
	char* claveABloquear = malloc(strlen(clave)+1); //LE ASIGNO MEMORIA PARA LA CLAVE
	strcpy(claveABloquear,clave);
	list_add((*instancia).clavesBloqueadas,claveABloquear);
	log_info(logger,"Se agrego correctamente la clave %s", claveABloquear);
}


void *conexionESI(void* nuevoCliente) //REFACTORIZAR EL FOKEN SWITCH
{
	int socketEsi = *(int*)nuevoCliente;
	sem_post(&esperaInicializacion);
    int recvValor;
    instancia* instanciaAEnviar;
    char* resultadoEsi;
    t_log* logOperaciones = log_create("../Logs/LogOperaciones.log","LogOperaciones",1,LOG_LEVEL_INFO);
   if((recvValor = recibir(socketEsi,&paqueteAEnviar)) >0){

    	switch (paqueteAEnviar.keyword){
    	case GET:
    		log_info(logger,"Estamos haciendo un GET");
    		log_info(logOperaciones,"GET %s",paqueteAEnviar.argumentos.GET.clave);
			send(socketPlanificador,"n",2,0);
			enviarCantBytes(socketPlanificador,paqueteAEnviar.argumentos.GET.clave);
			send(socketPlanificador,paqueteAEnviar.argumentos.GET.clave,strlen(paqueteAEnviar.argumentos.GET.clave)+1,0);
			sem_wait(&semaforoEsi);
    		if(!operacionValida){ //VERIFICO SI LA CLAVE ESTA TOMADA
    			send(socketPlanificador,"b",2,0);
    			enviarCantBytes(socketPlanificador,paqueteAEnviar.argumentos.GET.clave);
    			send(socketPlanificador,paqueteAEnviar.argumentos.GET.clave,strlen(paqueteAEnviar.argumentos.GET.clave)+1,0);
    			free(paqueteAEnviar.argumentos.GET.clave);
    			resultadoEsi = "b";
    			log_info(loggerReal,"No se puede realizar la operacion ya que la clave fue tomada");
    			break;
    		}
    		log_info(logger,"Se puede realizar el GET");
    		while(true){
    		instanciaAEnviar = algoritmoDeDistribucion(NULL,instancias);
    		log_info(logger,"La instancia elegida es %s, con el semaforo nro: %d",(*instanciaAEnviar).nombreInstancia, (*(*instanciaAEnviar).nroSemaforo));
    		operacion = "p";
    		log_info(loggerReal,"La instancia elegida para esta operacion es: %s", (*instanciaAEnviar).nombreInstancia);
    		sem_post(list_get(semaforosInstancias,(*(*instanciaAEnviar).nroSemaforo)));
    		algoritmoDeDistribucion(instanciaAEnviar,instancias);
    		sem_wait(&semaforoEsi);
    		if(operacionValida){
    			send(socketPlanificador,"b",2,0);
    			enviarCantBytes(socketPlanificador,paqueteAEnviar.argumentos.GET.clave);
    			send(socketPlanificador,paqueteAEnviar.argumentos.GET.clave,strlen(paqueteAEnviar.argumentos.GET.clave)+1,0);
    			agregarClave(instanciaAEnviar,paqueteAEnviar.argumentos.GET.clave);
    			free(paqueteAEnviar.argumentos.GET.clave);
    			log_info(loggerReal,"Se realizo de manera correcta la operacion!");
    	    	resultadoEsi = "e";
    			break;
    		} //COMO LA OPERACION NO ES VALIDA SIGNIFICA QUE HUBO UN ERROR CON LA CONEXION DE LA INSTANCIA, POR LO TANTO LO DEJO EN FALSE.
    		}
    		break;
    	case SET:
    		log_info(logOperaciones,"SET %s %s",paqueteAEnviar.argumentos.SET.clave,paqueteAEnviar.argumentos.SET.valor);
    		log_info(logger,"Estamos haciendo un SET");
    		if(!validarYenviarPaquete(paqueteAEnviar.argumentos.SET.clave, socketEsi)){
    		    close(socketEsi);
        		free(paqueteAEnviar.argumentos.SET.clave);
        	    free(paqueteAEnviar.argumentos.SET.valor);
    		    log_warning(loggerReal,"No se pudo realizar la operacion");
    		    log_destroy(logOperaciones);
    		    return 0;
    		}
    		free(paqueteAEnviar.argumentos.SET.clave);
    	    free(paqueteAEnviar.argumentos.SET.valor);
    	    log_info(loggerReal,"Se realizo de manera correcta la operacion!");
        	resultadoEsi = "e";
    		break;
    	case STORE:
    		log_info(logOperaciones,"STORE %s",paqueteAEnviar.argumentos.STORE.clave);
    		log_info(logger,"Estamos haciendo un STORE");
    		if(!validarYenviarPaquete(paqueteAEnviar.argumentos.STORE.clave, socketEsi)){
    			close(socketEsi);
        		free(paqueteAEnviar.argumentos.STORE.clave);
        		 log_warning(loggerReal,"No se pudo realizar la operacion");
        		    log_destroy(logOperaciones);
    			return 0;
    		}
    		instanciaAEnviar = buscarInstancia(paqueteAEnviar.argumentos.STORE.clave);
    		send(socketPlanificador,"l",2,0);
    		enviarCantBytes(socketPlanificador,paqueteAEnviar.argumentos.STORE.clave);
    		send(socketPlanificador,paqueteAEnviar.argumentos.STORE.clave,strlen(paqueteAEnviar.argumentos.STORE.clave)+1,0);
    		liberarClave(instanciaAEnviar,paqueteAEnviar.argumentos.STORE.clave);
    		free(paqueteAEnviar.argumentos.STORE.clave);
    		log_info(loggerReal,"Se realizo de manera correcta la operacion!");
        	resultadoEsi = "e";
    		break;
    	}
    }//GET SET STORE IMPLEMENTACION
    if(recvValor == 0)
            log_warning(loggerReal,"Se desconecto el ESI en medio de ejecucion");
        else if(recvValor == -1){
            log_error(logger,"Error al recibir el tam del codigo serializado");
            resultadoEsi = "a";
        }else{
        	send(socketEsi,resultadoEsi,2,0);
        }
    log_warning(logger,"Finalizo la atencion al esi");
    close(socketEsi); //SE OPERA SENTENCIA POR SENTENCIA POR LO TANTO LO CERRAMOS Y ESPERAMOS SU CONEXION DEVUELTA
    log_destroy(logOperaciones);
    return 0;
}

int validarYenviarPaquete(char* clave, int socketEsi) {
	instancia* instanciaAEnviar;
	instanciaAEnviar = buscarInstancia(clave); //BUSCO LA INSTANCIA QUE CONTIENE TAL CLAVE
	if(instanciaAEnviar == NULL){
		send(socketEsi,"a",2,0);
		enviarCantBytes(socketEsi,"Coordinador: Error de Clave no Identificada");
		send(socketEsi,"Coordinador: Error de Clave no Identificada",44,0);
		return 0;
	}
	log_info(logger,"Se encontro la instancia con tal clave");
	log_info(logger,"La instancia a enviar es: %s",(*instanciaAEnviar).nombreInstancia);
	send(socketPlanificador,"v",2,0);
	enviarCantBytes(socketPlanificador,clave);
	send(socketPlanificador,clave,strlen(clave)+1,0);
	sem_wait(&semaforoEsi);
	if (!operacionValida) {
		send(socketEsi,"a",2,0);
		enviarCantBytes(socketEsi,"Planificador: Error Clave No Bloqueada");
		send(socketEsi,"Planificador: Error Clave No Bloqueada",44,0);
		return 0;
	}
	operacion = "p";
	log_info(loggerReal,"La instancia elegida para esta operacion es: %s", (*instanciaAEnviar).nombreInstancia);
	sem_post(list_get(semaforosInstancias,(*(*instanciaAEnviar).nroSemaforo))); //LE AVISO A LA INSTANCIA QUE ES HORA DE ACTUAR
	sem_wait(&semaforoEsi);
	if (!operacionValida) {
		send(socketEsi,"a",2,0);
		enviarCantBytes(socketEsi,errorMensajeInstancia);
		send(socketEsi,errorMensajeInstancia,strlen(errorMensajeInstancia)+1,0);
		return 0;
	}
	return 1;
}

instancia* crearInstancia(int sockInstancia,char* nombreInstancia,int* cantidadDeEntradas,int* nroSemaforo){
	instancia* instanciaNueva = NULL;
	instanciaNueva = malloc(sizeof(instancia));
	(*instanciaNueva).cantEntradasDisponibles = cantidadDeEntradas;
	inicializarInstancia(instanciaNueva,nombreInstancia,nroSemaforo); //Inicializamos la instancia.
	return instanciaNueva;
}

algoritmo obtenerAlgoritmoDistribucion(){
	t_config *config=config_create(pathCoordinador);
	log_info(logger,"Vamos a seleccionar un algoritmo");
	char* algoritmo = config_get_string_value(config,"AlgoritmoDeDistribucion");
	if(strcmp("EL",algoritmo)==0){
		log_info(loggerReal,"Usted eligio el algoritmo Equitative Load");
		config_destroy(config);
		return &equitativeLoad;
	}
	if(strcmp("KE",algoritmo)==0){
		log_info(logger,"Usted eligio el algoritmo Key Explicit");
		config_destroy(config);
		return &keyExplicit;
	}
	if(strcmp("LSU",algoritmo)==0){
		log_info(logger,"Usted eligio el algoritmo LSU");
		config_destroy(config);
		return &lsu;
	}
	log_error(logger,"No se reconocio el algoritmo seleccionado");
	exit(-1);
	return NULL;
}

void inicializarInstancia(instancia* instanciaNueva,char* nombreInstancia,int* nroSemaforo){
	t_config *config=config_create(pathCoordinador);
	(*(*instanciaNueva).cantEntradasDisponibles) = config_get_int_value(config, "CantidadEntradas");
	config_destroy(config);
	(*instanciaNueva).clavesBloqueadas = NULL;
	(*instanciaNueva).nombreInstancia = malloc(string_length(nombreInstancia)+1);
	strcpy((*instanciaNueva).nombreInstancia,nombreInstancia);
	sem_t* semInstancia = malloc(sizeof(sem_t));
	if(sem_init(semInstancia,0,0)<0){
		log_error(logger,"No se pudo inicializar el semaforo nro: %d",cantidadDeInstancias);
	}
	list_add(semaforosInstancias,semInstancia);
	(*instanciaNueva).nroSemaforo = nroSemaforo;
	log_info(logger,"El nro semaforo asignado es %d",cantidadDeInstancias);
}

instancia* existeEnLaLista(char* id){
	int i =0;
	instancia* instancia;
	int noEncontrado = 1;
	while((noEncontrado != 0) && ((instancia = list_get(instancias,i))!= NULL)){
		noEncontrado = strcmp(id,(*instancia).nombreInstancia);
		i++;
	}
	return instancia;
}

instancia* equitativeLoad(instancia* instancia, t_list* listaInstancias){
	if(instancia == NULL){ //ES DE LECTURA SI ES NULL
		int i = 0;
		instancia = list_remove(listaInstancias,i);//SACA LA PRIMERA INSTANCIA DISPONIBLE Y LO ELIMINO (NO ESTA DISPONIBLE SI SURGIO UNA DESCONEXION CON EL SERVIDOR)
		return instancia;
	}
	list_add(listaInstancias, instancia); //ESTO ES CUANDO LLEGA UNA CONEXION DE UNA INSTANCIA LO METO AL FINAL DE LA LISTA
	return NULL; //NO IMPORTA LO QUE DEVUELTA POR QUE ES ESCRITURA
}

instancia* lsu(instancia* instanciaAUsar,t_list* listaInstancias){
	if(instanciaAUsar == NULL){
		int i=1;
		instancia* instanciaAux;
		instanciaAUsar = list_get(listaInstancias,0);
		do{
			instanciaAux = list_get(listaInstancias,i);
			if(instanciaAux!=NULL && (*(*instanciaAux).cantEntradasDisponibles) > (*(*instanciaAUsar).cantEntradasDisponibles)){
				instanciaAUsar = instanciaAux;
			}
			i++;
		}while(instanciaAux !=NULL);
		i=0;
		while((instanciaAux=list_get(listaInstancias,i))!=NULL){
			if(instanciaAux==instanciaAUsar){
				list_remove(listaInstancias,i);
				list_add(listaInstancias,instanciaAUsar);
				break;
			}
			i++;
		}
		log_info(logger,"Se logro realizar el lsu %s",instanciaAUsar!=NULL?(*instanciaAUsar).nombreInstancia:"Es null csm");
		return instanciaAUsar;
	}
	return NULL;
}


int obtenerLetra(){
	int restarSegunMayusOMinus = 65;
	if(paqueteAEnviar.argumentos.GET.clave[0] >= 97){
		restarSegunMayusOMinus = 97;
	}
	return paqueteAEnviar.argumentos.GET.clave[0]-restarSegunMayusOMinus;
}

instancia* keyExplicit(instancia* instancia,t_list* listaInstancias){
	if(instancia == NULL){
		int cantidadInstancias = list_size(instancias);
		int rangoAscii = 25/cantidadInstancias;
		if(25%cantidadInstancias){
			rangoAscii++;
		}
		log_info(logger,"El rango ascii es %d",rangoAscii);
		int i =0;
		int letraABuscar = obtenerLetra();
		while((instancia = list_get(listaInstancias,i))){
				if(letraABuscar>= i*rangoAscii && letraABuscar <= (i+1)*rangoAscii){
					return instancia;
				};
			i++;
		}
	}
	return NULL;
}

void compactacionSimultanea(int semaforoNoNecesario){
	int nroSemaforo = 0;
	sem_t* semaforo;
	log_info(loggerReal,"Se va realizar una compactacion simultantea");
	while((semaforo =list_get(semaforosInstancias,nroSemaforo)) != NULL){
		if(nroSemaforo != semaforoNoNecesario){
			operacion = "c";
			sem_post(semaforo);
		}
		nroSemaforo++;
	}
}

void realizarEnvioDeValor(int socketInstancia) {
	enviarCantBytes(socketInstancia, claveAComunicar);
	log_info(logger,"La clave a conocer el valor es %s",claveAComunicar);
	send(socketInstancia, claveAComunicar, strlen(claveAComunicar)+1, 0);
	free(claveAComunicar);
	int tamBuff = obtenerTamDelSigBuffer(socketInstancia);
	log_error(logger, "tam del valor %d", tamBuff);
	char * buff = malloc(tamBuff);
	recv(socketInstancia, buff, tamBuff, 0);
	log_error(logger, "el valor a enviar es: %s", buff);
	enviarCantBytes(socketPlanificador, buff);
	send(socketPlanificador, buff, strlen(buff)+1, 0);
	free(buff);
}

void eliminarClaves(char* clave){
	free(clave);
}

void liberarInstancia(instancia* instanciaALiberar){
	instancia* instancia;
	int i=0;
	while((instancia = list_get(instancias,i))!=NULL){
		log_info(logger,"bajando semaforo");
		log_info(logger,"con nro %d",(*(*instancia).nroSemaforo));
		if((*(*instancia).nroSemaforo) > (*(*instanciaALiberar).nroSemaforo)){
			(*(*instancia).nroSemaforo)--;
		}
		if(instanciaALiberar== instancia){
			log_info(logger,"Encontre el semaforo");
			list_remove(instancias,i);
			i--;
		}
		i++;
	}
	sem_t* semaforo = list_remove(semaforosInstancias,(*(*instanciaALiberar).nroSemaforo));
	log_info(logger,"liberando semaforo");
	sem_destroy(semaforo);
	log_info(logger,"liberando semaforo");
	free(semaforo);
	free((*instanciaALiberar).nroSemaforo);
	log_info(logger,"liberando nombre");
	free((*instanciaALiberar).nombreInstancia);
	log_info(logger,"liberando lista");
	if((*instanciaALiberar).clavesBloqueadas!=NULL){
	list_destroy_and_destroy_elements((*instanciaALiberar).clavesBloqueadas,&eliminarClaves);
	}
	log_info(logger,"liberando instancia");
	free(instanciaALiberar);
	cantidadDeInstancias--;

}
void *conexionInstancia(void* cliente){
	int socketInstancia = *(int*)cliente;
	int* nroSemaforo = malloc(sizeof(int));
	(*nroSemaforo)=cantidadDeInstancias;
	enviarDatosInstancia(socketInstancia,"CantidadEntradas"); //PRIMERO LE ENVIO LA CANTIDAD DE ENTRADAS
	enviarDatosInstancia(socketInstancia,"TamagnoEntradas"); //DESPUES LE ENVIO EL TAMAGNO DE ESAS ENTRADAS
	int tam = obtenerTamDelSigBuffer(socketInstancia);
	char* buff = malloc(tam);
	log_info(logger,"El tam del buffer es %d",tam);
	recv(socketInstancia,buff,tam,0);
	log_info(loggerReal,"Se conecto la instancia: %s",buff); // RECIBO EL ID DE LA INSTANCIA.
	int cantidadDeEntradasRestantes;
	instancia* instanciaNueva = crearInstancia(socketInstancia,buff,&cantidadDeEntradasRestantes,nroSemaforo);
	free(buff);
	if(instanciaNueva != NULL){ //Si es NULL significa que es una reconexion!, por lo tanto no hace falta meterlo en la lista!
		list_add(instancias,instanciaNueva);
	}
	sem_post(&esperaInicializacion);
	//ACA TERMINE DE INICIALIZAR LA INSTANCIA
	int  recvValor;
	while(true){
		log_error(logger,"Esperando semaforo %d",(*nroSemaforo));
		sem_wait(list_get(semaforosInstancias,(*nroSemaforo)));
		log_info(logger,"Se asigno la tarea a la instancia con el semaforo: %d",(*nroSemaforo));
		send(socketInstancia,operacion,2,MSG_NOSIGNAL);
		if(operacion[0]=='p'){
			t_config* config = config_create(pathCoordinador);
			usleep(config_get_int_value(config,"Retardo")*1000);
	    	config_destroy(config);
			buff = malloc(2);
			buff[0]='n';
			log_info(logger,"Vamos a enviarle un paquete a la instancia");
			enviar(socketInstancia,paqueteAEnviar);
			while(buff[0]!= 'r'&&buff[0]!='a'&& buff[0]!='e'){
				if((recvValor=recv(socketInstancia,buff,2,0))<=0){ //ESPERO EL RESULTADO DE LA INSTANCIA
					log_warning(loggerReal,"Se habia desconectado la instancia %s",(*instanciaNueva).nombreInstancia);
					errorMensajeInstancia = "Instancia: Error Clave Inaccesible";
					operacionValida=0; //SI HUBO UN ERROR EN LA CONEXION O LA INSTANCIA SE DESCONECTO
					liberarInstancia(instanciaNueva);
					sem_post(&semaforoEsi);
					free(buff);
					return 0;
				}
				switch(buff[0]){
					case 'c':
						compactacionSimultanea((*nroSemaforo));
						break;
					case 'r':
						operacionValida=1;
						int bytes = obtenerTamDelSigBuffer(socketInstancia);
						log_info(logger,"bytes: %d",bytes);
						char* cantidadDeEntradasARestar = malloc(sizeof(bytes));
						recv(socketInstancia,cantidadDeEntradasARestar,bytes,0);
						cantidadDeEntradasRestantes=transformarNumero(cantidadDeEntradasARestar,0);
						free(cantidadDeEntradasARestar);
						sem_post(&semaforoEsi);
						break;
					case 'e':
						operacionValida=0;
						errorMensajeInstancia = "Instancia: Error Clave No Encontrada";
						sem_post(&semaforoEsi);
						break;
					case 'a':
						operacionValida = 0;
						errorMensajeInstancia = "Instancia: Error No Tengo Espacio Suficiente";
						sem_post(&semaforoEsi);
						break;
				}
			}
			free(buff);
		}else if(operacion[0]=='o'){
			realizarEnvioDeValor(socketInstancia);
		}else if(operacion[0]=='l'){
			enviarCantBytes(socketInstancia,claveAComunicar);
			send(socketInstancia,claveAComunicar,strlen(claveAComunicar)+1,MSG_NOSIGNAL);
			int bytes = obtenerTamDelSigBuffer(socketInstancia);
			log_info(logger,"bytes: %d",bytes);
			char* cantidadDeEntradasARestar = malloc(sizeof(bytes));
			recv(socketInstancia,cantidadDeEntradasARestar,bytes,0);
			cantidadDeEntradasRestantes=transformarNumero(cantidadDeEntradasARestar,0);
			free(cantidadDeEntradasARestar);
			log_info(logger,"Se termino liberar la clave");
			sem_post(&semaforoLiberar);
		}
		log_info(logger,"Se envio completamente el paquete");
	}
}
