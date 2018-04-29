/*
 * FuncionesConexiones.c
 *
 *  Created on: 17 abr. 2018
 *      Author: utnso
 */
#include "FuncionesConexiones.h"

const char* ESI = "e";
const char* INSTANCIA = "i";

//Path de los servidores
const char *pathCoordinador="/home/utnso/git/tp-2018-1c-UAL-masters/Config/Coordinador.cfg";
const char *pathPlanificador="/home/utnso/git/tp-2018-1c-UAL-masters/Config/Planificador.cfg";

//Los pongo en escritorio para que no tengamos problemas al commitear
const char *logCoordinador="/home/utnso/Escritorio/Coordinador.log";
const char *logPlanificador="/home/utnso/Escritorio/Planificador.log";
const char *logESI="/home/utnso/Escritorio/ESI.log";
const char *logConsola="/home/utnso/Escritorio/Consola.log";
const char *logInstancias="/home/utnso/Escritorio/Instancia.log";

struct sockaddr_in dameUnaDireccion(char *path,int ipAutomatica){
	t_config *config=config_create(path);
	struct sockaddr_in midireccion;
	midireccion.sin_family = AF_INET;
	midireccion.sin_port = htons(config_get_int_value(config, "Puerto"));
	if(ipAutomatica){
		midireccion.sin_addr.s_addr = INADDR_ANY;
	}
	else{
		midireccion.sin_addr.s_addr = inet_addr(config_get_string_value(config,"Ip"));
	}
	config_destroy(config);
	return midireccion;
}
// estas Funciones retornan -1 en caso de error hay que ver despues como manejarlas
int crearConexionCliente(char*path){//retorna el descriptor de fichero
	int sock=socket(AF_INET, SOCK_STREAM, 0);
	if(sock<0){
	return -1;// CASO DE ERROR
	}
	struct sockaddr_in midireccion=dameUnaDireccion(path,0);
	memset(&midireccion.sin_zero, '\0', 8);
	if(connect(sock, (struct sockaddr *)&midireccion, sizeof(struct sockaddr))<0){
		return -1;
	}
	return sock;
}
int crearConexionServidor(char*path){//Retorna el sock del Servidor
	int sockYoServidor=socket(AF_INET, SOCK_STREAM, 0);// no lo retorno me sierve para la creacion del servidor
	if(sockYoServidor<0){
		printf("Error, no se pudo crear el socket\n");
		return -1;// CASO DE ERROR
	}
	struct sockaddr_in miDireccion=dameUnaDireccion(path,0);
	memset(&miDireccion.sin_zero, '\0', 8);
	struct sockaddr_in their_addr;//Aca queda almacenada la informacion del cliente
	int activado = 1;
    if(setsockopt(sockYoServidor, SOL_SOCKET,SO_REUSEADDR,&activado,sizeof(activado)) == -1){
    	printf("Error en la funcion setsockopt");
    	return -1;
    };
    if((bind(sockYoServidor, (struct sockaddr *)&miDireccion, sizeof(struct sockaddr)))<0){
    	printf("Error, no se pudo hacer el bind al puerto\n");
    	return -1;
    }
    return sockYoServidor;
}


//1024
int transformarNumero(char *a,int start){
	int tam=string_length(a);
	int resultado=0;
	for(int i=0;i<tam;i++){
		resultado=(a[i+start]-48)+resultado*10;
	}
	return resultado;
}
void deserializacion(char* texto, int* tipo, char clave[40], char** valor){
	*tipo = texto[0] -48;
	char* claveALiberar;
	if(!*tipo){
		int tam = ((texto[1])-48)*10 + texto[2]-48;
		claveALiberar =string_substring(texto,3,tam);

		*valor = string_substring_from(texto,tam+3);
	}
	else{
		claveALiberar = string_substring_from(texto,1);
	}
	strcpy(clave,claveALiberar);
	free(claveALiberar);
}

int recibir(int socket, Paquete* pack){
	char *total= string_new();
	char *buff=NULL;
	buff = malloc(5);
	int recvValor;
	char *aux = NULL;
	while(1){
		recvValor = recv(socket, buff, 5, 0);
		if(recvValor < 1){ //Se verifica si fallo el recv o el cliente se desconecto
			free(total);
			free(buff);
			return recvValor;
		}
		if(string_contains(buff, "z")){
			aux =malloc(5);
			strcpy(aux,buff);
			aux[string_length(buff)-1]='\0';
			string_append(&total,aux);
			free(aux);
			break;
			}
		string_append(&total, buff);
	}
	free(buff);
	int tot=transformarNumero(total,0);
	free(total);
	char* buf=malloc(tot);
	recvValor = recv(socket,buf,tot,0);
	if(recvValor <1){  //Se verifica si fallo el recv o el cliente se desconecto
		free(buf);
		return recvValor;
	}
	deserializacion(buf, &(*pack).a, (*pack).key, &(*pack).value);
	free(buf);
	return 1; //No hubo problema en recibir
}
//Funciones ESI
char* transformarTamagnoKey(char key[]){
	int tam=string_length(key);
	if(tam<10){
		char* tamKey = malloc(3);
		strcpy(tamKey,"0");
		char* tam1 = string_itoa(tam);
		string_append(&tamKey,tam1);
		free(tam1);
		return tamKey;
	}
	else
		return string_itoa(tam);
}
void serealizarPaquete(t_esi_operacion operacion,char** buff){

	switch(operacion.keyword){
		case SET:
			*buff = string_itoa(0);// 0 es SET
			char* tamkey = transformarTamagnoKey(operacion.argumentos.SET.clave);
			string_append(*buff,tamkey);
			string_append(*buff, operacion.argumentos.SET.clave);
			string_append(*buff, operacion.argumentos.SET.valor);
			free(tamkey);
			break;
		case GET:
			*buff = string_itoa(1);// 1 es GET
			string_append(*buff,operacion.argumentos.GET.clave);
			break;
		case STORE:
			*buff = string_itoa(2);// 2 es STORE
			string_append(*buff,operacion.argumentos.STORE.clave);
			break;
		default:
			log_error(logger, "No se entendio el comando");
			exit(-1);
	}
}

void enviar(int socket,t_esi_operacion operacion){
	int i =0;
	char *enviar;
	char *buff;
	serealizarPaquete(operacion,&buff);
	char *cantBytes=string_itoa(string_length(buff)+1);
	string_append(&cantBytes, "z");



	while(i<string_length(cantBytes)){
		enviar =string_substring(cantBytes, i, 4);
		send(socket,enviar,5,0);
		i=i+4;
		free(enviar);
	}
	send(socket,buff,string_length(buff)+1,0);
	free(cantBytes);
	free(buff);
}

void enviarTipoDeCliente(int socket,char* tipo){
	if(send(socket,tipo,2,0)<0){
		log_error(logger,"Se produjo un error al enviar el tipo de cliente");
		exit(-1);
	}
}
