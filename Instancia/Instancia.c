#include "Instancia.h"

struct TE{
	char clave[40]; //Clave
	char* entrada; //Direcciones de memoria de las entradas de la clave
	int tamValor; //Tamanio del valor asociado a la clave
	int seAlmacenoElValor;
}typedef tablaEntradas;

t_list* tablas;
int entradasTotales;
int tamEntradas;
t_list* entradas;
pthread_mutex_t mutexAlmacenamiento;
char* path;
t_list* bitArray;
int sockcoordinador;

int main(){
    logger =log_create(logInstancias,"Instancia",1, LOG_LEVEL_INFO);
    t_config* config = config_create(pathInstancia);
    if((sockcoordinador =crearConexionCliente(config_get_int_value(config,"Puerto"),config_get_string_value(config,"Ip"))) == -1){
    	config_destroy(config);
    	log_error(logger,"Error en la conexion con el coordinador");
    	return -1;
    }
    log_info(logger,"Se realizo correctamente la conexion con el coordinador");
    enviarTipoDeCliente(sockcoordinador,INSTANCIA);
    inicializarTablaEntradas();
    char *buff = config_get_string_value(config, "nombreInstancia"); //obtengo el id
    enviarCantBytes(sockcoordinador,buff);
    send(sockcoordinador,buff,string_length(buff) + 1,0); //ENVIO EL NOMBRE DE LA INSTANCIA
    config_destroy(config); //NO HACE FALTA HACER FREE AL BUFF YA QUE EL CONFIG DESTROY LO HACE SOLO
    config = config_create(pathInstancia);
    path = config_get_string_value(config,"PuntoMontaje");
    mkdir(path, 0777); // creo carpeta montaje SI NO ESTA HECHA
    int recvValor;
    buff = malloc(2);
    t_esi_operacion paquete;
    pthread_t id;
    tablas = list_create();
    pthread_create(&id,NULL,hacerDump,NULL);
    pthread_mutex_init(&mutexAlmacenamiento,NULL);
    while((recvValor =recv(sockcoordinador,buff,2,0))>0){
    	switch(buff[0])
	{
    		case 'r': //RECONEXION LE PIDO AL COORDINADOR CUALES FUERON LAS CLAVES BLOQUEADAS EN ESTA INSTANCIA
    			break;
    		case 'p': //RECIBI UN PAQUETE GET SET O STORE
    			log_info(logger,"Recibi un paquete");
    			if((recvValor = recibir(sockcoordinador,&paquete))<=0){
    				free(buff);
    				exit(-1);
    			}
    			log_info(logger,"Recibi la operacion");
    			manejarPaquete(paquete);

    			break;
    		case 'v':
    			send(sockcoordinador,"v",2,1); //LE DIGO AL COORDINADOR QUE SIGO VIVO
    			break;
    		case 'c':
    			compactacion();
    			break;
    	}
   }
    config_destroy(config);
    free(buff);
    log_destroy(logger);
    close(sockcoordinador);
	 return 0;
}

int cuantasEntradasAMover(int posicionDelArray){
	int cantEntradasLibres = 1;
	char* bit = list_get(bitArray,posicionDelArray);
	while(posicionDelArray< entradasTotales && !(bit[0]-48)){
		posicionDelArray++;
		bit = list_get(bitArray,posicionDelArray);
		cantEntradasLibres++;
	}
	return cantEntradasLibres;
}

void compactacion(){
	char* posicionConProblemas;
	int posicionDelArray=0;
	log_warning(logger,"Se va realizar una compactacion");
	while(posicionDelArray < entradasTotales){
		char* bit =list_get(bitArray,posicionDelArray);
		if(!(bit[0]-48)){
			int cantEntradasAMover = cuantasEntradasAMover(posicionDelArray+1);
			log_info(logger,"Se va compactar desde la posicion %d hasta %d",posicionDelArray,cantEntradasAMover);
			if(posicionDelArray+cantEntradasAMover < entradasTotales){ //VERIFICO QUE NO COMPACTE AL PEDO
				while(cantEntradasAMover !=0){
					posicionConProblemas = list_remove(entradas,posicionDelArray); //LO SACO
					list_add(entradas,posicionConProblemas); //Y LO METO AL FINAL DE LA LISTA
					posicionConProblemas = list_remove(bitArray, posicionDelArray); //LO MISMO PARA EL BIT ARRAY
					log_info(logger,"Posicion del array: %d con estado %c",posicionDelArray,posicionConProblemas[0]);
					list_add(bitArray,posicionConProblemas);
					cantEntradasAMover--;
					posicionDelArray++;
				}
			}else break; //EN ESTE CASO NO ME HACE FALTA SEGUIR
		}
		posicionDelArray++;
	}
}

