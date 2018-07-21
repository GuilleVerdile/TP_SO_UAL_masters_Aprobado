#include "Instancia.h"


t_list* tablas;
int entradasTotales;
int tamEntradas;
t_list* entradas;
pthread_mutex_t mutexAlmacenamiento;
char* path;
t_list* bitArray;
int sockcoordinador;
void (*algoritmoDeReemplazo)();
int nroEntrada = 0;
int nroOperacion = 0;
int dump=1;
t_log* loggerReal;
int main(int argc, char**argv){
    logger =log_create(logInstancias,"InstanciaTest",0, LOG_LEVEL_INFO);
    loggerReal = log_create(logInstancias,"Instancia",1,LOG_LEVEL_INFO);
    if(argc > 1)
    	pathInstancia = argv[1];
    t_config* config = config_create(pathInstancia);
    if((sockcoordinador =crearConexionCliente(config_get_int_value(config,"Puerto"),config_get_string_value(config,"Ip"))) == -1){
    	config_destroy(config);
    	log_error(logger,"Error en la conexion con el coordinador");
    	return -1;
    }
    log_info(loggerReal,"Se realizo correctamente la conexion con el coordinador");
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
    algoritmoDeReemplazo = obtenerAlgoritmoDeReemplazo();
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
    			log_info(loggerReal,"Recibi una operacion");
    			manejarPaquete(paquete);
    			break;
    		case 'v':
    			send(sockcoordinador,"v",2,1); //LE DIGO AL COORDINADOR QUE SIGO VIVO
    			break;
    		case 'c':
    			compactacion();
    			break;
    		case 'o':
    			recvValor = obtenerTamDelSigBuffer(sockcoordinador);
    			free(buff);
    			buff = malloc(recvValor);
    			recv(sockcoordinador,buff,recvValor,0);
    			log_info(loggerReal,"Me pidieron la solicitud del valor de la clave: %s",buff);
    			enviarValor(buff);
    			free(buff);
    			buff= malloc(2);
    			break;
    		case 'l':
    			recvValor =obtenerTamDelSigBuffer(sockcoordinador);
    			buff = malloc(recvValor);
    			recv(sockcoordinador,buff,recvValor,0);
    			log_info(loggerReal,"El coordinador me aviso que libere la clave %s",buff);
    			buscarYLiberarClave(buff);
    			mostrarEstadoEntradas();
    			free(buff);
    			buff = malloc(2);
    			break;
    	}
   }
    log_error(loggerReal,"Se perdio la conexion con el coordinador");
    log_info(loggerReal,"Espere unos momentos para cerrar la aplicacion...");
    liberarListas(id);
    pthread_mutex_destroy(&mutexAlmacenamiento);
    config_destroy(config);
    free(buff);
    log_destroy(logger);
    close(sockcoordinador);
	 return 0;
}
void liberarListas(pthread_t id){
	dump = 0;
	log_info(logger,"Esperando a que se libere el DUMP");
    pthread_join(id,NULL);
	int i = 0;
	while(list_get(tablas,i)){
		liberarClave(i);
		i++;
	}
	list_destroy(tablas);
	i = 0;
	char* entrada;
	log_info(logger,"Se van a liberar las entradas");
	while((entrada = list_get(entradas,i))!=NULL){
		free(entrada);
		entrada = list_get(bitArray,i);
		free(entrada);
		i++;
	}
	list_destroy(entradas);
	list_destroy(bitArray);
}
void buscarYLiberarClave(char* clave){
	int posTabla = encontrarTablaConTalClave(clave);
	if(posTabla >=0){
		liberarClave(posTabla);
	}
}

algoritmo obtenerAlgoritmoDeReemplazo(){
	t_config *config=config_create(pathInstancia);
	log_info(logger,"Vamos a seleccionar un algoritmo");
	char* algoritmo = config_get_string_value(config,"AlgoritmoDeReemplazo");
	if(strcmp("CIRCULAR",algoritmo) == 0){
		config_destroy(config);
		log_info(loggerReal,"El algoritmo de reemplazo elegido fue: CIRCULAR");
		return &circular;
	}
	if(strcmp("LRU",algoritmo) == 0){
			config_destroy(config);
			log_info(loggerReal,"El algoritmo de reemplazo elegido fue: LRU");
			return &lru;
	}
	log_error(loggerReal,"No se reconocio el algoritmo de reemplazo");
	exit(-1);
	return NULL;
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
	log_info(loggerReal,"Se va realizar una compactacion");
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
    inicializarBitArray();
    free(buff);
    tam = obtenerTamDelSigBuffer(sockcoordinador);
    buff = malloc(tam);
    recv(sockcoordinador,buff, tam , 0);
    tamEntradas = transformarNumero(buff,0);
    log_info(loggerReal,"Se van a crear %d entradas de tamagno %d", entradasTotales,tamEntradas);
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
	while(dump){
		t_config *config=config_create(pathInstancia);
		sleep(config_get_int_value(config,"dump"));	//Permite la ejecucion de manera periodica del dump
		config_destroy(config);
		almacenarTodaInformacion();
		log_info(loggerReal,"Se realizo el dump");
	}
}

