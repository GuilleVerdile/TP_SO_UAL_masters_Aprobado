#include "Instancia.h";

struct TE{
	char clave[40]; //Clave
	char** entradas; //Direcciones de memoria de las entradas de la clave
	int tamValor; //Tamanio del valor asociado a la clave
}typedef tablaEntradas;

t_list* tablas;
int entradasTotales;
int cantEntradasDisponibles;
int tamEntradas;
char** entradas;
int nroReemplazo;
pthread_mutex_t mutexAlmacenamiento;
char* path;
int main(){
    logger =log_create(logInstancias,"Instancia",1, LOG_LEVEL_INFO);
    int sockcoordinador;
    int nroReemplazo = 0;
    t_config* config = config_create(pathInstancia);
    if((sockcoordinador =crearConexionCliente(config_get_int_value(config,"Puerto"),config_get_string_value(config,"Ip"))) == -1){
    	config_destroy(config);
    	log_error(logger,"Error en la conexion con el coordinador");
    	return -1;
    }
    log_info(logger,"Se realizo correctamente la conexion con el coordinador");
    enviarTipoDeCliente(sockcoordinador,INSTANCIA);
    inicializarTablaEntradas(sockcoordinador);
    char *buff = config_get_string_value(config, "nombreInstancia"); //obtengo el id
    enviarCantBytes(sockcoordinador,buff);
    send(sockcoordinador,buff,string_length(buff) + 1,0); //ENVIO EL NOMBRE DE LA INSTANCIA
    config_destroy(config); //NO HACE FALTA HACER FREE AL BUFF YA QUE EL CONFIG DESTROY LO HACE SOLO
    config = config_create(pathInstancia);
    path = config_get_string_value(config,"PuntoMontaje");
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
    			if((recvValor = recibir(sockcoordinador,&paquete))<=0){
    				free(buff);
    				exit(-1);
    			}
    			manejarPaquete(paquete,sockcoordinador);
    			break;
    		case 'v':
    			send(sockcoordinador,"v",2,1); //LE DIGO AL COORDINADOR QUE SIGO VIVO
    			break;
    	}
   }
    config_destroy(config);
    free(buff);
    log_destroy(logger);
    close(sockcoordinador);
	 return 0;
}

void inicializarTablaEntradas(int sockcoordinador){
    int tam = obtenerTamDelSigBuffer(sockcoordinador);
    char* buff = malloc(tam);
    recv(sockcoordinador,buff, tam , 0);
    cantEntradasDisponibles = transformarNumero(buff,0);
    entradasTotales = cantEntradasDisponibles;
    log_info(logger,"La cantidad de entradas es %d", cantEntradasDisponibles);
    entradas = malloc(sizeof(char*)*cantEntradasDisponibles); //INICIALIZO LA TABLA
    free(buff);
    tam = obtenerTamDelSigBuffer(sockcoordinador);
    buff = malloc(tam);
    recv(sockcoordinador,buff, tam , 0);
    tamEntradas = transformarNumero(buff,0);
    log_info(logger,"El tamagno de entradas es %d", tamEntradas);
    int i = 0;
    while(i != (cantEntradasDisponibles-1)){
    	entradas[i] = malloc(tamEntradas);	 //Asigno un espacio de memoria para cada fila de la tabla de entradas
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
	pthread_mutex_lock(&mutexAlmacenamiento);	//Garantizo mutua exclusion al ejecutar la seccion critica
	char* aux = string_new();
	string_append(&aux,path);
	tablaEntradas* tabla = list_get(tablas,posTabla);
	char* valor = string_new();
	int cantidadEntradasALeer = (*tabla).tamValor/tamEntradas;
	for(int j = 0; j==cantidadEntradasALeer; j++){
		string_append(&valor, (*tabla).entradas[j]);
		j++;
	}
	string_append(&aux,(*tabla).clave);
	int desc = open(aux, O_RDWR | O_CREAT | O_TRUNC, 0777);
	free(aux);
	ftruncate(desc,strlen(valor));
	char* map = mmap(NULL,strlen(valor),PROT_WRITE,MAP_SHARED,desc,0);
	memcpy(map,valor,strlen(valor));
	munmap(map,strlen(valor));
	close(desc);
	free(valor);
	pthread_mutex_unlock(&mutexAlmacenamiento); //Garantizo mutua exclusion al ejecutar la seccion critica
}

void almacenarTodaInformacion(){
	int i = 0;
	tablaEntradas* tabla = list_get(tablas,i);
	while(tabla && (*tabla).entradas !=NULL)
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
	}
	return i;
}

void liberarClave(int posTabla){
	tablaEntradas* tabla = list_remove(tablas,posTabla);
	free((*tabla).entradas);
	free(tabla);
}