void inicializarBitArray(){
	bitArray = list_create();
	for(int i =0;i<entradasTotales;i++){
		char* bit = malloc(1);
		bit[0] = '0';
		list_add(bitArray,bit);
	}
}

void inicializarTablaEntradas(){
    int tam = obtenerTamDelSigBuffer(sockcoordinador);
    char* buff = malloc(tam);
    recv(sockcoordinador,buff, tam , 0);
    entradasTotales= transformarNumero(buff,0);

    log_info(logger,"La cantidad de entradas es %d",  entradasTotales);
    entradas = malloc(sizeof(char*)*entradasTotales); //INICIALIZO LA TABLA
    inicializarBitArray();
    free(buff);
    tam = obtenerTamDelSigBuffer(sockcoordinador);
    buff = malloc(tam);
    recv(sockcoordinador,buff, tam , 0);
    tamEntradas = transformarNumero(buff,0);
    log_info(logger,"El tamagno de entradas es %d", tamEntradas);
    int i = 0;
    entradas=list_create();
    char* unaEntrada;
    while(i < entradasTotales){
    	unaEntrada = malloc(tamEntradas+1);	 //ASIGNO EL TAMAGNO DE UNA ENTRADA + EL /0
    	list_add(entradas,unaEntrada); //METO UNA ENTRADA A LAS ENTRADAS :v
    	i++;
    }
    free(buff);
}
void* hacerDump(){
	while(1){
		t_config *config=config_create(pathInstancia);
		sleep(config_get_int_value(config,"dump"));	//Permite la ejecucion de manera periodica del dump
		config_destroy(config);
		almacenarTodaInformacion();
	}
}

void almacenarInformacionDeTalPosicionDeLaTabla(int posTabla){
	tablaEntradas* tabla = list_get(tablas,posTabla);
		if(!(*tabla).seAlmacenoElValor){
		pthread_mutex_lock(&mutexAlmacenamiento);	//Garantizo mutua exclusion al ejecutar la seccion critica
		char* aux = string_new();
		string_append(&aux,path);
		char* valor = string_new();
		int cantidadEntradasALeer = (*tabla).tamValor/tamEntradas;
		if((*tabla).tamValor%tamEntradas){
			cantidadEntradasALeer++;
		}
		int posEntrada = posicionDeLaEntrada((*tabla).entrada);
		log_trace(logger,"La posicion de entrada es %d",posEntrada);
		log_trace(logger,"La cantidad de entradas a leer son %d",cantidadEntradasALeer);
		for(int j = 0; j<cantidadEntradasALeer; j++){
			char* entrada = list_get(entradas,posEntrada);
			log_trace(logger,"La entrada a almacenar %s",entrada);
			string_append(&valor,entrada);
			posEntrada++;
		}
		log_trace(logger,"Se va almacenar el valor %s de la clave %s",valor, (*tabla).clave);
		string_append(&aux,(*tabla).clave);
		int desc = open(aux, O_RDWR | O_CREAT | O_TRUNC, 0777);
		free(aux);
		ftruncate(desc,strlen(valor));
		char* map = mmap(NULL,strlen(valor),PROT_WRITE,MAP_SHARED,desc,0);
		memcpy(map,valor,strlen(valor));
		munmap(map,strlen(valor));
		close(desc);
		free(valor);
		(*tabla).seAlmacenoElValor = 1;
		pthread_mutex_unlock(&mutexAlmacenamiento); //Garantizo mutua exclusion al ejecutar la seccion critica
	}
}

void almacenarTodaInformacion(){
	int i = 0;
	tablaEntradas* tabla = list_get(tablas,i);
	while(tabla && (*tabla).entrada !=NULL)
	{
		almacenarInformacionDeTalPosicionDeLaTabla(i);
		i++;
		tabla = list_get(tablas,i);
	}
}

int encontrarTablaConTalClave(char clave[40]){
	int i=0;
	tablaEntradas* tabla = list_get(tablas,i);
	while(strcmp((*tabla).clave,clave)!=0)
	{
		i++;
		tabla=list_get(tablas,i);
	}
	return i;
}

int posicionDeLaEntrada(char* entradaABuscar){
	for(int i=0;i<entradasTotales;i++){
		char* entrada = list_get(entradas,i);
		if(entrada == entradaABuscar){
			return i;
		}
	}
	return -1;
}