void almacenarInformacionDeTalPosicionDeLaTabla(int posTabla){
		//Garantizo mutua exclusion al ejecutar la seccion critica
	tablaEntradas* tabla = list_get(tablas,posTabla);
		if(!(*tabla).seAlmacenoElValor){
		log_trace(logger,"La posicion de la tabla %d",posTabla);
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
		//Garantizo mutua exclusion al ejecutar la seccion critica
	}
}

void almacenarTodaInformacion(){
	int i = 0;
	tablaEntradas* tabla = list_get(tablas,i);
	pthread_mutex_lock(&mutexAlmacenamiento);
	while(tabla && (*tabla).entrada !=NULL)
	{
		almacenarInformacionDeTalPosicionDeLaTabla(i);
		i++;
		tabla = list_get(tablas,i);
	}
	pthread_mutex_unlock(&mutexAlmacenamiento);
}

int encontrarTablaConTalClave(char clave[40]){
	int i=0;
	tablaEntradas* tabla = list_get(tablas,i);
	while(strcmp((*tabla).clave,clave)!=0)
	{
		i++;
		if((tabla=list_get(tablas,i))==NULL){
			return -1;
		}
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
	log_info(logger,"Se va liberar desde la posicion %d hasta %d",posEntrada,posEntrada+cantEntradasAOcupadas-1);
	char* valor = string_new();
	for(int i = 0;i<cantEntradasAOcupadas;i++){
		char* bit = list_get(bitArray,posEntrada+i);
		char* entrada = list_get(entradas,posEntrada+i);
		string_append(&valor,entrada);
		bit[0]='0'; //LIBERO :D
	}
	log_warning(loggerReal,"Se libero la clave %s",(*tabla).clave);
	free(valor);
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
	log_warning(logger,"LA CANTIDAD DE ENTRADAS QUE TENGO LIBRE SON: %d",cantidadEntradas);
	char* buff = string_itoa(cantidadEntradas);
	enviarCantBytes(sockcoordinador,buff);
	send(sockcoordinador,buff,strlen(buff)+1,0);
	free(buff);
	return;
}

char* encontrarTablaConTalEntrada(char* entrada){
	int i =0;
	tablaEntradas* tabla;
	while((tabla = list_get(tablas,i))!=NULL){
		if((*tabla).entrada == entrada){
			return (*tabla).clave;
		}
		i++;
	}
	return NULL;
}

void mostrarEstadoEntradas(){
	int i =0;
	char* bit;
	log_info(logger,"El estado de las entradas son: ");
	while((bit = list_get(bitArray,i))!=NULL){
		if(bit[0]-48){
			char* entrada = list_get(entradas,i);
			char* clave = encontrarTablaConTalEntrada(entrada);
			if(clave!=NULL){
				log_info(loggerReal,"Clave: %s",clave);
			}
			log_info(loggerReal,"Posicion: %d valor: %s",i,entrada);
		}
		i++;
	}
}

void manejarPaquete(t_esi_operacion paquete){
	int posTabla;
	char* resultado = "r";
	pthread_mutex_lock(&mutexAlmacenamiento);
	nroOperacion++;
	log_error(logger,"%d",nroOperacion);
	switch(paquete.keyword){
		case GET:
			log_info(logger,"Se selecciono el caso GET");
			log_info(loggerReal,"GET %s",paquete.argumentos.GET.clave);
			meterClaveALaTabla(paquete.argumentos.GET.clave);
			free(paquete.argumentos.GET.clave);
			break;
		case SET:
			log_info(logger,"Se selecciono el caso SET");
			log_info(loggerReal,"SET %s %s",paquete.argumentos.SET.clave,paquete.argumentos.SET.valor);
			posTabla = encontrarTablaConTalClave(paquete.argumentos.SET.clave);
			if(posTabla==-1){
				log_error(loggerReal,"No se puede realizar el SET ya que la clave no existe");
				resultado = "e";
				break;
			}
			tablaEntradas* tabla = list_get(tablas,posTabla);
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
				log_warning(loggerReal,"Ya tenia un valor asignado");
				log_info(logger,"La cantidad de entradas ocupadas son %d y las que necesita son %d",cantEntradasOcupadas,cantEntradasNecesarias);
				int posEntrada = posicionDeLaEntrada((*tabla).entrada);
				int j = 0;
				(*tabla).tamValor = strlen(paquete.argumentos.SET.valor);
				if(cantEntradasNecesarias>cantEntradasOcupadas){
					cantEntradasNecesarias = cantEntradasOcupadas;
					(*tabla).tamValor = tamEntradas*cantEntradasOcupadas;
				}
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
			}else{
				log_info(logger,"Estamos metiendo el valor %s de la clave %s",paquete.argumentos.SET.valor,paquete.argumentos.SET.clave);
				if(meterValorParTalClave(paquete.argumentos.SET.valor,tabla) ==-1){
					resultado = "a";
					log_error(loggerReal,"No hay espacio debido a que ningun valor es atomico");
					break;
				}
				(*tabla).tamValor = strlen(paquete.argumentos.SET.valor);
			}
			(*tabla).nroOperacion = nroOperacion;
			free(paquete.argumentos.SET.clave);
			free(paquete.argumentos.SET.valor);
			break;
		case STORE:
			log_info(logger,"Se selecciono el caso STORE");
			log_info(loggerReal,"STORE %s",paquete.argumentos.STORE.clave);
			if((posTabla = encontrarTablaConTalClave(paquete.argumentos.STORE.clave))==-1){
				log_error(loggerReal,"No se puede realizar el STORE ya que la clave no existe");
				resultado = "e";
				break;
			}
			almacenarInformacionDeTalPosicionDeLaTabla(posTabla);
			liberarClave(posTabla);
			free(paquete.argumentos.STORE.clave);
			break;
	}
	mostrarEstadoEntradas();
	pthread_mutex_unlock(&mutexAlmacenamiento);
	if(resultado[0]=='r'){
		log_info(loggerReal,"Se realizo correctamente la operacion!");
	}
	if(send(sockcoordinador,resultado,2,0)==-1)
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
	(*tabla).nroOperacion = 0;
	list_add(tablas,tabla);
}