void manejarPaquete(t_esi_operacion paquete, int sockcoordinador){
	int posTabla;
	switch(paquete.keyword){
		case GET:
			meterClaveALaTabla(paquete.argumentos.GET.clave);
			free(paquete.argumentos.GET.clave);
			break;
		case SET:
			posTabla = encontrarTablaConTalClave(paquete.argumentos.SET.clave);
			tablaEntradas* tabla = list_get(tablas,posTabla);
			pthread_mutex_lock(&mutexAlmacenamiento);
			if((*tabla).entradas!=NULL){ //ME FIJO SI YA TENIA UN VALOR ASIGNADO
				free((*tabla).entradas); //LIBERO LA ENTRADAS
				(*tabla).entradas = NULL;
				(*tabla).tamValor = 0;
			}
			(*tabla).tamValor = string_length(paquete.argumentos.SET.valor) + 1;
			if(cantEntradasDisponibles == 0)
			{
				//ALGORITMO DE REEMPLAZO
			}
			else
			{
				meterValorParTalClave(paquete.argumentos.SET.clave,paquete.argumentos.SET.valor,posTabla);
			}
			pthread_mutex_unlock(&mutexAlmacenamiento);
			free(paquete.argumentos.SET.clave);
			free(paquete.argumentos.SET.valor);
			break;
		case STORE:
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
	}
}

void meterClaveALaTabla(char* clave){
	tablaEntradas* tabla = malloc(sizeof(tablaEntradas));
	strcpy((*tabla).clave,clave);
	(*tabla).entradas = NULL;
	(*tabla).tamValor = 0;
	list_add(tablas,tabla);
	char* pathCompleto = malloc(strlen(path)+1);
	strcpy(pathCompleto,path);
	string_append(&pathCompleto,clave);
	int desc = open(pathCompleto, O_CREAT | O_TRUNC,0777); //CREA EL ARCHIVO
	free(pathCompleto);
	close(desc);
}

void meterValorParTalClave(char clave[40], char*valor,int posTabla){
	int j =0; //ME INDICA LA CANTIDAD DE ENTRADAS QUE ASIGNO AL VALOR
	tablaEntradas* tabla = list_get(tablas,posTabla);
	while((*tabla).tamValor - (tamEntradas * j)>0 && cantEntradasDisponibles > 0) //SI YA METI TODO EL VALOR O NO ME QUEDA ENTRADAS ME SALGO DE LA ITERACION
	{ 
		char* valorEntrada = string_substring(valor,tamEntradas*j,tamEntradas*(j+1));
		strcpy(entradas[entradasTotales-cantEntradasDisponibles],valorEntrada); //LE ASIGNO EL VALOR
		free(valorEntrada);
		(*tabla).entradas = realloc((*tabla).entradas,sizeof(char*)*(j+1));
		(*tabla).entradas[j] = entradas[entradasTotales-cantEntradasDisponibles]; //LE ASIGNO LA DIRECCION DE MEMORIA DE LA ENTRADA
		cantEntradasDisponibles --; //LA CANTIDAD DE ENTRADAS SE RESTAN POR CADA ENTRADA USADA
		j++;
	}

	if(cantEntradasDisponibles == 0 && (*tabla).tamValor - (tamEntradas * j)>0 ) //EL VALOR TOTAL - LA CANTIDAD DE ENTRADAS QUE LE QUITE * TAM ENTRADAS
	{ 
		char* valorRestante = string_substring_from(valor,tamEntradas*j);
		circular(clave,valorRestante,posTabla); //LE ENVIO LA CLAVE Y LO QUE SOBRO DEL VALOR
		free(valorRestante);
	}
}

void circular(char clave[40],char* valor, int posTabla){
	int valorAux = string_length(valor) + 1;
	int j = 0;
	int posEntrada = 0; //ESTA VARIABLE ME SIRVE PARA SABER SI PARTE DEL VALOR YA SE ASIGNO
	tablaEntradas* tabla = list_get(tablas,posTabla);
	if((*tabla).entradas !=NULL)
	{
		while((*tabla).entradas[posEntrada]!=NULL){

			posEntrada++;
		}
	}
	while(valorAux>0)
	{
		if(nroReemplazo == cantEntradasDisponibles){ //SI EL ALGORITMO CIRCULAR LLEGO A LA ULTIMA POSICION DE LAS ENTRADAS
			nroReemplazo = 0; //SE REINICIA
		}
		char* valorEntrada = string_substring(valor,tamEntradas*j,tamEntradas*(j+1));
		strcpy(entradas[nroReemplazo],valorEntrada);
		free(valorEntrada);
		(*tabla).entradas = realloc((*tabla).entradas,sizeof(char*)*(posEntrada+1));
		(*tabla).entradas[posEntrada] = entradas[nroReemplazo];
		posEntrada++;
		valorAux -= tamEntradas;
		nroReemplazo++; //ME POSICIONO A LA SIGUIENTE ENTRADA PARA REEMPLAZO
		j++;
	}
}
