#include "Coordinador.h"
t_list* instancias;
t_list* semaforosInstancias;
int cantidadDeInstancias;
sem_t esperaInicializacion;
sem_t semaforoEsi;
sem_t semaforoPlanificador;
t_esi_operacion paqueteAEnviar;
int operacionValida;
pthread_mutex_t mutexInstancias;
instancia* (*algoritmoDeDistribucion)();
char* claveAComunicar;
char* operacion;

int main(){
	mkdir("../Logs/", 0777); // creo carpeta Logs
	sem_init(&esperaInicializacion,0,0);
	sem_init(&semaforoEsi,0,0);
	sem_init(&semaforoPlanificador,0,0);
	pthread_mutex_init(&mutexInstancias,NULL);
	cantidadDeInstancias =0;
	semaforosInstancias = list_create();
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
	pthread_t hiloPlanificador;
	if((nuevoCliente = accept(listener, (struct sockaddr *)&their_addr,&addrlen))==-1){ //NOS CONECTAMOS CON EL PLANIFICADOR
	    log_error(logger, "No se aceptar la conexion");
	    log_destroy(logger);
	    return -1;
	}
	pthread_create(&hiloPlanificador,NULL,conexionPlanificador,(void*)&nuevoCliente);
	algoritmoDeDistribucion = obtenerAlgoritmoDistribucion(); //OBTENGO EL ALGORITMO DE DISTRUBUCION SEGUN EL ARCHIVO CONFIG
	log_info(logger, "Se acepto la conexion");
	pthread_t* hilosInstancias = NULL;
	pthread_t hiloEsi = 0;
	instancias = list_create();
	//ACA COMIENZA LO DIVERTIDO =)
	sem_wait(&esperaInicializacion);
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
			pthread_join(hiloEsi,NULL);
			log_info(logger,"El cliente es ESI");
			if((pthread_create(&hiloEsi , NULL , conexionESI, (void*)&nuevoCliente)) < 0) //HAY UN HILO QUE VA ATENDER LA CONEXION CON EL ESI
	    	{
				log_error(logger,"No se pudo crear un hilo");
				return -1;
	    	}
			sem_wait(&esperaInicializacion);
			log_info(logger,"Se asigno una conexion con hilos al ESI");
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

void* conexionPlanificador(void* cliente){
	int sockPlanificador = *(int*)cliente;
	sem_post(&esperaInicializacion);
	while(1){
		sem_wait(&semaforoPlanificador);
		send(sockPlanificador,operacion,2,0);
		enviarCantBytes(sockPlanificador,claveAComunicar);
		log_info(logger,"Se envio la operacion %s con la clave %s", operacion,claveAComunicar);
		send(sockPlanificador,claveAComunicar,strlen(claveAComunicar)+1,0);
		if(operacion[0] == 'n' || operacion[0] == 'v'){
			char* resultado = malloc(2);
			recv(sockPlanificador,resultado,2,0);
			operacionValida = resultado[0]-48;
			free(resultado);
		}
		sem_post(&semaforoEsi);
	}
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
	//pthread_mutex_unlock(&mutexInstancias);
	log_warning(logger,"No se encontro la instancia :c");
	return NULL; //ES NULL SI NO ENCUENTRO NINGUNA INSTANCIA QUE LA USE
}


void liberarClave(instancia* instancia,char* clave){
	operacion="l";
	sem_post(&semaforoPlanificador);
	int j = 0;
	while(strcmp(clave,list_get((*instancia).clavesBloqueadas,j)) != 0){ //BUSCO HASTA ENCONTRAR LA CLAVE ASOCIADA
		j++;
	}
	list_remove_and_destroy_element((*instancia).clavesBloqueadas,j,free);//ASIGNO EL PUNTERO A UN AUXILIAR PARA HACER EL FREE
	if(!list_get((*instancia).clavesBloqueadas,0)){
		list_destroy((*instancia).clavesBloqueadas);
		(*instancia).clavesBloqueadas = NULL;
	}
	sem_wait(&semaforoEsi);
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


int verificacionEsi(char* loQueEnvio){
	operacion = loQueEnvio;
	sem_post(&semaforoPlanificador);
	log_info(logger,"Enviado los datos del esi");
	sem_wait(&semaforoEsi);
	log_info(logger,"Se confirmo el resultado: %d",operacionValida);

}

void *conexionESI(void* nuevoCliente) //REFACTORIZAR EL FOKEN SWITCH
{
	int socketEsi = *(int*)nuevoCliente;
	sem_post(&esperaInicializacion);
    int recvValor;
    instancia* instanciaAEnviar;
    char* resultadoEsi;

   if((recvValor = recibir(socketEsi,&paqueteAEnviar)) >0){
    	t_config* config = config_create(pathCoordinador);
    	sleep(config_get_int_value(config,"Retardo"));
    	config_destroy(config);
    	switch (paqueteAEnviar.keyword){
    	case GET:
    		log_info(logger,"Estamos haciendo un GET");
    		claveAComunicar = paqueteAEnviar.argumentos.GET.clave;
    		verificacionEsi("n");
    		if(!operacionValida){ //VERIFICO SI LA CLAVE ESTA TOMADA
    			operacion = "b";
    			sem_post(&semaforoPlanificador);
    			sem_wait(&semaforoEsi);
    			free(paqueteAEnviar.argumentos.GET.clave);
    			resultadoEsi = "b";
    			break;
    		}
    		log_info(logger,"Se puede realizar el GET");
    		while(true){
    		instanciaAEnviar = algoritmoDeDistribucion(NULL); //BUSCO UNA INSTANCIA CON estaDisponible == 1.
    		log_info(logger,"La instancia elegida es %s, con el semaforo nro: %d",(*instanciaAEnviar).nombreInstancia, (*instanciaAEnviar).nroSemaforo);
    		operacion = "p";
    		sem_post(list_get(semaforosInstancias,(*instanciaAEnviar).nroSemaforo));  //LE DIGO A LA INSTANCIA QUE TRABAJE
    		algoritmoDeDistribucion(instanciaAEnviar);
    		sem_wait(&semaforoEsi);
    		if(operacionValida){
    			operacion = "b";
    			sem_post(&semaforoPlanificador);
    			agregarClave(instanciaAEnviar,paqueteAEnviar.argumentos.GET.clave);
    			sem_wait(&semaforoEsi);
    			free(paqueteAEnviar.argumentos.GET.clave);
    	    	resultadoEsi = "e";
    			break;
    		}
    		(*instanciaAEnviar).estaDisponible = 0; //COMO LA OPERACION NO ES VALIDA SIGNIFICA QUE HUBO UN ERROR CON LA CONEXION DE LA INSTANCIA, POR LO TANTO LO DEJO EN FALSE.
    		}
    		break;
    	case SET:
    		log_info(logger,"Estamos haciendo un SET");
    		if(!validarYenviarPaquete(paqueteAEnviar.argumentos.SET.clave, socketEsi)){
    			resultadoEsi = "a";
    		}
    		free(paqueteAEnviar.argumentos.SET.clave);
    	    free(paqueteAEnviar.argumentos.SET.valor);
        	resultadoEsi = "e";
    		break;
    	case STORE:
    		log_info(logger,"Estamos haciendo un STORE");
    		if(!validarYenviarPaquete(paqueteAEnviar.argumentos.STORE.clave, socketEsi)){
    			resultadoEsi = "a";
    		}
    		instanciaAEnviar = buscarInstancia(paqueteAEnviar.argumentos.STORE.clave);
    		liberarClave(instanciaAEnviar,paqueteAEnviar.argumentos.STORE.clave);
    		free(paqueteAEnviar.argumentos.STORE.clave);
        	resultadoEsi = "e";
    		break;
    	default:
    		log_error(logger,"Operacion invalida");
    		resultadoEsi = "a";
    		break;
    	}
    }//GET SET STORE IMPLEMENTACION
    if(recvValor == 0)
            log_info(logger,"Se desconecto un ESI");
        else if(recvValor == -1){
            log_error(logger,"Error al recibir el tam del codigo serializado");
            resultadoEsi = "a";
        }
    send(socketEsi,resultadoEsi,2,0);
    close(socketEsi); //SE OPERA SENTENCIA POR SENTENCIA POR LO TANTO LO CERRAMOS Y ESPERAMOS SU CONEXION DEVUELTA
    return 0;
}

int validarYenviarPaquete(char* clave, int socketEsi) {
	claveAComunicar = clave;
	instancia* instanciaAEnviar;
	verificacionEsi("v");
	if (!operacionValida) {
		//VERIFICO SI LA CLAVE ESTA TOMADA POR EL MISMO ESI
		return 0;
	}
	instanciaAEnviar = buscarInstancia(clave); //BUSCO LA INSTANCIA QUE CONTIENE TAL CLAVE
	log_info(logger,"Se encontro la instancia con tal clave");
	log_info(logger,"La instancia a enviar es: %s",(*instanciaAEnviar).nombreInstancia);
	operacion = "p";
	sem_post(list_get(semaforosInstancias,(*instanciaAEnviar).nroSemaforo)); //LE AVISO A LA INSTANCIA QUE ES HORA DE ACTUAR
	sem_wait(&semaforoEsi);
	if (!operacionValida) {
		return 0;
	}
	return 1;
}

instancia* crearInstancia(int sockInstancia,char* nombreInstancia,int* cantidadDeEntradas){
	instancia* instanciaNueva = NULL;
	if((instanciaNueva = existeEnLaLista(nombreInstancia))!=NULL){
		(*instanciaNueva).estaDisponible = 1;
		cantidadDeEntradas = (*instanciaNueva).cantEntradasDisponibles;
		send(sockInstancia,"r",2,0);//SE LE MANDA R DE QUE ES UNA RECONEXION POR QUE ESTA EN LA LISTA.
		log_info(logger,"Es una reconexion de la instancia %s", nombreInstancia);
		return NULL; //Si ya existia la instancia en la lista no me hace falta seguir operando
	}
	instanciaNueva = malloc(sizeof(instancia));
	(*instanciaNueva).cantEntradasDisponibles = cantidadDeEntradas;
	inicializarInstancia(instanciaNueva,nombreInstancia); //Inicializamos la instancia.
	return instanciaNueva;
}

algoritmo obtenerAlgoritmoDistribucion(){
	t_config *config=config_create(pathCoordinador);
	log_info(logger,"Vamos a seleccionar un algoritmo");
	char* algoritmo = config_get_string_value(config,"AlgoritmoDeDistribucion");
	if(strcmp("EL",algoritmo)==0){
		log_info(logger,"Usted eligio el algoritmo Equitative Load");
		config_destroy(config);
		return &equitativeLoad;
	}
	if(strcmp("KE",algoritmo)==0){
		log_info(logger,"Usted eligio el algoritmo KE");
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

void inicializarInstancia(instancia* instanciaNueva,char* nombreInstancia){
	t_config *config=config_create(pathCoordinador);
	(*(*instanciaNueva).cantEntradasDisponibles) = config_get_int_value(config, "CantidadEntradas");
	config_destroy(config);
	(*instanciaNueva).clavesBloqueadas = NULL;
	(*instanciaNueva).estaDisponible = 1;
	(*instanciaNueva).nombreInstancia = malloc(string_length(nombreInstancia)+1);
	strcpy((*instanciaNueva).nombreInstancia,nombreInstancia);
	sem_t* semInstancia = malloc(sizeof(sem_t));
	if(sem_init(semInstancia,0,0)<0){
		log_error(logger,"No se pudo inicializar el semaforo nro: %d",cantidadDeInstancias);
	}
	list_add(semaforosInstancias,semInstancia);
	(*instanciaNueva).nroSemaforo = cantidadDeInstancias;
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

instancia* lsu(instancia* instanciaAUsar){
	if(instanciaAUsar == NULL){
		int i=1;
		instancia* instanciaAux;
		instanciaAUsar = list_get(instancias,0);
		pthread_mutex_lock(&mutexInstancias);
		do{
			instanciaAux = list_get(instancias,i);
			if(instanciaAux!=NULL && (*(*instanciaAux).cantEntradasDisponibles) > (*(*instanciaAUsar).cantEntradasDisponibles)){
				instanciaAUsar = instanciaAux;
			}
			i++;
		}while(instanciaAux !=NULL);
		pthread_mutex_unlock(&mutexInstancias);
		log_info(logger,"Se logro realizar el lsu %s",instanciaAUsar!=NULL?(*instanciaAUsar).nombreInstancia:"Es null csm");
		return instanciaAUsar;
	}
	return NULL;
}

int contarCantidadDeInstanciasDisponibles(){
	int i = 0;
	instancia* unaInstancia;
	while((unaInstancia = list_get(instancias,i))!= NULL){
		if((*unaInstancia).estaDisponible)
		i++;
	}
	return i;
}

int obtenerLetra(){
	int restarSegunMayusOMinus = 65;
	if(claveAComunicar[0] >= 97){
		restarSegunMayusOMinus = 97;
	}
	return claveAComunicar[0]-restarSegunMayusOMinus;
}

instancia* keyExplicit(instancia* instancia){
	if(instancia == NULL){
		int cantidadInstancias = contarCantidadDeInstanciasDisponibles();
		int rangoAscii = 25/cantidadInstancias;
		if(25%cantidadInstancias){
			rangoAscii++;
		}
		log_info(logger,"El rango ascii es %d",rangoAscii);
		int nroInstancia = 0;
		int i =0;
		int letraABuscar = obtenerLetra();
		while((instancia = list_get(instancias,i))){
			if((*instancia).estaDisponible){
				if(letraABuscar>= nroInstancia*rangoAscii && letraABuscar <= (nroInstancia+1)*rangoAscii){
					return instancia;
				}
				nroInstancia++;
			}
			i++;
		}
	}
	return NULL;
}

void compactacionSimultanea(int semaforoNoNecesario){
	int nroSemaforo = 0;
	sem_t* semaforo;
	log_info(logger,"Se va realizar una compactacion simultantea");
	while((semaforo =list_get(semaforosInstancias,nroSemaforo)) != NULL){
		if(nroSemaforo != semaforoNoNecesario){
			operacion = "c";
			sem_post(semaforo);
		}
		nroSemaforo++;
	}
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
	int cantidadDeEntradasRestantes;
	instancia* instanciaNueva = crearInstancia(socketInstancia,buff,&cantidadDeEntradasRestantes);
	free(buff);
	if(instanciaNueva != NULL){ //Si es NULL significa que es una reconexion!, por lo tanto no hace falta meterlo en la lista!
		list_add(instancias,instanciaNueva);
	}
	sem_post(&esperaInicializacion);
	//ACA TERMINE DE INICIALIZAR LA INSTANCIA
	int  recvValor;
	while(true){
		sem_wait(list_get(semaforosInstancias,nroSemaforo));
		log_info(logger,"Se asigno la tarea a la instancia con el semaforo: %d",nroSemaforo);
		send(socketInstancia,operacion,2,0); //SE LE ENVIA UN "p" DE PAQUETE PARA DECIRLE QUE SE LE VA ENVIAR UNA SENTENCIA.
		if(operacion[0]=='p'){
			buff = malloc(2);
			buff[0]='n';
			enviar(socketInstancia,paqueteAEnviar);
			while(buff[0]!= 'r'){
				if(recv(socketInstancia,buff,2,0)<=0){ //ESPERO EL RESULTADO DE LA INSTANCIA
					log_warning(logger,"Se habia desconectado la instancia con el semaforo: %d",nroSemaforo);
					operacionValida=0; //SI HUBO UN ERROR EN LA CONEXION O LA INSTANCIA SE DESCONECTO
					sem_post(&semaforoEsi);
					free(buff);
					return 0;
				}
				switch(buff[0]){
					case 'c':
						compactacionSimultanea(nroSemaforo);
						break;
					case 'r':
						operacionValida=1;
						int bytes = obtenerTamDelSigBuffer(socketInstancia);
						char* cantidadDeEntradasARestar = malloc(sizeof(bytes));
						recv(socketInstancia,cantidadDeEntradasARestar,bytes,0);
						cantidadDeEntradasRestantes=transformarNumero(cantidadDeEntradasARestar,0);
						free(cantidadDeEntradasARestar);
						sem_post(&semaforoEsi);
						break;
				}
			}
			free(buff);
		}

	}
}
