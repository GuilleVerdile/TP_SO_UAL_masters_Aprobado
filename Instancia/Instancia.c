#include "Instancia.h";

struct TE{
	char clave[40]; //Clave xd
	char** entradas; //Direcciones de memoria de las entradas de la clave
	int tamValor; //Tamanio del valor asociado a la clave
}typedef tablaEntradas;

tablaEntradas *tablas =NULL;
int cantEntradasDisponibles;
int tamEntradas;
char** entradas;

int main(){
	logger =log_create(logInstancias,"Instancia",1, LOG_LEVEL_INFO);
	int sockcoordinador;
    if((sockcoordinador =crearConexionCliente("/home/utnso/git/tp-2018-1c-UAL-masters/Config/Instancia.cfg")) == -1){
    	log_error(logger,"Error en la conexion con el coordinador");
    }
    log_info(logger,"Se realizo correctamente la conexion con el coordinador");
    enviarTipoDeCliente(sockcoordinador,INSTANCIA);
    inicializarTablaEntradas(sockcoordinador);
    t_config *config=config_create("/home/utnso/git/tp-2018-1c-UAL-masters/Config/Instancia.cfg");
    char *buff = config_get_string_value(config, "nombreInstancia"); //obtengo el id
    enviarCantBytes(sockcoordinador,buff);
    send(sockcoordinador,buff,string_length(buff) + 1,0); //ENVIO EL NOMBRE DE LA INSTANCIA
    config_destroy(config); //NO HACE FALTA HACER FREE AL BUFF YA QUE EL CONFIG DESTROY LO HACE SOLO
    int recvValor;
    buff = malloc(2);
    t_esi_operacion paquete;
    while((recvValor =recv(sockcoordinador,buff,2,0))>0){
    	switch(buff[0]){
    		case 'r': //RECONEXION LE PIDO AL COORDINADOR CUALES FUERON LAS CLAVES BLOQUEADAS EN ESTA INSTANCIA
    			break;
    		case 'p': //RECIBI UN PAQUETE GET SET O STORE
    			if((recvValor = recibir(sockcoordinador,&paquete))<=0){
    				free(buff);
    				exit(-1);
    			}
    			manejarPaquete(paquete);
    			break;
    		case 'v':
    			send(sockcoordinador,"v",2,1); //LE DIGO AL COORDINADOR QUE SIGO VIVO
    			break;
    	}
   }
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
    	entradas[i] = string_new(); //INICIALIZO CADA ENTRADA CON ""
    	i++;
    }
    free(buff);
}

void manejarPaquete(t_esi_operacion paquete){
	switch(paquete.keyword){
		case GET:
			meterClaveALaTabla(paquete.argumentos.GET.clave);
			break;
		case SET:
			if(cantEntradasDisponibles < 0){
					//ALGORITMO DE REEMPLAZO
			}
			else{
				meterValorParTalClave(paquete.argumentos.SET.clave,paquete.argumentos.SET.valor);
			}
			break;
		case STORE:
			break;
	}
}

void meterClaveALaTabla(char clave[40]){
	int i = 0;
	if(tablas == NULL){
		tablas = malloc(sizeof(tablaEntradas));
		(&tablas)[1] = NULL;
		strcpy(tablas[0].clave, clave);
	}
	else{
		while(&tablas[i] !=NULL){
			i++;
		}
		tablas = realloc(tablas,sizeof(tablaEntradas)*(i+1));
		strcpy(tablas[i].clave,clave);
		(&tablas)[i+1] = NULL;
	}
	tablas[i].entradas = NULL;
}

void meterValorParTalClave(char clave[40], char*valor){
	int i=0;
	while(strcmp(tablas[i].clave,clave)!=0){
		i++;
	}
	int valorAux = string_length(valor) + 1;
	tablas[i].tamValor = valorAux; //+1 POR EL \0
	int k =0;
	while(strcmp(entradas[k],"") != 0){
		k++; //BUSCO HASTA ENCONTRAR UN NULL
	}
	int j =0;
	while(valorAux>0 && cantEntradasDisponibles > 0){ //SI YA METI TODO EL VALOR O NO ME QUEDA ENTRADAS ME SALGO DE LA ITERACION
		entradas[k] = malloc(tamEntradas); //LE ASIGNO MEMORIA A LA ENTRADA NULL
		strcpy(entradas[k],string_substring(valor,tamEntradas*j,tamEntradas*(j+1))); //LE ASIGNO EL VALOR
		if(tablas[i].entradas ==NULL){
			tablas[i].entradas = malloc(sizeof(char*));
			tablas[i].entradas[0]=NULL;
		}
		if(tablas[i].entradas[j] == NULL){
			tablas[i].entradas[j] = entradas[k]; //LE ASIGNO LA DIRECCION DE MEMORIA DE LA ENTRADA
			tablas[i].entradas[j+1] = NULL;
		}
		cantEntradasDisponibles --; //LA CANTIDAD DE ENTRADAS SE RESTAN POR CADA ENTRADA USADA
		valorAux-= tamEntradas; //LE RESTO AL VALOR POR LA CANTIDAD DE ENTRADAS QUE CONSUMIO
		k++;
		j++;
	}
	if(cantEntradasDisponibles == 0 && valorAux>0){
		//ALGORITMO DE REEMPLAZO
	}
	//TAL VEZ TENGO QUE TRABAJAR CON HILOS ACA POR EL DUMP
}