int llegaAOcuparTodasLaEntradas(int* posicion,int cantEntradasAOcupar,int* cantidadEntradasLibres){
	int i = 1;
	while(i < cantEntradasAOcupar && (*posicion) < entradasTotales){
		char * bit = list_get(bitArray,(*posicion));
		if(bit[0]-48){
			return 0; //COMO EN LA SIGUIENTE POSICION ESTA OCUPADO HAY FRAGMENTACION EXTERNA, ESTO PROVOCA QUE HAY QUE COMPACTAR SI NO LLEGO A ENCONTRAR LUGAR
		}
		log_info(logger,"Posicion libre: %d entradas permitidas: %d",(*posicion),i);
		(*cantidadEntradasLibres)++;
		(*posicion)++;
		i++;
	}
	if(i == cantEntradasAOcupar){
		return 1; //TENGO ESPACIO PARA INGRESAR EL VALOR :D
	}
	return 0; //ESTO SIGNIFICA QUE YA ME LEI TODAS LAS ENTRADAS POR LO TANTO GG
}
int hayAlgunoAtomico(){
	int i=0;
	tablaEntradas* tabla;
	while((tabla = list_get(tablas,i))!=NULL){
		if(esAtomico(tabla)){
			return 1;
		}
		i++;
	}
	return 0;
}
int obtenerPrimeraPosicionPermitida(int cantEntradasAOcupar){
	int primeraPosicionEncontrada;
	while(1){
		int encontrado = 0;
		int cantEntradasLibres = 0;
		int posicion = 0;
		while(!encontrado && posicion < entradasTotales){
			char* bit = list_get(bitArray,posicion);
			if(!(bit[0]-48)){ //SI LA POSICION NO ESTA OCUPADA TENGO QUE VERIFICAR QUE LLEGA ENTRAR EL VALOR
				primeraPosicionEncontrada = posicion;
				cantEntradasLibres++;
				posicion ++;
				encontrado = llegaAOcuparTodasLaEntradas(&posicion,cantEntradasAOcupar,&cantEntradasLibres); //VERIFICO SI CUMPLE LA CANTIDAD DE ENTRADAS A METER Y SI HAY QUE COMPACTAR
			}
			posicion ++;
		}
		log_warning(logger,"El valor de encontrado es %d y la cantidad de entradas libre %d",encontrado,cantEntradasLibres);
		if(encontrado){
			return primeraPosicionEncontrada;
		}else if(cantEntradasAOcupar > cantEntradasLibres){
			if(!hayAlgunoAtomico()){
				return -1;
			}
			log_warning(loggerReal,"Se va realizar el algoritmo de reemplazo");
			algoritmoDeReemplazo();
		}else{
			log_warning(loggerReal,"Se va solicitar compactacion simultanea al Coordinador");
			send(sockcoordinador,"c",2,0); //ENVIO UNA SOLICITUD DE COMPACTACION AL COORDINADOR
			compactacion();
		}
	}
}