void liberarClave(int posTabla){
	tablaEntradas* tabla = list_remove(tablas,posTabla);
	int posEntrada = posicionDeLaEntrada((*tabla).entrada);
	int cantEntradasAOcupadas = (*tabla).tamValor/tamEntradas;
	if((*tabla).tamValor%tamEntradas){
		cantEntradasAOcupadas++;
	}
	for(int i = 0;i<cantEntradasAOcupadas;i++){
		char* bit = list_get(bitArray,posEntrada+i);
		bit[0]='0'; //LIBERO :D
	}
	free(tabla);
}

void enviarEntradasRestantes(){
	int cantidadEntradas = 0;

	for(int i =0;i<entradasTotales;i++){
		char* bit = list_get(bitArray,i);
		if(!(bit[0]-48)){
			cantidadEntradas++;
		}
	}
	char* buff = string_itoa(cantidadEntradas);
	enviarCantBytes(sockcoordinador,buff);
	send(sockcoordinador,buff,strlen(buff)+1,0);
	free(buff);
	return;
}

void manejarPaquete(t_esi_operacion paquete){
	int posTabla;
	switch(paquete.keyword){
		case GET:
			log_info(logger,"Se selecciono el caso GET");
			meterClaveALaTabla(paquete.argumentos.GET.clave);
			free(paquete.argumentos.GET.clave);
			break;
		case SET:
			log_info(logger,"Se selecciono el caso SET");
			posTabla = encontrarTablaConTalClave(paquete.argumentos.SET.clave);
			tablaEntradas* tabla = list_get(tablas,posTabla);
			pthread_mutex_lock(&mutexAlmacenamiento);
			if((*tabla).entrada!=NULL){ //ME FIJO SI YA TENIA UN VALOR ASIGNADO
				int cantEntradasNecesarias = strlen(paquete.argumentos.SET.valor)/tamEntradas;
				if(strlen(paquete.argumentos.SET.valor)%tamEntradas){
					cantEntradasNecesarias++;
				}
				int cantEntradasOcupadas = (*tabla).tamValor/tamEntradas;
				if((*tabla).tamValor%tamEntradas){
					cantEntradasOcupadas++;
				}
				log_info(logger,"Ya tenia un valor asignado");
				log_info(logger,"La cantidad de entradas ocupadas son %d y las que necesita son %d",cantEntradasOcupadas,cantEntradasNecesarias);
				if(cantEntradasNecesarias>cantEntradasOcupadas){
					//QUE MIERDA HAGO ACA D:
				}else{
					int posEntrada = posicionDeLaEntrada((*tabla).entrada);
					int j = 0;
					while(j < cantEntradasNecesarias){
						char* entrada =	list_get(entradas,posEntrada);
						char* valorEntrada = string_substring(paquete.argumentos.SET.valor,tamEntradas*j,tamEntradas*(j+1));
						valorEntrada[tamEntradas] = '\0';
						strcpy(entrada,valorEntrada);
						free(valorEntrada);
						posEntrada++;
						j++;
					}
					while(j<cantEntradasOcupadas){
						char* bit = list_get(bitArray,posEntrada);
						bit[0]='0';
						posEntrada++;
						j++;
					}
					(*tabla).seAlmacenoElValor=0;
				}
			}else{
				log_info(logger,"Estamos metiendo el valor %s de la clave %s",paquete.argumentos.SET.valor,paquete.argumentos.SET.clave);
				meterValorParTalClave(paquete.argumentos.SET.valor,posTabla);
			}
			(*tabla).tamValor = strlen(paquete.argumentos.SET.valor);
			pthread_mutex_unlock(&mutexAlmacenamiento);
			free(paquete.argumentos.SET.clave);
			free(paquete.argumentos.SET.valor);
			break;
		case STORE:
			log_info(logger,"Se selecciono el caso STORE");
			posTabla = encontrarTablaConTalClave(paquete.argumentos.STORE.clave);
			almacenarInformacionDeTalPosicionDeLaTabla(posTabla);
			liberarClave(posTabla);
			free(paquete.argumentos.STORE.clave);
			break;
	}
	if(send(sockcoordinador,"r",2,0)==-1)
	{
		log_error(logger, "No se pudo enviar el mensaje de respuesta al Coordinador");
	}
	else
	{
		log_info(logger, "Mensaje enviado correctamente");
		enviarEntradasRestantes();
	}
}

