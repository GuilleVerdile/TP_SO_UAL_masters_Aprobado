#include "CoordinadorHilos.h"
t_list* instancias;
int socketPlanificador;
int main(){
	instancias = list_create();
	logger =log_create(logCoordinador,"crearHilos",1, LOG_LEVEL_INFO);
	int listener;
	struct sockaddr_in their_addr; // datos cliente
	if((listener=crearConexionServidor(pathCoordinador))==-1){
		log_error(logger, "No se pudo crear el socket servidor");
		log_destroy(logger);
		return -1;
	}
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
	log_info(logger, "Se acepto la conexion");
	printf("Nuevo cliente, se conecto el Planificador\n");
	fflush(stdout);
	pthread_t idHilo;
	//ACA COMIENZA LO DIVERTIDO =)
	while((nuevoCliente = accept(listener, (struct sockaddr *)&their_addr,&addrlen))) //Esperamos a que llegue la primera conexion
	{
		log_info(logger,"Se acepto una nueva conexion");
		int tipoCliente = esEsi(nuevoCliente);
		char* buff;
		buff = malloc(2);
		int recValor = recv(nuevoCliente, buff, 2, 0);
		int tipoCliente = 2;
		recValor == -1 ? exit():(recValor == 0 ? close(nuevoCliente):tipoCliente = transformarNumero(buff,0)); // borre funcion esEsi y uso operador ternario
		free(buff);
		switch(tipoCliente){
		case 1:
			log_info(logger,"El cliente es ESI");
			if(pthread_create(&idHilo , NULL , conexionESI, (void*) &nuevoCliente) < 0)
	    	{
				log_error(logger,"No se pudo crear un hilo");
				return -1;
	    	}
			log_info(logger,"Se asigno una conexion con hilos");
			pthread_join(idHilo,NULL);//No vamos a usar esta implementacion pero se usa para testear
			break;
		case 0:
			log_info(logger,"El cliente es INSTANCIA");
			enviarDatosInstancia(nuevoCliente,"CantidadEntradas"); //PRIMERO LE ENVIO LA CANTIDAD DE ENTRADAS
			enviarDatosInstancia(nuevoCliente,"TamagnoEntradas"); //DESPUES LE ENVIO EL TAMAGNO DE ESAS ENTRADAS
			int tam = obtenerTamDelSigBuffer(nuevoCliente);
			char* buff = malloc(tam);
			log_info(logger,"El tam del buffer es %d",tam);
			recv(nuevoCliente,buff,tam,0);
			log_info(logger,"Se conecto la instancia: %s",buff); // RECIBO EL ID DE LA INSTANCIA.
			instancia* instanciaNueva = crearInstancia(nuevoCliente,buff);
			free(buff);
			if(instanciaNueva != NULL){ //Si es NULL significa que es una reconexion!, por lo tanto no hace falta meterlo en la lista!
				algoritmoDeDistribucion(instanciaNueva);//LO METO EN LA LISTA SEGUN EL ALGORITMO DE DIST USADO
			}
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
	while((instancia = list_get(instancias,i))!= NULL){ //ME FIJO HASTA LA ULTIMA LISTA
		if((*instancia).clavesBloqueadas != NULL){ //PRIMERO ME FIJO QUE EXISTA AL MENOS UNA CLAVE
			int j =0;
			while((*instancia).clavesBloqueadas[j] != NULL){ //BUSCO HASTA ENCONTRAR LA ULTIMA CLAVE
				if(strcmp(clave,(*instancia).clavesBloqueadas[j]) == 0){ //SI ENCUENTRO UNA CLAVE QUE COINCIDA ENTONCES RETORNO LA INSTANCIA
					return instancia; //RETORNO LA INSTANCIA ASOCIADA A ESA CLAVE
				}
				j++;
			}
		}
		i++;
	}
	return NULL; //ES NULL SI NO ENCUENTRO NINGUNA INSTANCIA QUE LA USE
}

void verificarConexion(instancia* instancia){
	send((*instancia).socketInstancia, "v", 2,0);
	char* buff = malloc(2);
	int recvValor = recv((*instancia).socketInstancia,buff,2,0);
	free(buff);
	if(recvValor == 0){
		log_info(logger,"Se desconecto la instancia %s", (*instancia).nombreInstancia);
		close((*instancia).socketInstancia);
		(*instancia).estaDisponible = 0;
	}
	return;
}

void liberarClave(instancia* instancia,char* clave){
	int j = 0;
	while(strcmp((*instancia).clavesBloqueadas[j],clave) == 0){ //BUSCO HASTA ENCONTRAR LA CLAVE ASOCIADA
		j++;
	}
	char *aux = (*instancia).clavesBloqueadas[j]; //ASIGNO EL PUNTERO A UN AUXILIAR PARA HACER EL FREE
	char* aux2; //AUXILIAR PARA REORDENAR LAS CLAVES
	while((*instancia).clavesBloqueadas[j] !=NULL){ //REORDENO LAS CLAVES
		aux2 = (*instancia).clavesBloqueadas[j+1];
		(*instancia).clavesBloqueadas[j] = aux2; //EL ANTERIOR SE LA ASIGNA EL SIGUIENTE
		j++;
	}
	free(aux);//LIBERO LA CLAVE BLOQUEADA
	send(socketPlanificador, "d", 2,0); //SENIAL DE DESBLOQUEO
	enviarDatosEsi(clave);
}

void agregarClave(instancia* instancia,char* clave){
	if((*instancia).clavesBloqueadas == NULL){
		(*instancia).clavesBloqueadas = malloc(sizeof(char*)); //LE ASIGNO UNA DIRECCION DE MEMORIA
		(*instancia).clavesBloqueadas[0] = malloc(strlen(clave)+1);
		strcpy((*instancia).clavesBloqueadas[0],clave);
		(*instancia).clavesBloqueadas[1] = NULL; //EL SIGUIENTE ES NULL PARA HACER ALGUNAS VERIFICACIONES EN OTRAS FUNCIONES
	}
	else{
		int j =0;
		while((*instancia).clavesBloqueadas[j] !=NULL){
			j++; //BUSCO HASTA ENCONTRAR UN NULL
		}
		(*instancia).clavesBloqueadas = realloc((*instancia).clavesBloqueadas, sizeof(char*)*j); //LE ASIGNO MAS MEMORIA A LAS CLAVES BLOQUEADAS
		(*instancia).clavesBloqueadas[j] = malloc(strlen(clave)+1); //LE ASIGNO MEMORIA PARA LA CLAVE
		strcpy((*instancia).clavesBloqueadas[j],clave);
		(*instancia).clavesBloqueadas[j+1] = NULL; //PONGO NULL AL SIGUIENTE PARA VERIFICACIONES
	}
}

void enviarDatosEsi(char*clave){
	enviarCantBytes(socketPlanificador,clave);
	send(socketPlanificador,clave,strlen(clave)+1,0);
}

int verificacionEsi(char* clave){
	send(socketPlanificador,"v",2,0); //LE ENVIO UNA SENIAL DE VERIFICAR
	enviarDatosEsi(clave);
	char* resultado = malloc(2);
	recv(socketPlanificador,resultado,2,0); //OBTENGO EL RESULTADO
	if(resultado[0]){ //ES HORRIBLE PERO ESTO ES POR EL MEMORY LEAK
		free(resultado);
		return 1;
	}
	free(resultado);
	return 0;
}

void *conexionESI(void* cliente) //REFACTORIZAR EL FOKEN SWITCH
{
	log_info(logger,"ESTAMOS DENTRO DEL HILO");
    int socketEsi = *(int*)cliente; //LO CASTEAMOS
    int recvValor;
    t_esi_operacion paquete;
    instancia* instanciaAEnviar;
    while((recvValor = recibir(socketEsi,&paquete)) >0){
    	switch (paquete.keyword){
    	case GET:
    		if(buscarInstancia(paquete.argumentos.GET.clave) != NULL){ //VERIFICO SI LA CLAVE ESTA TOMADA
    			send(socketPlanificador,"b",2,0); //LE MANDO UNA SENIAL DE BLOQUEO
    			enviarDatosEsi(paquete.argumentos.GET.clave); //LE ENVIO LOS DATOS PARA BLOQUEARLO
    			return 0;  //CON ESTO NOS ASEGURAMOS QUE LA CONEXION CON EL ESI MUERA.
    		}
    		instanciaAEnviar = algoritmoDeDistribucion(NULL);
    		send((*instanciaAEnviar).socketInstancia,"p",2,0); //SE LE ENVIA UN "p" DE PAQUETE PARA DECIRLE QUE SE LE VA ENVIAR UNA SENTENCIA.
    		enviar((*instanciaAEnviar).socketInstancia,paquete);
    		algoritmoDeDistribucion(instanciaAEnviar);
    		agregarClave(instanciaAEnviar,paquete.argumentos.SET.clave);
    		break;
    	case SET:
    		if(!validarYenviarPaquete(paquete.argumentos.SET.clave, socketEsi, paquete)) return 0;
    		break;
    	case STORE:
    		if(!validarYenviarPaquete(paquete.argumentos.STORE.clave, socketEsi, paquete)) return 0;
    		liberarClave(instanciaAEnviar,paquete.argumentos.STORE.clave);
    		break;
    	}
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
	if (!verificacionEsi(clave)) {
		//VERIFICO SI LA CLAVE ESTA TOMADA POR EL MISMO ESI
		send(socketEsi, "a", 2, 0); //SE LE PIDE ABORTAR EL ESI POR CODEAR PARA EL OJETE YA QUE DE ALGUNA FORMA LA CLAVE NO FUE BLOQUEADA POR TAL ESI.
		close(socketEsi);
		return 0;
	}
	instanciaAEnviar = buscarInstancia(clave); //BUSCO LA INSTANCIA QUE CONTIENE TAL CLAVE
	verificarConexion(instanciaAEnviar); //SE VERIFICA LA CONEXION ANTES
	if (!(*instanciaAEnviar).estaDisponible) {
		send(socketEsi, "a", 2, 0); //SE LE PIDE ABORTAR EL ESI POR CODEAR PARA EL OJETE YA QUE DE ALGUNA FORMA LA CLAVE NO FUE BLOQUEADA POR TAL ESI.
		close(socketEsi);
		return 0;
	}
	send((*instanciaAEnviar).socketInstancia, "p", 2, 0); //SE LE ENVIA UN "p" DE PAQUETE PARA DECIRLE QUE SE LE VA ENVIAR UNA SENTENCIA.
	enviar((*instanciaAEnviar).socketInstancia, paquete);
	return 1;
}

instancia* crearInstancia(int sockInstancia,char* nombreInstancia){
	instancia* instanciaNueva = NULL;
	if((instanciaNueva = existeEnLaLista(nombreInstancia))!=NULL){
		(*instanciaNueva).socketInstancia = sockInstancia;
		(*instanciaNueva).estaDisponible = 1;
		send(sockInstancia,"r",2,0);//SE LE MANDA R DE QUE ES UNA RECONEXION POR QUE ESTA EN LA LISTA.
		log_info(logger,"Es una reconexion de la instancia %s", nombreInstancia);
		return NULL; //Si ya existia la instancia en la lista no me hace falta seguir operando
	}
	instanciaNueva = malloc(sizeof(instancia));
	inicializarInstancia(instanciaNueva,sockInstancia,nombreInstancia); //Inicializamos la instancia.
	return instanciaNueva;
}

instancia* algoritmoDeDistribucion(instancia* instanciaNueva){
	t_config *config=config_create(pathCoordinador);
	switch (config_get_int_value(config, "AlgoritmoDeDistribucion")){
	config_destroy(config);
	case EL: //EQUITATIVE LOAD
		return equitativeLoad(instanciaNueva);
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

void inicializarInstancia(instancia* instanciaNueva,int sockInstancia,char* nombreInstancia){
	t_config *config=config_create(pathCoordinador);
	(*instanciaNueva).cantEntradasDisponibles = config_get_int_value(config, "CantidadEntradas");
	config_destroy(config);
	(*instanciaNueva).clavesBloqueadas = NULL;
	(*instanciaNueva).estaDisponible = 1;
	(*instanciaNueva).nombreInstancia = malloc(string_length(nombreInstancia)+1);
	strcpy((*instanciaNueva).nombreInstancia,nombreInstancia);
	(*instanciaNueva).socketInstancia = sockInstancia;
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
		while(!disponibilidad && instancia != NULL){
			instancia = list_get(instancias,i);
			verificarConexion(instancia);
			disponibilidad = (*instancia).estaDisponible;
			i++;
		}
		return list_remove(instancias,i);//SACA LA PRIMERA INSTANCIA DISPONIBLE Y LO ELIMINO (NO ESTA DISPONIBLE SI SURGIO UNA DESCONEXION CON EL SERVIDOR)
	}
	list_add(instancias, instancia); //ESTO ES CUANDO LLEGA UNA CONEXION DE UNA INSTANCIA LO METO AL FINAL DE LA LISTA
	return NULL; //NO IMPORTA LO QUE DEVUELTA POR QUE ES ESCRITURA
}