int meterValorParTalClave(char*valor,tablaEntradas* tablaEntrada){
	int cantEntradasAOcupar = (string_length(valor))/tamEntradas;
	log_info(logger,"La cantidad de entradas a ocupar son: %d",cantEntradasAOcupar);
	if(string_length(valor)%tamEntradas){
		cantEntradasAOcupar++;
	}
	int posicionPermitidaParaOcupar = obtenerPrimeraPosicionPermitida(cantEntradasAOcupar);
	if(posicionPermitidaParaOcupar != -1){
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
	return posicionPermitidaParaOcupar;
}
int esAtomico(tablaEntradas* tabla){
	int cantidadEntradas = (*tabla).tamValor/tamEntradas;
	if((*tabla).tamValor%tamEntradas){
		cantidadEntradas++;
	}
	return cantidadEntradas==1;
}
int obtenerTablaDeTalEntrada(char* entrada){
	int posTabla=0;
	tablaEntradas* tabla;
	while((tabla = list_get(tablas,posTabla))!=NULL){
		if((*tabla).entrada == entrada){
			log_info(logger,"Encontre el valor xd que es: %s con la posicion %d", (*tabla).entrada,posTabla);
			if(!esAtomico(tabla)){
				log_warning(logger,"Pero no es atomico D:");
				break;
			}
			return posTabla;
		}
		posTabla++;
	}
	return -1;
}

void circular(){
	int posTabla;
	if(nroEntrada >= entradasTotales){ //SI SUPERE A LA CANTIDAD DE ENTRADAS TOTALES REINICIO
		nroEntrada = 0;
	}
	while(((posTabla=obtenerTablaDeTalEntrada(list_get(entradas,nroEntrada)))<0)){
		nroEntrada++;
		if(nroEntrada >= entradasTotales){ //SI SUPERE A LA CANTIDAD DE ENTRADAS TOTALES REINICIO
			nroEntrada = 0;
		}
	}
	tablaEntradas* tabla = list_get(tablas,posTabla);
	log_warning(loggerReal,"La victima es: %s",(*tabla).clave);
	nroEntrada++;
	liberarClave(posTabla);
}

int buscarTablaMenosUsada(){
	int i =0;
	tablaEntradas* tabla=NULL;
	tablaEntradas*tablaMenosUsada = NULL;
	while(((tablaMenosUsada= list_get(tablas,i))!=NULL) && (*tablaMenosUsada).entrada == NULL && !esAtomico(tablaMenosUsada)){
		i++;
	}
	int posTabla= i;
	while((tabla = list_get(tablas,i))!=NULL){
		log_info(logger,"Nro operacion = %d vs %d y su valor es: %s",(*tablaMenosUsada).nroOperacion,(*tabla).nroOperacion, (*tabla).entrada?(*tabla).entrada:"NULL");
		if(((*tabla).entrada !=NULL) && esAtomico(tabla) && ((*tablaMenosUsada).nroOperacion > (*tabla).nroOperacion)){
			tablaMenosUsada = tabla;
			posTabla = i;
		}
		i++;
	}
	log_warning(loggerReal,"La victima es: %s",(*tabla).clave);
	return posTabla;
}

void lru(){
	int posTabla= buscarTablaMenosUsada();
	liberarClave(posTabla);
}

void enviarValor(char* clave){
	int posTabla = encontrarTablaConTalClave(clave);
	tablaEntradas* tabla= list_get(tablas,posTabla);
	char* valor = string_new();
	if((*tabla).entrada){
		int cantidadEntradasALeer = (*tabla).tamValor/tamEntradas;
		if((*tabla).tamValor%tamEntradas){
			cantidadEntradasALeer++;
		}
		int posEntrada = posicionDeLaEntrada((*tabla).entrada);
		for(int j = 0; j<cantidadEntradasALeer; j++){
			char* entrada = list_get(entradas,posEntrada);
			string_append(&valor,entrada);
			posEntrada++;
		}
	}else{
		string_append(&valor,"No tiene valor");
		log_warning(loggerReal,"Valor no encontrado");
	}
	enviarCantBytes(sockcoordinador,valor);
	send(sockcoordinador,valor,strlen(valor)+1,0);
	free(valor);
}