void meterClaveALaTabla(char* clave){
	tablaEntradas* tabla = malloc(sizeof(tablaEntradas));
	strcpy((*tabla).clave,clave);
	(*tabla).entrada = NULL;
	(*tabla).tamValor = 0;
	(*tabla).seAlmacenoElValor = 0;
	list_add(tablas,tabla);
	char* pathCompleto = malloc(strlen(path)+1);
	strcpy(pathCompleto,path);
	string_append(&pathCompleto,clave);
	int desc = open(pathCompleto, O_CREAT | O_TRUNC,0777); //CREA EL ARCHIVO
	free(pathCompleto);
	close(desc);
}

int llegaAOcuparTodasLaEntradas(int* posicion,int* hayQueCompactar,int cantEntradasAOcupar){
	int i = 1;
	while(i < cantEntradasAOcupar && (*posicion) < entradasTotales){
		char * bit = list_get(bitArray,(*posicion));
		if(bit[0]-48){
			(*hayQueCompactar)=1;
			return 0; //COMO EN LA SIGUIENTE POSICION ESTA OCUPADO HAY FRAGMENTACION EXTERNA, ESTO PROVOCA QUE HAY QUE COMPACTAR SI NO LLEGO A ENCONTRAR LUGAR
		}
		log_info(logger,"Posicion libre: %d entradas permitidas: %d",(*posicion),i);
		(*posicion)++;
		i++;
	}
	if(i == cantEntradasAOcupar){
		return 1; //TENGO ESPACIO PARA INGRESAR EL VALOR :D
	}
	return 0; //ESTO SIGNIFICA QUE YA ME LEI TODAS LAS ENTRADAS POR LO TANTO GG
}

int obtenerPrimeraPosicionPermitida(int cantEntradasAOcupar){
	int encontrado = 0;
	int primeraPosicionEncontrada;
	int hayQueCompactar;
	char* valorCoordi;
	while(1){
		int posicion = 0;
		hayQueCompactar = 0;
		while(!encontrado && posicion < entradasTotales){
			char* bit = list_get(bitArray,posicion);
			if(!(bit[0]-48)){ //SI LA POSICION NO ESTA OCUPADA TENGO QUE VERIFICAR QUE LLEGA ENTRAR EL VALOR
				primeraPosicionEncontrada = posicion;
				posicion ++;
				encontrado = llegaAOcuparTodasLaEntradas(&posicion,&hayQueCompactar,cantEntradasAOcupar); //VERIFICO SI CUMPLE LA CANTIDAD DE ENTRADAS A METER Y SI HAY QUE COMPACTAR
			}
			posicion ++;
		}
		log_warning(logger,"El valor de encontrado es %d",encontrado);
		if(encontrado){
			return primeraPosicionEncontrada;
		}else if(!hayQueCompactar){
			return -1; //SI NO HACE FALTA COMPACTAR TENGO QUE HACER EL ALGORITMO DE REEMPLAZO
		}
		send(sockcoordinador,"c",2,0); //ENVIO UNA SOLICITUD DE COMPACTACION AL COORDINADOR
		compactacion();
	}
}

void meterValorParTalClave(char*valor,int posTabla){
	int cantEntradasAOcupar = (string_length(valor))/tamEntradas;
	log_info(logger,"La cantidad de entradas a ocupar son: %d",cantEntradasAOcupar);
	if(string_length(valor)%tamEntradas){
		cantEntradasAOcupar++;
	}
	int posicionPermitidaParaOcupar = obtenerPrimeraPosicionPermitida(cantEntradasAOcupar);
	if(posicionPermitidaParaOcupar !=-1){
		tablaEntradas* tablaEntrada = list_get(tablas,posTabla);
		(*tablaEntrada).entrada = list_get(entradas,posicionPermitidaParaOcupar);
		for(int j =0;j<cantEntradasAOcupar;j++){
			char* valorEntrada = string_substring(valor,tamEntradas*j,tamEntradas*(j+1));
			valorEntrada[tamEntradas] = '\0';
			log_warning(logger,"El valor a meter es %s desde %d hasta %d",valorEntrada,tamEntradas*j,tamEntradas*(j+1));
			char* entrada = list_get(entradas,posicionPermitidaParaOcupar);
			char* bit = list_get(bitArray,posicionPermitidaParaOcupar);
			bit[0]='1';
			strcpy(entrada,valorEntrada);
			log_info(logger,"El valor se copio en la posicion %d con el valor %s",posicionPermitidaParaOcupar,entrada);
			free(valorEntrada);
			posicionPermitidaParaOcupar++;
		}
	}
	else{
		log_info(logger,"Tengo que hacer el algoritmo de reemplazo D:");
		//ALGORITMO DE SUSTITUCION
	}
}

