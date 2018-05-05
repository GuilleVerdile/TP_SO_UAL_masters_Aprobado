#include "CoordinadorHilos.h"
t_list* instancias;

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
	int casoDiscriminador;
	if (listen(listener, 10) == -1){ //ESCUCHA!
	    log_error(logger, "No se pudo escuchar");
		log_destroy(logger);
		return -1;
	}
	log_info(logger, "Se esta escuchando");
	addrlen = sizeof(their_addr);
	if((casoDiscriminador = accept(listener, (struct sockaddr *)&their_addr,&addrlen))==-1){ //NOS CONECTAMOS CON EL PLANIFICADOR
	    log_error(logger, "No se aceptar la conexion");
	    log_destroy(logger);
	    return -1;
	}
	log_info(logger, "Se acepto la conexion");
	printf("Nuevo cliente, se conecto el Planificador\n");
	fflush(stdout);
	if(send(casoDiscriminador,"Hola capo soy el Coordinador\n",1024,0)==-1){
	    log_error(logger, "No se pudo enviar el mensaje");
	}
	else
	    log_info(logger, "Mensaje enviado correctamente");
	pthread_t idHilo;
	//ACA COMIENZA LO DIVERTIDO =)
	while((nuevoCliente = accept(listener, (struct sockaddr *)&their_addr,&addrlen))) //Esperamos a que llegue la primera conexion
	{
		log_info(logger,"Se acepto una nueva conexion");
		int tipoCliente = esEsi(nuevoCliente);
		if(tipoCliente == 0){ //VERIFICO SI ES ESI O INSTANCIA, SI ES 0 SIGNIFICA ESI
			log_info(logger,"El cliente es ESI");
			if(pthread_create(&idHilo , NULL , conexionESI, (void*) &nuevoCliente) < 0)
	    	{
				log_error(logger,"No se pudo crear un hilo");
				return -1;
	    	}
			log_info(logger,"Se asigno una conexion con hilos");
			pthread_join(idHilo,NULL);//No vamos a usar esta implementacion pero se usa para testear
		}else if(tipoCliente == 1){ //EN CASO DE QUE DE 1 ES INSTANCIA
			log_info(logger,"El cliente es INSTANCIA");
			t_config *config=config_create(pathCoordinador);
			char* buff=config_get_string_value(config, "CantidadEntradas");
			string_append(&buff,"z");
			string_append(&buff,config_get_string_value(config, "TamagnoEntradas"));
			config_destroy(config);
			enviarCantBytes(nuevoCliente,buff);
			send(nuevoCliente,buff,string_length(buff)+1,0); //Le envio la cantidad de entradas y tamagno, tiene un z en el medio para separarlas!
			free(buff);
			int tam = obtenerTamDelSigBuffer(nuevoCliente,NULL);
			buff = malloc(tam);
			recv(nuevoCliente,buff,tam,0);
			instancia* instanciaNueva = crearInstancia(nuevoCliente,buff);
			free(buff);
			if(instanciaNueva != NULL){ //Si es NULL significa que es una reconexion!
				algoritmoDeDistribucion(instanciaNueva);//LO METO EN LA LISTA SEGUN EL ALGORITMO DE DIST USADO
			}
		}
	}
	if(nuevoCliente <0){
		log_error(logger,"No se pudo aceptar la conexion al cliente");
		return -1;
	}
	return 0;
}

void *conexionESI(void* cliente)
{
	log_info(logger,"ESTAMOS DENTRO DEL HILO");
    int socketEsi = *(int*)cliente; //Lo casteamos
    int tam;
    char* buff;
    instancia* instanciaAEnviar = algoritmoDeDistribucion(NULL); //[3,4,5,1,2]

 /*  while((tam = obtenerTamDelSigBuffer(socketEsi,*sockInstancia))>0){
    	buff = malloc(tam);
    	recv(socketEsi,buff,tam,0);
    	printf("%s\n",buff);
    	fflush(stdout);
    	send(*sockInstancia,buff,tam,0);
    	free(buff);
    }*/ //GET SET STORE IMPLEMENTACION

   algoritmoDeDistribucion(instanciaAEnviar);
    if(tam == 0)
    {
        log_info(logger,"Se desconecto un ESI");
    }
    else if(tam == -1)
    {
        log_error(logger,"Error al recibir el tam del codigo serializado");
    }

    close(socketEsi);
    return 0;
}

int esEsi(int socket){
	int recvValor;
	char* buff = malloc(2);
	int esEsi;
	if((recvValor = recv(socket, buff, 2, 0) > 0)){
		esEsi= strcmp(buff,ESI); //SI RECIBO ESI ENTONCES NO PROBLEM SI ES MAYOR A 0 QUIERE DECIR QUE ES INSTANCIA
		free(buff);
		return esEsi;
	}
	if(recvValor == 0){
		log_info(logger,"Se desconecto el cliente");
		free(buff);
		return -1; //POR OTRO LADO NO ESTA EL CASO DE QUE SEA MENOR A 0, ACA SIGNIFICA QUE SE CORTO LA CONEXION
	}else if(recvValor == -1){
		log_error(logger,"Error al recibir el tipo de cliente");
		free(buff);
		exit(-1);
	}
}

instancia* crearInstancia(int sockInstancia,char* nombreInstancia){
	instancia* instanciaNueva = NULL;
	if((instanciaNueva = existeEnLaLista(nombreInstancia))!=NULL){
		(*instanciaNueva).socket = sockInstancia;
		//reestablecerInformacionInstancia(instanciaNueva); ACA SE MODELA MAS TARDE PARA CUANDO UNA INSTANCIA SE TRATA DE RECONECTAR
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
		//lsu(sockInstancia);
		break;
	case KE: //KEYEXPLICIT
		//keyExplicit(sockInstancia);
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
	(*instanciaNueva).nombreInstancia = malloc(strlen(nombreInstancia)+1);
	strcpy((*instanciaNueva).nombreInstancia,nombreInstancia);
	(*instanciaNueva).socket = sockInstancia;
}

instancia* existeEnLaLista(char* id){
	int i =0;
	instancia* instancia;
	int noEncontrado = 1;
	while((noEncontrado != 0) && ((instancia = list_get(instancias,i))!= NULL)){
		noEncontrado = strcmp(id,(*instancia).nombreInstancia);
		if(noEncontrado == 0){
			(*instancia).socket = socket;
		}
		i++;
	}
	return instancia;
}

instancia* equitativeLoad(instancia* instancia){
	if(instancia == NULL){ //ES DE LECTURA SI ES NULL
		int i = 0;
		instancia = list_get(instancias,i);
		while(!(*instancia).estaDisponible){
			i++;
			instancia = list_get(instancias,i);
		}
		return list_remove(instancias,i);//SACA LA PRIMERA INSTANCIA DISPONIBLE Y LO ELIMINO (NO ESTA DISPONIBLE SI SURGIO UNA DESCONEXION CON EL SERVIDOR)
	}
	list_add(instancias, instancia); //ESTO ES CUANDO LLEGA UNA CONEXION DE UNA INSTANCIA LO METO AL FINAL DE LA LISTA
	return NULL; //NO IMPORTA LO QUE DEVUELTA POR QUE ES ESCRITURA
}
