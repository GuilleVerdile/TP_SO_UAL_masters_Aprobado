#include "Coordinador.h"
t_list* instancias;
int socketPlanificador;
sem_t* semaforosInstancias;
int cantidadDeInstancias;
sem_t esperaInicializacion;
sem_t semaforoEsi;
t_esi_operacion paqueteAEnviar;
int operacionValida;
pthread_mutex_t mutexInstancias;
instancia* (*algoritmoDeDistribucion)();

int main(){
	cantidadDeInstancias =0;
	semaforosInstancias = NULL;
	logger =log_create(logCoordinador,"crearHilos",1, LOG_LEVEL_INFO);
	int listener;
	struct sockaddr_in their_addr; // datos cliente
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
	if((socketPlanificador = accept(listener, (struct sockaddr *)&their_addr,&addrlen))==-1){ //NOS CONECTAMOS CON EL PLANIFICADOR
	    log_error(logger, "No se aceptar la conexion");
	    log_destroy(logger);
	    return -1;
	}
	algoritmoDeDistribucion = obtenerAlgoritmoDistribucion(); //OBTENGO EL ALGORITMO DE DISTRUBUCION SEGUN EL ARCHIVO CONFIG
	log_info(logger, "Se acepto la conexion");
	pthread_t* hilosInstancias = NULL;
	pthread_t hiloEsi;
	instancias = list_create();
	sem_init(&esperaInicializacion,0,0);
	sem_init(&semaforoEsi,0,0);
	pthread_mutex_init(&mutexInstancias,NULL);
	//ACA COMIENZA LO DIVERTIDO =)
	while((nuevoCliente = accept(listener, (struct sockaddr *)&their_addr,&addrlen))) //Esperamos a que llegue la primera conexion
	{
		log_info(logger,"Se acepto una nueva conexion");
		char* buff;
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
			log_info(logger,"Se asigno una conexion con hilos al ESI");
			break;
		case 0:
			log_info(logger,"El cliente es INSTANCIA");
			semaforosInstancias = realloc(semaforosInstancias,sizeof(sem_t)*(cantidadDeInstancias+1));
			sem_init(&semaforosInstancias[cantidadDeInstancias],0,0);
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
	//pthread_mutex_lock(&mutexInstancias);
	while((instancia = list_get(instancias,i))!= NULL){ //ME FIJO HASTA LA ULTIMA LISTA
		if((*instancia).clavesBloqueadas != NULL){ //PRIMERO ME FIJO QUE EXISTA AL MENOS UNA CLAVE
			int j =0;
			while((*instancia).clavesBloqueadas[j]){ //BUSCO HASTA ENCONTRAR LA ULTIMA CLAVE
				if(strcmp(clave,(*instancia).clavesBloqueadas[j]) == 0){ //SI ENCUENTRO UNA CLAVE QUE COINCIDA ENTONCES RETORNO LA INSTANCIA
					log_info(logger,"Se encontro la instancia");
					return instancia; //RETORNO LA INSTANCIA ASOCIADA A ESA CLAVE
				}
				j++;
			}
		}
		i++;
	}
	//pthread_mutex_unlock(&mutexInstancias);
	return NULL; //ES NULL SI NO ENCUENTRO NINGUNA INSTANCIA QUE LA USE
}


void liberarClave(instancia* instancia,char* clave){
	send(socketPlanificador,"l",2,0);
	enviarDatosEsi(clave);
	int j = 0;
	while(strcmp(clave,(*instancia).clavesBloqueadas[j]) != 0){ //BUSCO HASTA ENCONTRAR LA CLAVE ASOCIADA
		j++;
	}
	char *aux = (*instancia).clavesBloqueadas[j]; //ASIGNO EL PUNTERO A UN AUXILIAR PARA HACER EL FREE
	char* aux2; //AUXILIAR PARA REORDENAR LAS CLAVES
	for(j;j < (*instancia).cantClavesBloqueadas-1;j++){
		aux2 = (*instancia).clavesBloqueadas[j+1];
		(*instancia).clavesBloqueadas[j] = aux2; //EL ANTERIOR SE LA ASIGNA EL SIGUIENTE
		j++;
	}
	log_info(logger,"Se va liberar la clave: %s",aux);
	free(aux);//LIBERO LA CLAVE BLOQUEADA
	(*instancia).cantClavesBloqueadas --;
	log_info(logger,"La cantidad de claves bloqueadas son %d",(*instancia).cantClavesBloqueadas);
	if((*instancia).cantClavesBloqueadas == 0){
		free((*instancia).clavesBloqueadas);
	}
	else{
		(*instancia).clavesBloqueadas = realloc((*instancia).clavesBloqueadas,sizeof(char*)*(*instancia).cantClavesBloqueadas);
		log_info(logger,"Se va realizar un reallocamiento de memoria en las claves bloqueadas");
	}
	send(socketPlanificador, "d", 2,0); //SENIAL DE DESBLOQUEO
	enviarDatosEsi(clave);
}

void agregarClave(instancia* instancia,char clave[40]){
		(*instancia).clavesBloqueadas = realloc((*instancia).clavesBloqueadas, sizeof(char*)*((*instancia).cantClavesBloqueadas+1)); //LE ASIGNO MAS MEMORIA A LAS CLAVES BLOQUEADAS
		(*instancia).clavesBloqueadas[(*instancia).cantClavesBloqueadas] = malloc(strlen(clave)+1); //LE ASIGNO MEMORIA PARA LA CLAVE
		strcpy((*instancia).clavesBloqueadas[(*instancia).cantClavesBloqueadas],clave);
		log_info(logger,"Se agrego correctamente la clave %s",(*instancia).clavesBloqueadas[(*instancia).cantClavesBloqueadas]);
		(*instancia).cantClavesBloqueadas++;
}

void enviarDatosEsi(char*clave){
	enviarCantBytes(socketPlanificador,clave);
	send(socketPlanificador,clave,strlen(clave)+1,0);
}

int verificacionEsi(char* clave,char* loQueEnvio){
	send(socketPlanificador,loQueEnvio,2,0); //LE ENVIO UNA SENIAL DE VERIFICAR
	enviarDatosEsi(clave);
	log_info(logger,"Enviado los datos del esi");
	char* resultado = malloc(2);
	recv(socketPlanificador,resultado,2,0); //OBTENGO EL RESULTADO
	log_info(logger,"Se confirmo el resultado: %s",resultado);
	if(resultado[0]-48){ //ES HORRIBLE PERO ESTO ES POR EL MEMORY LEAK
		free(resultado);
		return 1;
	}
	free(resultado);
	return 0;
}

void *conexionESI(void* nuevoCliente) //REFACTORIZAR EL FOKEN SWITCH
{
	int socketEsi = *(int*)nuevoCliente;
    int recvValor;
    instancia* instanciaAEnviar;
    while((recvValor = recibir(socketEsi,&paqueteAEnviar)) >0){
    	t_config* config = config_create(pathCoordinador);
    	sleep(config_get_int_value(config,"Retardo"));
    	config_destroy(config);
    	switch (paqueteAEnviar.keyword){
    	case GET:
    		log_info(logger,"Estamos haciendo un GET");
    		if(!verificacionEsi(paqueteAEnviar.argumentos.GET.clave,"n")){ //VERIFICO SI LA CLAVE ESTA TOMADA
    			send(socketPlanificador,"b",2,0); //LE MANDO UNA SENIAL DE BLOQUEO
    			enviarDatosEsi(paqueteAEnviar.argumentos.GET.clave); //LE ENVIO LOS DATOS PARA BLOQUEARLO
    			return 0;  //CON ESTO NOS ASEGURAMOS QUE LA CONEXION CON EL ESI MUERA.
    		}
    		log_info(logger,"Se puede realizar el GET");
    		while(true){
    		instanciaAEnviar = algoritmoDeDistribucion(NULL); //BUSCO UNA INSTANCIA CON estaDisponible == 1.
    		sem_post(&semaforosInstancias[(*instanciaAEnviar).nroSemaforo]);  //LE DIGO A LA INSTANCIA QUE TRABAJE
    		algoritmoDeDistribucion(instanciaAEnviar);
    		sem_wait(&semaforoEsi);
    		if(operacionValida){
    			send(socketPlanificador,"b",2,0); //LE MANDO UNA SENIAL DE BLOQUEO
    			enviarDatosEsi(paqueteAEnviar.argumentos.GET.clave); //LE ENVIO LOS DATOS PARA BLOQUEARLO
    			agregarClave(instanciaAEnviar,paqueteAEnviar.argumentos.SET.clave);
    			break;
    		}
    		(*instanciaAEnviar).estaDisponible = 0; //COMO LA OPERACION NO ES VALIDA SIGNIFICA QUE HUBO UN ERROR CON LA CONEXION DE LA INSTANCIA, POR LO TANTO LO DEJO EN FALSE.
    		}
    		break;
    	case SET:
    		log_info(logger,"Estamos haciendo un SET");
    		if(!validarYenviarPaquete(paqueteAEnviar.argumentos.SET.clave, socketEsi, paqueteAEnviar)) return 0;
    		break;
    	case STORE:
    		log_info(logger,"Estamos haciendo un STORE");
    		if(!validarYenviarPaquete(paqueteAEnviar.argumentos.STORE.clave, socketEsi, paqueteAEnviar)) return 0;
    		instanciaAEnviar = buscarInstancia(paqueteAEnviar.argumentos.STORE.clave);
    		liberarClave(instanciaAEnviar,paqueteAEnviar.argumentos.STORE.clave);
    		break;
    	default:
    		log_error(logger,"Operacion invalida");
    	}
    	send(socketEsi,"e",2,0);
    }//GET SET STORE IMPLEMENTACION
    if(recvValor == 0)
            log_info(logger,"Se desconecto un ESI");
        else if(recvValor == -1)
            log_error(logger,"Error al recibir el tam del codigo serializado");
    close(socketEsi); //SE OPERA SENTENCIA POR SENTENCIA POR LO TANTO LO CERRAMOS Y ESPERAMOS SU CONEXION DEVUELTA
    return 0;
}

int validarYenviarPaquete(char* clave, int socketEsi,t_esi_operacion* paquete) {
	instancia* instanciaAEnviar;
	if (!verificacionEsi(clave,"v")) {
		//VERIFICO SI LA CLAVE ESTA TOMADA POR EL MISMO ESI
		send(socketEsi, "a", 2, 0); //SE LE PIDE ABORTAR EL ESI POR CODEAR PARA EL OJETE YA QUE DE ALGUNA FORMA LA CLAVE NO FUE BLOQUEADA POR TAL ESI.
		close(socketEsi);
		return 0;
	}
	instanciaAEnviar = buscarInstancia(clave); //BUSCO LA INSTANCIA QUE CONTIENE TAL CLAVE
	log_info(logger,"La instancia a enviar es: %s",(*instanciaAEnviar).nombreInstancia);
	sem_post(&semaforosInstancias[(*instanciaAEnviar).nroSemaforo]); //LE AVISO A LA INSTANCIA QUE ES HORA DE ACTUAR
	sem_wait(&semaforoEsi);
	if (!operacionValida) {
		send(socketEsi, "a", 2, 0); //SE LE PIDE ABORTAR EL ESI POR DESCONEXION DE LA INSTANCIA
		close(socketEsi);
		return 0;
	}
	return 1;
}

instancia* crearInstancia(int sockInstancia,char* nombreInstancia){
	instancia* instanciaNueva = NULL;
	if((instanciaNueva = existeEnLaLista(nombreInstancia))!=NULL){
		(*instanciaNueva).estaDisponible = 1;
		send(sockInstancia,"r",2,0);//SE LE MANDA R DE QUE ES UNA RECONEXION POR QUE ESTA EN LA LISTA.
		log_info(logger,"Es una reconexion de la instancia %s", nombreInstancia);
		return NULL; //Si ya existia la instancia en la lista no me hace falta seguir operando
	}
	instanciaNueva = malloc(sizeof(instancia));
	inicializarInstancia(instanciaNueva,nombreInstancia); //Inicializamos la instancia.
	return instanciaNueva;
}

algoritmo obtenerAlgoritmoDistribucion(){
	t_config *config=config_create(pathCoordinador);
	switch (config_get_int_value(config, "AlgoritmoDeDistribucion")){
	config_destroy(config);
	case EL: //EQUITATIVE LOAD
		return &equitativeLoad;
		break;
	case LSU: //LSU
		//return lsu(sockInstancia);
		break;
	case KE: //KEYEXPLICIT
		//return keyExplicit(sockInstancia);
		break;
	default:
		log_error(logger,"No se reconocio el algoritmo de distribucion");
		exit(-1);
	}
}

void inicializarInstancia(instancia* instanciaNueva,char* nombreInstancia){
	t_config *config=config_create(pathCoordinador);
	(*instanciaNueva).cantEntradasDisponibles = config_get_int_value(config, "CantidadEntradas");
	config_destroy(config);
	(*instanciaNueva).clavesBloqueadas = NULL;
	(*instanciaNueva).estaDisponible = 1;
	(*instanciaNueva).nombreInstancia = malloc(string_length(nombreInstancia)+1);
	strcpy((*instanciaNueva).nombreInstancia,nombreInstancia);
	(*instanciaNueva).nroSemaforo = cantidadDeInstancias;
	(*instanciaNueva).cantClavesBloqueadas = 0;
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

instancia* equitativeLoad(instancia* instancia){
	if(instancia == NULL){ //ES DE LECTURA SI ES NULL
		int i = 0;
		int disponibilidad = 0;
		pthread_mutex_lock(&mutexInstancias);
		while(!disponibilidad && instancia != NULL){
			instancia = list_get(instancias,i);
			disponibilidad = (*instancia).estaDisponible;
			i++;
		}
		instancia = list_remove(instancias,i);//SACA LA PRIMERA INSTANCIA DISPONIBLE Y LO ELIMINO (NO ESTA DISPONIBLE SI SURGIO UNA DESCONEXION CON EL SERVIDOR)
		pthread_mutex_unlock(&mutexInstancias);
		return instancia;
	}
	pthread_mutex_lock(&mutexInstancias);
	list_add(instancias, instancia); //ESTO ES CUANDO LLEGA UNA CONEXION DE UNA INSTANCIA LO METO AL FINAL DE LA LISTA
	pthread_mutex_unlock(&mutexInstancias);
	return NULL; //NO IMPORTA LO QUE DEVUELTA POR QUE ES ESCRITURA
}

void *conexionInstancia(void* cliente){
	int socketInstancia = *(int*)cliente;
	int nroSemaforo = cantidadDeInstancias;
	enviarDatosInstancia(socketInstancia,"CantidadEntradas"); //PRIMERO LE ENVIO LA CANTIDAD DE ENTRADAS
	enviarDatosInstancia(socketInstancia,"TamagnoEntradas"); //DESPUES LE ENVIO EL TAMAGNO DE ESAS ENTRADAS
	int tam = obtenerTamDelSigBuffer(socketInstancia);
	char* buff = malloc(tam);
	log_info(logger,"El tam del buffer es %d",tam);
	recv(socketInstancia,buff,tam,0);
	log_info(logger,"Se conecto la instancia: %s",buff); // RECIBO EL ID DE LA INSTANCIA.
	instancia* instanciaNueva = crearInstancia(socketInstancia,buff);
	free(buff);
	if(instanciaNueva != NULL){ //Si es NULL significa que es una reconexion!, por lo tanto no hace falta meterlo en la lista!
		algoritmoDeDistribucion(instanciaNueva);//LO METO EN LA LISTA SEGUN EL ALGORITMO DE DIST USADO
	}
	sem_post(&esperaInicializacion);
	//ACA TERMINE DE INICIALIZAR LA INSTANCIA
	buff = malloc(2);
	int  recvValor;
	while(true){
		sem_wait(&semaforosInstancias[nroSemaforo]);
		send(socketInstancia,"p",2,0); //SE LE ENVIA UN "p" DE PAQUETE PARA DECIRLE QUE SE LE VA ENVIAR UNA SENTENCIA.
		enviar(socketInstancia,paqueteAEnviar);
		if(recv(socketInstancia,buff,2,0)<=0){ //ESPERO EL RESULTADO DE LA INSTANCIA
			operacionValida=0; //SI HUBO UN ERROR EN LA CONEXION O LA INSTANCIA SE DESCONECTO
			sem_post(&semaforoEsi);
			break;
		}
		operacionValida=1;
		sem_post(&semaforoEsi);
	}
	free(buff);
}
